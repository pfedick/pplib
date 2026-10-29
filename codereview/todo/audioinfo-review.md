# AudioInfo Review – Befunde

Review vom 2026-10-21, Scope: `include/pplib/audio/audioinfo.h` (99 Zeilen), `src/audio/AudioInfo.cpp` (262 Zeilen), `tests/src/audio/audioinfo.cpp` (221 Zeilen).
Öffentliche API: `pplib::AudioInfo`, `pplib::IdentAudioFile(FileObject&, AudioInfo&)`, `pplib::IdentAudioFile(FileObject&)`, `pplib::WAVEHEADER`, `pplib::UnsupportedAudioFormatException`
Review done by: Copilot (Copilot SDK in VS Code)

Teststand zum Review-Zeitpunkt: `make build_tests` erfolgreich, `test_all --gtest_filter=AudioInfoTest*` → 9 Tests, alle grün.

Alle kritischen Befunde 1–4 wurden mit einem separaten Probeprogramm gegen die Debug-Bibliothek verifiziert (keine Änderung am Projektcode). Reproduktion jeweils angegeben.

## Bugs (kritisch)

- [ ] **`IdentAudioFile` wirft `OverflowException` bei Dateien < 24 Bytes** (`src/audio/AudioInfo.cpp:218` bzw. `:241`)
  `file.map(0, 24)` ist nicht abgesichert. `File::map` wirft laut Implementierung (`src/core/File.cpp:769`) eine `OverflowException`, wenn `position + bytes > filesize`; `nullptr` gibt es nur bei leerer Datei oder Mapping-Größe 0 (`src/core/File.cpp:753`). Eine 4-Byte-Datei führt also zu einer ungefangenen Exception statt zu `false`. Die Doku in `include/pplib/core/fileobject.h:604–607` beschreibt dieses Verhalten inzwischen korrekt — die `if (!adr)`-Guards der Aufrufer greifen bei Bereichsüberschreitung nevertheless nicht.
  Verifiziert: 4-Byte-Datei → `EXCEPTION: OverflowException`.
  Bereits offener Befund in `codereview/todo/mp3-review.md` (Zeilen 218–219, 235) — hier mitvermerkt, damit beide Stellen gemeinsam behoben werden.
  Behebung: Größen-Check vor dem Mappen, oder helpers à la `mapNoThrow` (existiert bereits statisch in `src/audio/Mp3.cpp:135`) verwenden:
  ```cpp
  if (file.size() < 24) return false;
  const char* adr = file.map(0, 24);
  if (!adr) return false;
  ```

  UPDATE: Verhalten von File::map hat sich seit diesem Review geändert! Es wirft nun konsequent immer `OutOfBoundsException` oder andere Exceptions, und gibt niemals `nullptr` zurück.

- [ ] **`IdentAIFF`: Ergebnis von `file.map(p, 32)` und `file.map(p, 16)` ungeprüft** (`src/audio/AudioInfo.cpp:84`, `:115`)
  Beide `map`-Aufrufe werden sofort dereferenziert (`PeekN16(adr + 8)` bzw. `PeekN32(adr + 8)`). Ein kaputter AIFF-Chunk (z. B. COMM-Chunk-Size `0x7FFFFFFF`) lässt `p += size + 8` über das Dateiende laufen; die Schleifenbedingung `p + 8 < file.size()` prüft nur 8 Bytes, gemappt werden aber 32 bzw. 16 Bytes → `OverflowException`, oder (bei `filesize == 0`) `nullptr`-Deref.
  Verifiziert: AIFF-Kopf mit COMM-Size `0x7FFFFFFF` → `EXCEPTION: OverflowException`.
  Behebung:
  ```cpp
  adr = file.map(p, 32);
  if (!adr) return false;   // bzw. break, je nach gewünschter Semantik
  ```
  Zusätzlich sollte `size` gegen `file.size()` plausibilisiert werden, bevor `p += size + 8` gerechnet wird.

- [ ] **`IdentWave`: `file.map(fmtchunklen + 20, 8)` wirft bei manipulierter `fmtchunklen`** (`src/audio/AudioInfo.cpp:154`)
  Die Prüfung `if (!buffer) return false;` greift nicht, weil `map` bei Bereichsüberschreitung wirft statt `nullptr` zu liefern. Eine WAV-Datei mit `fmt chunk size = 0x7FFFFFFF` (Format-Tag PCM, 16 Bit) bringt `IdentAudioFile` zur Exception.
  Verifiziert: präparierte 36-Byte-WAV → `EXCEPTION: OverflowException`.
  Behebung: `fmtchunklen` gegen `file.size()` validieren (z. B. `if (fmtchunklen > file.size()) return false;`) bzw. `mapNoThrow` nutzen.

