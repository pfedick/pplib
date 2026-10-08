# MemFile Review – Befunde

Review vom 2026-10-08, Scope: `include/pplib/core/memfile.h` (220 Zeilen) + `src/core/MemFile.cpp` (457 Zeilen) (677 Zeilen gesamt).
Öffentliche API: `pplib::MemFile` (abgeleitet von `pplib::FileObject`).
Review done by: Copilot / Code Reviewer

Mitgelesene Dateien:
- `include/pplib/core/fileobject.h` + `src/core/FileObject.cpp`
- `include/pplib/core/file.h` + `src/core/File.cpp`
- `src/core/Json.cpp`
- `src/core/ConfigParser.cpp`
- `tests/src/core/memfile.cpp`
- `tests/src/core/fileobject.cpp`
- `tests/src/core/file.cpp`

---

## Bugs (kritisch)

- [✅] **`MemFile::fgetc()` wirft `EndOfFileException` statt `EOF` zurückzugeben** (`src/core/MemFile.cpp:372`)
  Der POSIX/C-Standard definiert `fgetc(FILE*)` so, dass am Dateiende `EOF` (`-1`) zurückgegeben wird. Auch die Schwesterklasse `File::fgetc()` gibt am Dateiende `EOF` zurück und wirft keine Exception:
  ```cpp
  int File::fgetc()
  {
      if (ff == NULL) throw FileNotOpenException();
      int ret = ::fgetc((FILE*)ff);
      if (ret != EOF) { pos++; return ret; }
      ...
      return EOF;
  }
  ```
  `MemFile::fgetc()` wirft jedoch bei `pos >= mysize` hart eine `EndOfFileException`:
  ```cpp
  int MemFile::fgetc()
  {
      if (MemBase == NULL) throw FileNotOpenException();
      if (pos >= mysize) throw EndOfFileException(); // <-- FEHLER!
      return static_cast<unsigned char>(MemBase[pos++]);
  }
  ```
  **Auswirkung:**
  Polymorpher Code, der auf `FileObject&` arbeitet und Standard-Schleifen wie `while ((c = file.fgetc()) != EOF)` verwendet (wie z. B. in `src/core/Json.cpp:137, 247`), stürzt beim Parsen von JSON aus einem `MemFile` mit einer ungefangenen `EndOfFileException` ab, anstatt regulär am Dateiende zu terminieren.
  **Fix:**
  ```cpp
  int MemFile::fgetc()
  {
      if (!isOpen()) throw FileNotOpenException();
      if (pos >= mysize) return EOF;
      return static_cast<unsigned char>(MemBase[pos++]);
  }
  ```

  ==> FIXED

- [✅] **`MemFile::fgetwc()` wirft `EndOfFileException` statt `(wchar_t)WEOF` zurückzugeben** (`src/core/MemFile.cpp:376-382`)
  Analog zu `fgetc`: Die POSIX-Funktion `fgetwc()` liefert am Dateiende `WEOF`. In `File::fgetwc()` wird am Dateiende `(wchar_t)WEOF` zurückgegeben.
  In `MemFile::fgetwc()` ruft die Implementierung `fread(buf, sizeof(wchar_t), 1)` auf:
  ```cpp
  wchar_t MemFile::fgetwc()
  {
      wchar_t buf[1];
      size_t n = fread(buf, sizeof(wchar_t), 1);
      if (n == 0) throw EndOfFileException();
      return buf[0];
  }
  ```
  Da `MemFile::fread` bei `pos >= mysize` oder wenn weniger als `sizeof(wchar_t)` Bytes verbleiben eine `EndOfFileException` wirft, wirft `fgetwc()` ebenfalls immer eine Exception am Dateiende.
  Zudem ist der Umweg über `fread` ineffizient und riskiert Exception-Overhead.
  **Fix:**
  ```cpp
  wchar_t MemFile::fgetwc()
  {
      if (!isOpen()) throw FileNotOpenException();
      if (pos + sizeof(wchar_t) > mysize) return static_cast<wchar_t>(WEOF);
      wchar_t ch;
      memcpy(&ch, MemBase + pos, sizeof(wchar_t));
      pos += sizeof(wchar_t);
      return ch;
  }
  ```
  ==> FIXED

