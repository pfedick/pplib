# AudioDecoder_Wave Review – Befunde

Review vom 2026-10-21, Scope: `src/audio/AudioDecoder_Wave.cpp` (155 Zeilen) und `include/pplib/audio/decoder.h:183-204` (Klassendeklaration).
Öffentliche API: `AudioDecoder_Wave()`, `~AudioDecoder_Wave()`, `open(FileObject&, const AudioInfo*)`, `close()`, `getAudioInfo()`, `seekSample(size_t)`, `getPosition()`, `getSamples(size_t, STEREOSAMPLE16*)`, `addSamples(size_t, STEREOSAMPLE32*)`, `getSamples(size_t, STEREOSAMPLE_FLOAT*)`, `addSamples(size_t, STEREOSAMPLE_FLOAT*)`.
Review done by: Copilot (Copilot SDK in VS Code)

Alle kritischen Befunde wurden mit Probeprogrammen gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` (GCC 14, Linux/x86-64, `-g -O0`) verifiziert. Verwendete Testdateien: `tests/testdata/audio/test_44kHz.wav` sowie handgebaute/abgeschnittene WAV-Dateien (`/tmp/short.wav`, `/tmp/short2.wav`: Header deklariert 100000 Frames, Datei enthält 3000; `/tmp/gen16.wav`: 8 Frames mit bekanntem 16-Bit-Rohmuster). Hinweis: ffmpeg-WAVs enthalten standardmäßig einen `LIST/INFO`-Chunk zwischen `fmt ` und `data`, den `IdentWave()` nicht akzeptiert – Test-WAVs daher mit `-map_metadata -1 -fflags +bitexact` erzeugen.

## Bugs (kritisch)

- [ ] **Zugriff auf `ff` ohne `NULL`-Prüfung in allen vier Lesepfaden → SIGSEGV nach `close()`** (`src/audio/AudioDecoder_Wave.cpp:101`, `111`, `127`, `143`)
  `close()` setzt `ff = NULL` (Zeile 74), jede Lesefunktion dereferenziert aber sofort `ff->seek(…)` bzw. `ff->map(…)` (Zeilen 101/102, 111, 127, 143).
  Verifiziert: `open()`, `close()`, danach `getSamples(10, buffer)` → Prozessabbruch mit Signal 11 (Exit-Code 139), keine Exception.
  Behebung (für alle vier Pfade identisch):
  ```cpp
  size_t AudioDecoder_Wave::getSamples(size_t num, STEREOSAMPLE16* interleafed)
  {
      if (!ff) throw DecoderException("AudioDecoder_Wave: Datei ist nicht geoeffnet");
      …
  }
  ```
  Zusätzlich sollte `seekSample()` (Zeile 84) und `getAudioInfo()` (Zeile 79) analog abgesichert werden.

- [ ] **Abgeschnittene WAV-Datei: je nach Überladung unterschiedliche Exception, teils mitten im Lesevorgang** (`src/audio/AudioDecoder_Wave.cpp:102` vs. `111`, `127`, `143`)
  `getSamples(size_t, STEREOSAMPLE16*)` liest über `ff->read()` → `File::fread()` wirft bei 0 gelesenen Bytes am Dateiende `EndOfFileException` (`src/core/File.cpp:489-491`). Die drei anderen Pfade nutzen `ff->map()` → `File::map()` wirft `OverflowException`, sobald `position + bytes` hinter dem echten Dateiende liegt (`src/core/File.cpp:769`).
  Verifiziert mit `/tmp/short2.wav` (Header: 100000 Frames, real 3000 Frames): int16-Pfad liefert erst korrekte Daten und wirft dann `EndOfFileException`, der Float-Pfad wirft sofort `OverflowException`.
  Auswirkung: Ein Aufrufer, der „solange lesen, bis 0 zurückkommt“ schreibt, crasht bzw. bricht mit einer nicht erwarteten Exception ab; die bereits gelieferten Daten sind ok, aber `position` wurde im `map`-Pfad nicht aktualisiert (Zeile 119/135/151 wird vor der Exception nicht erreicht, weil `map()` vorher wirft – der Zustand bleibt also konsistent, nur die Exception ist falsch).
  Behebung: Restgröße vor dem Zugriff begrenzen, z. B.
  ```cpp
  uint64_t avail = (info.AudioStart + info.AudioSize < ff->size())
                       ? (info.AudioStart + info.AudioSize) - off : 0;
  if (avail == 0) return 0;
  if (bytes > avail) bytes = avail;   // kurze Rückgabe == EOF
  ```

- [ ] **`addSamples(size_t, STEREOSAMPLE32*)` addiert vorzeichenlos — alle negativen Samples werden zu Großwerten** (`src/audio/AudioDecoder_Wave.cpp:114`, `115`)
  `Peek16()` liefert `uint16_t` (`include/pplib/core/functions.h:193`). Die Zuweisung `buffer[i].left += Peek16(data)` erfolgt damit vorzeichenlos; ein Sample mit Rohwert `0xEC78` (= −5000) wird als `+60536` aufaddiert. Die anderen drei Pfade machen es richtig: int16-Pfad über `STEREOSAMPLE16` (`SAMPLE16` = `int16_t`, `include/pplib/audio/sample_formats.h:38`) und beide Float-Pfade über den Cast `(int16_t)Peek16(...)` (Zeile 130/131, 146/147). Nur der 32-Bit-Akkumulator ist falsch.
  Verifiziert an einer handgeschriebenen 16-Bit-Stereo-WAV mit bekannten Rohwerten (`/tmp/gen16.wav`, 8 Frames):
  ```
  Rohwerte:      32767 -32767 -1 1 -5000 -10000 -15012 0
  getSamples16:  32767 -32767 -1 1 -5000 -10000 -15012 0     (korrekt)
  addSamples32:  32767  32769 65535 1 60536 55536  50524 0   (falsch)
  getSamplesflt: 1.0000 -1.0000 -0.0000 0.0000 -0.1526 …     (korrekt)
  ```
  Auswirkung: Jede Überlagerung über `addSamples(…, STEREOSAMPLE32*)` (Mix mehrerer Spuren, Echo/Delay, Normalisierung) bricht bei jedem Aussteuerungswert unter 0 um ≈ 65536 aus dem Wertebereich — hörbares Clipping/Rauschen; zusätzlich wächst eine akkumulierte Spur statt zu pendeln monoton Richtung `int32_t`-Überlauf.
  Behebung:
  ```cpp
  buffer[i].left  += static_cast<int16_t>(Peek16(data));
  buffer[i].right += static_cast<int16_t>(Peek16(data + 2));
  ```
  Die anderen Decoder sind nicht betroffen: OGG und MP3 liefern `int16_t`-Samples über `STEREOSAMPLE16` bzw. `int16_t*` und addieren erst danach vorzeichenbehaftet; AIFF castet in `addSamples(…, STEREOSAMPLE32*)` explizit (`src/audio/AudioDecoder_Aiff.cpp:133-146`).
  Ein Regressionstest für diesen Fall fehlt: in `tests/src/` wird `addSamples()` überhaupt nicht aufgerufen, und keine Testdatei referenziert `AudioDecoder`/`GetAudioDecoder` (`tests/src/audio/` deckt nur `AudioInfo`, `ID3Tag` und den MP3-Container `pplib::MP3` ab) — die gesamte Decoder-API ist ungetestet.

## Bugs (mittel / Warnungen)

- [ ] **Destruktor ruft `close()` nicht** (`src/audio/AudioDecoder_Wave.cpp:49-51`)
  Alle anderen Decoder (`_Aiff` Zeile 47-50, `_Ogg` Zeile 150-153, `_MP3` Zeile 80-92) rufen im Destruktor `close()`. Der Wave-Destruktor ist leer; `ff` bleibt als hängender Zeiger stehen, `position`/`info` bleiben „stale“. Aktuell kein Leak (der Wave-Decoder besitzt keinen Heap), aber die Inkonsistenz ist eine Fehlerquelle, sobald der Wave-Decoder Puffer bekommt. Empfehlung: `close()` aufrufen oder – nach Umstellung auf RAII – den Destruktor `= default` setzen und die Klasse explizit nicht kopierbar machen (ist über die Basisklasse bereits gegeben).

- [ ] **Toter Zweig: `if (info.BitsPerSample == 16)` ist immer wahr** (`src/audio/AudioDecoder_Wave.cpp:112`, `128`, `144`)
  `open()` lehnt alles außer 16 Bit bereits ab (Zeile 66). Die Prüfung ist damit überflüssig; fehlt sie künftig in `open()`, würden die Schleifen stillschweigend übersprungen und `position` trotzdem erhöht (Zeilen 119/135/151) → Datenverlust ohne Fehlermeldung. Empfehlung: entweder Prüfung entfernen oder mit `else throw UnsupportedAudioFormatException(…)` absichern.

- [ ] **Caller-übergebene `AudioInfo` wird ungeprüft übernommen** (`src/audio/AudioDecoder_Wave.cpp:59-60`, Validierung nur `62-67`)
  Bei `open(file, info)` werden `Samples`, `AudioStart` und `BytesPerSample` aus der Fremdinfo übernommen und direkt für Offset-Berechnungen benutzt (Zeilen 100-102, 110-111, …). Eine veraltete oder zu einer anderen Datei gehörende `AudioInfo` führt zu `map()`-Ausnahme oder – im int16-Pfad – zu Schreibzugriffen außerhalb des Caller-Puffers, weil `ff->read()` dort mehr Bytes nachliefert, als `info.Samples` suggeriert. Empfehlung: nach der Übernahme Plausibilität gegen `file.size()` prüfen (`info.AudioStart + info.Samples * info.BytesPerSample <= file.size()`).

- [ ] **Möglicher Unterlauf bei `info.Samples - position`** (`src/audio/AudioDecoder_Wave.cpp:100`, `110`, `126`, `142`)
  `if (position + samples > info.Samples) samples = info.Samples - position;` liefert einen riesigen Wert, falls `position > info.Samples` ist. Aktuell nicht erreichbar (`open()` setzt `position = 0`, `seekSample()` clampet), aber die Rechnung ist fragil. Robuster:
  ```cpp
  size_t samples = num;
  if (position >= info.Samples) return 0;
  if (samples > info.Samples - position) samples = info.Samples - position;
  ```

- [ ] **Unterstützte Formate sind nirgends dokumentiert** (`include/pplib/audio/decoder.h:183-204`, Validierung `src/audio/AudioDecoder_Wave.cpp:65-67`)
  Der Decoder akzeptiert ausschließlich unkomprimiertes 16-Bit-Stereo-PCM (`Channels == 2`, `BitsPerSample == 16`, `BytesPerSample == 4`). Mono-, 8-Bit-, 24-Bit-, 32-Bit-, Float- und komprimierte WAV-Dateien werden mit `UnsupportedAudioFormatException("Channels != 2")` abgewiesen – die Fehlermeldung ist für 24-Bit-Mono-Dateien irreführend. `GetAudioDecoder()` gibt in solchen Fällen `NULL` zurück, ohne den Grund zu nennen.

- [ ] **`samplesize` ist toter Ballast** (`src/audio/AudioDecoder_Wave.cpp:45`, `76`; Deklaration `include/pplib/audio/decoder.h:189`)
  Wird nur mit 0 beschrieben, nie gelesen. Entfernen.

- [ ] **Vollständiger Seek + sequenzieller Read pro Aufruf im int16-Pfad** (`src/audio/AudioDecoder_Wave.cpp:101-102`)
  Die drei anderen Pfade arbeiten mmap-basiert, dieser Pfad sucht und liest. Das ist funktional korrekt (gemischter `read()`/`map()`-Zugriff wurde verifiziert: keine Positions- oder Konsistenzprobleme, identische Samplewerte), aber ineffizient und vermeidet die `map()`-Pufferwiederverwendung. Für Mikrocontroller-Ziele (pico-pplib) wäre der `read()`-Pfad ohnehin der gemeinsame Nenner – eine einheitliche Strategie wäre wartbarer.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **C-Style-Konvertierung fehlt hier zwar, aber `(int16_t)Peek16(data)`-Casts** (`src/audio/AudioDecoder_Wave.cpp:130`, `131`, `146`, `147`)
  `static_cast<int16_t>(Peek16(data))` ist lesbarer und prüfbar. (Der Cast ist inhaltlich korrekt: `Peek16` liest Little-Endian byteweise, WAV ist Little-Endian – plattformunabhängig richtig.)

- [ ] **Division zur Float-Konvertierung mit `32768.0f`** (`src/audio/AudioDecoder_Wave.cpp:130-131`, `146-147`)
  Korrekt, aber als magische Zahl. Besser: `* (1.0f / 32768.0f)` oder eine benannte Konstante; der Compiler faltet die Konstante, und die Absicht ist erkennbar.

- [ ] **Vier nahezu identische Schleifenkörper** (`src/audio/AudioDecoder_Wave.cpp:107-153`)
  `addSamples`/`getSamples` × int/float unterscheiden sich nur in `+=`/`=` und Skalierung. Empfehlung: eine private Schablone/Funktion `template <class T, bool Add> size_t readSamples(size_t num, T* buffer)` – reduziert die Fehlerfläche (z. B. dass in einem Pfad eine Prüfung fehlt) erheblich.

- [ ] **`NULL` statt `nullptr`** (`src/audio/AudioDecoder_Wave.cpp:46`, `74`; Default-Argument `include/pplib/audio/decoder.h:195`)

## Dokumentation & Doxygen

- [ ] **Klasse `AudioDecoder_Wave` ohne jede Doxygen** (`include/pplib/audio/decoder.h:183-204`)
  Es fehlt: Klassenbeschreibung (unterstützte Formate: 16-Bit-Stereo-PCM, Little-Endian), Member-Doku, Methoden-Doku. Damit zeigt IntelliSense beim Aufruf von `AudioDecoder_Wave` nichts an.

- [ ] **`open()` dokumentiert die Format-Einschränkungen nicht** (`include/pplib/audio/decoder.h:195`)
  Welche Exceptions bei Mono/8-Bit/24-Bit geworfen werden, ist nur dem Quelltext zu entnehmen.

- [ ] **`getSamples()`/`addSamples()` ohne Aussage zu EOF und Exceptions** (`include/pplib/audio/decoder.h:200-203`)
  Siehe Basisklassen-Review: der Wave-Decoder wirft je nach Überladung `EndOfFileException` oder `OverflowException` (beide nicht dokumentiert).

- [ ] **`seekSample()`-Verhalten (Clamping) und `getPosition()`-Bezug undokumentiert** (`src/audio/AudioDecoder_Wave.cpp:84-90`)
  Die Position ist relativ zum Audio-Datenanfang (nicht zur Datei) – das ist nowhere dokumentiert und für `AudioInfo::AudioStart`-Rechnungen des Aufrufers relevant.

## Befunde in Drittdateien

- [ ] **`readWaveHeader(FileObject&, WAVEHEADER&)` deklariert, aber nie definiert** (`include/pplib/audio/decoder.h:190`)
  Kein Definition/Verwendung im gesamten Projekt gefunden. Entfernen der Deklaration entfernt auch die Abhängigkeit der Header von `WAVEHEADER` (`include/pplib/audio/audioinfo.h`).

- [ ] **`File::fread()` wirft bei EOF mit 0 gelesenen Bytes, liefert aber kurze Sätze kommentarlos** (`src/core/File.cpp:485-492`)
  Für Decoder wäre ein einheitlicher Vertrag („kurze Rückgabe, keine Exception am EOF“) sinnvoller. Betrifft auch andere Decoder (AIFF/OGG-Callbacks).

- [ ] **`File::map()` wirft `OverflowException` statt `nullptr` zu liefern** (`src/core/File.cpp:769`)
  Der Header-Kommentar von `File::map()` und die Aufrufstellen (u. a. `src/audio/AudioInfo.cpp:76`, `:155`) gehen von `nullptr` bei „Datei zu klein“ aus; tatsächlich fliegt eine Exception. Bereits in [audioinfo-review.md](./audioinfo-review.md) dokumentiert.

- [ ] **`IdentWave()` akzeptiert nur 8/16 Bit, berechnet `Samples` aus dem `data`-Chunk des Headers** (`src/audio/AudioInfo.cpp:152`, `162-164`)
  Bei abgeschnittenen Dateien stimmt `info.Samples` nicht mit der realen Datenmenge überein – die Ursache für den kritischen EOF-Befund oben. Eine Korrektur direkt in `IdentWave()` (Begrenzung an `file.size()`) würde alle vier Lesepfade des Wave-Decoders automatisch reparieren.
