# DateTime Review – Befunde

Review vom 2026-08-31, Scope: `include/pplib/types/datetime.h` + `src/types/DateTime.cpp`.
Für Konsistenzprüfung vollständig mitgelesen: `include/pplib/types/date.h` + `src/types/Date.cpp`,
`include/pplib/types/time.h` + `src/types/Time.cpp`, `include/pplib/types/timezone.h` + `src/types/TimeZone.cpp`,
`include/pplib/types/timedelta.h` + `src/types/TimeDelta.cpp`, `include/pplib/exceptions.h`.
Alle referenzierten Exceptions (`IllegalArgumentException`, `IllegalStateException`, `InvalidDateException`,
`InvalidFormatException`) sind in `exceptions.h` verifiziert. Verwendung von `DateTime` in `Variant.cpp`,
`AssocArray.cpp`, `Logger.cpp`, `dir.h` per grep geprüft (Details siehe unten).

## Bugs (kritisch)

- [ ] **Arithmetik-Operatoren sind `noexcept`, rufen aber eine werfende Funktion auf → `std::terminate()`** (`datetime.h:947-970`, `DateTime.cpp:449-451`)
  `toMicroseconds()` wirft `IllegalStateException`, wenn `isEmpty()==true`:
  ```cpp
  int64_t DateTime::toMicroseconds() const
  {
      if (isEmpty()) throw IllegalStateException("DateTime is invalid");
      ...
  ```
  `operator-(const DateTime&)`, `operator+(const TimeDelta&)`, `operator-(const TimeDelta&)`, `operator+=`, `operator-=`
  sind aber alle als `noexcept` deklariert und rufen `toMicroseconds()` intern auf:
  ```cpp
  TimeDelta operator-(const DateTime& other) const noexcept
  {
      return TimeDelta::fromMicroseconds(toMicroseconds() - other.toMicroseconds());
  }
  ```
  Konkretes Szenario:
  ```cpp
  DateTime a; // default, isEmpty() == true
  DateTime b;
  a - b;      // toMicroseconds() wirft IllegalStateException innerhalb einer noexcept-Funktion
              // -> std::terminate() wird aufgerufen, Programm stürzt sofort ab (kein catch möglich)
  ```
  Die Vergleichsoperatoren (`<`, `<=`, `==`, ...) sind dagegen korrekt NICHT `noexcept` und werfen sauber durch.
  Fix: `noexcept` von den vier arithmetischen Operatoren entfernen, oder `toMicroseconds()` intern
  (`noexcept`-fähig) über ein `bool tryToMicroseconds(int64_t&)`-Pattern absichern, oder in den Operatoren
  vorher `isEmpty()` prüfen und eine definierte (nicht-terminierende) Fehlerbehandlung wählen.

