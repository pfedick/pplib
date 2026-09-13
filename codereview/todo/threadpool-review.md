# ThreadPool Review – Befunde

Review vom 2026-09-13, Scope: `include/pplib/core/threadpool.h` + `src/core/ThreadPool.cpp` (345 Zeilen).
Mitgelesen: `include/pplib/core/threads.h` + `src/core/Threads.cpp`, da `ThreadPool` direkt Zeiger auf
`pplib::Thread` aggregiert und deren Lebenszyklus steuert.

* Review done by: GitHub Copilot (Gemini 3.8 Flash)

## Bugs (kritisch)

- [ ] **Data Race / Iteration unter unzureichendem Lock: `begin()` und `end()` exponieren ungeschützte Iteratoren** (threadpool.h:48-51, ThreadPool.cpp:168-202)
  ```cpp
  ThreadPool::iterator ThreadPool::begin()
  {
      return threads.begin();   // kein Lock!
  }
  ```
  `begin()` und `end()` greifen ohne jede Synchronisation auf `this->threads` zu und geben rohe `std::set::iterator`
  an den Aufrufer heraus. Modifiziert ein anderer Thread den Pool zeitgleich (`addThread`, `removeThread`, `clear`),
  liegt ein Data Race vor; bei `clear()` oder `destroyThread()` werden Iteratoren invalidiert (Undefined Behavior).
  Die Dokumentation schlägt zwar vor, den Pool vor Iteration manuell via `pool.lock()` und `pool.unlock()` abzusichern,
  aber:
  1. Vergisst der Aufrufer dies, gibt es keinerlei Schutz.
  2. Selbst mit `pool.lock()` bleibt der direkte Zugriff auf interne Container-Iteratoren fehleranfällig und bricht die Kapselung.
  Fix: Externe Iteratoren entfernen oder durch Thread-sichere Callbacks ersetzen (z.B. `forEach(std::function<void(Thread&)>)`),
  oder bei Beibehaltung einer Range-basierten API eine temporäre Snapshot-Kopie der Pointer unter Lock zurückgeben.

  ==> FIXED, Iteration erfolgt nun über `getThreads()` mit Snapshot-Kopie unter Lock. Ursprüngliche `begin()`/`end()`-Methoden wurden entfernt, ebenso `lock()`/`unlock()`.

- [ ] **`ThreadPool` ist unbeschränkt kopierbar → Flache Kopie des Pointers-Sets & Double-Free** (threadpool.h:35-64)
  `ThreadPool` deklariert weder Kopier-/Move-Konstruktor noch Zuweisungsoperatoren (`Rule of Three/Five` verletzt).
  Der Compiler generiert standardmäßig flache Kopien:
  ```cpp
  ThreadPool p1;
  p1.addThread(new MyThread());
  ThreadPool p2 = p1; // p2 hat dieselben rohen Thread*-Pointer
  // Bei Zerstörung beider Pools via destroyAllThreads() oder ~ThreadPool():
  // -> Double-Free der Thread-Instanzen!
  ```
  Zudem ist `pplib::Mutex` ohnehin non-copyable, sodass eine Zuweisung `p2 = p1;` Kompilierfehler erzeugen würde,
  die Kopierkonstruktion `ThreadPool p2(p1);` jedoch standardwidrig oder fehlerhaft ist.
  Fix:
  ```cpp
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;
  ```

  ==> FIXED

- [ ] **Deadlock-Gefahr / Ineffizienz in `stopThreads()` durch aktives Polling** (ThreadPool.cpp:226-234)
  ```cpp
  void ThreadPool::stopThreads()
  {
      signalStopThreads();
      while (running()) {
          MSleep(1);
      }
  }
  ```
  1. `stopThreads()` pollt mit 1ms-Sleeps, anstatt die Threads direkt zu joinen oder auf deren Beendigung zu warten.
  2. `running()` erwirbt bei jedem Schleifendurchlauf `MutexLock lock(mutex)` und iteriert über das gesamte Set.
  3. Da `Thread::threadJoin()` in der neuen `Thread`-Klasse sauber und thread-sicher blockiert, kann `stopThreads()`
     viel einfacher und deterministischer arbeiten: Nach `signalStopThreads()` wird über eine lokale Kopie der Threads
     iteriert und für jeden Thread `threadJoin()` aufgerufen – ganz ohne Polling, ohne CPU-Wakeups und ohne 1ms-Latenz.
  Fix:
  ```cpp
  void ThreadPool::stopThreads()
  {
      signalStopThreads();
      std::vector<Thread*> copy;
      {
          MutexLock lock(mutex);
          copy.assign(threads.begin(), threads.end());
      }
      for (auto* t : copy) {
          t->threadJoin();
      }
  }
  ```

  ==> FIXED, Beispiel hab ich so übernommen

## Bugs (mittel)

