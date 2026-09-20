# ID3Tag Review 2 – Befunde & Analyse

Review vom 2026-09-20, Scope: `include/pplib/audio/id3tag.h` + `src/audio/ID3Tag.cpp`.
Vorheriges Review: `codereview/done/id3tag-review.md` (von Claude Sonnet 5).
Ziel dieses Folgereviews:
1. Überprüfung des Codes auf verbliebene Bugs, Schwachstellen und Verbesserungspotenziale.
2. Verifikation der Sonnet-Bugfixes auf Vollständigkeit und eventuelle Regressionen.
3. Code-Snippets zur Behebung aller identifizierten Probleme.
4. Gesonderte Meldung von Fehlern in angrenzenden Dateien (`FileObject.cpp`, `Mp3.cpp`).

---

## Zusammenfassung der vorherigen Bugfixes (Sonnet-Review)

| Befund aus Review 1 | Status | Bewertung im Zweit-Review |
|---|---|---|
| `decode()`: Unterlauf `frame->size() - offset` | Gefixt | **Teilweise unvollständig**: `decode()` ist abgesichert, aber `copyAndDecodeText()` ruft davor ungeschützt `Peek8()` auf OOB-Offset auf. Encodings 4–31 unbehandelt. |
| `synchronize()`: Heap-Buffer-Overflow bei `size < 3` | Gefixt | **Logik noch fehlerhaft**: Der Buffer-Overflow ist behoben, aber die De-Unsynchronisations-Bedingung weicht von ID3v2.3/v2.4 Spec ab und belässt Escaping-Nullbytes in den Daten. |
| AIFF/WAVE: Integer-Overflow im Chunk-Walk | Gefixt | **Verifiziert solide**: Durchgängige `uint64_t`-Berechnungen mit Bounds-Check verhindern DoS zuverlässig. |
| `getPrivateData()`: Unterlauf bei kurzem Frame | Gefixt | **Verifiziert solide**: Sauber gegen Frame-Größe und Nullterminator geprüft. |
| ID3v2.3 Extended-Header-Größe normal statt synchsafe | Gefixt | **Regression bei v2.4**: Bei v2.4 wurde `p += exHdrSize` statt `p += 4 + exHdrSize` eingebaut ("excludes itself", Spec 3.2), wodurch bei v2.4 4 Bytes fehlen. |
| ID3v2.3 Frame-Size: Signed-Shift-UB | Gefixt | **Verifiziert solide**: `PeekN32` wird genutzt. |
| ID3v2.4 Footer-Flag nicht ausgewertet | Gefixt | **Verifiziert OK**: Flag 0x10 wird berücksichtigt, aber Unsigned-Unterlauf bei `buffer.size() < footerSize` möglich. |

---

## Bugs (kritisch)

- [x] **1. `strlen16()`: Undefined Behavior durch Unaligned Pointer Cast & Strict-Aliasing-Verletzung + unbegrenzter Speicherzugriff** (`ID3Tag.cpp:208-215`, aufgerufen in `decode()` Zeilen 288, 294)
  ```cpp
  static size_t strlen16(const char* data)
  {
      size_t l = 0;
      uint16_t* d = (uint16_t*)data;
      while (d[l] != 0)
          l++;
      return l;
  }
  ```
  **Problem:**
  `data` zeigt auf `frame->dataPtr() + offset`. Da `offset` nach einem 1-Byte Encoding-Feld oder nach variablen Texten ungerade sein kann, zeigt `data` oft auf eine **ungerade Adresse**.
  - Der Cast auf `uint16_t*` und der Lesezugriff `d[l]` auf ungerade Adressen ist formales Undefined Behavior in C++ (Strict-Aliasing- und Alignment-Verletzung). Auf Architekturen mit striktem Alignment (ARM, MIPS, SPARC) führt dies zu einem Hardware-Alignment-Fault (`SIGBUS`), unter x86/x64 zu Performance-Einbußen und potentiellem UB durch Compiler-Vektorisierung.
  - Darüber hinaus hat `strlen16` keinen Längenparameter `max_bytes`. Zwar garantiert `ByteArray` 4 Nullbytes Padding, aber bei Frames ohne Nullterminator läuft die Schleife unnötigerweise bis ins Padding.
  
  **Korrekturvorschlag:**
  Ersatz durch eine sichere, byteweise lesende Funktion mit Maximallänge:
  ```cpp
  static size_t strnlen16(const char* data, size_t max_bytes)
  {
      size_t max_words = max_bytes / 2;
      for (size_t i = 0; i < max_words; ++i) {
          if (data[i * 2] == 0 && data[i * 2 + 1] == 0) {
              return i;
          }
      }
      return max_words;
  }
  ```

  ==> FIXED: Durch sichere `strnlen8` und `strnlen16` Funktionen ersetzt.

