# Compression Review – Befunde

Review vom 2026-10-23, Scope: `include/pplib/core/compression.h` (611 Zeilen) + `src/core/Compression.cpp` (538 Zeilen) + `tests/src/core/compression.cpp` (525 Zeilen, 20 Tests).
Öffentliche API: `pplib::Compression` (Klasse), Enums `Algorithm`/`Level`/`Prefix`, Freie Funktionen `Compress`/`Uncompress`/`CompressZlib`/`CompressBZip2`.
Review done by: Claude (CodeReviewAgent)

Alle Tests laufen aktuell fehlerfrei (`test_core --gtest_filter='CompressionTest*'`: 20/20 PASSED).
Verifiziert gegen: `Poke8/16/24/32`, `Peek8/16/24/32` (`src/core/Functions.cpp`, little-endian, symmetrisch), `ByteArray::malloc` (allociert size+4, schreibt 4 abschließende NULs; `malloc(0)` liefert nicht-NULL), `ByteArray::truncate` (schreibt 4 NULs nach), Exceptions aus `include/pplib/exceptions.h`.
Empirisch verifiziert mit zlib/bzip2-Direkttests: `uncompress` bei abgeschnittenem Input → `Z_DATA_ERROR` (Wachstumsschleife terminiert, **kein** Endlosloop); bei zu kleinem Output-Puffer → `Z_BUF_ERROR` mit `destLen` = bereits geschriebene Bytes (nicht benötigte Größe); bzip2-Bound `size + size/100 + 600` war für alle getesteten Muster (random, alternierend, inkrementierend, Primstride) ausreichend.

## Umsetzung (2026-10-23, PPL8-Major-Update)

Alle Funde umgesetzt außer den unter **Nicht umgesetzt** aufgeführten.
Tests: 36 `CompressionTest`-Tests grün; `test_core` 2010 Tests (1 vorbestehender Skip), `test_crypto` 54/54.
Coverage `src/core/Compression.cpp`: Lines 100 %, Branches 93,9 %.

**Behoben / umgesetzt**

- *dstlen-Contract:* Doku korrigiert – `dstlen` enthält bei der Exception die bereits geschriebenen Bytes, nur bei `Algorithm::NONE` die tatsächlich benötigte Größe. Code bleibt unverändert, dstlen wird jetzt getestet.
- *Stille 32-Bit-Trunkierung / Win64-LLP64:* neue öffentliche Konstante `Compression::MAX_DATA_SIZE` (4 GiB − 1). Low-Level-`compress()`/`uncompress()` und alle ByteArray-Pfade prüfen Quell- und Ziellänge gegen diesen Wert und werfen `IllegalArgumentException`. bzip2-Casts `(int)` → `(unsigned int)`, damit entfällt die harte 2-GiB-Grenze.
- *V1-Decode:* reservierte Bits (`flag & 0xF8`) und unbekannte Algo-Bits (3–7) → `CorruptedDataException`. Zusätzlich Überlauf-Fix in den Größenprüfungen (`size_comp > size - 9` bzw. `size - bytes`); vorher lief die Prüfung bei 32 Bit über.
- *Undokumentierte `BufferTooSmallException`:* in allen `compress()`-Überladungen und freien `Compress*`-Funktionen dokumentiert (defensiv, die Bound ist garantiert).
- *Doppel-Puffer im Prefix-Pfad:* neue private Helfer `compressTo()`/`uncompressTo()` komprimieren direkt in das Ergebnis-`ByteArray` – kein `std::vector workbuf`, kein `memcpy`. Out-Parameter-Overloads rufen die Helfer statt `out = compress(in)` und vermeiden so die Deep-Copy.
- *Code Smells / Doku:* `usePrefix()` gestrichen, `HAVE_LIBZ` → `HAVE_ZLIB`, Parametername `srclen` → `size`, C-Casts → `static_cast`, `Unknown = 256` vollständig dokumentiert, `@exception`-Blöcke der freien Funktionen ergänzt.
- *Unscoped Enums:* `enum class Algorithm` / `Level` / `Prefix` mit neuen Namen (`NONE`, `ZLIB`, `BZIP2`, `Unknown`; `Fast`, `Normal`, `Default`, `High`; `None`, `V1`, `V2`) ohne `using enum`; alle Verwender angepasst (`PFPFile`, `Resource`, `Sprite`, `SocketMessage`). Die serialisierten Werte (u. a. `PFPFile` Offset 16) bleiben erhalten.
- *Tests:* dstlen-Wert nach `BufferTooSmallException`, abgeschnittener Stream ohne Prefix, leere Daten mit BZIP2 und mit V1, V2-3-Byte-Felder und 4-Byte-Felder (inkl. Roundtrip > 16 MiB), Sollwert des V1-Felds „Bytes komprimiert", Low-Level-Override und `Unknown`-Default, V2 mit NONE/BZIP2, mit V1 komprimierte und mit V2 gelesene Daten, freies `Uncompress()` mit korruptem Header, MAX_DATA_SIZE-Grenzen, Out-Parameter ersetzt alten Pufferinhalt.