- [ ] **AIFF-Sample-Rate-Parsing setzt 80-Bit-`long double` der Host-Plattform voraus** (`src/audio/AudioInfo.cpp:95–110`)
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

- [ ] **AIFF: `AudioSize = size - 8` kann unterlaufen; `sample offset != 0` verwirft gültige Dateien** (`src/audio/AudioInfo.cpp:115–120`)
  `if (PeekN32(adr + 8) != 0) return false;` lehnt konforme AIFF-Dateien ab, die einen Sample-Offset ungleich 0 verwenden (z. B. Samples, die nicht an einer Wortgrenze beginnen). Gleichzeitig wird der gelesene Offset nie verwendet: `info.AudioStart = p + 16` ignoriert ihn, die Audiodaten beginnen real aber erst dahinter. Bei `size < 8` unterläuft `info.AudioSize = size - 8` auf ~4 GiB und `AudioEnd` wird Unsinn.
  Behebung: Offset akzeptieren und einrechnen, Größe begrenzen:
  ```cpp
  uint32_t offset = PeekN32(adr + 8);
  if (size < 8) return false;
  info.AudioStart = p + 16 + offset;
  info.AudioSize = size - 8 - offset;   // nicht < 0
  ```

- [ ] **Typverengung: `uint64_t` → `uint32_t` bei Dateigröße und Audiodaten-Positionen** (`src/audio/AudioInfo.cpp:65`, `:135`, `:176–179`; `include/pplib/audio/audioinfo.h:81–84`)
  `FileObject::size()` liefert `uint64_t`, `AudioInfo::FileSize` ist aber `uint32_t`; `PPL_MPEG_HEADER::start`/`end` sind `uint64_t`, `AudioStart`/`AudioEnd`/`AudioSize` sind `uint32_t`. Dateien > 4 GiB (lange WAV/AIFF-Aufnahmen, große MP3-Deals) werden stillschweigend abgeschnitten, `AudioEnd` kann vor `AudioStart` landen.
  Behebung: Felder in `AudioInfo` auf `uint64_t` umstellen (ABI-Bruch bewusst einplanen, PPLib8 ist ein Modernisierungszweig) oder zumindest mit expliziter Prüfung begrenzen.

## Bugs (mittel / Warnungen)

- [ ] **Fehlschlag-Pfade verlassen `info` teilbefüllt** (`src/audio/AudioInfo.cpp:91`, `:147`, `:152`, `:201`)
  `IdentAIFF` setzt `Format`, `FileSize`, `Channels`, … und steigt dann mit `return false` aus (z. B. `Channels > 2`). Der Aufrufer bekommt ein Objekt mit halb gefüllten Feldern und kann nicht erkennen, ob die Werte noch gelten. Gleiches gilt für `IdentWave`/`IdentMP3`.
  Behebung: bei `false` ein lokales `AudioInfo`-Objekt verwerfen und `info` nur im Erfolgsfall zuweisen, oder `info` am Ausgang zurücksetzen.

- [ ] **AIFF-Erkennung: `PeekN32(adr + 4) < file.size()` verwirft gültige Dateien** (`src/audio/AudioInfo.cpp:221`, `:244`)
  Das FORM-Chunk-Feld zählt die 4 Bytes nach sich selbst und liegt bei vielen Encodern exakt bei `filesize - 8`; mit Padding/Trailing-Bytes am Dateiende wird es größer als `file.size()` und die Datei als „kein AIFF“ abgelehnt. Sinnvoller wäre `<=` bzw. eine Toleranz.

- [ ] **AIFF-Chunkschleife: `p += size + 8` mit `size == 0` und Schleifenbedingung zu eng** (`src/audio/AudioInfo.cpp:74`, `:76`, `:125`)
  Bei `size == 0` (legal bei Padding-Chunks) werden 8 Bytes auf einmal übersprungen, wodurch der nächste Chunkname aus Datenbytes gelesen wird. Ein Chunk, der exakt am Dateiende endet, wird von `p + 8 < file.size()` nicht mehr betrachtet (`<=` bzw. `p + 8 <= file.size()`). Zusätzlich sollte gegen Endlosschleifen/Überlauf bei `size == 0xFFFFFFFF` abgesichert werden.

