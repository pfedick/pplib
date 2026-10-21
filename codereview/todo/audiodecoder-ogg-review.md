# AudioDecoder_Ogg Review – Befunde

Review vom 2026-10-21, Scope: `src/audio/AudioDecoder_Ogg.cpp` (370 Zeilen).
Öffentliche API: `pplib::AudioDecoder_Ogg` (`open`, `close`, `getAudioInfo`, `seekSample`, `getPosition`, `getSamples(size_t, STEREOSAMPLE16*)`, `addSamples(size_t, STEREOSAMPLE32*)`, `getSamples(size_t, STEREOSAMPLE_FLOAT*)`, `addSamples(size_t, STEREOSAMPLE_FLOAT*)`), plus `IdentOggVorbisAudioFile()`.
Review done by: Copilot (Copilot SDK in VS Code)

Alle kritischen Befunde wurden mit Probeprogrammen gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` (GCC 14, Linux/x86-64, `-g -O0`) verifiziert; valgrind 3.x für Leaks/Invalid Access.

Referenz auf Basisreview: [audiodecoder-review.md](./audiodecoder-review.md).

## Bugs (kritisch)

- [ ] **NULL-Zeigerdereferenz in `open()`, `getSamples()` und `seekSample()` nach `close()`** (`src/audio/AudioDecoder_Ogg.cpp:168`, `220`, `239`, `301`, `337`)
  `close()` gibt das PImpl-Objekt frei und setzt `private_data = NULL` (Zeile 200-201). Jeder folgende Aufruf castet blind und schreibt in Adresse 0:
  ```cpp
  OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);  // == NULL
  int res = ov_open_callbacks(&file, &oggp->vf, NULL, 0, oggp->callbacks);        // Zeile 170: Schreibzugriff auf NULL+offsetof(vf)
  ```
  Verifiziert: `open()` → `close()` → erneut `open()` ergibt SIGSEGV (Exit 139). Dasselbe gilt für `getSamples(..., STEREOSAMPLE16*)` (Zeile 239/245), `seekSample()` (220/221) und beide Float-Pfade (301/306, 337/342). Da der Destruktor `close()` aufruft (Zeile 152), crasht bereits der ganz normale Ablauf „open → close → weiterarbeiten".
  Behebung: `private_data` im Konstruktor anlegen und dort belassen (nur im Destruktor freigeben), oder in `open()` neu allozieren, falls `private_data == NULL`. Zusätzlich in jeder Methode `if (!private_data) throw DecoderException("not open");` einziehen. Vor `ov_open_callbacks()` in `open()` muss außerdem ein bereits offener Stream mit `ov_clear()` geschlossen werden (siehe nächster Befund).

- [ ] **Speicherleck bei doppeltem `open()` ohne `close()`** (`src/audio/AudioDecoder_Ogg.cpp:170`)
  `open()` ruft `ov_open_callbacks()` ungeprüft erneut. libvorbisfile alloziert pro Aufruf intern den Dekoder, die Seitenerfassung und Puffer; der alte `OggVorbis_File`-Zustand in `oggp->vf` wird überschrieben, die alte Zuordnung ist damit verloren.
  Verifiziert mit valgrind (100× `open()` ohne `close()` an derselben Datei):
  `definitely lost: 4.125.016 Bytes in 1.385 Blöcken`, `indirectly lost: 12.404.756 Bytes`.
  Behebung:
  ```cpp
  void AudioDecoder_Ogg::open(FileObject& file, const AudioInfo* info)
  {
      if (!private_data) { private_data = calloc(sizeof(OggVorbisPrivateData), 1); init_callback(...); }
      OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
      ov_clear(&oggp->vf);                 // vorhandenen Stream sauber schließen (idempotent, wenn vf null ist)
      memset(&oggp->vf, 0, sizeof(oggp->vf));
      file.seek(0);
      int res = ov_open_callbacks(&file, &oggp->vf, NULL, 0, oggp->callbacks);
      ...
  }
  ```

- [ ] **`position` wird im 16-Bit-Pfad nie aktualisiert** (`src/audio/AudioDecoder_Ogg.cpp:236-279`, `281-296`)
  `getSamples(size_t, STEREOSAMPLE16*)` und `addSamples(size_t, STEREOSAMPLE32*)` ändern `position` nicht, die Float-Methoden tun es korrekt (Zeile 327, 363). Nach `seekSample(500)` und `getSamples(4, buf16)` bleibt `getPosition()` bei 500 stehen; der nächste Lesevorgang setzt aber physisch im Stream dahinter an. Folge: `seekSample(getPosition())` springt zurück und dieselben Samples werden endlos wiederholt (Endlosschleife in Mix-/Playback-Schleifen).
  Verifiziert: `seekSample(500)`; `getSamples(4, s16)` → Rückgabe 4, `getPosition()` weiterhin 500; danach `getSamples(4, f32)` → `getPosition()` = 508.
  Behebung: in beiden 16-Bit-Pfaden nach der Schleife `position += gelesene_samples;` (z. B. über eine lokale `size_t done = num - rest/Bpp;`), identisch zu Zeile 327.

- [ ] **Signed-Overflow in `int rest = num * info.BytesPerSample`** (`src/audio/AudioDecoder_Ogg.cpp:240`)
  `num` ist `size_t`; die Multiplikation erfolgt in `size_t` und wird dann in `int` verengt. Ab `num > ~536 Mio.` (Stereo) ist das Ergebnis negativ bzw. UB; `while (rest > 0)` wird dann übersprungen und `return num - (rest/4)` liefert eine völlig falsche, vom Aufrufer als „vollständig gelesen" interpretierte Zahl — der Aufrufer schreibt also über seinen Puffer hinaus, ohne dass Daten geliefert wurden.
  Verifiziert: `getSamples(1u<<30, buf)` liefert `1073741824`, obwohl laut libvorbisfile nur 4 Frames tatsächlich gelesen wurden.
  Behebung: `size_t rest = num * static_cast<size_t>(info.BytesPerSample);` und alle Abhängigen (`bytes_read`-Vergleiche, Rückgabe) auf `size_t` umstellen; zusätzlich Obergrenze pro Aufruf (Chunking, siehe Warnungsblock).

- [ ] **Rückgabewert von `ov_pcm_seek()` ignoriert, `position` ungeclamped** (`src/audio/AudioDecoder_Ogg.cpp:221-222`)
  Ein fehlgeschlagener Seek (z. B. Ziel hinter dem Dateiende → `OV_EINVAL`) wird nicht erkannt, `position` wird trotzdem auf den angeforderten (unsinnigen) Wert gesetzt.
  Verifiziert: `seekSample(1000000000)` → `getPosition()` = 1000000000; anschließend liefert `getSamples(16, …)` trotzdem gültige Samples (der Stream steht noch an alter Position). Ein Aufrufer, der playback-seitig bis `getAudioInfo().Samples` seekt, erhält Daten „aus dem Nichts" und eine Position jenseits des Dateiendes.
  Behebung:
  ```cpp
  void AudioDecoder_Ogg::seekSample(size_t sample)
  {
      if (!private_data) throw DecoderException("AudioDecoder_Ogg::seekSample: not open");
      OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
      if (info.Samples && sample > info.Samples) sample = info.Samples;   // wie Wave/AIFF
      int res = ov_pcm_seek(&oggp->vf, static_cast<ogg_int64_t>(sample));
      if (res != 0) throw DecoderException("AudioDecoder_Ogg::seekSample: %d", res);
      position = sample;
  }
  ```

## Bugs (mittel / Warnungen)

- [ ] **C-Callback wirft C++-Exception über eine C-Bibliothek hinweg** (`src/audio/AudioDecoder_Ogg.cpp:57`)
  `ppl_read_func()` ruft `FileObject::read()`, das bei Dateiende eine `EndOfFileException` wirft. Die Exception durchläuft libvorbisfile-C-Code (und ggf. dessen eigene Frames). Auf Linux/x86-64/GCC ist das abfangbar, aber nicht garantiert: libvorbisfile erwartet einen Short-Count als EOF-Signal, und bei Frames ohne Exception-Personality (bzw. mit `/EHsc`-Konfiguration auf MSVC, oder wenn libvorbis als C-Übersetzungseinheit gebaut ist) führt das zu `std::terminate`.
  Verifiziert: trunkierte OGG → `EndOfFileException` springt durch `ov_read()` bis in `main()`; valgrind dazu: 0 Fehler, keine Leaks (`~AudioDecoder_Ogg()` → `close()` → `ov_clear()`). Kein Leak nachgewiesen, aber ein Portabilitäts-/Vertragsverstoß gegen die Callback-Schnittstelle.
  Behebung: Short-Count liefern statt zu werfen:
  ```cpp
  static size_t ppl_read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
  {
      FileObject* f = static_cast<FileObject*>(datasource);
      try { return f->read(ptr, size * nmemb); }
      catch (const EndOfFileException&) { return 0; }   // libvorbisfile interpretiert 0 als EOF
  }
  ```

- [ ] **`ppl_seek_func()` liefert neue Dateiposition statt 0** (`src/audio/AudioDecoder_Ogg.cpp:62-64`)
  Der Callback-Vertrag von `ov_callbacks::seek_func` verlangt „0 bei Erfolg, ≠0 bei Fehler". Zurückgegeben wird die neue Position. Aktuell funktioniert es nur zufällig, weil libvorbisfile ausschließlich `< 0` prüft — und damit echte Seek-Fehler unsichtbar bleiben (z. B. Seek vor Dateianfang).
  Behebung: Ergebnis prüfen und nur `0`/`-1` zurückgeben:
  ```cpp
  static int ppl_seek_func(void* datasource, ogg_int64_t offset, int whence)
  {
      FileObject::SeekOrigin o = whence == SEEK_SET ? FileObject::SeekOrigin::SEEKSET
                             : whence == SEEK_CUR ? FileObject::SeekOrigin::SEEKCUR
                                                  : FileObject::SeekOrigin::SEEKEND;
      return static_cast<FileObject*>(datasource)->seek(offset, o) >= 0 ? 0 : -1;
  }
  ```

- [ ] **`ppl_tell_func()` gibt `long` zurück — auf Windows/LLP64 nur 32 Bit** (`src/audio/AudioDecoder_Ogg.cpp:76-78`)
  `FileObject::tell()` liefert eine 64-Bit-Position; der Cast auf `long` schneidet sie unter MSVC/Win64 (und generell LLP64) auf 32 Bit ab. Bei OGG-Dateien > 2 GiB (und MP3/OGG mit großen Offset-Werten) rechnet libvorbisfile mit falschen Positionen, was zu fehlerhaften Seeks und gelesenen Datenmüll führt.
  Behebung: libvorbisfile erwartet `long` in `ov_callbacks`; für >2 GiB ist der Callback-Ansatz so nicht korrekt nutzbar. Empfehlung: eigene `ov_callbacks`-Variante mit 64-Bit-Sicherung, sonst mindestens explizit dokumentieren und per `static_assert(sizeof(long)==8)` bzw. `#error` auf LLP64-Plattformen kennzeichnen, dass große Dateien nicht unterstützt werden.

