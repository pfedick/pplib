# Json Review – Befunde

Review vom 2026-10-11, Scope: `include/pplib/core/json.h` (213 Zeilen), `src/core/Json.cpp` (676 Zeilen), `tests/src/core/json.cpp` (781 Zeilen).
Öffentliche API: `class pplib::Json` (`loads`, `load`, `dumps`, `dump`, `pp`)
Review done by: Code Reviewer

---

## Bugs (kritisch)

- [ ] **Locale-Abhängigkeit beim Parsen von Gleitkommazahlen (`LC_NUMERIC`)** (`src/core/Json.cpp:237`)
  `getNumber()` delegiert das Parsen von Gleitkommazahlen an `str.toDouble()`. In `src/types/String.cpp:1287` ist `toDouble()` als `atof(ptr)` implementiert. Die Standard-C-Funktion `atof` ist jedoch abhängig von der aktuell gesetzten `LC_NUMERIC`-Locale. Wenn die Host-Anwendung oder das System eine Locale mit Komma als Dezimaltrenner verwendet (z. B. `de_DE.UTF-8`, `German` unter Windows), parst `atof("17.999")` nur `17.0`, da der Punkt nicht als Dezimaltrenner erkannt wird!
  Beim Serialisieren (`dumps`) wurde dieses Problem in `writeVariant` bereits erkannt und ein Komma explizit durch einen Punkt ersetzt; beim Deserialisieren (`loads`) führt eine deutsche Locale jedoch zu stiller Datenverfälschung.
  ```cpp
  // Json.cpp:237
  if (isFloat) {
      return pplib::Variant(str.toDouble()); // <- Ruft atof() auf!
  }
  ```
  *Behebung:*
  Locale-unabhängiges Parsen mittels C++17/C++20 `std::from_chars` aus `<charconv>` verwenden:
  ```cpp
  #include <charconv>
  ...
  double val = 0.0;
  auto [ptr, ec] = std::from_chars(str.c_str(), str.c_str() + str.size(), val);
  if (ec != std::errc()) {
      throw pplib::UnexpectedCharacterException("Invalid floating point number: >>%s<<", str.c_str());
  }
  return pplib::Variant(val);
  ```

- [ ] **Stack Overflow durch unbegrenzte Rekursionstiefe (CWE-674 / Denial of Service)** (`src/core/Json.cpp:247`, `274`, `302`)
  `parseValue()` ruft bei `{` `parseDict()` und bei `[` `parseArray()` auf, welche für jedes Element rekursiv wieder `parseValue()` aufrufen. Es gibt keinerlei Begrenzung der Rekursionstiefe.
  Eine Eingabe wie `[[[[...]]]]` mit hoher Schachtelungstiefe führt zum Aufbrauchen des Call-Stacks und damit zum sofortigen Prozessabsturz (Segmentation Fault / `0xc00000fd`).
  Besonders kritisch ist dies im Hinblick auf den geplanten Einsatz auf Mikrocontrollern (Raspberry Pi Pico), wo der Stack typischerweise nur wenige Kilobyte umfasst.
  *Behebung:*
  Einen Rekursionstiefenzähler (z. B. `max_depth = 128`) einführen und bei Überschreitung mit `pplib::LimitExceededException` oder `pplib::UnexpectedCharacterException` abbrechen:
  ```cpp
  static pplib::Variant parseValue(pplib::FileObject& file, int c, int depth = 0)
  {
      if (depth > 128) {
          throw pplib::UnexpectedCharacterException("JSON nesting depth exceeded maximum limit at position %lld", file.tell());
      }
      ...
  ```

