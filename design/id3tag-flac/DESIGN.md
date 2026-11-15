# DESIGN: FLAC-Metadaten in ID3Tag (`VORBIS_COMMENT` & `PICTURE`)

Stand: 2026-11-15. Entscheidungen 1–4 sind abgestimmt; bereit für Phase 2 (Gerüst).

## Zweck und Nicht-Ziele

**Zweck**

* Lesen und Schreiben von FLAC-Metadaten (`VORBIS_COMMENT`, Block-Typ 4; `PICTURE`, Block-Typ 6; `PADDING`, Block-Typ 1) direkt über die bestehende Klasse `ID3Tag` (`include/pplib/audio/id3tag.h`, `src/audio/ID3Tag.cpp`).
* Vollständige Kompatibilität zu bestehenden Gettern/Settern (`Artist`, `Title`, `Album`, `Genre`, `Year`, `Track`, `Comment`, `Remixer`, `Label`, `BPM`, `Key`, `EnergyLevel`, `UserDefinedText`, `Picture`, `Popularimeter`).
* Generischer Erhalt aller `VORBIS_COMMENT`-Einträge (inkl. `TRAKTOR4` oder beliebiger Custom-Tags) als `TXXX`-Frames beim Laden → Ändern → Speichern ohne Sonderbehandlung einzelner Anwendungen.
* Allgemeine `ToBase91` / `FromBase91`-Hilfsfunktionen in `include/pplib/core/stringfunctions.h` und `src/core/StringFunctions.cpp` (analog zu `ToBase64`/`FromBase64`), damit Anwendungen wie WinMusik basE91-kodierte Tags (wie Traktor `TRAKTOR4` in FLAC) komfortabel dekodieren/kodieren können.
* In-Place-Speichern über vorhandenen `PADDING`-Platz, mit Fallback auf Neuschreiben über eine temporäre Datei bei Größenüberschreitung.
* Verlustfreier Erhalt fremder FLAC-Metadata-Blöcke (`STREAMINFO`, `SEEKTABLE`, `CUESHEET`, `APPLICATION`).
* Unabhängig von `libFLAC`: reines Parsing/Serialisieren über `FileObject`.

**Nicht-Ziele**

* **Ogg Vorbis / Ogg FLAC (`OggS`):** nicht Teil dieser Aufgabe (anderes Container-Framing).
* **Traktor-Spezialbehandlung in `ID3Tag`:** `TRAKTOR4` wird in FLAC unverändert als `TXXX`-Frame (`TRAKTOR4`) geladen und gespeichert; die basE91-Dekodierung sowie Auswertung der inneren `TRAKTOR4`-Unterblöcke liegt in der Anwendung.
* **ID3v2-vor-FLAC:** Ein beim Laden ggf. vorhandener führender ID3v2-Tag vor `fLaC` wird beim Lesen ignoriert (nur übersprungen, um `fLaC` zu finden) und beim `saveFlac()` entfernt, da er nicht zur FLAC-Spezifikation gehört.
* **ID3v1-Genre-Nummern:** `saveFlac()` schreibt Genre-Strings ohne führende ID3v1-Nummer `(ID)` in `VORBIS_COMMENT`.

---

## Öffentliche Schnittstelle

### `include/pplib/core/stringfunctions.h` & `src/core/StringFunctions.cpp`

* `String ToBase91(const ByteArrayPtr& bin);`
* `ByteArray FromBase91(const String& str);`
* Standard-basE91-Alphabet (91 druckbare ASCII-Zeichen nach Joachim Henke). Ungültige Zeichen (z. B. Whitespace) beim Dekodieren ignorieren oder bei Bedarf `IllegalArgumentException` prüfen (analog `FromBase64` – siehe Fehlerbehandlung: ungültige Zeichen außerhalb des Alphabets ignorieren vs. werfen; in `FromBase91` Whitespace überspringen und bei anderen ungültigen Zeichen `IllegalArgumentException` werfen).

### `include/pplib/audio/id3tag.h`

* `enum AudioFormat`: neuer Wert `AF_FLAC` am Ende (`AF_WAVE` bleibt unverändert):
  ```cpp
  enum AudioFormat
  {
      AF_UNKNOWN = 0,
      AF_MP3,
      AF_AIFF,
      AF_WAVE,
      AF_FLAC
  };
  ```
