# DESIGN: AudioDecoder_Flac + FLAC-Formaterkennung

Stand: 2026-10-31. Entscheidungen 1–7 sind mit dem Auftraggeber gesetzt.

## Zweck und Nicht-Ziele

**Zweck**

* FLAC-Dateien dekodieren: 16 und 24 Bit, 1 oder 2 Kanäle, beliebige Abtastrate.
* Formaterkennung `AudioInfo::AudioFormat::FLAC` inklusive `STREAMINFO`-Auswertung, damit
  `IdentAudioFile()`, die AudioInfo-Tests und `open(file)` ohne mitgegebene Info funktionieren.
* Gleiche Schnittstelle und gleiche Konfektions-Matrix wie `AudioDecoder_Wave`/`_Aiff`.

**Nicht-Ziele**

* **Mikrocontroller:** libFLAC allokiert im Decoder-Objekt und braucht Puffer für Frames mit
  bis zu 4608 Samples je Kanal. Es gibt dort kein libFLAC → `open()` wirft
  `UnsupportedFeatureException`. Keine MCU-optimierten Pfade.
* **Enkodierung.** Der alte `AudioEncoder` ist noch nicht migriert; FLAC-Encoding
  (`FLAC__stream_encoder_*`) wird eine eigene spätere Aufgabe.
* **8 Bit und mehr als 24 Bit.** libFLAC 1.5 liefert 32-Bit-Flac ausschließlich über den
  Wide-Sample-Pfad (`FLAC__stream_decoder_set_decode_wide_samples_only` plus uint64-Puffer).
* **Mehrkanal (mehr als 2 Kanäle), FLAC im Ogg-Container (bleibt UNKNOWN),
  Vorbis-Comments, CoverArt.**
* ID3Tag-Klasse für FLAC-Tags ist nachgelagert; vorgemerkt werden dafür jetzt nur
  `HaveID3v2Tag` und `ID3v2TagStart`.
* Byte-zu-Sample-Zuordnung: FLAC-Frames sind variabel komprimiert. `AudioStart`/`AudioSize`
  beschreiben den komprimierten Bereich; sample-basierte Positionen kommen ausschließlich aus
  `Samples` (STREAMINFO) und der libFLAC-eigenen Position. Für FLAC gilt die in der
  AudioInfo-Doku genannte Gleichung `Samples * BytesPerSample == AudioSize` bewusst nicht.

## Öffentliche Grenzfälle

### `include/pplib/audio/audioinfo.h`

* `enum class AudioFormat`: neuer Wert `FLAC = 5` (hinten angehängt, bestehende Werte unverändert).
* Dokumentationsblock zur akzeptierten Teilmenge um eine FLAC-Zeile ergänzen: `fLaC`,
  `STREAMINFO` als erster Block, 1–2 Kanäle, 8..32 Bit, ID3v1-/ID3v2-Ränder erlaubt; kein
  FLAC im Ogg-Container.
* `SampleEncoding::INTEGER` für FLAC (Float-FLAC gibt es nicht).

### Erkennung: `pplib::IdentFlac(FileObject&, AudioInfo&)` (intern in `src/audio/AudioInfo.cpp`)

* `DetectFormat()`: `fLaC` an Offset 0, **oder** ID3v2-Präfix (`ID3`, Major >= 2,
  Synchsafe-Size) mit anschließendem `fLaC` an der Tag-Grenze. Der FLAC-Zweig liegt **vor**
  der MPEG-Suche, sonst findet die MPEG-Suche in FLAC-Nutzdaten vorgetäuschte Sync-Wörter.
* Block-Walk wie bei WAV/AIFF: Blockkopf = 1 Bit last + 7 Bit Typ + 3 Byte Länge.
  `STREAMINFO` muss Block 0 mit Länge 34 sein, sonst `false`.
