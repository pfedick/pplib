# Thread Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/core/threads.h` + `src/core/Threads.cpp` (923 Zeilen).
Mitgelesen: `include/pplib/core/mutex.h` + `src/core/Mutex.cpp` (siehe `mutex-review.md` für die
Mutex-eigenen Befunde), da `Thread` intern ausschließlich über `Mutex threadmutex` synchronisiert und der
einzige Verwender von `Mutex::wait()`/`signal()` im gesamten Repo ist. Kurz gegengelesen: `threadpool.h`
(nur Kontext, nicht Teil des Auftrags).

Mehrere Befunde wurden empirisch gegen `build/coverage/libpplib_core.a` verifiziert (kompiliert mit g++,
`--coverage -pthread`, verlinkt gegen die vorhandene Library plus `libpcre2-8`/`libpcre2-32`). Ein
negativer/unauffälliger Lauf wäre dabei KEIN Korrektheitsbeweis gewesen – hier sind alle kritischen Befunde
jedoch positiv (d.h. reproduzierbar) bestätigt, nicht nur aus dem Code abgeleitet.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`threadSleep()`/`threadWaitSuspended()`: Selbstdeadlock legt den gesamten Thread inkl. `threadmutex` dauerhaft lahm** (Threads.cpp:663-682)
  ```cpp
  void Thread::threadSleep(int msec)
  {
      threadmutex.lock();      // sperrt den internen std::mutex
      IsSuspended = 1;
      threadmutex.wait(msec);  // versucht, DENSELBEN std::mutex erneut zu sperren -> Deadlock (s. mutex-review.md)
      IsSuspended = 0;
      threadmutex.unlock();
  }
  ```
  `threadWaitSuspended()` hat exakt dasselbe Muster (Zeile 665 `lock()`, Zeile 669 `wait()`). Da `Mutex::wait()`
  intern denselben `std::mutex` erneut sperrt, den `lock()` schon hält (siehe `mutex-review.md`, Bug 1), blockiert
  der aufrufende Thread für immer – und zwar, während `threadmutex` noch als gesperrt gilt. Jeder andere Aufruf,
  der `threadmutex` braucht (`threadStop()`, `threadIsRunning()`, `threadResume()`, `threadGetFlags()`,
  `threadShouldStop()`, `threadDeleteOnExit()`, …), blockiert danach ebenfalls für immer, nicht nur der
  betroffene Thread selbst.

  Empirisch bestätigt (Worker-Thread ruft `threadSleep(3000)` in `run()` auf, Hauptthread prüft danach
  `threadIsRunning()`):
  ```
  [worker] run() started, calling threadSleep(3000)
  ```
  → Prozess hängt, `timeout 10` killt ihn (Exit 124). Nicht einmal die für "0.5s später" geplante Ausgabe des
  Hauptthreads (`threadIsRunning()`) erscheint noch – bestätigt, dass nicht nur der Worker, sondern der
  gesamte `threadmutex`-geschützte Zustand des Objekts unerreichbar wird.

  Praktische Konsequenz: Jeder `Thread`, der `threadSleep()` oder `threadWaitSuspended()` aus `run()` heraus
  benutzt (die dokumentierte, vorgesehene Art, einen Thread pausierbar zu machen – vgl. `Thread::threadSuspend()`-Doku),
  bricht die Klasse dauerhaft.

  Fix: siehe `mutex-review.md`. Zusätzlich hier zu prüfen, ob nach einem Fix von `Mutex::wait()` das
  Lock/Wait/Unlock-Muster in `threadSleep()`/`threadWaitSuspended()` überhaupt noch passt oder angepasst werden muss.

