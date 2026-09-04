# AssocArray Review (Zweiter Durchgang) – Befunde

Zweit-Review nach `done/assocarray-review.md` (Erstreview durch Qwen3.8-27B). Scope: `include/pplib/types/assocarray.h` +
`src/types/AssocArray.cpp` (aktueller Stand, vollständig neu gelesen, unabhängig vom alten Dokument). Zusätzlich gelesen:
`todo/variant-review.md` (offener Self-Assignment/Aliasing-Bug in `Variant::set()`), `include/pplib/types/variant.h`,
`src/types/Variant.cpp` (`clear()`, `set(const Variant&)`, `set(const AssocArray&)`, `toAssocArray()` gegengeprüft),
`include/pplib/exceptions.h` (verwendete Exception-Namen `InvalidKeyException`, `KeyNotFoundException`,
`TypeConversionException`, `ImportFailedException`, `ExportBufferToSmallException`, `IllegalArgumentException` – alle real vorhanden),
`done/bytearray-review.md` (Kalibrierung Tiefe/Rigor), `src/types/String.cpp` (`isNumeric()`, `toInt64()`).

**Empirisch verifiziert** gegen `build/coverage/libpplib_core.a` (g++ 17, `-fsanitize=address`, eigene Testprogramme gegen die
gebaute Lib gelinkt; die Lib selbst ist nicht mit ASan instrumentiert, ASan fängt daher nur Effekte, die bis in eigenen,
instrumentierten Code oder bis zur glibc-Allokator-Ebene durchschlagen – für die gefundenen Bugs war das ausreichend, s.u.):
Self-Aliasing über `set(key, get...(key))` (3 Varianten), Kollision großer numerischer Keys, Move-Assignment-Restzustand,
negativer Key wird umgeschrieben. Testprogramme liegen unter
`/tmp/claude-1000/-home-patrickf-git-ppl8/d6f044e8-1b6c-4022-9e6b-90280b45f5ca/scratchpad/test_*.cpp`.

## Fix-Verifikation (bereits als FIXED markierte Punkte)

- **Self-Assignment `operator=`** (`AssocArray.cpp:723-724`): `if (this == &other) return *this;` vorhanden. OK.
- **Self-Reference in `add()`/`operator+=`** (`AssocArray.cpp:220`): `if (this == &other) return;` vorhanden. OK.
- **`operator=(AssocArray&&)` fehlte** (`AssocArray.cpp:731-738`): jetzt definiert. Grundsätzlich OK, aber siehe
  **Bugs (mittel)** unten – die Funktion setzt `other.maxint` nicht zurück, anders als der Move-Konstruktor.
- **`importBinary` ohne Bounds-Checks**: im alten Review als `TODO` vermerkt, im aktuellen Code aber tatsächlich mit
  durchgängigen `if (p + n > buffersize) throw ImportFailedException(...)`-Prüfungen versehen (`AssocArray.cpp:582-707`).
  Der Status im alten Dokument ist damit veraltet (Fix wurde nachträglich gemacht, aber nicht im Dokument nachgetragen) –
  bis auf eine verbliebene Lücke im `TYPE_DATETIME`-Zweig, siehe **Bugs (mittel)** unten.
- **`maxint` nach Import**: Magic-Byte `"PPL8ASOC"` + Version 1 + expliziter `maxint`-Wert im Header (`AssocArray.cpp:430-435`
  Export, `584-588` Import) vorhanden und funktionsfähig. OK.
- **`createTree` hinterlässt UNKNOWN-Node bei BadAlloc, `findInternal` prüft darauf** (`AssocArray.cpp:107-110`): Guard
  `if (it->second->type() == Variant::TYPE_UNKNOWN) return NULL;` vorhanden. OK.
- **`erase` lässt leere Sub-Arrays stehen**: Verhalten wie beschrieben in der Doku dokumentiert (`assocarray.h:485-497`). OK.
- **Export eines UNKNOWN-Variants schreibt Type-Byte 0 (Stream-Terminator)**: im alten Review als `FIXED` markiert
  ("wir ignorieren TYPE_UNKNOWN und exportieren ihn nicht"). **Das trifft auf den aktuellen Code nicht zu** – siehe
  „Fix unvollständig/fehlerhaft" unter **Bugs (kritisch)**.

