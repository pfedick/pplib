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
 * sowie beliebig tief verschachtelte Strukturen.
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
    // --- Parsen (Basis auf Variant) ---
    static void loads(pplib::Variant& data, const pplib::String& json);
    static void load(pplib::Variant& data, pplib::FileObject& file);
    static pplib::Variant loads(const pplib::String& json);
    static pplib::Variant load(pplib::FileObject& file);

    // --- Parsen (Komfort für AssocArray und VariantArray) ---
    static void loads(pplib::AssocArray& data, const pplib::String& json);
    static void load(pplib::AssocArray& data, pplib::FileObject& file);
    static void loads(pplib::VariantArray& data, const pplib::String& json);
    static void load(pplib::VariantArray& data, pplib::FileObject& file);

    // --- Serialisieren (Basis auf Variant) ---
    static void dumps(pplib::String& json, const pplib::Variant& data, int indent = -1);
    static void dump(pplib::FileObject& file, const pplib::Variant& data, int indent = -1);
    static pplib::String dumps(const pplib::Variant& data, int indent = -1);

    // --- Serialisieren (Komfort für AssocArray und VariantArray) ---
    static void dumps(pplib::String& json, const pplib::AssocArray& data, int indent = -1);
    static void dump(pplib::FileObject& file, const pplib::AssocArray& data, int indent = -1);
    static pplib::String dumps(const pplib::AssocArray& data, int indent = -1);

    static void dumps(pplib::String& json, const pplib::VariantArray& data, int indent = -1);
    static void dump(pplib::FileObject& file, const pplib::VariantArray& data, int indent = -1);
    static pplib::String dumps(const pplib::VariantArray& data, int indent = -1);

    // --- Pretty-Print ---
    static pplib::String pp(const pplib::String& json, int indent = 4);
};

} // namespace pplib

#endif /* PPLIB_CORE_JSON_H_ */