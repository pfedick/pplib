# WideString Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/widestring.h` + `src/types/WideString.cpp` (~1720 Zeilen .cpp),
verglichen mit `include/pplib/types/string.h` + `src/types/String.cpp`. Exception-Namen gegen `include/pplib/exceptions.h`
verifiziert (`OutOfMemoryException`, `IllegalArgumentException`, `CharacterEncodingException`, `OutOfBoundsException` existieren
alle), `PrintDebug`/`HexDump`-Signaturen gegen `include/pplib/core/functions.h` verifiziert. Nutzung im Repo per grep geprüft
(u.a. `Iconv.cpp`, `ByteArrayPtr.cpp`, `Pcre.cpp`, `File.cpp`, `Dir.cpp`).
Zwei zentrale Befunde wurden nicht nur durch Code-Lesen, sondern zusätzlich **experimentell mit Valgrind gegen die gebaute
`libpplib_core.a`** verifiziert (Reproduktions-Code s. jeweiliger Fund) – beide Traces sind exakt wie vorhergesagt aufgetreten.

## Bugs (kritisch)

- [ ] **`set(const char*, size)` / `set(const wchar_t*, size)`: Heap-Buffer-Overread bei explizit angegebener Größe** (`WideString.cpp:253`, `WideString.cpp:283`)
  Beide Funktionen berechnen die zu übernehmende Länge korrekt per Ternary (`size` falls angegeben, sonst `strlen`/`wcslen`),
  klemmen das Ergebnis aber direkt danach **nochmal** über einen unbedingten `strlen`/`wcslen`-Aufruf auf den *echten* Puffer:
  ```cpp
  size_t inchars = (size != (size_t)-1) ? size : ::wcslen(str);
  if (inchars > ::wcslen(str)) inchars = ::wcslen(str);   // <- liest str auch dann komplett, wenn size explizit gesetzt ist
  ```
  Das setzt voraus, dass `str` in jedem Fall nullterminiert ist – genau die Annahme, die der `size`-Parameter eigentlich
  überflüssig machen soll (ein paar Zeilen weiter unten wird sogar extra ein `temp_holder`-Mechanismus gebaut, weil der Autor
  weiß, dass der Eingabepuffer nicht nullterminiert sein könnte – der schützt aber nur den späteren Self-Assignment-Check,
  nicht diesen `strlen`/`wcslen`-Aufruf).
  Reproduziert mit Valgrind (Puffer exakt `size` Elemente groß, ohne Platz für ein Null-Zeichen):
  ```cpp
  wchar_t* buf = (wchar_t*)malloc(3 * sizeof(wchar_t));
  buf[0]=L'a'; buf[1]=L'b'; buf[2]=L'c';       // NICHT nullterminiert
  pplib::WideString w(buf, 3);
  ```
  ```
  ==...== Invalid read of size 4
  ==...==    at wcslen (vg_replace_strmem.c:2139)
  ==...==    by pplib::WideString::set(wchar_t const*, unsigned long) (WideString.cpp:283)
  ==...==    by pplib::WideString::WideString(wchar_t const*, unsigned long) (WideString.cpp:72)
  ==...==  Address ... is 0 bytes after a block of size 12 alloc'd
  ```
  Das ist kein Nischenfall: Der Bug ist über eine reguläre öffentliche API direkt erreichbar, ganz ohne konstruierten Testcode –
  `ByteArrayPtr::toWideString()` (`ByteArrayPtr.cpp:186`) macht exakt das:
  ```cpp
  WideString ByteArrayPtr::toWideString() const
  {
      if (!ptradr) return WideString();
      return WideString((const wchar_t*)ptradr, ptrsize / sizeof(wchar_t));
  }
  ```
  `ByteArrayPtr` (anders als `ByteArray`) garantiert **keine** Null-Terminierung des referenzierten Speichers – es ist nur eine
  nicht-besitzende Sicht auf einen beliebigen Speicherblock. Jeder `ByteArrayPtr::toWideString()`-Aufruf auf einem Speicherblock,
  der nicht zufällig hinter dem letzten Element ein Null-`wchar_t` stehen hat, liest über die Puffergrenze hinaus (bestätigt
  mit Valgrind über exakt diesen Call-Pfad). Die char-Variante ist identisch reproduzierbar (`strlen` statt `wcslen`,
  `WideString.cpp:253`), und pflanzt sich sogar doppelt fort, weil `set(const char*, size)` intern nochmal `String::set(const
  char*, size)` aufruft, das denselben Fehler hat (`String.cpp:225`) – zwei OOB-Reads für einen Aufruf.
  Fix: Das nachträgliche Klemmen nur dann per `strlen`/`wcslen` durchführen, wenn `size == (size_t)-1` war (dann ist es
  ohnehin schon geschehen); andernfalls höchstens auf eine eingebettete Null **innerhalb** von `size` Zeichen prüfen, ohne
  darüber hinauszulesen:
  ```cpp
  size_t inchars = size;
  if (size == (size_t)-1) {
      inchars = ::wcslen(str);
  } else if (const wchar_t* z = (const wchar_t*)wmemchr(str, 0, size)) {
      inchars = z - str;
  }
  ```
  (analog mit `memchr`/`strlen`-Ersatz für die `char*`-Variante, und die identische Stelle in `String::set()` reparieren).

  ==> FIXED: unbedingtes `strlen`/`wcslen`-Nachklemmen entfernt, `size` wird jetzt wie in `String::set()` exakt vertraut
  (kein Scan mehr über die vom Aufrufer angegebene Größe hinaus). `set(const char*, size)` kann eingebettete 0-Bytes wegen
  `::mbstowcs()` weiterhin nicht verlustfrei übernehmen – das ist jetzt explizit im Header dokumentiert (@attention),
  siehe nächster Punkt.

