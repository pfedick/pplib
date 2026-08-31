# TimeZone Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/timezone.h` + `src/types/TimeZone.cpp`.
Verifiziert gegen `include/pplib/exceptions.h`, `include/pplib/core/functions.h` (`Trim`, `UpperCase`),
`String::isNumeric()`/`toInt()` (src/types/String.cpp:168, 1212). Gegengeprüft mit `DateTime`
(include/pplib/types/datetime.h, src/types/DateTime.cpp) als tatsächlichem Konsumenten – insbesondere
`DateTime::epoch()` (src/types/DateTime.cpp:167-190, ältere Implementierung neben der neueren
`toMicroseconds()`/`setMicroseconds()`), die `TimeZone::offsetSeconds()` mit `Date`/`Time`-Werten kombiniert.

## Bugs (kritisch)

- [ ] **`setOffset()` / `set()` / Konstruktor `TimeZone(int8_t,int8_t,...)`: Vorzeichen wird nur aus `hours`
  abgeleitet, negative Sub-Stunden-Offsets sind damit nicht darstellbar** (`TimeZone.cpp:84-96`)
  ```cpp
  TimeZone& TimeZone::setOffset(int8_t hours, int8_t minutes)
  {
      int s = 1;
      if (hours < 0) s = -1;
      return setOffsetMinutes((abs(hours) * 60 + abs(minutes)) * s);
  }
  ```
  Ist `hours == 0` und nur `minutes` negativ, wird das Vorzeichen verloren:
  ```cpp
  TimeZone tz(0, -30);
  tz.offsetMinutes();   // liefert +30, nicht -30
  ```
  Die Schwester-Factory `fromHoursAndMinutes()` (`timezone.h:90-94`) macht es **richtig**:
  ```cpp
  int sign = (hours < 0 || minutes < 0) ? -1 : 1;   // prüft BEIDE Vorzeichen
  ```
  Zwei Codepfade derselben Klasse widersprechen sich bei genau demselben Anwendungsfall (Halbe-Stunde-Zeitzone
  westlich von UTC, z.B. `-00:30`). `setOffset()` und `set()` sind zudem wortidentisch (siehe Design unten).
  Fix: Vorzeichenermittlung wie in `fromHoursAndMinutes()` angleichen:
  ```cpp
  int s = (hours < 0 || minutes < 0) ? -1 : 1;
  return setOffsetMinutes((abs(hours) * 60 + abs(minutes)) * s);
  ```

- [ ] **`toString()` zeigt negative Sub-Stunden-Offsets mit falschem Vorzeichen** (`TimeZone.cpp:151-166`)
  ```cpp
  int hours = offset_minutes / 60;
  int minutes = abs(offset_minutes % 60);
  ...
  snprintf(buffer, sizeof(buffer), "%+03d:%02d", hours, minutes);
  ```
  Für `offset_minutes == -30` (direkt und korrekt über `setOffsetMinutes(-30)` erzeugbar, die einzige Range-Prüfung
  ist `-720..840`, `TimeZone.cpp:79`) liefert die Ganzzahldivision `hours = -30 / 60 == 0` (Rundung Richtung Null).
  `%+03d` gibt für `0` immer ein `+` aus:
  ```cpp
  TimeZone tz; tz.setOffsetMinutes(-30);
  tz.toString();   // "+00:30" – falsch, korrekt wäre "-00:30"
  ```
  Fix: Vorzeichen aus `offset_minutes` selbst ableiten, nicht aus dem bereits gerundeten `hours`:
  ```cpp
  char sign = (offset_minutes < 0) ? '-' : '+';
  int hours = abs(offset_minutes) / 60;
  int minutes = abs(offset_minutes) % 60;
  snprintf(buffer, sizeof(buffer), "%c%02d:%02d", sign, hours, minutes);
  ```

## Bugs (mittel)