## Bugs (kritisch)

- [ ] **Fix unvollständig/fehlerhaft: `exportBinary` überspringt TYPE_UNKNOWN-Knoten immer noch nicht** (`AssocArray.cpp:436-545`)
  Der alte Review vermerkt zu diesem Punkt „=> FIXED: ... wir ignorieren TYPE_UNKNOWN und exportieren ihn nicht.". Der
  aktuelle Export-Loop enthält aber **keinerlei** Sonderbehandlung für `TYPE_UNKNOWN`:
  ```cpp
  for (it = Tree.begin(); it != Tree.end(); ++it) {
      const Variant* a = it->second;
      if (p < buffersize) {
          if (a->isByteArrayPtr())
              PokeN8(ptr + p, Variant::TYPE_BYTEARRAY);
          else
              PokeN8(ptr + p, a->type());   // <- a->type() ist 0 (TYPE_UNKNOWN), kein continue/skip davor
      }
      p++;
      key = it->first;
      ...                                    // Key wird trotzdem geschrieben
      if (a->isString()) { ... }
      else if (a->isWideString()) { ... }
      ...
      else if (a->isTimeZone()) { ... }
      // kein Zweig für TYPE_UNKNOWN -> keine Value-Bytes werden geschrieben
  }
  ```
  Für einen `TYPE_UNKNOWN`-Knoten (entsteht laut demselben Review-Dokument bei `bad_alloc` in `createTree()`/`set()`) wird
  also weiterhin ein Eintrag mit Type-Byte `0` geschrieben – exakt der Wert, den der Import-Parser als Stream-Terminator
  interpretiert: `while (p + 2 < buffersize && (type = PeekN8(ptr + p)) != 0)`. Der Import bricht an dieser Stelle
  kommentarlos ab und verwirft **diesen und alle nachfolgenden Keys** auf dieser Ebene, ohne Exception.
  Ein dynamischer Repro ist schwierig (erfordert echten `bad_alloc` in `Variant::set()` während `createTree()`), der
  fehlende Skip ist aber durch reine Code-Inspektion eindeutig belegt (kein `if (a->type() == Variant::TYPE_UNKNOWN) continue;`
  im gesamten Loop).
  Fix: am Anfang der Loop-Iteration `if (a->type() == Variant::TYPE_UNKNOWN) continue;` ergänzen (wie im alten Review
  bereits vorgeschlagen).