- [✅] **`MemFile::fgets()` wirft `EndOfFileException` statt `nullptr` zurückzugeben** (`src/core/MemFile.cpp:287`)
  POSIX `fgets()` und `File::fgets()` geben am Dateiende (wenn keine Zeichen mehr gelesen werden konnten) `NULL` zurück. `FileObject::fgets` dokumentiert dies ausdrücklich:
  `@return Bei Erfolg wird @p buffer zurückgegeben, bei Dateiende wird NULL zurückgegeben. Im Fehlerfall wird eine Exception geworfen.`
  In `MemFile::fgets` steht jedoch:
  ```cpp
  char* MemFile::fgets(char* buffer1, size_t num)
  {
      if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
      if (MemBase != NULL) {
          if (pos >= mysize) throw EndOfFileException(); // <-- FEHLER!
  ```
  **Auswirkung:**
  1. `FileObject::gets(String& buffer, size_t num)` ruft intern `fgets()` auf und prüft `if (ret == NULL) return 0;`. Durch das Werfen von `EndOfFileException` kann `gets(String&, size_t)` niemals `0` zurückgeben, sondern bricht mit Exception ab.
  2. Schleifen wie in `ConfigParser.cpp:506`:
     ```cpp
     while (!file.eof()) {
         if (!file.gets(buffer, 65535)) break;
         ...
     }
     ```
     stürzen mit `EndOfFileException` ab, sobald `file` ein `MemFile` ist.
  **Fix:**
  ```cpp
  char* MemFile::fgets(char* buffer1, size_t num)
  {
      if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
      if (!isOpen()) throw FileNotOpenException();
      if (pos >= mysize) return nullptr;

      size_t by = num - 1;
      if (pos + by > mysize) by = mysize - pos;
      const char* ptr = MemBase + pos;
      size_t i;
      for (i = 0; i < by; i++) {
          if ((buffer1[i] = ptr[i]) == '\n') {
              i++;
              break;
          }
      }
      buffer1[i] = '\0';
      pos += i;
      return buffer1;
  }
  ```

  ==> FIXED, ferner habe ich `isOpen()` jetzt als inline-Funktion implementiert.

- [✅] **`MemFile::fgetws()` wirft `EndOfFileException` & gerät in Endlosschleife bei unvollständigen Wide-Chars** (`src/core/MemFile.cpp:313, 315-328`)
  `MemFile::fgetws()` hat zwei gravierende Fehler:
  1. **Exception statt `nullptr` am Dateiende:** Bei `pos >= mysize` wird `throw EndOfFileException();` ausgeführt, anstatt `nullptr` zurückzugeben (`File::fgetws` gibt `NULL` zurück). Dadurch bricht auch `FileObject::getws(WideString&, size_t)` mit einer Exception ab, statt wie spezifiziert `0` zurückzugeben.
  2. **Endlosschleife (Infinite Loop):** Wenn am Dateiende noch Rest-Bytes vorhanden sind, die für kein vollständiges `wchar_t` mehr ausreichen (`0 < (mysize - pos) < sizeof(wchar_t)`):
     - `available_wchars = (mysize - pos) / sizeof(wchar_t);` ergibt `0`.
     - `max_read = 0;`.
     - Die Leseschleife führt 0 Iterationen aus.
     - `buffer1[0] = 0;` (Leerstring).
     - `pos += 0;` (Dateizeiger bewegt sich nicht).
     - `return buffer1;` (Gibt einen Nicht-Null-Zeiger zurück!).
     Eine Aufruferschleife wie `while (file.fgetws(buf, 1024) != NULL)` terminiert niemals, sondern hängt in einer 100%-CPU-Endlosschleife!
  **Fix:**
  ```cpp
  wchar_t* MemFile::fgetws(wchar_t* buffer1, size_t num)
  {
      if (buffer1 == nullptr || num == 0) throw IllegalArgumentException();
      if (!isOpen()) throw FileNotOpenException();
      if (pos >= mysize) return nullptr;

      size_t available_wchars = (mysize - pos) / sizeof(wchar_t);
      if (available_wchars == 0) return nullptr;

      size_t max_read = (num - 1 < available_wchars) ? (num - 1) : available_wchars;
      size_t i = 0;
      for (i = 0; i < max_read; i++) {
          wchar_t ch;
          memcpy(&ch, MemBase + pos + (i * sizeof(wchar_t)), sizeof(wchar_t));
          buffer1[i] = ch;
          if (ch == L'\n') {
              i++;
              break;
          }
      }
      buffer1[i] = L'\0';
      pos += i * sizeof(wchar_t);
      return buffer1;
  }
  ```

  ==> FIXED

