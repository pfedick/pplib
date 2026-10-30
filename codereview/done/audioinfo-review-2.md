# AudioInfo Review – Befunde

Review vom 2026-10-30, Scope: `src/audio/AudioInfo.cpp` (359 Zeilen), `include/pplib/audio/audioinfo.h` (138 Zeilen).
Nachgelagert geprüft (nur zur Verifikation der Auswirkungen): `src/audio/AudioDecoder_Aiff.cpp` (200), `src/audio/AudioDecoder_Ogg.cpp` (377).
Öffentliche API: `pplib::AudioInfo`, `IdentAudioFile(FileObject&, AudioInfo&)`, `IdentAudioFile(FileObject&)`, `AudioInfo::AudioFormat`, `AudioInfo::ChannelMode`.
Review done by: Qwen Code Reviewer (Copilot SDK in VS Code)

## Prüfergebnis zum ersten Review (`codereview/done/audioinfo-review.md`)

Alle dort dokumentierten Befunde sind abgearbeitet; die Fixes wurden stichprobenhaft gegen den aktuellen Code verifiziert und greifen:

| Fix aus Review 1 | Status | Nachweis |
|---|---|---|
| WAV: `AudioSize`/`Samples` auf Payload begrenzt (`AudioInfo.cpp:235-236,246`) | ✅ | `WaveDataChunkBeyondEof` (`tests/src/audio/audioinfo.cpp:536-548`), Fall `wav_datahuge` (`declaredDataSize=0xFFFFFFF8` → `AudioSize=120`) |
| AIFF: `map()` → `tryMap()`, Chunkgrenzen gegen `fsize` (`:139`, `:143`) | ✅ | `AiffSampleOffsetAndChunkBeyondEof:601-613`; Fälle `aiff_offbig/offhuge/offexact/ssndhuge` |
| AIFF: SSND-Messgrenze `payload + 8 <= fsize` (`:139`) | ✅ | Fall `aiff_ssnd_trunc_hdr` (kurzer SSND-Kopf) → sauber abgelehnt |
| RIFF/FORM-Padding (`if (size & 1) next++`) + Endlosschleifenschutz (`:155-156`, WAV analog) | ✅ | Fälle `wav_baddata_pad`, `aiff_badpad`; kein Hänger in 43 Sondenfällen |
| Kanalzahl-Grenze (`:161`, WAV `:228`) | ✅ | Fälle `aiff_ch3` (→ Ablehnung), WAV 6-ch `EXTENSIBLE` → `sniff=UNKNOWN` und Parse false |
| MPEG-Modus-Byte 3 (`AudioInfo.cpp:268`), ID3-Lookahead-Guard (`:170-172`), `uint32_t`-Frequency (`:136,171`), `Frequency==0`-Guards (`:160`, `:228`) | ✅ | Code-Analyse + Fälle `aiff_freqzero/freqhuge`, `wav_freqzero` |
| `tryMap`/NULL-Check in `IdentMP3` | ✅ | MP3-Trunkierungen ≥24 Byte parsebar (`probe6`), `24zeroes`/`syncjunk`/`id3only` → `UNKNOWN`, keine Exception |
| `bitdepth`-Untergrenze | ⚠️ teilweise | Unterer Guard vorhanden (`:163`, `:229`), **obere** Plausibilitätsgrenze fehlt → Befund 4 |

**Keine neuen Memory-Sicherheits-Regressionen:** 43 handgefertigte Feind-Header (WAV/AIFF, `probe4`) plus MP3-/OGG-/Opus-Korpus samt Trunkierungen (`probe6`) laufen unter ASan+UBSan fehlerfrei durch – kein OOB, keine UB-Tests, kein Null-Zugriff, keine Endlosschleife.
**Erste Review-Liste, offen geblieben (Drittdatei):** Item in `codereview/todo/mp3-review.md:218-219, :235` weiterhin offen (MP3-Decoder, außerhalb dieses Scopes).
**Validierung:** `make build_tests` fehlerfrei; `cd tests && ../build/debug/tests/test_all --gtest_filter='*Audio*:*Wave*:*Aiff*:*Mp3*:*MP3*:*Ogg*'` → **118 Tests grün** (der Filter trifft in `test_core` auf keine Tests – die Audiotests liegen in `test_all`).
**Plattformhinweis:** `readExtended80` (`:92-98`) decodiert 80-Bit-IEEE-754 rein integer-basiert → unabhängig von `long double` (8 Byte auf Windows/ARM/FreeBSD). Korrekt, kein Befund.