- [ ] **`threadStop()`/`threadSignalStop()`: `signal()` wird aufgerufen, während `threadmutex` noch gehalten ist → derselbe Selbstdeadlock-Mechanismus** (Threads.cpp:351-366, 376-386)
  ```cpp
  void Thread::threadStop()
  {
      threadmutex.lock();
      flags |= 1;
      if (IsSuspended) {
          threadmutex.signal();   // <- Lock noch gehalten! signal() versucht denselben Mutex erneut zu sperren
      }
      while (IsRunning) {
          threadmutex.unlock();
          MSleep(1);
          threadmutex.lock();
      }
      flags = flags & 0xfffffffe;
      threadmutex.unlock();
  }
  ```
  ```cpp
  void Thread::threadSignalStop()
  {
      threadmutex.lock();
      flags |= 1;
      if (IsSuspended) {
          threadmutex.signal();   // <- dasselbe Problem
      } else {
          threadmutex.unlock();
      }
  }
  ```
  Beide Funktionen halten `threadmutex` (per `lock()`) und rufen dann `threadmutex.signal()` auf, das intern
  denselben `std::mutex` noch einmal sperrt (`mutex-review.md`, Bug 1). Das ist ein eigenständiger Bug,
  unabhängig vom `wait()`-Problem oben: selbst wenn `Mutex::wait()` korrigiert wird, deadlocken diese beiden
  Funktionen weiterhin, sobald sie auf einen aktuell suspendierten Thread treffen.

  Aktuell praktisch nicht beobachtbar, weil `IsSuspended` nur innerhalb von `threadWaitSuspended()`/
  `threadSleep()` gesetzt wird – und die sind wegen des Bugs oben ohnehin schon permanent blockiert, sobald
  sie `IsSuspended = 1` gesetzt haben, sodass `threadmutex` für keinen anderen Aufrufer mehr erreichbar ist.
  Nach Behebung des `wait()`-Bugs wird dieser Deadlock hier aber sofort sichtbar.

  Der Vergleich mit `threadResume()` zwei Funktionen weiter unten zeigt, dass die korrekte Reihenfolge im
  selben File bereits bekannt ist:
  ```cpp
  void Thread::threadResume()
  {
      threadmutex.lock();
      flags = flags & ~2;
      if (IsSuspended) {
          threadmutex.unlock();     // <- unlock() VOR signal()
          threadmutex.signal();
      } else {
          threadmutex.unlock();
      }
  }
  ```
  Fix: In `threadStop()` und `threadSignalStop()` `threadmutex.unlock()` vor `threadmutex.signal()` aufrufen,
  analog zu `threadResume()`.

- [ ] **`threadSignalStop()`: fehlendes `unlock()` im `IsSuspended`-Zweig – Mutex bleibt gesperrt** (Threads.cpp:376-386)
  ```cpp
  void Thread::threadSignalStop()
  {
      threadmutex.lock();
      flags |= 1;
      if (IsSuspended) {
          threadmutex.signal();
      } else {
          threadmutex.unlock();
      }
  }
  ```
  Unabhängig vom Deadlock-Bug direkt darüber: Selbst wenn `signal()` normal zurückkehren würde, gibt es im
  `if`-Zweig **kein** `threadmutex.unlock()` – die Funktion verlässt den kritischen Abschnitt nur im
  `else`-Zweig korrekt. Klassisches Symptom von manuellem `lock()`/`unlock()` statt RAII/Scope-Guard: der
  zusätzliche früh divergierende Pfad wurde beim Schreiben schlicht vergessen.
  Fix: `unlock()` für beide Zweige sicherstellen, z.B.
  ```cpp
  threadmutex.lock();
  flags |= 1;
  bool wasSuspended = IsSuspended;
  threadmutex.unlock();
  if (wasSuspended) threadmutex.signal();
  ```

- [ ] **Exceptions aus `run()` (bzw. der freien Callback-Funktion) entkommen ungefangen aus der Thread-Einsprungfunktion → `std::terminate()` beendet den kompletten Prozess** (Threads.cpp:99-133, 517-531)
  ```cpp
  static void* ThreadProc(void* param)
  {
      THREADSTARTUP* ts = (THREADSTARTUP*)param;
      ...
      if (ts->threadClass) {
          ts->td->thread = local_thread_data.thread;
          ts->td->threadId = local_thread_data.threadId;
          ts->threadClass->threadStartUp();   // <- kein try/catch
          if (ts->threadClass->threadShouldDeleteOnExit()) delete ts->threadClass;
      } else {
          ts->threadFunction(ts->data);        // <- ebenfalls kein try/catch
      }
      ...
  }
  ```
  ```cpp
  void Thread::threadStartUp()
  {
      threadmutex.lock();
      runcount++;
      IsRunning = 1;
      IsSuspended = 0;
      threadmutex.unlock();
      threadSetPriority(myPriority);
      run();                 // <- virtuelle Methode der abgeleiteten Klasse, kein try/catch
      threadmutex.lock();
      flags = 0;
      IsRunning = 0;
      IsSuspended = 0;
      threadmutex.unlock();
  }
  ```
  Wirft eine überschriebene `run()` (oder die an `StartThread()` übergebene Callback-Funktion) eine Exception,
  läuft diese ungefangen bis zur Thread-Einsprungfunktion (`ThreadProc`), die als C-Callback an
  `pthread_create` übergeben wird. Eine C++-Exception, die aus einer solchen Funktion entkommt, führt zu
  `std::terminate()` – das beendet **den gesamten Prozess**, nicht nur den betroffenen Thread. Zusätzlich wird
  wegen des fehlenden Try/Catch der Aufräum-Epilog (`flags=0; IsRunning=0; IsSuspended=0; unlock()`) nie
  erreicht.

  Empirisch bestätigt: `run()` wirft `std::runtime_error`, Ausgabe:
  ```
  terminate called after throwing an instance of 'std::runtime_error'
    what():  boom from run()
  ```
  Prozess bricht mit `SIGABRT` ab (Exit 134) – der Hauptthread wird mit in den Tod gerissen, obwohl er selbst
  keinen Fehler hatte.

  Fix: `run()`-Aufruf (und den freien Callback-Pfad) in `try { ... } catch (...) { ... }` einbetten, damit
  wenigstens der Aufräum-Epilog garantiert läuft (idealerweise per RAII-Guard statt manuellem
  lock/unlock-Paar, siehe auch Bug oben zu `threadSignalStop()`). Ob die Exception geloggt, verschluckt oder
  über einen `std::exception_ptr` dem Aufrufer von `threadJoin()`/`threadStop()` zugänglich gemacht wird, ist
  eine Design-Entscheidung – ungefangen entkommen darf sie in keinem Fall.

