# MP3 Review – Befunde

Review vom 2026-10-20, Scope: `src/audio/Mp3.cpp` (331 Zeilen) und `include/pplib/audio/mp3.h` (72 Zeilen).
Öffentliche API: `bool IdentMPEG(FileObject&, PPL_MPEG_HEADER*)`, `void GetMP3Frame(FileObject&, PPL_MPEG_HEADER&, ByteArray&)`, Struktur `PPL_MPEG_HEADER`.
Empirisch verifiziert gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` mit einem separaten Testprogramm (leere Datei, 500-Byte-Datei, synthetische MP3 mit gepaddingten Frames). Ersetzt die Reviews in `codereview/obsolete/mp3-review.md` und `mp3-review-qwen-medium.md`.

* Review done by: Copilot (VS Code, Copilot SDK)

## Bugs (kritisch)

- [ ] **Ungeschütztes `map(0, 1024)` stürzt bei Dateien < 1024 Bytes ab** (`src/audio/Mp3.cpp:87`)
  `buffer = (unsigned char*)file.map(0, 1024);` ist weder auf Dateigröße begrenzt noch gegen `nullptr` geprüft.
  `File::map()` (`src/core/File.cpp:750`) wirft für `bytes > filesize` eine `OverflowException` und liefert für eine
  leere Datei (`mysize == 0`) `nullptr` zurück → Nullzugriff auf `buffer[0]` in Zeile 100.
  **Empirisch verifiziert:** 500-Byte-Datei → `OverflowException` (ungefangen); leere Datei → Segmentation Fault (Exit 139).
  Betroffene Aufrufer fangen die Exception nicht (siehe Befunde in Drittdateien). Das Test-Comment in
  `tests/src/audio/id3tag.cpp:1237` ("size >= 1024 so IdentMPEG doesn't overflow map") belegt, dass der Bug bekannt ist.
  Behebung:

  ```cpp
  uint64_t maplen = mpg->filesize < 1024 ? mpg->filesize : 1024;
  if (maplen < 10) return false;
  buffer = (unsigned char*)file.map(0, maplen);
  if (!buffer) return false;
  ```

- [ ] **Falsche Framesize-Formel: Padding im Nenner** (`src/audio/Mp3.cpp:132` und `134`)
  `144 * bitrate * 1000 / (mpg->frequency + mpg->padding)` — korrekt ist `144 * bitrate * 1000 / mpg->frequency + mpg->padding`
  (so ist es in `FindNextHeader`, Zeile 235/237, richtiggemacht). Bei gesetztem Padding-Bit wird die Framesize um 1 Byte
  zu klein berechnet.
  **Empirisch verifiziert:** Synthetische MP3 (MPEG1 Layer III, 128 kbps, 44,1 kHz, 10 Frames, 9 davon gepaddet,
  real 4179 Bytes): `IdentMPEG` liefert `framesize=417` statt 418 und `end=4178` statt 4179 — der letzte Frame wird
  abgeschnitten, `frames`/`mslength` (261 ms statt 262 ms) und `samples` sind falsch. Der Fehler wird teilweise durch
  das Header-Scannen in `CheckVBR` kaschiert, die Metadaten (Länge, Bitrate) sind bei gepaddeten Dateien aber messbar falsch.
  Behebung: Klammerung entfernen, Padding wie in `FindNextHeader` additiv behandeln.

- [ ] **Debug-`hexDump()` in `GetMP3Frame` gibt jede Frame zu stdout aus** (`src/audio/Mp3.cpp:166`)
  `buffer.hexDump();` ist ein Debug-Überrest und dumpft pro Frame bis zu `framesize` Bytes (typisch 400–1000) auf die
  Konsole — massiver I/O-Overhead und kaputte Ausgabe bei jeder Frame-basierten Verarbeitung. Die Datei enthält oben
  (Zeile 35) ein auskommentiertes `// #define ID3DEBUG`; der Aufruf gehört entsprechend eingepackt oder entfernt:

  ```cpp
  #ifdef ID3DEBUG
      buffer.hexDump();
  #endif
  ```

## Bugs (mittel / Warnungen)

- [ ] **ID3v1-Erkennung prüft 127 statt 128 Bytes und kann doppelt kürzen** (`src/audio/Mp3.cpp:119-126`)
  Erste Prüfung mappt `end-127, 127` Bytes und kürzt um 127, zweite Prüfung mappt `end-128, 128` und kürzt um 128.
  Steht zufällig ein "TAG" an Position `end-127` (z. B. Restdaten vor einem ID3v1-Tag), werden insgesamt 255 Bytes
  abgezogen. Für Dateien < 128 Bytes (von `FindNextHeader` ab ~16 Bytes akzeptiert) entsteht ein Unterlauf
  (`mpg->end - 127` als `uint64_t` → riesige Position) → `OverflowException` aus `map()`, die hier nicht gefangen wird.
  Behebung: nur eine Prüfung auf 128 Bytes, mit `if (mpg->end >= 128)` absichern.