## Bugs (kritisch)

- [x] **`IdentAudioFile` wirft Exception bei abgeschnittenem Ogg-Vorbis-Stream (Statt-zurückgabe-`false`-Vertragsbruch) und leaket libvorbis-Allokationen** (`src/audio/AudioInfo.cpp:344` → `src/audio/AudioDecoder_Ogg.cpp:128`, `:55-58`)
  `DetectFormat` erkennt `OggS` + `vorbis` (`AudioInfo.cpp:69-83`, `:312`) und schaltet auf `OGG`; `IdentOggVorbisAudioFile` ruft `ov_open_callbacks`. Dessen Read-Callback `ppl_read_func` (`AudioDecoder_Ogg.cpp:55-58`) gibt `FileObject::read(...)` weiter, das am Dateiende **`pplib::EndOfFileException` wirft**, statt – wie von libvorbis verlangt – kurz zu lesen und die Anzahl gelesener Elemente zu liefern.
  Reproduktion (`tests/testdata/audio/test_44kHz.ogg`, nur erste N Bytes): `N=3000` und `N=300` → `IdentAudioFile(file, info)` wirft `pplib::EndOfFileException` (gemessen: `typeid` = `pplib::EndOfFileException`). Dokumentiert sind für den Parser laut `audioinfo.h:122` nur `FileNotOpenException`, und das API-Versprechen „false bei nicht erkannt/unterstützt“ gilt als verletzt.
  Folge: Die Exception verlässt libvorbis-C-Stackframes (in C compilierte Frames → kein garantiertes Unwinding, auf Windows/MSVC Terminierung möglich; GCC/Linux: Exception schlägt durch, aber Cleanup unterbleibt): **`ov_clear(&vf)` in `AudioDecoder_Ogg.cpp:132` wird nie erreicht**. LeakSanitizer belegt pro Aufruf **34'904 Byte in 6 Allokationen**, alle via `ov_open_callbacks` (`calloc`/`malloc` in libvorbis) → bei wiederholtem Scannen beschädigter Dateien ein wirklicher Dauerleck (Pico-tauglichkeit).
  Fix (Callback Exception-frei + RAII):
  ```cpp
  static size_t ppl_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
  {
      if (size == 0 || nmemb == 0) return 0;
      FileObject* f = static_cast<FileObject*>(datasource);
      try {
          // libvorbis erwartet gelesene ELEMENTE, ein Kurzlesen am EOF ist normal
          return f->read(ptr, size * nmemb) / size;
      } catch (...) {
          return 0;   // EOF/Lesefehler als Signal an libvorbis, nicht als Exception
      }
  }
  ```
  und in `IdentOggVorbisAudioFile` das `OggVorbis_File` RAII-absichern (z. B. Wrapper mit `ov_clear()` im Destruktor oder `try { … } catch (…) { ov_clear(&vf); return false; }`), damit der `false`-Rückgabepfad garantiert wird. Zusätzlich Testfall `TruncatedOggReturnsFalse` in `tests/src/audio/audiodecoder.cpp` ergänzen.

  **Umsetzung:** `ppl_read_func` fängt die EOF-Exception von `FileObject::read()` intern ab und liefert die Zahl gelesener Elemente (Dateiende = Kurzlesen mit 0); `ppl_seek_func` vergleicht die neue Position mit dem erwarteten Ziel und gibt nach libvorbis-Konvention 0 bzw. −1 zurück; ein RAII-Wrapper `OggVorbisHandle` stellt `ov_clear()` auf jedem Rückgabepfad sicher (`src/audio/AudioDecoder_Ogg.cpp`).

  Regressionstest `AudioInfoTest.TruncatedOggVorbisReturnsFalse` in `tests/src/audio/audioinfo.cpp` (die Identifizierung liegt im bestehenden AudioInfo-Suite, nicht in einer separaten Decoderdatei); ohne den Fix schlägt er fehl. LeakSanitizer vorher 34.904 Byte / 6 Allokationen, nachher 0 Meldungen.

