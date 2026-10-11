# Yaml Review 2 – Befunde

Review vom 2026-10-11, Scope: `include/pplib/core/yaml.h` (196 Zeilen), `src/core/Yaml.cpp` (1460 Zeilen).  
Öffentliche API: `pplib::Yaml` (statische Methoden: `load`, `loads`, `dump`, `dumps`).  
Review done by: `Code Reviewer (KI-Software-Architekt)`

---

## Bugs (kritisch)

- [ ] **UTF-16 Surrogate Pair Escape-Sequenzen in `unescapeDoubleQuoted` springen über String-Ende / überspringen Folgezeichen** (`src/core/Yaml.cpp:399-401`)
  Beim Dekodieren von Unicode-Escape-Sequenzen (`\uXXXX`) prüft der Parser auf High-Surrogates (`0xD800`–`0xDBFF`) und liest das nachfolgende Low-Surrogate (`\uXXXX`) ein. Dabei wird `i` auf `save + 5` gesetzt (die letzte Hex-Ziffer des Low-Surrogates).
  Anschließend wird jedoch außerhalb der `if`-Bedingung bedingungslos `i += 4;` ausgeführt:
  ```cpp
  // src/core/Yaml.cpp:395-401
      cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
      i = save + 5;
  ...
  appendUtf8(str, cp);
  i += 4; // FEHLER: i wurde bereits auf save + 5 gesetzt!
  break;
  ```
  **Auswirkung:** Bei jedem Surrogate-Pair (z. B. Emojis wie `\uD83D\uDE00`) werden die nachfolgenden 4 Zeichen im Quell-String übersprungen oder `i` gerät über `s.size()` hinaus.
  **Behebung:** `i += 4;` nur im Non-Surrogate-Pfad ausführen:
  ```cpp
  if (isSurrogate) {
      i = save + 5;
  } else {
      i += 4;
  }
  ```

- [ ] **Inkonsistente Einrückung von Mappings in Sequenzen bei `indent != 2` erzeugt ungültiges YAML (`writeSequenceBlock`)** (`src/core/Yaml.cpp:1364-1367`)
  In `writeSequenceBlock` wird bei einem `AssocArray` ein kompakter Sequenz-Eintrag begonnen:
  ```cpp
  file.puts("-");
  ...
  } else if (v.isAssocArray()) {
      ...
      file.puts(" ");
      writeDictBlock(file, d, indent, col + indent);
  ```
  Der erste Schlüssel des Mappings steht hinter `- ` (also an Spalte `col + 2`). In `writeDictBlock` werden alle nachfolgenden Schlüssel jedoch mit Einrückung `col + indent` ausgegeben.
  Wenn `indent != 2` ist (z. B. Standardwert `indent = 4` oder Verschachtelung mit `col > 0`), beginnt der erste Schlüssel z. B. bei Spalte 2, aber der zweite Schlüssel bei Spalte 4:
  ```yaml
  - "name": "Alice"
      "age": 30
  ```
  **Auswirkung:** Dies ist syntaktisch ungültiges YAML. Beim Einlesen mit `loads` wirft der eigene Parser `UnexpectedCharacterException("Unexpected indentation in YAML mapping")`. Das Serialisieren und anschließende Einlesen von Listen mit Mappings schlägt bei `indent != 2` fehl.
  **Behebung:** Entweder für kompakte Mappings alle Schlüssel an Spalte `col + 2` ausrichten oder nach `-` einen Zeilenumbruch mit einheitlicher Einrückung `col + indent` einfügen.

- [ ] **Mehrzeilige Flow-Collections (`[...]`, `{...}`) scheitern an Zeilenumbrüchen in `skipFlowWs`** (`src/core/Yaml.cpp:522-536`)
  `skipFlowWs` soll Whitespace und Kommentare in Flow-Konstrukten überspringen, prüft dabei aber nur `isYamlSpace(c)`:
  ```cpp
  static bool isYamlSpace(int c) {
      return c == ' ' || c == '\t';
  }
  ```
  Zeilenumbrüche (`\n`, `\r`) werden nicht als Whitespace gewertet. Sobald ein `\n` auftritt, greift der `else`-Zweig:
  ```cpp
  } else {
      file.seek(-1, pplib::File::SEEKCUR);
      return;
  }
  ```
  **Auswirkung:** Sobald eine Flow-Sequenz oder ein Flow-Mapping über mehrere Zeilen formatiert ist (z. B. `[\n  1,\n  2\n]`), stoppt `skipFlowWs` am `\n`. Der nachfolgende `file.fgetc()` liest `\n` als Zeichen des nächsten Elements ein. Dies führt zu Syntaxfehlern oder fehlerhaften Daten.
  **Behebung:** `skipFlowWs` muss auch `\r` und `\n` überspringen:
  ```cpp
  if (isYamlSpace(c) || c == '\n' || c == '\r') continue;
  ```

