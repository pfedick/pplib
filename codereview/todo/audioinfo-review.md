# AudioInfo Review – Befunde

Review vom 2026-10-21, Scope: `include/pplib/audio/audioinfo.h` (99 Zeilen), `src/audio/AudioInfo.cpp` (262 Zeilen), `tests/src/audio/audioinfo.cpp` (221 Zeilen).
Öffentliche API: `pplib::AudioInfo`, `pplib::IdentAudioFile(FileObject&, AudioInfo&)`, `pplib::IdentAudioFile(FileObject&)`, `pplib::WAVEHEADER`, `pplib::UnsupportedAudioFormatException`
Review done by: Copilot (Copilot SDK in VS Code)

Teststand zum Review-Zeitpunkt: `make build_tests` erfolgreich, `test_all --gtest_filter=AudioInfoTest*` → 9 Tests, alle grün.

Alle kritischen Befunde 1–4 wurden mit einem separaten Probeprogramm gegen die Debug-Bibliothek verifiziert (keine Änderung am Projektcode). Reproduktion jeweils angegeben.

## Bugs (kritisch)

- [x] **`IdentAudioFile` wirft `OverflowException` bei Dateien < 24 Bytes** (`src/audio/AudioInfo.cpp:218` bzw. `:241`)
  `file.map(0, 24)` ist nicht abgesichert. `File::map` wirft laut Implementierung (`src/core/File.cpp:769`) eine `OverflowException`, wenn `position + bytes > filesize`; `nullptr` gibt es nur bei leerer Datei oder Mapping-Größe 0 (`src/core/File.cpp:753`). Eine 4-Byte-Datei führt also zu einer ungefangenen Exception statt zu `false`. Die Doku in `include/pplib/core/fileobject.h:604–607` beschreibt dieses Verhalten inzwischen korrekt — die `if (!adr)`-Guards der Aufrufer greifen bei Bereichsüberschreitung nevertheless nicht.
  Verifiziert: 4-Byte-Datei → `EXCEPTION: OverflowException`.
  Bereits offener Befund in `codereview/todo/mp3-review.md` (Zeilen 218–219, 235) — hier mitvermerkt, damit beide Stellen gemeinsam behoben werden.
  Behebung: Größen-Check vor dem Mappen, oder helpers à la `mapNoThrow` (existiert bereits statisch in `src/audio/Mp3.cpp:135`) verwenden:
  ```cpp
  if (file.size() < 24) return false;
  const char* adr = file.map(0, 24);
  if (!adr) return false;
  ```

  UPDATE 2026-10-29: Verhalten von File::map hat sich seit diesem Review geändert! Es wirft nun konsequent immer `OutOfBoundsException` oder andere Exceptions, und gibt niemals `nullptr` zurück.

  **Umsetzung:** Es wird erst gemappt, nachdem die Groessen geprueft sind: `DetectFormat()` bricht
  bei Dateien unter 24 Bytes mit UNKNOWN ab (src/audio/AudioInfo.cpp:294) und der
  MPEG-Zweig delegiert an `IdentMPEG()`, das selbst gueltige Header sucht. Tests
  `EmptyAndTruncatedFiles` und `NonAudioContentLeavesInfoUntouched` erwarten false
  statt einer Exception.

- [x] **`IdentAIFF`: Ergebnis von `file.map(p, 32)` und `file.map(p, 16)` ungeprüft** (`src/audio/AudioInfo.cpp:84`, `:115`)
  Beide `map`-Aufrufe werden sofort dereferenziert (`PeekN16(adr + 8)` bzw. `PeekN32(adr + 8)`). Ein kaputter AIFF-Chunk (z. B. COMM-Chunk-Size `0x7FFFFFFF`) lässt `p += size + 8` über das Dateiende laufen; die Schleifenbedingung `p + 8 < file.size()` prüft nur 8 Bytes, gemappt werden aber 32 bzw. 16 Bytes → `OverflowException`, oder (bei `filesize == 0`) `nullptr`-Deref.
  Verifiziert: AIFF-Kopf mit COMM-Size `0x7FFFFFFF` → `EXCEPTION: OverflowException`.
  Behebung:
  ```cpp
  adr = file.map(p, 32);
  if (!adr) return false;   // bzw. break, je nach gewünschter Semantik
  ```
  Zusätzlich sollte `size` gegen `file.size()` plausibilisiert werden, bevor `p += size + 8` gerechnet wird.

  **Umsetzung:** Saemtliche Maps laufen ueber den Helfer `mapRegion()` (src/audio/AudioInfo.cpp:60),
  der Position und Laenge gegen die Dateigroesse prueft, bevor `File::map()`
  aufgerufen wird. Der COMM-Chunk mit `0x7FFFFFFF` steht in `AiffCommChunkBeyondEof`,
  eine bis ans Dateiende reichende Chunkschleife in
  `AiffSampleOffsetAndChunkBeyondEof`.

