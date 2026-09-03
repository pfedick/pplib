# String Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/string.h` + `src/types/String.cpp`.
Verifiziert: Exception-Namen gegen `include/pplib/exceptions.h` / `include/pplib/core/baseexception.h`
(`Exception`, `OutOfMemoryException`, `IllegalArgumentException`, `CharacterEncodingException`, `OutOfBoundsException` existieren alle).
`ByteArrayPtr::map()` (`src/types/ByteArrayPtr.cpp:99`) liefert einen rohen `const char*` in beliebigen Speicher ohne Null-Terminierungsgarantie.
`WideString::getPtr()` liefert `const wchar_t*` (widestring.h), `String::set(const WideString&, size)` reicht diesen korrekt an die
`wchar_t*`-Overloads weiter. Verwendungsstellen von `String::set(ByteArrayPtr&, ...)` und `startsWith`/`endsWith` per grep in
`src/core/File.cpp` und `tests/src/types/string.cpp` verifiziert.

## Bugs (kritisch)

- [ ] **`String::prepend(const wchar_t*, size_t)` verwechselt Wide-Char-Anzahl mit Byte-Anzahl → zerstört UTF-8-Sequenzen** (`String.cpp:475-480`)
  ```cpp
  String& String::prepend(const wchar_t* str, size_t size)
  {
      String a;
      a.set(str, size);
      return prepend((const char*)a.ptr, size);   // <- size ist die Anzahl WIDE CHARS, nicht Bytes!
  }
  ```
  `a` enthält nach `a.set(str, size)` die UTF-8-kodierten Bytes der ersten `size` Wide-Chars – bei Nicht-ASCII-Zeichen i.d.R.
  mehr Bytes als `size`. Trotzdem wird `size` (Wide-Char-Zahl) direkt als Byte-Länge an `prepend(const char*, size_t)`
  weitergereicht, das `size` unverändert als `inchars` für den `memcpy` übernimmt (siehe `prepend(const char*, size_t)`,
  Zeile 510-544: `inchars = (size != (size_t)-1) ? size : ::strlen(str);`). Der Analog-Fall `append(const wchar_t*, size_t)`
  (Zeile 391-396) macht es korrekt: `return append((const char*)a, a.size());` – verwendet die tatsächliche Byte-Länge.
  Konkretes Beispiel (führt zu einer abgeschnittenen, ungültigen UTF-8-Sequenz):
  ```cpp
  pplib::String s("Text");
  s.prepend(L"ä", 1);   // 1 Wide-Char "ä" (2 Bytes UTF-8: 0xC3 0xA4)
  // erwartet: "äText"
  // tatsächlich: nur 1 Byte (0xC3) wird kopiert -> ungültige, abgeschnittene UTF-8-Sequenz + "Text"
  ```
  Der vorhandene Unittest `prependConstWchartWithSize` (`tests/src/types/string.cpp:773-780`) deckt das nicht auf, weil der
  gewählte Cut zufällig auf eine Zeichengrenze fällt (`L"äöü Second Part", 8` schneidet exakt nach "äöü S").
  Fix: wie bei `append` die tatsächliche Byte-Länge verwenden:
  ```cpp
  return prepend((const char*)a.ptr, a.size());
  ```
  ==> FIXED, Test ergänzt

