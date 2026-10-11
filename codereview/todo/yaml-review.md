# Yaml Review – Befunde

Review vom 2026-10-11, Scope: `include/pplib/core/yaml.h` (96 Zeilen), `src/core/Yaml.cpp` (1233 Zeilen), `tests/src/core/yaml.cpp` (166 Zeilen), `tests/testdata/test1.yaml` (48 Zeilen).
Öffentliche API: Klasse `pplib::Yaml` (statische Methoden: `load`, `loads`, `dump`, `dumps`).
Review done by: Copilot (Code Reviewer)

---

## Bewertung von `tests/testdata/test1.yaml`
Die Datei `tests/testdata/test1.yaml` ist **syntaktisch vollständig korrektes YAML (1.1 / 1.2)**:
- Die gemischten Datentypen, Arrays, verschachtelten Objekte und ISO-8601-Zeitstempel sind valide.
- Die kompakte Mappings-in-Listen-Syntax (`sub_array_with_objects: - sub_key1: ...`) entspricht der YAML-Spezifikation.
- `!!binary` ist das standardisierte Tag für Base64-Daten.
- **Wichtiges Detail:** `multiline_string: |` steht am Ende der Datei. Dadurch wurde im Test nicht bemerkt, dass nachfolgende Schlüssel nach einem Block-Skalar vom Parser komplett verworfen werden (siehe kritischen Bug unten).

---

## Bugs (kritisch)

- [✅] **Globale statische Parser-Zustände `g_pendingLine` & `g_hasPendingLine` (Thread-Safety & Exception-Poisoning)** (`src/core/Yaml.cpp:61-72`)
  Der Parser verwendet für den Zeilen-Lookahead globale statische Variablen:
  ```cpp
  static Line g_pendingLine;
  static bool g_hasPendingLine = false;
  ```
  Auswirkungen:
  1. **Thread-Safety-Verletzung:** Werden `Yaml::load()` oder `Yaml::loads()` parallel aus mehreren Threads aufgerufen (z. B. Worker-Threads oder Server-Requests), überschreiben sich die Threads gegenseitig `g_pendingLine` und geraten in Race Conditions bzw. Datenkorruption.
  2. **Exception-Poisoning:** Bricht ein Parse-Vorgang wegen einer Exception (z. B. `UnexpectedCharacterException`, `FileNotFoundException`) ab, bleibt `g_hasPendingLine == true`. Der nächste Aufruf von `Yaml::load()` oder `Yaml::loads()` im selben Thread liest als allererste Zeile die alte Geisterzeile aus dem vorherigen Dokument ein.
  3. **Keine Reentranz:** Verschachteltes Parsen (z. B. YAML innerhalb von YAML) ist unmöglich.
  
  *Behebung:* Einen lokalen Parser-Zustand (`struct ParserState` oder `class YamlParser`) einführen, der pro `load()`-Aufruf instanziiert wird und `FileObject&`, `std::optional<Line> pendingLine` etc. lokal hält.

  ==> FIXED, lokales `struct ParserState` mit `pendingLine`/`hasPendingLine`/`pendingRaw`/`hasPendingRaw` eingeführt; alle Parse-Funktionen nehmen `ParserState&` als ersten Parameter. Pro `load()`-Aufruf wird ein frischer State erzeugt — Thread-sicher, reentrant, kein Exception-Poisoning.

- [✅] **`parseBlockScalar` verschluckt die Folgezeile bei Mappings/Sequenzen (Datenverlust)** (`src/core/Yaml.cpp:573-585`)
  Beim Parsen von Block-Skalaren (`|` oder `>`) liest `parseBlockScalar` fortlaufend Zeilen per `readRawLine(file, raw)`. Sobald eine Zeile mit Einrückung `<= parentIndent` erreicht wird, bricht die Schleife per `break` ab:
  ```cpp
  if (i < raw.size() && i <= static_cast<size_t>(parentIndent)) break;
  ```
  Die soeben gelesene Zeile `raw` (z. B. der nächste Schlüssel des Mappings oder das nächste Element der Liste) wird jedoch **weder zurückgestellt noch gepusht**. Sie wird nach dem `break` verworfen!
  *Reproduktion:*
  ```yaml
  a: |
    Zeile 1
    Zeile 2
  b: "wichtig"
  ```
  Hier wird Schlüssel `b` vollständig verschluckt; das resultierende `AssocArray` enthält nur `a`. Dass `test1.yaml` funktionierte, lag ausschließlich daran, dass `multiline_string` dort das letzte Element der Datei war und auf EOF stieß.
  
  *Behebung:* Die Zeile, die das Ende des Block-Skalars markiert, muss geparst und über den Lookahead-Puffer (`pushLine`) an den umgebenden Mapping-/Sequenz-Parser zurückgegeben werden.

  ==> FIXED, `parseBlockScalar` pushbackt die terminierende Zeile via `state.pendingRaw = std::move(raw); state.hasPendingRaw = true;`. `readRawLine` prüft zuerst `pendingRaw` und konsumiert sie. Alle 5 Aufrufstellen (parseBlock, parseMapping ×2, parseSequence ×2) übergeben den ParserState.

