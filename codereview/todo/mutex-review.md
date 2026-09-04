# Mutex Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/core/mutex.h` + `src/core/Mutex.cpp` (135 Zeilen).
Mitgelesen: `include/pplib/core/threads.h` + `src/core/Threads.cpp`, da `Thread` die einzige Verwendung von
`Mutex::wait()`/`Mutex::signal()` im gesamten Repo ist (per `grep` verifiziert) und deren Aufrufmuster direkt
mit den hier gefundenen Bugs zusammenhängt. Konkrete Thread-spezifische Konsequenzen stehen in `threads-review.md`.
Alle als "empirisch bestätigt" markierten Befunde wurden gegen `build/coverage/libpplib_core.a` kompiliert und
laufen gelassen (siehe Fix-Hinweise für die jeweiligen Minimal-Repros).

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`wait()`/`signal()` sperren denselben `std::mutex` erneut, der über `lock()`/`unlock()` schon gehalten sein kann → Selbstdeadlock** (Mutex.cpp:95, Mutex.cpp:124)
  Der Header dokumentiert für `wait()` explizit das Muster "erst locken, dann warten":
  ```
  * @note
  * War der Mutex vor Aufruf dieser Funktion bereits gelocked, ist er es nach dem Aufruf
  * immer noch.
  ```
  Die Implementierung hält sich aber nicht an dieses Muster, sondern versucht, den *identischen* internen
  `std::mutex` ein zweites Mal zu sperren:
  ```cpp
  bool Mutex::wait(int milliseconds) noexcept
  {
      ...
      std::unique_lock<std::mutex> lock(h->mtx);   // <- h->mtx ist dasselbe Mutex-Objekt wie in lock()/unlock()
      ...
  }
  ```
  ```cpp
  bool Mutex::signal() noexcept
  {
      ...
      std::lock_guard<std::mutex> lock(h->mtx);    // <- ebenfalls dasselbe h->mtx
      ...
  }
  ```
  `std::mutex` ist nicht rekursiv: sperrt ein Thread ihn ein zweites Mal, ist das laut Standard undefiniertes
  Verhalten. Auf glibc/pthreads (Linux, `PTHREAD_MUTEX_NORMAL`, keine Fehlerprüfung) bedeutet das konkret:
  der Thread blockiert für immer auf einem Mutex, den er selbst schon hält – ein klassischer Selbstdeadlock,
  keine Exception (der `catch(...)` in `wait()`/`signal()` greift hier also gar nicht, da nichts geworfen wird).

  Das dokumentierte "lock() → wait() → unlock()"-Muster ist damit für *jeden* Aufrufer, der es genauso benutzt
  wie es beschrieben ist, kaputt. Genau dieses Muster verwendet `Thread::threadSleep()` und
  `Thread::threadWaitSuspended()` (siehe `threads-review.md`).

  Empirisch bestätigt (Minimal-Repro ohne Thread-Klasse):
  ```cpp
  Mutex m;
  m.lock();
  m.wait(2000);   // sollte laut Doku nach ~2s zurückkehren
  ```
  Kompiliert gegen `libpplib_core.a` und mit `timeout 5` laufen gelassen: Prozess hängt und wird nach 5s vom
  Timeout gekillt – `wait()` kehrt nie zurück, obwohl ein Timeout von 2000ms übergeben wurde. Das Timeout wird
  nie ausgewertet, weil der Thread schon beim Erwerb des `unique_lock` blockiert, bevor `wait_for()` überhaupt
  aufgerufen wird.

  Fix: Grundsätzliches Redesign nötig, da das Problem strukturell ist – `Mutex` kombiniert einen klassischen
  Mutex (lock/unlock/tryLock) mit einem Event/Signal-Objekt (wait/signal) auf demselben internen `std::mutex`,
  ohne dass der Aufrufer der `unique_lock` mitgeben kann, den er selbst schon besitzt. Optionen:
  - Sauber: `wait()` erwartet **nicht** gelockt zu werden – Doku korrigieren, Aufrufer in `Thread` anpassen
    (nicht mehr `lock(); ...; wait(); ...; unlock();`, sondern `wait()` übernimmt die Sperre selbst für die
    Dauer des Wartens).
  - Alternativ: Mutex und Event/Condition-Wrapper in zwei Klassen trennen, damit die Semantik pro Klasse klar
    und ohne Selbstüberschneidung ist.

