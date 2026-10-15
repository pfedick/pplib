# DESIGN: pplib::Yaml (Parser und Writer)

## Zweck

Statische Klasse `pplib::Yaml` zum Laden und Speichern von YAML-Daten in
pplib::Variant / AssocArray / VariantArray – analog zu `pplib::Json`.

## Nicht-Ziele

- Keine YAML-Schemas, keine Knoten-Farben, keine Streaming-Events
- Keine Merge-Keys (`<<:`), keine komplexen Mapping-Keys (`? `)
- Keine Multi-Dokumente in einem Stream (zweites `---` nach Start => Fehler)
- Kein `pp()` (YAML ist ohnehin zeilenbasiert; `dumps(data, indent)` genügt)
- Keine Alias-Auflösung von Anker-Zirkularität (Aliase zeigen auf bereits
  fertig geparste Werte; Vorwärts-/Zirkularbezug => Fehler)

## Öffentliche Grenzfäche (include/pplib/core/yaml.h)

Exakt analog zu Json:

- `static void loads(Variant&, const String&)` / `load(Variant&, FileObject&)`
- `static Variant loads(const String&)` / `load(FileObject&)`
- `static void loads(AssocArray&, const String&)` / `load(AssocArray&, FileObject&)`
- `static void loads(VariantArray&, const String&)` / `load(VariantArray&, FileObject&)`
- `static void dumps(String&, const Variant&, int indent = 2)` /
  `dump(FileObject&, const Variant&, int indent = 2)` /
  `static String dumps(const Variant&, int indent = 2)`
- Entsprechende Überladungen für AssocArray und VariantArray

Default-Indent ist 2 (YAML-Konvention), nicht -1: YAML braucht Zeilenumbrüche;
`indent = -1` wäre nicht darstellbar. `indent >= 0` steuert die Einrückungsbreite.

## YAML-Unterstützung (Parser)

- Block-Mappings und Block-Sequenzen mit Einrückung (Trennung über Leerzeichen,
  Tabs in der Einrückung => Fehler)
- Sequenz-Einträge auf gleicher Ebene wie der Parentschlüssel (gängiger Stil)
- Flow-Sammlungen `[...]`, `{...}` (auch mehrzeilig)
- Skalars: plain, doppelt- und einfachquotiert (inkl. Weiterführung über
  mehrere Zeilen), Block-Scalars `|`, `|-`, `|+`, `>`, `>-`, `>+` (chomping indicators)
- Kommentare (`#` ganz- und zeilenende), Leerzeilen
- Dokument-Marker `---` am Anfang, `...` am Ende
- Anker/Aliase: `&name` / `*name`
- Tags: `!!str`, `!!int`, `!!float`, `!!bool`, `!!null`, `!!binary` (Base64 =>
  ByteArray), `!!timestamp` (=> Date oder DateTime), `!!seq`, `!!map`,
  pplib-eigene: `!Time`, `!TimeDelta`, `!TimeZone`
- Unbekannte Tags (z. B. `!Foo`) werden ignoriert und der Wert wird normal
  typaufgelöst (robust gegenüber Fremd-YAML). Kurzformen ohne `!` (`!str`,
  `!int`, `!float`, `!bool`, `!null`, `!binary`, `!timestamp`) werden ebenfalls
  akzeptiert.
- `!!binary`/`!binary`: Leerraum (inkl. Zeilenumbrüche aus Block-Scalar-Faltung)
  wird vor der Base64-Dekodierung entfernt. Padding (`=`) ist nur am Ende des
  Stroms erlaubt; Padding mitten im Strom => `TypeConversionException`
  (`ByteArray::fromBase64` bricht am ersten `=` und würde sonst uninitialisierte
  Bytes liefern).
- Fortsetzung plain-quotierter Skalars über mehrere Zeilen endet, sobald die
  nächste Zeile wie ein Sequenz-/Mapping-Eintrag aussieht (`- ...` oder
  `key: ...`); solch eine Zeile wird nicht in den Skalar gezogen.
- Scalar-Typauflösung ohne Tag (YAML-1.1-Kern):
  - `null`, `~`, leer => Variant(nullptr)
  - `true/True/TRUE/false/False/FALSE` => bool
  - Integer (dezimal inkl. Vorzeichen, `0x`-Hex, `0o`-Oktal, `_`-Gruppierung)
    => int64_t; Overflow => double
  - Float (Dezimalzahl mit `.`/Exponent, `.inf`, `-.inf`, `.nan`) => double
  - `YYYY-MM-DD` => pplib::Date
  - `YYYY-MM-DD[T ]HH:MM:SS[.frac][Z|±hh:mm]` => pplib::DateTime
  - `HH:MM:SS[.frac]` => pplib::Time
  - sonst => String
  - Quotierte Skalars sind immer String; `!!str` erzwingt String.

## Ownership und Speicherverwaltung

- Keine statischen oder globalen mutablen Zustände.
- Parser-Kontext (`anchors`-Map und Zeilenzähler) ist ein lokales Objekt
  (`YamlParseCtx`), das explizit durch alle Parserfunktionen gereicht wird.
- Reader puffert 4096 Bytes (wie JsonReader); es wird immer nur die aktuelle
  Zeile + Lookahead einer Zeile gehalten, nicht die gesamte Datei.
- `loads(String)` wrappt wie Json in MemFile (nur dort wird der String komplett
  gehalten).

## Fehlermodell

Gleiche Exceptions wie Json, alle mit Zeilennummer im Text:

