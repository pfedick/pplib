# AssocArray Review – Befunde

Review vom 2026-08-26, Scope: `include/pplib/types/assocarray.h` + `src/types/AssocArray.cpp` (1394 Zeilen).
Abhängigkeiten in `variant.h`, `Variant.cpp`, `datetime.h` verifiziert.

* Review done by: Qwen3.8 27b

## Bugs

- [ ] **Self-Assignment zerstört Daten** – `operator=` (Zeile ~1111)
  `clear()` leert `*this`, dann iteriert `add(other)` über das jetzt leere Array → `a = a;` ergibt ein leeres Array.
  Fix: `if (this != &other)` guard oder Copy-and-Swap.

  => FIXED

- [ ] **Self-Reference in `add()` / `operator+=` zerstört Leaf-Werte** (Zeile ~448)
  Bei `a += a`: `set(it->first, *it->second)` findet Ziel = Quelle, `createTree` ruft `it->second->clear()` →
  Quellwert wird gelöscht, bevor er kopiert wird. Alle Leaf-Werte gehen verloren.
  Fix: Self-Check am Anfang von `add()`.

  => FIXED

- [ ] **`operator=(AssocArray&&)` deklariert, aber nie definiert**
  Im Header (Operatoren-Gruppe) deklariert, im .cpp existiert nur die Copy-Version.
  Jeder Move-Assign (`a = std::move(b)`, z.B. durch `std::vector<AssocArray>`) → Linker-Fehler.
  Fix: definieren (Map moven, `other` resetten) oder Deklaration entfernen.

  => FIXED

- [ ] **`importBinary` ohne Bounds-Checks** (ab Zeile ~960)
  `keylen`, `vallen`, `elements` werden blind aus dem Buffer gelesen – manipuliertes/abgeschnittenes Binary
  → OOB-Reads (`PeekN16`/`PeekN32` jenseits von `buffersize`, `String(ptr+p, vallen)` mit riesigem `vallen`).
  Fix: vor jedem Read prüfen `p + n <= buffersize`, sonst `ImportFailedException`.

  => TODO

- [ ] **`importBinary` stellt `maxint` nicht wieder her**
  Nach dem Import startet die `[]`-Auto-Nummerierung bei 0. Hat das Binary numerische Keys (`"0"`, `"1"`),
  überschreibt der nächste `set("[]", ...)` still importierte Werte (`createTree` macht `clear()` + set).
  Fix: beim Import den max. numerischen Key tracken und `maxint` daraus setzen.

  => Das Problem ist auch bein mergen/hinzufügen weiterer AssocArrays vorhanden
  => Import- und Export verwenden nun das Magic-Byte "PPL8ASOC" und die Version 1. Die Export-Funktion schreibt den maxint-Wert in das Binary, die Import-Funktion liest ihn wieder ein. Damit ist das Problem behoben.

- [ ] **`operator==` vergleicht Serialisierungs-Bytes → Key-Schreibweise-sensitiv** (Zeile ~1132)
  ```cpp
  a.set("Foo", "1"); b.set("foo", "1");   // a == b → false!
  a.set("1", "x");   b.set("01", "x");    // a == b → false!
  ```
  Laut Header-Vertrag ist die Schreibweise egal, `findInternal` behandelt beide als gleich –
  aber der Byte-Vergleich sieht die gespeicherte Original-Schreibweise.
  Fix: strukturell vergleichen (Keys iterieren, per `findInternal` auf beiden Seiten abgleichen) statt Bytes.

  => Interessantes Problem. Um dem zu entgehen, könnte man die Keys in der Map immer in einer Normalform speichern (z.B. immer lowercase). Was war der Grund, dass wir das nicht gemacht haben?

  => Bleibt so

- [ ] **`fromConfig`: `Row[-1]` – negativer Index** (OLDCODE-Block, Zeile ~1290)
  ```cpp
  if (Row[0] == L'[' && Row[-1] == L']') {
  ```
  Sollte `Row[Row.len()-1]` sein. OOB-Read, sobald der Block wieder aktiviert wird.

  => Out of Scope, da der Codeblock deaktiviert ist. Bleibt so.