- [ ] **`Mutex` ist ohne Einschränkung kopierbar → Double-Free beim Kopieren** (mutex.h:51-137)
  Die Klasse deklariert weder Kopier-/Move-Konstruktor noch die entsprechenden Zuweisungsoperatoren. Der
  Compiler generiert daher implizit einen (deprecated, aber gültigen) Kopierkonstruktor und
  Kopier-Zuweisungsoperator, die den rohen `handle`-Zeiger flach kopieren:
  ```cpp
  class Mutex
  {
  private:
      void* handle;   // einziges Datenmitglied, Eigentümer eines heap-allokierten PPLMUTEX
  public:
      Mutex();
      ~Mutex() noexcept;
      // kein Mutex(const Mutex&), kein operator=(const Mutex&), kein Move – alles implizit vorhanden
  ```
  Zwei `Mutex`-Objekte teilen sich danach denselben `handle`. Werden beide zerstört, ruft `~Mutex()` zweimal
  `delete h` auf denselben Zeiger auf.

  Empirisch bestätigt:
  ```cpp
  Mutex a;
  Mutex b = a;   // kompiliert anstandslos
  ```
  Ausführung gegen `libpplib_core.a`: `free(): double free detected in tcache 2`, Prozess bricht mit `SIGABRT`
  ab (Exit-Code 134).

  Betrifft auch `Thread`, das `Mutex threadmutex;` als Member hält und ebenfalls keine Kopiersperre deklariert
  – siehe `threads-review.md` (Design-Abschnitt).

  Fix:
  ```cpp
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  Mutex(Mutex&&) = delete;            // oder sauber implementieren: handle übernehmen, Quelle auf nullptr setzen
  Mutex& operator=(Mutex&&) = delete;
  ```

## Bugs (mittel)

