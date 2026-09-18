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
#include <pplib/types/assocarray.h>
#include <pplib/core/fileobject.h>

namespace pplib
{
/** @brief Hilfsklasse um JSON-Daten zu laden und zu speichern.
 *
 * Die Json-Klasse stellt statische Methoden zum Laden und Speichern von JSON-Daten
 * in und aus pplib::AssocArray-Objekten sowie zum Pretty-Printen von JSON-Strings bereit.
 *
 * Die Json-Klasse unterstützt sowohl einfache als auch verschachtelte Arrays und Assoziativ-Arrays.
 *
 * @note ByteArray und ByteArrayPtr werden als Base64-codierte Strings exportiert. Beim Import
 * werden diese allerdings nicht wieder zu ByteArray oder ByteArrayPtr konvertiert, sondern
 * als Base64-codierte Strings behandelt. Die Funktion kann nicht wissen, ob ein String
 * ursprünglich ein ByteArray oder bereits ein Base64-codierter String war.
 *
 * Ähnlich verhält es sich bei den Zeit-Objekten DateTime, Date, Time, TimeZone und TimeDelta. Auch diese
 * werden als String exportiert und als String wieder importiert.
 */
class Json
{
public:
    static void loads(pplib::AssocArray& data, const pplib::String& json);
    static void load(pplib::AssocArray& data, pplib::FileObject& file);
    static pplib::AssocArray loads(const pplib::String& json);
    static pplib::AssocArray load(pplib::FileObject& file);

    static void dumps(pplib::String& json, const pplib::AssocArray& data);
    static void dump(pplib::FileObject& file, const pplib::AssocArray& data);
    static pplib::String dumps(const pplib::AssocArray& data);
    static pplib::String pp(const pplib::String& json, unsigned int indent = 4);
};
} // namespace pplib

#endif /* PPLIB_CORE_JSON_H_ */