- [ ] **`threadJoin()`: `throw` fehlt in zwei `switch`-Zweigen → falscher Exception-Typ durch Fall-Through** (Threads.cpp:906-915)
  ```cpp
  int ret = pthread_join(t->thread, NULL);
  if (ret != 0) {
      switch (ret) {
      case EDEADLK:
          throw DeadlockException();
      case EINVAL:
          ThreadOperationFailedException("Thread is not joinable");   // <- kein throw, kein break!
      case ESRCH:
          ThreadOperationFailedException("Thread not found");         // <- kein throw, kein break!
      default:
          pplib::throwExceptionFromErrno(ret, "Thread is not joinable");
      }
  }
  ```
  Die beiden `ThreadOperationFailedException(...)`-Aufrufe konstruieren nur ein temporäres Objekt, das sofort
  wieder verworfen wird – kein `throw`. Ohne `break` fällt die Ausführung anschließend durch bis `default:`,
  das über `throwExceptionFromErrno()` eine andere Exception wirft, die zufällig zum `errno`-Wert passt.

  Empirisch bestätigt: `threadJoin()` auf einem per `threadStart()` gestarteten (und damit bereits
  detachten, siehe nächster Befund) Thread liefert `pthread_join()` → `EINVAL` zurück; tatsächlich geworfen
  wird aber `pplib::InvalidArgumentsException`, nicht die im Code sichtbar beabsichtigte
  `ThreadOperationFailedException("Thread is not joinable")`.
  Fix: `throw` vor beide `ThreadOperationFailedException(...)`-Aufrufe setzen (und der Klarheit halber
  zusätzlich `break;`, auch wenn nach einem unbedingten `throw` unerreichbar).

- [ ] **`threadJoin()` ist mit dem restlichen Lifetime-Modell inkompatibel: `threadStart()`/`StartThread()` detachen den Thread immer sofort** (Threads.cpp:162-168, 430-435, 894-920)
  ```cpp
  // StartThread():
  int ret = pthread_create(&t->thread, &t->attr, ThreadProc, ts);
  if (ret == 0) {
      pthread_detach(t->thread);   // <- immer, kein Ausweg
      return t->threadId;
  }
  ```
  ```cpp
  // Thread::threadStart():
  int ret = pthread_create(&t->thread, &t->attr, ThreadProc, ts);
  if (ret == 0) {
      pthread_detach(t->thread);   // <- ebenfalls immer
      return;
  }
  ```
  Jeder erfolgreiche Thread-Start detacht den Thread sofort. Laut POSIX ist `pthread_join()` auf einem
  bereits detachten Thread undefiniertes Verhalten; auf glibc liefert es `EINVAL`. Die öffentlich exponierte
  `Thread::threadJoin()`-Methode kann nach dem normalen Startpfad also **nie** erfolgreich funktionieren.

  Empirisch bestätigt: `threadStart()` gefolgt von `threadJoin()` wirft (wegen des Bugs oben:
  `InvalidArgumentsException`, semantisch aber ein `EINVAL` von `pthread_join()` auf einem detachten Thread).

  Fix: Design-Entscheidung nötig – entweder (a) `threadStart()`/`StartThread()` detachen nicht mehr
  automatisch und überlassen es dem Aufrufer, `threadJoin()` oder "detach and forget" zu wählen (erfordert
  Anpassung der Stop-Logik, die aktuell nicht auf `join()` sondern auf Polling von `IsRunning` setzt), oder
  (b) `threadJoin()` wird als nicht unterstützt dokumentiert/entfernt, da das Modell konsequent "detach-only"
  ist und `threadStop()` die einzige vorgesehene Synchronisationsmethode bleibt.

