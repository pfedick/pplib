# File Review – Befunde

Review vom 2026-09-18, Scope: `include/pplib/core/file.h` + `src/core/File.cpp`.
Abhängigkeiten und Querverweise in `include/pplib/core/fileobject.h`, `src/core/FileObject.cpp`, `include/pplib/core/dir.h`, `src/core/Exceptions.cpp`, `src/math/md5.cpp`, `src/math/sha256.cpp`, `tests/src/core/file.cpp` und `tests/src/core/filestatic.cpp` geprüft.

---

## Bugs (kritisch)

- [ ] **`File::erase()` im Header deklariert, aber in `File.cpp` nicht implementiert (Linker-Error)** (`include/pplib/core/file.h:223`)
  In `file.h` ist die nicht-statische Elementfunktion `void erase();` deklariert:
  ```cpp
  /** @brief Geöffnete Datei löschen
   * Mit dieser Funktion wird die aktuelle Datei zunächst geschlossen und dann gelöscht.
   */
  void erase();
  ```
  In `File.cpp` existiert jedoch nur die statische Überladung `File::erase(const String& filename)`. Ein Aufruf von `file.erase()` führt direkt zu einem Linkerfehler (`undefined reference to pplib::File::erase()`).
  *Achtung beim Fix:* Da `close()` aktuell den Dateinamen leert, muss der Dateiname vor dem Schließen gesichert werden.
  ```cpp
  void File::erase()
  {
      if (!isOpen()) throw FileNotOpenException();
      String fn = filename();
      close();
      if (!fn.isEmpty()) {
          File::erase(fn);
      }
  }
  ```

  ==> Methode angelegt

- [ ] **`File::seek(int64_t offset, SeekOrigin origin)`: 32-Bit-Truncation auf 64-Bit-Windows & `pos` wird nicht aktualisiert** (`src/core/File.cpp:392-395`)
  1. Auf 64-Bit-Windows (LLP64) ist `long` nur 32 Bit groß (`-2 GB` bis `+2 GB`). Zeile 392 castet:
     ```cpp
     int suberr = ::fseek((FILE*)ff, (long)offset, o);
     ```
     Jeder Offset > 2 GB wird abgeschnitten! Es muss `::fseeko((FILE*)ff, (off_t)offset, o)` gerufen werden (unter Windows via `_MSC_VER` bereits als `_fseeki64` gemappt).
  2. Im Gegensatz zu `seek(uint64_t)` wird `pos` hier überhaupt nicht aktualisiert! Zeile 394 ruft nur `return tell();` auf, lässt `pos` aber auf dem alten Wert stehen.
  3. Folge: `File::eof()` (`if (!isPopen && pos >= mysize)`), `File::truncate()` und nachfolgende `fwrite()`-Größenberechnungen arbeiten mit einem veralteten/falschen `pos`.
  ```cpp
  // Fix:
  if (::fseeko((FILE*)ff, (off_t)offset, o) != 0) {
      throwErrno(errno, filename());
  }
  pos = tell();
  return pos;
  ```

  ==> FIXED

- [ ] **`File::close()` löscht Dateinamen vor dem Schließen – Exceptions verlieren Dateinamen** (`src/core/File.cpp:228, 257`)
  `close()` ruft in Zeile 228 `setFilename("");` auf, *bevor* `::fclose((FILE*)ff)` ausgeführt wird:
  ```cpp
  setFilename("");
  if (ff != NULL) {
      ...
      if (::fclose((FILE*)ff) != 0) ret = 0;
      ...
      if (ret == 0) throwErrno(errno, filename()); // filename() ist hier immer leer!
      return;
  }
  ```
  Schlägt `fclose` fehl (z.B. gepufferter Schreibfehler auf vollem Datenträger / NFS-Fehler), enthält die geworfene Exception einen leeren Dateinamen `""`.
  *Fix:* `setFilename("")` erst nach erfolgreichem Schließen aufrufen, bzw. Dateinamen vorab in einer lokalen Variablen sichern.

  ==> FIXED