- [x] **`IdentWave`: `file.map(fmtchunklen + 20, 8)` wirft bei manipulierter `fmtchunklen`** (`src/audio/AudioInfo.cpp:154`)
  Die Prüfung `if (!buffer) return false;` greift nicht, weil `map` bei Bereichsüberschreitung wirft statt `nullptr` zu liefern. Eine WAV-Datei mit `fmt chunk size = 0x7FFFFFFF` (Format-Tag PCM, 16 Bit) bringt `IdentAudioFile` zur Exception.
  Verifiziert: präparierte 36-Byte-WAV → `EXCEPTION: OverflowException`.
  Behebung: `fmtchunklen` gegen `file.size()` validieren (z. B. `if (fmtchunklen > file.size()) return false;`) bzw. `mapNoThrow` nutzen.

  **Umsetzung:** Die `fmt`-Chunklaenge wird gegen das Dateiende validiert, bevor 16 Bytes gemappt
  werden (`size < 16 || payload + 16 > fsize`, src/audio/AudioInfo.cpp:194 (`size <
  16 || payload + 16 > fsize`)). Der Fall steht als `fmt`-Size `0x7FFFFFFF` in
  `EmptyAndTruncatedFiles`.

- [x] **AIFF-Sample-Rate-Parsing setzt 80-Bit-`long double` der Host-Plattform voraus** (`src/audio/AudioInfo.cpp:95–110`)
  Es werden 10 Bytes (`adr[16]..adr[25]`) über `char* f = (char*)&frequency` in ein `long double` geschrieben. Auf x86-64/Linux ist `long double` 80-Bit-Extended (16 Byte Speicher) — dort funktioniert es. Auf Windows, ARM (Raspberry Pi) und FreeBSD ist `long double` meist ein normales `double` (8 Byte): `f[8]`/`f[9]` schreiben dann außerhalb des Objekts (Stack-Schaden, UB), und die Frequenz wird ohnehin falsch interpretiert. Damit ist AIFF auf Nicht-x86-Plattformen kaputt — widerspricht direkt den Plattform-Zielen (Windows/FreeBSD/Pico) aus `REFACTORING.md`.
  Behebung: 80-Bit-IEEE-754-Extended manuell und plattformunabhängig dekodieren, z. B.:
  ```cpp
  // AIFF: 1 Sign-Bit, 15 Exponent-Bits, 63 Mantissen-Bits (big endian)
  uint8_t e[10];
  memcpy(e, adr + 16, 10);
  int sign = (e[0] & 0x80) ? -1 : 1;
  int exponent = ((int)(e[0] & 0x7F) << 8) | e[1];
  uint64_t mantissa = 0;
  for (int i = 2; i < 10; i++) mantissa = (mantissa << 8) | e[i];
  double f = (double)mantissa * std::ldexp(1.0, exponent - 16383 - 63);
  info.Frequency = (uint32_t)f;
  ```

  **Umsetzung:** `readExtended80()` dekodiert IEEE-754-Extended aus 10 Bytes plattformneutral mit
  `uint64_t` und `ldexp()` (src/audio/AudioInfo.cpp:74), ohne `long double` und ohne
  Cast auf `char*`. `AiffFractionalSampleRate` prueft die Rundung von 44100,5 Hz auf
  44101.

- [x] **AIFF: `AudioSize = size - 8` kann unterlaufen; `sample offset != 0` verwirft gültige Dateien** (`src/audio/AudioInfo.cpp:115–120`)
  `if (PeekN32(adr + 8) != 0) return false;` lehnt konforme AIFF-Dateien ab, die einen Sample-Offset ungleich 0 verwenden (z. B. Samples, die nicht an einer Wortgrenze beginnen). Gleichzeitig wird der gelesene Offset nie verwendet: `info.AudioStart = p + 16` ignoriert ihn, die Audiodaten beginnen real aber erst dahinter. Bei `size < 8` unterläuft `info.AudioSize = size - 8` auf ~4 GiB und `AudioEnd` wird Unsinn.
  Behebung: Offset akzeptieren und einrechnen, Größe begrenzen:
  ```cpp
  uint32_t offset = PeekN32(adr + 8);
  if (size < 8) return false;
  info.AudioStart = p + 16 + offset;
  info.AudioSize = size - 8 - offset;   // nicht < 0
  ```

  **Umsetzung:** Der Sample-Offset wird akzeptiert und einberechnet (`dataStart = payload + 8 +
  offset`, src/audio/AudioInfo.cpp:128) und `AudioSize` auf die bis zum Dateiende
  vorhandenen Bytes begrenzt; `AiffSampleOffsetAndChunkBeyondEof` liefert AudioStart
  62 und AudioSize 8 statt rund 4 GiB.

