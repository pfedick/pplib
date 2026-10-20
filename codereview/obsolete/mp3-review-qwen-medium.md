# MP3 Review – Befunde

Review vom 2026-10-20, Scope: `src/audio/Mp3.cpp` (332 Zeilen), `include/pplib/audio/mp3.h` (73 Zeilen).
Öffentliche API: `pplib::IdentMPEG()`, `pplib::GetMP3Frame()`, `PPL_MPEG_HEADER`
Review done by: Qwen3.8-Flash-Next

## Bugs (kritisch)

- [ ] **`IdentMPEG` crasht/throwt bei kleinen Dateien** (`src/audio/Mp3.cpp:87`)
  `file.map(0, 1024)` wird ohne Größenprüfung und ohne Exception-Handling aufgerufen.
  `File::map` (src/core/File.cpp:750) verhält sich so:
  - Leere Datei (`mysize == 0`): Rückgabe `nullptr` → anschließendes `buffer[0]` (Zeile 97) ist ein Nullpointer-Dereference → Crash.
  - Datei zwischen 1 und 1023 Bytes: Bedingung `bytes <= mysize - position` verletzt → `OverflowException` wird geworfen und ist nicht gefangen → IdentMPEG terminiert mit Exception statt `false`.
  Reproduktion: `IdentMPEG` auf eine 500-Byte- oder 0-Byte-Datei aufrufen (z. B. über `AudioInfo::IdentAudioFile`).
  Behebung:
  ```cpp
  uint64_t maplen = file.size();
  if (maplen > 1024) maplen = 1024;
  if (maplen < 10) return false;
  buffer = (unsigned char*)file.map(0, (size_t)maplen);
  if (!buffer) return false;
  ```

- [ ] **Falsche Framesize-Formel in `IdentMPEG`: Padding im Nenner** (`src/audio/Mp3.cpp:132`, `:134`)
  ```cpp
  mpg->framesize = 144 * mpg->bitrate * 1000 / (mpg->frequency + mpg->padding);
  ```
  Das Padding-Bit gehört zum Ergebnis, nicht zum Divisor. Richtig (so auch korrekt in `FindNextHeader`, Zeile 235/237):
  ```cpp
  mpg->framesize = 144 * mpg->bitrate * 1000 / mpg->frequency + mpg->padding;
  ```
  Auswirkung: Bei gepaddeten Frames (häufig bei 44,1 kHz/128 kbit) ist `framesize` um 1 Byte zu klein → `frames` und damit die CBR-Längenschätzung driften über die Dateilänge an.

- [ ] **Debug-Rest `buffer.hexDump()` in `GetMP3Frame`** (`src/audio/Mp3.cpp:166`)
  Jeder gelesene MP3-Frame wird als Hexdump auf die Konsole ausgegeben. In einer Bibliotheksfunktion inakzeptabel (Datenleak auf stdout, massiver Performance-Schaden beim Frame-für-Frame-Lesen). Zeile entfernen.

## Bugs (mittel / Warnungen)

- [ ] **Unsigned-Underflow bei ID3v1-TAG-Suche** (`src/audio/Mp3.cpp:119`, `:123`)
  `file.map(mpg->end - 127, 127)`: `mpg->end` ist `uint64_t`. Für Dateien zwischen ca. 16 und 127 Bytes (FindNextHeader akzeptiert ab 16 Bytes) unterläuft `end - 127` → riesige Position → `OverflowException` in `File::map`. Absicherung: Ausführen der TAG-Prüfung nur, wenn `mpg->end >= 128`.

- [ ] **`GetMP3Frame`: unbehandelte `EndOfFileException`** (`src/audio/Mp3.cpp:165`)
  `file.read(buffer, mpg.framesize)` wirft laut Doku (include/pplib/core/fileobject.h:217) eine Exception bei Dateiende. Bei abgeschnittenen Dateien (letzter Frame unvollständig) crasht der Aufrufer. Besser: verbleibende Bytes prüfen (`file.size() - pos`) oder try/catch mit `buffer.clear()`.

