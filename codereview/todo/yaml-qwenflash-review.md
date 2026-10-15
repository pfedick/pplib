# Yaml (Qwen3.8Flash Next) Review – Befunde

Review vom 2026-10-15, Scope: `include/pplib/core/yaml.h` (261 Zeilen), `src/core/Yaml.cpp` (1996 Zeilen), `tests/src/core/yaml.cpp` (583 Zeilen), `tests/testdata/test1.yaml` (48 Zeilen).  
Öffentliche API: `pplib::Yaml` (`loads`, `load`, `dumps`, `dump` für `Variant`, `AssocArray`, `VariantArray`)  
Review done by: Code Reviewer (AI Senior C++ Software-Architekt)

---

## Zusammenfassung & Gesamtbewertung

Qwen3.8Flash Next hat in 50 Minuten eine beachtliche Codebasis aufgesetzt:
- Die Architektur mit zeilenbasiertem Streaming-Reader (`YamlLineReader`), 4-KB-Puffer, vollständiger Vermeidung globaler/statischer Zustände (`YamlCtx`) und `std::from_chars`/`to_chars` ist sauber und durchdacht strukturiert.
- `DESIGN.md` beweist ein klares Problemverständnis und definierte Grenzen.
- Die Basistests (35 Tests in `tests/src/core/yaml.cpp`) laufen durch.

**Aber:** Die Robustheit trügt. Hinter der 74% Line- / 62% Branch-Coverage verbergen sich fundamentale Lücken und gravierende Bugs:
1. **Tags auf quotierten Skalaren** (z. B. `!!binary "..."`, wie in `test1.yaml`) werden ersatzlos ignoriert; der Wert wird zum String degradiert.
2. **Kommentare nach Keys** (`key: # comment \n child: 1`) führen zum sofortigen Parsing-Abbruch mit Syntaxfehler für den Folgeblock.
3. **Flow-Mappings auf Root-Ebene** (`{a: 1}`) werfen Syntaxfehler, weil der Parser sie fälschlich für Block-Mappings hält.
4. **Quotierte Keys in Sequenz-Mappings** (`- "key": value`) werfen Syntaxfehler.
5. **Strings mit großen Zahlen** führen beim Serialisieren zu einem unkontrollierten `OverflowException`-Crash.
6. **Writer emittiert reservierte YAML-Präfixe (`*`, `&`, `!`, `[`, `{`, etc.) unquotiert**, wodurch Round-Trips zerstört werden.
7. **Leere Root-Container (`AssocArray`, `VariantArray`)** werden als leere Datei serialisiert und können nicht zurückgelesen werden.
8. **Die Testdatei `tests/testdata/test1.yaml` ist syntaktisch 100% korrekt**, wird aber im Testcode überhaupt nicht verwendet – und würde beim Laden prompt fehlschlagen (Base64-Strings statt ByteArrays).

---

## Bugs (kritisch)

- [ ] **Tags auf quotierten Skalaren werden verworfen (`src/core/Yaml.cpp:990` und `1148`)**
  - **Beschreibung:** Wenn nach einem Tag ein quotierter Skalar folgt (z. B. `!!binary "SGVsbG8="` oder `!!int "42"` oder `!!binary ""` für leere ByteArrays), ruft `parseValueAfterIndicator` direkt `value = parseQuoted(...)` auf, ohne das eingelesene `tag` zu übergeben oder `resolveScalar` aufzurufen. Das Gleiche geschieht in `parseFlowInner` (Zeile 1148).
  - **Auswirkung:** Quotierte Tags werden ignoriert. Binärdaten mit Quotes werden fälschlich als `pplib::String` zurückgegeben statt als `pplib::ByteArray`. Das in `DESIGN.md` dokumentierte leere `!!binary ""` erzeugt einen leeren String statt ein leeres `ByteArray`. Genau dieser Fall tritt in `tests/testdata/test1.yaml:43-44` auf!
  - **Reproduktion:**
    ```cpp
    pplib::Variant v = pplib::Yaml::loads("bin: !!binary \"SGVsbG8=\"\n");
    EXPECT_TRUE(v.toAssocArray().get("bin").isByteArray()); // Schlägt fehl! Ist String!
    ```
  - **Behebung:**
    ```cpp
    // In parseValueAfterIndicator (Zeile 989):
    } else if (c == '"' || c == '\'') {
        pplib::Variant qs = parseQuoted(line.text, pos, c, parentIndent, false);
        value = resolveScalar(std::string(qs.toString().c_str(), qs.toString().size()), tag, true);
    }
    // Analog in parseFlowInner (Zeile 1147):
    } else if (c == '"' || c == '\'') {
        pplib::Variant qs = parseQuoted(line.text, pos, c, -1, true);
        value = resolveScalar(std::string(qs.toString().c_str(), qs.toString().size()), tag, true);
    }
    ```

