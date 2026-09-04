# Mp3 Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/audio/mp3.h` + `src/audio/Mp3.cpp` (331 Zeilen).
Verifiziert: `File::map(position, size)` (`src/core/File.cpp:636-656`, verwendet über `FileObject&`) prüft `position + bytes <= mysize` und wirft bei Verletzung `pplib::OverflowException` – kein `NULL`-Return, kein stillschweigend verkürzter Bereich. `File::read()` liest nicht weniger als angefordert, sondern wirft `EndOfFileException`/errno-Exception, wenn nicht genug Daten vorhanden sind. Beide Eigenschaften sind für die folgenden Befunde relevant.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`IdentMPEG()`: ungeprüftes `file.map(0, 1024)` wirft bei jeder Datei < 1024 Byte** (`Mp3.cpp:87`)
  ```cpp
  mpg->filesize = file.size();
  // Beginnt die Datei mit einem ID3-Tag?
  buffer = (unsigned char*)file.map(0, 1024);   // <-- kein Try/Catch, keine Prüfung der Dateigröße
  ```
  Anders als der Rest der Funktion (die für den eigentlichen MPEG-Sync-Scan `FindNextHeader()` mit sauberem Try/Catch um `file.map()` verwendet) ist dieser allererste Aufruf komplett ungeschützt und verlangt implizit mindestens 1024 Byte Dateigröße. Für jede kleinere – z.B. abgeschnittene, sehr kurze oder absichtlich winzige – Datei wirft `File::map()` eine `OverflowException`, die weder hier noch im Aufrufer `ID3Tag::identAudioFormat()` (`ID3Tag.cpp:400-412`, ebenfalls ohne Try/Catch) abgefangen wird. Ein `ID3Tag::load()` (nicht `tryLoad()`) für eine z.B. 200 Byte kleine Audiodatei bricht dadurch mit einer für den Aufrufer unerwarteten/undokumentierten I/O-Exception ab, statt sauber `AF_UNKNOWN` zu liefern.
  Fix: Mapping-Größe an die tatsächliche Dateigröße anpassen: `size_t n = std::min<uint64_t>(1024, mpg->filesize); if (n < 10) return false; buffer = (unsigned char*)file.map(0, n);` (und alle folgenden Indexzugriffe `buffer[0..9]` entsprechend gegen `n` absichern statt sie als garantiert vorhanden anzunehmen).

- [ ] **`IdentMPEG()`: `mpg->end - 127` / `mpg->end - 128` unterläuft für Dateien < 128 Byte** (`Mp3.cpp:118-126`)
  ```cpp
  mpg->end = file.size() - 1;
  buffer = (unsigned char*)file.map(mpg->end - 127, 127);   // <-- uint64_t-Unterlauf, falls file.size() < 128
  if (buffer[0] == 'T' && buffer[1] == 'A' && buffer[2] == 'G') {
      mpg->end = mpg->end - 127;
  }
  buffer = (unsigned char*)file.map(mpg->end - 128, 128);    // <-- dasselbe Muster nochmal
  ```
  `mpg->end` ist `uint64_t`. Ist die Datei kleiner als 128 Byte (z.B. eine gezielt winzige, aber mit einem gültig aussehenden 4-Byte-MPEG-Sync versehene Testdatei von z.B. 100 Byte – `FindNextHeader()` verlangt für einen Treffer nur `filesize - pos >= 16`, ist also mit sehr kleinen Dateien vereinbar), unterläuft `mpg->end - 127` zu einer Zahl nahe `UINT64_MAX`. Diese wird ungeprüft an `file.map()` weitergereicht. Da `File::map()`s eigene Bounds-Prüfung `position + bytes <= mysize` additionsbasiert ist, könnte der ohnehin schon gewaltige `position`-Wert plus `127`/`128` bei bestimmten Werten sogar unbemerkt zurück in den gültigen Bereich "wrappen" und die Prüfung fälschlich bestehen lassen; in der Praxis wird der anschließende reale `mmap()`/`pread()`-Syscall mit einem derart astronomischen Offset aber scheitern (typischerweise `EINVAL`/`EOVERFLOW`, da `off_t` vorzeichenbehaftet ist und dieses Bitmuster negativ ist) und eine I/O-Exception auslösen. Speicherkorruption ist damit unwahrscheinlich, aber die Funktion verlässt sich dabei komplett auf zufälliges Verhalten der darunterliegenden Schicht statt selbst korrekt zu prüfen – ein einfacher, gezielt kleiner Dateiausschnitt reicht, um diesen Pfad zu erreichen und eine für `IdentMPEG()`/`ID3Tag::load()` uncatchte Exception zu erzeugen.
  Fix: Vor beiden Aufrufen `if (mpg->end < 128) { /* kein Platz für ID3v1-Tag, überspringen */ }` prüfen.

## Bugs (mittel)

- [ ] **`GetMP3Frame()`: unbedingter Debug-`hexDump()` auf jedes gelesene Frame** (`Mp3.cpp:166`)
  ```cpp
  void GetMP3Frame(FileObject& file, PPL_MPEG_HEADER& mpg, ByteArray& buffer)
  {
      int64_t pos = file.tell();
      pos = FindNextHeader(file, pos, &mpg);
      if (pos < 0) { ... }
      file.read(buffer, mpg.framesize);
      buffer.hexDump();     // <-- offensichtlicher Debug-Leftover, unbedingt bei jedem Aufruf aktiv
      file.seek(pos + mpg.framesize);
  }
  ```
  Für jedes einzelne per `GetMP3Frame()` gelesene MPEG-Frame wird der komplette Frame-Inhalt auf `stdout` gedumpt – bei einem normal langen MP3 (tausende Frames) massives Log-/Konsolen-Rauschen und spürbarer Performance-Verlust in Produktionscode. Sieht nach vergessenem Debug-Code aus (kein `#ifdef ID3DEBUG` wie sonst im Audio-Code üblich).
  Fix: Entfernen oder hinter `#ifdef ID3DEBUG` (wie im Rest des Audio-Moduls üblich) verstecken.