- [x] **2. `copyAndDecodeText()`: Heap-Read Out-of-Bounds bei `offset >= frame->size()`** (`ID3Tag.cpp:244-268`, beide Überladungen)
  ```cpp
  void ID3Tag::copyAndDecodeText(String& s, const ID3Frame* frame, int offset) const
  {
      if (!frame || frame->isEmpty()) {
          s.clear();
          return;
      }
      int encoding = Peek8(frame->dataPtr() + offset);   // <-- OOB-Zugriff, wenn offset >= frame->size()
      if (encoding < 32) {
          ID3Tag::decode(frame, offset + 1, encoding, s, localCharset);
      ...
  ```
  **Problem:**
  Sonnets Review hat `ID3Tag::decode()` gegen `offset >= frame->size()` abgesichert, aber `copyAndDecodeText()` ruft `Peek8(frame->dataPtr() + offset)` **vor** dem Aufruf von `decode()` auf, ohne `offset` gegen `frame->size()` zu prüfen!
  Wird `copyAndDecodeText()` mit einem `offset >= frame->size()` aufgerufen (z.B. bei manipulierten Frames oder Teilfeldern), liest `Peek8` jenseits der Frame-Grenzen im Heap.
  
  **Korrekturvorschlag:**
  Zu Beginn beider Überladungen:
  ```cpp
  if (!frame || offset < 0 || (size_t)offset >= frame->size()) {
      s.clear();
      return;
  }
  ```

  ==> FIXED: `offset < 0 || (size_t)offset >= frame->size()` Guard eingebaut und Const-Überladung delegiert.

- [x] **3. `generateId3V2Tag()`: Heap-Buffer-Overread bei Frame-Name < 4 Zeichen & UB bei leeren Daten** (`ID3Tag.cpp:958-969`)
  ```cpp
  char* frame = buffer + 10;
  for (auto& Frame : frames) {
      memcpy(frame, Frame.name().c_str(), 4);   // <-- Overread, wenn Frame.name().size() < 4
      uint32_t fs = Frame.size();
      ...
      memcpy(frame + 10, Frame.dataPtr(), Frame.size()); // <-- UB wenn Frame.size() == 0 (nullptr)
      frame += 10 + Frame.size();
  }
  ```
  **Problem:**
  1. Wird ein Frame mit Standard-Konstruktor `ID3Frame()` erzeugt (Name leer) oder ein Frame mit weniger als 4 Zeichen Name hinzugefügt, hat `Frame.name().c_str()` weniger als 4 Bytes. `memcpy(..., 4)` liest über das Ende des String-Puffers hinaus.
  2. Wenn `Frame.size() == 0`, gibt `Frame.dataPtr()` `nullptr` zurück. `memcpy(dest, nullptr, 0)` ist formales Undefined Behavior in C++.
  
  **Korrekturvorschlag:**
  ```cpp
  for (auto& Frame : frames) {
      if (Frame.name().size() != 4) continue; // Ungültige Frame-IDs überspringen
      memcpy(frame, Frame.name().c_str(), 4);
      uint32_t fs = Frame.size();
      Poke8(frame + 7, (fs & 127));
      Poke8(frame + 6, ((fs >> 7) & 127));
      Poke8(frame + 5, ((fs >> 14) & 127));
      Poke8(frame + 4, ((fs >> 21) & 127));
      PokeN16(frame + 8, Frame.Flags);
      if (Frame.size() > 0 && Frame.dataPtr() != nullptr) {
          memcpy(frame + 10, Frame.dataPtr(), Frame.size());
      }
      frame += 10 + Frame.size();
  }
  ```

  ==> FIXED: Nur Frames mit `name().size() == 4` werden serialisiert und `memcpy` wird bei `Frame.size() == 0` umgangen.

