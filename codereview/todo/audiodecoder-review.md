# AudioDecoder (Basisklasse + Factory) Review – Befunde

Review vom 2026-10-21, Scope: `include/pplib/audio/decoder.h` (288 Zeilen) und `src/audio/AudioDecoder.cpp` (55 Zeilen).
Öffentliche API: `pplib::AudioDecoder` (Basisklasse), `pplib::AudioDecoder_Wave`, `pplib::AudioDecoder_Aiff`, `pplib::AudioDecoder_MP3`, `pplib::AudioDecoder_Ogg` (Deklarationen), `pplib::AudioDecoder* GetAudioDecoder(FileObject&)`.
Review done by: Copilot (Copilot SDK in VS Code)

Alle kritischen Befunde wurden mit Probeprogrammen gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` (GCC 14, Linux/x86-64, `-g -O0`) und valgrind verifiziert. Die Probeprogramme liegen im Session-Workspace (`files/probe*.cpp`), nicht im Repository.

Zugehörige Detail-Reviews: [audiodecoder-wave-review.md](./audiodecoder-wave-review.md), [audiodecoder-aiff-review.md](./audiodecoder-aiff-review.md), [audiodecoder-mp3-review.md](./audiodecoder-mp3-review.md), [audiodecoder-ogg-review.md](./audiodecoder-ogg-review.md).

## Bugs (kritisch)

- [ ] **`GetAudioDecoder()` leckt das Decoder-Objekt, wenn `open()` wirft** (`src/audio/AudioDecoder.cpp:50`)
  `decoder->open(file, &info)` steht ungeschützt da. Wirft `open()` (z. B. OGG mit 6 Kanälen: `AudioDecoder_Ogg::open` Zeile 178 `channels <1 or >2`), verliert die Funktion das bereits per `new` erzeugte Objekt samt interner Allokationen.
  Verifiziert mit valgrind (50 Aufrufe mit `surround.ogg`, 6 Kanäle):
  `definitely lost: 5.200 bytes in 50 blocks`, `indirectly lost: 4.183.150 bytes`, Allokations-Stack zeigt auf `pplib::GetAudioDecoder` (`AudioDecoder.cpp:47`).
  Noch leichter erreichbar: Mono-AIFF (`IdentAIFF` akzeptiert beliebige Kanalanzahl, `AudioDecoder_Aiff::open` Zeile 70 verwirft `Channels != 2`). valgrind (20 Aufrufe): `1.760 bytes in 20 blocks are definitely lost`, Allokations-Stack `AudioDecoder.cpp:44`. Hat der Aufrufer kein `try/catch`, beendet die Exception zusätzlich den Prozess (`terminate called after throwing an instance of 'pplib::UnsupportedAudioFormatException'`).
  Behebung:
  ```cpp
  AudioDecoder* GetAudioDecoder(FileObject& file)
  {
      AudioInfo info;
      if (!IdentAudioFile(file, info)) return nullptr;
      std::unique_ptr<AudioDecoder> decoder;
      switch (info.Format) {
          case AudioInfo::AIFF: decoder = std::make_unique<AudioDecoder_Aiff>(); break;
          case AudioInfo::WAVE: decoder = std::make_unique<AudioDecoder_Wave>(); break;
          case AudioInfo::MP3:  decoder = std::make_unique<AudioDecoder_MP3>();  break;
          case AudioInfo::OGG:  decoder = std::make_unique<AudioDecoder_Ogg>();  break;
          default: return nullptr;
      }
      decoder->open(file, &info);   // exceptionssicher: unique_ptr räumt auf
      return decoder.release();
  }
  ```
  (Langfristig besser: Rückgabe `std::unique_ptr<AudioDecoder>`, siehe Code-Smells.)

## Bugs (mittel / Warnungen)

- [ ] **Nach `close()` craschen alle vier Decoder – Basisklasse bietet keinen Zustandsschutz** (`include/pplib/audio/decoder.h:93`)
  `close()` ist Teil der öffentlichen API, aber es gibt keine Möglichkeit zu prüfen, ob ein Decoder offen ist, und die Basisklasse definiert kein Verhalten nach `close()`. Dokumentiert ist „Verhalten undefiniert“ nur für `getAudioInfo()` (`decoder.h:97-98`). Real ist es in allen vier Implementierungen ein harter Absturz (SIGSEGV, Exit-Code 139) bzw. Use-after-Free. Details und Belege in den Detail-Reviews.
  Empfehlung: in der Basisklasse einen Zustand einführen (`bool isOpen()` bzw. `FileObject* ff` + Guard in der Basisklasse) und in den Overrides `if (!ff) throw DecoderException(...)` – der MP3-Decoder macht das mit `if (!decoder)` bereits vorbildlich.

- [ ] **EOF-/Exception-Vertrag der Lese-API ist nirgends definiert und in den Implementierungen inkonsistent** (`include/pplib/audio/decoder.h:115-167`)
  Dieselbe Operation liefert je nach Decoder und Überladung unterschiedliche Ergebnisse:
  | Situation | Verhalten |
  |---|---|
  | exaktes Dateiende, `num` angefragt | Wave/AIFF: Rückgabe 0 (verifiziert); OGG/MP3: Rückgabe 0 |
  | abgeschnittene Datei, `getSamples(STEREOSAMPLE16*)` Wave | `EndOfFileException` (`File::fread`, `src/core/File.cpp:490`) |
  | abgeschnittene Datei, `getSamples(STEREOSAMPLE_FLOAT*)` Wave/AIFF | `OverflowException` (`File::map`, `src/core/File.cpp:769`) |
  | abgeschnittene OGG | `EndOfFileException` aus C-Callback heraus (siehe OGG-Review) |
  | MP3-Decodefehler | `DecoderException` |
  Ein Aufrufer, der „bis Dateiende lesen“ will, kann das nicht portabel formulieren. Empfehlung: Vertrag im Header festschreiben („kurze Rückgabe == EOF, Exceptions nur für echte Fehler“) und die Implementierungen daran anpassen.

- [ ] **`getSamples(size_t num, …)` ist nicht nach oben begrenzt** (`include/pplib/audio/decoder.h:125`, `140`, `152`, `167`)
  Wave/AIFF begrenzen an `info.Samples` (`AudioDecoder_Wave.cpp:100`, `AudioDecoder_Aiff.cpp:107`), OGG und MP3 tun das **nicht** (`AudioDecoder_Ogg.cpp:240`, `AudioDecoder_MP3.cpp:230` u. a.). Ein zu großes `num` schreibt daher über den Caller-Puffer hinaus, sofern der Stream mehr Daten liefert als `info.Samples` verspricht (bei MP3 real vorkommend, siehe MP3-Review „Frankenstein“-Dateien).
  Zusätzlich: Da `num` vorzeichenlos ist, führt ein versehentlich negatives Argument (implizite Konvertierung) zu `num ≈ SIZE_MAX`. Verifiziert: `getSamples(-1, …)` mit 16-Element-Puffer erzeugt bei Wave Heap-Korruption (`malloc(): corrupted top size`), bei AIFF/MP3 Hängen/Abstürze, bei OGG Rückgabe 0 bzw. `OutOfMemoryException`.
  Empfehlung: in allen Implementierungen `num` gegen `info.Samples - position` (bzw. `SIZE_MAX`-Plausibilität) begrenzen und das im Header dokumentieren.

- [ ] **`seekSample()` hat je nach Decoder völlig unterschiedliche Semantik** (`include/pplib/audio/decoder.h:107`)
  Wave/AIFF: clampen an `info.Samples` (`AudioDecoder_Wave.cpp:86-89`, `AudioDecoder_Aiff.cpp:93-96`). OGG: ignoriert den Fehler von `ov_pcm_seek()` und setzt `position` ungeclamped (`AudioDecoder_Ogg.cpp:221-222`). MP3: wirft für jedes `sample != 0` eine `UnsupportedFeatureException` (`AudioDecoder_MP3.cpp:171`). Die Doku (`decoder.h:104-106`) sagt dazu nichts. Empfehlung: Verhalten im Header normieren (mindestens: „Position wird an das Dateiende begrenzt, Sprünge sind nicht garantiert möglich“).

- [ ] **Skalen der vier Lese-APIs sind nicht dokumentiert und untereinander inkonsistent** (`include/pplib/audio/decoder.h:140`, `152`)
  `addSamples(size_t, STEREOSAMPLE32*)` addiert bei Wave/OGG Werte im Bereich ±32767, bei AIFF mit 24 Bit Werte im Bereich ±8388607 (`AudioDecoder_Aiff.cpp:143-144`). `addSamples(size_t, STEREOSAMPLE_FLOAT*)` addiert Werte ±1,0. Wer WAV- und AIFF-Quellen mischt, erhält eine 256-fache Verzerrung. Empfehlung: Zielwertbereich (z. B. immer 24-Bit-Spitzenwert) im Header festlegen und in allen Decodern einheitlich skalieren.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Roher `new`/`delete`-Vertrag der Factory** (`include/pplib/audio/decoder.h:174-175`, `181`)
  „Der Aufrufer ist für die Freigabe des Speichers verantwortlich (delete)“ ist ein häufiger Leak-Qualifier (siehe kritischer Befund oben). Empfehlung: `std::unique_ptr<AudioDecoder> GetAudioDecoder(FileObject&)`. Da die API neu ist (ppl8), wäre der Bruch jetzt vertretbar; andernfalls über einen neuen Namen (`CreateAudioDecoder`) parallel anbieten.

- [ ] **Vier separate `if` statt `switch`, überflüssiger C-Style-Cast** (`src/audio/AudioDecoder.cpp:44-47`)
  `(AudioDecoder*)new AudioDecoder_Aiff()` ist unnötig (abgeleitete Klasse ist sichtbar). `switch (info.Format)` ist billiger und verhindert, dass bei mehreren Treffern Objekte erzeugt und wieder überschrieben werden (aktuell undenkbar, aber fragil).

- [ ] **`NULL` statt `nullptr`** (`src/audio/AudioDecoder.cpp:42`, `43`, `49`; `include/pplib/audio/decoder.h:88`, `195`, `217`, `249`, `276`)

- [ ] **Überflüssiges Semikolon nach dem virtuellen Destruktor** (`include/pplib/audio/decoder.h:74`)
  `virtual ~AudioDecoder() {};` → `virtual ~AudioDecoder() = default;`. Die explizit `= delete` gesetzten Copy-/Move-Operationen (`decoder.h:65-68`) sind korrekt und verhindern flache Kopien – das ist positiv, sollte aber mit einem Kommentar versehen werden.

- [ ] **Default-Argument in der Override-Kette wiederholt** (`include/pplib/audio/decoder.h:195`, `217`, `249`, `276`)
  Default-Argumente werden statisch über den deklarierten Typ gebunden. `open(file)` über einen `AudioDecoder*` nutzt das Default aus Zeile 88, über einen `AudioDecoder_MP3*` das aus Zeile 249 – doppelte Wartung, Fehlerquelle. Empfehlung: Default nur in der Basisklasse deklarieren.

- [ ] **Typ-Erasure über `void*` in den öffentlichen Headern** (`include/pplib/audio/decoder.h:231` `void* decoder`, `263` `void* private_data`)
  Vermeidet den Include von `<mpg123.h>`/`<vorbis/vorbisfile.h>`, ist aber untypsicher und verhindert RAII. Empfehlung: klassisches Pimpl mit unvollständigem Typ (`struct Mp3Impl;` + `std::unique_ptr<Mp3Impl>` mit benutzerdefiniertem Destruktor in der `.cpp`).

- [ ] **Unbenutzte Member/Variablen** (`include/pplib/audio/decoder.h:189`, `212`, `238` `samplesize`; `240-241` `isRunning`/`needInput`)
  `samplesize` wird in Wave/AIFF/MP3 nur gesetzt, nie gelesen. `isRunning`/`needInput` werden in `AudioDecoder_MP3.cpp:62-63`, `123-124`, `149-150` geschrieben, aber nirgends ausgewertet. Entfernen.

- [ ] **Tote Funktionsdeklaration `readWaveHeader`** (`include/pplib/audio/decoder.h:190`)
  Nirgends definiert oder verwendet. Entfernen (entfernt damit auch die Abhängigkeit von `WAVEHEADER` im Header).

- [ ] **`#ifdef HAVE_MPG123` / `HAVE_LIBVORBIS` quer durch Klassen und Methoden**
  REFACTORING.md fordert „alle #ifdefs reduzieren, nur da wo unbedingt nötig“. Die Decoder haben 4–6 Verzweigungen pro Datei, teils mit toten Zweigen (`#else return 0;`). Empfehlung: Verfügbarkeit nur noch in der Factory (`AudioDecoder.cpp`) und in einer Fabrik-Funktion pro Format entscheiden; die Klassen selbst kompilieren nur noch, wenn das Backend vorhanden ist (CMake wählt die Quellcodedateien aus).

