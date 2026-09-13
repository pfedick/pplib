# Refactoring von PPL7 aka PPLIB

PPLIB steht für "Patrick's Programming Library" und ist eine C++-Bibliothek, die ich seit vielen Jahren weiter entwickele. Sie enthält viele nützliche Klassen und Funktionen, die ich in meinen Projekten immer wieder verwendet habe. Die Bibliothek ist inzwischen sehr umfangreich geworden und enthält einige Klassen, die nicht mehr zeitgemäß sind oder die ich heute anders implementieren würde.

Einige Klassen sind sehr gut dokumentiert und getestet, andere wiederum eher weniger.

## Ziele
- Nicht zu viel kaputt machen, außer es ist wirklich sinnvoll!
- Header aufteilen: jede Klasse in eine eigene Header-Datei, die dann in der Haupt-Header-Datei inkludiert wird
- Alle Klassen in den Header-Dateien dokumentieren, damit man Beispielsweise in VSCode die Dokumentation direkt sehen kann
- Alle "#ifdef"s reduzieren, nur da wo unbedingt nötig
- Modernisierung der Klassen
- weniger Exceptions
- Code mit KI auf Fehler prüfen (Codereviews)
- Testabdeckung erhöhen
- Alle Tests gehen von UTF-8 als locale aus, auch unter Windows
- pico-pplib integrieren (siehe eigenes Kapitel)?
- C++17 als Standard, oder besser gleich C++20?
- CMake statt autoconf/makefile ausprobieren

## autoconf / makefile vs. CMake
### vorheriger Zustand
- Das Makefile.in wurde per Script generiert, welches die Dateien in den Verzeichnissen sucht und passende Header als Abhängigkeit einträgt.
- Configure enthält unzählige Checks auf Header-Dateien und Funktionen, die in allen modernen Compilern aber vorhanden sind. Das ist unnötig und macht configure sehr langsam.

### Aktueller Stand
- Ich habe Makefile und Configure entfernt und durch CMake ersetzt. Ich möchte das mal ausprobieren.