- [ ] **`IdentMPEG()`: `mpg->size` kann durch Clamp einen unsinnigen, aber "gültig aussehenden" Wert annehmen, wenn ID3v1-Tag-Erkennung und MPEG-Start-Suche widersprüchliche Positionen liefern** (`Mp3.cpp:128-129`)
  ```cpp
  mpg->size = mpg->end - mpg->start + 1;
  if (mpg->size > mpg->filesize) mpg->size = mpg->filesize;
  ```
  Liegt der von `FindNextHeader()` gefundene Sync sehr nah am Dateiende und wird danach zusätzlich ein (mutmaßliches) ID3v1-`"TAG"` erkannt, kann `mpg->end` unter `mpg->start` fallen. `mpg->end - mpg->start + 1` unterläuft dann (beide `uint64_t`) zu einer riesigen Zahl, die der nachfolgende Clamp zwar auf `mpg->filesize` begrenzt (kein Speicherfehler), das Ergebnis (`mpg->size == mpg->filesize`) ist dann aber inhaltlich falsch und propagiert in `mpg->frames`, `mpg->length`, `mpg->samples` etc. als stille Fehlinformation statt eines erkennbaren Fehlers.
  Fix: Nach der Subtraktion explizit `if (mpg->end < mpg->start) { mpg->size = 0; ... }` statt sich auf den nachträglichen Clamp zu verlassen.

## Design

- [ ] **Inkonsistente Integer-Typen in `PPL_MPEG_HEADER`** (`mp3.h:39-64`)
  `start`, `end`, `size`, `filesize`, `mslength`, `length`, `samples` sind `uint64_t`, aber `frames`, `framesize`, `bitrate` etc. sind `int`. Für sehr lange bzw. sehr große Audiodateien (z.B. mehrstündige Streaming-Mitschnitte im Multi-GB-Bereich mit kleiner Framegröße) kann `mpg->frames = mpg->size / mpg->framesize` (`Mp3.cpp:136`) den Wertebereich von `int` überschreiten und umschlagen. Passt zur allgemeinen Modernisierungs-Agenda aus `REFACTORING.md` – kein akuter Parsing-Bug bei normalen MP3-Dateien, aber ein Konsistenz-/Zukunftssicherheits-Punkt.

- [ ] **`IdentMPEG()` akzeptiert MPEG Layer II, obwohl die Funktion "MP3" (=Layer III) heißt** (`Mp3.cpp:211`)
  ```cpp
  if (mpg->version > 0 && mpg->layer >= 2 && mpg->layer <= 3 && ...)
  ```
  `mpg->layer == 2` entspricht Layer II, nicht Layer III ("MP3"). Vermutlich historisch/bewusst so belassen (Kompatibilität zu älteren MPEG-Audio-Dateien), aber inkonsistent zum Funktionsnamen – falls beabsichtigt, wäre ein Kommentar hilfreich, falls nicht, sollte die Bedingung auf `mpg->layer == 3` verengt werden.

## Verifiziert OK (kein Handlungsbedarf)

- Tabellen-Indizierung in `FindNextHeader()` ist korrekt gegen Out-of-Bounds abgesichert: `mpg->bitrate_index` wird gegen `0` und `15` geprüft (gültiger Bereich 1-14, passt zur dritten Dimension von `MP3Bitrates[3][3][15]`), `mpg->frequency_index` gegen `3` geprüft (gültiger Bereich 0-2, passt zu `s_freq[3][4]`), `mpg->version` ist nach dem `MPEGVersion[]`-Lookup und der `version>0`-Prüfung auf 1-3 begrenzt (Index `version-1` ∈ {0,1,2}, passt zur ersten Dimension beider Tabellen).
- `FindNextHeader()`s Scan-/Remap-Logik hält den Lesezugriff `buffer+p` korrekt innerhalb des zuletzt gemappten Bereichs (`mapsize`); der Remap erfolgt immer, bevor `p` über `mapsize-4` hinausläuft, kein Off-by-one beim Buffer-Wechsel.
- `FindNextHeader()`/`CheckVBR()` terminieren immer nach endlich vielen Schritten, da jeder Fehlschlag von `FindNextHeader()` (Datei-Ende erreicht) mit `-1` sauber propagiert und die jeweilige `while`-Schleife verlässt – kein Endlosschleifen-Risiko wie im ID3Tag-Pendant (AIFF/WAVE-Chunk-Walk, siehe `id3tag-review.md`).
- Big-Endian- (`PeekN32`, AIFF) vs. Little-Endian-Zugriffe (`Peek32`, WAVE/RIFF) sind konsistent zum jeweiligen Dateiformat gewählt (relevant für `ID3Tag.cpp`, aber mitgeprüft, da `Mp3.cpp`/`ID3Tag.cpp` denselben `FileObject`-Zugriffspfad nutzen).
- Division durch `mpg->framesize`/`mpg->bitrate`/`mpg->frequency`/`frames` ist an allen Stellen durch vorherige `>0`-Prüfungen abgesichert (`Mp3.cpp:131-141`, `239-244`, `319-327`).