- [ ] **`size_t`-Position vs. `uint32_t`-Metadaten** (`include/pplib/audio/decoder.h:113` vs. `AudioInfo::Samples`)
  `position` ist `size_t`, `info.Samples`/`AudioStart` sind 32 Bit. In Ausdrücken wie `info.AudioStart + position * info.BytesPerSample` (`AudioDecoder_Wave.cpp:101`) mischen sich Typen. Siehe auch Querverweis `audioinfo-review.md` (64-Bit-Dateigrößen).

- [ ] **Die gesamte Decoder-API hat keinerlei Tests** (`tests/src/`)
  Keine Testdatei referenziert `AudioDecoder`, `GetAudioDecoder` oder `addSamples()`; `tests/src/audio/` deckt nur `AudioInfo`, `ID3Tag` und den MP3-Container (`pplib::audio/mp3.h`) ab. Alle in den fünf Reviews gemeldeten Fehler (NULL-Derefs, UAF/Double-Free, SIGFPE, `position`-Desync, Vorzeichenfehler) wären durch wenige Parameter-Tests mit bekannten Samplewerten aufgefallen. Empfehlung: pro Decoder eine Fixture mit generierter Referenzdatei (bekannte Rohwerte, s. `audiodecoder-wave-review.md`) plus Tests für `open`/`close`-Folgen, EOF, `seekSample` und alle vier Lesemethoden.

