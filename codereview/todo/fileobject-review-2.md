# File / FileObject Review (Zweiter Durchgang) – Befunde

Zweit-Review nach `todo/fileobject-review.md` (Erstreview vom 2026-08-28, Scope damals nur `fileobject.h`/`FileObject.cpp`).
Scope hier: `include/pplib/core/file.h` + `src/core/File.cpp`, `include/pplib/core/fileobject.h` + `src/core/FileObject.cpp`
(alle vier Dateien vollständig und unabhängig vom alten Dokument neu gelesen), dazu `src/core/MemFile.cpp` +
`include/pplib/core/memfile.h` vollständig gelesen (zur Prüfung der dort gemeldeten Punkte sowie – da die Datei ohnehin
komplett gelesen wurde – mit Meldung offensichtlicher Zusatzbefunde, die direkt mit den `FileObject`/`File`-Aufrufpfaden
zusammenhängen). Cross-referenziert: `include/pplib/exceptions.h` (alle zitierten Exception-Namen real vorhanden:
`FileSeekException`, `OverflowException`, `FileNotOpenException`, `EndOfFileException`, `IllegalArgumentException`,
`ReadOnlyException`, `BufferExceedsLimitException`, `OperationUnavailableException`, `IllegalOperationOnPipeException`
(ESPIPE), `UnknownException`), `include/pplib/types/bytearrayptr.h`, `include/pplib/types/bytearray.h`.

**Kalibrierung Tiefe/Rigor**: da `todo/bytearray-review.md` im Repo nicht (mehr) existiert, wurde stattdessen
`todo/array-review.md` (gründlichstes vorhandenes Review, inkl. Self-Aliasing-Repros gegen die echte Bibliothek) sowie
`todo/assocarray-review-2.md` (Format eines Zweit-Reviews) als Referenz für Tiefe und Format herangezogen.

**Empirisch verifiziert** gegen `build/coverage/libpplib_core.a` (g++ 15, C++17/20). Da die vorgebaute `.a` selbst NICHT
mit ASan instrumentiert ist (nur `--coverage`), wurden `File.cpp`, `FileObject.cpp` und `MemFile.cpp` zusätzlich separat
mit `-fsanitize=address` neu übersetzt und deren `.o`-Dateien beim Linken vor das Archiv gestellt, damit
Speicherzugriffe *innerhalb* dieser drei Dateien tatsächlich von ASan geprüft werden (reines Malloc/Memmove-Interception
ohne TU-Instrumentierung hätte die gefundenen Out-of-Bounds-Reads sonst nicht angezeigt – das wurde an einem Kontrollfall
explizit verifiziert). Zehn Testprogramme (`t1.cpp` … `t10.cpp`) liegen unter
`/tmp/claude-1000/-home-patrickf-git-ppl8/d6f044e8-1b6c-4022-9e6b-90280b45f5ca/scratchpad/`. Auffällig: für `MemFile` existiert
im gesamten Repo **keine einzige Unit-Test-Datei** (`tests/src/core/` enthält nur `file.cpp`/`filestatic.cpp`, kein
`memfile.cpp`) – das erklärt plausibel, warum die unten gefundenen, teils katastrophalen `MemFile`-Bugs nie auffielen.

## Status der bisherigen Funde

