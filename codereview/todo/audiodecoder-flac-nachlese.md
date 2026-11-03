# AudioDecoder_Flac Nachlese der Review-Fixes

Review vom 2026-11-01, Scope: `src/audio/AudioDecoder_Flac.cpp` (712 Zeilen),
`include/pplib/audio/decoder.h` (Abschnitt `AudioDecoder_Flac`),
`tests/src/audio/audiodecoder_flac.cpp` (1125 Zeilen).
Geprueft wurde der Stand Commit `2686913` ("Bugfixes im Flac-Decoder") gegen
`codereview/done/audiodecoder-flac-review.md`.
Offentliche API: `AudioDecoder_Flac` (open, close, seekSample, getPosition, getAudioInfo und die
vier Verfahren der Basisklasse `AudioDecoder`), Fabrik `GetAudioDecoder()`.
Review done by: GPT-5.5 (Copilot in VS Code)

## Prüfweg

Alle als verifiziert bezeichneten Aussagen beruhen auf ausgefuehrten Tests oder eigenen
Sondeprogrammen; reine Lesebefunde sind im Text als solche erkennbar.

- Debug-Bau `build/debug`, Tests aus `tests/` (Fixtures liegen unter `tests/testdata/audio/`):
  `test_all --gtest_filter='AudioDecoderFlacTest*'` = 31 von 31 grün.
- Debug-Bau, komplette Audio-Gruppe `--gtest_filter='Audio*'` = 161 von 161 grün.
- ASan/UBSan-Bau `build/asan` (Schalter aus dem Projekt-Cache: `-fsanitize=address,undefined
  -fno-omit-frame-pointer -g -O1`). gelaufen mit
  `ASAN_OPTIONS=detect_leaks=1:handle_segv=0:allocator_may_return_null=1:abort_on_error=0`, damit
  die Tests nach einer Meldung weiterlaufen und am Ende der Leak-Spiegel gedruckt wird. Die
  FLAC-Gruppe bleibt 31 von 31 grün, ohne Leck- und ohne UBSan-Meldung. Vor den Fixes hatte das
  Review 327 616 Byte
  in 249 Blöcken gemessen.
- ASan-Bau, komplette Audio-Gruppe: 161 von 161 grün, eine ASan-Meldung, die aus
  `tests/src/audio/audiodecoder_wave.cpp:705` kommt und nicht aus dem FLAC-Modul
  (Einzelheiten im Abschnitt Drittdateien).
- Sondeprogramme (eigene `main()`, gegen `build/debug/libpplib_audio.a`, nur in `/tmp`, nachher
  geloescht) fuer den Zustand von `FlacState` und den Nachweis der Endlosschleife.
- `g++ -fsyntax-only -std=c++20` ohne `-DHAVE_LIBFLAC` auf dem Modul: keine Meldung.

Kurzfazit: Alle abhakten Befunde sind im Code vorhanden und die Tests bestaetigen sie. Bei der
Callback-Absicherung ist zusaetzlich ein neuer kritischer Fehler eingeschleust worden, siehe
ersten Punkt im Abschnitt "Bugs (kritisch)".

## Abgleich der alten Befunde

Umsetzung bestaetigt:

- Metadaten werden nicht mehr doppelt gezaehlt: `open()` setzt
  `FLAC__stream_decoder_set_metadata_ignore_all()` (`:418`), STREAMINFO wird in genau einem
  Callback ausgewertet (`flacMetadataCb()` `:279` bis `:288`), `totalFrames` erhoht nur noch der
  Write-Callback (`:275`). Ein zweiter, unabhaengiger Metadaten-Pfad ueber ein eigenes
  Dateiobjekt existiert nicht mehr; die zaehlenden Abweichzaehlungen der Fruehfassung sind weg,
  nach `g_totalFrames`, `g_metaActive`, `instanceCount_` und `g_bufferLimitBytes` sucht grep
  vergeblich.
- Die Pruefsumme am Dateiende bleibt abgeschaltet (`FLAC__stream_decoder_set_md5_checking(dec,
  false)`, Begruendung Kommentar `:414` bis `:416` und `@note` `decoder.h:642` bis `:648`);
  `AudioInfo::Samples` wird nicht veraendert, der STREAMINFO-Wert nicht manipuliert.