- [ ] **Kommentare nach Mapping-Keys zerstören nachfolgende Einrückungsblöcke (`src/core/Yaml.cpp:949`, `1004`, `1018`)**
  - **Beschreibung:** Steht hinter einem Mapping-Key ein Kommentar, z. B.:
    ```yaml
    mapping: # Kommentar
      child: 42
    ```
    zeigt `pos` auf `#`. Die Abfrage `if (!hasLine || pos >= line.text.size())` in Zeile 949 ist `false`. Der Parser springt in Zeile 974 in den `else`-Zweig, interpretiert den Kommentar als Plain-Scalar, bricht bei `#` ab und liefert `val = nullptr` für `mapping`. Die nachfolgende Zeile `  child: 42` wird dann in `parseMapping` (Zeile 461) als unerwartete Einrückung interpretiert und wirft `UnexpectedCharacterException("Unexpected indentation at line 2")`.
  - **Auswirkung:** Gängiger YAML-Code mit Schlüssel-Kommentaren bricht mit einer Fatal-Exception ab.
  - **Reproduktion:**
    ```cpp
    pplib::Yaml::loads("root: # comment\n  child: 1\n"); // Wirft UnexpectedCharacterException!
    ```
  - **Behebung:**
    Vor Zeile 949 prüfen, ob die verbleibende Zeile leer ist oder nur noch aus einem Kommentar besteht:
    ```cpp
    skipSpaces();
    if (hasLine && pos < line.text.size() && line.text[pos] == '#') {
        pos = line.text.size(); // Zeile für Wertsuche als beendet betrachten
    }
    if (!hasLine || pos >= line.text.size()) {
        // Lookahead auf nächste Zeile für Block-Strukturen...
    ```

- [ ] **Flow-Mappings auf Root-Ebene führen zu Syntaxfehlern (`src/core/Yaml.cpp:388-408`)**
  - **Beschreibung:** In `parseDocument()` sucht der Parser nach einem Doppelpunkt in der ersten Zeile. Bei `{a: 1}` findet er `:` hinter `a`, setzt `isMapLine = true` und ruft `parseMapping(0, 1)` auf. `parseMapping` parst die Zeile zeilenbasiert als Block-Mapping, liest `"{a"` als Schlüsselnamen und wirft an der schließenden Klammer `}` die Exception `UnexpectedCharacterException("Unexpected characters after value at line 1")`.
  - **Auswirkung:** Flow-Mappings wie `{a: 1, b: 2}` auf oberster Dokument-Ebene schlagen fehl.
  - **Reproduktion:**
    ```cpp
    pplib::Variant v = pplib::Yaml::loads("{a: 1}\n"); // Wirft UnexpectedCharacterException!
    ```
  - **Behebung:**
    In `parseDocument()` vor der `isMapLine`-Heuristik prüfen, ob die Zeile mit `{` oder `[` beginnt:
    ```cpp
    if (ind < (int)t.size() && (t[ind] == '{' || t[ind] == '[')) {
        pos = ind;
        hasLine = true;
        reader.readLine(line);
        result = parseValueAfterIndicator(ind, -1, 1);
    }
    ```