- [✅] **Wurzel-Skalare (Root-Level Scalars) werden nicht unterstützt und werfen Exception** (`src/core/Yaml.cpp:700-720, 725-735`)
  `parseBlock` prüft nur auf Sequenz (`-`), Flow (`{`, `[`) oder Tag (`!!`). Trifft nichts davon zu, ruft es bedingungslos `parseMapping` auf:
  ```cpp
  static pplib::Variant parseBlock(pplib::FileObject& file)
  {
      Line line;
      if (!readLine(file, line)) throw pplib::UnexpectedEndOfDataException();
      const String& t = line.text;
      if (t == "-" || t.startsWith("- ")) return parseSequence(file, line);
      if (t[0] == '{' || t[0] == '[') return parseFlowValue(file, t);
      if (t.startsWith("!!binary")) { ... }
      if (t.startsWith("!!")) { ... }
      return parseMapping(file, line);
  }
  ```
  Wird ein einfacher Skalar an der Wurzel übergeben (z. B. `Yaml::loads("12345")`, `Yaml::loads("\"hello\"")` oder `Yaml::loads("true")`), scheitert `splitKeyValue` in `parseMapping`, und es wird eine `UnexpectedCharacterException` ("Expected 'key: value'") geworfen. Laut YAML-Spezifikation (und analog zu `pplib::Json`) sind Skalare an der Wurzel vollkommen legal.
  
  *Behebung:* Prüfen, ob `splitKeyValue` fehlschlägt und keine Unterzeilen folgen; in diesem Fall als Skalar parsen.

  ==> FIXED, `parseBlock` fällt auf `parseScalar` zurück wenn `splitKeyValue` fehlschlägt und keine einge-rückten Folgezeilen folgen. Wurzel-Skalare wie `42`, `"text"`, `true` werden korrekt geparst.

---

## Bugs (mittel / Warnungen)

- [✅] **`parseScalar`: Fehlerhafte Float-Erkennung macht aus Versionsnummern/IP-Adressen ungültige Doubles** (`src/core/Yaml.cpp:322-337`)
  In `parseScalar` bricht die Schleife zur Float-Erkennung beim ersten Vorkommen von `.` oder `e`/`E` sofort ab:
  ```cpp
  bool isFloat = false;
  for (size_t i = 0; i < t.size(); ++i) {
      char c = t[i];
      if (c == '.' || c == 'e' || c == 'E') {
          isFloat = true;
          break;
      }
      if (!isdigit(static_cast<unsigned char>(c))) {
          isFloat = false;
          break;
      }
  }
  if (isFloat) {
      return pplib::Variant(s.toDouble());
  }
  ```
  Zeichenketten wie unzitierte Versionsnummern (`version: 1.2.3`), IP-Adressen (`ip: 192.168.1.1`) oder Tokens wie `3.foo` werden sofort als Float eingestuft und über `s.toDouble()` verstümmelt (z. B. wird `1.2.3` zu `double 1.2` statt `String "1.2.3"`).
  *Behebung:* Strenge Validierung, dass nach dem Punkt Ziffern folgen und genau ein Punkt vorhanden ist (bzw. Regex / State-Machine für gültige Float-Syntax).

  ==> FIXED, strikte State-Machine-Validierung in `parseScalar`: nach Punkt müssen Ziffern folgen, genau ein Punkt erlaubt, Exponent nur mit Ziffern dahinter. `"1.2.3"` wird korrekt als String geparst.

- [✅] **`parseScalar`: Vorzeichen bei hexadezimalen Zahlen geht verloren** (`src/core/Yaml.cpp:304-307`)
  Bei negativen Hex-Zahlen (z. B. `-0x10`) setzt der Parser `neg = true` und schneidet das Minus ab (`t = "0x10"`). Bei der Rückgabe wird `neg` jedoch vergessen:
  ```cpp
  if (t.size() > 2 && t.startsWith("0x")) {
      return pplib::Variant(static_cast<int64_t>(strtoll(t.c_str(), NULL, 16)));
  }
  ```
  `-0x10` liefert `+16` statt `-16`. (Bei Oktal `0o` 4 Zeilen tiefer wurde `neg` dagegen korrekt berücksichtigt).

  ==> FIXED, neg wird bei hexadezimaler Rückgabe korrekt angewendet: `neg ? -value : value`.

