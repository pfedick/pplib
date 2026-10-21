# MP3 Review – Befunde

Review vom 2026-10-20, Scope: `src/audio/Mp3.cpp` (331 Zeilen) und `include/pplib/audio/mp3.h` (72 Zeilen).
Öffentliche API: `bool IdentMPEG(FileObject&, PPL_MPEG_HEADER*)`, `void GetMP3Frame(FileObject&, PPL_MPEG_HEADER&, ByteArray&)`, Struktur `PPL_MPEG_HEADER`.
Empirisch verifiziert gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` mit einem separaten Testprogramm (leere Datei, 500-Byte-Datei, synthetische MP3 mit gepaddingten Frames). Ersetzt die Reviews in `codereview/obsolete/mp3-review.md` und `mp3-review-qwen-medium.md`.
Merge-Hinweis (2026-10-20): Die korrekten Befunde aus `mp3-review-qwen-medium.md` (576 Samples bei MPEG-2/2.5, `memcpy`-Verlust des `vbr`-Flags, EOF-Off-by-one, `EndOfFileException`/Vorbedingung in `GetMP3Frame`, ID3v2-Validierung, `File::map`-Vertragsverstoß, `pos = 0`) sind eingearbeitet; ein fehlerhafter ID3v1-Befund dieses Reviews („127 statt 128") wurde korrigiert.

* Review done by: Copilot (VS Code, Copilot SDK)

## Bugs (kritisch)

- [x] **Ungeschütztes `map(0, 1024)` stürzt bei Dateien < 1024 Bytes ab** (`src/audio/Mp3.cpp:87`)
  *Behebungsstand:* behoben: `mapNoThrow()` mappt `min(filesize, SCAN_WINDOW)`; `nullptr` und Exceptionen ergeben `false`, Dateien unter 10 Bytes ebenfalls.
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

- [x] **Falsche Framesize-Formel: Padding im Nenner** (`src/audio/Mp3.cpp:132` und `134`)
  *Behebungsstand:* behoben: `computeFrameSize()` rechnet Padding additiv, identisch zu `FindNextHeader`.
  `144 * bitrate * 1000 / (mpg->frequency + mpg->padding)` — korrekt ist `144 * bitrate * 1000 / mpg->frequency + mpg->padding`
  (so ist es in `FindNextHeader`, Zeile 235/237, richtiggemacht). Bei gesetztem Padding-Bit wird die Framesize um 1 Byte
  zu klein berechnet.
  **Empirisch verifiziert:** Synthetische MP3 (MPEG1 Layer III, 128 kbps, 44,1 kHz, 10 Frames, 9 davon gepaddet,
  real 4179 Bytes): `IdentMPEG` liefert `framesize=417` statt 418 und `end=4178` statt 4179 — der letzte Frame wird
  abgeschnitten, `frames`/`mslength` (261 ms statt 262 ms) und `samples` sind falsch. Der Fehler wird teilweise durch
  das Header-Scannen in `CheckVBR` kaschiert, die Metadaten (Länge, Bitrate) sind bei gepaddeten Dateien aber messbar falsch.
  Behebung: Klammerung entfernen, Padding wie in `FindNextHeader` additiv behandeln.

- [x] **Debug-`hexDump()` in `GetMP3Frame` gibt jede Frame zu stdout aus** (`src/audio/Mp3.cpp:166`)
  *Behebungsstand:* behoben: `hexDump()`-Aufruf entfernt.
  `buffer.hexDump();` ist ein Debug-Überrest und dumpft pro Frame bis zu `framesize` Bytes (typisch 400–1000) auf die
  Konsole — massiver I/O-Overhead und kaputte Ausgabe bei jeder Frame-basierten Verarbeitung. Die Datei enthält oben
  (Zeile 35) ein auskommentiertes `// #define ID3DEBUG`; der Aufruf gehört entsprechend eingepackt oder entfernt:

  ```cpp
  #ifdef ID3DEBUG
      buffer.hexDump();
  #endif
  ```

## Bugs (mittel / Warnungen)