- [ ] **`fromString()` erkennt `"UTC"`/`"Z"` nur groß geschrieben** (`TimeZone.cpp:108-116`)
  ```cpp
  String s = Trim(str);   // kein UpperCase() – anders als bei Date::set/Time::set/DateTime::set
  ...
  if (s == "Z" || s == "UTC" || ...) return TimeZone::utc();
  ```
  `TimeZone::fromString("utc")` oder `("z")` erkennt das Sonderformat nicht und fällt in den numerischen Zweig,
  wo es mangels Ziffern mit `IllegalArgumentException` scheitert – obwohl `Date::set()`, `Time::set()` und
  `DateTime::set()` an vergleichbarer Stelle konsequent `UpperCase(Trim(...))` verwenden. Wird `fromString()``
  direkt (nicht nur über `DateTime::set()`, das vorher schon uppercased) mit kleingeschriebenem Text aufgerufen,
  ist das Verhalten inkonsistent zum Rest der Bibliothek.
  Fix: `String s = UpperCase(Trim(str));`

## Design

- [ ] **`setOffset()` und `set()` sind identischer Code** (`TimeZone.cpp:84-89` und `91-96`, Deklaration `timezone.h:73-74`)
  Beide Methoden haben exakt denselben Body (inkl. des oben beschriebenen Vorzeichen-Bugs). Eine der beiden
  Methoden sollte die andere aufrufen oder ganz entfernt werden, um Doppelpflege/Doppel-Bugs zu vermeiden.

- [ ] **`tz_name`-Doku suggeriert Named-Zone-Unterstützung, die es nicht gibt** (`timezone.h:50`)
  ```cpp
  String tz_name; //!< Optional: "UTC", "CET", "CEST", "Europe/Berlin"
  ```
  `fromString()` kennt aber ausschließlich `"Z"`/`"UTC"` und numerische Offsets (`+02:00`, `+0200`). Es gibt keine
  Zeitzonen-Datenbank/kein Mapping von `"CET"`/`"Europe/Berlin"` auf einen Offset – `TimeZone::fromString("CET")`
  wirft eine `IllegalArgumentException`. `tz_name` ist aktuell ein reines Freitext-Label ohne Rückwirkung auf
  `offset_minutes`. Falls Named-Zones (mit DST-Regeln) nicht geplant sind, Doku entsprechend präzisieren; falls
  doch, fehlt hier die eigentliche Implementierung.

- [ ] **Vorzeichen-Typmix zwischen `TimeZone::offsetSeconds()` (signed) und `Time`/`Date`-Arithmetik (unsigned)
  begünstigt stillen `uint64_t`-Wraparound bei Konsumenten** (Beleg: `src/types/DateTime.cpp:189`)
  ```cpp
  // DateTime::epoch() – bereits vorhandener Konsument aus Date + Time + TimeZone
  return total_days * 86400 + my_time.toSeconds() - my_tz.offsetSeconds();
  ```
  `total_days` ist `uint64_t`, `Time::toSeconds()` liefert `uint32_t`, `TimeZone::offsetSeconds()` liefert
  `int32_t` (kann negativ **oder** positiv bis `+50400` sein). Der gesamte Ausdruck wird wegen `total_days`
  in `uint64_t`-Arithmetik ausgewertet. Für ein Datum mit `year >= 1970` (die einzige Vorprüfung in `epoch()`),
  aber einer positiven Zeitzone, die den lokalen Zeitpunkt vor die UTC-Epoche schiebt, entsteht ein Unterlauf:
  ```cpp
  DateTime dt; dt.set(1970, 1, 1, 0, 0, 0); dt.setTimeZone(TimeZone(14, 0)); // UTC+14
  dt.epoch();   // total_days=0, toSeconds()=0, offsetSeconds()=50400
                // 0 + 0 - 50400 als uint64_t -> 18446744073709501216 (Wraparound), NICHT 0
  ```
  Das ist kein Bug *in* `TimeZone` selbst (der eigene Typ `int32_t` ist korrekt gewählt), aber die Kombination aus
  vorzeichenbehaftetem Offset und vorzeichenlosen Tages-/Sekundenwerten ist eine Falle für jeden Konsumenten, der
  `Date`+`Time`+`TimeZone` von Hand verrechnet – demonstriert am bereits existierenden `DateTime::epoch()`.
  Die neuere `DateTime::toMicroseconds()`/`setMicroseconds()` (DateTime.cpp:449-486) umgeht das Problem korrekt,
  indem konsequent mit `int64_t` gerechnet wird. Empfehlung: bei zukünftiger Verwendung von `offsetSeconds()`
  in Kombination mit `Date`/`Time`-Werten immer zuerst auf `int64_t` heben, wie es `toMicroseconds()` bereits tut.

## Doku / Kosmetik

- [ ] `setOffsetSeconds()` (`timezone.h:68-71`) rundet mit Ganzzahldivision Richtung Null (`offset_seconds / 60`),
  d.h. Sekundenanteile <60 gehen kommentarlos verloren (z.B. `setOffsetSeconds(-90)` ergibt `-1` statt der
  "korrekten" `-1.5` Minuten). Für praktisch alle real existierenden Zeitzonen (Minutenraster) irrelevant, aber
  nicht dokumentiert.

## Verifiziert OK (kein Handlungsbedarf)

- `setOffsetMinutes()` ist die einzige Stelle, die den gültigen Bereich (-720..840) durchsetzt (`TimeZone.cpp:79`)
  – alle anderen Setter/Konstruktoren laufen letztlich darüber, der Bereichs-Check selbst ist konsistent.
- `fromString()` extrahiert das Vorzeichen korrekt **vor** dem Aufsplitten in Stunden/Minuten (`TimeZone.cpp:121-127`)
  und kann dadurch anders als `setOffset()`/`set()`/der Zwei-Parameter-Konstruktor sehr wohl `-00:30`-artige
  Eingaben (z.B. `"-0030"`) korrekt in einen negativen Offset umrechnen – bestätigt, dass der oben gemeldete
  Vorzeichen-Bug spezifisch die numerischen (`int8_t,int8_t`)-Pfade betrifft, nicht das String-Parsing.
- Vorzeichen-Konvention ("Offset = lokale Zeit minus UTC") wird von `DateTime::toMicroseconds()`/`setMicroseconds()`
  konsistent in beide Richtungen verwendet (`UTC = lokal - offset` bzw. `lokal = UTC + offset`) – keine
  Diskrepanz zwischen Hin- und Rückrichtung gefunden.
- `get_local_utc_offset_minutes()` (TimeZone.cpp:42-69) behandelt glibc/BSD (`tm_gmtoff`) und den portablen
  Fallback (Windows/MSVCRT über `mktime`-Differenz) sauber getrennt über Compile-Time-Checks.
- `fromLocalTime()`/`fromEpoch()` liefern konsistent den Offset zu einem bestimmten Zeitpunkt (inkl. DST zu diesem
  Zeitpunkt), nicht eine "immer gültige" Zeitzonenregel – das ist eine bewusste Design-Grenze (kein
  IANA-Zeitzonen-Modell mit historischen DST-Übergängen), aber in sich konsistent mit dem Rest der Klasse (reiner
  fester Offset + Freitext-Name).
