# ByteArray / ByteArrayPtr Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/bytearrayptr.h` + `src/types/ByteArrayPtr.cpp`,
`include/pplib/types/bytearray.h` + `src/types/ByteArray.cpp`.
Verifiziert: `String::getPtr()`/`WideString::getPtr()` liefern `const char*`/`const wchar_t*` (string.h:1321, widestring.h:1001).
`ByteArrayPtr` wird an vielen Stellen per `const ByteArrayPtr&` durchgereicht (iconv.h, image.h, memfile.h, resource.h, pfpfile.h, functions.h) – nirgends per Value, Slicing über Funktionsparameter also aktuell kein Problem.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`ByteArray` erbt öffentlich von `ByteArrayPtr`, aber nichts davon ist virtuell → LSP-Verletzung** (`bytearray.h:73`, `bytearrayptr.h`)
  `ByteArrayPtr` hat weder virtuellen Destruktor noch virtuelle Methoden. `ByteArray::truncate(size_t)` (bytearray.h:277) *überschreibt nicht*,
  sondern *versteckt* `ByteArrayPtr::truncate(size_t)` (bytearrayptr.h:316) – gleicher Name, gleiche Signatur, kein `override` möglich
  (Basis ist nicht `virtual`). Zugriff über eine `ByteArrayPtr&`/`ByteArrayPtr*` auf ein `ByteArray`-Objekt ruft die Basis-Version:
  ```cpp
  ByteArray ba(10);
  ByteArrayPtr& ref = ba;
  ref.truncate(3);   // ruft ByteArrayPtr::truncate -> setzt nur ptrsize, schreibt KEINE 4 Null-Bytes
  const char* cstr = ba;   // Klassen-Invariante "immer 4 Null-Bytes am Ende" ist jetzt verletzt
  ```
  Zusätzlich: kein virtueller Destruktor →
  ```cpp
  ByteArrayPtr* p = new ByteArray(10);
  delete p;   // undefined behavior, ~ByteArray() läuft nicht -> ptradr wird nie freigegeben (Leak)
  ```
  Fix: entweder `virtual ~ByteArrayPtr()` + `virtual void truncate(size_t)` (und `override` in `ByteArray`) einführen, oder – sauberer,
  da die Klassen laut Doku keine echte Ist-eine-Beziehung haben, sondern nur Storage/Interface wiederverwenden – auf `private`/`protected`
  Vererbung oder Komposition umstellen, damit eine `ByteArray` gar nicht erst als `ByteArrayPtr` "gesliced" werden kann.

  ==> FIXED: Destruktor ist nun virtuell, Methode "truncate" aus ByteArrayPtr entfernt

- [ ] **`ByteArray::malloc()` / `ByteArray::calloc()`: Use-after-free bei Größenüberschreitung** (`ByteArray.cpp:235-267`)
  ```cpp
  void* ByteArray::malloc(size_t size)
  {
      ::free(ptradr);                       // <- alter Speicher wird sofort freigegeben
      if (size > MAX_BYTEARRAY_SIZE) {
          throw OutOfMemoryException();     // <- ptradr zeigt jetzt auf bereits freigegebenen Speicher!
      }
      ptradr = ::malloc(size + 4);
      ...
  ```
  Wirft die Exception, bleibt `ptradr` auf dem gerade freigegebenen Block stehen und `ptrsize` auf dem alten (jetzt ungültigen) Wert.
  Jeder weitere Zugriff auf das Objekt (Destruktor, `free()`, ein weiterer `copy()`/`malloc()`-Aufruf) ruft `::free()` ein zweites Mal
  auf denselben Pointer auf → Double-Free / Heap-Korruption. `calloc()` hat exakt dasselbe Muster.
  Fix: Größen-Check **vor** dem `::free()`:
  ```cpp
  void* ByteArray::malloc(size_t size)
  {
      if (size > MAX_BYTEARRAY_SIZE) {
          throw OutOfMemoryException();
      }
      ::free(ptradr);
      ptradr = ::malloc(size + 4);
      ...
  ```

  ==> FIXED

