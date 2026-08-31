# Array Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/array.h` + `src/types/Array.cpp`.
Verifiziert: `pplib::exceptions.h` (`OutOfBoundsException`, `EmptyDataException`, `OutOfMemoryException` existieren wie zitiert),
`pplib/core/functions.h` (dort zweite Deklaration von `Sort`/`SortReverse`, siehe unten), `pplib/types/string.h`
(`String` hält intern nur `char* ptr` – kein SSO, Move-Ctor `noexcept`, `operator<`/`operator const char*`/`getPtr()` vorhanden),
bestehende Testsuite `tests/src/types/array.cpp` (86 Tests, laufen gegen den aktuellen Stand alle grün – als Vertrags-Referenz benutzt,
z.B. für die Limit-Semantik von `explode()`, die dadurch **nicht** als Bug gilt).
Alle unten als "kritisch" markierten Bugs wurden nicht nur gelesen, sondern gegen die echte, kompilierte Bibliothek
(`build/coverage/libpplib_core.a`) reproduziert (Stand-alone-Programme mit echtem `pplib::Array`/`pplib::String`).

## Bugs (kritisch)

- [ ] **Self-Append `a.add(a)` / `a += a` zerstört Daten** (`Array.cpp:70-73`, `Array.cpp:296-300`)
  ```cpp
  void Array::add(const Array& other)
  {
      elements.insert(elements.end(), other.elements.begin(), other.elements.end());
  }
  ```
  Ist `other` dasselbe Objekt wie `*this` (`a += a` oder `a.add(a)`), ist `[other.elements.begin(), other.elements.end())`
  ein Bereich in den `elements`-Vektor selbst. Muss der Vektor beim Einfügen reallozieren, wird der alte Speicherblock
  (aus dem gerade noch gelesen werden müsste) bereits während des Kopiervorgangs freigegeben/verschoben – das Ergebnis
  sind leere Strings statt Duplikaten. Reproduziert mit dem echten `pplib::Array`:
  ```cpp
  Array a;
  a.reserve(2);
  a.add("aaaa..."); a.add("bbbb..."); a.add("cccc...");
  a.add(a);   // bzw. a += a;
  // a.size() == 6, aber a.get(3), a.get(4), a.get(5) sind "" statt Kopien von a,b,c
  ```
  Exakt dasselbe Bug-Muster wurde in `AssocArray::add()`/`operator+=` bereits gefunden und gefixt
  (siehe `done/assocarray-review.md`, "Self-Reference in add() / operator+= zerstört Leaf-Werte") – hier ist die
  Analogie in `Array` noch offen und durch keinen Test abgedeckt (`OperatorPlusEqual`-Test prüft nur `a1 += a2` mit
  unterschiedlichen Objekten).
  Fix: Self-Check am Anfang:
  ```cpp
  void Array::add(const Array& other)
  {
      if (this == &other) {
          Array copy(other);
          elements.insert(elements.end(), copy.elements.begin(), copy.elements.end());
          return;
      }
      elements.insert(elements.end(), other.elements.begin(), other.elements.end());
  }
  ```

- [ ] **Self-Insert `a.insert(index, a)` korrumpiert das Array** (`Array.cpp:124-133`)
  ```cpp
  void Array::insert(size_t index, const Array& other)
  {
      if (other.elements.empty()) return;
      if (index >= elements.size()) { elements.resize(index); add(other); return; }
      elements.insert(elements.begin() + index, other.elements.begin(), other.elements.end());
  }
  ```
  Gleiches Muster wie oben, nur mit `insert()` an einer mittleren Position statt `end()`. Reproduziert:
  ```cpp
  Array a;
  a.reserve(2);
  a.add("xxxx..."); a.add("yyyy..."); a.add("zzzz...");
  a.insert(1, a);
  // Ergebnis: [ "xxxx...", "", "yyyy...", "zzzz...", "yyyy...", "zzzz..." ]
  // statt der erwarteten Verdopplung [x, x,y,z, y, z] mit korrektem Inhalt an Index 1
  ```
  Index 1 ist leer statt "xxxx...", der Rest ist zwar der Länge nach passend, aber die Kopie ist durch die
  Reallokation während des Einfügens bereits kaputt. Fix analog zu oben: bei `this == &other` zuerst eine Kopie
  von `other` anlegen und aus der Kopie einfügen.