- [ ] **Fehlende Escape-Behandlung bei zitierten Schlüsseln in Flow-Mappings (`parseFlow`)** (`src/core/Yaml.cpp:595-603`)
  Beim Einlesen von zitierten Schlüsseln in `parseFlow` prüft die Schleife lediglich:
  ```cpp
  if (c == '"' || c == '\'') {
      while (true) {
          ch = file.fgetc();
          if (ch == EOF) throw pplib::UnexpectedEndOfDataException();
          if (ch == c) break;
          key.append(static_cast<char>(ch));
      }
  ```
  **Auswirkung:** Ein doppelzitierter Schlüssel wie `{"key\"name": 1}` wird am `\"` vorzeitig beendet, da der Backslash ignoriert wird. Ebenso bricht ein einfach zitierter Schlüssel `'key''name'` beim ersten Hochkomma ab. Der Parser gerät aus dem Takt und wirft Exceptions.
  **Behebung:** Die gleiche Escape-Logik wie beim Einlesen von Flow-Skalaren (Zeilen 630–637) implementieren (Backslash bei `"` beachten, doppeltes Quote bei `'` beachten).

---

## Bugs (mittel / Warnungen)

- [ ] **Ungefangene `IllegalArgumentException` bei Datum/Zeit-Werten in `parseScalar`** (`src/core/Yaml.cpp:513-529`)
  `isIsoDate` und `isIsoTime` prüfen lediglich das Muster von Ziffern und Trennzeichen (`YYYY-MM-DD` bzw. `HH:MM:SS`), nicht jedoch die semantischen Wertebereiche (z. B. Monat 01–12, Tag 01–31, Stunde 00–23, Minute 00–59).
  Werte wie `2026-99-99` oder `12:75` (z. B. Versionsnummern, Kennungen oder Seitenverhältnisse) werden von `isIsoDate`/`isIsoTime` als `true` eingestuft.
  `Date::set` bzw. `Time::set` werfen daraufhin eine `IllegalArgumentException`.
  **Auswirkung:** Da `parseScalar` die Exception nicht abfängt, bricht das Parsen des gesamten YAML-Dokuments mit einem Fehler ab, anstatt den ungültigen Datums-/Zeitwert als Plain String zu behandeln.
  **Behebung:** `try { ... } catch (const IllegalArgumentException&) { ... }` um die Zuweisung setzen oder Wertebereiche vorab prüfen und bei Fehlschlag auf String zurückfallen.

- [ ] **Explizites positives Vorzeichen `+` wird bei Zahlen nicht abgeschnitten** (`src/core/Yaml.cpp:468-472`)
  In `parseScalar` steht:
  ```cpp
  String t = s;
  bool neg = false;
  size_t start = 0;
  if (!t.isEmpty() && (t[0] == '-' || t[0] == '+')) {
      neg = (t[0] == '-');
      start = 1;
  }
  if (neg) t = t.substr(1);
  ```
  Wenn `s` mit `+` beginnt (z. B. `+42`, `+3.14`, `+0x10`), bleibt das `+` in `t` enthalten.
  **Auswirkung:** `t.startsWith("0x")`, `validFloat` und `allDigits` schlagen fehl. Alle positiven Zahlen mit Vorzeichen `+` werden fälschlicherweise als String geparst.
  **Behebung:**
  ```cpp
  if (!t.isEmpty() && (t[0] == '-' || t[0] == '+')) {
      neg = (t[0] == '-');
      t = t.substr(1);
  }
  ```