- [ ] **MP3: ID3v2-Erkennung zu streng** (`src/audio/AudioInfo.cpp:207–212`)
  Akzeptiert nur die Versionsbytes `(3,0)` und `(4,0)`. ID3v2.2 (`2,x`) sowie Revisionen ungleich 0 (z. B. 2.4.1) werden übersehen, obwohl `IdentMPEG` den Tag-Offset korrekt ermittelt hat. `info.ID3v2TagStart` bleibt bei MP3 immer 0, während AIFF dort den Chunk-Start einträgt — inkonsistente Semantik desselben Feldes.

- [ ] **WAV: `HaveID3v2Tag = false` hartkodiert** (`src/audio/AudioInfo.cpp:167`)
  WAV-Dateien können ID3-Chunks (`id3 `/`ID3 `) enthalten; `test_44kHz_tagged.wav` liegt bereits in `tests/testdata/audio/`, wird aber von keinem Test verwendet. Die Erkennung fehlt komplett.

- [ ] **WAV: starre kanonische Layout-Annahme, keine Begrenzung auf die Dateigröße** (`src/audio/AudioInfo.cpp:154`, `:156–163`)
  Der `data`-Chunk muss unmittelbar nach `fmt` liegen (`map(fmtchunklen + 20, 8)`), `AudioStart = fmtchunklen + 28`. Dateien mit `LIST`/`fact`-Chunks dazwischen (sehr verbreitet, z. B. von LAME, Sound Forge, Streaming-Encodern) werden abgelehnt. `AudioSize` wird unclamped aus dem Header übernommen: Streaming-WAVs mit `data`-Größe `0xFFFFFFFF` ergeben `AudioEnd = AudioStart + 0xFFFFFFFF - 1` (Underflow) und völlig falsche `Samples`/`Length`.
  Behebung: Chunkschleife wie in `IdentAIFF` (mit `RIFF`/`data`-Suche) und `AudioSize = min(AudioSize, file.size() - AudioStart)`.

- [ ] **Beide `IdentAudioFile`-Overloads duplizieren die Sniffing-Logik** (`src/audio/AudioInfo.cpp:216–236` und `:239–257`)
  Zweimal identische 20 Zeilen Magie-Check-Logik; Änderungen (z. B. neue Formate, korrigierter AIFF-Gate) müssen doppelt gepflegt werden. Format-Overload sollte auf die Info-Variante zurückgreifen oder ein gemeinsamer `DetectFormat(adr)`-Helfer verwendet werden.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **`AudioInfo` hat ausschließlich öffentliche Felder ohne Default Member Initializers** (`include/pplib/audio/audioinfo.h:77–92`, `src/audio/AudioInfo.cpp:41–59`)
  16 Zuweisungen im Konstruktor lassen sich in C++20 direkt als Initializer am Feld entfernen (`uint32_t FileSize{};` usw.). Das eliminiert die Fehlerquelle „neues Feld vergessen“ und macht `AudioInfo` zu einem Aggregat.

- [ ] **Unscoped Enums** (`include/pplib/audio/audioinfo.h:61–75`)
  `AudioFormat` und `ChannelMode` sollten `enum class` sein; sonst landen `UNKNOWN`, `STEREO`, `MONO` … im Namensraum der Klasse bzw. des Übersetzungstranskriptors. Achtung: breche Änderung an der API, daher in einer eigenen Modernisierungsrunde einplanen.

- [ ] **`WAVEHEADER` gehört nicht in `audioinfo.h`** (`include/pplib/audio/audioinfo.h:42–54`)
  Die Struktur wird ausschließlich von `readWaveHeader` in `include/pplib/audio/decoder.h:190` verwendet. Sie sollte dorthin (oder in einen internen Header) wandern; öffentliche Header sollten nur die öffentliche API enthalten.

- [ ] **`UnsupportedAudioFormatException` wird deklariert, aber nie geworfen** (`include/pplib/audio/audioinfo.h:40`)
  `IdentAudioFile` liefert `false`/`UNKNOWN`. Wenn die API bei unbekanntem Format werfen soll, muss das implementiert und dokumentiert werden; wenn nicht, sollte die Exception-Klasse aus dem öffentlichen Header entfernt oder klar zugeordnet werden (Decodern).

- [ ] **Konstruktor setzt `Mode = STEREO` als Default** (`src/audio/AudioInfo.cpp:58`)
  Ein unbestimmtes Objekt als „Stereo“ zu initialisieren ist irreführend, zumal `STEREO` den Zählerwert 0 hat und damit auch `Mode{}` abdeckt. Besser explizit ein `UNKNOWN`-Wert im Enum oder dokumentierter Default.

