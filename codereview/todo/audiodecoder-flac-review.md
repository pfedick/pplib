# AudioDecoder_Flac Review – Befunde

Review vom 2026-11-01, Scope: `src/audio/AudioDecoder_Flac.cpp` (620 Zeilen), `include/pplib/audio/decoder.h` (602–723), `tests/src/audio/audiodecoder_flac.cpp` (708 Zeilen, 20 Tests).
Öffentliche API: `AudioDecoder_Flac()`, `~AudioDecoder_Flac()`, `open()`, `close()`, `getAudioInfo()`, `getPosition()`, `seekSample()`, `getSamples()`/`addSamples()` je 2× (16/32/float), intern `fillFrames()`/`transferFrames()`.
Review done by: Copilot (Qwen3-8-Flash-Next, Reasoning: xhigh)

**Wie verifiziert wurde:** valgrind-Memcheck über `test_all --gtest_filter='AudioDecoderFlacTest*'`; separates ASan/LSan-Build (`-fsanitize=address,undefined`) mit der echten Test-Übersetzungseinheit; eigene Mess- und Regressionproben (Leak, Sprungverhalten, Speicherbedarf, RSS) unter x86-64 Linux/GCC gegen die installierte libFLAC (SONAME `libFLAC.so.14`). Für die beiden kritischen Befunde wurden gepatchte Kopien der Quelle in Kopien von `build/debug/libpplib_audio.a` bzw. `build/asan/libpplib_audio.a` eingesetzt und die unveränderten 20 Tests erneut ausgeführt (20/20 grün). Projektquellen wurden nicht geändert.

---

## Bugs (kritisch)