- Die Toleranz fuer Sync-Verlust ist an die STREAMINFO-Samplezahl gebunden (`:296` bis `:300`) und
  wird nach jedem Sprung wieder scharf gestellt (`state->totalFrames = 0` in beiden Reset-Blöcken,
  `:502` und `:519`). Die fruehere Puffer-Guard-Logik, die kaputte Dateien als Dateiende durchgehen
  liess, ist weg: `fillFrames()` bricht nur bei `atEnd` oder `error` ab (`:564`); `error` setzt
  der default-Zweig des Status-Switch (`:580`) und `flacErrorCb()` (`:301`).
- Alle funf Betriebs-Callbacks (`:129`, `:158`, `:173`, `:191`, `:207`) und der Write-Callback
  stehen in `try`/`catch`; eine Ausnahme aus pplib erreicht libFLAC nicht, die Callbacks melden
  ABORT und setzen `FlacState::ioError`. Der Lese-Callback begrenzt die Wunschgroesse auf die
  Restlaenge der Datei (`requested > remaining` liefert den Rest), damit bleibt `tell` monoton.
- `open()` prueft Kanalzahl und Bitbreite (`:387` bis `:392`) und vergleicht STREAMINFO gegen das
  mitgebrachte `AudioInfo` (`:450`, `UnsupportedAudioFormatException` bei Abweichung).
- Der Puffer ist ein flacher `std::vector<int32_t>` aus Kanal-Samples statt einer Chunk-Liste;
  reserviert wird am groessten Rahmen aus STREAMINFO (`:462`). Beim linearen Vorlesen wird in
  Portionen dieser Blockgroesse gearbeitet (`:533`), damit waechst der Puffer bei weiten Spruengen
  nicht mit der Distanz. Nach jedem Verbrauch werden Lese- und Schreibzeiger zurueckgesetzt
  (`fillFrames()` `:557` bis `:561`, `transferFrames()` `:665` bis `:669`).
- Das Handle lebt in der RAII-Huelle `FlacDecoderHandle` (`:94` bis `:113`); `open()` gibt es in
  jedem Fehlerpfad explizit frei (`:434`, `:443`, `:451`) und rueckt es erst nach erfolgreicher
  Initialisierung heraus (`handle.release()`, `:431`); der Standardablauf gibt ueber eine einzige
  Stelle zurueck (`:357`). Der LeakSanitizer-Lauf meldet nichts mehr.
- Fabrik: `GetAudioDecoder()` faengt die Ausnahme des `open()` und gibt `NULL` zurueck
  (`src/audio/AudioDecoder.cpp:47` bis `:59`), abgedeckt von
  `FactoryReturnsNullWhenTheFileCannotBeOpened`.
- Jede Methode hat einen eigenen `#ifdef HAVE_LIBFLAC`/`#else`-Zweig; der else-Zweig wirft
  ausnahmslos `UnsupportedFeatureException("AudioDecoder_Flac")`, bestaetigt durch den
  Syntax-Lauf ohne libFLAC.
- Doxygen-Punkte umgesetzt: `@note` zur MD5-Pruefung (`decoder.h:642` bis `:648`), `@note` zum
  exklusiven `FileObject` (`:650` bis `:652`), Hinweis, dass `AudioStart`/`AudioSize` beim Oeffnen
  ignoriert werden (`:689` bis `:693`), `DecoderInitializationException` in der Ausnahmeliste
  (`:702`), Hinweis auf den Neustart bei fehlgeschlagenem Sprung im `seekSample()`-`@note`
  (`:724`), Formulierung "mehr als zwei Kanaele" (`:687`).
- Testlücken geschlossen: 31 Testfälle, darunter `SeekRepositionsAcrossChunkBoundaries` (`:453`),
  `LinearFallbackSeekDoesNotBufferTheJumpDistance` (`:524`),
  `ClosedFileAbortsDecodingAtTheCallbacks` (`:890`), `MemFileDecodesLikeARealFile` (`:242`),
  `SmallestPlayableFlacIsAccepted` (`:1100`), `Samples24MatchTheReferenceDecoder` (`:381`),
  `ReadIntoTargetOverwritesPrefilledBuffer` (`:216`).