## Bugs (mittel / Warnungen)

- [x] **AIFF: `Samples`/`Length` ungeprüft aus COMM übernommen → deklarierte Samplezahl kann die reale Payload übertreffen** (`src/audio/AudioInfo.cpp:167`, `:176`)
  `info.Samples = samples` stammt ungekürzt aus dem COMM-Feld (`:132`), die Payload wird nur über `audioSize` begrenzt (`:146-148`). Bei absichtlich/versehentlich falscher COMM-Angabe entstehen inkonsistente Metadaten:
  - `aiff_samples_over` (COMM `Samples=100000`, Payload 120 Byte = 30 Frames): `Samples=100000`, `Length=2267 ms`, `AudioSize=120`.
  - `aiff_ssndsmall` (`Samples=30`, SSND größer als deklariert → `audioSize=0`): `Samples=30`, `Length=680 ms`, `AudioSize=0`, `AudioEnd=0`.
  - `aiff_offfill` (SSND-`offset=120` → `AudioStart=188 == fsize`): akzeptiert mit `Samples=30`, `Length=680 ms`, `AudioSize=0`; `AudioEnd` bleibt 0 → `AudioStart > AudioEnd`.
  Konsistenz zu WAV: dort werden `Samples` aus der **begrenzten** `audioSize` abgeleitet (`:246`, Test `WaveDataChunkBeyondEof:536-548`), bei AIFF nicht (Test `AiffSampleOffsetAndChunkBeyondEof:601-613` zementiert bisher genau diese Abweichung).
  Reale Auswirkung im Consumer: `AudioDecoder_Aiff` (`src/audio/AudioDecoder_Aiff.cpp:104-105`) mappt `info.AudioStart + position*info.BytesPerSample` und `samples*info.BytesPerSample` Bytes – abgesichert nur über `info.Samples`. `probe5` (3 KI-Fälle, ASan/UBSan-clean) zeigt in allen drei Fällen `pplib::OutOfBoundsException` beim Decode (Fälle `over`, `align`, `empty`), also Absturz des Decoders statt sauberem Ende.
  Fix (in Publikationsblock `:167-176` einfügen, `BytesPerSample` wird dafür bei `:164` bereits berechnet):
  ```cpp
  uint64_t frames = audioSize / bytesPerSample;      // real vorhandene Frames
  if (samples > frames) samples = frames;            // COMM darf nicht über der Payload liegen
  ```
  Erwartungshaltung in `tests/src/audio/audioinfo.cpp:613` (`Samples == 4`) auf den geklemmten Wert (2) anpassen. Zusätzlich: `audioSize == 0 && samples >= 1` ablehnen, sonst bleibt der Zustand `AudioStart >= fsize && AudioEnd == 0` erhalten (siehe nächster Punkt).

  **Umsetzung:** `samples` wird im Publikationsblock von `IdentAIFF` an die reale Payload geklemmt (`if (samples > frames) samples = frames;`). Regression: `AiffSampleOffsetAndChunkBeyondEof` erwartet jetzt `Samples == 2` statt 4, decoderseitig `AiffOverstatedSampleCountDecodesOnlyRealFrames`.

