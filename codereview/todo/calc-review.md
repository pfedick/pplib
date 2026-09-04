# Calc Review – Befunde

Review vom 2026-09-01, Scope: `src/math/calc.cpp` (241 Zeilen). Öffentliche API: `double Calc(const String& expression)`,
deklariert in `include/pplib/core/functions.h:84` (keine Doku-Kommentare dort, im Gegensatz zu `Crc32`/`Random`).
Empirisch verifiziert gegen einen selbst gebauten ASAN-Build der kompletten `core`-Sources (calc.cpp, String.cpp, etc.
mit `-fsanitize=address`) sowie gegen `build/coverage/libpplib_core.a`.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **Stack-Overflow durch unbegrenzte Rekursion bei verschachtelten Klammern** (`Tokenize`, Zeile 166-179)
  Jede öffnende Klammer führt zu einem rekursiven Aufruf von `Tokenize()` (Zeile 179), bevor der aktuelle Aufruf
  fortfährt. Die Rekursionstiefe ist exakt die Klammer-Verschachtelungstiefe der Eingabe – es gibt keine Grenze.
  Empirisch verifiziert: mit `ulimit -s` = 8192 (Standard) crasht ein Ausdruck der Form `"("*n + "1" + ")"*n` für
  `n=50000` (Ausdruckslänge ca. 100 KB) reproduzierbar mit SIGSEGV (`Speicherzugriffsfehler (Speicherabzug
  geschrieben)`, Exit-Code 139); bei `n=20000` läuft es noch durch. Ein Angreifer, der eine Formel/einen
  Konfigurationswert an `Calc()` übergeben kann, kann so den Prozess mit einer harmlos aussehenden ca. 100 KB
  großen Zeichenkette abschießen (DoS).
  ```cpp
  tokenlist.push_back(CalcToken(CalcToken::TYPE_VALUE, Tokenize(s, buffer)));  // Zeile 179 – 1 Rekursionsebene je "("
  ```
  Fix: Rekursionstiefe zählen (Parameter oder thread-lokaler Zähler) und ab einem sinnvollen Maximum (z.B. 100)
  `SyntaxException("Expression too deeply nested")` werfen, oder auf einen iterativen Klammer-Stack umstellen.

- [ ] **Fehlende schließende Klammer: toter `throw`, danach Out-of-Bounds-Read (verifiziert mit AddressSanitizer)** (`findClosingBracket`, Zeile 147-164; `Tokenize`, Zeile 180+188)
  ```cpp
  static int findClosingBracket(const char* ptr)
  {
      ...
      printf("Ups\n");
      return p;
      throw SyntaxException("Closing Bracket not found: %s", ptr);   // unreachable – toter Code nach `return`
  }
  ```
  Der `throw` ist durch das vorangehende `return p;` nie erreichbar (jeder halbwegs aufmerksame Compiler würde das
  als "unreachable code" anmerken). Bei fehlender schließender Klammer liefert die Funktion stattdessen `p` =
  Länge des Rest-Strings (Position der Null), und schreibt außerdem einen Debug-`printf("Ups\n")` nach stdout.
  Der Aufrufer verrechnet das ungeprüft weiter:
  ```cpp
  ptr += lastbracket + 2;   // Zeile 180 – springt mindestens 1 Byte hinter den Null-Terminator
  ```
  Mit AddressSanitizer (eigener Build aus den Quellen, da die vorgebaute `libpplib_core.a` ohne ASAN kompiliert
  ist) reproduzierbar:
  ```
  ==ERROR: AddressSanitizer: heap-buffer-overflow ... READ of size 1
      #0 pplib::Tokenize(pplib::String const&, char*) src/math/calc.cpp:188
  0x... is located 0 bytes after 7-byte region ...
      allocated by pplib::String::reserve() <- String::set() <- String::String(String const&) <- Calc()
  ```
  Ausgelöst durch `Calc("1+(2+3")` (fehlende schließende Klammer). Je nach Inhalt des Speichers hinter dem
  String-Puffer liefert das ein falsches Ergebnis, eine falsche `SyntaxException` an unerwarteter Stelle, oder
  – wie hier verifiziert – einen echten Heap-Buffer-Overflow-Read.
  Fix: den toten Code entfernen und tatsächlich werfen, wenn keine schließende Klammer gefunden wurde:
  ```cpp
  static int findClosingBracket(const char* ptr)
  {
      int p = 0, count = 1, c;
      while ((c = ptr[p]) != 0) {
          if (c == '(') count++;
          else if (c == ')') { if (--count == 0) return p; }
          p++;
      }
      throw SyntaxException("Closing Bracket not found");
  }
  ```
  Der `printf("Ups\n")` gehört ebenfalls komplett entfernt.