- [ ] **`allDigits` liefert `true` für leere Strings – Skalare `-`, `+` oder `""` werden zu `0`** (`src/core/Yaml.cpp:196-203, 509-511`)
  In `allDigits(s, start, len)` ist die Schleife bei `len == 0` leer und die Funktion gibt `true` zurück.
  Wird ein unzitierter Skalar `-` oder `+` übergeben, wird das Vorzeichen entfernt und `t` wird leer (`len == 0`). `allDigits` meldet `true` und `strtoll` parst den String als `0`.
  Ebenso liefert `parseScalar("")` die Zahl `0`.
  **Behebung:** In `allDigits` sofort `false` zurückgeben, wenn `len == 0`:
  ```cpp
  if (len == 0 || start + len > s.size()) return false;
  ```

- [ ] **Fehlende Hex-/Oktal-Zeichenvalidierung – ungültige Literale werden zu `0` statt String** (`src/core/Yaml.cpp:474-482`)
  Bei `0x...` und `0o...` ruft `parseScalar` direkt `strtoll` auf, ohne zu prüfen, ob nach dem Präfix gültige Ziffern folgen und ob bis zum Stringende nur Ziffern stehen.
  Ein Literal wie `0xxyz` oder `0o99` wird von `strtoll` bis zum ersten ungültigen Zeichen geparst und ergibt `0`, anstatt als String `"0xxyz"` behandelt zu werden.
  **Behebung:** Vor der Umwandlung prüfen, ob alle Zeichen hinter dem Präfix gültige Hex- bzw. Oktalziffern sind (oder `endptr` von `strtoll` gegen das Stringende vergleichen).

- [ ] **Verlust von `NaN` und `Infinity` beim Export (`writeScalar`)** (`src/core/Yaml.cpp:1172-1174`)
  ```cpp
  if (v.isDouble()) {
      double d = v.toDouble();
      if (std::isnan(d) || std::isinf(d)) {
          file.puts("null");
  ```
  Im Gegensatz zu JSON definiert YAML 1.2 die Literale `.nan`, `.inf` und `-.inf` (welche `parseScalar` in Zeile 485–487 auch korrekt parst).
  **Auswirkung:** `Variant(INFINITY)` oder `Variant(NAN)` wird zu `"null"` serialisiert und beim Re-Import als `TYPE_NULL` eingelesen. Es entsteht Datenverlust beim Roundtrip.
  **Behebung:**
  ```cpp
  if (std::isnan(d)) {
      file.puts(".nan");
  } else if (std::isinf(d)) {
      file.puts(d < 0 ? "-.inf" : ".inf");
  }
  ```

- [ ] **Leere YAML-Dokumente oder reine Kommentar-Dateien werfen `UnexpectedEndOfDataException`** (`src/core/Yaml.cpp:1034-1043, 853-855`)
  In `Yaml::load` wird `if (file.eof())` geprüft. Bei einer geöffneten Datei (oder Stream) ist EOF vor dem ersten Leseversuch oft noch nicht gesetzt.
  `parseBlock` ruft `readLine` auf, welches Leerzeilen und Kommentare bis EOF überliest und `false` zurückgibt.
  Daraufhin wirft `parseBlock` in Zeile 855:
  ```cpp
  if (!readLine(state, file, line)) throw pplib::UnexpectedEndOfDataException();
  ```
  **Auswirkung:** Ein leeres YAML-Dokument oder eine Datei, die nur aus Kommentaren besteht, wirft eine Exception, anstatt wie in YAML üblich ein leeres `Variant(nullptr)` zurückzugeben.
  **Behebung:** In `Yaml::load` vorab prüfen:
  ```cpp
  Line firstLine;
  if (!readLine(state, file, firstLine)) {
      data = pplib::Variant(nullptr);
      return;
  }
  pushLine(state, firstLine);
  ```

- [ ] **Dokumentgrenzen `---` und `...` werden nicht unterstützt** (`src/core/Yaml.cpp:855-875`)
  Standard-YAML-Dateien beginnen häufig mit `---` und/oder enden mit `...`.
  - Beginnt die Datei mit `---`, wertet `parseBlock` die Zeile als Root-Skalar `"---"` aus und beendet das Parsen; der gesamte nachfolgende Inhalt wird ignoriert.
  - Steht am Ende `...`, schlägt `splitKeyValue` fehl und es wird `UnexpectedCharacterException` geworfen.
  **Behebung:** Eine einleitende Zeile `---` und eine abschließende Zeile `...` als Dokumentbegrenzer erkennen und überspringen.

