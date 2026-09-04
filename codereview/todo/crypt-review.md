# Crypt / Encrypt / Decrypt Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/crypto/crypt.h` + `src/crypto/Crypt.cpp` (Klassen `Crypt`, `Encrypt`, `Decrypt`).
Exceptions gegen `include/pplib/exceptions.h` und `crypt.h` selbst verifiziert (alle verwendeten Exceptions existieren real:
`UnsupportedAlgorithmException`/`InvalidKeyLengthException` in `crypt.h`, `NotInitializedException`/`InitializationFailedException`/
`OperationFailedException`/`UnsupportedFeatureException` in `exceptions.h`). Verwendung geprüft in `tests/src/crypto/crypto.cpp`
(alle Algorithmen/Modi, Streaming-API, Fehlerpfade). `REFACTORING.md` (Abschnitt "Crypto-Funktionen") enthält bereits den
Hinweis, dass AEAD-Modi (GCM/ChaCha20-Poly1305) fehlen – das wird unten referenziert, nicht neu erfunden.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **Rule-of-Three-Verletzung: Copie einer `Encrypt`/`Decrypt`-Instanz dupliziert den rohen `ctx`-Zeiger → Double-Free/Use-after-Free** (`crypt.h:45-89`, `Crypt.cpp:305-310`)
  `Crypt` hat einen benutzerdefinierten Destruktor (`~Crypt()` gibt `ctx` via `EVP_CIPHER_CTX_free()` frei), aber weder Copy-Konstruktor
  noch Copy-Zuweisungsoperator sind deklariert oder verboten. Der Compiler generiert daher stillschweigend eine flache Kopie, die
  denselben `ctx`-Zeiger übernimmt:
  ```cpp
  pplib::Encrypt a(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
  pplib::Encrypt b = a;      // flache Kopie: b.ctx == a.ctx
  // a und b werden am Ende des Scopes beide zerstört
  // -> EVP_CIPHER_CTX_free(ctx) wird zweimal auf denselben Pointer aufgerufen (Double-Free)
  ```
  Verschärfend: weil `Crypt` einen benutzerdefinierten Destruktor hat, unterdrückt das die implizite Erzeugung von
  Move-Konstruktor/-Zuweisung (C++11-Regel) – jede vermeintliche "Move"-Operation (z.B. `std::vector<Encrypt>`-Reallokation,
  Rückgabe eines lokalen `Encrypt`-Objekts ohne garantierte Kopie-Elision, `std::swap`) fällt daher ebenfalls auf die kaputte
  Kopie zurück. Aktuell wird im Repo nirgends eine `Encrypt`/`Decrypt`-Instanz kopiert (nur im Testcode direkt konstruiert),
  d.h. der Bug ist heute nicht aktiv, aber jeder künftige Aufrufer, der z.B. `std::vector<Encrypt>` befüllt, tappt sofort hinein.
  Fix: Kopieren explizit verbieten und stattdessen Move unterstützen:
  ```cpp
  Crypt(const Crypt&) = delete;
  Crypt& operator=(const Crypt&) = delete;
  Crypt(Crypt&& other) noexcept : ctx(other.ctx) { other.ctx = nullptr; }
  Crypt& operator=(Crypt&& other) noexcept {
      if (this != &other) { if (ctx) EVP_CIPHER_CTX_free(...); ctx = other.ctx; other.ctx = nullptr; }
      return *this;
  }
  ```

