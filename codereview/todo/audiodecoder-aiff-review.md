# AudioDecoder_Aiff Review – Befunde

Review vom 2026-11-04, Scope: `src/audio/AudioDecoder_Aiff.cpp` (444 Zeilen) und die zugehörige Doxygen-Doku
`include/pplib/audio/decoder.h:325-458`.
Öffentliche API: `AudioDecoder_Aiff::AudioDecoder_Aiff()`, `~AudioDecoder_Aiff()`, `open()`, `close()`,
`getAudioInfo()`, `seekSample()`, `getPosition()`, `getSamples(STEREOSAMPLE16\*)`, `addSamples(STEREOSAMPLE32\*)`,
`getSamples(STEREOSAMPLE_FLOAT\*)`, `addSamples(STEREOSAMPLE_FLOAT\*)`.
Review done by: Copilot (Claude)

**Umgebung und Verifikationsweg:** Linux/x86-64, gcc 16.2.1. Alle Befunde mit „verifiziert" wurden praktisch
nachgestellt: eigene Kleinstprogramme außerhalb der Projektquellen, gelinkt gegen
`build/debug/libpplib_audio.a` + `build/debug/libpplib_core.a`. Für die Speicher- und UB-Befunde zusätzlich eine
extra mit `-fsanitize=address,undefined -fno-omit-frame-pointer -g` kompilierte `AudioDecoder_Aiff.o`.
Basis ist grün: `./build/debug/tests/test_all --gtest_filter='*Aiff*'` (aus `tests/`, wegen `test.conf`) läuft
50 Cases aus drei Suites durch — `AudioDecoderAiffTest` 21, `AudioInfoTest` 16, `ID3TagTest` 13, 0 Fehler.

---

## Bugs (kritisch)

- [x] **K1 — Fehlgeschlagenes `open()` hinterlässt einen Mischzustand aus zwei Dateien; danach stille Datenkorruption** (`AudioDecoder_Aiff.cpp:69-103`)
  `open()` schreibt das `AudioInfo` in beiden Zweigen sofort — Zeile 70 (`IdentAudioFile(file, this->info)`) bzw.
  Zeile 74 (`this->info = *audioinfo;`) — **bevor** irgendeine Prüfung läuft (Format `:76`, Kanäle `:78`,
  Framegröße `:82-84`, Encoding-Switch `:86-99`). `bytesPerChannel` (`:100`), `position` (`:102`) und `ff` (`:103`)
  werden dagegen erst ganz am Schluss gesetzt. Wirft eine der Prüfungen, bleibt das Objekt in einem Zustand, der
  die **Geometrie der gerade abgelehnten Datei** (B) mit der **Dekodierungs-Einstellung der vorher offen gewesenen
  Datei** (A) mischt: `info`, `AudioStart`, `AudioSize`, `Samples`, `BytesPerSample`, `Channels` von B; `ff` (zeigt
  weiter auf A), `encoding`, `bytesPerChannel`, `position` von A.
  Verifiziert (Repro `f2b`, wiederholt identisch, ohne ASan-Treffer): AIFF A ist 16 Bit stereo mit 40 Frames, AIFF B
  8 Bit stereo mit 40 Frames. Ablauf `d.open(A)`, `d.close()`, `d.open(A)`, dann `d.open(B)` → korrekt
  `UnsupportedAudioFormatException: BitsPerSample is not 16, 24 or 32` (`:98`). Danach meldet `getAudioInfo()`
  `Bits=8 BytesPS=2 Chan=2 Samples=40 Start=54 Size=80` (alles von B), der nächste `getSamples(40)` liest aber A bei
  `encoding=INT16`, `bytesPerChannel=2` — mit `frameBytes = info.BytesPerSample` (`:217`) jetzt 2 statt 4 Bytes.
  Ab Frame 1 gleitet jeder Frame um genau ein Kanal-Sample ab:

  | Frame | geliefert L/R | korrekt L/R |
  | ----- | ------------- | ----------- |
  | 0 | 11 / 5665 | 11 / 5665 |
  | 1 | 5665 / 9520 | 9520 / 15174 |
  | 2 | 9520 / 15174 | 19029 / 24683 |
  | 3 | 15174 / 19029 | 28538 / -31344 |

  Rückgabe ist `40` — also völlig unauffällig. Eine `try/catch`-Runde, die den Wurf abfängt und danach trotzdem
  weiterliest (typisch in Play-Loops mit Formatwechsel), spielt unhörbar verrutschte Samples ein; `seekSample()`
  (`:127-133`) und `getAudioInfo()` arbeiten in Geometrie von B.
  Die drei anderen Decoder machen es vor: `AudioDecoder_Ogg.cpp:291` (mit Begründungskommentar `:288-290`),
  `AudioDecoder_Flac.cpp:406` (`if (ff != nullptr) close();`), `AudioDecoder_MP3.cpp:262` — `open()` stellt dort
  zuerst den leeren Zustand her. AIFF und Wave sind die einzigen ohne dieses Muster (grep).
  Minimaler Fix, erste Zeile von `open()`; `close()` (`:112-120`) fasst die Datei des Aufrufers nicht an, nur die
  eigenen Felder werden zurückgesetzt:
  ```cpp
  void AudioDecoder_Aiff::open(FileObject& file, const AudioInfo* audioinfo)
  {
      close();   // Rest bleibt bei einem Wurf in einem definierten, leeren Zustand
      if (!audioinfo) {
  ```
  Damit ist der Fehlerfall durch die bestehende `close()`-Doku (`decoder.h:407-412`) abgedeckt: Reads werfen
  `DecoderException("No file opened")`, `getAudioInfo()` liefert ein leeres `AudioInfo`. K1 ist außerdem die
  einzige Tür zu K2 (SIGFPE).

  ==> Fixed durch Hinzufügen von `close()` am Anfang von `open()` 

