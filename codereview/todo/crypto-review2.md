# Crypto Review 2 (Crypt / Encrypt / Decrypt) – Befunde

Review vom 2026-10-24, Scope: `include/pplib/crypto/crypt.h` (119 Zeilen), `src/crypto/Crypt.cpp` (618 Zeilen).
Öffentliche API: `pplib::Crypt`, `pplib::Encrypt`, `pplib::Decrypt`, `UnsupportedAlgorithmException`, `InvalidAlgorithmException`, `NoAlgorithmSpecifiedException`, `InvalidKeyLengthException`.
Review done by: Copilot (VS Code Copilot SDK)

Umgebung der Verifikation: OpenSSL 3.6.5, Linux x86-64, gcc, C++20. Alle Befunde mit `(verifiziert)` wurden
mit eigenen Compile-/Runtime-Probes gegen die reale Bibliothek geprüft (Test-Suite: 14 Crypto-Tests grün,
`make build_tests` + `test_all --gtest_filter='CryptoTest*'`). Befunde ohne Marker sind Quellcode-Befunde.

Abgrenzung zum alten Review (`codereview/todo/crypt-review.md`, Claude Sonnet 5, 2026-09-01):
Bestätigte, bereits bekannte Punkte sind mit `(bekannt)` markiert, neu gefundene mit `(NEU)`,
widerlegte Sonnet-Vermutungen stehen am Ende unter „Überprüft und NICHT bestätigt".

---

## Bugs (kritisch)

- [ ] **Wiederverwendung desselben Objekts: jeder weitere `encrypt()`/`decrypt()`-Aufruf baut auf der alten Kette auf → Ergebnis ist nicht mehr entschlüsselbar** (`Crypt.cpp:465-486`, `588-609`) `(NEU)` `(verifiziert)`

  `Encrypt::encrypt()` und `Decrypt::decrypt()` sind als „Single-Shot" formuliert (ein Aufruf = ein kompletter
  Vorgang), initialisieren den `EVP_CIPHER_CTX` aber **nicht** neu. Der Context behält nach `EVP_EncryptFinal_ex()`
  den Cipher-Status (bei CBC den letzten Ciphertext-Block als neue IV, bei CFB/OFB den vollen Stromzustand).
  Ein zweiter Aufruf auf demselben Objekt verschlüsselt daher mit einer anderen effektiven IV.

  Reproduktion (pplib-API, reale Messung):
  ```cpp
  pplib::Encrypt enc(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
  enc.setKey(pplib::ByteArrayPtr(key, 32));
  enc.setIV(pplib::ByteArrayPtr(iv, 16));

  pplib::ByteArray c1 = enc.encrypt(pplib::ByteArrayPtr(clearData));
  pplib::ByteArray c2 = enc.encrypt(pplib::ByteArrayPtr(clearData));   // gleiches Klartext, gleiches Objekt
  // c1 = 28acff4282cec7ac74fa8f499f0ca4a9...
  // c2 = 8a9d10afa0ee9f6d2fb6875fa35d7531...   -> UNTERSCHIEDLICH, obwohl Key+IV identisch sind

  pplib::Decrypt dec(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
  dec.setKey(pplib::ByteArrayPtr(key, 32));
  dec.setIV(pplib::ByteArrayPtr(iv, 16));
  pplib::ByteArray back = dec.decrypt(c2);   // läuft durch (Padding zufällig OK), liefert aber Müll:
  // back = "r\0I\235c\354\365\203\341\210\216\356\376o\237\351rtped string, long enough for multiple blocks!"
  ```
  Auswirkung: Stille Datenkorruption (kein Exception, kein Hinweis). Jeder Aufrufer, der ein `Encrypt`-Objekt
  in einer Schleife oder für mehrere Nachrichten verwendet (der natürliche Instinkt bei einer Objekt-API),
  verliert Daten. Genau das ist der typische Use-Case für Streaming/Session-Verschlüsselung.
  Die bestehenden Tests erfassen den Fall nicht, weil dort pro Aufruf ein frisches Objekt erzeugt wird.

  Behebung: Single-Shot-Aufrufe müssen den Context zurücksetzen. Verifiziert:
  `EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, NULL)` setzt nur den Verarbeitungsstatus zurück und behält
  Cipher, Key und IV bei – damit ist `encrypt()` idempotent (c1 == c2, gemessen).
  ```cpp
  void Encrypt::encrypt(const ByteArrayPtr& in, ByteArray& out)
  {
      if (!ctx) throw pplib::NotInitializedException();
      // Single-Shot: Verarbeitungsstatus zuruecksetzen, Key/IV/Cipher bleiben erhalten
      if (!EVP_EncryptInit_ex(static_cast<EVP_CIPHER_CTX*>(ctx), nullptr, nullptr, nullptr, nullptr)) {
          throw pplib::InitializationFailedException("OpenSSL: EVP_EncryptInit_ex, reset");
      }
      ...
  }
  ```
  Zusätzlich eine explizite `reset()`-Methode anbieten (analog zu `Digest::reset()`, `digest.h:83`), damit
  Streaming-Nutzer den Vorgang bewusst starten/beenden können, und in den Header-Dokumenten, dass
  `update()`/`final()` ein Objekt zustandsbehaftet machen.

- [ ] **Rule-of-Three-Verletzung: Kopieren einer `Encrypt`/`Decrypt`-Instanz teilt den rohen `ctx`-Zeiger → Use-after-Free/Double-Free** (`crypt.h:45-89`, `91-102`, `104-115`; `Crypt.cpp:305-310`) `(bekannt)` `(verifiziert)`

  `Crypt` verwaltet `void* ctx` (freigegeben in `~Crypt()`, `Crypt.cpp:305-310`), löscht aber weder Copy-Constructor
  noch Copy-Assignment. Compiler erzeugt implizite Kopien (sichtbar in der Fehlermeldung: „Kandidat
  `Encrypt(const Encrypt&)`", „`Encrypt(Encrypt&&)`").

  Reproduktion mit AddressSanitizer (reale Ausgabe):
  ```cpp
  Encrypt* a = new Encrypt(Crypt::Algo_AES_256, Crypt::Mode_CBC);
  Encrypt b(*a);                       // teilt ctx
  delete a;                            // EVP_CIPHER_CTX_free(ctx)
  b.setKey(pplib::ByteArrayPtr(key, 32));
  ```
  ```
  ==ERROR: AddressSanitizer: SEGV on unknown address 0x0000000000a6
  The signal is caused by a READ memory access. Hint: address points to the zero page.
      #1 EVP_CipherInit_ex (libcrypto.so.3)
      #2 pplib::Encrypt::setKey(...) src/crypto/Crypt.cpp:412
  ```
  Behebung: Kopie löschen, Move erlauben (oder RAII für `ctx`):
  ```cpp
  class Crypt
  {
  public:
      Crypt();
      Crypt(const Crypt&) = delete;
      Crypt& operator=(const Crypt&) = delete;
      Crypt(Crypt&& other) noexcept;
      Crypt& operator=(Crypt&& other) noexcept;
      virtual ~Crypt();
  };
  ```
  Sauberer ist ein besitzender Wrapper, der `void* ctx` überflüssig macht:
  ```cpp
  struct CtxDeleter { void operator()(EVP_CIPHER_CTX* c) const { EVP_CIPHER_CTX_free(c); } };
  using CtxPtr = std::unique_ptr<EVP_CIPHER_CTX, CtxDeleter>;
  ```
  (Damit entfallen auch die `static_cast<EVP_CIPHER_CTX*>(ctx)`-Streuungen im gesamten File.)