## Risiken / Edge Cases

- [ ] **Export eines UNKNOWN-Variants schreibt Type-Byte 0 = Stream-Terminator** → Import bricht dort still ab.
      Nur erreichbar, wenn ein `set()` nach `createTree` geworfen hat (BadAlloc) und den leeren Node zurückgelassen hat.
      Defensive Fix: `if (a->type() == TYPE_UNKNOWN) continue;` im Export-Loop.

      => Verwendung eines anderen Terminators? Nein, wir ignorieren TYPE_UNKNOWN und exportieren ihn nicht.
      => FIXED 

- [ ] **`createTree` hinterlässt UNKNOWN-Nodes bei BadAlloc**: `new Variant()` wird in die Map gehängt,
      dann wirft `set()` → der Node bleibt als `TYPE_UNKNOWN` liegen. Folge: `get()` wirft danach
      `EmptyDataException` statt `KeyNotFoundException`.

      => FIXED: die Exception abzufangen und den Knoten zu löschen wäre zu aufwändig für einen Fall, der nur bei BadAlloc vorkommt. Wir lassen den Variant daher als TYPE_UNKNOWN stehen, prüfen aber in findInternal darauf und geben NULL zurück, damit get() die KeyNotFoundException wirft.

- [ ] **`erase` lässt leere Sub-Arrays stehen** – Design-Entscheidung, aber sollte dokumentiert
      oder mit-gepruned werden.

      => Bleibt so. Der erase geht auf einen bestimmten key. Dieser wird gelöscht. Wird dadurch ein Eltern-Array leer, so bleibt es bestehen. Wenn der zu löschende Key auf ein Eltern-Array zeigt, dann wird es gelöscht. Das ist ein Design-Entscheidung, die so bleiben soll.
      => Verhalten dokumentiert in `erase()`.

- [ ] **Numerische Key-Äquivalenz ist implizit**: `"1"` und `"01"` sind derselbe Key (Comparator),
      `exists("01")` nach `set("1", ...)` liefert true, Iteration zeigt die Original-Schreibweise.
      Wahrscheinlich beabsichtigt – gehört in die Doku. Randfall: `isNumeric()` akzeptiert ein führendes Minus →
      negativer Key macht `maxint` als `uint64_t` zu einer riesigen Zahl.

      => Gedanken dazu: der numeric-key ist ein Workaround, um im AssocArray sowas wie ein Array bzw. eine Liste abzubilden, deren Elemente auch wieder AssocArrays sein können. Vielleicht wäre es besser für diesen Anwendungsfall eine eigene Klasse zu haben, die eine Liste von AssocArrays abbildet?
      
      => Aufwand lohnt sich nicht, bleibt so.

- [ ] **DateTime-Import mit `vallen` in (0,8) oder (8,10)**: weder Legacy- noch PPL8-Zweig greift →
      leeres DateTime wird still importiert statt Exception.

      => Das ist so gewollt. DateTime kann Invalid sein. In dem Fall wird vallen 0 sein. Beim Import entsteht dann wieder ein Invalid DateTime. Größe von 8 ist Legacy PPL7-Datentyp. Größe von >=10 ist PPL8.

## Verifiziert OK (kein Handlungsbedarf)

- exportBinary-Größenlogik: Checks (`p + 4 < buffersize` für 2-Byte-Felder etc.) sind konservative Puffer –
  mit `buffersize == binarySize()` gehen alle Writes durch, inkl. verschachtelter Arrays (Budget wird korrekt weitergereicht).
- Comparator ist eine gültige Strict Weak Ordering (NUM vor TEXT, transitiv) → kein UB in der `std::map`.
- Copy-Konstruktor rechnet `maxint` korrekt neu aus; Move-Konstruktor setzt `other` sauber zurück.
- ByteArrayPtr→ByteArray-Roundtrip ist dokumentiert und funktioniert.
