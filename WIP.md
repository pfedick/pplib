Updated todo list

Hier ist die Zusammenfassung unseres aktuellen Stands:

---

# Zusammenfassung: Integration von `VariantArray` & neuer Datentypen in PPL8

## 1. Ziel
Ablösung des alten "Fake-Array"-Mechanismus in `AssocArray` (bei dem Arrays über `maxint` und numerische String-Keys in `std::map` simuliert wurden) durch eine echte Sequenz (`VariantArray`) sowie native Unterstützung der Datentypen für JSON/YAML (`Null`, `Boolean`, `Int64`, `Double`).

---

## 2. Erledigte Arbeiten

### A. `Variant`-Klasse erweitert & abgesichert
* **Neue Typen:** `TYPE_NULL` (`std::nullptr_t`), `TYPE_BOOL` (`bool`), `TYPE_INT64` (`int64_t`), `TYPE_DOUBLE` (`double`), `TYPE_VARIANTARRAY` (`VariantArray`).
* **C++20 Concepts gegen Ambiguitäten:**  
  Konstruktoren, `set()` und `operator=` für fundamentale Typen wurden mit `requires` abgesichert (z. B. `std::is_same_v<T, bool>`), damit `const char*` nicht mehr fälschlicherweise an `bool` bindet.
* **Typ-Getter:** `isNull()`, `isBool()`, `isInt64()`, `isInt()`, `isDouble()`, `isVariantArray()`, `toBool()`, `toInt64()`, `toInt()`, `toDouble()`, `toVariantArray()`.
* **Refactoring `clear()`:** Allokationsfreigaben wurden über das Template `deleteValue<T>()` vereinheitlicht.
* **Tests:** Vollständige Test-Suite in `variant.cpp` integriert und erfolgreich (`test_core`).

### B. `VariantArray`-Klasse implementiert
* **Container:** Dünner, robuster Wrapper um `std::vector<Variant>`.
* **Methoden:** `add()`, `append()` (Alias für `add`), `extend()`, `get()`, `set()`, `insert()`, `erase()`, `pop()`, `shift()`, `has()`, `indexOf()`, `toArray()`, STL-Iteratoren und Vergleichsoperatoren.
* **Sicherheit:** Sink-Parameter (Pass-by-Value) für `add`, `set`, `insert` verhindern Use-After-Free bei Selbstreferenzen/Reallokation.
* **Binär-Serialisierung:** `exportBinary` / `importBinary` mit Magic Bytes `"PPL8VAAR"`, Version 1 und sequentieller Speicherung (ohne Index-Overhead).

### C. Binär-Serialisierung ausgelagert (`VariantExportImport.cpp`)
* **`exportVariantBinary` / `importVariantBinary`:**  
  Zentrale Serialisierungslogik für einzelne `Variant`-Werte, wiederverwendbar für `AssocArray` und `VariantArray`.
* **Optimierungen:**
  * Typen mit fester Größe (`Null`, `Bool`, `Int64`, `Double`, `Date`, `Time`, `TimeDelta`) schreiben kein redundantes 4-Byte-Längenfeld.
  * `Double` wird verlustfrei und portabel via `std::bit_cast<uint64_t>` und `PokeN64` (Big-Endian) gespeichert.
  * `WideString` wird plattformunabhängig als UTF-8 exportiert und via `WideString::fromUtf8()` importiert.

### D. `AssocArray` angepasst
* Setter für fundamentale Typen via Concepts ergänzt.
* `exportBinary` & `importBinary` auf das neue Format umgestellt (`exportVariantBinary`).
* **Traversierung:** `findInternal` delegiert bei `isVariantArray()` an `findInVariantArray` (z. B. für Pfade wie `key1/subkey/0/name`).
* **Baumerstellung:** `createTree` erkennt `"[]"` als Folgetoken und legt ein `VariantArray` an; Weiterleitung an `createVariantArrayTree`.
* Neuer Test `AutocreateVariantArray` läuft erfolgreich durch.

---

## 3. Aktueller Status & offene Punkte für morgen

* **Kompilierung:** Sauber, keine Compiler-Warnungen oder -Fehler.
* **Fehlgeschlagene Tests:**  
  Aktuell schlagen **6 ältere Tests** in `AssocArrayTest` fehl.
  * **Ursache:** Diese Tests basieren noch auf dem alten Verhalten:
    1. Erwartung, dass `a.set("[]", "wert")` oder `a.set("users/[]", ...)` numerische String-Keys (`"0"`, `"1"`, ...) in der `std::map` erzeugt.
    2. Sortier- und Zähl-Erwartungen mit `maxint` bzw. `ArrayKeyCompare` auf numerischen Schlüsseln.
    3. Altes Verhalten beim Anhängen eines klassischen `Array` (reines String-Array).
    4. Bisher waren Array an oberster Ebene erlaubt (`set("[]", ...)`). Das geht jetzt nicht mehr.
  Weitere 6 Tests im Json-Parser schlagen fehl, teilweise offensichtlich im Zusammenhang mit Listen

* Fehlschlagende Tests:
[  FAILED  ] JsonTest.ParseFromStringWithDictToAssocArray
[  FAILED  ] JsonTest.ParseFromStringWithArrayToAssocArray
[  FAILED  ] JsonTest.NegativTest_GarbageAfterEnd
[  FAILED  ] JsonTest.DumpsSimpleListAtFirstLevel
[  FAILED  ] JsonTest.DumpsNestedListAtFirstLevel
[  FAILED  ] JsonTest.DumpsNestedListAtSecondLevel
[  FAILED  ] AssocArrayTest.addAssocArrayWithLists
[  FAILED  ] AssocArrayTest.NumericKeyOverflow
[  FAILED  ] AssocArrayTest.AutomaticKeyOverflow
[  FAILED  ] AssocArrayTest.NumericKeyNormalization
[  FAILED  ] AssocArrayTest.MoveAssignmentResetsMaxint
[  FAILED  ] AssocArrayTest.list   ==> FIXED


---

## 4. Nächste Schritte

1. **Prüfung der 6 fehlschlagenden Tests in `assocarray.cpp`, sowie weiterer 6 Tests in `json.cpp`**
   * Handelt es sich um Tests von altem, jetzt obsoletem Verhalten (simulierte Arrays via `maxint`)?
   * Falls ja: Erwartungshaltung auf `VariantArray` modernisieren oder Alttests anpassen/entfernen.
2. **`createVariantArrayTree` & `createTree` vervollständigen:**
   * Klären, ob `VariantArray::add()` direkt eine Referenz `Variant&` zurückgibt (analog zu `emplace_back`).
   * Saubere Behandlung von `[]` auf Root-Ebene vs. Unterbäumen.
3. **JSON-Parser/Writer vorbereiten:**
   * Rückgabe von `Variant` für beliebige Dokument-Roots (`AssocArray` oder `VariantArray`).


# Zu treffende Entscheidungen
- Wie gehen wir mit dem bisherigen Typ `Array` um? Transofmieren wir in in ein VariantArray, oder ist Array jetzt ein eigener Typ, der aber nur Strings enthält und nicht weiter verschachtelt werden kann? Wir müssen ihn dann aber überall gesondert behandeln.

- Was passiert beim Merge zweier `VariantArray`-Instanzen? Sollen die Elemente einfach angehängt werden? Bisher war der Index ein Key im AssocArray. Ein neues Element mit einem vorhandenen Key hat das alte Element überschrieben. Wie soll dieses Verhalten in `VariantArray` umgesetzt werden?