- [ ] **`set(const char*, size)` (und damit praktisch jeder char\*-Konstruktor/-Operator) ist Locale-abhängig statt UTF-8, entgegen eigener Doku** (`WideString.cpp:246-274`)
  Klassen-Doku (widestring.h:62-66) verspricht: *"Bei Übernahme eines C-Strings wird erwartet, dass dieser im UTF-8 Format
  vorliegt"*. Implementiert ist das aber über `::mbstowcs()`, das die aktuell über `setlocale()` gesetzte Prozess-Locale
  verwendet (`LC_CTYPE`) – nicht fest UTF-8. Unter der Default-Locale `"C"` (Standard direkt nach Prozessstart, siehe auch der
  eigene Hinweis bei `lowerCase()`: *"stehen standardmäßig auf 'C'"*) akzeptiert `mbstowcs` keine Bytes ≥ 0x80 als gültige
  Mehrbyte-Sequenz. Reproduziert (frischer Prozess, keine `setlocale()`-Aufruf, wie es z.B. in einem Cronjob/Container/Dienst
  ganz normal ist):
  ```cpp
  pplib::WideString w(L"café");
  pplib::ByteArray utf8 = w.toUtf8();      // korrekt: UTF-8 Bytes für "café" (garantiert lt. eigener Doku "immer UTF-8")
  pplib::WideString w2;
  w2.set((const char*)utf8.ptr(), utf8.size());   // wirft CharacterEncodingException statt "café" zu liefern!
  ```
  ```
  terminate called after throwing an instance of 'pplib::CharacterEncodingException'
  ```
  Damit ist nicht mal der Roundtrip `toUtf8()` → `set(char*)` **innerhalb desselben Prozesses** verlässlich – die einzige
  Methode, die tatsächlich garantiert UTF-8-fest ist, ist `toUtf8()` selbst (eigener handgeschriebener Encoder), die
  Rückrichtung fehlt komplett. Betroffen sind transitiv: alle `char*`-Konstruktoren/`set`/`append`/`prepend`/`operator=`,
  sowie `setf`/`appendf`/`prependf`/`format` (die über `String::vasprintf` + `set(String)` denselben Weg nehmen). Das ist
  exakt das in `REFACTORING.md` als offenes Problem benannte Thema (*"Unter Windows ist die Standard-Locale 'ANSI', unter
  Linux 'UTF-8' ... wir müssen die Tests anpassen, dass sie auch unter Windows mit UTF-8 als Locale laufen"* bzw. Ziel
  *"Alle Tests gehen von UTF-8 als locale aus, auch unter Windows"*) – nur dass es kein Test-, sondern ein Laufzeitproblem der
  Klasse selbst ist.
  Fix: `set(const char*, size)` analog zu `toUtf8()` einen fest verdrahteten, locale-unabhängigen UTF-8-Decoder verwenden
  (Gegenstück zum bereits vorhandenen handgeschriebenen UTF-8-Encoder), statt `mbstowcs()`. Damit wird gleichzeitig
  das REFACTORING.md-Ziel "UTF-8 überall, auch unter Windows" für diese Klasse erfüllt.

