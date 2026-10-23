# Compression Review – Befunde

Review vom 2026-10-23, Scope: `include/pplib/core/compression.h` (611 Zeilen) + `src/core/Compression.cpp` (538 Zeilen) + `tests/src/core/compression.cpp` (525 Zeilen, 20 Tests).
Öffentliche API: `pplib::Compression` (Klasse), Enums `Algorithm`/`Level`/`Prefix`, Freie Funktionen `Compress`/`Uncompress`/`CompressZlib`/`CompressBZip2`.
Review done by: Claude (CodeReviewAgent)

Alle Tests laufen aktuell fehlerfrei (`test_core --gtest_filter='CompressionTest*'`: 20/20 PASSED).
Verifiziert gegen: `Poke8/16/24/32`, `Peek8/16/24/32` (`src/core/Functions.cpp`, little-endian, symmetrisch), `ByteArray::malloc` (allociert size+4, schreibt 4 abschließende NULs; `malloc(0)` liefert nicht-NULL), `ByteArray::truncate` (schreibt 4 NULs nach), Exceptions aus `include/pplib/exceptions.h`.
Empirisch verifiziert mit zlib/bzip2-Direkttests: `uncompress` bei abgeschnittenem Input → `Z_DATA_ERROR` (Wachstumsschleife terminiert, **kein** Endlosloop); bei zu kleinem Output-Puffer → `Z_BUF_ERROR` mit `destLen` = bereits geschriebene Bytes (nicht benötigte Größe); bzip2-Bound `size + size/100 + 600` war für alle getesteten Muster (random, alternierend, inkrementierend, Primstride) ausreichend.

## Bugs (kritisch)

Keine.

## Bugs (mittel / Warnungen)

