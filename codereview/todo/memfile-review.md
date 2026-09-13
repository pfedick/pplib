# MemFile Review – Befunde

Review vom 2026-09-13, Scope: `include/pplib/core/memfile.h` + `src/core/MemFile.cpp` (560 Zeilen).
Mitgelesen: `include/pplib/core/fileobject.h` + `src/core/FileObject.cpp`, `Json.cpp`, `ConfigParser.cpp`.

* Review done by: GitHub Copilot (Gemini 3.8 Flash)

---

## Bugs (kritisch)

- [ ] **Destruktor `~MemFile()` gibt `buffer` nie frei → Permanentes Speicherleck bei jeder geschriebenen Datei** (MemFile.cpp:111-114)
  ```cpp
  MemFile::~MemFile()
  {
  }
  ```
  Wird ein `MemFile` zum Schreiben verwendet (z.B. `MemFile file; file.fwrite(...)` wie in `Json::dumps()` oder
  `ConfigParser`), allokiert `resizeBuffer()` Speicher via `realloc(buffer, newsize)`.
  Wird das `MemFile`-Objekt zerstört (Scope-Ende), passiert im Destruktor absolut gar nichts:
  Weder wird `close()` noch `free(buffer)` aufgerufen. Jeder dynamisch allokierte Puffer leakt vollständig,
  sofern die Anwendung nicht explizit `close()` vor der Destruktion aufgerufen hat.
  Fix:
  ```cpp
  MemFile::~MemFile()
  {
      close();
  }
  ```

  ==> FIXED

- [ ] **Gefährliche Ownership-Übernahme in `open(..., writeable=true)` und `openReadWrite(...)`** (MemFile.cpp:130-155, 189-201)
  ```cpp
  void MemFile::open(void* adresse, size_t size, bool writeable)
  {
      if (writeable == true) {
          buffer = MemBase; // MemBase = adresse
          readonly = false;
          buffersize = size;
      }
      ...
  }
  ```
  Wenn `writeable == true` übergeben wird, übernimmt `MemFile` die Verantwortung für den übergebenen Zeiger `adresse`
  und behandelt ihn in `resizeBuffer()` als `realloc(buffer, newsize)` und in `close()` als `free(buffer)`.
  1. Wurde `adresse` nicht exakt mit `malloc()` auf dem C-Heap allokiert (sondern z.B. auf dem Stack, mit C++ `new[]`,
     oder als statischer Puffer), führt jedes spätere `resizeBuffer()` oder `close()` unweigerlich zu Heap-Corruption
     und Programmabsturz.
  2. Selbst wenn der Puffer per `malloc` allokiert wurde, kennt `MemFile` nur die logische Datengröße `size`, nicht die
     tatsächliche Heap-Blockgröße.
  Fix: `MemFile` sollte niemals fremde Speicherbereiche "adoptieren". Entweder ist `MemFile` ein **nicht-besitzender
  Read-Only-View** auf vorhandenen Speicher (`readonly = true`), oder `MemFile` verwaltet seinen eigenen, dynamischen
  Puffer vollkommen selbstständig (Read/Write).

  ==> Ich erkenne den Konflikt, werde aber daran nichts ändern. Doku wird die Ownership-Regeln klarstellen.

- [ ] **Fehlende Rule of Five: `MemFile` ist unbeschränkt flach kopierbar → Double Free & Use-After-Free** (memfile.h:32-79)
  `MemFile` verwaltet rohe Zeiger (`buffer`, `MemBase`), deklariert aber weder Kopier-/Move-Konstruktoren noch
  Zuweisungsoperatoren.
  ```cpp
  MemFile f1;
  f1.fwrite("Hello", 1, 5);
  MemFile f2 = f1; // Flache Kopie von buffer und MemBase
  f1.close();      // free(buffer)
  f2.close();      // Double Free! Absturz.
  ```
  Fix: Kopieren verbieten und Move-Semantik implementieren (oder ebenfalls verbieten):
  ```cpp
  MemFile(const MemFile&) = delete;
  MemFile& operator=(const MemFile&) = delete;
  MemFile(MemFile&& other) noexcept;
  MemFile& operator=(MemFile&& other) noexcept;
  ```

  ==> FIXED, so übernommen

