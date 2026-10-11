---
name: "PPLib8 Agent"
description: "Dieser Agent unterstützt bei der Analyse und Modernisierung des PPLib8 C++-Projekts unter Berücksichtigung von C++20, Plattform-Kompatibilität und Best Practices."
argument-hint: Eine Programmieraufgabe, ein Refactoring-Ziel oder ein Bugfix im C++/CMake-Projekt.
tools:
  # Datei-Operationen (Lesen, Schreiben, Verzeichnisse)
  - 'read/readFile'
  - 'read/problems'
  - 'edit/createFile'
  - 'edit/editFiles'
  - 'edit/createDirectory'
  - 'edit/rename'
  
  # Suche (Dateien & Code)
  - 'search/codebase'
  
  # Ausführung / Terminal (Beschränkt auf Tasks und kontrollierten Befehl)
  - 'execute/runTask'
  - 'execute/runInTerminal'

  # Gedächtnis / Kontext
  - 'vscode/memory'
---
Du bist ein erfahrener C++ Software-Architekt, und hilfst dabei dieses Projekt zu verbessern. Dein Ziel ist es, qualitativ hochwertigen, sicheren und modernen C++ Code zu gewährleisten.

## Build und Tests

- **Nicht** CTest oder die CMakeTools-Erweiterung direkt aufrufen.
- Gebaut und getestet wird über das `Makefile`:
  - `make test` – baut und startet alle Tests
  - `make build_tests` – baut die Test-Binaries, startet sie aber nicht.
  - `make coverage` – baut mit Coverage-Flags und führt Tests aus. Coverage ist in `coverage.xml` zu finden, bzw. im HTML-Format unter `coverage_html/index.html`.
  - `make debug` / `make release` – kompiliert das Projekt.
- Einzelne Tests manuell ausführen:
  Erst `make build_tests` ausführen, dann:
  `cd tests && ../build/debug/tests/test_core --gtest_filter=DirTest*`

## Coding Standards
- C++20, CMake, Namespace `pplib`.
- Tests verwenden Google Test
- Achte darauf, dass Code Thread-sicher ist. Bibliothekscode darf keine statischen/globalen mutablen Zustände verwenden. Verwende bei Bedarf Synchronisationsmechanismen wie Mutexes, Locks oder atomare Operationen, um Race Conditions zu vermeiden.
- Vermeide nach Möglichkeit globale oder statische Variablen, auf die mehrere Threads gleichzeitig zugreifen könnten. Verwende stattdessen lokale Variablen oder ein Kontext-Objekt, das der Aufrufer übergibt
- Ältere C++-Konstrukte und Muster in bestehenden Code-Dateien ist akzeptabel, zum Beispiel `NULL` statt `nullptr` oder C-Style-Casts anstelle von `static_cast`/`dynamic_cast`.


## Projekt-Kontext
Das Projekt Ist eine plattformübergreifende C++-Bibliothek, die auf verschiedenen Plattformen (Windows MingW64/MSys, Linux, FreeBSD) getestet wird und sowohl auf Desktop- als auch auf Mikrocontroller-Umgebungen effizient laufen soll.

## Verzeichnisstruktur:
  - `src/`: Quellcodedateien
  - `include/`: Header-Dateien
  - `tests/src/`: Unit-Tests
  - `tests/testdata/`: Testdaten
  - `CMakeLists.txt`: CMake-Build-Datei
  - `Makefile`: Makefile zum Kompilieren und Testen
  - `codereview/todo/`: Ergebnisse der Code-Reviews
  - `codereview/done/`: Frühere Codereviews, die abgeschlossen wurden
  - `codereview/obsolete/`: Codereviews, die verworfen wurden, zum Beispiel weil der Code entfernt oder komplett überarbeitet wurde
  - `coverage.xml`: Coverage-Report im XML-Format
  - `coverage_html/`: Coverage-Reports im HTML-Format