- [ ] **Inkonsistenter `nullptr`-Umgang bei `addThread()`, `removeThread()`, `destroyThread()`** (ThreadPool.cpp:73-109)
  Wird `addThread(nullptr)` aufgerufen, fügt `std::set` den Null-Pointer klaglos ein.
  Nachfolgende Aufrufe von `startThreads()`, `stopThreads()`, `running()` oder `destroyAllThreads()`
  dereferenzieren den Zeiger unweigerlich (`(*it)->threadStart()`) $\rightarrow$ Sofortiger Absturz durch Segfault / Access Violation.
  Fix:
  ```cpp
  if (!thread) throw NullPointerException("ThreadPool::addThread");
  ```
  Und in `removeThread()` / `destroyThread()` bei `nullptr` frühzeitig abbrechen (`if (!thread) return;`).

  ==> FIXED

- [ ] **Unnötig exponierter interner Mutex (`lock()`, `unlock()`)** (threadpool.h:61-62, ThreadPool.cpp:315-343)
  Der Aufrufer kann von außen `lock()` und `unlock()` aufrufen. Vergisst er das `unlock()`, ist der gesamte Pool
  permanent blockiert. Alle anderen Methoden (`addThread`, `count`, `clear`, etc.) sperren den Mutex bereits intern
  über `MutexLock`.
  Der einzige historische Grund für `lock()/unlock()` war die ungeschützte Iteration über `begin()`/`end()`.
  Fix: `lock()` und `unlock()` aus der öffentlichen Schnittstelle entfernen, sobald Iteration thread-sicher gekapselt ist.

  ==> FIXED, Iteration erfolgt nun über `getThreads()` mit Snapshot-Kopie unter Lock. Ursprüngliche `begin()`/`end()`-Methoden wurden entfernt, ebenso `lock()`/`unlock()`.

## Design

- [ ] **Unklare Besitzverhältnisse (Ownership): Rohzeiger `Thread*` vs. `destroyThread()` / `destroyAllThreads()`**
  Der Pool verwaltet rohe Zeiger (`Thread*`). Manche Methoden (`removeThread`, `clear`) löschen die Objekte nicht,
  andere (`destroyThread`, `destroyAllThreads`, `~ThreadPool`) rufen `delete` auf.
  Wenn Threads mit `threadDeleteOnExit(true)` konfiguriert sind, löschen sie sich beim Beenden selbst, während
  der Pool im Destruktor oder bei `destroyAllThreads()` denselben Zeiger nochmals `delete`n könnte (Use-After-Free / Double-Free).
  Fix: Entweder klar dokumentieren, dass Threads im Pool niemals `threadDeleteOnExit(true)` nutzen dürfen,
  oder Ownership modernisieren (z.B. `std::unique_ptr<Thread>` / `std::shared_ptr<Thread>`).

  ==> TODO, Da muss ich noch drüber nachdenken

- [ ] **Redundante Methoden: `size()` und `count()`** (ThreadPool.cpp:257-280)
  Beide Methoden machen exakt dasselbe (`return threads.size();`). `count()` ist verwirrend, da bei STL-Containern
  `count(key)` die Anzahl von Elementen zu einem Schlüssel prüft, nicht die Gesamtgröße.
  Fix: `count()` als `deprecated` markieren oder zugunsten des STL-konformen `size()` entfernen.

  ==> FIXED, Da wir eh einen Major-Versionssprung machen, nehme ich count() raus.

- [ ] **Fehlende `const`-Korrektheit & `noexcept`** (threadpool.h:46-60)
  Methoden wie `size()`, `count()`, `count_running()`, `running()` modifizieren den Pool logisch nicht,
  sind aber nicht `const` deklariert (weil der Mutex nicht `mutable` war).
  Fix: `mutable pplib::Mutex mutex;` und Lese-Methoden als `const` deklarieren.

  ==> FIXED, Mutex ist mutable, Lese-Methoden sind nun `const`.

## Doku / Kosmetik

- [ ] **Dokumentation im .cpp-File statt im Header**
  Sämtliche Doxygen-Kommentare liegen historisch in `src/core/ThreadPool.cpp`, während `include/pplib/core/threadpool.h`
  fast völlig undokumentiert ist.
  Fix: Doxygen-Blöcke mittels Migrationsskript in den Header `threadpool.h` überführen.

  ==> FIXED, die Doxygen-Kommentare wurden in den Header `threadpool.h` verschoben.

- [ ] **Veraltete Dokumentations-Angaben**
  - In `ThreadPool::lock` und `unlock` (ThreadPool.cpp:322-340) werden noch `DeadlockException`, `MutexLockingException`
    und `MutexNotLockedException` dokumentiert, obwohl `Mutex` diese Exceptions gar nicht mehr wirft.
  - Das Codebeispiel in `ThreadPool::begin()` nutzt veraltetes `printf` mit `%llu`.

  ===> FIXED, die Methoden wurden entfernt.

## Verifiziert OK (kein Handlungsbedarf)

- `threads.insert(thread)` wirft korrekt `ThreadAlreadyInPoolException`, wenn derselbe Thread doppelt hinzugefügt wird.
- Destruktor ruft brav `stopThreads()` vor `destroyAllThreads()` auf.
- Alle internen Modifikationen (`addThread`, `removeThread`, `clear`, etc.) sind durch `MutexLock` geschützt.
