# ID3Tag Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/audio/id3tag.h` + `src/audio/ID3Tag.cpp` (1736 Zeilen).
Verifiziert: `ByteArrayPtr::map()` wirft bei Out-of-Bounds `OutOfBoundsException` (kein NULL-Return, siehe `ByteArrayPtr.cpp:99-105`) – die diversen `if (!adr) break;`-Checks im Code sind daher toter Code, ändern die Sicherheit aber nicht (die Exception propagiert trotzdem sauber statt eines OOB-Zugriffs). `ByteArray::copy()`/`malloc()`/`calloc()` garantieren 4 Null-Bytes Padding direkt hinter dem Nutzdatenbereich (siehe `done/bytearray-review.md`) – das federt einige der hier gefundenen Off-by-few-Zugriffe ab, aber nicht alle. `File::map()` (verwendet über `Mp3.cpp`, siehe `mp3-review.md`) wirft analog `OverflowException`.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`ID3Tag::decode()`: Unterlauf `frame->size() - offset` wenn `offset > frame->size()` → riesige Länge an Transcode/iconv** (`ID3Tag.cpp:278-312`, betroffene Aufrufer u.a. `getComment()` ~1298-1299, `getPicture()` ~1375, `getEnergyLevel()` ~1438)
  ```cpp
  int ID3Tag::decode(const ID3Frame* frame, int offset, int encoding, String& target, const pplib::String& charset)
  {
      size_t size = 0;
      const char* data = frame->dataPtr() + offset;
      if (encoding == 0) {
          size = strlen(data);
          if (size + offset > frame->size()) size = frame->size() - offset;   // <-- unterläuft, wenn offset > frame->size()
          target.set(Transcode(data, size, "ISO-8859-1", charset));
          return offset + size + 1;
      }
      ...
  ```
  `frame->size()` und `offset` sind hier faktisch beide `size_t`/`int`-gemischt; sobald `offset` allein schon größer ist als `frame->size()`, unterläuft die Subtraktion zu einer Zahl nahe `SIZE_MAX`. Diese Größe geht ungeprüft in `Transcode()` → `ByteArrayPtr source(str, size)` → `iconv()`, das dann versucht, ~10^19 Bytes ab `data` zu lesen – ein garantierter Absturz (wilder Heap-Read weit über die Allokation hinaus), im besten Fall SIGSEGV, im schlechtesten ein Information-Leak in den zurückgegebenen `String`, bevor der Prozess abstürzt.

  **Minimalstes Reproduktionsbeispiel** über `getComment()`: Ein ID3v2.4-Tag mit genau einem `COMM`-Frame der deklarierten Größe 1 (nur das Encoding-Byte, kein Language-Code, keine Description):
  ```
  "ID3" 0x04 0x00 0x00  0x00 0x00 0x00 0x0B      // Header, Size=synchsafe(11)
  "COMM" 0x00 0x00 0x00 0x01  0x00 0x00  0x00     // Frame-Header (Size=1) + 1 Byte Daten (Encoding=0)
  ```
  `getComment()` liest `encoding=Peek8(frame.dataPtr())=0`, ruft dann `decode(&frame, 4, 0, desc)` auf. Da `frame->size()==1 < offset(4)`, unterläuft `frame->size()-offset` zu `SIZE_MAX-2`, welches an `Transcode()` durchgereicht wird → Absturz.
  Dieselbe Grundursache trifft `getPicture(int,ByteArray&)`: Der erste `decode()`-Aufruf für den MimeType kann (wenn der MimeType-String bis exakt zum Frame-Ende ohne Null-Terminator läuft) einen `offset` zurückgeben, der bereits `frame->size()+1` beträgt; der darauffolgende zweite `decode()`-Aufruf für die Description trifft dann exakt denselben Unterlauf. Analog in `getEnergyLevel()` (Identifier- gefolgt von Value-Decode).
  Fix: Am Anfang von `decode()` klar behandeln, wenn `offset >= frame->size()`, statt in die Subtraktion zu laufen, z.B.:
  ```cpp
  if ((size_t)offset >= frame->size()) { target.clear(); return offset; }
  ```

