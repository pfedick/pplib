# Digest Review – Befunde

Review vom 2026-09-01, Scope: `include/pplib/crypto/digest.h` + `src/crypto/Digest.cpp` (Klasse `Digest`).
Exceptions gegen `include/pplib/exceptions.h` und `include/pplib/crypto/crypt.h` verifiziert (`InvalidAlgorithmException`,
`NoAlgorithmSpecifiedException` sind real, aber – siehe Design-Befund unten – in `crypt.h` deklariert, nicht in
`digest.h`; `OutOfMemoryException`, `UnsupportedFeatureException`, `ReadException`, `FileNotOpenException` in
`exceptions.h`). `Crc32()` gegen `include/pplib/core/functions.h:64` verifiziert. Verwendung geprüft in
`tests/src/crypto/digest.cpp` (alle Algorithmen, Streaming-API, Fehlerpfade) und `tests/src/math/sha256.cpp`.

* Review done by: Claude Sonnet 5

## Bugs (kritisch)

- [ ] **`getDigest()` prüft `ctx`/`m` nicht vor `EVP_DigestFinal()` → Absturz bei fehlendem Algorithmus** (`Digest.cpp:263-283`)
  ```cpp
  ByteArray Digest::getDigest()
  {
      unsigned int len = 0;
      if (!ret) { ret = (unsigned char*)malloc(EVP_MAX_MD_SIZE); ... }
      EVP_DigestFinal((EVP_MD_CTX*)ctx, ret, &len);   // ctx kann hier nullptr sein!
      ...
  ```
  Jede andere öffentliche Methode, die auf `ctx`/`m` zugreift (`addData()`, `reset()`), prüft das vorher explizit und
  wirft `NoAlgorithmSpecifiedException`. `getDigest()` vergisst diese Prüfung. Reproduktion:
  ```cpp
  pplib::Digest dig;      // Default-Konstruktor: ctx == nullptr, m == nullptr, kein setAlgorithm() aufgerufen
  dig.getDigest();        // EVP_DigestFinal(nullptr, ...) -> Nullpointer-Dereferenzierung in OpenSSL, Absturz
  ```
  Der Default-Konstruktor `Digest()` ist bewusst ohne Algorithmus nutzbar (siehe Test `ConstructorSimple`), das lädt
  gerade dazu ein, `getDigest()` versehentlich ohne vorheriges `setAlgorithm()` aufzurufen.
  Fix: Am Anfang ergänzen: `if (!ctx || !m) throw NoAlgorithmSpecifiedException();`

- [ ] **Rule-of-Three-Verletzung: Kopieren eines `Digest`-Objekts dupliziert `ctx`/`ret` → Double-Free/Use-after-Free** (`digest.h:47-51`, `Digest.cpp:81-93`)
  ```cpp
  Digest::~Digest()
  {
      if (ret) free(ret);
      if (ctx) { EVP_MD_CTX_free((EVP_MD_CTX*)ctx); /* oder _destroy auf alten OpenSSL-Versionen */ }
  }
  ```
  Wie bei `Crypt` (siehe `crypt-review.md`) gibt es weder Copy-Konstruktor noch Copy-Zuweisungsoperator, obwohl die Klasse
  über einen benutzerdefinierten Destruktor zwei rohe, eigentümerbehaftete Zeiger (`ctx`, `ret`) verwaltet:
  ```cpp
  pplib::Digest a(pplib::Digest::Algo_SHA256);
  pplib::Digest b = a;     // flache Kopie: b.ctx == a.ctx, b.ret == a.ret (b.ret ist zu diesem Zeitpunkt meist noch nullptr)
  // a und b werden beide zerstoert -> EVP_MD_CTX_free(ctx) wird zweimal auf denselben Pointer aufgerufen
  ```
  Auch hier unterdrückt der benutzerdefinierte Destruktor die implizite Erzeugung von Move-Operationen, sodass selbst
  Container-Reallokationen (`std::vector<Digest>`) auf dieselbe kaputte Kopie zurückfallen. Aktuell wird `Digest` im
  Repo nirgends kopiert, der Bug ist also latent, aber ein triviales Landmine für jede künftige Verwendung in einem
  Container oder als Rückgabewert.
  Fix: Kopieren verbieten, Move-Konstruktor/-Zuweisung ergänzen (Zeiger übernehmen, Quelle auf `nullptr` setzen).

## Bugs (mittel)

