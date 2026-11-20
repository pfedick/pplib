# DESIGN: Ogg-Vorbis-Erkennung in `AudioInfo` & Tag-Unterstützung in `ID3Tag` (ohne `libvorbis`)

Stand: 2026-11-20. Entscheidungen 1–3 sind abgestimmt; bereit für Phase 2 (Gerüst).

## Zweck und Nicht-Ziele

**Zweck**

1. **`AudioInfo::HaveVorbisComment` setzen (Option B):**
   * In `IdentAudioFile()` (`src/audio/AudioInfo.cpp`) wird `info.HaveVorbisComment = true` genau dann gesetzt, wenn tatsächlich mindestens ein Vorbis-Comment-Eintrag vorhanden ist (`user_comment_list_length > 0`), nicht schon bei einem leeren Block/Header nur mit Vendor-String.
   * Gilt einheitlich für `AudioFormat::FLAC` (Metadata-Block Typ `4`) und `AudioFormat::OGG` (Vorbis-Comment-Header-Paket `0x03vorbis`).
   * `AudioInfo::print()` gibt `HaveVorbisComment` zusätzlich aus.

2. **`IdentAudioFile()` für Ogg Vorbis ohne `libvorbis`:**
   * Ablösung von `IdentOggVorbisAudioFile()` in `src/audio/AudioDecoder_Ogg.cpp`: die Erkennung und Metadaten-Extraktion erfolgt direkt über `FileObject` (Ogg-Pages + Vorbis-ID-Header `0x01vorbis` + Comment-Header `0x03vorbis` + Ende der Header-Pages + letzte `granule_position` der ersten logischen Kette).
   * Damit funktioniert `IdentAudioFile()` für **alle** unterstützten Formate (WAVE, AIFF, MP3, FLAC, OGG) ohne Drittanbieter-Bibliothek – auch wenn `HAVE_LIBVORBIS` nicht gesetzt ist.

3. **Volle Ogg-Vorbis-Unterstützung in `ID3Tag` (ohne `libvorbis`):**
   * Lesen (`loadOgg`) und Schreiben (`saveOgg`) von Vorbis Comments in Ogg-Vorbis-Dateien über `ID3Tag`.
   * Wiederverwendung des bestehenden FLAC-Vorbis-Comment-Mappings (`ID3Frame` ↔ `KEY=VALUE`) sowie der Bild-Konvertierung (`APIC` ↔ FLAC-`PICTURE`-Struktur), für Ogg Base64-kodiert als `METADATA_BLOCK_PICTURE`-Tag (Vorbis-Comment-Standard).
   * Verlustfreier Erhalt des Vorbis-ID-Headers (`0x01vorbis`) und des Vorbis-Setup-Headers (`0x05vorbis`) sowie aller Audio-Pages beim Speichern.

**Nicht-Ziele**

* **Andere Codecs im Ogg-Container (Opus, Ogg-FLAC, Speex, Theora):** bleiben `AudioFormat::UNKNOWN` / `UnsupportedAudioFormatException`.
* **Verkettete Ogg-Streams (Chained Streams) beim Tag-Schreiben:** Unterstützt werden Single-Stream-Ogg-Vorbis-Dateien bzw. der erste logische Bitstream; mehrkettige Streams beim Schreiben werden abgewiesen oder nur auf Basis des ersten Bitstreams behandelt (siehe *Offene Punkte*).
* **Ogg-Dekodierung ohne `libvorbis`:** `AudioDecoder_Ogg` benötigt weiterhin `libvorbis` für die Audio-Dekodierung.
* **Änderung bestehender `ID3Tag`-Mappings:** Die bereits in `ID3Tag.cpp` definierten Tag-Mappings (inkl. `RATING WMP`, `EnergyLevel`, `INITIALKEY`/`INITIAL_KEY`, `DATE`/`YEAR`, `TRACKNUMBER`/`TRACKTOTAL`) bleiben unverändert.

---

## Öffentliche Schnittstelle

### `include/pplib/audio/audioinfo.h`
* Keine Signaturänderungen; bestehendes Feld `bool HaveVorbisComment = false;` wird in `IdentAudioFile()` befüllt und in `AudioInfo::print()` ausgegeben.

### `include/pplib/audio/id3tag.h`
* Erweiterung von `ID3Tag::AudioFormat`:
  ```cpp
  enum AudioFormat
  {
      AF_UNKNOWN = 0,
      AF_MP3,
      AF_AIFF,
      AF_WAVE,
      AF_FLAC,
      AF_OGG
  };
  ```
* Bestehende öffentliche Methoden (`load`, `tryLoad`, `save`, `exportVorbisComments`, alle Getter/Setter) bleiben unverändert; `exportFlacPictures` wird in `exportVorbisPictures` umbenannt (da für FLAC und Ogg genutzt).
* Private Ergänzungen in `ID3Tag`:
  * `void loadOgg(FileObject& file);`
  * `void saveOgg();`
  * *(Das bestehende Member `String flacVendor;` wird für FLAC und Ogg gemeinsam als Vorbis-Comment-Vendor genutzt.)*

### Modulinterne Bausteine (`src/audio/`)
* Gemeinsamer Ogg-Page-/CRC32-Helfer (intern, kein öffentlicher Header, z. B. `src/audio/OggVorbisHelper.h` / `.cpp` oder direkt geteilt zwischen `AudioInfo.cpp` und `ID3Tag.cpp`):
  * Ogg-CRC32 (nicht-reflektiertes Polynom `0x04C11DB7`, Init `0`, XorOut `0`).
  * Ogg-Page-Reader (Page-Header parsen, Segment-Tabelle/Lacing auflösen, logische Pakete über Page-Grenzen zusammensetzen).
  * Ogg-Page-Writer (Pakete in Ogg-Pages mit 255er-Lacing segmentieren, `page_sequence_number` setzen, Ogg-CRC32 berechnen).
