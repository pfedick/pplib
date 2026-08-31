# Variant Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/variant.h` + `src/types/Variant.cpp` (vollständig gelesen).
Exceptions verifiziert in `include/pplib/exceptions.h`: `EmptyDataException`, `TypeConversionException` existieren beide (`PPLIBEXCEPTION`-Makro, Zeilen 50/51).
Verwendung von `Variant` per grep geprüft in `include/pplib/types/assocarray.h` (`set(key, value)` → `createTree(key)->set(value)`, Zeilen 326-372) und `src/types/AssocArray.cpp` (`createTree()`, Zeilen 113-159: bestehender Knoten wird per `it->second->clear()` geleert, *bevor* der neue Wert hineingeschrieben wird) sowie `src/core/Json.cpp` (keine zusätzlichen Aliasing-Muster gefunden).
Kontext ByteArray/ByteArrayPtr-Review (`todo/bytearray-review.md`) verifiziert bzgl. der dort gefundenen LSP-Verletzung (`ByteArrayPtr::truncate` nicht virtuell) – relevant für einen Doku-Punkt unten.

## Bugs (kritisch)

- [ ] **Self-Assignment (`v = v;` / `v.set(v)`) löscht den Inhalt still auf `TYPE_UNKNOWN`** (`Variant.cpp:190-193`, `variant.h:1275-1279`)
  ```cpp
  Variant& Variant::set(const Variant& value)
  {
      clear();                       // this->value wird gelöscht, this->value = nullptr, t = TYPE_UNKNOWN
      if (!value.value) return *this;   // <- bei Self-Assignment ist "value" dasselbe Objekt wie "this"!
      ...
  ```
  `clear()` setzt `this->value` auf `nullptr`. Ist `value` (der Parameter) eine Referenz auf `*this` selbst, ist `value.value` durch denselben Seiteneffekt jetzt ebenfalls `nullptr` – die Prüfung `if (!value.value)` greift und die Funktion kehrt sofort zurück, mit `this->value == nullptr` und `t == TYPE_UNKNOWN`. Der ursprüngliche Inhalt ist weg, ohne dass irgendetwas kopiert wurde.
  `operator=(const Variant&)` (variant.h:1275) delegiert direkt an `set()` und hat – anders als `operator=(Variant&&)` (variant.h:1288, mit explizitem `if (this != &other)`-Guard) – **keinen** Self-Assignment-Schutz.
  Repro:
  ```cpp
  Variant v = pplib::String("hello");
  v = v;                 // oder: v.set(v);
  // v.type() == Variant::TYPE_UNKNOWN – "hello" ist verloren
  ```
  Auch über die öffentliche `AssocArray`-API real erreichbar, z.B. `a.set("x", a.get("x"));` (`AssocArray::set(key, const Variant&)` → `createTree(key)->set(value)`, wobei `createTree` den Zielknoten – der zufällig derselbe wie `value` ist – bereits selbst per `clear()` leert, siehe `AssocArray.cpp:143`).
  Fix: Guard am Anfang von `set(const Variant&)`:
  ```cpp
  Variant& Variant::set(const Variant& value)
  {
      if (this == &value) return *this;
      ...
  ```
  Zusätzlich empfehlenswert: denselben Guard auch direkt in `operator=(const Variant&)` ergänzen, damit der Vertrag zu `operator=(Variant&&)` symmetrisch ist und nicht implizit von der internen `set()`-Implementierung abhängt.