- [x] **ID3v1-Erkennung: Unterlauf bei Dateien < 128 Bytes** (`src/audio/Mp3.cpp:119-126`)
  *Behebungsstand:* behoben: `if (mpg->end < 128) break;` vor der Tag-Prüfung, Mapping läuft über `mapNoThrow()`.
  Für Dateien < 128 Bytes (von `FindNextHeader` ab ~16 Bytes akzeptiert) unterläuft `mpg->end - 127` bzw.
  `mpg->end - 128` als `uint64_t` → riesige Position → `OverflowException` aus `map()`, die hier nicht gefangen wird.
  Behebung: Prüfung mit `if (mpg->end >= 127)` absichern.
  *Korrigiert:* Die frühere Formulierung „prüft 127 statt 128, kann doppelt kürzen" war eine Fehlinterpretation —
  `end = filesize-1`, das ID3v1-Tag beginnt bei `end-127`, die 127-Byte-Mapping-Prüfung ist also korrekt; die
  zweite 128-Prüfung behandelt absichtlich doppelt vorhandene Tags.

- [x] **`FindNextHeader` hinterlässt teilbefüllte Struktur bei Fehlschlag** (`src/audio/Mp3.cpp:198-201`, Rückkehr `-1` in Zeile 180/250/265)
  *Behebungsstand:* behoben: `FindNextHeader` arbeitet auf einer lokalen Kopie der Struktur und schreibt nur validierte Kandidaten zurück.
  Die Felder `version`, `layer`, `bitrate_index`, `frequency_index` werden vor der Validierung geschrieben und bleiben
  bei Rückkehr `-1` inkonsistent stehen. `CheckVBR` (Zeile 305) vergleicht anschließend `m.layer`/`m.version` gegen
  veraltete Werte.

- [x] **`CheckVBR` verändert den übergebenen `FileObject` dauerhaft** (`src/audio/Mp3.cpp:288`)
  *Behebungsstand:* behoben: `setMapReadAhead()` wird nicht mehr aufgerufen, der Seiteneffekt auf die Aufrufer-Datei entfällt.
  `file.setMapReadAhead(1024 * 1024);` setzt den Read-Ahead der Aufrufer-Datei auf 1 MB — undichtes Side-Effect-Design.
  Zusätzlich: `FileObject::setMapReadAhead` (Basisimplementierung, `src/core/FileObject.cpp:414`) wirft
  `UnimplementedVirtualFunctionException`. `IdentMPEG` akzeptiert beliebige `FileObject`-Referenzen; für Subklassen ohne
  Override bricht `IdentMPEG` mit dieser Exception ab. Schutz: nur aufrufen, wenn überschrieben, oder Exception abfangen.

- [x] **`CheckVBR` überschreibt Bitrate und kann Metadaten korrumpieren** (`src/audio/Mp3.cpp:310`, `327`)
  *Behebungsstand:* behoben: kein `memcpy` mehr; nur `bitrate`, `frequency` und `framesize` eines als Frame überprüften Kandidaten werden übernommen, `vbr` bleibt erhalten, Division durch `frames == 0` ist abgesichert.
  Bei Layer-/Versions-Wechsel wird per `memcpy(mpg, &m, ...)` der komplette Strukturinhalt eines (möglicherweise
  fehlidentifizierten) Header-Kandidaten aus komprimierten ID3v2-Daten in die Metadaten des Aufrufers übernommen.
  Insbesondere verliert `mpg` dabei das in Zeile 297 gesetzte `vbr = true`-Flag: `FindNextHeader` setzt `m.vbr = false`
  (Zeile 216), das `memcpy` überschreibt es → eine VBR-Datei wird als CBR klassifiziert.
  Am Ende wird `mpg->bitrate` undocumented auf die Durchschnittsbitrate gesetzt; `avgbitrate / frames` (Zeile 327) ist
  eine Division durch `frames`, die nur durch den Kontrollfluss zufällig nie 0 ist (fragil).
  Behebung: statt `memcpy` nur die validen Header-Felder übernehmen und `vbr` explizit erhalten.

- [x] **`CheckVBR` berechnet Samples falsche bei MPEG-2/2.5 Layer III** (`src/audio/Mp3.cpp:320-323`)
  *Behebungsstand:* behoben: `samplesPerFrame()` liefert 1152 (MPEG-1) bzw. 576 (MPEG-2/2.5). Anmerkung: der Review-Vorschlag `version == 3` ist falsch, die Kodierung ist 1 = MPEG-1, 2 = MPEG-2, 0 = MPEG-2.5.
  `mpg->samples = frames * 1152` gilt nur für MPEG-1 Layer III. MPEG-2/2.5 Layer III haben **576 Samples pro Frame**
  (1152 nur für MPEG-1). Damit sind `samples` (und abgeleitete Längenangaben) für MPEG-2/2.5-Dateien um Faktor 2 falsch.
  Behebung:
  ```cpp
  int spf = (mpg->version == 3) ? 1152 : 576;   // MPEG-1 vs. MPEG-2/2.5, Layer III
  mpg->samples = frames * spf;
  ```