- [x] **AIFF: `audioSize` nicht frame-aligniert** (`src/audio/AudioInfo.cpp:148`)
  Für WAV wird die Payload auf ein Vielfaches der Framegröße gebracht (`audioSize -= audioSize % bytesPerSample`, `:236`), für AIFF fehlt das. Fall `aiff_align` (SSND-`Offset=3`, Stereo/16 bit): `AudioStart=71`, `AudioEnd=187`, `AudioSize=117`, `BytesPerSample=4` → `Samples=30` wollen 120 Byte, vorhanden sind 117 → Decoder liest über `AudioEnd`/EOF hinaus (`probe5` → `OutOfBoundsException`), und `AudioEnd` zeigt mitten in ein Sample.
  Fix: AIFF analog-alignieren, also in den Publikationsblock bei `:174` verschieben/ergänzen:
  ```cpp
  audioSize -= audioSize % bytesPerSample;   // wie WAV, :236
  ```
  (dafür `bytesPerSample`-Berechnung `:164` vor die SSND-Bearbeitung ziehen oder Clip erst bei der Publikation anwenden.) Damit greift auch die Klemmung aus dem vorherigen Punkt sauber.

  **Umsetzung:** `bytesPerSample` wird vor der SSND-Bearbeitung berechnet, die Ausrichtung erfolgt im Publikationsblock (`audioSize -= audioSize % bytesPerSample`). Test `AiffTrailingHalfFrameIsTrimmed`: 117 → 116 Byte, `AudioEnd` 186, `Samples` 29 – damit greift die Klemmung aus dem vorherigen Finding exakt.

- [x] **Keine obere Plausibilitätsgrenze für `bitdepth` → abgeschnittene, unsinnige Member bei „unterstützten“ Dateien** (`src/audio/AudioInfo.cpp:163-165` (AIFF), `:229-233` (WAV))
  Der Guard fängt nur die Untergrenze und den Fall „Kürzung auf 0“ ab. `BitsPerSample`/`BytesPerSample` sind `uint8_t`, `Bitrate` ist `uint16_t`, die Berechnung läuft aber über das ungekürzte `uint16_t bitdepth`:
  - `aiff_bigbits` (`bitdepth=300`): akzeptiert, `BitsPerSample=44`, `BytesPerSample=74`, `Bitrate=26107`.
  - `wav_bigbits` (`bitdepth=516`, mono): akzeptiert, `BitsPerSample=4`, `BytesPerSample=128`, `Bitrate=45158`, `Samples=0`.
  Folge: Consumer (`AudioDecoder_Aiff::open` prüft `BytesPerSample ∈ {4,6}` → lehnt ab; Decodierer ohne solche Prüfung) rechnen mit falschen Framegrößen; `Bitrate` läuft zudem in `uint16_t` über (theoretisch z. B. 8ch/24 bit/192 kHz → 46080 kbit/s).
  Fix: PCM-übliche Obergrenze einführen, z. B. `if (bitdepth > 32) return false;` (bei AIFF `:163`, bei WAV `:229`), danach kann der Rest-Guard entfallen.

  **Umsetzung:** Beide Parser lehnen `bitdepth < 8 || bitdepth > 32` ab (Entscheidung: Obergrenze 32 Bit). Test `BothBitDepth33IsRejected` prüft WAV und AIFF; die bestehenden 24-/32-Bit-Erwartungen bleiben unverändert grün.