- [ ] **Große numerische Keys kollidieren durch Sättigung von `String::toInt64()` im Comparator – stiller Datenverlust** (`assocarray.h:91-105` `ArrayKeyCompare`, `AssocArray.cpp:126-130` `createTree`, `src/types/String.cpp:1224-1232` `toInt64()`)
  `ArrayKeyCompare` sortiert zwei numerische Keys per `a.toInt64() < b.toInt64()`. `String::toInt64()` ruft `strtoll()` auf,
  das bei Eingaben außerhalb `[INT64_MIN, INT64_MAX]` auf `LLONG_MIN`/`LLONG_MAX` **sättigt** (C-Standardverhalten von
  `strtoll`, kein Overflow-Check im Aufrufer). Zwei unterschiedliche, aber beide zu große Zahlen-Strings ergeben also
  denselben `toInt64()`-Wert und werden vom Comparator als **identischer Key** behandelt – der zweite `set()` überschreibt
  den ersten kommentarlos, keine Exception, kein Hinweis. Reicht rein über die öffentliche API, keine Manipulation von
  Binärdaten nötig:
  ```cpp
  AssocArray a;
  a.set("99999999999999999999", "v1");   // 20 Neunen, reine Ziffernfolge
  a.set("88888888888888888888", "v2");   // andere, ebenfalls 20-stellige Zahl
  // a.size() == 1 (!)  – beide Keys wurden auf denselben Eintrag "9223372036854775807" abgebildet, "v1" ist weg
  ```
  Empirisch verifiziert (Ausgabe von `test_bigkey.cpp`):
  ```
  size = 1
  key='9223372036854775807'
  ```
  Das ist keine rein akademische Grenze: jeder 64-Bit-Unsigned-Wert oberhalb `INT64_MAX` (z.B. Hashes, IDs, Nanosekunden-Timestamps),
  der als Klartext-Key verwendet wird (nicht über `"[]"`, sondern direkt z.B. `a.set(hashToString(x), ...)`), ist betroffen –
  nicht nur absurd lange Ziffernfolgen.
  Verschärfend kommt eine zweite, verwandte Variante hinzu: **negative Keys werden zusätzlich in einen sinnlosen Wert umgeschrieben**.
  In `createTree()`:
  ```cpp
  if (firstkey.isNumeric()) {                  // isNumeric() erlaubt ein führendes '-' (String.cpp:168-185)
      uint64_t keyint = firstkey.toInt64();     // int64_t -5 -> implizite Konversion nach uint64_t = 18446744073709551611
      if (keyint >= maxint) maxint = keyint + 1;
      firstkey.setf("%llu", keyint);            // Key wird zu "18446744073709551611" umbenannt!
  }
  ```
  Empirisch verifiziert (`test_negkey.cpp`):
  ```cpp
  a.set("-5", "negative key test");
  a.exists("-5");   // false!
  // tatsächlicher Key in der Map: "18446744073709551611"
  ```
  und in Kombination mit der Sättigung kollidieren dann sogar unterschiedliche negative Keys miteinander (`test_negkey2.cpp`:
  `a.set("-1", ...); a.set("-2", ...);` → `a.size() == 1`, gespeicherter Key `"18446744073709551615"`, nur der zweite Wert bleibt übrig).
  Das ist eine deutlich konkretere/schädlichere Ausprägung als der im alten Review nur am Rande erwähnte Punkt
  „negativer Key macht `maxint` als `uint64_t` zu einer riesigen Zahl" (der alte Review hat nur den Seiteneffekt auf
  `maxint` gesehen, nicht dass der gespeicherte Key-Text selbst dadurch überschrieben wird bzw. dass beliebig viele
  Keys kollidieren können). Auch die „Verifiziert OK"-Aussage des alten Reviews („Comparator ist eine gültige Strict Weak
  Ordering ... kein UB in der std::map") gilt nur für Keys im normalen Wertebereich – für Keys, die `toInt64()` sättigen,
  bilden zwar weiterhin gültige (wenn auch degenerierte) Äquivalenzklassen, das Verhalten ist aber keinesfalls UB im Sinne
  eines kaputten Total-Order, sondern schlicht sehr aggressiver, für den Aufrufer unsichtbarer Datenverlust.
  Fix-Vorschläge:
  - `createTree()`: `uint64_t keyint = firstkey.toInt64();` durch eine Prüfung ersetzen, die negative bzw. nicht in
    `uint64_t` überführbare Werte als **nicht-numerisch** behandelt (z.B. `if (firstkey[0] == '-') { /* wie Text behandeln */ }`),
    statt sie stillschweigend zu casten und den Key-Text zu überschreiben.
  - `ArrayKeyCompare`: für sehr lange Ziffernfolgen (mehr als ~19-20 Stellen) nicht auf `toInt64()` verlassen, sondern
    vorher auf Länge vergleichen bzw. bei Überlänge als Text behandeln, damit keine zwei unterschiedlichen Strings auf
    denselben Vergleichswert abgebildet werden.