- [x] **dstlen-Contract der BufferTooSmallException wird gebrochen** (`src/core/Compression.cpp:141`, `:180`, `:214`, `:238`; Doku: `include/pplib/core/compression.h:291`, `:380`)
  Die Header-Doku verspricht: „Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes." Tatsächlich wird aber nur die Zahl der *bis zum Exception-Wurf geschriebenen* Bytes zurückgegeben: `doZlib`/`unZlib` setzen `*dstlen = (size_t)dstlen_zlib` (Zeile 141/214), `doBzip2`/`unBzip2` analog `bz_dstlen` (Zeile 180/238) — und das **vor** der Auswertung des Fehlercodes.
  Empirisch bestätigt: 5000 Byte Quelldaten, Zielbuffer 100 Byte → Exception mit `dstlen == 100`, tatsächlich benötigt wären 319 Byte. Nur `doNone`/`unNone` (Zeile 196f) melden korrekt die benötigte Größe.
  Folge: Ein Aufrufer, der der Doku vertraut und seinen Puffer auf `dstlen` vergrößert, bekommt beim zweiten Versuch wieder eine Exception (ggf. endlos). Kein Test prüft den dstlen-Wert nach der Exception.
  Behebung: Entweder Doku anpassen (dstlen = „bereits geschriebene Bytes") oder die benötigte Größe ermitteln (zlib: zweiter `compress2`-Durchlauf mit `Z_BUF_ERROR`-Iteration bzw. `compressBound()` als obere Schranke; bzip2: Bound-Funktion) und *dann* werfen.

  **Umsetzung:** Doku-Variante, Code unverändert. Die Zusicherung in `include/pplib/core/compression.h` lautet jetzt überall „Anzahl der bereits in @p dst geschriebenen Bytes"; dass nur `doNone()`/`unNone()` die tatsächlich benötigte Größe melden, ist in der Exception-Doku der Low-Level-Funktionen ausdrücklich getrennt vermerkt. Der dstlen-Wert ist jetzt getestet: `CompressionTest.BufferTooSmallReportsWrittenBytes` (Low-Level und ByteArray-Overload, erwartet `dstlen == 100`).

- [x] **Stille 32-Bit-Trunkierung der Längenfelder bei > 4 GiB** (`src/core/Compression.cpp:303`, `:304`, `:343`, `:353`)
  V1 schreibt `Poke32(prefix + 1, (uint32_t)size)` und `Poke32(prefix + 5, (uint32_t)dstlen)`; V2 nutzt maximal 4-Byte-Felder. Bei Daten > 4 GiB (bzw. > 16 MiB in den 3-Byte-V2-Feldern, die dann auf 4 Byte fallen — das ist korrekt) schneidet der Cast still Werte ab → der Header beschreibt falsche Längen, die Dekomprimierung liefert korrupte Daten ohne Fehlermeldung.
  Behebung: Vor dem Schreiben prüfen, ob `size`/`dstlen` ins Feld passen, sonst `IllegalArgumentException` (oder `CompressionFailedException`) werfen.

  **Umsetzung:** Über die neue öffentliche Konstante `Compression::MAX_DATA_SIZE` (4 GiB − 1) abgefangen: Low-Level-`compress()`/`uncompress()` und die Helfer `compressTo()`/`uncompressTo()` prüfen Ein- und Ausgangsgröße dagegen und werfen `IllegalArgumentException`, bevor ein Längenfeld beschrieben werden kann. Im V1-Encode-Pfad prüft zusätzlich `dstlen > 0xFFFFFFFF` explizit vor dem `Poke32`. Tests: `MaxDataSizeLimitLowLevel`, `MaxDataSizeLimitByteArray`.

- [x] **Win64/LLP64-Portabilität: stiller Datenverlust durch 32-Bit-zlib-Typen** (`src/core/Compression.cpp:51`, `:124`, `:140`, `:212`, `:213`)
  Unter Windows (LLP64) sind `uLong`/`uLongf` 32 Bit. Die Casts `(uLong)size`, `(uLongf)*dstlen` und `compressBound((uLong)size)` trunkieren still bei Blöcken > 4 GiB → zlib komprimiert/dekomprimiert nur einen Teil der Daten und meldet das Ergebnis als Erfolg.
  bzip2 ist unkritischer: `(int)size` / `(int)srclen` (Zeile 179/237) werden bei > 2 GiB negativ → `BZ_PARAM_ERROR` → Exception, also Fehler statt Korruption.
  Behebung: Auf Win64 Größen > 4 GiB (bzw. > 2 GiB für bzip2) von vornherein mit `IllegalArgumentException` ablehnen, oder `compress2`/`uncompress`-Ersatz mit 64-Bit-API (`deflate`/`inflate` mit `size_t`-Wrapper) verwenden.

  **Umsetzung:** Erste Variante: die `MAX_DATA_SIZE`-Prüfung greift vor allen zlib-Casts, damit wird > 4 GiB jetzt plattformunabhängig explizit abgelehnt statt still teilkomprimiert. bzip2: die Casts `(int)` → `unsigned int` in `doBzip2()`/`unBzip2()` berichtigt, wodurch die unnötige 2-GiB-Grenze des Signed-Casts entfällt. 64-Bit-`deflate`/`inflate`-Wrapper wurden bewusst nicht eingeführt.

- [ ] **Unbegrenzte Allokation aus fremdgesteuertem Header (DoS)** (`src/core/Compression.cpp:428`, `:462`-ish V2-Pendant)
  Beim Entpacken mit V1/V2-Prefix wird `size_unc` ungeprüft aus dem (angreifbaren) Header gelesen und direkt an `out.malloc(size_unc)` übergeben — bis zu 4 GiB aus 9 Byte Input. Für eingebettete Ziele (Pico) und bei Verarbeitung fremder Daten relevant.
  Behebung: Plausibilitätsgrenze (z. B. konfigurierbares Maximum oder Verhältnis `size_unc` zu `size_comp`) prüfen und sonst `CorruptedDataException` werfen.

  **Umsetzung:** Bewusst nicht umgesetzt (Entscheidung des Autors). Eine Verhältnis- oder Konfigurationsgrenze für `size_unc` gibt es weiterhin nicht; als absolute Obergrenze wirkt jetzt `Compression::MAX_DATA_SIZE`, die vor `out.malloc()` geprüft wird.

- [x] **V1-Decode ignoriert Versions-/Unused-Bits konträr zur Doku** (`src/core/Compression.cpp:423`, `:430`; Doku: `include/pplib/core/compression.h:74`)
  Die Format-Doku sagt für V1 „Bits 3-7: unbenutzt, müssen 0 sein", der Code prüft das nie. Empirisch: V1-Header mit `flag = 0x08` wird kommentarlos als Algo NONE akzeptiert (0 Bytes Ausgabe). Umgekehrt liefert `flag & 7` im Bereich 3–7 eine `UnsupportedFeatureException` statt der passenden `CorruptedDataException`.
  Behebung: `(flag & 0xF8) != 0` → `CorruptedDataException`; ungültige Algo-Bits (3–7) ebenfalls als `CorruptedDataException` (defekter Header, nicht fehlendes Feature) klassifizieren.

  **Umsetzung:** Genau so implementiert (`flag & 0xF8` bzw. Algo-Bits > `BZIP2` → `CorruptedDataException`). Zusätzlich ein Überlauf in derselben Größenprüfung behoben: verglichen wird jetzt `size_comp > size - 9` (V2 analog `size - bytes`) statt `size_comp + 9 > size`; der 32-Bit-Wert lieferte bei Blöcken < 10 Byte einen negativen Wert, wodurch die Prüfung durchfiel und `out.malloc()` mit Größen nahe `SIZE_MAX` versuchte zu allokieren. Tests: `PrefixV1FlagValidation`, `PrefixV1HugeLengthField`, `PrefixV2UnknownAlgorithm`.

- [x] **Undokumentierte `BufferTooSmallException` aus der ByteArray-`compress()`-Überladung** (`src/core/Compression.cpp:288`, `:297`)
  Beim Komprimieren gibt es — anders als beim Entpacken (`:405`-ff.) — keine Wachstums-/Retry-Schleife. Reicht `maxCompressedBound()` in einem pathologischen Fall nicht, entweicht eine `BufferTooSmallException` aus Overloads, deren Doku (`include/pplib/core/compression.h:305`-ff.) diese Exception nicht listet. Der Bound ist empirisch für alle getesteten Muster ausreichend; Restrisiko bleibt formal.
  Behebung: Entweder Retry-Schleife wie beim Entpacken einbauen oder Exception in `CompressionFailedException` übersetzen und/oder in der Doku dokumentieren.

  **Umsetzung:** Nur Doku: `@exception BufferTooSmallException` (Vermerk „defensiv, praktisch nicht erreichbar") in allen ByteArray-`compress()`-Overloads und den freien `Compress*`-Funktionen nachgetragen. Retry-Schleife im Komprimierpfad und Übersetzung in `CompressionFailedException` abgelehnt.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [x] **Doppel-Puffer im Prefix-Pfad: 2 Allokationen + überflüssiger memset + Extra-Kopie** (`src/core/Compression.cpp:294`-`:306`, `:356`)
  `std::vector<uint8_t> workbuf(maxbound)` wird value-initialisiert (memset über die komplette Bound-Größe), dann wird zusätzlich das Ziel-`ByteArray` allokiert und das Ergebnis per `memcpy` hinüberkopiert. Effizienter: direkt in `out.malloc(bytes + maxbound)` ab Offset `bytes` komprimieren, danach `truncate` — spart eine Allokation, den memset und eine Kopie. Relevant im Sinne von REFACTORING.md (Ressourceneffizienz, Pico-Tauglichkeit).

  **Umsetzung:** Umgesetzt. Die neuen privaten Helfer `Compression::compressTo()`/`Compression::uncompressTo()` komprimieren/dekomprimieren direkt in den per `out.malloc()` reservierten Zielbereich (V1: `out.malloc(maxbound + 9)`, Daten ab Offset 9; V2: analog hinter dem maximalen Header, danach `truncate` auf die tatsächliche Länge). `std::vector workbuf`, dessen Value-Initialisierung und der `memcpy` sind entfallen. Die Out-Parameter-Overloads rufen jetzt die Helfer statt `out = compress(in)` und vermeiden damit die Deep-Copy über `ByteArrayPtr`. Regressionstest: `OutParameterReplacesOldContent`.

- [x] **Duplizierter Code im `Prefix_None`-Zweig** (`src/core/Compression.cpp:283`-`:291`)

  Der `Prefix_None`-Zweig von `compress(ByteArrayPtr)` wiederholt fast wörtlich den Common-Path (Bound holen, malloc, low-level `compress`, truncate). Zusammenführung würde eine Änderung an zwei Stellen verhindern (z. B. oben genannte Retry-Logik).

  **Umsetzung:** Zusammengeführt: `compress(ByteArrayPtr)` und beide Out-Parameter-Overloads delegieren auf `compressTo()` (analog `uncompress()` → `uncompressTo()`), in dem der `Prefix::None`-Pfad und die Prefix-Pfade nur noch je einmal stehen; die Wachstums-/Retry-Schleife der Entpackroute liegt ebenfalls nur noch dort.

- [x] **`usePrefix()` und `setPrefix()` sind exakte Synonyme** (`include/pplib/core/compression.h:243`, `:252`)

  API-Ballast. Empfehlung: eine Variante als primär markieren, die andere mit `[[deprecated("use setPrefix()")]]` für einen späteren Major-Release kennzeichnen (nicht brechen — REFACTORING.md).

  **Umsetzung:** Im Major-Update ersatzlos gestrichen (kein `[[deprecated]]`). `setPrefix()`/`getPrefix()` sind die alleinige API; Verwender und Tests nutzen durchgängig `setPrefix()`.

- [ ] **`init()` validiert Parameter nicht** (`src/core/Compression.cpp:~70`)
  `init(Algorithm, Level)` akzeptiert beliebige Werte (z. B. `Algorithm(99)` oder `Level(50)`); der Fehler taucht erst verzögert beim `compress()`-Aufruf als `UnsupportedFeatureException` auf. Frühe Validierung (auch gegen `Unknown`) wäre fehlertoleranter.

  **Umsetzung:** Offen, bewusst noch nicht geändert. Der bestehende Test `CompressionTest.UnsupportedFeatureException` erwartet die bisherige Semantik (ungültiger Algorithmus erst beim Komprimieren). Eine Früherkennung wäre eine weitere Verhaltensänderung und müsste mit dem Test abgestimmt werden.

- [x] **Parametername weicht zwischen Header und Implementierung ab** (`include/pplib/core/compression.h:294` `size` vs. `src/core/Compression.cpp:254` `srclen`)

  Doxygen/IntelliSense zeigen den Header-Namen; inkonsistente Benennung über die Klasse hinweg (`srclen` nur in den Low-Level-Funktionen). Vereinheitlichen.

  **Umsetzung:** Vereinheitlicht: Low-Level-`compress()` heißt in Deklaration und Definition durchgängig `size`. `srclen` bleibt bewusst nur in den `uncompress()`-Funktionen, weil dort die Länge der komprimierten Quelle gemeint ist.

- [x] **Unscoped Enums** (`include/pplib/core/compression.h:114`, `:148`, `:162`)

  `enum class` wäre modern, bricht aber die öffentliche API (implizite Casts wie `(Algorithm)(flag & 7)` in `:430`/`:437` und existierender Nutzercode). Optionale Notiz für einen Major-Release; aktuell nicht ändern.

  **Umsetzung:** Im Major-Update umgesetzt: `enum class Algorithm { NONE = 0, ZLIB = 1, BZIP2 = 2, Unknown = 256 }`, `enum class Level { Fast, Normal, Default, High }`, `enum class Prefix { None = 0, V1, V2 }` — ohne `using enum`, alle Namen wie freigegeben. Die binär serialisierten Werte bleiben identisch (u. a. `PFPFile` Offset 16); Bitmasken laufen über `static_cast<unsigned>`. Verwender angepasst: `src/core/Compression.cpp`, `src/core/PFPFile.cpp`, `include/pplib/core/pfpfile.h`, `src/core/Resource.cpp`, `src/grafix/Sprite.cpp`, `src/grafix/ImageFilter_PNG.cpp`, `todo/internet/SocketMessage.cpp` sowie die Tests.

## Dokumentation & Doxygen

- [x] **Falsche dstlen-Zusicherung an zwei Stellen** (`include/pplib/core/compression.h:291`, `:380`)

  Siehe Bug „dstlen-Contract" — entweder Code korrigieren oder Doku auf „bereits geschriebene Bytes" umformulieren. Beides aktuell unentschieden, Befund doppelt relevant.

  **Umsetzung:** Doku an beiden Low-Level-Überladungen auf „bereits geschriebene Bytes" umformuliert; der irreführende Satz („… enthält nach Auftreten der Exception die tatsächlich benötigten Bytes") ist entfernt.

- [x] **`Unknown = 256` nur halbvollständig dokumentiert** (`include/pplib/core/compression.h:139`)

  Kommentar sagt „Wird als Defaulteinstellung beim **Dekomprimieren** verwendet" — tatsächlich ist `Unknown` auch Default-Parameter der Low-Level-`compress()`-Überladung (`:294`) und wirkt dort ebenso (Override des eingestellten Algorithmus).

  **Umsetzung:** Doku ergänzt: `Unknown` bedeutet jeweils „Algorithmus der Instanz verwenden" und ist Default beider Low-Level-Überladungen (`compress()` und `uncompress()`); ein explizit übergebener Wert überschreibt die Instanzeinstellung.

- [x] **`@exception BufferTooSmallException` fehlt bei den ByteArray-`compress()`-Overloads** (`include/pplib/core/compression.h:305`-ff.)

  Die Overload-Doku listet nur `UnsupportedFeature`/`OutOfMemory`/`CompressionFailed`; die Exception kann aber über die Low-Level-Route entweichen (siehe Warnungs-Befund).

  **Umsetzung:** In allen vier ByteArray-`compress()`-Overloads ergänzt (Formulierung mit Vermerk „defensiv, praktisch nicht erreichbar").

- [x] **Freie Funktionen ohne `@exception`-Abschnitt** (`include/pplib/core/compression.h:473`, `:493`, `:517`, `:530`, `:550`-`:607`)

  `Compress`/`Uncompress`/`CompressZlib`/`CompressBZip2` und ihre Out-Parameter-Varianten dokumentieren keinerlei Würfe (z. B. `CorruptedDataException` bei `Uncompress` mit defektem V2-Header, `UnsupportedFeatureException` wenn Zlib/Bzip2 rauskompiliert sind). Für IntelliSense/Doxygen nachrüsten.

  **Umsetzung:** `@exception`-Blöcke in allen freien Funktionen nachgetragen (`Compress`, `CompressZlib`, `CompressBZip2` und die zugehörigen Dekompressions-/Out-Parameter-Varianten): `UnsupportedFeatureException` (Algorithmus rauskompiliert bzw. nicht unterstützt), `OutOfMemoryException`, `CompressionFailedException`, `BufferTooSmallException` (Komprimierrichtung, defensiv), `IllegalArgumentException` (`MAX_DATA_SIZE`) sowie `CorruptedDataException` in der Dekomprimierrichtung.

## Bewertung der Unit-Tests (`tests/src/core/compression.cpp`)

**Stärken (insgesamt gute Grundabdeckung):**
- Roundtrips für alle drei Algorithmen, alle vier Level, beide Prefix-Versionen; V2 mit variablen Feldgrößen (`PrefixV2VariableSizes`).
- `IncompressibleDataBoundSafety` (Bound-Verhalten bei Zufallsdaten), `LargeBufferExponentialGrowth` (übt die Wachstums-/Retry-Schleife im Entpackpfad), `EmptyDataHandling`.
- Fehlerpfade: illegale Argumente (`NULL`-Pointer), korrupte Daten inkl. Header-Kantfälle (zu kurzer V1/V2-Header, Bit-3 nicht gesetzt), Algorithmus 999 → `UnsupportedFeatureException`.
- Copy/Move-Semantik, Out-Parameter-Overloads, freie Funktionen werden geprüft.

**Lücken (getestet wurde gegen alle 20 Tests; konkrete Vorschläge):**
- [x] **dstlen-Wert nach `BufferTooSmallException` wird nie geprüft** (`tests/src/core/compression.cpp:371` `LowLevelBufferTooSmallException`)

  Der Test prüft nur das Werfen. Eine `EXPECT_EQ`/`EXPECT_GE` auf `dstlen` würde den oben beschriebenen Doku-Verbruch sofort aufdecken — wichtigster neuer Test.

  **Umsetzung:** Neuer Test `CompressionTest.BufferTooSmallReportsWrittenBytes`: Low-Level-`compress()` in einen 100-Byte-Puffer (Zufallsdaten, 5000 Byte Quelle) erwartet `dstlen == 100`, analog der ByteArray-Overload mit vorgegebenem Zielbuffer.
- [x] **Abgeschnittener komprimierter Stream ohne Prefix fehlt** — der Pfad „Input defekt, Output groß genug" (Z_DATA_ERROR nach mehreren Puffer-Verdopplungen) ist ungetestet; empirisch terminiert er korrekt mit `CorruptedDataException`, das sollte als Regressionstest fixiert werden.

  **Umsetzung:** Neuer Test `CompressionTest.TruncatedStreamNoPrefix`: abgeschnittener Zlib-Stream bei ausreichend großem Ziel, erwartet `CorruptedDataException`; terminiert in endlicher Zeit.
- [x] **Kein BZIP2-Roundtrip mit leeren Daten, kein V1-Roundtrip mit leeren Daten** (`EmptyDataHandling` deckt nur Teilfälle ab).

  **Umsetzung:** In `EmptyDataHandling` ergänzt: BZIP2 ohne Prefix und BZIP2 mit `Prefix::V1`. Zusätzlich der neue Test `PrefixV1EmptyRoundtrip` (NONE, ZLIB, BZIP2 mit V1-Header, Ergebnisgröße 0).
- [x] **V2-4-Byte-Längenfeld (b_unc/b_comp = 3) ungetestet** — erfordert > 16 MiB Quelldaten; als langsamer Test (`GTEST_SKIP`-fähig oder per Umgebungsvariable steuerbar) nachrüsten. Ebenso wird das V1-Feld „Bytes Komprimiert" (`Peek32(buffer+5)`) nie gegen einen Sollwert geprüft.

  **Umsetzung:** Drei neue Tests: `PrefixV2ThreeByteLengthFields` (von Hand gebauter Header mit 3-Byte-Feldern), `PrefixV2FourByteLengthFields` (von Hand gebaut, 4-Byte-Felder) und `PrefixV2FourByteEncodeFields` (echter Roundtrip mit 16 MiB + 1 Byte Zufallsdaten, prüft `b_unc`/`b_comp` = 3 im Flag; dauert ca. 0,8 s). Das V1-Feld „Bytes komprimiert" wird jetzt in `PrefixV1Roundtrip` gegen `compressed.size() - 9` geprüft. Damit sind auch die Encode-Zweige `:303`, `:304`, `:343`, `:353` abgedeckt.
- [x] **`Unknown`-Default nie explizit getestet** — `LowLevelApi` (`:354`) übergibt immer explizit `Algo_ZLIB`. Ein Test „Instanz auf ZLIB init, aber `Algo_BZIP2` übergeben" (Override) und „Parameter `Unknown` → Instanz-Algorithmus" fehlt.

  **Umsetzung:** Neuer Test `CompressionTest.LowLevelAlgorithmOverride`: Instanz auf ZLIB, explizit `Algorithm::BZIP2` übergeben (Roundtrip nur mit BZIP2 dekomprimierbar) und derselbe Aufruf mit `Algorithm::Unknown` (nutzt den eingestellten ZLIB).
- [x] **V2-Prefix nur mit ZLIB getestet** — V2 + NONE und V2 + BZIP2 fehlen; ebenso gemischte Konstellationen (mit V1 komprimiert, mit V2 gelesen → `CorruptedDataException`).

  **Umsetzung:** Neue Tests `PrefixV2RoundtripNoneAndBzip2` (V2 mit NONE und BZIP2, inkl. Flag-/Header-Sollwerte) und `PrefixV1DataReadAsV2` (V1-Daten mit V2-Leser → `CorruptedDataException`).
- [x] **`usePrefix()`/`setPrefix()` nur über den Getter geprüft** — ein echter Roundtrip mit `Prefix_V1` über die Instanz-Konfiguration (nicht nur `PrefixV1Roundtrip`-Einzeltest) und `init()` mit unzulässigem Algorithmus sind ungetestet.

  **Umsetzung:** Teilweise: `PrefixV1Roundtrip` läuft jetzt über die Instanz-Konfiguration (`Compression comp(algo); comp.setPrefix(Prefix::V1)`) für alle drei Algorithmen und prüft Flag sowie beide Längenfelder gegen Sollwerte. `usePrefix()` ist entfallen, wird also nicht mehr getestet. `init()` mit unzulässigem Algorithmus bleibt ungetestet (siehe Befund „`init()` validiert Parameter nicht").
- [x] **Freie Funktion `Uncompress()` mit korruptem V2-Header** ungetestet (`StandaloneFunctions:477` nur Happy-Path).

  **Umsetzung:** Neuer Test `StandaloneUncompressCorruptHeader`: freies `pplib::Uncompress()` mit defektem V2-Header erwartet `CorruptedDataException`.

## Befunde in Drittdateien

- [ ] **`ByteArray::malloc(0)` liefert nicht-NULL und reserviert size+4** (`src/types/ByteArray.cpp`)
  Für die Compression-Klasse relevant, weil leere Prefix-Roundtrips (`out.malloc(0)` + `truncate`) dadurch definiert laufen. Verhalten ist korrekt, aber undocumented in `include/pplib/types/bytearray.h` — die +4-NUL-Semantik und „malloc(0) ≠ NULL" gehören dort in die Doku.

  **Umsetzung:** Nicht umgesetzt — außerhalb des Scopes dieses Reviews (bearbeitet wurde nur die Klasse `Compression`). Die Doku in `include/pplib/types/bytearray.h` bleibt als eigener Befund offen.
