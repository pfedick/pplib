# DESIGN.md — AudioDecoder_MP3 (Überarbeitung nach Review)

Stand: Review `codereview/todo/audiodecoder-mp3-review.md` (21 Funde). Referenz für Stil und
Konventionen: die überarbeiteten Decoder `AudioDecoder_AIFF` und `AudioDecoder_Flac`.

## Zweck und Nicht-Ziele

Zweck: MP3-Decode über libmpg123 in den Mixpuffer (`MixBuffer`). Getragen werden:

- Mono und Stereo
- 16-Bit-Pfad (`STEREOSAMPLE16`, unskaliert) und Float-Pfad (`STEREOSAMPLE_FLOAT`, [-1,1])
- feste und variable Bitrate, alle von libmpg123 unterstützten Abtastraten
- Sprünge innerhalb der Datei (_seek) und korrektes Überspringen von ID3-/Coverart-Vorspann

Nicht-Ziele:

- kein ID3-Tag-Parsing (Vorspann wird nur übersprungen, spätere `ID3Tag`-Klasse)
- keine exakte Samplezahl: MP3 hat keinen Header mit Frame- oder Samplezahl. Dekodiert wird bis zum
  letzten Frame; die vom Sniffer gemeldete Zahl (Xing-Header oder Framezählung) kann abweichen
- kein Mehrkanal (mehr als 2), kein Enkodierer, kein Mikrocontroller-Support
- kein Resampling und kein Upmixing durch die Bibliothek

## Öffentliche Grenzfäche

Es bleibt bei der Basisklasse `AudioDecoder` (identisch zu Wave und FLAC); die AIFF-eigenen Extras
(planare Kanäle, `setOutSampleRate()`, `isseekable()`) sind weiterhin nicht Teil dieser Klasse:

```cpp
AudioDecoder_MP3();
~AudioDecoder_MP3() override;
void open(pplib::FileObject &file, const AudioInfo *info = nullptr);
void close() override;
const AudioInfo &getAudioInfo() const override;
void seekSample(size_t sample) override;
size_t getPosition() const override;
size_t getSamples(size_t num, STEREOSAMPLE16 *buffer) override;
size_t addSamples(size_t num, STEREOSAMPLE32 *buffer) override;
size_t getSamples(size_t num, STEREOSAMPLE_FLOAT *buffer) override;
size_t addSamples(size_t num, STEREOSAMPLE_FLOAT *buffer) override;
```

private: `bool fillFrames()`, `void syncFormat()`, `void switchEncoding(size_t resume)`,
`size_t transferFrames(size_t num, void *dest)`. `transferFrames()` führt eine Typlose (`void *`)
wie bei Wave/FLAC, weil ein Puffer je nach Ziel-Encoding `STEREOSAMPLE16`- oder
`STEREOSAMPLE_FLOAT`-Elemente enthält.

Semantik-Klärungen (Kern der Überarbeitung):

1. Das Ausgabeformat von libmpg123 wird in `open()` aus `info` abgeleitet (Mono/Stereo-Maske, reale
   Abtastrate). Eine 22,05-kHz-Mono-Datei wird als 22,05-kHz-Mono geliefert, nicht als hochresamptes
   Stereo.
2. libmpg123 dekodiert nativ in das Ziel-Encoding, das in `open()` festgelegt wird
   (`MPG123_ENC_SIGNED_16` oder `MPG123_ENC_FLOAT_32`); der eine Ausgangspuffer enthaelt immer genau
   eine dieser Varianten. Nachweislich laesst sich das Encoding eines *geoeffneten* Handles nicht
   umstellen: `mpg123_format_none()` + `mpg123_format()` liefern zwar `MPG123_OK`, `mpg123_getformat()`
   meldet aber weiter die beim Oeffnen ausgehandelte Variante. Gewechselt wird deshalb nur an der
   Puffergrenze und nur ueber `switchEncoding()`: Handle schliessen, neue Format-Tabelle, Handle neu
   oeffnen, mit `mpg123_seek()` auf die Fortsetzungsposition zurueckspulen. Reste im Puffer werden
   vorher in das neue Ziel skaliert, dadurch gehen keine Samples verloren. Die Fortschrittsposition
   fuehrt `transferFrames()` pro Chunk; `mpg123_tellframe()` zaehlt MPEG-Rahmen, nicht Samples, und ist
   dafuer unbrauchbar.
3. libmpg123 liest über `mpg123_replace_reader_handle()` direkt aus der `FileObject`. Die Callbacks
   präsentieren einen virtuellen Stream, dessen Position 0 = `info.AudioStart` und dessen Länge
   `info.AudioEnd - info.AudioStart` ist; ID3v1-Anhängsel und Padding hinter `AudioEnd` werden nicht
   dekodiert. Feed-Modell und eigener Eingangsring sind damit entfallen.
4. Die Samplezahl wird am Ende nicht auf `info.Samples` begrenzt (letzte Frames werden bewusst
   dekodiert).

## Ownership und Speicherverwaltung

