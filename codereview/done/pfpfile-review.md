# PFPFile Review – Befunde

Review vom 2026-11-13, Scope: `src/core/PFPFile.cpp` (674 Zeilen), `include/pplib/core/pfpfile.h`, Spezifikation `docs/formate/PFPVersion3.dox`.
Öffentliche API: `PFPChunk`, `PFPFile` (u. a. [setStringParam](../../src/core/PFPFile.cpp:224), [getStringFromChunk](../../src/core/PFPFile.cpp:251), [save](../../src/core/PFPFile.cpp:314), [useMemory](../../src/core/PFPFile.cpp:585))
Review done by: Copilot (VS Code Copilot SDK)

## Kontext: Commits 1a01037 und 46f8af2

Die Refactorings erfolgten in zwei Schritten:
1. **Commit `1a01037` (02.08.2026, "Mutex und PFPFile refactored"):** `PFPChunk` und `PFPFile` wurden grundlegend modernisiert. Die alten Überladungen `PFPChunk::setData(const char*)` und `PFPChunk::setData(const String&)` wurden entfernt. `setParam` wurde zu `setStringParam` umbenannt und auf `PFPChunk chunk(chunkname, ByteArrayPtr(data))` umgestellt.
2. **Commit `46f8af2` (22.08.2026, "Bugfixes in PFPFile und KI-generierte Unittests hinzugefügt"):** Der Lesepfad wurde auf `std::string_view`-Vergleiche umgestellt, `getStringFromChunk` eingeführt sowie Härtungen (`try/catch`, `toupper`-Casts, Kompressions-Enum-Check) ergänzt. Der geänderte Schreibpfad (Komprimierung über `Compression::compress(ByteArrayPtr)`) ist kontraktgemäß in Ordnung (die Overloadversion allokiert intern gemäß `maxCompressedBound()`; laut Doku in [compression.h](../../include/pplib/core/compression.h)).

### Historie der 0-Byte-Terminierung

Die Annahme, dass das 0-Byte erst beim Umbau in `1a01037` verloren ging, täuscht aufgrund einer subtilen C++-Überladungsauflösung (Overload Resolution – Auswahl der passenden Funktionssignatur anhand der Argumenttypen durch den Compiler):

- **Vor 2015:** Es gab eine dreistufige Kaskade: `setData(const String& s)` rief `setData(s)` auf. Über `String::operator const char*()` wurde `setData(const char* s)` aufgerufen, und dieses rief `setData((void*)s, strlen(s) + 1)` auf – also **mit** 0-Byte.
- **Commit `d247a34` (30.11.2015, "Bugfix in PFPChunk::setData(const String &s)"):** Die Zeile `setData(s);` wurde ersetzt durch `setData((const char*)s, s.size());`. Da hier **zwei** Argumente übergeben wurden, wählte der Compiler direkt `setData(const void* ptr, size_t size)` (zwei Parameter) und **nicht** `setData(const char* s)` (ein Parameter). Weil `String::size()` die Zeichenanzahl **ohne** 0-Byte liefert, ging das 0-Byte bereits am 30.11.2015 verloren. Die Methode `setData(const char* s)` mit `strlen()+1` verblieb als toter Code (Dead Code – Programmcode, der existiert, aber nie aufgerufen wird).
- **Zustand auf `origin/main`:** Exakt dieser Zustand aus `d247a34` liegt bis heute auf `origin/main`. Auch dort schreibt `setParam("AUTH", author)` über `setData(const String&)` faktisch kein 0-Byte mehr in die Datei, obwohl `setData(const char* s)` mit `strlen()+1` in der Datei noch optisch vorhanden ist.
- **Commit `1a01037`:** Beim Aufräumen der Kaskade wurde `c->setData(data)` durch `ByteArrayPtr(data)` ersetzt. Das übernahm das Verhalten von `String::size()`, änderte aber am bereits seit 2015 bestehenden Byte-Layout nichts.

`getStringFromChunk` aus `46f8af2` ist die passende **Lese**-Gegenseite: ein vorhandenes abschließendes 0-Byte wird entfernt – funktional korrekt (siehe Verifikation unten), aber die Schreibseite lieferte es schon seit 2015 nicht mehr.

## Verifikation

Kleinprogramme in `/tmp/pfpcheck` (außerhalb der Projektquellen, nach Review entfernt), gelinkt gegen `build/debug/libpplib8-core.a` (Linux, gcc, C++20):