- [ ] **Fehlende Syntaxvalidierung bei Zahlen: Ungültiges JSON wird stillschweigend akzeptiert oder abgeschnitten** (`src/core/Json.cpp:213-241`)
  In `getNumber()` werden in einer `while`-Schleife wahllos Zeichen aus `[0-9eE.+-]` gesammelt, solange sie auftreten. Es wird lediglich geprüft, ob `str == "-"` ist.
  Dadurch werden nach RFC 8259 unzulässige Zeichenketten klaglos akzeptiert:
  - Mehrfache Dezimalpunkte wie `"1.2.3.4"` werden zu `1.2` geparst.
  - Unvollständige Exponenten wie `"1e"`, `"1e+"` werden zu `1.0` geparst.
  - Dezimalpunkte ohne nachfolgende Ziffer wie `"1."` werden zu `1.0` geparst.
  - Plus-Zeichen mitten in der Zahl wie `"1+2"` werden zu `1` geparst.
  - Führende Nullen wie `"0123"` (nach RFC 8259 §6 explizit verboten) werden ignoriert und als `123` geparst.
  *Behebung:*
  Strikte Syntaxprüfung gemäß RFC 8259 §6 vor dem Konvertieren durchführen oder die Konvertierung via `std::from_chars` validieren (prüfen, dass `ptr == str.c_str() + str.size()` gilt).

- [ ] **Absturz / Fehler bei nicht-seekbaren Streams und Pipes** (`src/core/Json.cpp:92`, `228`)
  Sowohl `nextNonWhitespace()` (beim Erkennen von Kommentaren) als auch `getNumber()` (beim Lesen des ersten Nicht-Ziffer-Zeichens) rufen `file.seek(-1, pplib::File::SEEKCUR)` auf, um ein Zeichen zurückzuspulen.
  Handelt es sich bei `file` um einen Stream, eine Pipe oder eine unpufferbare IO-Klasse (`pplib::FileObject`), die kein Rückwärts-Seek unterstützt (z. B. `IllegalOperationOnPipeException`), scheitert das Einlesen mit einer Exception.
  Zudem wird fälschlicherweise `pplib::File::SEEKCUR` statt `pplib::FileObject::SEEKCUR` verwendet.
  *Behebung:*
  Einen 1-Zeichen-Lookahead-Mechanismus implementieren, anstatt die Datei zurückzuspulen, oder für Streams das Lesen über einen Puffer abbilden.

- [ ] **Strukturverzerrung, Datenverlust und Absturz bei Schlüsseln mit `/` oder `[]` in `AssocArray`** (`src/core/Json.cpp:322`)
  In `parseDict()` wird `dict.set(key, std::move(val))` aufgerufen. Da `pplib::AssocArray::set()` intern `createTree(key)` aufruft und den Schlüssel an jedem `/` trennt, haben Objektschlüssel mit Schrägstrichen fatale Nebeneffekte:
  - Ein Schlüssel wie `"content/type": "text/html"` wird in ein verschachteltes Array umgewandelt: `dict["content"]["type"] = "text/html"`. Beim Exportieren entsteht `{"content":{"type":"text/html"}}` statt des flachen Schlüssels.
  - Existiert im JSON sowohl ein Schlüssel `"data"` als Skalar als auch `"data/sub"`, wird der Wert von `"data"` unwiederbringlich gelöscht und durch ein Dict überschrieben.
  - Enthält ein Schlüssel die Zeichenkette `"[]"` (z. B. `"items[]"` aus Formular-JSON), wirft `AssocArray::createTree()` sofort eine `InvalidKeyException`, wodurch das Parsen abbricht.
  *Behebung:*
  `AssocArray` benötigt entweder eine Methode `setLiteral(const String& key, ...)` zur direkten Eintragung in die interne `std::map`, oder `Json` muss Schlüssel mit `/` entsprechend maskieren oder davor warnen.

---

## Bugs (mittel / Warnungen)

- [ ] **Stille Schlüsselumbenennung: Leere Schlüssel `""` werden zu `"_empty_"` gemappt** (`src/core/Json.cpp:315`)
  ```cpp
  pplib::String key = getString(file);
  if (key.isEmpty()) key = "_empty_";
  ```
  Enthält ein valides JSON-Dokument einen leeren Objektschlüssel `""` (`{"": "wert"}`), wird dieser klammheimlich in `"_empty_"` umbenannt. Wenn das Original-JSON bereits einen Schlüssel `"_empty_"` besitzt, überschreiben sich die Werte stillschweigend.
  Ursache ist, dass `AssocArray::createTree()` bei leeren Schlüsseln eine `InvalidKeyException` wirft.
  *Empfehlung:*
  Falls `AssocArray` leere Keys nicht unterstützen kann, sollte dies klar dokumentiert werden oder `AssocArray` sollte leere Strings als gültige Wurzel-Keys akzeptieren.

