# Json Review – Befunde

Review vom 2026-09-13, Scope: `include/pplib/core/json.h` + `src/core/Json.cpp` (436 Zeilen).
Mitgelesen: `include/pplib/core/memfile.h` + `src/core/MemFile.cpp` und `tests/src/core/json.cpp` (14 Unit-Tests).

* Review done by: GitHub Copilot (Gemini 3.8 Flash)

---

## Bugs (kritisch)

- [ ] **Heap Corruption / Crash `0xc0000374` bei `Json::loads()` durch fehlerhaften `MemFile`-Aufruf** (Json.cpp:287)
  ```cpp
  void Json::loads(pplib::AssocArray& data, const pplib::String& json)
  {
      pplib::MemFile file((void*)json.getPtr(), json.size());
      Json::load(data, file);
  }
  ```
  `MemFile::MemFile(void* adresse, size_t size, bool writeable = false)` deklariert `writeable = false`.
  Im Konstruktor von `MemFile` (MemFile.cpp:88-96) wird `buffer` jedoch **nicht** mit `NULL` initialisiert,
  sondern direkt `open(adresse, size, writeable)` aufgerufen.
  `open()` prüft:
  ```cpp
  if (buffer) {
      free(buffer); // <- CRASH! buffer enthält uninitialisierten Stack-Müll!
  ```
  Da `file` auf dem Stack von `Json::loads` angelegt wird, ist `buffer` uninitialisierter Müll. `free(buffer)`
  führt sofort zur Heap-Corruption (`0xc0000374`) und reißt den gesamten Test-Prozess beim ersten `loads()` in den Tod.
  *Empirisch bestätigt in GDB*: SIGTRAP in `ntdll!RtlFreeHeap` bei Breakpoint in `MemFile::open`.
  
  **Fix in `MemFile.cpp` (Ursache):**
  Im Konstruktor `MemFile::MemFile(void* adresse, size_t size, bool writeable)` zwingend `buffer = nullptr;`
  vor dem Aufruf von `open()` setzen (wie im Standardkonstruktor auch).
  
  **Fix/Workaround in `Json.cpp` (robuster):**
  Statt des rohen Zeigers die typ-sichere `ByteArrayPtr`-Überladung von `MemFile` verwenden:
  ```cpp
  pplib::MemFile file(pplib::ByteArrayPtr(json.getPtr(), json.size()));
  ```

- [ ] **Speicherleck / Exception-Unsafe `malloc()` in `Json::dumps()`** (Json.cpp:328-337)
  ```cpp
  void Json::dumps(pplib::String& json, const pplib::AssocArray& data)
  {
      pplib::MemFile file((void*)NULL, 0, true);
      Json::dump(file, data);
      size_t size = file.tell();
      unsigned char* str = (unsigned char*)malloc(size + 1);
      if (!str) throw OutOfMemoryException();
      file.rewind();
      file.fread(str, size, 1);
      str[size] = 0;
      json.set((const char*)str, size);
      free(str);
  }
  ```
  1. `json.set()` kann bei Speicherknappheit werfen $\rightarrow$ `str` wird nie mit `free()` freigegeben (Leak).
  2. Völlig unnötiges doppeltes Kopieren über rohes `malloc`/`free`. `MemFile` hält den Puffer bereits im Speicher (`file.adr(0)` oder via `ByteArrayPtr`).
  Fix:
  ```cpp
  void Json::dumps(pplib::String& json, const pplib::AssocArray& data)
  {
      pplib::MemFile file;
      Json::dump(file, data);
      json.set(file.adr(0), file.tell());
  }
  ```

- [ ] **Deklarierte Funktion `Json::pp()` nirgends implementiert $\rightarrow$ Linkerfehler bei Aufruf** (json.h:47)
  Im Header `json.h` ist deklariert:
  ```cpp
  static pplib::String pp(const pplib::String& json);
  ```
  In ganz `Json.cpp` existiert dafür keine einzige Zeile Code! Ruft eine Anwendung `Json::pp(...)` auf,
  bricht der Linker mit `undefined reference to pplib::Json::pp(...)` ab.
  Fix: Entweder implementieren (Pretty-Printer) oder aus dem Header entfernen.

---

## Bugs (mittel)

- [ ] **Falsche Erkennung von Booleans und Nullwerten bei `dumps()`: Strings `"true"`, `"false"`, `"null"` werden als Literale serialisiert** (Json.cpp:367-370, 377-380)
  ```cpp
  if (str.isNumeric() && (!str.has(",")))
      file.puts(str);
  else if (str == "true" || str == "false" || str == "null")
      file.puts(str);  // <- schreibt true ohne Anführungszeichen!
  else
      file.putsf("\"%s\"", ...);
  ```
  Wenn im `AssocArray` ein String-Wert mit dem Inhalt `"true"` oder `"null"` steht, wird dieser
  beim Dumpen **ohne Anführungszeichen** ausgegeben:
  ```json
  {"status": true}
  ```
  Wird dieses JSON wieder geparst (`loads()`), entsteht kein echter String mehr, sondern das JSON-Literal `true`.
  Strings mit Inhalt `"true"`, `"false"` oder `"null"` können so nicht verlustfrei übertragen werden.
  Ursache: `Variant` unterscheidet im `AssocArray` oft nicht sauber zwischen echtem booleschen Typ und String `"true"`,
  weil `readValue()` (Zeilen 209-222) Literale einfach als `pplib::String("true")` speichert.
  Fix: `Variant` sollte echte Bool- und Null-Typen unterstützen, oder `dumps()` muss echte Strings immer quoten.