## Bugs (mittel)

- [ ] **`len()`/`size()`/`operator[]`/`get()`/`left()`/`right()`/`mid()`/`chopLeft()`/`chopRight()`/`cut()` zählen unter Windows UTF-16-Code-Units statt Zeichen – Surrogate-Pairs können zerschnitten werden** (durchgängig, z.B. `WideString.cpp:711-730, 861-886`)
  Unter Windows (`wchar_t` = 2 Byte, UTF-16) ist ein Zeichen jenseits des BMP (z.B. Emoji, U+10000+) intern **zwei**
  `wchar_t`-Einheiten (Surrogate-Pair) lang. `toUtf8()`/`toUCS4()`/`fromUCS4()` sind sich dessen bewusst (eigene
  `#if defined(_WIN32) || __SIZEOF_WCHAR_T__==2`-Behandlung, korrekt implementiert). Alle anderen Methoden, die mit
  Positionen/Längen arbeiten (`len()`, `operator[]`, `get()`, `left()`, `right()`, `mid()`, `chopLeft()`, `chopRight()`,
  `cut(pos)`, `set(position, wchar_t)` ...) kennen dieses Konzept nicht: `stringlen` ist schlicht die Anzahl `wchar_t`-Einheiten.
  Ein `left(1)` auf einem String, der mit einem Emoji beginnt, liefert unter Windows nur das High-Surrogate – ein für sich
  genommen ungültiges, unpaariges Surrogate. Ein anschließendes `toUtf8()` auf diesem "abgeschnittenen" String kodiert dann
  (siehe nächster Punkt) eine ungültige UTF-8-Sequenz statt einer Exception oder des vollständigen Zeichens.
  Unter Linux/FreeBSD (`wchar_t` = 4 Byte) tritt das Problem nicht auf, da dort ein `wchar_t` immer einen vollständigen
  Codepoint fasst – auf dieser Plattform daher nicht reproduzierbar, aber ein reales Problem für den Windows-Build.
  Fix: Mindestens dokumentieren, dass Positionsangaben Code-Units und keine Zeichen sind (analog zu UTF-16-APIs wie
  `std::u16string`); für "sauberes" Verhalten müssten die positionsbasierten Methoden Surrogate-Grenzen respektieren.

- [ ] **Integer-Overflow in `repeat(size_t num)` / `repeat(const WideString&, size_t num)` vor `reserve()`** (`WideString.cpp:1227-1243, 1259-1282`)
  ```cpp
  reserve(stringlen * num);   // Multiplikation kann size_t überlaufen, BEVOR reserve() seinen Größen-Check sieht
  ...
  for (size_t i = 1; i < num; i++) {
      wmemcpy(tmp, ptr, stringlen);   // schreibt num-1 mal stringlen Zeichen in einen zu klein reservierten Puffer
  ```
  Ist `stringlen * num` groß genug, um `size_t` zu überlaufen (z.B. `stringlen=2`, `num=SIZE_MAX/2+1` → Produkt wraps auf 1),
  reserviert `reserve()` einen viel zu kleinen Puffer, und die anschließende Kopierschleife schreibt weit über das Ende hinaus.
  Analog zum bereits in `ByteArray::malloc()`/`calloc()` gefundenen Muster (Größen-Check muss vor jeder Multiplikation
  stehen, nicht danach). Praktische Relevanz ist gering (setzt ein extrem großes `num` voraus), aber bei extern kontrolliertem
  Wiederholungsfaktor ein reales Risiko.
  Fix: Vor der Multiplikation prüfen, z.B. `if (num != 0 && stringlen > SIZE_MAX / num) throw IllegalArgumentException(...);`.

  ==> FIXED: Overflow-Check vor beiden Multiplikationen ergänzt (`IllegalArgumentException`, analog zu `reserve()`s
  eigenem Overflow-Check in dieser Klasse).