## Bugs (mittel)

- [ ] **Division durch 0 liefert `inf`/`nan` statt Exception** (`CalcType`, Zeile 108)
  ```cpp
  if (type == CalcToken::TYPE_DIVIDE) return v1 / v2;
  ```
  Verifiziert: `Calc("1/0")` liefert `inf`, keine Exception. Für einen Taschenrechner/Expression-Parser, der an
  anderer Stelle konsequent `SyntaxException` für ungültige Eingaben wirft, ist das inkonsequent – ein
  Aufrufer, der das Ergebnis z.B. in einer Konfiguration weiterverwendet, bekommt eine stille `inf`/`nan`-Kontamination
  statt eines Fehlers.
  Fix: `if (v2 == 0.0) throw SyntaxException("Division by zero");` vor der Division.

- [ ] **Schiebeoperatoren mit Schiebeweite außerhalb [0,63] sind Undefined Behaviour** (`CalcType`, Zeile 110-111)
  ```cpp
  if (type == CalcToken::TYPE_SHIFT_LEFT) return (uint64_t)v1 << (int)v2;
  if (type == CalcToken::TYPE_SHIFT_RIGHT) return (uint64_t)v1 >> (int)v2;
  ```
  Ein Shift um `>= 64` oder um eine negative Zahl ist laut C++-Standard undefiniertes Verhalten. Verifiziert
  (aktuell "nur" durch x86-Maskierung des Shift-Counts sichtbar, kein Crash, aber sinnlose Ergebnisse):
  ```
  Calc("1<<100")  = 68719476736       (= 1<<36, da 100 mod 64 == 36 auf x86)
  Calc("1<<-5")   = 576460752303423488  (= 1<<59, negativer int als riesiger Shift interpretiert)
  ```
  Fix: `int shift = (int)v2; if (shift < 0 || shift > 63) throw SyntaxException("Shift out of range");`

## Design

- [ ] **Undokumentierte, unüblich hohe Präzedenz der Shift-Operatoren** (`Tokenize`, Zeile 216-222)
  ```cpp
  ResolveType(tokenlist, CalcToken::TYPE_SHIFT_LEFT);
  ResolveType(tokenlist, CalcToken::TYPE_SHIFT_RIGHT);
  ResolveType(tokenlist, CalcToken::TYPE_POWER);
  ResolveType(tokenlist, CalcToken::TYPE_MULTIPLY);
  ResolveType(tokenlist, CalcToken::TYPE_DIVIDE);
  ResolveType(tokenlist, CalcToken::TYPE_PLUS);
  ResolveType(tokenlist, CalcToken::TYPE_MINUS);
  ```
  Diese Reihenfolge macht `<<`/`>>` zur höchsten Präzedenzstufe – höher als Potenz, Punkt- und
  Strich-Rechnung. Verifiziert: `Calc("2^2<<1")` = 16 (= 2^(2<<1) = 2^4), nicht 8 (= (2^2)<<1), wie man es aus
  C/C++/den meisten Sprachen erwarten würde (dort hat `<<` eine sehr niedrige Präzedenz). `functions.h` dokumentiert
  `Calc()` überhaupt nicht (kein `@brief`, keine Erläuterung der Operatoren oder ihrer Reihenfolge) – jeder Nutzer
  muss das über Trial-and-Error oder Quellcode-Lektüre herausfinden.
  Fix: mindestens dokumentieren; ggf. Präzedenz an gängige Konventionen anpassen (Shift zwischen Additions- und
  Vergleichsoperatoren, falls letztere je hinzukommen).