- [ ] **`unlock()` ohne vorheriges `lock()` ist UB, nicht "nur" eine Exception** (Mutex.cpp:73-82)
  ```cpp
  void Mutex::unlock()
  {
      PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
      try {
          h->mtx.unlock();
      }
      catch (...) {
          throw MutexLockingException("Mutex::unlock");
      }
  }
  ```
  Der Header verspricht `@exception MutexLockingException Mutex konnte nicht entsperrt werden`. Für
  `std::mutex::unlock()` ist das Entsperren eines nicht vom aufrufenden Thread gehaltenen Mutex laut Standard
  aber undefiniertes Verhalten – es ist nicht garantiert, dass überhaupt eine Exception geworfen wird (auf
  den meisten Implementierungen, inkl. glibc/pthreads mit `PTHREAD_MUTEX_NORMAL`, wird sie es nicht, das
  Verhalten ist einfach unspezifiziert/kann still "funktionieren" oder Datenstrukturen korrumpieren). Der
  `try/catch` erweckt den Eindruck einer sicheren Fehlerbehandlung, die es für den dokumentierten Problemfall
  in der Praxis nicht gibt.
  Fix: Dokumentation präzisieren ("undefiniertes Verhalten bei unlock() ohne vorheriges lock() – die Exception
  wird nur in den Fällen geworfen, in denen die Standardbibliothek tatsächlich eine wirft"), oder eigenen
  "locked"-Zustand mitführen (z.B. via `thread_local`/Zähler), um definitiv und portabel zu erkennen und zu
  werfen.

- [ ] **Rekursives `lock()` durch denselben Thread deadlockt still, statt wie dokumentiert eine Exception zu werfen** (Mutex.cpp:62-71, mutex.h:73-81)
  ```cpp
  void Mutex::lock()
  {
      PPLMUTEX* h = static_cast<PPLMUTEX*>(handle);
      try {
          h->mtx.lock();
      }
      catch (...) {
          throw MutexLockingException("Mutex::lock");
      }
  }
  ```
  Header: `@exception MutexLockingException Mutex konnte nicht gesperrt werden`. Ruft derselbe Thread `lock()`
  ein zweites Mal auf (z.B. durch einen Programmierfehler mit verschachtelten `MutexLock`-Guards auf demselben
  Objekt), wird auf Linux/glibc **nicht** geworfen, sondern der Thread blockiert für immer (non-rekursiver
  `std::mutex`, keine Fehlerprüfung). Derselbe Denkfehler wie beim Kernbefund oben (`wait()`/`signal()`), hier
  aber für den ganz einfachen Fall "Anwender ruft `lock()` zweimal auf".
  Fix: Doku ergänzen ("Mutex ist nicht rekursiv – erneutes Sperren durch denselben Thread führt zu Deadlock,
  nicht zu einer Exception"), oder – falls rekursive Nutzung erwünscht ist – auf `std::recursive_mutex`
  umstellen.

## Design

- [ ] **Eine Klasse, zwei Verantwortlichkeiten: klassischer Mutex + binäres Signal/Event, beide auf demselben internen Lock** (mutex.h gesamte Klasse)
  `lock()/unlock()/tryLock()` implementieren einen Mutex, `wait()/signal()` ein binäres Event (mit
  `condition_variable` + `signaled`-Flag). Beide Verantwortlichkeiten teilen sich denselben `std::mutex`
  (`h->mtx`), was exakt der Auslöser für den Selbstdeadlock oben ist. Empfehlung: in zwei Klassen trennen
  (`Mutex` rein für Locking, ein separates `Event`/`ConditionVariable`-Objekt für das Signalisieren), damit
  die beiden Nutzungsmuster sich nicht gegenseitig über denselben internen Lock in die Quere kommen können.

- [ ] **`signal()` weckt mit `notify_one()` nur einen Warter** (Mutex.cpp:127)
  Für den aktuellen Anwendungsfall (ein `Thread` signalisiert sich selbst über sein eigenes `threadmutex`)
  unproblematisch, aber falls `Mutex` als generisches Utility auch mit mehreren Wartern auf demselben Objekt
  verwendet werden soll, sollte das (kein Broadcast) explizit dokumentiert werden.

## Doku / Kosmetik

- [ ] **Inkonsistente `handle`-Nullprüfung**: `wait()` (Zeile 92) und `signal()` (Zeile 120) prüfen
  `if (!handle) return false;`, `lock()`, `unlock()` und `tryLock()` dagegen nicht. Da `handle` nur in
  `~Mutex()` auf `nullptr` gesetzt wird, ist die Prüfung in der Praxis toter Code (jede Verwendung nach der
  Destruktion ist bereits für sich UB) – entweder konsequent überall ergänzen oder als das erkennen, was sie
  ist, und entfernen.
- [ ] `lock()`/`unlock()` verschlucken über `catch (...)` die ursprüngliche `std::system_error`-Information
  (Fehlercode, Message) komplett und werfen stattdessen eine kontextlose `MutexLockingException("Mutex::lock")`.
  Für die Fehlersuche wäre es hilfreich, `e.what()` oder den `error_code` in die eigene Exception-Message zu
  übernehmen.

## Verifiziert OK (kein Handlungsbedarf)

- `Mutex::Mutex()` verwendet korrekt `new (std::nothrow) PPLMUTEX()` und wirft explizit `OutOfMemoryException`
  bei Fehlschlag – im Gegensatz zum `new THREADDATA` in `Thread::Thread()` (siehe `threads-review.md`).
- `~Mutex()` ist `noexcept`, prüft `handle` vor dem `delete` und setzt ihn auf `nullptr` – Selbstschutz gegen
  doppelte Zerstörung *desselben* Objekts ist gegeben (das Problem ist ausschließlich die Kopierbarkeit, s.o.).
- Spurious-Wakeup-Schutz in `wait()` ist korrekt über das Prädikat `[h]() { return h->signaled; }` umgesetzt,
  das nach jedem Aufwachen erneut geprüft wird (`wait_for`/`wait` mit Prädikat-Überladung).
- `signal()` setzt das Flag unter Lock, ruft `notify_one()` aber erst **nach** Verlassen des Lock-Scopes
  (Zeilen 123-127) – das ist die empfohlene Reihenfolge, um zu vermeiden, dass der aufgeweckte Warter sofort
  wieder auf denselben Mutex blockiert.