- [ ] **`fromBase64()` überschreibt das garantierte Null-Terminator-Byte bei jedem gepaddeten Input** (`ByteArray.cpp:393-443`)
  Die Klasse garantiert laut Header-Doku (bytearray.h:47-49) immer 4 Null-Bytes hinter dem Speicherbereich, damit er direkt als
  C-String verwendet werden kann. Der Decoder schreibt aber spekulativ bereits das *nächste* Ausgabe-Byte vor, bevor er weiß,
  ob noch ein weiteres Zeichen kommt:
  ```cpp
  case 1:
      t[target++] |= value >> 4;
      t[target] = (value & 0x0F) << 4;   // <- schreibt "vorsorglich" das nächste Byte
      break;
  ```
  Bei einem regulären, korrekt gepaddeten Input wie `"QQ=="` (1 Byte Nutzdaten) wird `bytes = 1` berechnet, aber dieses Statement
  schreibt trotzdem auf `t[1]` – das erste der 4 reservierten Null-Bytes – bevor die Schleife beim `=` abbricht. Test:
  ```cpp
  ByteArray ba = ByteArray::fromBase64("QQ==");
  // ba.size() == 1, aber ba.adr()[1] ist KEIN 0-Byte mehr, sondern Deko-Datenmüll
  ```
  Wird `ba` danach als C-String benutzt (`(const char*)ba`), liest man über die eigentlichen Daten hinaus Müll statt eines
  Terminators. Betrifft jeden Aufruf mit 1 oder 2 Padding-Zeichen (also praktisch jeden Base64-String, dessen Länge nicht durch 3 teilbar ist).
  Fix: Schreibzugriff auf das "nächste" Byte nur, wenn es tatsächlich noch zu den Nutzdaten gehört:
  ```cpp
  case 1:
      t[target++] |= value >> 4;
      if (target < bytes) t[target] = (value & 0x0F) << 4;
      break;
  case 2:
      t[target++] |= value >> 2;
      if (target < bytes) t[target] = (value & 0x03) << 6;
      break;
  ```
  ==> FIXED
## Bugs (mittel)

- [ ] **`fromBase64()` validiert die Eingabelänge nicht** (`ByteArray.cpp:393-406`)
  Es wird nirgends geprüft, dass `chars % 4 == 0` ist (Basis-Voraussetzung für gültiges Base64). Bei einer Länge, die kein
  Vielfaches von 4 ist, liefert `(chars * 3) / 4 - padding` eine Byte-Zahl, die nicht zur tatsächlichen Anzahl decodierter Bytes passt –
  je nach Rest bleiben Teile des mit `::malloc` (nicht `::calloc`!) allokierten Speichers uninitialisiert, oder es wird wie oben
  beschrieben über `bytes` hinausgeschrieben. Ein manipulierter/kaputter Base64-String führt so zu Datenmüll statt der dokumentierten
  `IllegalArgumentException("invalid chars in input string")`.
  Fix: am Anfang `if (chars % 4 != 0) throw IllegalArgumentException("invalid length");`.

  ==> FIXED

## Design

- [ ] **`ByteArrayPtr(const String&)` / `(const WideString&)` hebeln die Konstantheit des referenzierten Objekts komplett aus** (`ByteArrayPtr.cpp:52-62`)
  ```cpp
  ByteArrayPtr::ByteArrayPtr(const String& data)
  {
      ptradr = (void*)data.getPtr();   // const wird weggecastet
      ptrsize = data.size();
  }
  ```
  `ByteArrayPtr` bietet nicht-konstanten Schreibzugriff auf `ptradr` (`operator[]` non-const, `set()`, `memset()`). Dadurch kann man
  über den Umweg `ByteArrayPtr` in eine als `const` übergebene `String`/`WideString` schreiben:
  ```cpp
  void f(const String& s) {
      ByteArrayPtr bp(s);
      bp.memset(0);   // "const" String wird trotzdem überschrieben - kein Compiler-Warning
  }
  ```
  Das ist eine stille Aushebelung der Const-Korrektheit und eine Falle für jeden, der sich auf `const String&` als "read-only" verlässt.
  Fix: entweder diese Konstruktoren nur noch aus nicht-konstanten `String&`/`WideString&` erlauben (sofern `getPtr()` dafür eine
  non-const Variante bekommt), oder – einfacher – `ByteArrayPtr`, das per String/WideString konstruiert wurde, intern als "read-only"
  markieren und bei `set()`/`operator[]`(non-const)/`memset()` eine Exception werfen.

  ==> bleibt so

