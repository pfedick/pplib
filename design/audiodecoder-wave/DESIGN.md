# DESIGN – AudioDecoder_Wave Überarbeitung

Stand: 2026-10-31. Grundlage: `codereview/todo/audiodecoder-wave-review.md`. Pattern: der
bereits überarbeitete `AudioDecoder_Aiff` (`src/audio/AudioDecoder_Aiff.cpp`).

## Zweck und Nicht-Ziele

**Zweck**
- Kritische Befunde des Reviews beheben: fehlende `ff`-Prüfung in allen Lesepfaden,
  uneinheitliche EOF-Exceptions (`EndOfFileException` vs. `OverflowException`),
  vorzeichenloses Aufaddieren im 32-Bit-Mixpfad, toter `BitsPerSample == 16`-Zweig,
  ungesicherte Fremd-`AudioInfo`, Destruktor ohne `close()`.
- Formaterweiterung: Mono (Kanal wird dupliziert), 24-Bit-PCM, 32-Bit-PCM und
  32-Bit-IEEE-Float (`fmt`-Format-Tag 3) — alles Little Endian.
- Struktur wie AIFF: gemeinsame Lesevorbereitung `prepareRead()`, Kodierungsenum
  `FrameEncoding`, vier Lesemethoden mit gleichem Aufbau.

**Nicht-Ziele**
- Keine 8-Bit-Unterstützung (bewusster Beschluss). `IdentWave()` erkennt 8 Bit weiterhin als
  Container-Metadaten, `open()` lehnt das Format ab.
- Kein `WAVE_FORMAT_EXTENSIBLE` (Tag `0xFFFE`) und keine komprimierten Tags
  (`ADPCM` 2, `µ-law` 6, `a-law` 7, …) — Abweisung bereits in `IdentWave()`.
- Keine Abtastratenkonvertierung, kein Rounding beim Herunterskalieren (wie AIFF).
- Kein 20-Bit-Decoding: 20 Bit in 3 Bytes wird erkannt, vom Dekoder abgelehnt.

## Öffentliche Grenzfäche

`include/pplib/audio/decoder.h`, Klasse `AudioDecoder_Wave`:

```cpp
private:
    enum class FrameEncoding : uint8_t { NONE = 0, INT16 = 1, INT24 = 2, INT32 = 3, FLOAT32 = 4 };
    FileObject* ff;
    AudioInfo info;
    size_t position;
    size_t bytesPerChannel;   // Bytes eines Kanal-Samples im Frame
    FrameEncoding encoding;
    size_t prepareRead(size_t num, const void* buffer, uint64_t& offset) const;

public:
    AudioDecoder_Wave();
    ~AudioDecoder_Wave() override;      // ruft close()
    void open(FileObject& file, const AudioInfo* info = nullptr) override;
    void close() override;
    const AudioInfo& getAudioInfo() const override;
    void seekSample(size_t sample) override;
    size_t getPosition() const override;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
```

Entfernt: `size_t samplesize` (toter Ballast) und die nie definierte Deklaration
`readWaveHeader(FileObject&, WAVEHEADER&)`, die den Header unnötig an `WAVEHEADER` bindet.

Skalierung der Samples — bewusst identisch zu AIFF, damit Quellen unterschiedlicher Bitbreite
im `STEREOSAMPLE32`-Mischpuffer ohne Nachskalierung addierbar bleiben:

| Quelle | `getSamples(STEREOSAMPLE16)` | `addSamples(STEREOSAMPLE32)` | `getSamples`/`addSamples(STEREOSAMPLE_FLOAT)` |
| --- | --- | --- | --- |
| 16 Bit Integer | unverändert | unverändert, vorzeichenbehaftet | / 32768,0f |
| 24 Bit Integer | >> 8 | >> 8 | / 8388608,0f |
| 32 Bit Integer | >> 16 | >> 16 | / 2147483648,0f |
| 32 Bit Float | × 32768, hart begrenzt, NaN → 0 | × 32768, hart begrenzt, additiv | unverändert |

Sampleabruf über die Little-Endian-Familie `Peek16`/`Peek24`/`Peek32`/`PeekFloat`, nicht über
`PeekN*` (das wäre Big Endian wie bei AIFF).

