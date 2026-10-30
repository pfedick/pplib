# AudioDecoder_Aiff Review – Befunde

Review vom 2026-10-21, Scope: `src/audio/AudioDecoder_Aiff.cpp` (200 Zeilen).
Öffentliche API: `pplib::AudioDecoder_Aiff` (`open`, `close`, `getAudioInfo`, `seekSample`, `getPosition`, `getSamples(size_t, STEREOSAMPLE16*)`, `addSamples(size_t, STEREOSAMPLE32*)`, `getSamples(size_t, STEREOSAMPLE_FLOAT*)`, `addSamples(size_t, STEREOSAMPLE_FLOAT*)`).
Review done by: Copilot (Copilot SDK in VS Code)

Alle Befunde wurden mit Probeprogrammen gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` (GCC 14, Linux/x86-64, `-g -O0`) verifiziert. Testdateien: ffmpeg-`pcm_s16be`/`pcm_s24be`-AIFFs sowie handgeschriebene AIFFs mit bekanntem Byte-Muster (Padding, Sign-Magnitude, gefälschte COMM-Felder).

> **Hinweis zur Abarbeitung:** Die Befunde beschreiben den Stand vor der Überarbeitung. Die Zeilenangaben in den Titeln stammen aus der damaligen Fassung von `AudioDecoder_Aiff.cpp` (198 Zeilen, vier separate Lesemethoden). Zwischenzeitlich gab es eine gemeinsame Template-Kernmethode; auf Wunsch des Betreuers ist die Datei wieder je Lesemethode ausgeschrieben (aktuell 444 Zeilen), die für alle Formate identische Grenzziehung steckt in `prepareRead()`. Die NOTIZEN unter jedem Befund verweisen auf die aktuellen Stellen.

> Messung nach der Überarbeitung mit `make coverage` (gcovr, GCC 14): `src/audio/AudioDecoder_Aiff.cpp` 255/265 Zeilen (96.2 %) und 167/189 Zweige (88.4 %), `src/audio/AudioInfo.cpp` 223/225 Zeilen. Nicht ausgelaufen ist dort nur noch defensiver Code: die vier `default:`-Zweige der Format-`switch` (das private `encoding` hat nach `open()` immer einen der vier unterstützten Werte) und der leere Zerstörer, den der Compiler an den Aufrufstellen inline auflöst.

Referenz auf Basisreview: [audiodecoder-review.md](./audiodecoder-review.md).

## Bugs (kritisch)

- [x] **NULL-Zeigerdereferenz in allen vier Lesemethoden nach `close()`** (`src/audio/AudioDecoder_Aiff.cpp:108`, `132`, `156`, `180`)
  `close()` setzt `ff = NULL` (Zeile 81), die Lesemethoden rufen aber ungeprüft `ff->map(...)` auf:
  ```cpp
  const char* data = ff->map(info.AudioStart + position * info.BytesPerSample, samples * info.BytesPerSample);
  ```
  Verifiziert: `open(); close(); getSamples(…, STEREOSAMPLE16*)` → SIGSEGV, Exit-Code 139. Da der Destruktor `close()` aufruft (Zeile 49), ist jeder Ablauf „objektweit open → close → weiterlesen" betroffen; `FileObject::map()` wirft zwar `FileNotOpenException`, wenn das File selbst geschlossen ist (Zeile 752 in `src/core/File.cpp`), aber hier ist der **Zeiger auf das File-Objekt** NULL, davor gibt es keine Prüfung.
  Behebung (in allen vier Methoden identisch):
  ```cpp
  if (!ff) throw DecoderException("AudioDecoder_Aiff: not open");
  ```
    **Umsetzung:** In `prepareRead()` steht jetzt als erste Prüfung `if (!ff) throw DecoderException("No file opened");` (`src/audio/AudioDecoder_Aiff.cpp:189`), die alle vier Lesemethoden abschneidet. Regulär getestet in `AudioDecoderTest.AiffReadWithoutOpenOrAfterCloseThrows` (vor `open()` und nach `close()`); derselbe Ablauf läuft zusätzlich im ASan-Build (`make asan`) ohne Meldung durch.

- [ ] **24-Bit-Samples: fester 6-Byte-Stride ignoriert AIFF-Sample-Padding** (`src/audio/AudioDecoder_Aiff.cpp:115-122`, `139-146`, `163-170`, `187-194`; Ursache `src/audio/AudioInfo.cpp:93`)
  Der Decoder nimmt für 24 Bit genau 3 Bytes pro Kanal an (`data += 6`). Die AIFF-Spezifikation schreibt aber vor, dass Samples auf eine 16-Bit-Wortgrenze aufgefüllt werden — ein 24-Bit-Sample belegt dann 4 Bytes (8 Bytes pro Stereo-Frame). `IdentAIFF` leitet `BytesPerSample` ausschließlich aus der Bitbreite ab (`(PeekN16(adr+14) * info.Channels)/8`, Zeile 93) und meldet damit 6, unabhängig davon, was die Datei tatsächlich enthält.
  Verifiziert an zwei 24-Bit-Stereo-Dateien:
  - `/tmp/padded24.aiff` (gepolstert, 4 Bytes/Sample, `SSND` = 72000 Bytes bei 9000 Frames): Decoder meldet `BytesPerSample=6` und liefert ab Frame 2 Datenmüll — `[2]=255/-256`, `[3]=2/-1`, `[4]=0/768`, `[5]=-3/0` (erwartet 0/0). Zusätzlich werden nur 9000 · 6 = 54000 der 72000 Audio-Bytes erreicht; die letzten 18000 Bytes (Frames 6750-8999) sind unerreichbar.
  - `/tmp/a24f.aiff` (ffmpeg `pcm_s24be`, gepackt, 3 Bytes/Sample): Decoder liefert korrekt `60/60`, `350/350`, `723/723`, `1059/1059`, …
  Der Decoder funktioniert also nur mit ungepolsterten Dateien. Welche Encoder polstern, ist nicht verifiziert — die Spezifikation erlaubt/es verlangt das Auffüllen, daher ist das ein reales Kompatibilitätsrisiko, kein hypothetisches.
  Behebung: Stride nicht aus der Bitbreite raten, sondern aus den Chunk-Daten ableiten (in `IdentAIFF`):
  ```cpp
  // nach SSND-Chunk: info.AudioSize ist bereits bekannt, info.Samples aus COMM
  uint32_t bytes_per_frame = (info.Samples > 0) ? (info.AudioSize / info.Samples) : 0;
  info.BytesPerSample = bytes_per_frame;   // tatsächlicher Frame-Stride (4 bei 16 Bit, 6 oder 8 bei 24 Bit)
  ```
  Im Decoder dann `const size_t stride = info.BytesPerSample;` statt der Konstanten 4/6 verwenden und die Sample-Position innerhalb des Frames aus `BitsPerSample` ableiten.
    **Umsetzung:** Bewusst nicht umgesetzt — unterstützt werden nur ungepolsterte (gepackte) Frames; alle in der Praxis erzeugten Dateien (Audacity, ffmpeg `pcm_s24be`) schreiben gepackt, und die Obergrenze `BitsPerSample <= 32` bleibt erhalten. Abgesichert wird der Fall über die Konsistenzprüfung `BytesPerSample == ((BitsPerSample + 7) / 8) * Channels` in `open()` (`src/audio/AudioDecoder_Aiff.cpp:83`), die ungepolsterte Rahmen mit `UnsupportedAudioFormatException` ablehnt (`AudioDecoderTest.AiffOpenRejectsFrameSizeNotMatchingDepthAndChannels`); in `decoder.h` ist der gepackte Frame als Annahme dokumentiert.

- [ ] **Zweierkomplement statt Sign-Magnitude interpretiert** (`src/audio/AudioDecoder_Aiff.cpp:111-112`, `117-118`, `135-136`, `141-142`, `159-160`, `165-166`, `183-184`, `189-190`)
  Die AIFF-Spezifikation definiert Sample-Werte als **Sign-Magnitude**, der Decoder interpretiert die Bytes als Zweierkomplement (`(int16_t)PeekN16(data)` bzw. `(int32_t)(PeekN24(data) << 8) >> 8`).
  Verifiziert an einer handgeschriebenen 16-Bit-Stereo-Datei `/tmp/sm16.aiff` mit den Rohwerten `0x7FFF,0x8001 / 0xFFFF,0x0001 / 0x8000,0x1234`:
  Decoder: `[0]=32767/-32767`, `[1]=-1/1`, `[2]=-32768/4660`.
  Spezifikationsgemäß wären es `[0]=32767/-1`, `[1]=-32767/1`, `[2]=0/4660`.
  Alle negativen Samples einer spec-konformen Datei sind damit komplett falsch (Größenordnung statt Kleinsignal). Einschränkung der Beweislage: ffmpeg schreibt AIFF im Zweierkomplement, moderne Encoder halten sich offenbar mehrheitlich nicht an Sign-Magnitude — welche Encoder spec-konform schreiben, wurde nicht untersucht. Der Befund ist deshalb als **Spezifikations-/Interoperabilitätsrisiko** zu bewerten, nicht als „alle Dateien kaputt".
  Behebung: Verhalten explizit machen — entweder
  1. in `decoder.h` dokumentieren, dass nur Zweierkomplement-AIFFs unterstützt werden (und die Klasse heißt dann praktisch „unofficial AIFF"), oder
  2. Sign-Magnitude korrekt anwenden und für Zweierkomplement-Dateien eine Erkennung/Option anbieten. Ein robustes Automatik-Kriterium gibt es nicht; ein Heuristik-Test über die ersten Frames (Anteil ausgeschlagener Bits) ist möglich, aber unsicher.
    **Umsetzung:** Variante 1 gezogen: `decoder.h` dokumentiert jetzt ausdrücklich, dass nur Zweierkomplement gelesen wird (Audacity und ffmpeg schreiben durchweg Zweierkomplement, Sign-Magnitude-AIFFs werden nicht unterstützt). Umsetzung und Skalierung stehen als Tabelle im Klassenblock, damit die Konvertierung für Anwender eindeutig ist.

- [x] **`addSamples(size_t, STEREOSAMPLE32*)` addiert 16-Bit- und 24-Bit-Quellen in unterschiedlichen Skalen** (`src/audio/AudioDecoder_Aiff.cpp:133-146`)
  Im 16-Bit-Zweig werden Rohwerte des Bereichs ±32767 aufaddiert (Zeile 135-136), im 24-Bit-Zweig die vollen 24-Bit-Werte ±8388607 (Zeile 143-144) — anders als in `getSamples(..., STEREOSAMPLE16*)`, wo 24 Bit auf 16 Bit heruntergerechnet wird (Zeile 119-120).
  Folge: Wird eine 24-Bit-Datei mit einer 16-Bit-Datei über `addSamples(…, STEREOSAMPLE32*)` gemischt, ist die 24-Bit-Spur 256× zu laut; zudem ist der Wertebereich von `SAMPLE32` (int32_t) bei 24-Bit-Mehrkanal-/Mehrfachüberlagerung deutlich schneller ausgeschöpft.
  Behebung: einheitliche Ziel-Skala definieren und im Header dokumentieren. Für Konsistenz mit dem 16-Bit-Pfad:
  ```cpp
  buffer[i].left  += l >> 8;   // wie getSamples(STEREOSAMPLE16*): 24 Bit -> 16-Bit-Wertebereich
  buffer[i].right += r >> 8;
  ```
  (Oder umgekehrt den 16-Bit-Zweig auf 24-Bit-Skala hochskalieren — dann muss das aber in `decoder.h` stehen.)
    **Umsetzung:** Gemischt wird jetzt durchgehend auf 16-Bit-Pegel: 24 Bit `>> 8`, 32-Bit-Integer `>> 16`, Float `* 32768.0f` (Vorzeichen erhalten), also exakt die Werte, die auch `getSamples(…, STEREOSAMPLE16*)` liefert. Getestet in `AudioDecoderTest.AiffTwentyFourBitMixBufferUsesSixteenBitLevel` (`0x010203` -> `0x0102`), `AiffThirtyTwoBitIntegerDownscaled` und `AiffFloatSamplesScaleToAllTargets`; die Regel steht als Skala-Tabelle im Klassenblock von `decoder.h`.

- [x] **`info.Samples` wird nie gegen `AudioSize`/Dateigröße geprüft → `OverflowException` statt sauberem Dateiende** (`src/audio/AudioDecoder_Aiff.cpp:107-108`, Ursache `src/audio/AudioInfo.cpp:92`)
  `info.Samples` kommt ungeprüft aus dem COMM-Chunk (Zeile 92), die Lesebegrenzung in Zeile 107 verlässt sich darauf. `File::map()` wirft bei Bereichsüberschreitung `OverflowException` (`src/core/File.cpp:769`) statt `nullptr`.
  Verifiziert an `/tmp/aiff_lie.aiff` (COMM-Feld `numSampleFrames` von 44100 auf 100000 geändert, Datei unverändert 176454 Bytes): `seekSample(44000)`; `getSamples(4096, s)` → `OverflowException` (statt erwarteterweise 100 Samples oder 0).
  Behebung: In `IdentAIFF` `info.Samples = min(info.Samples, info.AudioSize / BytesPerSample)` (nach Bestimmung von `AudioSize`), und im Decoder zusätzlich die Byte-Anzahl an die Dateigrenze anpassen:
  ```cpp
  size_t avail = (info.AudioStart + samples * info.BytesPerSample > ff->size())
                     ? static_cast<size_t>(ff->size() - info.AudioStart) / info.BytesPerSample
                     : samples;
  if (avail < samples) samples = avail;
  ```
    **Umsetzung:** Beide Stufen sind drin: `IdentAIFF` begrenzt `Samples` auf `AudioSize / BytesPerSample` (`src/audio/AudioInfo.cpp:214`), und `prepareRead()` klemmt Start bzw. Länge zusätzlich an `ff->size()` (`src/audio/AudioDecoder_Aiff.cpp:202-204`, Helfer `ClampToAudioSize()` ab Zeile 47). `AudioDecoderTest.AiffOverstatedSampleCountDecodesOnlyRealFrames` (COMM-Sollmenge 4, Datei hat 2 Frames), `AiffHostileAudioInfoIsClampedToPayload` und `AiffHostileAudioSizeBeyondFileIsClampedToFileEnd` prüfen das; eine Exception gibt es beim Dateiende nicht mehr, nur `0`.

- [x] **`open()` prüft `BitsPerSample` und `BytesPerSample` unabhängig voneinander** (`src/audio/AudioDecoder_Aiff.cpp:71-74`)
  Ein `AudioInfo` mit `BitsPerSample=16` und `BytesPerSample=6` passiert beide Prüfungen; die Lesemethoden laufen dann mit 16-Bit-Logik (Stride 4) über einen 6-Byte-Stride.
  Verifiziert: `open(f, &info)` mit `Bits=16, Bpp=6` an einer 24-Bit-Datei → keine Exception, Ergebnisse `[0]=60/-26368`, `[1]=15513/350`, `[2]=24833/24161`, `[3]=723/-8446`.
  Behebung: Kombination prüfen, z. B.
  ```cpp
  if (this->info.BitsPerSample == 16 && this->info.BytesPerSample != 4) throw UnsupportedAudioFormatException("16 bit requires 4 bytes/frame");
  if (this->info.BitsPerSample == 24 && this->info.BytesPerSample != 6 && this->info.BytesPerSample != 8)
      throw UnsupportedAudioFormatException("24 bit requires 6 or 8 bytes/frame");
  ```
    **Umsetzung:** Es gibt jetzt eine einzige Kombinationsprüfung: `info.BytesPerSample != (((info.BitsPerSample + 7) / 8) * info.Channels)` -> `UnsupportedAudioFormatException` (`src/audio/AudioDecoder_Aiff.cpp:83`, Bytes je Kanal in Zeile 82), zusätzlich Formatvergleich (Zeile 76) und Kanal 1..2 (Zeile 78). `AudioDecoderTest.AiffOpenRejectsFrameSizeNotMatchingDepthAndChannels` (u. a. 16 Bit mit `BytesPerSample=6`), `AiffAudioInfoOfAnotherFormatIsRejected` (fremdes Format, `Channels=5`, `Channels=0`) und `Aiff20BitStereoIsRejectedByOpenGuard` prüfen die Mischkonstellationen.

## Bugs (mittel / Warnungen)

- [x] **Bei `BitsPerSample ∉ {16, 24}` werden Samples „gelesen", ohne Daten zu liefern** (`src/audio/AudioDecoder_Aiff.cpp:123-125`, `147-149`, `171-173`, `195-197`)
  Haben die `if/else if`-Ketten keinen Treffer, wird die Schleife komplett übersprungen, `position += samples` und `return samples` ausgeführt — der Aufrufer glaubt, `samples` gültige Samples erhalten zu haben (der Puffer bleibt unbeschrieben). Über `open()` ist dieser Fall aktuell ausgeschlossen; die Methoden sind aber auch für direkt gesetztes `info` ausgelegt und die Basisklasse garantiert nichts.
  Behebung: `else throw UnsupportedAudioFormatException("unsupported BitsPerSample");` bzw. `return 0;` ohne `position`-Vorschub.
    **Umsetzung:** Die if-Ketten sind weg: `open()` entscheidet einmal per `switch` über `BitsPerSample` (16/24/32 plus `FL32` bei AIFF-C) und wirft im `default:` `UnsupportedAudioFormatException` (`src/audio/AudioDecoder_Aiff.cpp:86-101`); die Leseschleifen kennen nur noch `encoding`. `AudioDecoderTest.AiffOpenRejectsSampleFormatesOutsideSupport` prüft 8, 12 und 20 Bit, `Aiff20BitStereoIsRejectedByOpenGuard` denselben Fall mit vom Aufrufer gesetztem `info`.

- [x] **Möglicher Unterlauf in `samples = info.Samples - position`** (`src/audio/AudioDecoder_Aiff.cpp:107`, `131`, `155`, `179`)
  Ist `position > info.Samples` (z. B. nach `open()` mit einem zweiten, kleineren `AudioInfo`, ohne dass `position` zurückgesetzt würde — `open()` setzt `position = 0` in Zeile 75, aber `seekSample()` clampet an ein ggf. veraltetes `info`), wird `info.Samples - position` zu einem riesigen `size_t` und `map()` entsprechend fehlparametrisiert. Aktuell nicht erreichbar, aber eine reine Verteidigungszeile behebt das:
  ```cpp
  if (position >= info.Samples) return 0;
  size_t samples = std::min(num, info.Samples - position);
  ```
    **Umsetzung:** Der Vorschlag ist wörtlich übernommen: `if (position >= info.Samples) return 0;` vor der Subtraktion (`src/audio/AudioDecoder_Aiff.cpp:192`, zweite Grenze in `ClampToAudioSize()` Zeile 51) und die Begrenzung nach unten in Zeile 202. `AudioDecoderTest.AiffZeroSampleReadsLeavePositionUnchanged` sichert zusätzlich ab, dass `num == 0` nichts liest und `position` unverändert lässt; `seekSample()` clamppt an das aktuelle `info` (`src/audio/AudioDecoder_Aiff.cpp:130-134`).

- [x] **`close()` setzt `info` nicht zurück** (`src/audio/AudioDecoder_Aiff.cpp:79-84`)
  Nach `close()` liefert `getAudioInfo()` (Zeile 86-89) weiterhin die Werte der alten Datei. Zusammen mit `seekSample()` (Clamping an das alte `info.Samples`, Zeile 93-96) kann ein wiederverwendetes Objekt an einer Position landen, die zur neuen Datei nicht passt.
  Behebung: `info = AudioInfo{};` in `close()` — oder in `decoder.h` garantieren, dass `getAudioInfo()` nach `close()` gültig bleibt (dann aber in allen vier Decodern einheitlich).
    **Umsetzung:** `close()` setzt jetzt `info = AudioInfo()` zurück (`src/audio/AudioDecoder_Aiff.cpp:121`, zusammen mit `ff`, `position`, `bytesPerChannel` und `encoding`), und `decoder.h` dokumentiert, dass `getAudioInfo()` nur bei geöffnetem Dateiobjekt sinnvolle Werte liefert. Damit kann ein wiederverwendetes Objekt nicht an einer Position des alten `info` landen; `AudioDecoderTest.AiffReadWithoutOpenOrAfterCloseThrows` prüft den Zustand nach `close()`.

- [x] **Mono-AIFFs werden abgelehnt, aber von `IdentAudioFile` akzeptiert** (`src/audio/AudioDecoder_Aiff.cpp:70`)
  `IdentAIFF` akzeptiert `Channels == 1` (`src/audio/AudioInfo.cpp:86-87`), der Decoder wirft `UnsupportedAudioFormatException("Channels != 2")`. Über `GetAudioDecoder()` führt das zum Leak des bereits erzeugten Objekts (Basisreview, Befund „`GetAudioDecoder()` leckt das Decoder-Objekt"). Verifiziert: Mono-AIFF → `1.760 bytes in 20 blocks definitely lost` (Allokation `AudioDecoder.cpp:44`), ohne `try/catch` des Aufrufers `terminate`.
  Behebung: Mono unterstützen (Kanal duplizieren, wie OGG/MP3 es tun) **oder** bereits in `IdentAIFF` auf Stereo beschränken, damit die Fabrik gar keinen Decoder erzeugt.
    **Umsetzung:** Mono wird jetzt unterstützt: in jeder Leseschleife wird der einzige Kanal auf links und rechts dupliziert: der Local `stereo` heißt (`src/audio/AudioDecoder_Aiff.cpp:219`, `279`, `340`, `398`) und die zweite Kanalzeile lautet überall `... : l` (zum Beispiel Zeile 225), `open()` lässt `Channels == 1` zu. Getestet mit der echten Audacity-Datei `tests/testdata/audio/test_44kHz_16bit_mono.aiff` (`AiffRealFixturesDecodeToTheSameSixteenBitValues`) sowie inline in `AiffMonoChannelIsReplicated` (16-Bit- und Float-Ziel) und `AiffMonoFloatSourceIsReplicatedAndClipped`.

- [x] **`File::map()` pro Aufruf — Remapping bei jeder sequentiellen Leseoperation** (`src/audio/AudioDecoder_Aiff.cpp:108`, `132`, `156`, `180`)
  `File::map()` liefert nur dann einen bestehenden Mapping-Bereich zurück, wenn die angeforderte Region vollständig in der letzten liegt (`src/core/File.cpp:757-759`); sonst `munmap` + `mmap`. Da `position` bei jedem Aufruf weiterwandert, wird bei sequentiellem Playback **pro `getSamples()`-Aufruf** ein Syscall-Paar ausgeführt. Standardmäßig ist `ReadAhead = 0` (`src/core/File.cpp:88`), es gibt also keinen Puffer. Auf Plattformen ohne `mmap` (RP2040/Pico) ist der Pfad grundsätzlich nicht nutzbar.
  Behebung: `ff->setMapReadAhead(64 * 1024)` nach `open()`, oder auf `read()` in einen internen Ringpuffer umstellen (einheitlich mit Wave/MP3/OGG).
    **Umsetzung:** Nach `open()` steht `ff->setMapWindowSize(32 * 1024)`, sofern der Aufrufer nicht schon einen größeren Wert gesetzt hat (`src/audio/AudioDecoder_Aiff.cpp:111`; der im Befund genannte Name heißt `setMapReadAhead()` nicht, das Mappingfenster heißt `setMapWindowSize()`/`ReadAhead`). Damit bleibt `map()` bei normalem Playback meist bei einem Mapping, ohne dass der Wert wieder zurückgenommen wird. Belegt durch `AudioDecoderTest.AiffRaisesSmallMapWindowAndKeepsALargerOne` (4 KiB -> 32 KiB, 64 KiB bleibt 64 KiB); Umstellung auf `read()`+Ringpuffer bewusst nicht gemacht, weil es alle vier Decoder gleichzeitig betrifft.

- [ ] **24→16-Bit-Downscale ohne Rundung** (`src/audio/AudioDecoder_Aiff.cpp:119-120`)
  `(int16_t)(l >> 8)` schneidet die niederen 8 Bits ab; bei negativen Werten wird Richtung −∞ gerundet, was einen konstanten Bias von bis zu 1 LSB (und bei kleinen Signalen sichtbares Rauschen) erzeugt.
  Behebung: `(int16_t)((l + 0x80) >> 8)` (mit Sättigung), oder bewusst dokumentieren, dass trunciert wird.
    **Umsetzung:** Bewusst nicht umgesetzt, sondern dokumentiert: alle Downscale-Pfade truncieren (`>> 8` bei 24 Bit, `>> 16` bei 32 Bit), weil der Mix-Buffer bewusst ungerundet bleibt und erst am Ausgang geclamppt wird. In `decoder.h` steht die Skala-Tabelle mit dem Hinweis, dass beim Herunterskalen abgeschnitten wird; der 24-Bit-Pfad ist in `Aiff24BitDownmixSignExtended` und `AiffTwentyFourBitMixBufferUsesSixteenBitLevel` festgeschrieben.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Vier nahezu identische Methoden** (`src/audio/AudioDecoder_Aiff.cpp:104-198`)
  Die Blöcke unterscheiden sich nur in Zieltyp, `=`/`+=` und der Skalierung. Alle oben genannten Befunde (Padding, Skala, toter Zweig, NULL-Check) sind viermal kopiert. Empfehlung: eine private Kernmethode
  ```cpp
  template <class T> size_t readFrames(size_t num, T* dst, bool additive);
  ```
  mit `if constexpr`-Zweig für `STEREOSAMPLE16`/`STEREOSAMPLE32`/`STEREOSAMPLE_FLOAT`; damit gibt es genau eine Stelle für Stride, Skala und Grenzziehung.
    **Umsetzung:** Teilweise umgesetzt, der Vorschlag mit `if constexpr` bewusst nicht: die Template-Kernmethode war zeitweise drin und wurde auf Wunsch des Betreuers wieder zurückgebaut, weil sie ihm nicht lesbar genug war. Jetzt stehen die vier Sampleformate je Lesemethode als ausgeschriebener `switch` über `encoding` da (Zeilen 221, 285, 342, 400), wodurch die Skalierung direkt lesbar bleibt. Dupliziert ist dadurch nur noch die Formatumschaltung - Prüfen, Klemmen und Dateigrenze erledigt `prepareRead()` (`src/audio/AudioDecoder_Aiff.cpp:186`) gemeinsam für alle vier Methoden.
- [x] **Totes Member `samplesize`** (`include/pplib/audio/decoder.h:212`, gesetzt in `src/audio/AudioDecoder_Aiff.cpp:43`, `83`)
  Wird von keiner Methode gelesen. Entfernen (Header-Änderung, API-relevant).
    **Umsetzung:** `samplesize` ist aus `decoder.h` und dem Decoder entfernt; die Bytezahl je Kanal heißt jetzt `bytesPerChannel` und wird in `open()` gesetzt (`src/audio/AudioDecoder_Aiff.cpp:102`) und in den Kanal-Offsets aller vier Lesemethoden benutzt (INT24-Zweige ab Zeile 231, 295, 352, 410). Da das Member privat war, ändert sich an der öffentlichen Schnittstelle nichts.
- [x] **Magische Zahlen für Stride und Skalierung** (`src/audio/AudioDecoder_Aiff.cpp:113`, `121`, `137`, `145`, `159`, `167`, `183`, `191`)
  `data += 4` / `data += 6`, `32768.0f`, `8388608.0f`. Empfehlung: `constexpr size_t kBytesPerChannel16 = 2;`, `constexpr float kScale16 = 1.0f / 32768.0f;` usw. und `info.BytesPerSample` als einzige Stride-Quelle (siehe Padding-Befund).
    **Umsetzung:** Der Frame-Stride kommt ausschließlich aus `info.BytesPerSample` (Local `frameBytes` in jeder Lesemethode), der Offset des zweiten Kanals aus dem Member `bytesPerChannel` (`src/audio/AudioDecoder_Aiff.cpp:102`) über das Local `chanBytes`. Die Skalen sind benannt: `FULL_SCALE_F = 2147483648.0f` (Zeile 175) für 32-Bit-Integer zu Float und `ScaleFloatToSample16()` (Zeile 163) für Float zu 16 Bit; die Schiebebreiten `>> 8`/`>> 16` stehen sichtbar im jeweiligen `encoding`-Zweig.
- [x] **Manuelle Vorzeichenextension über Unsigned-Shift** (`src/audio/AudioDecoder_Aiff.cpp:117-118`, `141-142`, `165-166`, `189-190`)
  `PeekN24()` liefert `uint32_t` (`include/pplib/core/functions.h:307`); `(int32_t)(wert << 8) >> 8` ist zwar mit C++20 definiert (modulare Konversion, arithmetisches Shift), aber schwer zu lesen. Empfehlung: eigene Hilfsfunktion `constexpr int32_t SignExtend24(uint32_t v) { return static_cast<int32_t>(v << 8) >> 8; }` mit Kommentar — einmal dokumentiert, viermal verwendet.
    **Umsetzung:** Die Helferfunktion existiert jetzt genau so im anonymen Namespace (`src/audio/AudioDecoder_Aiff.cpp:145`) und wird in jedem INT24-Zweig der vier Lesemethoden aufgerufen (ab Zeile 231, 295, 352, 410) statt den rohen Shift auszuschreiben. Getestet über `Aiff24BitDownmixSignExtended` (negative 24-Bit-Werte bleiben negativ).
- [x] **`NULL`/`nullptr` gemischt** (`src/audio/AudioDecoder_Aiff.cpp:44`, `81` verwenden `NULL`; sonst `nullptr` in `map()`-Vergleichen)
  Für neuen Code einheitlich `nullptr`; die Member-Deklaration in `decoder.h:209` bleibt unverändert.
    **Umsetzung:** Im Decoder durchgehend `nullptr`, auch in Konstruktor und `close()` (`src/audio/AudioDecoder_Aiff.cpp:59`, `116`); alle Zeigervergleiche laufen gegen `nullptr`. Am Member `FileObject* ff` änderte sich nichts Wesentliches, es ist jetzt mit `nullptr` initialisiert.

## Dokumentation & Doxygen

- [x] **Klasse `AudioDecoder_Aiff` ohne Doxygen** (`include/pplib/audio/decoder.h:206`)
  Es fehlen: unterstützte Varianten (AIFF/AIFF-C unkomprimiert, Big Endian), Kanalbeschränkung auf Stereo, unterstützte Bitbreiten (16/24), Annahme Zweierkomplement statt Sign-Magnitude, Umgang mit Sample-Padding. Ohne diesen Block ist die Klasse per IntelliSense nicht korrekt anwendbar.
    **Umsetzung:** Klassendoku im Header neu geschrieben: Formate (unkomprimiertes AIFF, AIFF‑C mit `NONE`/`FL32`), Big Endian, Mono und Stereo, identifizierte Bitbreiten 8..32 mit dekodierten 16/24/32, Zweierkomplement, gepackte Frames, die vollständige Skalierungstabelle als Markdown sowie MCU-Hinweise zu Stack (`BUFFER`) und Map-Fenster.
- [x] **`open()`-Fehlerbedingungen undokumentiert** (`include/pplib/audio/decoder.h:217`)
  Welche `AudioInfo`-Felder welche Exception auslösen (Zeile 67-74), und dass ein vom Aufrufer übergebenes `info` ungeprüft (bis auf diese vier Bedingungen) übernommen wird.
    **Umsetzung:** `open()` dokumentiert jetzt jede Ablehnung (fremdes Format, Kanalanzahl, unsauberes Seitenverhältnis der Framegröße, Bitbreite außerhalb 16/24/32) als `DecoderInitializationException`; außerdem steht im Header, dass nur Format, Kanäle, Bitbreite und Frames/Sekunde aus dem übergebenen `info` geprüft werden, `AudioStart`/`Samples` jedoch gegen die Dateigrenze validiert werden (`AiffOverstatedSampleCountDecodesOnlyRealFrames`, `AiffHostileAudioInfoIsClampedToPayload`).
- [x] **Rückgabewert der Lesemethoden undokumentiert** (`include/pplib/audio/decoder.h:222-225`)
  Rückgabe = tatsächlich gelesene Frames; am Dateiende 0 ohne Exception; `num` wird an `info.Samples` begrenzt (Zeile 107). Genau diese Begrenzung ist durch den `OverflowException`-Befund außer Kraft gesetzt — Doku muss das Verhalten nach Fix beschreiben.
    **Umsetzung:** Doku der drei `getSamples`/`addSamples`-Methoden beschreibt jetzt die Rückgabe (tatsächlich dekoderte Frames), die Begrenzung an `info.Samples` **und** an das Dateiende sowie 0 ohne Exception am Ende. Die Verifikation der Mitgabe von `AudioInfo` fehlt weiterhin in den Deklarationen der betroffenen Decoder, ist hier aber über `getAudioInfo()`-Doku abgedeckt.
- [ ] **`seekSample()`-Clamping nicht dokumentiert** (`include/pplib/audio/decoder.h:220`)
  Die Implementierung clampet an `info.Samples` (Zeile 93-96) und wirft nie — das unterscheidet sich von MP3 (Exception) und OGG (kein Clamping, kein Fehlercheck) und gehört in die Header-Doku aller vier Klassen.
    **Umsetzung:** Für den AIFF-Decoder dokumentiert (`include/pplib/audio/decoder.h`: Clamping an die Zahl der Frames, kein Werfen, wirksam erst nach `open()`). Die Doku der Mp3‑, Ogg‑ und Wave-Klassen ist damit ebenfalls offen.
- [x] **`samplesize` ohne Kommentar** (`include/pplib/audio/decoder.h:212`)
  Weder Einheit noch Zweck erkennbar; toter Code (siehe Code-Smell-Block).
    **Umsetzung:** Gegenstandslos: das Member ist entfernt und durch das dokumentierte `bytesPerChannel` (Bytes pro Samplekanal) ersetzt.

## Befunde in Drittdateien

- [ ] **`IdentAIFF` leitet `BytesPerSample` aus der Bitbreite ab** (`src/audio/AudioInfo.cpp:93`)
  Root-Cause des Padding-Befunds; Korrektur gehört in `IdentAIFF` (Ableitung aus `AudioSize / Samples`).
    **Umsetzung:** Bewusst nicht umgesetzt, aus demselben Grund wie der Padding-Befund: die Bitbreitenformel bleibt Quelle von `BytesPerSample` (`src/audio/AudioInfo.cpp:207`, gesetzt in Zeile 221).
- [x] **`IdentAIFF` prüft `Samples` nicht gegen `AudioSize`** (`src/audio/AudioInfo.cpp:92`, `119`)
  Ermöglicht den `OverflowException`-Pfad; zusätzlich ist `info.AudioSize = size - 8` (Zeile 119) ein Unterlauf, wenn die SSND-Nutzlänge < 8 Bytes ist (Wert wird zu einem riesigen `uint32_t`).
    **Umsetzung:** Behoben: der SSND-Zweig betritt die Berechnung nur bei `size >= 8` und rechnet in `uint64_t` mit `audioSize = min(size - 8, Dateiende - Datenbeginn)`; danach wird `Samples` auf `audioSize / bytesPerSample` begrenzt (`src/audio/AudioInfo.cpp:211` bis `214`). Getestet durch `AiffSampleOffsetAndChunkBeyondEof`, `AiffTrailingHalfFrameIsTrimmed`, `AiffFormSizeBeyondEof` und decoderseitig `AiffHostileAudioSizeBeyondFileIsClampedToFileEnd`.
- [x] **`IdentAIFF` lehnt AIFF-C mit Kompressions-Offset ab** (`src/audio/AudioInfo.cpp:116-117`)
  `PeekN32(adr + 8) != 0` bzw. `!= 0` bei `adr + 12` → `return false`. Für unkomprimiertes AIFF-C sind beide Felder 0, daher funktioniert AIFF-C; für komprimierte Varianten (`MAC3`, `sowt` …) wird abgelehnt — das ist korrekt, sollte aber als bewusste Einschränkung dokumentiert sein.
    **Umsetzung:** Die Prüfung der beiden Kompressionsfelder ist durch eine explizite `compType`-Auswahl ersetzt: nur `NONE` und `FL32`/`fl32` (erzwinger 32 Bit) passieren, AIFF‑C ohne Kompressionsfeld wird abgelehnt (`src/audio/AudioInfo.cpp:191` bis `201`). Der Comment ab Zeile `191` und die Klassendoku nennen `sowt`, `ulaw`, `alaw`, `adpc` und 64‑Bit‑Float ausdrücklich als ununterstützt; Tests `AiffcCompressedIsRejected`, `AiffcUncompressedIsAccepted` und `AiffcFl32WithWrongBitdepthIsRejected`.
- [x] **80-Bit-`long double`-Konvertierung der Abtastrate** (`src/audio/AudioInfo.cpp:95-110`)
  Direkte Byte-Zuweisung in ein `long double` setzt x87-80-Bit-Layout voraus (10 Byte, Exponent 15 Bit). Auf ARM/AArch64 (`long double` = IEEE 128 Bit) oder MSVC (`long double` = `double`, 8 Byte) ist der Code falsch und liefert eine unplausible `Frequency`. Bereits in [audioinfo-review.md](./audioinfo-review.md) dokumentiert — hier nur als Querverweis, weil der AIFF-Decoder die Konsequenz (falsche `Length`/`Bitrate`) direkt weiterreicht.
    **Umsetzung:** Für AIFF erledigt: `readExtended80()` (`src/audio/AudioInfo.cpp:88`) liest Mantisse und Exponent byteweise aus und berechnet den Wert in `double` über `ldexp()`; `Frequency` wird als `uint64_t` übernommen und auf `UINT32_MAX` geprüft (Zeile `160`). Die Abtastrate ist damit auch auf ARM und MSVC korrekt; abgesichert durch `IdentAiffWithoutID3` und `AiffFractionalSampleRate`.
- [ ] **`File::map()`-Vertrag: `nullptr` nur bei 0 Bytes, sonst `OverflowException`** (`src/core/File.cpp:750-770`)
  Für Decoder, die Bereichsgrenzen aus Chunk-Metadaten berechnen, ist die Exception ein harter Bruch. Empfehlung: entweder `map()` mit `bytes == 0`-Guard plus expliziter Größenprüfung im Decoder (siehe Behebungsvorschlag) oder eine `mapChecked()`-Variante, die `nullptr` liefert.
    **Umsetzung:** Eine `mapChecked()`-Variante wurde nicht eingeführt. Stattdessen prüft `IdentAIFF()` alle Chunk-Grenzen mit `tryMap()` und der Decoder jede `map()`-Länge gegen das Dateiende sowie volle Frames (`src/audio/AudioDecoder_Aiff.cpp:201` bis `204`) — ein Bereichslauf über das Dateiende hinaus ist damit ausgeschlossen. Der `nullptr`-Fall bleibt als letztes Glied abgefangen (siehe Befund 1 zur Nullkontrolle).
- [x] **`IdentAudioFile` verwirft AIFFs mit inkonsistenter FORM-Größe** (`src/audio/AudioInfo.cpp:221`)
  `PeekN32(adr + 4) < file.size()` — schützt wirksam vor truncierten Dateien (verifiziert: `truncate` auf 1054 Bytes → `ident=0`), lehnt aber auch Dateien ab, deren FORM-Feld korrekt, deren `file.size()` aber durch nachträglich angehängte Daten größer ist. Grenzwertig, aber erwähnenswert, weil `<=` hier toleranter wäre.
    **Umsetzung:** Die Bedingung steht jetzt als `(uint64_t)PeekN32(adr + 4) + 8 <= fsize` in `DetectFormat()` (`src/audio/AudioInfo.cpp:353`), rechnet also in `uint64_t` (kein Overflow bei `FORM`-Größe nahe `UINT32_MAX`) und toleriert am Dateiende angehängte Bytes. Getestet mit `AiffWithTagsAtEndOfFile` (Annahme) und `AiffFormSizeBeyondEof` (Ablehnung einer truncierten Datei).
