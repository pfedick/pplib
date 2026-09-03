# Dir / DirEntry Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/core/dir.h` + `src/core/Dir.cpp` + `src/core/DirEntry.cpp`.
Abhängigkeiten verifiziert: `src/core/File.cpp` (`File::statFile`/`getResultFromStat`, für Symlink-Erkennung),
`src/core/Pcre.cpp` (`RegEx::escape`/`RegEx::compile`, für Wildcard-Matching), `include/pplib/exceptions.h`
(reale Exception-Namen). `Dir::open()` nutzt ausschließlich `std::filesystem` zur Enumeration – es gibt keine
rohen `opendir`/`readdir`/`closedir`- oder `FindFirstFile`/`FindNextFile`-Aufrufe in diesen Dateien.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`Dir::open()`: Dateinamen mit Nicht-ASCII-Zeichen verschwinden lautlos unter Windows** (`Dir.cpp:466`)
  ```cpp
  for (const auto& entry : it) {
      String currentFile = entry.path().string();   // <-- .string()
      DirEntry de;
      try {
          File::statFile(currentFile, de);
          Files.push_back(de);
      }
      catch (...) {
          // Einzelne Dateien ohne Rechte/Zugriff ignorieren
      }
  }
  ```
  `std::filesystem::path::string()` konvertiert unter Windows über die **native ANSI-Codepage**, nicht UTF-8
  (das macht nur `path::wstring()`/`path::u8string()` richtig). An jeder anderen Stelle in derselben Datei
  (z.B. `Dir::mkDir`, Zeile ~225: `String(WideString(accumulated.c_str()))`) wird korrekt der
  UTF-16→UTF-8-Roundtrip über `WideString` gemacht – hier nicht. Für eine Datei mit z.B. Umlauten oder
  kyrillischen Zeichen im Namen liefert `.string()` unter Windows verstümmelte/falsche Bytes. `currentFile`
  wird direkt an `File::statFile()` weitergereicht, das daraus intern wieder ein `WideString` baut und
  `_wstat` aufruft – mit dem falschen (verstümmelten) Pfad schlägt das mit `ENOENT` fehl, die Exception wird
  vom `catch (...)` verschluckt, und die Datei fehlt komplett und ohne jeden Hinweis in `Dir::size()`/
  `Dir::begin()..end()`. Verstößt direkt gegen die REFACTORING.md-Vorgabe "Alle Tests gehen von UTF-8 als
  Locale aus, auch unter Windows".
  Szenario: Verzeichnis mit Datei `"Käse.txt"` unter Windows auflisten → Datei taucht in der `Dir`-Instanz
  nicht auf, ohne Fehlermeldung.
  Fix: wie an anderer Stelle im selben File `String currentFile = String(WideString(entry.path().c_str()));`
  (oder mit C++20 `entry.path().u8string()`).

- [ ] **Symlinks werden nie als solche erkannt, defekte Symlinks verschwinden lautlos aus der Auflistung** (`Dir.cpp:469`, `DirEntry.cpp:47-51`)
  `Dir::open()` befüllt jeden Eintrag über `File::statFile()`, das intern `stat()`/`_wstat()` verwendet
  (`File.cpp:1117`), also dem Symlink **folgt**. `st_mode` von `stat()` kann `S_IFLNK` grundsätzlich nie
  enthalten (das liefert nur `lstat()`) – im gesamten Repo wird `lstat` nirgends verwendet. Damit:
  1. `DirEntry::isLink()` (`DirEntry.cpp:47-51`) liefert **immer** `false`, obwohl das Feature laut Doku
     ausdrücklich unterstützt sein soll (`FileAttr::IFLINK` existiert, `getAttrStr()` behandelt `'l'`).
  2. Ein **defekter/dangling Symlink** lässt `stat()` mit `ENOENT` fehlschlagen → die komplette
     `try { File::statFile(...); } catch (...) {}`-Behandlung in `Dir::open()` (Zeile 468-474) verschluckt
     das und der Eintrag fehlt komplett in der Auflistung, statt (wie z.B. `ls -la` es tut) als kaputter Link
     angezeigt zu werden.
  Szenario: `ln -s /nonexistent broken; ` Verzeichnis mit `Dir::open()` einlesen → `broken` erscheint nicht,
  `Dir::size()` ist um 1 zu niedrig, ohne dass der Aufrufer davon erfährt.
  Fix: In `Dir::open()`/`File::statFile()`-Pfad zusätzlich `lstat()` (bzw. unter Windows
  `FILE_ATTRIBUTE_REPARSE_POINT`) abfragen und bei Symlink `IFLINK` setzen sowie das Link-Ziel separat mit
  Fehlerbehandlung stat'en, statt beim Fehler den ganzen Eintrag zu verwerfen. `std::filesystem::directory_entry`
  liefert dafür bereits günstig `is_symlink()` ohne zusätzlichen Syscall.