- [x] **K2 — Division durch null in `prepareRead()` → SIGFPE (nicht abfangbar)** (`AudioDecoder_Aiff.cpp:203-204`)
  `info.BytesPerSample` fließt ungeschützt in zwei Divisionen. `ClampToAudioSize()` (`:49`) prüft die 0 zwar für
  sich, `prepareRead()` aber nicht. Erreichbar über den K1-Mischzustand oder ein handgereichtes `AudioInfo` mit
  `BytesPerSample = 0`: Die Geometrieprüfung `:82-83` lässt 0 durch (`chanBytes = (0+7)/8 = 0`, `0*2 == 0`), der
  Encoding-Switch wirft danach bei `default:` (`:98`) — und damit genau im K1-Mischzustand.
  Verifiziert (Repro `f3b`): 16-Bit-Datei offen, dann `open(f, &info)` mit `BitsPerSample=0, BytesPerSample=0,
  Channels=2` → Wurf, danach `getAudioInfo()` = `Bits=0 BytesPS=0 Samples=40 Start=54`; nächster Leseversuch:
  UBSan `src/audio/AudioDecoder_Aiff.cpp:203:60: runtime error: division by zero`, danach ASan
  `AddressSanitizer: FPE … #0 prepareRead (:203)`, Exit 1. Eine `catch`-Runde fängt einen SIGFPE nicht — der
  Prozess ist weg.
  Hinweise zur Reichweite: eine richtige AIFF-Datei liefert diese 0 nicht, `IdentAudioFile()` lehnt eine
  Bitbreite außerhalb 8..32 ab (`src/audio/AudioInfo.cpp:192`; verifiziert mit Repro `bd0`: `IdentAudioFile()`
  == false für eine AIFF mit Bitbreite 0 im COMM-Chunk). Erreichbar ist der Fall nur über ein vom Aufrufer
  selbst konstruiertes `AudioInfo` — abgesichert werden muss es trotzdem, weil `open()` diesen Weg öffentlich
  anbietet.
  ```cpp
  if (info.BytesPerSample == 0) return 0;   // Hand-AudioInfo mit unplausibler Framegroesse
  ```

  ==> FIXED