| # | Punkt aus altem Review | Status jetzt |
|---|---|---|
| 1 | `MemFile::seek(int64_t,SeekOrigin)`: SEEKEND offset=0 wird abgelehnt | **Noch offen, aber weit gravierender als beschrieben** – siehe neuer Fund „SEEKSET/SEEKEND komplett invertiert" unten. Alter Befund war nur die Spitze des Eisbergs. |
| 2 | `MemFile::seek(uint64_t)`: Off-by-one, Seek exakt aufs Dateiende (Append-Fall) wird abgelehnt | **Noch offen.** Empirisch bestätigt: `seek((uint64_t)10)` auf einem 10-Byte-Buffer wirft `OverflowException`. |
| 3 | `MemFile::fgetws`: OOB-Read am Dateiende, fehlender `num<1`-Check | **Noch offen.** Empirisch mit ASan bestätigt: Heap-buffer-overflow in `MemFile.cpp:386` (siehe unten). `num<1`-Check weiterhin nicht vorhanden. |
| 4 | `MemFile::fgetc`: OOB-Read bei `pos==mysize` (`>` statt `>=`) | **Noch offen.** Empirisch mit ASan bestätigt: Heap-buffer-overflow in `MemFile.cpp:435`. |
| 5 | `FileObject::getws` nahm `String&` statt `WideString&` | **Behoben.** Header deklariert jetzt `int getws(WideString& buffer, size_t num = 1024)` + `WideString getws(size_t num = 1024)`, symmetrisch zu `putws(const WideString&)`. |
| 6a | `copyFrom(FileObject&, uint64_t)`: Rückgabewert = angefragte statt kopierter Bytes | **Behoben.** `copyFrom` führt jetzt einen eigenen `copied`-Zähler und gibt ihn zurück. Empirisch bestätigt: Anfrage von 20 Bytes aus einer 5-Byte-Quelle liefert korrekt `5`. |
| 6b | `copyFrom`: Infinite-Loop-Risiko bei `fread`-Rückgabe 0 ohne Exception | **Gegenstandslos/entschärft**, aber neues Problem entstanden: `File::fread` wirft bei totalem EOF (`by==0`) jetzt eine `EndOfFileException` statt `0` zurückzugeben – die Infinite-Loop ist damit ausgeschlossen, aber die Exception ist in `copyFrom` **uncaught** und propagiert ungefiltert nach außen (siehe neuer Fund „`available`-Underflow"). |
| 6c | `copyFrom`: Self-Copy = stiller No-Op | **Noch offen, aber schädlicher als beschrieben** – kein No-Op, sondern echte, unbeabsichtigte Datenkorruption. Siehe neuer Fund unten mit konkretem Byte-für-Byte-Nachweis. |
| 6d | `copyFrom`: Pipes/`popen` als Quelle umgehen die `size()`-Guard | **Verhalten geändert (entschärft):** `copyFrom` ruft intern `quellfile.tell()` auf; bei einem `File`, das über `popen()` geöffnet wurde, wirft `File::tell()` (`ftello` auf einer Pipe) sofort `IllegalOperationOnPipeException` (ESPIPE) – kein Silent-Fail mehr. Pipes sind mit `copyFrom` dadurch aber komplett unbenutzbar (jeder Aufruf wirft), was als Design-Einschränkung bestehen bleibt. |
| 7 | `File::fread`: Partial Read am EOF (`0 < by < nmemb`) wirft keine Exception | **Noch offen** (nur der Fall `by==0` wurde ergänzt). Empirisch bestätigt: `fread(buf,1,10)` auf einer 7-Byte-Datei liefert `7` zurück, keine Exception. |
| 8 | `FileObject::read(ByteArray&)`: zerstört Target vor Open-Prüfung | **Noch offen**, nur die Implementierung hat sich geändert (`realloc` statt `free`+`malloc`, dazu ein Kurzschluss falls die Zielgröße zufällig schon passt). Der eigentliche Fehler – kein `isOpen()`-Check vor der destruktiven Operation – wurde nicht behoben. Empirisch bestätigt: Ziel-`ByteArray` mit 27 Nutzbytes wird auf 100 (Garbage-)Bytes umallokiert, obwohl anschließend `FileNotOpenException` fliegt. |
| 9 | `FileObject::load()`: liefert `NULL` statt Exception | **Gegenstandslos durch Refactoring.** `load()` gibt inzwischen ein `ByteArray` *by value* zurück (kein Pointer mehr) und kann daher kein `NULL` mehr liefern; die Doku verspricht auch keine Exception mehr für diesen Fall. Es bleibt aber ein Restrisiko: ein „silent partial read" (Punkt 7) wird von `load()` klaglos auf die tatsächlich gelesene Länge gekürzt, statt eine Exception zu werfen – siehe Design-Abschnitt unten. |
| 10 | `MemFile::map`/`mapRW`: `NULL` statt Exception bei OOB; `position+bytes` kann overflowen | **Noch offen**, beide Teile empirisch bestätigt: `map(10,10)` auf einem 16-Byte-Buffer liefert `NULL`; `map(UINT64_MAX-2, 10)` (Overflow von `position+bytes` auf `7 <= mysize`) liefert einen nicht-NULL, wilden Pointer außerhalb des Buffers. |
| 11 | `MemFile::openReadWrite`: übernimmt Ownership ohne `@attention`-Hinweis | **Noch offen**, Doku-Kommentar unverändert – nur der Konstruktor hat den Hinweis, `openReadWrite` nicht. |
| 12 | `MemFile::fwrite`: Overflow bei `nmemb*size`; „nie geöffnet"-Zustand rutscht durch | **Noch offen**, Code unverändert (`size_t bytes = nmemb*size;` ohne Vorab-Check; Default-konstruiertes `MemFile` mit `MemBase==NULL, readonly==false` lässt `fwrite` weiterhin nur „zufällig" über `resizeBuffer` funktionieren). |
| 13 | `write(ByteArrayPtr&, bytes=0)` wirft bei leerem Objekt | **Uneinheitlich behoben.** Für `File` weiterhin bestätigt bestehend (`File::fwrite` prüft `ptr==NULL` *vor* `nmemb==0` → `IllegalArgumentException`). Für `MemFile` inzwischen (zufällig, da `MemFile::fwrite` gar keinen `ptr==NULL`-Check besitzt) kein Fehler mehr – die beiden `FileObject`-Subklassen verhalten sich jetzt unterschiedlich für denselben API-Aufruf. |
| 14 | Include-Guard-Kommentar-Mismatch in `fileobject.h` | **Noch offen**, unverändert (`#ifndef PPLIB_CORE_FILES_H_` vs. `#endif /* PPLIB_CORE_THREADS_H_ */`). Zusätzlich: derselbe Fehlertyp jetzt auch in `file.h` gefunden (siehe Doku/Kosmetik). |
| 15 | Typos in Doxygen (`%fgwets`, „Ausgahgsbasis", „miitels", „solle" …) | **Noch offen**, unverändert; dieselben Typos zusätzlich in `file.h` dupliziert (z.B. `%fgwets` in der `fgetws`-Doku von `file.h:346`). |
| 16 | `load()`/`load(ByteArray&)`: `seek(0)`-Seiteneffekt undokumentiert | **Noch offen**, unverändert. |
| 17 | `gets()`: Locale-basierte Konvertierung passt nicht zu UTF-8-Ziel | Nicht erneut vertieft geprüft (kein Code-Unterschied ersichtlich, weiterhin als offene Design-Frage zu betrachten). |

## Bugs (kritisch)

- [ ] **`MemFile::seek(int64_t offset, SeekOrigin origin)`: SEEKSET und SEEKEND haben komplett invertierte Grenzprüfung – gültige Seeks schlagen fehl, ungültige gelingen und setzen `pos` außerhalb des Buffers** (`src/core/MemFile.cpp:288-322`)

  ```cpp
  case SEEKSET:
      pos = offset;
      if ((int64_t)pos < 0) { pos = 0; return pos; }
      if (pos > mysize) return pos;      // <- "Erfolg", wenn pos AUSSERHALB des Buffers liegt!
      break;                              // <- Absturz in throw, wenn pos GÜLTIG ist!
  ```
  Gleiches Muster bei `SEEKEND` (`pos = mysize - offset; if (pos > mysize) return pos; ...`). Empirisch mit einem
  10-Byte-Buffer nachgewiesen (Testprogramm `t10.cpp`):

  | Aufruf | Erwartet | Tatsächlich |
  |---|---|---|
  | `seek(0, SEEKSET)` | OK, pos=0 | **`FileSeekException`** |
  | `seek(5, SEEKSET)` | OK, pos=5 | **`FileSeekException`** |
  | `seek(10, SEEKSET)` | OK, pos=10 (Ende) | **`FileSeekException`** |
  | `seek(15, SEEKSET)` | Fehler (außerhalb) | **OK, pos=15** (!) |
  | `seek(0, SEEKEND)` | OK, pos=10 | **`FileSeekException`** |
  | `seek(5, SEEKEND)` | OK, pos=5 | **`FileSeekException`** |
  | `seek(-5, SEEKEND)` | Fehler (vor Dateianfang… hier: hinter Ende) | **OK, pos=15** (!) |

  Damit ist `SEEKSET` für `MemFile` faktisch komplett unbenutzbar (jede normale, gültige Position wirft eine Exception),
  während ein Seek weit hinter das Dateiende **klaglos gelingt** und `pos` auf einen Wert > `mysize` setzt. Der nächste
  `fread`/`fwrite`/`fgetc` auf dieser Position ist dann katastrophal: in `MemFile::fread` wird
  `by = (size_t)(mysize - pos) / size` berechnet – bei `pos > mysize` unterläuft `mysize - pos` als `size_t` und ergibt
  einen Wert nahe `SIZE_MAX`. Empirisch reproduziert (Testprogramm `t2.cpp`, ASan):
  ```
  ==ERROR: AddressSanitizer: negative-size-param: (size=-10)
      #1 pplib::MemFile::fread(...) MemFile.cpp:337
  ```
  Ein `seek(20, SEEKSET)` gefolgt von einem einzigen `fread(buf, 1, 5)` auf einem 10-Byte-Buffer stürzt sofort ab
  (ohne ASan: nicht-deterministischer Absturz oder massives Auslesen fremden Speichers).
  Fix: Die Bedingung ist in allen drei `switch`-Zweigen umzudrehen – „Erfolg" muss der `0 <= pos <= mysize`-Fall sein,
  nicht der Fall außerhalb. Am saubersten: komplette Neuimplementierung der Funktion (analog zu `File::seek`, das diesen
  Bug nicht hat), z.B.:
  ```cpp
  uint64_t MemFile::seek(int64_t offset, SeekOrigin origin)
  {
      if (MemBase == NULL && readonly) throw FileNotOpenException();
      int64_t newpos = 0;
      switch (origin) {
      case SEEKSET: newpos = offset; break;
      case SEEKCUR: newpos = (int64_t)pos + offset; break;
      case SEEKEND: newpos = (int64_t)mysize + offset; break;
      default: throw IllegalArgumentException();
      }
      if (newpos < 0 || (uint64_t)newpos > mysize) throw FileSeekException(...);
      pos = (size_t)newpos;
      return pos;
  }
  ```
  (`SEEKCUR` ist von diesem Totalausfall nicht betroffen, hat aber weiterhin den bereits im alten Review vermerkten
  Off-by-one bei `offset==mysize-pos` – siehe Tabelle oben, Punkt 2/Zeile `SEEKCUR offset=10`.)

- [ ] **`FileObject::copyFrom()`: Self-Copy erzeugt keine bloße No-Op, sondern echte, unbemerkte Datenkorruption** (`src/core/FileObject.cpp:105-130`)
  Bei `obj.copyFrom(obj, quelloffset, bytes, zieloffset)` überschreibt der zweite `seek()`-Aufruf (`seek(zieloffset)`)
  die zuvor gesetzte Quellposition (`quellfile.seek(quelloffset)`) auf demselben Objekt – der `quelloffset` geht komplett
  verloren. Die anschließende Kopierschleife liest und schreibt dadurch **beide** ab `zieloffset`, wobei jeder
  `read()`+`write()`-Schritt die Position weiterschiebt: es werden fortlaufend bereits gelesene Bytes direkt dahinter
  noch einmal eingefügt. Konkret nachgewiesen (Testprogramm `t1.cpp`) an einem 37-Byte-Buffer
  `"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"` mit `copyFrom(this, quelloffset=0, bytes=5, zieloffset=20)`:
  ```
  vorher:  ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789
  nachher: ABCDEFGHIJKLMNOPQRSTUVWXYUVWXY456789
                               ^^^^^ ^^^^^
                          (Position 20-24, unverändert) kopiert nach 25-29
  ```
  Statt der erwarteten Kopie von `"ABCDE"` (Position 0-4) nach Position 20 werden die Bytes `"UVWXY"` (Position 20-24,
  zufällig die aktuelle Position nach dem verlorenen Seek) nach Position 25-29 dupliziert – die Bytes `"Z0123"` an
  dieser Stelle gehen unbemerkt verloren. Die Funktion meldet dabei `copied=5`, also **Erfolg**, obwohl weder die
  angeforderte Quelle noch das Ziel tatsächlich bedient wurden.
  Fix: Am Anfang von `copyFrom(quellfile, quelloffset, bytes, zieloffset)` prüfen, ob `&quellfile == this`; in diesem
  Fall entweder explizit ablehnen (`throw IllegalArgumentException()`, da ein self-overlapping copy mit dieser
  Streaming-Implementierung ohnehin nicht sicher generisch lösbar ist) oder über einen Zwischenpuffer/eine Kopie realisieren.

## Bugs (mittel)

- [ ] **`FileObject::copyFrom()`: Integer-Underflow der `available`-Berechnung, wenn `quellfile.tell() > quellfile.size()`** (`src/core/FileObject.cpp:112-116`)
  ```cpp
  uint64_t available = quellfile.size() - quellfile.tell();
  if (bytes > available) bytes = available;
  ```
  Für eine reguläre Datei ist ein Seek über das Dateiende hinaus mit `File::seek(uint64_t)` erlaubt (kein Bounds-Check,
  siehe `File::seek`), `tell()` liefert dann einen Wert `> size()`. `available` unterläuft dadurch (uint64_t) zu einem
  Wert nahe `UINT64_MAX`; der Clamp `if (bytes > available) bytes = available;` greift somit nie. Die Kopierschleife
  versucht daraufhin, weit hinter dem Dateiende zu lesen; `File::fread` liefert `by==0` bei erreichtem EOF und wirft
  seit dem Fix aus Punkt 6b/7 der Tabelle oben eine `EndOfFileException` – die aber in `copyFrom` **nicht abgefangen**
  wird und ungefiltert nach außen propagiert. Empirisch bestätigt (Testprogramm `t9.cpp`):
  ```cpp
  File src("small.bin" /* 5 Bytes */, File::FileMode::READ);
  src.seek((uint64_t)100);       // gültig laut File::seek, auch wenn Datei nur 5 Bytes hat
  dst.copyFrom(src, 20);          // wirft EndOfFileException statt geordnet 0 zurückzugeben
  ```
  Das ist zwar (anders als die zwei kritischen Bugs oben) speichersicher, aber ein Vertragsbruch: `copyFrom` dokumentiert
  „Bei Erfolg liefert die Funktion die Anzahl kopierter Bytes zurück", nicht eine Exception für einen an sich harmlosen
  Randfall (Quelle bereits am/hinter dem Ende).
  Fix: `available` mit Sättigung statt Subtraktion berechnen: `uint64_t available = (t >= s) ? 0 : s - t;` (mit
  `s=quellfile.size()`, `t=quellfile.tell()`), oder generell prüfen `if (quellfile.tell() >= quellfile.size()) return 0;`
  vor der Berechnung.

- [ ] **`FileObject::load()` / `load(ByteArray&)` liefern für Pipes/`popen`-Handles immer ein leeres Ergebnis, auch wenn Daten anstehen** (`src/core/FileObject.cpp:200-233`)
  ```cpp
  ByteArray FileObject::load()
  {
      uint64_t s = size();
      ByteArray ba;
      if (s == 0) return ba;   // <- für eine Pipe IMMER wahr, unabhängig vom tatsächlichen Inhalt
      ...
  }
  ```
  `File::size()` liefert für `ff` grundsätzlich `st_size` aus `fstat`; bei einer über `File::popen()` geöffneten Pipe
  ist das systembedingt `0`, unabhängig davon, wie viele Bytes tatsächlich zum Lesen bereitstehen. `load()`/`load(ByteArray&)`
  brechen daher sofort mit einem leeren Ergebnis ab, ohne überhaupt einen Leseversuch zu unternehmen. Empirisch bestätigt
  (Testprogramm `t3.cpp`):
  ```cpp
  File p; p.popen("echo hello12345", File::FileMode::READ);
  p.size();              // -> 0
  p.FileObject::load();  // -> leeres ByteArray, obwohl "hello12345\n" im Pipe-Puffer wartet
  ```
  Anders als bei `copyFrom` (siehe Tabelle, Punkt 6d) gibt es hier keinen `tell()`/`seek()`-Aufruf, der den Fehler durch
  eine Exception sichtbar machen würde – das Ergebnis ist ein stiller, nicht von einer echten leeren Datei
  unterscheidbarer Datenverlust für jeden Aufrufer, der `load()` auf einem `popen`-`File` verwendet.
  Fix: Für nicht-reguläre Handles (Pipe erkennbar über das interne `isPopen`-Flag) entweder explizit
  `UnsupportedFileOperationException` werfen, oder `load()` auf ein Read-until-EOF-Verfahren umstellen, das nicht auf
  `size()` angewiesen ist.

- [ ] **`File::close()`: Ein per `popen()` gestartetes Kommando, das mit Exit-Code ≠ 0 endet, führt zu einer bedeutungslosen `UnknownException("Success")`** (`src/core/File.cpp:243-263`)
  ```cpp
  if (isPopen) {
      if (::pclose((FILE*)ff) != 0) ret = 0;
  } else { ... }
  ...
  if (ret == 0) throwErrno(errno, filename());
  ```
  `pclose()` liefert den rohen Wait-Status des Kindprozesses zurück; dieser ist bei jedem Kommando, das mit einem
  Exit-Code ≠ 0 terminiert (z.B. `grep` ohne Treffer, `test`, `false`, oder ein Skript mit `exit 1`), **ungleich 0** –
  das ist ein völlig normales, kein Fehlerfall im Sinne von I/O. Der Code interpretiert das aber unbesehen als
  Systemfehler und ruft `throwErrno(errno, ...)` auf – `errno` ist an dieser Stelle aber i.d.R. `0` (der letzte
  erfolgreiche Syscall), was in `throwExceptionFromErrno` im `default`-Zweig landet und dort sogar einen
  Debug-`printf` auslöst, bevor eine nichtssagende `UnknownException("Success")` geworfen wird. Empirisch bestätigt
  (Testprogramm `t4.cpp`):
  ```
  --- popen('exit 1') then explicit close() ---
  Hier fliegt errno: 0, info: , Success:
    EXC: pplib::UnknownException : UnknownException
  --- popen('true') then explicit close() (exit 0) ---
    close() OK, no exception
  ```
  Jede Anwendung, die `File::popen()` für ein Kommando nutzt, dessen Exit-Code sie eigentlich selbst auswerten möchte
  (z.B. `grep`, Test-Skripte), bekommt beim Schließen eine irreführende `UnknownException` statt des tatsächlichen
  Exit-Codes – im Destruktor wird das zwar von `catch(...)` verschluckt, bei explizitem `close()`-Aufruf aber nicht.
  Fix: `pclose()`-Rückgabewert per `WIFEXITED`/`WEXITSTATUS` auswerten und den Exit-Code entweder ignorieren (Doku
  entsprechend anpassen: „Exit-Code wird nicht geprüft") oder über eine eigene, aussagekräftige Exception/Methode
  (`getExitCode()`) zugänglich machen – keinesfalls über `errno`, das hier semantisch nicht zutrifft.

## Design

- [ ] **`write(ByteArrayPtr&, bytes=0)` verhält sich zwischen `File` und `MemFile` unterschiedlich** (`src/core/File.cpp:384-395` vs. `src/core/MemFile.cpp:342-351`)
  `File::fwrite` prüft `ptr==NULL` **vor** `nmemb==0` und wirft daher bei einem leeren `ByteArrayPtr` (Default: `bytes=0`
  → `object.size()==0` → `fwrite(NULL,1,0)`) weiterhin `IllegalArgumentException` (alter Bug, Tabellenpunkt 13).
  `MemFile::fwrite` hat gar keinen `ptr==NULL`-Check und lässt denselben Aufruf klaglos durch. Ein Aufrufer, der
  gegen die `FileObject`-Basisklasse programmiert, bekommt also für dieselbe, an sich sinnvolle No-Op-Operation
  (leeres Objekt schreiben) je nach konkreter Subklasse ein unterschiedliches Verhalten. Fix: `bytes==0` (nach dem
  Clamp auf `object.size()`) in `FileObject::write(const ByteArrayPtr&, size_t)` einheitlich als No-Op behandeln
  (`if (!bytes) return 0;`), statt sich auf das Verhalten der jeweiligen `fwrite`-Implementierung zu verlassen.

- [ ] **`FileObject::load()` maskiert einen „silent partial read" durch klagloses Trunkieren statt einer Exception** (`src/core/FileObject.cpp:200-214`)
  ```cpp
  r = read(b, (size_t)s, 0);
  if (r < s) {
      ba[r] = 0;
      ba.truncate(r + 1);
  }
  ```
  Kombiniert mit dem weiterhin offenen Bug „`File::fread`: Partial Read am EOF wirft keine Exception" (Tabellenpunkt 7)
  bedeutet das: liest `load()` z.B. wegen einer parallelen Verkleinerung der Datei zwischen `size()`- und `read()`-Aufruf
  weniger Bytes als erwartet, wird das Ergebnis stillschweigend auf die tatsächlich gelesene Länge gekürzt – der Aufrufer
  hat keine Möglichkeit, einen unvollständigen Read von einer regulär kürzeren Datei zu unterscheiden. Das ist zwar keine
  akute Sicherheitslücke, aber ein API-Vertragsrisiko, sobald der `fread`-Bug (Punkt 7) behoben wird: dann müsste
  `load()` diesen Fall ebenfalls überdenken (aktuell wäre die Exception aus `fread` unkommentiert durchgereicht).
  Fix: nach Behebung von Punkt 7 prüfen, ob `load()` weiterhin ein sinnvolles Trunkierungsverhalten braucht oder ob
  eine Exception (`ReadException`) die bessere Wahl ist.

## Doku / Kosmetik

- [ ] Include-Guard-Kommentar-Mismatch jetzt auch in `file.h` gefunden (zusätzlich zum bereits gemeldeten in
  `fileobject.h`): `#ifndef PPLIB_CORE_FILE_H_` (Zeile 30) vs. `#endif /* PPLIB_CORE_MEMFILE_H_ */` (Zeile 885) –
  offensichtlich aus `memfile.h` kopiert.
- [ ] Dieselben Typos wie im alten Review (`%fgwets`, „Ausgahgsbasis", „miitels", „solle") sind in `fileobject.h`
  weiterhin unverändert vorhanden und zusätzlich in `file.h` dupliziert (z.B. `file.h:346` „%fgwets liest…").
- [ ] `load()`/`load(ByteArray&)`: `seek(0)`-Seiteneffekt weiterhin nicht dokumentiert.
- [ ] `MemFile::openReadWrite` weiterhin ohne `@attention`-Hinweis zur Speicher-Ownership-Übernahme (im Gegensatz zum
  Konstruktor `MemFile(void*, size_t, bool writeable)`, der diesen Hinweis hat).

## Verifiziert OK (kein Handlungsbedarf)

- `FileObject::getws`/`getws(size_t)` sind jetzt korrekt auf `WideString` typisiert und symmetrisch zu `putws`.
- `copyFrom()` gibt bei einer kürzeren Quelle korrekt die tatsächlich kopierte Byte-Anzahl zurück (empirisch: `5` statt
  fälschlich `20` bei einer Anfrage von 20 Bytes gegen eine 5-Byte-Quelle).
- `File::write(ByteArrayPtr&, bytes=0)` mit einem leeren Objekt gegen eine reguläre Datei bleibt konsistent zum alten
  Review beschrieben (wirft `IllegalArgumentException`) – kein neues, unerwartetes Verhalten.
- `MemFile::seek(int64_t, SEEKCUR)` funktioniert für normale, im gültigen Bereich liegende relative Sprünge korrekt
  (nur der Exakt-EOF-Randfall ist betroffen, siehe Tabelle).
- Keine neuen Funde in den rein statischen `File`-Hilfsfunktionen (`copy`, `rename`, `erase`, `statFile`, `chmod`,
  `getPath`/`getFilename`/`getSuffix`) – Code-Pfade wurden mitgelesen, aber keine über die bereits gemeldeten Punkte
  hinausgehenden Auffälligkeiten gefunden.