- [ ] **`FindNextHeader` hinterlässt teilbefüllte Struktur bei Fehlschlag** (`src/audio/Mp3.cpp:198-201`, Rückkehr `-1` in Zeile 180/250/265)
  Die Felder `version`, `layer`, `bitrate_index`, `frequency_index` werden vor der Validierung geschrieben und bleiben
  bei Rückkehr `-1` inkonsistent stehen. `CheckVBR` (Zeile 305) vergleicht anschließend `m.layer`/`m.version` gegen
  veraltete Werte.

- [ ] **`CheckVBR` verändert den übergebenen `FileObject` dauerhaft** (`src/audio/Mp3.cpp:288`)
  `file.setMapReadAhead(1024 * 1024);` setzt den Read-Ahead der Aufrufer-Datei auf 1 MB — undichtes Side-Effect-Design.
  Zusätzlich: `FileObject::setMapReadAhead` (Basisimplementierung, `src/core/FileObject.cpp:414`) wirft
  `UnimplementedVirtualFunctionException`. `IdentMPEG` akzeptiert beliebige `FileObject`-Referenzen; für Subklassen ohne
  Override bricht `IdentMPEG` mit dieser Exception ab. Schutz: nur aufrufen, wenn überschrieben, oder Exception abfangen.

- [ ] **`CheckVBR` überschreibt Bitrate und kann Metadaten korrumpieren** (`src/audio/Mp3.cpp:310`, `327`)
  Bei Layer-/Versions-Wechsel wird per `memcpy(mpg, &m, ...)` der komplette Strukturinhalt eines (möglicherweise
  fehlidentifizierten) Header-Kandidaten aus komprimierten ID3v2-Daten in die Metadaten des Aufrufers übernommen.
  Am Ende wird `mpg->bitrate` undocumented auf die Durchschnittsbitrate gesetzt; `avgbitrate / frames` (Zeile 327) ist
  eine Division durch `frames`, die nur durch den Kontrollfluss zufällig nie 0 ist (fragil).

- [ ] **`GetMP3Frame` begrenzt `framesize` nicht auf `mpg.end`/EOF und stellt Position nicht wieder her** (`src/audio/Mp3.cpp:161-172`)
  `file.read(buffer, mpg.framesize)` liest über das Daten-Ende hinaus in den ID3v1-Bereich hinein. Schlägt
  `FindNextHeader` fehl, wird die Dateiposition nicht auf den Ausgangswert (`pos` aus `file.tell()`, Zeile 163)
  zurückgesetzt — der Aufrufer findet die Frame nicht mehr.

- [ ] **`IdentMPEG(file, nullptr)` erkennt kein VBR** (`src/audio/Mp3.cpp:80`, `108`; Pfad `src/audio/ID3Tag.cpp:416`)
  Bei `mpg == nullptr` wird die lokale Struktur verwendet und in Zeile 108 früh zurückgekehrt, bevor `mpg->end` gesetzt
  wird (bleibt 0). Der Aufruf in `ID3Tag::identAudioFormat` ist damit zwar funktional (nur Format-Erkennung), aber jede
  spätere Wiederverwendung des Musters mit ausgefüllter Struktur würde scheitern; die API ist hier irreführend.

- [ ] **`mpg->size`-Unterlauf und stillschweigende Korrektur** (`src/audio/Mp3.cpp:128-130`, ebenso `FindNextHeader:219-220`)
  `mpg->size = mpg->end - mpg->start + 1` läuft als `uint64_t` über, falls nach ID3v1-Kürzung `end < start` wird
  (start aus ID3v2-Tag-Länge nahe Dateiende). Die anschließende Korrektur `if (size > filesize) size = filesize`
  versteckt den Fehler statt ihn zu melden.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Toter Code: Layer-I-Äste sind unerreichbar** (`src/audio/Mp3.cpp:131`, `321`)
  `FindNextHeader` akzeptiert nur `layer >= 2 && layer <= 3` (Zeile 208), daher können `mpg->layer == 1` in
  `IdentMPEG` (Zeile 131) und `CheckVBR` (Zeile 321) nie eintreten. Zusätzlich: die Bitraten-Tabelle
  `MP3Bitrates[3][3][15]` (Zeile 57-63) enthält gar keine Layer-I-Werte — die erste Schicht je Version sind
  Layer-II-Werte. Tote Äste entfernen oder Layer I vollständig implementieren (inkl. Tabelle).

- [ ] **Auskommentierte Reste** (`src/audio/Mp3.cpp:35`, `52`, `66-70`, `103`, `112`, `139-146`, `186`, `194`, `213`, `292`)
  `// #define ID3DEBUG`, `// static int MPEGLayer[]`, `mpg_channels[]`, diverse `printf`-Kommentare. Für Debug-Ausgaben
  ein einheitliches `#ifdef ID3DEBUG`-Muster einführen, Rest löschen (Versionskontrolle vorhanden).