- [x] **Inkonstanter Zustand `AudioSize == 0 && Samples > 0 && AudioEnd == 0 < AudioStart` wird akzeptiert** (`src/audio/AudioInfo.cpp:175` AIFF, `:245` WAV)
  Köpfe mit leerer Payload (`audioSize == 0`) werden als „unterstützt“ zurückgegeben; laut Header (`audioinfo.h:99`) ist `AudioEnd` dann „undefiniert“, Consumer (`src/audio/AudioDecoder_MP3.cpp:102`, `AudioDecoder_Wave.cpp:56`) rechnen aber mit `AudioEnd - AudioStart + 1` → Unterlauf. Bei WAV entsteht das durch die Alignmentkürzung (`:236`) bzw. `Samples == 0`, bei AIFF durch `offset`/Payload-Konstellationen (siehe Befund oben).
  Empfehlung: `if (audioSize == 0) return false;` (beide Formate) oder zumindest `Samples = 0` erzwingen und im Header die Bedeutungslosigkeit von `Samples`/`Length` bei leerer Payload dokumentieren.

  **Umsetzung:** Beide Parser lehnen `AudioSize == 0` ab, eine leere Payload ergibt `false` (Entscheidung 2). Tests `EmptyAudioPayloadIsRejected` (beide Container) und `AudioDecoderTest.WaveEmptyPayloadOpenThrows` (`UnsupportedAudioFormatException`).

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [x] **`BytesPerSample`-Formel ignoriert Bitpacking bei 20/24 Bit** (`src/audio/AudioInfo.cpp:164`, `:232`)
  `(bitdepth / 8) * channels` trunciert: 20 bit mono → 2 statt 3 Bytes, 20 bit stereo → 4 statt 6, 30 bit analog. Tests zementieren die Konvention bereits (`Wave20BitMono:571-584` erwartet `BytesPerSample == 2`, `Aiff20BitMono:435-443` erwartet 4), daher sind Samples/`Length`/`Bitrate` bei 20-bit-Dateien falsch (Fall `wav_20bit`: `Samples=60` statt 40). Empfehlung: `(bitdepth * channels + 7) / 8` und die betroffenen Testerwartungen korrigieren; das entfernt zugleich die Notwendigkeit des `bytesPerSample == 0`-Guards.

  **Umsetzung:** Formel auf `((bitdepth + 7) / 8) * channels` korrigiert – je Kanal-Sample auf volle Bytes aufgerundet (entspricht `nBlockAlign` bei WAV bzw. der byte-gepolsterten Sampleablage bei AIFF), dann mit den Kanälen multipliziert: 20 bit mono 3, 20 bit stereo 6 Bytes pro Frame. Testerwartungen angepasst (`Wave20BitPadsEachChannelSampleToThreeBytes`, `Aiff20BitStereoIsRejectedByOpenGuard`, `Wave20BitMonoIsPublishedWithThreeBytesPerFrame`). Ein `bytesPerSample == 0`-Guard ist durch die Untergrenze 8 Bit überflüssig.

- [ ] **Sniff- und Parse-Pfad widersprechen sich** (`src/audio/AudioInfo.cpp:295-312` vs. `IdentAIFF`/`IdentWave`)
  `IdentAudioFile(FileObject&)` (nur Format) akzeptiert Konstellationen, die der Parse-Later ablehnt (z. B. `wav_eof44`, `wav_fmt14`, `wav_nodata` → `Format=WAV`, `IdentAudioFile(…, info) == false`) und lehnt welche ab, die geparst werden (`wav_id3mid`/`AiffSSNDBeforeFMT`: erster Chunk nicht `fmt ` → `UNKNOWN`, aber `IdentWave` findet FMT/SSND und einen ID3v2-Tag). Außerdem unterschiedliche Header-Strenge: FORM-Größe gegen `fsize` validiert (`:301`), RIFF-Größe dagegen ignoriert (`:305`).
  Empfehlung: beide Pfade auf dieselbe Akzeptanzmenge ziehen (Sniff nur als Vorfilter, dessen `UNKNOWN` nicht „nicht audiodatei“ bedeutet) oder explizit im Header dokumentieren, dass `IdentAudioFile(FileObject&)` ein best-effort-Sniff ist.

  **bewusst nicht umgesetzt** (Entscheidung: Wave-Erkennung bleibt von AIFF getrennt, keine gemeinsame Akzeptanzmenge beider Pfade.) Stattdessen dokumentiert: die Format-Überladung von `IdentAudioFile` ist ein best-effort-Sniff, dessen `UNKNOWN` nicht „kein Audio“ bedeutet (`include/pplib/audio/audioinfo.h`).