- [ ] **Quotierte Keys in Block-Sequenzen werden nicht als Mapping erkannt (`src/core/Yaml.cpp:231-233`, `559`)**
  - **Beschreibung:** `looksLikeMappingAt` schließt Anführungszeichen explizit aus:
    `if (c == '"' || c == '\'' ...) return false;`
    Wenn in einer Sequenz ein Mapping mit quotiertem Schlüssel beginnt, z. B.:
    ```yaml
    - "name": Alice
      "age": 30
    ```
    liefert `looksLikeMappingAt` fälschlich `false`. Zeile 566 behandelt die Zeile als Skalar `"- \"name\""`. Der Rest der Zeile `: Alice` führt in Zeile 1019 zu `UnexpectedCharacterException("Unexpected characters after value at line 1")`.
  - **Auswirkung:** Sequenzen von Objekten mit quotierten Keys können nicht geparst werden.
  - **Reproduktion:**
    ```cpp
    pplib::Yaml::loads("- \"id\": 1\n"); // Wirft UnexpectedCharacterException!
    ```
  - **Behebung:** In `looksLikeMappingAt` Anführungszeichen erlauben (`scanMapColon` überspringt Quotes ohnehin bereits korrekt):
    ```cpp
    static bool looksLikeMappingAt(const std::string& text, size_t p)
    {
        while (p < text.size() && (text[p] == ' ' || text[p] == '\t')) p++;
        if (p >= text.size()) return false;
        char c = text[p];
        if (c == '[' || c == '{' || c == '|' || c == '>' || c == '#' || c == '&')
            return false;
        return scanMapColon(text, p) != std::string::npos;
    }
    ```

- [ ] **Writer stürzt mit `OverflowException` ab bei Strings mit großen Ganzzahlen (`src/core/Yaml.cpp:1234`, `1515`)**
  - **Beschreibung:** Beim Serialisieren prüft der Writer mittels `isPlainSafeValue` -> `resolvesToString`, ob ein String unquotiert ausgegeben werden darf. `resolvesToString` ruft dafür `yamlParseInt(raw, iv)` auf. Wenn ein String eine Zahl mit mehr als 19 Ziffern enthält (z. B. eine IBAN, Kreditkartennummer oder UUID als Zahl: `"9999999999999999999999"`), wirft `yamlParseInt` in Zeile 1234 unkontrolliert `OverflowException`.
  - **Auswirkung:** Der Writer bricht beim Serialisieren gültiger Variant-Strings mit einer Exception ab, statt den String einfach in Anführungszeichen zu setzen.
  - **Reproduktion:**
    ```cpp
    pplib::AssocArray d;
    d.set("big", pplib::Variant(pplib::String("123456789012345678901234567890")));
    pplib::Yaml::dumps(d); // Wirft OverflowException!
    ```
  - **Behebung:** `yamlParseInt` darf im Fehlerfall keine Exception werfen, sondern muss `false` zurückgeben (oder einen Fehlercode):
    ```cpp
    if (res.ec != std::errc() || res.ptr != t.data() + t.size()) {
        return false;
    }
    ```

