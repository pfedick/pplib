# Iconv Review – Befunde

Review vom 2026-09-19, Scope: `include/pplib/core/iconv.h` + `src/core/Iconv.cpp`.
Exceptions gegen `include/pplib/exceptions.h` verifiziert: `UnsupportedFeatureException`, `CharacterEncodingException`, `UnsupportedCharacterEncodingException`, `CharacterEncodingNotInitializedException` existieren alle real.
Verwendung geprüft in `tests/src/core/iconv.cpp`, `src/core/StringFunctions.cpp`, `src/audio/ID3Tag.cpp`, `src/types/WideString.cpp`.

* Review done by: GitHub Copilot

---

## Bugs (kritisch)

- [ ] **Heap Out-of-Bounds Read bei Fehlerbehandlung in `Iconv::transcode`** (`Iconv.cpp:108-111`)
  Wenn `iconv()` fehlschlägt (`res == (size_t)(-1)`), versucht die Fehlerbehandlung, einen Textausschnitt zur Exception-Meldung hinzuzufügen:
  ```cpp
  if (res == (size_t)(-1)) {
      String e = inbuffer;
      e.cut(64);
      e.append("...");
      throw CharacterEncodingException("Problematische Stelle: Byte %i, Text: %s", (int)(outbuf - ret), (const char*)e);
  }
  ```
  `inbuffer` ist ein Zeiger in ein `ByteArrayPtr` (rohe Binärdaten) und ist **nicht zwingend null-terminiert**. Die Zuweisung `String e = inbuffer;` ruft den C-String-Konstruktor `String(const char*)` auf, der `strlen(inbuffer)` ausführt. Liegt der Fehler am Ende des Puffers oder enthält der Datenstrom kein Nullbyte, liest `strlen` ungebremst über das Pufferende hinaus (Out-of-Bounds Read / Segfault). Zudem enthält `inbuffer` an dieser Stelle typischerweise ungültige UTF-8-Bytes (z. B. `\xFF`), was beim Einlesen in ein UTF-8-basiertes `String`-Objekt problematisch ist.
  `inbytes` gibt bereits exakt an, wie viele Bytes in `inbuffer` noch verbleiben.
  
  **Korrekturvorschlag:**
  ```cpp
  if (res == static_cast<size_t>(-1)) {
      size_t error_offset = from.size() - inbytes;
      size_t snippet_len = std::min(inbytes, static_cast<size_t>(16));
      String hex;
      for (size_t i = 0; i < snippet_len; ++i) {
          hex.appendf("%02X ", static_cast<unsigned char>(inbuffer[i]));
      }
      throw CharacterEncodingException(
          "Transcode fehlgeschlagen an Eingabe-Offset %zu (Restbytes: %zu, Hex: %s)",
          error_offset, inbytes, hex.c_str());
  }
  ```

  ==> FIXED

- [ ] **Irreführende Fehlerposition: Ausgabe-Offset statt Quell-Offset gemeldet** (`Iconv.cpp:111`)
  In der Exception-Meldung steht:
  ```cpp
  throw CharacterEncodingException("Problematische Stelle: Byte %i, Text: %s", (int)(outbuf - ret), (const char*)e);
  ```
  `(int)(outbuf - ret)` ist die Anzahl der bisher in den **Zielpuffer** geschriebenen Bytes, **nicht** die Position im Eingabedatenstrom! Für den Aufrufer ist das völlig irreführend, da die defekte Zeichenfolge im Quelltext gesucht werden muss. Die fehlerhafte Position im Quellpuffer ist `from.size() - inbytes` bzw. `inbuffer - static_cast<const char*>(from.ptr())`.

  ==> Mit dem Fix oben ebenfalls gefixt

