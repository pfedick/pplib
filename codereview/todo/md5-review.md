# Md5 Review – Befunde

Review vom 2026-09-01, Scope: `src/math/md5.cpp` (341 Zeilen). Öffentliche API: `String Md5(const void*, size_t)`,
`String Md5(const ByteArrayPtr&)` (`include/pplib/core/functions.h:80-81`), zusätzlich `String FileObject::md5()`
(implementiert in dieser Datei). Algorithmus (RFC-1321-Referenzimplementierung) empirisch gegen
`build/coverage/libpplib_core.a` mit Standard-Testvektoren verifiziert:
`MD5("abc") == 900150983cd24fb0d6963f7d28e17f72` ✓, `MD5("message digest") == f96b697d7cb7938d525a2f31aaf161d0` ✓,
`MD5("a"*26) == c3fcd3d76192e4007dfb496cca67e13b` ✓, `MD5("a"*1000000) == 7707d6ae4e027c70eea2a935c2296f21` ✓
(deckt auch den Mehrblock-Pfad in `MD5Update` ab). Die Kernimplementierung ist korrekt.

* Review done by: Claude Sonnet 5

## Bugs (mittel)

- [ ] **`Md5(buffer, 0)` wirft `EmptyDataException` statt des wohldefinierten MD5-Hashes des leeren Strings – inkonsistent zu `FileObject::md5()`** (Zeile 291-309 vs. 311-338)
  ```cpp
  String Md5(const void* buffer, size_t size)
  {
      if (buffer == NULL || size == 0) throw EmptyDataException();   // Zeile 295
      ...
  }
  ```
  `MD5("")` ist laut RFC 1321 wohldefiniert (`d41d8cd98f00b204e9800998ecf8427e`) und wird in der Praxis häufig als
  Sentinel-/Vergleichswert verwendet (z.B. "ist dieser Blob leer?"). Diese Funktion verweigert die Berechnung für
  leere Eingaben komplett. Empirisch verifiziert – und zwar mit einem konkreten Widerspruch im selben File:
  ```
  Md5("", 0)                                  -> wirft EmptyDataException
  FileObject::md5()  auf einer leeren Datei   -> liefert korrekt d41d8cd98f00b204e9800998ecf8427e
  ```
  `FileObject::md5()` (Zeile 311-338) implementiert `MD5Init`/`MD5Update`/`MD5End` direkt und hat diesen
  Leer-Check nicht – für eine 0-Byte-Datei läuft die `while(!eof())`-Schleife einfach nie und `MD5End` liefert
  korrekt den Standard-Hash der leeren Zeichenkette (empirisch mit einer echten 0-Byte-Datei nachgestellt). Ein
  und dieselbe konzeptionelle Operation ("MD5 von 0 Byte") verhält sich also je nach Aufrufpfad unterschiedlich:
  einmal Exception, einmal korrektes Ergebnis. Passt außerdem nicht zum REFACTORING.md-Ziel "weniger Exceptions".
  Fix: den `size == 0`-Sonderfall in `Md5(const void*, size_t)` genauso behandeln wie in `FileObject::md5()`
  (d.h. entfernen und stattdessen einfach den leeren Puffer durch `MD5Init`+`MD5Final` laufen lassen); nur bei
  `buffer == NULL && size > 0` bleibt eine Exception (z.B. `IllegalArgumentException`) sinnvoll.

## Design

- [ ] **Redundanter Doppel-Check in `Md5(const ByteArrayPtr&)`** (Zeile 305-309)
  ```cpp
  String Md5(const ByteArrayPtr& buffer)
  {
      if (buffer.isNull() || buffer.size() == 0) throw EmptyDataException();
      return Md5(buffer.ptr(), buffer.size());
  }
  ```
  Der Check ist identisch zu dem in der aufgerufenen Funktion (Zeile 295) und daher überflüssig – nicht falsch,
  aber unnötige Doppelpflege einer Bedingung an zwei Stellen. Fällt mit dem Fix oben ohnehin weg bzw. vereinfacht
  sich.

## Doku / Kosmetik

- `functions.h` dokumentiert `Md5()` gar nicht (kein `@brief`/`@param`/`@return`, im Gegensatz zu `Crc32`/`Crc16`/
  `Random`). Insbesondere das Verhalten bei leerer Eingabe (Exception statt Hash) ist an keiner Stelle erwähnt und
  daher für Aufrufer überraschend.
- `typedef struct MD5Context {...} MD5_CTX;` (Zeile 43-48) ist reiner C-Stil; in C++ wäre `struct MD5Context`
  ausreichend. Rein kosmetisch, keine funktionalen Auswirkungen (der Typ ist intern, keine Namenskollision mit
  OpenSSLs `MD5_CTX`, da innerhalb `namespace pplib` deklariert).

## Verifiziert OK (kein Handlungsbedarf)

- MD5-Kernalgorithmus (`MD5Init`/`MD5Update`/`MD5Pad`/`MD5Final`/`MD5Transform`) ist bit-genau korrekt für kurze,
  mittlere und lange (1 MB, Mehrblock-Update) Eingaben – vier verschiedene Standard-Testvektoren empirisch
  bestätigt.
- Alle internen Hilfsfunktionen (`Encode`, `Decode`, `MD5Init`, `MD5Update`, `MD5Pad`, `MD5Final`, `MD5Transform`,
  `MD5End`) sind korrekt `static`, keine versehentliche Symbol-Exportierung wie bei `calc.cpp`.
- Kein globaler/statischer mutable State – `MD5_CTX` wird pro Aufruf auf dem Stack gehalten, thread-sicher.
- `FileObject::md5()` sichert Lese-Position korrekt ab (`seek(oldpos)` sowohl im Erfolgsfall als auch im
  `catch(...)`-Pfad).
