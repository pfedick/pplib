# Regeln für lokale LLMs

## Kontext
- Du läufst auf einer GPU mit 15 GB verfügbarem VRAM
- GPU ist Nvidia RTC 5070 Ti
- Laufzeitumgebung für das LLM ist Unsloth Studio
- Speicher ist ein Problem
- Anzahl Tokens sind begrenzt auf 100000 Tokens
- Output-Tokens sind begrenzt auf 50000 Tokens

## Codeanalyse
- Ziel ist es, den Quellcode auf Fehler zu analysieren und Verbesserungsvorschläge zu machen
- Aufgrund der Tokenbegrenzung ist es nicht möglich, den gesamten Quellcode auf einmal zu analysieren
- Gehe daher schrittweise vor
- Analysiere nur die angegebenen Header- und Quellcodedateien
- lese weitere Dateien, sofern erforderlich
- speichere Zwischenschritte und Ergebnisse, zum Biespiel in einer Markdown-Datei, die den Namen der zu Überprüfenden Datei oder Klasse trägt

# Aufgabe

In diesem C++ Projekt möchte ich die vorhandene Dokumentation im Doxygen-Format aus den Quellcode-Dateien unter "src" in die jeweiligen Header-Dateien unter "include" verschieben. Diesen Vorgang möchte ich mittels Python-Skript automatisieren. Das Skript bekommt als Eingabe den Pfad zu einer Quellcode-Datei, sowie den Pfad zur Header-Datei. Alle Doxygen-Kommentare, die sich auf eine Klassenmethode beziehen, sollen dann aus der Quellcode-Datei entfernt und in der Header-Datei an die entsprechende Definition der Methode eingefügt werden. Dokumentationen, die sich auf die Klasse selbst beziehen (z.B. \class oder @class), oder Member-variablen, sollen ignoriert werden um im Quellcode bestehen bleiben.

Darauf ist zu achten:

- Die Doxygen-Kommentare sind entweder direkt über oder direkt unter der Methodendefinition in der Quellcode-Datei zu finden.
- Die Doxygen-Kommentare in der Header-Datei sollen aber immer direkt über der Methodendefinition eingefügt werden.
- Die Syntax der Doxygen-Kommentare soll angepasst werden: 
- Einleitung immer mit "/** @..." also Slash, Stern, Stern, Leerzeichen, @, dann der Doxygen-Tag
- Jeder Doxygen-Tag soll mit "@" anfangen und nicht mit "\"
- Der propritäre Doxygen-Tag "\desc" soll vollständig entfernt werden, inklusive Zeilenumbruch
- Der Doxygen-Tag "\par" oder "@par" soll entfernt werden und eine Leerzeile hinterlassen
- Wichtig: falls in der Header-Datei bereits ein Doxygen-Kommentar für die Methode existiert, soll dieser nicht überschrieben werden. In diesem Fall soll der Kommentar aus der Quellcode-Datei gelöscht werden, aber nicht in die Header-Datei verschoben werden.

Zum Testen können folgende Dateien verwendet werden:
- Quellcode-Datei: tests/testdata/doku/AssocArray.cpp
- Header-Datei: tests/testdata/doku/assocarray.h

Ein Beispiel, wie es am Ende aussehen soll, ist hier zu finden: tests/testdata/doku/string.h
Das Skript soll "migrate_docs.py" heißen und im Hauptordner des Projeks liegen.

Temporäre Dateien können unter "tmp" abgelegt werden.

# Limitierungen
- Du bist ein lokal laufendes LLM
- Es stehen maximal 90000 Input-Tokens und 10000 Output-Tokens zur Verfügung