# Crc Review – Befunde

Review vom 2026-09-01, Scope: `src/math/crc.cpp` (106 Zeilen). Öffentliche API: `uint32_t Crc32(const void*, size_t,
uint32_t initial_crc = 0xFFFFFFFF)` und `uint16_t Crc16(const void*, size_t, uint16_t initial_crc = 0xFFFF)`,
dokumentiert in `include/pplib/core/functions.h:52-78`. Beide Tabellen und Algorithmen empirisch gegen
`build/coverage/libpplib_core.a` mit dem CRC-Standard-Testvektor `"123456789"` verifiziert:
`Crc32("123456789") == 0xCBF43926` ✓ (Standard-CRC-32-Check-Value), `Crc16("123456789") == 0x29B1` ✓
(passt zur verwendeten Parametrisierung CRC-16/CCITT-FALSE: Poly 0x1021, Init 0xFFFF, kein RefIn/RefOut/XorOut).

* Review done by: Claude Sonnet 5

## Bugs (mittel)

- [ ] **`initial_crc` erlaubt bei `Crc32()` kein Fortsetzen einer Prüfsumme über mehrere Aufrufe – anders als bei `Crc16()` und von der Doku nahegelegt** (Zeile 67-75; Doku in `functions.h:75` "Nützlich zum Fortsetzen einer Checksumme")
  ```cpp
  uint32_t Crc32(const void* buffer, size_t size, uint32_t initial_crc)
  {
      uint32_t ulCRC = initial_crc;
      ...
      return ulCRC ^ 0xffffffff;   // <- Finalisierung wird bei JEDEM Aufruf angewendet
  }
  ```
  `Crc16()` (Zeile 95-103) gibt den rohen Laufwert ohne Schluss-XOR zurück, wodurch `initial_crc` dort tatsächlich
  zum Fortsetzen einer Prüfsumme über mehrere Chunks verwendet werden kann. `Crc32()` wendet dagegen bei jedem
  Aufruf `^ 0xffffffff` an – reicht man das (bereits finalisierte) Ergebnis eines Aufrufs als `initial_crc` in den
  nächsten Aufruf hinein, wird die interne CRC-Registerkette dadurch verfälscht. Empirisch verifiziert:
  ```
  Crc32("123456789", 9)                              = 0xCBF43926   // Referenzwert, am Stück berechnet
  Crc32("56789", 5, Crc32("1234", 4))                 = 0xF22931C4   // falsch! naive Chunk-Verkettung
  // zum Vergleich: Crc16 verkettet korrekt:
  Crc16("56789", 5, Crc16("1234", 4)) == Crc16("123456789", 9)       // beides 0x29B1 ✓
  ```
  Da beide Funktionen im selben File stehen, dieselbe Parameter-Reihenfolge/-Bedeutung haben und `functions.h`
  bei `Crc16` explizit auf die Fortsetzungs-Fähigkeit hinweist, ist es naheliegend, dass ein Aufrufer dasselbe
  Muster auch bei `Crc32()` erwartet und dabei einen unbemerkt falschen (aber gültig aussehenden) Prüfsummenwert
  erhält – z.B. beim chunk-weisen Hashen eines Downloads/einer Datei zur Integritätsprüfung.
  Fix: entweder dokumentieren, dass `initial_crc` bei `Crc32()` nur ein alternativer Seed ist und NICHT zum
  Fortsetzen taugt (Diskrepanz zu `Crc16()` klar herausstellen), oder für echtes Chunking eine
  Update/Finalize-Aufspaltung anbieten (`Crc32Update(uint32_t& state, ...)` + `Crc32Finalize(uint32_t state)`),
  wobei `Crc32()` selbst weiterhin als Bequemlichkeits-Wrapper für den Einzelaufruf bestehen bleibt.

## Design

- [ ] **`Crc32()` castet `const void*` unnötig auf ein nicht-konstantes `unsigned char*`** (Zeile 70)
  ```cpp
  unsigned char* b = (unsigned char*)buffer;
  ```
  Direkt darunter macht `Crc16()` es richtig (Zeile 98: `const unsigned char* b = (const unsigned char*)buffer;`).
  Da `b` in `Crc32()` nur lesend verwendet wird (`*b++`), ist der Cast auf einen nicht-konstanten Pointer unnötig
  und inkonsistent mit der Schwesterfunktion im selben File.
  Fix: `const unsigned char* b = (const unsigned char*)buffer;`

- [ ] **Keine Eingabevalidierung, inkonsistent zu `Md5`/`Sha256` im selben `math/`-Modul**
  `Crc32(nullptr, 10)` / `Crc16(nullptr, 10)` dereferenzieren den Null-Pointer direkt (Absturz), während
  `Md5(nullptr, 0)`/`Sha256(nullptr, 0)` in dieser Fällen dokumentiert per `EmptyDataException` abbrechen. Für
  `size == 0` ist `Crc32`/`Crc16` unkritisch (die Schleife läuft dann nicht), aber `buffer == nullptr` mit
  `size > 0` bleibt ungeprüft. Passend zum REFACTORING.md-Ziel "weniger Exceptions" ist ein Crash hier vermutlich
  nicht gewollt, ein einheitliches Verhalten über die math-Funktionen hinweg (entweder alle prüfen, oder keine)
  wäre aber wünschenswert.

## Doku / Kosmetik

- Keine nennenswerten Funde – `Crc32`/`Crc16` sind in `functions.h` im Vergleich zu den übrigen math-Funktionen
  am besten dokumentiert (inkl. Parameter- und Rückgabewert-Beschreibung).

## Verifiziert OK (kein Handlungsbedarf)

- CRC-32-Tabelle und -Algorithmus liefern exakt den Standard-Check-Value `0xCBF43926` für `"123456789"`.
- CRC-16-Tabelle und -Algorithmus liefern exakt `0x29B1` für `"123456789"`, passend zur CRC-16/CCITT-FALSE-Variante
  (Init 0xFFFF, kein Reflect, kein XorOut) – Chunk-Verkettung über `initial_crc` funktioniert hier korrekt.
- `while (len--)` mit `size == 0` terminiert korrekt sofort (kein Underflow-Zugriff), da die Schleifenbedingung
  den Wert vor dem Dekrement prüft.
