# AudioDecoder_Ogg Review – Befunde

Review vom 2026-11-01, Scope: `src/audio/AudioDecoder_Ogg.cpp` (439 Zeilen), `include/pplib/audio/decoder.h` – Klasse `AudioDecoder_Ogg` (Zeilen 576–597).
Öffentliche API: `pplib::AudioDecoder_Ogg`, `pplib::IdentOggVorbisAudioFile`
Review done by: Copilot (Qwen3-8-Flash-Next, Reasoning: medium)

Verifiziert wurde mit Testprogrammen außerhalb der Projektquellen (`/tmp/oggcheck`, nach Review gelöscht) gegen die ASan/UBSan-Builds `build/asan/libpplib_audio.a` + `libpplib_core.a` (GCC 16.2.1, Linux x86-64, libvorbisfile 1.3.7).

## Bugs (kritisch)

- [ ] **`getSamples(STEREOSAMPLE16)` und `addSamples(STEREOSAMPLE32)` führen `position` nie nach** (`src/audio/AudioDecoder_Ogg.cpp:304-363`)
  `position` wird nur in `open()`, `close()`, `seekSample()` und den beiden Float-Varianten (`:395`, `:431`) aktualisiert. Nach `seekSample(0)` + `getSamples(100, buf16)` liefert `getPosition()` weiterhin 0.
  **Verifiziert:** Testprogramm gegen ASan-Build: `getSamples(16) got=100, getPosition()=0`; die Float-Variante korrekt `getPosition()=100`. Wer die Stream-Position über `getPosition()` trackt (Mischen, Fortschrittsanzeige), läuft mit dem 16-Bit-Pfad permanent in Konflikt mit `seekSample()`.
  Minimaler Fix (Mono analog mit `/2`):
  ```cpp
  size_t done = num - (rest / 4);
  position += done;
  return done;
  ```

- [ ] **Erneutes `open()` ohne vorheriges `close()` leakt libvorbis-interne Strukturen** (`src/audio/AudioDecoder_Ogg.cpp:233-255`)
  `ov_open_callbacks()` wird erneut auf ein bereits offenes `OggVorbis_File` gerufen; die beim erstenOpen allokierten Strukturen (`links`, `offsets`, Ogg-/Vorbis-Synthese-Puffer) gehen verloren – bei wiederholtem Öffnen (z. B. Reload eines Streams) ein Dauerleck.
  **Verifiziert:** LeakSanitizer meldet nach doppeltem `open()` 167176 Bytes in 456 Allokationen (`ogg_stream_init`, `ogg_sync_buffer`, `vorbis_synthesis_init`, …). Ohne zweites `open()` meldet derselbe Test keine Lecks.
  Minimaler Fix vor dem `ov_open_callbacks()` in `open()`:
  ```cpp
  ov_clear(&oggp->vf); // nach calloc() nur NULL-Zeiger; ov_clear auf geleerter Struktur gefahrlos
  ```
  **Verifiziert:** Testprogramm ruft `ov_clear()` auf eine per `memset` Null-gesetzte, nie geöffnete `OggVorbis_File` unter ASan – Rückgabe 0, kein Crash/Report. Derselbe Pfad läuft ohnehin jedes Mal im Destruktor über `close()`.

## Bugs (mittel / Warnungen)

- [ ] **`IdentOggVorbisAudioFile()` akzeptiert mehr als 2 Kanäle** (`src/audio/AudioDecoder_Ogg.cpp:183-196`)
  Anders als `IdentWave()`/`IdentAIFF()` (weisen >2 Kanäle ab, `AudioInfo.cpp:188`/`:281`) erkennt die Ogg-Ident auch Multichannel-Dateien; abgelehnt wird erst im `open()` mit `UnsupportedAudioFormatException`. Folge: `IdentAudioFile()` meldet Erfolg, `GetAudioDecoder()` wirft nach dem `new` eine Exception und leakt den Decoder.
  **Verifiziert:** mit ffmpeg erzeugte 5.1-Ogg: `IdentAudioFile` liefert `true, channels=6`; `open()` wirft; `GetAudioDecoder()` leakt laut ASan Decoder plus libvorbis-State (83789 Bytes in 130 Allokationen).
  Minimaler Fix in `IdentOggVorbisAudioFile()` nach erfolgreichem `ov_open_callbacks()`:
  ```cpp
  if (vf->vi->channels < 1 || vf->vi->channels > 2) return false; // Handle-Destruktor uebernimmt ov_clear()
  ```

