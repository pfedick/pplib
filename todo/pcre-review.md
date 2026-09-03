# Pcre Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/core/regex.h` + `src/core/Pcre.cpp` (478 Zeilen).
Die Klasse heißt intern `RegEx` (mit innerer Klasse `RegEx::Pattern`), ist ein reiner PCRE2-Wrapper (kein PCRE1 mehr,
`pcre_compile`/`pcre_free` existieren in diesem Code nicht). Aufrufstellen geprüft in `src/core/Dir.cpp`,
`src/core/Time.cpp`, `src/core/PerlHelper.cpp`, `src/core/PythonHelper.cpp`, `src/audio/ID3Tag.cpp` sowie
`tests/src/core/regex.cpp`. Exception-Namen gegen `include/pplib/exceptions.h` verifiziert.
Mehrere der unten genannten Bugs wurden mit einem Standalone-Repro gegen echtes `libpcre2-8`/g++ 17 empirisch bestätigt
(siehe Fußnoten in den jeweiligen Einträgen).

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`replace()`: Zero-Length-Match führt zu Endlosschleife + unbegrenztem Speicherwachstum** (`Pcre.cpp:357-380` und `403-426`)
  Die Replace-Schleife rückt den Suchoffset nur über `offset = ovector[1]` vor. Bei einem Pattern, das an einer Stelle
  eine leere Zeichenkette matchen kann (jedes `*`, `?`, `{0,n}`, optionale Gruppe `(...)?` – alltägliche Konstrukte),
  liefert `pcre2_match` `ovector[0] == ovector[1] == offset`. Der Offset ändert sich nicht, `while (offset <= subj_len)`
  bleibt wahr, und `result += replacement` wird bei jeder Iteration erneut angehängt → Endlosschleife, `result` wächst
  unbegrenzt (Hang/OOM-Crash), Prozess muss extern getötet werden.
  ```cpp
  PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(md);
  result += subject.mid(offset, ovector[0] - offset);
  result += replacement;
  offset = ovector[1];   // <- bei Zero-Length-Match unverändert
  count++;
  if (max > 0 && count >= max) break;
  ```
  Empirisch bestätigt mit Pattern `"a*"` gegen `pcre2_match` direkt (identische Aufrufsequenz wie im Code): der Offset
  bleibt bei jeder Iteration `0`, das Match ist `[0,0]`. `RegEx::replace("a*", "test", "X")` hängt sich in der Praxis auf.
  Betrifft narrow- und wide-Variante gleichermaßen.
  Fix: nach einem Zero-Length-Match den Offset manuell um mindestens ein (UTF-8-Zeichen-)Codepoint erhöhen, das
  übersprungene Zeichen unverändert in `result` übernehmen:
  ```cpp
  if (ovector[1] == ovector[0] && ovector[0] == offset) {
      if (offset >= subj_len) break;
      // ein UTF-8-Zeichen unverändert kopieren und Offset entsprechend weiterschieben
      size_t skip = utf8_charlen(subj_ptr + offset); // bzw. 1 Wide-Unit bei WideString
      result += subject.mid(offset, skip);
      offset += skip;
      continue;
  }
  ```

- [ ] **„Move-Konstruktor“ von `Pattern` nullt die Quelle nicht → Double-Free** (`regex.h:67`, `Pcre.cpp:104-109`)
  ```cpp
  // regex.h
  Pattern(const Pattern&& other);
  ...
  // Pcre.cpp
  RegEx::Pattern::Pattern(const Pattern&& other)
  {
      p = other.p;      // <- Quelle bleibt unverändert, weil `other` const ist!
      bits = other.bits;
  }
  ```
  Der Parameter ist `const Pattern&&` statt `Pattern&&`. Das ist laut Standard trotzdem ein Move-Konstruktor (jede
  rvalue-Referenz auf T zählt), aber weil `other` `const` ist, kann `other.p` nicht auf `nullptr` gesetzt werden.
  Beide `Pattern`-Objekte zeigen danach auf denselben `pcre2_code*`; sobald beide Destruktoren laufen, wird derselbe
  Handle zweimal freigegeben → Double-Free/Heap-Korruption.
  Empirisch reproduziert (Nachbau exakt dieser drei Methoden `struct Pattern`): `Pattern b(std::move(a));` gefolgt vom
  Scope-Ende beider Objekte erzeugt zuverlässig `free(): double free detected in tcache 2` und Absturz.
  Aktuell nutzt kein Call-Site im Repo explizit `std::move(pattern)` – der einzige Rückgabepfad in `compile()` ist ein
  NRVO-Kandidat (einzelne benannte lokale Variable, ein `return`) und wird von gängigen Compilern wegoptimiert. Das
  ist aber reine Optimierung, keine Garantie (NRVO ist – im Gegensatz zur Pflicht-Elision bei `T x = f();` seit C++17 –
  nicht vorgeschrieben) und bricht z.B. sofort bei `-fno-elide-constructors` oder sobald jemand künftig
  `Pattern p2(std::move(p1))` schreibt, was ein ganz normaler, zu erwartender Gebrauch eines Move-Konstruktors wäre.
  Fix: Parameter zu `Pattern&& other` ändern und `other.p = nullptr; other.bits = 0;` nach der Übernahme setzen.