- [ ] **Ignorieren von `errno` und fehlende dynamische Pufferanpassung (`E2BIG`)** (`Iconv.cpp:98-111`)
  `outbytes` wird statisch auf `inbytes * 4 + 10` geschätzt. Reicht dieser Puffer nicht aus (z. B. bei zustandsbehafteten Encodings mit langen Escape-Sequenzen, UTF-7 oder zukünftigen Erweiterungen), bricht `iconv()` mit `errno == E2BIG` ab.
  Der Code prüft `errno` überhaupt nicht und wirft sofort eine `CharacterEncodingException`, anstatt den Zielpuffer zu vergrößern und fortzufahren.
  Ebenso sollte bei echten Fehlern zwischen `EILSEQ` (ungültige Bytefolge) und `EINVAL` (unvollständige Multibyte-Sequenz am Dateiende) unterschieden werden.

  **Korrekturvorschlag (Ausschnitt):**
  ```cpp
  while (inbytes > 0) {
      size_t res = iconv(static_cast<iconv_t>(iconv_handle),
                         const_cast<char**>(&inbuffer), &inbytes,
                         &outbuf, &outbytes);
      if (res == static_cast<size_t>(-1)) {
          if (errno == E2BIG) {
              // Puffer vergrößern und Pointer nachjustieren
              ...
              continue;
          }
          if (errno == EINVAL) {
              throw CharacterEncodingException("Unvollstaendige Bytefolge am Ende der Eingabe (Offset %zu)",
                                               from.size() - inbytes);
          }
          // EILSEQ oder sonstiger Fehler
          throw CharacterEncodingException("Ungueltige Bytefolge an Offset %zu", from.size() - inbytes);
      }
  }
  ```

  ==> FIXED

- [ ] **Fehlender Reset des Shift-States bei zustandsbehafteten Encodings** (`Iconv.cpp:92-116`)
  Zustandsbehaftete Zeichensätze (z. B. `ISO-2022-JP`, `HZ`) schalten per Escape-Sequenz zwischen Zeichensätzen um. Laut POSIX muss nach Abschluss der Eingabedaten ein Aufruf von `iconv` mit `inbuf == NULL` erfolgen, um den Deskriptor in den Ausgangszustand zurückzusetzen und eventuell benötigte Shift-Sequenzen in den Zielpuffer zu spülen (z. B. Rückkehr zu ASCII):
  ```c
  iconv(cd, NULL, NULL, &outbuf, &outbytes);
  ```
  Da dies in `Iconv::transcode` nie geschieht:
  1. Ist die Ausgabe bei zustandsbehafteten Encodings unvollständig / ungültig, weil die abschließende Reset-Sequenz fehlt.
  2. Verbleibt der `iconv_t`-Deskriptor in einem modifizierten Shift-Zustand, wodurch nachfolgende `transcode`-Aufrufe auf derselben `Iconv`-Instanz korrumpierte Daten produzieren.

  ==> FIXED

- [ ] **Globales Symbol `iconv_charsets` hat externe Linkage (ODR-Risiko / Namespace-Verschmutzung)** (`Iconv.cpp:142`)
  ```cpp
  const char* iconv_charsets = "437,500,500V1,...";
  ```
  In C++ hat ein Zeiger auf `const char` (`const char*`) standardmäßig **externe Linkage**, da der Zeiger selbst nicht `const` ist! Dadurch wird `iconv_charsets` als globales Symbol in die Bibliothek exportiert.
  **Fix:** Entweder `static const char* const iconv_charsets = ...;`, `constexpr const char* iconv_charsets = ...;` oder in einen anonymen Namespace verschieben.

  ==> FIXED, ist jetzt static

---

## Bugs / Schwachstellen (mittel)

- [ ] **Move-Operationen explizit gelöscht (`= delete`), obwohl `Iconv` eine RAII-Ressource kapselt** (`iconv.h:62-63`)
  ```cpp
  Iconv(Iconv&& other) = delete;
  Iconv& operator=(Iconv&& other) = delete;
  ```
  `Iconv` verwaltet ausschließlich das Handle `iconv_handle`. Kopieren zu löschen ist korrekt (verhindert Double-Free des `iconv_t`), aber Move zu verbieten verhindert:
  - Rückgabe von `Iconv`-Instanzen aus Hilfs-/Factory-Funktionen per Value
  - Speichern in Containern wie `std::vector<Iconv>`
  - Verschieben per `std::move` oder `std::swap`
  
  **Korrekturvorschlag:**
  ```cpp
  Iconv(Iconv&& other) noexcept
      : iconv_handle(other.iconv_handle)
  {
      other.iconv_handle = nullptr;
  }

  Iconv& operator=(Iconv&& other) noexcept
  {
      if (this != &other) {
          if (iconv_handle) {
  #ifdef HAVE_ICONV
              iconv_close(static_cast<iconv_t>(iconv_handle));
  #endif
          }
          iconv_handle = other.iconv_handle;
          other.iconv_handle = nullptr;
      }
      return *this;
  }
  ```

  ==> FIXED