**Nicht umgesetzt (bewusst)**

- DoS-Grenze für `size_unc` aus dem Header: bleibt unverändert (Bestandsentscheidung; Obergrenze ist `MAX_DATA_SIZE`).
- `init()` validiert weiterhin nicht – der bestehende Test erwartet, dass ein ungültiger Algorithmus erst beim Komprimieren als `UnsupportedFeatureException` erscheint. Offene Frage, ob hier auf `IllegalArgumentException` umgestellt werden soll.
- Retry-Schleife im Komprimierpfad, Streaming-API, `[[nodiscard]]`, `std::span`: abgelehnt.
- Doku zu `ByteArray::malloc(0)` in `include/pplib/types/bytearray.h`: außerhalb des Scopes (nur Klasse `Compression`).

## Bugs (kritisch)

Keine.

## Bugs (mittel / Warnungen)

- [ ] **dstlen-Contract der BufferTooSmallException wird gebrochen** (`src/core/Compression.cpp:141`, `:180`, `:214`, `:238`; Doku: `include/pplib/core/compression.h:291`, `:380`)
  Die Header-Doku verspricht: „Der Parameter @p dstlen enthält nach Auftreten der Exception die tatsächlich benötigten Bytes." Tatsächlich wird aber nur die Zahl der *bis zum Exception-Wurf geschriebenen* Bytes zurückgegeben: `doZlib`/`unZlib` setzen `*dstlen = (size_t)dstlen_zlib` (Zeile 141/214), `doBzip2`/`unBzip2` analog `bz_dstlen` (Zeile 180/238) — und das **vor** der Auswertung des Fehlercodes.
  Empirisch bestätigt: 5000 Byte Quelldaten, Zielbuffer 100 Byte → Exception mit `dstlen == 100`, tatsächlich benötigt wären 319 Byte. Nur `doNone`/`unNone` (Zeile 196f) melden korrekt die benötigte Größe.
  Folge: Ein Aufrufer, der der Doku vertraut und seinen Puffer auf `dstlen` vergrößert, bekommt beim zweiten Versuch wieder eine Exception (ggf. endlos). Kein Test prüft den dstlen-Wert nach der Exception.
  Behebung: Entweder Doku anpassen (dstlen = „bereits geschriebene Bytes") oder die benötigte Größe ermitteln (zlib: zweiter `compress2`-Durchlauf mit `Z_BUF_ERROR`-Iteration bzw. `compressBound()` als obere Schranke; bzip2: Bound-Funktion) und *dann* werfen.

- [ ] **Stille 32-Bit-Trunkierung der Längenfelder bei > 4 GiB** (`src/core/Compression.cpp:303`, `:304`, `:343`, `:353`)
  V1 schreibt `Poke32(prefix + 1, (uint32_t)size)` und `Poke32(prefix + 5, (uint32_t)dstlen)`; V2 nutzt maximal 4-Byte-Felder. Bei Daten > 4 GiB (bzw. > 16 MiB in den 3-Byte-V2-Feldern, die dann auf 4 Byte fallen — das ist korrekt) schneidet der Cast still Werte ab → der Header beschreibt falsche Längen, die Dekomprimierung liefert korrupte Daten ohne Fehlermeldung.
  Behebung: Vor dem Schreiben prüfen, ob `size`/`dstlen` ins Feld passen, sonst `IllegalArgumentException` (oder `CompressionFailedException`) werfen.

- [ ] **Win64/LLP64-Portabilität: stiller Datenverlust durch 32-Bit-zlib-Typen** (`src/core/Compression.cpp:51`, `:124`, `:140`, `:212`, `:213`)
  Unter Windows (LLP64) sind `uLong`/`uLongf` 32 Bit. Die Casts `(uLong)size`, `(uLongf)*dstlen` und `compressBound((uLong)size)` trunkieren still bei Blöcken > 4 GiB → zlib komprimiert/dekomprimiert nur einen Teil der Daten und meldet das Ergebnis als Erfolg.
  bzip2 ist unkritischer: `(int)size` / `(int)srclen` (Zeile 179/237) werden bei > 2 GiB negativ → `BZ_PARAM_ERROR` → Exception, also Fehler statt Korruption.
  Behebung: Auf Win64 Größen > 4 GiB (bzw. > 2 GiB für bzip2) von vornherein mit `IllegalArgumentException` ablehnen, oder `compress2`/`uncompress`-Ersatz mit 64-Bit-API (`deflate`/`inflate` mit `size_t`-Wrapper) verwenden.

- [ ] **Unbegrenzte Allokation aus fremdgesteuertem Header (DoS)** (`src/core/Compression.cpp:428`, `:462`-ish V2-Pendant)
  Beim Entpacken mit V1/V2-Prefix wird `size_unc` ungeprüft aus dem (angreifbaren) Header gelesen und direkt an `out.malloc(size_unc)` übergeben — bis zu 4 GiB aus 9 Byte Input. Für eingebettete Ziele (Pico) und bei Verarbeitung fremder Daten relevant.
  Behebung: Plausibilitätsgrenze (z. B. konfigurierbares Maximum oder Verhältnis `size_unc` zu `size_comp`) prüfen und sonst `CorruptedDataException` werfen.

- [ ] **V1-Decode ignoriert Versions-/Unused-Bits konträr zur Doku** (`src/core/Compression.cpp:423`, `:430`; Doku: `include/pplib/core/compression.h:74`)
  Die Format-Doku sagt für V1 „Bits 3-7: unbenutzt, müssen 0 sein", der Code prüft das nie. Empirisch: V1-Header mit `flag = 0x08` wird kommentarlos als Algo NONE akzeptiert (0 Bytes Ausgabe). Umgekehrt liefert `flag & 7` im Bereich 3–7 eine `UnsupportedFeatureException` statt der passenden `CorruptedDataException`.
  Behebung: `(flag & 0xF8) != 0` → `CorruptedDataException`; ungültige Algo-Bits (3–7) ebenfalls als `CorruptedDataException` (defekter Header, nicht fehlendes Feature) klassifizieren.

- [ ] **Undokumentierte `BufferTooSmallException` aus der ByteArray-`compress()`-Überladung** (`src/core/Compression.cpp:288`, `:297`)
  Beim Komprimieren gibt es — anders als beim Entpacken (`:405`-ff.) — keine Wachstums-/Retry-Schleife. Reicht `maxCompressedBound()` in einem pathologischen Fall nicht, entweicht eine `BufferTooSmallException` aus Overloads, deren Doku (`include/pplib/core/compression.h:305`-ff.) diese Exception nicht listet. Der Bound ist empirisch für alle getesteten Muster ausreichend; Restrisiko bleibt formal.
  Behebung: Entweder Retry-Schleife wie beim Entpacken einbauen oder Exception in `CompressionFailedException` übersetzen und/oder in der Doku dokumentieren.

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Doppel-Puffer im Prefix-Pfad: 2 Allokationen + überflüssiger memset + Extra-Kopie** (`src/core/Compression.cpp:294`-`:306`, `:356`)
  `std::vector<uint8_t> workbuf(maxbound)` wird value-initialisiert (memset über die komplette Bound-Größe), dann wird zusätzlich das Ziel-`ByteArray` allokiert und das Ergebnis per `memcpy` hinüberkopiert. Effizienter: direkt in `out.malloc(bytes + maxbound)` ab Offset `bytes` komprimieren, danach `truncate` — spart eine Allokation, den memset und eine Kopie. Relevant im Sinne von REFACTORING.md (Ressourceneffizienz, Pico-Tauglichkeit).

- [ ] **Duplizierter Code im `Prefix_None`-Zweig** (`src/core/Compression.cpp:283`-`:291`)
  Der `Prefix_None`-Zweig von `compress(ByteArrayPtr)` wiederholt fast wörtlich den Common-Path (Bound holen, malloc, low-level `compress`, truncate). Zusammenführung würde eine Änderung an zwei Stellen verhindern (z. B. oben genannte Retry-Logik).

- [ ] **`usePrefix()` und `setPrefix()` sind exakte Synonyme** (`include/pplib/core/compression.h:243`, `:252`)
  API-Ballast. Empfehlung: eine Variante als primär markieren, die andere mit `[[deprecated("use setPrefix()")]]` für einen späteren Major-Release kennzeichnen (nicht brechen — REFACTORING.md).

- [ ] **`init()` validiert Parameter nicht** (`src/core/Compression.cpp:~70`)
  `init(Algorithm, Level)` akzeptiert beliebige Werte (z. B. `Algorithm(99)` oder `Level(50)`); der Fehler taucht erst verzögert beim `compress()`-Aufruf als `UnsupportedFeatureException` auf. Frühe Validierung (auch gegen `Unknown`) wäre fehlertoleranter.

- [ ] **Parametername weicht zwischen Header und Implementierung ab** (`include/pplib/core/compression.h:294` `size` vs. `src/core/Compression.cpp:254` `srclen`)
  Doxygen/IntelliSense zeigen den Header-Namen; inkonsistente Benennung über die Klasse hinweg (`srclen` nur in den Low-Level-Funktionen). Vereinheitlichen.

- [ ] **Unscoped Enums** (`include/pplib/core/compression.h:114`, `:148`, `:162`)
  `enum class` wäre modern, bricht aber die öffentliche API (implizite Casts wie `(Algorithm)(flag & 7)` in `:430`/`:437` und existierender Nutzercode). Optionale Notiz für einen Major-Release; aktuell nicht ändern.

## Dokumentation & Doxygen

- [ ] **Falsche dstlen-Zusicherung an zwei Stellen** (`include/pplib/core/compression.h:291`, `:380`)
  Siehe Bug „dstlen-Contract" — entweder Code korrigieren oder Doku auf „bereits geschriebene Bytes" umformulieren. Beides aktuell unentschieden, Befund doppelt relevant.

- [ ] **`Unknown = 256` nur halbvollständig dokumentiert** (`include/pplib/core/compression.h:139`)
  Kommentar sagt „Wird als Defaulteinstellung beim **Dekomprimieren** verwendet" — tatsächlich ist `Unknown` auch Default-Parameter der Low-Level-`compress()`-Überladung (`:294`) und wirkt dort ebenso (Override des eingestellten Algorithmus).

- [ ] **`@exception BufferTooSmallException` fehlt bei den ByteArray-`compress()`-Overloads** (`include/pplib/core/compression.h:305`-ff.)
  Die Overload-Doku listet nur `UnsupportedFeature`/`OutOfMemory`/`CompressionFailed`; die Exception kann aber über die Low-Level-Route entweichen (siehe Warnungs-Befund).

- [ ] **Freie Funktionen ohne `@exception`-Abschnitt** (`include/pplib/core/compression.h:473`, `:493`, `:517`, `:530`, `:550`-`:607`)
  `Compress`/`Uncompress`/`CompressZlib`/`CompressBZip2` und ihre Out-Parameter-Varianten dokumentieren keinerlei Würfe (z. B. `CorruptedDataException` bei `Uncompress` mit defektem V2-Header, `UnsupportedFeatureException` wenn Zlib/Bzip2 rauskompiliert sind). Für IntelliSense/Doxygen nachrüsten.

## Bewertung der Unit-Tests (`tests/src/core/compression.cpp`)

**Stärken (insgesamt gute Grundabdeckung):**
- Roundtrips für alle drei Algorithmen, alle vier Level, beide Prefix-Versionen; V2 mit variablen Feldgrößen (`PrefixV2VariableSizes`).
- `IncompressibleDataBoundSafety` (Bound-Verhalten bei Zufallsdaten), `LargeBufferExponentialGrowth` (übt die Wachstums-/Retry-Schleife im Entpackpfad), `EmptyDataHandling`.
- Fehlerpfade: illegale Argumente (`NULL`-Pointer), korrupte Daten inkl. Header-Kantfälle (zu kurzer V1/V2-Header, Bit-3 nicht gesetzt), Algorithmus 999 → `UnsupportedFeatureException`.
- Copy/Move-Semantik, Out-Parameter-Overloads, freie Funktionen werden geprüft.

**Lücken (getestet wurde gegen alle 20 Tests; konkrete Vorschläge):**
- [ ] **dstlen-Wert nach `BufferTooSmallException` wird nie geprüft** (`tests/src/core/compression.cpp:371` `LowLevelBufferTooSmallException`)
  Der Test prüft nur das Werfen. Eine `EXPECT_EQ`/`EXPECT_GE` auf `dstlen` würde den oben beschriebenen Doku-Verbruch sofort aufdecken — wichtigster neuer Test.
- [ ] **Abgeschnittener komprimierter Stream ohne Prefix fehlt** — der Pfad „Input defekt, Output groß genug" (Z_DATA_ERROR nach mehreren Puffer-Verdopplungen) ist ungetestet; empirisch terminiert er korrekt mit `CorruptedDataException`, das sollte als Regressionstest fixiert werden.
- [ ] **Kein BZIP2-Roundtrip mit leeren Daten, kein V1-Roundtrip mit leeren Daten** (`EmptyDataHandling` deckt nur Teilfälle ab).
- [ ] **V2-4-Byte-Längenfeld (b_unc/b_comp = 3) ungetestet** — erfordert > 16 MiB Quelldaten; als langsamer Test (`GTEST_SKIP`-fähig oder per Umgebungsvariable steuerbar) nachrüsten. Ebenso wird das V1-Feld „Bytes Komprimiert" (`Peek32(buffer+5)`) nie gegen einen Sollwert geprüft.
- [ ] **`Unknown`-Default nie explizit getestet** — `LowLevelApi` (`:354`) übergibt immer explizit `Algo_ZLIB`. Ein Test „Instanz auf ZLIB init, aber `Algo_BZIP2` übergeben" (Override) und „Parameter `Unknown` → Instanz-Algorithmus" fehlt.
- [ ] **V2-Prefix nur mit ZLIB getestet** — V2 + NONE und V2 + BZIP2 fehlen; ebenso gemischte Konstellationen (mit V1 komprimiert, mit V2 gelesen → `CorruptedDataException`).
- [ ] **`usePrefix()`/`setPrefix()` nur über den Getter geprüft** — ein echter Roundtrip mit `Prefix_V1` über die Instanz-Konfiguration (nicht nur `PrefixV1Roundtrip`-Einzeltest) und `init()` mit unzulässigem Algorithmus sind ungetestet.
- [ ] **Freie Funktion `Uncompress()` mit korruptem V2-Header** ungetestet (`StandaloneFunctions:477` nur Happy-Path).

## Befunde in Drittdateien

- [ ] **`ByteArray::malloc(0)` liefert nicht-NULL und reserviert size+4** (`src/types/ByteArray.cpp`)
  Für die Compression-Klasse relevant, weil leere Prefix-Roundtrips (`out.malloc(0)` + `truncate`) dadurch definiert laufen. Verhalten ist korrekt, aber undocumented in `include/pplib/types/bytearray.h` — die +4-NUL-Semantik und „malloc(0) ≠ NULL" gehören dort in die Doku.
