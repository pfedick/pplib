# Refactoring von PPLIB aka PPL7

PPLIB steht für "Patrick's Programming Library" und ist eine C++-Bibliothek, die ich seit vielen Jahren weiter entwickele. Sie enthält viele nützliche Klassen und Funktionen, die ich in meinen Projekten immer wieder verwendet habe. Die Bibliothek ist inzwischen sehr umfangreich geworden und enthält einige Klassen, die nicht mehr zeitgemäß sind oder die ich heute anders implementieren würde.

Einige Klassen sind sehr gut dokumentiert und getestet, andere wiederum eher weniger.



## Ziele
- Nicht zu viel kaputt machen, außer es ist wirklich sinnvoll!
- Header aufteilen: jede Klasse in eine eigene Header-Datei, die dann in der Haupt-Header-Datei inkludiert wird
- Alle Klassen in den Header-Dateien dokumentieren, damit man Beispielsweise in VSCode die Dokumentation direkt sehen kann
- Alle "#ifdef"s reduzieren, nur da wo unbedingt nötig
- Modernisierung der Klassen
- weniger Exceptions
- Code ist vollständig mit KI auf Fehler geprüft
- Testabdeckung erhöhen
- Alle Tests gehen von UTF-8 als locale aus, auch unter Windows
- pico-pplib integrieren (siehe eigenes Kapitel)?
- C++17 oder C++20 als Standard?
- CMake statt autoconf/makefile verwenden?
- configure abspecken: wir brauchen einen Großteil der Checks auf Header und Funktion nicht, das diese laut Standard in allen modernen Compilern vorhanden sind. Wir brauchen nur noch die Checks auf C++17/C++20, sowie die Checks auf die Plattform (Windows, Linux, FreeBSD)


## autoconf / makefile
Das Makefile.in wird im Moment per Script generiert, welches die Dateien in den Verzeichnissen sucht und passende Header als Abhängigkeit einträgt.

- Gibt es Möglichkeiten das Makefile automatisch zu generieren?
- Abhängigkeit zu den jeweiligen Header-Dateien automatisch finden?
- Manuell pflegen? So oft kommen keine neuen Dateien hinzu
- Würden sich die Probleme von selbst erledigen, wenn make die Dateien selbst findet? (z.B. mit wildcards)

 
## pico-pplib
Siehe: [https://](https://github.com/pfedick/pico-pplib)

"picopplib" enthält ein Subset von Klassen und Funktionen aus PPL7 und ist für Microcontroller optimiert, und zwar in dem Sinne, dass weniger RAM benötigt wird. Viele Objekte verwenden daher uint16_t anstelle von int oder es gibt duplikate von Klassen, die 16-Bit Integer verwenden.

Der Code ist inzwischen auseinander gelaufen, so dass es schwierig ist, Änderungen in beiden Codebasen zu pflegen. Daher ist es sinnvoll, die picopplib-Version in PPL7 zu integrieren und die Unterschiede über "#ifdef"s zu steuern.

In pico-pplib gibt es auch Hardware-spezifische Klassen/Treiber, die in PPL7 nicht benötigt werden. Hierfür wäre ein eigenes Projekt sinnvoll, das die Hardware-spezifischen Klassen enthält und von PPL7 abhängig ist.

## Fragen
- Sollen wir auf PPL8 erhöhen? Der Aufwand alle Programme, die ppl7 verwenden, auf PPL8 zu portieren wäre sehr hoch.
- Sollen wir die Library aufsplitten in mehrere kleinere Module?
  - Core
  - Grafix
  - Audio
  - Crypto
  - Internet


# Fortschritt
## TODO
- Array
- AssocArray
- DateTime

- Funktionen
- File (viele fehlschlagende Unittests)
- Dir

- Makefile überarbeiten

## NEU
- HttpRequest, HttpResponse, HttpClient

## Erledigt
- Umstellung von Autoconf auf CMake

### Datenobjekte (Types)
- Variant Klasse refakturiert und geprüft
- Pointer Klasse refakturiert und geprüft => als obsolete markiert
- ByteArray Klasse refakturiert und geprüft
- ByteArrayPtr Klasse refakturiert und geprüft
- String Klasse refakturiert und geprüft
- WideString Klasse refakturiert und geprüft




# Auftrag an die KI