- [ ] **Interne Einrückung in Block-Skalaren (`|` und `>`) geht vollständig verloren** (`src/core/Yaml.cpp:737-750`)
  In `parseBlockScalar` wird auf jeder Zeile die Einrückung bis zum ersten Nicht-Leerzeichen gezählt (`i`) und der Inhalt per `raw.substr(i)` extrahiert:
  ```cpp
  size_t i = 0;
  while (i < raw.size() && raw[i] == ' ') i++;
  ...
  String content = raw.substr(i);
  ```
  **Auswirkung:** Jede Zeile verliert ihre relative Einrückung. Wenn der Block-Skalar z. B. eingerückten Quellcode, Listen oder ASCII-Grafiken enthält, wird der gesamte Text linksbündig plattgedrückt.
  **Behebung:** Die Basis-Einrückung anhand der ersten nicht-leeren Zeile des Block-Skalars ermitteln und bei allen Folgezeilen nur genau diesen Offset an Leerzeichen abschneiden.

- [ ] **Apostroph/Anführungszeichen in Plain-Strings verhindern Kommentarerkennung (`#`)** (`src/core/Yaml.cpp:106-118, 143-155, 793-808`)
  Sowohl in `readLine` als auch in `splitKeyValue` wird bei Antreffen von `'` oder `"` die Variable `quote` gesetzt.
  Wenn in einem unzitierten Skalar ein Apostroph vorkommt (z. B. `title: Patrick's Library # comment`), wird `quote = '\''` gesetzt. Da kein zweites Hochkomma folgt, bleibt `quote` bis Zeilenende aktiv.
  **Auswirkung:** Das abschließende `# comment` wird nicht abgeschnitten, da es als "innerhalb von Quotes" angesehen wird. In `splitKeyValue` führt ein Apostroph im Schlüssel dazu, dass der nachfolgende Doppelpunkt `: ` ignoriert wird.
  **Behebung:** `quote` nur aktivieren, wenn das Anführungszeichen am Anfang des Werts/Schlüssels steht (echte Zitate).

- [ ] **Tag `!!binary` liefert bei leerem Inhalt `ByteArrayPtr` statt `ByteArray`** (`src/core/Yaml.cpp:692`)
  In `parseTaggedValue` steht:
  ```cpp
  if (r.isEmpty()) return pplib::Variant(pplib::ByteArrayPtr());
  ```
  Alle anderen Pfade (Zeilen 860, 915, 989) und der Header dokumentieren:
  `"...wieder in ByteArray konvertiert."`
  `ByteArrayPtr` und `ByteArray` sind in `Variant` unterschiedliche Typen (`TYPE_BYTEARRAYPTR` vs. `TYPE_BYTEARRAY`).
  **Behebung:** Einheitlich `pplib::Variant(pplib::ByteArray())` zurückgeben.

- [ ] **Locale-Abhängigkeit bei `s.toDouble()` in `parseScalar`** (`src/core/Yaml.cpp:509`)
  `s.toDouble()` ruft intern `atof` auf. Wenn die C-Locale auf ein Land mit Komma als Dezimaltrenner gesetzt ist (z. B. `de_DE`), parst `atof("3.14")` nur `3.0`.
  **Behebung:** Für das Parsen von Gleitkommazahlen eine locale-unabhängige Funktion wie `std::from_chars` (`<charconv>`) oder `pplib::toDouble` verwenden.

---

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **Unnötige Deep-Copies von `AssocArray` und `VariantArray` beim Serialisieren** (`src/core/Yaml.cpp:1418, 1442`)
  In den Überladungen von `Yaml::dump` für `AssocArray` und `VariantArray` wird das Argument per Kopie in ein `pplib::Variant` eingepackt:
  ```cpp
  void Yaml::dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent) {
      ...
      writeVariant(file, pplib::Variant(data), indent); // Kopiert den kompletten Baum!
  }
  ```
  **Empfehlung:** Direkt `writeDictBlock(file, data, indent, 0)` bzw. `writeSequenceBlock(file, data, indent, 0)` aufrufen, um unnötige Allokationen und Datenkopien zu vermeiden.

- [ ] **Hohe Heap-Allokationsrate durch wiederholte `String::substr`-Aufrufe** (`src/core/Yaml.cpp` div.)
  Beim Parsen von Zeilen, Datumsstrings, Skalaren und Base64-Blöcken wird exzessiv `String::substr` verwendet. Für jede Teilzeichenkette wird ein neues Heap-Objekt angelegt. Auf Mikrocontrollern (Raspberry Pi Pico) führt dies zu Fragmentierung und Speicherüberläufen.
  **Empfehlung:** Für temporäre Teilstrings C++20 `std::string_view` oder Indizes/Pointer-Bereiche nutzen.

