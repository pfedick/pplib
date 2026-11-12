# Drawable-Tests — Design und Entscheidungen

Stand: 2026-11-12. Grundlage: Code-Analyse von `src/grafix/Drawable{,1,8,16,32,Default,Lines}.cpp`,
`include/pplib/grafix/drawable.h`, `Grafix::getGrafixFunctions()`.

## Zweck

Testsuite für `Drawable` über alle Pixelformate, als Voraussetzung für die spätere
Assembler-Portierung der Blt-Funktionen (A8R8G8B8). Keine neuen Feature-Tests für Linien/Formen
in diesem Schritt — `line/lineAA/elipse/floodFill/colorGradient` folgen in einem eigenen Durchgang.

## Verifizierte Architektur (Kontext für neue Sessions)

- `Grafix::getGrafixFunctions(format)`: kopiert `memcpy` die Default-Tabelle aus
  `getDefaultGrafixFunctions()` (Basis = **nop**-Struktur!), dann überschreiben
  `initDrawable{32,16,8,1}(fn, format)` je nach `bitdepth()`+`format()`. Ergebnis wird lazy
  in statischen Array `drawable_functions[MaxIdentifiers]` in `src/grafix/Grafix.cpp` gecacht.
- Default-Fallbacks (`default*` in `DrawableDefault.cpp`) sind fast alle NUR über die 4
  Pflicht-Slots (`PutPixel/GetPixel/ToNativeColor/FromNativeColor`) komponiert.
  Ausnahmen: `defaultBltChromaKey` und `defaultBltBackgroundOnChromaKey` sind **leere Stubs**.
- 16-Bit-Zugriffe rechnen `pitch >> 1`, 32-Bit `pitch >> 2`; Pitch ist in BYTES.
- Engine: `GetGrafix()` wirft `NoGrafixEngineException`; eigener Konstruktor wirft
  `DuplicateGrafixEngineException`. In `test_all` (alle Suiten ein Binary!) deshalb genau
  EINE Engine-Instanz via `inline`-Meyers-Singleton im Test-Kit.

## Format-Abdeckung (Ist-Zustand, verifiziert)

| Bittiefe | abgedeckt |NICHT abgedeckt (fallen auf nop zurück) |
|---|---|---|
| 1  | Monochrome1BitVertical | – |
| 8  | GREY8 | Palette, A8, R3G3B2 |
| 16 | R5G6B5, B5G6R5, X1R5G5B5, X1B5G5R5, A1R5G5B5, A1B5G5R5, X4R4G4B4, X4B4G4R4, A4R4G4B4, A4B4G4R4, A8R3G3B2 | – |
| 24 | – | R8G8B8, B8G8R8 (kein initDrawable für bitdepth()==24) |
| 32 | A8R8G8B8, A8B8G8R8, R8G8B8A8, B8G8R8A8, X8R8G8B8, X8B8G8R8, R8G8B8X8, B8G8R8X8 | – |

RGBBlend255 nativ: GREY8, alle 16-Bit, A8R8G8B8/A8B8G8R8 (A8X8X8X8-Variante),
R8G8B8A8/B8G8R8A8 (X8X8X8A8-Variante). Die X8-32-Bit-Formate nutzen defaultRGBBlend255.
Alle Blt-Natives existieren nur für 32-Bit (16-Bit wiederverwendet die 32-Bit-Versionen).
DrawRect/Clear/Xchange/Invert/Negativ haben nirgends eine native Variante.

## Beschlossene Entscheidungen

1. **Ununterstützte Formate werfen.** Sentinel = `nopPutPixel` (bleibt static in
   `DrawableDefault.cpp`). Neu: `bool usesNopPutPixel(const DRAWABLE_FUNCTIONS* fn)` dort
   exportieren; `getGrafixFunctions()` wirft `UnknownColorFormatException`, wenn nach allen
   `initDrawable*`-Aufrufen noch nopPutPixel drinsteht — VOR dem Cachen. Keine Implementierung
   von Palette/A8/R3G3B2/24-Bit erzwingen (24-Bit wird beim Laden ohnehin nach 32-Bit konvertiert).
2. **Chroma-Key nur für 32-Bit.** Tests für `BltChromaKey`/`BltBackgroundOnChromaKey` nur auf
   Formaten mit nativer Implementierung. Auf den übrigen Formaten ist Noop-doku okay.
3. **Exakte Werte prüfen**, insbesondere 32-Bit:EXPECT_EQ auf den exakten nativen Wert
   (verifiziert auch Rundungsformeln in ToNativeColor).
4. **Keine Referenzimplementierungen in den Tests und keine Differenztests gegen default*.**
   Erwartungswerte werden pro Testfall handgerechnet/handgeschrieben (kleine Flächen 3x3, 5x4).
5. **Referenz-Bilder (BMP, keine Third-Party-Lib) nur wo sinnvoll** — Kandidaten:
   Monochrome1BitVertical-Packing, GREY8-Icons. Nur, wenn Byte-Shots nicht ausreichen.