1. `setAuthor("Patrick")` + `setName("Test")` + `save()` → Hexdump der Datei ab Header:
   `NAME[0C 00 00 00]TestAUTH[0F 00 00 00]PatrickENDF[00 00 00 00]`
   → AUTH-Chunk: Längenfeld 15, Payload 7 Bytes, **kein 0x00**. Spezifikationsgemäß wären 16/8 Bytes erwartet. **Befund 1 bestätigt.**
2. Handgebauter Spezifikations-konformer Puffer (`AUTH`, Größe 16, Payload `"Patrick\0"`) über `useMemory` → `getAuthor()` liefert `"Patrick"`, d. h. das abschließende 0-Byte wird korrekt entfernt. Lese-/Kompatibilitätspfad **funktional verifiziert**.
3. Zlib-komprimierter `save()`→`load()`-Roundtrip → `getAuthor()` korrekt. Der in 46f8af2 umgebaute Kompressionspfad **verifiziert**.

## Bugs (kritisch)

- [x] **Schreibpfad hält Spezifikation nicht ein: schließendes 0-Byte fehlt** (`src/core/PFPFile.cpp:224`)
  [setStringParam](../../src/core/PFPFile.cpp:224) schreibt die String-Parameter `NAME`/`AUTH`/`DESC`/`COPY` ohne abschließendes 0-Byte (`ByteArrayPtr(data)` nutzt `String::size()` ohne Terminus). Die Spezifikation ([PFPVersion3.dox](../../docs/formate/PFPVersion3.dox), Zeilen 60–95) verlangt ausdrücklich "… mit schließendem 0-Byte"; damit ist auch das Längenfeld des Chunks um 1 Byte zu klein. Verifiziert per Hexdump (siehe oben). Betroffen sind alle seit `d247a34` (2015) von dieser Bibliothek geschriebenen Dateien – ältere ppl-Versionen vor 2015 sowie Referenzimplementierungen nach Spezifikation erzeugten Dateien **mit** Terminus. Kompatibel ist beides nur zufällig, weil die Getter auf C-String-Semantik bzw. NUL-Entfernung ausgelegt sind; strikte externe Parser, die Länge == Stringgröße inkl. Terminus erwarten, scheitern.
  Minimaler Fix:
  ```cpp
  void PFPFile::setStringParam(const String& chunkname, const String& data)
  {
      deleteChunk(chunkname);
      // Spezifikationsgemäß mit schliessendem 0-Byte ablegen
      ByteArrayPtr param(data.getPtr(), data.len() + 1);   // String::getPtr() liefert stets 0-terminierten Buffer
      PFPChunk chunk(chunkname, param);
      Chunks.push_back(std::move(chunk));
  }
  ```
  Hinweis: Bei leerem String entsteht so ein 1 Byte großer Chunk (nur Terminus). Falls das unerwünscht ist, bei `data.notEmpty()` verzweigen – Spezifikation lässt das offen; bitte entscheiden.
  Bestehende "kaputte" Dateien (2015–heute) bleiben lesbar, `getStringFromChunk` toleriert beides.

  **Umsetzung:** Fix in der vom Nutzer vorgegebenen Form (`PFPChunk chunk(chunkname, ByteArrayPtr(data.c_str(), data.len() + 1));`) ist mit Commit `1813637` im Repo. Abgesichert durch `PFPFileTest.StringParamSavedWithTrailingNullByte` (Dateibytes: Längenfeld 16, `Peek32(p+28)`, Payload `Patrick`, `p[39] == 0x00`) und `PFPFileTest.EmptyStringParamRoundtrip` (leerer String → 9 Byte Chunk, nur Terminus, Datei 41 Byte, Roundtrip wieder leer). Die offene Frage zum leeren String ist damit zugunsten der spezifikationskonformen 1-Byte-Variante entschieden.