- [ ] **`setKey()`/`setIV()` prüfen die Puffergröße nicht → Out-of-Bounds-Read aus dem Aufrufer-Puffer** (`Crypt.cpp:405-415`, `417-426`, `528-538`, `540-549`) `(bekannt)` `(verifiziert)`

  `EVP_EncryptInit_ex()` liest immer `keyLength()` bzw. `ivLength()` Bytes – die Länge des übergebenen
  `ByteArrayPtr` wird nie gegen `keyLength()`/`ivLength()` geprüft.

  Verifikation: Eine Guard-Page-Probe (4 Byte Puffer direkt vor einer unzugänglichen Seite) liefert
  **SIGSEGV (Exit-Code 139)** beim `setKey()`-Aufruf. ASAN meldet nichts, weil `ByteArrayPtr` auf
  Stack/Statik zeigt – d. h. der Fehler ist real, aber für ASAN oft unsichtbar. Ein zweiter Test mit
  AES-256 und 4-Byte-Puffer wird von pplib kommentarlos akzeptiert.

  Behebung (Größenprüfung, bevor OpenSSL liest):
  ```cpp
  void Encrypt::setKey(const ByteArrayPtr& key)
  {
      if (!ctx) throw pplib::NotInitializedException();
      const int need = EVP_CIPHER_CTX_key_length(static_cast<EVP_CIPHER_CTX*>(ctx));
      if (need > 0 && static_cast<int>(key.size()) < need) {
          throw InvalidKeyLengthException("setKey: %u bytes given, %d required", key.size(), need);
      }
      ...
  }
  ```
  Analog `setIV()` mit `EVP_CIPHER_CTX_iv_length()` (bei ECB ist `ivLength() == 0`, siehe eigener Befund).

---
## Bugs (mittel / Warnungen)

- [ ] **`maxKeyLength()` liefert die OpenSSL-Konstante, nicht die Obergrenze des gewählten Algorithmus** (`Crypt.cpp:322-329`, Zeile 327) `(NEU)` `(verifiziert)`

  `maxKeyLength()` gibt pauschal `EVP_MAX_KEY_LENGTH` (64) zurück – unabhängig von Algorithmus und Modus.
  Gemessen: AES-128-CBC → `keyLength()=16`, `maxKeyLength()=64`; AES-256-CBC → `keyLength()=32`,
  `maxKeyLength()=64`. Ein Aufrufer, der daraus eine Schlüssellänge ableitet, läuft in eine Falle:
  ```
  AES-256-CBC: setKeyLength(64) -> InvalidKeyLengthException: 64
  AES-256-CBC: setKeyLength(72) -> InvalidKeyLengthException: 72
  ```
  Der Test `CryptoTest.MaxKeyLength` (`tests/src/crypto/crypto.cpp:354-358`) zementiert die falsche Semantik
  mit `ASSERT_EQ(64, encrypt.maxKeyLength())`.

  Behebung: echte Obergrenze des Ciphers ermitteln (für feste AES-Varianten == `keyLength()`; für variable
  Cipher wie Blowfish/RC2 die Bereichsgrenzen des Ciphers auswerten) und den Test anpassen. Wenn die
  Konstante gewollt ist, muss der Name/Doku das klarstellen (`maxPossibleKeyLength()`), sonst ist die Methode
  für Aufrufer unbrauchbar.

- [ ] **`update()` nach `final()` läuft fehlerfrei weiter und verschlüsselt in die alte Kette** (`Crypt.cpp:428-445`, `447-463`) `(NEU)` `(verifiziert)`

  ```
  encrypt("first message!!")  -> 16 Bytes
  update("second message!!")  -> kein Fehler, 16 Bytes (setzt den alten Strom fort)
  ```
  Ein zu früh/beendetes Objekt wird also nicht als Fehler erkannt. Ebenso ist `final()` zweimal aufrufbar.
  Behebung: Zustand im Objekt führen (`enum State { Fresh, Updating, Finished }`) und `update()` nach
  `final()` mit `OperationFailedException` bzw. `IllegalStateException` ablehnen; `reset()` zum bewussten
  Neustart (siehe kritischer Befund 1).

- [ ] **Streaming-API: `out` wird bei jedem Aufrufer überschrieben, es gibt keine Append-Variante** (`Crypt.cpp:443`, `461`, `484`, `566`, `584`, `607`) `(NEU)` `(verifiziert)`

  `update(in, out)`/`final(out)` rufen `out.copy(...)` auf – `ByteArray::copy()` ersetzt den Inhalt
  (`src/types/ByteArray.cpp:106-137`). Wer denselben `ByteArray` für alle Chunks verwendet, verliert
  jede vorherige Ausgabe; die API bietet kein `append` an und dokumentiert das nicht.
  Verifikation (AES-256-CBC, zwei Chunks in denselben `out`):
  ```
  out nach update#1: 765d2bd7a0a353df...   (16 Bytes)
  out nach update#2: a7f78e60165aad6a...   (16 Bytes)  <- update#1 ist weg
  out nach final():  98a6fac2d569b12e...   (16 Bytes)  <- update#2 ist weg
  ```
  Nur wer selbst `acc.append(out)` hintert (existiert, `ByteArray.cpp:152`), bekommt ein korrektes Ergebnis –
  der Round-Trip klappt dann. Behebung: entweder Doku + Beispiel im Header, oder API ergänzen
  (`updateAppend(const ByteArrayPtr&, ByteArray&)` bzw. `ByteArray& out` mit Append-Semantik).
  Anmerkung für die Pico-Tauglichkeit: pro `update()` wird zusätzlich ein temporäres `ByteArray`
  allokiert und einmal kopiert (`Crypt.cpp:435-436` + `443`) – direkt in `out` verschlüsseln spart
  eine Allokation und eine Kopie pro Chunk.