- [ ] **`IdentAIFF` und `IdentWave` duplizieren Chunk-Walk + Publikation** (`src/audio/AudioInfo.cpp:99-177` vs. `181-251`)
  Nahezu identische Schleifen (`tryMap`, `next`-Berechnung, Pad-Handling, Stillstandschutz), nur Endianness (`PeekN32`/`PeekN16` vs. `Peek32`/`Peek16`) und Feldzuordnung unterscheiden sich – die Unterschiede in diesem Review gefundenen Inkonsistenzen (Frame-Alignment, Sample-Clamping) sind direkte Folge der Verdopplung. Empfehlung: gemeinsamen Chunk-Iterator mit Byte-Order-Policy bzw. kleine `struct ChunkVisitor`-Vorlage; Publikation in eine gemeinsame Funktion mit Format-Flag.

  **bewusst nicht umgesetzt** (Entscheidung: Wave-Erkennung soll bewusst von AIFF getrennt bleiben) – weder gemeinsamer Chunk-Iterator mit Byte-Order-Policy noch eine Publikationsfunktion mit Format-Flag; die zuvor getrennt entstandenen Inkonsistenzen sind über die Tests der beiden Parser getrennt abgesichert.

- [x] **`Bitrate` als `uint16_t`** (`include/pplib/audio/audioinfo.h:104`, berechnet in `AudioInfo.cpp:177`, `:248`)
  Für PCM rechnerisch am oberen Rand (192 kHz/24 bit/2 ch = 9216 kbit/s passt, Mehrkanal/absurde Header laufen über, siehe Befund 4). Wenn schon `uint16_t` behalten wird, dann Plausibilitätsgrenze + `std::clamp` statt stiller Modulo-Kürzung; sonst Typ auf `uint32_t` erweitern (ABI-ändernd, daher mit `REFACTORING.md` abgleichen).

  **Umsetzung:** Typ bleibt `uint16_t` (Entscheidung 5b); stattdessen rechnet `ToKbitRate()` in `uint64_t` und klemmt auf `UINT16_MAX` statt still zu modulieren. Test `ExtremeFrequencyClampsBitrate`: Sample-Rate 100 MHz → `Bitrate == UINT16_MAX` statt 4166.

## Dokumentation & Doxygen

- [x] **Unterstütztes Format-Set nicht dokumentiert** (`include/pplib/audio/audioinfo.h:70-91`, `:111-124`)
  Der Parser akzeptiert bewusst nur: WAV mit `fmt ` als erstem Chunk und Format-Tag 1 (PCM) sowie maximal 2 Kanälen (`AudioInfo.cpp:228`; `EXTENSIBLE`/Float → abgelehnt bzw. gar nicht erst erkannt, Fälle `wav_ext40`, `wav_float`, `wav_pre`), AIFF aber **nicht** AIFF-C (`aiff_aifc` → `UNKNOWN`, Code-Kommentar `:297`), MP3, Ogg-Vorbis (Opus → `UNKNOWN`, Fall `test_48kHz.opus`, weil `isOggVorbis:71-81` nach demLiteral `vorbis` sucht). Das gehört an die Doxygen von `IdentAudioFile`/`AudioFormat`, sonst ist `UNKNOWN` für Caller nicht interpretierbar („kein Audio“ vs. „nur PCM“ vs. „Opus“).

  **Umsetzung:** Doxygen von `AudioFormat` und beiden `IdentAudioFile`-Überladungen nennt jetzt die akzeptierte Teilmenge: WAV mit `fmt ` als erstem Chunk, Format-Tag 1, maximal 2 Kanälen und 8..32 Bit (`EXTENSIBLE`, Float, Mehrkanal abgelehnt); AIFF, aber nicht AIFF-C; Ogg nur Vorbis (Opus/FLAC → `UNKNOWN`); MP3 Layer 1–3.

- [x] **Semantik von `Samples` formatabhängig, nicht dokumentiert** (`include/pplib/audio/audioinfo.h:101`)
  WAV: aus gekürzter Payload (`AudioInfo.cpp:246`); AIFF: aus COMM (`:167`); OGG: `ov_pcm_total` (`AudioDecoder_Ogg.cpp:113`); MP3: Schätzung des MP3-Parsers (`AudioInfo.cpp:260`). Für AIFF/Ogg/MP3 gilt `Samples * BytesPerSample != AudioSize` – nur bei WAV/AIFF wäre die Übereinstimmung zu erwarten. Kurz in der Member-Doku festhalten (und nach Fix von Befund 2: AIFF-Samples ebenfalls payload-begrenzt).

  **Umsetzung:** Member-Doku `Samples`: `Samples * BytesPerSample == AudioSize` gilt nur für WAV/AIFF; bei Ogg kommt die Zahl vom Container (`ov_pcm_total`), bei MP3 schätzt der Parser. AIFF-`Samples` sind zusätzlich payload-begrenzt (Befund 2).

