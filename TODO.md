# TODO: Offene Funde aus dem Sanitizer-Lauf (2026-10-29)

Aufgetaucht beim vollständigen ASan+UBSan-Lauf von `test_all` im Rahmen der
ByteArray-Depadding-Umstellung. Alle drei Punkte sind unabhängig von der
Depadding-Aenderung und waren schon vorher vorhanden. Nichts davon ist umgesetzt.

Teststand: 2244 passed / 1 skipped (`FileStaticTest.isLinkWithJunction`) = Basis.

---

## 1. `set(const char*, size_t)`: uebergebene Groesse wird ungeprueft gelesen

Fundstelle (Test): [tests/src/types/widestring.cpp:424](tests/src/types/widestring.cpp) in `WideStringTest.setConstCharWithSize`
Fundstelle (Bibliothek): [src/types/WideString.cpp:265](src/types/WideString.cpp), [src/types/String.cpp:241](src/types/String.cpp), Uebergriff beim `memmove` in [src/types/String.cpp:262](src/types/String.cpp)

```cpp
// Size is bigger than the string length
s1.set((const char*)"äöü, a test string with unicode characters", 100);
```

Der Literal ist 45 Byte + 0-Byte = 46 Byte, uebergeben wird `size = 100`. ASan meldet:

```
ERROR: AddressSanitizer: global-buffer-overflow ... unknown read of size 100
    #1 pplib::String::set(char const*, unsigned long)      src/types/String.cpp:262
    #2 pplib::WideString::set(char const*, unsigned long)  src/types/WideString.cpp:280
    #3 WideStringTest_setConstCharWithSize_Test::TestBody()
```

Ursache: `String::set(const char*, size_t)` und `WideString::set(const char*, size_t)`
uebernehmen die deklarierte Groesse unveraendert und lesen `size` Byte ab der Quelladresse.
Bei `WideString` wird zusaetzlich `::mbstowcs(ptr, str, inbytes + 1)` mit `inbytes = size`
aufgerufen.

Kontrakt-Inkonsistenz (eigentlicher Punkt fuer ein Review): die Overloads mit Referenztypen
begrenzen die Groesse auf die Quelllaenge, die Rohzeiger-Overloads nicht
([src/types/String.cpp:301](src/types/String.cpp) bis 328):

```cpp
if (size == (size_t)-1 || size > str.stringlen) size = str.stringlen;   // String
if (size == (size_t)-1 || size > str.size())    size = str.size();      // ByteArrayPtr, WideString
if (size == (size_t)-1 || size > str.length())  size = str.length();    // std::string, std::wstring
```

Deshalb sind `set(s3, 500)` in `StringTest.setStringRefWithSize` / `setSTDStringRefWithSize`
([tests/src/types/string.cpp:459](tests/src/types/string.cpp), Zeile 497 und 520) harmlos,
`set((const char*) ..., 100)` nicht. In [tests/src/types/widestring.cpp](tests/src/types/widestring.cpp)
(`setConstWChartWithSize`) ist der Rohzeiger-Kontrakt durch einen Kommentar bewusst
dokumentiert ("the caller is responsible for passing an accurate size"); im Header
[include/pplib/types/string.h:527](include/pplib/types/string.h) fehlt jeder Hinweis.

Auswirkung: Der Test ist gruen, aber nur durch Glueck - die mitgelesenen Fremdbits enthalten
zufaellig ein 0-Byte, woran `mbstowcs` stoppt. Bei anderer rodata-Anordnung (Linker,
Optimierungsstufe) entsteht Muell oder eine `CharacterEncodingException`.

Moegliche Behebung (Entscheidung noetig):

1. Absicht des Tests behalten, Puffer gueltig machen: Literal in ein lokales `char buf[128]{}`
   kopieren und `set(buf, 100)`; oder
2. korrekte Groesse uebergeben (`45` bzw. `::strlen(...)`), und den Fall "size groesser als die
   Quelle" als expliziten Test mit ausreichend dimensioniertem Puffer nachbauen; und
3. Header-Doku bei `set(const char*, size_t)` / `set(const wchar_t*, size_t)` (und analog
   `append`, `prepend`) ergaenzen, dass `size` die Leselaenge bestimmt und die Quelle
   entsprechend abgesichert sein muss.

Eine bibliothekseitige Begrenzung auf das 0-Byte waere eine Veraenderungs der Semantik
(zerstoert eingebettete 0-Bytes und die Moeglichkeit, nicht terminierte Puffer zu uebernehmen) -
bewusst nicht empfohlen.

---

## 2. UBSan: `memcpy` mit `nullptr` in `ID3Tag::toUtf16LE`

Fundstelle: [src/audio/ID3Tag.cpp:666](src/audio/ID3Tag.cpp)

```cpp
char* b = (char*)enc.malloc(2 + buffer.size());
if (!b) throw pplib::OutOfMemoryException();
b[0] = 0xff;
b[1] = 0xfe;
memcpy(b + 2, buffer.ptr(), buffer.size());   // buffer leer: ptr() == nullptr
```

