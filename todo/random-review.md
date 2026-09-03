# Random Review – Befunde

Review vom 2026-09-01, Scope: `src/math/random.cpp` (96 Zeilen). Öffentliche API: `void srand(uint32_t)`,
`size_t rand(size_t, size_t)`, `float randf(float, float)`, `double randd(double, double)`,
`ByteArray Random(size_t)`, `ByteArray& Random(ByteArray&, size_t)` – alle deklariert und dokumentiert in
`include/pplib/core/functions.h:153-228`. Codebase-weit geprüft: es gibt aktuell **keine** andere
Zufallszahlenquelle (kein `/dev/urandom`, `getrandom()`, `RAND_bytes`, `arc4random`, `CryptGenRandom`) in
`pplib` – diese Datei ist die einzige Zufallsquelle der gesamten Bibliothek. `pplib::Crypt`/`Encrypt`
(`src/crypto/Crypt.cpp`) rufen `Random()` aktuell nicht selbst auf (IV wird vom Aufrufer per `setIV()`
übergeben), sind aber der naheliegendste künftige Konsument dieser API für Schlüssel-/IV-/Nonce-Generierung,
da bereits OpenSSL gelinkt ist und `RAND_bytes()` dort trivial verfügbar wäre.

* Review done by: Claude Sonnet 5

## Bugs (kritisch) – Design-Flaw, sicherheitsrelevant

- [ ] **Zufallsquelle ist ein Mersenne-Twister (`std::mt19937_64`), kein kryptographisch sicherer Zufallszahlengenerator – ungeeignet für Schlüssel-/IV-/Token-Erzeugung** (Zeile 38)
  ```cpp
  static thread_local std::mt19937_64 rng(std::random_device{}());
  ...
  ByteArray Random(size_t bytes)
  {
      ...
      char* buffer = (char*)bin.malloc(bytes);
      std::uniform_int_distribution<unsigned int> dist(0, 255);
      for (size_t i = 0; i < bytes; i++) {
          buffer[i] = static_cast<char>(dist(rng));
      }
      return bin;
  }
  ```
  `std::mt19937_64` ist ein **nicht-kryptographischer** Pseudozufallsgenerator. Sein interner Zustand (2496 Byte /
  312 x 64-Bit-Worte) lässt sich aus **624 aufeinanderfolgenden 64-Bit-Ausgabewerten vollständig rekonstruieren**
  (Standard-Angriff auf Mersenne-Twister-Generatoren) – danach sind alle vergangenen und zukünftigen Ausgaben des
  Generators exakt vorhersagbar. Jede Verwendung dieser Funktion zur Erzeugung von kryptographischem Material
  (AES-Schlüssel, IV/Nonce, Session-Token, Passwort-Salt, CSRF-Token o.ä.) wäre komplett unsicher, sobald ein
  Angreifer genug Ausgabebytes derselben Thread-Instanz beobachten kann (z.B. mehrere erzeugte IVs/Tokens einer
  Anwendung), da er daraus alle weiteren "Zufalls"-Werte exakt vorherberechnen kann. Da `functions.h` diese
  Funktion ohne jede Sicherheits-Einschränkung dokumentiert ("Zufallsdaten erzeugen") und sie die einzige
  Zufallsquelle der gesamten Bibliothek ist, ist es naheliegend, dass ein künftiger Aufrufer (z.B. `Crypt`/`Encrypt`
  bei der IV-Erzeugung, siehe paralleler Crypt-Review) sie unreflektiert für sicherheitsrelevante Zwecke
  einsetzt.
  Fix: Für sicherheitsrelevante Zwecke einen kryptographisch sicheren Zufallsgenerator verwenden – da die
  Bibliothek bereits gegen OpenSSL linkt (`pplib_crypto`), böte sich `RAND_bytes()` an; plattformunabhängig auch
  `getrandom()` (Linux), `/dev/urandom`, `CryptGenRandom`/`BCryptGenRandom` (Windows) oder `arc4random_buf()`
  (BSD/macOS). Mindestens sollte die Doku von `Random()` in `functions.h` explizit und unübersehbar festhalten,
  dass die Funktion **nicht** für kryptographische Zwecke geeignet ist, damit niemand sie versehentlich dafür
  einsetzt (analog `Math.random()` in Java/JS, wo diese Einschränkung ebenfalls explizit dokumentiert ist).

## Design