- [ ] **`GetMP3Frame` setzt vorinitialisiertes `mpg.end`/`mpg.filesize` voraus (undokumentiert)** (`src/audio/Mp3.cpp:160`, via `FindNextHeader` `:216-218`)
  `FindNextHeader` berechnet `mpg->size = mpg->end - mpg->start + 1`. Übergibt der Caller ein frisch mit 0 initialisiertes Struct (nahe liegend, z. B. nach dem Fehlerpfad in `GetMP3Frame` selbst, der `memset` ausführt), unterläuft `0 - pos + 1`, wird dann auf `mpg->filesize == 0` geklemmt → `size`, `frames`, `mslength`, `samples` sind alle 0. Entweder in `GetMP3Frame` selbst `mpg.end = file.size()-1; mpg.filesize = file.size();` setzen, oder die Vorbedingung in der Header-Doku klar benennen.

- [ ] **`CheckVBR`: `memcpy` überschreibt erkanntes VBR-Flag** (`src/audio/Mp3.cpp:310`)
  Im Ast „Layer/Version-Wechsel“ wird das komplette Struct per `memcpy(mpg, &m, ...)` zurückkopiert, inkl. `m.vbr` (von `FindNextHeader` immer auf `false` gesetzt). Ein zuvor gesetztes `mpg->vbr = true` geht dabei verloren; bleiben die folgenden Frames konstant, wird die Datei fälschlich als CBR klassifiziert. Behebung: Flag sichern und nach dem memcpy wiederherstellen, oder nur die benötigten Felder kopieren.

- [ ] **`CheckVBR`: Samples/Frame für MPEG-2/2.5 falsch** (`src/audio/Mp3.cpp:320-323`)
  MPEG-2 und MPEG-2.5 Layer III haben 576 Samples pro Frame, nicht 1152. Für VBR-MP3s mit `version != 1` ist die berechnete Länge (`samples`, `mslength`, `length`) damit um Faktor 2 zu groß. Zudem ist der Ast `layer == 1 → 384` toter Code, da `FindNextHeader` Layer I grundsätzlich verwirft (`layer >= 2 && layer <= 3`).
  ```cpp
  int spf = (mpg->version == 1) ? 1152 : 576;
  mpg->samples = frames * spf;
  ```

- [ ] **`FindNextHeader`: Off-by-one am Dateiende** (`src/audio/Mp3.cpp:192`)
  `while (pos + p + 4 < filesize)` prüft einen Header exakt in den letzten 4 Bytes (`pos+p+4 == filesize`) nie. Zusätzlich verwirft `if (mapsize < 16) return -1;` (Zeile 180 und 253) jeden Header in den letzten 4–15 Bytes. Korrekt wäre `<= filesize` und eine Mindest-Mapsize von 4. Praktische Auswirkung gering, aber nicht korrekt.

- [ ] **ID3v2-Erkennung unvollständig** (`src/audio/Mp3.cpp:97-98`)
  Die Bedingung prüft `buffer[3] < 0xff && buffer[4] < 0xff`, aber nicht, dass in Byte 3 die Versionsbits != 0 sind (reservierter Wert), und Byte 5 (Flags) fehlt ganz. Fehlalarme („ID3“ in Binärdaten, z. B. in eingebetteten Cover-Bildern) werden nicht sicher ausgeschlossen. Empfehlung: mindestens `(buffer[3] & 0xE0) != 0` ergänzen.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **`setMapReadAhead(1 MB)` ohne Rücksetzung** (`src/audio/Mp3.cpp:288`)
  `CheckVBR` verändert dauerhaften Zustand des übergebenen `FileObject` (1-MB-Read-Ahead bleibt danach aktiv). Vorher Wert sichern und am Ende wiederherstellen, oder Verhalten dokumentieren.