- [ ] **Kopieren von `File` erzeugt Double-Close / Double-Free (Rule of 5 verletzt)** (`include/pplib/core/file.h`, `src/core/File.cpp`)
  `File` besitzt einen benutzerdefinierten virtuellen Destruktor, der `close()` (`fclose`, `munmap`) aufruft. Kopierkonstruktor und Kopierzuweisungsoperator sind weder gelöscht noch implementiert.
  Wird eine `File`-Instanz versehentlich by-value übergeben oder kopiert, teilen sich beide Instanzen denselben `FILE*`-Zeiger und Speicher-Mapping. Die Zerstörung der Kopie schließt das Handle; die Originalinstanz greift danach auf einen geschlossenen Deskriptor zu und ruft im eigenen Destruktor ein zweites `fclose()` (Undefined Behavior / Crash) auf.
  *Fix:* In `include/pplib/core/file.h` Copy verbieten und Move explizit definieren:
  ```cpp
  File(const File&) = delete;
  File& operator=(const File&) = delete;
  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;
  ```

  ==> FIXED

- [ ] **`File::copy()`: Heap-Leak bei Exceptions und OOM bei 0-Byte-Dateien** (`src/core/File.cpp:948-968`)
  1. Bei einer 0-Byte-Quelldatei ist `f1.mysize == 0` -> `bsize = 0`. `malloc((size_t)0)` liefert auf manchen Plattformen/Runtimes `NULL`. Dadurch wirft `copy()` bei leeren Dateien fälschlicherweise `OutOfMemoryException`.
  2. `malloc((size_t)bsize)` allokiert einen rohen Zeiger. Wirft `f1.fread()` oder `f2.fwrite()` eine Exception (z.B. Disk Full `FilesystemFullException`, `IOErrorException`), wird `free(buffer)` nie aufgerufen -> Memory-Leak.
  3. Selbst-Kopie bei unterschiedlichen Pfad-Aliassen (`./foo.txt` vs `foo.txt`, Symlinks): `oldfile == newfile` prüft nur String-Gleichheit. Zeigen beide Pfade auf dieselbe Datei, öffnet `f2` die Datei mit `FileMode::WRITE` (`wb`), was sie sofort auf 0 Bytes leert – vollständiger Datenverlust!
  *Fix:* RAII-Buffer (`ByteArray` oder `std::vector<char>`), Puffergröße minimal auf 4 KB klemmen, und Same-File-Check via `stat` (Inode/Dev-Check unter POSIX bzw. FileIndex unter Windows):
  ```cpp
  uint64_t bsize = 1024 * 1024;
  if (f1.mysize > 0 && f1.mysize < bsize) bsize = f1.mysize;
  ByteArray buffer(bsize);
  ```

  ==> Beispiel ist quatsch, wenn Quelldatei leer ist, brauche ich keinen riesigen Copy-Buffer zu allokieren, um danach nichts damit zu machen. Wir steigen hier einfach frühzeitig aus und hinterlassen eine neue Datei, die ebenfalls leer ist, ohne eine Exception zu werfen. FIXED

- [ ] **`File::open(..., FileMode::APPEND)` ruft `seek(0)` auf – bricht Append-Semantik** (`src/core/File.cpp:202`, `include/pplib/core/file.h:122`)
  Die Doku in `file.h` verspricht:
  *"Datei zum Anhängen öffnen. Zeiger wird ans Ende der Datei positioniert."*
  In `File::open` steht jedoch bedingungslos:
  ```cpp
  mysize = size();
  seek(0);
  setFilename(filename);
  ```
  Dadurch wird der Zeiger auf den Dateianfang gesetzt (`pos = 0`). Beim ersten Schreibzugriff fügt das OS zwar hinten an, `pos` wird aber ausgehend von 0 inkrementiert (`pos += by * size`). `pos` ist nun vollkommen falsch, und `if (pos > this->mysize) this->mysize = pos;` aktualisiert `mysize` nicht, solange `pos < alter mysize` ist.
  *Fix:* Bei `FileMode::APPEND` nicht `seek(0)` aufrufen, sondern `pos = mysize;` setzen.

  ==> oder "pos=tell()" aufrufen. FIXED

