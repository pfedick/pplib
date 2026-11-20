# DESIGN: `AudioEncoder_Ogg`

Stand: 2026-11-20. Entscheidungen 1–3 sind abgestimmt; bereit für Phase 2 (Gerüst).

## Zweck und Nicht-Ziele

**Zweck**

* Neue Encoder-Klasse `AudioEncoder_Ogg` in `include/pplib/audio/encoder.h` und `src/audio/AudioEncoder_Ogg.cpp`, stilistisch angelehnt an `AudioEncoder_Flac`, `AudioEncoder_Aiff` und `AudioEncoder_MP3`.
* Kodierung von Audio-Daten aus beliebigen `AudioDecoder`-Quellen in das Ogg-Vorbis-Format über `libvorbisenc` und `libogg`.
* Konfiguration der Kompressionsparameter:
  * **VBR (Qualitäts-Modus, Default):** Qualität `-0.1f` bis `1.0f` (`vorbis_encode_init_vbr`, Standard `0.4f` ≈ 128 kbps).
  * **ABR (Average Bitrate, optional mit Min/Max):** nominale Bitrate in kbps (`vorbis_encode_init`).
  * **CBR (Constant Bitrate):** feste Bitrate (`min = nominal = max` in `vorbis_encode_init`).
* Unterstützung von Vorbis-Comments und Cover-Bildern über `setVorbisComments(const ID3Tag&)` (inkl. `METADATA_BLOCK_PICTURE` als Base64-kodierter FLAC-Picture-Block).
* Fortschritts-Callback (`setProgressFunction`) und Thread-sicherer Abbruch (`stopEncoder()`).
* Verlustfreie Nutzung von `STEREOSAMPLE_FLOAT` aus `AudioDecoder::getSamples()`, da `libvorbis` intern nativ auf `float`-Samples (`-1.0f .. +1.0f`) arbeitet.

**Nicht-Ziele**

* **Low-Level-Buffer-API (`encodeBuffer` / `flushBuffer`):** analog zu `AudioEncoder_Flac` und `AudioEncoder_Aiff` wird nur die `AudioDecoder`-basierte API (`startEncode`, `encode`, `finish`) implementiert.
* **Andere Ogg-Codecs (Opus, Ogg-FLAC, Speex):** nicht Teil dieser Komponente.
* **Ogg-Encoding ohne `libvorbisenc` / `libogg`:** Vorbis-Psychoakustik-Encoding erfordert `libvorbisenc` + `libogg` (analog zu `libFLAC` bei `AudioEncoder_Flac` und `LAME` bei `AudioEncoder_MP3`).
* **Verkettete Ogg-Streams (Chained Streams):** mehrere `encode()`-Aufrufe hintereinander fügen Audio nahtlos in denselben logischen Vorbis-Bitstream an (bei identischer Sample-Rate und Kanalzahl).

---

## Öffentliche Schnittstelle

### `CMakeLists.txt`
* Erweiterung der Ogg/Vorbis-Erkennung:
  ```cmake
  pkg_check_modules(VORBIS QUIET vorbis vorbisfile vorbisenc ogg)
  ```
  *(Damit stehen `vorbisenc` und `ogg` automatisch für `pplib_audio` zur Verfügung, wenn `PPLIB_WITH_VORBIS=ON`.)*

### `include/pplib/audio/encoder.h`
```cpp
class AudioEncoder_Ogg
{
private:
    enum class EncoderMode
    {
        VBR,
        ABR,
        CBR
    };
    struct OggState;
    OggState* state = nullptr;
    FileObject* out = nullptr;
    ByteArray readcache;
    EncoderMode encoder_mode = EncoderMode::VBR;
    float quality = 0.4f;
    int min_bitrate = -1;
    int avg_bitrate = 192;
    int max_bitrate = -1;
    uint32_t frequency = 0;
    uint8_t channels = 0;
    bool have_firstaudio = false;
    bool started = false;
    std::atomic<bool> bStopEncode{false};
    void (*progressFunc)(int progress, void* priv) = nullptr;
    void* progressPriv = nullptr;

    void initEncoder(uint32_t freq, uint8_t ch);
    void drainPages(bool flush);

public:
    AudioEncoder_Ogg();
    AudioEncoder_Ogg(const AudioEncoder_Ogg&) = delete;
    AudioEncoder_Ogg(AudioEncoder_Ogg&&) = delete;
    AudioEncoder_Ogg& operator=(const AudioEncoder_Ogg&) = delete;
    AudioEncoder_Ogg& operator=(AudioEncoder_Ogg&&) = delete;
    ~AudioEncoder_Ogg();

    void setVBR(float quality = 0.4f);
    void setABR(int nominalKbps = 192, int minKbps = -1, int maxKbps = -1);
    void setCBR(int kbps = 192);
    void setProgressFunction(void (*ProgressFunc)(int progress, void* priv), void* priv);

    void startEncode(FileObject& output);
    void setVorbisComments(const ID3Tag& tag);
    void encode(AudioDecoder& decoder);
    void finish();
    void stopEncoder();
};
```

---

## Ownership und Speicherverwaltung

