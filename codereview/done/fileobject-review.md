# FileObject Review – Befunde

Review vom 2026-08-28, Scope: `include/pplib/core/fileobject.h` + `src/core/FileObject.cpp`.
Abhängigkeiten in `File.cpp`, `MemFile.cpp`, `string.h`, `widestring.h`, `bytearrayptr.h` verifiziert.

## Bugs (kritisch)

- [ ] **`MemFile::seek(int64_t, SeekOrigin)` lehnt die Position "genau Dateiende" ab** (`src/core/MemFile.cpp`, ~Zeile 305)
  `SEEKEND` mit `offset=0`: `pos == mysize` fällt durch beide Checks (`> mysize` / `< 0`) → `break` → `FileSeekException`.
  `fseek(0, SEEK_END)` ist aber die häufigste Seek-Operation! Analog: `seek(uint64_t)` verwendet `position < mysize` (Off-by-one) →
  bei schreibbaren MemFiles wird ein Seek auf das Dateiende (Append-Fall) mit `OverflowException` abgelehnt, obwohl `fwrite` dort
  via `resizeBuffer` legal schreiben würde.
  Fix: Grenzen auf `<=` bzw. `>=` korrigieren; zusätzlich den SEEKCUR-Branch umschreiben (`pos += offset` auf `uint64_t` mit negativem
  Offset wrappt und "funktioniert" nur durch Zufall dank Two's-Complement-Roundtrip über `(int64_t)pos < 0`).

- [ ] **`MemFile::fgetws`: OOB-Read am Dateiende** (`src/core/MemFile.cpp`, ~Zeile 379)
  Im Gegensatz zu `fgets` fehlt die Clamp-Kontrolle (`if (pos + by > mysize)`). Läuft eine Zeile bis zum EOF ohne `\n`, liest die
  Schleife jenseits von `MemBase + mysize` → Heap-Overread. Zusätzlich: kein Check auf `num < 1` (direkter Aufruf mit `num=0`
  lässt `num - 1` auf huge wrappen), und `wchar_t* ptr = MemBase + pos` ist bei ungeradem `pos` misaligned (UB, falls wchar_t
  Alignment verlangt).
  Fix: wie in `fgets` clampen (`i`-Limit an `mysize - pos` orientieren), `num < 1` ablehnen.

- [ ] **`MemFile::fgetc`: OOB-Read bei `pos == mysize`** (~Zeile 436)
  `if (pos > mysize)` müsste `>=` sein; im Readonly-Fall zeigt `MemBase` auf exakt `mysize` Bytes fremden Speichers,
  `MemBase[mysize]` liest dann ein Byte zu weit. Fix: `if (pos >= mysize) throw EndOfFileException();`.

- [ ] **`FileObject::getws` nimmt `String&`, die Doku verspricht WideString** (`src/core/FileObject.cpp`, ~Zeile 423; Header-Zeile ~85)
  Die Deklaration ist `int getws(String& buffer, size_t num)` und der Code räumt Wide-Daten in einen (schmalen) `String`.
  Das passt weder zur Doku ("speichert sie im Wide-Character-String-Objekt") noch zum Gegenstück `putws(const WideString&)` —
  die API ist asymmetrisch, und die Umkonvertierung via Encoding kann Daten verlieren.
  Fix: `int getws(WideString& buffer, size_t num)` + `WideString getws(size_t)` ergänzen (PPL8 erlaubt Interface-Änderungen;
  alte Signatur ggf. als deprecated beibehalten).

- [ ] **`copyFrom(FileObject&, uint64_t)`: Rückgabewert, Self-Copy und Pipes** (`src/core/FileObject.cpp`, ~Zeile 327)
  1. Es wird `bytes` (Anfrage) statt der tatsächlich kopierten Bytes zurückgegeben — bei kurzem Source meldet die Funktion Erfolg.
     Fix: eigenen Zähler `copied += by` führen und den zurückgeben.
  2. Infinite-Loop-Risiko, wenn eine abgeleitete `fread` 0 liefert, ohne Exception zu werfen (`rest -= 0`).
  3. Self-Copy (`quellfile == this`): die 4-Arg-Version seekt beide Zeiger auf dasselbe Objekt → letztes Seek gewinnt →
     gelesen und geschrieben wird an derselben Stelle = stiller No-Op statt Datenkopie. Fix: Self-Check am Anfang.
  4. Pipes/popen als Source: `size()` liefert 0 → Guard `size() > tell()` ist falsch → Kopier-Schleife wird ganz übersprungen,
     aber `bytes` (Vollmenge) wird zurückgemeldet.

- [ ] **`File::fread`: Partial Read am EOF wirft keine Exception** (`src/core/File.cpp`, Zeile 640)
  Geprüft wird nur `if (by == 0)` gegen `feof`. Ein Partial Read (`0 < by < nmemb`) am Dateiende gibt still zurück,
  obwohl die Doku "Dateiende → Exception" verspricht. Folgewirkungen: `load()` liefert NULL statt Exception,
  `read(ByteArray&)` hinterlässt ein Objekt mit `ptrsize > tatsächlich gelesene Bytes` (Garbage-Tail),
  und `copyFrom` bricht mitten im Kopiervorgang mit `EndOfFileException` ab.
  Fix: `if (by != nmemb) { if (::ferror(...)) throwErrno(...); else if (::feof(...)) throw EndOfFileException(); }`.

- [ ] **`FileObject::read(ByteArray&)`: Zerstört Target vor der Open-Prüfung** (`src/core/FileObject.cpp`, ~Zeile 280)
  `target.free()` + `target.malloc(bytes)` laufen VOR dem eigentlichen Lesevorgang. Wirft `fread` (z.B. `FileNotOpenException`),
  bleibt das Target im Zustand "freigegeben, neu allokiert, leer" — die alten Daten des Aufrufers sind weg.
  Fix: wie in `load(ByteArray&)` am Anfang `if (!isOpen()) throw FileNotOpenException();`.

- [ ] **`FileObject::load()`: Liefert NULL statt Exception** (~Zeile 580)
  Bei `r != s` wird `free(b)` + `return NULL`; die Doku sagt "Im Fehlerfall wird eine Exception geworfen". Aufrufer,
  die das nicht prüfen (weil die Klasse sonst nur Exceptions wirft), segfaulten. Fix: `throw ReadException()`/`EndOfFileException`.

## Bugs (mittel)

- [ ] **`MemFile::map`/`mapRW`: NULL statt Exception** (~Zeile 460)
  Bei OOB liefert der Code `NULL`, die Basisklassen-Doku verspricht aber eine Exception. Aufrufer, die nur Exceptions abfangen,
  crashen auf dem NULL-Zeiger. Außerdem: `position + bytes` (uint64_t + size_t) kann overflowen → Guard wird umgangen → OOB-Pointer.
  Fix: wie in `File::map` mit `OverflowException` werfen und Overflow-sichere Addition (`position > mysize - bytes`).

- [ ] **`MemFile::openReadWrite`: Nimmt still Ownership am Aufrufer-Speicher** (~Zeile 215)
  Setzt `buffer = MemBase`, der Destruktor (via FileObject) macht `free(buffer)` — der vom Aufrufer übergebene Speicher wird also
  von der Klasse freigegeben. Der Konstruktor mit `writeable=true` dokumentiert das per `@attention`, `openReadWrite` aber nicht →
  nutzt die Applikation ihren Buffer weiter: Use-after-Free. Fix: `@attention`-Hinweis ergänzen (oder Ownership nicht übernehmen).

- [ ] **`MemFile::fwrite`: Overflow bei `nmemb * size`; Closed-State rutscht durch** (~Zeile 345)
  `size_t bytes = nmemb * size;` ohne Vorab-Check (wie `MAX_BYTEARRAY_SIZE` in ByteArray). Und der Zustand "nie geöffnet"
  (`MemBase == NULL`, `readonly == false`) läuft an der ersten Condition vorbei und "funktioniert" nur, weil `resizeBuffer`
  per Zufall allokiert — semantisch sollte hier `FileNotOpenException` fliegen.

- [ ] **`write(ByteArrayPtr&, bytes = 0)` wirft bei leerem Objekt** (`src/core/FileObject.cpp`, ~Zeile 213)
  Header-Default ist `bytes = 0` (= "alles schreiben"). Ist das Objekt leer, wird `fwrite(NULL, 1, 0)` gerufen und
  `File::fwrite` wirft `IllegalArgumentException` auf den NULL-Zeiger — ein legitimer No-Op schlägt fehl.
  Fix: nach dem Clamp `if (!bytes) return 0;`.

## Doku / Kosmetik

- [ ] Include-Guard von `fileobject.h`: `#ifndef PPLIB_CORE_FILES_H_`, aber das `#endif`-Kommentar sagt `PPLIB_CORE_THREADS_H_` (Copy-Paste).
- [ ] Typos/Kopierfehler in Doxygen-Kommentaren: `%fgwets` (statt fgetws), `fputws`-Kommentar beginnt mit "%fputs",
      "Ausgahgsbasis"/"miitels" im SeekOrigin-Brief, "solle" (statt soll) etc.
- [ ] `load()`/`load(ByteArray&)`: Seiteneffekt `seek(0)` (Dateizeiger wird zurückgesetzt) ist nicht dokumentiert.
- [ ] `gets()`: Konvertierung über die System-Locale — passt nicht zum Ziel "UTF-8 als Locale, auch unter Windows".
      Kandidat für ein späteres Refactoring: Encoding explizit parametrieren statt Locale zu nutzen.