- [ ] **`setMicroseconds()` ist `noexcept`, ruft aber das werfende `Date::set()` auf → `std::terminate()`** (`DateTime.cpp:465-486`)
  Nachgetragen am 2026-09-05, aufgefallen bei der Arbeit am Date-Review (dortiges Finding "weekISO8601 wirft
  für Jahr 0"). Dieselbe Bug-Klasse wie beim vorherigen Punkt, aber eine andere Stelle:
  ```cpp
  DateTime& DateTime::setMicroseconds(int64_t epoch_microseconds, const TimeZone& tz) noexcept
  {
      ...
      civilFromDays(days, y, m, d);
      my_date.set(y, m, d);      // wirft IllegalArgumentException fuer y ausserhalb 0-9999
  ```
  `civilFromDays()` rechnet korrekt auch weit vor Jahr 0 bzw. nach 9999, `Date::set()` lehnt solche Jahre aber
  ab. Ein ausreichend großer negativer oder positiver `epoch_microseconds`-Wert (vor 0000-01-01 bzw. nach
  9999-12-31) lässt die Exception also aus einer `noexcept`-Funktion entkommen → sofortiger Prozessabbruch,
  nicht abfangbar. Praktisch reproduzierbar über jeden Epoch-Wert kleiner als -62167219200000000 µs.
  Fix: analog zum vorherigen Punkt – entweder `noexcept` entfernen, oder den Wertebereich vor dem
  `my_date.set()` prüfen und definiert behandeln (clampen oder Objekt leeren).

- [ ] **`setEpoch(0)` verwechselt den gültigen Unix-Zeitstempel 0 mit „leer“** (`DateTime.cpp:192-197`)
  ```cpp
  DateTime& DateTime::setEpoch(uint64_t time)
  {
      if (time == 0) {
          clear();
          return *this;
      }
      ...
  ```
  Epoch `0` ist ein vollkommen gültiger Zeitpunkt (1970-01-01 00:00:00 UTC), wird hier aber unbedingt als
  „Objekt leeren“ interpretiert – nirgends dokumentiert (weder bei `setEpoch` noch bei `setTime_t`).
  ```cpp
  DateTime dt;
  dt.setEpoch(0);
  dt.isEmpty();      // true  – sollte false sein, Zeitpunkt ist ja gesetzt
  dt.toString();      // "0000-00-00 00:00:00.000000" statt "1970-01-01 00:00:00.000000"
  ```
  Noch gravierender in Kombination mit dem Konstruktor/`set(uint64_t,uint32_t)`, die nach `setEpoch()` noch
  die Mikrosekunden setzen (`datetime.h:123-127, 257-262`):
  ```cpp
  DateTime dt(0, 500000);      // epoch=0, 500000 µs
  dt.isEmpty();                 // true  (my_date wurde von clear() genullt)
  dt.microsecond();             // 500000 (wird NACH clear() trotzdem gesetzt!)
  dt.toString();                 // "0000-00-00 00:00:00.500000" – inkonsistenter Zustand
  dt - DateTime();               // toMicroseconds() wirft wegen isEmpty()==true -> std::terminate() (s.o.)
  ```
  Fix: `setEpoch()` darf `time==0` nicht als Sonderfall behandeln, sondern muss das reguläre
  1970-01-01-Datum berechnen. „Leer“ sollte ausschließlich über `clear()`/den Default-Konstruktor erreichbar sein.

- [ ] **`setLongInt()`: `(uint16_t)`-Cast verstümmelt das Jahr für alle Jahre ≥ 5462** (`DateTime.cpp:238-254`)
  ```cpp
  int mm = (i % 12) + 1;
  int yy = (uint16_t)i / 12;   // <- Cast auf 16 Bit VOR der Division
  ```
  `i` ist an dieser Stelle `year*12 + (month-1)` (als `uint64_t`). Der Cast auf `uint16_t` schneidet alles
  oberhalb von 65535 ab, bevor durch 12 geteilt wird – für Jahre oberhalb von `65536/12 ≈ 5461` liefert das
  ein falsches Ergebnis. Konkret nachgerechnet für Jahr 9999 (laut Doku von `set(int year, ...)` gültig,
  `date.h`/`Date::set` erlaubt bis 9999):
  ```cpp
  DateTime dt;
  dt.set(9999, 1, 1, 0, 0, 0, 0);
  uint64_t li = dt.longInt();     // = 321375859200000000
  DateTime dt2;
  dt2.setLongInt(li);
  dt2.year();                     // liefert 4537 statt 9999!
  ```
  Rechnung: nach den Divisionen ist `i == 119988` (= 9999*12). `(uint16_t)119988 == 54452`
  (119988 mod 65536), `54452 / 12 == 4537`. Ohne den Cast (`i / 12`) käme korrekt `9999` heraus.
  Fix: Cast ersatzlos entfernen: `int yy = i / 12;` (i ist zu diesem Zeitpunkt bereits `uint64_t`, kein Cast nötig).

- [ ] **Implizite Narrowing-Conversion in `set(int,...)` hebelt die dokumentierte Bereichsprüfung aus** (`datetime.h:440-445`)
  ```cpp
  inline DateTime& set(int year, int month, int day, int hour = 0, int minute = 0, int sec = 0, int usec = 0)
  {
      my_date.set(year, month, day);   // Date::set(uint16_t, uint8_t, uint8_t)
      my_time.set(hour, minute, sec, usec); // Time::set(uint8_t, uint8_t, uint8_t, uint32_t)
      ...
  ```
  `Date::set`/`Time::set` prüfen zwar ihre Parameter (`Date.cpp:41-53`, `Time.cpp:79-89`), aber erst
  *nachdem* der `int`-Wert stillschweigend auf `uint8_t`/`uint16_t` gekürzt wurde. Ein Wert, der zufällig
  auf einen *gültigen* Wert umläuft, wird nicht mehr erkannt:
  ```cpp
  DateTime dt;
  dt.set(2024, 1, 287);   // day=287 ist klar ungueltig
  // uint8_t(287) == 31 (287 % 256), 31 <= daysInMonth(1,2024)==31 -> KEINE Exception!
  dt.day();                // 31 statt der dokumentierten IllegalArgumentException
  ```
  Gleiches Muster für Monat (`month=268` → `uint8_t(268)==12` → wird als Dezember akzeptiert) und für
  Stunden über `Time::set` (`hour=256` → `uint8_t(256)==0` → Mitternacht statt Exception).
  Fix: In `DateTime::set(int,...)` selbst vor der Weitergabe grob validieren (z.B. `year<0||year>9999`
  etc. prüfen und `IllegalArgumentException` werfen), statt sich auf den impliziten Cast zu verlassen.

- [ ] **`getRFC822Date()`: Zeitzonen-Offset wird als Sekunden statt Minuten interpretiert → falscher Wert** (`DateTime.cpp:149-165, 317-342`)
  `toPPLTIME()` schreibt den Offset in **Minuten** in die Struktur (unverändert von `TimeZone::offsetMinutes()`):
  ```cpp
  pt.gmt_offset = my_tz.offsetMinutes();   // Minuten, z.B. 60 für UTC+1
  ```
  `getRFC822Date()` behandelt denselben Wert aber wie einen **Sekunden**-Offset (klassisches `tm_gmtoff`-Idiom):
  ```cpp
  if (t.gmt_offset >= 0)
      s.appendf(" +%02i%02i", abs(t.gmt_offset / 3600), abs(t.gmt_offset % 3600));
  ```
  Für CET (UTC+1, `offsetMinutes()==60`): `60/3600==0`, `60%3600==60` → Ausgabe `" +0060"` statt korrekt
  `" +0100"`. Das betrifft praktisch jede Zeitzone außer UTC selbst – die generierten RFC-822-Datumsangaben
  (z.B. für Mail-Header) sind fast immer falsch.
  Zusätzlich ist `PPLTIME::gmt_offset` als `int8_t` deklariert (`datetime.h:58`), Minutenwerte bis ±840
  (siehe `timezone.h:49`) passen da gar nicht hinein – für Zonen wie UTC-5 (-300 Minuten) kommt zusätzlich
  ein Wrap-around dazu, der das Ergebnis noch weiter verfälscht.
  Fix: entweder `gmt_offset` konsequent in Sekunden führen (`pt.gmt_offset = my_tz.offsetSeconds();`, Feldtyp
  auf `int32_t` erweitern) oder in `getRFC822Date()` durch 60 statt 3600 teilen. Feldtyp in jedem Fall auf
  mindestens `int16_t` (besser `int32_t`) vergrößern.

- [ ] **`strftime()` liefert für Daten vor 1970 ein komplett falsches Ergebnis (1970-01-01, nicht das gespeicherte Datum)** (`DateTime.cpp:352-369, 167-190`)
  ```cpp
  String DateTime::strftime(const String& format) const
  {
      ...
      ::time_t tp = time_t();          // == epoch(), liefert 0 für Jahr < 1970 (dokumentiertes Verhalten von epoch())
      tp += my_tz.offsetSeconds();
      if (!safe_gmtime(tp, &tt)) throw InvalidDateException();
      ...
  ```
  `epoch()`/`time_t()` geben laut eigener Doku „0, wenn das Jahr vor 1970 liegt“ zurück. `strftime()` nutzt
  das aber als *einzige* Quelle für Datum und Uhrzeit – anders als `format()`/`get()`/`toString()`, die
  direkt mit `my_date`/`my_time` formatieren. Für jedes gültige (nicht-leere) DateTime vor 1970 liefert
  `strftime()` daher nicht das gespeicherte Datum, sondern den 1.1.1970 (ggf. verschoben um den TZ-Offset):
  ```cpp
  DateTime dt;
  dt.setDate(1950, 1, 1);
  dt.setTime(12, 0, 0);
  dt.strftime("%Y-%m-%d %H:%M:%S");   // liefert "1970-01-01 00:00:00" statt "1950-01-01 12:00:00"
  ```
  Fix: `strftime()` sollte – wie `format()`/`get()` – auf Basis der tatsächlichen Kalenderfelder eine
  `struct tm` befüllen (Jahr/Monat/Tag/Stunde/Minute/Sekunde direkt aus `my_date`/`my_time`), statt über
  den auf Jahr≥1970 beschränkten `epoch()`-Weg zu gehen.

## Bugs (mittel)

- [ ] **`longInt()` auf einem leeren/Default-DateTime erzeugt durch Unsigned-Underflow einen sinnlosen Riesenwert** (`DateTime.cpp:227-236`)
  ```cpp
  uint64_t r = my_date.year() * 12 + (my_date.month() - 1);
  ```
  Bei einem Default-`DateTime` ist `month()==0` (`uint8_t`). `0 - 1` wird als `int` zu `-1`, die Zuweisung
  an `uint64_t r` macht daraus `UINT64_MAX`. Alle folgenden Multiplikationen/Additionen rechnen auf dieser
  Basis weiter (wrapped, aber sinnlos):
  ```cpp
  DateTime dt;               // isEmpty() == true
  dt.longInt();               // riesiger, nicht nachvollziehbarer Wert nahe UINT64_MAX statt z.B. 0
  ```
  Im Gegensatz dazu wirft `toMicroseconds()` für den leeren Fall sauber `IllegalStateException`, und
  `epoch()` gibt (zufällig, weil `year()<1970`) `0` zurück – drei verschiedene Verhaltensweisen für denselben
  Fall „leeres DateTime“. Fix: `longInt()` sollte bei `isEmpty()` entweder `0` zurückgeben oder ebenfalls
  eine `IllegalStateException` werfen, statt mit Unsigned-Wraparound weiterzurechnen.

- [ ] **`DateTime::set(const String&)`: „-“ als Zeit-Trennzeichen (laut `Time::set` dokumentiert erlaubt) wird als Zeitzonen-Vorzeichen fehlinterpretiert** (`DateTime.cpp:78-142`)
  `Time::set()` erlaubt laut Doku (`time.h:196-206`) `-` als Alternative zu `:` als Trennzeichen. Die
  DateTime-eigene Zeitzonen-Extraktion `parse_time()` sucht aber blind nach einem `-` im verbleibenden
  String, *bevor* er an `Time::set()` weitergereicht wird:
  ```cpp
  tz_start = parse.find("-", -1);
  if (tz_start != String::npos) {
      tz = TimeZone::fromString(parse.substr(tz_start));
      parse = parse.substr(0, tz_start);
      return tz;
  }
  ```
  Konkret:
  ```cpp
  DateTime dt;
  dt.set("2024-01-01T12-30-00");   // "12-30-00" ist laut Time::set-Doku eine gültige Uhrzeit (12:30:00)
  // parse_time() findet das erste "-" in "12-30-00" (Position 2), reisst daraus eine "Zeitzone" "-30-00"
  // TimeZone::fromString("-30-00") wirft IllegalArgumentException("... invalid timezone format (30-00)")
  // -> DateTime::set() wirft statt korrekt 12:30:00 zu parsen
  ```
  Direkt aufgerufen (`Time t; t.set("12-30-00");`) funktioniert das Format dagegen anstandslos. Fix: entweder
  aus der Doku von `Time::set()` `-` als Trennzeichen streichen (Widerspruch zur DateTime-Nutzung), oder
  `parse_time()` das `-` nur als Zeitzone werten, wenn es in den letzten ~6 Zeichen liegt und einem
  Offset-Muster (`\d{2}:?\d{2}`) entspricht.

- [ ] **`TimeZone::toString()` verliert das Vorzeichen bei Offsets zwischen -59 und -1 Minuten** (`TimeZone.cpp:151-166`)
  ```cpp
  int hours = offset_minutes / 60;              // Integer-Division rundet Richtung 0
  int minutes = abs(offset_minutes % 60);
  ...
  snprintf(buffer, sizeof(buffer), "%+03d:%02d", hours, minutes);
  ```
  Für `offset_minutes == -30`: `hours = -30/60 == 0` (Trunkierung Richtung 0, nicht floor), `minutes = 30`.
  `"%+03d"` formatiert `0` immer mit `+` → Ausgabe `"+00:30"` statt korrekt `"-00:30"`. Wird u.a. von
  `DateTime::get("%z")` verwendet (`DateTime.cpp:344-350`), betrifft also direkt die DateTime-Formatierung
  für negative Halbstunden-Offsets nahe 0.
  Fix: Vorzeichen separat aus `offset_minutes < 0` ableiten und explizit voranstellen, statt es implizit
  über `hours` zu transportieren:
  ```cpp
  const char* sign = (offset_minutes < 0) ? "-" : "+";
  int hours = abs(offset_minutes) / 60;
  int minutes = abs(offset_minutes) % 60;
  snprintf(buffer, sizeof(buffer), "%s%02d:%02d", sign, hours, minutes);
  ```

- [ ] **`setEpoch()`/`epoch()`: lineare Jahr-Schleife statt geschlossener Formel – Performance-Falle bei großen Werten** (`DateTime.cpp:167-190, 192-225`)
  Beide Funktionen ermitteln das Jahr durch eine `while`-Schleife, die pro Iteration ein Jahr weiterzählt.
  `toMicroseconds()`/`setMicroseconds()` (`DateTime.cpp:424-486`) lösen exakt dasselbe Problem dagegen in
  O(1) über die klassische `daysFromCivil`/`civilFromDays`-Umrechnung. Für plausible Fehleingaben – z.B.
  eine versehentlich in Mikrosekunden statt Sekunden übergebene Epoche (`dt.setEpoch(1'700'000'000'000'000ULL)`,
  ein leicht zu verwechselnder Aufrufer-Fehler) – muss die Schleife viele Millionen Iterationen durchlaufen,
  bevor sie terminiert; mit `UINT64_MAX` sind es Größenordnungen von ~5·10^11 Iterationen (spürbarer Hänger
  bis zu mehreren Minuten). `setEpoch()` besitzt zudem keine Obergrenzenprüfung vor dem finalen
  `my_date.set(year, ...)`, das bei sehr großem `year` durch den `uint16_t`-Parameter ebenfalls umlaufen kann.
  Fix: `setEpoch()`/`epoch()` auf denselben `daysFromCivil`/`civilFromDays`-Mechanismus wie `toMicroseconds()`/
  `setMicroseconds()` umstellen; das behebt gleichzeitig die Performance- und die Umlauf-Problematik.

## Design

- [ ] **Zwei parallele, inkonsistente Epoch-Implementierungen** (`epoch()` vs. `toMicroseconds()`/`setMicroseconds()`)
  `epoch()`/`setEpoch()` (naive Jahres-Schleife, Jahr<1970 → `0`) und `toMicroseconds()`/`setMicroseconds()`
  (`daysFromCivil`/`civilFromDays`, voller Wertebereich inkl. negativer Jahre) berechnen dieselbe Sache mit
  unterschiedlichem Gültigkeitsbereich und unterschiedlicher Fehlerbehandlung (stille `0` vs. Exception vs.
  falsches Ergebnis in `strftime()`, siehe Bugs oben). Für Vergleichsoperatoren/Arithmetik wird ausschließlich
  `toMicroseconds()` verwendet, für `time_t()`/`longInt()`/`toPPLTIME()`/`strftime()` dagegen `epoch()` – zwei
  Werkzeuge mit unterschiedlichen Garantien für dieselbe Klasse. Empfehlung: einen einzigen, korrekten
  (Howard-Hinnant-artigen) Umrechnungsweg für alle epoch-basierten Funktionen verwenden.

- [ ] **`DateTime::set(const PPLTIME&)`: Dokumentierte Clamping-Semantik existiert nicht** (`datetime.h:326-336`, `DateTime.cpp:144-147`)
  Die Doku warnt explizit: „Gegenwärtig werden Werte ausserhalb des Gültigkeitsbereiches abgeschnitten!
  Aus dem Monat 0 oder -10 würde 1 werden, aus 13 oder 12345 würde 12 werden.“ Die Implementierung reicht
  die Rohwerte aber nur durch:
  ```cpp
  DateTime& DateTime::set(const PPLTIME& pt)
  {
      return set(pt.year, pt.month, pt.day, pt.hour, pt.min, pt.sec, 0);
  }
  ```
  `set(int,...)` → `Date::set`/`Time::set` werfen bei out-of-range Werten `IllegalArgumentException` (bzw.
  liefern durch das Narrowing-Problem oben teils stillschweigend falsche Werte) – es wird nirgends geclampt.
  Ein Aufrufer, der sich auf die dokumentierte Clamping-Semantik verlässt und keinen try/catch verwendet,
  bekommt eine unerwartete Exception. Fix: Doku korrigieren (Clamping-Hinweis entfernen) oder tatsächliches
  Clamping implementieren – je nachdem, welches Verhalten gewünscht ist.

- [ ] **`Date::operator=` gibt `Date` statt `Date&` zurück – Bruch mit der Standard-Zuweisungs-Konvention** (`date.h:86-103`)
  ```cpp
  Date operator=(const Date& other) noexcept { ...; return *this; }
  Date operator=(Date&& other) noexcept { ...; return *this; }
  ```
  Beide Operatoren geben eine Kopie statt einer Referenz zurück – inkonsistent zu `Time::operator=`
  (`time.h:223,232`) und dem defaulteten `TimeZone`/`DateTime`-Zuweisungsoperator, die korrekt `T&` liefern.
  Funktional (aktuell) unauffällig, weil `DateTime::operator=` selbst `=default` ist und den Rückgabewert von
  `my_date = other.my_date` nirgends weiterverwendet, aber es kostet eine unnötige Kopie pro Zuweisung und
  bricht mit `a = b = c;`/`(a = b).format(...)`-Idiomen, sobald `Date` isoliert verwendet wird.
  Fix: `Date& operator=(...)` statt `Date operator=(...)`.

## Doku / Kosmetik

- [ ] **`getRFC822Date()`: dokumentierte Exception existiert nicht** (`datetime.h:647`)
  ```
  * \exception Exception::FunctionFailed Die Funktion wirft eine Exception, wenn ...
  ```
  `Exception::FunctionFailed` ist in `exceptions.h` nicht vorhanden (vermutlich Altlast aus PPL7). Tatsächlich
  wirft die Funktion `IllegalStateException` (leeres Datum) bzw. `IllegalArgumentException`
  (`day_of_week`/`month` außerhalb des Bereichs). Doku-Kommentar entsprechend korrigieren.

- [ ] **`toPPLTIME()` hat überhaupt keinen Doku-Kommentar** (`datetime.h:907`)
  Einzige öffentliche Methode der Klasse ohne `@brief`/`@return` – im Kontrast zur sonst sehr ausführlichen
  Dokumentation aller anderen Methoden. Sollte mindestens erwähnen, dass `PPLTIME::epoch` für Daten vor 1970
  immer `0` ist (siehe `epoch()`-Doku) und `gmt_offset` in Minuten (nicht Sekunden) vorliegt.

- [ ] **`compareSeconds()`-Doku enthält PPL7-Altlast** (`datetime.h:817-827`)
  „Es wird kein Fehlercode gesetzt“ – Restformulierung aus der alten Fehlercode-API, bei `bool`-Rückgabewert
  ohne Bedeutung. Kann ersatzlos gestrichen werden.

- [ ] **`format()` dokumentiert kein `%z`, `get()` schon – bei Verwendung von `%z` in `format()` bleibt der Platzhalter unverändert im Ergebnis stehen**, da weder `Time::format()` noch `Date::format()` ihn kennen. Sollte in der Doku von `format()` kurz erwähnt werden (Verweis auf `get()` für Zeitzonen-Ausgabe).

## Verifiziert OK (kein Handlungsbedarf)

- `daysFromCivil()`/`civilFromDays()` (`DateTime.cpp:425-447`) implementieren korrekt den bekannten
  Howard-Hinnant-Algorithmus inkl. korrekter Behandlung negativer Jahre; `setMicroseconds()` rundet negative
  Tagesreste korrekt nach unten (`rem_us<0 → +=US_PER_DAY, days-=1`), kein Rundungsfehler bei Daten vor 1970.
- Self-Assignment (`dt = dt;`) ist unkritisch: `operator=(const DateTime&)`/`operator=(DateTime&&)` sind
  `=default`, alle Member (`Date`, `Time`, `TimeZone`) sind reine Wertetypen ohne eigene Ressourcenverwaltung,
  Move-Konstruktoren von `Date`/`Time` setzen die Quelle sauber auf 0 zurück.
  Vergleichsoperatoren (`<`,`<=`,`==`,`!=`,`>=`,`>`) sind absichtlich nicht `noexcept` und propagieren die
  `IllegalStateException` aus `toMicroseconds()` bei leeren Objekten sauber nach außen – im Gegensatz zu den
  fälschlich `noexcept` deklarierten Arithmetik-Operatoren (siehe Bugs).
- `getISO8601()`/`getISO8601withMsec()`/`getISO8601withUsec()` ermitteln das Vorzeichen des Zeitzonen-Offsets
  korrekt über `offsetMinutes() >= 0` (nicht über eine Division wie `TimeZone::toString()`), daher nicht vom
  „-00:30 wird +00:30“-Bug betroffen.
  `Variant`/`AssocArray`-Integration (`AssocArray.cpp:484-486, 793-794, 916-917`) behandelt ein „leeres“
  DateTime beim Export bewusst als Sonderfall (Kommentar „DateTime könnte invalid sein“) und verwendet dafür
  `getISO8601withMsec()`, die auf leeren Objekten nicht wirft (keine `isEmpty()`-Prüfung nötig) – konsistent
  zum dokumentierten Verhalten.