- [ ] **`allocateBuffer()` belässt `buffersize` auf altem Wert, wenn `malloc()` fehlschlägt** (`src/audio/AudioDecoder_Ogg.cpp:157-161`)
  Nach `free(readbuffer)` (Zeile 158) ist `readbuffer` ein hängender Zeiger, `buffersize` bleibt aber der alte Wert. Wirft `malloc()` (Zeile 160), ist der nächste Aufruf mit derselben Größe wegen `if (size != buffersize)` ein No-Op — `addSamples()` arbeitet dann mit freigegebenem Speicher weiter.
  Behebung: erst allozieren, dann freigeben, und `readbuffer`/`buffersize` sofort nach dem `free()` auf `NULL`/`0` setzen.

- [ ] **`addSamples(size_t, STEREOSAMPLE32*)` allokiert `4 * num` Bytes am Stück** (`src/audio/AudioDecoder_Ogg.cpp:285`)
  Bei `num = 100000` sind das 400 KiB, bei `num = 1 Mio.` 4 MiB — für Mikrocontroller-Ziele (RP2040) ungeeignet, und die Float-`addSamples()`-Variante (Zeile 334) zeigt, dass es ohne Zwischenpuffer geht. Zusätzlich fehlt hier die in `decoder.h` dokumentierte Obergrenze pro Aufruf.
  Behebung: in festen Blöcken (z. B. 4096 Samples) nachladen und sofort aufaddieren; Puffergröße einmal im Konstruktor/`open()` festlegen.

