# Exception Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/core/baseexception.h` + `src/core/Exceptions.cpp` (Basis-Klasse `pplib::Exception`).
`include/pplib/exceptions.h` (185× `PPLIBEXCEPTION`-Makro-Aufrufe) nur zur Prüfung des Makro-Musters auf strukturelle Konsistenz
gegengelesen, einzelne konkrete Exception-Typen sowie deren Wurfstellen im Code sind NICHT Teil dieses Reviews.
Verifiziert: `String::vasprintf()` (String.cpp:375-397, allokiert dynamisch über `reserve()`, kein Fixed-Size-Puffer),
`String::c_str()` (string.h:1341, trivial/non-throwing), `String::reserve()` (String.cpp:155-166, wirft `OutOfMemoryException`).
C++-Standard laut `CMakeLists.txt`: C++17.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **Self-Assignment in `operator=` führt zu Use-after-Free** (`Exceptions.cpp:71-80`)
  ```cpp
  Exception& Exception::operator=(const Exception& other) throw()
  {
      if (ErrorText) free(ErrorText);                 // <- eigener Speicher wird freigegeben
      if (other.ErrorText) {                           // <- other IST this bei Self-Assignment!
          ErrorText = strdup(other.ErrorText);         // <- liest bereits freigegebenen Speicher
      } else {
          ErrorText = NULL;
      }
      return *this;
  }
  ```
  Bei `e = e;` ist `other` dieselbe Instanz wie `this`. `free(ErrorText)` gibt den Block frei, `other.ErrorText`
  zeigt danach auf denselben (nun freigegebenen) Speicher – der Pointer wird durch `free()` nicht auf `NULL`
  gesetzt. Der anschließende `strdup(other.ErrorText)` liest also aus bereits freigegebenem Speicher
  (Heap-Use-after-Free), was unter ASan/Valgrind sofort auffällt und je nach Allocator zu Heap-Korruption führen kann.
  Genau das gleiche Muster, das im AssocArray-Review als kritisch eingestuft wurde (`a = a;`).
  Fix: Self-Check am Anfang, oder Copy-and-Swap:
  ```cpp
  Exception& Exception::operator=(const Exception& other) throw()
  {
      if (this == &other) return *this;
      if (ErrorText) free(ErrorText);
      ErrorText = other.ErrorText ? strdup(other.ErrorText) : NULL;
      return *this;
  }
  ```

- [ ] **Makro-generierte Subklassen haben keinen NULL-Schutz für den Format-String → Absturz statt Exception** (`baseexception.h:72-78`, `Exceptions.cpp:113-124`)
  Die Basisklasse selbst prüft in ihrem eigenen Varargs-Konstruktor `if (msg) { ... }` (Exceptions.cpp:84), bevor
  formatiert wird. Der von `PPLIBEXCEPTION` generierte Konstruktor jeder der 185 konkreten Exception-Klassen tut das
  **nicht**:
  ```cpp
  name(const char* msg, ...) noexcept
  {
      va_list args;
      va_start(args, msg);
      copyText(msg, args);        // kein if(msg)-Check!
      va_end(args);
  }
  ```
  `copyText(const char* fmt, va_list args)` reicht `fmt` ungeprüft an `Msg.vasprintf(fmt, args)` weiter, was intern
  `::vsnprintf(nullptr, 0, fmt, args_copy)` aufruft – ein NULL-Formatstring ist laut C-Standard undefiniertes
  Verhalten und führt in der Praxis (glibc, MSVC-CRT) zu einem Absturz (SIGSEGV), *nicht* zu einer C++-Exception.
  Der umgebende `try { ... } catch(...)` in `copyText()` fängt das nicht ab, da es kein C++-Exception-Mechanismus ist.
  Konkretes Szenario: `throw pplib::IllegalArgumentException(getenv("OPTIONAL_VAR"));` – ist die Env-Var nicht gesetzt,
  liefert `getenv()` `NULL`, und statt einer sauber geworfenen Exception stürzt der Prozess ab.
  `throw pplib::Exception(nullptr)` (Basisklasse direkt) ist dagegen sicher – Inkonsistenz zwischen Basis- und
  generierten Klassen.
  Fix: Guard in `copyText(fmt, args)` selbst ergänzen (schützt automatisch alle 185 Subklassen zentral):
  ```cpp
  void Exception::copyText(const char* fmt, va_list args) throw()
  {
      free(ErrorText);
      if (!fmt) { ErrorText = NULL; return; }
      try { ... }
  }
  ```