## Dokumentation & Doxygen

- [ ] **Abgeleitete Decoder-Klassen haben keinerlei Doxygen** (`include/pplib/audio/decoder.h:183-204`, `206-226`, `228-258`, `260-285`)
  REFACTORING.md-Ziel: „Alle Klassen in den Header-Dateien dokumentieren, damit man beispielsweise in VSCode die Dokumentation direkt sehen“. Es fehlt jede Klassen- und Methoden-Doku, insbesondere zu den unterstützten Formaten (Wave: nur 16-Bit-Stereo-PCM; AIFF: nur 16/24-Bit-Stereo; OGG/MP3: nur 1–2 Kanäle).

- [ ] **`getSamples()`/`addSamples()` ohne `@exception` und ohne EOF-Aussage** (`include/pplib/audio/decoder.h:115-125`, `127-140`, `142-152`, `154-167`)
  Es wird nicht gesagt, was eine Rückgabe `< num` bedeutet, und keine Exception deklariert. `open()` (`decoder.h:88`) listet zwei Exceptions plus „andere können auftreten“ – für IntelliSense zu vage.

- [ ] **`seekSample()` ohne Bereichs-/Exception-Doku** (`include/pplib/audio/decoder.h:104-107`)

- [ ] **`GetAudioDecoder()`-Doku verschweigt, dass `open()` aufgerufen wird und Exceptions durchreichen** (`include/pplib/audio/decoder.h:170-180`)
  Der Hinweis „oder NULL, falls kein passender Decoder gefunden wurde“ suggeriert, die Funktion werfe nie; tatsächlich wirft sie Decoder-Exceptions (z. B. „channels <1 or >2“).