- [ ] **libFLAC-Decoder wird nie freigegeben – Speicherleck in `close()`, Destruktor und allen drei `open()`-Fehlerpfaden** (`src/audio/AudioDecoder_Flac.cpp:404`, `:301`, `:372`, `:380`, `:386`)
  `FlacDecoderHandle` (`:86–104`) gehört der libFLAC-Handle nur bis `handle.release()` in `open()` (`:367–369`). Danach ist der Decoder im Objekt, aber `close()` (`:404`) und der Destruktor (`:301`) rufen nur `FLAC__stream_decoder_finish()` auf – das zerstört die Decodier-Interna nicht. Der eigentliche Freigeber `FLAC__stream_decoder_delete()` fehlt an allen fünf Stellen, an denen der Handle lebt; die drei Fehlerpfade in `open()` (`:371–376`, `:379–384`, `:385–390`) machen `delete state` ohne Decoder-Freigabe.
  **Messung:** `AudioInfo`-Handle selbst sind klein; das Leck ist die libFLAC-Interne Struktur: ca. 16 B direkt + ca. 9 776 B indirekt pro geöffnetem Decoder. Die komplette FLAC-Suite hängt am Ende mit **327 616 B in 249 Blöcken** – valgrind („definitely lost 496 B/31 Blocke, indirectly lost 327 120 B/218 Blocke", Stack `FLAC__stream_decoder_new` → `FlacDecoderHandle()` `:93` → `open()` `:349`) und LSan (`SUMMARY: 327616 byte(s) leaked in 249 allocation(s)`) nennen deckungsgleich dieselben Stellen.
  **Reproduktion auch ohne Suite:** eigener Probe = 3× `open()/close()` plus 3 abgelehnte `open()`-Aufrufe (Post-Init-Mismatch gegen ein handgebautes 16-Bit-`AudioInfo` zu `test_44kHz_8bit.flac`). Ungepatchte Bibliothek: `Direct leak of 48 byte(s) in 3 object(s)` für die abgelehnten Versuche, LSan-Summe 29 376 B in 15 Allokationen. Dasselbe Programm gegen die gepatchte Bibliothek: keinerlei Leak-Ausgabe, Exit-Code 0.
  **Minimaler Fix (Variante B, geprüft):** Handle-Lifecycle unverändert lassen, nur zusätzlich löschen.
  ```cpp
  // Destruktor (:301) und close() (:404), identisch:
  if (state->dec != nullptr) {
      FLAC__stream_decoder_finish(state->dec);
      FLAC__stream_decoder_delete(state->dec);   // NEU: zerstört auch die internen Puffer
      state->dec = nullptr;
  }
  delete state;
  ```
  ```cpp
  // in allen drei open()-Fehlerpfaden (:372, :380, :386) ergänzen:
  FLAC__stream_decoder_finish(dec);
  FLAC__stream_decoder_delete(dec);              // NEU
  delete state;
  state = nullptr;
  ```
  Damit verschwindet das Leck vollständig (siehe Messung oben), und die 20 bestehenden Tests bleiben grün. `FLAC__stream_decoder_delete()` darf laut libFLAC-Doku auch auf einem nicht initialisierten Handle aufgerufen werden; hier wird es bewusst erst nach `finish()` gerufen.

- [ ] **`totalFrames` wird bei keinem Sprung zurückgesetzt – Fehlerberichterstattung hängt vom Decode-Verlauf ab, Materialrutsch möglich** (`src/audio/AudioDecoder_Flac.cpp:249`, Zähler `:225`, Rücksetzung in `seekSample()` `:422–427` und `:438–442` fehlt)
  Die Sonderregel in `flacErrorCb()` (`:246–250`) unterdrückt `LOST_SYNC` und `UNPARSEABLE_STREAM`, sobald `totalFrames >= metaSamples` erreicht ist – gedacht für abhängende Daten nach dem letzten Rahmen (ID3v1-Tag am Dateiende). `totalFrames` steigt nur in `flacWriteCb()` (`:225`) und wird beim struct-Init auf 0 gesetzt (`:79`). `seekSample()` leert zwar Rohpuffer und Flags, **nicht** aber `totalFrames`. Ist das Sample-Ende einmal erreicht, gilt die Unterdrückung in jeder späteren Decode-Phase – auch für einen echten Sync-Verlust mitten im Material.
  **Konsequenz:** Dieselbe Datei wird je nach Aufrufhistorie unterschiedlich bewertet. Einmal bis zum Ende gelesen, melden spätere defekte Abschnitte keinen Fehler mehr; der Decoder liefert dann Frames aus kaputtem Material, ohne Exception – der Verbraucher hält falsche Samples für korrekt.
  **Reproduktion (Probe, selbe Datei, drei Decode-Durchläufe über ein Korpus mit defektem Schluss):** Durchlauf 1 und 2 werfen erwartet `DecoderException` nach 77 824 Frames; Durchlauf 3 (nachdem `totalFrames` bereits am Ende war) liefert **173 695 Frames ohne Exception**. Mit der Rücksetzung ist das Verhalten in allen drei Durchläufen identisch (Exception bei 77 824), und die Toleranz gegenüber Dateien mit abgeschnittenem Ende oder Tag bleibt erhalten (Tests `TruncatedFileYieldsPrefixOfIntactFile`, `TaggedAndSeektableFilesDecodeLikePlainFile` bleiben grün).
  **Minimaler Fix:** An beiden Reset-Stellen in `seekSample()` den Zähler mitnehmen.
  ```cpp
  // :422–427 (erster Reset-Block)
      state->seekTarget = kNoSeek;
      state->totalFrames = 0;    // NEU: Unterdrückungsregel wieder scharf stellen

  // :438–442 (zweiter Reset-Block nach fehlgeschlagenem Sprung)
      state->error = false;
      state->totalFrames = 0;    // NEU
  ```

## Bugs (mittel / Warnungen)

- [ ] **C++-Exceptionen aus libFLAC-Callbacks fliegen durch C-Stackframes** (`src/audio/AudioDecoder_Flac.cpp:118`, `:119`, `:144`, `:156`, `:165`, `:174`)
  Die fünf Betriebs-Callbacks rufen ungeschützt `FileObject`-Methoden: `tell()` und `size()` in `flacReadCb()` (`:118–119`), `size()` in `flacSeekCb()` (`:144`), `tell()` in `flacTellCb()` (`:156`), `size()` in `flacLengthCb()` (`:165`), `eof()` in `flacEofCb()` (`:174`). `File::size()` wirft in `src/core/File.cpp:370`, `File::tell()` in `src/core/File.cpp:470` (z. B. `FileNotOpenException`, wenn der Verbraucher das `FileObject` zwischenzeitlich geschlossen hat). libFLAC ruft diese Callbacks aus eigenem C-Code auf und besitzt keinen Exception-handling-Vertrag – eine Exception, die durch C-Frames des Dekoders wandert, ist nicht definiertes Verhalten; mindestens werden libFLAC-eigene Ressourcen nicht freigegeben.
  **Nachweis:** eigener Probe, der während des Decodens das `FileObject` schliesst und dann weiterliest – eine `FileNotOpenException` kommt aus dem libFLAC-Stack heraus (ASan-Build zeigt zusätzlich die hängenden libFLAC-Puffer).
  **Minimaler Fix:** Muster aus `flacWriteCb()` (`:204–209`) auf die Betriebs-Callbacks anwenden, z. B. `flacReadCb`:
  ```cpp
  static FLAC__StreamDecoderReadStatus flacReadCb(..., void* client)
  {
      FlacState* st = static_cast<FlacState*>(client);
      try {
          ... bisheriger Rumpf ...
      } catch (...) {
          st->error = true;   // trägt sich wie in flacWriteCb() über das Fehlerflag ein
          return FLAC__STREAM_DECODER_READ_STATUS_END_OF_INPUT;
      }
  }
  ```
  Ebenso `flacSeekCb` → `FLAC__STREAM_DECODER_SEEK_STATUS_ERROR`, `flacTellCb`/`flacLengthCb` → Rückgabewert mit Fehlercode-Flag, `flacEofCb` → `false` plus `st->error = true`. (Für `flacTellCb`/`flacLengthCb` liefert libFLAC keinen expliziten Fehlerstatus; `*return_offset = 0` zusammen mit gesetztem `st->error` ist hier die praktikabelste Minimallösung – das Verhalten ist bereits durch `transferFrames()` sichtbar.)

- [ ] **Linearer Fallback-Sprung puffert die übersprungene Strecke unbegrenzt** (`src/audio/AudioDecoder_Flac.cpp:452–458`, Puffer-Reset `:491–495`)
  Kann libFLAC nicht springen (Samplezahl unbekannt, oder `FLAC__stream_decoder_seek_absolute()` fehlgeschlagen), liest `seekSample()` vom Datenanfang bis zum Ziel und ruft dafür `fillFrames(sample - position)` (`:453`) – also eine einzige Chunk-Anfrage über die komplette Distanz. Der Rohpuffer wächst damit linear zur Sprungentfernung.
  **Messung (RSS eines Probestarts mit `test_44kHz_16_stereo.flac`):** nach `open()` 5 460 kB resident, nach `seekSample(170000)` 7 092 kB; das sind ca. 1,6 MB für 170 000 Frames, hochgerechnet **ca. 1,27 GB pro Materialstunde** (44,1 kHz, 16 Bit, Stereo). Bei großen Dateien mit langer Spieldauer führt ein einzelner Spürlauf durch die Datei zum Auslagern oder zum `bad_alloc`, der im Write-Callback zwar abgefangen wird (`:206–209`), dessen Folge aber nur ein pauschaler `DecoderException` ist.
  **Minimaler Fix:** in kleinen Portionen vorlesen, der Puffer-Reset in `fillFrames()` (`:491–495`) hält den Speicher dann konstant:
  ```cpp
  while (position < sample) {
      const size_t chunk = (state->metaMaxBlocksize > 0) ? (size_t)state->metaMaxBlocksize : 4096u;
      const size_t want = (sample - position < chunk) ? (sample - position) : chunk;   // NEU
      const size_t avail = fillFrames(want);                                            // war: sample - position
      if (avail == 0) return;
      ...
  }
  ```

- [ ] **Fehlschlag des Sprungs verändert die Position – Header sagt das Gegenteil** (`src/audio/AudioDecoder_Flac.cpp:436–451`, Doku `include/pplib/audio/decoder.h:690–691`)
  Nach fehlgeschlagenem `seek_absolute()` setzt der zweite Reset-Block (`:438–442`) `position` nicht zurück; der lineare Pfad endet an der Stelle, an der er abbricht. **Gemessen:** auf einer abgeschnittenen Datei mit Stand 1000 führt `seekSample(170000)` zu Position **8192**, nicht zu 1000. `decoder.h:690–691` behauptet „Ein Fehlschlag des Sprungs lässt die Position unverändert." Beides ist möglich: entweder Doku auf „die Position bleibt am letzten lesbar erreichten Sample stehen" korrigieren, oder in `:436` den früheren Stand merken und bei Abbruch (`:454`) wiederherstellen – die Doku-Variante ist die kleinere Änderung.

- [ ] **Nach Dateiende werden Decodierfehler verschluckt** (`src/audio/AudioDecoder_Flac.cpp:521`)
  `if (state->error && !state->atEnd) throw DecoderException(...)` – sobald `atEnd` gesetzt ist, wird ein gesetztes `error`-Flag ignoriert. Das ist laut Kommentar beabsichtigt (Anhängsel nach dem letzten Rahmen), macht aber den Preis der Toleranz unsichtbar: ein Sync-Verlust, der erst nach dem regulären Ende gemeldet wird, ist von einem echten Defekt nicht zu unterscheiden, und weder Verbraucher noch Tests bekommen das mit. Siehe dazu der Testvorschlag `Erneut lesen nach einem Fehler` im Abschnitt Testqualität. Empfehlung: mindestens ein `bool hasError()`-Zugriff oder ein Debug-Hinweis – eine Änderung des Wurfverhaltens ist hier nicht zwingend.

- [ ] **`AudioStart`/`AudioSize` aus dem `AudioInfo` werden nicht ausgewertet** (`src/audio/AudioDecoder_Flac.cpp:385–394`, vergleiche `src/audio/AudioDecoder_Aiff.cpp:47–49`, `:196–201` und `src/audio/AudioDecoder_MP3.cpp:302–305`)
  `IdentFLAC()` berechnet in `src/audio/AudioInfo.cpp:446–459` Datenanfang und Datenlänge und zieht ein ID3v1-Tag am Dateiende (128 Byte, Magic `TAG`) bereits ab. Der FLAC-Decoder verwendet diese Angaben nicht; er liest bis zum physischen Dateiende und verlässt sich für das Tag auf die Fehlerunterdrückung in `flacErrorCb()` (`:249`, kombiniert mit Befund 2 oben). Folge: beliebiges Anhängsel nach dem letzten Rahmen wird kommentarlos als Dateiende interpretiert, und ein `AudioInfo` mit bewusst gesetzter `AudioSize` (z. B. aus einem Container) wirkt nicht.
  **Empfehlung:** entweder Doku in `decoder.h:617–618` an die Realität anpassen (siehe Doku-Befunde) oder – aufwändiger – `state->audioStop = src->AudioStart + src->AudioSize` setzen und in `flacLengthCb()` (`:162–169`) zurückgeben, damit libFLAC am Audioende stoppt.

- [ ] **Untergrenze `fsize < 44` passt nicht zur Datei-Erkennung, Kommentar rechnet falsch** (`src/audio/AudioDecoder_Flac.cpp:261`)
  Der Kommentar „10 Byte Tag + 4 Magie + Kopf + STREAMINFO" ergibt 52, nicht 44; ohne ID3v2-Tag wäre die wahre Untergrenze 43 (4 Magie + 4 Blockkopf + 34 STREAMINFO + mindestens 1 Byte Audiodaten, genau diese Bedingung prüft `IdentFLAC()` mit `audioSize == 0` in `src/audio/AudioInfo.cpp:455`). **Gemessen:** ein künstlich konstruierter 43-Byte-FLAC (STREAMINFO als letzter Block, ein Byte Daten) wird von `IdentAudioFile()` als `FLAC` erkannt (`Format=5`, `Channels=2`, `Bits=16`, `AudioSize=1`), `AudioDecoder_Flac::open()` wirft dazu `DecoderException: FLAC-Datenanfang nicht gefunden`. Praktisch kaum relevant, aber ein unnötiger Widerspruch zwischen Sniffer und Decoder. **Fix:** `if (fsize < 43)` und Kommentar auf „4 Magie + 4 Blockkopf + 34 STREAMINFO + mindestens 1 Byte Rahmen" korrigieren.

- [ ] **MD5-Prüfsumme abgeschaltet, nirgends dokumentiert** (`src/audio/AudioDecoder_Flac.cpp:355`)
  `FLAC__stream_decoder_set_md5_checking(dec, false)` ist die einzige Stelle dieser Art in den Audio-Decodern und hat weder Kommentar noch Doku-Gegenstück. Die Abschaltung ist bei stückweisem Lesen mit toleriertem Dateiende plausibel (libFLAC kann die Prüfsumme bei abgebrochenem Stream nicht valide abschließen), sollte aber begründet werden, sonst wirkt sie wie ein versehentlich deaktivierter Fehlercheck. **Fix:** ein Kommentar an `:355` plus ein `@note` im Klassenblock, das dass die Prüfsumme bewusst nicht ausgewertet wird.

- [ ] **Zwei Decoder an einem `FileObject` teilen sich die Dateiposition** (`src/audio/AudioDecoder_Flac.cpp:112–175`)
  Alle Betriebs-Callbacks arbeiten direkt auf dem übergebenen `FileObject`. **Gemessen:** zwei Decoder auf demselben `FileObject`; nach einem Sprung von Decoder 1 steht die Dateiposition von 16 384 auf 278 361; decoder 2 liest anschließend von dort und liefert einen anderen Stream als erwartet. Das ist in Misch- oder Einblend-Szenarien ein echter Fehler, und nirgends dokumentiert. Minimallösung: Doku-Hinweis bei `open()`, dass das `FileObject` exklusiv genutzt werden muss; alternativ (größerer Eingriff) intern ein eigenes `File` vom selben Pfad öffnen.

- [ ] **[Prüfen] Write-Callback prüft nur `buffer[0]` auf `NULL`, nicht `buffer[1]`** (`src/audio/AudioDecoder_Flac.cpp:191` vs. `:220`)
  Bei einer als Stereo registrierten Quelle (`st->channels == 2`, erzwungen durch die Kanalzahl-Prüfung in `:187–189`) dereferenziert `:220` `buffer[1]`, ohne es wie `buffer[0]` in `:191` getestet zu haben. libFLAC liefert pro Kanal eine nicht-`NULL`-Ebene, und die Kanalzahl-Prüfung verhindert den Mismatch – reproduzieren liess sich der Fall mit der installierten libFLAC nicht. Befund bleibt defensiv: einheitliche Prüfung beider Ebenen (z. B. `if (frames == 0 || buffer[0] == nullptr || (st->channels == 2 && buffer[1] == nullptr))`) kostet nichts und macht den Callback robust.

- [ ] **[Prüfen] Verwerflogik nach Sprung wirkt nur bei sample-nummerierten Rahmen** (`src/audio/AudioDecoder_Flac.cpp:194–202`)
  Der Block verwirft vor dem Ziel liegende Rahmen nur, wenn `number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER`; bei rahmen-nummerierten Strömen wird nichts verworfen, weil `seekTarget` in `:201` unbedingt auf `kNoSeek` gesetzt wird. Das ist in der Praxis unkritisch, weil Ströme ohne Samplezahl nie per `seek_absolute()` springt (dann übernimmt der lineare Pfad in `seekSample()` das Weglassen) – ohne eigenen Fixture ist das nicht prüfbar. Empfehlung: einen kurzen Kommentar ergänzen, warum der `else`-Fall nichts tun muss, damit die Stelle nicht als Fehler gelesen wird.

## Testqualität (explizit angefragt)

Die 20 Tests in `tests/src/audio/audiodecoder_flac.cpp` sind deutlich besser als der Durchschnitt: Sie prüfen Ergebnisse statt Interna, und sie nutzen existierende Referenzdecoder statt den FLAC-Decoder gegen sich selbst zu melden.

**Was stark ist**
- **Bit-genaue Ganzdatei-Vergleiche gegen `AudioDecoder_Wave`** (`:141` 16 Bit, `:165` 32-Bit-Ziel, `:188` Float) über 173 695 Frames. Die Helper `DecodeAll16/Float/32` (`:46–89`) lesen in Stücken von `kChunk = 1009` – eine Primzahl, damit Puffer- und Rahmen-Grenzen (4096) nie zusammenfallen. Das fängt Off-by-One-Fehler in der Pufferlogik zuverlässig, die mit runden Chunkgrößen unsichtbar bleiben.
- **Absolute Anker zusätzlich zum Vergleich** (`:210` `EXPECT_NEAR(452 / 32768.0f, …)`, `:587` feste Framezahl): damit fällt auch ein gemeinsamer Fehler beider Decoder auf.
- **Materialvarianten sind abgedeckt**: ID3v1 (`test_44kHz_16bit_id3v1.flac`), ID3v2, SEEKTABLE, Stream ohne Samplezahl, abgeschnittene Datei, Mono (`:415`, `:306`, `:446`, `:473`, `:236`).
- **Sprungsemantik wird ausgereizt**: exakter Treffer (`:255`), rückwärts und wiederholt (`:278`), Klemmen über das Ende (`:349`), Mehrfachsprung auf abgeschnittener Datei (`:473`), EOF liefert dauerhaft 0 (`:368`).
- **Zustand nach `close()`** (`:605`) und **abgelehnte Formate inklusiv Post-Init-Prüfung** (`:636`, `:668`) sind getestet; `:636` prüft zusätzlich, dass die Fabrik den richtigen Decoder auswählt.

**Lücken, die echte Fehler übersehen**

- [ ] **Kein Test liest nach einem Durchlauf bis zum Ende erneut durch defektes Material** (`tests/src/audio/audiodecoder_flac.cpp:564`, `:473`)
  Das ist genau die Lücke hinter Befund 1 (kritisch). `CorruptAudioMaterialStopsWithoutHanging` liest die kaputte Datei einmal und erwartet korrekt eine Exception; dasselbe Material wird aber nie ein zweites Mal durch denselben Decoder gelesen. **Ergänzung (vor dem Fix rot, nach dem Fix grün – an gepatchter Bibliothek gemessen):**
  ```cpp
  TEST_F(AudioDecoderFlacTest, SecondPassOverCorruptMaterialStillReports)
  {
      // wie oben: Bytes 150000..150511 mit 0xFF überschreiben, MemFile darüber
      std::vector<pplib::STEREOSAMPLE16> out;
      EXPECT_THROW(DecodeAll16(dec, out), pplib::DecoderException);
      dec.seekSample(0);
      out.clear();
      EXPECT_THROW(DecodeAll16(dec, out), pplib::DecoderException);  // ohne Fix: kein Wurf
  }
  ```
- [ ] **Kein Reichweiten-/Speichertest für den linearen Fallback** (`:473`)
  `TruncatedFileSeeksFallBackToLinearSkip` prüft nur das Ergebnis (8192 Frames), nicht den Aufwand. Der unbegrenzte Puffer (Befund „linearer Fallback“ oben) fällt deshalb nie auf. Nach dem Chunking-Fix lässt sich das billig fixieren: ein zählendes `operator new`/`operator delete` nur für diesen Test, oder eine feste Obergrenze der Puffergröße über einen friend-/Testhook; ohne Infrastruktur reicht schon ein Test, der `seekSample(kurzhin vor Ende)` auf dem Stream-Fixture lädt und damit die volle Distanz durchläuft.
- [ ] **Kein Lifecycle-/Leak-Test** (`:556` deckt nur `open()`-Fehler ab)
  Nirgends wird `open() → close() → erneut open()` an einem Objekt geprobt, obwohl genau diese Folge die Handle-Behandlung in `open()`/`close()` belastet. Ein solcher Test hätte das Speicherleck nicht gefunden (gtest meldet Leaks nicht), aber zusammen mit LSan schon: **LSan/ASan hängt nicht in der regulären Testroutine**, und der ASan-Bau ist derzeit nicht nutzbar – ein eigener Versuch bricht in `AudioDecoderWaveTest` mit `global-buffer-overflow` in gtest-eigenem `testing::ScopedTrace` ab, bevor die FLAC-Suite läuft. Bis das behoben ist: `./build/asan/tests/test_all --gtest_filter='AudioDecoder*Flac*'` mit `ASAN_OPTIONS=detect_leaks=1` als Kontrolllauf; das meldet heute 327 616 B in 249 Allokationen.
- [ ] **MemFile-Pfad kaum getestet** (`:663–665`)
  `MemFile` kommt nur als Negativ-Fixtur der Erkennung vor. Das Decodieren über eine nicht dateibasierte Quelle ist ungetestet, obwohl dafür alle fünf Betriebs-Callbacks (`:112–175`) auf anderen Pfaden laufen (ohne echtes `seek()`/`tell()`-System). Und dort wäre der Befund „Exceptionen fliegen durch libFLAC-C-Frames“ testbar: `MemFile` während des Decodens zerstören bzw. ein geschlossenes `FileObject` weiterverwenden.
- [ ] **Zwei Decoder an einem `FileObject` ungetestet**
  Die Kopplung über die Dateiposition (Warnungs-Befund) ist ein realistisches Muster in Misch- oder Einblend-Code. Ein Zwillingstest mit zwei Decodern auf einem `File` würde das Verhalten festlegen (welcher Stand gilt nach einem Sprung des anderen).
- [ ] **24-Bit-Kette hat keinen externen Anker** (`:213–234`)
  Die Erwartung wird aus dem Float-Ergebnis desselben Decoders zurückgerechnet (`:228–231`), verankert ist der Test nur über die Bereichsprüfung in `:232` und über `BitsPerSample == 24`. Ein Maßstab existiert bereits im Material: `test_44kHz_24bit.wav` wird in den Wave-Tests benutzt (`tests/src/audio/audiodecoder_wave.cpp:603`, `:645`). **Praktisch nachgeprüft:** die 24-Bit-Konvertierung ist in beiden Decodern identisch (Wave `src/audio/AudioDecoder_Wave.cpp:226–227` `SignExtend24(v) >> 8`, FLAC `:538–539` `>>= 8`) – die beiden vorhandenen Fixtures enthalten aber **nicht dasselbe Material** (direkter Vergleich: 2628 von 4096 Frames unterscheiden sich um je 1 LSB, erste Frames stimmen überein). Aus einem Encode von `test_44kHz_24bit.wav` mit `flac` wäre die Erwartung bit-genau; sonst genügen hartvercodete Einzelwerte als Anker.
- [ ] **Erwartungen an defektes Material zu weich** (`:591–592`)
  `EXPECT_GT(corrupt.size(), 4000)` und `EXPECT_LE(corrupt.size(), good.size())` erlauben fast alles zwischen 4001 und 173 695 Frames. Nach dem Fix der Fehlerhistorie lässt sich der Wert festnageln (im Probe: Exception nach exakt 77 824 Frames), was den Test zu einer echten Regressionssicherung macht.
- [ ] **`getSamples(num, STEREOSAMPLE32*)` wird von keinem Test aufgerufen** (`src/audio/AudioDecoder_Flac.cpp:549–550`, ohne Abdeckung)
  Die vier Lesemethoden sind laut Header eine geschlossene Gruppe (`decoder.h:654`), getestet sind 16-Bit-Lesen, 32-Bit-*Addieren* und 16/32/float-Addieren. Die Zuweisung ins 32-Bit-Ziel fehlt – genau der Pfad, der für Mischkanal-Puffer zentral ist.

**Kleine Mängel in den Tests**
- Kommentare enthalten systematisch Tippfehler: „Dekoder“ statt „Decoder“ (`:107`, `:556`, `:567`, `:599`, `:622`), „Decder“ (`:131`), „Zeroertes“ (`:566`), „geschoebenen“ (`:221`), „Raeume“ (`:345`, `:451`, `:465`; in `:451` zusätzlich „zwei komplette Raeume zu je 4096 Rahmen“ – nach Sinn wohl „zwei Rahmen zu je 4096 Samples“), „cutscharfe“ (`:451`), „unbeschnittnen“ (`:459`).
- Die Fixture-Helfer nutzen feste Pfadpräfixe `testdata/audio/` (`:92–113`); die Läufe müssen aus `tests/` starten. Das ist konsistent, aber still: ein fehlendes Fixture bricht mit `ASSERT_TRUE(file.isOpen())` ohne Dateinamen. Ein `<< name` an den Assertions würde zusätzlichem Aufwand beim Debuggen sparen.

## Wartbarkeit & Aufräumen

- [ ] **Ohne libFLAC ist der `#else`-Zweig zur Hälfte toter Code und in sich uneinheitlich** (`src/audio/AudioDecoder_Flac.cpp:595–618`)
  Bereits der Konstruktor wirft (`:287`), also sind `open()` (`:599`), `transferFrames()` (`:608`) und die Rückgaben von `close()`/`seekSample()`/`getSamples()`/`addSamples()` (`:602`, `:604`, `:611`, `:613–616`) für Aufrufer nie erreichbar. Gleichzeitig ist das Verhalten uneinheitlich: die einen Methoden werfen, die anderen liefern still 0 – und die Leseschnittstelle ist damit je nach Methode unterschiedlich. Da der Konstruktor ohnehin wirft, ist die einfachste Vereinheitlichung, den Kommentar an `:595` zu ergänzen („nur für ABI-Kompatibilität; ein Objekt entsteht hier nie“) und die stillen Varianten auf Wurf umzustellen – oder umgekehrt alles auf „liefern nichts“ zu ziehen und doc in `decoder.h:635` nachzuziehen.
- [ ] **ID3v2-Synchsafe-Länge zweimal implementiert** (`src/audio/AudioDecoder_Flac.cpp:263–273`, parallel `src/audio/AudioInfo.cpp:347` `ID3v2TagSize()`)
  Die Handauswertung (vier mal 7 Bit), die Versionsgrenze (`tag[3] < 2`) und die Plausibilitätsgrenze sind in beiden Dateien sinngemäß gleich (`size + 10 + 4 > fsize` gegen `pos + 4 > fsize`). Befund details stehen im Abschnitt Drittdateien; hier nur: entweder `ID3v2TagSize()` aus `AudioInfo` nutzen (der Header ist in beiden Fällen eingebunden) oder den Grund für die eigene Fassung kommentieren (`ID3v2TagSize()` ist eine interne Funktion der Übersetzungseinheit in `AudioInfo.cpp`).
- [ ] **Kommentar-Rechenfehler und uneindeutige Begründung** (`src/audio/AudioDecoder_Flac.cpp:261`)
  Siehe Warnungs-Befund zur Untergrenze: „10 Byte Tag + 4 Magie + Kopf + STREAMINFO“ beschreibt 52 Byte, verglichen wird mit 44.
- [ ] **Tippfehler in Kommentaren** (`src/audio/AudioDecoder_Flac.cpp:192` „Raeume“, `:553` „geschoebenen“)
  Beide betreffen erklärende Kommentare an kritischen Stellen (Verwerfen vor dem Sprungziel, Float-Skalierung) – dort sollte der Wortlaut stimmen.
- [ ] **Sentinel-Wert dreimal als Zahl ausgeschrieben** (`src/audio/AudioDecoder_Flac.cpp:258`, `:261`, `:264`, `:266`, `:268`, `:270`, `:337–341`)
  `0xFFFFFFFFFFFFFFFFULL` für „nicht gefunden“ neben dem benannten `kNoSeek` (`:52`). Ein `static const uint64_t kNoStream = ...` kostete nichts und macht die Stelle in `open()` (`:338`, `:341`) weniger fehleranfällig.

## Dokumentation & Doxygen

- [ ] **Klassendoku behauptet das Gegenteil des Codes** (`include/pplib/audio/decoder.h:613–614`)
  „… `IdentAudioFile()` erkennt solche Dateien nicht als dekodierbares `FLAC`." Tatsächlich trennt `IdentFLAC()` Erkennung und Unterstützung bewusst: `src/audio/AudioInfo.cpp:441–443` dokumentiert das wörtlich („Erkennung und Unterstützung sind getrennt; abgelehnt wird erst in AudioDecoder_Flac::open()") und meldet Kanalzahl/Bitbreite auch für 8/32 Bit und 5.1. Der Test `tests/src/audio/audiodecoder_flac.cpp:636` prüft genau dieses Verhalten – die Doku ist der Ausreißer. **Fix:** „… erkennt sie als `FLAC` und meldet Kanalzahl und Bitbreite; abgelehnt wird erst in `open()`."