- [ ] **Kein Bits/Width-Check zwischen `Pattern` und aufgerufener Overload → Type Confusion** (`match`, `capture`, `replace`,
  jeweils String- und WideString-Overload, z.B. `Pcre.cpp:234-250, 252-272, 280-304, 312-336, 344-382, 390-428`)
  `Pattern::bits` (8, 16 oder 32) wird beim Kompilieren gesetzt, aber keine der sechs `match`/`capture`/`replace`-
  Funktionen prüft, ob `pattern.bits` zur aufgerufenen Overload passt – nur `pattern.p == NULL` wird geprüft:
  ```cpp
  bool RegEx::match(const Pattern& pattern, const WideString& subject)
  {
      if (pattern.p == NULL) { throw ...; }
      ...
      pcre2_match_data_wide* md = pcre2_match_data_create_from_pattern_wide((pcre2_code_wide*)pattern.p, NULL);
      // <- pattern.p könnte ein pcre2_code_8* sein, wenn Pattern mit compile(const String&) erzeugt wurde!
  ```
  Szenario: `RegEx::Pattern p = RegEx::compile(narrowString);` (setzt `bits = 8`), anschließend versehentlich
  `RegEx::match(p, someWideString)` aufgerufen. Der Cast interpretiert die interne PCRE2-Bytecode-Struktur eines
  8-Bit-kompilierten Patterns als 16-/32-Bit-Struktur (bzw. umgekehrt) – diese Layouts sind zwischen den PCRE2-
  Bibliotheksvarianten nicht kompatibel. Ergebnis: Speicherkorruption/Absturz, nicht nur eine harmlose Fehlermeldung.
  `bits`/`p` sind private und nur über `friend class RegEx` erreichbar – der Aufrufer kann sich also nicht selbst
  schützen, die Prüfung muss in der Klasse selbst erfolgen.
  Fix: in den String-Overloads `if (pattern.bits != 8) throw IllegalArgumentException(...)`, in den Wide-Overloads
  `if (pattern.bits != pcre2_bits_wide) throw IllegalArgumentException(...)`.

- [ ] **`RegEx::escape()` escaped nur 5 Zeichen, wird aber von `PerlHelper`/`PythonHelper` als vollständiges
  Regex-Escaping angeboten → Regex-Injection** (`Pcre.cpp:438-452`, `src/core/PerlHelper.cpp:47`, `src/core/PythonHelper.cpp:49`)
  ```cpp
  String compare = "-+\\*/";   // nur diese 5 Zeichen werden escaped
  ```
  `PerlHelper::escapeRegExp(s)` und `PythonHelper::escapeRegExp(s)` delegieren beide 1:1 an `RegEx::escape()`. Der
  Funktionsname suggeriert, dass ein beliebiger String sicher in einen Perl-/Python-regulären Ausdruck eingebettet
  werden kann. Tatsächlich werden PCRE-Metazeichen wie `. ^ $ ( ) [ ] { } |` und `?` **nicht** escaped – ein String wie
  `"a.b"` bleibt `"a.b"` (der Punkt bleibt Metazeichen "beliebiges Zeichen"), `"(x)"` bleibt eine Capture-Gruppe.
  Wird `escapeRegExp()` benutzt, um z.B. Nutzereingaben vor der Einbettung in ein größeres Suchmuster zu neutralisieren,
  ist das Ergebnis nicht das dokumentierte/erwartete Verhalten, sondern eine klassische Regex-Injection-Lücke.
  (In `Dir.cpp` wird `RegEx::escape()` dagegen bewusst nur als Baustein einer eigenen Wildcard→Regex-Konvertierung
  benutzt, dort ist der eingeschränkte Umfang durch die nachfolgenden `Pattern.replace(...)`-Aufrufe beabsichtigt – nur
  die beiden `escapeRegExp()`-Wrapper haben ein falsches Vertragsversprechen.)
  Fix: entweder `RegEx::escape()` um die fehlenden PCRE-Metazeichen erweitern (dann aber `Dir.cpp`s Wildcard-Logik
  gegenprüfen, die sich auf den aktuellen engen Umfang verlässt), oder für `PerlHelper`/`PythonHelper` eine eigene,
  vollständige Escape-Funktion einführen statt `RegEx::escape()` zu missbrauchen.