- [x] **`FindNextHeader` verpasst Header am Dateiende (Off-by-one)** (`src/audio/Mp3.cpp:192`, `180`/`253`)
  *Behebungsstand:* behoben: Scan-Grenze `pos + p + 4 <= filesize`, Mindestlänge des Fensters 4 Bytes; Header direkt vor EOF wird gefunden.
  Die Scan-Bedingung `while (pos + p + 4 < filesize)` prüft einen Header, der exakt mit `filesize` abschließt, nie
  (nötig: `pos + p + 4 <= filesize`). Zusätzlich verwirft `if (mapsize < 16) return -1` jeden Header in den letzten
  4–15 Bytes der Datei. Praktisch relevant bei Dateien ohne ID3v1-Anhängsel, deren letzter Frame direkt an EOF grenzt.

- [x] **`GetMP3Frame`: ungefangene `EndOfFileException` bei abgebrochener Datei** (`src/audio/Mp3.cpp:165`)
  *Behebungsstand:* behoben: Rückgabewert von `read()` wird ausgewertet, `EndOfFileException` und Mapping-Fehler werden gefangen; `GetMP3Frame` wirft nicht mehr.
  `file.read(buffer, mpg.framesize)` wirft laut Doku (`include/pplib/core/fileobject.h:216`) eine Exception bei
  Dateiende. Ist die Datei abgeschnitten (Framesize rechnet über EOF hinaus, vgl. Padding-Bug), stürzt der Aufrufer
  ab, statt mit leerem Buffer/ungültigem Header zurückzukehren.

- [x] **`GetMP3Frame` begrenzt `framesize` nicht auf `mpg.end`/EOF und stellt Position nicht wieder her** (`src/audio/Mp3.cpp:161-172`)
  *Behebungsstand:* behoben: Leselänge an `mpg.end` und reale Dateigröße geklemmt, `seek()` auf `min(start + framesize, filesize)`, Position bei Fehlschlag wiederhergestellt. Die Struktur bleibt im Fehlerfall unverändert (statt genullt) — Doku entsprechend angepasst.
  `file.read(buffer, mpg.framesize)` liest über das Daten-Ende hinaus in den ID3v1-Bereich hinein. Schlägt
  `FindNextHeader` fehl, wird die Dateiposition nicht auf den Ausgangswert (`pos` aus `file.tell()`, Zeile 163)
  zurückgesetzt — der Aufrufer findet die Frame nicht mehr.
  Undokumentierte Vorbedingung: `mpg.end` und `mpg.filesize` müssen vom Aufrufer bereits gesetzt sein (üblich:
  Voraufruf von `IdentMPEG`), da `FindNextHeader` `size = end - start + 1` daraus berechnet; mit leerer Struktur
  (z. B. nach `memset`) schlägt die Frame-Suche fehl.

- [x] **ID3v2-Header-Erkennung validiert Versions-Reservebits und Flags nicht** (`src/audio/Mp3.cpp:100-108`)
  *Behebungsstand:* behoben: `getId3v2TagSize()` prüft Version 2..4, Reservebits `0xE0`, die je Version definierten Flag-Bits und die synchsafe-Kodierung.
  Die Prüfung akzeptiert `buffer[3]` (Version) ungeprüft; die Bits `0xE0` sind als Reservebits gemäß ID3v2-Spec
  immer 0, und das Flags-Byte `buffer[4]` (erweitertes Header/unsynchronisierung) wird ignoriert. Ungültige oder
  Version-2.4+-Tags mit experimentellen Flags werden dadurch ggf. fehlidentifiziert; `buffer[3] & 0xE0` prüfen
  und unterstützte Versionen explizit machen.