- [x] **4. `saveMP3()`: Fehlende Exception-Safety, unnötige `.rename.tmp`-Datei & Datenverlust-Risiko** (`ID3Tag.cpp:943-1025`)
  ```cpp
  void ID3Tag::saveMP3()
  {
      String tmpfile = Filename;
      tmpfile += ".rename.tmp";
      File n;
      File o;
      n.open(tmpfile, File::FileMode::WRITE);   // <-- Wird IMMER geöffnet, selbst bei useoldfile!
      o.open(Filename, File::FileMode::READWRITE);
      ...
      if (useoldfile) {
          ...
          n.close();
          o.close();
          File::remove(tmpfile);                // <-- Sofort wieder gelöscht
      } else {
          n.copyFrom(o, ...);
          ...
          n.close();
          o.close();
          File::remove(Filename);               // <-- Nicht-atomar vor rename!
          File::rename(tmpfile, Filename);
      }
  }
  ```
  **Problem:**
  1. Anders als in `saveAiff()` und `saveWave()` gibt es in `saveMP3()` **kein `try / catch`**, das bei Schreibfehlern `tmpfile` aufräumt. Scheitert z.B. `copyFrom()` wegen Festplattenvollstand, bleibt die halbfertige `.rename.tmp`-Datei dauerhaft liegen.
  2. `n.open(tmpfile)` wird bedingungslos ganz am Anfang aufgerufen. Wenn der Tag in den vorhandenen Padding-Bereich passt (`useoldfile == true`), wurde die Datei umsonst erstellt und direkt wieder gelöscht. Befindet sich die MP3-Datei in einem Verzeichnis ohne Berechtigung zur Dateineuerstellung (aber mit Schreibrecht auf die Datei selbst), schlägt `saveMP3()` fehl!
  3. `File::remove(Filename)` vor `File::rename(tmpfile, Filename)` zerstört die Atomarität. Da `File::rename()` unter Windows `MoveFileExW(..., MOVEFILE_REPLACE_EXISTING)` und unter POSIX `rename()` nutzt, ersetzt `rename` das Ziel bereits atomar. Ein vorheriges Löschen führt bei einem anschließenden Rename-Fehler (z.B. Lock durch Antivirus) zu komplettem Dateiverlust!
  
  **Korrekturvorschlag:**
  `tmpfile` erst dann öffnen, wenn `!useoldfile` feststeht, und den Vorgang in einen `try/catch`-Block kapseln:
  ```cpp
  if (useoldfile) {
      o.seek(0);
      o.write(tagV2);
      // rest auffüllen...
      if (tagV1.size() > 0) {
          o.write(tagV1.ptr(), tagV1.size(), mpg.end);
      } else if (mpg.end < o.size()) {
          o.truncate(mpg.end);
      }
      o.close();
  } else {
      n.open(tmpfile, File::FileMode::WRITE);
      try {
          n.write(tagV2);
          // padding & Musikdaten kopieren...
          n.copyFrom(o, (uint64_t)mpg.start, (uint64_t)mpg.size, (uint64_t)pn);
          if (tagV1.size() > 0) n.write(tagV1);
          n.close();
          o.close();
          File::rename(tmpfile, Filename);
      } catch (...) {
          n.close();
          o.close();
          File::remove(tmpfile);
          throw;
      }
  }
  ```

  ==> FIXED: Temporäre Datei wird nur bei Bedarf erstellt, per `try/catch` aufgeräumt und atomar umbenannt.

