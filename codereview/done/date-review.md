# Date Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/date.h` + `src/types/Date.cpp`.
Verifiziert gegen `include/pplib/exceptions.h` (nur existierende Exceptions verwendet), `include/pplib/core/functions.h`
(`Trim`, `UpperCase`), `String::isNumeric()`/`toInt()` (src/types/String.cpp:168, 1212 – `toInt()` liefert `int` via `strtol`,
kein Bounds-Check). Gegengeprüft mit dem tatsächlichen Konsumenten `DateTime` (include/pplib/types/datetime.h,
src/types/DateTime.cpp), der `Date` bereits mit `Time`/`TimeZone` kombiniert.

## Bugs (kritisch)

- [ ] **`Date::set(const String&)`: Narrowing-Cast vor der Bereichsprüfung erlaubt stillschweigend ungültige Monate/Tage** (`Date.cpp:69-77`)
  ```cpp
  if (a[0].length() == 4) {
      // YYYY.MM.DD
      return set((uint16_t)a[0].toInt(), (uint8_t)a[1].toInt(), (uint8_t)a[2].toInt());
  }
  ```
  `a[1].toInt()`/`a[2].toInt()` liefern `int`, werden aber schon beim Funktionsaufruf auf `uint8_t` runtergecastet –
  die eigentliche Bereichsprüfung in `set(uint16_t,uint8_t,uint8_t)` (Date.cpp:41) sieht nur noch den bereits
  umgewickelten Wert:
  ```cpp
  Date d;
  d.set("2024.268.03");   // Monat "268" -> (uint8_t)268 = 12  -> Ergebnis: 2024-12-03, KEINE Exception
  d.set("2024.01.287");   // Tag   "287" -> (uint8_t)287 = 31  -> Ergebnis: 2024-01-31, KEINE Exception
  ```
  Statt der erwarteten `IllegalArgumentException` für offensichtlich unsinnige Eingaben entsteht ein still
  falsches, aber gültig aussehendes Datum. Nur Werte, die "unglücklich" in den gültigen Bereich zurückwickeln,
  sind betroffen – andere große Werte (z.B. Tag "2024") werden weiterhin korrekt abgefangen, weil sie nach dem
  Wrap immer noch außerhalb 1-31 liegen.
  Fix: Länge/Wert vor dem Cast prüfen, z.B.
  ```cpp
  if (a[1].length() > 2 || a[2].length() > 2) {
      throw IllegalArgumentException("Date::set: invalid date format (%s)", d.c_str());
  }
  ```
  oder generell erst als `int` validieren (`1<=m<=12`, `1<=day<=31`) und danach erst auf `uint8_t` casten.

  ==> Fixed. Wir verwenden jetzt Integer als Datentyp für Year, Month und Day im Konstruktur und im Setter

- [ ] **`Date::fromInt()` umgeht jede Validierung von `set()`** (`date.h:122-129`)
  ```cpp
  static Date fromInt(uint32_t date)
  {
      Date d;
      d.yy = date / 10000;
      d.mm = (date / 100) % 100;
      d.dd = date % 100;
      return d;
  }
  ```
  Im Gegensatz zu jedem anderen Weg, ein `Date` zu erzeugen (Konstruktor, `set()`), prüft `fromInt()` weder
  Monat (1-12) noch Tag (1-`daysInMonth`). Damit lässt sich ein Objekt erzeugen, das die Klassen-Invariante
  verletzt:
  ```cpp
  Date d = Date::fromInt(20241340);   // yy=2024, mm=13, dd=40 – kein Fehler
  d.toString();      // "2024-13-40" – Garbage-Output, keine Exception
  d.dayOfYear();      // Schleife "for(m=1;m<mm;m++) daysInMonth(m,yy)" läuft m=1..12 (alles gültig)
                       // -> liefert 406, ein Tag, den es im Jahr nicht gibt, statt zu werfen
  ```
  `isEmpty()` (prüft nur `dd==0`) erkennt diesen Zustand nicht, da `dd=40 != 0`.
  Fix: `fromInt()` über `set()` implementieren, damit dieselbe Validierung greift:
  ```cpp
  static Date fromInt(uint32_t date)
  {
      return Date((uint16_t)(date / 10000), (uint8_t)((date / 100) % 100), (uint8_t)(date % 100));
  }
  ```

  ==> FIXED, wir rufen jetzt den Konstruktor von Date auf, wodurch die Werte validiert werden

- [ ] **Self-Move-Assignment zerstört das Datum** (`date.h:94-103`)
  ```cpp
  Date operator=(Date&& other) noexcept
  {
      dd = other.dd; mm = other.mm; yy = other.yy;
      other.dd = 0; other.mm = 0; other.yy = 0;   // <- other IST *this bei Self-Move
      return *this;
  }
  ```
  ```cpp
  Date d(2024, 5, 17);
  d = std::move(d);      // dd/mm/yy werden erst kopiert (no-op), dann auf 0 gesetzt
  assert(d.isEmpty());   // true – Datum ist weg
  ```
  Self-Move ist selten explizit im Anwendungscode, aber ein bekannter Stolperstein bei generischem/Template-Code
  (z.B. Swap-Implementierungen, Reallocation-Pfade). Analog zu Time (siehe time-review.md).
  Fix: Guard `if (this != &other)` oder Self-Move separat behandeln.

  ==> FIXED, Guard eingebaut

