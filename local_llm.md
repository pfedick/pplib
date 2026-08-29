# Regeln für lokale LLMs

## Kontext
- Du läufst auf einer GPU mit 15 GB verfügbarem VRAM
- GPU ist Nvidia RTC 5070 Ti
- Laufzeitumgebung für das LLM ist Unsloth Studio
- Speicher ist knapp
- Anzahl Tokens sind begrenzt auf 100000 Tokens

## Codeanalyse
- Ziel ist es, den Quellcode auf Fehler zu analysieren und Verbesserungsvorschläge zu machen
- Aufgrund der Tokenbegrenzung ist es nicht möglich, den gesamten Quellcode auf einmal zu analysieren
- Gehe daher schrittweise vor
- Analysiere nur die angegebenen Header- und Quellcodedateien
- lese weitere Dateien, sofern erforderlich
- speichere Zwischenschritte und Ergebnisse, zum Beispiel in einer Markdown-Datei, die den Namen der zu Überprüfenden Datei oder Klasse trägt
- Wenn Du bei der Analyse andere Datein und Klassen einließt un darin Fehler findest, melde diese bitte am Ende, getrennt vom Hauptteil.
- Im Report gerne auch Code-Snippets zur Korrektur des Fehlers

# Unittests schreiben
Du bist ein lokal laufendes LLM mit begrenzter Kontext-Größe (ca. 80000 Token). Einige meiner Quelldateien sind möglicherweise zu groß, um sie vollständig auf einmal zu verarbeiten.

Hilf mir beim Schreiben von Unittests für googletest

## Verzeichnisstruktur
- Quellcode: src/**
- Header: include/pplib/**
- Tests: tests/src/**
- Testdaten: tests/testdata/**
- Temporäres Verzeichnis für Testdaten: tests/tmp

## Ziele
Ziel ist es, Funktion für Funktion vorzugehen, zu schauen, ob es schon Unittests gib, und diese entweder zu ergänzen, falls nötig, oder neu hinzuzufügen. Ziel ist es, eine möglichst Hohe Codeabdeckung zu erhalten, sofern der Aufwand nicht zu hoch ist. Nicht jede Exception muss geworfen werden. Eine Abdeckung von >90% ist gut, >95% wäre super!

## Vorgehen
Ich werde Dir sagen, welche Funktion Du dir anschauen sollst, Du schreibst die Unittests dazu. Falls Du bei der Analyse einer Funktion Fehler findest, melde mir die bitte!

## Ausführung der Tests
Mit "make file" wird der Quellcode kompiliert und nur die Tests ausgeführt, an denen wir grad arbeiten.

## Regeln für Datei-Edits
- Für Einfügungen/Änderungen in bestehenden Dateien NUR `single_find_and_replace`
  mit kurzen, eindeutigen Strings verwenden — keine kontextbasierten "lazy"-Edits
  (die haben die Datei bereits einmal weitgehend zerstört).
- `old_string` so kurz wie möglich halten, aber eindeutig im File.
- Nach JEDEM Edit sofort per `view_diff` (ggf. `read_file`) prüfen, ob der Diff
  exakt dem entspricht, was beabsichtigt war — erst dann kompilieren/testen.
- Bei beschädigter Datei: Rest mit einem Replace reparieren, dessen `new_string`
  den kompletten beabsichtigten Dateiabschluss enthält (Diff zeigt dann nur noch
  die echten Änderungen).

## Tests: Dateien löschen
- `File::erase()`/`unlink()` auf eine Datei, die von einem File-Objekt noch offen
  gehalten wird, wirft unter Windows PermissionDeniedException.
- Zwei gleichwertige Lösungen:
  1. Objekt in einen geschweiften Klammern-Block setzen (Scope endet vor dem erase)
  2. explizit `f.close()` aufrufen vor dem erase
- Der Destruktor schließt die Datei zwar automatisch, aber für eine deterministische
  Reihenfolge vor dem Löschen ist eines der beiden nötig.
  
## Aktuelle Aufgabe
Aktuell möchte ich die File-Klasse bearbeiten.

Quellcode: src/core/File.cpp
Header: include/pplib/core/file.h
Unittests: tests/src/core/file.cpp