- [ ] **`synchronize()`: `size_t`-Unterlauf bei `frameSize` 1 oder 2 unter gesetztem Unsynchronisation-Flag → Heap-Buffer-Overflow (Schreiben!)** (`ID3Tag.cpp:453-476`, aufgerufen aus `load()` bei `unsyncFlag` in Zeile 579)
  ```cpp
  static size_t synchronize(unsigned char* adr, size_t size)
  {
      size_t src = 0;
      size_t tgt = 0;
      for (src = 0; src < size - 3; src += 1) {   // <-- size-3 unterläuft für size ∈ {1,2}
          unsigned char byte = adr[src];
          adr[tgt] = byte;
          tgt++;
          ...
  ```
  `frameSize` wird in `load()` nur auf `!frameSize` (also exakt 0) geprüft, bevor `synchronize()` aufgerufen wird (Zeile 572-579) – 1 oder 2 sind gültige, durchlaufende Werte. Ist das Unsynchronisation-Flag im Tag-Header gesetzt (`Flags & 128`), wird für **jedes** Frame `synchronize()` aufgerufen; bei `size==1` oder `size==2` wird `size-3` als `size_t` zu einer riesigen Zahl (`SIZE_MAX` bzw. `SIZE_MAX-1`), die Schleife läuft weit über den Frame-Puffer (nur wenige Bytes groß, Teil des größeren `buffer`-ByteArrays aus `file.read()`) hinaus, liest und **schreibt** dabei fortlaufend (`adr[tgt]=byte`) – ein waschechter Heap-Buffer-Overflow mit Schreibzugriff, der erst mit einem Segfault endet (oder früher unbemerkt Heap-Metadaten korrumpiert).
  **Reproduktion:** ID3v2.4-Header mit Flags-Byte `0x80` (Unsynchronisation-Flag gesetzt) und einem Frame beliebiger 4-Byte-ID mit Size=synchsafe(1) und 1 Byte Nutzdaten.
  Fix: Guard am Anfang der Funktion:
  ```cpp
  static size_t synchronize(unsigned char* adr, size_t size)
  {
      if (size < 3) return size;   // oder passendes Kopieren ohne Unsync-Suche
      ...
  ```

- [ ] **AIFF/WAVE-Chunk-Walk: Integer-Overflow bei `chunkSize + 8` → Endlosschleife (DoS)** (`findId3Tag()` AIFF-Zweig `ID3Tag.cpp:421-430`, WAVE-Zweig `431-440`; identisches Muster in `trySaveAiffInExistingFile()` `1111-1143`, `copyAiffToNewFile()` `1145-1194`, `trySaveWaveInExistingFile()` `1632-1658`, `copyWaveToNewFile()` `1660-1700`)
  ```cpp
  uint64_t p = 12;
  while (p + 8 < File.size()) {
      const char* adr = File.map(p, 8);
      if (!adr) break;
      uint32_t size = PeekN32(adr + 4);           // <-- direkt aus der Datei, ungeprüft
      if (PeekN32(adr) == 0x49443320) return p + 8;
      p += size + 8;                               // <-- uint32_t-Addition, kann wrappen
      if (size % 2) p++;
  }
  ```
  `size` ist `uint32_t` und kommt direkt aus der Datei. Wählt der Angreifer z.B. `size = 0xFFFFFFF8`, ergibt `size + 8` (als `uint32_t` berechnet) exakt `0` – `p` bewegt sich in dieser Iteration überhaupt nicht mehr vorwärts, die `while`-Bedingung bleibt für immer wahr → Endlosschleife, die den Thread/Prozess dauerhaft blockiert (Denial of Service). Reproduzierbar mit einer 20-Byte-Datei: `"FORM" + 4-Byte-Größe + "AIFF"` (12 Byte Header) gefolgt von einem 8-Byte-Chunk-Header mit beliebiger 4-Byte-ID (≠ `"ID3 "`) und Size-Feld `0xFFFFFFF8`. Dasselbe Muster (und dieselbe uint32_t-Falle, in `trySaveAiffInExistingFile`/`copyAiffToNewFile`/WAVE-Pendants sogar mit `uint32_t qp` statt `uint64_t`) betrifft auch `ID3Tag::save()` beim Umschreiben bestehender AIFF/WAVE-Dateien.
  Fix: `chunkSize` gegen die verbleibende Dateigröße validieren, bevor damit weitergerechnet wird, und die Addition in einem overflow-sicheren (z.B. `uint64_t`) Typ durchführen: `uint64_t next = (uint64_t)p + chunkSize + 8; if (next <= p) throw CorruptedDataException();`.