- [x] **K3 — `ff` ist ein unbesitzender Raw-Pointer auf das `FileObject` des Aufrufers; nach dem Ableben stiller UB** (`AudioDecoder_Aiff.cpp:103`, Nutzung `:202`/`:214`)
  `this->ff = &file` merkt sich nur die Adresse. Lebt das Objekt des Aufrufers nicht mehr (Scope verlassen,
  vorzeitiger `delete`), zeigt `ff` ins Leere; `close()` wird vom Aufrufer typischerweise nicht mehr aufgerufen.
  Verifiziert (Repro `f5`): `File` im Inneren einer Funktion erzeugt, `open()` darauf, Scope-Ende, dann
  `getSamples()` → ASan `stack-use-after-scope … READ of size 8`, Stack `#0 prepareRead (:202)`, `#1 getSamples (:211)`.
  Ein Fix ohne API-Änderung ist nicht möglich (Kopieren des Dateobjekts verbietet sich — `include/pplib/core/file.h:220-221`
  deleted Copy-CTor und Copy-Zuweisung, `MemFileCopy` hat ihn privat). Zu schließen ist der Vertrag durch Doku; FLAC (`decoder.h:884-886`) und Ogg
  (`decoder.h:673-676`) dokumentieren die Exklusivität des übergebenen Objekts, AIFF und Wave schweigen dazu.
  siehe Dokumentations-Befunde.

  ==> Doku nachgeschärft

---

## Bugs (mittel / Warnungen)

- [x] **M1 — Bei extern geschlossener Datei werfen die Lesemethoden `FileNotOpenException`, nicht die dokumentierte `DecoderException`** (`AudioDecoder_Aiff.cpp:202`, `:214`)
  Es gibt nirgends eine `isOpen()`-Prüfung (grep in `src/audio/*.cpp`: 0 Treffer). `prepareRead()` prüft nur den
  eigenen Zeiger (`:189`), nicht den Zustand des Objekts. Dann wirft `File::size()` die Exception
  (`src/core/File.cpp:383`) oder `File::map()` (`src/core/File.cpp:746`).
  Verifiziert (Repro `f4`): `f.close()` nach dem `open()`, danach `getSamples()` → Exception mit dem
  typeid `N5pplib20FileNotOpenExceptionE`. Das ist relevant, weil `FileNotOpenException` von `IOException :
  BaseException` abstammt, **nicht** von `DecoderException` (`include/pplib/exceptions.h`); ein
  `catch (DecoderException&)`, wie es die Doku `decoder.h:435/443/449/455` nahelegt, greift also nicht.
  Empfehlung (dokuseitig, kein Verhalten ändern): `FileNotOpenException` in der Exception-Aufzählung der vier
  Lesemethoden ergänzen. Alternativ eine Zeile in `prepareRead()` nach `:189`:
  ```cpp
  if (!ff->isOpen()) throw DecoderException("File closed");   // fileobject.h:772
  ```

  ==> Abgelehnt und in Doku vermerkt