- [ ] **`File::map()`: Integer-Overflow im Bounds-Check und fehlerhafter Pointer bei Re-Use** (`src/core/File.cpp:720-740`)
  1. `if (position + bytes <= mysize)` kann bei großen `position`-Werten überlaufen (`position + bytes < position`).
     Sicherer Check:
     ```cpp
     if (position <= mysize && bytes <= mysize - position)
     ```

     ==> FIXED

  2. Zeile 724: `if (LastMapStart == position && bytes <= LastMapSize) return MapBase;`:
     Wenn `position` zuvor nicht seiten-ausgerichtet war, hat `mmap` intern aligned gemappt (`LastMapStart` ist die Page-Basis) und `MapBase + rest` zurückgegeben. Zeile 724 würde bei exaktem Match `MapBase` (fälschlicherweise ohne Offset!) zurückgeben.
     Korrekt und einheitlich für den gesamten Cache-Hit:
     ```cpp
     if (LastMapProtection == prot && position >= LastMapStart &&
         (bytes + position - LastMapStart) <= LastMapSize) {
         return MapBase + (position - LastMapStart);
     }
     ```
  3. Bei einer 0-Byte-Datei (`mysize == 0`): `map(0, 0)` führt unter Linux zu `mmap(..., size=0)`, was fehlschlägt (`EINVAL`). Unter Windows schlägt `CreateFileMapping` mit `ERROR_FILE_INVALID` (1006) fehl. Dieser Fall sollte sauber abgefangen werden (z.B. Rückgabe von `nullptr` oder Exception).

- [ ] **`File::mmap()`: Diskrepanz zwischen Linux (`MAP_PRIVATE`) und Windows (`PAGE_READWRITE` / shared)** (`src/core/File.cpp:770, 824`)
  Unter Linux nutzt `File::mmap` fest `MAP_PRIVATE`. Bei `MapProtection::READWRITE` erzeugt das OS Copy-on-Write-Pages. Änderungen werden beim Unmap **verworfen** und niemals in die Datei geschrieben!
  Unter Windows wird `PAGE_READWRITE` und `FILE_MAP_WRITE` verwendet, was Änderungen direkt in die Datei schreibt (Shared Mapping).
  *Folge:* Code, der eine Datei per `map(..., MapProtection::READWRITE)` ändert, verhält sich unter Linux und Windows exakt gegensätzlich.
  *Fix:* Für `READWRITE` unter Linux `MAP_SHARED` verwenden (falls Shared-Semantik gewünscht ist) oder den Modus explizit als Shared/Private konfigurierbar machen.

  ==> FIXED
---

## Bugs (mittel)

- [ ] **`File::fread`: Partial Read am EOF wirft keine Exception (Inkonsistenz)** (`src/core/File.cpp:446-452`)
  ```cpp
  if (by < nmemb) {
      if (::ferror((FILE*)ff)) {
          throwErrno(errno != 0 ? errno : EIO, filename());
      }
      if (by == 0 && ::feof((FILE*)ff)) {
          throw EndOfFileException();
      }
  }
  return by;
  ```
  Wenn `0 < by < nmemb` gelesen wird und das Dateiende erreicht ist, kehrt `fread` still zurück, obwohl die Doku sagt: *"Wenn ein Fehler auftritt oder das Dateiende erreicht ist, wird eine Exception geworfen."*
  Zusätzlich: In `fread` steht `if (ptr == NULL) throw IllegalArgumentException();` vor dem Größen-Check `if (size == 0 || nmemb == 0) return 0;`. Bei `fread(NULL, 1, 0)` fliegt daher eine Exception, während `fwrite(NULL, 1, 0)` sauber 0 zurückgibt.

  ==> Hier ist POSIX-Verhalten gewünscht! Prüfen und ggfs. Doku anpassen => FIXED

- [ ] **`File::openTemp`: `(char*)(const char*)tmpname` und fehlende MSVC-Portabilität** (`src/core/File.cpp:209-221`)
  `int f = ::mkstemp((char*)((const char*)tmpname));` castet die `const`-Qualifikation des internen Puffers von `pplib::String` weg. Unter nativem MSVC (Windows) existiert `mkstemp` zudem nicht.
  *Fix:* Temporären `char buffer[1024]` verwenden, diesen an `mkstemp` übergeben und das Ergebnis anschließend dem `String` zuweisen; für Windows/MSVC eine Alternative (`_mktemp_s` / `GetTempFileName`) vorsehen.

  ==> Wenn dann ByteArray verwenden. Es spricht aber auch nix dagegen in den Buffer des Strings zu schreiben. Daher prüfen.
  ==> Unter Windows wird jetzt _wmktemp_s verwendet, der String-Buffer bleibt aber so.