- [ ] **Falscher Exception-Typ bei OpenSSL-3-Legacy-Algorithmen: `InitializationFailedException` statt `UnsupportedAlgorithmException`** (`Crypt.cpp:396-401`, `519-524`; `CMakeLists.txt:296-311`) `(NEU)` `(verifiziert)`

  Die Configure-Prüfung `check_symbol_exists(EVP_des_ecb, ...)` testet nur die Symbol-Präsenz in
  `openssl/evp.h`. Unter OpenSSL 3.x sind DES/IDEA/RC2/Blowfish/CAST5 weiterhin exportiert, stecken aber
  im Provider `legacy`, der nicht automatisch geladen wird. `config_pplib.h` setzt deshalb
  `HAVE_EVP_DES_ECB=1`, `HAVE_EVP_BF_ECB=1`, `HAVE_EVP_IDEA_ECB=1`, `HAVE_EVP_RC2_ECB=1`,
  `HAVE_EVP_CAST5_ECB=1` (nur `HAVE_EVP_RC5_32_12_16_ECB` ist undef).
  `getCipher()` liefert also einen Nicht-NULL-Cipher, aber `EVP_EncryptInit_ex()` schlägt fehl:
  ```
  Encrypt(Algo_BLOWFISH, Mode_CBC) -> InitializationFailedException: OpenSSL: EVP_EncryptInit_ex
  OpenSSL-Fehler: error:0308010C:digital envelope routines::unsupported
  ```
  Nach `OSSL_PROVIDER_load(nullptr, "legacy")` funktioniert derselbe Cipher (`ret=1`).
  Folge: Ein Aufrufer, der `UnsupportedAlgorithmException` fängt, um „nicht verfügbare Algorithmen"
  abzufangen, sieht einen anderen Typ; die Ursache (Provider fehlt) ist aus der Meldung nicht erkennbar.
  Behebung:
  1. Availability zur Laufzeit mit `EVP_CIPHER_fetch()` prüfen und dann `UnsupportedAlgorithmException` werfen.
  2. Optional ein Opt-in zum Laden des Legacy-Providers anbieten (`Crypt::loadLegacyProvider()`),
     dokumentieren, dass es `OSSL_PROVIDER_load()` global für den Prozess tut.
  3. `ERR_get_error()` in die Exception-Meldung einbauen (siehe nächster Befund).

- [ ] **OpenSSL-Fehlerqueue wird nie ausgewertet oder geleert** (`Crypt.cpp:441`, `459`, `478`, `482`, `564`, `582`, `601`, `605`) `(NEU)` `(verifiziert)`

  Im gesamten `src/` wird keine der Funktionen `ERR_get_error()`, `ERR_clear_error()`, `ERR_error_string()`
  verwendet (grep: keine Treffer). Die Exception-Meldungen enthalten nur den Funktionsnamen
  (`"EVP_EncryptFinal_ex"`), nicht die eigentliche Ursache. Zusätzlich bleiben Fehlermeldungen von
  fehlgeschlagenen OpenSSL-Aufrufen in der Thread-Queue stehen und werden von späteren, erfolgreichen
  Operationen wieder sichtbar (in den Probes mehrfach beobachtet, z. B. ein hängender
  `error:03000082:digital envelope routines::invalid key length` nach einer erfolgreichen Sequenz).
  Behebung: vor jedem OpenSSL-Aufruf `ERR_clear_error()`, nach einem Fehlschlag
  `ERR_get_error()`/`ERR_error_string_n()` in die Exception-Text aufnehmen.

- [ ] **Padding aus + Eingabe kein Blockvielfaches: bereits erzeugter Ciphertext wird verworfen** (`Crypt.cpp:465-486`, `588-609`) `(NEU)` `(verifiziert)`

  ```
  setPadding(false); encrypt("12345678901")
    -> OperationFailedException: EVP_EncryptFinal_ex   (out.size() == 0)
  ```
  `EVP_EncryptUpdate()` hat 16 Bytes Ciphertext erzeugt, `EVP_EncryptFinal_ex()` schlägt fehl
  (`error:1C80006B ... wrong final block length`), und weil das Ergebnis erst am Ende per `out.copy()`
  übergeben wird, ist die Teilmenge verloren. Für Blockmodi mit deaktiviertem Padding sollte die Klasse
  die Blockvielfach-Bedingung vorher prüfen und `InvalidParameterException`/`InvalidKeyLengthException`-artig
  ablehnen, statt mitten im Vorgang zu scheitern.
  Nebenbefund (verifiziert): `setPadding(false)` wird durch `setKey()`/`setIV()` **nicht** zurückgesetzt –
  die Vermutung des alten Reviews, das Re-Init über `EVP_EncryptInit_ex()` könnte das Padding reaktivieren,
  ist falsch (nach `setKey()` wirft derselbe Fall weiterhin).

- [ ] **`setIV()` bei ECB wird kommentarlos akzeptiert** (`Crypt.cpp:417-426`, `540-549`) `(NEU)` `(verifiziert)`

  ```
  Encrypt(Algo_AES_128, Mode_ECB): ivLength() = 0
  setIV(ByteArrayPtr(iv, 16))  -> kein Fehler, IV wird von OpenSSL ignoriert
  ```
  Kein Crash, aber eine stillschweigend ignorierte Eingabe. Empfehlung: bei `ivLength() == 0` und
  übergebenem IV eine Exception oder zumindest dokumentieren; umgekehrt bei CBC/CFB/OFB ein fehlendes
  IV erkennen (siehe nächster Punkt).

- [ ] **Ohne gesetzten Key liefert `update()` eine irreführende Exception** (`Crypt.cpp:428-445`) `(NEU)` `(verifiziert)`

  `Encrypt(Algo_AES_256, Mode_CBC)` ohne `setKey()` → `EVP_EncryptUpdate()` mit
  `error:1C800072 Provider routines::no key set`, pplib wirft `OperationFailedException("EVP_EncryptUpdate")`.
  Kein Silent-Zero-Key (gut), aber die Meldung ist für Aufrufer nicht nachvollziehbar. Empfehlung:
  Zustand „Key gesetzt" im Objekt tracken und `NotInitializedException("key")` werfen.

- [ ] **`static_cast<int>(in.size())` truncatiert bei großen Puffern** (`Crypt.cpp:438`, `475`, `561`, `598`) `(bekannt)`

  `ByteArray::size()` ist 32 Bit, OpenSSL erwartet `int`. Ab >2 GB (bzw. negativem `int`) ist die Länge
  falsch/negativ → UB. Zusätzlich ist `max_out = in.size() + 2*block_size` (`Crypt.cpp:471`, `594`)
  ohne Überlaufprüfung. Empfehlung: Größe vorab auf `INT_MAX` prüfen und mit klarer Exception ablehnen.