- [ ] **„ID3v1-Tag am Dateiende über das `AudioInfo` berücksichtigt" ist falsch** (`include/pplib/audio/decoder.h:617–618`)
  `IdentFLAC()` rechnet das Tag in `AudioSize` heraus (`src/audio/AudioInfo.cpp:446–459`), der Decoder wertet diese Felder aber nirgends aus (weder `AudioDecoder_Flac.cpp` noch `AudioDecoder.cpp` verwenden `AudioStart`/`AudioSize`; AIFF und MP3 tun es, z. B. `src/audio/AudioDecoder_Aiff.cpp:196–201`, `src/audio/AudioDecoder_MP3.cpp:302–305`). Das Tag wird allein über die Fehlerunterdrückung in `flacErrorCb()` (`src/audio/AudioDecoder_Flac.cpp:249`) toleriert – also genau über die Stelle, die Befund 1 (kritisch) betrifft. **Fix:** Doku auf „ein ID3v2-Tag wird beim Auffinden des Datenanfangs übersprungen; ein ID3v1-Tag am Dateiende wird über die Fehlerbehandlung toleriert" umstellen und die MD5-Entscheidung ergänzen.
- [ ] **`open()`-Doku zu übergebenem `AudioInfo`** (`include/pplib/audio/decoder.h:666–667`)
  „verwendet … Kanalzahl, Bitbreite, Abtastrate und Ortsangaben unverändert" – Kanalzahl und Bitbreite werden ausdrücklich gegen den STREAMINFO-Block geprüft und die Datei sonst abgelehnt (`src/audio/AudioDecoder_Flac.cpp:385–389`), Bitrate/Ortsangaben werden nicht übernommen (`src/audio/AudioDecoder_Flac.cpp:392–394` berechnet die Bitrate aus den Werten). **Fix:** „die Angaben werden übernommen und gegen den STREAMINFO-Block geprüft; `AudioStart`/`AudioSize` wertet der Decoder nicht aus."
