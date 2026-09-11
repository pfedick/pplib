# Exception Review 2 – Befunde & Nachprüfung

Review vom 2026-09-11, Scope: `include/pplib/core/baseexception.h` + `src/core/Exceptions.cpp` (Basis-Klasse `pplib::Exception` und Makro `PPLIBEXCEPTION`).
Gegenprüfung des vorherigen Reviews (`codereview/todo/exceptions-review.md`) sowie Analyse der durchgeführten Änderungen.
C++-Standard laut `CMakeLists.txt`: C++17.

## Status der bisherigen Befunde aus Review 1

| Befund aus Review 1 | Status | Anmerkung |
|---|---|---|
| Self-Assignment in `operator=` führt zu Use-after-Free | **Erledigt** | Self-Check `if (this == &other)` vorhanden |
| Makro-generierte Subklassen ohne NULL-Schutz | **Teilweise / Neu** | Neuer Varargs-Ctor ruft `initFromFormat` auf; dort fehlt der `if (!fmt)` Guard |
| `toString()` als `noexcept` mit werfendem Code | **Erledigt** | `noexcept` wurde aus Header & Cpp entfernt |
| `what()` liefert nur Klassennamen | **Erledigt / Neu** | `className()` + Lazy-Formatting in `what()` implementiert; Format-Dopplung in `toString()` |
| 185 Subklassen konsolidieren | **Offen** | Architektur-Ziel für später |
| Fehlender Move-Ctor / Move-Assignment | **Erledigt** | Vorhanden |
| `copyText()` ist public | **Erledigt** | `copyText` entfernt, `initFromFormat` ist `protected` |
| Uneinheitliche Spezifikation (`noexcept` vs `throw()`) | **Teilweise** | Zeile 116 in `Exceptions.cpp` nutzt noch `throw()` |
| Unbenutzte `copyText(const char*)` | **Erledigt** | Entfernt |
| `if (ErrorText) free(ErrorText)` Redundanz | **Erledigt** | Bereinigt |
| Fehlende Dokumentation | **Offen** | Doxygen-Blöcke stehen noch aus |

---

## Neue Befunde / Regressionen

### Bugs (kritisch / fehlerhaftes Verhalten)

- [ ] **1. `toString()` verdoppelt die Fehlermeldung** (`Exceptions.cpp:144-150`)
  ```cpp
  String Exception::toString() const
  {
      String str;
      str.setf("%s", what());
      if (ErrorText) str.appendf(" [%s]", (const char*)ErrorText);
      return str;
  }
  ```
  Da `what()` nun `"ClassName: ErrorText"` zurückgibt, liefert `toString()` bei gesetztem Fehlertext:
  `"FileNotFoundException: /path/file.txt [/path/file.txt]"`.
  Die Nachricht ist doppelt im String.
  Fix: `str.setf("%s", className());` statt `what()` verwenden, um das ursprüngliche Format `"ClassName [ErrorText]"` beizubehalten, oder direkt `return String(what());`.

  ==> FIXED

- [ ] **2. Fehlender NULL-Check in `initFromFormat` führt zu Absturz** (`Exceptions.cpp:89-99`)
  ```cpp
  void Exception::initFromFormat(const char* fmt, va_list args) noexcept
  {
      try {
          String Msg;
          Msg.vasprintf(fmt, args);    // <- Crash wenn fmt == nullptr!
          ErrorText = strdup((const char*)Msg);
      }
      catch (...) {
          ErrorText = NULL;
      }
  }
  ```
  Der Konstruktor `Exception(const char* msg, ...)` in Zeile 116 prüft `if (msg)`. Der vom Makro aufgerufene Pfad `initFromFormat` tut das nicht. Wenn `fmt == nullptr` übergeben wird (z. B. `throw IllegalArgumentException(getenv("UNSET"))`), stürzt `vsnprintf` im CRT ab (UB / Access Violation).
  Fix: Am Anfang von `initFromFormat`:
  ```cpp
  if (!fmt) {
      ErrorText = nullptr;
      return;
  }
  ```

  ==> FIXED

- [ ] **3. `what()` ist thread-unsicher (Data Race auf `whatBuffer`)** (`Exceptions.cpp:52-69`)
  ```cpp
  const char* Exception::what() const noexcept
  {
      if (whatBuffer.isEmpty()) {
          try {
              if (ErrorText && *ErrorText) {
                  whatBuffer = String(className()) + ": " + (const char*)ErrorText;
  ...
      return whatBuffer.c_str();
  }
  ```
  `what()` ist eine `const`-Methode. Wenn dieselbe Exception-Instanz (z. B. eine gecachte oder über Threads geteilte `std::exception_ptr`) in mehreren Threads gefangen und `e.what()` aufgerufen wird, schreiben mehrere Threads ungeschützt gleichzeitig in das `mutable String whatBuffer`.
  Zudem ist `whatBuffer` ein `pplib::String` (Refcounted / dynamisch).

  ==> FIXED

- [ ] **4. Move-Operationen hinterlassen Quelle in inkonsistentem Zustand** (`Exceptions.cpp:80-87`, `109-114`)
  ```cpp
  Exception::Exception(Exception&& other) noexcept
  {
      ErrorText = other.ErrorText;
      whatBuffer = other.whatBuffer;
      other.ErrorText = NULL;
      other.whatBuffer.clear();
  }
  ```
  `whatBuffer = std::move(other.whatBuffer);` wäre sauberer als Kopie. Wichtiger noch: Wenn `other` vor dem Move bereits `what()` aufgerufen hatte, behält `whatBuffer` der Zielinstanz den Text, aber falls `what()` noch nicht gerufen wurde, ist `whatBuffer` leer – das ist in Ordnung, sollte aber einheitlich per Move bewegt werden.

### Kosmetik & Konsistenz

- [ ] **5. Veraltetes `throw()` in Zeile 116 von `Exceptions.cpp`**
  ```cpp
  Exception::Exception(const char* msg, ...) throw()
  ```
  Header deklariert `noexcept`, cpp definiert `throw()`. Bitte auf `noexcept` vereinheitlichen.

  ==> FIXED

- [ ] **6. Doppelte Initialisierung von `ErrorText` im Default-Ctor** (`baseexception.h:46`, `baseexception.h:55`)
  Im Header ist `char* ErrorText = nullptr;` deklariert, und der Inline-Konstruktor setzt erneut `ErrorText = nullptr;`. Das `using std::exception::exception;` in Zeile 53 kann zudem verwirren und ist überflüssig, da alle relevanten Konstruktoren explizit definiert sind.

  ==> FIXED