- [✅] **`readLine` / `splitKeyValue`: Escapte Anführungszeichen (`\"`) heben den Quote-Modus vorzeitig auf** (`src/core/Yaml.cpp:103-118, 634-648`)
  Sowohl in `readLine` als auch in `splitKeyValue` prüft der Scanner:
  ```cpp
  if (quote != 0) {
      if (ch == quote) quote = 0;
  } else if (ch == '"' || ch == '\'') {
      quote = ch;
  }
  ```
  Enthält ein String ein escaped Quote (z. B. `"Text mit \" Anführungszeichen # kein Kommentar"`), wird `quote` durch das `\"` vorzeitig auf `0` gesetzt. Ein nachfolgendes `#` wird fälschlicherweise als Kommentar interpretiert und schneidet den String ab. Ebenso wird `splitKeyValue` bei Keys mit `\"` und nachfolgendem Doppelpunkt fehlerhaft getrennt.

  ==> FIXED, `readLine` und `splitKeyValue` erkennen `\"` in doppelten Quotes und heben den Quote-Modus nicht vorzeitig auf. Escaptes Backslash wird beim Scannen korrekt übersprungen.

- [✅] **`unescapeDoubleQuoted` stürzt bei UTF-16 Surrogate-Pairs ab und ignoriert YAML-spezifische Escapes** (`src/core/Yaml.cpp:211-213, 244-282`)
  1. `appendUtf8` wirft bei Surrogate-Codepoints (`0xD800`–`0xDFFF`) sofort eine `CharacterEncodingException`. Werden Zeichen jenseits des BMP als Surrogate-Pair (`\uD83D\uDE00` für Emojis) angegeben, bricht das Parsen mit Exception ab.
  2. YAML 1.2 spezifiziert Escapes wie `\0`, `\a`, `\v`, `\e`, `\xXX` und `\UXXXXXXXX`. Diese führen alle zu `InvalidEscapeSequenceException`.

  ==> FIXED, `appendUtf8` behandelt Surrogate-Pairs (D800–DBFF + DC00–DFFF) korrekt und kombiniert sie zu einem Codepoint. YAML-Escapes `\0`, `\a`, `\v`, `\e`, `\xXX`, `\UXXXXXXXX` unterstützt.

- [✅] **`writeScalar` für `double` verliert Typ-Identität bei ganzzahligen Gleitkommazahlen** (`src/core/Yaml.cpp:947-952`)
  `snprintf(buf, sizeof(buf), "%.15g", d)` schreibt eine Gleitkommazahl wie `1.0` als `"1"`. Beim anschließenden Einlesen mit `loads` erkennt `parseScalar` `"1"` als Integer (`allDigits == true`). Dadurch wird aus `double(1.0)` beim Roundtrip ein `int64_t(1)`.
  *Behebung:* Falls `buf` weder `.` noch `e`/`E` enthält, `".0"` anhängen.

  ==> FIXED, `writeScalar` hängt `.0` an wenn kein `.` oder `e`/`E` im Ergebnis vorhanden ist (z.B. `1.0` → `"1.0"`). Roundtrip erhält den Double-Typ.

- [✅] **`parseFlow`: Trailing Commas in Flow-Mappings und Sequenzen werfen Exception** (`src/core/Yaml.cpp:425-430, 470-480`)
  In YAML (Flow-Style) ist ein abschließendes Komma wie `[1, 2, 3,]` oder `{a: 1, b: 2,}` erlaubt. Der Parser setzt nach `,` die Schleife fort und erwartet zwingend ein weiteres Element. Er stürzt bei `]` oder `}` mit `UnexpectedCharacterException` ab.

  ==> FIXED, `parseFlow` akzeptiert abschließendes Komma vor `]` oder `}`. `[1,2,3,]` und `{a:1,b:2,}` werden korrekt geparst.

- [✅] **`writeSequenceBlock` hardcodet Einrückung von 2 Leerzeichen** (`src/core/Yaml.cpp:1095, 1101, 1118, 1124`)
  In `writeSequenceBlock` wird für Folgeeinrückungen fest `col + 2` verwendet, anstatt den Konfigurationsparameter `indent` zu nutzen (`col + indent`).

  ==> FIXED, `writeSequenceBlock` verwendet den Konfigurationsparameter `indent` statt hartkodiertem `+2`.

---

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [✅] **Klasse `Yaml` als statische Utility-Klasse absichern** (`include/pplib/core/yaml.h:60`)
  Der Konstruktor sollte gelöscht werden (`Yaml() = delete;`), um eine fehlerhafte Instanziierung zu verhindern.
  ==> FIXED, Konstruktor gelöscht (`Yaml() = delete;`).
  