* Aus `STREAMINFO` (alle Werte bitverschachtelt, MSB-zuerst): min/max Blockgröße,
  min/max Framegröße, Sample-Rate (20 Bit), Kanäle (3 Bit), Bits per Sample (5 Bit),
  Gesamtzahl Samples (36 Bit), MD5 (128 Bit).
* `AudioStart` = erster Byte nach dem letzten Metadata-Block (Block mit gesetztem last-Bit).
  `AudioEnd`/`AudioSize` = bis Dateiende, wobei ein ID3v1-Tag (128 Bytes, `TAG` am EOF)
  abgezogen wird.
* ID3v2 am Anfang: `HaveID3v2Tag = true`, `ID3v2TagStart` = Anfang des Tags.
* `Samples`: Wert aus STREAMINFO, aber an eine plausible obere Grenze aus `AudioSize`
  geklemmt (Frame <= 4608 Samples, Frame >= 1 Byte -> `Samples <= 4608 * AudioSize`), sowie
  nach unten gegen 0 geprüft. Steht im Feld 0 (unbekannte Gesamtsamplezahl), aus `AudioSize`
  geschätzt und `IsVBR = true` gesetzt.
  *Umsetzung:* bewusst nicht umgesetzt — bei dekokierter Gesamtsamplezahl 0 bleiben
  `Samples`, `Length` und `Bitrate` 0 (`if (samples > 0)` in `IdentFLAC`). Eine Schätzung
  ware eine erfundene Obergrenze fur `seekSample()`; der Decoder endet stattdessen am
  physischen Streamende, das libFLAC uber seinen Decoderzustand meldet.
* `Length = Samples * 1000 / Frequency` mit 64-Bit-Zwischenrechnung (4608-Sample-Frames bei
  8 kHz laufen sonst in uint32 über).
* `Bitrate` = mittlere Rate aus `AudioSize` und `Length` in kbit/s, wie bei MP3.
* `BytesPerSample` = je Kanal auf volle Bytes aufgerundet mal Kanäle, damit die
  AudioInfo-Felddoku („Bytes pro Sample-Frame“) weiterhin trägt.
* `Mode` = MONO/STEREO, analog zu WAV/AIFF.
* Abweisung nur strukturell: `fLaC` fehlt, `STREAMINFO` nicht Block 0 oder falsche Länge,
  Sample-Rate 0, Block ragt über das Dateiende, keine nutzbaren Samples.
  *Umsetzung:* Kanäle und Bitbreite werden **nicht** mehr abgewiesen. `IdentFLAC` meldet die
  STREAMINFO-Fakten auch für 8-Bit-, 32-Bit- und Mehrkanal-Flac; entschieden wird die
  Unterstuetzung allein in `AudioDecoder_Flac::open()`, das dann
  `UnsupportedAudioFormatException` wirft. `AudioInfoTest.FlacReportsUnsupportedParametersAsFLAC`
  und `AudioDecoderFlacTest.OpenRejectsUnsupportedFlacAndForeignFiles` prüfen das.

### Decoderklasse `AudioDecoder_Flac` (`include/pplib/audio/decoder.h`, nach `AudioDecoder_Ogg`)

```cpp
class AudioDecoder_Flac : public AudioDecoder
{
public:
    AudioDecoder_Flac();
    ~AudioDecoder_Flac() override;
    void open(FileObject& file, const AudioInfo* info = nullptr) override;
    void close() override;
    const AudioInfo& getAudioInfo() const override;
    void seekSample(size_t sample) override;
    size_t getPosition() const override;
    size_t getSamples(size_t num, STEREOSAMPLE16* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE32* buffer) override;
    size_t getSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
    size_t addSamples(size_t num, STEREOSAMPLE_FLOAT* buffer) override;
};
```

* Defaultargument `nullptr` (neuer Stil wie Wave), nicht `NULL` wie bei MP3/Ogg.
* Klassen-Doku enthält Matrix und Skalierungstabelle:

| Quelle | Ziel `STEREOSAMPLE16` | Ziel `STEREOSAMPLE32` | Ziel `STEREOSAMPLE_FLOAT` |
|---|---|---|---|
| 16 Bit | `v` | `v` (16-Bit-Pegel) | `v / 32768.0f` |
| 24 Bit | `v >> 8` | `v >> 8` (bewusst 16-Bit-Pegel, Entscheidung zu int32-Ziel) | `v / 8388608.0f` |

* Mono wird auf beide Ausgabekanäle repliziert. Trunkierung ohne Rundung; identisch zu
  WAV/AIFF, damit mehrere Quellen ohne Zusatzskalierung gemischt werden koennen.
* libFLAC liefert pro Kanal separate, LSB-rechtsbuendige int32-Ebenen -> Interleave und
  Skalierung in einem Zug.
* Konstanten `FULL_SCALE_16` und `FULL_SCALE_24` wie im Wave-Decoder.

## Ownership und Speicherverwaltung

* `FLAC__StreamDecoder*` gehoert dem Objekt: erzeugt in `open()`, beendet und geloescht in
  `close()` (`FLAC__stream_decoder_finish()` + `FLAC__stream_decoder_delete()`); der
  Zerstörer ruft `close()`. RAII-Hueller im cpp wie `OggVorbisHandle`, damit Throw-Pfade
  nichts leaks.
* Ausgabe-Puffer fuer Frame-Anteile, die ueber die angeforderte Zahl hinauslaufen, als
  Member (`outbuffer`/`out_offset`/`out_size`), Bewirtschaftung wie bei `AudioDecoder_MP3`.
  Groesse aus `max block size` aus STREAMINFO (hochstens 65535 Samples je Kanal), hart
  gedeckelt, damit eine beschaedigte Kopfzeile nicht riesige Allokationen ausloest.
* `FileObject`-Zugriff ausschliesslich ueber read/seek/tell/eof-Callbacks an libFLAC.
  Diese duerfen **keine Exception ueber die C-Frames der Bibliothek werfen**: Dateiende und
  Lesefehler werden als Abbruch/0 Byte gemeldet und am Objekt vorgemerkt (Muster aus
  `AudioDecoder_Ogg`).
* Keine mutablen Globalen; libFLAC-Instanzen sind unabhaengig, daherkein Mutex noetig
  (anders als mpg123 mit Globalzustand). Ein Decoder-Objekt ist wie alle pplib-Decoder
  nicht parallel benutzbar.
* `close()` setzt `info = AudioInfo()` und `position = 0` (wie Wave/AIFF); na
  ch `close()` liefert `getAudioInfo()` eine leere Info.

## Fehlermodell

* Bibliothek fehlt (`HAVE_LIBFLAC` nicht definiert): jede öffentliche Methode wirft
  `pplib::UnsupportedFeatureException("AudioDecoder_Flac: libFLAC")` – exakt das Muster von
  `AudioDecoder_MP3`/`_Ogg`.
* Kein FLAC-Format (`IdentAudioFile()` schlägt fehl oder `info.Format != FLAC`):
  `UnsupportedAudioFormatException()` in `open()`.
* Mehr als 2 Kanäle: `UnsupportedAudioFormatException("Channels not 1 or 2")`.
* Bitbreite nicht 16/24: `UnsupportedAudioFormatException("BitsPerSample is not 16 or 24")`.
* `DecoderException("No file opened")` bzw. `("Buffer is null")` wie in Wave/AIFF.
* Decode-Fehler der Bibliothek (Error-Callback, ABORTED, unbrauchbarer Stream): die
  betroffene Lesemethode wirft `DecoderException`; kein `EndOfFileException`.
  *Umsetzung:* Der Fehler bleibt im Zustand stehen, bis `seekSample()` oder `close()` ihn
  zuruecksetzt; schon gelieferte Samples bleiben erhalten, weitere Lesungen werfen erneut
  (`AudioDecoderFlacTest.CorruptAudioMaterialStopsWithoutHanging`). Meldet libFLAC den Fehler
  erst zusammen mit dem Ende des Samples-Datensatzes, liegt eine abgeschnittene Datei vor:
  dann liefern die Lesemethoden 0 statt zu werfen. Loss of Sync nach dem letzten Rahmen
  (abhaengige Anhaenge wie ID3v1) gilt ohnehin nicht als Fehler.