---
## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **`void* ctx` plus `friend` statt Typsicherheit und RAII** (`crypt.h:47-51`; `Crypt.cpp` ca. 30 `static_cast<EVP_CIPHER_CTX*>(ctx)`) `(NEU)`

  `void*` verliert die Typinformation, jeder Zugriff braucht einen Cast, und `friend class Encrypt/Decrypt`
  umgeht den Kapselungsgrund („warum ist `ctx` privat, wenn beide abgeleiteten Klassen vollen Zugriff brauchen?").
  Empfehlung: `EVP_CIPHER_CTX*` (oder `std::unique_ptr` mit Deleter, siehe kritischer Befund 2) als
  `protected`-Mitglied; `friend`-Deklarationen entfallen. Der Header braucht dann einen
  Vorwärtsdeklarations-Handle, damit OpenSSL nicht in den Header muss:
  ```cpp
  // crypt.h
  struct evp_cipher_ctx_st;              // OpenSSL-Forward-Decl.
  class Crypt {
  protected:
      evp_cipher_ctx_st* ctx;
  };
  ```

- [ ] **`Crypt` ist öffentlich vererbbar, direkt instanziierbar und hat einen nicht-virtuellen Destruktor** (`crypt.h:45-89`, `81-82`) `(bekannt)`

  `Crypt c;` ist kompilierbar, aber jedes Methodenaufruf wirft `NotInitializedException` – eine Sackgasse.
  `delete` über ein `Crypt*` auf einer `Encrypt`-Instanz ist UB (nicht-virtueller DTOR).
  Empfehlung: Konstruktor/Destruktor `protected`, DTOR `virtual`, oder (C++20, schlanker für Pico)
  DTOR `protected` + nicht-virtual und Vererbung explizit als beabsichtigt dokumentieren.

- [ ] **Enums sollten `enum class` sein; die Algorithmus-/Modus-Kombination ist nicht typgesichert** (`crypt.h:54-80`) `(NEU)`

  `Encrypt((Crypt::Algorithm)999, (Crypt::Mode)99)` ist gültiger C++ und wird erst zur Laufzeit erkannt
  (Tests `ConstructWithInvalidAlgorithmThrows`/`ConstructWithInvalidModeThrows`,
  `tests/src/crypto/crypto.cpp:317-330`, nutzen genau diese Casts). `enum class Mode`/`enum class Algorithm`
  verhindert unbeabsichtigte Implizitkonvertierungen; zusätzlich eine `constexpr`-Tabelle
  `{Algorithm, Mode, EVP_CIPHER*()}` statt des 250-zeiligen verschachtelten Switch.

- [ ] **`getCipher()` ist eine 250-zeilige Switch-Wüste mit doppelter `#ifdef`-Logik** (`Crypt.cpp:50-297`) `(NEU)`

  Pro Algorithmus ein `switch (mode)` mit vier Fällen, außen `#ifdef HAVE_EVP_*_ECB` plus
  `#ifndef OPENSSL_NO_*`. Fehleranfällig (Kopierfehler wären leicht möglich – geprüft, aktuell keine
  gefunden) und bläht den Text für eingebettete Builds auf. Empfehlung: tabellengesteuert:
  ```cpp
  struct CipherEntry { Crypt::Algorithm algo; Crypt::Mode mode; const EVP_CIPHER* (*fn)(); };
  static const CipherEntry table[] = { ... };   // im Debug-Build per #ifdef gefiltert
  ```
  Das spart Code, ist data-driven testbar und erlaubt die im AEAD-Abschnitt beschriebene Erweiterung
  um GCM/ChaCha20-Poly1305 ohne erneuten Switch-Ausbau.

- [ ] **Doppelte Kopie/Allokation pro `update()`/`final()`/`encrypt()`** (`Crypt.cpp:435-436` + `443`, `454-455` + `461`, `472-473` + `484`, analog Decrypt) `(NEU)`

  Jedes `update()` allokiert ein temporäres `ByteArray` und kopiert dann per `out.copy()` in das Ziel –
  pro Chunk also eine Heap-Allokation plus zwei Kopien. Für die Pico-/Mikrocontroller-Ziele
  (REFACTORING.md) relevant. Empfehlung: direkt in `out` verschlüsseln (`out.malloc(max_out)` →
  `EVP_*Update(out.ptr(), ...)`) und die Größe über `out.resize()`/`shrink` anpassen; oder eine
  Puffer-Selbstverwaltungsvariante `update(const ByteArrayPtr&, unsigned char* out, size_t outCap, size_t& written)`.

- [ ] **Kein Zeroing sensibler Zwischenpuffer** (`Crypt.cpp:435`, `454`, `472`, `558`, `577`, `595`) `(bekannt)`

  `temp` enthält Klartext-/Ciphertext-Reste und Schlüsselmaterial-Spuren und wird nur freigegeben.
  Empfehlung: vor der Freigabe `explicit_bzero()`/`OPENSSL_cleanse()` (plattformabhängig, siehe
  `REFACTORING.md`-Kompatibilitätsziele) und im Header dokumentieren, dass der Aufrufer für das
  Key-/IV-`ByteArray` selbst säubern muss.

- [ ] **Keine Aussage zur Thread-Sicherheit / Reentrancy** (`crypt.h:45-115`) `(NEU)`

  Ein `EVP_CIPHER_CTX` ist nicht thread-sicher; die Klasse bietet keinen Schutz. Da `Random` explizit
  „Thread local" dokumentiert (`include/pplib/core/random.h:60`), sollte auch hier stehen, ob ein Objekt
  pro Thread verwendet werden muss. Ebenso ist die OpenSSL-Fehlerqueue thread-lokal – relevant, sobald
  `ERR_*` ausgewertet wird.

- [ ] **`NoAlgorithmSpecifiedException` ist toter Code** (`crypt.h:42`) `(NEU)`

  Die Exception wird nirgends geworfen (es gibt keinen default-Constructor für `Encrypt`/`Decrypt`).
  Ebenso ist `InvalidAlgorithmException` ungenutzt – `getCipher()` wirft `UnsupportedAlgorithmException`.
  Empfehlung: entfernen oder tatsächlich einsetzen (z. B. für `setAlgorithm()` mit ungültigen Werten).

- [ ] **Schwache/veraltete Cipher sind ohne jede Kennzeichnung anwählbar** (`crypt.h:62-80`, `Crypt.cpp:139-292`) `(bekannt)`

  DES (56 Bit), IDEA (64 Bit), RC2, Blowfish, CAST5 und der ECB-Modus stehen gleichberechtigt neben AES.
  Empfehlung: `[[deprecated]]`-Marker auf die Enum-Werte bzw. eine Build-Option
  `PPLIB_WITH_LEGACY_CIPHERS` (OFF per Default), damit Embedded-Builds die Tabellen verkleinern und
  Anwender gewarnt werden.

---
## Dokumentation & Doxygen

- [ ] **`crypt.h` enthält keinerlei Doxygen-Kommentare** (`crypt.h:30-119`) `(NEU)` `(verifiziert)`

  Der Header hat 0 Treffer für `@class`/`@brief`/`@param`/`@return` – zum Vergleich:
  `include/pplib/types/bytearrayptr.h:41` nutzt `/**@class ByteArrayPtr`. Für IntelliSense/Quickinfo
  bedeutet das: bei `enc.setKey(...)` oder `enc.encrypt(...)` erscheint nur die Signatur, ohne jede
  Aussage über Semantik, Ausnahmen oder Zustandsabhängigkeit. Bei einer Krypto-API ist das der
  kritischste Punkt, weil die gefährlichsten Fehler (IV-Wiederverwendung, Objekt-Reuse, fehlende
  Authentizität) reine Semantik-Fragen sind.
  Empfehlung: Klassen- und Methoden-Doku ergänzen; Pflichtaussagen:
  - `Crypt`: Thread-Safety, Lebensdauer des `ctx`, dass ECB/DES/RC2/… unsicher sind.
  - `setKey`/`setIV`: erwartete Länge (`keyLength()`/`ivLength()`), dass IV bei CBC/CFB/OFB
    für jede Nachricht neu und unvorhersehbar sein muss, dass bei ECB kein IV existiert.
  - `update`/`final`: Zustandsabhängigkeit, dass `out` ersetzt (nicht angehängt) wird,
    dass `final()` zwingend aufgerufen werden muss.
  - `encrypt`/`decrypt`: Single-Shot-Semantik, dass kein Authentizitätsschutz enthalten ist.
  - Alle Methoden: `@throws`-Liste (`NotInitializedException`, `InitializationFailedException`,
    `OperationFailedException`, `InvalidKeyLengthException`, `OutOfMemoryException`).

- [ ] **Header ist nur zufällig eigenständig kompilierbar (fehlende Vorwärtsdeklarationen)** (`crypt.h:33-35`, `96-101`, `109-114`) `(NEU)` `(verifiziert)`

  `crypt.h` nutzt `ByteArrayPtr`/`ByteArray`, inkludiert aber weder `bytearrayptr.h`/`bytearray.h` noch
  eine eigene Vorwärtsdeklaration. Es kompiliert nur, weil `baseexception.h:38` → `pplib/types/string.h:53`
  zufällig `class ByteArrayPtr;` deklariert. `digest.h:42-43` macht es richtig (eigene Forward-Decl.).
  Behebung: in `crypt.h` direkt
  ```cpp
  class ByteArrayPtr;
  class ByteArray;
  ```
  ergänzen. (Kein Compile-Fehler heute, aber eine stille Abhängigkeit von der Include-Kette einer
  Exception-Basisklasse – bricht, sobald `baseexception.h` aufgeräumt wird.)

- [ ] **`setKeyLength()` ist Reihenfolge-abhängig, das ist undokumentiert** (`crypt.h:88`; `Crypt.cpp:361-370`) `(NEU)` `(verifiziert)`

  Für Cipher mit variabler Schlüssellänge muss `setKeyLength()` vor `setKey()` aufgerufen werden;
  OpenSSL übernimmt beim `EVP_*Init_ex(..., key, ...)` die Länge des Contexts. Auf AES (feste Länge) ist
  jeder andere Wert ein Fehler:
  ```
  AES-256-CBC: setKeyLength(16) -> ret=0 -> InvalidKeyLengthException
  Blowfish   : setKeyLength(56) ok, danach setKey(56 Bytes) -> key_len=56, Verschluesselung funktioniert
  ```
  Empfehlung: Reihenfolge + erlaubte Bereiche im Header dokumentieren (oder `setKey()` die Länge aus
  `key.size()` ableiten und `setKeyLength()` überflüssig machen – sauberere API).

- [ ] **`setPadding()`-Verhalten und Auswirkung auf Blockvielfach-Anforderung sind undokumentiert** (`crypt.h:87`; `Crypt.cpp:351-359`) `(NEU)`

  Ebenso undokumentiert: dass `EVP_CIPHER_CTX_set_padding()` nur das *letzte* Block-Padding betrifft,
  dass `ivLength()` bei ECB 0 ist, und dass `blockSize()` bei CFB/OFB **1** meldet (OpenSSL meldet für
  AES-256-CFB/OFB `block_size=1`, für CBC 16 – die Puffermathematik von pplib ist dadurch korrekt,
  aber für Leser überraschend).

- [ ] **Rückgabewert von `EVP_CIPHER_CTX_set_padding()` wird ignoriert** (`Crypt.cpp:357`) `(NEU)`

  Aktuell unkritisch (die Funktion liefert immer 1), aber konsistent zum Rest der Datei wäre eine Prüfung
  mit Exception – oder ein Kommentar, warum sie entfällt.

---
## Klassenstruktur: Basisklasse behalten, oder Encrypt/Decrypt zusammenlegen?

**Kurzfassung: Die Aufteilung `Crypt` (Basis) + `Encrypt` + `Decrypt` ist richtig – nicht zusammenlegen,
und die Basisklasse nicht streichen. Was geändert werden muss, ist der *Zustand* in der Basisklasse
(RAII, Copy/Move, `reset()`), nicht die Vererbungsstruktur.**

Bewertung der Ist-Struktur:

| Aspekt | Bewertung |
|---|---|
| Basisklasse `Crypt` für Schlüssellänge/IV-Länge/Blockgröße/Padding | sinnvoll – diese fünf Methoden sind für beide Richtungen identisch und machen ~40 % der API aus |
| Trennung `Encrypt` / `Decrypt` | sinnvoll – verhindert, dass Ver- und Entschlüsselung verwechselt werden, und erlaubt richtungsspezifische Methoden |
| `ctx` `private` + `friend` | schlecht – Kapselung wird nur formal eingehalten; `protected` + typisierter Zeiger ist ehrlicher |
| Kein Copy/Move-Schutz, nicht-virtueller DTOR | muss geändert werden (siehe kritische Befunde) |
| `Crypt` öffentlich instanziierbar | entfernen (`protected`-CTor) |

Warum `Encrypt` und `Decrypt` **nicht** in eine Klasse gehören:

1. **Typsicherheit.** `Encrypt e; e.decrypt(...)` wäre zur Compile-Zeit erlaubt und ist ein klassischer
   Anwendungsfehler (verschlüsseltes Material nochmal verschlüsseln). Getrennte Type machen das unmöglich.
2. **AEAD macht die Richtungen asymmetrisch.** Sobald GCM/ChaCha20-Poly1305 dazukommen (siehe nächstes
   Kapitel), braucht `Encrypt` ein `getTag()` und `Decrypt` ein `setTag()`. In einer gemeinsamen Klasse
   entstünden Methoden, die in der jeweils anderen Richtung einen Laufzeitfehler werfen müssen –
   genau das, was man mit Typen vermeiden will.
3. **Ressourcen.** Eine gemeinsame Klasse müsste zwei Zustände (Richtung) pro Objekt tragen; für Pico-Builds
   ist die jetzige Aufteilung kleiner.

Warum die Basisklasse **nicht** gestrichen werden sollte:

- Die geteilten Parameter (`keyLength()`, `ivLength()`, `blockSize()`, `setPadding()`, `setKeyLength()`)
  gehören semantisch zum Cipher, nicht zur Richtung. Verdopplung wäre Copy-Paste mit 250 Zeilen
  `getCipher()`-Wiederholung.
- Wer polymorph arbeiten will (z. B. „Algorithmus aus Konfiguration"), braucht ohnehin einen gemeinsamen
  Basistyp. Der fehlt dann.

Konkreter Zielvorschlag (C++20, minimaler Bruch mit der bestehenden API):

```cpp
class Crypt
{
protected:
    explicit Crypt(Crypt::Algorithm algo, Crypt::Mode mode);   // kein Default-CTor mehr
    void setDirection(bool encrypting);                        // intern EVP_*Init_ex-Auswahl
    evp_cipher_ctx_st* ctx;                                    // typisiert, protected
public:
    Crypt(const Crypt&) = delete;
    Crypt& operator=(const Crypt&) = delete;
    Crypt(Crypt&&) noexcept;
    Crypt& operator=(Crypt&&) noexcept;
    virtual ~Crypt();                                          // polymorph nutzbar

    int keyLength() const;
    int maxKeyLength() const;                                  // echte Algorithmus-Obergrenze
    int ivLength() const;
    int blockSize() const;
    void setPadding(bool enabled);
    void setKeyLength(int keylen);
    void reset();                                              // wie Digest::reset()
};

class Encrypt : public Crypt { public: /* setKey/setIV/update/final/encrypt + getTag */ };
class Decrypt : public Crypt { public: /* setKey/setIV/update/final/decrypt + setTag */ };
```

Zusätzlich empfohlen, passend zu deinem Stil in `Digest`: statische One-Shot-Helfer, damit der
Normalfall ohne Objekt-Zustand auskommt (und der Reuse-Bug erst gar nicht auftreten kann):

```cpp
static ByteArray seal(Algorithm a, Mode m, const ByteArrayPtr& key,
                      const ByteArrayPtr& iv, const ByteArrayPtr& plain);
static ByteArray open(Algorithm a, Mode m, const ByteArrayPtr& key,
                      const ByteArrayPtr& iv, const ByteArrayPtr& cipher);
```

Das spiegelt exakt das Muster aus `include/pplib/crypto/digest.h:86-95`
(`Digest::hash()`, `Digest::sha256()` …) und hält die Streaming-Objekte dem Spezialfall vorbehalten.
Da `Encrypt`/`Decrypt` aktuell nur in `tests/src/crypto/crypto.cpp` verwendet werden (grep über `src/`,
`tests/src/`, `include/`), ist ein API-Bruch intern praktisch kostenlos.

---
## Moderne Algorithmen ergänzen (AEAD gemäß `REFACTORING.md:95-102`)

### Ausgangslage (verifiziert)

Alle vier `Mode`s sind unauthentifiziert. Ein Angreifer kann Ciphertext unbemerkt verändern
(Bit-Flipping bei CBC/CFB/OFB, Blockvertauschung bei ECB). `DecryptWithCorruptedCiphertextThrows`
(`tests/src/crypto/crypto.cpp:411-428`) funktioniert nur, weil das Padding zufällig kaputt geht –
eine Garantie gibt es nicht.

Die benötigten OpenSSL-3.6-Funktionen sind alle vorhanden und wurden in Probes durchgetestet:

| Eigenschaft | AES-256-GCM | ChaCha20-Poly1305 |
|---|---|---|
| `ivLength()` | 12 | 12 (OpenSSL 3.x; OpenSSL 1.1 meldete 16) |
| `keyLength()` | 32 | 32 |
| `blockSize()` | 1 (Stromverschlüsselung) | 1 |
| `tagLength()` des Contexts nach Init | 16 | **0** – Tag-Größe muss vor dem Verschlüsseln gesetzt werden |
| AAD | `EVP_EncryptUpdate(ctx, NULL, &len, aad, aadlen)` | identisch |
| Tag holen | `EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, 16, tag)` | identisch |
| Tag setzen (Entschlüsselung) | `EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, 16, tag)` **vor** `DecryptFinal_ex` | identisch |
| Tag-Größe ändern | `EVP_CTRL_AEAD_SET_TAG` mit `(len, NULL)` vor der Verschlüsselung | Pflicht |
| Verifikation | `EVP_DecryptFinal_ex()` liefert 0 bei falschem Tag (gemessen) | identisch |

Zwei Fallstricke, die in den Probes aufgefallen sind und direkt in die Implementierung gehören:
- `EVP_CIPHER_tag_length()` existiert in OpenSSL 3.6 **nicht** als Header-Symbol – die
  Context-Variante `EVP_CIPHER_CTX_get_tag_length()` verwenden.
- ChaCha20-Poly1305 startet mit `tag_length == 0`; wird die Tag-Größe nicht vor dem ersten `update()`
  gesetzt, liefert `EVP_CTRL_AEAD_GET_TAG` nichts Sinnvolles.

### API-Erweiterung (Vorschlag, abwärtskompatibel)

```cpp
// crypt.h
enum class Mode { ECB, CBC, CFB, OFB, CTR, GCM };                 // CTR + GCM als Modi
enum class Algorithm {
    Algo_AES_128, /* ... bestehende ... */,
    Algo_CHACHA20_POLY1305,        // eigener Algorithmus, Mode wird ignoriert
};

class Crypt {
public:
    int  tagLength() const;                       // EVP_CIPHER_CTX_get_tag_length()
    void setTagLength(int len);                   // EVP_CTRL_AEAD_SET_TAG(ctx, len, NULL)
    void setNonceLength(int len);                 // EVP_CTRL_AEAD_SET_IVLEN  (GCM: 12 empfohlen)
    bool isAEAD() const;                          // fuer Modus-/Algorithmus-Weichen
};

class Encrypt : public Crypt {
public:
    void setAAD(const ByteArrayPtr& aad);         // vor dem ersten update()
    ByteArray getTag();                           // nach final()
    // One-Shot mit eingebettetem Tag:
    ByteArray seal(const ByteArrayPtr& plain, const ByteArrayPtr& aad = {});
};

class Decrypt : public Crypt {
public:
    void setAAD(const ByteArrayPtr& aad);         // vor dem ersten update()
    void setTag(const ByteArrayPtr& tag);         // vor final()
    ByteArray open(const ByteArrayPtr& cipher, const ByteArrayPtr& aad = {});
};
```

Reihenfolge, die die Implementierung einhalten muss (GCM, verifiziert):

```
setAlgorithm() -> setNonceLength()/setTagLength() -> setKeyLength() (optional)
              -> setKey() -> setIV() -> setAAD() -> update()* -> final() -> getTag()
```

Kernstück in `getCipher()` (tabellengesteuert, s. o.):

```cpp
case Crypt::Algo_AES_256:
    switch (mode) {
        ...
        case Crypt::Mode_GCM:
            return EVP_aes_256_gcm();
    }
    break;
case Crypt::Algo_CHACHA20_POLY1305:
    return EVP_chacha20_poly1305();
```

Und in `Encrypt::final()`/`Decrypt::final()` die AEAD-Äste:

```cpp
// Encrypt::final(): AEAD erzeugt kein Padding, aber ein Tag
if (isAEAD()) {
    if (EVP_CIPHER_CTX_ctrl(static_cast<EVP_CIPHER_CTX*>(ctx),
                            EVP_CTRL_AEAD_GET_TAG, tagLen, tagBuf) != 1) {
        throw OperationFailedException("EVP_CTRL_AEAD_GET_TAG");
    }
} else if (!EVP_EncryptFinal_ex(...)) { ... }

// Decrypt::final(): Tag vor final setzen; Final prueft konstantzeitlich
if (isAEAD()) {
    if (EVP_CIPHER_CTX_ctrl(static_cast<EVP_CIPHER_CTX*>(ctx),
                            EVP_CTRL_AEAD_SET_TAG, tagLen, tagBuf) != 1) {
        throw OperationFailedException("EVP_CTRL_AEAD_SET_TAG");
    }
    if (!EVP_DecryptFinal_ex(...)) throw AuthenticationFailedException("AEAD tag mismatch");
}
```

Wichtig: **Tag niemals selbst mit `memcmp()` vergleichen.** `EVP_DecryptFinal_ex()` vergleicht
konstantzeitlich; ein eigener Vergleich ist ein Timing-Oracle. Ein eigener Exception-Typ
(`AuthenticationFailedException`) ist sinnvoll, damit Aufrufer Integritätsfehler von echten
Bedienfehlern unterscheiden können.
### Sichere Zufallszahlen (`REFACTORING.md:99`)

`include/pplib/core/random.h` bietet nur `seed(uint64_t)` + `get()`/`bytes()` – ein deterministischer,
nicht-kryptografischer Generator. Für IV/Nonce/Key ist er unbrauchbar (vorhersagbare IVs brechen CBC,
wiederholte Nonces brechen GCM komplett). Ergänzen:

```cpp
class Random {
public:
    static ByteArray secureBytes(size_t numBytes);      // RAND_bytes() aus libcrypto
    static bool secureFill(ByteArray& buffer);          // RAND_bytes()
};
```

`RAND_bytes()` ist in OpenSSL 3.6 immer verfügbar (kein Provider-Problem, CSPRNG ist im Default-Provider),
liefert 0 bei Fehler → als Exception melden. Für Pico-Builds ohne OpenSSL: Kompilat unter
`#ifdef HAVE_OPENSSL` halten und sonst `UnsupportedFeatureException("OpenSSL")` – konsistent zum
Bestandsmuster in `Crypt.cpp`.

**Nicht-Verhandlungssache für GCM/ChaCha20-Poly1305:** Nonce + Key-Paar darf nie wiederverwendet werden.
Bei GCM führt Nonce-Wiederverwendung zum Keystream-Reuse **und** zur Rekonstruktion des GHASH-Auth-Keys
(damit Totalverlust der Authentizität). Empfehlung: `seal()` erzeugt die 12-Byte-Nonce selbst via
`RAND_bytes()` und stellt sie vor den Ciphertext; `open()` liest sie aus dem Container. Damit ist der
häufigste AEAD-Anwendungsfehler API-seitig ausgeschlossen.

### Container-Format für One-Shot-Ausgabe

Ohne Format weiß `open()` nicht, wo Nonce/Tag stehen. Vorschlag (klein, endian-definiert,
pico-tauglich, 12 Byte Overhead plus 16 Byte Tag):

```
magic "PPLC" (4) | version uint8 (1) | algorithm uint8 (1) | mode uint8 (1) | flags uint8 (1)
nonce_len uint8 (1) | nonce (nonce_len) | ciphertext (rest - tag_len) | tag (16)
```

Der `version`-Byte ist der Punkt, an dem du später DES/RC2 entfernen kannst, ohne alte Dateien
fehlzudeuten: beim Lesen wird geprüft, ob der kodierte Algorithmus noch erlaubt ist, und andernfalls
eine klare `UnsupportedAlgorithmException` statt stiller Fehlentschlüsselung geworfen.

### Tests, die dafür nötig sind

1. **Known-Answer-Tests** (Pflicht, sonst ist die Implementierung unbewiesen):
   - ChaCha20-Poly1305: RFC 7539 §2.8.2 (Key/Nonce/Plaintext → Ciphertext + Tag als Konstanten).
   - AES-GCM: NIST-SP-800-38D/CAVP-Vektoren, inkl. AAD-Fall und 8/12/16-Byte-Tags.
2. **Negativtests:** manipuliertes Ciphertext-Byte, falsches Tag, manipuliertes AAD, falsche Nonce-Länge,
   `getTag()` vor `final()`, `setTag()` nach `final()` – jeweils mit erwarteter Exception.
3. **Regressions-Test für den Reuse-Bug:** zweimal `encrypt()` auf demselben Objekt muss identischen
   Ciphertext liefern (nach Fix) bzw. ein Round-Trip muss gelingen (heute fehlerhaft).
4. **Streaming-AEAD:** `update()` in mehreren Chunks + `final()` + `getTag()` → Round-Trip über
   `update()`/`setTag()`/`final()`.
5. Bestehende Tests behalten, aber `CryptoTest.MaxKeyLength` (`crypto.cpp:354-358`) an die korrigierte
   Semantik von `maxKeyLength()` anpassen.

### Migrations-/Aufräumplan (Realisierung in dieser Reihenfolge)

1. **Sicherheitsbugs zuerst** (kritische Befunde 1–3): Context-Reset, Copy/Move-Löschung,
   Key-/IV-Längenprüfung. Das sind kleine, testbare Patches ohne API-Bruch.
2. **Doku im Header** (Doxygen) – kostet nichts, verhindert die häufigsten Anwenderfehler sofort.
3. **AEAD:** `Mode_GCM` + `Algo_CHACHA20_POLY1305`, `setAAD`/`setTagLength`/`getTag`/`setTag`,
   `Random::secureBytes()`, `seal()`/`open()` mit Container-Format, KAT-Tests.
4. **Legacy bereinigen:** `check_symbol_exists` durch Laufzeit-Check (`EVP_CIPHER_fetch()`) ersetzen,
   `PPLIB_WITH_LEGACY_CIPHERS` einführen, DES/IDEA/RC2/RC5/Blowfish/CAST5 und `Mode_ECB` kennzeichnen
   oder entfernen.
5. **Effizienz:** tabellengesteuerte Cipher-Auswahl, Verschlüsseln direkt in `out` statt
   Temp-Buffer + `copy()`.

Aufwand grob: Schritt 1 ≈ 1 Tag, Schritt 2 ≈ 0,5 Tag, Schritt 3 ≈ 2–3 Tage inkl. Tests,
Schritt 4 ≈ 1 Tag, Schritt 5 ≈ 1 Tag.

---
## Befunde in Drittdateien

- [ ] **`check_symbol_exists()` prüft Verfügbarkeit, nicht Nutzbarkeit** (`CMakeLists.txt:296-311`)
  Die `HAVE_EVP_*`-Defines werden allein über die Symbol-Existenz in libcrypto gesetzt. In OpenSSL 3.x
  sind die Legacy-Chiffren zwar einkompiliert, aber ohne geladenen Legacy-Provider nicht nutzbar.
  Folge: `config_pplib.h:58-73` verspricht `HAVE_EVP_DES_ECB/BF_ECB/IDEA_ECB/RC2_ECB/CAST5_ECB = 1`,
  die Laufzeit wirft trotzdem `InitializationFailedException`. Richtige Prüfung:
  `EVP_CIPHER_fetch(NULL, "des-ecb", NULL) != nullptr` (Laufzeit) oder zumindest Doku, dass Legacy
  einen Provider-Load erfordert.

- [ ] **Test-Suite prüft Objekt-Lebenszyklus und Wiederverwendung nicht** (`tests/src/crypto/crypto.cpp:283-315`)
  Jeder Algorithmus wird genau einmal mit frischem Objekt getestet; es gibt keinen Test für zwei
  `encrypt()`-Aufrufe auf demselben Objekt, für Streaming mit anhängendem Output, oder für
  `update()` nach `final()`. Genau diese Lücken haben die drei kritischen Befunde verdeckt.

- [ ] **Statischer Key/IV für alle Tests** (`tests/src/crypto/crypto.cpp:64-66`)
  `static const unsigned char key[32]`/`iv[16]` sind in Ordnung für Determinismus, schließen aber
  IV-Herkunftsfragen komplett aus. Zusätzlich ein Test mit `RAND_bytes()`-IV und Round-Trip.

- [ ] **`CryptoTest.MaxKeyLength` zementiert die falsche Semantik** (`tests/src/crypto/crypto.cpp:354-358`)
  `ASSERT_EQ(64u, enc.maxKeyLength())` für AES-128. Bei Korrektur des Befunds muss der Test angepasst
  werden – sonst sichert er dauerhaft die OpenSSL-Maximal-Konstante statt der Algorithmus-Grenze.

- [ ] **`Random` ist kein CSPRNG** (`include/pplib/core/random.h:49-141`)
  Kein `RAND_bytes()`/`EVP_RAND` im gesamten Projekt. Für Crypto-Schlüssel/IVs ungeeignet;
  siehe `REFACTORING.md:99`.

- [ ] **`Digest` zeigt dasselbe RAII-Muster** (`include/pplib/crypto/digest.h:47-51`)
  Rohe `void* ctx` ohne Copy-Schutz; ist in `codereview/todo/digest-review.md` bereits als
  Rule-of-Three-Verletzung dokumentiert. Beide Klassen sollten identisch fixes bekommen, damit das
  Muster im Projekt einheitlich wird.

- [ ] **`ByteArray::copy()` ersetzt den Zielinhalt** (`src/types/ByteArray.cpp:106-137`)
  Verhalten ist korrekt implementiert, aber für Streaming-Ausgabe irreführend (`append()` existiert
  in Zeile 152). In `bytearray.h` explizit dokumentieren, damit Aufrufer nicht versehentlich
  Chunks überschreiben.

- [ ] **`class ByteArrayPtr;` in `include/pplib/types/string.h:53`**
  Die Forward-Deklaration, von der `crypt.h` unbewusst profitiert, gehört in den ByteArray-Header
  (`digest.h:42-43` macht es vor). Sonst bleibt `crypt.h` nur durch eine Inklusionskette kompilierfähig.

---

## Überprüft und NICHT bestätigt (keine Maßnahme)

Folgende Punkte wurden aktiv geprüft und sind **in Ordnung** – sie stehen hier, damit sie nicht
erneut als Verdacht auftauchen:

- **Padding-Status überlebt `setKey()`:** die Vermutung aus `crypt-review.md`, `setKey()` setze das
  Padding zurück, ist falsch. `EVP_CIPHER_CTX_reset()` wird in `Crypt.cpp` nirgends aufgerufen;
  gemessenes Verhalten: `setPadding(false)` bleibt nach `setKey()` wirksam.
- **Puffergröße in `final()`:** `block_size` als `max_out` ist für Blockchiffren ausreichend
  (`EVP_EncryptFinal_ex` schreibt maximal eine Blocklänge). Für AEAD/stromartige Modi liefert OpenSSL
  `block_size = 1`; dort wird `max_out` ohnehin ignoriert. Kein Überlauf möglich.
- **CFB/OFB-Puffermathematik:** `in.size() + block_size` ist für diese Modi korrekt dimensioniert
  (OpenSSL meldet für AES-CFB/OFB `block_size = 1`, `update()` schreibt exakt `in.size()` Bytes).
- **`ByteArray::malloc()` liefert nie `nullptr`:** `src/types/ByteArray.cpp:50-53` wirft
  `OutOfMemoryException`. Ein nicht allokiertes Output-Buffer in `update()`/`final()` ist damit kein
  Risiko (nur die `size == 0`-Grenze wäre relevant, s. o.).
- **Leerer Input:** `encrypt()` mit leerem ByteArray funktioniert in allen Modi (Round-Trip OK).
- **`getCipher()` (`Crypt.cpp:50-297`):** alle 13 Algorithmus-Gruppen wurden Zeile für Zeile geprüft;
  keine Copy-Paste-Fehler (die AES-Zweige nutzen durchgängig `EVP_aes_XXX_XXX`, ARIA/CAMELLIA korrekt
  nach Schlüssellänge sortiert), `#ifdef`-Guards passen zu den Defines, `return nullptr` am Ende ist
  erreichbar und wird von den Aufrufern behandelt.
- **`setAlgorithm()`-Fehlerzustand ist reparierbar:** schlägt `EVP_CipherInit_ex()` fehl (z. B.
  Legacy-Provider), bleibt der Context unverändert; ein erneuter `setAlgorithm()` mit verfügbarem
  Algorithmus funktioniert danach.
- **`check_symbol_exists`-Prüfung in `CMakeLists.txt` ist nicht schuld an `RC5`:** `RC5_32_12_16` ist
  in OpenSSL 3.6 tatsächlich nicht enthalten (`HAVE_EVP_RC5_32_12_16_ECB` undef) – der
  `UnsupportedAlgorithmException` dafür ist korrekt.