- [✅] **`MemFile::fwrite()` gibt geschriebene Bytes statt Elemente zurück** (`src/core/MemFile.cpp:270-281`)
  Die Signatur in `FileObject` lautet:
  ```cpp
  virtual size_t fwrite(const void* ptr, size_t size, size_t nmemb);
  ```
  Die Doxygen-Doku in `include/pplib/core/fileobject.h:352` fordert explizit:
  `%fwrite gibt die Anzahl der erfolgreich geschriebenen Elemente zurück (nicht die Anzahl der Zeichen).`
  In `File::fwrite()` wird der Rückgabewert der libc-Funktion `::fwrite()` (`by` = Anzahl geschriebener Elemente) zurückgegeben.
  In `MemFile::fwrite()` steht jedoch:
  ```cpp
  size_t MemFile::fwrite(const void* ptr, size_t size, size_t nmemb)
  {
      ...
      size_t bytes = nmemb * size;
      if (pos + bytes > mysize) resizeBuffer(pos + bytes);
      memmove(MemBase + pos, ptr, bytes);
      pos += bytes;
      return bytes; // <-- FEHLER! Gibt BYTES statt NMEMB zurück!
  }
  ```
  **Auswirkung:**
  Wird `fwrite` mit `size > 1` aufgerufen (z.B. `fwrite(wchars, sizeof(wchar_t), 10)`), gibt `File::fwrite` den Wert `10` zurück, `MemFile::fwrite` dagegen `40` (unter Linux) bzw. `20` (unter Windows).
  Jeder polymorphe Aufrufer, der den Rückgabewert gegen `nmemb` prüft (`if (f.fwrite(...) != count)`), scheitert bei `MemFile`.
  (Hinweis: `MemFile::fread` gibt korrekt die Anzahl Elemente `by` zurück — `fread` und `fwrite` sind in `MemFile` asymmetrisch!).
  **Fix:**
  ```cpp
  return nmemb;
  ```
  ==> FIXED

- [✅] **Inkonsistenter `isOpen()`-Zustand und ungültige Exceptions bei Default-Konstruktion** (`src/core/MemFile.cpp:35-44, 173-176, 256, 369, 434`)
  1. Laut Dokumentation in `memfile.h:60`:
     `Durch Verwendung dieses Konstruktors wird die Klasse zum Lesen und Schreiben geöffnet, wobei der Speicherbereich initial 0 Byte gross ist.`
     Nach `MemFile f;` ist `MemBase == NULL` und `readonly == false`.
     - `f.isOpen()` prüft `if (MemBase != NULL) return true;` und liefert `false`!
     - `f.tell()`, `f.seek()`, `f.eof()`, `f.fwrite()` prüfen `MemBase != NULL || readonly == false` und behandeln die Datei als geöffnet.
     - `f.fread()`, `f.fgetc()`, `f.fgets()`, `f.fgetws()`, `f.adr()`, `f.map()` prüfen jedoch starr `if (MemBase == NULL)` und werfen `FileNotOpenException`!
  2. Eine frisch erstellte, leere Datei ist geöffnet, enthält jedoch 0 Bytes. Leseoperationen darauf müssen sofort EOF signalisieren (`fread` wirft `EndOfFileException`, `fgetc` liefert `EOF`, `fgets` liefert `nullptr`), dürfen aber keinesfalls `FileNotOpenException` werfen!
  3. Nach `f.close()` wird `readonly = true` gesetzt, um den geschlossenen Zustand zu markieren. Dadurch wirft z. B. `truncate(0)` auf einer geschlossenen Datei fälschlicherweise `ReadOnlyException` anstelle von `FileNotOpenException`.
  **Fix:**
  Eindeutige Repräsentation des Geöffnet-Zustands (z. B. boolesches Flag `is_open`):
  - Konstruktor `MemFile()` setzt `is_open = true`, `readonly = false`.
  - `close()` setzt `is_open = false`.
  - `isOpen()` liefert `is_open`.
  - Alle Methoden prüfen einheitlich `if (!isOpen()) throw FileNotOpenException();`.

---

## Bugs (mittel / Warnungen)