```
runtime error: null pointer passed as argument 2, which is declared to never be null
```

Ursache: Liefert `Iconv::transcode()` einen leeren `buffer` (leerer Tag-Text), ist
`buffer.size() == 0` und `buffer.ptr() == nullptr`. Kopiert wird nichts, aber `nullptr` als
Argument an `memcpy` ist formal UB, und Optimierungen duerften das ausnutzen.

Behebung: Groesse pruefen, z. B. `if (buffer.size() != 0) memcpy(b + 2, buffer.ptr(), buffer.size());`.
Ein Test mit leerem Tag-Text fehlt; der Pfad ist ueber die öffentliche API erreichbar.

---

## 3. `TimeZoneTest::TearDown` hinterlaesst `TZ=""` - 11 zeitabhaengige Tests scheitern

Fundstelle: [tests/src/types/timezone.cpp:85](tests/src/types/timezone.cpp)

```cpp
void TearDown() override
{
    // leerer Wert entfernt die Variable (auch unter MSVCRT)
    PPLIB_TZ_SET("TZ", had_tz ? prev_tz.c_str() : "");
    PPLIB_TZ_REFRESH();
}
```

Ursache: Die Annahme stimmt nur fuer MSVCRT. Unter glibc bedeutet `TZ=""` UTC (die Variable ist
gesetzt, aber ohne gueltige Zeitzonenangabe) - sie wird nicht entfernt. Nach dem ersten
`TimeZoneTest` laeuft der Rest der Suite in UTC.

Reproduktion (deterministisch, ohne Shuffle):

```bash
cd tests && TZ= ../build/debug/tests/test_all --gtest_filter='TimeFunction*'
```

Fehlgeschlagen sind die 11 Tests, die von der lokalen Zone (CET/CEST) ausgehen:
`LocalTime`, `MkTime`, `MkTimeFromPPLTIME`, `MkRFC822DateWithUnixTime`,
`MkISO8601DateWithUnixTime`, `MkTime_withIso8601String_withoutTimezone`,
`MkTime_withIso8601String_withZeroTimeoffset`, `MkTime_withIso8601String_withPositiveTimeoffset`,
`MkTime_withIso8601String_withNegativeTimeoffset`, `MkDateWithUnixTime`, `MkDateWithPPLTIME`.

In der normalen Suite fallen sie nur bei anderer Reihenfolge auf (bestaetigt mit
`--gtest_shuffle --gtest_random_seed=7` und `=42`); ohne vorauslaufenden `TimeZoneTest` sind sie
gruen. Auf einer Maschine in UTC waeren umgekehrt die Erwartungen der `TimeZoneTest`-Tests
betroffen.

Behebung: Im POSIX-Zweig (`PPLIB_TZ_SET`, Zeile 49) `unsetenv("TZ")` verwenden, wenn
`had_tz == false`; Windows-Zweig (`_putenv_s("TZ", "")`, Zeile 46) beibehalten und Kommentar
berichtigen. Alternativ global ueber eine `TestEnvironment`, die den Ausgangszustand einmal
sichert.

---

## Wie die Funde 1 und 2 erzeugt wurden

Dafuer gibt es keinen Makefile-Target; gebaut hatte ich in ein separates Verzeichnis:

```bash
cmake -S . -B /tmp/pplib-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g -O1" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build /tmp/pplib-asan --target test_all
cd /tmp/pplib-asan/tests
ASAN_OPTIONS=detect_leaks=0:halt_on_error=0:max_errors=30:allocator_may_return_null=1 UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0 ./test_all
```

Hinweise:

* `allocator_may_return_null=1` ist noetig, weil die `SIZE_MAX`-Boundary-Tests sonst ASans Abort
  "allocation size too big" ausloesen.
* Der CMake-Post-Build-Schritt `gtest_discover_tests` laeuft unter ASan in den 5-Sekunden-Timeout
  (statische Initialisierung "Wortliste laden"); im Testbuild hatte ich den Post-Build-Befehl in
  `tests/CMakeFiles/test_all.dir/build.make` provisorisch durch `true` ersetzt.
* `detect_leaks=0`, weil die Suite bewusst nicht auf Leaks geprueft wurde.

## Nicht (mehr) offen

* ByteArray-Depadding: implementiert, Suite gruen, keine Sanitizer-Befunde im Zusammenhang mit
  `ByteArray`.
* Die drei Tests, die auf die entfernten 0-Byte angewiesen waren, sind korrigiert:
  `setSizeParameterWithCharptr` ([tests/src/types/string.cpp:568](tests/src/types/string.cpp)),
  `setSizeParameterWithWcharptr` ([tests/src/types/string.cpp:591](tests/src/types/string.cpp)),
  `ReadIntoByteArrayPreservesTargetWhenNotOpen` ([tests/src/core/fileobject.cpp:73](tests/src/core/fileobject.cpp),
  frueher `ASSERT_STREQ` auf `ByteArray`-Speicher).