## Bugs (mittel)

- [ ] **`Date::week()` ist off-by-one gegenüber der eigenen Dokumentation** (`Date.cpp:178-189`, Doku `date.h:195-206`)
  Header-Doku: *"Am 1. Januar beginnt stets – unabhängig vom Wochentag – die 1. Kalenderwoche"*. Die Implementierung:
  ```cpp
  int doy = dayOfYear() - 1; // 0-basiert
  return (doy + jan1_wday) / 7;
  ```
  liefert für den 1. Januar (`doy=0`) immer `jan1_wday/7`, was für **jedes** Jahr `0` ergibt, sobald `jan1_wday<7`
  (immer der Fall) – nicht `1`, wie dokumentiert:
  ```cpp
  Date(2024, 1, 1).week();  // liefert 0, dokumentiert ist 1
  ```
  Das ist ein klassisches "POSIX `%U`"-Verhalten (Wochen vor dem ersten vollen 7er-Block = Woche 0), widerspricht
  aber explizit dem selbst formulierten Vertrag der Funktion.
  Fix: entweder `+ 1` ergänzen (`return (doy + jan1_wday) / 7 + 1;`), falls 1-basiert gewollt ist, oder die
  Doku korrigieren, falls 0-basiert (POSIX-`%U`-kompatibel) tatsächlich beabsichtigt ist.

  ==> FIXED, aber der Befund war ungenau: Gewollt ist `%U`-Verhalten. Gegen die echte `%U`-Definition
  durchgerechnet war die alte Formel für 6 der 7 möglichen Wochentage des 1. Januar bereits korrekt –
  falsch war sie nur, wenn der 1. Januar selbst ein Sonntag ist (dann kollabiert `(7-jan1_wday)%7` auf 0
  und das Ergebnis ist um 1 zu klein). Das im Review genannte Beispiel `Date(2024,1,1).week()==0` war
  sogar korrekt (1.1.2024 war ein Montag, liegt nach `%U` also zu Recht in Woche 0) – falsch war die
  *Doku* ("Am 1. Januar beginnt stets die 1. Kalenderwoche"), die eine andere Zählweise beschreibt.
  Das vorgeschlagene pauschale `+1` hätte die 6 korrekten Fälle kaputt gemacht.
  Umgesetzt: Formel auf `(dayOfYear()-1 - dayOfWeek() + 7) / 7` umgestellt (braucht kein `Date(yy,1,1)`
  Hilfsobjekt mehr und ist für alle Wochentage korrekt), Doku auf die echte `%U`-Regel korrigiert.
  Tests: `weekYearStartingOnSunday` (2023) und `weekYearNotStartingOnSunday` (2021) ergänzt, die
  Erwartung für `Date(0,1,1)` von 0 auf 1 korrigiert (dieser Tag ist laut `dayOfWeek()` ein Sonntag).

