# pplib
Das Projekt "pplib", hier version 8, ist eine plattformübergreifende C++-Bibliothek, die auf verschiedenen Plattformen (Windows MingW64/MSys, Linux, FreeBSD) getestet wird und sowohl auf Desktop- als auch auf Mikrocontroller-Umgebungen effizient laufen soll. Nur ein Subset davon wird auf Mikrocontrollern verwendet, und hier aktuell auch nur Raspberry Pico 2 (siehe Define `PICO_BUILD`).

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
- Tests verwenden Google Test

## Coding Standards
- **Pragmatisches C++20:** Schreibe geradlinigen, wartbaren Code. Passe dich strikt dem vorhandenen Stil der Datei an. Keine verschachtelten Ranges-Pipelines oder Template-Metaprogrammierung, wenn einfache Schleifen und Funktionen genügen.
- Achte darauf, dass Code Thread-sicher ist. Bibliothekscode darf keine statischen/globalen mutablen Zustände verwenden. Verwende bei Bedarf Synchronisationsmechanismen wie Mutexes, Locks oder atomare Operationen, um Race Conditions zu vermeiden.
- Vermeide nach Möglichkeit globale oder statische Variablen, auf die mehrere Threads gleichzeitig zugreifen könnten. Verwende stattdessen lokale Variablen oder ein Kontext-Objekt, das der Aufrufer übergibt
- Ältere C++-Konstrukte und Muster in bestehenden Code-Dateien ist akzeptabel, zum Beispiel `NULL` statt `nullptr` oder C-Style-Casts anstelle von `static_cast`/`dynamic_cast`.



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