6. **pitch wird NICHT validiert** (Drawable darf ein Ausschnitt aus einem größeren Puffer
   sein, pitch kann weit größer als width*bpp sein). Aber: Testfälle mit pitch > width*bpp
   (Polsterbytes müssen unangetastet bleiben) als Standard-Aufbau in jedem Test.
7. **Thread-Safety des Format-Caches fixen**: `drawable_functions[]` statisches Array in
   Grafix.cpp mit vorhandenem Grafix-Mutex synchronisieren (doppelte Prüfung inside lock).

## Pixel-Semantik (für Tests verbindlich)

- `putPixel` überschreibt immer (kein Blending des Quell-Alpha).
- `blendPixel`/`alphaPixel` blenden: `RGBBlend255(ground, top, intensity)`;
  defaultRGBBlend255 nutzt Porter-Duff-Over mit effektiver Alpha `a_src*intensity/255`.
- Blt-Familie: semi-transparent über semi-transparent ist der klassische Fehlerfall →
  eigene Testfälle mit src.alpha() < 255 UND dst.alpha() < 255 für BltAlpha, BltAlphaMod,
  BltBlend, BltDiffuse. Erwartungswerte handgerechnet nach Porter-Duff.
- Boundary: Put/Get/Blend/Alpha clippen still (no throw); FillRect clippt per intersected();
  DrawRect clippt nicht (PutPixel clippt); Blt clippt über `clip()`.

## Testschichten

1. **Format-Vertrag** (alle aktiven Formate, farbbezogene Semantik): Roundtrip Put/Get
   (exakt), Clipping 4 Ränder, Clear/DrawRect/FillRect/Xchange/Invert/Negativ auf
   2x3-Muster, Blt-Varianten (Identität, Teilquelle, negatives Ziel).
   Pro Format dünne `TEST(...)`-Hülsen mit lesbarem Namen, Vertrag als gemeinsame Funktion
   im Test-Kit (keine TEST_P, Projektregel!).
2. **Bitlayout bytegenau** mit Pitch-Polster: A8R8G8B8, R5G6B5, Monochrome1BitVertical,
   GREY8, plus je ein Repräsentant A1R5G5B5, X4R4G4B4, A8R3G3B2, R8G8B8A8.
3. (ursprünglich: Differenztest nativ vs. default — ENTFALLEN, Entscheidung 4.)

## Testdateien (Glob `tests/src/grafix/*.cpp`pickt sie automatisch auf)

```
tests/include/drawable-testkit.h        Surface-RAII (Buffer+pitch-Polster), Engine-Singleton,
                                        Vergleichs-/Muster-Helfer, Vertragstest-Funktionen
tests/src/grafix/drawable_lifecycle.cpp create-Konstruktor, copy/Move, clear, adr(),
                                        getDrawable(Rect), Exceptions (base=null, pitch=0,
                                        w/h=0, unsupported-Format nach Fix 1)
tests/src/grafix/drawable_pixels.cpp    putPixel/getPixel/alphaPixel/blendPixel, Porter-Duff-Faelle
tests/src/grafix/drawable_rects.cpp     Clear/DrawRect/FillRect/Xchange/Invert/Negativ
tests/src/grafix/drawable_blt.cpp       Blt/BltDiffuse/BltColorKey/BltAlpha/BltAlphaMod/BltBlend
                                        (ChromaKey nur 32-Bit, Entscheidung 2)
tests/src/grafix/drawable_memlayout.cpp Rohbyte-Shots, Pitch, Monochrom-Packung, BMP-Referenzbilder
tests/src/grafix/drawable_formats.cpp   usesNopPutPixel-Wurf fuer Palette/A8/R3G3B2/R8G8B8/B8G8R8
```

## Reihenfolge der Umsetzung (Chargen, je Build+Tests)

1. Fix `getGrafixFunctions` throws bei nop-Sentinel + Mutex um den Cache (Entscheidungen 1+7),
   dazu Tests in `drawable_formats.cpp` (Regressionsschutz: Wurf-Test vor dem Fix rot).
2. `drawable-testkit.h` + `drawable_lifecycle.cpp`.
3. `drawable_pixels.cpp` (4 Ankerformate: A8R8G8B8, R5G6B5, GREY8, Monochrome1BitVertical).
4. Vertragsschicht auf die 16-Bit-Restformate + 32-Bit-Restformate ausdehnen.
5. `drawable_rects.cpp`, dann `drawable_blt.cpp`, dann `drawable_memlayout.cpp`.

## Offene Punkte / Funde (noch nicht umgesetzt)

- Stummer nop-Fallback fuer unimplementierte Formate -> wird mit Chargen 1 behoben.
- `defaultBltChromaKey`/`defaultBltBackgroundOnChromaKey` leer: bleibt so (Entscheidung 2),
  Header-Doku sollte "nur 32-Bit-Formate" sagen — Doku-Korrektur offen.
- `putPixel`-Doku sagt nichts ueber Deckverhalten — Semantik steht oben, Doku-Korrektur offen.
- Assembler-Portierung der Blt-Funktionen (A8R8G8B8): erst NACH Abschluss dieser Testsuite.
- `gfxreftest.cpp` (alte manuelle Suite, main-basiert, nicht in CMake-Tests) bleibt unangetastet.