## Bugs (mittel)

- [ ] **`compile(const String&)`: `PCRE2_NO_UTF_CHECK` ohne jede Validierung des Patterns** (`Pcre.cpp:135`)
  ```cpp
  int options = PCRE2_UTF | PCRE2_NO_UTF_CHECK;
  ```
  `PCRE2_NO_UTF_CHECK` überspringt laut PCRE2-Doku ausdrücklich die Prüfung auf gültige UTF-8-Bytefolgen im Pattern-Text
  – bei fehlerhaftem UTF-8 ist das Verhalten laut PCRE2 explizit undefiniert ("may crash or give wrong results").
  `String` garantiert laut eigener Doku (`string.h:63`) *nicht*, dass der Inhalt immer gültiges UTF-8 ist ("kann sich
  um einen UTF-8 String, einen ASCII-String oder einen String in einer anderen Kodierung handeln"). Jeder Aufrufer,
  der ein Pattern aus nicht garantiert UTF-8-sauberer Quelle zusammenbaut (z.B. Dateinamen unter Linux, die beliebige
  Bytes enthalten dürfen), kann so über `RegEx::compile()`/`Dir::filterRegExp()` eine Struktur mit ungültigem UTF-8 in
  den Compiler geben, ohne dass PCRE2 das abfängt.
  Fix: `PCRE2_NO_UTF_CHECK` entfernen (Kosten: eine zusätzliche, schnelle Validierung beim Compile) oder zumindest
  dokumentieren, dass der Aufrufer für garantiert valides UTF-8 im Pattern verantwortlich ist.

- [ ] **`compile(const WideString&)` setzt nie `PCRE2_UTF` – auf 16-Bit-`wchar_t`-Plattformen (Windows) falsch** (`Pcre.cpp:182`)
  ```cpp
  int options = 0;   // kein PCRE2_UTF16 trotz HAVE_PCRE2_BITS_16-Pfad
  ```
  Auf Plattformen mit 4-Byte-`wchar_t` (Linux/macOS) ist das unschädlich, da dort jede Einheit bereits ein vollständiger
  Codepoint ist. Auf Plattformen mit 2-Byte-`wchar_t` (Windows, `HAVE_PCRE2_BITS_16`-Zweig) werden Zeichen jenseits der
  Basic Multilingual Plane als UTF-16-Surrogatpaare in zwei `wchar_t`-Einheiten gespeichert. Ohne `PCRE2_UTF16`
  interpretiert PCRE2 jede 16-Bit-Einheit als eigenständiges Zeichen statt das Surrogatpaar zu einem Codepoint zu
  kombinieren – `.`, Zeichenklassen (`\w`, `\d` etc.) und Quantifizierer verhalten sich für Zeichen außerhalb der BMP
  auf Windows dann falsch (zwei "kaputte" Lone-Surrogate statt einem Zeichen).
  Fix: `PCRE2_UTF` (und ggf. `PCRE2_NO_UTF_CHECK` mit denselben Vorbehalten wie oben) nur für den 16-Bit-Zweig setzen;
  für den 32-Bit-Zweig kann es wie bisher entfallen.

- [ ] **PerlRegEx-Parsing (`/pattern/flags`) ohne schließenden Trenner interpretiert den Pattern-Text teilweise als
  Flags** (`Pcre.cpp:145-158` narrow, `193-208` wide)
  ```cpp
  if (r[0] == '/') { // PerlRegEx
      ByteArray expr(regex);
      const char* oo = ::strrchr((const char*)expr, '/');   // findet ggf. nur den öffnenden Slash erneut
      if (oo) {
          expr.set(oo - (const char*)expr, 0);
          oo++;
          if (::strchr(oo, 'i')) options |= PCRE2_CASELESS;
          if (::strchr(oo, 'a')) options |= PCRE2_ANCHORED;
          ...
      }
      re = pcre2_compile_8((PCRE2_SPTR8)expr + 1, PCRE2_ZERO_TERMINATED, options, ...);
  ```
  Enthält `regex` nur einen einzigen `/` (fehlender schließender Trenner, z.B. Tippfehler `"/said"` statt `"/said/"`),
  findet `strrchr` denselben führenden `/` erneut. Der Code nullt diese Position und scannt ab `oo+1` **denselben
  Text**, der anschließend auch als Pattern (`expr + 1`) verwendet wird, auf Flag-Buchstaben. Bei `"/said"` enthält der
  vermeintliche "Flags"-String `"said"` zufällig ein `a` und ein `i` → `PCRE2_ANCHORED` und `PCRE2_CASELESS` werden
  fälschlich gesetzt, obwohl der Nutzer gar keine Flags angegeben hat. Das Pattern selbst bleibt zwar korrekt
  (`"said"`), aber die effektiven Matching-Optionen ändern sich abhängig vom zufälligen Inhalt des Patterns – ein
  stiller, schwer nachvollziehbarer Verhaltensunterschied bei einem simplen Tippfehler.
  Fix: nur dann `if (oo)`-Zweig nehmen, wenn `oo` **nicht** der erste Zeichen-Zeiger ist (`oo > (const char*)expr`),
  sonst explizit als "kein schließender Trenner gefunden" behandeln (z.B. Exception oder gesamten String als Pattern
  ohne Flags verwenden).

- [ ] **Kein NULL-Check von `pcre2_match_data_create_from_pattern_*` vor Gebrauch** (alle sechs `match`/`capture`/
  `replace`-Implementierungen, z.B. `Pcre.cpp:240, 261, 286, 318, 350, 396`)
  ```cpp
  pcre2_match_data_8* md = pcre2_match_data_create_from_pattern_8((pcre2_code_8*)pattern.p, NULL);
  int rc = pcre2_match_8((pcre2_code_8*)pattern.p, ..., md, NULL);   // md könnte NULL sein (OOM)
  ```
  Bei Speichermangel liefert `pcre2_match_data_create_from_pattern_*` `NULL`. Alle sechs Funktionen übergeben `md`
  direkt an `pcre2_match_*`, ohne vorher zu prüfen – Aufruf mit `NULL` als Match-Data ist laut PCRE2-Doku undefiniert.
  Seltener Pfad (nur bei OOM), aber leicht behebbar.
  Fix: nach jedem `pcre2_match_data_create_from_pattern_*`-Aufruf auf `NULL` prüfen und ggf. `OutOfMemoryException`
  werfen, bevor `pcre2_match_*` aufgerufen wird.

## Design

- [ ] **`Pattern` ist nach aktueller Deklaration überhaupt nicht zuweisbar** (`regex.h:64-69`)
  Durch die Kombination aus user-deklariertem Kopier-Konstruktor, dem (kaputten) "Move-Konstruktor" und dem
  user-deklarierten Destruktor wird der implizite Kopier-Zuweisungsoperator laut Standard als `deleted` deklariert,
  und ein impliziter Move-Zuweisungsoperator wird gar nicht erst generiert. Weder `p1 = p2;` noch `p1 = std::move(p2);`
  kompilieren aktuell (nur Initialisierung `Pattern p = compile(...);` funktiert, weil das eine Konstruktion ist).
  Aktuell kein Problem, weil alle Call-Sites im Repo nur initialisieren, aber für eine Werttyp-artige Klasse mit
  öffentlichem Kopier-Konstruktor ist "nicht zuweisbar" eine unerwartete Einschränkung und in der Praxis eine
  Falle für zukünftigen Code (z.B. `Pattern p; if (x) p = compile(...);` kompiliert nicht).
  Fix: zusammen mit dem Move-Konstruktor-Fix oben auch `operator=(const Pattern&)` und `operator=(Pattern&&)` explizit
  implementieren (klassisches Copy-and-Swap bietet sich an, inkl. Self-Assignment-Sicherheit).

- [ ] **Kein Match-/Backtracking-Limit gesetzt → ReDoS-Risiko** (alle `pcre2_match_*`-Aufrufe übergeben `NULL` als
  Match-Context)
  Weder für `pcre2_compile_*` noch für `pcre2_match_*` wird jemals ein `pcre2_match_context` mit
  `pcre2_set_match_limit()`/`pcre2_set_depth_limit()` erzeugt und übergeben – überall wird `NULL` (= PCRE2-Compile-Time-
  Default, i.d.R. mehrere Millionen) verwendet. Patterns kommen teils aus Aufrufer-Kontrollierten Strings
  (`Dir::filterRegExp`, generische `RegEx::compile`-API), Subjects ebenso. Ein pathologisches Pattern
  (verschachteltes `(a+)+b`-Muster o.ä.) kombiniert mit einem passenden Subject kann so den Prozess für lange Zeit
  blockieren, bevor der (sehr hohe) Default-Limit greift.
  Vorschlag: einen projektweiten `pcre2_match_context` mit sinnvollem, konfigurierbarem `match_limit`/`depth_limit`
  einführen und bei `PCRE2_ERROR_MATCHLIMIT`/`_RECURSIONLIMIT` eine eigene Exception werfen statt generisch
  `IllegalRegularExpressionException`.

- [ ] **`IllegalRegularExpressionException` wird auch für reine Laufzeitfehler beim Matchen geworfen, nicht nur für
  ungültige Syntax** (z.B. `Pcre.cpp:245, 265, 292, 323, 365, 411`)
  In `match`/`capture`/`replace` ist das Pattern zu diesem Zeitpunkt bereits erfolgreich kompiliert (sonst wäre
  `compile()` schon fehlgeschlagen). Ein negativer Rückgabewert von `pcre2_match_*` ungleich `PCRE2_ERROR_NOMATCH`
  bedeutet i.d.R. einen Laufzeitfehler (z.B. `PCRE2_ERROR_MATCHLIMIT`, ungültiges UTF-8 im *Subject*), nicht eine
  "illegale reguläre Expression". Die Exception ist damit semantisch irreführend für Aufrufer, die anhand des
  Exception-Typs unterscheiden wollen, ob ihr Pattern fehlerhaft war.
  Fix: für den Matching-Fehlerfall `OperationFailedException` (existiert bereits in `exceptions.h:55`) statt
  `IllegalRegularExpressionException` verwenden.

## Doku / Kosmetik

- [ ] Falscher Include-Guard-Kommentar am Dateiende: `#endif // PPLIB_CORE_MEMORYHEAP_H_` (`regex.h:97`) – Copy-&-Paste-
      Rest aus einem anderen Header, sollte `PPLIB_CORE_REGEX_H_` heißen.
- [ ] Debug-Ausgaben in Produktionscode: `pplib::PrintDebug("RegEx::Pattern::Pattern using copy constructor\n")`
      (`Pcre.cpp:89`), `"...using move constructor\n"` (`Pcre.cpp:106`) sowie `pplib::PrintDebug("debug 1\n")` in
      `match(const Pattern&, const WideString&)` (`Pcre.cpp:255`) und ein auskommentiertes `// pplib::PrintDebug("debug 2, rc=%d\n", rc);`
      (`Pcre.cpp:266`) wirken wie vergessene Debugging-Reste; `PrintDebug` formatiert und gibt bei jedem Aufruf
      tatsächlich Text aus (kein No-Op im Release-Build).
- [ ] `RegEx::escape()` ist dokumentiert als "Fügt dem String Escape-Zeichen zu, zur Verwendung in einem Regulären
      Ausdruck" – das klingt nach einem allgemeinen Escaping, ist aber bewusst nur auf `- + \ * /` beschränkt (siehe
      Bug-Eintrag oben zu `PerlHelper`/`PythonHelper`). Sollte in der Doku explizit als "nur für die Wildcard→Regex-
      Konvertierung in Dir" gekennzeichnet werden, um Fehlgebrauch wie in `PerlHelper::escapeRegExp` zu vermeiden.