- [x] **5. AIFF & WAVE `copy*ToNewFile`: Datenverlust (Verwerfen des letzten Chunks) bei ungerader Chunk-Größe ohne Dateiende-Padding** (`ID3Tag.cpp:1152-1157`, `1647-1652`)
  ```cpp
  uint32_t chunkSize = PeekN32(adr + 4); // bzw. Peek32
  uint64_t physicalSize = (uint64_t)chunkSize + 8 + (chunkSize % 2);
  if (qp + physicalSize <= qp || qp + physicalSize > o.size()) break;
  ```
  **Problem:**
  Wenn in einer AIFF- oder WAVE-Datei der letzte Chunk (z.B. der `data`- bzw. `SSND`-Audiodaten-Chunk) eine ungerade Bytezahl hat und die Datei am Dateiende kein Null-Padbyte besitzt (also `o.size() == qp + 8 + chunkSize`), dann ist `qp + physicalSize == o.size() + 1 > o.size()`.
  Die Schleife bricht mit `break;` ab, und der Chunk wird **überhaupt nicht in die neue Datei kopiert**! Die Zieldatei verliert dadurch sämtliche Audiodaten.
  
  **Korrekturvorschlag:**
  Prüfen, ob die Nutzdaten des Chunks in der Datei liegen (`qp + 8 + chunkSize <= o.size()`), und nur so viele Bytes kopieren, wie tatsächlich vorhanden sind:
  ```cpp
  if (qp + 8 + chunkSize > o.size() || qp + 8 + chunkSize < qp) break;
  uint64_t copyBytes = std::min<uint64_t>(physicalSize, o.size() - qp);
  if (chunkType != 0x49443320) {
      n.copyFrom(o, qp, copyBytes, tp);
      tp += physicalSize; // Im Ziel immer sauberes Padding einhalten
      formsize += physicalSize;
  }
  qp += physicalSize;
  ```

  ==> FIXED: Vorhandensein des Nutzdaten-Chunks geprüft und Padding bei Bedarf im Zieldateipfad geschrieben.

- [x] **6. ID3v2.4 Extended Header Offset: 4 Bytes zu weit vorne (Sonnet-Fix unvollständig)** (`ID3Tag.cpp:520-532`)
  ```cpp
  if (version == 4) {
      exHdrSize = Peek8(adr + 3);
      exHdrSize |= (Peek8(adr + 2)) << 7;
      exHdrSize |= (Peek8(adr + 1)) << 14;
      exHdrSize |= (Peek8(adr + 0)) << 21;
      p += exHdrSize;                         // <-- FALSCH: schließt sich selbst aus!
  } else {
      exHdrSize = PeekN32(adr);
      p += 4 + exHdrSize;                     // <-- Richtig für v2.3
  }
  ```
  **Problem:**
  Nach ID3v2.4 Spezifikation (Abschnitt 3.2): *"Where the 'Extended header size', currently 6 or 10 bytes, excludes itself."*
  Das bedeutet: Auch in ID3v2.4 ist das 4-Byte Größenfeld selbst in `exHdrSize` **nicht** enthalten. Der gesamte Extended Header ist daher `4 + exHdrSize` Bytes groß.
  Durch `p += exHdrSize` landet `p` 4 Bytes vor dem tatsächlichen Frame-Beginn; der Frame-Parser liest die letzten 4 Bytes des Extended Headers als angebliche Frame-ID.
  
  **Korrekturvorschlag:**
  ```cpp
  p += 4 + exHdrSize;
  ```

  ==> FIXED: Bei v2.4 auf `p += 4 + exHdrSize;` korrigiert.

- [x] **7. `load()`: `size_t`-Unterlauf bei `buffer.size() < footerSize`** (`ID3Tag.cpp:514`, `538`)
  ```cpp
  while (p + 10 <= buffer.size() - footerSize) {
  ```
  **Problem:**
  `buffer.size()` ist `size_t` (unsigned), `footerSize` ist `int` (10). Wenn `buffer.size() < footerSize` (z.B. abgeschnittene Datei oder Lesefehler), unterläuft `buffer.size() - footerSize` zu einer riesigen Zahl (`SIZE_MAX - x`), und die Schleifenbedingung wird fälschlicherweise wahr.
  
  **Korrekturvorschlag:**
  ```cpp
  size_t effectiveBufferSize = (buffer.size() >= (size_t)footerSize) ? (buffer.size() - footerSize) : 0;
  while (p + 10 <= effectiveBufferSize) {
  ```

  ==> FIXED: `effectiveBufferSize` schützt vor Unterlauf.