- [✅] **Fehlende Bounds-Prüfung in `MemFile::adr(size_t adresse)` → Zeiger ins Nirvana** (`src/core/MemFile.cpp:393-399`)
  ```cpp
  char* MemFile::adr(size_t adresse)
  {
      if (MemBase != NULL) {
          return (MemBase + adresse);
      }
      throw FileNotOpenException();
  }
  ```
  Es findet keinerlei Prüfung statt, ob `adresse <= mysize` ist.
  Ein Aufruf `f.adr(1000000)` auf einer 10-Byte-Datei liefert ohne Fehlermeldung einen wilden Zeiger in fremden Speicherbereich.
  **Fix:**
  ```cpp
  char* MemFile::adr(size_t adresse)
  {
      if (!isOpen()) throw FileNotOpenException();
      if (adresse > mysize) throw OutOfBoundsException();
      return (MemBase + adresse);
  }
  ```
  ==> FIXED

- [✅] **Integer-Überlauf bei Puffer-Rundung in `resizeBuffer()`** (`src/core/MemFile.cpp:160`)
  ```cpp
  size_t newsize = (((size + 8191) >> 13) << 13);
  ```
  Wenn `size > SIZE_MAX - 8191`, läuft `size + 8191` über (`wraparound`) und ergibt eine sehr kleine Zahl. `realloc` allokiert daraufhin einen Winzpuffer, und das anschließende Schreiben führt zu fataler Heap-Corruption.
  **Fix:**
  ```cpp
  if (size > SIZE_MAX - 8191) throw OverflowException();
  ```
  ==> FIXED

- [✅] **Möglicher Additions-Überlauf in `fwrite()` bei `pos + bytes`** (`src/core/MemFile.cpp:276-277`)
  `fwrite` prüft zwar Multiplikationsüberlauf bei `nmemb * size`, aber nicht:
  ```cpp
  size_t bytes = nmemb * size;
  if (pos + bytes > mysize) resizeBuffer(pos + bytes);
  ```
  Wenn `bytes > SIZE_MAX - pos`, läuft `pos + bytes` über und umgeht die Prüfung bzw. übergibt einen falschen Wert an `resizeBuffer`.
  **Fix:**
  ```cpp
  if (bytes > SIZE_MAX - pos) throw OverflowException();
  ```
  ==> FIXED

- [✅] **32-Bit-Truncation und Überlauf-Risiko in `fputs()` und `fputws()`** (`src/core/MemFile.cpp:339, 349`)
  In `fputs`:
  ```cpp
  fwrite((void*)str, 1, (uint32_t)strlen(str));
  ```
  `strlen` gibt `size_t` zurück. Der Cast nach `(uint32_t)` schneidet auf 64-Bit-Plattformen Strings > 4 GB ab. Zudem ist `(void*)` überflüssig.
  In `fputws`:
  ```cpp
  fwrite(str, 1, (uint32_t)wcslen(str) * sizeof(wchar_t));
  ```
  Schneidet ebenfalls auf 32 Bit ab, riskiert Multiplikationsüberlauf und ruft `fwrite` mit `size=1` statt `sizeof(wchar_t)` auf.
  **Fix:**
  ```cpp
  void MemFile::fputs(const char* str)
  {
      if (!str) throw IllegalArgumentException();
      if (!isOpen()) throw FileNotOpenException();
      fwrite(str, 1, strlen(str));
  }

  void MemFile::fputws(const wchar_t* str)
  {
      if (!str) throw IllegalArgumentException();
      if (!isOpen()) throw FileNotOpenException();
      fwrite(str, sizeof(wchar_t), wcslen(str));
  }
  ```
  ==> FIXED

- [✅] **`MemFile::rewind()` ignoriert geschlossenen Zustand** (`src/core/MemFile.cpp:198-201`)
  ```cpp
  void MemFile::rewind()
  {
      pos = 0;
  }
  ```
  `File::rewind()` ruft `seek(0)` auf und wirft bei geschlossener Datei `FileNotOpenException`. `MemFile::rewind()` setzt blind `pos = 0`, selbst wenn die Datei längst mit `close()` geschlossen wurde.
  **Fix:**
  ```cpp
  void MemFile::rewind()
  {
      seek(0);
  }
  ```
  ==> FIXED

- [✅] **Move-Konstruktor und Move-Zuweisung verlieren `filename()`** (`src/core/MemFile.cpp:63-111`)
  Die Basisklasse `FileObject` verwaltet `MyFilename`. Im Gegensatz zu `File::File(File&& other)`:
  ```cpp
  setFilename(other.filename());
  other.setFilename("");
  ```
  überträgt `MemFile` den Dateinamen beim Move überhaupt nicht. Ein zuvor gesetzter Dateiname geht bei Move verloren, während das verschobene Quellobjekt seinen Namen behält.
  **Fix:**
  `setFilename(other.filename()); other.setFilename("");` in Move-Konstruktor und Move-Operator aufnehmen.

  ==> FIXED