* **PImpl (`OggState`):** Kapselt alle `libvorbis`/`libogg`-C-Strukturen (`vorbis_info`, `vorbis_comment`, `vorbis_dsp_state`, `vorbis_block`, `ogg_stream_state`), sodass `<vorbis/vorbisenc.h>` und `<ogg/ogg.h>` nicht im öffentlichen Header `encoder.h` exponiert werden.
* **RAII-Aufräumen:** `OggState` verfolgt über eigene Flags (`infoInit`, `commentInit`, `dspInit`, `blockInit`, `streamInit`), welche C-Strukturen bereits initialisiert sind, und gibt sie im Destruktor (`~AudioEncoder_Ogg`) sowie bei Fehler/Neu-Start sauber frei.
* **`FileObject* out`:** Non-owning Zeiger auf das vom Aufrufer übergebene `FileObject` (muss bis nach `finish()` gültig und geöffnet bleiben, identisch zu `AudioEncoder_Flac`/`AudioEncoder_Aiff`).
* **Sample-Puffer:** `ByteArray readcache` für `STEREOSAMPLE_FLOAT`-Blöcke (RAII, keine manuellen `new`/`delete`-Arrays).

---

## Fehlermodell

* **Fehlende Library (`!HAVE_LIBVORBIS`):** `startEncode()` wirft `UnsupportedFeatureException("AudioEncoder_Ogg")` (analog zu `AudioEncoder_Flac` / `AudioDecoder_Ogg`).
* **Ungültige Parameter:**
  * `setVBR(quality)`: `quality < -0.1f || quality > 1.0f` → `IllegalArgumentException`.
  * `setABR(...)` / `setCBR(...)`: `nominalKbps <= 0` (oder inkonsistente `min > max`, `nominal < min`, `nominal > max` bei gesetztem `min`/`max`) → `IllegalArgumentException`.
  * Konfigurationsänderungen (`setVBR`, `setABR`, `setCBR`) nach `startEncode()` bzw. nach Beginn der Kodierung: erlaubt vor `startEncode()`; während laufender Kodierung (`started == true`) → `EncoderAlreadyStartedException`.
* **Zustandsfehler:**
  * `startEncode()` bei bereits laufendem Encoder (`started == true`) → `EncoderAlreadyStartedException`.
  * `startEncode()` auf nicht geöffnetes `FileObject` → `FileNotOpenException` (oder `EncoderInitializationException` falls Stream nicht schreibbar).
  * `setVorbisComments()` vor `startEncode()` → `EncoderNotStartedException`; nach dem ersten `encode()` → `EncoderAlreadyStartedException`.
  * `encode()` / `finish()` vor `startEncode()` → `EncoderNotStartedException`.
  * Zweites `encode()` mit abweichender `Frequency` oder `Channels` → `EncoderAudioFormatMismatchException`.
  * Ungültige Kanalzahl (`channels < 1 || channels > 2`) oder `frequency == 0` → `EncoderInitializationException`.
* **`libvorbisenc`-Initialisierungsfehler:** Rückgabewert `!= 0` bei `vorbis_encode_init*` → `EncoderInitializationException`.
* **I/O-Fehler beim Schreiben:** `out->write(...) != expected` → `EncoderException` (bzw. durchgereichte File-Exception).
* **Abbruch durch `stopEncoder()`:** `bStopEncode` gesetzt → `EncoderAbortedException`.

---

## Thread-Safety

* Kein globaler oder statischer veränderlicher Zustand.
* `bStopEncode` ist `std::atomic<bool>`, sodass `stopEncoder()` sicher aus einem zweiten Thread während `encode()` aufgerufen werden kann (identisch zu `AudioEncoder_Flac`, `AudioEncoder_MP3`, `AudioEncoder_Aiff`).
* Paralleles Encodieren in getrennten `AudioEncoder_Ogg`-Instanzen auf getrennten `FileObject`s ist vollständig thread-sicher.

---

## Plattform-Grenzen

* Benötigt `libvorbisenc` und `libogg` (verfügbar auf Windows MSYS2/UCRT64, Linux, FreeBSD).
* Ohne `HAVE_LIBVORBIS` kompiliert die Klasse sauber als Stub und wirft `UnsupportedFeatureException` bei `startEncode()`.
* Mikrocontroller (`PICO_BUILD`): `AudioEncoder_Ogg` wird dort nicht eingesetzt.

---

## Entscheidungen (abgestimmt)

1. **Serial-Number des Ogg-Streams:** Zufällige 32-Bit-Serial-Number über `std::random_device`, wie im Ogg-Standard für eindeutige Bitstreams empfohlen.
2. **Qualitäts-Skala bei `setVBR`:** `float quality` von `-0.1f` bis `1.0f` (Standard `0.4f`), direkt wie `libvorbisenc`, im Header entsprechend dokumentiert.
3. **Wiederverwendung von `buildFlacPictureBlock`:** Als gemeinsame interne Hilfsfunktion in `OggVorbisHelper.h` / `.cpp` (bzw. `BuildFlacPictureBlock(const ID3Tag::FlacPicture&, ByteArray&)`), genutzt von `ID3Tag::saveOgg()` und `AudioEncoder_Ogg::setVorbisComments()`.