- [x] **`AudioEnd`-Hinweis präziser fassen** (`include/pplib/audio/audioinfo.h:99`)
  „undefiniert wenn `AudioSize == 0`“ ist zu dürftig: Bei AIFF kann zugleich `AudioStart >= fsize` gelten (Fall `aiff_offfill`: `AudioStart=188`, `AudioEnd=0`, `AudioSize=0`). Empfehlung: „`AudioStart <= AudioEnd` gilt nur wenn `AudioSize > 0`; bei `AudioSize == 0` ist `AudioEnd` nicht gesetzt“ bzw. (bei Umsetzung von Befund 5) `AudioSize == 0` gar nicht zulassen.

  **Umsetzung:** Doku jetzt: `AudioEnd - AudioStart + 1 == AudioSize`, wobei `AudioSize` bei WAV/AIFF frame-aligniert und > 0 ist; der Fall `AudioStart >= fsize` bei `AudioEnd == 0` existiert nicht mehr, weil leere Payload abgelehnt wird (Befund 5).

- [x] **`BytesPerSample`-Doku: Einheiten klarer** (`include/pplib/audio/audioinfo.h:106`)
  „Bytes pro Sample (alle Kanäle zusammen)“ ist mißverständlich, weil `BitsPerSample` sich auf *ein* Kanal-Sample bezieht (`:105`). Besser: „Bytes pro **Frame** = alle Kanäle eines Sample-Zeitpunkts“ (Formel `(bitdepth/8)*channels`).

  **Umsetzung:** `BytesPerSample` ist jetzt als „Bytes pro Frame (alle Kanäle eines Sample-Zeitpunkts)“ beschrieben, `BitsPerSample` je Kanal; die Formel wird als gerundete Bytezahl je Kanal mal Kanäle angegeben statt als `(bitdepth/8)*channels`.

## Befunde in Drittdateien

- [x] **`ppl_read_func` verträgt kein EOF** (`src/audio/AudioDecoder_Ogg.cpp:55-58`)
  Hauptursache des kritischen Befunds oben (Exception statt Kurzlesen/Fehlsignal).

  **Umsetzung:** siehe kritischer Befund oben: Callback-Aufrufe laufen exceptionsicher, Dateiende/Lesefehler werden als Kurzlesen statt als Exception gemeldet (`src/audio/AudioDecoder_Ogg.cpp:55-77`). Abgedeckt durch `AudioInfoTest.TruncatedOggVorbisReturnsFalse`.

- [x] **`ppl_seek_func` missachtet die libvorbis-Callback-Konvention** (`src/audio/AudioDecoder_Ogg.cpp:60-64`)
  `FileObject::seek(int64_t, SeekOrigin)` (`include/pplib/core/fileobject.h:441`) liefert die **neue Position**; libvorbis verlangt laut `vorbisfile.h` `0` bei Erfolg und `!= 0` als Fehler. Jeder erfolgreiche Seek auf eine von 0 verschiedene Position wirkt damit als Fehler (z. B. `OV_EREAD`). Im aktuellen Testkorpus wird der Callback während `ov_open_callbacks` nicht ausgeführt (Dateien ≤ 45 kB, eigenes Callback-Repro ergab `seeks=0` → nicht beobachtbar), bei größeren/segmentierten Streams ist mit Abbruch zu rechnen. Fix: `return (f->seek(...) == erwartetePosition) ? 0 : -1;`; zusätzlich Rückgabe > `UINT32_MAX` abschneiden vermeiden.

  **Umsetzung:** Wie empfohlen: `return (f->seek(offset, origin) == (uint64_t)target) ? 0 : -1;` mit vorab berechnetem `target` (SEEKCUR/SEEKEND aufgelöst), unbekannte `whence` und Ziel < 0 ergeben −1; der Offset wird als `ogg_int64_t` übernommen, ein Abschneiden bei Größen über `UINT32_MAX` entfällt damit.

