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
 *       documentation or other materials provided with the distribution.
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
 * Unterstützt wird ein YAML-Untermenge: Block-Mappings, Block-Sequenzen, Flow-Style
 * ({...} und [...]), Plain-, einfach- und doppelzierte Skalare, Block-Skalare (| und >)
 * sowie Kommentare (#). Anker/Aliase und mehrere Dokumente werden nicht unterstützt.
 *
 * @note Im Gegensatz zu Json konvertiert Yaml ISO-8601-Zeiträume beim Import in die
 * passenden Typen: Datum+Zeit wird zu DateTime, ein reines Datum zu Date und eine
 * reine Zeit zu Time. Beim Export werden diese Typen wieder als ISO-8601 geschrieben.
 *
 * @note ByteArray und ByteArrayPtr werden über den Tag !!binary als Base64 exportiert
 * und beim Import von !!binary-Werten (Base64) wieder in ByteArray konvertiert.
 *
 * @note Strings bleiben beim Export immer Strings: Sie werden doppelzitiert geschrieben,
 * damit sie auch dann als String zurückgelesen werden, wenn ihr Inhalt wie eine Zahl
 * oder ein Datum aussieht. Nur echte DateTime-, Integer-, Double- und Boolean-Werte
 * werden unziert exportiert.
 */
class Yaml
{
public:
    Yaml() = delete; // Prevent instantiation of this static class

    // --- Parsen (Basis auf Variant) ---

    /** @brief Lädt YAML-Daten aus einem String in ein Variant-Objekt.
     *  @param data Das Variant-Objekt, in das die YAML-Daten geladen werden.
     *  @param yaml Der YAML-String, der geparst werden soll.
     */
    static void loads(pplib::Variant& data, const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein Variant-Objekt.
     *  @param data Das Variant-Objekt, in das die YAML-Daten geladen werden.
     *  @param file Das FileObject, aus dem die YAML-Daten gelesen werden.
     */
    static void load(pplib::Variant& data, pplib::FileObject& file);

    /** @brief Lädt YAML-Daten aus einem String in ein Variant-Objekt und gibt das Ergebnis zurück.
     *  @param yaml Der YAML-String, der geparst werden soll.
     *  @return Das Variant-Objekt, das die geladenen YAML-Daten enthält.
     */
    static pplib::Variant loads(const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein Variant-Objekt und gibt das Ergebnis zurück.
     *  @param file Das FileObject, aus dem die YAML-Daten gelesen werden.
     *  @return Das Variant-Objekt, das die geladenen YAML-Daten enthält.
     */
    static pplib::Variant load(pplib::FileObject& file);

    // --- Parsen (Komfort für AssocArray und VariantArray) ---
    /** @brief Lädt YAML-Daten aus einem String in ein AssocArray-Objekt.
     *  @param data Das AssocArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param yaml Der YAML-String, der geparst werden soll.
     */
    static void loads(pplib::AssocArray& data, const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein AssocArray-Objekt.
     *  @param data Das AssocArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param file Das FileObject, aus dem die YAML-Daten gelesen werden.
     */
    static void load(pplib::AssocArray& data, pplib::FileObject& file);

    /** @brief Lädt YAML-Daten aus einem String in ein VariantArray-Objekt.
     *  @param data Das VariantArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param yaml Der YAML-String, der geparst werden soll.
     */
    static void loads(pplib::VariantArray& data, const pplib::String& yaml);

    /** @brief Lädt YAML-Daten aus einer Datei in ein VariantArray-Objekt.
     *  @param data Das VariantArray-Objekt, in das die YAML-Daten geladen werden.
     *  @param file Das FileObject, aus dem die YAML-Daten gelesen werden.
     */
    static void load(pplib::VariantArray& data, pplib::FileObject& file);

    // --- Serialisieren (Basis auf Variant) ---
    /** @brief Serialisiert ein Variant-Objekt in einen YAML-String.
     *  @param yaml Der String, in den die YAML-Daten geschrieben werden.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     */
    static void dumps(pplib::String& yaml, const pplib::Variant& data, int indent = 2);

    /** @brief Serialisiert ein Variant-Objekt in eine YAML-Datei.
     *  @param file Das FileObject, in das die YAML-Daten geschrieben werden.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     */
    static void dump(pplib::FileObject& file, const pplib::Variant& data, int indent = 2);

    /** @brief Serialisiert ein Variant-Objekt in einen YAML-String und gibt das Ergebnis zurück.
     *  @param data Das Variant-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     *  @return Der YAML-String, der die serialisierten Daten enthält.
     */
    static pplib::String dumps(const pplib::Variant& data, int indent = 2);

    // --- Serialisieren (Komfort für AssocArray und VariantArray) ---
    /** @brief Serialisiert ein AssocArray-Objekt in einen YAML-String.
     *  @param yaml Der String, in den die YAML-Daten geschrieben werden.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     */
    static void dumps(pplib::String& yaml, const pplib::AssocArray& data, int indent = 2);

    /** @brief Serialisiert ein AssocArray-Objekt in eine YAML-Datei.
     *  @param file Das FileObject, in das die YAML-Daten geschrieben werden.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     */
    static void dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent = 2);
    /** @brief Serialisiert ein AssocArray-Objekt in einen YAML-String und gibt das Ergebnis zurück.
     *  @param data Das AssocArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     *  @return Der YAML-String, der die serialisierten Daten enthält.
     */
    static pplib::String dumps(const pplib::AssocArray& data, int indent = 2);

    /** @brief Serialisiert ein VariantArray-Objekt in einen YAML-String.
     *  @param yaml Der String, in den die YAML-Daten geschrieben werden.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     */
    static void dumps(pplib::String& yaml, const pplib::VariantArray& data, int indent = 2);

    /** @brief Serialisiert ein VariantArray-Objekt in eine YAML-Datei.
     *  @param file Das FileObject, in das die YAML-Daten geschrieben werden.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     */
    static void dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent = 2);

    /** @brief Serialisiert ein VariantArray-Objekt in einen YAML-String und gibt das Ergebnis zurück.
     *  @param data Das VariantArray-Objekt, das serialisiert werden soll.
     *  @param indent Die Einrückungsebene für die YAML-Daten. Der Default ist "2".
     * Bei Werten kleiner 1 wird Flow-Style verwendet (JSON-ähnlich).
     *  @return Der YAML-String, der die serialisierten Daten enthält.
     */
    static pplib::String dumps(const pplib::VariantArray& data, int indent = 2);
};

} // namespace pplib

#endif /* PPLIB_CORE_YAML_H_ */