- [ ] **`File::truncate` (Windows): 32-Bit-Truncation durch `(long)length`** (`src/core/File.cpp:652`)
  ```cpp
  int fd = fileno((FILE*)ff);
  if (_chsize_s(fd, (long)length) == 0) { ... }
  ```
  `_chsize_s` akzeptiert `__int64` (64 Bit). Durch den expliziten Cast `(long)length` wird die Länge auf 32 Bit gekürzt (`long` ist auf Windows immer 32-Bit). Dateien > 2 GB werden dadurch korrumpiert!
  *Fix:* `(__int64)length` bzw. `(int64_t)length` verwenden.

  ==> FIXED

- [ ] **`File::sync`: Auf POSIX fehlt `fflush` vor `fsync`** (`src/core/File.cpp:626-630`)
  Unter Windows ruft `File::sync()` vorbildlich erst `::fflush((FILE*)ff)` und dann `_commit(fileno)` auf.
  Unter POSIX wird nur `fsync(fileno((FILE*)ff))` gerufen. Daten, die sich noch im User-Space-Puffer der C-Standardbibliothek (`FILE*`) befinden, werden an das Betriebssystem noch gar nicht übergeben und somit von `fsync` nicht erfasst!
  *Fix:* Auch im POSIX-Branch vor `fsync` `fflush((FILE*)ff)` aufrufen:
  ```cpp
  #ifndef _WIN32
      if (::fflush((FILE*)ff) != 0) throwErrno(errno);
      int ret = fsync(fileno((FILE*)ff));
      if (ret == 0) return;
      throwErrno(errno);
  ```

  ==> FIXED

- [ ] **`fgetws`, `fputws`, `fputwc`, `fgetwc`: Falsche `pos`-Berechnung bei Multibyte/UTF-8** (`src/core/File.cpp:520, 542, 563, 589`)
  Beispiel `fgetws`:
  ```cpp
  uint64_t by = (uint64_t)wcslen(buffer) * sizeof(wchar_t);
  pos += by;
  ```
  Unter Linux ist die Datei üblicherweise UTF-8-kodiert. `fgetws` wandelt UTF-8 in `wchar_t` (4 Bytes) um. Ein ASCII-Zeichen belegt in der Datei 1 Byte, `pos` wird aber um 4 Bytes erhöht! Dadurch desynchronisiert `pos` sofort von der realen Dateiposition, `eof()` meldet verfrüht Dateiende und nachfolgende Seeks/Writes arbeiten mit falschen Werten.
  *Fix:* Nach Wide-Char-Lese-/Schreiboperationen `pos` über `pos = tell();` synchronisieren (außer bei Pipes).
  Zusätzlich: `fgetwc()` gibt am Dateiende `return EOF;` (`-1`) statt `WEOF` zurück.

  ==> FIXED

- [ ] **`File::statFile` vs. `File::tryStatFile`: Inkosistentes Symlink-Verhalten auf POSIX** (`src/core/File.cpp:1198, 1229`)
  `File::statFile` verwendet `::lstat` (Symlinks werden *nicht* dereferenziert, `FileAttr::IFLINK` wird gesetzt).
  `File::tryStatFile` verwendet `::stat` (Symlinks *werden* dereferenziert, `FileAttr::IFLINK` wird *nie* gesetzt).
  Derselbe Pfad liefert über `statFile` und `tryStatFile` unterschiedliche Attribute.
  *Fix:* Beide Funktionen sollten einheitlich `lstat` verwenden (bzw. optional per Parameter steuern, ob Links aufgelöst werden).

  ==> FIXED, Wir rufen stattdessen File::statFile in einem try-catch-block auf