## Bugs (mittel)

- [ ] **Datenrace auf `myPriority`** (Threads.cpp:774-778, 524)
  ```cpp
  int Thread::threadSetPriority(int priority)
  {
      THREADDATA* t = (THREADDATA*)threaddata;
      myPriority = priority;     // <- kein threadmutex-Schutz
      ...
  ```
  Alle anderen veränderlichen Member (`flags`, `IsRunning`, `IsSuspended`, `deleteMe`, s. "Verifiziert OK")
  werden konsequent unter `threadmutex` gelesen/geschrieben – `myPriority` ist die einzige Ausnahme. Die
  Funktion wird sowohl von außen (öffentliche API, potenziell aus einem fremden Thread) als auch intern vom
  Worker-Thread selbst aufgerufen (`threadStartUp()`, Zeile 524: `threadSetPriority(myPriority);`). Ruft ein
  externer Thread `threadSetPriority()` auf, während der Worker gerade startet, liegt ein unsynchronisierter
  gleichzeitiger Schreib-/Lesezugriff auf ein `int` vor – Data Race nach C++-Speichermodell (UB), keine
  Sichtbarkeitsgarantie für den jeweils anderen Thread.
  Fix: Zugriff auf `myPriority` unter `threadmutex` kapseln, oder `std::atomic<int>` verwenden.

- [ ] **Datenrace auf `runcount`: Schreiben unter Lock, Lesen ohne Lock** (Threads.cpp:520, 642-645)
  ```cpp
  size_t Thread::threadRunCount()
  {
      return runcount;   // kein threadmutex.lock()
  }
  ```
  `threadStartUp()` erhöht `runcount++` korrekt unter `threadmutex` (Zeile 520), `threadRunCount()` liest den
  Wert aber ohne jede Synchronisation. Analoge Funktionen wie `threadIsRunning()`, `threadGetFlags()` machen
  es konsequent richtig (lock/read/unlock).
  Fix:
  ```cpp
  size_t Thread::threadRunCount()
  {
      threadmutex.lock();
      size_t ret = runcount;
      threadmutex.unlock();
      return ret;
  }
  ```

- [ ] **`THREADDATA::thread` wird von zwei Threads unsynchronisiert beschrieben** (Threads.cpp:106-119, 162, 430)
  ```cpp
  // im aufrufenden Thread (StartThread()/threadStart()):
  int ret = pthread_create(&t->thread, &t->attr, ThreadProc, ts);   // schreibt t->thread als Out-Parameter
  ```
  ```cpp
  // im NEUEN Thread (ThreadProc()):
  local_thread_data.thread = pthread_self();
  ...
  ts->td->thread = local_thread_data.thread;    // schreibt dasselbe Feld ein zweites Mal
  ```
  Zwei verschiedene Threads schreiben ohne jede Synchronisation auf dasselbe, nicht-atomare `THREADDATA::thread`-Feld.
  Auch wenn faktisch derselbe Wert geschrieben wird, ist das nach C++-Speichermodell ein Data Race (zwei
  konfligierende, unsynchronisierte Zugriffe, mindestens einer schreibend). Zusätzlich lesen
  `threadJoin()`, `threadGetPriority()`, `threadSetPriority()` und `threadSetName()` dieses Feld von
  potenziell dritten Threads aus, ebenfalls ohne Synchronisation mit dem schreibenden Worker-Thread.
  Fix: Den redundanten Schreibzugriff in `ThreadProc()` entfernen (`t->thread` steht nach erfolgreichem
  `pthread_create()` bereits fest) oder zumindest über `threadmutex` absichern.