## Bugs (mittel)

- [ ] **`getPrivateData()`: `size_t`-Unterlauf bei PRIV-Frame, dessen Inhalt exakt der Identifier-Länge entspricht** (`ID3Tag.cpp:1415-1426`)
  ```cpp
  ByteArrayPtr ID3Tag::getPrivateData(const String& identifier) const
  {
      for (const auto& frame : frames) {
          if (frame.ID == "PRIV") {
              if (identifier.strcmp(frame.dataPtr()) == 0) {
                  return ByteArrayPtr(frame.dataPtr() + identifier.size() + 1, frame.size() - identifier.size() - 1);
              }
          }
      }
      return ByteArrayPtr();
  }
  ```
  Enthält das PRIV-Frame exakt `identifier` ohne folgendes Null-Byte und ohne Nutzdaten (Frame endet direkt nach dem Identifier-String, `strcmp` matcht dank des garantierten Null-Padding von `ByteArray` trotzdem), dann ist `frame.size() == identifier.size()` und `frame.size() - identifier.size() - 1` unterläuft zu `SIZE_MAX`. Die konstruierte `ByteArrayPtr` zeigt danach zwar auf gültigen (Padding-)Speicher, hat aber eine absurde Größe. In `getPrivateData(ByteArray&, const String&)` (Zeile 1407-1413) wird das Ergebnis ungeprüft an `bin.copy(ref)` weitergereicht; `ByteArray::copy()` prüft die Größe vorab gegen `MAX_BYTEARRAY_SIZE` (siehe `ByteArray.cpp:114`) und wirft dann `OutOfMemoryException` – **kein Speicherfehler**, aber eine für eine reine Getter-Funktion unerwartete, undokumentierte Exception, ausgelöst durch eine plausible, harmlos aussehende Datei.
  Fix: vor der Subtraktion prüfen: `if (identifier.size() + 1 > frame.size()) return ByteArrayPtr();`.

- [ ] **ID3v2.3 Extended-Header-Größe wird fälschlich als synchsafe (7-Bit) statt als normale 32-Bit-Zahl dekodiert** (`ID3Tag.cpp:538-549`)
  ```cpp
  if (extendedHeader) {
      adr = buffer.map(p, 4);
      exHdrSize = Peek8(adr + 3);
      exHdrSize |= (Peek8(adr + 2)) << 7;
      exHdrSize |= (Peek8(adr + 1)) << 14;
      exHdrSize |= (Peek8(adr + 0)) << 21;
      p += exHdrSize;
  }
  ```
  Nach ID3v2.3-Spezifikation ist die Extended-Header-Größe eine normale big-endian 32-Bit-Zahl (kein Synchsafe-Integer) – nur in ID3v2.4 ist sie synchsafe. Der Code wendet das 7-Bit-Schema unabhängig von `version` an. Bei einem (seltenen, aber gültigen) ID3v2.3-Tag mit Extended Header wird `exHdrSize` dadurch falsch berechnet, was den Frame-Scan ab einem falschen Offset beginnen lässt (Frames werden verpasst oder Datenmüll als Frame-Header interpretiert – letzteres bleibt durch die `buffer.map()`-Bounds-Checks zwar speichersicher, liefert aber falsche/leere Tag-Daten).
  Fix: Verzweigung nach `version`, analog zur bereits korrekt versions-abhängigen Frame-Size-Dekodierung weiter unten (Zeile 567-571).