* Bestehende öffentliche Methoden von `ID3Tag` behalten ihre Signaturen und Verträge:
  * `load(const String&)`, `load(FileObject&)`, `tryLoad(...)`
  * `save()`
  * Alle `get*()` / `set*()` / `has*()` / `remove*()`-Methoden
* Private Ergänzungen in `ID3Tag`:
  * `String flacVendor;` – gemerkter Vendor-String aus `VORBIS_COMMENT` (Default `"pplib"` beim Speichern, falls leer).
  * `void loadFlac(FileObject& file);`
  * `void saveFlac();`
  * `bool trySaveFlacInExistingFile(FileObject& o, const ByteArrayPtr& metadataBlocks, uint64_t flacStart, uint64_t audioStart);`
  * `void copyFlacToNewFile(FileObject& o, FileObject& n, const ByteArrayPtr& metadataBlocks, uint64_t flacStart, uint64_t audioStart);`

---

## Architektur & internes Mapping (`ID3Frame`)

Intern bleibt `ID3Tag` bei `std::list<ID3Frame>`, damit alle vorhandenen Getter/Setter unverändert funktionieren.

### 1. Erkennung & Verzweigung

* **`identAudioFormat(FileObject& File)`:**
  * Vor `IdentMPEG` prüfen (da `IdentMPEG` in komprimierten FLAC-Daten falsche Sync-Wörter findet).
  * Prüft `fLaC` an Offset `0` oder direkt hinter einem optionalen führenden ID3v2-Header (`ID3`, Major >= 2, Synchsafe-Größe). Gibt dann `AF_FLAC` zurück.
* **`load(FileObject& file)`:**
  1. `clear()`-Reset (`frames.clear()`, `myAudioFormat = AF_UNKNOWN`, `flacVendor.clear()`, `Size = 0`, `Flags = 0`).
  2. `myAudioFormat = identAudioFormat(file);`
  3. Wenn `myAudioFormat == AF_UNKNOWN`: `throw pplib::UnsupportedAudioFormatException();`
  4. Wenn `myAudioFormat == AF_FLAC`: `loadFlac(file); return;`
  5. Sonst wie bisher über `findId3Tag(file)` (MP3, AIFF, WAVE).
* **`save()`:**
  * Bei `myAudioFormat == AF_FLAC` → `saveFlac();`.

### 2. `VORBIS_COMMENT` ↔ `ID3Frame` (alle Text-Frames in `ENC_UTF8`)

Keys im `VORBIS_COMMENT` werden beim Lesen case-insensitiv verglichen und beim Schreiben einheitlich großgeschrieben:

| ID3v2-Frame | `VORBIS_COMMENT`-Key (Schreiben) | Zusätzliche Lese-Fallbacks | Hinweise |
|---|---|---|---|
| `TPE1` | `ARTIST` | — | |
| `TIT2` | `TITLE` | — | |
| `TALB` | `ALBUM` | — | |
| `TCON` | `GENRE` | — | Klartext ohne ID3v1-`(ID)`-Präfix |
| `TRCK` | `TRACKNUMBER` | — | Optional `TRACKTOTAL` bei `"x/y"` |
| `TYER` | `DATE` | `YEAR` | Bei Doppelbelegung (`DATE` + `YEAR`) bevorzugt `DATE` |
| `TBPM` | `BPM` | — | |
| `TKEY` | `INITIALKEY` | `INITIAL_KEY` | Traktor/MixedInKey nutzt `INITIALKEY`, Beatport `INITIAL_KEY` |
| `TPUB` | `LABEL` | `ORGANIZATION` | |
| `TPE4` | `REMIXER` | — | |
| `COMM` | `COMMENT` | `DESCRIPTION` | `setComment(desc, val)` → `TXXX` bzw. benannter Key |
| `TXXX` (`EnergyLevel`) | `ENERGYLEVEL` | — | |
| `TXXX` (`<KEY>`) | `<KEY>` | — | Alle übrigen Keys bleiben als `TXXX` erhalten |