- [ ] **`toString()` als `noexcept` deklariert, ruft aber werfenden Code auf → `std::terminate()` statt sauberer Fehlerausgabe** (`baseexception.h:56`, `Exceptions.cpp:134-140`)
  ```cpp
  String Exception::toString() const throw()      // == noexcept
  {
      String str;
      str.setf("%s", what());
      if (ErrorText) str.appendf(" [%s]", (const char*)ErrorText);   // <- kann werfen!
      return str;
  }
  ```
  `setf()`/`appendf()` laufen über `String::vasprintf()`, welches bei `vsnprintf`-Fehler `throw Exception(...)` wirft,
  und über `reserve()`, welches bei fehlgeschlagener Allokation `throw OutOfMemoryException()` wirft. Verlässt eine
  Exception eine als `noexcept` markierte Funktion, ruft die Laufzeit sofort `std::terminate()` auf – der Prozess
  stirbt sofort, unabhängig von jedem umgebenden `try/catch`. Besonders bitter: gerade bei einer `OutOfMemoryException`
  (wenig Speicher verfügbar) ist die Wahrscheinlichkeit, dass `toString()` selbst beim Versuch, die Meldung
  zusammenzubauen, erneut aus Speichermangel wirft, am höchsten – das Catch-and-log-Muster
  `catch (Exception& e) { std::cerr << e; }` (nutzt `operator<<`, das intern `toString()` aufruft) kann so den
  Prozess hart abschießen, statt die Fehlermeldung auszugeben.
  Fix: `toString()` intern absichern (z.B. `try { ... } catch(...) { return String(what()); }`) oder die
  `noexcept`-Zusage aus Header und Implementierung entfernen, wenn ein Wurf toleriert werden soll.

## Design