- [ ] **ID3v2.3-Frame-Size: Signed-Shift-UB + Sign-Extension bei gesetztem High-Bit** (`ID3Tag.cpp:569-570`)
  ```cpp
  } else { // version == 3
      frameSize = Peek8(adr + 7) | (Peek8(adr + 6) << 8) | (Peek8(adr + 5) << 16) | (Peek8(adr + 4) << 24);
  }
  ```
  `Peek8()` liefert `uint8_t`, das für die Shift-Operation zu `int` promotet wird. Ist das MSB des Größenfeldes (`adr[4]`) `>= 0x80` (nach Spec ungültig, aber von einer manipulierten Datei problemlos setzbar), erzeugt `Peek8(adr+4) << 24` einen Wert, der als `int` nicht mehr darstellbar ist (undefiniertes Verhalten vor C++20 bei signed-Overflow durch Shift). In der Praxis (2er-Komplement) entsteht ein negativer `int`, der bei der Zuweisung an `size_t frameSize` vorzeichenerweitert wird – aus einer eigentlich nur 32-Bit-großen (wenn auch ungültigen) Zahl wird ein ~64-Bit-Wert nahe `SIZE_MAX`. Der darauffolgende `buffer.map(p + 10, frameSize)` (Zeile 575) fängt das zwar zuverlässig per `OutOfBoundsException` ab (kein Speicherfehler), aber die Berechnung selbst basiert auf undefiniertem Verhalten und ist damit compilerabhängig fragil.
  Fix: Byteweise ohne Shift-UB zusammensetzen, z.B. über `uint32_t` statt `int`-Zwischenwerte: `frameSize = ((uint32_t)Peek8(adr+4)<<24) | ...;` (explizit unsigned rechnen, keine Vorzeichenerweiterung).

- [ ] **`ID3Tag::load()`: `int footerSize` wird deklariert, aber nie gesetzt – ID3v2.4-Footer wird nicht übersprungen** (`ID3Tag.cpp:537, 556`)
  Das (auskommentierte) `footerFlag`-Handling weiter oben (Zeile 517-522) wurde nie fertiggestellt; `footerSize` bleibt immer `0`. Ist im Tag ein 10-Byte-Footer vorhanden (v2.4, Flag `0x10`), wird dessen Signatur `"3DI"` + Spiegel-Header als vermeintliches letztes Frame interpretiert. Durch die Bounds-Checks in `buffer.map()` bleibt das speichersicher, liefert aber im Zweifel ein unsinniges Phantom-Frame oder bricht mit einer Exception ab, statt den Footer sauber zu ignorieren.
  Fix: Footer-Flag auswerten und `footerSize = 10;` setzen, wenn vorhanden – oder die tote Variable und den zugehörigen Kommentarblock entfernen, falls Footer-Unterstützung bewusst zurückgestellt wird.

## Design

- [ ] **Pro-Frame-Unsynchronisation (ID3v2.4) wird nicht ausgewertet** (`ID3Tag.cpp:578-583`)
  In ID3v2.4 kann jedes Frame unabhängig vom globalen Tag-Flag ein eigenes Unsynchronisation-Flag in seinem 2-Byte-Frame-Flags-Feld tragen. Der Code wendet `synchronize()` nur basierend auf dem globalen `unsyncFlag` aus dem Tag-Header an, nie pro Frame. Für v2.4-Tags, die das Frame-Flag statt des Tag-Flags nutzen, werden Frames dann nicht de-synchronisiert (führt zu Datenmüll im Frame-Inhalt, keine Speicherverletzung).