* libmpg123-Handle, Eingangs- und Ausgangspuffer sowie der Formatstatus ziehen in eine
  `struct Mp3State` (PImpl) wie beim Flac-Decoder. Der Header behält `Mp3State *mp3` und `info`;
  damit entfallen `void *decoder`, die C-Casts und die toten Mitglieder `isRunning`, `needInput`,
  `samplesize`.
* Puffer: nur noch der Ausgangspuffer `Mp3Data::outbuffer` (`constexpr OUTBUFF`, 32 KiB, mit
  `PICO_BUILD` 4 KiB). `READBUFFER` entfällt, die Eingangsseite puffert libmpg123 selbst.
* `close()` ist idempotent: Zeiger werden auf `nullptr` gesetzt, `mpg123_exit()` nur bei letzter
  Instanz, `info` wird zurückgesetzt.
* Nur ein Owner pro Handle; kein Dateizugriff nach `close()`.
* interner Code nutzt `pplib::String` und `std::string_view`, kein `std::string`

## Fehlermodell

* libmpg123 nicht verfügbar: `DecoderException` im Konstruktor (Referenz: Flac-Decoder).
* `open()`: fremdes Format, `info.Channels == 0`, `info.Channels > 2`, `info.AudioStart` hinter
  `info.AudioEnd`, nicht unterstützte Abtastrate: `UnsupportedAudioFormatException`.
* `getSamples()`/`addSamples()` vor `open()` oder nach `close()`: `DecoderException`.
* Kein Fortschritt trotz offenem Stream (libmpg123 meldet 0 Bytes ohne Ende): `DecoderException`.
* `seekSample(sample)`: rahmenkanten-genau über `mpg123_seek()`; ein Sprung unter die erste bzw. über
  die letzte Hörbare liefert die zugehörige Grenze. Kein Wurf, außer bei geschlossener Datei.
* Feed- oder Decode-Fehler von libmpg123: `DecoderException`, danach `close()`; `open()` ist erneut möglich.
* Dateiende: kein Wurf, `getSamples()` liefert 0.
* Ohne `HAVE_MPG123`: alle Methoden werfen `UnsupportedFeatureException` (ein Block, nicht pro Methode).

## Thread-Safety-Regeln für diese Komponente

* `mpg123_init()` und `mpg123_exit()` laufen unter dem statischen Initialisierungsmutex mit
  Referenzzähler.
* Eine `AudioDecoder_MP3`-Instanz ist nicht thread safe: Zugriff sequentiell durch den Aufrufer.
* Kein geteilter Zustand außerhalb des Objekts; alle Puffer hängen an der jeweiligen Instanz.

## Plattform-Grenzen

* Linux, FreeBSD, Windows/MinGW mit libmpg123: voller Funktionsumfang.
* Ohne libmpg123 (`HAVE_MPG123` undefiniert): Kompilieren möglich, alle Methoden werfen
  `UnsupportedFeatureException`.
* Mikrocontroller (`PICO_BUILD`): keine Decode-Unterstützung, nur Header kompiliert, kleine
  Pufferkonstanten.
* Keine Abhängigkeit von `wchar_t`- oder `long`-Größen (nur `size_t`, `int16_t`, `int32_t`).

## Offene Punkte

1. Seeking: umgesetzt als echtes Spulen über `mpg123_seek()` auf der Handle-I/O-Datei
   (Review-Funde 13 und 14). Rahmenkanten-genau, wie vom Sniffer gemeldete Framezahlen.
2. `info.Samples` ist keine harte Obergrenze (User-Entscheidung: bis zum letzten Frame dekodieren);
   `getPosition()` kann daher über die vom Sniffer gemeldete Samplezahl laufen.
   Eine Rahmenzahlungleichheit zwischen Sniffer und libmpg123 ist normal (Sicht auf Teilrahmen).
3. Mischformate in einer Datei ("Frankenstein"-Stream, zusaemmengesetzte MP3s mit unterschiedlichen
   Abtastraten): User-Entscheidung, daran aendert sich nichts. `mpg123_new()` wird ohne
   `MPG123_NEW_FORMAT` aufgerufen, deshalb meldet libmpg123 einen Formatwechsel nicht; der Strom
   laeuft weiter, `syncFormat()` laeuft ins Leere (die Tabelle ist festgelegt, der Koerper liegt
   nie an) und `info` beschreibt abschnittsweise nicht mehr das kodierte Format. Die Ausgaben von
   libmpg123 auf stderr sind ueber das Flag `MPG123_QUIET` (als `MPG123_ADD_FLAGS`-Bit in `open()`)
   unterdrueckt. Ein Wechsel des Ziel-Encodings laeuft unabhaengig davon ausschliesslich ueber
   `switchEncoding()`: Handle schliessen, neue Format-Tabelle, erneut oeffnen, auf die
   Fortsetzungsposition zurueckspulen.
4. FLAC-Nachschau: Der User wollte die Flac-Implementierung nochmal anschauen. Skaliertechnisch ist
   sie konventionskonform (16 Bit unskaliert, 24 Bit >>8, float durch 32768 bzw. 8388608). Offen
   ist, was genau gemeint ist.
