# TimeDelta Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/timedelta.h` + `src/types/TimeDelta.cpp`.
Verifiziert gegen `include/pplib/exceptions.h`, `include/pplib/core/functions.h` (`Trim`, `UpperCase`),
`String::isNumeric()` (src/types/String.cpp:168, akzeptiert führendes `-`) und `Array::explode()`
(src/types/Array.cpp:236, `skipemptylines` default `false` → führende/leere Trenner erzeugen leere Elemente).
Gegengeprüft mit `DateTime::operator+/-(const TimeDelta&)` (include/pplib/types/datetime.h:947-970) als
tatsächlichem Konsumenten.

## Bugs (kritisch)

- [ ] **`operator/` hat keinen Schutz gegen Division durch 0** (`timedelta.h:175-178`)
  ```cpp
  TimeDelta operator/(int64_t divisor) const noexcept
  {
      return TimeDelta(us / divisor);
  }
  ```
  Ganzzahl-Division durch 0 ist Undefined Behavior (auf x86/Linux typischerweise `SIGFPE`, Programmabsturz).
  Naheliegender Aufrufer-Fehler, z.B. beim Berechnen eines Durchschnitts über eine potenziell leere Menge:
  ```cpp
  TimeDelta avg = sum / count;   // count == 0 -> Crash, keine Exception, kein definiertes Verhalten
  ```
  Die Funktion ist zudem `noexcept` deklariert, ein Fix per Exception würde also zusätzlich `std::terminate`
  auslösen statt sauber zu propagieren.
  Fix: Guard einbauen und `noexcept` entfernen (konsistent mit den anderen werfenden Methoden der Klasse):
  ```cpp
  TimeDelta operator/(int64_t divisor) const
  {
      if (divisor == 0) throw IllegalArgumentException("TimeDelta::operator/: division by zero");
      return TimeDelta(us / divisor);
  }
  ```

  ==> FIXED, Methode ist jetzt in src und wirft IllegalArgumentException

## Design

- [ ] **Konstruktor und `set()` haben unterschiedliche Default-Parameter für `hours`** (`timedelta.h:58-62` vs. `93-103`)
  ```cpp
  explicit TimeDelta(int64_t days, int64_t hours, int64_t minutes = 0, ...) noexcept { ... }   // hours: Pflicht
  TimeDelta& set(int64_t days, int64_t hours = 0, int64_t minutes = 0, ...) noexcept { ... }   // hours: optional
  ```
  `TimeDelta(5)` kompiliert nicht (fehlendes `hours`-Argument), `td.set(5)` dagegen schon und setzt `hours` auf 0.
  Zwei API-Oberflächen für dieselbe Semantik mit unterschiedlicher Ergonomie – leicht zu übersehende Inkonsistenz.
  Fix: `hours = 0` auch im Konstruktor als Default ergänzen.

  ===> Nicht Umsetzbar wegen Kollision mit privatem Konstruktor mit Microsekunden. Stattdessen wird hour in der set-Methode auch Pflicht, um die Konsistenz zu wahren


- [ ] **`fromString()` kann grundsätzlich kein negatives `TimeDelta` erzeugen, obwohl die Klasse negative Werte
  sonst vollständig unterstützt** (`TimeDelta.cpp:41-44`)
  ```cpp
  String s = UpperCase(Trim(str));
  s.replace(",", ":");
  s.replace(".", ":");
  s.replace("-", ":");   // <- ein führendes Vorzeichen "-" wird zum Trennzeichen statt erkannt zu werden
  ```
  Die Doku (`timedelta.h:66-71`) listet `-` als akzeptiertes *Trennzeichen*, nicht als Vorzeichen – das ist in
  sich konsistent dokumentiert, steht aber im Kontrast zum Rest der Klasse (`operator-()`, negative Faktoren bei
  `operator*`, etc.), die `TimeDelta` explizit als vorzeichenbehaftete Dauer behandelt. Ein Aufrufer, der
  `TimeDelta::fromString("-05:30")` für "minus 5:30" erwartet, bekommt eine `IllegalArgumentException`
  (führendes `-` wird zu `:`, dadurch ein leeres erstes Element, das `isNumeric()` als `false` erkennt) statt
  eines negativen Deltas.
  Fix: Vorzeichen vor dem Ersetzen der Trennzeichen separat erkennen und behandeln, z.B.
  ```cpp
  bool negative = s.startsWith("-");   // vor dem Trim/Replace prüfen
  if (negative) s = s.substr(1);
  // ... restliches Parsing wie bisher ...
  return negative ? -result : result;
  ```
  oder, falls kein Vorzeichen gewünscht ist, den Doku-Kommentar präzisieren ("negative Deltas können nicht aus
  einem String erzeugt werden").

  ==> "-" am beginn des Strimgs wird jetzt unterstützt. Ferner habe ich die Methode toString() ergänzt, die im Header noch als TODO vermerkt war

## Doku / Kosmetik

- [ ] `operator*`/`fromDays()`/`fromWeeks()` etc. prüfen nicht auf `int64_t`-Overflow bei sehr großen Faktoren/Werten
  (z.B. `TimeDelta::fromDays(INT64_MAX)`). Bei realistischen Eingaben (Tage/Stunden im Anwendungsbereich der
  Klasse) irrelevant, aber falls `TimeDelta` später auch für arithmetisch abgeleitete Werte (Multiplikation
  großer Reihen) verwendet wird, wäre ein Kommentar zum bewusst fehlenden Overflow-Schutz hilfreich.

  ==> Wird nicht geändert

## Verifiziert OK (kein Handlungsbedarf)

- `toMilliseconds()`/`toSeconds()` (timedelta.h:81-88) runden für negative Werte konsistent **Richtung Null**
  (Ganzzahldivision), nicht "floor" – das entspricht der üblichen `std::chrono::duration_cast`-Konvention und ist
  innerhalb der Klasse konsistent (`toMilliseconds`/`toSeconds` verhalten sich identisch).
  Beispiel: `TimeDelta::fromMicroseconds(-1500).toMilliseconds() == -1` (nicht `-2`).
- Alle Vergleichsoperatoren und `operator+/-/+=/-=` sind einfache, korrekte Wrapper um `us` – keine
  Vorzeichen-Inkonsistenzen zwischen ihnen gefunden.
- Named Factories (`fromWeeks`, `fromDays`, `fromHours`, ...) verwenden konsistente Umrechnungsfaktoren
  untereinander und mit `Time::toMicroseconds()`/`toSeconds()` (gleiche Konstanten: 86400/3600/60/1000000).
- Der private `explicit constexpr TimeDelta(int64_t)`-Konstruktor verhindert sinnvoll eine mehrdeutige öffentliche
  Ein-Parameter-Konstruktion aus rohen Mikrosekunden (muss über die benannten `from*`-Factories laufen).