- [ ] **Typverengung: `uint64_t` → `uint32_t` bei Dateigröße und Audiodaten-Positionen** (`src/audio/AudioInfo.cpp:65`, `:135`, `:176–179`; `include/pplib/audio/audioinfo.h:81–84`)
  `FileObject::size()` liefert `uint64_t`, `AudioInfo::FileSize` ist aber `uint32_t`; `PPL_MPEG_HEADER::start`/`end` sind `uint64_t`, `AudioStart`/`AudioEnd`/`AudioSize` sind `uint32_t`. Dateien > 4 GiB (lange WAV/AIFF-Aufnahmen, große MP3-Deals) werden stillschweigend abgeschnitten, `AudioEnd` kann vor `AudioStart` landen.
  Behebung: Felder in `AudioInfo` auf `uint64_t` umstellen (ABI-Bruch bewusst einplanen, PPLib8 ist ein Modernisierungszweig) oder zumindest mit expliziter Prüfung begrenzen.

  **Bewusst nicht umgesetzt:** Die Felder bleiben 32 Bit; die Typverengung ist an allen betroffenen
  Zuweisungen im Code vermerkt (src/audio/AudioInfo.cpp:92, :175, :244)
  und die Klassen-Doku erklaert die Grenze
  (include/pplib/audio/audioinfo.h:73: Dateien ueber 4 GiB derzeit
  nicht unterstuetzt). Die Umstellung auf `uint64_t`waere ein ABI-Bruch
  und bleibt deshalb ein Entscheidungspunkt .

## Bugs (mittel / Warnungen)

- [x] **Fehlschlag-Pfade verlassen `info` teilbefüllt** (`src/audio/AudioInfo.cpp:91`, `:147`, `:152`, `:201`)
  `IdentAIFF` setzt `Format`, `FileSize`, `Channels`, … und steigt dann mit `return false` aus (z. B. `Channels > 2`). Der Aufrufer bekommt ein Objekt mit halb gefüllten Feldern und kann nicht erkennen, ob die Werte noch gelten. Gleiches gilt für `IdentWave`/`IdentMP3`.
  Behebung: bei `false` ein lokales `AudioInfo`-Objekt verwerfen und `info` nur im Erfolgsfall zuweisen, oder `info` am Ausgang zurücksetzen.

  **Umsetzung:** Beide Public-Overloads arbeiten auf einem lokalen `AudioInfo result` und weisen
  `info = result` erst nach erfolgreichem Abschluss zu (src/audio/AudioInfo.cpp:325
  und :344). `NonAudioContentLeavesInfoUntouched` fuellt `info` vorher mit einem
  Sentinel und vergleicht nach dem Misserfolg alle 16 Felder.

- [x] **AIFF-Erkennung: `PeekN32(adr + 4) < file.size()` verwirft gültige Dateien** (`src/audio/AudioInfo.cpp:221`, `:244`)
  Das FORM-Chunk-Feld zählt die 4 Bytes nach sich selbst und liegt bei vielen Encodern exakt bei `filesize - 8`; mit Padding/Trailing-Bytes am Dateiende wird es größer als `file.size()` und die Datei als „kein AIFF“ abgelehnt. Sinnvoller wäre `<=` bzw. eine Toleranz.

  **Umsetzung:** Das FORM-Gate prueft jetzt `(uint64_t)PeekN32(adr + 4) + 8 <= fsize`
  (src/audio/AudioInfo.cpp:301), Trailing-Padding am Dateiende wird also akzeptiert.
  Ein FORM-Feld deutlich jenseits des Dateiende lehnt `AiffFormSizeBeyondEof`
  weiterhin ab, `tags_at_eof.aiff` wird dagegen als AIFF erkannt.

- [x] **AIFF-Chunkschleife: `p += size + 8` mit `size == 0` und Schleifenbedingung zu eng** (`src/audio/AudioInfo.cpp:74`, `:76`, `:125`)
  Bei `size == 0` (legal bei Padding-Chunks) werden 8 Bytes auf einmal übersprungen, wodurch der nächste Chunkname aus Datenbytes gelesen wird. Ein Chunk, der exakt am Dateiende endet, wird von `p + 8 < file.size()` nicht mehr betrachtet (`<=` bzw. `p + 8 <= file.size()`). Zusätzlich sollte gegen Endlosschleifen/Überlauf bei `size == 0xFFFFFFFF` abgesichert werden.

  **Umsetzung:** Die Schleife lautet `while (p + 8 <= fsize)` mit Wort-Padding (`if (size & 1)
  next++`) und Abbruch (`if (p >= next) break`), damit `size == 0` nicht endlos
  laeuft (src/audio/AudioInfo.cpp:141-143, analog in `IdentWave()`)). Bei `size ==
  0xFFFFFFFF` bricht der Lauf ab, weil die Payload-Reichweite nicht ausreicht
  (`AiffCommChunkBeyondEof`).

- [x] **MP3: ID3v2-Erkennung zu streng** (`src/audio/AudioInfo.cpp:207–212`)
  Akzeptiert nur die Versionsbytes `(3,0)` und `(4,0)`. ID3v2.2 (`2,x`) sowie Revisionen ungleich 0 (z. B. 2.4.1) werden übersehen, obwohl `IdentMPEG` den Tag-Offset korrekt ermittelt hat. `info.ID3v2TagStart` bleibt bei MP3 immer 0, während AIFF dort den Chunk-Start einträgt — inkonsistente Semantik desselben Feldes.

  **Umsetzung:** ID3v2 wird ab Major-Version 2 und in jeder Revision akzeptiert (`Peek8(adr + 3) >=
  2`, src/audio/AudioInfo.cpp:277); `ID3v2TagStart` traegt bei MP3 den Anfang des
  Tags ein (0), also dieselbe Semantik wie bei AIFF und WAV. Tests:
  `Mp3WithID3v2TagAtStart` und `IdentMp3CBR192WithID3`.