- [ ] **`Array::set()`: Dangling Reference, wenn `value` auf ein Element von `elements` verweist** (`Array.cpp:97-103`)
  ```cpp
  void Array::set(size_t index, const String& value)
  {
      if (index >= elements.size()) {
          elements.resize(index + 1);      // kann reallozieren!
      }
      elements[index] = value;             // value kann jetzt eine dangling reference sein
  }
  ```
  `get()`/`operator[]` geben `String&`/`const String&` **Referenzen** auf Elemente von `elements` zurück – ein
  ganz natürlicher Aufruf wie „letztes Element duplizieren“ (`a.set(a.count(), a.get(a.count()-1))`) bindet `value`
  an ein bestehendes Element. Löst `resize()` eine Reallokation aus, wird der alte Speicherblock, aus dem `value`
  liest, freigegeben, **bevor** `elements[index] = value` ausgeführt wird. Reproduziert mit dem echten `pplib::Array`
  (unter ASan als heap-use-after-free bestätigt, mit der echten Bibliothek als stille Datenkorruption):
  ```cpp
  Array a;
  a.reserve(2);
  a.add("aaaa..."); a.add("bbbb...");
  a.set(10, a.get(1));
  // a.get(10) liefert "" statt der erwarteten Kopie von "bbbb..."
  ```
  Betroffen ist auch `insert(size_t index, const String& value)` (`Array.cpp:115-122`), da es für `index >= size()`
  auf `set()` delegiert.
  Fix: `value` vor dem `resize()` in eine lokale Kopie nehmen:
  ```cpp
  void Array::set(size_t index, const String& value)
  {
      String tmp(value);           // Kopie ziehen, BEVOR resize() reallozieren kann
      if (index >= elements.size()) {
          elements.resize(index + 1);
      }
      elements[index] = std::move(tmp);
  }
  ```

- [ ] **Negativer/zu großer Index bei `set()`/`insert()`: Integer-Wraparound → Out-of-Bounds-Schreibzugriff** (`Array.cpp:97-103`)
  `set()`/`insert()` nehmen `size_t index` entgegen, während `get()`/`at()`/`operator[]` bewusst `ssize_t` verwenden und
  negative Werte als "von hinten zählen" interpretieren (dokumentiert und getestet, z.B. `a.get(-1)`). Ein Aufrufer, der
  in Analogie zu `get(-1)` versucht `a.set(-1, "x")` aufzurufen, bekommt das nicht – der `int`/`ssize_t`-Literal `-1`
  konvertiert stillschweigend (kein Compiler-Fehler/Warning in normaler Build-Konfiguration) zu `size_t` = `SIZE_MAX`.
  Damit läuft `index + 1` in `set()` in einen Integer-Overflow zu `0`, `elements.resize(0)` leert das Array, und
  anschließend greift `elements[index]` mit `index == SIZE_MAX` weit außerhalb des (jetzt leeren) Vektors zu.
  Reproduziert mit der echten Bibliothek – mit `_GLIBCXX_ASSERTIONS` (Standard-Hardening dieses Builds) ein sofortiger
  Abort, in einem Release-Build ohne Hardening wäre es ein spekulativer Schreibzugriff weit außerhalb des Heaps
  (Speicherkorruption):
  ```cpp
  Array a;
  a.add("x");
  a.set(-1, "y");
  // libstdc++: "vector::operator[]: Assertion '__n < this->size()' failed." -> Abort
  // ohne Hardening: Out-of-Bounds-Schreibzugriff auf Zufallsadresse
  ```
  Fix: `index` explizit gegen ein Maximum absichern (z.B. `if (index == static_cast<size_t>(-1)) throw OutOfBoundsException();`),
  oder – konsistenter zum Rest der Klasse – `set()`/`insert()` ebenfalls auf `ssize_t` mit derselben
  "negativ = von hinten"-Konvention wie `get()` umstellen.

## Bugs (mittel)

- [ ] **`Sort()`/`SortReverse()`: doppelte, inkonsistente Deklaration in zwei Headern** (`array.h:808`, `array.h:820` vs. `functions.h:150-151`)
  ```cpp
  // array.h
  Array Sort(const Array& array, bool unique);
  Array SortReverse(const Array& array, bool unique);
  // functions.h
  Array Sort(const Array& array, bool unique = false);
  Array SortReverse(const Array& array, bool unique = false);
  ```
  Beide Deklarationen beziehen sich auf dieselbe Definition in `Array.cpp`, aber nur `functions.h` liefert den
  Default-Wert für `unique`. Die Test-Datei `tests/src/types/array.cpp` ruft `Sort(a1)`/`SortReverse(a1)` mit nur
  einem Argument auf – das kompiliert nur, weil zusätzlich `functions.h` inkludiert wird. Wird `array.h` alleine
  eingebunden (was das Refactoring-Ziel "jede Klasse eigener Header" ja gerade nahelegt), bricht der Aufruf mit nur
  einem Argument. Verifiziert per Testcompile:
  ```
  error: zu wenige Argumente für Funktion »pplib::Array pplib::Sort(const Array&, bool)«
  ```
  Fix: Deklaration an einer Stelle konsolidieren (sinnvollerweise in `array.h`, da es Array-Funktionalität ist) und
  den Default-Wert dort setzen; `functions.h` sollte `array.h` inkludieren statt die Signatur zu duplizieren.