- [ ] **`(int)(num - total)` schneidet `size_t` auf `int` ab** (`src/audio/AudioDecoder_Ogg.cpp:306`, `342`)
  `ov_read_float()` erwartet die Anzahl Frames als `int`. Bei `num > INT_MAX` wird die Anfrage negativ/abgeschnitten; zudem wird `total` als `long` geführt und mit `(long)num` verglichen (Zeile 304, 340) — auf Windows (LLP64, `long` = 32 Bit) ebenfalls Abschneidegefahr.
  Behebung: Schleife in `size_t` führen und pro Iteration `int want = static_cast<int>(std::min<size_t>(num - total, 4096));` verwenden.

- [ ] **`decodebuffer` wird 16 KiB groß allokiert, aber nur mit 4096 Bytes genutzt** (`src/audio/AudioDecoder_Ogg.cpp:181`, `256-260`)
  `decodebuffer_size = 4096` ist die Elementanzahl, allokiert wird `decodebuffer_size * 4` (16 KiB). Der Mono-Pfad übergibt an `ov_read()` jedoch nur `size = decodebuffer_size` Bytes (Zeile 259-260) — es werden also maximal 2048 Samples decodiert, obwohl 4096 Samples Platz hätten, und 12 KiB bleiben ungenutzt.
  Behebung: Eine Konstante mit klarer Bedeutung: `decodebuffer_bytes = 8192` (Bytes) und `ov_read(..., decodebuffer_bytes, ...)`, Loop-Grenze aus `bytes_read/2` ableiten.