- [x] **WAV: `HaveID3v2Tag = false` hartkodiert** (`src/audio/AudioInfo.cpp:167`)
  WAV-Dateien können ID3-Chunks (`id3 `/`ID3 `) enthalten; `test_44kHz_tagged.wav` liegt bereits in `tests/testdata/audio/`, wird aber von keinem Test verwendet. Die Erkennung fehlt komplett.

  **Umsetzung:** WAV erkennt `id3 `- und `ID3 `-Chunks in der Chunkschleife und setzt
  `HaveID3v2Tag`/`ID3v2TagStart` (src/audio/AudioInfo.cpp:204). Da
  `test_44kHz_tagged.wav` keinen ID3v2-Chunk besitzt (nur ein Tag am Dateiende), ist
  der Fall in `WaveId3ChunkBeforeData` nachgebaut; die Datei steht zusaetzlich in der
  Format-Matrix.

- [x] **WAV: starre kanonische Layout-Annahme, keine Begrenzung auf die Dateigröße** (`src/audio/AudioInfo.cpp:154`, `:156–163`)
  Der `data`-Chunk muss unmittelbar nach `fmt` liegen (`map(fmtchunklen + 20, 8)`), `AudioStart = fmtchunklen + 28`. Dateien mit `LIST`/`fact`-Chunks dazwischen (sehr verbreitet, z. B. von LAME, Sound Forge, Streaming-Encodern) werden abgelehnt. `AudioSize` wird unclamped aus dem Header übernommen: Streaming-WAVs mit `data`-Größe `0xFFFFFFFF` ergeben `AudioEnd = AudioStart + 0xFFFFFFFF - 1` (Underflow) und völlig falsche `Samples`/`Length`.
  Behebung: Chunkschleife wie in `IdentAIFF` (mit `RIFF`/`data`-Suche) und `AudioSize = min(AudioSize, file.size() - AudioStart)`.

  **Umsetzung:** `IdentWave()` arbeitet jetzt mit einer Chunkschleife ab Offset 12
  (src/audio/AudioInfo.cpp:188ff), LIST- und id3-Chunks zwischen `fmt` und `data`
  werden uebersprungen, und `AudioSize` wird auf `fsize - AudioStart` begrenzt.
  Tests: `WaveListChunkBeforeData` (AudioStart 66) und `WaveDataChunkBeyondEof`.

- [x] **Beide `IdentAudioFile`-Overloads duplizieren die Sniffing-Logik** (`src/audio/AudioInfo.cpp:216–236` und `:239–257`)
  Zweimal identische 20 Zeilen Magie-Check-Logik; Änderungen (z. B. neue Formate, korrigierter AIFF-Gate) müssen doppelt gepflegt werden. Format-Overload sollte auf die Info-Variante zurückgreifen oder ein gemeinsamer `DetectFormat(adr)`-Helfer verwendet werden.

  **Umsetzung:** Beide Overloads nutzen denselben internen Helfer `DetectFormat()`
  (src/audio/AudioInfo.cpp:320 und :350); die doppelt gepflegten Sniffing-Zeilen sind
  entfallen. `FormatOverloadMatchesInfoOverload` fahrt beide Varianten durch ein und
  dieselbe Format-Matrix.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [x] **`AudioInfo` hat ausschließlich öffentliche Felder ohne Default Member Initializers** (`include/pplib/audio/audioinfo.h:77–92`, `src/audio/AudioInfo.cpp:41–59`)
  16 Zuweisungen im Konstruktor lassen sich in C++20 direkt als Initializer am Feld entfernen (`uint32_t FileSize{};` usw.). Das eliminiert die Fehlerquelle „neues Feld vergessen“ und macht `AudioInfo` zu einem Aggregat.

  **Umsetzung:** Alle Felder haben jetzt Default Member Initializer in
  `include/pplib/audio/audioinfo.h`; der 16-Zeilen-Konstruktor in
  src/audio/AudioInfo.cpp ist entfallen und durch `AudioInfo() = default;` ersetzt.
  `ConstructorDefaults` prueft ausdruecklich jedes einzelne Feld.

- [ ] **Unscoped Enums** (`include/pplib/audio/audioinfo.h:61–75`)
  `AudioFormat` und `ChannelMode` sollten `enum class` sein; sonst landen `UNKNOWN`, `STEREO`, `MONO` … im Namensraum der Klasse bzw. des Übersetzungstranskriptors. Achtung: breche Änderung an der API, daher in einer eigenen Modernisierungsrunde einplanen.

  **Bewusst nicht umgesetzt:** `AudioFormat` und `ChannelMode` bleiben unscoped, weil die Werte
  1-Byte-struct-Felder belegen und `enum class` Quell- wie ABI-
  Kompatibilitaet brechen wuerde. Die Umstellung bleibt ein
  Entscheidungspunkt; die Werte selbst sind inzwischen voll
  dokumentiert.

