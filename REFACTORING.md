# Refactoring von PPLIB aka PPLIB

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


## autoconf / makefile vs. CMake
Das Makefile.in wird im Moment per Script generiert, welches die Dateien in den Verzeichnissen sucht und passende Header als Abhängigkeit einträgt.

- Gibt es Möglichkeiten das Makefile automatisch zu generieren?
- Abhängigkeit zu den jeweiligen Header-Dateien automatisch finden?
- Manuell pflegen? So oft kommen keine neuen Dateien hinzu
- Würden sich die Probleme von selbst erledigen, wenn make die Dateien selbst findet? (z.B. mit wildcards)

Entscheidung: Ich probiere mal CMake aus.

Vielleicht können wir auch beide Varianten anbieten.

 
## pico-pplib
Siehe: [https://](https://github.com/pfedick/pico-pplib)

"picopplib" enthält ein Subset von Klassen und Funktionen aus PPLIB und ist für Microcontroller optimiert, und zwar in dem Sinne, dass weniger RAM benötigt wird. Viele Objekte verwenden daher uint16_t anstelle von int oder es gibt duplikate von Klassen, die 16-Bit Integer verwenden.

Der Code ist inzwischen auseinander gelaufen, so dass es schwierig ist, Änderungen in beiden Codebasen zu pflegen. Daher ist es sinnvoll, die picopplib-Version in PPLIB zu integrieren und die Unterschiede über "#ifdef"s zu steuern.

In pico-pplib gibt es auch Hardware-spezifische Klassen/Treiber, die in PPLIB nicht benötigt werden. Hierfür wäre ein eigenes Projekt sinnvoll, das die Hardware-spezifischen Klassen enthält und von PPLIB abhängig ist.

## Namespace pplib, ppl8 oder pplib?
Der Aufwand alle Programme, die PPLIB verwenden, auf PPL8 zu portieren wäre sehr hoch. Allerdings sind die Überarbeitungen schon recht umfangreich, so dass ein größerer Versionssprung gerechtfertigt wäre. Die Interfaces bleiben weitgehend kompatibel.

Da das GitHub-Repository "pplib" heißt, und ich auch die Domain "pplib.de" besitze, wäre es sinnvoll auf die Version im Namespace zu verzichten und stattdessen den neutralen Namespace "pplib" zu verwenden. "ppl" kollidiert bereits mit dem Namespace "ppl" von Microsoft, der in Visual Studio verwendet wird.

Für die Portierung vorhandener Projekte könnte ein Alias "pplib" definiert werden, der auf den Namespace "pplib" verweist:
```cpp
namespace pplib = pplib;
```


## Fragen
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


- Makefile überarbeiten

## NEU
- HttpRequest, HttpResponse, HttpClient

## Erledigt
- Umstellung von Autoconf auf CMake
- Dir-Klasse komplett überarbeitet
- DirEntry-Klasse überarbeitet

### Datenobjekte (Types)
- Variant Klasse refakturiert und geprüft
- Pointer Klasse refakturiert und geprüft => als obsolete markiert
- ByteArray Klasse refakturiert und geprüft
- ByteArrayPtr Klasse refakturiert und geprüft
- String Klasse refakturiert und geprüft
- WideString Klasse refakturiert und geprüft