- [✅] **32-Bit-Plattform-Truncation in `truncate(uint64_t length)`** (`src/core/MemFile.cpp:434-445`)
  Der Parameter `length` ist `uint64_t`. Auf 32-Bit-Systemen (wie Raspberry Pico / x86-32) ist `size_t` 32 Bit groß. Wenn `length > SIZE_MAX`, wird der Wert bei `size_t increase = length - mysize;` und `resizeBuffer(length)` unbemerkt auf 32 Bit abgeschnitten.
  **Fix:**
  ```cpp
  if (length > SIZE_MAX) throw OverflowException();
  ```
  ==> FIXED

- [✅] **Inkonsistente Exceptions bei Positionierung über das Dateiende hinaus** (`src/core/MemFile.cpp:208, 240`)
  - `seek(uint64_t position)`: wirft bei `position > mysize` eine `OverflowException`.
  - `seek(int64_t offset, SeekOrigin origin)`: wirft bei `newpos > mysize` eine `FileSeekException`.
  Da beide Methoden die gleiche fachliche Operation durchführen, sollte bei Überschreitung des Dateiendes einheitlich `FileSeekException` geworfen werden.
  ==> FIXED

---

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [✅] **Verwendung von `NULL` und `0` statt `nullptr`** (`src/core/MemFile.cpp`)
  In `MemFile.cpp` wird an zahlreichen Stellen noch C-artiges `NULL` bzw. `0` für Zeiger verwendet:
  - `buffer = NULL;`, `MemBase = NULL;` (Zeile 37, 40, 180, 186)
  - `if (buffer != 0)` (Zeile 184)
  - `if (ptr == NULL)` (Zeile 257, 272)
  Empfehlung: Konsequent durch C++11/C++20 `nullptr` ersetzen.
  ==> FIXED

- [❌] **C-Style Typecasts statt C++-Casts** (`src/core/MemFile.cpp`)
  Verwendung veralteter C-Casts wie `(char*)adresse`, `(char*)realloc(...)`, `(int64_t)pos`, `(void*)str`.
  Empfehlung: Durch `static_cast<char*>` bzw. `reinterpret_cast` ersetzen.

  ==> Ich habe nichts gegen C-Style-Casts. Bei neu-Implementierung achte ich darauf, würde es hier aber so lassen

- [✅] **Unnötiger Cast in `size()`** (`src/core/MemFile.cpp:195`)
  ```cpp
  uint64_t MemFile::size() const
  {
      return (int64_t)mysize;
  }
  ```
  `mysize` ist `size_t`. Der Zwischen-Cast nach `(int64_t)` ist redundant und birgt bei extrem großen Werten das Risiko von Signed Integer Overflows.
  Empfehlung: Direkt `return mysize;`.

  ==> Das muss auf `uint64_t` geändert werden. FIXED

- [✅] **Konstruktor-Delegation für `MemFile(const ByteArrayPtr& memory)`** (`src/core/MemFile.cpp:52-61`)
  ==> FIXED
  Aktuell dupliziert dieser Konstruktor die Zuweisungen von `open()` manuell.
  Empfehlung: Delegierender Konstruktor an `open(memory)`:
  ```cpp
  MemFile::MemFile(const ByteArrayPtr& memory)
      : MemFile()
  {
      open(memory);
  }
  ```
  ==> FIXED

- [✅] **Interne Pufferverwaltung via `malloc`/`realloc`/`free`** (`src/core/MemFile.cpp:163, 185`)
  Historisch nutzt `MemFile` rohe C-Heap-Funktionen. Gemäß REFACTORING.md (Modernisierung) wäre die Kapselung in einen RAII-Container (z. B. `pplib::ByteArray` oder `std::vector<char>`) deutlich robuster gegen Memory Leaks und Heap Corruption.

  ==> FIXED, verwende jetzt `pplib::ByteArray` für die interne Pufferverwaltung.

---

## Dokumentation & Doxygen

- [ ] **Fehlende Doxygen-Kommentare im Header `include/pplib/core/memfile.h` für virtuelle Overrides**
  Im Header `include/pplib/core/memfile.h` fehlen ab Zeile 163 für fast alle geerbten Funktionen Doxygen-Dokumentationen oder `@copydoc FileObject::<methode>`. Dadurch fehlen in IntelliSense und IDE-Hover Beschreibungen und Hinweistexte zu den Methoden von `MemFile`.

