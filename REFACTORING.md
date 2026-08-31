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
- Code ist vollständig mit KI auf Fehler geprüft
- Testabdeckung erhöhen
- Alle Tests gehen von UTF-8 als locale aus, auch unter Windows
- pico-pplib integrieren (siehe eigenes Kapitel)?
- C++17 oder C++20 als Standard?
- CMake statt autoconf/makefile verwenden?
- configure / Makefile abspecken: wir brauchen einen Großteil der Checks auf Header und Funktion nicht, das diese laut Standard in allen modernen Compilern vorhanden sind. Wir brauchen nur noch die Checks auf C++17/C++20, sowie die Checks auf die Plattform (Windows, Linux, FreeBSD). Alternative: CMake?


## autoconf / makefile vs. CMake
### bisheriger Zustand
- Das Makefile.in wird im Moment per Script generiert, welches die Dateien in den Verzeichnissen sucht und passende Header als Abhängigkeit einträgt.
- Configure enthält unzählige Checks auf Header-Dateien und Funktionen, die in allen modernen Compilern aber vorhanden sind. Das ist unnötig und macht configure sehr langsam.

Fragen dazu:
- Gibt es Möglichkeiten das Makefile automatisch zu generieren?
- Abhängigkeit zu den jeweiligen Header-Dateien automatisch finden?
- Manuell pflegen? So oft kommen keine neuen Dateien hinzu
- Würden sich die Probleme von selbst erledigen, wenn make die Dateien selbst findet? (z.B. mit wildcards)

### Aktueller Stand
- Ich habe Makefile und Configure entfernt und durch CMake ersetzt. Ich möchte das mal ausprobieren.