- [x] **`IdentMPEG(file, nullptr)` erkennt kein VBR** (`src/audio/Mp3.cpp:80`, `108`; Pfad `src/audio/ID3Tag.cpp:416`)
  *Behebungsstand:* behoben: `IdentMPEG` befüllt immer eine vollständige Struktur (inkl. `end`) und ruft `CheckVBR` auch ohne Aufrufer-Pointer.
  Bei `mpg == nullptr` wird die lokale Struktur verwendet und in Zeile 108 früh zurückgekehrt, bevor `mpg->end` gesetzt
  wird (bleibt 0). Der Aufruf in `ID3Tag::identAudioFormat` ist damit zwar funktional (nur Format-Erkennung), aber jede
  spätere Wiederverwendung des Musters mit ausgefüllter Struktur würde scheitern; die API ist hier irreführend.

- [x] **`mpg->size`-Unterlauf und stillschweigende Korrektur** (`src/audio/Mp3.cpp:128-130`, ebenso `FindNextHeader:219-220`)
  *Behebungsstand:* behoben: Invariante `end >= start` wird vor der `size`-Berechnung geprüft; die stillschweigende Korrektur `size = filesize` ist entfallen.
  `mpg->size = mpg->end - mpg->start + 1` läuft als `uint64_t` über, falls nach ID3v1-Kürzung `end < start` wird
  (start aus ID3v2-Tag-Länge nahe Dateiende). Die anschließende Korrektur `if (size > filesize) size = filesize`
  versteckt den Fehler statt ihn zu melden.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [x] **Toter Code: Layer-I-Äste sind unerreichbar** (`src/audio/Mp3.cpp:131`, `321`)
  *Behebungsstand:* teilweise behoben: die unerreichbaren Layer-I-Äste sind entfernt. Layer I bleibt bewusst unsupported — die Bitraten-Tabelle enthält keine Layer-I-Werte, `FindNextHeader` lehnt Layer I weiterhin ab.
  `FindNextHeader` akzeptiert nur `layer >= 2 && layer <= 3` (Zeile 208), daher können `mpg->layer == 1` in
  `IdentMPEG` (Zeile 131) und `CheckVBR` (Zeile 321) nie eintreten. Zusätzlich: die Bitraten-Tabelle
  `MP3Bitrates[3][3][15]` (Zeile 57-63) enthält gar keine Layer-I-Werte — die erste Schicht je Version sind
  Layer-II-Werte. Tote Äste entfernen oder Layer I vollständig implementieren (inkl. Tabelle).

- [x] **Auskommentierte Reste** (`src/audio/Mp3.cpp:35`, `52`, `66-70`, `103`, `112`, `139-146`, `186`, `194`, `213`, `292`)
  *Behebungsstand:* behoben: alle auskommentierten Reste (`ID3DEBUG`, `MPEGLayer[]`, `mpg_channels[]`, `printf`-Kommentare) sind gelöscht.
  `// #define ID3DEBUG`, `// static int MPEGLayer[]`, `mpg_channels[]`, diverse `printf`-Kommentare. Für Debug-Ausgaben
  ein einheitliches `#ifdef ID3DEBUG`-Muster einführen, Rest löschen (Versionskontrolle vorhanden).

- [x] **C-Idiome und Typmischung** (`include/pplib/audio/mp3.h:36-61`, `src/audio/Mp3.cpp:87`, `119`, `123`, `327`)
  *Behebungsstand:* behoben: `struct PPL_MPEG_HEADER` ohne `typedef`, NSDMI `= 0`, `int32_t` für Header-Felder, `uint64_t` für Positionen/Größen, `std::array` für die Tabellen, `static_cast`. ABI-Änderung: Feldtypen und Layout der Struktur haben sich geändert.
  `typedef struct tagMPEGHeader` mit 25 gemischten Feldern (`uint64_t` für `size`/`mslength`, aber `int` für
  `frames`/`framesize`/`bitrate`); C-Style-Casts `(unsigned char*)file.map(...)`, `(uint32_t)`-Zuweisung in `int`-Feld.
  C++20: `struct PPL_MPEG_HEADER { ... }` mit Default-Initialisierung (`= {}`), `std::array` für die Tabellen,
  `static_cast`, durchgängige Typen (`uint64_t` für Positionen/Größen, `int32_t` für Header-Felder).

- [x] **`FindNextHeader` akzeptiert MPEG Layer II trotz "MP3"-Namensgebung** (`src/audio/Mp3.cpp:208`)
  *Behebungsstand:* bewusst beibehalten: Layer II (MP2) wird weiterhin akzeptiert, `layer` ist dokumentiert ausgefüllt. Nebenbefund behoben: die Bitratenzeile MPEG-1 Layer II war fehlerhaft (320 an Stelle 13, 384 fehlte) und ist korrigiert.
  Layer-II-Dateien (MP2) werden als MP3 identifiziert. Falls beabsichtigt (Container-Detection), sollte die
  `layer`-Information des Aufrufers zugänglich dokumentiert werden; sonst auf Layer III einschränken.