## Verifiziert OK (kein Handlungsbedarf)

- Kein manuell dimensioniertes Offsets-/Ovector-Array wie beim klassischen PCRE1-API: überall wird
  `pcre2_match_data_create_from_pattern_*` verwendet, das die Ovector-Größe passend zur Anzahl der Capture-Gruppen des
  jeweiligen kompilierten Patterns allokiert – kein Off-by-one- oder "Array zu klein für N Gruppen"-Risiko wie es bei
  fest kodierten Offsets-Arrays typisch wäre.
- Kein globaler/statischer PCRE(2)-Zustand (keine gecachten Character-Tables, kein globales Pattern-Cache) – jede
  `Pattern`/`match_data`-Instanz ist unabhängig; PCRE2 selbst ist laut Upstream-Doku thread-sicher, solange
  verschiedene Threads unterschiedliche `pcre2_code`/`pcre2_match_data`-Objekte verwenden, was hier der Fall ist.
- `pattern.p == NULL`-Check vor jeder Verwendung verhindert zumindest die Nutzung eines default-konstruierten/leeren
  `Pattern`-Objekts.
- Kopier-Konstruktor von `Pattern` verwendet korrekt `pcre2_code_copy_*` (echte Tiefenkopie des kompilierten Patterns)
  statt den rohen Zeiger zu teilen – zwei unabhängig über den Kopier-Konstruktor erzeugte `Pattern`-Objekte haben
  getrennte Handles und können unabhängig voneinander zerstört werden.