- [ ] **`fillFrames()` dokumentiert eine Exception, die nie geworfen wird** (`include/pplib/audio/decoder.h:651`)
  `@exception DecoderException interner Decodierfehler.` – `fillFrames()` (`src/audio/AudioDecoder_Flac.cpp:466–497`) wirft niemals; der Fehler wird über `state->error` gemerkt und erst in `transferFrames()` (`:521`) zum Wurf. **Fix:** `@exception`-Zeile von `fillFrames()` zu `transferFrames()` verschieben und bei `fillFrames()` ergänzen: „setzt bei Dekoderfehler das interne Fehlermerkmal, werft nicht".
- [ ] **`seekSample()`-Doku zur Fehlerversion ist falsch** (`include/pplib/audio/decoder.h:690–691`)
  „Ein Fehlschlag des Sprungs lässt die Position unverändert." Gemessen: Position 1000 → 8192 nach `seekSample(170000)` auf abgeschnittener Datei. Siehe Warnungs-Befund, Minimalvariante ist die Dokukorrektur.
- [ ] **`DecoderInitializationException` nicht dokumentiert** (`include/pplib/audio/decoder.h:670–673`)
  `open()` wirft sie, wenn `FLAC__stream_decoder_new()` nichts liefert (`src/audio/AudioDecoder_Flac.cpp:352`); diese Exception und der Wurf des Konstruktors bei fehlender libFLAC (`:287`) fehlen im Exception-Block, dort steht nur `open()` warf `UnsupportedFeatureException` (`:599`).