- [x] **Dead Assignment `pos = 0;` und Operator-Präzedenz** (`src/audio/Mp3.cpp:315`, `207`)
  *Behebungsstand:* behoben: `pos = 0;` entfernt, `(header[1] >> 3) & 0x3` geklammert.
  `pos = 0;` nach der Frame-Schleife wird nie gelesen (tote Zuweisung, löschen). In
  `MPEGVersion[header[1] >> 3 & 0x3]` funktioniert die Präzedenz zwar (`>>` vor `&`), sollte aber der Lesbarkeit
  wegen als `(header[1] >> 3) & 0x3` geklammert werden.

- [x] **`file.tell()` (Rückgabe `uint64_t`) in `int64_t` ohne Negativ-Check** (`src/audio/Mp3.cpp:163`, `FindNextHeader`-Aufrufe)
  *Behebungsstand:* behoben: Positionen durchgängig `uint64_t` mit Clamp auf `INT64_MAX`; Mapping-Ausnahmen laufen über `mapNoThrow()`.
  Theoretischer Überlauf bei Positionen > `INT64_MAX`; `map()` wirft dann `OverflowException` (in `FindNextHeader`
  gefangen, in `IdentMPEG` Zeile 119/123 nicht). Konsistent vorzeichenlose Typen für Positionen verwenden.

## Dokumentation & Doxygen

- [x] **`GetMP3Frame` ohne jede Doku** (`include/pplib/audio/mp3.h:66`)
  *Behebungsstand:* behoben: Doxygen-Kommentar im Header und in der Quelle, inkl. Seiteneffekten (Dateiposition) und Fehlerverhalten.
  Weder im Header noch in der Quelle existiert ein Doxygen-Kommentar. Parameter, Seiteneffekte (Dateiposition wird
  gelesen und verändert, `mpg` wird bei Fehler genullt) und Ausnahmen sind undokumentiert.

- [x] **`IdentMPEG` mit leerem `\ingroup`-Stub** (`src/audio/Mp3.cpp:75-76`)
  *Behebungsstand:* behoben: Kommentierung vor die Definition gezogen, `\ingroup` außerhalb der Funktion; Verhalten bei `mpg == nullptr`, Ausnahmen und Seiteneffekte dokumentiert.
  `/*!\ingroup PPLGroupSound */` direkt nach der Signatur ohne Inhalt; der Kommentar steht innerhalb des
  Funktionsrumpfs und wird von Doxygen nicht als Funktionsdoku erkannt (Platzierung vor der Definition nötig);
  im Header (Zeile 63) nur ein Einzeiler.
  Verhalten bei `mpg == nullptr`, Ausnahmen (`OverflowException` bei < 1024 Bytes — nach Fix: keine) und die
  Seiteneffekte auf `file` (Read-Ahead, Position) gehören dokumentiert.

- [x] **VBR-Heuristik (Abbruch nach 50 Frames) undokumentiert** (`src/audio/Mp3.cpp:300`)
  *Behebungsstand:* behoben: `VBR_SCAN_LIMIT = 50` als `constexpr` inkl. Begründung und Auswirkung auf die Erkennung dokumentiert.
  `CheckVBR` bricht die Frame-Analyse nach 50 Frames ab; bei langen Dateien mit verzögerter Bitraten-Änderung
  wird VBR dadurch ggf. nicht erkannt. Verhalten und Konstante (als `constexpr`) dokumentieren.

- [x] **Strukturfelder undokumentiert** (`include/pplib/audio/mp3.h:38-60`)
  *Behebungsstand:* behoben: alle Felder mit feldweiser `rief`-Kommentierung inkl. Einheiten und Kodierung von `version`/`layer`.
  Nur `start`/`end`, `mslength`, `length` haben Kurzkommentare. Einheiten und Sematik fehlen: `bitrate` in kbps,
  `frequency` in Hz, `framesize` in Bytes, `version`/`layer`-Kodierung (1=MPEG2/2.5, 3=MPEG1 bzw. 2/3), `vbr`.
  Für IntelliSense Feldweise `\brief`-Kommentare ergänzen.