---

## Bugs (mittel)

- [x] **8. `synchronize()` (De-Unsynchronisation): Fehlerhafte Logik belässt Escaping-Bytes im Puffer** (`ID3Tag.cpp:453-475`)
  ```cpp
  if (adr[src] == 255 && src + 1 < size) {
      if (adr[src + 1] == 0) {
          if (src + 2 >= size) {
              src += 2;
          } else if ((adr[src + 2] & (32 + 64 + 128)) == (32 + 64 + 128) || adr[src + 2] == 0) {
              src += 2;
          } else {
              src++;   // <-- Belässt das 0x00 im Datenstrom!
          }
      }
  ...
  ```
  **Problem:**
  Laut ID3v2.3 (Section 5) und ID3v2.4 (Section 5) Spezifikation:
  *"Reversing the process of unsynchronisation is done ... by simply removing any 00 byte that is preceded by a FF byte."*
  Die Bedingung `else if ((adr[src + 2] & 0xE0) == 0xE0 || adr[src + 2] == 0)` ist spec-widrig. Wenn eine Datei legitim unsynchronisiert wurde und z.B. Bilddaten oder UTF-16-Texte mit Sequenzen wie `FF 00 41` enthält, wird das `00` **nicht** entfernt. Die Nutzdaten sind nach dem Laden beschädigt.
  
  **Korrekturvorschlag:**
  ```cpp
  static size_t synchronize(unsigned char* adr, size_t size)
  {
      if (size < 2) return size;
      size_t src = 0;
      size_t tgt = 0;
      while (src < size) {
          adr[tgt++] = adr[src];
          if (adr[src] == 255 && src + 1 < size && adr[src + 1] == 0) {
              src += 2; // 0x00 überspringen
          } else {
              src++;
          }
      }
      return tgt;
  }
  ```

  ==> FIXED: Exakt gemäß Spec implementiert.

- [x] **9. `decode()`: Ungültige Text-Encodings (4..31) lassen `target` unverändert** (`ID3Tag.cpp:278-316`)
  **Problem:**
  Wenn `encoding` im Bereich 4..31 liegt, greift kein `if`-Zweig. `size` bleibt 0, `target` wird nicht geleert und behält den vorherigen String-Inhalt. Der Aufrufer erhält alte Daten.
  
  **Korrekturvorschlag:**
  Zu Beginn oder als Fallback:
  ```cpp
  target.clear();
  ```
  und unbekannte Encodings (4..31) sauber mit leeren Daten und `return offset + 1;` beenden.

  ==> FIXED: `target.clear()` zu Beginn und sauberes `return offset + 1;` bei Encodings 4..31.

- [x] **10. `decode()`: `encoding == 3` (UTF-8) ignoriert Parameter `charset`** (`ID3Tag.cpp:305-309`)
  ```cpp
  } else if (encoding == 3) {
      size = strlen(data);
      if (size + offset > frame->size()) size = frame->size() - offset;
      target.set(data, size);                  // <-- Ignoriert charset komplett!
      return offset + size + 1;
  }
  ```
  **Problem:**
  Für Encodings 0, 1, 2 und >31 transkodiert `decode()` das Ergebnis nach `charset`. Bei `encoding == 3` wird der Text ungeprüft als rohes UTF-8 in `target` gesetzt. Wenn `charset` z.B. `ISO-8859-1` ist, erhält der Aufrufer UTF-8 statt des gewünschten Zeichensatzes.
  
  **Korrekturvorschlag:**
  ```cpp
  if (charset.isEmpty() || charset.strCaseCmp("UTF-8") == 0) {
      target.set(data, size);
  } else {
      target.set(Transcode(data, size, "UTF-8", charset));
  }
  ```

  ==> FIXED: Transcoding von UTF-8 nach `charset` implementiert.