- [ ] **Duplizierter Self-Overlap-Schutz in `copy()`, `append()`, `prepend()`** (`ByteArray.cpp:118-122, 160-164, 197-201`)
  Identischer 5-Zeiler (Adressbereich-Check + `std::vector`-Staging) dreimal kopiert. Kandidat für eine private Hilfsfunktion, z.B.
  `static const void* stageIfOverlapping(const void* adr, size_t size, size_t ptrsize_self, void* ptradr_self, std::vector<char>& holder)`.

  ==> bleibt erstmal so

- [ ] **Sehr breite, teils implizite Pointer-Schnittstelle** (`bytearrayptr.h:157-165, 203-219, 223-226`)
  `adr()`, `ptr()` und `toCharPtr()` liefern alle denselben Pointer nur mit anderem Namen/Typ, zusätzlich drei *implizite*
  Konvertierungsoperatoren (`operator const void*`, `operator const char*`, `operator const unsigned char*`). Das lädt zu versehentlichen
  impliziten Konvertierungen ein (z.B. eine `ByteArrayPtr` landet ungewollt dort, wo ein `const char*` erwartet wird). Für neuen Code
  wäre `explicit` auf den Konvertierungsoperatoren sicherer; für Bestandscode vermutlich zu riskant, daher eher ein Punkt für die
  große Namespace/API-Aufräumrunde als ein akuter Fix.

  ==> bleibt

## Doku / Kosmetik

- [ ] `ByteArrayPtr::map()` (ByteArrayPtr.cpp:101) hat einen auskommentierten Bounds-Check (`// if (!ptradr) throw ...`) – entweder
      bewusst entfernt (dann Kommentar löschen) oder vergessene Baustelle.

    ==> FIXED
- [ ] Inkonsistentes Verhalten bei leerem Objekt: `crc32()` wirft `EmptyDataException` (ByteArrayPtr.cpp:230), aber `toHex()`,
      `toBase64()`, `toString()` liefern für den leeren Fall still ein leeres Ergebnis zurück. Falls beabsichtigt, wenigstens in der
      Doku von `crc32()` explizit erwähnen (aktuell nicht dokumentiert).

    ==> So gewollt, in Doku vermerkt

- [ ] `useadr(adr, size)` (ByteArray.cpp:143-148) hat keinen Schutz gegen `adr == ptradr` (eigene Adresse erneut "übernehmen") –
      würde den Speicher freigeben und danach als "neu" wieder zuweisen (dangling). Sehr unwahrscheinlicher Aufrufer-Fehler, aber
      billig abzusichern mit `if (adr == ptradr) return;`.
    
    ==> FIXED

## Verifiziert OK (kein Handlungsbedarf)

- Self-Assignment (`a = a;`) ist über `copy()`s frühen Vergleich `adr == ptradr && size == ptrsize` sicher abgefangen.
- Move-Assignment prüft `this != &other` korrekt.
- Self-Overlap-Schutz in `copy()`/`append()`/`prepend()` selbst ist korrekt implementiert (Staging über `std::vector`, bevor der alte
  Speicher freigegeben wird) – nur die Code-Duplizierung ist ein Thema (siehe oben).
- `fromHex()` berechnet die Zielgröße exakt (`chars >> 1`) und die Schleife schreibt exakt so viele Bytes – kein Tail-Overflow wie bei `fromBase64()`.
- `calloc()` schreibt die 4 Endpadding-Bytes nicht explizit, das ist aber unschädlich, da `::calloc` den gesamten Block bereits nullt.
