# DESIGN: Zstandard und Brotli in der Compression-Klasse

## Zweck

Die Klasse `pplib::Compression` um zwei weitere verlustfreie Algorithmen erweitern:

- `Algorithm::ZSTD` (Zstandard, BSD-Lizenz, Meta)
- `Algorithm::BROTLI` (Brotli, MIT-Lizenz, Google)

Beide werden exakt nach dem vorhandenen Muster von Zlib und Bzip2 angebunden:
One-Shot-Buffer-API, keine Streaming-API, keine neuen Klassen, keine neuen öffentlichen
Signaturen abgesehen von zwei Enumeratoren und zwei Convenience-Funktionen.

## Nicht-Ziele

- **Kein Streaming** (weder `ZSTD_decompressStream` noch `BrotliDecoderDecompressStream`).
  Die Wachstums­schleife in `uncompressTo()` bleibt unverändert.
- **Kein Dictionary-/Wörterbuch-Support**, keine Trainings-API, keine Frame-Parameter
  (windowLog, chainLog, lgblock) jenseits der Level-Abbildung.
- **Keine Änderung am Prefix-Format.** Die Bits 0-2 des Flags fassen Werte 0-7; belegt sind
  bisher 0-2. ZSTD = 3, BROTLI = 4. Bestehende komprimierte Daten bleiben lesbar.
- **Keine Mikrocontroller-Optimierung.** Desktop-Speicherbedarf von zstd/brotli wird akzeptiert.
- **Keine `TEST_P`-Tests.** Alle neuen Tests sind klassische `TEST`-Funktionen.

## Öffentliche Grenzfäche

### include/pplib/core/compression.h

```cpp
enum class Algorithm {
    NONE = 0,
    ZLIB,      // 1
    BZIP2,     // 2
    ZSTD,      // 3  neu
    BROTLI,    // 4  neu
    Unknown = 256
};
```

Vier neue freie Funktionen im Namespace `pplib`, analog zu `CompressZlib`/`CompressBZip2`:

```cpp
ByteArray CompressZstd  (const ByteArrayPtr& in, Compression::Level level = Compression::Level::Default);
void      CompressZstd  (ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level::Default);
ByteArray CompressBrotli(const ByteArrayPtr& in, Compression::Level level = Compression::Level::Default);
void      CompressBrotli(ByteArray& out, const ByteArrayPtr& in, Compression::Level level = Compression::Level::Default);
```

An der übrigen öffentlichen API ändert sich nichts. `MAX_DATA_SIZE` bleibt bei 4 GiB - 1
(Begründung: das eigene Prefix-Format hält Längen in 32 Bit; die Bibliotheken können zwar
64 Bit, das Format nicht).

### Private Methoden in `Compression`

```cpp
void doZstd  (void* dst, size_t* dstlen, const void* src, size_t size);
void unZstd  (void* dst, size_t* dstlen, const void* src, size_t srclen);
void doBrotli(void* dst, size_t* dstlen, const void* src, size_t size);
void unBrotli(void* dst, size_t* dstlen, const void* src, size_t srclen);
```

## Ownership und Speicherverwaltung

- Keine eigenen Allokationen in den neuen Methoden. Wie bei Zlib/Bzip2 wird direkt in den vom
  Aufrufer gestellten Puffer `dst` geschrieben.
- `ZSTD_compress()` / `ZSTD_decompress()` und `BrotliEncoderCompress()` /
  `BrotliDecoderDecompress()` verwenden intern einen eigenen, begrenzten Workspace. Diese
  Allokationen laufen im Heap der Fremdbibliothek, nicht in `pplib::ByteArray`.
- `maxCompressedBound()` wird um `ZSTD_compressBound()` und `BrotliEncoderMaxCompressedSize()`
  erweitert, jeweils nur wenn die Bibliothek einkompiliert ist. Brotli kann im Worst Case um
  rund 25 % expandieren - der generische Fallback `size + size/100 + 600` reicht dafür nicht.
  Der Bound bleibt algorithmus-agnostisch (Maximum aller verfügbaren Bounds) wie bisher.

## Fehlermodell

Identisch zu Zlib/Bzip2:

| Situation | Exception |
|---|---|
| Algorithmus nicht einkompiliert (`init`, Konstruktor, Low-Level-Dispatch) | `UnsupportedFeatureException("Zstd")` / `("Brotli")` |
| `dst`/`dstlen` NULL, `src` NULL bei size > 0, Größe > `MAX_DATA_SIZE` | `IllegalArgumentException` |
| Ausgabepuffer zu klein | `BufferTooSmallException` |
| Eingabedaten korrupt / abgeschnitten | `CorruptedDataException` |
| Bibliotheksmeldung "kein Speicher" | `OutOfMemoryException` |
| alles andere | `CompressionFailedException` / `DecompressionFailedException` |

Wichtige Semantik-Entscheidung: `*dstlen` wird im Fehlerfall nicht garantiert gesetzt.
Bei Zlib/Bzip2 passiert das nebenbei, weil deren One-Shot-API Teil-Output schreibt.
Bei zstd/brotli ist der Wert nach `BufferTooSmallException` undefiniert - und wird auch
nirgends gelesen: die Schleife in `uncompressTo()` verwirft den Puffer und startet mit
größerem `buffersize` komplett neu. Das ist der bestehende Vertrag und bleibt so.