- [x] **11. `decode()`: Ungerade Bytezahl bei UTF-16 führt zu Exception in `Transcode`** (`ID3Tag.cpp:288-303`)
  **Problem:**
  Wenn bei fehlerhaften Tags `frame->size() - offset` ungerade ist, wird ein unvollständiges UTF-16 Code-Unit-Paar an `Transcode()` übergeben. `iconv` bricht mit `EINVAL` ab und wirft `CharacterEncodingException`.
  
  **Korrekturvorschlag:**
  ```cpp
  if (size % 2 != 0) size--;
  ```

  ==> FIXED: `if (size % 2 != 0) size--;` eingebaut.

- [x] **12. `getPicture()` meldet Erfolg (`true`) bei Frame ohne Bilddaten** (`ID3Tag.cpp:1376-1380`)
  ```cpp
  offset = decode(&frame, offset + 1, encoding, Description);
  if (offset <= (int)frame.size()) {
      bin.copy(frame.dataPtr() + offset, frame.size() - offset);
      return true; // <-- Liefert true bei frame.size() - offset == 0
  }
  ```
  **Problem:**
  Endet das APIC-Frame exakt mit dem Nullterminator der Description, ist `offset == frame.size()`. `bin.copy` kopiert 0 Bytes, und `getPicture()` gibt `true` zurück, obwohl gar kein Bild vorhanden ist.
  
  **Korrekturvorschlag:**
  ```cpp
  if (offset < (int)frame.size()) {
      bin.copy(frame.dataPtr() + offset, frame.size() - offset);
      return true;
  }
  ```

  ==> FIXED: Strikte Prüfung `offset < (int)frame.size()` in `getPicture` und `hasPicture`.

- [x] **13. Frame-Unsynchronisationsflag (ID3v2.4) wird beim erneuten Speichern nicht zurückgesetzt** (`ID3Tag.cpp:577-584`, `964-968`)
  **Problem:**
  Wird ein ID3v2.4 Frame mit Frame-Unsynchronisation (`flags & 2`) geladen, wird es per `synchronize()` de-unsynchronisiert. Im `Flags`-Member bleibt das Bit `0x0002` jedoch gesetzt. `generateId3V2Tag()` schreibt `Frame.Flags` unverändert zurück in die Datei. Die neue Datei deklariert das Frame somit fälschlich als unsynchronisiert, obwohl die Daten roh/synchronisiert vorliegen.
  
  **Korrekturvorschlag:**
  In `load()` nach erfolgreichem `synchronize()`:
  ```cpp
  Frame.setFlags(Frame.flags() & ~2);
  ```

  ==> FIXED: Bit 0x0002 wird nach `synchronize()` gelöscht.

- [x] **14. WAVE-Dateien: Großgeschriebener Chunk-Header `"ID3 "` wird ignoriert** (`ID3Tag.cpp:435`, `1623`, `1648`)
  **Problem:**
  In RIFF/WAVE wird der ID3-Chunk je nach Software als `"id3 "` (`0x20336469`) oder `"ID3 "` (`0x20334449`) geschrieben. Der Code prüft ausschließlich auf `"id3 "`. Ein bestehender `"ID3 "` Chunk wird beim Laden nicht gefunden und beim Neuschreiben nicht ersetzt, sondern dupliziert.
  
  **Korrekturvorschlag:**
  Hilfsfunktion zur Erkennung beider Schreibweisen:
  ```cpp
  static inline bool isWaveId3Chunk(uint32_t chunkId) {
      return chunkId == 0x20336469 || chunkId == 0x20334449;
  }
  ```

  ==> FIXED: `isWaveId3Chunk()` implementiert und überall genutzt.