- [ ] **Unsaubere Formulierung in `open()`** (`include/pplib/audio/decoder.h:668`)
  „Abgelehnt werden mehrere als ein oder zwei Kanäle" → „mehr als ein oder zwei Kanäle".
- [ ] **Kommentar im Code rechnet falsch** (`src/audio/AudioDecoder_Flac.cpp:261`)
  „10 Byte Tag + 4 Magie + Kopf + STREAMINFO" = 52, verglichen wird mit 44; siehe Warnungs-Befund zur Untergrenze.

## Code Coverage (nachgefragt)

`coverage.xml` für `src/audio/AudioDecoder_Flac.cpp`: **89,0 % Zeilen (275/309), 74,9 % Zweige.** Die 34 ungedeckten Zeilen (`121–122, 128–129, 133–137, 148–150, 188–189, 206–209, 341, 351–352, 380–383, 448–449, 483–485, 549–550`) zerfallen in drei Gruppen:

- [ ] **Fehlerpfade der libFLAC-Callbacks sind völlig ungetestet** (`:121–137`, `:148–150`)
  Alle Fehlerzweige von `flacReadCb()` (Seek-Fehlschlag, Lesefehler, Kurzlesen) und `flacSeekCb()` sind nie durchlaufen worden. Das ist zugleich die Begründung für den Warnungsbefund „Exceptionen durch C-Frames": kein Test ist dort entlanggelaufen. **Testvorschlag:** `FileObject` während des Decodens schließen und weiterlesen – deckt die Fehlerzweige und gleichzeitig den Exception-Pfad ab; ein Fixture, bei dem `seek()` an einer Stelle außerhalb der Datei scheitert, deckt `:148–150`.