- [ ] **`File::rename` (POSIX): Gefährliches manuelles `unlink` nach angeblichem Erfolg** (`src/core/File.cpp:1008-1025`)
  ```cpp
  if (::rename((const char*)oldfile, (const char*)newfile) == 0) {
      FILE* fd = fopen((const char*)oldfile, "rb"); // Ist die alte Datei noch da?
      if (fd) {
          fclose(fd);
          if (::unlink((const char*)oldfile) == 0) return;
          ...
      }
      return;
  }
  ```
  Auf Case-Insensitive Dateisystemen (z.B. macOS APFS oder NTFS/FAT-Mounts unter Linux) öffnet `fopen(oldfile)` bei einem Rename von `test.txt` zu `TEST.TXT` die Zieldatei! Das anschließende `unlink(oldfile)` löscht die Zieldatei – Datenverlust! Standard-POSIX-`rename` ist atomar; diese Sonderbehandlung (die unter Windows bereits auskommentiert wurde) sollte auch unter POSIX entfernt werden.

  ==> FIXED, Sonderbehandlung unter POSIX entfernt

- [ ] **`FileAttr`: `translate_FileAttr` nutzt `+=` statt `|=`** (`src/core/File.cpp:1046-1057`)
  Beim Mappen von `FileAttr::Attributes` auf `mode_t` wird auf POSIX `+=` statt `|=` verwendet:
  ```cpp
  if (attr & FileAttr::USR_READ) m += S_IRUSR;
  ```
  Wenn Flags kombiniert oder Bits mehrfach ausgewertet werden (oder bei zusammengesetzten Konstanten wie `CHMOD_755`), kann ein arithmetischer Übertrag andere Bits verfälschen.
  *Fix:* Konsequent bitweises `m |= S_IRUSR;` nutzen.

  ==> FIXED

- [ ] **`File::popen`: `fmodepopen` erlaubt `FileMode::READWRITE` (`"r+"`)** (`src/core/File.cpp:148, 172`)
  Weder Windows `_popen` noch POSIX `popen` unterstützen bidirektionale Pipes (`"r+"` führt zu `EINVAL`). Auch die Doku in `file.h` sagt explizit: *"darf das Argument mode nur Lesen oder Schreiben angeben, nicht beides"*.
  `fmodepopen` sollte bei `FileMode::READWRITE` eine `IllegalArgumentException` werfen.

  ==> FIXED

- [ ] **`File::popen` ruft `mysize = size()` auf einer Pipe auf** (`src/core/File.cpp:322`)
  In `popen()` wird `mysize = size();` gerufen. `size()` führt `fstat`/`_fstat` auf dem Pipe-Deskriptor aus. Unter Windows kann `_fstat` auf einer anonymen Pipe fehlschlagen; unter POSIX hat eine Pipe keine sinnvolle Dateigröße.
  *Fix:* Bei `isPopen` `mysize = 0` setzen, ohne `size()` aufzurufen.

  ==> FIXED

- [ ] **`isDir`, `isFile`, `isLink`, `isReadable`, `isWritable`, `isExecutable`: Redundante Syscalls & Race Condition** (`src/core/File.cpp:1282-1329`)
  Alle diese Hilfsfunktionen rufen zuerst `File::exists(filename)` auf und danach `File::statFile(filename, stat)`.
  1. Es werden zwei Syscalls statt einem ausgeführt.
  2. TOCTOU-Race Condition: Wird die Datei zwischen beiden Aufrufen gelöscht, wirft `statFile` eine `FileNotFoundException`, obwohl ein boolsches Prädikat wie `isDir()` einfach `false` liefern sollte.
  *Fix:* Direkt `tryStatFile(filename, stat)` verwenden:
  ```cpp
  bool File::isDir(const String& filename)
  {
      DirEntry stat;
      if (tryStatFile(filename, stat)) {
          return stat.isDir();
      }
      return false;
  }
  ```

  ==> FIXED