Bewusst nicht umgesetzt (laut Review so vermerkt, Stand unveraendert): ein
`getSamples(num, STEREOSAMPLE32*)` gibt es weiterhin nicht, das Fixture fuer einen Kanalwechsel
innerhalb einer Datei fehlt, die ID3v2-Erkennung bleibt in `src/audio/AudioInfo.cpp` zweimal
kopiert (`:178` und `:269`; die FLAC-Erkennung in `FlacStreamStart()` `:304` bis `:334` ist eine
dritte Variante), und es gibt weiterhin keinen Baudurchlauf ohne libFLAC, in dem die Tests die
`#else`-Zweige pruefen.

Keiner der abhakten Befunde ist halb oder falsch repariert. Beim Nachziehen der
Callback-Absicherung ist jedoch ein neuer Fehler entstanden.

## Bugs (kritisch)

- [x] **`seekSample()` loescht `FlacState::ioError` nicht, danach laeuft `getSamples()` endlos
      weiter** (`src/audio/AudioDecoder_Flac.cpp:496` bis `:502`, Folgewirkung `:577`)
      Der Reset-Block am Anfang von `seekSample()` setzt `atEnd`, `error`, `totalFrames` und
      `seekTarget` zurueck, nicht aber `ioError`. Der Schutzgedanke dahinter, naemlich das Flag
      dauerhaft stehen zu lassen, damit ein spaeteres `FLAC__stream_decoder_reset()` die
      Dateiposition nicht entwertet, gilt nur, wenn kein neuer Versuch anlaeuft. Im erfolgreichen
      Zweig (`FLAC__stream_decoder_seek_absolute()` `:506`, Rueckgabe `:508`) bleibt das Flag von
      einem frueheren Dateifehler stehen, und `fillFrames()` kann sein Ende-Flag nicht mehr
      setzen: `if (!state->ioError) state->atEnd = true;` (`:577`). Damit endet die
      Nachleseschleife (`while (avail < num)`, `:563`) nie, weil weder `error` noch `atEnd`
      gesetzt sind: jeder Durchlauf ruft `FLAC__stream_decoder_process_single()` (`:565`) erneut,
      libFLAC ist erschopft und meldet nur noch Streamende: die Statusweiche (`:569`) laeuft auf
      den Fall `FLAC__STREAM_DECODER_END_OF_STREAM` (`:573`), dessen Zeile `atEnd` wegen des
      Altschadens nicht setzt (`:577`), und der default-Zweig, der `error` setzen wuerde
      (`:579` bis `:581`), wird nie erreicht. Das Werfen in `transferFrames()` (`:616`) verlangt
      zusaetzlich ein nicht gesetztes `atEnd`, und der Rest des Puffers wird vorher noch
      abgegeben: der Aufrufer sieht also weder eine Ausnahme noch ein Ende, sondern dreht in
      `fillFrames()` ohne Rueckkehr.
      Reproduktionsablauf der Sonde gegen die Debug-Bibliothek: oeffnen, 20 Frames lesen,
      dieselbe `FileObject`-Instanz mit `close()` schliessen, `getSamples(TARGET, ...)` wirft
    korrekt `DecoderException` (ueber den Lese-Callback eines geschlossenen Streams werden
      `error` und `ioError` gesetzt), Datei mit derselben Instanz wieder oeffnen,
      `seekSample(40000)` laeuft erfolgreich durch, die naechste Portion bleibt haengen.
      Verifiziert: Endlosschleife in der Sonde durch `SIGALRM`-Timeout nachgewiesen (der Prozess
      laeuft nach dem Timeout weiterhin in derselben Schleife). Gegenbeweis mit einer gepatchten
      Kopie der Bibliothek (`ar r` in eine Kopie von `libpplib_audio.a`; der Quelltext im Projekt
      wurde nicht angefasst): dieselben Spruenge laufen mit dem Patch aus, 153 Aufrufe mit
      153 515 Frames bis 173 695, nach einem Sprung zur Haelfte 133 Aufrufe mit 133 695 Frames,
      Exit-Code 0. Ohne Patch dreht die Schleife unendlich. Der ASan-Lauf zeigt den HANG nicht,
      ein gdb-Anhaengen war in dieser Umgebung nicht moeglich, deshalb der Timeout-Beweis.
      Fix, eine Zeile im vorhandenen Stil:
      ```cpp
      state->atEnd = false;
      state->error = false;
      state->ioError = false;  // neuer Versuch: der naechste Durchlauf beginnt wieder sauber
      state->totalFrames = 0;
      ```
      Der Kommentar, der das dauerhafte Setzen begruendet (`:494` bis `:495`), muss auf den Pfad
      ohne erneuten Versuch eingeschraenkt werden, sonst widersprechen sich Kommentar und Code
      (siehe Abschnitt Strukturvorschlaege).

      **Umsetzung:** Reproduziert mit `AudioDecoderFlacTest.SeekAfterReopeningTheFileStillDetectsTheEnd`
      (tests/src/audio/audiodecoder_flac.cpp:931): Datei schliessen, bis die `DecoderException` kommt,
      dieselbe `FileObject`-Instanz wieder oeffnen, `seekSample(40000)`; ohne Fix dreht der Test endlos
      (`timeout`-Rueckgabe 124), mit Fix endet er bei 133 695 Frames. Fix ist die fehlende Zeile
      `state->ioError = false;` im ersten Reset-Block (`src/audio/AudioDecoder_Flac.cpp:505`) samt
      Kommentar, der den Grund nennt. Die FLAC-Suite laeuft danach 32 von 32 gruen.