- [ ] **Aliasing über `set(key, get...(key))` – Use-after-free / stiller Datenverlust, weil `createTree()` den Zielknoten vor dem Kopieren löscht** (`AssocArray.cpp:142-144` `createTree`, zusammen mit dem in `todo/variant-review.md` dokumentierten Aliasing-Bug in `Variant::set()`/`clear()`)
  `createTree()` leert einen bereits existierenden Zielknoten **bevor** der Aufrufer (`AssocArray::set(key, value)`) den
  neuen Wert hineinschreibt:
  ```cpp
  iterator it = Tree.find(firstkey);
  if (it != Tree.end()) {
      if (tok.count() > 0) { ... }
      it->second->clear();   // <- Zielknoten wird JETZT schon geleert/freigegeben
      return it->second;     // Aufrufer schreibt erst danach: createTree(key)->set(value)
  }
  ```
  Übergibt der Aufrufer als `value` eine Referenz, die aus **demselben Key** stammt (z.B. via `getString()`, `getAssocArray()`,
  `get()`), zeigt `value` auf genau das Objekt, das `clear()` gerade zerstört – ein Spezialfall des in `todo/variant-review.md`
  beschriebenen Aliasing-Problems, hier aber zusätzlich durch `createTree()`s eigenes „erst löschen, dann zurückgeben"
  verschärft (der Bug tritt selbst dann noch auf, wenn `Variant::set()` irgendwann nach dem Muster aus dem Variant-Review
  gefixt würde, weil `createTree()` den Knoten schon *vor* dem Aufruf von `set()` leert).
  Drei empirisch verifizierte Ausprägungen:
  1. **Scalar String → harter Absturz (SEGV)**:
     ```cpp
     AssocArray a;
     a.set("s", "some longer string value to avoid small string optimization games");
     String& ref = a.getString("s");
     a.set("s", ref);   // value referenziert exakt den String, den createTree() gerade freigibt
     ```
     Ergebnis unter ASan (`test_uaf.cpp`):
     ```
     ==...==ERROR: AddressSanitizer: SEGV on unknown address 0x0000000000cb
         #2 pplib::String::set(char const*, unsigned long) String.cpp:225
         #3 pplib::String::set(pplib::String const&, unsigned long) String.cpp:286
         #4 pplib::String::String(pplib::String const&) String.cpp:91
         #5 pplib::Variant::set(pplib::String const&) Variant.cpp:256
         #6 pplib::AssocArray::set(pplib::String const&, pplib::String const&) assocarray.h:328
     ```
     Absturz beim Lesen aus bereits freigegebenem Speicher (`rax=0xcb`, klassisches Freed-Memory-Füllmuster).
  2. **Verschachteltes AssocArray → stiller Datenverlust** (`test_uaf1.cpp`):
     ```cpp
     AssocArray a;
     a.set("x/inner", "hello world this is a fairly long string to defeat SSO");
     AssocArray& ref = a.getAssocArray("x");
     a.set("x", ref);
     // a.exists("x/inner") == false  -> der komplette Unterbaum ist weg
     ```
  3. **Generischer `Variant`-Alias** (`test_uaf3.cpp`):
     ```cpp
     AssocArray a;
     a.set("v", "another longer string for variant alias test case, long enough");
     a.set("v", a.get("v"));
     // a.exists("v") == false
     ```
  Alle drei Fälle sind über die rein öffentliche `AssocArray`-API erreichbar, ganz ohne Kenntnis interner Implementierungsdetails
  – ein naheliegender Anwendungsfall ist z.B. „Wert normalisieren/neu setzen": `a.set(key, transform(a.get(key)))`, wo
  `transform()` bei einem Fehler/Copy-Elision-Sonderfall die Originalreferenz zurückgibt, oder schlicht `a.set(k, a.getString(k))`
  als (vermeintlich harmloses) No-Op.
  Fix: `createTree()` darf den Zielknoten nicht *vor* dem Schreiben des neuen Werts leeren. Zwei Optionen:
  - `createTree()` gibt bei einem bereits vorhandenen Blatt-Knoten den Knoten **ungeleert** zurück; das Leeren geschieht
    dann implizit durch `Variant::set()` selbst – vorausgesetzt, `Variant::set()` wird (wie in `todo/variant-review.md`
    vorgeschlagen) so umgebaut, dass der neue Wert zuerst konstruiert und erst danach `clear()` aufgerufen wird
    ("construct-before-clear").
  - Zusätzlich/alternativ: In den `AssocArray::set(key, T)`-Inline-Wrappern (`assocarray.h:326-431`) einen expliziten
    Alias-Check ergänzen, bevor `createTree()` aufgerufen wird (z.B. `if (findInternal(key) == &value) return;` bzw.
    Wert vorher kopieren). Das ist der robustere Fix, da er unabhängig vom internen `Variant`-Verhalten funktioniert.