- [ ] **`Thread::~Thread()` gibt `threaddata` nie frei – Speicherleck pro Instanz** (Threads.cpp:315-323, 292-296)
  ```cpp
  Thread::Thread()
  {
      threaddata = new THREADDATA;
      ...
  }

  Thread::~Thread()
  {
      threadStop();
  #ifdef HAVE_PTHREADS
      if (threaddata) {
          pthread_attr_destroy(&((THREADDATA*)threaddata)->attr);
      }
  #endif
      // kein delete (THREADDATA*)threaddata;
  }
  ```
  Fix: nach `pthread_attr_destroy()` zusätzlich `delete (THREADDATA*)threaddata; threaddata = nullptr;`.

- [ ] **`GetCurrentProcess()` statt `GetCurrentThread()` in der freien Funktion `ThreadGetPriority()` (Windows)** (Threads.cpp:248-249)
  ```cpp
  Thread::Priority ThreadGetPriority()
  {
  #ifdef _WIN32
      HANDLE h = GetCurrentProcess();     // <- falsch
      int p = GetThreadPriority(h);
  ```
  Das Pendant `ThreadSetPriority()` wenige Zeilen darüber macht es richtig:
  ```cpp
  void ThreadSetPriority(Thread::Priority priority)
  {
  #ifdef _WIN32
      HANDLE h = GetCurrentThread();      // <- korrekt
  ```
  Klassischer Copy-Paste-Fehler; `GetThreadPriority()` mit einem Prozess- statt Thread-Handle liefert ein
  falsches/undefiniertes Ergebnis. Nur unter `_WIN32` aktiv, in dieser (Linux-)Umgebung nicht compile- bzw.
  laufzeitgetestet, aber im Quelltext eindeutig erkennbar.
  Fix: `GetCurrentThread()`.

## Design

- [ ] **`Thread` ist trotz `Mutex`-Member und Rohzeiger `threaddata` nicht explizit non-copyable** (threads.h:51-102)
  Kein Kopier-/Move-Konstruktor bzw. -Zuweisungsoperator deklariert. Kompiliert anstandslos:
  ```cpp
  class MyThread : public Thread {};
  MyThread a;
  MyThread b = a;   // kompiliert
  ```
  Erbt damit nicht nur das Double-Free-Problem von `Mutex` (siehe `mutex-review.md`, Bug 2), sondern legt mit
  dem flach kopierten `threaddata`-Zeiger eine zweite, unabhängige Double-Free-/Use-after-free-Quelle an
  (zwei `Thread`-Objekte, die sich dasselbe `THREADDATA` teilen, während beide potenziell `pthread_join`/
  `pthread_attr_destroy` darauf aufrufen).
  Fix: `Thread(const Thread&) = delete; Thread& operator=(const Thread&) = delete;` (und Move analog, sofern
  kein Ownership-Transfer explizit gewünscht/implementiert ist).

- [ ] **`threadStop()` pollt mit 1ms-Sleep statt korrekt zu signalisieren** (Threads.cpp:359-363)
  ```cpp
  while (IsRunning) {
      threadmutex.unlock();
      MSleep(1);
      threadmutex.lock();
  }
  ```
  Aktiv-Warten mit Sleep(1) statt einer korrekten Condition-Variable-Benachrichtigung, sobald `run()` beendet
  ist. Funktional nicht falsch (nur unter Lock ausgewertet, s. "Verifiziert OK"), aber unnötig ineffizient
  (Latenz bis zu 1ms, ständiges Lock/Unlock) und exakt die Stelle, die von einem korrekt funktionierenden
  `Mutex::wait()`/`signal()` profitieren würde – hängt also mit den Bugs oben zusammen und sollte im selben
  Aufwasch überarbeitet werden.