- [ ] **`AudioSize`/`AudioEnd` können 0 bzw. kleiner als `AudioStart` werden** (`src/audio/AudioDecoder_Ogg.cpp:103-105`)
  `info.AudioStart = vf.offset` und `info.AudioEnd = vf.end` sind bei libvorbisfile keine robusten Dateibereiche. Verifiziert an einer kleinen OGG: `AudioStart = 5237`, `AudioEnd = 5237`, `AudioSize = 0` — ein Aufrufer, der daraus `AudioSize / BytesPerSample` oder `AudioEnd - AudioStart` berechnet, erhält 0 (oder bei vorzeichenloser Arithmetik einen riesigen Wert).
  Behebung: `AudioStart`/`AudioEnd` für OGG nicht aus `vf.offset`/`vf.end` ableiten, sondern mit `ov_raw_total(&vf, -1)` (oder Dateigröße) bestimmen; `AudioEnd >= AudioStart` sicherstellen.

- [ ] **`getSamples()` begrenzt `num` nicht an `info.Samples`** (`src/audio/AudioDecoder_Ogg.cpp:243`, `304`)
  Nach `seekSample()` nahe ans Dateiende liefert die Methode weniger als angefordert (korrekt), aber ein Aufrufer, der „bis `Samples`" durchliest, erhält keine klare Grenze, solange `position` (16-Bit-Pfad) nicht gepflegt wird. Zusammen mit Befund „`position` wird nie aktualisiert" ergibt das Endlosschleifen.
  Behebung: `num = min(num, info.Samples - position)` (nach Fix von `position`), Rest als 0 zurückgeben.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Typlose PImpl via `void*`** (`include/pplib/audio/decoder.h:263`)
  `void* private_data` verhindert Typsicherheit und macht jeden `static_cast` (Zeile 140, 168, 198, 220, 239, 301, 337) zum manuellen Downcast. Empfehlung: `struct OggVorbisPrivateData;` im Header vordeklarieren, Member als `std::unique_ptr<OggVorbisPrivateData>` — dann entfallen `free()`/`calloc()` komplett und der Destruktor wird implizit korrekt.