Das CMakeLists.txt ist inzwischen allerdings auch recht umfangreich geworden, weshalb ich mir nicht sicher bin, ob das eine gute Idee war. Eventuell hole ich Automake wieder zurück oder biete beides an.

 
## pico-pplib
Siehe: [https://](https://github.com/pfedick/pico-pplib)
Quellcode liegt auch im aktuellen Ordner "pico-pplib" im Repository.

"pico-pplib" ist eine Portierung von pplib auf den Raspberry Pico. Es enthält ein Subset von Klassen und Funktionen aus PPLIB und ist teilweise für Microcontroller optimiert, in dem Beispielsweise kleinere Datentypen verwendet werden, um RAM zu sparen.

Im Grafik-Bereich enthält er teilweise Maßgeschneiderte Routinen zur Ausgabe auf kleinen OLED- oder TFT-Displays, die in der großen PPLIB fehlen.

Der Code ist inzwischen auseinander gelaufen, so dass es schwierig ist, Änderungen in beiden Codebasen zu pflegen. Daher ist es sinnvoll, die pico-pplib-Version in PPLIB zu integrieren und eventuelle Unterschiede über "#ifdef"s zu steuern.

Es ist nicht sinnvoll den kompletten Umfang der PPLIB auf dem Pico bereitzustellen, daher ist es wahrscheinlich sinnvoll im CMakefile.txt ein spezielles Pico-Target zu definieren, oder den Umfang auf andere Art zu steuern.

In pico-pplib gibt es auch Hardware-spezifische Klassen/Treiber, die in PPLIB nicht benötigt werden. Hierfür wäre ein eigenes Projekt sinnvoll, das die Hardware-spezifischen Klassen enthält und von PPLIB abhängig ist.

## Namespace: pplib

Aus "historischen Gründen" hat die Library bisher den Namespace "ppl7" verwendet, und davor "ppl6". Dies machte damals Sinn, da die Library von Version 6 auf 7 grundlegend überarbeitet wurde, und beide Versionen teilweise parallel in Projekten verwendet wurden.

Bei der derzeitigen Überarbeitung erfolgen zwar ebenfalls größere Änderungen, die eine neue Major-Version rechtfertigen (Version 8), allerdings überwiegend intern mit wenig Auswirkungen auf die API.

Da das GitHub-Repository bereits "pplib" heißt, und ich auch die Domain "pplib.de" besitze, scheint es logisch den Namespace ebenfalls pplib zu nennen, zumal "ppl" bereits mit Projekten von Microsoft kollidiert.

Für die Portierung vorhandener Projekte könnte ein Alias "ppl7" definiert werden, der auf den Namespace "pplib" verweist:
```cpp
namespace ppl7 = pplib;
```

## Module
Da in Anwenungsprogrammen, die pplib verwenden, oft nur ein Subset an Funktionen benötigt wird, teilen wir die Library in Module ein, die über CMake gesteuert werden können:
- Core
- Crypto
- Grafix
- Audio
- Internet
- Databases

## Unittests => Google Test
Es gibt bereits recht viele Unittests, die Google Test verwenden, bisher aber ohne Messung der Testabdeckung.

Über CMake "-DPPLIB_ENABLE_COVERAGE=ON" können wir nun den Code passend kompilieren und nach Durchlauf der Tests das Ergebnis mit gcovr visualisieren.

Über das Makefile können die Tests mit Messung der Coverage gestartet werden:

```bash
make coverage
```

Temporär existieren im Makefile auch andere Targets, die gezielt nur die Tests durchlaufen lassen, an denen ich grad arbeite. Oft können diese mit

```bash
make wip
```

gestartet werden (wip = work in progress).

# Fortschritt
## TODO
- Funktionen reviewn, überarbeiten, Dokumentieren, Tests erstellen
- Codereviews sichten, bewerten und abarbeiten
- Grafix überarbeiten (teilweise done)
- Audio überarbeiten
- Internet überarbeiten
- Database überarbeiten (oder in separates Projekt auslagern?)


### Codereviews
Ich habe mittels KI eine Reihe von Klassen auf Fehler prüfen lassen. Die Ergebnisse der Reviews sind im Order "codereview/todo" zu finden. Diese müssen Schritt für Schritt geprüft, bewertet und abgearbeitet werden.

Bereits erledigte Codereviews werden nach "codereview/done" verschoben.

### Font6
Option, damit die Hints auf dem Pico nicht geladen werden, oder durch Kompiler-Option generell deaktivieren. Oder Fonts ohne Hints generieren?

### Crypto-Funktionen
- Unterstützung für moderne Algorithmen hinzufügen (AEAD-Modi)
  - GCM
  - ChaCha20-Poly1305

KI:
Es fehlen AEAD-Modi (GCM, ChaCha20-Poly1305). Aktuell bietet Crypt::Mode nur ECB/CBC/CFB/OFB an — alles unauthentifizierte Modi. Das ist der eigentliche "veraltet vs. aktuell"-Punkt: Heute empfiehlt man praktisch immer AES-GCM oder ChaCha20-Poly1305 (Integrität + Vertraulichkeit in einem), nicht CBC/CFB/OFB ohne HMAC. Das würde aber eine API-Erweiterung brauchen (Tag setzen/holen via EVP_CTRL_AEAD_GET_TAG/SET_TAG, zusätzliche AAD-Methode), keine reine Bugfix-Änderung.

## Erledigt
- Umstellung von Autoconf auf CMake
- Dir-Klasse komplett überarbeitet
- DirEntry-Klasse überarbeitet
- File-Klasse überarbeitet
- Alle Quellcode-Dateien unter "core", "crypto", "math" und "types" kompilieren wieder ohne Fehler
- Tests sind wieder lauffähig
- Array refaktoriert
- AssocArray refaktoriert
- Alte Curl-Klasse entfernt und durch HttpRequest, HttpResponse und HttpClient ersetzt
- Exceptions-Klassen überarbeitet und gereviewed
- Neue ThreadEvent eingeführt, inklusive Doku und Codereview. Tests stehen noch aus.
- Mutex-Klasse überarbeitet
- Thread-Klasse komplett überarbeitet, inklusive Doku, Codereview und Tests
- ThreadPool-Klasse nach Codereview refaktoriert, dokumentiert

### Datenobjekte (Types)
- Variant Klasse refakturiert und geprüft
- Pointer Klasse ausgebaut (Obsolete)
- ByteArray Klasse refakturiert und geprüft
- ByteArrayPtr Klasse refakturiert und geprüft
- String Klasse refakturiert und geprüft
- WideString Klasse refakturiert und geprüft
- DateTime Klasse refakturiert und geprüft
- Neu: Date, Time, TimeZone, TimeDelta

### Drawable
Die Implementierung der DRAWABLE_FUNCTIONS muss überarbeitet werden. Sie erstreckt sich über mehrere Dateien und unterstützt eigentlich nur ein 32-Bit-Format. Es wäre besser, wenn wir pro Format eine Datei mit der vollständigen Implementierung hätten. Die Verwendung von Assembler verkompliziert das ganze zusätzlich und ist für so simple-Methoden wie PutPixel eigentlich nicht notwendig.

Methoden, wie Line oder LineAA verwenden am Ende dann doch PutPixel, weshalb wir sie auch direkt in der Drawable-Klasse einheitlich für alle Formate implementieren können.

Einige Funktionen bekommen das native Farbformat, müssen dass dann aber wieder nach RGBA konvertieren, um zum Beispiel Pixel zu blenden. Vielleicht wäre es an einigen Stellen sinnvoller Color als parameter zu verwenden.

### Imagefilter
Die save-Methode mit dem AssocArray für Parameter war schlecht.

Ich habe die save-Methode aus der Abstrakten Basisklasse ImageFilter entfernt und in den konkreten Klassen implementiert. Dort habe ich bei bedarf dedizierte und passende Parameter-Typen definiert, die die jeweiligen Optionen enthalten. Damit ist die save-Methode typsicher und es gibt keine Probleme mehr mit dem AssocArray.


### Array
Elemente, die keinen Inhalt enthalten, werden nicht als leerer String abgelegt, sondern als nullptr. Methoden, die darauf zugreifen, wie zum Beispiel #sym:get  prüfen auf den nullptr und geben dann das globale, statische Objekt #sym:EmptyString zurück. Sofern es sich um einen const getter handelt, ist das ok, aber es gibt auch getter, die eine schreibbare referenz zurückgeben, und da ist EmptyString dann völlig falsch.

Ferner verkompliziert dieses Herangehen den Code an diversen Stellen unnötig. Ich denke, es wäre besser, immer ein String-Objekt zu allokieren. Was denkst Du?

Wenn jedes Element immer ein String enthält, könnte ich außerdem statt "String *" einen normalen String verwenden. Und wenn ich da angekommen bin, stellt sich mir die Frage, ob ich nicht besser einen Vektor verwenden sollte.

Lösung:
- Umstellung auf std::vector<String> statt String* und numElements
- kein EmptyString mehr, sondern immer ein String-Objekt, auch wenn es leer ist


# Pico

Im Raspberry-Pico werden die Pixel-Daten im 16-Bit-Format beim Zeichnen vertauscht, weshalb
das Format R5G6B5 eigentlich ein Byte-Swapped-Format ist. Host ist Little-Endian, TFT ond OLED sind BigEndian.

Es gibt aber Möglichkeiten den Tausch über die Hardware zu machen.

Beim ST7789 den RAMCTL-Befehl (0xB0), der allerdings bei SPI oft nicht funktioniert. Eine Alternative wäre aber, mit #sym:spi_set_format  den SPI-Modus auf 16 Bit umzuschalten, wodurch der SPI-Controller wohl das höherwertige Byte zuerst schickt. Allerdings muss ich danach dann auch spi_write16_blocking statt spi_write_blocking verwenden, vermutlich größere Änderungen:

// Für Befehle: 8-Bit-Modus
spi_set_format(spi_default, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
spi_write_blocking(spi_default, cmd, 1);

// Für Pixeldaten: Wechsel auf 16-Bit-Modus
spi_set_format(spi_default, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
spi_write16_blocking(spi_default, frame_buffer, buffer_size);