- [x] **Header nutzt Vorwärtsdeklarationen ohne zugehörige Includes** (`include/pplib/audio/mp3.h:32-33`)
  *Behebungsstand:* bewusst beibehalten: die Forward-Deklarationen bleiben (schlanke Header), im Header steht jetzt ein expliziter Hinweis, dass `GetMP3Frame()` die Einbindung von `fileobject.h` und `bytearray.h` durch den Aufrufer verlangt.
  `class FileObject; class ByteArray;` — Aufrufer, die `GetMP3Frame` nutzen, müssen `bytearray.h`/`fileobject.h`
  selbst einbinden. Entweder im Header includen oder in der API-Doku explizit erwähnen.

## Befunde in Drittdateien

- [x] **`File::map`-Vertragsverstoß: Doku verspricht Exception, liefert bei leerer Datei `nullptr`** (`include/pplib/core/file.h:616`, `src/core/File.cpp:750`)
  *Behebungsstand:* behoben (Doku): `include/pplib/core/file.h` und `fileobject.h` dokumentieren jetzt den `nullptr`-Fall bei leerer Datei bzw. Mapping-Größe 0. Die Implementation ist unverändert geblieben, um keine Aufrufer zu brechen.
  Die Header-Doku sagt „im Fehlerfall wird eine Exception geworfen"; die Implementation gibt für `size == 0` oder
  leere Datei (`mysize == 0`) jedoch `nullptr` zurück (nur bei `bytes > mysize-position` kommt `OverflowException`).
  Entweder Doku anpassen (nullptr-Fall dokumentieren) oder durchgängig Exception werfen — `IdentMPEG` (kritischer
  Befund 1) ist einer der Nutzer, die diese Uneinheitlichkeit ausnutzen.

- [x] **`ID3Tag::identAudioFormat` guardiert nur `< 12` Bytes, ruft aber `IdentMPEG`** (`src/audio/ID3Tag.cpp:409-416`)
  *Behebungsstand:* behoben (indirekt): `IdentMPEG()` wirft bei kleinen Dateien nicht mehr, die Guardierung `< 12` in `ID3Tag::identAudioFormat()` reicht damit aus.
  Dateien ab 12 Bytes landen in `IdentMPEG` und lösen dort die `OverflowException` aus (siehe kritischer Befund 1) —
  `load()` wirft dann eine unerwartete Exception statt `UnsupportedAudioFormatException`.

- [x] **`AudioInfo::IdentAudioFile` fängt keine Exceptions** (`src/audio/AudioInfo.cpp:235`, `258`)
  *Behebungsstand:* behoben (indirekt): beide `IdentMPEG`-Aufrufe werfen nicht mehr. Offener Punkt: das eigene `map(0, 24)` in `AudioInfo::IdentAudioFile()` ist weiterhin nicht geguardet (Dateien < 24 Bytes → `OverflowException`) — Defekt außerhalb von `Mp3.cpp`, Entscheidung erforderlich.
  Beide `IdentMPEG`-Aufrufe propagieren `OverflowException` bei kleinen Dateien an die Endanwender.

- [x] **Test umgeht den Small-File-Bug statt ihn zu testen** (`tests/src/audio/id3tag.cpp:1237`)
  *Behebungsstand:* behoben: `tests/src/audio/mp3.cpp` prüft leere Datei, 1-Byte- und 500-Byte-Datei; der Kommentar in `tests/src/audio/id3tag.cpp` ist angepasst.
  Kommentar "size >= 1024 so IdentMPEG doesn't overflow map" — nach dem Fix sollte ein Test mit kleiner Datei
  (< 1024 Bytes, inkl. leerer Datei) die saubere `false`-Rückgabe absichern.

- [x] **`ID3Tag::saveMP3` verlässt sich auf `IdentMPEG` ohne Größen-Check** (`src/audio/ID3Tag.cpp:1084`)
  *Behebungsstand:* behoben: `saveMP3()` wertet den Rückgabewert aus und wirft `UnsupportedAudioFormatException`. Zusätzlich wurde `mpg.end + 1` als Schreibposition/Truncate-Marke korrigiert (ID3v1-Tag wurde sonst überschrieben); drei Größen-/md5-Erwartungen in `tests/src/audio/id3tag.cpp` sind aktualisiert.
  Kleine/leere Dateien werfen hier `OverflowException`/Segfault statt `UnsupportedAudioFormatException`.