- [ ] **Header ist nicht 'self-contained' (fehlende Includes für Typen)** (`iconv.h:34-36`)
  `iconv.h` gibt `ByteArray` per Value zurück (`ByteArray transcode(...)`), bindet aber weder `bytearray.h`, `bytearrayptr.h` noch `array.h` ein. Die Typen sind in `string.h` lediglich vorwärtsdeklariert.
  Wenn eine externe `.cpp`-Datei ausschließlich `#include <pplib/core/iconv.h>` inkludiert und `transcode()` aufruft, schlägt die Übersetzung fehl ("return type has incomplete type").
  **Fix:** In `iconv.h` explizit einbinden:
  ```cpp
  #include <pplib/types/bytearray.h>
  #include <pplib/types/bytearrayptr.h>
  #include <pplib/types/array.h>
  ```

  ==> FIXED

- [ ] **Doppelte Allokation und unnötige Kopie in `transcode(const ByteArrayPtr&, ByteArray&)`** (`Iconv.cpp:102-114`)
  ```cpp
  ByteArray ba;
  char* buffer = (char*)ba.malloc(outbytes);
  ...
  size_target = outbuf - ret;
  to.copy(ret, size_target);
  ```
  Hier wird zuerst ein lokales `ba` allokiert, dort hinein dekodiert und danach mit `to.copy(...)` ein zweites Mal Heap-Speicher allokiert und alles umkopiert.
  **Fix:** Direkt in `to` allokieren (`to.malloc(outbytes)`), hineinschreiben und anschließend auf die tatsächliche Zielgröße kürzen (`to.truncate(size_target)`).
  *(Hinweis: Bei möglichem Aliasing `&to == &from` vorher prüfen).*

  ==> FIXED

- [ ] **Extrem ineffizienter Fallback in `enumerateCharsets` bei glibc/Linux (400+ `iconv_open`-Aufrufe pro Aufruf)** (`Iconv.cpp:241-251, 269-279`)
  Unter glibc (Standard auf Linux) ist `iconvlist` nicht vorhanden (GNU libiconv-Spezifikum). Der `#else`-Zweig wird daher auf jedem Standard-Linux ausgeführt: Bei **jedem** Methodenaufruf wird der 2 KB lange String `iconv_charsets` geparst und über 400 Mal `iconv_open("UTF-8", ...)` und `iconv_close(...)` aufgerufen!
  Zudem enthalten `enumerateCharsets(Array&)` und `enumerateCharsets(std::list<pplib::String>&)` 100% redundanten Code.
  **Fix:**
  1. Das Ergebnis der Enumeration einmalig ermitteln und statisch cachen (Thread-sicher per `std::call_once`).
  2. Den Algorithmus vereinheitlichen (Template oder interne Hilfsfunktion).

  ==> FIXED

- [ ] **`getLocalCharset()`: Parsing-Schwachstellen und Widerspruch zur Dokumentation** (`Iconv.cpp:283-316`, `iconv.h:145-148`)
  - Die Dokumentation verspricht: *"Unter Windows wird dadurch zum Beispiel anstelle von '.28591' 'ISO-8859-1' zurückgegeben."* Im Code (`#else`-Zweig von `getLocalCharset()`) existiert jedoch keinerlei Mapping-Tabelle. Es wird schlicht `"CP" + tmp` erzeugt (also `"CP28591"`).
  - Wenn `setlocale` einen Locale-String mit Modifier zurückgibt (z. B. `de_DE.UTF-8@euro`), extrahiert `loc.mid(p + 1)` den String `"UTF-8@EURO"`. Der Vergleich auf `"UTF-8"` schlägt fehl, `isNumeric()` ist false, und es wird das ungültige Charset `"UTF-8@EURO"` zurückgegeben.
  - Wenn der Locale-String keinen Punkt enthält (z. B. `"German_Germany"` auf manchen Windows-Konfigurationen), wird `"GERMAN_GERMANY"` als Charset zurückgeliefert.

  ==> Ausnahmebehandlung entfernt, wir werfen eine Exception, wenn Iconv keine passende Methode bereitstellt. Doku angepasst.

- [ ] **Keine Abfrage des Initialisierungsstatus oder der konfigurierten Encodings** (`iconv.h:46-70`)
  Es gibt keine Methoden `bool isInitialized() const noexcept`, `const String& fromEncoding() const` oder `const String& toEncoding() const`. Die Namen der Encodings werden nach `init()` verworfen. Tritt ein Fehler auf, kann die Exception dem Benutzer nicht mitteilen, welche Zeichensätze konfiguriert waren.

  ==> Habe ich noch nie gebraucht, wird nicht eingeführt