- [ ] **Unzulässige Steuerzeichen in Strings werden nicht abgewiesen** (`src/core/Json.cpp:208`)
  Nach RFC 8259 §7 müssen Zeichen im Bereich `0x00` bis `0x1F` (insbesondere Zeilenumbrüche `\n`, `\r` und Tabs `\t`) in String-Literalen zwingend als Escape-Sequenz (`\n` oder `\u000a`) vorliegen.
  In `getString()` fallen diese Zeichen in den `else`-Zweig und werden ungeprüft an den String angehängt. Unescapte Zeilenumbrüche in Strings verstoßen gegen den JSON-Standard.
  *Empfehlung:*
  Zeichen `< 0x20` in `getString()` abweisen und `UnexpectedCharacterException` werfen.

- [ ] **Präzisionsverlust bei `double`-Serialisierung** (`src/core/Json.cpp:557`)
  `writeVariant()` verwendet `snprintf(buf, sizeof(buf), "%.15g", d)`.
  Ein IEEE-754 64-Bit-Gleitkommawert (`double`) benötigt für einen exakten, verlustfreien Roundtrip bis zu 17 Dezimalstellen (`std::numeric_limits<double>::max_digits10`). Bei 15 Stellen gehen niederwertige Bits verloren.
  *Empfehlung:*
  `std::to_chars` aus `<charconv>` verwenden (garantiert kürzeste, exakte und locale-freie Darstellung) oder mindestens `"%.17g"` formatieren.

- [ ] **`Json::dump(FileObject&, ...)` führt unaufgefordert `rewind()` und `truncate(0)` durch** (`src/core/Json.cpp:595`)
  `dump()` setzt die übergebene Datei gnadenlos an den Anfang und schneidet sie auf Länge 0 ab.
  Wenn ein Aufrufer JSON an eine bestehende Datei anhängen oder an einer bestimmten Position schreiben möchte, wird die Datei zerstört. Zudem wirft `FileObject::truncate()` eine `UnimplementedVirtualFunctionException`, falls das konkrete `FileObject` kein Truncate implementiert (z. B. Sockets oder Streams).
  *Empfehlung:*
  Das Zurücksetzen und Kürzen sollte dem Aufrufer überlassen werden oder als optionaler Parameter steuerbar sein.

- [ ] **Undokumentierte `//`-Kommentare und fehlender Support für `/* ... */`** (`src/core/Json.cpp:82-96`)
  `nextNonWhitespace()` überliest einzeilige Kommentare (`//`). Dies ist eine praktische Erweiterung (JSON5/JSONC), jedoch nicht RFC 8259-konform und im Header `json.h` nirgends erwähnt.
  Mehrzeilige Blockkommentare (`/* ... */`) werden hingegen nicht unterstützt; ein `/*` führt zu einem Abbruch mit `UnexpectedCharacterException`.
  *Empfehlung:*
  Den Support für `//` im Header dokumentieren und entweder `/* ... */` ebenfalls unterstützen oder klarstellen, dass es sich um ein proprietäres Feature handelt.

---

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Massiver Performance- und Allokations-Overhead durch Byte-weises `fgetc()`** (`src/core/Json.cpp:73`, `129`, `217`)
  Jedes einzelne Zeichen wird über den virtuellen Methodenaufruf `file.fgetc()` gelesen, welcher wiederum die virtuelle Methode `fread(&c, 1, 1)` aufruft. Bei einem 1-MB-JSON-Dokument führt dies zu über 1.000.000 virtuellen Funktionsaufrufen und Einzel-Byte-Reads.
  *Empfehlung:*
  Gepuffertes Einlesen (z. B. 4-KB- bis 64-KB-Blöcke oder Speicher-Mapping via `file.map()`) beschleunigt das Parsen um Größenordnungen und schont Mikrocontroller-Ressourcen.