- [ ] **`toUtf8()`/`toUCS4()`: unpaarige Surrogates unter Windows erzeugen still ungültige Ausgabe statt Fehlerbehandlung** (`WideString.cpp:606-616, 653-663`)
  Wird ein High-Surrogate ohne nachfolgendes Low-Surrogate (oder ein alleinstehendes Low-Surrogate) angetroffen – z.B. genau
  durch den vorherigen Punkt (`left()`/`mid()` auf einer Surrogate-Grenze) – bleibt `codepoint` auf dem rohen
  Surrogate-Wert (0xD800–0xDFFF) stehen und wird anschließend ganz normal als 3-Byte-UTF-8-Sequenz kodiert bzw. 1:1 in
  UCS-4 übernommen. Das erzeugt laut RFC 3629 **ungültiges UTF-8** (Surrogates sind dort explizit ausgeschlossen), statt
  eine Exception zu werfen oder ein Replacement-Zeichen (U+FFFD) einzusetzen.
  Fix: Bei unpaarigem Surrogate entweder `CharacterEncodingException` werfen oder durch U+FFFD ersetzen, statt den
  rohen Surrogate-Wert weiterzukodieren.

## Design

- [ ] **`WideString(const ByteArrayPtr&)` / `ByteArrayPtr::toWideString()` interpretieren die Bytes roh als natives `wchar_t`-Array, `String(const ByteArrayPtr&)` dagegen als kodierten Text – identische Doku, komplett andere Semantik** (`WideString.cpp:125-131` vs. `String.cpp:289-298`)
  ```cpp
  // WideString.cpp
  WideString::WideString(const ByteArrayPtr& str)
  { ...; set((wchar_t*)str.adr(), str.size() / sizeof(wchar_t)); }   // Bytes = rohe wchar_t-Elemente

  // String.cpp
  String& String::set(const ByteArrayPtr& str, size_t size)
  { ...; return set((const char*)str.adr(), inbytes); }              // Bytes = kodierter Text (lokale Kodierung)
  ```
  Beide Konstruktoren/Setter tragen in der Doku wortgleich *"Ein String wird aus einem ByteArrayPtr erstellt"* – nichts
  deutet auf die fundamental unterschiedliche Interpretation hin. Wer testet, dass `String(bap)` denselben Inhalt liefert
  wie `bap.toString()`, und in Analogie `WideString(bap)`/`bap.toWideString()` erwartet, bekommt keinen dekodierten Text,
  sondern je nach Plattform 1/2/4 Bytes pro "Zeichen" aus dem Rohspeicher reinterpretiert (durch Tests bestätigt als
  bewusst gewählter Binär-Rundtrip für `ByteArray(WideString)`/`toWideString()`, s. `tests/src/types/bytearray.cpp:150-159`
  – aber nirgends von der `String`-Variante abgegrenzt dokumentiert). Genau das Bug-Muster, das im FileObject-Review bereits
  bei `getws` auffiel: String- und WideString-Pendants sehen gleich aus, verhalten sich aber nicht analog.
  Fix: Doku klar trennen ("roher Binär-Rundtrip, kein Encoding" vs. "dekodierter Text"), ggf. Methode umbenennen
  (z.B. `fromRawWideChars`/`toRawWideChars`) um Verwechslung mit dem textuellen `String`-Pendant auszuschließen.

- [ ] **Kein `WideString::set(const ByteArrayPtr&, size_t)` als Gegenstück zu `String::set(const ByteArrayPtr&, size_t)`** (widestring.h, vgl. string.h:503)
  `String` kann sowohl per Konstruktor als auch per `set()` aus einem `ByteArrayPtr` befüllt werden, `WideString` nur per
  Konstruktor (`explicit WideString(const ByteArrayPtr& str)`, widestring.h:142) – eine bestehende Instanz kann nicht per
  `set()` neu aus einem `ByteArrayPtr` belegt werden. Kleine, aber unnötige API-Asymmetrie.