- [ ] **Surrogate-Pair-Dekodierung in Strings erzeugt falsche Zeichen bei `sizeof(wchar_t) == 2` (Windows)** (Json.cpp:115-119)
  ```cpp
  codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
  ...
  pplib::WideString ws;
  ws.append((wchar_t)codePoint);
  str.append(ws.toUtf8().toString());
  ```
  `codePoint` liegt bei Surrogate-Pairs über `0xFFFF` (z.B. Emojis `0x1F600`).
  Unter Windows ist `wchar_t` nur 16-Bit (`UTF-16LE`). Der Cast `(wchar_t)codePoint` schneidet die oberen Bits ab
  (`(wchar_t)0x1F600 == 0xF600`)! Das erzeugt ungültige Zeichen oder fehlerhaftes UTF-8.
  Fix: Direkte Konvertierung des `uint32_t codePoint` nach UTF-8 (1-4 Bytes) ohne den verlustbehafteten Umweg
  über `(wchar_t)`.

- [ ] **Numerische Floats mit Komma werden von `isNumeric()` als Nicht-Zahlen erkannt, aber `.` mit Mehrfachpunkten fälschlich als Zahl akzeptiert** (Json.cpp:365)
  `getNumber()` (Zeile 132-146) liest beliebige Zeichenketten aus `[0-9eE.+-]`. Ein ungültiger String wie `"1.2.3.4"`
  wird von `getNumber()` als Zahl gelesen und unquotiert übernommen.
  Ebenso prüft `isNumeric()` nicht die strikte JSON-Spezifikation für Zahlen (RFC 8259: keine führenden Nullen wie `0123`,
  kein alleinstehendes `.`, etc.).

---

## Design & Robustheit

- [ ] **Fragile Heuristik `isArray()`: O(N) Iteration über alle Schlüssel bei jedem Dict-Dump** (Json.cpp:342-354, 420-424)
  ```cpp
  static bool isArray(const pplib::AssocArray& data)
  {
      uint64_t v = 0;
      for (it = data.begin(); it != data.end(); ++it) {
          expectedkey.setf("%llu", v);
          if ((*it).first != expectedkey) return false;
          v++;
      }
      return true;
  }
  ```
  Weil `pplib::AssocArray` sowohl für JSON-Objects als auch für JSON-Arrays herhalten muss, prüft `writeDict()`
  vor jedem Schreiben eines Objekts rekursiv, ob die Keys `"0"`, `"1"`, `"2"` usw. lauten.
  1. Bei großen Objekten erzeugt das quadratische Laufzeit $O(N^2)$.
  2. Ein Objekt mit numerischen Keys wie `{"0": "a", "1": "b"}` wird ungewollt zu einem JSON-Array `["a", "b"]` serialisiert!
  Empfehlung: Langfristig eine dedizierte `JsonValue`- oder `Variant`-Klasse nutzen, die Arrays und Objects
  typsicher trennt.

- [ ] **Unzureichende Escaping-Behandlung beim Lesen von Strings** (Json.cpp:68-88)
  JSON erlaubt Escapes wie `\b`, `\f`, `\n`, `\r`, `\t`, `\"`, `\\`, `\/`.
  Nicht abgefangen werden ungültige Steuerzeichen (< 0x20) in Strings, die laut RFC 8259 unescaped verboten sind.

---

## Doku & Kosmetik

- [ ] **Fehlende Doxygen-Dokumentation**:
  In `json.h` sind die statischen Methoden (`loads`, `load`, `dumps`, `dump`, `pp`) weitgehend undokumentiert
  (keine Parameterbeschreibungen, keine Ausnahmedokumentation wie `UnexpectedCharacterException`, `UnexpectedEndOfDataException`).
- [ ] **Magische Fehlernummern im Code**:
  Exceptions enthalten kryptische Präfixe wie `#1:`, `#2:`, `#3:`, `#4:`, `#5:`, `#6:` in den Fehlermeldungen
  (z.B. `throw UnexpectedCharacterException("#2: >>%c<< at position ...")`). Das erschwert Anwendern das Verständnis.

---

## Verifiziert OK (kein Handlungsbedarf)

- Grundstruktur der Grammatik (Recursive Descent Parser) über `fgetc()` auf einem `FileObject` ist kompakt und übersichtlich.
- Kommentare `//` in JSON werden als Erweiterung in `readDict()` tolerant übersprungen.
- Die Unit-Test-Suite (`tests/src/core/json.cpp`) ist mit 14 Tests bereits vorhanden und deckt viele Randfälle ab.