## Bugs (mittel / Warnungen)

- [x] **Doppeltes `FLAC__stream_decoder_reset()` im Fehlsuche-Zweig**
      (`src/audio/AudioDecoder_Flac.cpp:512` und `:523`)
      Der Zweig ruft zuerst `FLAC__stream_decoder_reset()` (`:512`), geht in den sequenziellen
      Pfad und ruft dort erneut `FLAC__stream_decoder_reset()` (`:523`) vor dem Metadata-Nachlauf
      (`:524`). Dazwischen liegt kein Durchlauf, libFLAC setzt seinen Status auf `SEARCH_METADATA`
      zurueck, der erste Aufruf ist also wirkungslos. Gefaehrdlich ist weniger die Doppelung als
      dass an beiden Stellen unterschiedliche Einstellungen stehen koennten. Empfehlung: die
      erste Stelle streichen oder den Reset in `seekSample()` auf eine Stelle ziehen.

      **Umsetzung:** Der erste Aufruf ist gestrichen; `FLAC__stream_decoder_reset()` steht jetzt
      nur noch vor dem sequenziellen Pfad (`src/audio/AudioDecoder_Flac.cpp:528`). Der Zustand des
      Readers wird weiterhin direkt nach dem fehlgeschlagenen Sprung bereinigt, weil `seek_absolute()`
      ueber die Betriebs-Callbacks `error` und `ioError` gesetzt haben kann; das sagt jetzt auch der
      Kommentar an dieser Stelle.
- [x] **Dokumentation verspricht eine Abtastraten-Pruefung, die es nicht gibt**
      (`include/pplib/audio/decoder.h:684`)
      Die Doku zu `open()` sagt, der Decoder pruefe "Kanalzahl, Bitbreite und Abtastrate", der
      Klassenkommentar (`:612` bis `:617`) verspricht eine oder zwei Kanaele und 16 oder 24 Bit.
      Gepueft werden das Format (`:386`), Kanaelzahl ausserhalb 1 bis 2 (`:387`), Bitbreite ausser
      16 und 24 (`:388`, 8 Bit faengt diese Pruefung mit ab) und FLOAT (`:392`), dazu die
      UEbereinstimmung von STREAMINFO mit dem mitgebrachten
      `AudioInfo` (`:450`). Die Abtastrate wird nirgends verglichen, eine Datei mit abweichender
      RATE wird genommen. **[Pruefen]** Entweder die Abtastrate mitpruefen (libFLAC liefert sie in
      `metadata->data.stream_info.sample_rate`, fuer einen Vergleich fehlt in `FlacState` aber ein
      Feld) oder die Doku auf Kanalzahl und Bitbreite einschraenken.

      **Umsetzung:** Verifiziert: `open()` vergleicht Format, Kanaelzahl, Bitbreite, SampleTyp und
      STREAMINFO gegen `AudioInfo`, die Abtastrate nirgends. Gewaehlt wurde die Doku-Variante. Nach
      Rueckmeldung des Nutzers ist der Punkt damit auch inhaltlich erledigt: Die Spec legt die Rate
      im STREAMINFO-Block fuer den gesamten Stream fest, `IdentAudioFile()` liest sie dort
      (`src/audio/AudioInfo.cpp:428`, `:466`), und der Decoder gibt sie mit `info = *src`
      unveraendert weiter. Der Halbsatz "die Abtastrate wird nicht verglichen" ist ersatzlos
      gestrichen (`include/pplib/audio/decoder.h:699` bis `:702`); noetige Doku bleibt der Satz,
      dass die Rate aus dem STREAMINFO-Block stammt und fuer den ganzen Stream gilt.