### 3. Sonder-Mappings

* **Popularimeter (`POPM` ↔ `RATING WMP`):**
  * Lesen: `RATING WMP=<0..255>` (als Integer geparst, auf `0..255` begrenzt) wird als `POPM`-Frame abgelegt (synthetischer Identifier `"Windows Media Player 9 Series"`, damit `getPopularimeter()` funktioniert).
  * Schreiben: Erster vorhandener `POPM`-Frame wird als `RATING WMP=<wert>` geschrieben.
* **Cover-Bilder (`APIC` ↔ FLAC-`PICTURE`, Block-Typ 6):**
  * 1:1-Mapping von `pictureType` (`PIC_COVER_FRONT` = 3, `PIC_COVER_BACK` = 4, usw.), MIME-Type, Description (UTF-8) und Bild-Binärdaten.
  * Beim Schreiben wird jedes `APIC`-Frame als eigener `PICTURE`-Block serialisiert (Breite/Höhe/Farbtiefe/Farben = 0, falls nicht geparst).
* **Beliebige/unbekannte Tags (`TXXX` ↔ `VORBIS_COMMENT`):**
  * Alle nicht direkt gemappten `VORBIS_COMMENT`-Einträge (inkl. `TRAKTOR4`, `ISRC`, `KEY`, `ENERGY`, usw.) werden 1:1 als `TXXX`-Frames (UTF-8, Description = Key, Value = Wert) geladen und beim `saveFlac()` unverändert wieder als `KEY=VALUE` geschrieben.
  * Falls über `addFrame` ein `PRIV`-Frame im Speicher liegt (z. B. nach Formatwechsel), wird es in FLAC nicht stillschweigend verworfen, sondern ebenfalls nicht-destruktiv oder per `PRIV_<IDENT>` behandelt? Nein: FLAC kennt kein `PRIV`; in-memory `PRIV`-Frames bleiben bei `saveFlac()` unberücksichtigt bzw. `TXXX` ist das generische Schlüssel-Wert-Format in FLAC.

---

## Save-Strategie (`saveFlac()`)

1. Quelldatei öffnen (`READ`), `fLaC`-Startoffset bestimmen (nach optionalem führendem ID3v2-Tag).
2. Vorhandene Metadata-Blöcke parsen:
   * `STREAMINFO` (Typ 0, Pflicht) sowie `APPLICATION` (2), `SEEKTABLE` (3), `CUESHEET` (5) unverändert als Rohblöcke übernehmen.
   * Bisherige `VORBIS_COMMENT` (4), `PICTURE` (6) und `PADDING` (1) verwerfen (werden aus `frames` neu erzeugt).
   * `audioStart` = Offset direkt hinter dem Block mit gesetztem `last`-Bit (`0x80`).