- [ ] **`setKey()`/`setIV()` prüfen die Puffergröße nicht → Heap-Out-of-Bounds-Read** (`Crypt.cpp:405-426, 528-548`)
  ```cpp
  void Encrypt::setKey(const ByteArrayPtr& key)
  {
      if (!ctx) throw pplib::NotInitializedException();
      int ret = EVP_EncryptInit_ex(ctx, nullptr, nullptr, static_cast<const unsigned char*>(key.ptr()), nullptr);
      ...
  ```
  Es wird nirgends geprüft, ob `key.size()` zur laut Cipher konfigurierten Schlüssellänge (`EVP_CIPHER_CTX_key_length`)
  passt. `EVP_EncryptInit_ex` liest intern immer genau `keyLength()` Bytes ab dem übergebenen Zeiger – unabhängig davon,
  wie viele Bytes das übergebene `ByteArrayPtr` tatsächlich referenziert. Szenario:
  ```cpp
  pplib::Encrypt enc(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC); // erwartet 32-Byte-Key
  unsigned char shortKey[16] = { ... };
  enc.setKey(pplib::ByteArrayPtr(shortKey, 16));   // kein Fehler, aber:
  // EVP_EncryptInit_ex liest 32 Bytes ab shortKey -> liest 16 Bytes ueber den Puffer hinaus
  ```
  Das liest fremden Heap-Speicher in den Schlüsselplan ein (Information Leak in die Verschlüsselung hinein bzw.
  undefiniertes Verhalten/Absturz, je nach Heap-Layout). Exakt dasselbe Muster bei `setIV()` gegen `ivLength()`.
  Fix: Länge vor dem EVP-Aufruf verifizieren, z.B.
  ```cpp
  if ((int)key.size() != keyLength()) throw InvalidKeyLengthException("%d", (int)key.size());
  ```
  (für `setIV()` analog, z.B. mit `IllegalArgumentException` aus `exceptions.h`, da `InvalidKeyLengthException` dem Namen
  nach für Schlüssel reserviert ist).

## Bugs (mittel)

- [ ] **`static_cast<int>(in.size())` truncatiert bei sehr großen Puffern** (`Crypt.cpp:438, 475, 561, 598` u.a.)
  `update()`, `encrypt()` und `decrypt()` casten `size_t in.size()` ungeprüft nach `int`, bevor sie es an
  `EVP_EncryptUpdate`/`EVP_DecryptUpdate` übergeben. Bei einem Einzelaufruf mit einem Puffer > `INT_MAX` Bytes (~2 GiB)
  wird die Länge falsch (ggf. negativ) interpretiert → falsche/verkürzte Ver-/Entschlüsselung oder Absturz in OpenSSL.
  `Digest::addData(FileObject&)` löst dasselbe Problem korrekt durch Chunking in ≤1-MiB-Blöcken (siehe `digest-review.md`),
  `Encrypt`/`Decrypt` haben dagegen keinerlei Chunking für die Single-Shot-Methoden.
  Fix: entweder vor dem Cast `if (in.size() > (size_t)INT_MAX) throw ...`, oder intern in Blöcken über `update()` verarbeiten.

## Design

**Kryptografische Befunde:**

- [ ] **Kein AEAD, keine Authentifizierung – reines Confidentiality-only-API** (`crypt.h:54-60`, `getCipher()` in `Crypt.cpp`)
  `Crypt::Mode` bietet nur ECB/CBC/CFB/OFB an, alles unauthentifizierte Modi. Es gibt keinen HMAC-Support in dieser Klasse
  (siehe auch `digest-review.md` zum `HMAC`-Draft in `pplib-crypto.h`, der nicht implementiert ist) und keine AEAD-Modi
  (GCM/ChaCha20-Poly1305). Ein Aufrufer, der `Encrypt`/`Decrypt` "wie empfohlen" nutzt, bekommt Vertraulichkeit, aber keine
  Integritäts-/Authentizitätsgarantie – ein Angreifer kann Ciphertext-Bits gezielt manipulieren (Bit-Flipping bei
  CFB/OFB/CTR-artigen Modi wirkt sich vorhersagbar auf den Klartext aus), ohne dass dies erkannt wird. Bei CBC führt ein
  manipuliertes letztes Byte typischerweise zu einem Padding-Fehler, der pauschal als `OperationFailedException` propagiert
  wird (siehe Test `DecryptWithCorruptedCiphertextThrows`) – wird dieser Fehler (oder die Zeit bis dahin) für einen
  Angreifer von außen unterscheidbar gemacht (z.B. über eine Netzwerkantwort), entsteht ein klassisches Padding-Oracle.
  Dieser Punkt ist bereits in `REFACTORING.md` ("Crypto-Funktionen") als bekannte Lücke vermerkt – hier nur zusätzlich
  konkretisiert. Empfehlung bis zur AEAD-Umsetzung: Doku-Warnung, dass `Encrypt`/`Decrypt` **immer** mit einem separaten
  HMAC (Encrypt-then-MAC) kombiniert werden müssen, sobald die Daten über eine nicht vertrauenswürdige Verbindung/Speicherung
  laufen.