- [✅] **Fehlende Dokumentation von `adr(size_t)`** (`include/pplib/core/memfile.h:145`)
  Die Methode `char* adr(size_t adresse);` besitzt keinerlei Doxygen-Kommentar. Es sollte dokumentiert werden, was der Rückgabewert ist und welche Exceptions geworfen werden.

  ==> FIXED

- [✅] **Klarstellung der Ownership bei `open(..., writeable=true)` und `openReadWrite()`** (`include/pplib/core/memfile.h:116, 143`)
  `MemFile::open(void* adresse, size_t size, bool writeable = false)` besitzt einen `@attention`-Hinweis, dass der Speicher an `MemFile` übergeht (`free`/`realloc`). Bei `openReadWrite()` fehlt dieser `@attention`-Hinweis vollständig, obwohl es dieselbe Ownership-Übernahme durchführt.

  ==> FIXED
---

## Befunde in Drittdateien

- [ ] **`ConfigParser::loadFromMemory` und `loadFromString` führen `ff.load()` statt `load(ff)` aus (Toter Code / Parser-Ausfall)** (`src/core/ConfigParser.cpp:448, 460, 474`)
  In `src/core/ConfigParser.cpp`:
  ```cpp
  void ConfigParser::loadFromMemory(const void* buffer, size_t bytes)
  {
      if (!buffer) throw IllegalArgumentException("buffer");
      if (!bytes) throw IllegalArgumentException("bytes");
      MemFile ff;
      ff.open((void*)buffer, bytes);
      ff.load(); // <-- FEHLER! Lädt nur Bytes in ein temporäres ByteArray und verwirft es!
  }
  void ConfigParser::loadFromMemory(const ByteArrayPtr& ptr)
  {
      MemFile ff;
      ff.open(ptr);
      ff.load(); // <-- FEHLER!
  }
  void ConfigParser::loadFromString(const String& string)
  {
      MemFile ff;
      ff.open((void*)string.getPtr(), string.size());
      ff.load(); // <-- FEHLER!
  }
  ```
  Alle drei Methoden lesen den Speicherinhalt über `ff.load()` ein und ignorieren den Rückgabewert. Die tatsächliche Parsing-Methode `load(FileObject& file)` wird niemals aufgerufen! Das ConfigParser-Objekt bleibt nach Aufruf dieser Funktionen komplett leer.
  **Fix:**
  `ff.load();` in allen drei Funktionen durch `load(ff);` ersetzen.

- [ ] **Unit-Tests in `tests/src/core/memfile.cpp` codieren das fehlerhafte Verhalten fest** (`tests/src/core/memfile.cpp:115, 125, 142, 332`)
  Vier Unit-Tests wurden so geschrieben, dass sie das fehlerhafte Werfen von `EndOfFileException` erwarten:
  - `TEST_F(MemFileTest, FgetcThrowsEndOfFileException)`: `ASSERT_THROW(f.fgetc(), pplib::EndOfFileException);`
  - `TEST_F(MemFileTest, FgetwcThrowsEndOfFileException)`: `ASSERT_THROW(f.fgetwc(), pplib::EndOfFileException);`
  - `TEST_F(MemFileTest, FgetwsClampingAndIllegalArgument)`: `ASSERT_THROW(f.fgetws(buf, 64), pplib::EndOfFileException);`
  - `TEST_F(MemFileTest, FgetsReadsLinesAndEof)`: `EXPECT_THROW(f.fgets(buf, sizeof(buf)), pplib::EndOfFileException);`
  Sobald `MemFile` POSIX-konform korrigiert wird (`fgetc` -> `EOF`, `fgetwc` -> `WEOF`, `fgets` -> `nullptr`, `fgetws` -> `nullptr`), müssen diese Tests entsprechend aktualisiert werden.

- [ ] **`FileObjectTest::GetsAndGetwsOverloads` musste `MockStringIoFile` als Workaround verwenden** (`tests/src/core/fileobject.cpp:302-337`)
  In `tests/src/core/fileobject.cpp` wurde ein spezieller Mock `MockStringIoFile` gebaut, um `gets` und `getws` bei EOF zu testen, weil `MemFile` aufgrund der geworfenen `EndOfFileException` nicht für diese Standardtests verwendet werden konnte. Nach dem Fix von `MemFile` kann `MemFile` auch hier regulär getestet werden.