3. Neue Metadata-Sequenz bauen:
   * `STREAMINFO` (immer Block #0)
   * `SEEKTABLE`, `CUESHEET`, `APPLICATION` (falls vorhanden)
   * `VORBIS_COMMENT` (falls mindestens ein Kommentar oder Vendor gesetzt)
   * `PICTURE`-Blöcke (für jedes `APIC`-Frame)
4. **In-Place-Prüfung (nur wenn kein führender ID3v2-Tag vorliegt, `flacStart == 0`):**
   * Verfügbarer Platz = `audioStart - 4`.
   * Passt die neue Sequenz exakt (`rest == 0`, `last=true` am letzten Block) oder bleibt Restplatz `rest >= 4` Bytes für einen `PADDING`-Block mit `last=true`:
     * Quelldatei `READWRITE` öffnen und direkt ab Offset `4` überschreiben.
   * Sonderfall `1 <= rest <= 3` oder `flacStart > 0` (führender ID3v2-Tag soll entfernt werden): Fallback auf temporäre Datei.
5. **Fallback (temporäre Datei):**
   * `PADDING`-Block (Länge `PaddingSize`, falls `> 0`) mit `last=true` anhängen.
   * Direkt ab Offset 0 mit `fLaC` beginnen (ein ggf. vorher vorhandener führender ID3v2-Tag entfällt), gefolgt von den neuen Metadata-Blöcken und dem unveränderten Audio-Teil (`audioStart .. EOF`).
   * Temporäre Datei über das bestehende Schema ersetzen (Original löschen/umbenennen).

---

## Ownership und Speicherverwaltung

* `ID3Tag` besitzt alle `ID3Frame`-Instanzen per Wert in `std::list<ID3Frame>`.
* Rohdaten von `STREAMINFO` und erhaltenen Nebenblöcken werden während `saveFlac()` lokal in `ByteArray`-Objekten gehalten (RAII – Resource Acquisition Is Initialization).
* Keine statischen oder globalen mutablen Puffer; basE91-Lookup-Tabellen werden `constexpr` / `const` gehalten.

---

## Fehlermodell

* Unbekanntes Dateiformat in `load()` → `UnsupportedAudioFormatException` (wie bisher).
* Ungültige FLAC-Struktur beim Laden (fehlendes `STREAMINFO`, abgeschnittener Block, Blockgröße über EOF) → `UnsupportedAudioFormatException` (oder `tryLoad()` liefert `false` über die bestehende Exception-Behandlung).
* `save()` ohne gesetzten Dateinamen → `FilenameNotSetException` (wie bisher).
* I/O-Fehler beim Lesen/Schreiben → bestehende `File`-/`FileObject`-Exceptions.
* Fehlerhafte basE91-Zeichen in `TRAKTOR4` → ungültige Zeichen ignorieren oder leeren `PRIV`-Frame vermeiden (kein Absturz bei korruptem Tag).

---

## Thread-Safety

* `ID3Tag` ist wie bisher nicht für gleichzeitigen schreibenden Zugriff mehrerer Threads auf dieselbe Instanz gedacht; parallele Nutzung verschiedener `ID3Tag`-Instanzen ist vollständig thread-safe, da keine globalen/statischen mutablen Zustände verwendet werden.
* Die basE91-Hilfsfunktionen sind zustandslos (`const`/`constexpr`-Tabellen).

---

## Plattform-Grenzen

* Plattformübergreifend (Windows MinGW/MSys, Linux, FreeBSD) über `FileObject`, `Peek8`/`Peek32`/`PeekN32` und `Poke32`/`PokeN32` (Little-Endian für `VORBIS_COMMENT`, Big-Endian für FLAC-Blockheader und `PICTURE`).
* **`PICO_BUILD`:** `ID3Tag` wird auf dem Raspberry Pico nicht verwendet; keine speziellen MCU-Pufferlimits nötig, dennoch sauber ohne `libFLAC`-Abhängigkeit.

---

## Testplan

* **Keine Nutzung von `tests/testdata/hidden/`** (urheberrechtlich geschütztes Originalmaterial).
* Bestehende freie Testdateien in `tests/testdata/audio/` (`test_44kHz_16bit.flac`, `test_44kHz_16bit_seektable.flac`, `test_44kHz_16bit_id3v2.flac`) jeweils in temporäre Dateien unter `tests/tmp/` kopieren.
* Neue Tests in `tests/src/audio/flactest.cpp`:
  * Roundtrip aller Standard-Text-Tags (`Artist`, `Title`, `Album`, `Genre`, `Year`, `Track`, `Comment`, `Remixer`, `Label`, `BPM`, `Key`, `EnergyLevel`, `UserDefinedText`).
  * `Picture` setzen, speichern, neu laden und binär vergleichen (`PIC_COVER_FRONT`, `PIC_COVER_BACK`).
  * `Popularimeter` (`RATING WMP`) setzen, speichern, neu laden.
  * `PrivateData`-API ist ID3v2-spezifisch; in FLAC werden alle benutzerdefinierten/unbekannten `VORBIS_COMMENT`-Einträge (inkl. `TRAKTOR4`) unverändert über `TXXX` (`getUserDefinedText`/`setUserDefinedText`) abgebildet.
  * `ToBase91` / `FromBase91` werden separat im Core-Test (`stringfunctions`) mit Längen-Grenzfällen (`0`, `1`, `2`, `3` Bytes und Restlängen) getestet.

---

  ## Offene Punkte

  Keine – alle Design-Entscheidungen sind geklärt. Phase 2 (Gerüst) kann starten.