- [ ] **EVP-Rückgabewerte werden in `Digest.cpp` nirgends geprüft** (`Digest.cpp:126, 183, 193, 273, 279, 296`)
  `EVP_DigestInit_ex()`, `EVP_DigestInit()`, `EVP_DigestUpdate()` und `EVP_DigestFinal()` werden durchweg ohne
  Rückgabewertprüfung aufgerufen – im Gegensatz zu `Crypt.cpp`, das jeden `EVP_*Init_ex`-Aufruf explizit auf `!= 1`
  prüft und eine Exception wirft. Schlägt z.B. `EVP_DigestInit_ex()` in `setAlgorithm()` fehl (Rückgabewert 0, z.B. durch
  eine ENGINE/FIPS-Fehlkonfiguration), bleibt das `Digest`-Objekt scheinbar korrekt initialisiert (keine Exception),
  liefert aber möglicherweise einen falschen oder nicht reproduzierbaren Hash-Wert, ohne dass der Aufrufer das erkennen
  kann.
  Fix: Rückgabewerte prüfen und bei `0` konsistent `OperationFailedException` werfen (analog zu `Crypt.cpp`).

- [ ] **Inkonsistenter Objektzustand, wenn `EVP_MD_CTX_new()` fehlschlägt** (`Digest.cpp:130-185`)
  ```cpp
  case Algo_MD5: m = EVP_md5(); break;   // <- m wird SOFORT zugewiesen
  ...
  if (!ctx) {
      ctx = EVP_MD_CTX_new();
      if (!ctx) throw OutOfMemoryException();   // <- ctx bleibt nullptr, m ist aber schon gesetzt!
  }
  ```
  `m` wird zugewiesen, bevor `ctx` erfolgreich aufgebaut ist. Schlägt `EVP_MD_CTX_new()` fehl (z.B. OOM) und
  `setAlgorithm()` wirft `OutOfMemoryException`, bleibt das Objekt in einem inkonsistenten Zustand zurück: `m != nullptr`,
  aber `ctx == nullptr`. Ein nachfolgender Aufruf von `addData()` besteht die Prüfung `if (!m) throw ...` (da `m` gesetzt
  ist) und ruft anschließend `EVP_DigestUpdate((EVP_MD_CTX*)ctx, ...)` mit `ctx == nullptr` auf → Absturz statt einer
  erneuten, aussagekräftigen Exception. Nur über OOM erreichbar, daher eher ein Robustheits- als ein akuter
  Sicherheitsbefund.
  Fix: `m` erst zuweisen, nachdem `ctx` erfolgreich steht, oder im Fehlerfall `m = nullptr;` explizit zurücksetzen.

## Design

**Kryptografische Befunde:**

- [ ] **Kein HMAC, keine konstante-Zeit-Vergleichsfunktion** (`digest.h`, `include/pplib-crypto.h`)
  `Digest` bietet ausschließlich rohe Hash-Primitiven (MD5…SHA512, RIPEMD160) sowie CRC32/Adler32-Prüfsummen an. Ein
  `HMAC`-Klassenentwurf existiert zwar als "Draft" in `include/pplib-crypto.h`, ist aber nicht implementiert (kein
  `.cpp`, Aufruf würde zum Linker-Fehler führen). Ohne echtes HMAC besteht die naheliegende Versuchung, MACs von Hand
  über `Digest`-Primitiven zu bauen, z.B. `hash(key || message)` – das ist bei MD5/SHA-1/SHA-2 (allesamt
  Merkle-Damgård-Konstruktionen) anfällig für Length-Extension-Angriffe, mit denen ein Angreifer ohne Kenntnis des
  Schlüssels eine gültige MAC-Erweiterung für `message || padding || attacker_data` berechnen kann. `Digest` warnt an
  keiner Stelle vor dieser Fehlnutzung.
  Fix/Empfehlung: In der Klassendoku explizit vor `hash(key||data)`-Konstruktionen warnen, solange `HMAC` nicht real
  implementiert ist.

- [ ] **Kein konstanter-Zeit-Vergleich für Hash-/MAC-Werte** (`digest.h` – fehlende API; `bytearrayptr.h:441-444`)
  `Digest` liefert Ergebnisse als `ByteArray` (`getDigest()`) bzw. `String`/`WideString` (`saveDigest()`), bietet aber
  keine eigene `compare()`/`verify()`-Methode an. Der naheliegende Weg für einen Aufrufer, zwei Digests zu vergleichen
  (z.B. um einen erwarteten Hash oder eine MAC zu verifizieren), ist `ByteArrayPtr::operator==`/`String::operator==`,
  und `ByteArrayPtr::operator==` ist explizit über `memcmp()` implementiert:
  ```cpp
  inline bool operator==(const ByteArrayPtr& other) const { return memcmp(other) == 0; }
  ```
  `memcmp()` bricht beim ersten abweichenden Byte ab – kein konstanter Zeitbedarf. Für reine Prüfsummen (Datei-Integrität
  gegen Übertragungsfehler) unkritisch, aber sobald ein Aufrufer `Digest`/`ByteArray::operator==` zur Prüfung eines
  geheimen MAC- oder Passwort-Hash-Werts gegen Angreifer-kontrollierten Input verwendet, entsteht ein klassischer
  Timing-Seitenkanal (CWE-208). Diese beiden Klassen (`Digest`, `Crypt`) selbst führen aktuell nirgends einen solchen
  Vergleich durch – das Risiko liegt ausschließlich bei künftigen Aufrufern, die aber von der API keinerlei sicheren
  Weg angeboten bekommen.
  Fix/Empfehlung: Eine `constantTimeEquals(const ByteArrayPtr&, const ByteArrayPtr&)`-Hilfsfunktion anbieten (z.B. in
  `pplib-crypto.h`) und in der `Digest`-Doku auf das Risiko hinweisen.