- `UnexpectedEndOfDataException` – vorzeitiges Ende (unvollständiger Block,
  offene Flow-Klammer, `...` ohne Inhalt, leere Eingabe)
- `UnexpectedCharacterException` – Syntaxfehler (falsche Einrückung, Tab in
  Einrückung, unbekannte Escape-Sequenz, Alias auf unbekannten Anker, zweites
  `---`)
- `CharacterEncodingException` – ungültige `\u`/`\U`-Codepoints, Lone-Surrogate
- `IllegalArgumentException` – ungültige Datums-/Zeit-/Base64-Werte in Tags
- `TypeConversionException` – Wurzeltyp passt nicht zum Zielparameter
- `UnsupportedDataTypeException` – Variant-Typ nicht schreibbar
- `OverflowException` – Schachtelungstiefe > 128

## Thread-Safety

Alle Funktionen stateless bis auf lokale Objekte => beliebig parallel aufrufbar.

## Plattform-Grenzen

- Windows/MinGW, Linux, FreeBSD: identischer Code, keine POSIX-API.
- Mikrocontroller: Streaming-Reader (4 KB Puffer), kein Regex;
  `std::from_chars`/`to_chars` wie in Json.cpp.
- `wchar_t`-Unabhängigkeit: WideString wird wie in Json über UTF-8-Bytes behandelt.

## Serialisieren (Writer)

- AssocArray/VariantArray/Array/Variant wie Json, aber Blockstil mit `indent`
  Leerzeichen pro Ebene; leere Sammlungen inline (`{}` / `[]`).
- Skalar-Emission:
  - String: plain wenn eindeutig und ohne Sonderzeichen/Zeilenenden, sonst
    doppeltquotiert mit Escapes (`\n`, `\t`, `\b`, `\f`, `\r`, `\"`, `\\`,
    `\u00xx` für < 0x20). Strings, die wie null/bool/number/datum aussehen,
    werden quotiert (Round-Trip-Sicherheit). Leading/Trailing-Leerraum => quotiert.
    Enthaltenes `'` oder `"` => quotiert. Unterscheidung: *Keys* werden nur auf
    strukturelle Sicherheit geprüft (der Parser liest plain Keys immer als
    String), *Werte* zusätzlich auf Typsicherheit (sonst `2021-01-02` => Date).
  - WideString => UTF-8-Bytes, wie String behandelt
  - int64 => Dezimal; double => `to_chars`; NaN => `.nan`, ±Inf => `.inf`/`-.inf`
  - true/false/null
  - ByteArray/ByteArrayPtr => `!!binary ` + Base64 (wird beim Parsen wieder zu
    ByteArray => verlustfreier Round-Trip). Leere Base64 => `!!binary ""`
    (ein leeres unquotiertes `!!binary` ist ungültig und wirft beim Parsen).
    Base64 bis 76 Zeichen (BASE64_LINE_WIDTH) inline auf einer Zeile; längere
    Werte als gefalteter Block-Scalar `|-` mit Umbruch nach jeweils 76 Zeichen
    (76 % 4 == 0, dadurch bleibt der aneinandergereihte Base64-Strom gültig).
  - DateTime => `!!timestamp <ISO8601withUsec>`; Date => `!!timestamp <YYYY-MM-DD>`
    (Date bleibt beim Lesen Date, DateTime bleibt DateTime – Round-Trip)
  - Time => `!Time <HH:MM:SS[.usec]>`, TimeDelta => `!TimeDelta <toString()>`,
    TimeZone => `!TimeZone <toString()>`
- Keys: wie Strings behandelt (quotiert bei Bedarf).
- Mehrzeilige Strings: Block-Scalar `|-` wenn jede Zeile ohne führendes oder
  abschließendes Leerzeichen/Tab/Steuerzeichen auskommt; Leeren Zeilen wird kein
  Indent-Leerzeichen vorangestellt (Round-Trip ohne Sinneseinbuße). Sonst
  Rückfall auf doppeltquotiert mit `\n`-Escapes.
- Verschachtelte Werte unter Schlüsseln: Blockstil; leere Sammlungen inline.
- Block-Scalar-Inhalte (auch `!!binary`-Faltung) erhalten einen eigenen
  Inhalts-Indent: `keyColumn + (indent > 1 ? indent : 2)`. Der Parser verlangt,
  dass Block-Scalar-Inhalte mindestens zwei Spalten tiefer stehen als das
  Mutter-Indikator (`|`, `>` oder `- key:`), deshalb wird `keyColumn` durch
  `writeMapEntry`/`writeValueForKey` durchgereicht.

## Offene Punkte

- Mehrere Dokumente (`---` im Stream) werden nicht unterstützt: das erste
  Dokument wird gelesen, der Rest ignoriert.
- Anker/Aliase (`&a` / `*a`): Alias wird als flache Kopie des Zielknotens
  aufgelöst, nicht als Referenz (keine zirkulären Strukturen möglich).
- `!!seq`/`!!map`-Tags auf dem Wert eines Schlüssels werden verworfen; die
  folgende Blockstruktur bestimmt den Typ.
- Verschachtelte Sammlungen direkt als Sequenzelement (`-` in eigener Zeile)
  werden bei `indent == 0` geschrieben, aber vom Parser nicht mehr akzeptiert
  (`UnexpectedCharacterException`). Ab `indent >= 1` ist der Round-Trip korrekt.
  Bestehende Einschränkung des Writers, unabhängig von der Base64-Faltung.
- (alle übrigen Punkte auf "pplib-Pragmatismus" entschieden: unbekannte Tags
  ignorieren, keine Merge-Keys, Depth-Limit 128)