- [ ] **`[[nodiscard]]` Attribut für Rückgabewerte** (`include/pplib/core/yaml.h:65-92`)
  Alle Methoden, die `pplib::Variant` oder `pplib::String` zurückgeben (`loads`, `dumps`), sollten mit `[[nodiscard]]` markiert werden.
- [ ] **Ineffiziente Dreifach-Pufferkopie in `dumps()`** (`src/core/Yaml.cpp:1150-1163, 1175-1188, 1200-1213`)
  `Yaml::dumps` schreibt in ein temporäres `MemFile`, alloziert dann ein lokales `ByteArray`, liest via `fread` zurück und kopiert anschließend in `yaml.set(...)`. Da `MemFile` direkten Zugriff auf den internen Speicher bietet, kann das Ergebnis ohne Zwischenallokation direkt übergeben werden.
- [✅] **C-Style-Casts und `NULL` durch modernes C++ ersetzen** (`src/core/Yaml.cpp`)
  Verwendung von `NULL` (z. B. `(void*)NULL`, `strtoul(hex, NULL, 16)`) durch `nullptr` ersetzen. C-Style-Casts (`(char*)`, `(void*)`) durch `static_cast` / `reinterpret_cast` ersetzen.

  ==> FIXED, alle C-Style-Casts durch `static_cast`/`reinterpret_cast` ersetzt, `NULL` → `nullptr`, `snprintf` → `std::snprintf`.
- [ ] **Unnötige String-Kopien bei Datumsprüfungen vermeiden** (`src/core/Yaml.cpp:160-204`)
  Funktionen wie `isIsoDate`, `isIsoTime` und `isIsoDateTime` arbeiten mit `substr()`, was Allokationen erzeugt. `std::string_view` bzw. Index-basierte Vergleiche sind effizienter.

---

## Dokumentation & Doxygen

- [✅] **Vollständige Doxygen-Methodenkommentare in `yaml.h` ergänzen** (`include/pplib/core/yaml.h:64-92`)
  Zwar existiert eine ausführliche Doxygen-Klassenbeschreibung, aber die einzelnen Methoden (`load`, `loads`, `dump`, `dumps`) besitzen keinerlei Doxygen-Kommentare (`@param`, `@return`, `@throws`, `@note`). Für IntelliSense sollten alle Überladungen dokumentiert sein.

  ==> FIXED, Doku ergänzt, insbesondere den Parameter "indent" bei den dump-Funktionen genau spezifiziert.
---

## Code Coverage & Unit Tests

- [✅] **Lücken in der Testabdeckung schließen (aktuell 65.9% Lines, 54.7% Branches)** (`tests/src/core/yaml.cpp`)
  Folgende Pfade sind derzeit völlig ungetestet:
  1. **Block-Skalar Folded-Style (`>`):** Bisher wird nur `|` getestet.
  2. **Block-Skalar mit nachfolgenden Keys:** Nach Behebung des Datenverlust-Bugs testen, dass nachfolgende Mappings/Listen korrekt gelesen werden.
  3. **Wurzel-Skalare und Wurzel-Sequenzen:** `- item1\n- item2` sowie skalare Root-Werte (`42`, `"text"`).
  4. **Zahlenformate:** Hex (`0x1F`, `-0x10`), Oktal (`0o77`, `-0o77`), Fließkommazahlen mit Exponent (`1e-3`), `.inf`, `-.inf`, `.nan`.
  5. **String-Escapes & Single-Quotes:** Alle Escape-Sequenzen in doppelten Quotes (`\n`, `\t`, `\"`, `\uXXXX`) und einfache Quotes mit `''`.
  6. **Flow-Style:** Mehrzeilige Flow-Mappings/Sequenzen, verschachtelte Flows, Flow mit Trailing-Commas.
  7. **Kommentare:** Inline-Kommentare an verschiedenen Stellen, Leerzeilen zwischen Einträgen.
  8. **FileObject-APIs:** Direkte Tests von `Yaml::load(..., FileObject&)` und `Yaml::dump(FileObject&, ...)`.
  9. **Fehlerbehandlung:** Tests für ungültige Einrückungen, Tabs, unvollständige Strings/Flows, ungültige Datumsformate.

  ==> PARTIALLY FIXED, 63 Tests (vorher 8) decken Block-Skalare, Root-Skalare, Zahlenformate, Escapes, Flow-Style, Fehlerbehandlung ab. Offen: FileObject-API-Direkttests (Punkt 8).

---

## Befunde in Drittdateien

- [ ] **`pico-pplib/src/core/stringfunctions.cpp:59`:** Die fehlerhafte Version von `encodeblock` und `ToBase64` ist dort noch vorhanden.
- [ ] **`src/core/Json.cpp:603`:** In `Json::dumps` existiert das gleiche ineffiziente Dreifach-Pufferkopie-Muster wie in `Yaml::dumps`.