- [ ] **Schlüssel-/IV-Herkunft: Klasse erzwingt/unterstützt keine sichere Zufallsquelle** (`Crypt.cpp:405-426, 417-426, 528-548`)
  `Encrypt`/`Decrypt` generieren selbst nie Schlüssel oder IV – beide kommen ausschließlich vom Aufrufer über
  `setKey()`/`setIV()` (`ByteArrayPtr`, roher Speicher). Positiv: es gibt daher keine versteckte, unüberprüfte Abhängigkeit
  dieser Klassen von `pplib::Random()` (dessen Implementierung Gegenstand eines separaten Reviews ist). Negativ: es gibt
  auch keinerlei eingebaute Hilfestellung/Erzwingung für eine kryptografisch sichere IV-Erzeugung – ein Aufrufer kann
  trivial denselben IV wiederverwenden, ohne dass die Klasse das verhindert oder wenigstens warnt. Die mitgelieferten
  Tests (`tests/src/crypto/crypto.cpp:64-66`) verwenden für **alle** Testfälle denselben hartkodierten, statischen
  IV-Byte-Array über sämtliche Algorithmen/Modi hinweg – als Vorlage für Nutzer der Bibliothek ein schlechtes Beispiel,
  denn IV-Wiederverwendung bei CBC/CFB/OFB unter demselben Schlüssel bricht die semantische Sicherheit (z.B. XOR zweier
  Klartexte bei OFB/CFB-Wiederverwendung berechenbar). Empfehlung: eine Convenience-Methode (z.B.
  `Encrypt::generateIV(ByteArray&)` auf Basis von `pplib::Random()`) anbieten und in der Klassendoku explizit vor
  IV-Wiederverwendung warnen.

- [ ] **Veraltete/schwache Algorithmen ohne jede Warnung anwählbar** (`crypt.h:62-80`)
  `Algo_DES`, `Algo_RC2`, `Algo_RC5`, `Algo_IDEA`, `Algo_CAST5`, `Algo_BLOWFISH`, `Algo_TRIPLE_DES` sind gleichberechtigt
  neben AES/ARIA/CAMELLIA aufgeführt. DES hat nur 56 Bit effektive Schlüssellänge (heute mit überschaubarem Aufwand
  brute-forcebar). Alle genannten Algorithmen haben eine 64-Bit-Blockgröße und sind damit in CBC/CFB unter einem
  Schlüssel ab ca. 32 GiB Datenvolumen anfällig für die Sweet32-Kollisionsangriffe. Keine Doku-Warnung, keine
  Kennzeichnung im Enum. Empfehlung: zumindest per Kommentar im Enum als "veraltet/nur für Kompatibilität" markieren.

- [ ] **Kein Zeroing sensibler Zwischenpuffer** (`Crypt.cpp:428-486, 551-609`)
  `update()`/`final()`/`encrypt()`/`decrypt()` legen Klartext bzw. Geheimtext in einem lokalen `ByteArray temp` ab. Beim
  Verlassen der Funktion – auch beim regulären Verlassen nach `out.copy(...)`, erst recht bei einer Exception mitten in
  `EVP_EncryptUpdate`/`EVP_DecryptFinal_ex` – wird dieser Speicher über den `ByteArray`-Destruktor via `::free()`
  freigegeben, ohne vorher genullt zu werden (kein `OPENSSL_cleanse`/`explicit_bzero`). Enthält der Puffer entschlüsselte
  Klartextdaten (z.B. ein Passwort), bleibt dieser für die Prozesslaufzeit im freigegebenen Heap-Speicher lesbar
  (klassisches "sensitive data not cleared before free"). Gleiches gilt implizit für die vom Aufrufer übergebenen
  Schlüssel-/IV-`ByteArrayPtr`-Objekte, die von dieser Klasse gar nicht berührt werden – das Zeroing nach Gebrauch liegt
  hier vollständig beim Aufrufer, ohne dass die Doku das erwähnt. Empfehlung: für die internen Zwischenpuffer ein
  "secure wipe before free" ergänzen; in der Doku von `setKey()`/`setIV()` erwähnen, dass der Aufrufer für das Löschen
  seines eigenen Schlüsselmaterials verantwortlich ist.

**Sonstige Design-Punkte:**