- [ ] **Detach-only-Lifetime vs. öffentliches `threadJoin()`**: siehe Bug oben ("threadJoin() ist mit dem
  restlichen Lifetime-Modell inkompatibel") – strukturelle Design-Entscheidung, kein reiner Implementierungsfehler.

- [ ] **Globale `Win32ThreadMutex`/`Win32ThreadTLS` sind deklariert, aber nirgends verwendet** (Threads.cpp:61-64)
  ```cpp
  #ifdef _WIN32
  Mutex Win32ThreadMutex;
  DWORD Win32ThreadTLS = TLS_OUT_OF_INDEXES;
  #endif
  ```
  Keine weitere Referenz im gesamten File. Totes Legacy-Konstrukt, das unter Windows dennoch bei
  Programmstart eine zusätzliche `Mutex`-Instanz (inkl. Heap-Allokation, siehe `mutex-review.md`) anlegt.
  Kandidat zum Entfernen im Zuge der Aufräumarbeiten.

- [ ] **Statische Initialisierungsreihenfolge von `GlobalThreadMutex`** (Threads.cpp:59)
  `static Mutex GlobalThreadMutex;` ist ein globales Objekt mit nicht-trivialem Konstruktor. Wird
  `StartThread()`/`ThreadID()`/`Thread::threadStart()` aus dem Konstruktor eines globalen Objekts einer
  *anderen* Übersetzungseinheit heraus aufgerufen, bevor diese TU initialisiert wurde ("static initialization
  order fiasco"), ist das UB. Eher theoretisches Risiko für eine Bibliothek mit vielen globalen `Mutex`-Instanzen
  (`__OpenSSLGlobalMutex`, `InitMutex` u.a. wurden per `grep` gefunden), aber erwähnenswert.

## Doku / Kosmetik

- [ ] **`Thread::Thread()`: toter Null-Check nach `new` ohne `nothrow`** (Threads.cpp:294-295)
  ```cpp
  threaddata = new THREADDATA;
  if (!threaddata) throw OutOfMemoryException();
  ```
  `new` (ohne `std::nothrow`) wirft bei Fehlschlag `std::bad_alloc`, liefert aber niemals `nullptr` – der
  Check danach ist unerreichbarer Code. `Mutex::Mutex()` macht es an vergleichbarer Stelle richtig
  (`new (std::nothrow) PPLMUTEX()` + expliziter Check, siehe `mutex-review.md`, "Verifiziert OK").
  Fix: entweder `new (std::nothrow) THREADDATA` verwenden (dann ist der Check sinnvoll) oder den toten Check
  entfernen und sich auf die automatisch propagierte `std::bad_alloc` verlassen (ggf. in `OutOfMemoryException`
  umwandeln, falls das die einheitliche Exception-Konvention der Bibliothek ist).

- [ ] **`new THREADDATA` gefolgt von `memset(..., 0, sizeof(THREADDATA))`** (Threads.cpp:142+147, 294+296)
  `THREADDATA` hat mit `uint64_t threadId = 0;` einen In-Class-Initializer und ist damit kein trivialer Typ
  mehr im Sinne des Standards. Ein `memset` auf ein bereits per `new` konstruiertes Objekt ist bei strikter
  Lesart fragwürdig – praktisch harmlos, da alle übrigen Member (`pthread_t`, `HANDLE`, `DWORD`) trivial
  kopierbare PODs sind, aber ein Stilbruch, der bei künftigen Erweiterungen von `THREADDATA` (z.B. um ein
  Objekt mit nicht-trivialem Konstruktor) zu echten Problemen führen könnte.

- [ ] **`threadSetStackSize()`: totes `return 0;` nach `throw`** (Threads.cpp:849-851)
  ```cpp
  if (size < (size_t)PTHREAD_STACK_MIN) {
      throw IllegalArgumentException("Stacksize must not be smaller than %u Bytes", PTHREAD_STACK_MIN);
      return 0;    // unerreichbar
  }
  ```

## Verifiziert OK (kein Handlungsbedarf)

- `flags`, `IsRunning`, `IsSuspended`, `deleteMe` werden über die gesamte Datei hinweg konsistent unter
  `threadmutex` gelesen/geschrieben (Ausnahmen sind ausschließlich `myPriority` und `runcount`, s. Bugs oben).
- `threadResume()` macht die Lock/Signal-Reihenfolge richtig (`unlock()` vor `signal()`, Zeilen 499-500) –
  Gegenbeispiel, das zeigt, dass die korrekte Reihenfolge dem Autor grundsätzlich bekannt ist; die anderen
  Funktionen (`threadStop()`, `threadSignalStop()`) weichen davon ab.
- `threadStartUp()` hält `threadmutex` nicht während der potenziell langen Laufzeit von `run()` – Lock wird
  vor dem Aufruf freigegeben und erst danach wieder erworben, kein unnötig langes Halten des Locks.
- `global_thread_id` ist an allen Schreibstellen (`ThreadID()`, `ThreadProc()`, `StartThread()`,
  `Thread::threadStart()`) konsistent durch `GlobalThreadMutex` geschützt.
- `THREADDATA::threadId` wird – anders als `THREADDATA::thread` – vor dem `pthread_create()`-Aufruf im
  aufrufenden Thread unter `GlobalThreadMutex` gesetzt; dafür besteht kein Race (nur `::thread` ist betroffen,
  siehe Bug oben).
- `thread_local THREADDATA local_thread_data` ist korrekt pro Thread isoliert, keine versehentliche
  Cross-Thread-Sichtbarkeit dieses Zustands.