- [ ] **`File::getPath`, `getFilename`, `getSuffix`: Randfälle und Doku-Widerspruch** (`src/core/File.cpp:1255-1279`)
  1. `File::getPath("/file")` liefert `""` statt `"/"`.
    ==> Hmm, der erste Slash sollte stehen bleiben => FIXED

  2. Die Doku in `file.h:815` sagt: `"/home/patrick/svn/pplib/README.TXT"` -> `"/home/patrick/svn/pplib/"` (mit Trailing Slash). Der Code und die Tests liefern den Pfad jedoch ohne Slash (`"/home/patrick/svn/pplib"`).
    ==> Doku gefixt

  3. `getSuffix("my.dir/file")` teilt den gesamten Pfad an Punkten auf; `Token.get(-1)` liefert `"dir/file"` als Extension, obwohl die Datei keine Endung hat!
    ==> FIXED

  4. `getSuffix("Makefile")` liefert `"Makefile"` als Extension.
  *Fix:* Zuerst den Dateinamen isolieren (`getFilename`), dann ab dem letzten Punkt suchen. Wenn kein Punkt oder der Punkt am Index 0 steht (z.B. `.gitignore`), leeren String zurückgeben.
    ==> FIXED

- [ ] **Win32-Fehlercodes (`GetLastError()`) werden an `throwErrno` übergeben** (`src/core/File.cpp:669, 690, 710, 847, 857, 1190`)
  In den Windows-Zweigen von `lockExclusive`, `lockShared`, `unlock`, `mmap` und `statFile` wird `GetLastError()` an `throwErrno()` übergeben. `throwErrno()` leitet an `throwExceptionFromErrno` weiter, welches CRT-`errno`-Codes erwartet. Da Win32-Error-Codes und CRT-`errno` unterschiedliche Zahlenräume haben (z.B. Win32 `ERROR_ACCESS_DENIED = 5`, aber `errno 5 = EIO`), führt dies zu völlig irreführenden Exceptions (z.B. `IOErrorException` statt `PermissionDeniedException`).
  *Fix:* Eigene Mapping-Funktion `throwExceptionFromWinError(DWORD err, const String& info)` implementieren.

  ==> FIXED

---

## Doku / Design / Kosmetik

- [ ] **`FileMode::READWRITE_CREATE` nutzt `"w+b"` (Dateien werden geleert!)** (`include/pplib/core/file.h:125`, `src/core/File.cpp:126`)
  Die Doku sagt: *"Datei zum Lesen und Schreiben öffnen. Falls die Datei noch nicht existiert, wird sie erstellt."*
  `"w+b"` trunkiert jedoch existierende Dateien sofort auf 0 Bytes! Wenn beabsichtigt ist, existierende Dateien nicht zu löschen, muss `open(..., O_RDWR | O_CREAT, 0666)` verwendet werden. Falls das Truncate gewollt ist, muss die Doku dringend warnen: *"Achtung: Falls die Datei existiert, wird ihr Inhalt gelöscht."*
  ==> Ist so gewollt, Doku angepasst
- [ ] **Doxygen-Typo in `file.h:346`**: `%fgwets` statt `fgetws`.
  ==> FIXED
- [ ] **Doxygen-Copy-Paste-Fehler in `file.h:362`**: Beschreibung von `fputws` beginnt mit `* %fputs schreibt ...`.
  ==> FIXED
- [ ] **`FileAttr::Attributes`**: Unscoped Enum mit Bitflags, aber ohne überladene Operatoren `|` und `&`. Erfordert überall unschöne explizite Casts wie `(FileAttr::Attributes)(result.Attrib | FileAttr::IFDIR)`.
  ==> FIXED

---

## Befunde in fremden / externen Dateien

Beim Nachvollziehen der Aufrufpfade von `File` wurden folgende Fehler in anderen Projektdateien festgestellt:

1. **`FileObject::md5()` und `FileObject::sha256()` fangen `EndOfFileException` nicht ab** (`src/math/md5.cpp:325`, `src/math/sha256.cpp:211`)
   Wenn `fread()` am Dateiende ankommt und `EndOfFileException` wirft (insbesondere bei Streams / Pipes, bei denen `eof()` nicht vorab `true` liefert), wird die Exception im `catch (...)`-Block gefangen und re-thrown:
   ```cpp
   try {
       while (!eof()) {
           size_t bytes_read = fread(...);
           ...
       }
   } catch (...) {
       seek(oldpos);
       throw; // EndOfFileException bricht Berechnung ab!
   }
   ```
   *Fix:* `catch (const EndOfFileException&) { break; }` vor `catch (...)` einfügen, damit das reguläre Stream-Ende nicht als Fehler gewertet wird.