- [ ] **Unnötige doppelte Heap-Allokationen bei der String-Ausgabe** (`src/core/Json.cpp:498`, `519`, `540-575`)
  In `writeDict()` und `writeVariant()` wird `file.putsf("\"%s\"", (const char*)escapeString(...))` aufgerufen.
  Hierbei allokiert `escapeString()` einen dynamischen String auf dem Heap. Anschließend parst `FileObject::putsf()` das Format `\"%s\"` mit `vasprintf()` und allokiert dabei einen *zweiten* temporären String auf dem Heap!
  *Empfehlung:*
  Direktes Schreiben ohne Umweg über `putsf` und `vasprintf`:
  ```cpp
  file.fputc('"');
  file.fputs(escapeString(value.toString()));
  file.fputc('"');
  ```
  Noch effizienter: `escapeString(file, str)` direkt in das `FileObject` streamen lassen, ohne einen Zwischenstring zu erzeugen.

- [ ] **Hohe Code-Duplizierung in `dump` / `dumps`** (`src/core/Json.cpp:593-671`)
  Die Überladungen von `dump()` und `dumps()` für `Variant`, `AssocArray` und `VariantArray` wiederholen denselben MemFile-Wrapper-Code sechsmal identisch.
  Da `writeVariant()` intern bereits `AssocArray` und `VariantArray` abwickelt, können die Komfort-Überladungen direkt an die `Variant`-Implementierung delegieren.

- [ ] **C-Style Casts und veraltete Konstrukte** (`src/core/Json.cpp:342`, `363`, `609`, etc.)
  - `(void*)json.getPtr()`: Unsauberes Weg-Casten der Constness (C-Style `const_cast`). Besser: `MemFile file(ByteArrayPtr(json.getPtr(), json.size()))`.
  - `pplib::MemFile file((void*)NULL, 0, true)`: Veraltetes `NULL` und unötiger Konstruktoraufruf. Der Standardkonstruktor `pplib::MemFile file;` erzeugt bereits ein leeres, beschreibbares In-Memory-File.
  - `(char)h`: Besser `static_cast<char>(h)`.

---

## Dokumentation & Doxygen

- [ ] **Vollständiges Fehlen von `@throw`-Dokumentation im Header** (`include/pplib/core/json.h:58-208`)
  Im Gegensatz zu `include/pplib/core/yaml.h`, wo alle Exceptions sauber deklariert sind, enthält keine einzige Methode in `json.h` eine `@throw`- oder `@exception`-Dokumentation.
  Methoden wie `loads()` und `load()` werfen jedoch diverse Exceptions:
  - `pplib::UnexpectedEndOfDataException`
  - `pplib::UnexpectedCharacterException`
  - `pplib::InvalidEscapeSequenceException`
  - `pplib::CharacterEncodingException`
  - `pplib::TypeConversionException`
  - `pplib::UnsupportedDataTypeException`

- [ ] **Fehlende Dokumentation von Sonderverhalten** (`include/pplib/core/json.h:35-50`)
  - Es fehlt der Hinweis, dass leere Schlüssel `""` automatisch zu `"_empty_"` konvertiert werden.
  - Es fehlt der Hinweis, dass einzeilige Kommentare mit `//` zulässig sind.

---

## Code Coverage & Unit-Test Beurteilung

Die gemessene Testabdeckung von **98 % Line Coverage** und **96 % Branch Coverage** erzeugt eine trügerische Scheinsicherheit (*Coverage Fallacy*).
Coverage misst lediglich, ob die *geschriebenen* Zeilen und Verzweigungen durchlaufen wurden – sie kann prinzipbedingt nicht feststellen, welche Verzweigungen und Randbedingungen im Code **fehlen**.

### Warum die 98% / 96% täuschen:
1. **Fehlende Zweige werden von Coverage-Tools nicht erfasst:**
   Da in `getNumber()` keine Prüfungen auf mehrfache Dezimalpunkte (`1.2.3`), unvollständige Exponenten (`1e`) oder führende Nullen (`0123`) implementiert sind, existieren dafür im Code keine Branches. Die bestehenden Tests erreichen 100 % Branch-Coverage für `getNumber()`, obwohl die Funktion ungültiges JSON fehlerhaft verarbeitet.
2. **Locale-Blindheit der Test-Suite:**
   In `JsonTest::JsonTest()` wird lediglich `setlocale(LC_CTYPE, DEFAULT_LOCALE)` gesetzt. `LC_NUMERIC` verbleibt in der Standard-`"C"`-Locale. Ein Test mit deutscher Locale (`setlocale(LC_NUMERIC, "de_DE.UTF-8")`) fehlt komplett und würde sofort aufdecken, dass `loads()` keine Floats mehr parsen kann.