- [ ] **`fgetc()`: Out-of-Bounds Read bei `pos == mysize` und Vorzeichenfehler bei `0xFF`** (MemFile.cpp:424-429)
  ```cpp
  int MemFile::fgetc()
  {
      if (MemBase == NULL) throw FileNotOpenException();
      if (pos > mysize) throw OverflowException();
      return MemBase[pos++];
  }
  ```
  1. **Off-by-One**: Wenn `pos == mysize` ist (Dateiende erreicht), schlägt `pos > mysize` fehl!
     Die Funktion liest `MemBase[pos++]`, also ein Byte *hinter* dem gültigen Pufferbereich (OOB-Read),
     und erhöht `pos` auf `mysize + 1`. Bei exakt allokierten Puffern liest sie Müll oder segfaultet an Page-Grenzen.
     Korrekt muss am Dateiende eine `EndOfFileException` geworfen werden (`if (pos >= mysize)`).
  2. **Vorzeichen-Bug**: `MemBase` ist `char*`. Auf Plattformen mit `signed char` (x86_64 Standard) wird ein Byte
     mit dem Wert `0xFF` (255) beim impliziten Cast auf `int` zu `-1` (was `EOF` entspricht!).
     Standard-konforme Streams casten Bytes vor der Rückgabe immer nach `unsigned char`.
  Fix:
  ```cpp
  int MemFile::fgetc()
  {
      if (MemBase == NULL) throw FileNotOpenException();
      if (pos >= mysize) throw EndOfFileException();
      return static_cast<unsigned char>(MemBase[pos++]);
  }
  ```

  ==> FIXED

- [ ] **`seek()`: Dateiende (`mysize`) wird als ungültige Position abgelehnt** (MemFile.cpp:266-315)
  1. In `seek(uint64_t position)`:
     ```cpp
     if (position < mysize) {
         pos = position;
     } else if (mysize == 0 && position == 0) {
         return;
     } else {
         throw OverflowException(); // position == mysize wird abgelehnt!
     }
     ```
     `position == mysize` ist eine absolut legale Dateiposition (Dateiende für nachfolgende Schreiboperationen).
     Hier wird sie fälschlicherweise mit `OverflowException` verweigert.

     ==> FIXED

  2. In `seek(int64_t offset, SeekOrigin origin)` für `SEEKEND`:
     ```cpp
     case SEEKEND:
         pos = mysize - offset;
         if (pos > mysize) return pos;
         if ((int64_t)pos < 0) { ... }
         break;
     ```
     Wird standardmäßig `seek(0, SEEKEND)` aufgerufen, ist `pos == mysize`. Weder `pos > mysize` noch
     `(int64_t)pos < 0` treffen zu. Die Ausführung verlässt das `switch` per `break` und landet direkt in:
     `throw FileSeekException(...);`!
     Ein Seek an das Dateiende mit `offset=0` schlägt in `MemFile` immer fehl!

     ==> FIXED
  Fix: Bedingungen auf `position <= mysize` und `pos <= mysize` korrigieren.

- [ ] **`fgets()`: Datenverlust bei Zeilen am Dateiende ohne `\n`** (MemFile.cpp:346-367)
  ```cpp
  char* MemFile::fgets(char* buffer1, size_t num)
  {
      ...
      buffer1[i] = 0;
      pos += i;
      if (pos >= mysize) throw EndOfFileException();
      return buffer1;
  }
  ```
  Wenn die letzte Zeile einer Datei nicht mit einem Zeilenumbruch `\n` endet, liest die Schleife alle restlichen
  Zeichen bis `mysize` korrekt in `buffer1` ein und setzt den Null-Terminator. Danach prüft Zeile 363:
  `if (pos >= mysize) throw EndOfFileException();`.
  Da das Dateiende erreicht wurde, wird eine Exception geworfen – und der Aufrufer erhält die erfolgreich gelesene
  letzte Zeile niemals zurück!
  Fix: Exception darf nur geworfen werden, wenn vor dem Lesen bereits `pos >= mysize` war (wie am Anfang der Methode).

  ==> FIXED
---

## Bugs (mittel)

- [ ] **`fgetws()` liest ungeprüft über das Pufferende und riskiert Alignment-Fehler** (MemFile.cpp:369-390)
  1. Anders als in `fgets()` fehlt in `fgetws()` die Beschränkung `if (pos + by > mysize) by = mysize - pos;`.
     Läuft ein UTF-16/32 String ohne L'\n' bis zum Dateiende, liest die Schleife beliebig weit über `MemBase + mysize`
     hinaus (Heap-Overread).
  2. `wchar_t* ptr = (wchar_t*)(MemBase + pos);`: Wenn `pos` ungerade ist, ist `ptr` nicht auf 2- oder 4-Byte-Grenzen
     ausgerichtet. Auf Architekturen mit Alignment-Anforderungen führt dies zu einem Hardware-Trap (SIGBUS).
  Fix: Bounds-Clamping wie in `fgets` einbauen und Lesezugriffe über Byteweise Kopie oder Alignment-Prüfung absichern.