- [x] **Regression in `save()`: vordefinierte Chunks werden mehrfach geschrieben** (`src/core/PFPFile.cpp:345`)
  Vor dem Refactoring (`46f8af2`) wurde pro vordefinierter Kategorie via `reset(it)` + `findFirstChunk` genau **ein** Chunk geschrieben. Die neuen vier Schleifen schreiben **alle** passenden Chunks. Fügt der Anwender zusätzlich manuell weitere `AUTH`-Chunks via `addChunk` hinzu (die API erlaubt gleichnamige Chunks), landen mehrere `AUTH`-Chunks am Dateianfang; die Getter liefern nur den ersten, die übrigen sind für diese Klasse unsichtbarer Datenballast. Die Spezifikation definiert Mehrfach-Chunks ausdrücklich nur für *nicht*-vordefinierte Namen.
  Minimaler Fix: in jeder der vier Schleifen nach `saveChunk` ein `break;`.

  **Umsetzung:** In allen vier Schleifen von [save](../../src/core/PFPFile.cpp:314) steht nach `saveChunk` jetzt ein `break;` mit Kommentar. Neuer Test `PFPFileTest.PredefinedChunkNamesWrittenOnce`: `setAuthor("ErsterAutor")` plus zusätzliches `addChunk(PFPChunk("AUTH", …))`; der Byte-Scan der geschriebenen Datei zählt genau ein `AUTH`-Segment, `getAuthor()` nach `load()` liefert `"ErsterAutor"`.

## Bugs (mittel / Warnungen)

- [x] **`PFPChunk::useData` ohne Größenbegrenzung** (`src/core/PFPFile.cpp:94`)
  [setData](../../src/core/PFPFile.cpp:69) prüft `size > (0xffffffff - 8)`; [useData](../../src/core/PFPFile.cpp:94) nicht. Ein per `useData` angebundener Chunk > 4 GiB schreibt in [saveChunk](../../src/core/PFPFile.cpp:302) ein trunciertes Längenfeld (`Poke32`) und erzeugt eine korrupte Datei: `save()` kalkuliert die Puffergröße mit `size_t`, das Größensfeld im Layout ist aber 32 Bit. Risiko praktisch erst auf 64-Bit-Systemen mit sehr großen Chunks. Fix: die Prüfung aus `setData` auch in `useData` ergänzen.

  **Umsetzung:** Beide `useData`-Überladungen prüfen jetzt analog zu `setData`: `if (size > (0xffffffff - 8) || (!ptr && size > 0)) throw IllegalArgumentException();` ([src/core/PFPFile.cpp:94](../../src/core/PFPFile.cpp:94)). Der Vergleich steht vor dem `free()`, abgewiesene Aufrufe lassen den Chunk unverändert. Test `PFPChunkTest.UseDataRejectsOversizedOrNull` (u. a. `0x100000000ULL`).

- [x] **Unbegrenzte Dekompressions-Allokation in `useMemory`** (`src/core/PFPFile.cpp:618`)
  `sizeunk` kommt ungeprüft aus dem Dateiheader (bis 4 GiB, `uint32_t`) → `uncompressedData.malloc(sizeunk + 1)`. Eine präparierte Datei mit kleinem komprimiertem Inhalt und großem `sizeunk` erzwingt eine Allokation bis ~4 GiB (Denial of Service). Plausibilitäts-Check fehlt, z. B. Verhältnis `sizeunk`/Dateigröße nach oben begrenzen. Bestehender Code, nicht aus 46f8af2. **[Prüfen]** Gewolltes Verhalten bei potenziell feindlichen Dateien?

  **Umsetzung:** Von dir ergänzt: [useMemory](../../src/core/PFPFile.cpp:621) prüft jetzt `if (sizeunk >= SIZE_MAX) throw InvalidFormatException();` vor der Allokation. Damit ist der kritische Fall auf 32-Bit-Plattformen dicht: ohne die Prüfung lief `sizeunk + 1` bei `sizeunk == SIZE_MAX` auf 0 über, `ByteArray::malloc()` holte nur 1 Byte ([ByteArray.cpp:46](../../src/types/ByteArray.cpp:46)), als Zielgröße wurde aber weiterhin `sizeunk` übergeben – die Dekomprimierung schrieb also in einen 1-Byte-Puffer (Heap-Overflow). Auf 64-Bit-Plattformen kann das Feld weiterhin eine Allokation bis ~4 GiB aus einer kleinen Datei auslösen; bewusst nicht weiter begrenzt (Entscheidung vom 2026-11-14, keine Verhältnissgrenze zu `sizecomp`). Auf x86-64 ist der Überlauf nicht reproduzierbar, deshalb kein Test.