## Wartbarkeit & Aufräumen

- [x] **Zwei unterschiedliche Reset-Zustaende in `seekSample()`**
      (`src/audio/AudioDecoder_Flac.cpp:496` bis `:502` und `:513` bis `:519`): direkte Quelle des
      Haengt-Befundes oben, Zusammenfassung im Abschnitt Strukturvorschlaege.

      **Umsetzung:** Behoben durch den oben genannten Fix: beide Bloecke setzen jetzt
      `atEnd`, `error`, `ioError`, `totalFrames` und den Puffer zurueck. Die Zusammenlegung der
      Bloecke (Abschnitt Strukturvorschlaege) ist bewusst nicht umgesetzt.

- [ ] **Die Position des Dekoderstroms wird an zwei Stellen unterschiedlich begruendet**
      (`src/audio/AudioDecoder_Flac.cpp:36` bis `:38` und `:494` bis `:495`): Der eine Kommentar
      sagt, der Dateiposition werde nicht hinterhergearbeitet, der andere begruendet genau damit,
      dass man sie nicht verlieren duerfe. Ein Kommentar, der den Mechanismus nennt (sequenzieller
      Pfad setzt alles zurueck, Zielsprung bei intaktem Zugriff bleibt erhalten), erspart die
      naechste Verwechslung.

      **Umsetzung nicht wie beschrieben:** Der Befund trifft in den Code nicht zu. `:36` bis `:38`
      ist die Zustandsbeschreibung des verskraenkten Rohpuffers, `:494` bis `:495` enthaelt keinen
      Kommentar zur Dateiposition; eine Suche nach "Dateiposition" trifft in der .cpp ueberhaupt
      nicht. Der gewuenschte Mechanismus-Satz steht jetzt im Kommentar von `seekSample()`
      (`src/audio/AudioDecoder_Flac.cpp:491` bis `:493`) und in der neuen Beschreibung von
      `fillFrames()`; der Haken bleibt wegen des unpassenden Befunds aus.

- [x] **Uneinheitliche Schreibweise in Kommentaren**: `Unterdriickungsregel`
      (`src/audio/AudioDecoder_Flac.cpp:502`), `exclusiv` (`include/pplib/audio/decoder.h:695`)
      neben `exklusiv` (`:650`, `:652`), "Metadaten-Blöcke ( Padding, ...)" mit ueberfluessigem
      Leerzeichen (`:619`). `Abtstrate` und `Metedaten` aus dem alten Code kommen nirgends mehr
      vor. Rein kosmetisch.

      **Umsetzung:** `Unterdriickungsregel` zu "Unterdückungsregel" berichtigt
      (`src/audio/AudioDecoder_Flac.cpp:507`), `exclusiv` zu "exklusiv" und das Leerzeichen nach der
      Klammer in "Metadaten-Blöcke (Padding, ...)" entfernt (`include/pplib/audio/decoder.h:619`
      und `:697`).

- [x] **Der gemeinsame Transferkern `transferFrames()` verzweigt in der innersten Schleife**
      (auf Zuruf des Nutzers, nicht aus dem urspruenglichen Befundskatalog): Alle vier
      Lesemethoden riefen denselben Kern mit drei Zielzeigern und einem `add`-Schalter auf
      (`src/audio/AudioDecoder_Flac.cpp:605` bis `:680` in der besprochenen Fassung). Pro Sample
      wurden Zielzeiger, Bitbreite und Zuweisung oder Zuendung entschieden; der Zuweisungszweig des
      32-Bit-Ziels war ueber die oeffentliche Schnittstelle ueberhaupt nicht erreichbar.

      **Umsetzung:** `transferFrames()` ist gestrichen, die Struktur entspricht jetzt
      `src/audio/AudioDecoder_Wave.cpp`: `prepareRead()` (`src/audio/AudioDecoder_Flac.cpp:618`)
      prueft die Voraussetzungen und stellt Frames bereit, `claimFrames()` (`:646`) zaehlt weiter,
      und jede der vier Lesemethoden (`:666`, `:703`, `:738`, `:771`) schaltet vor ihrer Schleife
      ueber das neue Feld `encoding` (`FrameEncoding::INT16`/`INT24`, gesetzt in `open()`); in der
      innersten Schleife stehen nur noch Kopie oder Addition und bei 24 Bit das Schieben. Damit
      faellt auch der tote Zuweisungszweig weg. Verhalten unveraendert: alle bisherigen FLAC-Tests
      blieben unveraendert gruuen; neu ist
      `AudioDecoderFlacTest.MonoFileFeedsEveryTargetLoopWithDuplicatedChannel`, das die jetzt
      getrennten Schleifen an einer Monodatei gegeneinander abgleicht.