- [ ] **`WAVEHEADER` gehört nicht in `audioinfo.h`** (`include/pplib/audio/audioinfo.h:42–54`)
  Die Struktur wird ausschließlich von `readWaveHeader` in `include/pplib/audio/decoder.h:190` verwendet. Sie sollte dorthin (oder in einen internen Header) wandern; öffentliche Header sollten nur die öffentliche API enthalten.

  **Teilweise umgesetzt:** Die nie definierte private Deklaration `readWaveHeader(FileObject&,
  WAVEHEADER&)` ist aus `AudioDecoder_Wave` in
  include/pplib/audio/decoder.h entfernt, womit die Struktur dort nicht
  mehr referenziert wird. Der Umzug von `WAVEHEADER` nach decoder.h
  unterbleibt vorerst, weil der Typ oeffentlich aus `pplib` exportiert ist
  (API-Aenderung, Entscheidungspunkt).

- [x] **`UnsupportedAudioFormatException` wird deklariert, aber nie geworfen** (`include/pplib/audio/audioinfo.h:40`)
  `IdentAudioFile` liefert `false`/`UNKNOWN`. Wenn die API bei unbekanntem Format werfen soll, muss das implementiert und dokumentiert werden; wenn nicht, sollte die Exception-Klasse aus dem öffentlichen Header entfernt oder klar zugeordnet werden (Decodern).

  **Umsetzung:** Kein Wurf in `IdentAudioFile()`, aber eine klare Zuordnung: der Doxygen-Block ueber
  `UnsupportedAudioFormatException` (include/pplib/audio/audioinfo.h:30ff) nennt die
  werfenden Stellen - die Decoder und der Tag-Zugriff in src/audio/ID3Tag.cpp - und
  verweist darauf, dass die Identifikation false bzw. UNKNOWN liefert.

- [x] **Konstruktor setzt `Mode = STEREO` als Default** (`src/audio/AudioInfo.cpp:58`)
  Ein unbestimmtes Objekt als „Stereo“ zu initialisieren ist irreführend, zumal `STEREO` den Zählerwert 0 hat und damit auch `Mode{}` abdeckt. Besser explizit ein `UNKNOWN`-Wert im Enum oder dokumentierter Default.

  **Umsetzung:** Der Default bleibt `STEREO`, ist aber jetzt als Default ausgewiesen (Feld `Mode` in
  audioinfo.h) und wird von `ConstructorDefaults` eingefroren; die bestehenden
  Aufrufer in src/audio/Decoder.cpp lesen den Wert 0 als Stereo. Ein zusaetzlicher
  `UNKNOWN`-Wert bleibt ein Entscheidungspunkt .

- [x] **Chunk-Erkennung über Hex-Konstanten** (`src/audio/AudioInfo.cpp:81`, `:114`, `:121`)
  `PeekN32(adr) == 0x434f4d4d` ist schwer lesbar. C++20-Lösung: `std::string_view(adr, 4) == "COMM"` (oder `std::memcmp(adr, "COMM", 4) == 0`) — selbe Laufzeit, deutlich wartbarer.

  **Umsetzung:** Chunk-Namen werden ueber den Helfer `fourcc(p, "MARK")` verglichen
  (src/audio/AudioInfo.cpp:48), ebenso `"COMM"`, `"SSND"`, `"FORM"`, `"AIFF"`,
  `"RIFF"`, `"WAVE"`, `"fmt "`, `"data"`, `"id3 "` und `"OggS"`; Hex-Konstanten sind
  keine mehr vorhanden.

- [x] **`long double` für reine Ganzzahl-/Exponent-Arithmetik** (`src/audio/AudioInfo.cpp:95–109`)
  Unabhängig vom Plattform-Bug (siehe kritischer Befund) ist die Byte-Shuffle-Technik mit C-Style-Cast auf `char*` streng genommen ein Verstoß gegen Strict-Aliasing. Plattformneutrale Integer-Dekodierung (s. o.) löst beides.

  **Umsetzung:** Wie beim kritischen Sample-Rate-Befund ueblich: `readExtended80()` dekodiert
  ausschliesslich in lokale `uint64_t`- und `int`-Werte, der C-Style-Cast auf `char*`
  und damit der Strict-Aliasing-Verstoss sind weg.

## Dokumentation & Doxygen

- [x] **`AudioInfo` und alle Felder ohne Doxygen** (`include/pplib/audio/audioinfo.h:56–93`)
  Ziel aus `REFACTORING.md` ist IntelliSense-taugliche Doku direkt im Header. Mindestens nötig: Klassenbeschreibung, Bedeutung und Einheiten je Feld (`Length` in ms ist nur als Inline-Kommentar vorhanden, `Bitrate` in kbit/s gar nicht, `AudioStart`/`AudioEnd` inklusiv/exklusiv unklar, `ID3v2TagStart` nur für AIFF gesetzt).

  **Umsetzung:** Klassen-, Enum- und Felddoku sind im Header vorhanden, inklusive Einheiten
  (`Length` in ms, `Bitrate` in kbit/s, `Frequency` in Hz) und der Klarstellung, dass
  `AudioEnd` den letzten Byte der Audiodaten inklusiv nennt und `ID3v2TagStart` auf
  den Tag-Anfang zeigt. Alle 16 Felder tragen einen `//!<`-Kommentar.