- [ ] **Eine öffentliche Lesemethode hat gar keine Abdeckung** (`:549–550`)
  Der Zuweisungszweig `buf32 != nullptr && !add` gehört zu `getSamples(num, STEREOSAMPLE32*)`; einer der vier Lesemethoden wird von keinem FLAC-Test aufgerufen. **Testvorschlag:** in `Samples16MatchWaveDecoder` (`tests/src/audio/audiodecoder_flac.cpp:141`) einen zweiten Vergleichslauf gegen `STEREOSAMPLE32` ergänzen – Referenzmaterial existiert bereits.
- [ ] **Unerreichbare Verteidigung als „Lücke" steht in der Statistik** (`:351–352`, `:380–383`)
  `FLAC__stream_decoder_new()` kann praktisch nicht fehlschlagen. Der `!haveMeta`-Block (`:379–384`) ist nur über einen Aufrufer erreichbar, der ein eigenes `AudioInfo` mitliefert – ohne Mitgabe lehnt `IdentAudioFile()` eine Datei ohne STREAMINFO als ersten Block bereits ab (`src/audio/AudioInfo.cpp:422`). **Empfehlung:** diese beiden Blöcke im Kommentar als reine Verteidigung kennzeichnen, damit sie bei Coverage-Arbeit nicht als Testziel missverstanden werden – oder mit `assert`-Absicherung in Debug-Builds versehen.