## Dokumentation & Doxygen

- [x] **Der Kommentar zu `fillFrames()` verschweigt den Positionsmechanismus**
      (`src/audio/AudioDecoder_Flac.cpp:546` bis `:550`): `fillFrames()` synchronisiert an der
      Dateiposition nichts; ein Sprung innerhalb der Puffergroessenordnung verschiebt die Position
      des Dekoderstroms nicht. Das Verhalten ist richtig und durch
      `DecodingLeavesTheFileOpenAndPositionUseable` abgedeckt, aber die Kommentierung der Klasse
      (`:36` bis `:38`) und des `@note` in `decoder.h:724` beschreiben es unterschiedlich. Kein
      Programmfehler.

      **Umsetzung:** Der Kommentar von `fillFrames()` nennt den Mechanismus jetzt selbst
      (`src/audio/AudioDecoder_Flac.cpp:552` bis `:555`): Die Dateiposition wird nicht nachgefuehrt,
      libFLAC steht am Ende des zuletzt gelieferten Rahmens.

- [x] **Begruendung fuer die Kompaktierung in `transferFrames()` fehlt**
      (`src/audio/AudioDecoder_Flac.cpp:665` bis `:669`): Nach der Uebernahme werden Lese- und
      Schreibzeiger auf null zurueckgesetzt und der Puffer geleert, dieselbe Stelle gibt es in
      `fillFrames()` (`:557` bis `:561`) mit Begruendung (`:554` bis `:556`: Kapazitaet bleibt
      erhalten, Speicher wird wiedergenutzt). An der zweiten Stelle fehlt der Satz; ohne den
      Schritt wuerden die Zeiger mit der Zahl der gelesenen Frames weiterlaufen und der Vektor
      immer weiter anwachsen. Ein Halbsatz dort reicht.

      **Umsetzung:** Halbsatz am Ruecksetzblock ergaenzt: Zeiger wachsen sonst mit der Zahl der
      gelesenen Frames, die Kapazitaet bleibt erhalten. Der Block sitzt nach dem Umbau der
      Lesemethoden (siehe Befund unten) in `claimFrames()` (`src/audio/AudioDecoder_Flac.cpp:646`
      bis `:660`).

