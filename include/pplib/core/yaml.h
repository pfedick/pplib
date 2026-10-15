/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef PPLIB_CORE_YAML_H_
#define PPLIB_CORE_YAML_H_

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/assocarray.h>
#include <pplib/core/fileobject.h>

namespace pplib
{
/** @brief Hilfsklasse um YAML-Daten zu laden und zu speichern.
 *
 * Die Yaml-Klasse stellt statische Methoden zum Laden und Speichern von YAML-Daten
 * in und aus pplib::Variant-Objekten (sowie AssocArray und VariantArray) bereit.
 *
 * Die Yaml-Klasse unterstützt Mappings und Sequenzen sowie Skalare auf Root-Ebene
 * sowie beliebig tief verschachtelte Strukturen (bis zu einem Schachtelungslimit von 128 Ebenen).
 *
 * @note Unterstützt werden Block-Mappings, Block-Sequenzen, Flow-Sammlungen (`[...]`, `{...}`),
 * einfache und doppelt quotierte Strings, Block-Scalars (`|`, `>`) sowie Kommentare (`#`).
 *
 * @note Da AssocArray als hierarchischer Baum organisiert ist, werden Schrägstriche (`/`)
 * in Objektschlüsseln als Pfadtrenner interpretiert. Leere Objektschlüssel (`""`) werden
 * automatisch als `"_empty_"` gemappt, da AssocArray keine leeren Schlüssel zulässt.
 *
 * @note Im Unterschied zu JSON werden binäre und Zeit-Daten verlustfrei round-trip-fähig
 * exportiert und importiert: ByteArray und ByteArrayPtr werden mit dem Tag `!!binary` als
 * Base64-String exportiert und beim Import wieder zu ByteArray. DateTime und Date werden
 * mit dem Tag `!!timestamp` als ISO-8601-String exportiert und beim Import wieder zu
 * DateTime bzw. Date. Time, TimeDelta und TimeZone werden mit den pplib-eigenen Tags
 * `!Time`, `!TimeDelta` und `!TimeZone` exportiert und entsprechend importiert.
 *
 * @note Auch ohne Tags werden YAML-Typen erkannt: `null`/`~`, `true`/`false`, Ganzzahlen
 * (dezimal, `0x`-Hexadezimal, `0o`-Oktal), Fließkommazahlen (inkl. `.inf` und `.nan`) sowie
 * Datums- (`YYYY-MM-DD`), Datumszeit- (`YYYY-MM-DDTHH:MM:SS...`) und Zeitangaben (`HH:MM:SS`).
 *
 * @note Anker (`&name`) und Aliase (`*name`) werden unterstützt. Unbekannte Tags werden
 * ignoriert; der Wert wird wie ohne Tag behandelt.
 *
 * @note Nicht unterstützt werden Merge-Keys (`<<:`), Multiple Documents (ein Stream enthält
 * genau ein Dokument) und Knoten-Farben.
 */
class Yaml
{
public:
    Yaml() = delete; // Verhindert die Instanziierung der Yaml-Klasse.

    // --- Parsen (Basis auf Variant) ---