- [ ] **`String::set(const char*, size_t)` ruft `strlen(str)` immer auf, auch wenn `size` explizit gesetzt ist → OOB-Read bzw. stille Trunkierung bei nicht garantiert Null-terminierten Puffern** (`String.cpp:218-245`)
  ```cpp
  String& String::set(const char* str, size_t size)
  {
      if (!str) { clear(); return *this; }
      size_t inbytes = (size != (size_t)-1) ? size : ::strlen(str);
      if (size > strlen(str)) inbytes = strlen(str);   // <- wird IMMER ausgeführt, unabhängig von size
      ...
  ```
  Anders als `append(const char*, size_t)` und `prepend(const char*, size_t)`, die `strlen()` nur aufrufen, wenn
  `size == (size_t)-1` ist, berechnet `set()` `strlen(str)` bedingungslos ein zweites Mal. Das bricht jede Verwendung mit
  einem expliziten `size` auf einem Puffer, der nicht (oder nicht rechtzeitig) Null-terminiert ist:
  - **OOB-Read:** `ByteArrayPtr::map(pos, len)` liefert laut Implementierung (`ByteArrayPtr.cpp:99-106`) einen rohen Pointer
    ohne Terminierungsgarantie. `String s(ba.map(10, 20), 20);` läuft in `set(str, 20)`, das trotz explizitem `size=20`
    per `strlen()` über die 20 Bytes hinausliest, bis zufällig eine Null im Speicher gefunden wird – potenziell weit
    außerhalb des gemappten Bereichs (Heap-Buffer-Overflow-Read, Crash/Infoleak möglich).
  - **Stille Datenkorruption:** `String::set(const ByteArrayPtr&, size_t)` (Zeile 289-298) reicht `str.adr()` mit exaktem
    `size` durch. `src/core/File.cpp:800-805` (`File::load(String&, filename)`) lädt eine komplette Datei in ein `ByteArray`
    und ruft dann `object.set(ba, ba.size())` auf. Enthält die Datei ein eingebettetes `\0`-Byte (z.B. eine Textdatei mit
    einem Steuerzeichen oder versehentlich eine Binärdatei), wird der String beim ersten `\0` still abgeschnitten statt
    den vollen Inhalt zu übernehmen oder eine Exception zu werfen – trotz des Klassenversprechens „String, der beliebige
    Zeichen enthalten kann" (string.h:61).
  Fix: die `strlen()`-Prüfung nur ausführen, wenn `size == (size_t)-1` war, analog zu `append`/`prepend`:
  ```cpp
  size_t inbytes = (size != (size_t)-1) ? size : ::strlen(str);
  if (size == (size_t)-1 && inbytes > ::strlen(str)) inbytes = ::strlen(str); // eigentlich dann redundant
  ```
  bzw. einfacher: die zweite Zeile ganz entfernen, da die erste Zeile bereits das Gewünschte leistet.

  ==> FIXED

- [ ] **`append`/`prepend` mit `String&`/`std::string&`-Quelle klemmen `size` nicht auf die tatsächliche Quelllänge → OOB-Read** (`String.cpp:432-435, 444-448, 482-485, 494-498`)
  ```cpp
  String& String::append(const String& str, size_t size)
  {
      return append(str.ptr, size);   // <- kein Clamp auf str.stringlen!
  }
  ```
  Die Doku von `append(const String&, size_t)` (string.h:640-650) verspricht explizit: „Ist der Wert größer als der
  angegebene String, wird er ignoriert und der komplette String importiert." Das analoge `set(const String&, size_t)`
  (String.cpp:278-287) hält dieses Versprechen korrekt ein (`if (inbytes > str.stringlen) inbytes = str.stringlen;`),
  `append`/`prepend` dagegen reichen `size` unclamped an `append(const char*, size_t)`/`prepend(const char*, size_t)`
  durch, die einen expliziten `size` ungeprüft für den `memcpy`/`memmove` übernehmen. Gleiches gilt für
  `append(const std::string&, size_t)` (Zeile 444-448) und `prepend(const std::string&, size_t)` (Zeile 494-498).
  Konkretes Beispiel:
  ```cpp
  pplib::String a("ab");                 // a.stringlen==2, Puffer typischerweise nur 3 Bytes groß
  pplib::String b("existing content");   // b.stringlen != 0 -> Self-Append-Fastpath via set() greift nicht
  b.append(a, 100);                      // liest 100 Bytes ab a.ptr -> Heap-Buffer-Overflow-Read
  ```
  Der bestehende Unittest `append_string_with_size` (`tests/src/types/string.cpp:632` etc.) prüft nur Größen *kleiner*
  als die Quelllänge, der Clamp-Fall (`size` größer als Quelle) ist ungetestet.
  Fix: in allen vier Methoden zuerst auf die tatsächliche Länge klemmen, z.B.:
  ```cpp
  String& String::append(const String& str, size_t size)
  {
      if (size > str.stringlen) size = str.stringlen;
      return append(str.ptr, size);
  }
  ```

  ==> FIXED

## Bugs (mittel)

- [ ] **`isNumeric()`/`isInteger()` akzeptieren ein alleinstehendes "-" (bzw. "," oder ".") als Zahl** (`String.cpp:168-198`)
  ```cpp
  bool String::isInteger() const
  {
      ...
      if (c == '-' && i == 0) continue;   // erlaubt "-" ohne jede Ziffer
      ...
  }
  ```
  `String("-").isInteger()` liefert `true`, `String("-").isNumeric()` ebenso, `String(",").isNumeric()` ebenfalls – obwohl
  in keinem Fall eine einzige Ziffer vorkommt. Kein Unittest deckt diesen Fall ab (die vorhandenen Tests prüfen nur
  vollständige Zahlen und mehrfache Trenner).
  Fix: zusätzlich mitzählen, ob mindestens eine Ziffer gesehen wurde, und das am Ende prüfen:
  ```cpp
  bool sawDigit = false;
  ...
  if (c >= '0' && c <= '9') sawDigit = true;
  ...
  return sawDigit;
  ```

  ===> FIXED