- [ ] **Komplett ungenutzt: der `#else`-Zweig ohne libFLAC** (`:595–618`)
  Die Testdatei ist als Ganzes durch `#ifdef HAVE_LIBFLAC` (`tests/src/audio/audiodecoder_flac.cpp:45` … `:708`) geschützt, für den anderen Zweig existiert also kein Test. Zwei Tests (`Konstruktor wirft `UnsupportedFeatureException``, `Lesen ist unmöglich`) in einem Build ohne `HAVE_LIBFLAC` wären billig und würden die API-Zusage aus `decoder.h:635` haltbar.
- [ ] **Kanalwechsel im Stream ohne Fixture** (`:188–189`)
  Die einzige Schutzmaßnahme gegen wechselnde Kanalzahl ist ungetestet; ein Fixture mit zwei Rahmen unterschiedlicher Kanalzahl lässt sich mit `metaflac`/eigenem Bitstrom erzeugen. Ohne Test ist nicht gesichert, dass der Abbruch (und nicht ein stiller Pufferüberlauf) folgt – bei einem Decoder, der hier `ABORT` liefert, wäre ein Test die günstigste Absicherung.

## Befunde in Drittdateien

- [ ] **`GetAudioDecoder()` fängt keine Exceptions um Konstruktion und `open()`** (`src/audio/AudioDecoder.cpp:39–55`)
  Je nach Fehlerquelle wirft der Konstruktor (`src/audio/AudioDecoder_Flac.cpp:287`) oder `open()` (`:308`, `:338`, `:352`, `:375`, `:383`, `:389`); die Fabrik gibt das halb fertiggestellte Objekt frei, ohne den Decode-Teil bereinigt zu haben – bei `AudioDecoder_Flac` verstärkt durch das Speicherleck (Befund 1, kritisch), weil das Leck schon beim fehlgeschlagenen `open()` entsteht. Derselbe Punkt ist in `codereview/todo/audiodecoder-ogg-review.md` bereits festgehalten; Fix dort vorgeschlagen (`try`/`catch` plus Rückgabe `nullptr` oder dokumentierter Wurf).
- [ ] **Größenuntergrenzen von Sniffer und Decoder passen nicht zusammen** (`src/audio/AudioInfo.cpp:455` vs. `src/audio/AudioDecoder_Flac.cpp:261`)
  `IdentFLAC()` verlangt mindestens ein Byte Audiodaten hinter dem letzten Metadata-Block, der Decoder verlangt pauschal 44 Byte Datei. Gemessen bei 43 Byte: `IdentAudioFile()` meldet `FLAC` (`AudioSize=1`), `open()` wirft `DecoderException`. Ebenso unterschiedlich: `DetectFormat()` in `src/audio/AudioInfo.cpp:497` verlangt mindestens 24 Byte.
- [ ] **ID3v2-Größenberechnung doppelt implementiert** (`src/audio/AudioInfo.cpp:347–360`, `src/audio/AudioDecoder_Flac.cpp:266–277`)
  Synchsafe-Auswertung und Versionsgrenze (`< 2`) sind in beiden Dateien gleich, ebenso die Plausibilitätsgrenze (`size + 10 + 4 > fsize` bzw. `pos + 4 > fsize`). `ID3v2TagSize()` liegt in der anonymen Umbegung von `AudioInfo.cpp` und ist von außen nicht nutzbar. Befund: duplizierte Logik mit Driftrisiko, kein aktueller Verhaltensunterschied.