- [ ] **Chunk-Erkennung über Hex-Konstanten** (`src/audio/AudioInfo.cpp:81`, `:114`, `:121`)
  `PeekN32(adr) == 0x434f4d4d` ist schwer lesbar. C++20-Lösung: `std::string_view(adr, 4) == "COMM"` (oder `std::memcmp(adr, "COMM", 4) == 0`) — selbe Laufzeit, deutlich wartbarer.

- [ ] **`long double` für reine Ganzzahl-/Exponent-Arithmetik** (`src/audio/AudioInfo.cpp:95–109`)
  Unabhängig vom Plattform-Bug (siehe kritischer Befund) ist die Byte-Shuffle-Technik mit C-Style-Cast auf `char*` streng genommen ein Verstoß gegen Strict-Aliasing. Plattformneutrale Integer-Dekodierung (s. o.) löst beides.

## Dokumentation & Doxygen

- [ ] **`AudioInfo` und alle Felder ohne Doxygen** (`include/pplib/audio/audioinfo.h:56–93`)
  Ziel aus `REFACTORING.md` ist IntelliSense-taugliche Doku direkt im Header. Mindestens nötig: Klassenbeschreibung, Bedeutung und Einheiten je Feld (`Length` in ms ist nur als Inline-Kommentar vorhanden, `Bitrate` in kbit/s gar nicht, `AudioStart`/`AudioEnd` inklusiv/exklusiv unklar, `ID3v2TagStart` nur für AIFF gesetzt).

- [ ] **`IdentAudioFile` undeklarierte Exception-Semantik** (`include/pplib/audio/audioinfo.h:95–96`)
  Der Header sagt nichts darüber, dass `IdentAudioFile` werfen kann (`FileNotOpenException` bei geschlossener Datei, `OverflowException` über `map`). Für Aufrufer ist das entscheidend — entweder Doku oder (besser im Sinne von `REFACTORING.md`: weniger Exceptions) die Exception-Pfade auf `false`/`UNKNOWN` umstellen.

- [ ] **Enum-Werte ohne Beschreibung** (`include/pplib/audio/audioinfo.h:61–75`)
  `AudioFormat`/`ChannelMode` brauchen `\brief`-Einträge inkl. Hinweis, dass `ChannelMode` die MPEG-Modi abbildet (nicht einfach „Mono/Stereo“).

- [ ] **`WAVEHEADER`-Felder undokumentiert** (`include/pplib/audio/audioinfo.h:45–53`)
  Nur die Struktur selbst hat einen `\brief`; die Einheiten (`bytes` vs. `numSamples`, `bytespersecond`) bleiben unklar.

## Testqualität

- [ ] **Keine negativen Tests vorhanden** (`tests/src/audio/audioinfo.cpp:59–219`)
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

- [ ] **`ConstructorAudioInfo` prüft nichts** (`tests/src/audio/audioinfo.cpp:59–61`)
  Lediglich `ASSERT_NO_THROW`. Es wird kein einziger Defaultwert verifiziert — ein weggelassenes Feld im Konstruktor fiele nicht auf. Besser: alle Felder gegen erwartete Defaults prüfen (oder nach Umstellung auf Default Member Initializers: `EXPECT_EQ(0u, info.FileSize)` usw.).

- [ ] **Format-Overload nur für zwei Fälle getestet** (`tests/src/audio/audioinfo.cpp:203–205`)
  `IdentAudioFile(file)` (nur Format) wird ausschließlich für WAV (negativ) und OGG (positiv) aufgerufen. MP3, AIFF, WAV-positiv und `UNKNOWN` über diesen Overload sind ungetestet — obwohl es ein öffentlicher API-Zweig mit duplizierter Logik ist (siehe Duplikats-Befund).

- [ ] **Positions-/Größenfelder nur im WAV-Test geprüft** (`tests/src/audio/audioinfo.cpp:175–197`)
  `FileSize`, `AudioStart`, `AudioEnd`, `AudioSize` werden nur in `IdentWaveWithoutID3` verglichen. Bei MP3 wird `Length` nie geprüft, bei OGG und AIFF keine der Positionsfelder. Damit bleiben Rechenfehler in `IdentMP3`/`IdentAIFF`/`GetVorbisAudioInfo` unentdeckt.