- [x] **OGG: `FileSize`-Trunkierung und Bitrate-Einheiten** (`src/audio/AudioDecoder_Ogg.cpp:92`, `:96-100`)
  `info.FileSize` (`uint64_t`) wird aus `(uint32_t)file.size()` befüllt → Dateien > 4 GB bekommen eine abgeschnittene Größe. In `GetVorbisAudioInfo` wird `bitrate_nominal / 1000` (`:96`) gegen `bitrate_lower` ohne `/1000` (`:99`) gemischt → inkonsistente Bitrate-Einheiten je nach Encodereinstellung.

  **Umsetzung:** `info.FileSize` wird ohne `(uint32_t)`-Cast aus `file.size()` befüllt; Bitrate durchgängig in kbit/s (`bitrate_nominal / 1000`, `bitrate_upper.bitrate / 1000`) mit Klemmung auf den `uint16_t`-Bereich (`ToKbitRate`-Entsprechung in `GetVorbisAudioInfo`).

- [x] **Decoder-Mapping basiert auf `info.Samples`** (`src/audio/AudioDecoder_Aiff.cpp:104-105`)
  Liest `info.AudioStart + position*info.BytesPerSample` über `samples*info.BytesPerSample` Bytes; nur durch `info.Samples` begrenzt. Nach AudioInfo-seitiger Klemmung (Befund 2) entfällt die Exception; dennoch defensiv `position + samples <= info.Samples` bzw. `<= AudioSize/BytesPerSample` prüfen.

  **Umsetzung:** dateilokale Funktion `ClampToAudioSize()` in `src/audio/AudioDecoder_Aiff.cpp` und `src/audio/AudioDecoder_Wave.cpp` begrenzt alle vier Fetch-Funktionen beider Decoder: Start muss `AudioStart` sein, `samples` wird auf `Samples` bzw. die frame-alignierte Payload geklemmt, außerhalb der Payload wird 0 statt ein Lesezugriff geliefert.

  Tests `AiffHostileAudioIsClampedToPayload` und `WaveHostileAudioInfoIsClampedToPayload`: ohne die Klemmung wirft AIFF `OutOfBoundsException` und WAV liest hinter das Dateiende (von ASan als OOB-Zugriff gemeldet).

- [ ] **`AudioInfo::AudioEnd` in Consumern ohne `AudioSize`-Check** (`src/audio/AudioDecoder_MP3.cpp:102`, `src/audio/AudioDecoder_Wave.cpp:56`)
  Rechnung `AudioEnd - AudioStart + 1` unterläuft, falls `AudioSize == 0` durchgelassen wird; mit Befund 5 (Ablehnung leerer Payload) erledigt sich das, sonst `if (info.AudioSize == 0)` vorher abfangen.

  **bewusst nicht umgesetzt** – Prämisse veraltet: kein Consumer berechnet `AudioEnd - AudioStart + 1`. Die Differenz entsteht nur beim Parsen (`src/audio/AudioInfo.cpp:194`, `:265`, `:278`) und in `src/audio/AudioDecoder_Ogg.cpp:170`; `AudioDecoder_Wave.cpp` rechnet nicht mit der Länge und `AudioDecoder_MP3.cpp:101` setzt nur `curPos = info.AudioStart`. Da die Parser leere Payload ablehnen (Befund 5), ist ein Unterlauf zusätzlich ausgeschlossen.

- [ ] **Aus Review 1 offen gebliebene MP3-Punkte** (`codereview/todo/mp3-review.md:218-219`, `:235`)
  Weitere Abarbeitung steht weiterhin aus (MP3-Identifikation/Decoder, außerhalb des AudioInfo-Scopes).

  **bewusst nicht umgesetzt** – außerhalb des AudioInfo-Scopes; die Punkte bleiben in `codereview/todo/mp3-review.md:218-219`, `:235` stehen und werden in einer eigenen MP3-Runde abgearbeitet.