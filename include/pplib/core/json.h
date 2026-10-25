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

#ifndef PPLIB_CORE_JSON_H_
#define PPLIB_CORE_JSON_H_

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/variant.h>
#include <pplib/types/variantarray.h>
#include <pplib/types/assocarray.h>
#include <pplib/core/fileobject.h>

namespace pplib
{
/** @brief Hilfsklasse um JSON-Daten zu laden und zu speichern.
 *
 * Die Json-Klasse stellt statische Methoden zum Laden und Speichern von JSON-Daten
 * in und aus pplib::Variant-Objekten (sowie AssocArray und VariantArray) sowie zum
 * Pretty-Printen von JSON-Strings bereit.
 *
 * Die Json-Klasse unterstützt sowohl Objekte als auch Arrays und Skalare auf Root-Ebene
 * sowie beliebig tief verschachtelte Strukturen (bis zu einem Schachtelungslimit von 128 Ebenen).
 *
 * @note Unterstützt werden einzeilige (`//`) und mehrzeilige (`/\* ... *\ /`) Kommentare.
 *
 * @note Da AssocArray als hierarchischer Baum organisiert ist, werden Schrägstriche (`/`)
 * in Objektschlüsseln als Pfadtrenner interpretiert. Leere Objektschlüssel (`""`) werden
 * automatisch als `"_empty_"` gemappt, da AssocArray keine leeren Schlüssel zulässt.
 *
 * @note ByteArray und ByteArrayPtr werden als Base64-codierte Strings exportiert. Beim Import
 * werden diese allerdings nicht wieder zu ByteArray oder ByteArrayPtr konvertiert, sondern
 * als String behandelt.
 *
 * Ähnlich verhält es sich bei den Zeit-Objekten DateTime, Date, Time, TimeZone und TimeDelta. Auch diese
 * werden als ISO-8601-String exportiert und als String wieder importiert.
 */
class Json
{
public:
    Json() = delete; // Verhindert die Instanziierung der Json-Klasse.

    // --- Parsen (Basis auf Variant) ---