- [ ] **`if (!adr) break;` ist toter Code, da `map()` nie `NULL` zurückgibt, sondern wirft** (`ID3Tag.cpp:404, 425, 435, 558, 576, 1116, 1153, 1636, 1665` u.a.)
  Sowohl `ByteArrayPtr::map()` als auch `FileObject`/`File::map()` (siehe `mp3-review.md`) werfen bei Out-of-Bounds eine Exception statt `NULL` zu liefern. Diese Defensivchecks suggerieren eine Fehlerbehandlung, die faktisch nie greift – die tatsächliche Fehlerbehandlung ist die (in `load()`/`tryLoad()` teils gar nicht, teils per catch-all abgefangene) Exception. Kein Sicherheitsproblem, aber irreführender Code, der bei zukünftigem API-Wechsel (z.B. falls `map()` doch mal `NULL` zurückgeben würde) fälschlich als "schon abgesichert" gelesen wird.
  Fix: entweder entfernen (Doku-Kommentar, dass `map()` wirft) oder – falls gewünscht – tatsächlich `try/catch` um die einzelnen `map()`-Aufrufe legen, um pro Frame/Chunk sauber abzubrechen statt die ganze `load()` mit einer Exception zu beenden.

- [ ] **`setPaddingSize`/`setPaddingSpace`/`setMaxPaddingSpace` nehmen `int`, Member sind `uint32_t`** (`id3tag.h:281-283`, `ID3Tag.cpp:366-379`, Member in `id3tag.h:178`)
  Ein negativer Aufruf (z.B. versehentlich `setMaxPaddingSpace(-1)`) wird stillschweigend zu einer riesigen `uint32_t`-Zahl. Passt zur generellen Empfehlung aus REFACTORING.md, Interfaces zu modernisieren – hier würde `size_t`/`uint32_t` direkt in der Signatur Klarheit schaffen.

## Doku / Kosmetik

- [ ] `NoID3TagFoundException` ist in `id3tag.h:46` deklariert, wird aber nirgends in `ID3Tag.cpp` geworfen (stattdessen wird bei fehlendem Tag in `load()` einfach still zurückgekehrt). Entweder tatsächlich verwenden (z.B. optional aus `load()` werfen, `tryLoad()` fängt es weiterhin ab) oder als unbenutzte Exception entfernen (passt zum bereits laufenden Cleanup, siehe Commit "Nicht verwendete Exceptions gelöscht").

- [ ] `Mp3.cpp`/`ID3Tag.cpp`: mehrere `int`/`size_t`-Mischtypen bei Offsets (`decode()`s `int offset` vs. `size_t frame->size()`) – siehe Bug oben. Für neuen Code wäre eine konsistente `size_t`-API für Byte-Offsets robuster.

## Verifiziert OK (kein Handlungsbedarf)

- `GetID3GenreName()`: Die Berechnung `sizeof(genres)/sizeof(char*) - 2` ergibt korrekt `147` (148 echte Einträge, Index 0-147, plus `NULL`-Sentinel) – kein Off-by-one.
- `strlen16()` (UTF-16-Terminator-Suche) ist trotz möglicher ungerader Start-Offsets sicher: Das garantierte 4-Byte-Null-Padding von `ByteArray` stellt für jede Ausrichtung mindestens ein vollständiges 16-Bit-Null-Wort innerhalb der Allokation sicher, bevor ein Zugriff über das Ende hinaus nötig wäre.
- `ID3Tag::setComment()` prüft vor dem `memcmp()` auf die Description korrekt `f.size() > 4 + descEnc.size()` – im Gegensatz zu `getComment()` (siehe Bug oben) hier sauber gegen Unterlauf abgesichert.
- Alle `Peek8()`/`Peek32()`/`PeekN32()`-Aufrufe auf frisch aus `load()` geladene Frames sind unkritisch, da `frameSize==0` das Parsen bereits vorher abbricht (`if (!frameSize) break;`, Zeile 572-574) – jedes über `load()` erzeugte Frame hat also mindestens 1 Byte Nutzdaten.
- Frame-Size-Dekodierung wählt korrekt versions-abhängig zwischen synchsafe (v2.4) und normaler 32-Bit-Dekodierung (v2.3), siehe `ID3Tag.cpp:567-571` – nur die Extended-Header-Größe (siehe Bug oben) hat diese Versionsunterscheidung nicht. Die v2.3-Variante selbst hat aber ein Signed-Shift-Problem, siehe Bugs (mittel).