Das CMakeLists.txt ist inzwischen allerdings auch recht umfangreich geworden, weshalb ich mir nicht sicher bin, ob das eine gute Idee war. Eventuell hole ich Automake wieder zurück oder biete beides an.


 
## pico-pplib
Siehe: [https://](https://github.com/pfedick/pico-pplib)
Quellcode liegt auch im aktuellen Ordner "pico-pplib" im Repository.

"pico-pplib" enthält ein Subset von Klassen und Funktionen aus PPLIB und ist für Microcontroller optimiert, und zwar in dem Sinne, dass weniger RAM benötigt wird. Viele Objekte verwenden daher uint16_t anstelle von int oder es gibt duplikate von Klassen, die 16-Bit Integer verwenden.

Der Code ist inzwischen auseinander gelaufen, so dass es schwierig ist, Änderungen in beiden Codebasen zu pflegen. Daher ist es sinnvoll, die pico-pplib-Version in PPLIB zu integrieren und eventuelle Unterschiede über "#ifdef"s zu steuern.

Wahrscheinlich möchte ich auf dem pico auch weiterhin nur ein subset der hier vorhandenen Funktionen und Klassen verwenden, da wir mit Speicher sparen müssen und viele Klassen auf dem pico nicht benötigt werden.

In pico-pplib gibt es auch Hardware-spezifische Klassen/Treiber, die in PPLIB nicht benötigt werden. Hierfür wäre ein eigenes Projekt sinnvoll, das die Hardware-spezifischen Klassen enthält und von PPLIB abhängig ist.

## Namespace ppl7, ppl8 oder pplib?

Aus "historischen Gründen" hat die Library bisher den Namespace "ppl7" verwendet. Dies hing vor vielen Jahren mit der Umstellung von Version 6 auf 7 zusammen. Damals erfolgte eine sehr umfangreiche Überarbeitung der Library, die viele Änderungen an den Interfaces mit sich brachte. Damit beide Versionen parallel in einem projekt verwendet werden konnten, wurde die alte Version in den Namespace "ppl6" und die neue Version in den Namespace "ppl7" gelegt.

Hier erfolgen jetzt ebenfalls größere Änderungen, weshalb eine neue Major-Version gerechtfertigt ist. Allerdings bleiben die meisten Klassen und Funktionen unverändert, weshlab eine Umstellung vorhandener Programme weniger aufwendig ist. Übergangsweise könnte man auch einen Alias "ppl7" definieren, der auf den neuen Namespace verweist.

Da das GitHub-Repository "pplib" heißt, und ich auch die Domain "pplib.de" besitze, wäre es sinnvoll auf die Version im Namespace zu verzichten und stattdessen den neutralen Namespace "pplib" zu verwenden. "ppl" kollidiert bereits mit dem Namespace "ppl" von Microsoft, der in Visual Studio verwendet wird.

Für die Portierung vorhandener Projekte könnte ein Alias "ppl7" definiert werden, der auf den Namespace "pplib" verweist:
```cpp
namespace ppl7 = pplib;
```

Entscheidung: wir verwenden den Namespace "pplib"

## Fragen
- Sollen wir die Library aufsplitten in mehrere kleinere Module?
  - Core
  - Grafix
  - Audio
  - Crypto
  - Internet

## Unittests
Es gibt bereits recht viele Unittests, die Google Test verwenden. Es gibt aber aktuell keine Möglichkeit die Testabdeckung zu messen. Dies soll bei der Refakturierung geändert werden, unter Verwendung von gcov.

Durch die laufende Refakturierung and der Haupt-Library sind die Unittests aktuell leider nicht kompilierbar. Hier sind auch noch Anpassungen an die geänderten Interfaces notwendig.

Ein grundsätzliches Problem der Tests waren in der Vergangenheit die unterschiedlichen Locale-Einstellungen unter Windows und Linux. Unter Windows ist die Standard-Locale "ANSI", unter Linux "UTF-8". Viele Tests sind auf die Locale "UTF-8" angewiesen, weshalb wir die Tests so anpassen müssen, dass sie auch unter Windows mit UTF-8 als Locale laufen.

# Fortschritt
## TODO
- Array checken und ggf. refaktorisieren
- AssocArray
- Funktionen
- File (viele fehlschlagende Unittests)
- Grafix überarbeiten
- Audio überarbeiten
- Internet überarbeiten
- Database überarbeiten (oder in separates Projekt auslagern?)
- Tests wieder lauffähig bekommen


## Font6
Option, damit die Hints auf dem Pico nicht geladen werden, oder durch Kompiler-Option generell deaktivieren. Oder Fonts ohne Hints generieren?

## Crypto-Funktionen
- Unterstützung für moderne Algorithmen hinzufügen (AEAD-Modi)
  - GCM
  - ChaCha20-Poly1305

KI:
Es fehlen AEAD-Modi (GCM, ChaCha20-Poly1305). Aktuell bietet Crypt::Mode nur ECB/CBC/CFB/OFB an — alles unauthentifizierte Modi. Das ist der eigentliche "veraltet vs. aktuell"-Punkt: Heute empfiehlt man praktisch immer AES-GCM oder ChaCha20-Poly1305 (Integrität + Vertraulichkeit in einem), nicht CBC/CFB/OFB ohne HMAC. Das würde aber eine API-Erweiterung brauchen (Tag setzen/holen via EVP_CTRL_AEAD_GET_TAG/SET_TAG, zusätzliche AAD-Methode), keine reine Bugfix-Änderung.

## NEU
- HttpRequest, HttpResponse, HttpClient

## Erledigt
- Umstellung von Autoconf auf CMake
- Dir-Klasse komplett überarbeitet
- DirEntry-Klasse überarbeitet
- Alle Quellcode-Dateien unter "core", "crypto", "math" und "types" kompilieren wieder ohne Fehler

### Drawable
Die Implementierung der DRAWABLE_FUNCTIONS muss überarbeitet werden. Sie erstreckt sich über mehrere Dateien und unterstützt eigentlich nur ein 32-Bit-Format. Es wäre besser, wenn wir pro Format eine Datei mit der vollständigen Implementierung hätten. Die Verwendung von Assembler verkompliziert das ganze zusätzlich und ist für so simple-Methoden wie PutPixel eigentlich nicht notwendig.

Methoden, wie Line oder LineAA verwenden am Ende dann doch PutPixel, weshalb wir sie auch direkt in der Drawable-Klasse einheitlich für alle Formate implementieren können.

Einige Funktionen bekommen das native Farbformat, müssen dass dann aber wieder nach RGBA konvertieren, um zum Beispiel Pixel zu blenden. Vielleicht wäre es an einigen Stellen sinnvoller Color als parameter zu verwenden.

### Datenobjekte (Types)
- Variant Klasse refakturiert und geprüft
- Pointer Klasse refakturiert und geprüft => als obsolete markiert
- ByteArray Klasse refakturiert und geprüft
- ByteArrayPtr Klasse refakturiert und geprüft
- String Klasse refakturiert und geprüft
- WideString Klasse refakturiert und geprüft
- DateTime Klasse refakturiert und geprüft
- Neu: Date, Time, TimeZone, TimeDelta

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