- [ ] **`set(const T&)` / `set(T&&)` aller konkreten Typen: Use-after-free bei Selbstreferenz** (`Variant.cpp`, alle typspezifischen Overloads, z.B. String 253-267, WideString 269-283, Array 285-299, AssocArray 301-315, ByteArray 317-331, ByteArrayPtr 333-347, DateTime 349-363, Date 365-379, Time 381-395, TimeDelta 397-411, TimeZone 413-427)
  Jeder dieser Overloads folgt demselben Muster:
  ```cpp
  Variant& Variant::set(const AssocArray& value)
  {
      clear();                          // löscht das aktuell gespeicherte Objekt
      this->value = new AssocArray(value);  // <- "value" kann genau dieses gelöschte Objekt referenzieren!
      t = TYPE_ASSOCARRAY;
      return *this;
  }
  ```
  Übergibt der Aufrufer eine Referenz auf den *aktuell in diesem Variant gespeicherten* Wert (z.B. über `toAssocArray()`/`toString()` geholt), zerstört `clear()` genau dieses Objekt, **bevor** `new T(value)` davon liest. `value` ist danach eine dangling reference – klassisches Use-after-free, kein sauberer Absturz wie bei Bug 1 oben, sondern undefiniertes Verhalten (kann in Release-Builds mit "gültig aussehendem" Speicher scheinbar funktionieren, schlägt unter ASan/Valgrind oder mit Debug-Allocator sofort zu).
  Repro:
  ```cpp
  Variant v = pplib::AssocArray();
  v = v.toAssocArray();      // äquivalent: v.set(v.toAssocArray());
  // clear() löscht das AssocArray-Objekt, auf das "value" zeigt,
  // "new AssocArray(value)" kopiert anschließend aus bereits freigegebenem Speicher
  ```
  Betrifft identisch auch die Move-Overloads, z.B. `v.set(std::move(v.toAssocArray()));` – noch offensichtlicher falscher Aufruf, aber derselbe UB-Pfad.
  Fix: neues Objekt **zuerst** aus `value` konstruieren, erst danach den alten Speicher freigeben:
  ```cpp
  Variant& Variant::set(const AssocArray& value)
  {
      AssocArray* p = new AssocArray(value);   // value ist hier noch gültig
      clear();
      this->value = p;
      t = TYPE_ASSOCARRAY;
      return *this;
  }
  ```
  Dieses Muster (Konstruktion vor `clear()`) behebt gleichzeitig Bug 1 oben, sofern konsequent auf alle ~20 Overloads angewendet, und verbessert nebenbei die Exception-Safety (siehe Bugs (mittel)).

## Bugs (mittel)

- [ ] **`set()` hat keine Exception-Safety – Originalwert geht bei Fehlschlag der Neuzuweisung verloren** (alle `set(const T&)`-Overloads, z.B. `Variant.cpp:253-259`)
  `clear()` läuft in jedem `set()` als Erstes und zerstört den alten Wert unbedingt. Wirft die anschließende `new T(value)`-Konstruktion (z.B. `std::bad_alloc`, oder eine werfende Kopie/Move im Zieltyp selbst), bleibt `*this` als `TYPE_UNKNOWN` zurück – der alte Inhalt ist endgültig weg, obwohl die Operation fehlgeschlagen ist. Nur Basisgarantie statt starker Garantie.
  Ein eng verwandtes Verhalten wurde im AssocArray-Review für `createTree()` bei `BadAlloc` bereits bewusst akzeptiert ("Aufwand lohnt sich nicht"); hier betrifft es aber direkt jeden `set()`-Aufruf am Variant selbst, nicht nur den Sonderfall verschachtelter AssocArrays.
  Fix: identisch zum Fix der beiden kritischen Bugs oben – neues Objekt zuerst konstruieren, `clear()` erst nach erfolgreicher Konstruktion aufrufen.

## Design

- [ ] **Kein `default:`-Zweig in `switch(t)`** – `clear()` (Variant.cpp:151-185), `set(const Variant&)` (Variant.cpp:194-238) und `operator==` (Variant.cpp:586-609)
  Wird künftig ein neuer `DataType`-Wert ergänzt und in einem dieser drei Switches vergessen, verhält sich der Code still falsch statt zu knallen: `clear()` gibt den Pointer nie frei (Leak, da `value=nullptr` am Ende trotzdem gesetzt wird), `operator==` fällt durch den Switch und liefert `true` für tatsächlich unterschiedliche Werte. Aktuell nur durch `-Wswitch`-Compilerwarnungen (sofern aktiviert) abgesichert.
  Fix: `default: throw ...` bzw. `assert(false)` ergänzen, damit ein vergessener Case beim nächsten neuen Typ sofort auffällt.

- [ ] **Sehr breite, komplett implizite, bidirektionale Konvertierungsfläche**
  11 nicht-`explicit` Ein-Parameter-Konstruktoren + 22 implizite Konvertierungsoperatoren (`operator T&` / `operator const T&`) + 22 `operator=`-Overloads für dieselben 11 Typen (String, WideString, Array, AssocArray, ByteArray, ByteArrayPtr, DateTime, Date, Time, TimeDelta, TimeZone). Das ist zwar der Kernzweck von `Variant`, erzeugt aber ein erhebliches Mehrdeutigkeits-Risiko bei überladenen Funktionen (z.B. eine Funktion mit `f(const String&)` und `f(const WideString&)` – ein `Variant`-Argument ist dann je nach Kontext ambig oder wählt überraschend die "falsche" Konvertierung). Passt zum Refactoring-Ziel "Modernisierung" – für Bestandscode aber vermutlich zu invasiv für einen Rutsch, eher etwas für die große API-Aufräumrunde.