- [x] **KI-Tests prüfen nur die interne API, nicht das Dateiformat** (`tests/src/core/pfpfile.cpp`)
  Der Roundtrip `setAuthor` → `getAuthor` ist gegen beide Schreibvarianten (mit/ohne Terminus) grün – genau deshalb wurde der Spezifikationsverstoß nicht gefunden. Die Testerwartung "Test trailing null stripping" prüft nur den Lesepfad. Ergänzen: ein Test, der `save()` in eine Temp-Datei schreibt und die Bytes des `AUTH`-Chunks gegen die Spezifikation prüft (Längenfeld == strlen+1+8, letztes Datenbyte == 0x00). Das wäre ein Regressionstest für Befund 1.

  **Umsetzung:** Genau dieser Test existiert jetzt: `PFPFileTest.StringParamSavedWithTrailingNullByte` liest `tmp/test_terminator.pfp` byte-genau (`Peek32(p + 28) == 16`, `p[39] == 0x00`, `ENDF` bei Offset 40). Zusätzlich prüft `PFPFileTest.EmptyStringParamRoundtrip` den Randfall leerer String (Chunkgröße 9, Datei 41 Byte).

## Wartbarkeit & Aufräumen

- [x] **`getStringFromChunk` schwer lesbar** (`src/core/PFPFile.cpp:251`)
  Verhalten korrekt (ein anhängendes 0-Byte wird entfernt – gewollte Abwärtskompatibilität zu Spezifikations-konformen Dateien), aber die Bedingungskette ist verwirrend: erst `len > 0 && str` im ersten Check, dann erneut `!str` im zweiten. Klarere Fassung gleichen Verhaltens:
  ```cpp
  static String getStringFromChunk(const PFPChunk& chunk)
  {
      const char* str = (const char*)chunk.data();
      size_t len = chunk.size();
      if (!str || len == 0) return String();
      if (str[len - 1] == '\0') len--;   // Spezifikationsgemaess enthaelt der String ein schliessendes 0-Byte
      return String(str, len);
  }
  ```

  **Umsetzung:** In [getStringFromChunk](../../src/core/PFPFile.cpp:251) steht jetzt die klarere Fassung (erst `!str || len == 0`, dann Terminus-Entfernung, dann `String(str, len)`). Verhalten identisch; der Roundtrip-Test `PFPFileTest.StringParamSavedWithTrailingNullByte` und das Laden der Real-World-Fonts bleiben grün.

- [x] **Ungenutztes Include** (`include/pplib/core/pfpfile.h:38`)
  `<pplib/core/mutex.h>` wird im Header nirgends benötigt – es gibt keine Mutex-Mitglieds in `PFPChunk`/`PFPFile`.

  **Umsetzung:** Include entfernt; weder Header noch Übersetzer der Nutzer benötigen es indirekt. Kompilieren der Bibliothek sowie aller 69 Testsuiten bestätigt das.

- [x] **Toter Kommentar** (`src/core/PFPFile.cpp:434`)
  Auskommentierte `remove_if`-Zeile in [deleteChunk](../../src/core/PFPFile.cpp:430) kann entfallen.

  **Umsetzung:** Zeile entfernt. Der loeschende Teil von [deleteChunk(const String&)](../../src/core/PFPFile.cpp:425) benutzt bereits das sichere Muster `it = Chunks.erase(it)`; der Test `PFPFileTest.DeleteChunkByName` deckt ihn ab.

- [x] **Inkonsistente `memcpy`-Längen beim Move** (`src/core/PFPFile.cpp`)
  Der Move-Konstruktor kopiert 5 Bytes (`memcpy(chunkname, other.chunkname, 5)`), die Move-Zuweisung 4 Bytes plus manuell `chunkname[4] = '\0'`. Beides korrekt, aber vereinheitlichen (5 Bytes) vermeidet zukünftige Flüchtigkeitsfehler.

  **Umsetzung:** In [PFPChunk::operator=(PFPChunk&&)](../../src/core/PFPFile.cpp:159) steht jetzt ebenfalls `memcpy(chunkname, other.chunkname, 5)`; der separate Terminus-Schreibschritt ist entfallen. `PFPChunkTest.MoveAssignment` bleibt unverändert grün.

## Dokumentation & Doxygen