- [ ] **`startsWith`/`endsWith`: Parameter `end` ist als "Endposition" dokumentiert, wird aber als Länge an `mid()` übergeben** (`string.h:1220-1242`, `String.cpp:1294-1314`)
  ```cpp
  bool String::startsWith(const String& prefix, size_t start, size_t end) const
  {
      String part;
      if (start > 0 || end != (size_t)-1) {
          part = mid(start, end).left(prefix.size());   // "end" wird als LÄNGE an mid() übergeben
      ...
  ```
  Doku: „end Endposition innerhalb des Strings, bis zu der die Prüfung erfolgen soll" – tatsächlich verhält sich `end`
  wie `mid()`s zweiter Parameter, also eine **Länge ab `start`**, nicht eine absolute Endposition. Beispiel:
  ```cpp
  pplib::String s("abcXabc");
  s.startsWith("abc", 4, 6);
  // Doku-Erwartung (end=Position 6): Prüfbereich s[4..6) = "ab" -> kann nicht mit "abc" beginnen -> false
  // tatsächlich: mid(4,6) liefert s[4..7) = "abc" (Länge 6, geklemmt auf Rest) -> left(3) = "abc" -> true
  ```
  Die vorhandenen Unittests (`tests/src/types/string.cpp:2143-2162`) verwenden `end`-Werte, die zufällig mit beiden
  Interpretationen zum gleichen Ergebnis führen, decken die Diskrepanz also nicht auf.
  Fix: entweder Doku korrigieren („Länge des zu prüfenden Bereichs ab `start`" statt „Endposition"), oder Implementierung
  anpassen (`mid(start, end - start)`, falls tatsächlich eine Position gemeint war) – Rücksprache mit den Callern nötig,
  da beide Lesarten in Gebrauch sein könnten.

  ===> FIXED, Parameter umbenannt in "len" für Länge. Damit
  konsistent zum Rest der Klasse.

## Design

- [ ] **`wcstombs`/`mbstowcs` sind Locale-abhängig – widerspricht dem Refactoring-Ziel „UTF-8 überall, auch unter Windows"** (`String.cpp:267`, `set(const wchar_t*, size_t)`)
  ```cpp
  size_t formatted_bytes = ::wcstombs(ptr, str, outbytes);
  ```
  Das Verhalten hängt komplett von `setlocale(LC_CTYPE, ...)` ab; der Klassenkommentar selbst warnt, dass der Default
  „C" ist (string.h:66-68), wodurch jede Konvertierung mit Nicht-ASCII-Zeichen fehlschlägt (`CharacterEncodingException`).
  `<pplib/core/iconv.h>` wird inkludiert (Zeile 48), aber nirgends in `String.cpp` tatsächlich verwendet – toter Include,
  vermutlich Rest eines angefangenen, aber nicht fertiggestellten Umbaus auf eine explizite UTF-8-Konvertierung.
  Zusätzlich referenziert die Doku an zwei Stellen (string.h:143, string.h:625) eine Methode `String::setGlobalEncoding`
  mit Standardwert UTF-8 – **diese Methode existiert nicht** (grep über das gesamte Repository liefert außer den beiden
  Doku-Kommentaren und einer auskommentierten Test-Zeile keinen Treffer). Die Dokumentation beschreibt also eine API, die
  nie implementiert wurde (oder aus ppl7 nicht übernommen wurde) und suggeriert fälschlich eine Locale-unabhängige
  UTF-8-Garantie. `WideString` hat exakt dasselbe Muster (`WideString.cpp:265`, `mbstowcs`, sowie dieselbe
  `setGlobalEncoding`-Doku-Referenz in `widestring.h:66/518`).
  Fix: entweder `setGlobalEncoding`/`getGlobalEncoding` tatsächlich implementieren (fest auf UTF-8 mit `iconv`, unabhängig
  von der Prozess-Locale) und `wcstombs`/`mbstowcs` dadurch ersetzen, oder die Doku entfernen/korrigieren, falls die
  Locale-Abhängigkeit bewusst beibehalten wird.