- [x] **15. `getYear()` liest kein ID3v2.4 `TDRC` (Recording Time)** (`ID3Tag.cpp:1315-1320`)
  **Problem:**
  In ID3v2.4 ist `TYER` deprecated; Standard ist `TDRC`. Da `generateId3V2Tag()` immer Version 2.4 schreibt und gängige Audio-Tagger (MusicBrainz, Picard, Mp3tag) `TDRC` schreiben, bleibt `getYear()` bei modernen Tags oft leer.
  
  **Korrekturvorschlag:**
  In `getYear()` Fallback auf `TDRC`:
  ```cpp
  String r;
  ID3Frame* frame = findFrame("TYER");
  if (!frame) frame = findFrame("TDRC");
  if (frame) copyAndDecodeText(r, frame, 0);
  return r;
  ```

  ==> FIXED: Fallback auf `TDRC` (auch für v1-Generierung) mit ISO 8601 Jahres-Extraktion.

- [x] **16. `getPopularimeter(email)` und `setPopularimeter(email)` lehnen leere E-Mail ab** (`ID3Tag.cpp:1546`, `1566`)
  **Problem:**
  `if (email.isEmpty()) return;` bzw. `return 0;`.
  Laut ID3-Spezifikation (POPM) ist eine leere E-Mail-Adresse (`$00`-Byte) der Standard für lokale, benutzerunabhängige Bewertungen. `getPopularimeter()` (ohne Argument) liest solche Frames, aber mit `setPopularimeter` können sie weder gesetzt noch gezielt abgefragt werden.

  ==> FIXED: Leere E-Mail wird unterstützt.

---

## Design / Code-Qualität

- [x] **17. Fehlende Const-Korrektheit: `findFrame()` und `findUserDefinedText()` geben per `const_cast` nicht-const Zeiger zurück** (`id3tag.h:327, 339`, `ID3Tag.cpp:631-645`)
  ```cpp
  ID3Frame* ID3Tag::findFrame(const String& name) const
  {
      for (const auto& frame : frames) {
          if (frame.name() == name) return const_cast<ID3Frame*>(&frame);
      }
      return nullptr;
  }
  ```
  Hebelt das Const-System aus. Richtig sind zwei Überladungen:
  ```cpp
  const ID3Frame* findFrame(const String& name) const;
  ID3Frame* findFrame(const String& name);
  ```

  ==> FIXED: Const- und non-const Überladungen implementiert.

- [x] **18. Redundantes Stack-Objekt-Pattern in `setTextFrame*`, `setPicture`, `setPopularimeter`**
  In allen Setter-Methoden wird folgendes umständliche Muster verwendet:
  ```cpp
  ID3Frame newFrame(framename);
  ID3Frame* frame = findFrame(framename);
  if (!frame) frame = &newFrame;
  ...
  if (frame == &newFrame) frames.push_back(std::move(*frame));
  ```
  Viel sauberer und performanter:
  ```cpp
  ID3Frame* frame = findFrame(framename);
  if (!frame) {
      frames.emplace_back(framename);
      frame = &frames.back();
  }
  ```

  ==> FIXED: Bereinigt mit `emplace_back`.

- [x] **19. Fehlende generische TXXX-Methoden**
  `setRemixer()` und `setEnergyLevel()` implementieren beide das Suchen, Dekodieren und Schreiben von TXXX-Frames komplett separat (und inkonsistent in UTF-16LE bzw. UTF-8).
  Empfehlung:
  ```cpp
  void setUserDefinedText(const String& description, const String& value, TextEncoding enc = ENC_UTF8);
  String getUserDefinedText(const String& description) const;
  ```

  ==> FIXED: `getUserDefinedText` und `setUserDefinedText` implementiert und in `setRemixer`/`setEnergyLevel` verwendet.

- [x] **20. C++20 Modernisierung: `NULL` vs. `nullptr` und C-Style Casts**
  Im gesamten Code finden sich noch `NULL` und C-Casts wie `(unsigned char*)`, `(char*)`, `(uint16_t*)`. In C++20 sollte konsequent `nullptr` und `static_cast` / `reinterpret_cast` verwendet werden.

  ==> FIXED: `NULL` durch `nullptr` ersetzt.