### Konkret fehlende Edge-Case-Tests:

- [ ] **Gleitkommazahlen unter nicht-englischer Locale**
  Testfall fehlt: Parsen von Floats (`17.999`) nach `setlocale(LC_NUMERIC, "de_DE.UTF-8")` oder `setlocale(LC_ALL, "German")`.
- [ ] **Ungültige Zahlenformate nach RFC 8259**
  Testfälle fehlen:
  - Mehrfache Dezimalpunkte: `loads("{\"a\": 1.2.3.4}")`
  - Abgeschnittene Exponenten: `loads("{\"a\": 1e}")`, `loads("{\"a\": 1e+}")`
  - Dezimalpunkt ohne Nachkommastellen: `loads("{\"a\": 1.}")`
  - Plus-Zeichen in Zahlen: `loads("{\"a\": 1+2}")`
  - Führende Nullen: `loads("{\"a\": 0123}")`
  - Zahlenbereichsüberschreitung: `loads("9999999999999999999999999999999999999999")`
- [ ] **Objektschlüssel-Grenzfälle und Kollisionen**
  Testfälle fehlen:
  - Kollision mit `_empty_`: `loads("{\"\": 1, \"_empty_\": 2}")`
  - Objektschlüssel mit Slash: `loads("{\"content/type\": \"text/html\"}")` (Prüfung auf Strukturverzerrung und Roundtrip)
  - Objektschlüssel mit `"[]"`: `loads("{\"items[]\": [1, 2]}")`
  - Mehrfache/duplizierte Schlüssel: `loads("{\"a\": 1, \"a\": 2}")`
- [ ] **Steuerzeichen und NUL-Bytes in Strings**
  Testfälle fehlen:
  - Unescapte Steuerzeichen: `loads("{\"text\": \"zeile1\nzeile2\"}")` (muss nach RFC 8259 fehlschlagen)
  - Escapter Null-Terminator: `loads("{\"text\": \"hello\\u0000world\"}")` (Roundtrip-Prüfung)
- [ ] **Rekursionstiefe und Stack-Exhaustion**
  Testfall fehlt: Tief verschachtelte Arrays/Objekte (z. B. 1.000 Ebenen), um Stack-Overflows abzufangen.
- [ ] **Verlustfreier IEEE-754 `double`-Roundtrip**
  Testfall fehlt: Zahlen, die 17 signifikante Stellen benötigen (z. B. `0.1 + 0.2` bzw. `0.30000000000000004` oder `1.2345678901234567`).
- [ ] **Nicht-seekbare Dateiobjekte**
  Testfall fehlt: Parsen aus einem `FileObject`, dessen `seek()` eine Exception wirft (z. B. Pipe/Stream-Mock).
- [ ] **Mehrzeilige Kommentare (`/* ... */`)**
  Testfall fehlt: Überprüfung, ob `/* ... */` korrekt abgewiesen oder unterstützt wird.

---

## Befunde in Drittdateien

- [ ] **`String::toDouble()` ist nicht thread-safe und locale-abhängig** (`src/types/String.cpp:1287`)
  `pplib::String::toDouble()` ruft ungeschützt `atof(ptr)` auf. Wenn in einer Multithread-Umgebung ein Thread `setlocale()` ändert, führt dies zu unvorhersehbarem Verhalten. Eine Umstellung auf `std::from_chars` in `String.cpp` würde alle Aufrufer in der Bibliothek vor Locale-Fehlern schützen.
- [ ] **`AssocArray::createTree()` verhindert flache Schlüssel mit `/` und Schlüssel `"[]"`** (`src/types/AssocArray.cpp:140-155`)
  Die feste Koppelung von `AssocArray::set()` an eine hierarchische Pfad-Syntax (`/`) führt in Serialisierern (wie JSON und YAML) zu Inkonsistenzen. `AssocArray` sollte eine Möglichkeit bieten, Schlüssel literal ohne Pfad-Parsing zu setzen (z. B. `setLiteral()`).
