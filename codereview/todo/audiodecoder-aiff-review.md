# AudioDecoder_Aiff Review – Befunde

Review vom 2026-10-21, Scope: `src/audio/AudioDecoder_Aiff.cpp` (200 Zeilen).
Öffentliche API: `pplib::AudioDecoder_Aiff` (`open`, `close`, `getAudioInfo`, `seekSample`, `getPosition`, `getSamples(size_t, STEREOSAMPLE16*)`, `addSamples(size_t, STEREOSAMPLE32*)`, `getSamples(size_t, STEREOSAMPLE_FLOAT*)`, `addSamples(size_t, STEREOSAMPLE_FLOAT*)`).
Review done by: Copilot (Copilot SDK in VS Code)

Alle Befunde wurden mit Probeprogrammen gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` (GCC 14, Linux/x86-64, `-g -O0`) verifiziert. Testdateien: ffmpeg-`pcm_s16be`/`pcm_s24be`-AIFFs sowie handgeschriebene AIFFs mit bekanntem Byte-Muster (Padding, Sign-Magnitude, gefälschte COMM-Felder).

Referenz auf Basisreview: [audiodecoder-review.md](./audiodecoder-review.md).

## Bugs (kritisch)

- [ ] **NULL-Zeigerdereferenz in allen vier Lesemethoden nach `close()`** (`src/audio/AudioDecoder_Aiff.cpp:108`, `132`, `156`, `180`)
  `close()` setzt `ff = NULL` (Zeile 81), die Lesemethoden rufen aber ungeprüft `ff->map(...)` auf:
  ```cpp
  const char* data = ff->map(info.AudioStart + position * info.BytesPerSample, samples * info.BytesPerSample);
  ```
  Verifiziert: `open(); close(); getSamples(…, STEREOSAMPLE16*)` → SIGSEGV, Exit-Code 139. Da der Destruktor `close()` aufruft (Zeile 49), ist jeder Ablauf „objektweit open → close → weiterlesen" betroffen; `FileObject::map()` wirft zwar `FileNotOpenException`, wenn das File selbst geschlossen ist (Zeile 752 in `src/core/File.cpp`), aber hier ist der **Zeiger auf das File-Objekt** NULL, davor gibt es keine Prüfung.
  Behebung (in allen vier Methoden identisch):
  ```cpp
  if (!ff) throw DecoderException("AudioDecoder_Aiff: not open");
  ```

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

- [ ] **Zweierkomplement statt Sign-Magnitude interpretiert** (`src/audio/AudioDecoder_Aiff.cpp:111-112`, `117-118`, `135-136`, `141-142`, `159-160`, `165-166`, `183-184`, `189-190`)
  Die AIFF-Spezifikation definiert Sample-Werte als **Sign-Magnitude**, der Decoder interpretiert die Bytes als Zweierkomplement (`(int16_t)PeekN16(data)` bzw. `(int32_t)(PeekN24(data) << 8) >> 8`).
  Verifiziert an einer handgeschriebenen 16-Bit-Stereo-Datei `/tmp/sm16.aiff` mit den Rohwerten `0x7FFF,0x8001 / 0xFFFF,0x0001 / 0x8000,0x1234`:
  Decoder: `[0]=32767/-32767`, `[1]=-1/1`, `[2]=-32768/4660`.
  Spezifikationsgemäß wären es `[0]=32767/-1`, `[1]=-32767/1`, `[2]=0/4660`.
  Alle negativen Samples einer spec-konformen Datei sind damit komplett falsch (Größenordnung statt Kleinsignal). Einschränkung der Beweislage: ffmpeg schreibt AIFF im Zweierkomplement, moderne Encoder halten sich offenbar mehrheitlich nicht an Sign-Magnitude — welche Encoder spec-konform schreiben, wurde nicht untersucht. Der Befund ist deshalb als **Spezifikations-/Interoperabilitätsrisiko** zu bewerten, nicht als „alle Dateien kaputt".
  Behebung: Verhalten explizit machen — entweder
  1. in `decoder.h` dokumentieren, dass nur Zweierkomplement-AIFFs unterstützt werden (und die Klasse heißt dann praktisch „unofficial AIFF"), oder
  2. Sign-Magnitude korrekt anwenden und für Zweierkomplement-Dateien eine Erkennung/Option anbieten. Ein robustes Automatik-Kriterium gibt es nicht; ein Heuristik-Test über die ersten Frames (Anteil ausgeschlagener Bits) ist möglich, aber unsicher.

- [ ] **`addSamples(size_t, STEREOSAMPLE32*)` addiert 16-Bit- und 24-Bit-Quellen in unterschiedlichen Skalen** (`src/audio/AudioDecoder_Aiff.cpp:133-146`)
  Im 16-Bit-Zweig werden Rohwerte des Bereichs ±32767 aufaddiert (Zeile 135-136), im 24-Bit-Zweig die vollen 24-Bit-Werte ±8388607 (Zeile 143-144) — anders als in `getSamples(..., STEREOSAMPLE16*)`, wo 24 Bit auf 16 Bit heruntergerechnet wird (Zeile 119-120).
  Folge: Wird eine 24-Bit-Datei mit einer 16-Bit-Datei über `addSamples(…, STEREOSAMPLE32*)` gemischt, ist die 24-Bit-Spur 256× zu laut; zudem ist der Wertebereich von `SAMPLE32` (int32_t) bei 24-Bit-Mehrkanal-/Mehrfachüberlagerung deutlich schneller ausgeschöpft.
  Behebung: einheitliche Ziel-Skala definieren und im Header dokumentieren. Für Konsistenz mit dem 16-Bit-Pfad:
  ```cpp
  buffer[i].left  += l >> 8;   // wie getSamples(STEREOSAMPLE16*): 24 Bit -> 16-Bit-Wertebereich
  buffer[i].right += r >> 8;
  ```
  (Oder umgekehrt den 16-Bit-Zweig auf 24-Bit-Skala hochskalieren — dann muss das aber in `decoder.h` stehen.)

- [ ] **`info.Samples` wird nie gegen `AudioSize`/Dateigröße geprüft → `OverflowException` statt sauberem Dateiende** (`src/audio/AudioDecoder_Aiff.cpp:107-108`, Ursache `src/audio/AudioInfo.cpp:92`)
  `info.Samples` kommt ungeprüft aus dem COMM-Chunk (Zeile 92), die Lesebegrenzung in Zeile 107 verlässt sich darauf. `File::map()` wirft bei Bereichsüberschreitung `OverflowException` (`src/core/File.cpp:769`) statt `nullptr`.
  Verifiziert an `/tmp/aiff_lie.aiff` (COMM-Feld `numSampleFrames` von 44100 auf 100000 geändert, Datei unverändert 176454 Bytes): `seekSample(44000)`; `getSamples(4096, s)` → `OverflowException` (statt erwarteterweise 100 Samples oder 0).
  Behebung: In `IdentAIFF` `info.Samples = min(info.Samples, info.AudioSize / BytesPerSample)` (nach Bestimmung von `AudioSize`), und im Decoder zusätzlich die Byte-Anzahl an die Dateigrenze anpassen:
  ```cpp
  size_t avail = (info.AudioStart + samples * info.BytesPerSample > ff->size())
                     ? static_cast<size_t>(ff->size() - info.AudioStart) / info.BytesPerSample
                     : samples;
  if (avail < samples) samples = avail;
  ```

- [ ] **`open()` prüft `BitsPerSample` und `BytesPerSample` unabhängig voneinander** (`src/audio/AudioDecoder_Aiff.cpp:71-74`)
  Ein `AudioInfo` mit `BitsPerSample=16` und `BytesPerSample=6` passiert beide Prüfungen; die Lesemethoden laufen dann mit 16-Bit-Logik (Stride 4) über einen 6-Byte-Stride.
  Verifiziert: `open(f, &info)` mit `Bits=16, Bpp=6` an einer 24-Bit-Datei → keine Exception, Ergebnisse `[0]=60/-26368`, `[1]=15513/350`, `[2]=24833/24161`, `[3]=723/-8446`.
  Behebung: Kombination prüfen, z. B.
  ```cpp
  if (this->info.BitsPerSample == 16 && this->info.BytesPerSample != 4) throw UnsupportedAudioFormatException("16 bit requires 4 bytes/frame");
  if (this->info.BitsPerSample == 24 && this->info.BytesPerSample != 6 && this->info.BytesPerSample != 8)
      throw UnsupportedAudioFormatException("24 bit requires 6 or 8 bytes/frame");
  ```

## Bugs (mittel / Warnungen)

- [ ] **Bei `BitsPerSample ∉ {16, 24}` werden Samples „gelesen", ohne Daten zu liefern** (`src/audio/AudioDecoder_Aiff.cpp:123-125`, `147-149`, `171-173`, `195-197`)
  Haben die `if/else if`-Ketten keinen Treffer, wird die Schleife komplett übersprungen, `position += samples` und `return samples` ausgeführt — der Aufrufer glaubt, `samples` gültige Samples erhalten zu haben (der Puffer bleibt unbeschrieben). Über `open()` ist dieser Fall aktuell ausgeschlossen; die Methoden sind aber auch für direkt gesetztes `info` ausgelegt und die Basisklasse garantiert nichts.
  Behebung: `else throw UnsupportedAudioFormatException("unsupported BitsPerSample");` bzw. `return 0;` ohne `position`-Vorschub.

- [ ] **Möglicher Unterlauf in `samples = info.Samples - position`** (`src/audio/AudioDecoder_Aiff.cpp:107`, `131`, `155`, `179`)
  Ist `position > info.Samples` (z. B. nach `open()` mit einem zweiten, kleineren `AudioInfo`, ohne dass `position` zurückgesetzt würde — `open()` setzt `position = 0` in Zeile 75, aber `seekSample()` clampet an ein ggf. veraltetes `info`), wird `info.Samples - position` zu einem riesigen `size_t` und `map()` entsprechend fehlparametrisiert. Aktuell nicht erreichbar, aber eine reine Verteidigungszeile behebt das:
  ```cpp
  if (position >= info.Samples) return 0;
  size_t samples = std::min(num, info.Samples - position);
  ```

- [ ] **`close()` setzt `info` nicht zurück** (`src/audio/AudioDecoder_Aiff.cpp:79-84`)
  Nach `close()` liefert `getAudioInfo()` (Zeile 86-89) weiterhin die Werte der alten Datei. Zusammen mit `seekSample()` (Clamping an das alte `info.Samples`, Zeile 93-96) kann ein wiederverwendetes Objekt an einer Position landen, die zur neuen Datei nicht passt.
  Behebung: `info = AudioInfo{};` in `close()` — oder in `decoder.h` garantieren, dass `getAudioInfo()` nach `close()` gültig bleibt (dann aber in allen vier Decodern einheitlich).

- [ ] **Mono-AIFFs werden abgelehnt, aber von `IdentAudioFile` akzeptiert** (`src/audio/AudioDecoder_Aiff.cpp:70`)
  `IdentAIFF` akzeptiert `Channels == 1` (`src/audio/AudioInfo.cpp:86-87`), der Decoder wirft `UnsupportedAudioFormatException("Channels != 2")`. Über `GetAudioDecoder()` führt das zum Leak des bereits erzeugten Objekts (Basisreview, Befund „`GetAudioDecoder()` leckt das Decoder-Objekt"). Verifiziert: Mono-AIFF → `1.760 bytes in 20 blocks definitely lost` (Allokation `AudioDecoder.cpp:44`), ohne `try/catch` des Aufrufers `terminate`.
  Behebung: Mono unterstützen (Kanal duplizieren, wie OGG/MP3 es tun) **oder** bereits in `IdentAIFF` auf Stereo beschränken, damit die Fabrik gar keinen Decoder erzeugt.

- [ ] **`File::map()` pro Aufruf — Remapping bei jeder sequentiellen Leseoperation** (`src/audio/AudioDecoder_Aiff.cpp:108`, `132`, `156`, `180`)
  `File::map()` liefert nur dann einen bestehenden Mapping-Bereich zurück, wenn die angeforderte Region vollständig in der letzten liegt (`src/core/File.cpp:757-759`); sonst `munmap` + `mmap`. Da `position` bei jedem Aufruf weiterwandert, wird bei sequentiellem Playback **pro `getSamples()`-Aufruf** ein Syscall-Paar ausgeführt. Standardmäßig ist `ReadAhead = 0` (`src/core/File.cpp:88`), es gibt also keinen Puffer. Auf Plattformen ohne `mmap` (RP2040/Pico) ist der Pfad grundsätzlich nicht nutzbar.
  Behebung: `ff->setMapReadAhead(64 * 1024)` nach `open()`, oder auf `read()` in einen internen Ringpuffer umstellen (einheitlich mit Wave/MP3/OGG).

- [ ] **24→16-Bit-Downscale ohne Rundung** (`src/audio/AudioDecoder_Aiff.cpp:119-120`)
  `(int16_t)(l >> 8)` schneidet die niederen 8 Bits ab; bei negativen Werten wird Richtung −∞ gerundet, was einen konstanten Bias von bis zu 1 LSB (und bei kleinen Signalen sichtbares Rauschen) erzeugt.
  Behebung: `(int16_t)((l + 0x80) >> 8)` (mit Sättigung), oder bewusst dokumentieren, dass trunciert wird.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Vier nahezu identische Methoden** (`src/audio/AudioDecoder_Aiff.cpp:104-198`)
  Die Blöcke unterscheiden sich nur in Zieltyp, `=`/`+=` und der Skalierung. Alle oben genannten Befunde (Padding, Skala, toter Zweig, NULL-Check) sind viermal kopiert. Empfehlung: eine private Kernmethode
  ```cpp
  template <class T> size_t readFrames(size_t num, T* dst, bool additive);
  ```
  mit `if constexpr`-Zweig für `STEREOSAMPLE16`/`STEREOSAMPLE32`/`STEREOSAMPLE_FLOAT`; damit gibt es genau eine Stelle für Stride, Skala und Grenzziehung.
- [ ] **Totes Member `samplesize`** (`include/pplib/audio/decoder.h:212`, gesetzt in `src/audio/AudioDecoder_Aiff.cpp:43`, `83`)
  Wird von keiner Methode gelesen. Entfernen (Header-Änderung, API-relevant).
- [ ] **Magische Zahlen für Stride und Skalierung** (`src/audio/AudioDecoder_Aiff.cpp:113`, `121`, `137`, `145`, `159`, `167`, `183`, `191`)
  `data += 4` / `data += 6`, `32768.0f`, `8388608.0f`. Empfehlung: `constexpr size_t kBytesPerChannel16 = 2;`, `constexpr float kScale16 = 1.0f / 32768.0f;` usw. und `info.BytesPerSample` als einzige Stride-Quelle (siehe Padding-Befund).
- [ ] **Manuelle Vorzeichenextension über Unsigned-Shift** (`src/audio/AudioDecoder_Aiff.cpp:117-118`, `141-142`, `165-166`, `189-190`)
  `PeekN24()` liefert `uint32_t` (`include/pplib/core/functions.h:307`); `(int32_t)(wert << 8) >> 8` ist zwar mit C++20 definiert (modulare Konversion, arithmetisches Shift), aber schwer zu lesen. Empfehlung: eigene Hilfsfunktion `constexpr int32_t SignExtend24(uint32_t v) { return static_cast<int32_t>(v << 8) >> 8; }` mit Kommentar — einmal dokumentiert, viermal verwendet.
- [ ] **`NULL`/`nullptr` gemischt** (`src/audio/AudioDecoder_Aiff.cpp:44`, `81` verwenden `NULL`; sonst `nullptr` in `map()`-Vergleichen)
  Für neuen Code einheitlich `nullptr`; die Member-Deklaration in `decoder.h:209` bleibt unverändert.

## Dokumentation & Doxygen

- [ ] **Klasse `AudioDecoder_Aiff` ohne Doxygen** (`include/pplib/audio/decoder.h:206`)
  Es fehlen: unterstützte Varianten (AIFF/AIFF-C unkomprimiert, Big Endian), Kanalbeschränkung auf Stereo, unterstützte Bitbreiten (16/24), Annahme Zweierkomplement statt Sign-Magnitude, Umgang mit Sample-Padding. Ohne diesen Block ist die Klasse per IntelliSense nicht korrekt anwendbar.
- [ ] **`open()`-Fehlerbedingungen undokumentiert** (`include/pplib/audio/decoder.h:217`)
  Welche `AudioInfo`-Felder welche Exception auslösen (Zeile 67-74), und dass ein vom Aufrufer übergebenes `info` ungeprüft (bis auf diese vier Bedingungen) übernommen wird.
- [ ] **Rückgabewert der Lesemethoden undokumentiert** (`include/pplib/audio/decoder.h:222-225`)
  Rückgabe = tatsächlich gelesene Frames; am Dateiende 0 ohne Exception; `num` wird an `info.Samples` begrenzt (Zeile 107). Genau diese Begrenzung ist durch den `OverflowException`-Befund außer Kraft gesetzt — Doku muss das Verhalten nach Fix beschreiben.
- [ ] **`seekSample()`-Clamping nicht dokumentiert** (`include/pplib/audio/decoder.h:220`)
  Die Implementierung clampet an `info.Samples` (Zeile 93-96) und wirft nie — das unterscheidet sich von MP3 (Exception) und OGG (kein Clamping, kein Fehlercheck) und gehört in die Header-Doku aller vier Klassen.
- [ ] **`samplesize` ohne Kommentar** (`include/pplib/audio/decoder.h:212`)
  Weder Einheit noch Zweck erkennbar; toter Code (siehe Code-Smell-Block).

## Befunde in Drittdateien

- [ ] **`IdentAIFF` leitet `BytesPerSample` aus der Bitbreite ab** (`src/audio/AudioInfo.cpp:93`)
  Root-Cause des Padding-Befunds; Korrektur gehört in `IdentAIFF` (Ableitung aus `AudioSize / Samples`).
- [ ] **`IdentAIFF` prüft `Samples` nicht gegen `AudioSize`** (`src/audio/AudioInfo.cpp:92`, `119`)
  Ermöglicht den `OverflowException`-Pfad; zusätzlich ist `info.AudioSize = size - 8` (Zeile 119) ein Unterlauf, wenn die SSND-Nutzlänge < 8 Bytes ist (Wert wird zu einem riesigen `uint32_t`).
- [ ] **`IdentAIFF` lehnt AIFF-C mit Kompressions-Offset ab** (`src/audio/AudioInfo.cpp:116-117`)
  `PeekN32(adr + 8) != 0` bzw. `!= 0` bei `adr + 12` → `return false`. Für unkomprimiertes AIFF-C sind beide Felder 0, daher funktioniert AIFF-C; für komprimierte Varianten (`MAC3`, `sowt` …) wird abgelehnt — das ist korrekt, sollte aber als bewusste Einschränkung dokumentiert sein.
- [ ] **80-Bit-`long double`-Konvertierung der Abtastrate** (`src/audio/AudioInfo.cpp:95-110`)
  Direkte Byte-Zuweisung in ein `long double` setzt x87-80-Bit-Layout voraus (10 Byte, Exponent 15 Bit). Auf ARM/AArch64 (`long double` = IEEE 128 Bit) oder MSVC (`long double` = `double`, 8 Byte) ist der Code falsch und liefert eine unplausible `Frequency`. Bereits in [audioinfo-review.md](./audioinfo-review.md) dokumentiert — hier nur als Querverweis, weil der AIFF-Decoder die Konsequenz (falsche `Length`/`Bitrate`) direkt weiterreicht.
- [ ] **`File::map()`-Vertrag: `nullptr` nur bei 0 Bytes, sonst `OverflowException`** (`src/core/File.cpp:750-770`)
  Für Decoder, die Bereichsgrenzen aus Chunk-Metadaten berechnen, ist die Exception ein harter Bruch. Empfehlung: entweder `map()` mit `bytes == 0`-Guard plus expliziter Größenprüfung im Decoder (siehe Behebungsvorschlag) oder eine `mapChecked()`-Variante, die `nullptr` liefert.
- [ ] **`IdentAudioFile` verwirft AIFFs mit inkonsistenter FORM-Größe** (`src/audio/AudioInfo.cpp:221`)
  `PeekN32(adr + 4) < file.size()` — schützt wirksam vor truncierten Dateien (verifiziert: `truncate` auf 1054 Bytes → `ident=0`), lehnt aber auch Dateien ab, deren FORM-Feld korrekt, deren `file.size()` aber durch nachträglich angehängte Daten größer ist. Grenzwertig, aber erwähnenswert, weil `<=` hier toleranter wäre.