- [ ] **`Tokenize()` und `GetValue()` sind interne Hilfsfunktionen ohne `static`** (Zeile 69, 166)
  Im Gegensatz zu `CalcType`, `ResolveType` und `findClosingBracket` (alle `static`) haben `Tokenize` und
  `GetValue` externe Sichtbarkeit, obwohl sie nirgends in `functions.h` deklariert sind und nur innerhalb dieser
  Datei aufgerufen werden. Das exportiert zwei implementierungsinterne Symbole `pplib::Tokenize`/`pplib::GetValue`
  unnötig aus der Bibliothek (ODR-Kollisionsrisiko, größere Symboltabelle).
  Fix: beide als `static` markieren.

- [ ] **`ResolveType` verlässt sich stillschweigend auf die Alternierungs-Invariante der Tokenliste** (Zeile 115-145)
  ```cpp
  before = it; before--;
  after = it; after++;
  if (before != tokenlist.end() && after != tokenlist.end()) { ... }
  ```
  Die Sicherheit dieses Codes (kein Zugriff vor `begin()`/hinter das letzte Element) beruht vollständig darauf,
  dass `Tokenize()` immer eine strikt alternierende Wert/Operator/Wert-Sequenz erzeugt. Wird diese Invariante
  durch einen Bug wie den oben beschriebenen OOB-Read je verletzt (z.B. Tokenliste beginnt/endet mit einem
  Operator), ist das Verhalten von `before--`/`after++` nicht mehr abgesichert. Kein eigenständig reproduzierbarer
  Fund, aber ein Robustheits-Punkt, der sich mit der Behebung des Klammer-Bugs mit erledigt.

## Doku / Kosmetik

- [ ] **Debug-Leftover `printf("Ups\n")`** (`findClosingBracket`, Zeile 161) – landet in Produktions-Code auf stdout,
      siehe kritischer Fund oben.

- [ ] **Toter `#ifdef DEBUG`-Codepfad ruft nirgends definiertes `PrintTokenList()` auf** (Zeile 137, 214, 225)
  ```cpp
  #ifdef DEBUG
      printf("After erase\n");
      PrintTokenList(tokenlist);
  #endif
  ```
  `PrintTokenList` ist in der gesamten Codebase nicht deklariert/definiert (grep-verifiziert). Würde jemand
  `#define DEBUG 1` (Zeile 43, aktuell auskommentiert) wieder aktivieren, kompiliert die Datei nicht mehr.
  Fix: entweder `PrintTokenList` implementieren oder die toten `#ifdef DEBUG`-Blöcke ganz entfernen.

- [ ] **Schwer lesbare Pointer-Dereferenzierung `*ptr[0]`** (`GetValue`, mehrfach, z.B. Zeile 73, 78, 85)
  Funktional identisch zu `(*ptr)[0]` bzw. `**ptr` (liefert das aktuelle Zeichen), aber wegen der
  `[]`-vor-`*`-Präzedenz beim Lesen leicht mit `ptr[0][0]` zu verwechseln. Empfehlung: `(*ptr)[0]` schreiben.

## Verifiziert OK (kein Handlungsbedarf)

- Grundrechenarten, Klammerungen, Vorzeichen (`"3*-2"`, `"-3+2"`, `"3--2"`), Komma-als-Dezimaltrennzeichen
  (`current.replace(",", ".")`) sowie Leerzeichen/Tab-Entfernung in `Calc()` funktionieren korrekt (empirisch
  gegen die gebaute Bibliothek getestet).
- Der Buffer-Bounds-Check in `GetValue` (Zeile 97, `if (bptr >= CALC_BUFFERSIZE) throw ...`) sitzt trotz
  ungewöhnlicher Position (nach dem Schreiben/Inkrementieren statt davor) tatsächlich an der richtigen Stelle:
  der letzte gültige Schreibzugriff erfolgt bei `bptr == CALC_BUFFERSIZE-1`, die Exception greift, bevor je
  außerhalb des 64-Byte-Buffers geschrieben wird – kein Off-by-one.
- `Calc("")` liefert `0.0` ohne Exception (dokumentiert ist das nicht, aber kein Crash/UB).