- [x] **`IdentAudioFile` undeklarierte Exception-Semantik** (`include/pplib/audio/audioinfo.h:95–96`)
  Der Header sagt nichts darüber, dass `IdentAudioFile` werfen kann (`FileNotOpenException` bei geschlossener Datei, `OverflowException` über `map`). Für Aufrufer ist das entscheidend — entweder Doku oder (besser im Sinne von `REFACTORING.md`: weniger Exceptions) die Exception-Pfade auf `false`/`UNKNOWN` umstellen.

  **Umsetzung:** Beide Deklarationen dokumentieren `@exception pplib::FileNotOpenException`
  (audioinfo.h:151 und :165) sowie dass beschaedigte Header ausdruecklich keine
  Exception erzeugen; Map-Ausnahmen sind nach dem Umbau auf `mapRegion()` nicht mehr
  erreichbar. Die Doku der Exception-Klasse ordnet das Werfen den Decodern zu.

- [x] **Enum-Werte ohne Beschreibung** (`include/pplib/audio/audioinfo.h:61–75`)
  `AudioFormat`/`ChannelMode` brauchen `\brief`-Einträge inkl. Hinweis, dass `ChannelMode` die MPEG-Modi abbildet (nicht einfach „Mono/Stereo“).

  **Umsetzung:** Alle Werte beider Enums haben `//!<`-Beschreibungen, bei `ChannelMode` inklusive
  dem Hinweis, dass `DUAL_CHANNEL` und `JOINT_STEREO` die MPEG-Modi widerspiegeln.

- [x] **`WAVEHEADER`-Felder undokumentiert** (`include/pplib/audio/audioinfo.h:45–53`)
  Nur die Struktur selbst hat einen `\brief`; die Einheiten (`bytes` vs. `numSamples`, `bytespersecond`) bleiben unklar.

  **Umsetzung:** Alle neun Felder von `WAVEHEADER` sind mit `//!<` und Einheit kommentiert, etwa
  `bytes` = Audiodaten in Bytes, `numSamples` = Anzahl Samples, `bytespersecond` =
  Datenrate in Bytes pro Sekunde.

## Testqualität

- [x] **Keine negativen Tests vorhanden** (`tests/src/audio/audioinfo.cpp:59–219`)
  Alle 9 Tests arbeiten mit intakten Beispieldateien. Es fehlt komplett: Nicht-Audio-Datei, leere Datei, Datei < 24 Bytes, abgeschnittene/kaputte Header, geschlossene Datei. Genau die Pfade, die laut Befund 1–4 zu Exceptions führen, sind ungetestet. Empfehlung: mit `pplib::MemFile` (Include ist bereits vorhanden, wird aber nie genutzt) künstliche Byte-Konstrukte testen — das macht die Tests plattformunabhängig und schnell:
  ```cpp
  TEST_F(AudioInfoTest, IdentAudioFileRejectsTinyFile) {
      const char data[] = "RIFF";
      pplib::MemFile file(data, sizeof(data) - 1);
      pplib::AudioInfo info;
      EXPECT_FALSE(pplib::IdentAudioFile(file, info));
      EXPECT_EQ(pplib::AudioInfo::UNKNOWN, pplib::IdentAudioFile(file));
  }
  ```

  **Umsetzung:** Negative Tests existieren mit `pplib::MemFile`:
  `NonAudioContentLeavesInfoUntouched` (2048 Byte Text), `EmptyAndTruncatedFiles`
  (leere Datei, angeschnittener 20-Byte-Rumpf, `fmt`-Size `0x7FFFFFFF`),
  `AiffFormSizeBeyondEof`, `AiffCommChunkBeyondEof`, `WaveDataChunkBeyondEof` und
  `AiffSampleOffsetAndChunkBeyondEof`; in der Matrix steht ausserdem `cover.jpg` als
  UNKNOWN-Fall.

- [x] **`ConstructorAudioInfo` prüft nichts** (`tests/src/audio/audioinfo.cpp:59–61`)
  Lediglich `ASSERT_NO_THROW`. Es wird kein einziger Defaultwert verifiziert — ein weggelassenes Feld im Konstruktor fiele nicht auf. Besser: alle Felder gegen erwartete Defaults prüfen (oder nach Umstellung auf Default Member Initializers: `EXPECT_EQ(0u, info.FileSize)` usw.).

  **Umsetzung:** Der inhaltsleere Test `ConstructorAudioInfo` ist entfernt; `ConstructorDefaults`
  prueft samtliche Defaults (Format UNKNOWN, alle Zahlen 0, `Mode` STEREO, beide
  Flags false).

- [x] **Format-Overload nur für zwei Fälle getestet** (`tests/src/audio/audioinfo.cpp:203–205`)
  `IdentAudioFile(file)` (nur Format) wird ausschließlich für WAV (negativ) und OGG (positiv) aufgerufen. MP3, AIFF, WAV-positiv und `UNKNOWN` über diesen Overload sind ungetestet — obwohl es ein öffentlicher API-Zweig mit duplizierter Logik ist (siehe Duplikats-Befund).

  **Umsetzung:** `FormatOverloadMatchesInfoOverload` ruft den Format-Overload fuer alle Eintraege
  der Matrix (MP3, AIFF, WAV einschliesslich getaggter Varianten, OGG) sowie
  `cover.jpg` (UNKNOWN) und `test_48kHz.opus` auf und vergleicht das Ergebnis mit dem
  Info-Overload.