* Dateiende: Lesemethoden geben 0 zurück, werfen nicht; `seekSample()` klemmt an `Samples`
  und wirft nie, auch nicht nach `close()`.
* Nach erkanntem Stream-Ende liefern weitere Lesungen 0, ohne Neuinitialisierung.

## Plattform-Grenzen

* libFLAC über `pkg_check_modules(FLAC QUIET flac)`; Linux/FreeBSD `libflac-dev`,
  MSys2 `mingw-w64-x86_64-flac`. Header `<FLAC/stream_decoder.h>`, Linkflag `-lFLAC`
  (verifiziert an libFLAC 1.5.0). Vorbereitete CMake-Flags: `option(PPLIB_WITH_FLAC … ON)`,
  `HAVE_LIBFLAC=1` als Compile-Definition, pkg-config-Requirement `flac`;
  `include/config_pplib.h.in` hat die `#cmakedefine`-Zeile bereits.
* `MD5`-Signaturprüfung der Bibliothek wird abgeschaltet (`set_md5_checking(false)`), da die
  Dekodierung bei Random-Zugriff abgebrochen und neu angesetzt wird und die Signatur sonst
  als Fehler erschiene.
* Keine Assembler-, keine 32-/64-Bit- und keine `wchar_t`-/`long`-Abhängigkeit.
  `bits_per_sample == 0` bei ffprobe ist egal: maßgeblich ist das 5-Bit-Feld in STREAMINFO.
* Windows/MSVC ohne pkg-config: FLAC bleibt deaktiviert (`PPLIB_WITH_FLAC=OFF`), identisch
  zum Verhalten von libvorbis/mpg123 heute.

## Teststrategie ( functional, keine Coverage-Jagd )

* Fixtures (ergänzt, abgeleitet von den vorhandenen 4-Sekunden-Dateien):
  `test_44kHz_16bit.flac`, `test_44kHz_24bit.flac` (vorhanden), dazu mono, ID3v2-präfixiert,
  ID3v1 am EOF, mit SEEKTABLE, abgeschnitten, 8 Bit und 5.1 für die Abweisungen.
* Eine Testdatei `tests/src/audio/audiodecoder_flac.cpp`, Fixture `AudioDecoderFlacTest`,
  Zielgröße 10–16 `TEST`s: Happy Paths (16/24 Bit je Zieltyp), Mono-Replikation,
  Seek/Round-Trip, Quervergleich 16 gegen 24 Bit, Abweisungen (8 Bit, mehrkanal,
  ID3-randige Erkennung), Ende/0-Lesen, Zustand nach `close()`, `addSamples`-Mischung.
* Erkennungstests in `tests/src/audio/audioinfo.cpp`: inline gebaute `fLaC`-Header
  (STREAMINFO-Grenzfälle), ID3-Präfix, ID3v1-EOF, beschnittene `Samples`.
* Decode-Tests überspringen sich mit `GTEST_SKIP()`, wenn `HAVE_LIBFLAC` nicht definiert ist;
  die reinen Parse-Tests laufen immer.

## Offene Punkte

* Die vorhandenen Fixtures tragen kein SEEKTABLE; libFLAC sucht dann ohne seek table von
  Streamanfang (korrekt, aber langsamer). Ein SEEKTABLE-Fixture wird erzeugt, damit beide
  Faelle abgedeckt sind. Auf einen eigenen Seektable-Parser wird verzichtet; das uebernimmt
  `FLAC__stream_decoder_seek_absolute()` ueber die Callbacks der Bibliothek. Vermerk in der
  Doku von `seekSample()`.