Zugehörige Änderung in `IdentWave()` (`src/audio/AudioInfo.cpp`): Format-Tag aus `fmt`+0 lesen.
Tag 1 → `AudioInfo::SampleEncoding::INTEGER`, Tag 3 → `SampleEncoding::FLOAT` (nur bei 32 Bit
zulässig), alle anderen Tags → `return false`. Doxygen in `audioinfo.h`
(„WAVE erkennt bewusst nur PCM“) und bei `AudioFormat`/`IdentAudioFile` nachziehen.

## Ownership und Speicherverwaltung

- `ff` ist ein nicht-besitzender Zeiger auf das `FileObject` des Aufrufers; die Datei muss
  zwischen `open()` und `close()` offen bleiben.
- Der Decoder besitzt keinen Heap-Speicher, der Destruktor ruft nur `close()`.
- Gelesen wird ausschließlich über `ff->map()` innerhalb des Map-Fensters. `open()` hebt das
  Fenster auf mindestens 32 KiB; ein größerer Wert des Aufrufers bleibt erhalten.
- `close()` setzt `ff = nullptr`, `position = 0`, `bytesPerChannel = 0`,
  `encoding = FrameEncoding::NONE` und `info = AudioInfo()`.

## Fehlermodell

- `open()` wirft in allen Fällen `UnsupportedAudioFormatException` — sowohl bei nicht
  erkennbarem Container (`IdentAudioFile()` liefert false) als auch bei erkennbarem, aber
  nicht dekodierbarem Format (Format != WAVE, Kanäle < 1 oder > 2, `BytesPerSample` passt
  nicht zu `BitsPerSample`/`Channels`, Bitbreite außerhalb 16/24/32, 32-Bit-Float bei
  abweichendem `SampleType`). Beschluss: AIFFs `DecoderInitializationException` an dieser
  Stelle war eine Inkonsistenz und ist bereits korrigiert.
- Lesende Methoden (über `prepareRead()`): `DecoderException`, wenn kein Dateihandel offen
  ist oder `buffer == nullptr`.
- EOF ist kein Fehler: `prepareRead()` begrenzt an `info.Samples`, `info.AudioSize` und das
  echte Dateiende; Rückgabe 0 ohne Exception. Damit entfallen die alte
  `EndOfFileException` (sequenzieller `read()`-Pfad) und die `OverflowException` aus `map()`.
- `seekSample()` begrenzt an `info.Samples` und wirft nie.

## Thread-Safety-Regeln

Keine neuen statics und kein neuer gemeinsam genutzter Zustand. Ein Decoder-Objekt ist wie
die anderen Decoder nicht thread-safe — sequenzieller Zugriff ist Sache des Aufrufers. Die
Hilfsfunktionen der `.cpp` (`SignExtend24`, `ScaleFloatToSample16`) sind zustandslos.

## Plattform-Grenzen

- Rein byteweiser Sampleabruf (Peek-Familie) → keine Alignment- oder Endian-Annahmen des
  Hosts; läuft auf Windows/MinGW, Linux, FreeBSD und Mikrocontrollern.
- `static_assert(sizeof(float) == 4)` für den Float-Pfad.
- Kein `std::string`, keine Heap-Allokation im Lesepfad.
- Map-Fenster klein gehalten (32 KiB), damit kein Adressraum-Problem auf kleinen Systemen.

## Testorganisation

- Neue Datei `tests/src/audio/audiodecoder_wave.cpp`, Fixture `AudioDecoderWaveTest`.
- AIFF-Tests aus `tests/src/audio/audiodecoder.cpp` nach
  `tests/src/audio/audiodecoder_aiff.cpp` (Fixture `AudioDecoderAiffTest`) verlagern — sie
  waren dort nur zwischenzeitlich gelandet. `audiodecoder.cpp` enthält danach keine Tests mehr
  und entfällt. Der Glob `src/audio/*.cpp` in `tests/CMakeLists.txt` erfasst neue Dateien
  automatisch.
- Echte Fixtures: `test_44kHz.wav` (16 Bit stereo), `test_44kHz_24bit.wav`,
  `test_44kHz_32bit.wav`, `test_44kHz_float.wav` (Tag 3, `fact`-Chunk vor `data`).
  Corner-Cases (Mono, Hostile-`AudioInfo`, NaN, abgeschnittene Payload) als Inline-Container.

## Offene Punkte

- `File::map()` wirft weiterhin `OverflowException` statt `nullptr` zurückzugeben
  (Drittdatei-Befund des Reviews, bereits in `audioinfo-review.md` dokumentiert). Der
  Wave-Dekoder umgeht das durch Begrenzung vor dem `map()`-Aufruf.