- [ ] **ASan-Bau ist für Audio-Tests derzeit unbenutzbar** (`tests/src/audio/audiodecoder_wave.cpp`, gtest `:2095` in `googletest-src`)
  `./build/asan/tests/test_all` bricht in `AudioDecoderWaveTest` mit `global-buffer-overflow` in `testing::ScopedTrace` ab, bevor andere Audio-Suiten laufen. valgrind meldet zusätzlich „957 Errors aus 32 Kontexten", die alle auf gtest-eigenes `PrintByteSegmentInObjectTo`/`_itoa_word` (uninitialisiertes Padding in den Parametern von `YamlScalarTest`) zurückgehen – kein Fehlverhalten des Audio-Codes, aber die Fehlermeldungen überdecken ihn. Bis das behoben ist, FLAC-Tests gezielt per `--gtest_filter` ausführen.

## Strukturvorschläge (nur zur Diskussion)

- **`FlacState` besitzt den libFLAC-Handle** (`src/audio/AudioDecoder_Flac.cpp:54–104`): Ein `std::unique_ptr<FLAC__stream_decoder, decltype(&FLAC__stream_decoder_delete)>` (vergleichbar `FlacDecoderHandle` `:86–104`) als Mitglied von `FlacState` machte das Leck strukturell unmöglich, statt es an fünf Stellen zu beheben. Nutzen: kein Pfad kann vergessen werden; Aufwand: mittel (die `open()`-Fehlerpfade und `close()` werden kürzer); Risiko: Reihenfolge `finish()` vor `delete()` muss erhalten bleiben. Die im Review geprüfte Minimallösung (Befund 1) ist bewusst kleiner.
- **`AudioStart`/`AudioSize` aus dem AudioInfo nutzen** (`src/audio/AudioDecoder_Flac.cpp:385–394`, `:162–169`): Statt Anhängsel nach dem letzten Rahmen über Fehlerunterdrückung zu ignorieren, könnte `flacLengthCb()` die vom Sniffer bestimmte Audiolänge zurückgeben. Nutzen: ID3v1 und Container-Anhängsel werden sauber abgeschnitten, Unterdrückungsregel in `flacErrorCb()` überflüssig; Aufwand: mittel; Risiko: Verhalten bei abgeschnittenen Dateien ändert sich, vorhandene Fixtures (`:446`, `:473`) müssten neu bewertet werden.
- **Chunkung im linearen Fallback als decode-Puffer-Obergrenze ausbauen** (`:446–458`): eine konfigurierbare Obergrenze für die Puffergröße (statt der festen Ein-Chunk-Anfrage) würde zugleich den `bad_alloc`-Pfad (`:206–209`) erreichenbar machen und ließe sich im Test über einen zählenden `operator new` messen. Aufwand: klein bis mittel, Risiko: gering (Verhalten unverändert, nur Speicherprofil).
- **Leck-Messung in die Testroutine einhängen**, z. B. ein `make leak-audio`-Ziel mit `valgrind --errors-for-leak-kinds=definite,possible --leak-check=full --error-exitcode=1` über allen Audio-Suiten. Nutzen: die in diesem Review per Hand gefundenen 327 616 B würden sofort auffallen; Aufwand: klein; Risiko: Laufzeit (Memcheck Faktor ~30), daher als eigenes Ziel und nicht im `make file`-Pfad.
- **Parameterisierte Bit-exakt-Vergleiche** (`tests/src/audio/audiodecoder_flac.cpp:141`, `:165`, `:188`, `:213`): ein `TEST_P`-Parameterpaar (FLAC-Fixture, Referenz-Fixture, Zielformat) würde die 24-Bit-Kette automatisch mit abdecken, sobald ein 24-Bit-FLAC-Fixture aus `test_44kHz_24bit.wav` erzeugt ist. Aufwand: klein, Risiko: keiner.

## Glossar

- **ASan (AddressSanitizer):** Compiler-Instrumentierung (`-fsanitize=address`), die Speicherfehler zur Laufzeit meldet.
- **`bad_alloc`:** C++-Exception, die `new` bei fehlendem Speicher wirft; im Write-Callback abgefangen (`:206–209`).
- **Branch-Abdeckung:** Anteil der entschiedenen Programmzweige (wahr/falsch eines `if`), gemessen in `coverage.xml`.
- **Callback:** von der Bibliothek aufgerufene Kundenfunktion; hier die sechs libFLAC-Rückruffunktionen in `:112–252`.
- **C-Stackframe („durch C-Frames werfen"):** Eine C++-Exception verlässt fremden C-Code ohne `catch`; dafür gibt es keinen Standard – Verhalten undefiniert.
- **definitely lost / indirectly lost:** valgrind-Kategorien; „indirectly" beschreibt Blöcke, die nur über ein Leck erreichbar sind (hier die libFLAC-Puffer).
- **ID3v1 / ID3v2:** Metadaten-Tag am Dateiende (128 Byte, Magic `TAG`) bzw. am Dateianfang (10 Byte Kopf, Größe synchsafe).
- **LSan (LeakSanitizer):** Teil von ASan, meldet nicht freigegebenen Speicher am Programmende (`detect_leaks=1`).
- **`LOST_SYNC` / `UNPARSEABLE_STREAM`:** libFLAC-Fehlermeldungen für verlorenen Rahmentakt bzw. unlesbaren Rahmen (`flacErrorCb`, `:246–250`).
- **Memcheck:** valgrind-Werkzeug für Speicher- und Initialisierungsfehler.
- **Rahmen (frame):** kleinste decodierbare Einheit eines FLAC-Stroms, bis `max_blocksize` Samples; 4096 im Testmaterial.
- **RSS (Resident Set Size):** tatsächlich genutzter physischer Speicher eines Prozesses, hier zur Messung des Pufferwachstums.
- **SEEKTABLE / STREAMINFO:** FLAC-Metadata-Blöcke mit Sprungzielen bzw. mit Abtastrate, Kanälen, Bitbreite und Samplezahl.
- **synchsafe Integer:** ID3v2-Größenformat, 4 × 7 Bit, ohne High-Bits.
- **undefiniertes Verhalten (UB):** Programmzustand, für den der Standard keine Ergebnisse vorschreibt; hier durch Exceptionen aus Callbacks und ggf. durch übergelaufene Puffer.
- **Übersetzungseinheit (translation unit):** eine `.cpp`-Datei mit ihren Headers, hier als separate Objektdatei gegen die Bibliothek gelinkt.