- [ ] **`std::filesystem::filesystem_error` kann unbehandelt aus `Dir::open()` entkommen (TOCTOU bei der Enumeration)** (`Dir.cpp:454-475`)
  ```cpp
  auto it = std::filesystem::directory_iterator(fsPath, ec);   // <- mit error_code
  if (ec) { ... }
  for (const auto& entry : it) {                                 // <- Increment OHNE error_code!
  ```
  Nur die Konstruktion von `it` ist gegen Exceptions abgesichert (`ec`-Overload). Die Range-based-for-Schleife
  ruft aber `operator++()` **ohne** `error_code`-Parameter auf; verändert sich das Verzeichnis während der
  Iteration (Datei wird von einem anderen Prozess gelöscht, Netzlaufwerk/NFS-Hänger, etc. – klassisches
  TOCTOU zwischen Öffnen und Lesen), wirft dieser Aufruf ein `std::filesystem::filesystem_error`. Diese Klasse
  erbt **nicht** von `pplib::Exception` (nur von `std::exception` über `std::system_error`), sondern ist ein
  komplett anderer Exception-Typ als der Rest der Bibliothek. Ein Aufrufer, der wie überall sonst in PPLIB
  `catch (const pplib::Exception&)` verwendet, fängt das nicht ab → im schlimmsten Fall `std::terminate()`.
  Fix: Iteration ebenfalls mit der `error_code`-Variante durchführen (`it.increment(ec)` in einer klassischen
  Schleife statt Range-for), oder die gesamte Schleife in einen zusätzlichen `try/catch` einwickeln, der
  `std::filesystem::filesystem_error` gezielt in eine pplib-Exception übersetzt.

## Bugs (mittel)

- [ ] **`Dir::exists()` widerspricht der eigenen Dokumentation – liefert `false` für normale Dateien** (`Dir.cpp:171-184`)
  ```cpp
  bool Dir::exists(const String& dirname)
  {
      try {
          DirEntry f;
          File::statFile(dirname, f);
          if (f.isDir()) return true;
          if (f.isLink()) return true;
          return false;
      }
      catch (...) { return false; }
  }
  ```
  Header-Doku (`dir.h:576-582`): "Dabei wird nicht geprüft, ob es sich um ein Verzeichnis oder eine Datei
  handelt. Es wird lediglich geprüft, ob der Pfad existiert." Der Code tut aber genau das Gegenteil: Für einen
  Pfad, der eine reguläre Datei ist (z.B. `Dir::exists("/etc/passwd")`), liefert die Funktion `false`, obwohl
  der Pfad offensichtlich existiert. (Der `isLink()`-Zweig ist wegen des Bugs oben ohnehin nie erreichbar.)
  Wird intern in `Dir::mkDir()` als Vorab-Check verwendet – dort ist die Auswirkung harmlos, weil das
  nachfolgende `mkdir()` bei einer existierenden Datei ohnehin mit einem OS-Fehler abbricht, aber als
  öffentliche API mit diesem Namen und dieser Doku ist das Verhalten irreführend.
  Fix: entweder Doku an das tatsächliche Verhalten anpassen (Funktion effektiv umbenennen/dokumentieren als
  "ist das ein Verzeichnis oder Link"), oder die Datei-Prüfung ergänzen (`isFile()`) bzw. generell nur auf
  Existenz prüfen unabhängig vom Typ.