- [ ] **`set()`/`insert()` lassen rohe `std::length_error`/`std::bad_alloc` durch statt `OutOfMemoryException`** (`Array.cpp:97-103` vs. `Array.cpp:52-63`)
  `reserve()` fängt `std::bad_alloc`/`std::length_error` explizit ab und wirft stattdessen `OutOfMemoryException`
  (auch getestet: `reserveThrowsOutOfMemory`). `set()`/`insert()` rufen intern ebenfalls `elements.resize(...)` auf,
  ohne dieselbe Absicherung. Bei einem sehr großen (aber nicht überlaufenden) Index bricht die interne
  `std::vector`-Exception ungefiltert durch:
  ```cpp
  Array a;
  a.set(SIZE_MAX / sizeof(String) - 1, "y");
  // wirft std::length_error ("vector::_M_default_append"), NICHT pplib::OutOfMemoryException
  ```
  Das bricht den in `reserve()` etablierten Vertrag "Allokationsfehler = `OutOfMemoryException`" und zwingt
  Aufrufer, zusätzlich `std::exception` abzufangen, obwohl sie sich laut restlicher API auf pplib-Exceptions
  verlassen können sollten.
  Fix: dieselbe try/catch-Absicherung wie in `reserve()` auch um die `resize()`-Aufrufe in `set()`/`insert()` legen.

## Design

- [ ] **`getRest()` fehlt `const`** (`array.h:384`, `Array.cpp:202-210`)
  Die Methode liest nur (`for`-Schleife über `elements`, baut einen neuen `String` zusammen), verändert das Array
  nicht. Aktuell aber `String getRest(size_t index, const String& delimiter = " ");` ohne `const` – lässt sich daher
  nicht auf einem `const Array&` aufrufen, obwohl das inhaltlich möglich wäre. Passt zum Refactoring-Ziel
  Const-Korrektheit.
  Fix: `String getRest(size_t index, const String& delimiter = " ") const;`

- [ ] **Inkonsistenter Index-Vertrag zwischen `get()`/`at()`/`operator[]` (negativ = von hinten) und `set()`/`insert()`/`erase()` (nur `size_t`, kein "von hinten")**
  `get(ssize_t index)`, `at(ssize_t index)` und `operator[](ssize_t index)` unterstützen bewusst negative Indizes
  ("`-1`" = letztes Element, dokumentiert und getestet). `set()`, `insert()` und `erase()` nehmen dagegen `size_t`
  entgegen und haben dieses Konzept nicht – ein negativer Wert wird (siehe Bug oben) stillschweigend zu einem riesigen
  `size_t`. Für den Aufrufer ist nicht ersichtlich, warum die "lesenden" Methoden von hinten zählen können, die
  "schreibenden" aber nicht (bzw. dabei abstürzen). Entweder die Methoden vereinheitlichen (alle `ssize_t` mit
  gleicher Konvention), oder zumindest in der Doku explizit klarstellen, dass negative Werte bei `set()`/`insert()`/`erase()`
  nicht unterstützt und nicht abgefangen werden.

- [ ] **`sort()`/`sortReverse()`/`sortUnique()` und die freien Funktionen `Sort()`/`SortReverse()` implementieren dieselbe Logik fünf Mal, über den Umweg von `std::(multi)set`** (`Array.cpp:309-346`, `Array.cpp:379-423`)
  Alle fünf Funktionen bauen einen `std::set`/`std::multiset<String>` auf (jedes Element wird dabei per
  Baum-Knoten-Allokation ein zweites Mal kopiert), leeren dann `elements` und kopieren die Elemente ein drittes Mal
  zurück. Das ist sowohl langsamer als auch mehr Code, als nötig wäre. `makeUnique()` (`Array.cpp:348-359`) zeigt mit
  `std::remove_if` + `erase` bereits den saubereren, modernen Ansatz für dieselbe Klasse.
  Fix-Vorschlag (passt zu "Modernisierung der Klassen" aus REFACTORING.md):
  ```cpp
  void Array::sort() { std::sort(elements.begin(), elements.end()); }
  void Array::sortReverse() { std::sort(elements.begin(), elements.end(), std::greater<String>()); }
  void Array::sortUnique() { sort(); elements.erase(std::unique(elements.begin(), elements.end()), elements.end()); }
  ```
  Die freien Funktionen `Sort()`/`SortReverse()` könnten dann einfach eine Kopie anlegen und die entsprechende
  Member-Funktion aufrufen, statt die Logik erneut zu duplizieren.