## Befunde in Drittdateien

- [ ] **`AudioInfo::AudioStart`/`AudioEnd`/`AudioSize` sind 32 Bit** (`include/pplib/audio/audioinfo.h`)
  Alle Decoder berechneten Dateioffsete daraus; Dateien > 4 GiB (bzw. Audioabschnitte) laufen über. Bereits in [audioinfo-review.md](./audioinfo-review.md) dokumentiert – hier nur als Betroffenheitsvermerk.

- [ ] **`IdentOggVorbisAudioFile()` ist in keinem öffentlichen Header deklariert** (`src/audio/AudioDecoder_Ogg.cpp:113`)
  Verwendet wird die Funktion nur über eine Forward-Deklaration in `src/audio/AudioInfo.cpp:39`. Entweder in `audioinfo.h` (analog `IdentAudioFile`) aufnehmen oder `static` machen.

  ==> WIE? Wenn ich sie static mache, kann sie nur innerhalb der Datei `AudioDecoder_Ogg.cpp` verwendet werden. Ich brauche sie aber in `AudioInfo.cpp`. In die öffentliche Header-Datei `audioinfo.h` will ich sie nicht aufnehmen, da es eine interne Funktion ist. Es bleibt daher, wie es ist!


- [ ] **`IdentAIFF()` berechnet `BytesPerSample` aus der Bitbreite, nicht aus dem SSND-Layout** (`src/audio/AudioInfo.cpp:93`)
  Details und Reproduktion im AIFF-Review.

- [ ] **`File::map()` wirft `OverflowException` statt `nullptr` bei Bereichsüberschreitung** (`src/core/File.cpp:769`)
  Die AIFF-Identifikation verlässt sich auf `if (!adr) break;` (`src/audio/AudioInfo.cpp:76`) bzw. `if (!buffer) return false;` (`:155`) – dieser Pfad ist bei `File::map` nie erreichbar, stattdessen fliegt eine Exception durch die Identifikation. Bereits in [audioinfo-review.md](./audioinfo-review.md) vermerkt.

  ==> Das Verhalten ist so dokumentiert und muss bei der Identifikation berücksichtigt werden.