- [x] **Positions-/Größenfelder nur im WAV-Test geprüft** (`tests/src/audio/audioinfo.cpp:175–197`)
  `FileSize`, `AudioStart`, `AudioEnd`, `AudioSize` werden nur in `IdentWaveWithoutID3` verglichen. Bei MP3 wird `Length` nie geprüft, bei OGG und AIFF keine der Positionsfelder. Damit bleiben Rechenfehler in `IdentMP3`/`IdentAIFF`/`GetVorbisAudioInfo` unentdeckt.

  **Umsetzung:** `FileSize`, `AudioStart`, `AudioEnd` und `AudioSize` sind jetzt in den bestehenden
  Tests verankert: `IdentMp3CBR192WithoutID3` (95920/0/95919/95920),
  `IdentAiffWithoutID3` (AudioStart 54), `IdentOggVorbis` (8060/90767/82708) und
  `IdentWaveWithoutID3`; bei MP3 kommen `Length` und `Mode` hinzu
  (`Mp3ModeFieldSelectsChannelMode`).

- [x] **Vorhandene Testdaten ungenutzt, Formate fehlen** (`tests/testdata/audio/`)
  `test_44kHz_tagged.wav` (ID3 in WAV), `test_48kHz.opus`, `test_44kHz_taggedWithCover.aiff` und `cover.jpg` werden von keinem Test verwendet. Es fehlen: 8-Bit-WAV, Mono-WAV, 48-kHz-WAV, AIFF-C, WAV mit `LIST`/`fact`-Chunk, MP3 mit ID3v2.2.

  **Umsetzung:** Die Matrix nutzt elf Dateien, darunter `test_44kHz_tagged.wav`,
  `test_44kHz_taggedWithCover.aiff`, `tags_at_eof.aiff` (eigener Test
  `AiffWithTagsAtEndOfFile`), `test_48kHz.opus` (`OpusInOggContainerIsNotSupported`)
  und `cover.jpg` (UNKNOWN). Fehlende Formate werden synthetisch abgedeckt: 8-Bit-
  Mono-WAV bei 48 kHz mit 40-Byte-fmt-Chunk (`Wave8BitMono48kHzExtensibleFmt`),
  20-Bit-Mono, LIST-Chunk und die vier MPEG-Modi. AIFF-C fehlt weiterhin, da es
  dafuer keine Beispieldatei gibt.

- [x] **Unbenutzte Includes im Test** (`tests/src/audio/audioinfo.cpp`, Kopfzeilen)
  `memfile.h`, `iconv.h`, `string.h` werden nicht gebraucht (bzw. `memfile.h` sollte gebraucht werden, siehe negativer Test oben).

  **Umsetzung:** `iconv.h` und `id3tag.h` sind aus den Test-Includes entfernt, neu sind `<cstdio>`,
  `<string>` und `pplib/audio/audioinfo.h`, das vorher nur transitiv ueber `id3tag.h`
  kam. `memfile.h` wird nun von allen Kopfzeilen-Konstrukten der Negativtests
  benignotigt.

- [x] **`file.open()` wird ohne Fehlerbehandlung gerufen** (`tests/src/audio/audioinfo.cpp:67`, `86`, `104`, `122`, `140`, `159`, `178`, `202`, `204`)
  `File::open` wirft bei fehlender Datei; bricht ein Testpfad ab, ist die Fehlermeldung wenig aussagekräftig. Ein `ASSERT_NO_THROW`/Helper mit Dateipfad im Text verbessert die Diagnose. (Doppelt `open()` ohne `close()` ist dagegen unkritisch — `File::open` schließt intern vorher.)

  **Umsetzung:** Der Fixture-Helfer `OpenTestFile()` (tests/src/audio/audioinfo.cpp:65) sichert
  `open()` mit `ASSERT_NO_THROW` und prueft `isOpen()`; beide Meldungen enthalten den
  kompletten Pfad. Saemtliche Dateizugriffe der Suite laufen darueber, die doppelten
  `open()`-Aufrufe ohne `close()` sind entfallen.

- [x] **Gewicht der Tests einseitig** (`tests/src/audio/audioinfo.cpp`)
  4 MP3-Tests, 2 AIFF, 1 WAV, 1 OGG, 1 Konstruktor. Der komplexeste Parser (`IdentAIFF` mit Chunkschleife) hat nur zwei Happy-Path-Tests; die fehleranfälligsten Codezeilen (Chunkschleife, `long double`, SSND-Offset) haben 0 % Abdeckung.

  **Umsetzung:** Die Suite waechst von 9 auf 25 Tests: vier AIFF-Grenzfaelle, vier WAV-Faelle (LIST-
  Chunk, id3-Chunk, 20 Bit, 8 Bit/48 kHz), zwei MP3-Faelle (Tag am Anfang, vier Modi)
  und die Matrix ueber beide Overloads. Coverage von src/audio/AudioInfo.cpp: 98,1 %
  Lines, 75,9 % Branches.

## Befunde in Drittdateien