- [ ] **C-Idiome und Typmischung** (`include/pplib/audio/mp3.h:36-61`, `src/audio/Mp3.cpp:87`, `119`, `123`, `327`)
  `typedef struct tagMPEGHeader` mit 25 gemischten Feldern (`uint64_t` für `size`/`mslength`, aber `int` für
  `frames`/`framesize`/`bitrate`); C-Style-Casts `(unsigned char*)file.map(...)`, `(uint32_t)`-Zuweisung in `int`-Feld.
  C++20: `struct PPL_MPEG_HEADER { ... }` mit Default-Initialisierung (`= {}`), `std::array` für die Tabellen,
  `static_cast`, durchgängige Typen (`uint64_t` für Positionen/Größen, `int32_t` für Header-Felder).

- [ ] **`FindNextHeader` akzeptiert MPEG Layer II trotz "MP3"-Namensgebung** (`src/audio/Mp3.cpp:208`)
  Layer-II-Dateien (MP2) werden als MP3 identifiziert. Falls beabsichtigt (Container-Detection), sollte die
  `layer`-Information des Aufrufers zugänglich dokumentiert werden; sonst auf Layer III einschränken.

- [ ] **`file.tell()` (Rückgabe `uint64_t`) in `int64_t` ohne Negativ-Check** (`src/audio/Mp3.cpp:163`, `FindNextHeader`-Aufrufe)
  Theoretischer Überlauf bei Positionen > `INT64_MAX`; `map()` wirft dann `OverflowException` (in `FindNextHeader`
  gefangen, in `IdentMPEG` Zeile 119/123 nicht). Konsistent vorzeichenlose Typen für Positionen verwenden.

## Dokumentation & Doxygen

- [ ] **`GetMP3Frame` ohne jede Doku** (`include/pplib/audio/mp3.h:66`)
  Weder im Header noch in der Quelle existiert ein Doxygen-Kommentar. Parameter, Seiteneffekte (Dateiposition wird
  gelesen und verändert, `mpg` wird bei Fehler genullt) und Ausnahmen sind undokumentiert.

- [ ] **`IdentMPEG` mit leerem `\ingroup`-Stub** (`src/audio/Mp3.cpp:75-76`)
  `/*!\ingroup PPLGroupSound */` direkt nach der Signatur ohne Inhalt; im Header (Zeile 63) nur ein Einzeiler.
  Verhalten bei `mpg == nullptr`, Ausnahmen (`OverflowException` bei < 1024 Bytes — nach Fix: keine) und die
  Seiteneffekte auf `file` (Read-Ahead, Position) gehören dokumentiert.

- [ ] **Strukturfelder undokumentiert** (`include/pplib/audio/mp3.h:38-60`)
  Nur `start`/`end`, `mslength`, `length` haben Kurzkommentare. Einheiten und Sematik fehlen: `bitrate` in kbps,
  `frequency` in Hz, `framesize` in Bytes, `version`/`layer`-Kodierung (1=MPEG2/2.5, 3=MPEG1 bzw. 2/3), `vbr`.
  Für IntelliSense Feldweise `\brief`-Kommentare ergänzen.

- [ ] **Header nutzt Vorwärtsdeklarationen ohne zugehörige Includes** (`include/pplib/audio/mp3.h:32-33`)
  `class FileObject; class ByteArray;` — Aufrufer, die `GetMP3Frame` nutzen, müssen `bytearray.h`/`fileobject.h`
  selbst einbinden. Entweder im Header includen oder in der API-Doku explizit erwähnen.

## Befunde in Drittdateien

- [ ] **`ID3Tag::identAudioFormat` guardiert nur `< 12` Bytes, ruft aber `IdentMPEG`** (`src/audio/ID3Tag.cpp:409-416`)
  Dateien ab 12 Bytes landen in `IdentMPEG` und lösen dort die `OverflowException` aus (siehe kritischer Befund 1) —
  `load()` wirft dann eine unerwartete Exception statt `UnsupportedAudioFormatException`.

- [ ] **`AudioInfo::IdentAudioFile` fängt keine Exceptions** (`src/audio/AudioInfo.cpp:235`, `258`)
  Beide `IdentMPEG`-Aufrufe propagieren `OverflowException` bei kleinen Dateien an die Endanwender.

- [ ] **Test umgeht den Small-File-Bug statt ihn zu testen** (`tests/src/audio/id3tag.cpp:1237`)
  Kommentar "size >= 1024 so IdentMPEG doesn't overflow map" — nach dem Fix sollte ein Test mit kleiner Datei
  (< 1024 Bytes, inkl. leerer Datei) die saubere `false`-Rückgabe absichern.

- [ ] **`ID3Tag::saveMP3` verlässt sich auf `IdentMPEG` ohne Größen-Check** (`src/audio/ID3Tag.cpp:1084`)
  Kleine/leere Dateien werfen hier `OverflowException`/Segfault statt `UnsupportedAudioFormatException`.