## Bugs (mittel)

- [ ] **`operator=(AssocArray&&)` setzt `other.maxint` nicht zurück – inkonsistenter Moved-from-Zustand** (`AssocArray.cpp:731-738`)
  ```cpp
  AssocArray& AssocArray::operator=(AssocArray&& other) noexcept
  {
      if (this == &other) return *this;
      clear();
      Tree = std::move(other.Tree);
      maxint = other.maxint;
      return *this;
      // other.maxint wird NICHT zurückgesetzt (anders als im Move-Konstruktor, Zeile 66-72: "other.maxint = 0;")
  }
  ```
  `std::map::operator=(&&)` leert `other.Tree` (in der Praxis mit libstdc++ zuverlässig, auch wenn der Standard das nicht
  hart garantiert), aber `other.maxint` behält seinen alten Wert. Ein danach auf `other` ausgeführtes `set("[]", ...)`
  erzeugt dadurch einen "hohen" Key statt bei `0` zu beginnen, obwohl `other` leer aussieht. Empirisch verifiziert
  (`test_move.cpp`):
  ```
  src.size() after move = 0
  src after set([],x): size=1, key of new element = 3
  ```
  Fix: analog zum Move-Konstruktor `other.maxint = 0;` ergänzen.

- [ ] **`importBinary`: fehlender Bounds-Check im `TYPE_DATETIME`-Zweig für `0 < vallen < 10`** (`AssocArray.cpp:649-666`)
  ```cpp
  case Variant::TYPE_DATETIME: {
      if (p + 4 > buffersize) throw ImportFailedException(...);
      vallen = PeekN32(ptr + p);
      p += 4;
      DateTime dt;
      if (vallen >= 10) {
          if (p + vallen > buffersize) throw ImportFailedException(...);   // Check nur in diesem Zweig
          ...
      }
      // vallen könnte auch 0 sein, wenn das DateTime invalid ist
      p += vallen;              // <- bei 0 < vallen < 10 KEIN Bounds-Check vor diesem Update!
      set(key, dt);
  } break;
  ```
  Bei einem manipulierten/kaputten Buffer mit `vallen` z.B. `= 5` wird `p` um 5 erhöht, ohne dass geprüft wird, ob der
  Buffer überhaupt noch 5 weitere Bytes hat. Das führt zu keinem OOB-Read (reine Zeiger-Arithmetik, es wird nichts an
  Position `p` gelesen), aber `p` kann dadurch über `buffersize` hinauswachsen; die äußere `while`-Schleife bricht dann
  beim nächsten Bedingungscheck `p + 2 < buffersize` kommentarlos ab – alle **restlichen Keys** auf dieser Ebene (und bei
  einem verschachtelten `TYPE_ASSOCARRAY`-Import sogar auf der Elternebene, da der zurückgegebene `bytes`-Wert dann zu groß
  ist) werden stillschweigend verworfen, statt die sonst überall in dieser Funktion konsequent geworfene
  `ImportFailedException` auszulösen. Bricht den sonst durchgängigen "jede Inkonsistenz wirft eine Exception"-Vertrag der Funktion.
  Fix: Bounds-Check aus dem `if (vallen >= 10)`-Zweig herausziehen und unconditional vor `p += vallen;` prüfen:
  `if (p + vallen > buffersize) throw ImportFailedException(...);`.

## Design

- [ ] **Numerischer Key-Comparator ist nicht robust gegen Zahlen außerhalb des `int64_t`-Wertebereichs** (siehe Bug oben)
  – grundsätzlicher gehört das in eine Überarbeitung von `ArrayKeyCompare`/`createTree()`: die aktuelle Strategie „String
  parsen, wenn's aussieht wie eine Zahl" hat keinerlei Sicherheitsnetz gegen Parse-Sättigung. Ein robusteres Muster wäre
  z.B. `strtoll`+`errno`-Check und bei `ERANGE` den Key als Text statt als Zahl behandeln.