- [ ] **`what()` liefert nie die eigentliche Fehlermeldung, nur den Klassennamen** (`baseexception.h:54`, `Exceptions.cpp:57-60`, Makro `baseexception.h:79-82`)
  Sowohl `Exception::what()` ("PPLException") als auch jede generierte Subklasse (`STR_VALUE(name)`) geben ausschließlich
  einen statischen Klassennamen zurück – die über `copyText()`/den Konstruktor gesetzte Nachricht steckt nur in
  `text()`/`toString()`. Das weicht vom üblichen `std::exception`-Vertrag ab, bei dem `what()` die aussagekräftige
  Beschreibung liefert. Jeder generische Catch-Handler, der (wie in sehr viel Fremd- und Bibliothekscode üblich)
  nur `catch (const std::exception& e) { log(e.what()); }` schreibt, sieht nur z.B. `"FileNotFoundException"` und
  verliert die eigentliche Information (welche Datei, welcher Pfad). Das ist zwar bewusst so gebaut, um dangling
  Pointer über `what()` zu vermeiden (siehe „Verifiziert OK"), aber als API-Falle für jeden erwähnenswert, der die
  Klassen nur über `std::exception&` konsumiert.
  Vorschlag: entweder `what()` in der Basisklasse so überschreiben, dass sie Klassenname *und* Text kombiniert
  (das wird bereits stabil in `ErrorText` gehalten, also kein neues Lifetime-Problem), oder das Verhalten prominent
  in der Klassendoku festhalten.

- [ ] **185 nahezu identische Subklassen ohne Nutzlast – passt zu „weniger Exceptions" aus REFACTORING.md**
  `exceptions.h` besteht ausschließlich aus `PPLIBEXCEPTION(Name, Basis)`-Zeilen; jede Klasse unterscheidet sich nur
  durch Namen und Vererbungshierarchie, keine hat zusätzliche Member oder Fehlercode. Fürs Catchen nach Kategorie
  reicht die Vererbungshierarchie zwar aus, erzeugt aber sehr viel Boilerplate und einen riesigen, schwer zu
  überblickenden Header. Passend zum Refactoring-Ziel „weniger Exceptions": Konsolidierung auf deutlich weniger
  Klassen mit einem Fehlercode/Enum als Member wäre eine Option, ohne bei den meisten `catch`-Stellen die
  Granularität zu verlieren (Vergleich über den Code statt über den `catch`-Typ).

- [ ] **Kein Move-Konstruktor / keine Move-Assignment – nur Rule-of-Three statt Rule-of-Five** (`baseexception.h:48-53`)
  Da Copy-Konstruktor und `operator=` explizit deklariert sind, generiert der Compiler keine impliziten Move-Operationen.
  Jedes `throw`/Catch-by-value sowie ein eventuelles `std::move(exceptionObj)` fällt auf den Copy-Konstruktor
  zurück (`strdup` statt Pointer-Übernahme). Bei Exceptions performance-unkritisch, aber inkonsistent mit der
  „Modernisierung der Klassen" aus REFACTORING.md. Ein `noexcept`-Move (Pointer übernehmen, `other.ErrorText = NULL`)
  wäre trivial zu ergänzen.

- [ ] **`copyText()` ist public und erlaubt nachträgliches Mutieren der Fehlermeldung nach dem Werfen**
  Beide Overloads (`copyText(const char*)`, `copyText(const char*, va_list)`) sind Teil der öffentlichen Schnittstelle,
  obwohl sie laut Grep aktuell nirgends außerhalb der Makro-generierten Konstruktoren aufgerufen werden. Eine
  Exception-Instanz sollte nach dem Werfen als unveränderlicher Wert gelten; ein öffentliches „Nachträglich-Text-Setzen"
  öffnet die Tür für Überraschungen (z.B. `catch(e) { e.copyText(...); throw; }` ändert die Nachricht einer bereits
  fliegenden Exception). Erwägenswert: beide Methoden `protected` machen (nur für abgeleitete Klassen/Konstruktoren
  nutzbar).

## Doku / Kosmetik

- [ ] **Uneinheitliche Exception-Spezifikation: `noexcept` im Header, überwiegend `throw()` in der Implementierung**
  (`baseexception.h` durchgehend `noexcept`; `Exceptions.cpp` fast überall `throw()`, nur `Exception(const String&)`
  nutzt `noexcept`). Funktional identisch (in C++17 ist `throw()` ein deprecated Alias für `noexcept(true)`), aber
  stilistisch inkonsistent und erzeugt ggf. Deprecation-Warnings. Passt zur „Modernisierung der Klassen" – überall
  auf `noexcept` vereinheitlichen.

- [ ] **`copyText(const char* str)` (1-Arg-Variante) hat aktuell keinen einzigen Aufrufer im gesamten Repo**
  (`baseexception.h:58`, `Exceptions.cpp:107-111`) – weder in dieser Basisklasse noch im Makro noch sonstwo im Code
  wird sie benutzt. Entweder dokumentieren, wofür sie als öffentliche API gedacht ist, oder entfernen. Sie teilt
  übrigens denselben Freigabe-vor-Lese-Fehler wie `operator=`, falls sie einmal mit dem eigenen `text()`-Rückgabewert
  aufgerufen würde (`e.copyText(e.text())` → UAF) – aktuell nur latent, da unbenutzt.

- [ ] **`if (ErrorText) free(ErrorText);`-Muster mehrfach unnötig verklausuliert** (`Exceptions.cpp:73`, `~Exception`, etc.)
  `free(NULL)` ist laut C-Standard ein garantiertes No-op; der `if`-Guard ist überflüssig, kein Bug, nur Rauschen.

- [ ] **Keine Doku-Kommentare an der Klasse `Exception` bzw. am Makro** – andere Kern-Header (z.B. `assocarray.h`)
  sind ausführlich mit Doxygen-Blöcken dokumentiert, `baseexception.h` hat keinen einzigen Klassenkommentar. Speziell
  der Vertrag „`what()` liefert Klassenname, `text()`/`toString()` liefern die eigentliche Meldung" gehört dringend
  dokumentiert (siehe Design-Punkt oben).

## Verifiziert OK (kein Handlungsbedarf)

- `what()` liefert in der Basisklasse und in allen 185 generierten Subklassen ausschließlich String-Literale
  (statische Speicherdauer) – niemals einen Pointer auf einen lokalen/temporären Puffer. Kein Dangling-Pointer-Risiko,
  unabhängig von der Lebensdauer des Exception-Objekts.
- `text()` liefert entweder den heap-gehaltenen `ErrorText` (gültig für die gesamte Lebensdauer des Objekts) oder das
  Literal `""` – ebenfalls nie dangling.
- Copy-Konstruktor und `operator=` verwenden konsequent `strdup`/`free` (kein Shared-Ownership, kein Double-Free bei
  normaler, nicht-self-referenzierender Nutzung).
- Kein globaler/statischer Zustand in `Exception` – jede Instanz hält ihre Nachricht ausschließlich in ihrem eigenen
  `ErrorText`-Member. Die Klasse ist von Haus aus thread-sicher (keine „letzter Fehler"-Globalvariable o.ä.).
- `Exception(const char* msg, ...)` (Basis-Konstruktor) prüft `msg` korrekt auf `NULL`, bevor formatiert wird –
  im Gegensatz zu den generierten Subklassen (siehe Bug oben) hier sicher.
- Nachrichtenformatierung läuft komplett über `String::vasprintf()`, welches die Zielgröße per doppeltem
  `vsnprintf`-Aufruf dynamisch bestimmt (String.cpp:375-397) – kein Fixed-Size-Puffer, keine Truncation bei langen
  Fehlermeldungen.
- Formatierungsfehler innerhalb der Konstruktoren/`copyText(fmt,args)` werden durch ein lokales `try { ... } catch(...)`
  abgefangen, sodass eine während des Bauens der Fehlermeldung geworfene Exception (z.B. `OutOfMemoryException` aus
  `reserve()`) nicht aus dem Exception-Konstruktor selbst entkommt – korrektes Verhalten für die
  Stack-Unwinding-Sicherheit der Konstruktoren (im Gegensatz zu `toString()`, siehe Bug oben).
- `exceptions.h` besteht tatsächlich ausschließlich aus `PPLIBEXCEPTION(...)`-Aufrufen (185 Stück) – keine Subklasse
  weicht vom Makro-Muster ab oder implementiert eigene Konstruktoren/`what()`-Overrides; das Makro-Muster selbst ist
  strukturell konsistent (identischer Default-Ctor, Varargs-Ctor, `what()` in jeder generierten Klasse).
- `Exception(const String& msg)` ist zurecht als `noexcept` ohne Try/Catch markiert: `String::c_str()` ist eine
  triviale, nicht werfende Inline-Funktion (string.h:1341), `strdup()` ist eine C-Funktion und wirft nie eine
  C++-Exception (liefert bei OOM `NULL`, was hier korrekt zu einem leeren `ErrorText` statt einem Crash führt).