- [ ] **MD5/SHA-1 ohne Warnung gleichrangig neben SHA-256/384/512 angeboten** (`digest.h:56-63`)
  Beide Algorithmen gelten kryptografisch als gebrochen bzw. stark geschwächt (MD5: Kollisionen trivial erzeugbar;
  SHA-1: seit "SHAttered" 2017 praktisch kollidierbar). Für reine Prüfsummen/Nicht-Sicherheitszwecke unproblematisch,
  aber ohne jede Kennzeichnung im Enum oder in der Doku, dass sie nicht für Signaturen, Zertifikate oder
  Passwort-Hashing gegen einen aktiven Angreifer verwendet werden dürfen.

**Sonstiger Design-Punkt:**

- [ ] **`Digest.cpp` verwendet Exceptions, die nicht im eigenen Header, sondern in `crypt.h` deklariert sind** (`Digest.cpp:114, 168, 192, 290`, vgl. `crypt.h:41-42`)
  `InvalidAlgorithmException` und `NoAlgorithmSpecifiedException` sind in `include/pplib/crypto/crypt.h` deklariert, nicht
  in `include/pplib/crypto/digest.h`. `Digest.cpp` inkludiert nicht `digest.h` direkt, sondern nur `<pplib-crypto.h>`,
  welches transitiv sowohl `digest.h` als auch `crypt.h` einbindet – nur dadurch kompiliert der Code. Ein Nutzer, der
  gezielt nur `<pplib/crypto/digest.h>` einbindet (wie es der Header-Name nahelegt), sieht diese beiden von `Digest`
  geworfenen Exceptions nirgends deklariert.
  Fix: Beide Exceptions in eine gemeinsame, algorithmus-neutrale Datei verschieben (z.B. eine künftige
  `crypto/exceptions.h`), oder zumindest in `digest.h` selbst (zusätzlich zu `crypt.h`) deklarieren.

## Doku / Kosmetik

- [ ] `digest.h` enthält keinerlei Methodendokumentation (kein `@brief`, keine Parameter-/Exception-Beschreibung).
- [ ] `addData(FileObject& file)` (`Digest.cpp:213-232`) setzt intern immer `file.seek(0)` – nicht dokumentiert, dass dies
      die Position eines vom Aufrufer bereits anders positionierten `FileObject` überschreibt.
- [ ] `crc32()`/`adler32()` sind reine Nicht-kryptografische Prüfsummen ohne Integritätsschutz gegen aktive Manipulation,
      stehen aber unkommentiert direkt neben den kryptografischen Hash-Funktionen in derselben Klasse – Verwechslungsgefahr
      ("auch für Integritätsschutz geeignet").
- [ ] Uneinheitliche OpenSSL-API-Nutzung: `setAlgorithm()` verwendet `EVP_DigestInit_ex(ctx, m, nullptr)`, `reset()`
      verwendet stattdessen die ältere `EVP_DigestInit(ctx, m)` – funktional äquivalent, aber unnötig zwei verschiedene
      Aufrufstile für denselben Zweck im selben File.

## Verifiziert OK (kein Handlungsbedarf)

- `addData(FileObject&)` verarbeitet Dateien in ≤1-MiB-Chunks statt alles auf einmal in einen `int`-Parameter zu casten –
  vermeidet damit die Integer-Truncation-Problematik, die bei `Encrypt`/`Decrypt` besteht (siehe `crypt-review.md`).
- `addData(const void*, size_t)` prüft korrekt `if (!m) throw NoAlgorithmSpecifiedException();` vor jedem Zugriff auf `ctx`.
- `reset()` prüft korrekt `if (!m || !ctx) throw NoAlgorithmSpecifiedException();` vor Zugriff.
- `bytesHashed()` wird bei `getDigest()`/`reset()` korrekt auf 0 zurückgesetzt und bei jedem `addData()`-Aufruf korrekt
  aufsummiert (verifiziert über mehrere Dateien hinweg, Test `TestAddDataFromSeveralFiles`).
- `setAlgorithm()` ist bei erneutem Aufruf auf einem bereits initialisierten Objekt exception-safe: der alte Digest-Status
  wird über `reset()` sauber verworfen, bevor mit dem neuen Algorithmus neu initialisiert wird (Test `TestChangeAlgorithm`).
- `crc32()`/`adler32()` delegieren korrekt an die vorhandene `Crc32()`-Implementierung (`functions.h:64`) bzw. sind eine
  korrekte Standard-Adler32-Implementierung ohne Off-by-one (verifiziert gegen Test-Erwartungswerte).