* Refactoring in `src/audio/ID3Tag.cpp`:
  * `parseVorbisCommentPayload(...)` (aus `loadFlac` extrahiert, von `loadFlac` und `loadOgg` genutzt; optional mit `METADATA_BLOCK_PICTURE`-Dekodierung).
  * `buildFlacPictureBlock(...)` (existiert bereits) wird wiederverwendet, um für Ogg das binäre `FLAC_PICTURE`-Payload zu erzeugen und per `ByteArrayPtr::toBase64()` als `METADATA_BLOCK_PICTURE=<base64>` in die Vorbis-Comment-Einträge aufzunehmen.

---

## Ownership und Speicherverwaltung

* Alle Lesezugriffe erfolgen über `FileObject::tryMap()` / `FileObject::map()` / `FileObject::seek()` / `FileObject::read()` ohne rohe `new`/`delete`-Allokationen.
* Temporäre Puffer für Pakete, Ogg-Pages und `METADATA_BLOCK_PICTURE` verwenden `ByteArray` und `std::vector` (RAII).
* Beim Speichern (`saveOgg()`) wird wie bei `saveFlac()` über eine temporäre Datei (`Filename + ".rename.tmp"`) gearbeitet, um die Originaldatei bei Fehlern nicht zu beschädigen; `File::rename()` ersetzt das Original atomar erst nach erfolgreichem Abschluss.
* In `IdentAudioFile(FileObject&, AudioInfo&)` gilt weiterhin: bei Rückgabe `false` bleibt der vom Aufrufer übergebene `AudioInfo` unverändert.

---

## Fehlermodell

* **`IdentAudioFile()`:** wirft nur `FileNotOpenException` (wenn `file` nicht geöffnet ist). Ungültige, abgeschnittene oder nicht-Vorbis-Ogg-Dateien liefern sauber `false` bzw. `AudioFormat::UNKNOWN` – kein Throw beim Format-Sniffing.
* **`ID3Tag::load()` / `loadOgg()`:**
  * Nicht-Vorbis-Ogg oder fehlender/kaputter ID- bzw. Comment-Header → `UnsupportedAudioFormatException`.
  * Ungültige Base64-Daten in einem `METADATA_BLOCK_PICTURE`-Eintrag werden tolerant übersprungen (kein Abbruch des gesamten Ladevorgangs wegen eines defekten Cover-Tags).
* **`ID3Tag::save()` / `saveOgg()`:**
  * Nicht gesetzter Dateiname / schreibgeschützte Datei / I/O-Fehler → bestehende File-Exceptions (`FileNotOpenException`, `FileException` etc.); temporäre Datei wird im `catch`-Block entfernt.
  * Beschädigte Header-Struktur in der Zieldatei → `UnsupportedAudioFormatException`.

---

## Thread-Safety

* Kein globaler oder statischer veränderlicher Zustand.
* Falls für Ogg-CRC32 eine Lookup-Tabelle verwendet wird, ist sie `constexpr` / `const` (rein lesend).
* `AudioInfo` und `ID3Tag` bleiben pro Instanz nicht thread-sicher bei gleichzeitigen Schreibzugriffen; paralleles Lesen/Identifizieren auf getrennten Instanzen und getrennten `FileObject`s ist vollständig thread-sicher.

---

## Plattform-Grenzen

* Plattformunabhängig (Windows MinGW64, Linux, FreeBSD), kein `libvorbis`/`libogg` erforderlich.
* Endianness: Ogg-Page-Header und Vorbis-Comment-Längenfelder sind *Little Endian* (`Peek32`/`Poke32`, `Peek64`/`Poke64` aus `pplib`); Bild-Blöcke (`METADATA_BLOCK_PICTURE`) verwenden im Inneren *Big Endian* (`PeekN32`/`PokeN32`), identisch zu FLAC.
* `FileObject` unterstützt 64-Bit-Dateioffsets (`uint64_t`), daher keine 2-GB-/4-GB-Grenze bei großen Dateien.
* Mikrocontroller (`PICO_BUILD`): `AudioInfo` und `ID3Tag` werden dort nicht verwendet bzw. bleiben ohne externe Library-Abhängigkeiten kompilierbar.

---

## Entscheidungen (abgestimmt)

1. **`saveOgg()` über Temp-Datei:** `saveOgg()` schreibt immer sauber über `.rename.tmp` (inkl. ggf. angepasster `page_sequence_number` und Ogg-CRC32 der nachfolgenden Audio-Pages).
2. **`METADATA_BLOCK_PICTURE` beim Export:** `METADATA_BLOCK_PICTURE` wird bei Ogg in `APIC`-Frames dekodiert; `exportFlacPictures()` wird in `exportVorbisPictures()` umbenannt und funktioniert identisch für FLAC und Ogg. `exportVorbisComments()` liefert weiterhin Text-Tags.
3. **`IdentAudioFile` für unseekbare / abgeschnittene Ogg-Dateien:** Fehlt am Dateiende eine gültige Page mit `granule_position != -1` derselben Serial-Number, bleiben `Samples = 0` und `Length = 0` (analog zu Streaming-FLAC), während Format, Kanäle, Frequenz, Bitrate und `HaveVorbisComment` trotzdem gesetzt werden.