- [ ] **Vorhandene Testdaten ungenutzt, Formate fehlen** (`tests/testdata/audio/`)
  `test_44kHz_tagged.wav` (ID3 in WAV), `test_48kHz.opus`, `test_44kHz_taggedWithCover.aiff` und `cover.jpg` werden von keinem Test verwendet. Es fehlen: 8-Bit-WAV, Mono-WAV, 48-kHz-WAV, AIFF-C, WAV mit `LIST`/`fact`-Chunk, MP3 mit ID3v2.2.

- [ ] **Unbenutzte Includes im Test** (`tests/src/audio/audioinfo.cpp`, Kopfzeilen)
  `memfile.h`, `iconv.h`, `string.h` werden nicht gebraucht (bzw. `memfile.h` sollte gebraucht werden, siehe negativer Test oben).

- [ ] **`file.open()` wird ohne Fehlerbehandlung gerufen** (`tests/src/audio/audioinfo.cpp:67`, `86`, `104`, `122`, `140`, `159`, `178`, `202`, `204`)
  `File::open` wirft bei fehlender Datei; bricht ein Testpfad ab, ist die Fehlermeldung wenig aussagekräftig. Ein `ASSERT_NO_THROW`/Helper mit Dateipfad im Text verbessert die Diagnose. (Doppelt `open()` ohne `close()` ist dagegen unkritisch — `File::open` schließt intern vorher.)

- [ ] **Gewicht der Tests einseitig** (`tests/src/audio/audioinfo.cpp`)
  4 MP3-Tests, 2 AIFF, 1 WAV, 1 OGG, 1 Konstruktor. Der komplexeste Parser (`IdentAIFF` mit Chunkschleife) hat nur zwei Happy-Path-Tests; die fehleranfälligsten Codezeilen (Chunkschleife, `long double`, SSND-Offset) haben 0 % Abdeckung.

## Befunde in Drittdateien

- [ ] **`GetVorbisAudioInfo` setzt `FileSize` nie und verwendet andere Konvention** (`src/audio/AudioDecoder_Ogg.cpp:89–109`)
  `info.FileSize` bleibt auf Konstruktorwert 0, obwohl `AudioInfo::FileSize` für WAV/AIFF/MP3 gesetzt wird. `info.AudioSize = vf.end - vf.offset` ist exklusiv berechnet, während `IdentWave`/`IdentAIFF`/`IdentMP3` `AudioEnd` inklusiv führen. `Length` wird ohne 64-Bit-Zwischenwert berechnet (Überlauf bei langen Dateien). Test `IdentOggVorbis` prüft die Felder bewusst nicht — die Lücke fällt also nicht auf.

- [ ] **Exception-Vertrag von `FileObject::map` wird durchgängig missachtet** (`include/pplib/core/fileobject.h:604–607`, `src/core/File.cpp:750–770`)
  Der Vertrag ist exception-basiert: `File::map` wirft bei Bereichsüberschreitung `OverflowException` und liefert nur bei leerer Datei bzw. 0 Bytes `nullptr` (`File.cpp:753`, `:769`; Doku in `fileobject.h` ist inzwischen korrekt). Die in `AudioInfo.cpp` (und teils in `AudioDecoder_Ogg.cpp`) allgegenwärtigen `if (!buffer)`-Guards greifen daher nie — jeder kaputte Header wird zur Exception. Entweder alle Aufrufer auf den Exception-Pfad anpassen oder eine durchgehende `mapNoThrow`-Variante (Muster: `src/audio/Mp3.cpp:135`) in `FileObject` anbieten und dort die nullptr-Prüfung wieder verlässlich machen.

- [ ] **`mapNoThrow` ist statisch in `Mp3.cpp` dupliziert** (`src/audio/Mp3.cpp:135`)
  Sobald `AudioInfo.cpp` denselben Helfer braucht, sollte er als öffentliche Methode/Hilfsfunktion (z. B. `FileObject::mapNoThrow`) zentralisiert werden, statt ihn ein drittes Mal zu kopieren.

- [ ] **`WAVEHEADER`-Nutzung in `decoder.h`** (`include/pplib/audio/decoder.h:190`)
  `readWaveHeader` verwendet die in `audioinfo.h` definierte Struktur; bei Umzug der Struktur (siehe Smells) muss dieser Include-Pfad mitgezogen werden.

- [ ] **Verwandter offener Befund** (`codereview/todo/mp3-review.md:218–219`, `:235`)
  Bereits dort ist das ungesicherte `map(0, 24)` in `IdentAudioFile` dokumentiert. Empfehlung: Befund dort schließen und zentral in diesem Review abarbeiten, damit die Fixup-Arbeit nur einmal erfolgt.