- [ ] **Integer-Verengung bei großer Samplezahl: `int rest = num * info.BytesPerSample`** (`src/audio/AudioDecoder_Ogg.cpp:308`)
  `num` ist `size_t`; ab ca. 2^29 Samples (rund 2 GB Puffer) wird `rest` negativ, die `while (rest > 0)`-Schleife entfällt komplett, und `return num - (rest / 4)` liefert mehr Samples zurück als dekodiert wurden – der Aufrufer liest uninitialisierten Pufferinhalt. Gleiches Muster in den Float-Varianten: `(long)num` bzw. `(int)(num - total)` (`:370`, `:374`, `:406`, `:410`).
  Minimaler Fix: Klammerung am Funktionsanfang, z. B.
  ```cpp
  const size_t maxnum = static_cast<size_t>(INT_MAX) / 4; // 4 Byte je Frame (16-Bit-Stereo)
  if (num > maxnum) num = maxnum;
  ```

- [ ] **Fehlcode von `ov_pcm_total()` wird als Samplezahl übernommen** (`src/audio/AudioDecoder_Ogg.cpp:174`)
  `ov_pcm_total()` kann negativ fehlschlagen (`OV_EREAD`, `OV_HOLE`); `info.Samples` ist `uint64_t` – daraus wird eine Zahl nahe dem Maximalwert, `info.Length` (`:175-176`) entsprechender Müll.
  Minimaler Fix:
  ```cpp
  ogg_int64_t pcm = ov_pcm_total(&vf, -1);
  if (pcm >= 0) info.Samples = static_cast<uint64_t>(pcm);
  ```

- [ ] **`ppl_tell_func()` liefert auf Windows 32-Bit-Werte und fängt keine Exception** (`src/audio/AudioDecoder_Ogg.cpp:107-110`)
  Der Callback gibt `long` zurück; unter MSVC ist `long` 32 Bit, Dateipositionen über 2 GB werden abgeschnitten (libvorbisfile nutzt `tell` für Chunk-Loop-/Seek-Plausibilitätsprüfungen). Außerdem kann `FileObject::tell()` werfen – die Exception wandert durch die C-Frames von libvorbis (für `read`/`seek` ist das per Catch abgefangen, für `tell` nicht).
  Minimaler Fix:
  ```cpp
  static long ppl_tell_func(void* datasource)
  {
      try {
          uint64_t t = static_cast<FileObject*>(datasource)->tell();
          return (t > LONG_MAX) ? -1 : (long)t;
      } catch (...) { return -1; }
  }
  ```

- [ ] **`seekSample()` ignoriert Fehler von `ov_pcm_seek()` und setzt `position` unbedingt** (`src/audio/AudioDecoder_Ogg.cpp:285-292`)
  Ein Sprung über das Dateiende hinaus schlägt fehl (`OV_EINVAL`), `position` wird trotzdem auf das Ziel gesetzt – `getPosition()` und wirkliche Streamposition driften dauerhaft auseinander. Der Header dazu ist leer; die Nachbarklassen dokumentieren explizit anderes Verhalten (Wave: Begrenzung auf `Samples`; Flac: „Fehlschlag lässt die Position unverändert“). **[Prüfen]** Welche Variante gewünscht ist.

- [ ] **Lesen nach `close()` = NULL-Zeigerzugriff statt Exception** (`src/audio/AudioDecoder_Ogg.cpp:257-278`, `:304` ff.)
  `close()` setzt `private_data = NULL`; jeder folgende Lesecall deklariert `static_cast<OggVorbisPrivateData*>(NULL)` und greift auf `&oggp->vf` zu – UB/Absturz. Wave/Flac dokumentieren „Lesemethoden werfen `DecoderException` nach `close()`“, Ogg schweigt dazu.
  Minimaler Fix (in allen vier Lesecalls und `seekSample()`):
  ```cpp
  if (!private_data) throw DecoderException("AudioDecoder_Ogg: Decoder ist geschlossen");
  ```

- [ ] **`AudioInfo::Mode` wird bei Ogg nie gesetzt** (`src/audio/AudioDecoder_Ogg.cpp:139-181`)
  Alle anderen Ident-Pfade setzen `MONO`/`STEREO` (`AudioInfo.cpp:219`, `:294`, `:324`, `:468`); Ogg lässt `UNKNOWN` stehen. Konsistenzbefund, Auswirkung auf Konsumenten **[Prüfen]**.

## Wartbarkeit & Aufräumen

- [ ] **`ppl_close_func()` ist toter Code** (`src/audio/AudioDecoder_Ogg.cpp:98-105`): entfernen, oder als Kommentar mit Begründung „Datei gehört dem Aufrufer“ auf eine Zeile kürzen.
- [ ] **Falsche Fehlermeldung in `addSamples(STEREOSAMPLE_FLOAT)`** (`src/audio/AudioDecoder_Ogg.cpp:428`): Message lautet `"AudioDecoder_Ogg::getSamples: %d"`; sollte `addSamples` heißen.
- [ ] **Rückgabe-Berechnung hartkodiert `/4` bzw. `/2`** (`src/audio/AudioDecoder_Ogg.cpp:322`, `:341`), während `rest` über `info.BytesPerSample` (`:308`) läuft – bei extern übergebenem `AudioInfo` mit abweichendem `BytesPerSample` widersprechen sich die Größen. Konstante aus `info.BytesPerSample` ableiten oder konsistent aus `Channels` berechnen.

