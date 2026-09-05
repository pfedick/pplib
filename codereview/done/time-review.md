# Time Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/time.h` + `src/types/Time.cpp`.
Verifiziert gegen `include/pplib/exceptions.h`, `include/pplib/core/functions.h` (`Trim`, `UpperCase`),
`String::isNumeric()`/`toInt()` (src/types/String.cpp:168, 1212) und `Array::explode()` (src/types/Array.cpp:236,
`skipemptylines` default `false`). Gegengeprüft mit `DateTime` (include/pplib/types/datetime.h,
src/types/DateTime.cpp) als tatsächlichem Konsumenten von `Time` zusammen mit `Date`/`TimeZone`.

## Bugs (kritisch)

- [ ] **`Time::set(const String&)`: Narrowing-Cast vor der Bereichsprüfung erlaubt stillschweigend ungültige Stunden** (`Time.cpp:116`)
  ```cpp
  return set(parts.at(0).toInt(), parts.at(1).toInt(), parts.at(2).toInt(), parts.at(3).toInt());
  ```
  Ziel-Signatur ist `Time& set(uint8_t hour, uint8_t minute, uint8_t second, uint32_t microseconds)` (`time.h:194`).
  `toInt()` liefert `int`; der Cast auf `uint8_t` passiert schon beim Aufruf, **bevor** die Bereichsprüfung in
  `set()` (`Time.cpp:81`) den Wert überhaupt sieht:
  ```cpp
  Time t;
  t.set("256:30:00");   // Stunde "256" -> (uint8_t)256 = 0 -> Ergebnis: 00:30:00, KEINE Exception
  ```
  Statt der erwarteten `IllegalArgumentException` für eine offensichtlich unsinnige Stunde entsteht eine still
  falsche, aber gültig aussehende Uhrzeit. Betrifft jeden Eingabewert, der modulo 256 in den gültigen Bereich
  zurückfällt (z.B. auch Minute/Sekunde mit dreistelligen Werten ≥256).
  Fix: vor dem Aufruf als `int` validieren und erst danach auf die engen Typen casten, z.B.
  ```cpp
  int h = parts.at(0).toInt();
  int m = parts.at(1).toInt();
  int s = parts.at(2).toInt();
  int u = parts.at(3).toInt();
  if (h > 23 || m > 59 || s > 59 || u > 999999) {
      throw IllegalArgumentException("Time::set: invalid time format (%s)", time.c_str());
  }
  return set((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint32_t)u);
  ```

  ==> FIXED wie bei date: wir erlauben uint32_t, damit kein narrowing. Negative Werte fallen raus, weil diese vorzeichenlos betrachtet größer als der Maximalwert sind

- [ ] **Self-Move-Assignment zerstört die Uhrzeit** (`Time.cpp:156-167`)
  ```cpp
  Time& Time::operator=(Time&& other) noexcept
  {
      hh = other.hh; ii = other.ii; ss = other.ss; us = other.us;
      other.hh = 0; other.ii = 0; other.ss = 0; other.us = 0;   // <- other IST *this bei Self-Move
      return *this;
  }
  ```
  ```cpp
  Time t(12, 30, 0);
  t = std::move(t);      // Felder werden erst kopiert (no-op), danach auf 0 gesetzt
  assert(t == Time(0,0,0));  // true – Uhrzeit ist weg
  ```
  Identisches Muster wie bei `Date` (siehe date-review.md). Self-Move ist im Anwendungscode selten explizit,
  aber ein bekannter Stolperstein bei generischem Code.
  Fix: Guard `if (this != &other)` oder Self-Move separat behandeln.

  ===> FIXED mit Guard, Tests ergänzt

## Design

- [ ] **`Time` hat keine Arithmetik mit `TimeDelta`, obwohl `DateTime` genau das für Datum+Uhrzeit anbietet**
  `DateTime::operator+/-(const TimeDelta&)` existiert (`datetime.h:952-970`), rechnet aber über den Umweg
  `toMicroseconds()`/`fromMicroseconds()` auf Basis des kompletten `DateTime`. `Time` selbst bietet kein
  `operator+(TimeDelta)`. Falls das später ergänzt wird (naheliegend, da `Time` und `TimeDelta` beide auf
  Mikrosekunden basieren), muss das Wrap-Verhalten bei Tagesüberlauf bzw. bei negativen Deltas definiert werden
  (aktuell macht das nur `Time::setFromSeconds()`/`setFromMicroseconds()` implizit über Modulo auf `uint32_t`/`uint64_t`,
  was für negative `int64_t`-Deltas aus `TimeDelta` nicht direkt passt).

## Doku / Kosmetik

- [ ] Fehlerhafte Fehlermeldung: Copy-Paste von `Date::set` in `Time::set(const String&)` (`Time.cpp:106`)
  ```cpp
  if (!parts[i].isNumeric()) {
      throw IllegalArgumentException("Date::set: invalid date format (%s)", time.c_str());
  }
  ```
  Meldung nennt die falsche Klasse/Methode. Fix: `"Time::set: invalid time format (%s)"`.
- [ ] Toter Code: `t.replace(".", ".")` (`Time.cpp:98`) ist ein No-Op (ersetzt "." durch "."), vermutlich Copy-Paste-Rest
      der vorherigen Zeile `t.replace(",", ".")`. Entweder entfernen oder war ein anderes Zeichen gemeint.
  ==> FIXED

- [ ] `%I`-Formatplatzhalter (`Time::format`, `Time.cpp:182-183`) zeigt für Mitternacht/Mittag `"00"` statt der im
      12-Stunden-Format gebräuchlichen `"12"` (`hh % 12` ergibt für `hh=0` und `hh=12` beides `0`). Ist konsistent mit
      der eigenen Doku ("Stunde (00-11)"), weicht aber vom verbreiteten 12h-Format (01-12) ab – falls das beabsichtigt
      keine 12h-Uhr im klassischen Sinn sein soll, wenigstens in der Doku explizit erwähnen, dass `%I` kein "echtes"
      12-Stunden-Format ist.
  ===> Fixed

## Verifiziert OK (kein Handlungsbedarf)

- `toMicroseconds()`/`toSeconds()` (time.h:140-154) rechnen konsistente Konstanten (3600000000ULL/60000000ULL/1000000ULL),
  keine Diskrepanz zwischen beiden Repräsentationen.
- `setFromSeconds()`/`setFromMicroseconds()` verwenden konsequent Modulo auf vorzeichenlosen Typen – da `Time` per
  Definition nie negativ ist, gibt es hier keine Rundungs-/Vorzeichenproblematik wie bei `TimeDelta`.
- Alle Vergleichsoperatoren basieren einheitlich auf `toMicroseconds()`, keine Inkonsistenz zwischen `==`/`<`/etc.
- `now()`/`utcNow()` verwenden `localtime`/`gmtime` konsistent mit der übrigen Codebasis (z.B. `TimeZone::fromLocalTime()`).