- [x] **Header-Doku zu `saveChunk` veraltet** (`include/pplib/core/pfpfile.h:254`–261)
  Beschreibung ("Speichern von vordefinierten Chunks", Parameter `chunkname`/`data`) passt nicht zur Signatur `saveChunk(char* buffer, size_t pp, const PFPChunk* chunk)`; die Funktion speichert alle Chunks.

  **Umsetzung:** Neuer Doku-Block fuer das private `saveChunk`: Name, die drei Parameter, Rueckgabe als writing distance (`size() + 8`) und der Verweis auf den Aufrufer [PFPFile::save](../../src/core/PFPFile.cpp:314).

- [x] **Veraltete Funktionsnamen in Doku** (`include/pplib/core/pfpfile.h:231`, `256`–259, `reset`-@see-Blöcke)
  `PFPFile::add`, `SetName`/`SetAuthor`/`SetDescription`/`SetCopyright`, `FindFirstChunk`/`FindNextChunk`/`GetFirst`/`GetNext`/`Reset` – die API heißt seit v8 `addChunk`/`setName`/… ; Doku-Links ins Leere.

  **Umsetzung:** Saemtliche Doku-Verweise auf camelCase-Namen der v8-API gezogen (`add` -> `addChunk`, `FindFirstChunk`/`FindNextChunk`/`Reset`/`Save` etc.). Ein grep nach den alten PascalCase-Namen im Header liefert keine Treffer mehr.

- [x] **`addChunk`-Doku falsch bzgl. Fehlerverhalten** (`include/pplib/core/pfpfile.h:413`, `436`)
  "gibt die Funktion eine Fehlermeldung zurück" – tatsächlich wird `IllegalArgumentException` geworfen (bei Chunkname "UNKN"); `@exception` fehlt komplett. Ebenso fehlt bei `findFirstChunk`/`findNextChunk` die Angabe, dass `IllegalArgumentException` bei Name ≠ 4 Byte geworfen wird.

  **Umsetzung:** Beide `addChunk`-Ueberladungen dokumentieren jetzt `@return` und `@exception IllegalArgumentException` (abgelehnt wird der Platzhaltername "UNKN") sowie den Hinweis, dass vordefinierte Namen nur einmal geschrieben werden. Bei `findFirstChunk`/`findNextChunk` fehlt die Exception-Angabe weiterhin nicht: der Wurf bei laengem != 4 steht bereits in [PFPFile::findNextChunk](../../src/core/PFPFile.cpp:462) und ist im Header ergaenzt.

- [x] **`useMemory` ohne Doku** (`include/pplib/core/pfpfile.h:536`)
  Öffentliche Funktion ohne Doxygen-Kommentar. Zu dokumentieren: Lebensdauer des übergebenen Speichers (unkomprimiert zeigen die Chunks direkt in den fremden Puffer; bei Komprimierung wird in `payload` kopiert) und Exception-Verhalten.

  **Umsetzung:** Doxygen-Block ergaenzt: gepruefte Felder (Magie, Hauptversion 3, Headergroesse), ignorieren ungueltiger Chunks, `InvalidFormatException`/`DecompressionFailedException` und die Pufferlebensdauer (unkomprimiert zeigen die Chunks direkt in den Fremdpuffer, komprimiert wird in den eigenen `payload` kopiert).

- [x] **Doku `getName`/`getAuthor`/…: Verhalten bzgl. 0-Byte nicht beschrieben** (`include/pplib/core/pfpfile.h:586`–616)
  Nach dem Fix von Befund 1 sollte dokumentiert sein: geschrieben wird spec-konform mit abschließendem 0-Byte; die Getter entfernen ein vorhandenes Terminus (read-seitige Abwärtskompatibilität).

  **Umsetzung:** Bei allen vier Gettern steht jetzt "Ein abschließendes 0-Byte des Chunks gehört nicht zum zurückgegebenen String."; die schreibende Seite (`setStringParam`) beschreibt den Terminus bereits in ihrer Doku.

- [x] **Move-Konstruktor-Doku ungenau** (`include/pplib/core/pfpfile.h:103`)
  "Der Move-Konstruktor erstellt eine Kopie…" – er verschiebt; Quelle-Chunk wird auf "UNKN"/leer gesetzt.

  **Umsetzung:** Formulierung auf "uebernimmt die Daten des Quellchunks; der Quellchunk bleibt ohne Daten zurueck" korrigiert.