- [ ] **M2 — Wächst die Datei nach `open()`, wirft der nächste Lesevorgang `OutOfBoundsException`** (`AudioDecoder_Aiff.cpp:202-204` vs. `src/core/File.cpp:748`)
  `prepareRead()` klemmt die Samplezahl an die **live** Dateigröße (`ff->size()`), `File::map()` prüft die
  angeforderte Länge aber gegen das bei `open()` gespeicherte `mysize` (`src/core/File.cpp:748`, wirft `:763`), und
  `File::size()` frischt genau dieses `mysize` nicht auf (`src/core/File.cpp:370-384`). Für streamende/appendende
  Dateien (typisch bei Aufnahmen oder Stream-Downloads) führt das zur Exception mitten im Play-Loop.
  Verifiziert (Repro `grow`): Hand-`AudioInfo` (`Samples=200`, 16 Bit/`BytesPS=4`, `Chan=2`, `Start=54`) auf einer
  AIFF-Kopie mit real 40 Frames. Phase 1: `getSamples(200)` → `got=40` (Klemmung über die Live-Größe, wie
  erwartet). Datei danach um 200 Nullbytes vergrößert. Phase 2: `OutOfBoundsException`
  (typeid `N5pplib20OutOfBoundsExceptionE`). Der Gegenpfad (Datei nach `open()` verkleinern) ist unkritisch: Repro
  `F8/F9` mit auf 60 Bytes truncierter Datei → `got=1`, um 128 Bytes vergrößert → `got=40`, jeweils ohne Exception.
  Empfehlung: Verhalten in der `open()`-Doku festhalten („eine nach `open()` wachsende Datei wird erst nach einem
  `close()`/`open()`-Paar erkannt; `map()` kann `OutOfBoundsException` melden"). Substanzieller nur als
  Strukturvorschlag (`tryMap()`/`mapAtMost()`, siehe unten) — Bewusstsein: `FileObject::map()` dokumentiert genau
  ein Mapping pro Objekt (`include/pplib/core/fileobject.h:603-607`), jeder Umbau muss das erhalten.

  ==> In Doku vermerkt.

- [ ] **M3 — `open()` verändert dauerhaft ein Objekt des Aufrufers: `MapWindowSize` wird angehoben und in `close()` nie zurückgenommen** (`AudioDecoder_Aiff.cpp:109`)
  ```cpp
  if (file.getMapWindowSize() < 32 * 1024) file.setMapWindowSize(32 * 1024);
  ```
  Der Quellkommentar `:105-108` begründet nur, dass ein größerer Wert des Aufrufers stehen bleibt, nicht aber, dass
  ein kleinerer dauerhaft angehoben wird — auch nach `close()` (`:112-120` fasst die Datei nicht an). Genutzt wird
  das Fenster dann von jedem anderen Mapper desselben Objekts.
  Verifiziert (Repro `F6`, zweimal gemessen): `f.setMapWindowSize(512)` → vor `open()` 512, nach `open()` 32768,
  nach `close()` weiterhin 32768. Wave analog (`AudioDecoder_Wave.cpp:109`), dort ebenso gemessen.
  Empfehlung: Nebenwirkung in der `open()`-Doku nennen (Parameter-Beschreibung oder `@note`), damit der Aufrufer das
  Fenster bei Bedarf selbst wieder setzen kann. Kein Verhalten ändern — der Wert ist für sequentielles Playen sinnvoll.

  ==> Bleibt so

- [ ] **[Prüfen] `map()`-Aufruf mit `samples * info.BytesPerSample` — Überlauf nur bei ungewöhnlicher Geometrie denkbar** (`AudioDecoder_Aiff.cpp:214`, `:274`, `:335`, `:393`)
  Die Multiplikation ist `size_t`, kann also auf 64 Bit praktisch nicht überlaufen; bei `uint8_t`-`BytesPerSample`
  ( ≤ 255) und `size_t`-`samples` bleibt sie unkritisch. Auf 32-Bit-Zielplattformen (`size_t` = 32 Bit) wäre ein
  Überlauf ab ~16,7 Mio. Frames je Lesevorgang erreichbar, also realistisch nur bei einem `AudioInfo` mit
  übersteuerter `AudioSize`. Nur der Vollständigkeit halber aufgeführt; konkret nicht nachgestellt.

  ==> Bleibt so

---

## Wartbarkeit & Aufräumen

- [x] **Vier tote `if (!data)`-Prüfungen nach `map()`** (`AudioDecoder_Aiff.cpp:215`, `:275`, `:336`, `:394`)
  `FileObject::map()` liefert laut Vertrag niemals `nullptr`, alle Fehler kommen als Exception
  (`include/pplib/core/fileobject.h:600-601` und `:614`; `File::map()` `src/core/File.cpp:744-764`,
  `MemFile::map()` analog). Die vier Würfe sind unerreichbar. Empfehlung: entfernen **oder** — sinnvoller — durch
  `tryMap()` ersetzen, dann wird die Prüfung lebendig (siehe Strukturvorschläge). Wave hat dieselben vier
  (`AudioDecoder_Wave.cpp:208/268/330/389`).

  ==> Fixed

- [x] **`prepareRead()` ruft dreimal virtuell `ff->size()` auf und berechnet dieselbe Division zweimal** (`AudioDecoder_Aiff.cpp:202-204`)
  Pro Lesevorgang drei virtuelle Aufrufe plus zwei gleichartige Divisionen. Kleine, lokale Verbesserung:
  ```cpp
  const uint64_t fileSize = (uint64_t)ff->size();
  if (offset >= fileSize) return 0;
  const size_t fits = (size_t)((fileSize - offset) / info.BytesPerSample);
  if (samples > fits) samples = fits;
  ```
  Achtung: Das ändert nichts an M2 — `ff->size()` bleibt Live-Wert, `map()` prüft weiterhin gegen `mysize`.

  ==> Fixed

- [x] **Kommentar nennt eine Exception, die `map()` nie wirft** (`AudioDecoder_Aiff.cpp:198-200`)
  „sonst wirft `map()` eine OverflowException" — `File::map()` meldet Grenzverstöße als `OutOfBoundsException`
  (`src/core/File.cpp:763`); `OverflowException` existiert zwar (`include/pplib/exceptions.h:147`), kommt hier aber
  nicht vor. Kommentar auf `OutOfBoundsException` korrigieren (Wave `:185-187` ebenfalls).

  ==> Fixed

- [x] **`ClampToAudioSize()` ist die statische Kopie desselben Helpers in Wave** (`AudioDecoder_Aiff.cpp:47-52`)
  Wortgleiche Duplikation in `AudioDecoder_Wave.cpp:46`. Da beide Dateien im selben Ordner und Linkschritt liegen,
  liegt ein gemeinsamer interner Helfer nah (eine nicht öffentliche Deklaration in einer neuen Datei
  `src/audio/audioimpl.h` o. ä.; `src/audio/` hat bisher keinen gemeinsamen internen Header) — der
  Stilregeln halber nur als Hinweis, keine Pflicht.

  == Bleibt so
---

## Dokumentation & Doxygen

- [ ] **`open()` dokumentiert eine Exception, die AIFF nie wirft** (`decoder.h:403`)
  `@exception DecoderInitializationException Das Format wurde nicht erkannt.` Geworfen wird die Klasse nur in
  `AudioDecoder_MP3.cpp:173`/`:287` und `AudioDecoder_Flac.cpp:439` — AIFF wirft bei unerkanntem Format
  `UnsupportedAudioFormatException` (`AudioDecoder_Aiff.cpp:71`). Die Zeile kann raus; das Format-Thema steht schon
  in der Zeile darüber (`:401`).

- [ ] **Exception-Aufzählung der vier Lesemethoden ist zu eng** (`decoder.h:435`, `:443`, `:449`, `:455`)
  Jede nennt nur `@exception DecoderException`. Tatsächlich können zusätzlich `FileNotOpenException` (M1) und
  `OutOfBoundsException` (M2) durchreichen, zudem `IllegalArgumentException` bei `map()` mit Länge 0
  (`include/pplib/core/fileobject.h:616`) — praktisch nicht erreichbar, weil `prepareRead()` Länge 0 mit `return 0`
  abfängt. Nachzug wie beim jeweiligen `prepareRead()`-Kommentar (`:384`) plus die beiden realen Fälle.

- [ ] **`getPosition()` ist undokumentiert** (`decoder.h:427`)
  Die Wave-Deklaration hat denselben Text bereits (`decoder.h:289-292`: Position in Frames bezogen auf
  `AudioInfo::AudioStart`). Für AIFF wortgleich übernehmen; die Implementierung (`AudioDecoder_Aiff.cpp:135-138`)
  tut genau das.

- [ ] **Float-Skala in der Klassentabelle weicht vom Code ab** (`decoder.h:345`)
  Tabellenzeile „32 Bit Float … Wert × 32768, auf ±32768 begrenzt". Tatsächlich begrenzt
  `ScaleFloatToSample16()` auf **+32767 / −32768** (`AudioDecoder_Aiff.cpp:169-170`). Runtime-verifiziert an
  `tests/testdata/audio/test_44kHz_32bit_float.aiff` (Kopie nach `/tmp/pplib_asan/fc.aiff`, Payload mit
  `struct.pack('>f', …)` geschrieben): 1.5 → +32767, −1.5 → −32768, NaN → 0. Zusätzlich fehlt dort, wie bei Wave
  (`decoder.h:220-221`), der Hinweis „NaN wird 0" — der steht nur im cpp-Kommentar `:161`/`:165-167`.

- [ ] **`open()`-Doku verschweigt die Dateilebensdauer und die Nebenwirkung am `FileObject`** (`decoder.h:392-405`)
  Drei Punkte fehlen: (a) das Objekt muss länger leben als der Decoder, und der Leser gehört exklusiv dem Decoder
  (Vorbild Flac `decoder.h:884-886`); (b) `open()` kann `setMapWindowSize()` am Objekt des Aufrufers ändern (M3);
  (c) was nach einem Wurf gilt — mit dem K1-Fix exakt die `close()`-Semantik, einen Satz wert.
  Korrigiert wird damit auch eine Lücke zur Klassenbeschreibung: AIFF liest im Gegensatz zu Ogg/Flac nicht über
  `seek()`, sondern ausschließlich über `map()` mit absoluten Offsets. Ein vom Decoder veränderter
  Dateipositions-Stand des Aufrufers ist also unkritisch — die Ogg-/Flac-Formulierung („verändert die Dateiposition")
  darf für AIFF nicht einfach kopiert werden.

- [ ] **Basisklasse und AIFF-Doku zu `getAudioInfo()` widersprechen sich** (`decoder.h:97-98` vs. `:414-418`)
  Die Basisklasse erklärt das Verhalten vor `open()`/nach `close()` für undefiniert, AIFF (und Wave) liefern dort
  dokumentiert ein leeres `AudioInfo`. Für AIFF ist die abgeleitete Doku die bessere Aussage; die Basis-Zeile
  gehört irgendwann entschärft (Befund in Drittdatei, siehe unten).


===> Doku drastisch abgespeckt!
---

## Befunde in Drittdateien

- [ ] **`AudioDecoder_Wave.cpp` hat dieselben Befunde 1:1** (`AudioDecoder_Wave.cpp:73/:74`, `:109`, `:176`, `:185-187`, `:190-191`, `:208/268/330/389`)
  inhaltlich identisch: `info` vor allen Prüfungen (`:73`/`:74` → Mischzustand K1), `setMapWindowSize(32*1024)`
  ohne Rücknahme in `close()` (M3, `:109`), fehlende `isOpen()`-Prüfung (`:176`, M1), `OverflowException`-Kommentar
  (`:185-187`), Divisionen durch `info.BytesPerSample` ohne 0-Schutz (`:190-191`, K2), vier tote `if (!data)`-Checks.
  Verifiziert mit echten WAV-Dateien (Repro `wv`: 16-Bit- stereo WAV A, 8-Bit-stereo WAV B):
  `d.open(A)`, `close()`, `open(A)`, `open(B)` → korrekt `UnsupportedAudioFormatException: BitsPerSample is not 16,
  24 or 32` (`:99`); danach `getAudioInfo()` = `Bits=8 BytesPS=2 Chan=2 Samples=40 Start=44` (von B), Encoding und
  `bytesPerChannel` von A; `getSamples(40)` → `got=40`, ab Frame 1 je ein Kanal-Sample verrutscht
  (Frame 1 kaputt L=-14699/R=-14398, korrekt L=-14398/R=-14097; Frame 2 kaputt L=-14398, korrekt L=-13796;
  Frame 3 kaputt L=-14097, korrekt L=-13194). Kein ASan-Treffer. `FileNotOpenException` bei externem
  `close()` ebenfalls gemessen, Fenster 512 → 32768 → 32768 wie bei AIFF.
  Hinweis: Die Mischzustands-Repro zu Wave braucht **8-Bit**-WAV; die zunächst verwendete 24-Bit-Datei wird von Wave
  unterstützt (`open()` gelingt), damit entsteht gar kein Mischzustand.

- [ ] **`File::size()` aktualisiert `mysize` nicht, `File::map()` prüft aber dagegen** (`src/core/File.cpp:370-384`, `:748`, `:763`)
  Zwei verschiedene Größenbegriffe im selben Objekt: Live-`fstat` für `size()`, Cache `mysize` fürs Mapping
  (`mysize` nur gesetzt in `:85/:104/:125/:133/:155/:163/:287/:316/:356/:403`). Jeder Aufrufer, der erst mit `size()`
  rechnet und dann `map()` aufruft (AIFF `:202-204` + `:214`), bekommt bei wachsender Datei eine
  `OutOfBoundsException`, obwohl die Bytes da sind. Option: `size()` setzt `mysize` neu, oder `map()` prüft gegen
  `fstat`. `tryMap()` (`include/pplib/core/fileobject.h:643-675`) umschifft das bereits, weil sie gegen `size()`
  prüft und `OutOfBoundsException` in `nullptr` übersetzt.

- [ ] **`File::close()` hebt nur das zuletzt gemappte Fenster auf** (`src/core/File.cpp`, `close()`-Implementierung, `unmap()` `:776-779`)
  Nach einem `map()` bleibt das Mapping bis zum nächsten `map()`/`unmap()` stehen; `File::close()` löst nur das
  letzte Fenster (`LastMapSize`) auf. Kein DEC-Decoder ruft `unmap()` nach der letzten Sample-Gruppe (grep: 0
  Treffer in `src/audio/`), d. h. ein geschlossener AIFF-Wiedergabeprozess hält bis zum `close()` des Objekts ein
  Mapping. Kein Fehler, aber relevant für Speicherverbrauch bei großen Dateien — eher ein Thema fürs
  Audio-Subsystem als für diese Datei.

---

## Verifiziert OK (kein Handlungsbedarf)

- Skalierungstabelle im Header stimmt für alle vier Quellformate mit dem Code überein — ausgenommen die
  Float-Zeile (Doku-Befund oben): 16 Bit unverändert (`:224-228`), 24 Bit `>> 8`
  (`:234-235`), 32 Bit `>> 16` (`:243-244`); im `STEREOSAMPLE_FLOAT`-Zielpuffer Pass-through bei `FL32`
  (`:430-433`), Division durch `2^31` bei 32-Bit-Int (`FULL_SCALE_F` `:175`). Runtime an echter Datei
  `/tmp/pplib_asan/fc.aiff` geprüft. Mono wird korrekt dupliziert (`stereo ? … : l` in allen vier Schleifen).
- `ClampToAudioSize()` (`:47-52`) verhindert wirksam das Lesen hinter dem durch `AudioSize` markierten Audioende und
  ist selbst gegen `BytesPerSample == 0` gefeit (`:49`). Repro `F7`: COMM nennt 4000 Frames, Datei hat 150 →
  `getAudioInfo().Samples` = 150 (der Parser klemmt bereits in `IdentAudioFile()`), `getSamples()` liefert 64, kein
  Zugriff außerhalb des Chunks, kein ASan-Report.
- AIFF-C-Kompressionen außer `NONE`/`FL32`/`fl32` lehnt schon `AudioInfo::IdentAudioFile()` ab
  (`src/audio/AudioInfo.cpp:194-204`), der `open()`-Switch (`:86-99`) sieht `sowt`/`ulaw`/`alaw`/`adpc`/`fl64` also
  nie — die Klassenbeschreibung (`decoder.h:329-334`) ist damit korrekt, es fehlt nichts.
- `seekSample()` begrenzt stillschweigend (`:129-132`) und wirft nie — wie dokumentiert (`decoder.h:422-423`);
  `open()` auf nicht-AIFF-Dateien wirft `UnsupportedAudioFormatException` ohne Seiteneffekte im Erstaufruf
  (`ff` bleibt `nullptr`, Reads bringen `DecoderException("No file opened")`, Repro `f3`).
- Die Sign-Magnitude→Zweierkomplement-Entscheidung und das fehlende 16-Bit-Wort-Padding sind bewusst
  dokumentiert (`decoder.h:352-358`) und werden nicht erneut gemeldet.

---

## Strukturvorschläge (nur zur Diskussion)

- **`tryMap()` statt `map()` in den vier Leseschleifen** (`:214/:274/:335/:393`): Damit werden die vier toten
  `if (!data)`-Prüfungen lebendig, und der M2-Fall (`OutOfBoundsException` bei wachsender Datei) wird zu einem
  sauberen Kurzlesen ohne Exception.
  Der Helper ist im Audio-Modul bereits im Einsatz (`AudioInfo.cpp:77`, `Mp3.cpp:143/:257` — grep).
  Aufwand klein, Risiko: `tryMap()` ist nicht virtuell (`fileobject.h:659-660`), liefert `char*` und `nullptr` auch
  bei `size == 0`; „kurz lesen statt werfen" muss in den Play-Loop des Aufrufers passen.
- **Geometrie-Validierung zentral in `prepareRead()`** (statt nur in `open()`): ein `if (info.BytesPerSample == 0)
  return 0;` (`:192`) plus ein Plausibilitätscheck `bytesPerChannel * info.Channels == info.BytesPerSample` macht
  K2 und den halben K1-Mischzustand unschädlich, unabhängig davon, woher das `AudioInfo` stammt.
- **Gemeinsamer Geometrie-Kern für AIFF und Wave:** `open()`-Prüfkette, `prepareRead()` und `ClampToAudioSize()` sind
  in beiden Dateien Zeile für Zeile gleich (AIFF `:47-52`/`:82-99`/`:186-206`, Wave `:46`/`:81-99`/`:173-193`).
  Ein gemeinsamer Helfer (z. B. `src/audio/audioreadutil.h`, ohne öffentliche API) beseitigt die Duplikation und
  stellt sicher, dass Fixes (K1/K2/M1) nicht zweimal vergessen werden. Nutzen hoch, Risiko mittel — die
  Encoding-Switches bleiben getrennt, weil AIFF big-endian und Wave little-endian liest.
- **`FileObject*` durch Referenz + Vertragsdokument ersetzen** (`ff` `:375` im Header): eine
  `FileObject*` mit expliziter „darf nicht leben"-Regel ist der Kern von K3. Vollständig löst es nur ein
  `std::unique_ptr<FileObject>`-Besitz oder ein `FileObject&`-Member mit Lösch-`close()`-Pflicht; beides ändert die
  API-Semantik der Basisklasse. Aufwand und Risiko hoch — bewusst nur als Diskussion.
- **`AudioInfo::BytesPerSample` von `uint8_t` auf eine breitere Breite** (`include/pplib/audio/audioinfo.h:138`:
  byte-basierte Felder): heute irrelevant (maximal 8 Byte je Frame bei 32 Bit × 2 Kanälen), aber ein `uint8_t`-Feld,
  das in `size_t`-Rechnungen und Divisionen landet, ist ein latenter Verjüngungs-/Abschneidepunkt. Nicht umsetzen,
  solange die Binärkompatibilität der Struktur gilt.

---

## Glossar

- **ASan (AddressSanitizer):** Compiler-Instrumentierung (`-fsanitize=address`), die Zugriffe auf freed oder
  außerhalb eines Objekts liegenden Speicher meldet.
- **UBSan (UndefinedBehaviorSanitizer):** Compiler-Instrumentierung (`-fsanitize=undefined`), die undefiniertes
  Verhalten meldet, hier die Division durch null.
- **SIGFPE:** Prozesssignal für einen arithmetischen Fehler (Ganzzahl-Division durch null). Als Signal nicht von
  C++-`try/catch` erfassbar — anders als eine Exception.
- **TOCTOU (Time-of-Check-to-Time-of-Use):** Wettlauf zwischen einer Prüfung (hier `ff->size()` in
  `prepareRead()`) und der Benutzung (hier `File::map()`), bei dem sich der Zustand dazwischen ändern kann.
- **RAII (Resource Acquisition Is Initialization):** Ressourcen werden an Objektlebensdauer gekoppelt; hier der
  Grund, warum ein roher `FileObject*` ohne Besitzregel semantische Lücken öffnet.
- **PImpl (Pointer to Implementation):** Idiom, das eine Implementierung hinter einem Zeiger verbirgt; im Review nur
  als Stil-Vergleich erwähnt, nicht als Befund.
- **Sign-Magnitude / Zweierkomplement:** Zwei Darstellungswurzen für vorzeichenbehaftete Ganzzahlen. AIFF definiert
  Erstere, der Decoder liest Letzteres (im Header dokumentiert).
- **COMM / SSND / FORM / AIFC:** Chunks einer AIFF-Datei; COMM enthält Abtastrate, Framezahl und Bitbreite, SSND die
  Audiodaten.
- **FL32:** AIFF-C-Kompressionstyp für unkomprimierte 32-Bit-IEEE-Float-Samples.
- **Mischzustand:** Objekt, nach einem fehlgeschlagenen `open()` Teilmengen der neuen und der alten Parameter enthält;
  Kern von K1.