- [ ] **Seed wird aus nur einem einzigen 32-Bit-Wert von `std::random_device` gezogen, nicht aus einer vollen Seed-Sequenz** (Zeile 38)
  ```cpp
  static thread_local std::mt19937_64 rng(std::random_device{}());
  ```
  `std::random_device{}()` liefert einen einzelnen `unsigned int` (typischerweise 32 Bit). Der
  Einzelwert-Konstruktor von `std::mt19937_64::seed(result_type)` expandiert daraus über eine interne LCG den
  kompletten 19937-Bit-Zustand – das bedeutet, der gesamte Anfangszustand des Generators hat effektiv nur ca.
  32 Bit Entropie statt der potenziell verfügbaren deutlich größeren Menge. Das verschärft das Grundproblem oben
  zusätzlich (weniger Aufwand für einen Angreifer, den Seed durch Ausprobieren zu treffen, falls der
  MT19937-Ausgabe-Angriff selbst nicht anwendbar wäre). Für den aktuellen nicht-kryptographischen Einsatzzweck
  unkritisch, aber ein weiterer Beleg dafür, dass dieser Generator nicht für sicherheitsrelevante Zwecke gedacht
  war/ist.
  Fix (falls an dieser Stelle festgehalten wird, aber Entropie verbessert werden soll): `std::seed_seq` aus
  mehreren `std::random_device`-Aufrufen befüllen und damit seeden, z.B.
  `std::array<uint32_t,16> seeds; std::generate(seeds.begin(),seeds.end(),std::ref(rd)); std::seed_seq seq(seeds.begin(),seeds.end()); rng.seed(seq);`

- [ ] **`srand(x)` seedet nur die Instanz des aufrufenden Threads – nicht dokumentiert** (Zeile 37-43)
  ```cpp
  static thread_local std::mt19937_64 rng(std::random_device{}());
  void srand(uint32_t x) { rng.seed(x); }
  ```
  Die Doku in `functions.h:156-165` beschreibt `srand()` so, als gäbe es einen globalen Generator ("wird der
  Zufallszahlengenerator... initialisiert"). Tatsächlich ist `rng` `thread_local`: ein Aufruf von
  `pplib::srand(42)` in Thread A hat keinerlei Effekt auf die (unabhängig aus `random_device` geseedete)
  Generator-Instanz in Thread B. Wer `srand()` zur Reproduzierbarkeit von Tests einsetzt (genau der in der Doku
  genannte Anwendungsfall) und dabei mit mehreren Threads arbeitet, bekommt in jedem Thread unterschiedliche,
  nicht reproduzierbare Ergebnisse, ohne dass die Doku das erwarten lässt.
  Fix: in der Doku explizit auf den Thread-lokalen Geltungsbereich hinweisen.

- [ ] **Namenskollision mit der C-Standardbibliothek (`::rand`/`::srand`)** (Zeile 40, 45)
  `pplib::srand(uint32_t)` und `pplib::rand(size_t,size_t)` tragen dieselben Namen wie `<cstdlib>`s `::srand(unsigned)`/
  `::rand()`, haben aber andere Signaturen/Semantik (unterschiedlicher Zufallsgenerator, andere Parameter). Bei
  `using namespace pplib;` in einer Datei, die auch `<cstdlib>` nutzt, ist Verwechslungsgefahr real – ein Aufrufer
  könnte `srand(time(0))` schreiben und in der irrigen Annahme sein, damit `::rand()`-Aufrufe andernorts im
  Programm zu beeinflussen, während tatsächlich nur `pplib`s Thread-lokaler MT19937 geseedet wird (oder
  umgekehrt). Aktuell im Code nur konsistent qualifiziert verwendet (`pplib::rand(...)` in `Array.cpp`), aber ein
  Footgun für künftigen Code.

## Doku / Kosmetik

- [ ] **`rand(min, max)` mit `min > max` liefert stillschweigend `min` statt zu validieren** (Zeile 45-50)
  ```cpp
  size_t rand(size_t min, size_t max)
  {
      if (min >= max) return min;
      ...
  }
  ```
  Für `min == max` ist das sinnvoll (einziger möglicher Wert), für `min > max` (vertauschte Argumente, ein
  typischer Aufrufer-Fehler) wird der Fehler aber verschluckt statt z.B. `IllegalArgumentException` zu werfen –
  macht das Debuggen eines vertauschten Aufrufs schwerer. Gleiches Muster in `randf`/`randd` (Zeile 53-64).

## Verifiziert OK (kein Handlungsbedarf)

- `Random(size_t)`/`Random(ByteArray&, size_t)` nutzen `ByteArray::malloc()`, dessen früher bekannter
  Double-Free-Bug bei Größenüberschreitung laut `done/bytearray-review.md` bereits gefixt ist – keine erneute
  Prüfung hier nötig, Verhalten ist konsistent mit der aktuellen `ByteArray`-Implementierung.
- Byte-Erzeugung selbst (`std::uniform_int_distribution<unsigned int> dist(0,255)`) ist auf allen Pfaden korrekt
  konvertiert und befüllt (kein Off-by-one, `bytes == 0` wird jeweils als Sonderfall früh behandelt).
- Kein global geteilter mutable State zwischen Threads (jede Thread-Instanz von `rng` ist unabhängig) – abgesehen
  vom oben genannten Doku-Gap bei `srand()` kein Thread-Safety-Problem.