- [ ] **`Dir::mkDir(..., recursive=true)`: TOCTOU-Race erzeugt spontane `FileExistsException`** (`Dir.cpp:236-248`)
  ```cpp
  if (!Dir::exists(currentPathStr)) {
  #ifdef _WIN32
      ...
      if (_wmkdir(...) != 0) throwExceptionFromErrno(errno, currentPathStr);
  #else
      if (mkdir((const char*)currentPathStr, mode) != 0) {
          throwExceptionFromErrno(errno, currentPathStr);
      }
  #endif
  }
  ```
  Klassisches Check-then-Act: Zwischen `Dir::exists(...)` und `mkdir(...)` kann ein anderer Thread/Prozess
  genau dasselbe Zwischenverzeichnis anlegen. `mkdir()` liefert dann `EEXIST`, was `throwExceptionFromErrno`
  in eine `FileExistsException` übersetzt (`Exceptions.cpp:209-210`) – obwohl das eigentliche Ziel ("Pfad
  existiert danach") erreicht ist. Konkretes Szenario: zwei Threads rufen gleichzeitig
  `Dir::mkDir("/var/cache/app/session123", true)` auf (z.B. zwei parallele Requests, die denselben
  Cache-Unterordner anlegen wollen) → einer der beiden bekommt eine Exception, obwohl kein echter Fehler
  vorliegt.
  Fix: `EEXIST` beim rekursiven Anlegen tolerieren, z.B.
  `if (mkdir(...) != 0 && errno != EEXIST) throwExceptionFromErrno(...)`.

- [ ] **Wildcard→RegEx-Konvertierung escaped nicht alle Regex-Metazeichen** (`Dir.cpp:362-397` `filterPattern`, `Dir.cpp:399-418` `findPattern`; `Pcre.cpp:438-452` `RegEx::escape`)
  ```cpp
  String Pattern = RegEx::escape(pattern);   // escaped nur: - + \ * /
  Pattern.replace(".", "\\.");
  Pattern.replace("\\*", ".*");
  Pattern.replace("?", ".");
  Pattern = "^" + Pattern + "$";
  ```
  `RegEx::escape()` escaped laut eigenem Kommentar nur `- + \ * /`. Zeichen wie `[ ] ( ) { } ^ $ |`, die in
  echten Dateinamen durchaus vorkommen (z.B. `"[Draft] Report.txt"`, `"(final).docx"`), bleiben als aktive
  Regex-Metazeichen stehen. Konkretes Beispiel: `filterPattern("[Draft]*.txt")` soll Dateien finden, die mit
  dem literalen Präfix `"[Draft]"` beginnen; nach der Konvertierung entsteht aber das Regex `^[Draft].*\.txt$` –
  `[Draft]` ist dort eine **Zeichenklasse**, die genau ein Zeichen aus `{D,r,a,f,t}` matcht, nicht die
  7-Zeichen-Folge `"[Draft]"`. Die Funktion liefert damit für den eigentlich beabsichtigten Anwendungsfall
  falsche/keine Treffer. Ein unbalanciertes Bracket im Pattern (z.B. `"[Draft*.txt"`) lässt `RegEx::compile`
  sogar mit `IllegalRegularExpressionException` abbrechen, statt dass die Wildcard-Suche einfach "kein
  Treffer" liefert.
  Fix: eigene Escape-Logik für Wildcard→Regex verwenden, die alle PCRE-Metazeichen escaped
  (`. ^ $ | ( ) [ ] { } * + ? \`), bevor `*`/`?` gezielt wieder durch `.*`/`.` ersetzt werden – `RegEx::escape()`
  ist dafür nicht das richtige Werkzeug, da es für einen anderen Zweck (Perl-Style-Escaping) gedacht ist.

- [ ] **`Dir::currentPath()`/`Dir::tempPath()`: gleicher Encoding-Bug wie oben, plus inkonsistentes Trailing-Separator-Verhalten** (`Dir.cpp:63-78`, `Dir.cpp:107-124`)
  ```cpp
  String Dir::currentPath()
  {
      auto path = std::filesystem::current_path(ec);
      if (!ec) return String(path.string());     // <- .string(), gleiches Problem wie oben
      ...
  }
  String Dir::tempPath()
  {
      auto path = std::filesystem::temp_directory_path(ec);
      if (!ec) return String(path.string()).trimRight("/");   // <- trimmt nur "/", nicht "\\"
      ...
  #ifdef _WIN32
      if (const wchar_t* tmp = _wgetenv(L"TEMP")) return String(tmp).trimRight("\\");
  ```
  Enthält das aktuelle Arbeitsverzeichnis bzw. der Temp-Pfad unter Windows Nicht-ASCII-Zeichen, liefert
  `path.string()` (ANSI-Codepage) potenziell falsche Bytes zurück. Zusätzlich: `temp_directory_path()` liefert
  unter Windows meist einen Pfad **mit** abschließendem `\`; der Erfolgspfad trimmt aber nur `"/"`, während der
  Fallback-Pfad direkt darunter explizit `"\\"` trimmt – je nachdem, ob der `std::filesystem`-Aufruf klappt
  oder nicht, bekommt der Aufrufer einen Pfad mit oder ohne trailing Backslash zurück.
  Fix: analog zum Rest der Datei über `WideString` roundtrippen; Trim-Zeichensatz einheitlich `"/\\"` verwenden.

- [ ] **`Path.trimRight("\\")` wird auch auf Linux/FreeBSD ausgeführt** (`Dir.cpp:437-438` `open()`, `Dir.cpp:484-485` `canOpen()`)
  ```cpp
  Path = path.trimmed();
  Path.trimRight("/");
  Path.trimRight("\\");   // <- kein #ifdef _WIN32
  ```
  Unter POSIX ist `\` ein ganz normales, gültiges Dateinamenszeichen (kein Pfadtrenner). Ein Verzeichnis, das
  legitim mit einem literalen `\` endet (z.B. durch ein Samba-Share oder absichtlich erzeugt), würde beim
  Öffnen über `Dir::open()`/`Dir::canOpen()` diesen abschließenden Backslash stillschweigend abgeschnitten und
  ggf. ein anderes (oder gar nicht existierendes) Verzeichnis geöffnet. Randfall mit geringer
  Eintrittswahrscheinlichkeit, aber ein klarer Verstoß gegen "Pfadtrenner-Behandlung Windows vs. Linux sauber
  trennen".
  Fix: das `trimRight("\\")` in ein `#ifdef _WIN32` verschieben.

## Design

- [ ] **Blanket `catch (...)` in `Dir::open()` maskiert alle Fehler unterschiedslos** (`Dir.cpp:468-474`)
  Jede Exception aus `File::statFile()` – egal ob `PermissionDeniedException` (erwartet/harmlos) oder z.B.
  `OutOfMemoryException` oder die oben beschriebenen Encoding-/Symlink-Bugs – wird kommentarlos verschluckt.
  Ein `Dir`-Objekt kann dadurch eine unvollständige Liste liefern, ohne dass der Aufrufer eine Möglichkeit hat,
  das zu erkennen (kein Zähler für übersprungene Einträge, kein Logging-Hook). Zumindest ein optionaler
  Fehler-Sammel-Mechanismus (`size_t skippedCount()` o.ä.) wäre hilfreich, oder gezielteres Catching
  (`PermissionDeniedException`/`FileNotFoundException` explizit, alles andere weiterreichen).

- [ ] **Vierfache Code-Duplikation bei den Filterfunktionen** (`Dir.cpp:362-430`)
  `filterPattern`/`findPattern` enthalten identische 6-zeilige Wildcard→Regex-Konvertierung,
  `filterRegExp`/`findRegExp` identische Match-Schleifen. Alle vier ließen sich über den bereits vorhandenen
  generischen `filter()`-Template (`dir.h:461`) und eine private `static String wildcardToRegex(const String&, bool)`-
  Hilfsfunktion auf ca. ein Drittel des Codes reduzieren – und der Wildcard-Bug oben müsste dann nur an einer
  Stelle gefixt werden.

- [ ] **`Dir::homePath()` (POSIX-Fallback) nutzt `getpwuid()`, nicht threadsicher** (`Dir.cpp:99`)
  ```cpp
  if (struct passwd* pw = getpwuid(getuid()); pw && pw->pw_dir) {
  ```
  `getpwuid()` liefert einen Zeiger auf einen statischen, nicht reentranten Puffer (POSIX). Läuft ein anderer
  Thread zeitgleich eine andere `getpwuid`/`getpwnam`/`getgrnam`-Familie-Funktion, kann der Puffer überschrieben
  werden, bevor `pw->pw_dir` gelesen wird. Für eine Bibliothek, die explizit Multithreading unterstützen soll,
  wäre `getpwuid_r()` die sauberere Wahl.

## Doku / Kosmetik

- [ ] `Dir(const String& path, Sort sortOrder)`-Konstruktor dokumentiert eine Exception, die es nicht gibt
      (`dir.h:290`): `@exception NonexistingPathException` – in `exceptions.h` existiert keine Klasse dieses
      Namens. Der tatsächliche Code (`Dir::open`) wirft `FileNotFoundException`, `PermissionDeniedException`
      oder `CouldNotOpenDirectoryException`. Doku entsprechend korrigieren.

- [ ] Beide `mkDir()`-Überladungen (`dir.h:606`, `dir.h:619`) dokumentieren `@return Liefert true/false zurück`,
      die Funktionen sind aber als `void` deklariert (Leftover einer älteren bool-basierten API).

- [ ] `DirEntry::ATime` verwendet den Doxygen-Tag `@authors` statt `@brief` (`dir.h:68`) – Copy-Paste-Fehler,
      wird in der generierten Doku vermutlich als Autorenliste statt als Beschreibung interpretiert.

- [ ] `Dir::homePath()`: lokale Variable heißt `homePath` und verdeckt damit den Namen der umschließenden
      statischen Memberfunktion (`Dir.cpp:89`) – syntaktisch unproblematisch, aber verwirrend beim Lesen/
      Refactoring. Umbenennen (z.B. `homePathEnv`).

- [ ] `DirEntry::getAttrStr()` rendert kein Sticky-Bit (`FileAttr::STICKY`/`ISVTX` existiert, wird hier aber
      nicht abgefragt), und überschreibt bei `ISUID`/`ISGID` das `x`/`-` an Position 3/6 immer mit `'s'`,
      unabhängig davon, ob das jeweilige Execute-Bit gesetzt ist – `ls` unterscheidet hier klein/groß (`s`/`S`,
      `t`/`T`). Rein kosmetisch, aber inkonsistent mit der Unix-üblichen Darstellung, die die Funktion laut
      Kommentar nachbilden will.

- [ ] `Dir::applicationDataPath()` deklariert `String path;`, die nie verwendet wird (`Dir.cpp:128`) – toter Code.

- [ ] `Dir::canOpen()`: `if (ec == ...) return false; else if (ec == ...) return false; else return false;`
      (`Dir.cpp:503-509`) – alle drei Zweige tun dasselbe, lässt sich auf `if (ec) return false;` reduzieren.

## Verifiziert OK (kein Handlungsbedarf)

- Keine rohen Verzeichnis-Handles im Spiel: `Dir::open()`/`canOpen()`/`rmDir()` verwenden ausschließlich
  `std::filesystem` (RAII-basiert), `File::statFile()` öffnet kein persistentes Handle (nur `stat`/`_wstat`).
  Es gibt daher keinen exceptionpfad-abhängigen Leak wie bei klassischem `opendir`/`readdir`/`closedir`.
- Copy-/Move-/Zuweisungssemantik von `Dir` und `DirEntry`: beide bestehen ausschließlich aus Value-Type-Membern
  (`String`, `DateTime`, `std::vector<DirEntry>`, PODs/Enums) ohne eigene Ressourcenverwaltung (kein Rohzeiger,
  kein manuelles `new`/`delete`) → die compilergenerierten Copy-/Move-Operationen (Rule of Zero) sind korrekt,
  Self-Assignment (`a = a;`) ist unproblematisch.
- `DirEntry::getAttrStr()`: alle `attrStr.set(index, ...)`-Aufrufe liegen im Bereich `[0,9]` des 10 Zeichen
  langen `"----------"`-Strings – kein Out-of-Bounds.
- `Dir::mkDir`/`Dir::rmDir` (nicht-rekursiver Pfad) und `Dir::rmDir` (rekursiv, über
  `std::filesystem::remove_all`) übersetzen `std::error_code` korrekt in passende pplib-Exceptions, ohne dabei
  selbst `std::filesystem::filesystem_error` durchzulassen (im Gegensatz zum Iterations-Bug in `Dir::open()`
  oben).