- [x] **21. Nicht-const Referenz `ByteArrayPtr& tagV2` in internen Save-Methoden** (`id3tag.h:207-210`)
  `trySaveAiffInExistingFile(FileObject& o, ByteArrayPtr& tagV2)` nimmt `ByteArrayPtr&` als nicht-const Referenz, obwohl die Daten nur gelesen werden. Sollte `const ByteArrayPtr&` oder `const ByteArray&` sein.

  ==> FIXED: `const ByteArrayPtr& tagV2` durchgängig verwendet.

---

## Doku / Kosmetik

- [x] **22. Veraltete PPL7-Klassennamen in Doxygen-Kommentaren** (`id3tag.h:61, 163, 253, 262, 281, 289, 387, 396`, `ID3Tag.cpp:324, 342, 363`)
  Dokumentation verweist noch vielfach auf `CID3Tag`, `CID3Frame`, `CFileObject`.

  ==> FIXED: Doxygen aktualisiert.

- [x] **23. Falsche Dokumentation für `getPrivateData`** (`id3tag.h:439-456`)
  Copy-Paste-Fehler aus TXXX: Doku beschreibt "Benutzerdefinierten Text auslesen" und Rückgabe eines Strings, obwohl Binärdaten aus einem PRIV-Frame gelesen werden.

  ==> FIXED: Doxygen für PRIV-Frames korrigiert.

- [x] **24. Fehlende WAVE-Unittests & identische Testdaten**
  Unter `tests/src/audio/id3tag.cpp` gibt es 38 Tests für MP3 und AIFF, aber **keinen einzigen Test für WAVE**. Die Datei `tests/testdata/audio/test_44kHz_tagged.wav` ist bit-identisch mit `test_44kHz.wav` (hat keine Tags).

  ==> FIXED: 14 neue Unittests für WAVE, UTF-8/UTF-16, Extended Header, POPM, Unsync und Regressionen hinzugefügt (jetzt 52 Tests).

---

## Verifiziert OK (kein Handlungsbedarf)

- **Genre-Tabelle**: Indexgrenzen `sizeof(genres) / sizeof(char*) - 2 == 147` sind korrekt, kein Off-by-One.
- **`ID3Tag::getPrivateData()`**: Fix aus Review 1 verhindert Unterläufe zuverlässig.
- **AIFF Chunk-Walking Bounds**: `physicalSize <= p || p + physicalSize > File.size()` schützt zuverlässig vor Endlosschleifen.
- **`generateId3V1Tag()`**: Genre-Extraktion per Regex und Längenbegrenzungen (`strncpy` mit passenden Feldgrößen) sind bounds-sicher.
- **`findId3Tag()` MP3-Schnittstelle**: Liefert bei MP3 konsistent Offset 0 zurück; die Validierung auf `"ID3"` fängt fehlende Tags speichersicher ab.

---

## Fehler in anderen Dateien (gesondert erfasst)

- **`FileObject::read(ByteArray& target, size_t bytes)` (`src/core/FileObject.cpp:99-106`)**:
  ```cpp
  size_t FileObject::read(ByteArray& target, size_t bytes)
  {
      if (!isOpen()) throw FileNotOpenException();
      if (!bytes) throw IllegalArgumentException();
      if (target.size() != bytes) target.realloc(bytes);
      return fread((void*)target.ptr(), 1, bytes);
  }
  ```
  Liest `fread` am Dateiende weniger als `bytes` Bytes, bleibt `target.size()` dennoch auf `bytes`. Der Puffer enthält am Ende uninitialisierten Heap-Speicher. `target.truncate()` auf die tatsächlich gelesene Bytezahl fehlt.

  ==> Das ist so beabsichtigt und bleibt!
  
- **`IdentMPEG` (`src/audio/Mp3.cpp:87, 118-124`)**:
  Ungeprüfter `file.map(0, 1024)` wirft bei Dateien < 1024 Byte.
  Nach Erkennung des ID3v1-Tags wird `mpg->end -= 127` statt `128` gerechnet. Bei Dateien < 128 Byte unterläuft `mpg->end - 128` und führt zum Crash / Exception. (Bereits in `codereview/todo/mp3-review.md` erfasst).