- [ ] **`explode()` castet die Konstantheit von `String::getPtr()` unnötig weg** (`Array.cpp:243-245`)
  ```cpp
  const char* del = (const char*)delimiter;
  char* etext = (char*)text.getPtr();   // const wird weggecastet
  char* _t;
  ```
  `etext`/`_t` werden im gesamten weiteren Verlauf nur lesend verwendet (`strstr`, `strlen`, Zeigerarithmetik) –
  es gibt keinen Grund, den `const`-Rückgabewert von `getPtr()` wegzucasten. Rein kosmetisch/sicherheitshalber
  sollten `etext`/`_t` als `const char*` deklariert werden.

## Doku / Kosmetik

- [ ] Auskommentierte Zeile `// add(etext,p);` in `explode()` (`Array.cpp:259`) – toter Code, sollte entfernt werden.
- [ ] Doku von `set()`/`insert(size_t, ...)` (`array.h:151-175`, `array.h:177-214`) erwähnt nichts zum Verhalten bei
      sehr großen `index`-Werten (Exception? Absturz?) – nach Fix der obigen Bugs entsprechend nachdokumentieren.
- [ ] `Array::add(const String& value, size_t size)` (`Array.cpp:80-85`) nutzt implizit `String::operator const char*()`
      über `str.set(value, size)`, da es keine `String::set(const String&, size_t)`-Überladung gibt. Funktioniert,
      ist aber nicht auf den ersten Blick offensichtlich – ein Kommentar oder eine explizite Überladung wären lesbarer.

## Verifiziert OK (kein Handlungsbedarf)

- Self-Assignment `a1 = a1` ist sicher (`operator=(const Array&) = default`, deckt sich mit std::vector's
  Selbst-Zuweisungssicherheit) – abgedeckt durch Test `OperatorCopyOnSameObject`.
- Self-Move-Assignment `a1 = std::move(a1)` ist durch den expliziten `this != &other`-Guard in
  `operator=(Array&&)` (`array.h:560-566`) sicher – abgedeckt durch Test `OperatorMoveOnSameObject`.
- `operator+(a, a)` ist sicher: `ret` ist bereits eine unabhängige Kopie von `a1`, bevor `ret.add(a2)` mit dem
  zweiten (aliasing) Argument aufgerufen wird – anders als bei `add()`/`insert()` direkt liegt hier keine
  Selbstreferenz auf denselben Vektor vor.
- Selbstreferenzierendes `push_back`/`insert(pos, value)` mit einem einzelnen Element (z.B. `a.add(a.get(0))`,
  `insert(size_t, const String&)` im Zweig `index < size()`) ist **sicher** – seit LWG 526 garantiert der Standard,
  dass eine Referenz auf ein Element desselben Containers auch bei Reallokation korrekt kopiert wird. Empirisch
  bestätigt (im Gegensatz zum bereits gemeldeten `set()`-Bug, der einen separaten `resize()` + Zuweisung verwendet
  und dadurch **nicht** von dieser Garantie profitiert).
- `copy()` ist self-copy-safe (`elements = other.elements` mit `other == *this` entspricht `elements = elements`,
  von `std::vector::operator=` sicher behandelt).
- `erase()`/`shift()`/`pop()` ziehen den Rückgabewert sauber per `std::move` aus dem Vektor, bevor das Element
  entfernt wird – kein Double-Free, kein unnötiger Copy.
- `makeUnique()` ist bereits eine effiziente, moderne Implementierung (`std::remove_if` + einmaliges `erase`) –
  siehe Design-Hinweis oben zur Vereinheitlichung mit `sort()`/`sortReverse()`/`sortUnique()`.
- `explode()`'s Limit-Semantik (überzählige Elemente werden verworfen, nicht als Rest-Element angehängt wie z.B.
  bei PHPs `explode()`) ist durch `explodeWithLimitWithinEmptyLines1`/`...2` in `tests/src/types/array.cpp`
  explizit spezifiziert und getestet – kein Bug, sondern bewusster Vertrag.
- Alle 86 bestehenden Unit-Tests in `tests/src/types/array.cpp` laufen gegen den aktuellen Stand grün.