    /** @brief Lädt YAML-Daten aus einem String in ein Variant-Objekt.
     *  @param data Das Variant-Objekt, in das die YAML-Daten geladen werden.
     *  @param yaml Der YAML-String, der geladen werden soll.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static void loads(pplib::Variant& data, const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein Variant-Objekt.
     *  @param data Das Variant-Objekt, in das die YAML-Daten geladen werden.
     *  @param file Das FileObject, aus dem die YAML-Daten geladen werden.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static void load(pplib::Variant& data, pplib::FileObject& file);

    /** @brief Lädt YAML-Daten aus einem String und gibt das Ergebnis als Variant-Objekt zurück.
     *  @param yaml Der YAML-String, der geladen werden soll.
     *  @return Das Variant-Objekt, das die geladenen YAML-Daten enthält. Je nach Struktur der
     * YAML-Daten kann es sich dabei um ein AssocArray, ein VariantArray oder einen Skalar handeln.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static pplib::Variant loads(const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei und gibt das Ergebnis als Variant-Objekt zurück.
     *  @param file Das FileObject, aus dem die YAML-Daten geladen werden.
     *  @return Das Variant-Objekt, das die geladenen YAML-Daten enthält. Je nach Struktur der
     * YAML-Daten kann es sich dabei um ein AssocArray, ein VariantArray oder einen Skalar handeln.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitiges Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static pplib::Variant load(pplib::FileObject& file);

    // --- Parsen (Komfort für AssocArray und VariantArray) ---

    /** @brief Lädt YAML-Daten aus einem String in ein AssocArray-Objekt.
     *  @param data Das AssocArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param yaml Der YAML-String, der geladen werden soll.
     *  @throw pplib::TypeConversionException Wenn der YAML-Wurzeltyp kein AssocArray (Mapping) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static void loads(pplib::AssocArray& data, const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein AssocArray-Objekt.
     *  @param data Das AssocArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param file Das FileObject, aus dem die YAML-Daten geladen werden.
     *  @throw pplib::TypeConversionException Wenn der YAML-Wurzeltyp kein AssocArray (Mapping) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static void load(pplib::AssocArray& data, pplib::FileObject& file);

    /** @brief Lädt YAML-Daten aus einem String in ein VariantArray-Objekt.
     *  @param data Das VariantArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param yaml Der YAML-String, der geladen werden soll.
     *  @throw pplib::TypeConversionException Wenn der YAML-Wurzeltyp kein VariantArray (Sequenz) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static void loads(pplib::VariantArray& data, const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein VariantArray-Objekt.
     *  @param data Das VariantArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param file Das FileObject, aus dem die YAML-Daten geladen werden.
     *  @throw pplib::TypeConversionException Wenn der YAML-Wurzeltyp kein VariantArray (Sequenz) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der YAML-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     *  @throw pplib::IllegalArgumentException Bei ungültigen Datums-, Zeit- oder Base64-Werten.
     */
    static void load(pplib::VariantArray& data, pplib::FileObject& file);

    // --- Serialisieren (Basis auf Variant) ---

    /** @brief Serialisiert ein Variant-Objekt in einen YAML-String.
     *  @param yaml Der String, in den die YAML-Daten geschrieben werden.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  Da YAML zeilenbasiert ist, wird immer mit Zeilenumbrüchen und Einrückung gearbeitet.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dumps(pplib::String& yaml, const pplib::Variant& data, int indent = 2);

    /** @brief Serialisiert ein Variant-Objekt in eine YAML-Datei.
     *  @param file Das FileObject, in das die YAML-Daten geschrieben werden.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  Da YAML zeilenbasiert ist, wird immer mit Zeilenumbrüchen und Einrückung gearbeitet.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dump(pplib::FileObject& file, const pplib::Variant& data, int indent = 2);

    /** @brief Serialisiert ein Variant-Objekt in einen YAML-String und gibt das Ergebnis zurück.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  Da YAML zeilenbasiert ist, wird immer mit Zeilenumbrüchen und Einrückung gearbeitet.
     *  @return Der YAML-String, der das serialisierte Variant-Objekt enthält.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static pplib::String dumps(const pplib::Variant& data, int indent = 2);

    // --- Serialisieren (Komfort für AssocArray und VariantArray) ---

    /** @brief Serialisiert ein AssocArray-Objekt in einen YAML-String.
     *  @param yaml Der String, in den die YAML-Daten geschrieben werden.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dumps(pplib::String& yaml, const pplib::AssocArray& data, int indent = 2);

    /** @brief Serialisiert ein AssocArray-Objekt in eine YAML-Datei.
     *  @param file Das FileObject, in das die YAML-Daten geschrieben werden.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent = 2);

    /** @brief Serialisiert ein AssocArray-Objekt in einen YAML-String und gibt das Ergebnis zurück.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  @return Der YAML-String, der das serialisierte AssocArray-Objekt enthält.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static pplib::String dumps(const pplib::AssocArray& data, int indent = 2);

    /** @brief Serialisiert ein VariantArray-Objekt in einen YAML-String.
     *  @param yaml Der String, in den die YAML-Daten geschrieben werden.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dumps(pplib::String& yaml, const pplib::VariantArray& data, int indent = 2);

    /** @brief Serialisiert ein VariantArray-Objekt in eine YAML-Datei.
     *  @param file Das FileObject, in das die YAML-Daten geschrieben werden.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent = 2);

    /** @brief Serialisiert ein VariantArray-Objekt in einen YAML-String und gibt das Ergebnis zurück.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "2".
     *  @return Der YAML-String, der das serialisierte VariantArray-Objekt enthält.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach YAML serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static pplib::String dumps(const pplib::VariantArray& data, int indent = 2);
};

} // namespace pplib

#endif /* PPLIB_CORE_YAML_H_ */