Konsequenz, bewusst in Kauf genommen: bei `Prefix::NONE` und stark komprimierbaren Daten
braucht zstd/brotli mehr Schleifendurchläufe als Zlib, weil aus der Exception kein Hinweis
auf die wahre Zielgröße kommt. Funktional korrekt, nur langsamer.

## Level-Abbildung

| `Compression::Level` | Zlib | Bzip2 | Zstd | Brotli |
|---|---|---|---|---|
| `Fast` | `Z_BEST_SPEED` (1) | 1 | 1 | 1 |
| `Normal` | 5 | 5 | 5 | 5 |
| `Default` | `Z_DEFAULT_COMPRESSION` (-1) | 5 | `ZSTD_CLEVEL_DEFAULT` (3) | 6 |
| `High` | `Z_BEST_COMPRESSION` (9) | 9 | 19 | 11 |

Brotli-Quality ist 0-11, `BROTLI_DEFAULT_QUALITY` ist 11. Wir nutzen 6 für `Default`, damit
`Default` nicht deutlich teurer ist als `Normal`.

## Thread-Safety

Unverändert: die Klasse hält ihren Zustand (`algorithm_`, `level_`, `prefix_`) in der Instanz,
es gibt keine neuen statischen oder globalen mutablen Daten. Die verwendeten One-Shot-APIs
sind zustandslos und aufrufbar, solange verschiedene Threads verschiedene
`Compression`-Instanzen verwenden - wie bisher.

## Plattform-Grenzen

- **Linux**: `pkg_check_modules` gegen `libzstd`, `libbrotlienc`, `libbrotlidec`. Vorhanden
  (libzstd 1.5.7, brotli 1.2.0).
- **Windows/MinGW (MSYS2)**: Pakete `mingw-w64-x86_64-zstd`, `-brotli`. Falls nicht
  installiert, greifen die `#ifndef HAVE_*`-Zweige -> `UnsupportedFeatureException`, die
  Bibliothek baut trotzdem.
- **FreeBSD**: Ports `archivers/zstd`, `archivers/brotli`, pkg-config vorhanden.
- **Pico-Zweig** (`PPLIB_ENABLE_PICO`): `pplib_core` wird dort nicht gebaut, daher keine
  besonderen Vorkehrungen.

## Build-Integration

CMakeLists.txt, direkt nach dem BZip2-Block, im Stil des PCRE2-Blocks:

```cmake
pkg_check_modules(ZSTD QUIET libzstd)
if(ZSTD_FOUND)
    set(HAVE_ZSTD 1)
    target_include_directories(pplib_core PRIVATE ${ZSTD_INCLUDE_DIRS})
    target_link_libraries(pplib_core PRIVATE ${ZSTD_LIBRARIES})
    target_compile_definitions(pplib_core PRIVATE HAVE_ZSTD=1)
    pplib_register_pc_dependency(libzstd "${ZSTD_LIBRARIES}")
endif()

pkg_check_modules(BROTLIENC QUIET libbrotlienc)
pkg_check_modules(BROTLIDEC QUIET libbrotlidec)
if(BROTLIENC_FOUND AND BROTLIDEC_FOUND)
    set(HAVE_BROTLI 1)
    ...
endif()
```

Zusätzlich: `#cmakedefine HAVE_ZSTD 1` / `HAVE_BROTLI 1` in `include/config_pplib.h.in`,
zwei Zeilen in der Config-Summary.

## Offene Punkte

1. `PrefixV1FlagValidation` benutzt aktuell `algo = 3` als "unbekannter Algorithmus".
   Mit ZSTD = 3 muss der Test auf einen wirklich freien Wert (7) umgestellt werden.
2. `BrotliEncoderMaxCompressedSize()` liefert 0 zurück, wenn die Größe nicht darstellbar ist -
   dann muss der generische Fallback greifen, nicht 0.
3. Coverage: die `#ifndef HAVE_*`-Throw-Zweige sind auf Build-Maschinen mit installierten
   Bibliotheken nicht erreichbar und bleiben mit `LCOV_EXCL` markiert.

## Umsetzung (abgeschlossen)

Alle drei offenen Punkte sind erledigt. Abweichungen vom Entwurf:

- **Fehlerermittlung Zstd** erfolgt als `switch` über `ZSTD_getErrorCode()` statt als
  `if/else`-Kette. Grund: gcov konnte die mehrzeilige `||`-Bedingung nicht vollständig zuordnen.
  Das Fehlermodell (Tabelle oben) ist davon unberührt.
- **Brotli-Kompression** prüft vor dem Aufruf gegen `BrotliEncoderMaxCompressedSize(size)`,
  weil `BrotliEncoderCompress()` zwischen "Puffer zu klein" und echten Fehlern nicht
  unterscheidet (rückgabewert `BROTLI_FALSE`, `encoded_size = 0` in beiden Fällen).
- **Brotli-Dekompression** unterscheidet anhand von `decoded_size == *dstlen`: der One-Shot-Decoder
  fuellt den Zielbuffer bei "zu klein" komplett, bei korrupten Daten bleibt er leer.
  Beides wurde mit einem Programm gegen die installierten Bibliotheken verifiziert
  (zstd 1.5.7, brotli 1.2.0) - die Header-Doku gibt das nicht her.

Coverage `src/core/Compression.cpp`: Lines 100 % (417/417), Branches 91 % (265/289).
Nicht erreichbar bleiben die `LCOV_EXCL`-Zweige (`OutOfMemory`, `CompressionFailed`,
`DecompressionFailed`, `#ifndef HAVE_*`).