- [ ] **`Date::weekISO8601()` wirft für Jahr 0 statt korrekt zu rechnen (uint16_t-Unterlauf)** (`Date.cpp:205-208`)
  ```cpp
  if (thu_doy < 1) {
      Date prev_year_dec31(yy - 1, 12, 31);
      return prev_year_dec31.weekISO8601();
  }
  ```
  `Date::set()` erlaubt `year == 0` (nur `year > 9999` wird geprüft, `date.h:71-74`/`Date.cpp:43`), ein `Date(0,1,1)`
  ist also ein gültiges Objekt. Fällt der Donnerstag der ersten Januarwoche ins Vorjahr, wird `yy - 1` für `yy==0`
  als `uint16_t`-Parameter des `Date`-Konstruktors zu `65535` (Wraparound bei der impliziten Konvertierung von
  `int(-1)` nach `uint16_t`). `set()` prüft dann `year > 9999` → `65535 > 9999` → `IllegalArgumentException`,
  obwohl das Ausgangsobjekt `Date(0,1,1)` selbst gültig war.
  Fix: entweder `Date`-Konstruktion mit `int`-Jahr intern erlauben, oder `year==0` in `set()` von vornherein
  ablehnen (dann wäre der Fall unerreichbar), oder explizit auf `yy==0` prüfen und einen Sentinel-Wert/Exception
  mit klarer Meldung verwenden statt des zufälligen Wraparounds.

  ==> FIXED. Zwischenstand: Durch die Umstellung auf `int`-Parameter (siehe erstes Finding) gab es schon
  keinen 65535-Wraparound mehr, sondern eine saubere `IllegalArgumentException` – das Symptom blieb aber.
  Der Weg "Jahr 0 verbieten" wurde geprüft und wieder verworfen: `DateTime` dokumentiert (datetime.h:431)
  und testet (datetime.cpp:1152) Jahr 0 bewusst, und `DateTime::setMicroseconds()` ist `noexcept` – eine
  Exception aus `Date::set()` heraus hätte dort `std::terminate()` ausgelöst.
  Umgesetzt stattdessen:
  1. `weekISO8601()` konstruiert für das Vorjahr kein `Date`-Objekt mehr (das wäre für `yy==0` das nicht
     darstellbare Jahr -1), sondern bestimmt dessen letzte ISO-Woche rechnerisch: 53 Wochen genau dann,
     wenn der 31.12. des Vorjahres ein Donnerstag ist, oder ein Freitag und das Vorjahr ein Schaltjahr war.
  2. Dabei fiel auf, dass `dayOfWeek()` für Jahr 0 ohnehin falsch rechnete: Sakamoto braucht Floor-Division,
     C++ trunkiert aber Richtung Null, was für `mm < 3` und `yy == 0` (also `y == -1`) danebengeht. Der
     1.1.0000 war ein Samstag, die Klasse lieferte Sonntag. Behoben durch `int y = yy + 400;` – ein voller
     Gregorianischer Zyklus (146097 Tage = exakt 20871 Wochen) ändert den Wochentag nicht, hält `y` aber
     positiv. Für alle Jahre >= 1 bleiben die Ergebnisse dadurch unverändert.
  Tests: untere Bereichsgrenze in `week`/`weekISO8601` ergänzt (1.1.0000 = Samstag → Woche 0 bzw. ISO-Woche
  52 des Jahres -1; 1.1.0001 = Montag → Woche 0 bzw. ISO-Woche 1) und in `setWithWrongValues` abgesichert,
  dass Jahr 0 gültig, ein negatives Jahr aber ungültig ist.

## Design

- [ ] **`Date::operator=` gibt `Date` (per Wert) statt `Date&` zurück – inkonsistent mit `Time::operator=`** (`date.h:86-103`)
  ```cpp
  Date operator=(const Date& other) noexcept { ... return *this; }
  Date operator=(Date&& other) noexcept { ... return *this; }
  ```
  Beide Operatoren kopieren `*this` in einen Rückgabewert. Funktional bleibt `a = b;` korrekt, aber:
  - jede Zuweisung erzeugt eine unnötige zusätzliche Kopie,
  - Verkettungen mit Referenzsemantik brechen: `(a = b).clear();` löscht die *temporäre Kopie*, nicht `a` selbst,
  - inkonsistent zur Schwesterklasse `Time`, deren `operator=` korrekt `Time&` zurückgibt (Time.cpp:147, 156).
  Fix: Rückgabetyp auf `Date&` ändern (Standard-Idiom für Zuweisungsoperatoren).

  ==> FIXED, die Operatoren geben jetzt eine Referenz zurück

## Doku / Kosmetik

- [ ] Auskommentierter `isEmpty()`-Sonderfall in `toString()` und `format()` (`Date.cpp:82-86`, `92-96`) – unklar, ob ein
      leeres `Date` einen leeren String oder `"0000-00-00"` liefern soll. Aktuell wird immer `"0000-00-00"` erzeugt,
      auch für ein per Default-Konstruktor erzeugtes, leeres Datum. Entweder Kommentar entfernen (Entscheidung: immer
      formatieren) oder Sonderfall aktivieren.
  ==> Auskommentierten Code entfernt, Dokumentiert, dass "0000-00-00" zurückgegeben wird, wenn Date leer ist
- [ ] Doku von `week()`/`weekISO8601()` (`date.h:195-207`) hat einen Copy-Paste-Fehler: `@return Integer-Wert mit dem Jahr`
      – gemeint ist die Wochennummer, nicht das Jahr.

## Verifiziert OK (kein Handlungsbedarf)

- `Date::dayOfWeek()` implementiert Sakamotos Algorithmus korrekt (Standard-Tabelle, `y-=1` für Jan/Feb, `int y`
  vermeidet den `uint16_t`-Unterlauf, der `weekISO8601()` betrifft).
- `Date::isLeapYear(int year)` setzt die Gregorianische Regel (400/100/4) korrekt um.
- `Date::weekISO8601()` verwendet den korrekten "Donnerstag-der-Woche"-Trick für ISO-8601-Wochen (abgesehen vom
  Jahr-0-Randfall oben).
- `daysInMonth()`/`isLeapYear()` sind konsistent und werden überall (auch in `DateTime::epoch()`/`setEpoch()`)
  identisch verwendet – keine widersprüchliche Schaltjahr-Logik zwischen den Klassen gefunden.
- Vergleichsoperatoren basieren konsistent auf `toInt()`, keine Diskrepanz zwischen `==`/`<`/etc.