- [ ] **Tote Zuweisung** (`src/audio/Mp3.cpp:315`)
  `pos = 0;` direkt vor dem `if (mpg->vbr == true)`-Block wird nie gelesen. Entfernen.

- [ ] **`memset` auf Struct → Wertinitialisierung** (`src/audio/Mp3.cpp:79`, `:163`, `:272`)
  C++20-Idiom: `*mpg = {};` bzw. `m = {};` statt `memset`.

- [ ] **Makros und nicht-constexpr Tabellen** (`src/audio/Mp3.cpp:43-49`, `:56`, `:60`, `:66`)
  `#define MPG_MD_*` → `enum` oder `constexpr int`. `MPEGVersion`, `MP3Bitrates`, `s_freq` → `static constexpr` (liegt dann in `.rodata`, compileroptimierbar).

- [ ] **C-Style-Cast** (`src/audio/Mp3.cpp:327`)
  `(uint32_t)(avgbitrate / frames)` → `static_cast<int>(...)`; das Struct-Feld `bitrate` ist `int`.

- [ ] **Operatorpräzedenz ohne Klammern** (`src/audio/Mp3.cpp:207`)
  `MPEGVersion[(header[1] >> 3 & 0x3)]` funktioniert wegen `>>` vor `&`, ist aber leicht falsch zu lesen → `(header[1] >> 3) & 0x3`.

- [ ] **VBR-Heuristik bricht nach 50 Frames ab** (`src/audio/Mp3.cpp:300`)
  Beginnt eine VBR-Datei mit >50 Frames identischer Bitrate, wird sie als CBR klassifiziert. Bewusste Performance-Entscheidung — sollte zumindest als Kommentar/Doku festgehalten sein.

## Dokumentation & Doxygen

- [ ] **`IdentMPEG` und `GetMP3Frame` ohne Doxygen im Header** (`include/pplib/audio/mp3.h:67`, `:69`)
  Beide öffentlichen Funktionen haben im Header keinerlei Doku → kein IntelliSense-Popup. Parameter, Rückgabewert, Exception-Verhalten (`map`-Exceptions) und die Vorbedingung von `GetMP3Frame` (`mpg.end`/`mpg.filesize` pflegen) dokumentieren.

- [ ] **`!\ingroup PPLGroupSound` wirkt nicht** (`src/audio/Mp3.cpp:71-72`)
  Der Doxygen-Kommentar steht *nach* der Signatur von `IdentMPEG` und wird nicht als Member-Doku erkannt. In den Header vor die Deklaration ziehen.

- [ ] **Struct-Felder ohne Einheiten-Doku** (`include/pplib/audio/mp3.h:40-63`)
  Nur `mslength`/`length` haben Zeilenkommentare. Einheiten sind essentiell: `bitrate` in kbit/s, `frequency` in Hz, `framesize` in Bytes, `version`-Werte (1=MPEG-1, 2=MPEG-2, 3=MPEG-2.5), `mode`/`mode_ext`-Bedeutung. Als `//!<`-Member-Doku nachtragen.

## Befunde in Drittdateien

- [ ] **`AudioInfo` fängt keine Exceptions von `IdentMPEG`** (`src/audio/AudioInfo.cpp:235`, `:258`)
  `IdentAudioFile` ruft `IdentMPEG` ohne try/catch auf; der `OverflowException`-Pfad aus Befund 1 (Dateien < 1024 Bytes) schlägt bis zum Endanwender durch. Nach Fix in `IdentMPEG` erledigt sich das, defensiv wäre ein Catch trotzdem sinnvoll.

- [ ] **`File::map`-Vertrag inkonsistent** (`src/core/File.cpp:750-770`)
  Die Header-Doku (include/pplib/core/file.h:616) sagt „im Fehlerfall wird eine Exception geworfen“, tatsächlich liefert `map` bei `mysize == 0` aber `nullptr`. Aufrufer wie `IdentMPEG` werden dadurch zu Nullpointer-Checks verleitet; Vertrag vereinheitlichen (immer Exception, oder Doku anpassen).