- [ ] **Writer emittiert reservierte YAML-Zeichen (`*`, `&`, `!`, `[`, `{`, `@`, etc.) unquotiert (`src/core/Yaml.cpp:1530-1553`)**
  - **Beschreibung:** `isPlainStructural` prüft Zeichen am String-Anfang mit:
    `if (n == 1 || p[1] == ' ' || p[1] == '\t') return false;`
    Wenn ein String mehr als ein Zeichen hat und `p[1]` kein Leerzeichen ist, gibt die Funktion `true` zurück! Dadurch werden Strings wie `"*ref"`, `"&anchor"`, `"!tag"`, `"[array]"`, `"{map}"`, `"@user"` unquotiert ausgegeben: `key: *ref`.
  - **Auswirkung:** Beim Zurücklesen wird `*ref` als Alias interpretiert (und wirft `ItemNotFoundException: Unknown YAML alias '*ref'`), `&anchor` als Anker und `[array]` als Array. Der Round-Trip ist vollständig defekt.
  - **Reproduktion:**
    ```cpp
    pplib::AssocArray d;
    d.set("v", pplib::Variant(pplib::String("*not_an_alias")));
    pplib::String y = pplib::Yaml::dumps(d);
    pplib::Yaml::loads(y); // Wirft ItemNotFoundException: Unknown YAML alias '*not_an_alias'!
    ```
  - **Behebung:**
    Strings, die mit `*`, `&`, `!`, `[`, `{`, `]`, `}`, `@`, `` ` ``, `,`, `%`, `?`, `:` beginnen, müssen IMMER quotiert werden, unabhängig vom Folgezeichen:
    ```cpp
    switch (p[0]) {
    case '*': case '&': case '!': case '[': case ']': case '{': case '}':
    case '@': case '`': case '%': case '?': case ':': case ',':
        return false;
    case '-': case '#': case '|': case '>': case '\'': case '"':
        if (n == 1 || p[1] == ' ' || p[1] == '\t') return false;
        break;
    ...
    ```

- [ ] **Leere Root-Container (`AssocArray`, `VariantArray`) werden unlesbar serialisiert (`src/core/Yaml.cpp:1936-1945`)**
  - **Beschreibung:** In `Yaml::dump` prüft der Code `if (data.toAssocArray().size() > 0)` bzw. `if (data.toVariantArray().size() > 0)`. Bei Größe 0 wird gar nichts geschrieben, sondern nur `file.fputc('\n')`.
  - **Auswirkung:** Ein leeres `AssocArray` oder `VariantArray` wird als leere Zeile (`"\n"`) ausgegeben. Beim anschließenden Einlesen wirft `Yaml::loads` sofort `UnexpectedEndOfDataException`.
  - **Reproduktion:**
    ```cpp
    pplib::AssocArray empty;
    pplib::String y = pplib::Yaml::dumps(empty);
    pplib::AssocArray back;
    pplib::Yaml::loads(back, y); // Wirft UnexpectedEndOfDataException!
    ```
  - **Behebung:**
    ```cpp
    if (data.isAssocArray()) {
        if (data.toAssocArray().size() > 0) writeDict(file, data.toAssocArray(), indent, 0, 0);
        else file.fputs("{}");
    } else if (data.isVariantArray()) {
        if (data.toVariantArray().size() > 0) writeArray(file, data.toVariantArray(), indent, 0, 0);
        else file.fputs("[]");
    }
    ```

- [ ] **`INT64_MIN` (-9223372036854775808) kann nicht geparst werden (`src/core/Yaml.cpp:1229-1237`)**
  - **Beschreibung:** In `yamlParseInt` werden die Ziffern ohne Vorzeichen in eine vorzeichenbehaftete Variable `int64_t v` eingelesen. `9223372036854775808` ist größer als `INT64_MAX` (9223372036854775807). `std::from_chars` schlägt mit `result_out_of_range` fehl und wirft `OverflowException`. Zudem ist `-v` für `INT64_MIN` in C++ Undefined Behavior (Signed Integer Overflow).
  - **Auswirkung:** Die kleinste gültige 64-Bit-Ganzzahl kann nicht geladen werden.
  - **Behebung:** Vorzeichen direkt an `std::from_chars` übergeben oder vorzeichenlos in `uint64_t` parsen und Wertebereich für negative Zahlen (`<= 9223372036854775808ULL`) prüfen.

---

## Bugs (mittel / Warnungen)

- [ ] **Keine Fehlererkennung bei Tabs in Einrückungen (`src/core/Yaml.cpp:193-199`)**
  - **Beschreibung:** In `DESIGN.md` steht: *"Tabs in der Einrückung => Fehler"*. `indentOf` zählt aber lediglich Leerzeichen (`' '`). Beginnt eine Zeile mit `\t`, liefert `indentOf` `0`. Dadurch wird kein Fehler geworfen, sondern der Tab wird entweder als Schlüsselname interpretiert (z. B. `"\tkey"`) oder beendet vorzeitig Blöcke.
  - **Empfehlung:** Vor der Einrückungsberechnung prüfen, ob führende Zeichen Tabs enthalten, und ggf. `UnexpectedCharacterException("Tabs are not allowed in YAML indentation at line %d", line.number)` werfen.

- [ ] **Escaped Newline (`\`) in doppelten Anführungszeichen fügt fälschlich Leerzeichen ein (`src/core/Yaml.cpp:643`, `740`)**
  - **Beschreibung:** Nach YAML-Spezifikation (Abschnitt 7.3.1) unterdrückt ein `\` am Zeilenende den Zeilenumbruch und führenden Whitespace der Folgezeile komplett. In `parseQuoted` wird jedoch über Zeile 740 ein Leerzeichen (`str.append(" ")`) eingefügt.
  - **Empfehlung:** Flag `escapedNewline` merken; wenn gesetzt, kein Leerzeichen anfügen.

- [ ] **Mehrzeilige Strings in Flow-Sammlungen werfen unerwartet Exception (`src/core/Yaml.cpp:733`, `743`)**
  - **Beschreibung:** In `parseQuoted` verhindert `!inFlow` in Zeile 733 das Fortsetzen eines quotierten Strings auf der nächsten Zeile innerhalb von `[...]` oder `{...}`. Stattdessen wird in Zeile 743 `UnexpectedEndOfDataException("Unterbrechener String vor Zeilenende")` geworfen.
  - **Empfehlung:** Zeilenfortsetzung auch für Flow-Sammlungen erlauben.

- [ ] **Fehlende Surrogate-Pair-Behandlung bei Unicode-Escapes (`src/core/Yaml.cpp:602`, `703`)**
  - **Beschreibung:** Anders als `Json.cpp` dekodiert `Yaml.cpp` keine UTF-16-Surrogate-Paare (`\uD83D\uDE00`). `appendUtf8` wirft bei jedem High-Surrogate (`0xD800-0xDBFF`) sofort `CharacterEncodingException("Lone surrogate")`.
  - **Empfehlung:** Die aus `Json.cpp` bekannte Surrogate-Logik übernehmen.

- [ ] **Ganzzahlige Fließkommazahlen verlieren beim Round-Trip ihren Datentyp (`src/core/Yaml.cpp:1645-1647`)**
  - **Beschreibung:** `std::to_chars` formatiert `double 1.0` als `"1"`. Beim Wiedereinlesen parst `yamlParseInt` den Wert als `int64_t` (`TYPE_INT64`).
  - **Empfehlung:** Wenn die `to_chars`-Ausgabe weder `.` noch `e`/`E` enthält, `".0"` anhängen.

- [ ] **Block-Scalar-Emitter prüft nicht auf Steuerzeichen (`src/core/Yaml.cpp:1577-1598`)**
  - **Beschreibung:** `useBlockScalar` prüft nicht auf Steuerzeichen (`\0`, `\b`, `\x1B`, `\r`). Strings mit solchen Zeichen werden als Block-Skalar `|-` mit rohen Bytes emittiert, was die YAML-Datei korrumpiert.
  - **Empfehlung:** Strings mit Zeichen `< 32` (außer `\n`, `\t`) immer mit `writeQuotedString` ausgeben.

- [ ] **Single-Quoted Key mit Escapes `'it''s': 1` wird auf Root-Ebene nicht als Mapping erkannt (`src/core/Yaml.cpp:384-387`)**
  - **Beschreibung:** In `parseDocument()` wird das Single-Quote-Escape `''` nicht übersprungen, wodurch die Suche nach dem Doppelpunkt fehlschlägt und die Zeile fälschlich als Skalar behandelt wird.
  - **Empfehlung:** Escaping in `parseDocument()` identisch zu `parseQuoted()` behandeln.

- [ ] **Untagged ISO-8601 Timestamps mit Leerzeichen (`YYYY-MM-DD HH:MM:SS`) werden nicht erkannt (`src/core/Yaml.cpp:1419`)**
  - **Beschreibung:** Laut `DESIGN.md` sollte `YYYY-MM-DD[T ]...` als `DateTime` aufgelöst werden. `Yaml.cpp` prüft jedoch nur `raw[10] == 'T' || raw[10] == 't'`.
  - **Empfehlung:** Auch `raw[10] == ' '` als Trenner für `DateTime` akzeptieren.

- [ ] **Komplexe Flow-Keys werfen `TypeConversionException` statt Syntaxfehler (`src/core/Yaml.cpp:1089`)**
  - **Beschreibung:** In `parseFlow` wird bei `c == '[' || c == '{'` `parseFlow(c, depth).toString()` aufgerufen. Da `Variant::toString()` bei Arrays und Dictionaries eine `TypeConversionException` wirft, bricht der Parser mit einem irreführenden Ausnahmetyp ab.
  - **Empfehlung:** Komplexe Keys explizit mit `UnexpectedCharacterException("Complex mapping keys are not supported at line %d", line.number)` abweisen.

- [ ] **Rechtschreibfehler und inkonsistente Sprache in Fehlermeldungen (`src/core/Yaml.cpp:730`, `743`, `1039`, `1059`, etc.)**
  - **Beschreibung:** Der Code verwendet `"Unterbrechener String vor Zeilenende"` und `"Unterbrechener Flow in Zeile %d"` (Tippfehler: Unterbrechener statt Unterbrochener). Zudem ist der Rest der Bibliothek und von `Yaml.cpp` konsistent auf Englisch verfasst (`"Unexpected character..."`, `"Expected ':' after key..."`).
  - **Empfehlung:** Vereinheitlichung auf Englisch: `"Unterminated string at line %d"`, `"Unterminated flow collection at line %d"`.

---

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Nutzung von `std::string_view` statt `const std::string&` und Substrings**
  - `Yaml.cpp` erzeugt an vielen Stellen unnötige Heap-Allokationen durch `std::string::substr` (z. B. in `looksLikeMappingAt`, `scanMapColon`, `yamlParseInt`, `yamlParseFloat`, `resolveScalar`). Unter C++20 sollte konsequent `std::string_view` eingesetzt werden, um Allokationen zu minimieren (besonders vorteilhaft für ressourcenbeschränkte Umgebungen wie den Raspberry Pico).

- [ ] **Temporäre Kopien bei `dumps` über `MemFile` (`src/core/Yaml.cpp:1951-1957`)**
  - `dumps(String&, const Variant&, int)` serialisiert über ein `pplib::MemFile`, liest dann die Größe aus und kopiert das Ergebnis in `pplib::String`. Da `pplib::String` direkt befüllt werden kann, könnte ein `StringWriter` unnötige Kopien vermeiden.

---

## Dokumentation & Doxygen

- [ ] **Falsche Exceptions in Doxygen-Kommentaren (`include/pplib/core/yaml.h`)**
  - In `yaml.h` ist für alle `loads`/`load`-Methoden `@throw pplib::IllegalArgumentException` dokumentiert. Diese Exception wird in `Yaml.cpp` jedoch an keiner Stelle geworfen (dort wird stattdessen `TypeConversionException` verwendet).
  - Tatsächlich geworfene Exceptions wie `pplib::ItemNotFoundException` (bei unbekannten Aliasen), `pplib::OverflowException` (bei Überschreitung von `MAX_YAML_DEPTH`) und `pplib::InvalidEscapeSequenceException` (bei ungültigen Escapes) fehlen in den Header-Doxygen-Kommentaren.

---

## Code Coverage & Test-Qualität

### Ist-Zustand
- **Line Coverage:** 74,4 % (648 Zeilen in `src/core/Yaml.cpp` ungetestet)
- **Branch Coverage:** 62,6 %
- **Test-Suite:** 35 Tests (33 `YamlTest`, 2 `YamlErrorTest`)

### Analyse der Testabdeckung & Lücken

1. **Testdatei `tests/testdata/test1.yaml` ist vollständig ungenutzt:**
   - Die Datei wurde vom Modell nicht in die Testsuite eingebunden.
   - **Bewertung von `test1.yaml`:** Die Datei ist syntaktisch **100% valides YAML**.
   - Würde man `pplib::Yaml::load(file)` auf `test1.yaml` anwenden, schlagen Zeilen 43–44 fehl: `binary: !!binary "SGVsbG8gd29ybGQ="` wird wegen Bug #1 als `String` statt als `ByteArray` geparst!

2. **Fehlende Unittests für Syntaxfehler (`YamlErrorTest`):**
   - Nur 2 Tests existieren in `YamlErrorTest` (`WrongRootType`, `NestingDepthLimit`).
   - Keine Tests für ungültige Escape-Sequenzen (`\q`, `\xG1`, Lone Surrogates).
   - Keine Tests für Tab-Einrückungen.
   - Keine Tests für unvollständige Flow-Klammern (`[1, 2`, `{a: 1`).
   - Keine Tests für unbekannte Aliase (`*missing` im Flow).
   - Keine Tests für Mappings mit Schlüsselkommentaren (`key: # comment`).

3. **Vollständig ungetestete Code-Bereiche:**
   - **Quotierte Keys in Mappings (`"key": value`):** Völlig ungetestet, weshalb der Parsing-Bug bei `- "key": value` unbemerkt blieb.
   - **Escape-Sequenzen:** `\0`, `\a`, `\b`, `\v`, `\f`, `\r`, `\e`, `\N`, `\_`, `\L`, `\P`, `\u`, `\U` sind in `src/core/Yaml.cpp` implementiert, aber im Unittest ungetestet.
   - **Flow-Sammlungen mit Ankern und Tags:** Flow-spezifische Anker/Aliase und Tags sind zu 0% getestet.
   - **Leere Root-Container:** Serialisierung und Deserialisierung von leeren Mappings/Sequenzen auf Dokumentebene fehlen.
   - **Öffentliche Komfort-Methoden:** Überladungen wie `load(AssocArray&, FileObject&)`, `load(VariantArray&, FileObject&)` sowie `dumps(const VariantArray&)` haben 0 Aufrufe in der Testsuite.

### Empfehlungen zur Test-Erweiterung
- Hinzufügen eines Integrationstests `TEST_F(YamlTest, LoadTest1YamlFile)`, der `tests/testdata/test1.yaml` lädt und alle Typen (insbesondere `ByteArray` und `DateTime`) explizit verifiziert.
- Systematischer Ausbau von `YamlErrorTest` mit gezielten Tests für jeden Exception-Typ.
- Parameterisierte Tests für alle YAML-Escapes.
- Round-Trip-Tests für leere Container (`{}` und `[]`).
- Test für Zahlen-Strings mit Überlänge zur Absicherung des Writers.

---

## Fazit & Einordnung der Modell-Leistung

Qwen3.8Flash Next hat in 50 Minuten einen beeindruckenden Sprung gegenüber dem früheren 27b-Modell gemacht:
- Die Architektur ist sauberer, der Code modularer, und es wurden fortgeschrittene YAML-Features (Block-Scalars mit Chomping, Flow-Collections, Type-Inference, PPLIB-Tags) implementiert.
- Allerdings zeigt sich das klassische Muster von LLM-Code: **Code, der durch Tests abgedeckt ist, funktioniert meist – aber ungetestete Pfade enthalten gravierende Design- und Logikfehler** (wie das Verwerfen von Tags bei Anführungszeichen oder der Crash beim Serialisieren langer Zahlen-Strings).
- Die Tests testen überwiegend den "Happy Path" mit einfachen, unquotierten Schlüsseln und Standard-Werten.