- [ ] **Fehlender virtueller Destruktor in `Crypt` trotz öffentlicher Vererbung** (`crypt.h:45-89`)
  `Encrypt`/`Decrypt` erben `public` von `Crypt`, das über `friend`-Deklarationen erkennbar eng mit ihnen zusammenarbeitet,
  aber `~Crypt()` ist nicht `virtual`. `delete` über einen `Crypt*`, der auf ein `Encrypt`/`Decrypt`-Objekt zeigt, ist damit
  laut Standard undefiniertes Verhalten:
  ```cpp
  pplib::Crypt* c = new pplib::Encrypt(pplib::Crypt::Algo_AES_128, pplib::Crypt::Mode_CBC);
  delete c;   // UB: nicht-virtueller Destruktor der Basisklasse
  ```
  Aktuell folgenlos, da weder `Encrypt` noch `Decrypt` zusätzliche Member/Ressourcen über `Crypt::ctx` hinaus deklarieren
  (der aufgerufene `~Crypt()` räumt exakt das auf, was auch bei korrektem virtuellen Dispatch passieren würde) – eine
  tickende Falle aber, sobald eine der beiden Klassen künftig eigene Ressourcen bekommt. Fix: `virtual ~Crypt() = default;`
  (bzw. den bestehenden Destruktor `virtual` machen).

- [ ] **`Crypt` ist direkt instanziierbar, aber ohne Subklasse komplett funktionslos** (`crypt.h:45-89`)
  Eine nackte `pplib::Crypt c;` lässt sich anlegen, aber jede Methode wirft sofort `NotInitializedException`, da nur
  `Encrypt::setAlgorithm()`/`Decrypt::setAlgorithm()` jemals `ctx` befüllen. `Crypt` fungiert de facto als abstrakte Basis,
  ist aber nicht als solche markiert. Fix: Konstruktor `protected` machen oder eine reine virtuelle Methode ergänzen.

## Doku / Kosmetik

- [ ] `crypt.h` enthält keinerlei Methodendokumentation (kein `@brief`, keine Parameter-/Exception-Beschreibung) – für
      sicherheitsrelevanten Code besonders relevant, insbesondere sobald die Längenprüfung aus dem Bugfix oben ergänzt
      wird, sollte die erwartete Exception dokumentiert werden.
- [ ] `Mode`/`Algorithm`-Enums haben keine Wertkommentare; welche Modi/Algorithmen als "nicht für neue Systeme empfohlen"
      gelten (ECB, DES, RC2 …), ist nirgends vermerkt.

## Verifiziert OK (kein Handlungsbedarf)

- `getCipher()`: alle Case-Blöcke schließen korrekt mit `break` ab – keine Fallthrough-Bugs zwischen den Algorithmus-Cases
  (passend zum vorherigen Commit "added missing breaks").
- `Encrypt::setAlgorithm()`/`Decrypt::setAlgorithm()` sind exception-safe: bei ungültigem Algorithmus/Modus bzw.
  fehlgeschlagenem `EVP_*Init_ex` wird der bereits erzeugte `ctx` sauber wieder freigegeben und auf `nullptr` gesetzt,
  bevor die Exception geworfen wird – das Objekt bleibt danach konsistent im Zustand "nicht initialisiert"
  (getestet durch `ConstructWithInvalidAlgorithmThrows`, `ConstructWithInvalidModeThrows`, `MethodsThrowWhenNotInitialized`).
- Wiederholtes `setAlgorithm()` auf einem bereits initialisierten Objekt gibt den alten `ctx` korrekt frei, bevor ein
  neuer erzeugt wird – kein Leak beim Algorithmuswechsel.
- `update()`/`final()`/`encrypt()`/`decrypt()` dimensionieren den Ausgabepuffer korrekt gemäß OpenSSL-Empfehlung
  (`insize + blocksize` für Streaming, `insize + 2*blocksize` für Single-Shot inkl. `final`).
- Padding-Fehler bei `Decrypt::decrypt()`/`final()` werden korrekt über den `EVP_DecryptFinal_ex`-Rückgabewert erkannt und
  als `OperationFailedException` propagiert (Test `DecryptWithCorruptedCiphertextThrows`).
- `setKeyLength()` prüft den `EVP_CIPHER_CTX_set_key_length`-Rückgabewert korrekt und wirft `InvalidKeyLengthException`
  bei ungültiger Länge (Test `SetKeyLength`).