---

## Modern C++ & Code Quality

- [ ] **`ICONV_UNICODE`: GCC-spezifische Makros statt C++20 `std::endian`** (`Iconv.cpp:48-54`)
  `#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__` ist GCC-spezifisch und unter MSVC nicht definiert. Da PPL8 auf C++20 aufbaut, sollte `<bit>` und `std::endian::native == std::endian::big` verwendet werden:
  ```cpp
  #include <bit>

  constexpr const char* getIconvUnicode() noexcept
  {
      if constexpr (std::endian::native == std::endian::big) {
          return sizeof(wchar_t) == 2 ? "UTF-16BE" : "UTF-32BE";
      } else {
          return sizeof(wchar_t) == 2 ? "UTF-16LE" : "UTF-32LE";
      }
  }
  ```
  ==> FIXED

- [ ] **C-Style Casts und `NULL` statt Modern C++** (`Iconv.cpp` durchgehend)
  - Durchgängig `NULL` statt `nullptr` (`Iconv.cpp:55, 60, 77, 80, 89` usw.).
  - C-Style Casts wie `(iconv_t)iconv_handle`, `(const char*)toEncoding`, `(char**)(void*)&inbuffer`, `(char*)ba.malloc(...)`.
  - Stattdessen `nullptr`, `static_cast`, `reinterpret_cast` und `toEncoding.c_str()` verwenden.

  ==> NULL durch nullptr ersetzt, die C-Style Casts stören mich nicht

- [ ] **Redundante Überladung `static ByteArray transcode(const String&, ...)`** (`iconv.h:173`, `Iconv.cpp:342`)
  `ByteArrayPtr` hat einen impliziten Konstruktor `ByteArrayPtr(const String& data)`. Die Überladung für `const String& text` leitet lediglich an `ByteArrayPtr` weiter und ist überflüssig.

  ==> Man soll aber explizit sehen können, dass man hier einen String verwenden kann. Daher bleibt es so



- [ ] **Auskommentierter Debug-Code**
  - `Iconv.cpp:129`: `// to.hexDump();`
  - `Iconv.cpp:193, 195, 198, 207, 209, 212`: Auskommentierte `printf`-Zeilen in Enumeration-Callbacks.
  ==> ENTFERNT

- [ ] **Falsche Doxygen-Tags**
  - `Iconv.cpp:224`: `@return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.` — die Funktion gibt `void` zurück.
  - `Iconv.cpp:222`: `@param[out] list Ein CArray-Objekt...` — veralteter PPL7-Name (jetzt `Array`).
  - Parameter-Namen in `iconv.h` (`fromEncoding`, `toEncoding`) weichen von `Iconv.cpp` ab (`fromCode`, `toCode`).

  ==> FIXED
---

## Befunde in anderen Dateien (separat)

- [ ] **Tote Includes in `src/types/WideString.cpp`** (`WideString.cpp:42, 47`)
  `#include <pplib/core/iconv.h>` und `#include <iconv.h>` werden eingebunden, aber in `WideString.cpp` an keiner Stelle verwendet.
  ==> FIXED

- [ ] **Fehlende CMake-Erkennung für `iconvlist`** (`CMakeLists.txt:143-157`)
  In `CMakeLists.txt` wird `check_symbol_exists(iconvlist "iconv.h" HAVE_ICONVLIST)` nicht ausgeführt. `Iconv.cpp` prüft `#ifdef iconvlist`, was nur greift, wenn der Header der Library zufällig ein Präprozessormakro definiert (GNU libiconv), nicht aber bei regulärer Funktionsdeklaration.
  ==> FIXED

- [ ] **Auskommentierter Test mit ungültiger Methode in `tests/src/core/iconv.cpp`** (`tests/src/core/iconv.cpp:88-94`)
  In `IconvTest.enumerateCharsetsStdList` ist der Test-Body auskommentiert, da `std::list` keine `.has()`-Methode besitzt. Kann durch `std::find(list.begin(), list.end(), "UTF-8") != list.end()` aktiviert werden.
  ==> FIXED

- [ ] **Falscher Kommentar in Testfall** (`tests/src/core/iconv.cpp:218`)
  `// "Hällo Wörld" in ISO-8859-1 should be 12 bytes` während gegen 11 Bytes geprüft wird (11 ist korrekt).

  ==> FIXED