- [ ] **`String::shl()`/`String::shr()` haben kein Gegenstück in `WideString`** (string.h:1080/1093)
  Falls die Funktionen für `String` als gebraucht gelten, fehlt die konsistente `WideString`-Variante; falls nicht,
  sollten sie ggf. aus `String` entfernt werden (Symmetrie in beide Richtungen möglich).

## Doku / Kosmetik

- [ ] **`WideString::setGlobalEncoding` in der Doku referenziert, existiert aber nirgends im Code** (widestring.h:66, 518)
  Klassen-Doku und `append()`-Doku verweisen auf eine statische Funktion `WideString::setGlobalEncoding`, die weder deklariert
  noch implementiert ist (per grep im gesamten Repo nicht gefunden – dasselbe Phantom existiert identisch bei `String`,
  string.h:143/625). Vermutlich ein Überbleibsel aus einer älteren PPL-Version. Sollte entweder nachgezogen oder aus der
  Doku entfernt werden, insbesondere da der o.g. Locale-Bug (`set(const char*, size)`) nahelegt, dass gerade eine
  konfigurierbare/verlässliche Kodierung fehlt.

- [ ] **`chop()`/`chopRight()`: identischer Code dupliziert statt delegiert** (`WideString.cpp:1089-1107`)
  Beide Methoden haben einen Byte-für-Byte identischen Funktionskörper. `String` macht es sauberer – `chop()` ist dort
  inline als Weiterleitung auf `chopRight()` implementiert (string.h:1027-1030). Kandidat für Angleichung.

- [ ] **`trimLeft(const WideString& chars)`: lokale Variable `s` überschattet das private Member `s` (Kapazität)** (`WideString.cpp:1027-1052`)
  ```cpp
  WideString& WideString::trimLeft(const WideString& chars)
  {
      if (stringlen > 0 && chars.stringlen > 0) {
          size_t i, start, s, z;   // <- "s" ist auch der Name des privaten Kapazitäts-Members
  ```
  Funktional unkritisch (der Compiler löst korrekt auf die lokale Variable auf), aber verwirrend beim Lesen/Debuggen und
  ein Stolperstein für künftige Refactorings. Zusätzlich unterscheidet sich der Algorithmus unnötig vom saubereren,
  früh abbrechenden Pendant in `String::trimLeft(chars)`/`String::trimRight(chars)` (String.cpp:852-905).
  Fix: Variable umbenennen (z.B. `foundMismatch`), Implementierung ggf. an den `String`-Stil angleichen.

## Verifiziert OK (kein Handlungsbedarf)

- Self-Assignment-/Self-Append-/Self-Prepend-/Self-Repeat-Schutz (`set()`, `append()`, `prepend()`, `repeat(const
  WideString&, size_t)`) ist überall korrekt über Pointer-Overlap-Check + Staging in einen unabhängigen `WideString`
  bzw. `std::vector`-Puffer *vor* dem eigentlichen Kopiervorgang gelöst – analog zum bereits geprüften Muster in `ByteArray`.
- `reserve()` ist gegen Integer-Overflow bei `(size + 1) * sizeof(wchar_t)` explizit abgesichert (`WideString.cpp:153`).
- Move-Konstruktor und Move-Zuweisung setzen die Quelle korrekt zurück (`ptr=NULL, s=0, stringlen=0`), Self-Move wird
  in `operator=(WideString&&)` per `this != &other`-Guard abgefangen.
- Kein hartes `sizeof(wchar_t) == 4` irgendwo im Code gefunden – `toUtf8()`, `toUCS4()`, `fromUCS4()`, `byteLength()` und
  der `ByteArrayPtr`-Konstruktor verwenden durchgängig `sizeof(wchar_t)` bzw. eine `#if`-Weiche auf `_WIN32`/
  `__SIZEOF_WCHAR_T__ == 2`, keine hartkodierte Breitenannahme.
- Keine Vererbungshierarchie wie bei `ByteArray`/`ByteArrayPtr` – die dort gefundene LSP-Problematik (kein virtueller
  Destruktor/keine virtuellen Methoden) ist für `WideString` nicht relevant.
- Exception-Namen (`OutOfMemoryException`, `IllegalArgumentException`, `CharacterEncodingException`,
  `OutOfBoundsException`) sind korrekt aus `exceptions.h` verwendet, keine erfundenen Typen.
