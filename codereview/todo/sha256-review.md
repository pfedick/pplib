# Sha256 Review – Befunde

Review vom 2026-09-01, Scope: `src/math/sha256.cpp` (237 Zeilen). Öffentliche API: `String Sha256(const void*, size_t)`,
`String Sha256(const ByteArrayPtr&)` (`include/pplib/core/functions.h:82-83`), zusätzlich `String FileObject::sha256()`
(implementiert in dieser Datei). Algorithmus empirisch gegen `build/coverage/libpplib_core.a` mit
NIST-Testvektoren verifiziert: `SHA256("abc") == ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` ✓,
`SHA256("a"*1000000) == cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0` ✓ (per `sha256sum`
gegengeprüft, deckt auch den Mehrblock-Pfad in `SHA256Update` ab). Die Kernimplementierung ist korrekt.

* Review done by: Claude Sonnet 5

## Bugs (mittel)

- [ ] **`Sha256(buffer, 0)` wirft `EmptyDataException` statt des wohldefinierten SHA-256-Hashes des leeren Strings – inkonsistent zu `FileObject::sha256()`** (Zeile 178-195 vs. 202-234)
  ```cpp
  String Sha256(const void* buffer, size_t size)
  {
      if (buffer == NULL || size == 0) throw EmptyDataException();   // Zeile 180
      ...
  }
  ```
  `SHA256("")` ist wohldefiniert (`e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`) und wird u.a.
  als bekannter "leer"-Sentinel verwendet (z.B. Git-Objekt-Hashes, TLS-Handshakes). Dieselbe Inkonsistenz wie bei
  `Md5()` ist hier ebenfalls vorhanden und wurde empirisch bestätigt:
  ```
  Sha256("", 0)                                 -> wirft EmptyDataException
  FileObject::sha256()  auf einer leeren Datei  -> liefert korrekt e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  ```
  `FileObject::sha256()` (Zeile 202-234) hat diesen Check nicht (nutzt `SHA256Init`/`Update`/`Final` direkt), eine
  0-Byte-Datei ergibt daher korrekt den Standard-Leer-Hash, während der freie Funktionsaufruf mit Größe 0 eine
  Exception wirft. Widerspricht außerdem dem REFACTORING.md-Ziel "weniger Exceptions".
  Fix: den `size == 0`-Fall genauso wie in `FileObject::sha256()` behandeln (einfach durchlaufen lassen); nur bei
  `buffer == NULL && size > 0` eine Exception (z.B. `IllegalArgumentException`) werfen.

## Design

- [ ] **`FileObject::sha256()` hat – anders als `FileObject::md5()` in `md5.cpp` – keine defensive Absicherung gegen einen 0-Byte-Read ohne `eof()`** (Zeile 213-218 vs. `md5.cpp:324-328`)
  ```cpp
  while (!eof()) {
      size_t bytesRead = read(buffer, buffer.size());
      if (bytesRead > 0) {
          SHA256Update(ctx, (const unsigned char*)buffer.ptr(), bytesRead);
      }
  }   // kein "else break" – im Unterschied zu md5.cpp
  ```
  `md5.cpp`s Pendant hat zusätzlich `if (bytes_read == 0) break;`. Mit den aktuellen `FileObject`-Implementierungen
  (`File`, `MemFile`, `GzFile` – alle verifiziert) ist das nicht aktiv ausnutzbar: `eof()` ist dort entweder
  korrekt positionsbasiert vor dem Read gesetzt, oder ein echter 0-Byte-Read wirft bereits eine
  `EndOfFileException`, statt still `0` zurückzugeben. Für eine zukünftige/externe `FileObject`-Unterklasse, die
  `fread()`/`eof()` nicht exakt so synchron hält, wäre `sha256()` aber anfällig für eine Endlosschleife, während
  `md5()` das abfangen würde. Kein aktuell reproduzierbarer Bug, aber eine Inkonsistenz zwischen zwei nahezu
  identischen Funktionen, die sich beim nächsten Refactoring leicht angleichen ließe (den `break` übernehmen).

## Doku / Kosmetik

- `functions.h` dokumentiert `Sha256()` nicht (kein `@brief`/`@param`/`@return`, im Gegensatz zu `Crc32`/`Crc16`/
  `Random`). Das Verhalten bei leerer Eingabe ist nirgends erwähnt.
- `Sha256(const ByteArrayPtr&)` (Zeile 197-200) hat – anders als das Pendant in `md5.cpp` – gar keinen expliziten
  `isNull()`/`size()==0`-Check, sondern verlässt sich vollständig auf den Check in der aufgerufenen
  `Sha256(const void*, size_t)`. Funktional gleichwertig, aber ein weiteres kleines Stil-Inkonsistenz-Detail
  zwischen den beiden ansonsten fast identischen Dateien `md5.cpp`/`sha256.cpp`.

## Verifiziert OK (kein Handlungsbedarf)

- SHA-256-Kernalgorithmus (`SHA256Init`/`SHA256Update`/`SHA256Final`/`SHA256Transform`, inkl. Konstanten-Tabelle
  `K[64]`) ist bit-genau korrekt für kurze und lange (1 MB, Mehrblock-Update) Eingaben – Testvektoren empirisch
  bestätigt und zusätzlich gegen `sha256sum` desselben 1-MB-Inputs gegengeprüft.
- Kein globaler/statischer mutable State – `Sha256Context` wird pro Aufruf auf dem Stack/in lokalen Variablen
  gehalten, thread-sicher.
- `FileObject::sha256()` sichert die Lese-Position korrekt ab (`seek(oldpos)` im Erfolgsfall und im
  `catch(...)`-Pfad).
- Alle internen Hilfsfunktionen sind korrekt `static`.
