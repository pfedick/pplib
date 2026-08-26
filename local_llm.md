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