- [x] **`@param` beschreibt eine Bitrate-Berechnung, die der Dekoder nicht ausfuehrt**
      (`include/pplib/audio/decoder.h:686`): "Die Bitrate wird_moeglichst aus den Metadaten des
      STREAMINFO-Blocks ermittelt, nicht aus der Dateigroesse geschaetzt." Die Bitrate steht nach
      dem Oeffnen in `info`, aber `open()` setzt sie nicht; sie kommt aus `info = *src`
      (`src/audio/AudioDecoder_Flac.cpp:458`), also vom `AudioInfo`-Objekt, das sie in
      `src/audio/AudioInfo.cpp` selbst aus den Metadaten liest. Am Verhalten aendert sich nichts,
      nur der Satzbau der Doku ist irrefuehrend: "wird ermittelt" bezieht sich auf den Dekoder.
      **[Pruefen]** Doku praezisieren ("`info` uebernimmt die Bitrate aus den Metadaten des
      `AudioInfo`-Objekts") oder die Berechnung tatsaechlich in den Dekoder ziehen.

      **Umsetzung:** Verifiziert: `open()` setzt die Bitrate nicht selbst, uebernommen wird sie mit
      `info = *src` (`src/audio/AudioDecoder_Flac.cpp:458`); gelesen hat sie `AudioInfo.cpp`. Die
      Doku ist auf die Uebernahme umgestellt (`include/pplib/audio/decoder.h:686` bis `:688`), am
      Verhalten aendert sich nichts.

## Befunde in Drittdateien

- [x] **`SCOPED_TRACE` mit Zeigerarithmetik in `tests/src/audio/audiodecoder_wave.cpp:705`**
      `SCOPED_TRACE(w.bits + (w.tag == 3 ? " Bit float" : " Bit integer"));`: `w.bits` ist
      `uint16_t`, der Conditional-Ausdruck ein `const char*`. Der Plus-Operator promoviert
      `w.bits` auf `int` und verschiebt den Zeiger des 13 Byte langen Literals um 16, 24 oder 32
      Bytes aus dem Objekt heraus; `SCOPED_TRACE` baut daraus einen `std::string`, also `strlen()`
      jenseits des Literalendes. Verifiziert mit AddressSanitizer (`build/asan`, GCC 16.2.1) bei
      `--gtest_filter='AudioDecoderWaveTest*'`: `global-buffer-overflow`, READ of size 1 in
      `std::char_traits<char>::length`, aufgerufen von `testing::ScopedTrace::ScopedTrace`
      (`build/asan/_deps/googletest-src/googletest/include/gtest/gtest.h:2095`), naechster Frame
      ist unsere Teststelle. Der Test bleibt gruen, nur der Spurtext ist Unsinn. Vorbestehend seit
      `decf3f1` (Wave-Umarbeitung), hat mit den FLAC-Fixes nichts zu tun. Fix:
      ```cpp
      SCOPED_TRACE(std::to_string(w.bits) + (w.tag == 3 ? " Bit float" : " Bit integer"));
      ```

      **Umsetzung:** Genau dieser Fix steht jetzt in der Testdatei; `build/asan` meldet bei
      `--gtest_filter='AudioDecoderFlacTest*:AudioDecoderWaveTest*'` keine ASan-Meldung mehr,
      52 von 52 Tests sind gruen.

## Strukturvorschläge (nur zur Diskussion)

- **Reset in `seekSample()` auf eine Stelle ziehen** (`src/audio/AudioDecoder_Flac.cpp:496` bis
  `:502` und `:513` bis `:519`): Beide Zweige setzen denselben Zustand, nur der erste laesst
  `ioError` aus, und genau das ist der Haengt-Befund. Ein gemeinsamer Reset von `atEnd`, `error`,
  `ioError`, `totalFrames` und `seekTarget` vor der libFLAC-Aktion ist drei Zeilen kürzer und
  robust gegen kuenftige Folgezweige. Fuer den Fall "Zugriff ist gestoert, Dateiposition bitte
  behalten" waere ein eigenes Flag wie `ioBroken` noetig, das ausschliesslich die
  Fehlerunterdriickung in `flacErrorCb()` beeinflusst. Aufwand gering, Risiko gering, beide
  Zweige bleiben erhalten.

## Glossar

- **ABORT**: Rueckgabewert einer libFLAC-Callbackfunktion, die den Decoder abbrechen laesst.
- **ASan**: AddressSanitizer, Compiler-Instrumentierung `-fsanitize=address`.
- **Conditional-Ausdruck**: `bedingung ? a : b`, hier mit zwei String-Literalen als Zweige.
- **Endlosschleife, HANG**: Schleife ohne erfuülltes Abbruchkriterium; hier per `SIGALRM`-Timeout
  nachgewiesen, nicht ueber einen Stack-Trace.
- **Flanken**: partielle Frames an einer Sprunggrenze, die libFLAC am Seek-Ziel liefert.
- **LSan**: LeakSanitizer, prueft beim Prozessende auf nicht freigegebenen Speicher, laeuft mit
  ASan zusammen (`ASAN_OPTIONS=detect_leaks=1`).
- **Promotion, Zeigerarithmetik**: kleine Ganzzahlen werden vor der Rechnung auf `int`
  umgewandelt; `int + const char*` verschiebt den Zeiger, hier aus dem String-Literal heraus.
- **STREAMINFO**: Metadatenblock mit Abtastrate, Kanalzahl, Bitbreite, maximaler Blockgroesse und
  Gesamtzahl der Samples.
- **Sync-Verlust**: libFLAC meldet `BAD_SYNC`, wenn keine Rahmen-Signatur mehr kommt; trifft
  Anhaengsel wie ein ID3v1-Tag am Dateiende und echte Defekte gleichermassen.
- **UBSan**: UndefinedBehaviorSanitizer, `-fsanitize=undefined`.