- [ ] **`int decodebuffer_size`** (`include/pplib/audio/decoder.h:268`)
  Größen gehören zu `size_t`; `int` ist hier zusätzlich semantisch uneindeutig (Elemente oder Bytes, siehe Befund oben).
- [ ] **Vier nahezu identische Methoden** (`src/audio/AudioDecoder_Ogg.cpp:236-296`, `298-368`)
  Die Float-Methoden unterscheiden sich nur in `=` vs. `+=`; die 16-Bit-Methoden ebenfalls. Empfehlung: eine private Kernmethode `template <class T, class Op> size_t readSamples(size_t num, T* buffer, bool additive)` bzw. Concepts + Lambda für das Combine-Verhalten. Das behebt auch die unterschiedliche `position`-Pflege (Quelle des kritischen Bugs).
- [ ] **`#ifdef HAVE_LIBVORBIS` in jeder Methode** (`src/audio/AudioDecoder_Ogg.cpp:167`, `191`, `219`, `238`, `283`, `300`, `336`)
  Sieben Kopien derselben Bedingung, teils mit stillschweigend `return 0;` im `#else`-Zweig (Zeile 277, 294, 330, 366) — der Aufrufer kann „kein Vorbis im Build" nicht von „Dateiende" unterscheiden. Empfehlung: Kompatibilitätsklasse mit `throw UnsupportedFeatureException(...)` in einer einzigen `#else`-Implementierung (Muster wie Konstruktor Zeile 134-135), Datei in `AudioDecoder_Ogg.cpp` + `AudioDecoder_Ogg_stub.cpp` trennen.
- [ ] **C-Style-Casts und ungenutzte Includes** (`src/audio/AudioDecoder_Ogg.cpp:242`, `257`, `286`, `288-289`; `34`, `35`)
  `(char*)buffer`, `(pplib::SAMPLE16*)decodebuffer`, `(STEREOSAMPLE16*)readbuffer` → `reinterpret_cast` mit Kommentar, warum die Aliasse ok sind. `#include <pplib/core/functions.h>` und `<pplib/core/mutex.h>` werden nicht benötigt (im Gegensatz zur MP3-Implementierung gibt es hier keinen Global-Mutex).
- [ ] **Mono-Unterstützung ist nur im Code sichtbar** (`src/audio/AudioDecoder_Ogg.cpp:178`, `255`)
  Als einzige der vier Decoder-Klassen akzeptiert OGG Mono (und dupliziert den Kanal auf rechts). Das ist eine API-Eigenschaft und gehört in die Klassen-Doku in `decoder.h`.
- [ ] **Auskommentierter `ppl_close_func`** (`src/audio/AudioDecoder_Ogg.cpp:68-74`)
  Toter Code im Kommentarblock; `callbacks.close_func = NULL` (Zeile 83) ist korrekt begründet (Dateibesitz bleibt beim Aufrufer) — das als Kommentar neben Zeile 83 sichern und den Block entfernen.

## Dokumentation & Doxygen

