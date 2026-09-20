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

#ifndef PPLIB_CORE_ICONV_H_
#define PPLIB_CORE_ICONV_H_

#include <list>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/bytearrayptr.h>
#include <pplib/types/array.h>

namespace pplib
{

/** @class Iconv
 * @brief Klasse zur Konvertierung von Zeichencodierungen
 *
 * Diese Klasse bietet Methoden zur Konvertierung von Strings und ByteArrays zwischen verschiedenen Zeichencodierungen.
 * Sie basiert auf der Iconv-Bibliothek.
 */
class Iconv
{
private:
    void* iconv_handle; //< Handle für die Iconv-Konvertierungssitzung

public:
    /** @brief Standardkonstruktor
     *
     * Initialisiert die Iconv-Konvertierungssitzung ohne spezifische Quell- und Ziel-Encodings.
     */
    Iconv();

    /** @brief Konstruktor mit spezifischen Quell- und Ziel-Encodings
     *
     * Initialisiert die Iconv-Konvertierungssitzung mit den angegebenen Quell- und Ziel-Encodings.
     *
     * @param[in] fromEncoding Das Quell-Encoding (z. B. "UTF-8", "ISO-8859-1")
     * @param[in] toEncoding Das Ziel-Encoding (z. B. "UTF-8", "ISO-8859-1")
     */
    Iconv(const String& fromEncoding, const String& toEncoding);

    Iconv(const Iconv& other) = delete;
    Iconv& operator=(const Iconv& other) = delete;
    Iconv(Iconv&& other);
    Iconv& operator=(Iconv&& other);
    /** @brief Destruktor
     *
     * Schließt die Iconv-Konvertierungssitzung und gibt alle Ressourcen frei.
     */
    ~Iconv();

    /** @brief Initialisiert die Iconv-Konvertierungssitzung mit den angegebenen Quell- und Ziel-Encodings
     *
     * @param[in] fromEncoding Das Quell-Encoding (z. B. "UTF-8", "ISO-8859-1")
     * @param[in] toEncoding Das Ziel-Encoding (z. B. "UTF-8", "ISO-8859-1")
     */
    void init(const String& fromEncoding, const String& toEncoding);

    /** @brief Prüft, ob die Iconv-Konvertierungssitzung initialisiert wurde
     *
     * @return true, wenn die Iconv-Konvertierungssitzung initialisiert ist, sonst false
     */
    inline bool isInitialized() const
    {
        return iconv_handle != nullptr;
    }

    /** @brief Konvertiert ein ByteArray von einem Encoding in ein anderes
     *
     * @param[in] from Das Quell-ByteArray
     * @param[out] to Das Ziel-ByteArray
     */
    void transcode(const ByteArrayPtr& from, ByteArray& to);

    /** @brief Konvertiert ein ByteArray von einem Encoding in ein anderes und gibt das Ergebnis zurück
     *
     * @param[in] from Das Quell-ByteArray
     * @return Das konvertierte ByteArray im Ziel-Encoding
     */
    ByteArray transcode(const ByteArrayPtr& from);

    /** @brief Konvertiert einen String von einem Encoding in ein anderes
     *
     * @param[in] from Der Quell-String
     * @param[out] to Der Ziel-String
     */
    void transcode(const String& from, String& to);

    /** @brief Konvertiert einen String von einem Encoding in ein anderes und gibt das Ergebnis zurück
     *
     * @param[in] from Der Quell-String
     * @return Der konvertierte String im Ziel-Encoding
     */
    String transcode(const String& from);

    /**
     * @brief Konvertiert einen WideString (wchar_t) in einen String mit dem angegebenen Ziel-Encoding
     *
     * @param[in] from Der Quell-WideString (UTF-16 unter Windows, UTF-32 unter Linux)
     * @param[in] toEncoding Das gewünschte Ziel-Encoding (z. B. "UTF-8", "ISO-8859-1")
     * @return String im angegebenen Ziel-Encoding
     */
    static String fromWideString(const WideString& from, const String& toEncoding);

    /**
     * @brief Konvertiert einen String aus einem angegebenen Quell-Encoding in einen WideString (wchar_t)
     *
     * @param[in] from Der Quell-String
     * @param[in] fromEncoding Das Quell-Encoding des Strings (z. B. "UTF-8", "ISO-8859-1")
     * @return WideString (UTF-16 unter Windows, UTF-32 unter Linux)
     */
    static WideString toWideString(const String& from, const String& fromEncoding);

    /** @brief Gibt eine Liste aller unterstützten Zeichencodierungen zurück
     * @param[out] list Die Liste, die mit den unterstützten Zeichencodierungen gefüllt wird
     */
    static void enumerateCharsets(Array& list);

    /** @brief Gibt eine Liste aller unterstützten Zeichencodierungen zurück
     * @param[out] list Die Liste, die mit den unterstützten Zeichencodierungen gefüllt wird
     */
    static void enumerateCharsets(std::list<pplib::String>& list);

    /** @brief Gibt das lokale Charset zurück
     *
     * Diese Methode gibt das lokale Charset des Systems im MIME-, bzw. IANA-Standard zurück (z. B. "UTF-8", "ISO-8859-1"),
     * wie es von der Iconv-Bibliothek oder nl_langinfo(CODESET) ermittelt wird.
     *
     * @return String mit dem Namen des lokalen Charsets
     * @exception UnsupportedFeatureException Wenn die Plattform weder localcharset.h noch nl_langinfo unterstützt.
     */
    static String getLocalCharset();

    /** @brief Konvertiert einen UTF-8 codierten String in das lokale Charset des Systems
     *
     * @param[in] text Der UTF-8 codierte String
     * @return String im lokalen Charset des Systems
     */
    static String utf8ToLocal(const String& text);

    /** @brief Konvertiert einen String aus dem lokalen Charset des Systems in UTF-8
     *
     * @param[in] text Der String im lokalen Charset des Systems
     * @return UTF-8 codierter String
     */
    static String localToUtf8(const String& text);

    /** @brief Konvertiert einen String von einem Quell-Encoding in ein Ziel-Encoding
     *
     * @param[in] text Der zu konvertierende String
     * @param[in] fromEncoding Das Quell-Encoding des Strings (z. B. "UTF-8", "ISO-8859-1")
     * @param[in] toEncoding Das gewünschte Ziel-Encoding (z. B. "UTF-8", "ISO-8859-1")
     * @return ByteArray im Ziel-Encoding
     */
    static ByteArray transcode(const String& text, const String& fromEncoding, const String& toEncoding);

    /** @brief Konvertiert einen ByteArray von einem Quell-Encoding in ein Ziel-Encoding
     *
     * @param[in] text Der zu konvertierende ByteArray
     * @param[in] fromEncoding Das Quell-Encoding des ByteArray (z. B. "UTF-8", "ISO-8859-1")
     * @param[in] toEncoding Das gewünschte Ziel-Encoding (z. B. "UTF-8", "ISO-8859-1")
     * @return ByteArray im Ziel-Encoding
     */
    static ByteArray transcode(const ByteArrayPtr& text, const String& fromEncoding, const String& toEncoding);

    [[deprecated("Use fromWideString instead.")]]
    static inline String fromUnicode(const WideString& from, const String& toEncoding)
    {
        return fromWideString(from, toEncoding);
    }
    [[deprecated("Use toWideString instead.")]]
    static inline WideString toUnicode(const String& from, const String& fromEncoding)
    {
        return toWideString(from, fromEncoding);
    }
};

}; // namespace pplib

#endif /* PPLIB_CORE_ICONV_H_ */