---

## Behebungsstand (2026-10-20)

Alle 30 Befunde sind abgearbeitet: 26 behoben, 2 bewusst beibehalten (Layer-II-Akzeptanz, Forward-Deklarationen im Header), 1 teilweise behoben (Layer-I-Äste entfernt, Layer I bleibt unsupported), 1 mit offenem Folgebefund in einer Drittdatei (`AudioInfo::IdentAudioFile()`, eigenes `map(0, 24)` ohne Größen-Guard).

### Geänderte Dateien

| Datei | Änderung |
|---|---|
| `src/audio/Mp3.cpp` | Parser neu gefasst: `mapNoThrow()`, `getId3v2TagSize()`, `computeFrameSize()`, `samplesPerFrame()`, `FindNextHeader()` mit Fenster-Scan auf lokaler Kopie, `CheckVBR()` ohne `memcpy`/Read-Ahead |
| `include/pplib/audio/mp3.h` | `struct PPL_MPEG_HEADER` ohne `typedef`, NSDMI `= 0`, `int32_t`/`uint64_t`, feldweise Doxygen-Kommentare; API-Doku zu `IdentMPEG()`/`GetMP3Frame()` — **ABI-Änderung** |
| `src/audio/ID3Tag.cpp` | `saveMP3()`: Rückgabewert von `IdentMPEG()` ausgewertet, Schreibposition/Truncate auf `mpg.end + 1` korrigiert |
| `include/pplib/core/file.h`, `include/pplib/core/fileobject.h` | `map()`-Doku: `nullptr`-Rückgabe bei leerer Datei bzw. Größe 0 dokumentiert (Implementation unverändert) |
| `tests/src/audio/mp3.cpp` | 60 Unit-Tests (Realdateien, ID3v2/ID3v1, Framesize-Matrix, ungültige Header, VBR, `GetMP3Frame`) |
| `tests/src/audio/id3tag.cpp` | drei Größen-/md5-Erwartungen an die korrigierte `saveMP3()`-Semantik angepasst |

### Semantik-Änderungen der API

- `end` ist das letzte Byte der Audiodaten **inklusive**; Invariante `size == end - start + 1`; bis zu zwei ID3v1-Tags werden von `end` abgezogen.
- `IdentMPEG()` wirft nicht mehr (kleine/leere Dateien, Mapping-Fehler) und nullsetzt die Struktur bei Fehlschlag vollständig.
- `GetMP3Frame()` wirft nicht mehr; bei Fehlschlag leerer Buffer, wiederhergestellte Dateiposition und **unveränderte** Struktur (statt `memset`).

### Tests und Coverage

- `tests/src/audio/mp3.cpp`: 60 Tests grün. Gesamt-Suite: 2188 Tests, 2187 grün, 1 übersprungen (unabhängig: `FileStaticTest.isLinkWithJunction`).
- Coverage `src/audio/Mp3.cpp` (gcovr, `--exclude-throw-branches`): **Lines 97,1 %**, **Branches 82,4 %**.
- Unerreichbar bleiben defensive Zweige: `catch (const EndOfFileException&)` in `GetMP3Frame()` (`MemFile::fread()` liefert zu kurz, statt zu werfen), die Kurzlese-Prüfung `got != readsize` sowie die `READWRITE`-Pfade von `MemFile::map()`.

### Entscheidungspunkte

1. **Layer I** wird nicht implementiert (Bitraten-Tabelle enthält keine Layer-I-Werte); die toten Äste sind entfernt.
2. **Layer II (MP2)** bleibt akzeptiert — die Bitratenzeile MPEG-1 Layer II war fehlerhaft und ist korrigiert.
3. **Strukturänderung** (`typedef` weg, Feldtypen): Quellkompatibilität für Aufrufer gegeben, Binärkompatibilität nicht.
4. **`File::map()`-Kontrakt** ist nur dokumentarisch vereinheitlicht; ein durchgängiges Werfen einer Exception wäre ein Eingriff in `src/core/File.cpp` mit Auswirkung auf alle Aufrufer.
5. **Formaterkennung** in `IdentMPEG()` bleibt byte-musterbasiert; `AudioInfo::IdentAudioFormat()` und `ID3Tag::identAudioFormat()` prüfen WAV/OGG/AIFF vor MPEG, daher entstehen keine praktischen False-Positives.