- [ ] **Inneffizientes zeichenweises Lesen (`fgetc`) und String-Reallokation in `readLine`** (`src/core/Yaml.cpp:126-128`)
  `while ((c = file.fgetc()) != '\n' && c != EOF) raw.append(static_cast<char>(c));`
  Jedes Zeichen wird einzeln gelesen und ohne `reserve` an den String angehängt.
  **Empfehlung:** Zeilenpufferung nutzen (z. B. `file.gets` oder blockweises Lesen) und vorab Speicher reservieren.

- [ ] **`writeBase64Lines` allokiert für jede 76-Zeichen-Zeile einen neuen String** (`src/core/Yaml.cpp:1290`)
  ```cpp
  file.puts(b64.substr(off, len));
  ```
  Bei einem 1 MB großen Byte-Array entstehen hierbei über 17.000 temporäre Heap-Strings.
  **Empfehlung:** Direkt `file.write(b64.c_str() + off, len)` verwenden.

- [ ] **Keine Unterstützung für `Variant::TYPE_ARRAY` beim Serialisieren** (`src/core/Yaml.cpp:1223`)
  `pplib::Array` ist ein nativer PPLIB-Typ für String-Listen. `v.isArray()` wird in `writeVariantFlow` und `writeVariant` nicht behandelt; es wird `UnsupportedDataTypeException` geworfen.
  **Empfehlung:** `v.isArray()` analog zu `Json.cpp` als Sequenz von Strings serialisieren.

- [ ] **Modernisierung auf C++20 `<charconv>` und Structured Bindings** (`src/core/Yaml.cpp:1140, 1175, 1232, 1297`)
  - `snprintf` in `escapeString` und `writeScalar` kann durch `std::to_chars` ersetzt werden.
  - Iteration über `dict` kann mit C++17/20 Structured Bindings vereinfacht werden: `for (const auto& [key, val] : dict)`.

- [ ] **Code-Duplikation bei Einrückungszählung und Kommentarbereinigung** (`src/core/Yaml.cpp:92-120` vs. `135-163`)
  Die Bereinigung von Einrückung und Kommentaren für `pendingRaw` ist identisch zum Code in der Lese-Schleife von `readLine`.
  **Empfehlung:** In eine gemeinsame Hilfsfunktion auslagern.

- [ ] **Vorzeichen-Warnung bei `padUsec`** (`src/core/Yaml.cpp:420`)
  `size_t p = s.find(".");` weist den Rückgabewert `ssize_t` an `size_t` zu. Danach wird `p == String::npos` verglichen (`size_t` mit `ssize_t`).
  **Empfehlung:** `ssize_t p = s.find(".");` deklarieren.

---

## Dokumentation & Doxygen

- [ ] **Fehlende `@throw`-Dokumentation in Header `include/pplib/core/yaml.h`**
  Keine der Methoden im Header dokumentiert die möglichen Exceptions:
  - `pplib::UnexpectedCharacterException`
  - `pplib::UnexpectedEndOfDataException`
  - `pplib::TypeConversionException`
  - `pplib::InvalidEscapeSequenceException`
  - `pplib::UnsupportedDataTypeException`
  **Empfehlung:** `@throw`-Tags bei allen Methoden ergänzen.

- [ ] **Fehlende Dokumentation von Parser-Einschränkungen im Header**
  Im Klassenkommentar sollte präzisiert werden:
  - Dokumentbegrenzer `---` und `...` werden derzeit nicht toleriert/überlesen.
  - Mehrzeilige zitierte Strings in Block-Mappings werden nicht unterstützt.
  - Chomping-Indikatoren (`|-`, `|+`, `>-`, `>+`) bei Block-Skalaren werden ignoriert.

---

## Code Coverage (nur falls explizit vom User gewünscht)

*Auf Wunsch des Nutzers für dieses Review ausgelassen.*

---

## Befunde in Drittdateien

- [ ] **Locale-Abhängigkeit in `String::toDouble()`** (`src/types/String.cpp:1283`)
  `String::toDouble()` verwendet direkt `atof(ptr)`. Dies führt bei System-Locales mit Dezimalkomma (wie Deutsch) zu falschem Verhalten beim Parsen von Gleitkommazahlen.