## Dokumentation & Doxygen

- [ ] **Klasse `AudioDecoder_Ogg` ohne jede Doxygen-Doku** (`include/pplib/audio/decoder.h:576-597`)
  Einziger Decoder der Header-Datei ohne Klassen- und Methoden-Kommentare (Wave/AIFF/MP3/Flac sind vollständig dokumentiert). Zu beschreiben wären mindestens:
  - Unterstützte Formate: Vorbis in Ogg, 1–2 Kanäle; Multichannel lehnt `open()` ab (siehe Befund oben).
  - Verhalten ohne libvorbis (`HAVE_LIBVORBIS` undefiniert): Konstruktor wirft `UnsupportedFeatureException`; `open()`/`close()`/`seekSample()` tun stillschweigend nichts, Lesecalls liefern 0 – widerspricht der MP3-Klassendoku („alle Methoden … werfen `UnsupportedFeatureException`“).
  - Sample-Skalierung: 16-Bit-Pfad über `ov_read()` (Integer, native 16-Bit-Skala), Float-Pfad über `ov_read_float()` (normiert, etwa −1,0 bis 1,0) – Tabelle wie bei den Nachbarklassen.
  - `close()`-Kontrakt (zweites `open()` nach `close()` ist erlaubt) und Exception-Verhalten: EOF liefert 0 ohne Exception, Decoderfehler werfen `DecoderException`.
- [ ] **Methoden im Header ohne `@exception`/Vertragsangaben** (`include/pplib/audio/decoder.h:585-596`): Die Basisklasse verspricht `UnsupportedAudioFormatException` in `open()`; ohne libvorbis-Bau kommt dort aber gar keine Exception.

## Befunde in Drittdateien

- [ ] **`GetAudioDecoder()` leakt den Decoder, wenn `open()` wirft** (`src/audio/AudioDecoder.cpp:33-47`)
  `new AudioDecoder_X()` vor `decoder->open(file, &info)`; bei Exception (z. B. Ogg-Multichannel, Multichannel-Flac) wird das Objekt nie freigegeben. Zusätzlich propagiert die Funktion eine Exception, obwohl die Doku „NULL, falls kein passender Decoder“ verspricht.
  **Verifiziert:** ASan-Report 83789 Bytes in 130 Allokationen, Trace über `GetAudioDecoder` (`AudioDecoder.cpp:47`).
- [ ] **`IdentFLAC()` akzeptiert >2 Kanäle, obwohl die Klassen-Doku es anders darstellt** (`src/audio/AudioInfo.cpp:429-468`, `src/audio/AudioDecoder_Flac.cpp:330`)
  Gleiches Muster wie beim Ogg-Befund: Ident erkannt, `open()` wirft; die Flac-Klassenbeschreibung behauptet, solche Dateien würden „nicht als dekodierbares FLAC erkannt“.
- [ ] **Kein Ogg-Decoder-Test** (`tests/src/audio/`): Testdateien für Wave/AIFF/MP3/Flac existieren, aber keine `audiodecoder_ogg.cpp`; `test_44kHz.ogg` wird nur von `audioinfo.cpp` (Ident) benutzt. Nur als Auffälligkeit vermerkt (Code-Coverage nur auf Nachfrage).

## Strukturvorschläge (nur zur Diskussion)

- **PImpl-Aufbau vereinheitlichen** (`include/pplib/audio/decoder.h:578`, `src/audio/AudioDecoder_Ogg.cpp:46-51`): MP3 und Flac nutzen vollständige in der .cpp definierte Zustandsstructen (`Mp3Data`, `FlacState`); Ogg deklariert dagegen `void* private_data` plus sechs Einzelfelder im Header. Nutzen: Formatdetails komplett in der .cpp gekapselt. Aufwand: mittel (alle Zugriffe in `AudioDecoder_Ogg.cpp` anfassen). Risiko: nur Quellkompatibilität für Übersetzer mit veralteten Headern; zur Laufzeit keine.
- **Positions-/Seek-Vertrag über alle Decoder vereinheitlichen**: aktuell weichen Ogg, Wave, MP3 und Flac bei `seekSample()`-Begrenzung, `getPosition()`-Semantik und Verhalten nach `close()` voneinander ab. Einheitliche Doku plus kleine Guards machen das Mixing-Verhalten vorhersagbar. Aufwand: Doku + wenige Prüfungen.