- [ ] **`fgetwc()` gibt bei EOF uninitialisierten Speicher zurück** (MemFile.cpp:431-436)
  ```cpp
  wchar_t MemFile::fgetwc()
  {
      wchar_t buf[1];
      fread(buf, sizeof(wchar_t), 1);
      return buf[0];
  }
  ```
  Wenn `fread` 0 Elemente liest (weil `pos >= mysize`), bleibt `buf[0]` uninitialisierter Stack-Müll und wird zurückgegeben.
  Fix: Rückgabewert von `fread` prüfen; wenn 0, `EndOfFileException` werfen.

- [ ] **`open(const ByteArrayPtr& memory)` setzt `buffersize` nicht zurück** (MemFile.cpp:162-174)
  Wird ein zuvor beschriebenes `MemFile` (`buffersize > 0`) per `open(byteArrayPtr)` wiederverwendet, wird zwar
  `buffer` freigegeben, `buffersize` bleibt jedoch auf dem alten Wert stehen.
  Fix: `buffersize = 0;` ergänzen.

- [ ] **Inkonsistenter `isOpen()`-Status bei Default-Konstruktion** (MemFile.cpp:239-242, 317-323, 438-445)
  Nach `MemFile f;` (Standardkonstruktor):
  - `f.isOpen()` liefert `false` (prüft `MemBase != NULL`).
  - `f.tell()`, `f.seek()`, `f.eof()` prüfen `if (MemBase != NULL || readonly == false)` und liefern Erfolg (`0`, `true`).
  - `f.fread()` prüft `if (MemBase == NULL)` und wirft `FileNotOpenException`.
  Fix: Einheitliche Definition: Ein Dateiobjekt ohne Puffer ist entweder geschlossen (`isOpen() == false`), oder
  ein neu erstelltes, beschreibbares `MemFile` gilt als leer geöffnete Datei (`mysize=0, pos=0`).

- [ ] **Mögliche Ganzzahl-Überläufe in `resizeBuffer` und `fwrite`** (MemFile.cpp:225, 338)
  - `(((size + 8191) >> 13) << 13)` überläuft bei Werten nahe `SIZE_MAX`.
  - `size_t bytes = nmemb * size;` in `fwrite` prüft nicht auf Multiplikations-Überlauf vor der Addition zu `pos`.

---

## Design & Architekturvorschlag

### Warum die bisherige Implementierung so fehleranfällig ist:
`MemFile` versucht drei grundverschiedene Verantwortlichkeiten in einer Klasse mit zwei Zeigern (`buffer` und `MemBase`)
zu vereinen:
1. **Dynamischer Schreib-/Lese-Puffer**: Allokiert eigenen Speicher via `malloc`/`realloc`, wächst bei `fwrite()`,
   Besitz liegt bei `MemFile`.
2. **Read-Only View**: Schaut nur auf fremden Speicher (`const void*`, `ByteArrayPtr`), darf nichts freigeben oder verändern.
3. **Puffer-Adoption**: Versucht fremden Speicher zu übernehmen und per `realloc`/`free` weiterzuverwalten (höchst fehlerträchtig).

### Empfohlene Bereinigung:
1. **Strikte Trennung von View und Dynamic Buffer**:
   - Entweder: `MemFile` nutzt intern für den dynamischen Modus einen echten RAII-Container (wie `std::vector<char>`
     oder `pplib::ByteArray`), womit manuelle `malloc`/`realloc`/`free`-Aufrufe und Leaks komplett entfallen.
   - Oder: `MemFile` ist rein für das Lesen/Schreiben im Speicher da; fremder Speicher wird für Lesezwecke nur
     als `const void*` referenziert (keine Adoption per `writeable=true`).
2. **Destruktor:** Muss `close()` aufrufen, damit dynamische Puffer freigegeben werden.
3. **Rule of Five:** `delete` für Copy-Konstruktor und Zuweisungsoperator; Move-Operationen unterstützen.

---

## Doku / Kosmetik

- Sämtliche Doxygen-Dokumentation befindet sich historisch in `src/core/MemFile.cpp` statt im Header `include/pplib/core/memfile.h`.
- Unnötiger Cast in `size()`: `return (int64_t)mysize;` bei Rückgabetyp `uint64_t`.
- Die Doku zu `open()` behauptet `Wird der Parameter writeable auf true gesetzt, geht die Verwaltung des Speichers an die MemFile-Klasse über`, was in C++ ohne Allocator-Garantie eine gefährliche Falle darstellt.