- [ ] **Klasse `AudioDecoder_Ogg` ohne Doxygen** (`include/pplib/audio/decoder.h:260`)
  Es fehlt der für alle Decoder gleiche Block: unterstützte Formate/Profile (Vorbis, 1-2 Kanäle, beliebig Abtastrate), Hinweis, dass `BitsPerSample` immer 16 beträgt (Vorbis ist intern Float), sowie Exception-Verhalten.
- [ ] **`getSamples(size_t, STEREOSAMPLE16*)`: Rückgabewert und Verhalten undokumentiert** (`include/pplib/audio/decoder.h:281`)
  Offen: Rückgabe = tatsächlich gelesene Frames; was passiert am Dateiende (0, keine Exception); ob `num` über die Dateigröße hinaus begrenzt wird. Genau hier weichen 16-Bit- und Float-Pfad voneinander ab (`position`).
- [ ] **`seekSample()`-Verhalten undokumentiert** (`include/pplib/audio/decoder.h:279`)
  Nicht dokumentiert: Clamping an `info.Samples` (fehlt, siehe Bug), Exception bei Fehler (fehlt, siehe Bug), Verhalten nach `close()` (Crash).
- [ ] **`close()`-Semantik widerspricht der Basisklassen-Doku** (`include/pplib/audio/decoder.h:97-98`)
  Die Basis sagt, `getAudioInfo()` sei nach `close()` „undefiniert". `AudioDecoder_Ogg::close()` setzt `info` gar nicht zurück (Zeile 189-210), macht das Objekt aber durch `private_data = NULL` weitgehend unbenutzbar. Doku präzisieren: entweder `info` in `close()` nullen oder explizit garantieren, dass `getAudioInfo()` nach `close()` gültig bleibt.
- [ ] **`IdentOggVorbisAudioFile()` nirgends deklariert** (`src/audio/AudioDecoder_Ogg.cpp:113`)
  Die Funktion ist extern (nicht `static`), aber es existiert nur eine lokale Vorwärtsdeklaration in `src/audio/AudioInfo.cpp:39`. Damit gibt es keinen Header, kein `#include`-Guard gegen Signaturdrift, und `-Wmissing-declarations` schlägt an. Entweder in `include/pplib/audio/audioinfo.h` aufnehmen oder `static` machen (dann muss `AudioInfo.cpp` sie allerdings über einen anderen Weg erreichen).

## Befunde in Drittdateien

- [ ] **`FileObject::read()` wirft bei Dateiende statt Short-Count** (`src/core/File.cpp:489-491`)
  Für Stream-/Callback-basierte Decoder (OGG, MP3-Feed) ist eine Exception am Datenende ein schlechtes Signal; sie durchquert C-Bibliotheken. Betrifft auch `AudioDecoder_MP3::fillDecodeBuffer()` (`src/audio/AudioDecoder_MP3.cpp:193`).
- [ ] **`IdentAIFF` berechnet `BytesPerSample` aus der Bitbreite statt aus der Sample-Stride** (`src/audio/AudioInfo.cpp:93`)
  `(PeekN16(adr+14) * info.Channels)/8` ignoriert, dass AIFF-Samples auf 16-Bit-Wortgrenzen aufgefüllt sein dürfen. Für den OGG-Report nicht relevant, aber identische Fehlerquelle im AIFF-Decoder — Details in [audiodecoder-aiff-review.md](./audiodecoder-aiff-review.md).
- [ ] **`AudioInfo::Samples`/`Length`-Berechnung mit `uint32_t`-Multiplikation** (`src/audio/AudioDecoder_Ogg.cpp:108`)
  `info.Samples * 1000` wird in `uint32_t` (bzw. `long long` je nach Typ von `Samples`) gerechnet; bei `Samples > ~4,3 Mio.` (≈97 s bei 44,1 kHz, falls `Samples` als 32-Bit-Typ vorliegt) droht Überlauf. In `AudioInfo.cpp:165` wird dasselbe Problem bereits mit `(uint64_t)`-Cast gelöst — hier fehlt er.