- [ ] **Byte- statt Zeichen-Zählung: `left()`/`right()`/`mid()`/`chopLeft()`/`chopRight()` können mitten in einer UTF-8-Mehrbyte-Sequenz schneiden** (`String.cpp:718-743, 915-934`)
  Die Klasse speichert UTF-8 als Byte-Folge und alle Positions-/Längenangaben sind Byte-Offsets. Das ist für eine
  Byte-orientierte Klasse (ähnlich `std::string`) an sich legitim, aber `len()`/`size()`/`length()` dokumentieren
  explizit „Anzahl Zeichen" (string.h:255-289), nicht Bytes – irreführend für Mehrbyte-Inhalte:
  ```cpp
  pplib::String s(u8"hällo");   // "hällo", ä = 2 Bytes UTF-8, s.size() == 6 (nicht 5!)
  pplib::String cut = s.left(2);     // erwartet ggf. "hä" (2 Zeichen), tatsächlich "h" + 0xC3 (halbes ä)
                                      // -> ungültige UTF-8-Sequenz
  ```
  Angesichts des Refactoring-Ziels „Tests gehen von UTF-8 aus" ist das kein Rand-, sondern ein Kernfall (z.B. deutsche
  Umlaute in jedem zweiten Testfall). Ein Codepoint-bewusstes Pendant (oder zumindest ein dokumentierter Hinweis, dass
  Schnitt-Operationen byteweise arbeiten und der Aufrufer für gültige UTF-8-Grenzen selbst sorgen muss) fehlt.
  Fix (minimal): Doku von `len()`/`size()`/`left()`/`right()`/`mid()` präzisieren („Byte-Länge", „Byte-Position"). Für
  echte UTF-8-Sicherheit wäre eine optionale Codepoint-bewusste Variante (oder Validierung/Clamping auf Zeichengrenzen)
  nötig – das betrifft dann aber vermutlich WideString als Konvertierungsweg für UTF-8-sichere Teilstring-Bildung.

- [ ] **Text-Suchfunktionen sind nicht binär-sicher, obwohl die Klasse „beliebige Zeichen" verspricht** (`String.cpp:963-1057`)
  `strchr()`, `strrchr()`, `strstr()`, `find()`, `has()` sowie `strcmp`/`strCaseCmp` ohne explizite `size` basieren auf
  `::strchr`/`::strstr`/`::strcasecmp`, die alle beim ersten `\0`-Byte abbrechen. Enthält der String (der laut
  Klassendoku „beliebige Zeichen enthalten kann", string.h:61) ein eingebettetes Nullbyte, liefern diese Methoden
  stillschweigend falsche/unvollständige Ergebnisse statt einer Exception oder eines dokumentierten Hinweises. Das ist
  vermutlich für die meisten Anwendungsfälle (Text) irrelevant, sollte aber zumindest in der Klassendoku als
  Einschränkung („kein eingebettetes Nullbyte in Textverarbeitungsmethoden") vermerkt werden.

## Doku / Kosmetik

- [ ] `String::isTrue()` akzeptiert zusätzlich `"t"` als Wahr-Wert (`String.cpp:208`), was in der Methoden-Doku
      (string.h:335-348: nur "true", "wahr", "yes", "ja") nicht erwähnt wird.
- [ ] `get(ssize_t)`, `operator[](ssize_t) const` und `operator[](ssize_t)` (`String.cpp:564-583`) enthalten dieselbe
      Bounds-Check-Logik dreifach dupliziert – Kandidat für eine private, gemeinsam genutzte Hilfsfunktion.
- [ ] `String::vasprintf()` (`String.cpp:367-389`) reserviert mit `reserve(size + 1)`, wobei `reserve()` intern selbst
      nochmal `+1` für den Nullterminator addiert – es wird ein Byte mehr alloziert als nötig. Harmlos, aber verwirrend
      beim Lesen.
- [ ] `#include <pplib/core/iconv.h>` in `String.cpp:48` wird nirgends verwendet (siehe Design-Punkt zu `wcstombs`).

## Verifiziert OK (kein Handlungsbedarf)

- Self-Assignment/Self-Overlap-Schutz in `set(const char*, size_t)`, `append(const char*, size_t)`,
  `prepend(const char*, size_t)` und `repeat(const String&, size_t)` über Bereichsvergleich + `String temp_holder` ist
  korrekt implementiert; `append_same_string`-Unittest (`tests/src/types/string.cpp:715-723`, `a.append(a)` zweimal
  hintereinander) bestätigt das.
- Move-Konstruktor und Move-Assignment (`String(String&&)`, `operator=(String&&)`) setzen die Quelle korrekt zurück und
  prüfen `this != &other`.
- `reserve()` prüft `size == SIZE_MAX` gegen Integer-Wrap und wirft `IllegalArgumentException`; bei `realloc`-Fehlschlag
  bleibt der alte, gültige `ptr` erhalten (kein Double-Free/Leak).
- `shl()`/`shr()` erhalten die dokumentierte Gesamtlänge korrekt (Padding bei `c != 0`, Verkürzung bei `c == 0`).
- `operator+`-Familie und `operator<<` sind konsistent implementiert, keine Self-Assignment-Fallstricke, da immer über
  Kopien (`String s = str1;`) gearbeitet wird.