- [x] **Spezifikation selbst: mehrdeutige Längenangabe** (`docs/formate/PFPVersion3.dox:63`, `73`, `83`, `93`)
  "Byte 4: Länge des nachfolgenden Strings" ist inkonsistent zum allgemeinen Chunk-Kapitel ("Größe des Chunks einschließlich 8-Byte Header", Zeile 51): Ist das Feld die reine Stringlänge oder die Chunkgröße inkl. Header? Präzisieren (z. B. "Größe des Chunks einschließlich Header; String inklusive schließendem 0-Byte"), sonst bleibt die Längendiskussion zu Befund 1 dauerhaft offen.

  **Umsetzung:** Von dir präzisiert: an allen vier vordefinierten Chunks (und `ENDF`) steht jetzt "Größe des Chunks einschließlich 8-Byte Header  4 Byte" plus der Zusatzsatz "Die Größe des Chunks ergibt sich aus der Länge des … plus 0-Byte Terminator, zuzüglich 8-Byte Header." Damit stimmt die Spezifikation mit dem Implementierungsverhalten und dem Byte-Test `PFPFileTest.StringParamSavedWithTrailingNullByte` (Feldwert 16 bei `"Patrick"`) überein.

## Befunde in Drittdateien

- [ ] **`ByteArrayPtr(const String&)` ohne Terminus** (`src/types/ByteArrayPtr.cpp`)
  Konstruiert aus `String::size()` – also bewusst ohne 0-Byte. Das ist die Wurzel des Missverständnisses in `setStringParam`; die Semantik ist korrekt, überrascht aber an dieser Stelle. Nur als Warnung für künftige Verwendungen notieren, keine Änderung.

  **Umsetzung:** Bewusst keine Änderung an `ByteArrayPtr`. In `setStringParam` wird die Länge jetzt explizit mit `data.len() + 1` übergeben; die Fallstelle ist zusätzlich durch den Byte-Test abgesichert.

- [ ] **`String::set(const ByteArrayPtr&)`** (`src/types/String.cpp`, von `String(const ByteArrayPtr&)` aufgerufen)
  Für das Verständnis der Alt-Getter (`String(chunk.chunkdata)` vor dem Refactoring) relevant: Der Alt-Code konnte eingebettete NUL-Bytes nicht sauber verarbeiten; die neue Variante mit expliziter Länge ist stringenter. Keine Änderung nötig.

  **Umsetzung:** Bewusst keine Änderung; der Befund ist rein einordnend. Der Lesepfad ueber `getStringFromChunk` mit expliziter Laenge bleibt unveraendert.

## Strukturvorschläge (nur zur Diskussion)

- **Einheitliches String-Chunk-Handling**: `setStringParam` (Schreiben mit Terminus) und `getStringFromChunk` (Lesen mit Terminus-Entfernung) sind ein Paar, das nur durch zwei Stellen in zwei Dateien verteilt korrekt zusammenwirkt. Ein kleiner privater Helfer `makeStringChunk(name, value)` + ein Test der Byte-Ebene würde die Paarung erzwingen. Aufwand gering, Risiko gering.
- **Konstanten für den Dateikopf** in `save()` (Magic, Headergröße 24, `ENDF`): heute als Literale verteilt; Konstanten würden die Doku-Referenz auf die Spezifikation leichter nachvollziehbar machen. Kein Funktionszwang.

## Glossar

- **Dead Code** – Programmcode, der kompiliert, aber zur Laufzeit an keiner Stelle aufgerufen wird.
- **Doxygen** – Doku-Generator für Quelltextkommentare mit Befehlen wie `@brief`, `@exception`.
- **hexdump** – bytegenaue Anzeige von Dateiinhalten, hier zur Formatprüfung.
- **mmap** – Datei direkt in den Prozessspeicher einblenden.
- **NUL / 0-Byte / Terminus** – Byte mit Wert 0, C-String-Endemarkierung.
- **Overload Resolution** – Auswahl der passenden Funktionsüberladung anhand der Argumenttypen durch den Compiler.
- **Peek32/Poke32/Poke8** – pplib-eigene Funktionen zum LittleEndian-Lesen/Schreiben von Werten im Speicher.
- **Regressionstest** – Test, der einen konkret behobenen Fehler dauerhaft absichert.
- **Roundtrip** – Speichern und unmittelbares Wiederladen desselben Objekts.
- **strlen** – C-Funktion: Zeichenanzahl eines Strings ohne abschließendes NUL.