    /** @brief Lädt JSON-Daten aus einem String in ein Variant-Objekt.
     *  @param data Das Variant-Objekt, in das die JSON-Daten geladen werden.
     *  @param json Der JSON-String, der geladen werden soll.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static void loads(pplib::Variant& data, const pplib::String& json);

    /** @brief Lädt JSON-Daten aus einer Datei in ein Variant-Objekt.
     *  @param data Das Variant-Objekt, in das die JSON-Daten geladen werden.
     *  @param file Das FileObject, aus dem die JSON-Daten geladen werden.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static void load(pplib::Variant& data, pplib::FileObject& file);

    /** @brief Lädt JSON-Daten aus einem String und gibt das Ergebnis als Variant-Objekt zurück.
     *  @param json Der JSON-String, der geladen werden soll.
     *  @return Das Variant-Objekt, das die geladenen JSON-Daten enthält. Je nach Struktur der
     * JSON-Daten kann es sich dabei um ein AssocArray, ein VariantArray oder einen Skalar handeln.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static pplib::Variant loads(const pplib::String& json);

    /** @brief Lädt JSON-Daten aus einer Datei und gibt das Ergebnis als Variant-Objekt zurück.
     *  @param file Das FileObject, aus dem die JSON-Daten geladen werden.
     *  @return Das Variant-Objekt, das die geladenen JSON-Daten enthält. Je nach Struktur der
     * JSON-Daten kann es sich dabei um ein AssocArray, ein VariantArray oder einen Skalar handeln.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static pplib::Variant load(pplib::FileObject& file);

    // --- Parsen (Komfort für AssocArray und VariantArray) ---

    /** @brief Lädt JSON-Daten aus einem String in ein AssocArray-Objekt.
     *  @param data Das AssocArray-Objekt, in das die JSON-Daten geladen werden.
     *  @param json Der JSON-String, der geladen werden soll.
     *  @throw pplib::TypeConversionException Wenn der JSON-Wurzeltyp kein AssocArray (JSON-Objekt) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static void loads(pplib::AssocArray& data, const pplib::String& json);

    /** @brief Lädt JSON-Daten aus einer Datei in ein AssocArray-Objekt.
     *  @param data Das AssocArray-Objekt, in das die JSON-Daten geladen werden.
     *  @param file Das FileObject, aus dem die JSON-Daten geladen werden.
     *  @throw pplib::TypeConversionException Wenn der JSON-Wurzeltyp kein AssocArray (JSON-Objekt) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static void load(pplib::AssocArray& data, pplib::FileObject& file);

    /** @brief Lädt JSON-Daten aus einem String in ein VariantArray-Objekt.
     *  @param data Das VariantArray-Objekt, in das die JSON-Daten geladen werden.
     *  @param json Der JSON-String, der geladen werden soll.
     *  @throw pplib::TypeConversionException Wenn der JSON-Wurzeltyp kein VariantArray (JSON-Array) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static void loads(pplib::VariantArray& data, const pplib::String& json);

    /** @brief Lädt JSON-Daten aus einer Datei in ein VariantArray-Objekt.
     *  @param data Das VariantArray-Objekt, in das die JSON-Daten geladen werden.
     *  @param file Das FileObject, aus dem die JSON-Daten geladen werden.
     *  @throw pplib::TypeConversionException Wenn der JSON-Wurzeltyp kein VariantArray (JSON-Array) ist.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static void load(pplib::VariantArray& data, pplib::FileObject& file);

    // --- Serialisieren (Basis auf Variant) ---

    /** @brief Serialisiert ein Variant-Objekt in einen JSON-String.
     *  @param json Der String, in den die JSON-Daten geschrieben werden.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dumps(pplib::String& json, const pplib::Variant& data, int indent = -1);

    /** @brief Serialisiert ein Variant-Objekt in eine JSON-Datei.
     *  @param file Das FileObject, in das die JSON-Daten geschrieben werden.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dump(pplib::FileObject& file, const pplib::Variant& data, int indent = -1);

    /** @brief Serialisiert ein Variant-Objekt in einen JSON-String und gibt das Ergebnis zurück.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @return Der JSON-String, der das serialisierte Variant-Objekt enthält.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static pplib::String dumps(const pplib::Variant& data, int indent = -1);

    // --- Serialisieren (Komfort für AssocArray und VariantArray) ---

    /** @brief Serialisiert ein AssocArray-Objekt in einen JSON-String.
     *  @param json Der String, in den die JSON-Daten geschrieben werden.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dumps(pplib::String& json, const pplib::AssocArray& data, int indent = -1);

    /** @brief Serialisiert ein AssocArray-Objekt in eine JSON-Datei.
     *  @param file Das FileObject, in das die JSON-Daten geschrieben werden.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent = -1);

    /** @brief Serialisiert ein AssocArray-Objekt in einen JSON-String und gibt das Ergebnis zurück.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @return Der JSON-String, der das serialisierte AssocArray-Objekt enthält.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static pplib::String dumps(const pplib::AssocArray& data, int indent = -1);

    /** @brief Serialisiert ein VariantArray-Objekt in einen JSON-String.
     *  @param json Der String, in den die JSON-Daten geschrieben werden.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dumps(pplib::String& json, const pplib::VariantArray& data, int indent = -1);

    /** @brief Serialisiert ein VariantArray-Objekt in eine JSON-Datei.
     *  @param file Das FileObject, in das die JSON-Daten geschrieben werden.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static void dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent = -1);

    /** @brief Serialisiert ein VariantArray-Objekt in einen JSON-String und gibt das Ergebnis zurück.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die JSON-Daten. Der Default ist "-1" und bewirkt,
     *  dass die JSON-Daten ohne zusätzliche Einrückung, Zeilenumbrüche und Leerzeichen ausgegeben werden.
     *  Mit "0" wird nicht eingerückt, aber Zeilenumbrüche und Leerzeichen werden hinzugefügt.
     *  @return Der JSON-String, der das serialisierte VariantArray-Objekt enthält.
     *  @throw pplib::UnsupportedDataTypeException Wenn ein Variant-Typ nicht nach JSON serialisiert werden kann.
     *  @throw pplib::OverflowException Wenn die Schachtelungstiefe das Limit überschreitet.
     */
    static pplib::String dumps(const pplib::VariantArray& data, int indent = -1);

    // --- Pretty-Print ---
    /** @brief Formatiert einen JSON-String mit Einrückungen für eine bessere Lesbarkeit.
     *  @param json Der JSON-String, der formatiert werden soll.
     *  @param indent Die Anzahl der Leerzeichen für jede Einrückungsebene. Der Default ist "4".
     *  @return Der formatierte JSON-String mit Einrückungen.
     *  @throw pplib::UnexpectedEndOfDataException Bei vorzeitigem Ende der JSON-Daten.
     *  @throw pplib::UnexpectedCharacterException Bei ungültigen Zeichen oder Syntaxfehlern.
     *  @throw pplib::InvalidEscapeSequenceException Bei fehlerhaften Escape-Sequenzen in Strings.
     *  @throw pplib::CharacterEncodingException Bei ungültigen Unicode-Codepoints oder Surrogaten.
     */
    static pplib::String pp(const pplib::String& json, int indent = 4);
};

} // namespace pplib

#endif /* PPLIB_CORE_JSON_H_ */