- [ ] **`exportBinary`/`importBinary`: Key-Länge auf 16 Bit begrenzt** (`AssocArray.cpp:448`, `594`)
  `PokeN16(ptr + p, (int)keylen)` / `size_t keylen = PeekN16(ptr + p);` – Keys länger als 65535 Zeichen werden beim Export
  stillschweigend auf die unteren 16 Bit der Länge abgeschnitten (keine Exception, kein Hinweis), während Value-Längen an
  anderer Stelle konsequent 32 Bit breit sind. Sehr unwahrscheinlicher Fall in der Praxis, aber inkonsistent zum Rest der
  Funktion und leicht mit einer expliziten Prüfung (`if (keylen > 0xFFFF) throw ...`) abzusichern.
- [ ] **`p + vallen > buffersize`-Prüfungen in `importBinary` sind auf 32-Bit-`size_t`-Plattformen theoretisch durch
  Integer-Overflow umgehbar** (durchgängig in `importBinary`, z.B. `AssocArray.cpp:601-606`)
  `vallen` wird aus einem 32-Bit-Feld gelesen (bis zu ~4 GiB), `p` ist zu diesem Zeitpunkt ein kleiner Wert. Auf 64-Bit-
  Systemen (dort ist `size_t` 64 Bit) kann `p + vallen` nicht überlaufen, die Prüfung ist also sicher. Baut PPLIB auch für
  32-Bit-Zielsysteme (Windows x86 wird an anderer Stelle im Repo erwähnt), könnte `p + vallen` dort umlaufen und die
  Prüfung fälschlich passieren lassen, was echte Out-of-Bounds-Reads ermöglichen würde. Nur relevant, falls 32-Bit-Builds
  tatsächlich unterstützt werden sollen – falls nicht, kein Handlungsbedarf.

## Doku / Kosmetik

- [ ] `append(key, value, concat)` (`AssocArray.cpp:196-206`) ruft `str.append(value)` auf, ohne einen Alias-Check.
  `a.append(key, a.getString(key))` (Selbst-Anhängen) ist über die öffentliche API erreichbar; ob `String::append()`
  Selbst-Referenzierung sicher handhabt (insbesondere bei einer Reallokation mitten im Kopiervorgang), wurde in diesem
  Review nicht verifiziert (gehört eher in ein `String`-Review). Empfehlung: bei Gelegenheit dort mit-prüfen, da der
  Aufrufpfad hier real existiert.
- [ ] Die Klassendoku (`assocarray.h:63-65`) verspricht nur "Gross-/Kleinschreibung wird ignoriert", macht aber keine
  Aussage zu numerischen Keys jenseits des normalen Bereichs. Angesichts der oben gefundenen Kollisionen wäre ein Hinweis
  sinnvoll, dass Keys, die rein aus Ziffern bestehen, als 64-Bit-Zahl interpretiert werden und daher nicht beliebig groß
  oder negativ sein sollten.

## Verifiziert OK (kein Handlungsbedarf)

- Self-Assignment `operator=`, Self-Reference in `add()`/`operator+=`, `operator=(AssocArray&&)`-Existenz, `maxint`-Erhalt
  beim Binary-Import, `findInternal`-Guard gegen `TYPE_UNKNOWN`-Knoten, dokumentiertes `erase()`-Verhalten: alle wie im
  alten Review als „FIXED"/„Bleibt so" vermerkt, im aktuellen Code bestätigt (siehe Fix-Verifikation oben).
- Move-Konstruktor (`AssocArray(AssocArray&&)`, Zeile 66-72) setzt `other` korrekt zurück (`maxint = 0`, `Tree.clear()`)
  – im Gegensatz zum Move-Assignment-Operator (siehe Bugs (mittel)).
- `importBinary` hat inzwischen (anders als im alten Review als `TODO` vermerkt) durchgängige Bounds-Checks; einzige
  gefundene Lücke ist der `TYPE_DATETIME`-Sonderfall oben.
- `operator==`/Byte-Vergleich-Verhalten bei unterschiedlicher Key-Schreibweise: bewusste, bereits diskutierte
  Design-Entscheidung ("Bleibt so"), nicht erneut aufgegriffen.