- [x] **`GetVorbisAudioInfo` setzt `FileSize` nie und verwendet andere Konvention** (`src/audio/AudioDecoder_Ogg.cpp:89–109`)
  `info.FileSize` bleibt auf Konstruktorwert 0, obwohl `AudioInfo::FileSize` für WAV/AIFF/MP3 gesetzt wird. `info.AudioSize = vf.end - vf.offset` ist exklusiv berechnet, während `IdentWave`/`IdentAIFF`/`IdentMP3` `AudioEnd` inklusiv führen. `Length` wird ohne 64-Bit-Zwischenwert berechnet (Überlauf bei langen Dateien). Test `IdentOggVorbis` prüft die Felder bewusst nicht — die Lücke fällt also nicht auf.

  **Umsetzung:** `GetVorbisAudioInfo()` in src/audio/AudioDecoder_Ogg.cpp:89 setzt jetzt `FileSize =
  file.size()`, fuehrt `AudioEnd` wie MP3 inklusiv (`if (vf.end > vf.offset)`) und
  rechnet `Length`/`Bitrate` in 64 Bit mit Guard gegen `Frequency == 0`.
  `IdentOggVorbis` prueft FileSize 90768, AudioStart 8060, AudioEnd 90767, AudioSize
  82708.

- [x] **Exception-Vertrag von `FileObject::map` wird durchgängig missachtet** (`include/pplib/core/fileobject.h:604–607`, `src/core/File.cpp:750–770`)
  Der Vertrag ist exception-basiert: `File::map` wirft bei Bereichsüberschreitung `OverflowException` und liefert nur bei leerer Datei bzw. 0 Bytes `nullptr` (`File.cpp:753`, `:769`; Doku in `fileobject.h` ist inzwischen korrekt). Die in `AudioInfo.cpp` (und teils in `AudioDecoder_Ogg.cpp`) allgegenwärtigen `if (!buffer)`-Guards greifen daher nie — jeder kaputte Header wird zur Exception. Entweder alle Aufrufer auf den Exception-Pfad anpassen oder eine durchgehende `mapNoThrow`-Variante (Muster: `src/audio/Mp3.cpp:135`) in `FileObject` anbieten und dort die nullptr-Prüfung wieder verlässlich machen.

  **Umsetzung:** Fuer src/audio/AudioInfo.cpp ist der exception-basierte Vertrag erfuellt: Jedes
  Mapping geht durch `mapRegion()` (src/audio/AudioInfo.cpp:60) mit vorheriger
  Reichweitenpruefung, die `nullptr`-Guards sind ersatzlos entfallen. Belegt durch
  `EmptyAndTruncatedFiles`, `AiffCommChunkBeyondEof`, `WaveDataChunkBeyondEof` und
  `Wave8BitMono48kHzExtensibleFmt`.

- [ ] **`mapNoThrow` ist statisch in `Mp3.cpp` dupliziert** (`src/audio/Mp3.cpp:135`)
  Sobald `AudioInfo.cpp` denselben Helfer braucht, sollte er als öffentliche Methode/Hilfsfunktion (z. B. `FileObject::mapNoThrow`) zentralisiert werden, statt ihn ein drittes Mal zu kopieren.

  **Bewusst nicht umgesetzt:** src/audio/AudioInfo.cpp nutzt den lokalen Helfer `mapRegion()`,
  `mapNoThrow()` in src/audio/Mp3.cpp:134 bleibt unangetastet. Eine
  gemeinsame Methode in `FileObject` (oder gleich `File::mapAtMost()`)
  ware eine oeffentliche API-Erweiterung und bleibt wegen der
  Speicherziele auf Mikrocontrollern eine Entscheidung.

- [x] **`WAVEHEADER`-Nutzung in `decoder.h`** (`include/pplib/audio/decoder.h:190`)
  `readWaveHeader` verwendet die in `audioinfo.h` definierte Struktur; bei Umzug der Struktur (siehe Smells) muss dieser Include-Pfad mitgezogen werden.

  **Umsetzung:** Die Struktur bleibt in `audioinfo.h`; da die ungenutzte Deklaration
  `readWaveHeader()` aus `AudioDecoder_Wave` (include/pplib/audio/decoder.h) entfernt
  ist, referenziert decoder.h `WAVEHEADER` derzeit gar nicht, sodass sich der
  Include-Pfad aktuell nicht stellt.

- [ ] **Verwandter offener Befund** (`codereview/todo/mp3-review.md:218–219`, `:235`)
  Bereits dort ist das ungesicherte `map(0, 24)` in `IdentAudioFile` dokumentiert. Empfehlung: Befund dort schließen und zentral in diesem Review abarbeiten, damit die Fixup-Arbeit nur einmal erfolgt.

  **Bewusst nicht umgesetzt:** Der in src/audio/AudioInfo.cpp gemeldete Teil ist erledigt
  (Groessenpruefung vor dem Mappen, Gate `fsize < 24` in `DetectFormat()`). Die
  entsprechende Stelle in src/audio/Mp3.cpp bleibt der eigenen Runde
  `codereview/todo/mp3-review.md` ueberlassen, damit derselbe Fehler
  nicht zweimal bearbeitet und doppelt gezaehlt wird.