- [ ] **Move-Konstruktor/-Zuweisung nicht `noexcept`** (`variant.h:116`, `variant.h:1288`)
  Beide bestehen nachweislich nur aus Pointer-/Tag-Übernahme und können nicht werfen. Ohne `noexcept` fällt z.B. `std::vector<Variant>` bei Reallocation auf Copy statt Move zurück (Standard-Verhalten von `std::vector` mit `std::move_if_noexcept`). Passt zur C++17-Modernisierung aus REFACTORING.md, geringer Aufwand.

- [ ] **~20 fast identische `set()`/`operator=()`-Paare** (Variant.cpp, komplette Datei)
  Jeder Overload ist `clear(); this->value = new T(value); t = TYPE_T; return *this;` mit nur T/TYPE_T unterschiedlich. Kandidat für eine private Hilfsfunktion/Template, das den oben empfohlenen Fix ("neu konstruieren vor `clear()`") an einer einzigen Stelle statt in ~20 Kopien korrekt umsetzt – reduziert das Risiko, den Fix nur teilweise anzuwenden.

- [ ] **Langfristig: `std::variant`-Basis erwägen**
  Die komplette manuelle `void*` + Tag + Switch-Verwaltung (inkl. aller oben gefundenen Aliasing-Bugs) wäre mit einem `std::variant<String, WideString, Array, AssocArray, ...>`-Unterbau strukturell ausgeschlossen (Value-Semantik, kein manuelles `new`/`delete`, `std::visit` statt Switch). Passt zur in REFACTORING.md offenen Frage "C++17 oder C++20 als Standard?" – nur als Hinweis, kein akuter Handlungsbedarf.

## Doku / Kosmetik

- [ ] `toByteArrayPtr()` / `toByteArrayPtr() const` (Variant.cpp:499-511) akzeptieren neben `TYPE_BYTEARRAYPTR` stillschweigend auch `TYPE_BYTEARRAY` (Cast auf die Basisklasse). Weder Funktionsname noch Header-Doku (`variant.h:816-838`) erwähnen das. In Kombination mit der im ByteArray-Review dokumentierten LSP-Verletzung (`ByteArrayPtr::truncate` nicht virtuell) kann ein Aufrufer über `variant.toByteArrayPtr().truncate(n)` unbemerkt die invariantenverletzende Basisklassen-Methode statt der `ByteArray`-Version aufrufen. Fix: mindestens in der Doku erwähnen.
- [ ] Alle im `.cpp` definierten Konstruktoren (z.B. `Variant.cpp:71-146`) setzen redundant `this->value = nullptr; t = TYPE_UNKNOWN;`, bevor `set()` aufgerufen wird – das leisten die In-Class-Defaults (`variant.h:89-90`) bereits automatisch. Unschädlich, aber unnötiges Boilerplate.
- [ ] `enum DataType` trägt bereits den Kommentar `// TODO: Das sollte eine enum class werden` (`variant.h:71`) – deckt sich mit dem obigen Design-Punkt zum fehlenden `default:`-Zweig; eine `enum class` mit vollständigem `switch` erzwingt eine Compiler-Warnung bei vergessenen Cases.

## Verifiziert OK (kein Handlungsbedarf)

- Move-Konstruktor (`Variant(Variant&&)`, Variant.cpp:63-69) korrekt: kein Leak, `other` wird sauber zurückgesetzt.
- `operator=(Variant&&)` (variant.h:1288) hat einen korrekten Self-Move-Guard (`this != &other`).
- `clear()` setzt in allen (nicht-aliasierten) Fällen korrekt `value = nullptr` und `t = TYPE_UNKNOWN`, kein Doppel-Free im Normalablauf beobachtet.
- Alle `toX()`/`toX() const`-Getter prüfen konsistent zuerst `EmptyDataException` (leer), dann `TypeConversionException` (falscher Typ) – Reihenfolge stimmt mit der Header-Doku überein, durchgängig für alle 11 Typen.
- `operator==`/`operator!=`: Typvergleich (`t != other.t`) zuerst, danach Delegation an den jeweiligen Werttyp; zwei `TYPE_UNKNOWN`-Variants gelten korrekt als gleich (fallen durch den Switch, `return true` danach).
- `clear()` für `TYPE_BYTEARRAYPTR` löscht nur den Wrapper, nicht den referenzierten Speicher – konsistent mit der Nicht-Eigentümer-Semantik von `ByteArrayPtr`.
- Der `static_cast<ByteArrayPtr*>`-Reinterpret eines tatsächlich als `ByteArray` gespeicherten `void*` in `toByteArrayPtr()` ist speicherlayout-technisch korrekt (öffentliche, nicht-virtuelle Single-Inheritance, `ByteArrayPtr`-Subobjekt liegt am Offset 0) – nur die fehlende Dokumentation ist ein Thema (siehe oben).
