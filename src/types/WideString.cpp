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

#include <limits>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>
#include <pplib/core/iconv.h>

#include "config_pplib.h"

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

namespace pplib
{

/*!\class WideString
 * \ingroup PPLGroupDataTypes
 * \ingroup PPLGroupStrings
 * \brief WideString-Klasse
 *
 * \desc
 * Diese Klasse kann verwendet werden, um beliebige Strings zu speichern und zu verarbeiten. Dabei
 * braucht sich der Anwender keine Gedanken um den verwendeten Speicher zu machen.
 * Die einzelnen Zeichen des Strings werden intern im Unicode-Format gespeichert. Bei Übernahme eines
 * C-Strings wird erwartet, dass dieser im UTF-8 Format vorliegt, mit der statischen Funktion
 * WideString::setGlobalEncoding kann jedoch auch eine andere Kodierung vorgegeben werden.
 *
 */

/*!\brief Konstruktor für leeren String
 *
 * \desc
 * Es wird ein leerer String erstellt.
 */
WideString::WideString() noexcept
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
}

/*!\brief Konstruktor aus Wide-Character-String
 *
 * \desc
 * Ein String wird aus einem Wide-Character-String erstellt.
 *
 * @param str Wide-Character-String, der mit einem 0-Wert Endet
 * @exception OutOfMemoryException
 */
WideString::WideString(const wchar_t* str)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str);
}

/*!\brief Konstruktor aus Wide-Character-String mit bestimmer Länge
 *
 * \desc
 * Ein String wird aus dem Wide-Character-String \p str erstellt, von dem maximal
 * \p size Zeichen übernommen werden.
 *
 * @param str Wide-Character-String, der mit einem 0-Wert Endet
 * @param size Maximale Anzahl Zeichen, die übernommen werden sollen
 * @exception OutOfMemoryException
 */
WideString::WideString(const wchar_t* str, size_t size)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str, size);
}

WideString::WideString(const char* str, size_t size)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str, size);
}

/*!\brief Konstruktor aus anderem String (Copy-Konstruktor)
 *
 * \desc
 * Ein String wird aus einem anderen String erstellt.
 *
 * @param str Referenz auf einen anderen String
 * @exception OutOfMemoryException
 */
WideString::WideString(const WideString& str)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str);
}

WideString::WideString(WideString&& str) noexcept
{
    ptr = str.ptr;
    stringlen = str.stringlen;
    s = str.s;
    str.ptr = NULL;
    str.stringlen = 0;
    str.s = 0;
}

/*!\brief Konstruktor aus anderem String (Copy-Konstruktor)
 *
 * \desc
 * Ein String wird aus einem anderen String erstellt.
 *
 * @param str Referenz auf einen anderen String
 * @exception OutOfMemoryException
 */
WideString::WideString(const String& str)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str);
}

/*!\brief Konstruktor aus Standard-Template String
 *
 * \desc
 * Ein String wird aus einem String der Standard-Template-Library (STL) erstellt.
 *
 * @param str Referenz auf String der STL
 * @exception OutOfMemoryException
 * @exception UnsupportedFeatureException
 * @exception UnsupportedCharacterEncodingException
 * @exception CharacterEncodingException
 */
WideString::WideString(const std::string& str)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str.data(), str.size());
}

/*!\brief Konstruktor aus Standard-Template Wide-String
 *
 * \desc
 * Ein String wird aus einem Wide-String der Standard-Template-Library (STL) erstellt.
 *
 * @param str Referenz auf Wide-String der STL
 * @exception OutOfMemoryException
 */
WideString::WideString(const std::wstring& str)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set(str.data(), str.size());
}

WideString::WideString(const ByteArrayPtr& str)
{
    ptr = NULL;
    stringlen = 0;
    s = 0;
    set((wchar_t*)str.adr(), str.size() / sizeof(wchar_t));
}

/*!\brief Destruktor
 *
 * \desc
 * Der Destructor gibt den durch den String belegten Speicher wieder frei.
 *
 */
WideString::~WideString() noexcept
{
    free(ptr);
}

/*!\brief String leeren
 *
 * \desc
 * Mit dieser Funktion wird der String geleert und der bisher allokierte Speicher wieder
 * freigegeben.
 */
void WideString::clear() noexcept
{
    free(ptr);
    ptr = NULL;
    stringlen = 0;
    s = 0;
}

/*!\brief Anzahl Zeichen, die in den bereits allokierten Speicher passen
 *
 * \desc
 * Diese Funktion liefert die Anzahl Zeichen zurück, die in den derzeitig allokierten
 * Puffer passen, ohne dass neuer Speicher allokiert werden muss.
 *
 * @return Anzahl Zeichen
 */
size_t WideString::capacity() const
{
    return s;
}

/** @brief Reserviert Speicher für den String
 *
 * Mit dieser Funktion kann vor Verwendung des Strings vorgegeben werden, wieviel
 * Speicher initial reserviert werden soll. Dies ist insbesondere dann sinnvoll,
 * wenn der String während seiner Lebenszeit häufig verlängert wird.
 *
 * @param[in] size Anzahl Zeichen, für die Speicher reserviert werden soll.
 *
 * \note
 * Enthält der String bereits Zeichen, gehen diese nicht verloren, der existierende
 * Speicherbereich kann aber zwecks Vergrößerung umkopiert werden. Der Aufruf
 * der Funktion WideString::clear führt dazu, dass der Speicher wieder freigegeben wird.
 *
 */
void WideString::reserve(size_t size)
{
    if (size >= (std::numeric_limits<size_t>::max() / sizeof(wchar_t)) - 1) throw IllegalArgumentException("size is too large");

    if (size <= s) return; // Nothing to do
    size_t bytes = (size + 1) * sizeof(wchar_t);
    wchar_t* p = (wchar_t*)realloc(ptr, bytes);
    if (!p) throw OutOfMemoryException();
    ptr = p;
    s = size;
    ptr[stringlen] = 0;
}

/*!\brief Länge des Strings
 *
 * \desc
 * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
 *
 * \note
 * Die Funktionen WideString::len, WideString::length und WideString::size sind identisch.
 * \see
 * WideString::capacity
 *
 * @return Anzahl Zeichen
 */
size_t WideString::len() const
{
    return stringlen;
}

/*!\brief Länge des Strings
 *
 * \desc
 * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
 *
 * \note
 * Die Funktionen WideString::len, WideString::length und WideString::size sind identisch.
 * \see
 * WideString::capacity
 *
 * @return Anzahl Zeichen
 */
size_t WideString::length() const
{
    return stringlen;
}

/*!\brief Länge des Strings
 *
 * \desc
 * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
 *
 * \note
 * Die Funktionen WideString::len, WideString::length und WideString::size sind identisch.
 * \see
 * WideString::capacity
 *
 * @return Anzahl Zeichen
 */
size_t WideString::size() const
{
    return stringlen;
}

/*!\brief Länge des Strings in Byte
 *
 * \desc
 * Diese Funktion gibt die Anzahl Byte zurück, die durch den String belegt werden.
 *
 * @return Anzahl Bytes
 */
size_t WideString::byteLength() const
{
    return stringlen * sizeof(wchar_t);
}

/*! \brief Prüft, ob der String leer ist.
 *
 * \desc
 * Diese Funktion prüft, ob der String leer ist.
 *
 * \returns Ist der String leer, liefert die Funktion \c true zurück, sonst \c false.
 * \see WideString::notEmpty
 */
bool WideString::isEmpty() const
{
    if (stringlen == 0) return true;
    return false;
}

/*! \brief Prüft, ob der String Zeichen enthält
 *
 * \desc
 * Diese Funktion prüft, ob der String Zeichen enthält.
 *
 * \returns Enthält der String Zeichen, liefert die Funktion \c true zurück, sonst \c false.
 * \see WideString::isEmpty
 */
bool WideString::notEmpty() const
{
    if (stringlen == 0) return false;
    return true;
}

/*!\brief Prüft, ob der String nummerisch ist
 *
 * \desc
 * Diese Funktion prüft, ob im String nur nummerische Zeichen vorhanden sind, also die Ziffern
 * 0-9, Punkt, Komma und Minus.
 *
 * @return Ist der String nummerisch, wird 1 zurückgegeben. Ist er es nicht oder ist der String
 * leer, wird 0 zurückgegeben.
 */
bool WideString::isNumeric() const
{
    if (!stringlen) return false;
    size_t dotcount = 0;
    for (size_t i = 0; i < stringlen; i++) {
        wchar_t c = ((wchar_t*)ptr)[i];
        if (c < '0' || c > '9') {
            if (c != '.' && c != ',' && c != '-') return false;
            if (c == '-' && i > 0) return false;
            if (c == '.' || c == ',') {
                dotcount++;
                if (dotcount > 1) return false;
            }
        }
    }
    if (ptr[stringlen - 1] == '.') return false;
    return true;
}

/*!\brief Prüft, ob der String einen Integer Wert enthält
 *
 * \desc
 * Diese Funktion prüft, ob im String einen integer Wert enthält, also nur die Ziffern
 * 0-9 und optional ein Minus am Anfang enthalten sind
 *
 * @return Ist der String ein Integer, wird 1 zurückgegeben. Ist er es nicht oder ist der String
 * leer, wird 0 zurückgegeben.
 */
bool WideString::isInteger() const
{
    if (!stringlen) return false;
    for (size_t i = 0; i < stringlen; i++) {
        wchar_t c = ((wchar_t*)ptr)[i];
        if (c < '0' || c > '9') {
            if (c == '-' && i == 0) continue; // Minus am Anfang ist erlaubt
            return false;
        }
    }
    return true;
}

/*!\brief Prüft, ob der String "wahr" ist
 *
 * Diese Funktion überprüft den aktuellen String, ob er "wahr" ist. Dies ist der Fall,
 * wenn eine der folgenden Bedingungen erfüllt ist:
 * - Der String enthält eine Ziffer ungleich 0
 * - Der String enthält das Wort "true" (Gross- oder Kleingeschrieben)
 * - Der String enthält das Wort "wahr" (Gross- oder Kleingeschrieben)
 * - Der String enthält das Wort "yes" (Gross- oder Kleingeschrieben)
 * - Der String enthält das Wort "ja" (Gross- oder Kleingeschrieben)
 *
 * \returns Liefert true (1) zurück, wenn der String "wahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
 * \see CWWideString::IsFalse()
 */
bool WideString::isTrue() const
{
    if (!stringlen) return false;
    if (wcstol(ptr, NULL, 0) != 0) return true;
    if (strCaseCmp(L"true") == 0) return true;
    if (strCaseCmp(L"wahr") == 0) return true;
    if (strCaseCmp(L"ja") == 0) return true;
    if (strCaseCmp(L"yes") == 0) return true;
    if (strCaseCmp(L"t") == 0) return true;
    return false;
}

/*!\brief Prüft, ob der String "unwahr" ist
 *
 * Diese Funktion überprüft den aktuellen String, ob er "unwahr" ist. Dies ist der Fall,
 * wenn eine der folgenden Bedingungen erfüllt ist:
 * - Der String zeigt auf NULL
 * - Die Länge des Strings ist 0
 * - Der String enthält die Ziffer 0
 * - Der String enthält nicht das Wort "true", "wahr", "yes" oder "ja" (Gross-/Kleinschreibung egal)
 *
 * \returns Liefert true (1) zurück, wenn der String "unwahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
 * \see CWWideString::IsTrue()
 */
bool WideString::isFalse() const
{
    if (isTrue()) return false;
    return true;
}

/*!\brief String anhand eines C-Strings setzen
 *
 * \desc
 * Mit dieser Funktion wird der String anhand eines char * gesetzt. Dabei wird er
 * intern nach Unicode konvertiert.
 *
 * \param str Pointer auf einen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 * \exception UnsupportedFeatureException
 * \exception UnsupportedCharacterEncodingException
 * \exception CharacterEncodingException
 *
 * \note
 * Multibyte-Characters zählen als ein Zeichen.
 *
 */
WideString& WideString::set(const char* str, size_t size)
{
    if (!str) {
        clear();
        return *this;
    }
    size_t inbytes = (size != (size_t)-1) ? size : ::strlen(str);
    if (inbytes > ::strlen(str)) inbytes = ::strlen(str);
    if (inbytes == 0) {
        clear();
        return *this;
    }
    reserve(inbytes);
    // Temp-Kopie anlegen, falls der Eingabe-String nicht nullterminiert ist
    String temp_str;
    if (size != (size_t)-1) {
        temp_str.set(str, inbytes);
        str = temp_str.c_str();
    }
    size_t ret = ::mbstowcs(ptr, str, inbytes + 1);
    if (ret == (size_t)-1) {
        clear();
        throw CharacterEncodingException();
    }

    stringlen = ret;
    ptr[stringlen] = 0;
    return *this;
}

/*!\brief String anhand eines wchar_t* setzen
 *
 * \desc
 * Mit dieser Funktion wird der String anhand eines wchar_t * gesetzt.
 *
 * \param str Pointer auf einen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
WideString& WideString::set(const wchar_t* str, size_t size)
{
    if (!str) {
        clear();
        return *this;
    }
    size_t inchars = (size != (size_t)-1) ? size : ::wcslen(str);
    if (inchars > ::wcslen(str)) inchars = ::wcslen(str);
    if (inchars == 0) {
        clear();
        return *this;
    }
    // Self-Assignment Schutz
    WideString temp_holder;
    if (str >= ptr && str < ptr + stringlen) {
        temp_holder.set(str, inchars);
        str = temp_holder.getPtr();
    }

    if (inchars > s) {
        reserve(inchars);
    }
    wmemmove(ptr, str, inchars);
    stringlen = inchars;
    ((wchar_t*)ptr)[stringlen] = 0;
    return *this;
}

/*!\brief Wert eines anderen Strings übernehmen
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings \p str übernommen.
 *
 * \param str Referenz auf einen anderen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
WideString& WideString::set(const WideString& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.stringlen;
    if (inbytes > str.stringlen) inbytes = str.stringlen;
    return set((wchar_t*)str.ptr, inbytes);
}

WideString& WideString::set(const String& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return set((const char*)str.c_str(), inbytes);
}

/*!\brief Wert eines Strings der STL übernehmen
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des STL-Strings \p str übernommen.
 *
 * \param str Referenz auf einen String der Standard Template Library (STL)
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
WideString& WideString::set(const std::string& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return set((const char*)str.c_str(), inbytes);
}

/*!\brief Wert eines Wide-Strings der STL übernehmen
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des STL-Wide-Strings \p str übernommen.
 *
 * \param str Referenz auf einen Wide-String der Standard Template Library (STL)
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
WideString& WideString::set(const std::wstring& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return set(str.c_str(), inbytes);
}

/*!\brief Einzelnes Zeichen ersetzen
 *
 * \desc
 * Mit dieser Funktion wird ein einzelnes Zeichen eines Strings an der Position
 * \p position durch das Zeichen \p c ersetzt.
 *
 * @param position Position innerhalb des Strings (Zählung beginnt bei 0)
 * @param c Unicode-Wert, der gesetzt werden soll
 * @return Referenz auf den String
 * \throw OutOfBoundsException: Wird geworfen, wenn \p position größer ist, als die
 * Länge des Strings
 */
WideString& WideString::set(size_t position, wchar_t c)
{
    if (position >= stringlen) throw OutOfBoundsException();
    ptr[position] = c;
    return *this;
}

/*! \brief Erzeugt einen formatierten String
 *
 * \desc
 * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
 * und den optionalen Parametern \p ...
 *
 * \param fmt Der Formatstring
 * \param ... Optionale Parameter
 *
 * @return Referenz auf den String
 *
 * \par Example:
 * \code
int main(int argc, char **argv)
{
    String s;
    s.setf ("Anzahl Parameter: %i\n",argc);
    s.printnl();
    return 0;
}
\endcode
 *
 * \copydoc sprintf.dox
 */
WideString& WideString::setf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    this->vasprintf(fmt, args);
    va_end(args);
    return *this;
}

WideString WideString::format(const char* fmt, ...)
{
    WideString s;
    va_list args;
    va_start(args, fmt);
    s.vasprintf(fmt, args);
    va_end(args);
    return s;
}

/*!\brief Einzelnes Unicode-Zeichen übernehmen
 *
 * \desc
 * Ein einzelnes Unicode-Zeichen \p c wird in den String übernommen.
 *
 * @param c Unicode-Wert des gewünschten Zeichens
 *
 * @return Referenz auf den String
 */
WideString& WideString::set(wchar_t c)
{
    wchar_t buffer[2];
    buffer[0] = c;
    buffer[1] = 0;
    return set(buffer, 1);
}

/*! \brief Erzeugt einen formatierten String
 *
 * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
 * und den optionalen Parametern in \p args.
 *
 * \param fmt Der Formatstring
 * \param args Pointer auf Liste der Parameter. Muss zuvor mit va_start initialisiert worden sein.
 * @return Referenz auf den String
 * \par Example:
 * \code
void MyFunction(const char *fmt, ...)
{
    String s;
    va_list args;
    va_start(args, fmt);
    s.vasprintf(fmt,args);
    va_end(args);
    printf ("Ergebnis: %ls",(const wchar_t*)s);
}
\endcode
 *
 * \copydoc sprintf.dox
 */
WideString& WideString::vasprintf(const char* fmt, va_list args)
{
    String tmp;
    tmp.vasprintf(fmt, args);
    set(tmp);
    return *this;
}

/*!\brief Fügt einen Wide-Character String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen Wide-Character String an das Ende des bestehenden an
 *
 * \param[in] str Pointer auf einen Wide-Character String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::append(const wchar_t* str, size_t size)
{
    if (str == NULL || size == 0) return *this;
    if (stringlen == 0) {
        return set(str, size);
    }
    size_t inchars = (size != (size_t)-1) ? size : ::wcslen(str);

    // Self-Append Schutz: Zeigt "str" auf unseren eigenen Speicher block?
    WideString temp_holder;
    if (str >= ptr && str < ptr + stringlen) {
        temp_holder.set(str, inchars);
        str = temp_holder.getPtr(); // Zeigt jetzt auf einen sicheren Stack-Puffer
    }

    size_t required_size = (stringlen + inchars);
    if (required_size > s) {
        // Geometrische Vergrößerung, um die Anzahl der Speicherallokationen zu reduzieren
        size_t newbuffersize = s * 2;
        if (newbuffersize < required_size) {
            newbuffersize = required_size + 16; // Fallback, falls Verdopplung nicht reicht
        }
        reserve(newbuffersize);
    }
    wmemcpy(((wchar_t*)ptr) + stringlen, str, inchars);
    stringlen += inchars;
    ((wchar_t*)ptr)[stringlen] = 0;
    return *this;
}

/*!\brief Fügt einen C-String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen C-String an das Ende des bestehenden an. Der String muss entweder
 * UTF-8 kodiert sein, oder es muss mit der statischen Funktion WideString::setGlobalEncoding
 * zuvor eine andere Kodierung gesetzt worden sein.
 *
 * \param[in] str Pointer auf einen Wide-Character String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 * \exception UnsupportedFeatureException
 * \exception UnsupportedCharacterEncodingException
 * \exception CharacterEncodingException
 *
 */
WideString& WideString::append(const char* str, size_t size)
{
    WideString a;
    a.set(str, size);
    return append((wchar_t*)a.ptr, a.stringlen);
}

/*!\brief Fügt einen String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen String an das Ende des bestehenden an.
 *
 * \param[in] str Referenz auf ein String-Objekt
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::append(const WideString& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return append((wchar_t*)str.ptr, inbytes);
}

WideString& WideString::append(const String& str, size_t size)
{
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return append((const char*)str.c_str(), inbytes);
}

/*!\brief Fügt einen std::string an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen std::string an das Ende des bestehenden an.
 *
 * \param[in] str Referenz auf ein String-Objekt der STL
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::append(const std::string& str, size_t size)
{
    WideString a;
    a.set(str, size);
    return append((wchar_t*)a.ptr, a.stringlen);
}

/*!\brief Fügt einen std::wstring an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen std::wstring an das Ende des bestehenden an.
 *
 * \param[in] str Referenz auf ein Wide-String-Objekt der STL
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::append(const std::wstring& str, size_t size)
{
    WideString a;
    a.set(str, size);
    return append((wchar_t*)a.ptr, a.stringlen);
}

/*!\brief Fügt einen Formatierten String an das Ende des bestehenden an
 *
 * Anhand des übergebenen Formatstrings \p fmt und den optionalen Parametern \p ...
 * wird ein neuer String gebildet, der an das Ende des bestehenden angehangen wird
 *
 * \param fmt Der Formatstring
 * \param ... Optionale Parameter
  * @return Referenz auf den String
 *
 * \par Example:
 * \code
int main(int argc, char **argv)
{
    String s;
    s="Hallo Welt!";
    s.appendf (" Es wurden %i Parameter übergeben\n",argc);
    s.printnl();
    return 0;
}
\endcode
 *
 * \copydoc sprintf.dox
 */
WideString& WideString::appendf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String s;
    s.vasprintf(fmt, args);
    va_end(args);
    return append(WideString(s));
}

/*!\brief Einzelnes Unicode-Zeichen anhängen
 *
 * \desc
 * Ein einzelnes Unicode-Zeichen \p c wird in an den String angehangen.
 *
 * @param c Unicode-Wert des gewünschten Zeichens
 *
 * @return Referenz auf den String
 */
WideString& WideString::append(wchar_t c)
{
    wchar_t buffer[2];
    buffer[0] = c;
    buffer[1] = 0;
    return append(buffer, 1);
}

/*!\brief Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Pointer auf einen Wide-Character String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::prepend(const wchar_t* str, size_t size)
{
    if (str == NULL || size == 0) return *this;
    if (stringlen == 0) {
        return set(str, size);
    }
    size_t inchars = (size != (size_t)-1) ? size : ::wcslen(str);

    // Self-Prepend Schutz: Zeigt "str" auf unseren eigenen Speicher block?
    WideString temp_holder;
    if (str >= ptr && str < ptr + stringlen) {
        temp_holder.set(str, inchars);
        str = temp_holder.getPtr();
    }

    size_t required_size = stringlen + inchars;
    if (required_size > s) {
        // Geometrische Vergrößerung, um die Anzahl der Speicherallokationen zu reduzieren
        size_t newbuffersize = s * 2;
        if (newbuffersize < required_size) {
            newbuffersize = required_size + 16; // Fallback, falls Verdopplung nicht reicht
        }
        reserve(newbuffersize);
    }
    // Bestehenden Speicherblock nach hinten moven
    wmemmove(((wchar_t*)ptr) + inchars, ptr, stringlen);
    // Neuen Speicherblock davor kopieren
    wmemcpy(ptr, str, inchars);
    stringlen += inchars;
    ((wchar_t*)ptr)[stringlen] = 0;
    return *this;
}

/*!\brief Fügt einen String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Referenz auf einen String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::prepend(const WideString& str, size_t size)
{
    if (stringlen == 0) {
        set(str, size);
        return *this;
    }
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return prepend((wchar_t*)str.ptr, inbytes);
}

WideString& WideString::prepend(const String& str, size_t size)
{
    if (stringlen == 0) {
        set(str, size);
        return *this;
    }
    size_t inbytes;
    if (size != (size_t)-1)
        inbytes = size;
    else
        inbytes = str.length();
    if (inbytes > str.length()) inbytes = str.length();
    return prepend(str.c_str(), inbytes);
}

/*!\brief Fügt einen std::string der STL am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen std::string der Standard Template Library am Anfang des bestehenden Strings ein
 *
 * \param[in] str Referenz auf einen std::string
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::prepend(const std::string& str, size_t size)
{
    if (!ptr) {
        set(str, size);
        return *this;
    }
    WideString a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

/*!\brief Fügt einen std::wstring der STL am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen std::wstring der Standard Template Library am Anfang des bestehenden Strings ein
 *
 * \param[in] str Referenz auf einen std::wstring
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::prepend(const std::wstring& str, size_t size)
{
    if (!ptr) {
        set(str, size);
        return *this;
    }
    WideString a;
    a.set(str, size);
    return prepend(a.ptr, a.stringlen);
}

/*!\brief Fügt einen C-String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen C-String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Pointer auf einen C-String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
WideString& WideString::prepend(const char* str, size_t size)
{
    WideString a;
    a.set(str, size);
    return prepend((wchar_t*)a.ptr, a.stringlen);
}

/*!\brief Fügt einen Formatierten String am Anfang bestehenden ein
 *
 * Anhand des übergebenen Formatstrings \p fmt und den optionalen Parametern \p ...
 * wird ein neuer String gebildet, der am Anfang des bestehenden eingehangen wird.
 *
 * \param fmt Der Formatstring
 * \param ... Optionale Parameter
 * \par Example:
 * \code
int main(int argc, char **argv)
{
    String s;
    s="Vielen Dank!\n";
    s.prependf ("Es wurden %i Parameter übergeben. ",argc);
    s.printnl();
    return 0;
}
\endcode
 *
 * \copydoc sprintf.dox
 */
WideString& WideString::prependf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String s;
    s.vasprintf(fmt, args);
    va_end(args);
    return prepend(WideString(s));
}

/*!\brief Einzelnes Unicode-Zeichen am Anfang einfügen
 *
 * \desc
 * Ein einzelnes Unicode-Zeichen \p c wird in am Anfang des Strings eingefügt.
 * Die nachfolgenden Zeichen des Strings verschieben sich nach rechts.
 *
 * @param c Unicode-Wert des gewünschten Zeichens
 *
 * @return Referenz auf den String
 */
WideString& WideString::prepend(wchar_t c)
{
    wchar_t buffer[2];
    buffer[0] = c;
    buffer[1] = 0;
    return prepend(buffer, 1);
}

ByteArray WideString::toUtf8() const
{
    if (stringlen == 0) return ByteArray();

    // 1. Allokieren des maximal benötigten Speichers (max 4 Bytes pro Character)
    size_t max_bytes = stringlen * 4 + 1;
    ByteArray tmp_buffer;
    char* buffer = (char*)tmp_buffer.malloc(max_bytes);
    size_t dest_len = 0;

    for (size_t i = 0; i < stringlen; i++) {
        uint32_t codepoint = ptr[i];

// Falls wchar_t 16-Bit ist (Windows), müssen wir Surrogate-Pairs behandeln
#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
            // High Surrogate - prüfen ob das nächste Zeichen ein Low Surrogate ist
            if (i + 1 < stringlen) {
                uint32_t low = ptr[i + 1];
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                    i++; // Nächstes wchar_t überspringen (da im selben Codepoint verwendet)
                }
            }
        }
#endif

        // UTF-8 Kodierung schreiben (Direktzugriff)
        if (codepoint < 0x80) {
            buffer[dest_len++] = (char)codepoint;
        } else if (codepoint < 0x800) {
            buffer[dest_len++] = (char)(0xC0 | (codepoint >> 6));
            buffer[dest_len++] = (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint < 0x10000) {
            buffer[dest_len++] = (char)(0xE0 | (codepoint >> 12));
            buffer[dest_len++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
            buffer[dest_len++] = (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint < 0x200000) {
            buffer[dest_len++] = (char)(0xF0 | (codepoint >> 18));
            buffer[dest_len++] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
            buffer[dest_len++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
            buffer[dest_len++] = (char)(0x80 | (codepoint & 0x3F));
        }
    }
    buffer[dest_len] = 0;
    tmp_buffer.truncate(dest_len);
    return tmp_buffer;
}

ByteArray WideString::toUCS4() const
{
    if (stringlen == 0) return ByteArray();

    ByteArray ret;
    // Jedes UCS-4 Zeichen belegt exakt 4 Byte. Wir reservieren Platz für max. stringlen Elemente +1
    uint32_t* ucs4 = (uint32_t*)ret.malloc((stringlen + 1) * sizeof(uint32_t));
    size_t dest_len = 0;

    for (size_t i = 0; i < stringlen; i++) {
        uint32_t codepoint = ptr[i];

#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
            // High Surrogate - prüfen ob das nächste Zeichen ein Low Surrogate ist
            if (i + 1 < stringlen) {
                uint32_t low = ptr[i + 1];
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                    i++; // Nächstes wchar_t überspringen
                }
            }
        }
#endif
        ucs4[dest_len++] = codepoint;
    }

    // Explizite Nullterminierung (UCRT / POSIX kompatibel)
    ucs4[dest_len] = 0;

    // Logische Größe des ByteArrays in Bytes anpassen
    ret.truncate(dest_len * sizeof(uint32_t));
    return ret;
}

WideString& WideString::fromUCS4(const uint32_t* str, size_t size)
{
    clear();
    if (!str) return *this;

    for (size_t i = 0; (size == (size_t)-1 || i < size) && str[i] != 0; i++) {
        uint32_t codepoint = str[i];

#if defined(_WIN32) || (defined(__SIZEOF_WCHAR_T__) && __SIZEOF_WCHAR_T__ == 2)
        if (codepoint > 0xFFFF) {
            // Encode als UTF-16 Surrogate Pair
            codepoint -= 0x10000;
            wchar_t high = (wchar_t)(0xD800 + (codepoint >> 10));
            wchar_t low = (wchar_t)(0xDC00 + (codepoint & 0x3FF));
            append(high);
            append(low);
        } else {
            append((wchar_t)codepoint);
        }
#else
        append((wchar_t)codepoint); // Auf Linux/FreeBSD ist wchar_t bereits 32-Bit
#endif
    }
    return *this;
}

WideString& WideString::fromUCS4(const ByteArrayPtr& bin)
{
    return fromUCS4((uint32_t*)bin.ptr(), bin.size() / sizeof(uint32_t));
}

String WideString::toString() const
{
    return String((const wchar_t*)ptr, stringlen);
}

/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Unicode-Wert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
 * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
 * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
 * dem letzten Zeichen des Strings entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Unicode-Wert des Zeichens
 * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
wchar_t WideString::get(ssize_t pos) const
{
    if (pos >= 0 && stringlen > (size_t)pos) return ptr[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ptr[stringlen + pos];
    throw OutOfBoundsException();
}

/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit diesem Operator kann der Unicode-Wert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
 * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
 * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
 * dem letzten Zeichen des Strings entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Unicode-Wert des Zeichens
 * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
wchar_t WideString::operator[](ssize_t pos) const
{
    if (pos >= 0 && stringlen > (size_t)pos) return ptr[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ptr[stringlen + pos];
    throw OutOfBoundsException();
}

wchar_t& WideString::operator[](ssize_t pos)
{
    if (pos >= 0 && stringlen > (size_t)pos) return ptr[pos];
    if (pos < 0 && (size_t)(0 - pos) <= stringlen) return ptr[stringlen + pos];
    throw OutOfBoundsException();
}

/*!\brief String auf STDOUT ausgeben
 *
 * Diese Funktion gibt den aktuellen String auf STDOUT aus. Dazu ist es notwendig den String vom internen
 * Unicode-Format in das Encoding des Betriebssystems umzurechnen. Da dieses von den lokalen Einstellungen
 * des Betriebssystems und des Users abhängig ist, wird die Environment-Variable "LANG" ausgewertet.
 * Ist diese nicht gesetzt oder enthält ein unbekanntes Encoding, wird der String immer in UTF-8 ausgegeben.
 *
 * \param withNewline Ein optionaler Parameter, der angibt, ob nach der Ausgabe ein Zeilenumbruch
 * angehangen werden soll (default=false)
 * \par Exceptions:
 * Keine
 *
 */
void WideString::print(bool withNewline) const throw()
{
    if (ptr != NULL && stringlen > 0) {
        if (withNewline)
            printf("%ls\n", (wchar_t*)ptr);
        else
            printf("%ls", (wchar_t*)ptr);
    } else if (withNewline) {
        printf("\n");
    }
}

/*!\brief String auf STDOUT mit abschließendem Zeilenumbruch ausgeben
 *
 * Diese Funktion gibt den aktuellen String mit abschließendem Zeilenumbruch auf STDOUT aus.
 * Dazu ist es notwendig den String vom internen
 * Unicode-Format in das Encoding des Betriebssystems umzurechnen. Da dieses von den lokalen Einstellungen
 * des Betriebssystems und des Users abhängig ist, wird die Environment-Variable "LANG" ausgewertet.
 * Ist diese nicht gesetzt oder enthält ein unbekanntes Encoding, wird der String immer in UTF-8 ausgegeben.
 *
 * \par Exceptions:
 * Keine
 *
 */
void WideString::printnl() const throw()
{
    print(true);
}

/*!\brief Hexdump des Strings ausgeben
 *
 * \desc
 * Mit dieser zu Debug-Zwecken gedachten Funktion wird der Inhalt des
 * Strings als HexDump auf der Konsole ausgegeben.
 */
void WideString::hexDump() const
{
    PrintDebug("HEXDUMP of String %p: %zi Bytes starting at Address %p:\n", this, stringlen * sizeof(wchar_t), ptr);
    if (stringlen) HexDump(ptr, stringlen * sizeof(wchar_t), true);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion WideString::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(const char* str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion WideString::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(const wchar_t* str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion WideString::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(const WideString& str)
{
    return set(str);
}

WideString& WideString::operator=(WideString&& other) noexcept
{
    if (this != &other) {
        free(ptr);
        ptr = other.ptr;
        s = other.s;
        stringlen = other.stringlen;
        other.ptr = NULL;
        other.s = 0;
        other.stringlen = 0;
    }
    return *this;
}
/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion WideString::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(const String& str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion WideString::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(const std::string& str)
{
    return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion WideString::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(const std::wstring& str)
{
    return set(str);
}

/*!\brief Zeichen übernehmen
 *
 * \desc
 * Mit diesem Operator wird ein einzelnes Zeichen in den String kopiert.
 *
 * @param[in] c Unicode Wert des zu übernehmenden Zeichens
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator=(wchar_t c)
{
    return set(c);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator+=(const char* str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator+=(const wchar_t* str)
{
    return append(str);
}

WideString& WideString::operator+=(const String& str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator+=(const WideString& str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator+=(const std::string& str)
{
    return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator+=(const std::wstring& str)
{
    return append(str);
}

/*!\brief Zeichen anhängen
 *
 * \desc
 * Mit diesem Operator wird das angegebene Zeichen \p c an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
 *
 * @param[in] c Unicode-Wert des anzuhängenden Zeichens
 * @return Referenz auf diese Instanz der Klasse
 */
WideString& WideString::operator+=(wchar_t c)
{
    return append(c);
}

/*!\brief Führt einen Vergleich mit einem anderen String durch
 *
 * \desc
 * Führt einen Vergleich mit einem anderen String durch.
 *
 * \param str String, mit dem verglichen werden soll
 * \param size Optionaler Parameter, der die Anzahl zu berücksichtigender Zeichen innerhalb des
 * Strings \p str angibt. Wird er nicht angegeben, wird ein vergleich mit dem kompletten String
 * \p str durchgeführt.
 *
 * \return Ist der String innerhalb dieses Objekts kleiner als der mit \a str angegebene, wird ein
 * negativer Wert zurückgegeben, ist er größer, erfolgt ein positiver Return-Wert,
 * sind beide identisch, wird 0 zurückgegeben.
 *
 * \see strCaseCmp Vergleich zweier Strings unter Ignorierung der Gross-/Kleinschreibung
 */
int WideString::strcmp(const WideString& str, size_t size) const
{
    const wchar_t* mystr = ptr;
    const wchar_t* otherstr = str.ptr;
    if (stringlen == 0) mystr = L"";
    if (str.stringlen == 0) otherstr = L"";
    if (size != (size_t)-1) return wcsncmp(mystr, otherstr, size);
    return wcscmp(mystr, otherstr);
}

/*!\brief Stringvergleich mit Ignorierung von Gross-/Kleinschreibung
 *
 * \desc
 * Führt einen Vergleich mit einem anderen String durch, unter Ignorierung der
 * Gross-/Kleinschreibung.
 *
 * \param str String, mit dem verglichen werden soll
 * \param size Optionaler Parameter, der die Anzahl zu berücksichtigender Zeichen innerhalb des
 * Strings \p str angibt. Wird er nicht angegeben, wird ein vergleich mit dem kompletten String
 * \p str durchgeführt.
 *
 *
 * \return Ist der String innerhalb dieses Objekts kleiner als der mit \a str angegebene, wird ein
 * negativer Wert zurückgegeben, ist er größer, erfolgt ein positiver Return-Wert,
 * sind beide identisch, wird 0 zurückgegeben.
 *
 * \see strcmp Vergleich zweier Strings unter Berücksichtigung der Gross-/Kleinschreibung
 */
int WideString::strCaseCmp(const WideString& str, size_t size) const
{
    const wchar_t* mystr = ptr ? ptr : L"";
    const wchar_t* otherstr = str.ptr ? str.ptr : L"";
#ifndef _WIN32
    if (size != (size_t)-1) return wcsncasecmp(mystr, otherstr, size);
    return wcscasecmp(mystr, otherstr);
#else
    if (size != (size_t)-1) return _wcsnicmp(mystr, otherstr, size);
    return _wcsicmp(mystr, otherstr);
#endif
}

/*!\brief Linken Teilstring zurückgeben
 *
 * \desc
 * Gibt die ersten \p len Zeichen des Strings als neuen zurück.
 *
 * @param len Länge des Teilstrings
 * @return Neuer String
 */
WideString WideString::left(size_t len) const
{
    if (len > stringlen) len = stringlen;
    return WideString(ptr, len);
}

/*!\brief Rechten Teilstring zurückgeben
 *
 * \desc
 * Gibt die letzten \p len Zeichen des Strings als neuen zurück.
 *
 * @param len Länge des Teilstrings
 * @return Neuer String
 */
WideString WideString::right(size_t len) const
{
    if (len > stringlen) len = stringlen;
    return WideString(ptr + stringlen - len, len);
}

/*!\brief Teilstring zurückgeben
 *
 * \desc
 * Gibt \p len Zeichen des Strings, beginnend ab Position \p start als
 * neuen String zurück.
 *
 * @param start Startposition
 * @param len Optionale Länge des Teilstrings. Ist der Parameter nicht angegeben, wird
 * der komplette String ab Position \p start zurückgegeben.
 * @return Neuer String
 */
WideString WideString::mid(size_t start, size_t len) const
{
    if (len == (size_t)-1) len = stringlen;
    if (start < stringlen && stringlen > 0 && len > 0) {
        if (start + len > stringlen) len = stringlen - start;
        return WideString(ptr + start, len);
    }
    return WideString();
}

/*!\brief Teilstring zurückgeben
 *
 * \desc
 * Gibt \p len Zeichen des Strings, beginnend ab Position \p start als
 * neuen String zurück.
 *
 * @param start Startposition
 * @param len Optionale Länge des Teilstrings. Ist der Parameter nicht angegeben, wird
 * der komplette String ab Position \p start zurückgegeben.
 * @return Neuer String
 */
WideString WideString::substr(size_t start, size_t len) const
{
    return mid(start, len);
}

/*! \brief Wandelt alle Zeichen des Strings in Kleinbuchstaben um
 *
 * \desc
 * Diese Funktion wandelt alle Zeichen des Strings in Kleinbuchstaben um. Die genaue Funktionsweise hängt davon ab,
 * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
 *
 * \attention Unter UNIX (und möglicherweise anderen Betriebssystemen) werden die Lokalisationseinstellungen der
 * Umgebung nicht automatisch übernommen, sondern stehen standardmäßig auf "C". Dadurch werden nur US-ASCII
 * (ASCII 32 bis 127) umgewandelt. Man sollte daher nach Programmstart mit "setlocale" die gewünschte
 * Spracheinstellung vornehmen.
 *
 * \example
 * \code
 * #include <locale.h>
 * ...
 * setlocale(LC_CTYPE,"de_DE.UTF-8");
 * \endcode
 * \par
 */
WideString& WideString::lowerCase()
{
    if (ptr != NULL && stringlen > 0) {
        for (size_t i = 0; i < stringlen; i++) {
            wchar_t wc = ptr[i];
            wchar_t c = towlower(wc);
            if (c != (wchar_t)WEOF) {
                ptr[i] = c;
            }
        }
    }
    return *this;
}

/*! \brief Wandelt alle Zeichen des Strings in Grossbuchstaben um
 *
 * \desc
 * Diese Funktion wandelt alle Zeichen des Strings in Großbuchstaben um. Die genaue Funktionsweise hängt davon ab,
 * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
 *
 * \attention Unter UNIX (und möglicherweise anderen Betriebssystemen) werden die Lokalisationseinstellungen der
 * Umgebung nicht automatisch übernommen, sondern stehen standardmäßig auf "C". Dadurch werden nur US-ASCII
 * (ASCII 32 bis 127) umgewandelt. Man sollte daher nach Programmstart mit "setlocale" die gewünschte
 * Spracheinstellung vornehmen.
 *
 * \example
 * \code
 * #include <locale.h>
 * ...
 * setlocale(LC_CTYPE,"de_DE.UTF-8");
 * \endcode
 */
WideString& WideString::upperCase()
{
    if (ptr != NULL && stringlen > 0) {
        for (size_t i = 0; i < stringlen; i++) {
            wchar_t wc = ptr[i];
            wchar_t c = towupper(wc);
            if (c != (wchar_t)WEOF) {
                ptr[i] = c;
            }
        }
    }
    return *this;
}

WideString WideString::toLowerCase() const
{
    WideString result = *this;
    result.lowerCase();
    return result;
}

WideString WideString::toUpperCase() const
{
    WideString result = *this;
    result.upperCase();
    return result;
}

/*!\brief Anfangsbuchstaben der Wörter groß
 *
 * \desc
 * Diese Funktion wandelt die Anfangsbuchstaben aller im String enthaltenen Wörter in
 * Großbuchstaben um.
 */
WideString& WideString::upperCaseWords()
{
    if (ptr != NULL && stringlen > 0) {
        bool wordstart = true;
        for (size_t i = 0; i < stringlen; i++) {
            wchar_t wc = ptr[i];
            if (wordstart) {
                wchar_t c = towupper(wc);
                if (c != (wchar_t)WEOF) {
                    ptr[i] = c;
                }
            }
            if (wc != '\'' && (wc < 48 || (wc > 57 && wc < 65) || (wc > 90 && wc < 97) || (wc > 122 && wc < 127))) {
                wordstart = true;
            } else {
                wordstart = false;
            }
        }
    }
    return *this;
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang und Ende des Strings ab
WideString& WideString::trim()
{
    if (stringlen == 0) return *this;

    size_t start = 0;
    while (start < stringlen && (ptr[start] == L' ' || ptr[start] == L'\t' || ptr[start] == L'\r' || ptr[start] == L'\n')) {
        start++;
    }

    if (start == stringlen) {
        clear();
        return *this;
    }

    size_t end = stringlen - 1;
    while (end > start && (ptr[end] == L' ' || ptr[end] == L'\t' || ptr[end] == L'\r' || ptr[end] == L'\n')) {
        end--;
    }

    size_t new_len = end - start + 1;
    if (start > 0) {
        wmemmove(ptr, ptr + start, new_len);
    }
    stringlen = new_len;
    ptr[stringlen] = 0;
    return *this;
}

WideString WideString::trimmed() const
{
    WideString result = *this;
    result.trim();
    return result;
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang des Strings ab
WideString& WideString::trimLeft()
{
    if (stringlen == 0) return *this;
    size_t start = 0;
    while (start < stringlen && (ptr[start] == L' ' || ptr[start] == L'\t' || ptr[start] == L'\r' || ptr[start] == L'\n')) {
        start++;
    }

    if (start == stringlen) {
        clear();
        return *this;
    }
    if (start > 0) {
        size_t new_len = stringlen - start;
        wmemmove(ptr, ptr + start, new_len + 1); // Kopiert den Null-Terminator mit
        stringlen = new_len;
    }
    return *this;
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Ende des Strings ab
WideString& WideString::trimRight()
{
    if (stringlen == 0) return *this;
    size_t end = stringlen;
    while (end > 0 && (ptr[end - 1] == L' ' || ptr[end - 1] == L'\t' || ptr[end - 1] == L'\r' || ptr[end - 1] == L'\n')) {
        end--;
    }
    if (end == 0) {
        clear();
    } else {
        stringlen = end;
        ptr[stringlen] = 0;
    }
    return *this;
}

//! \brief Schneidet die definierten Zeichen am Anfang des Strings ab
WideString& WideString::trimLeft(const WideString& chars)
{
    if (stringlen > 0 && chars.stringlen > 0) {
        size_t i, start, s, z;
        start = 0;
        s = 0;
        for (i = 0; i < stringlen; i++) {
            int match = 0;
            for (z = 0; z < chars.stringlen; z++) {
                if (ptr[i] == chars.ptr[z]) {
                    if (s == 0) start = i + 1;
                    match = 1;
                    break;
                }
            }
            if (!match) {
                s = 1;
            }
        }
        if (start > 0) {
            memmove(ptr, ptr + start, (stringlen - start + 1) * sizeof(wchar_t));
            stringlen = wcslen(ptr);
        }
    }
    return *this;
}

//! \brief Schneidet die definierten Zeichen am Ende des Strings ab
WideString& WideString::trimRight(const WideString& chars)
{
    if (stringlen > 0 && chars.stringlen > 0) {
        size_t i, ende, z;
        ende = 0;
        for (i = stringlen; i > 0; i--) {
            wchar_t w = ptr[i - 1];
            int match = 0;
            for (z = 0; z < chars.stringlen; z++) {
                if (w == chars.ptr[z]) {
                    // if (s==0) start=i+1;
                    match = 1;
                    break;
                }
            }
            if (!match) {
                ende = i;
                break;
            }
        }
        ptr[ende] = 0;
        stringlen = wcslen(ptr);
    }
    return *this;
}

//! \brief Schneidet die definierten Zeichen am Anfang und Ende des Strings ab
WideString& WideString::trim(const WideString& chars)
{
    trimLeft(chars);
    trimRight(chars);
    return *this;
}

/*!\brief Schneidet Zeichen am Ende des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 */
WideString& WideString::chopRight(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        stringlen -= num;
        ptr[stringlen] = 0;
    }
    return *this;
}

/*!\brief Schneidet Zeichen am Ende des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 *
 * \see
 * Die Funktion ist identisch zu WideString::chopRight
 */
WideString& WideString::chop(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        stringlen -= num;
        ptr[stringlen] = 0;
    }
    return *this;
}

/*!\brief Schneidet Zeichen am Anfang des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Anfang des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 */
WideString& WideString::chopLeft(size_t num)
{
    if (stringlen > 0) {
        if (stringlen < num) num = stringlen;
        memmove(ptr, ptr + num, (stringlen - num) * sizeof(wchar_t));
        stringlen -= num;
        ptr[stringlen] = 0;
    }
    return *this;
}

/*!\brief Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
 *
 * \desc
 * Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
 */
WideString& WideString::chomp()
{
    trim(L"\n\r");
    return *this;
}

/*!\brief Schneidet den String an einer bestimmten Stelle ab
 *
 * \desc
 * Der String wird an einer bestimmten Stelle einfach abgeschnitten
 * \param pos Die Position, an der der String abgeschnitten wird. Bei Angabe von 0 ist der String anschließend
 * komplett leer. Ist \c pos größer als die Länge des Strings, passiert nichts.
 */
WideString& WideString::cut(size_t pos)
{
    if (stringlen == 0) return *this;
    if (pos > stringlen) return *this;
    ptr[pos] = 0;
    stringlen = pos;
    return *this;
}

/*! \brief Schneidet den String beim ersten Auftauchen eines Zeichens/Strings ab
 *
 * Der String wird beim ersten Auftauchen eines Zeichens oder eines Strings abgeschnitten.
 * \param[in] letter Buchstabe oder Buchstabenkombination, an der der String abgeschnitten werden
 * soll. Zeigt der Pointer auf NULL oder ist der String leer, passiert nichts.
 */
WideString& WideString::cut(const WideString& letter)
{
    if (stringlen == 0) return *this;
    if (letter.isEmpty()) return *this;
    ssize_t p = instr(letter, 0);
    if (p >= 0) {
        ptr[p] = 0;
        stringlen = p;
    }
    return *this;
}

WideString WideString::strchr(wchar_t c) const
{
    WideString ret;
    if (stringlen > 0) {
        wchar_t* p = wcschr(ptr, c);
        if (p) ret.set(p);
    }
    return ret;
}

WideString WideString::strrchr(wchar_t c) const
{
    WideString ret;
    if (stringlen > 0) {
        wchar_t* p = wcsrchr(ptr, c);
        if (p) ret.set(p);
    }
    return ret;
}

/*!\brief Teilstring finden
 *
 * \desc
 * Diese Funktion findet die erste Position der Zeichenfolge \p needle
 * innerhalb des Strings. Abschließende `\0'-Zeichen werden nicht
 * miteinander verglichen.
 *
 * @param needle
 * @return
 * Die Funktion gibt einen String zurück, der mit der gefundenen
 * Zeichenkette beginnt und den Rest des Strings bis zum Ende enthält.
 * Wurde die Zeichenkette nicht gefunden, wird ein leerer String
 * zurückgegeben.
 * \note
 * Ein Sonderfall besteht, wenn \p needle leer ist. In diesem Fall wird
 * der komplette String zurückgegeben.
 */
WideString WideString::strstr(const WideString& needle) const
{
    WideString ret;
    if (stringlen > 0) {
        if (needle.len() == 0) return *this;
        wchar_t* p = wcsstr(ptr, needle.ptr);
        if (p) ret.set(p);
    }
    return ret;
}

/*! \brief Sucht nach einem String
 *
 * Find sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
 * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
 * Ende des Strings gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder WideString::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t WideString::find(const WideString& needle, ssize_t start) const
{
    if (stringlen == 0 || needle.stringlen == 0) return WideString::npos;
    if (start > 0 && (size_t)start >= stringlen) return WideString::npos;
    if (start < 0 && ((size_t)((ssize_t)stringlen + start)) >= stringlen) return WideString::npos;

    // Position to return
    size_t p = WideString::npos;
    // Length of the string to search for
    size_t lstr = needle.stringlen;
    // Current position to search from and position of found string
    wchar_t *found = NULL, *tmp = NULL;

    // Search forward
    if (start >= 0) {
        // Search first occurence, starting at the given position...
        found = wcsstr(ptr + start, needle.ptr);
        //...and calculate the position to return if str was found
        if (found != NULL) {
            p = found - ptr;
        }
    } else {
        // Search backward
        // Start counting from behind at 0, not -1
        start++;

        /* Beginning at the start of the contained string, start searching for
               every occurence of the str and make it the position last found as long
               as the found string doesn't exceed the defined end of the search */
        while ((found = wcsstr((tmp == NULL ? ptr : tmp + 1), needle.ptr)) != NULL && found - ptr + lstr <= stringlen + start)
            tmp = found;

        // Calculate the position to return if str was found
        if (tmp != NULL) {
            p = tmp - ptr;
        }
    }
    return p;
}

/*! \brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
 *
 * \desc
 * Diese Funktion sucht innerhalb des aktuellen String nach \p needle ab der
 * gewünschten Position \a start. Gross-/Kleinschreibung wird dabei ignoriert.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
 * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
 * Ende des Strings gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder WideString::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t WideString::findCase(const WideString& needle, ssize_t start) const
{
    WideString CaseNeedle(needle);
    WideString CaseSearch(ptr, stringlen);
    CaseNeedle.lowerCase();
    CaseSearch.lowerCase();
    return CaseSearch.find(CaseNeedle, start);
}

/*!\brief String wiederholen
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings mehrfach wiederholt.
 *
 * @param num Anzahl Wiederholungen. Falls \p num 0 ist, ist der String anschließend leer.
 *
 * @return Referenz auf den verlängerten String.
 */
WideString& WideString::repeat(size_t num)
{
    if (stringlen == 0) return *this;
    if (num == 0) {
        clear();
        return *this;
    }
    reserve(stringlen * num);
    wchar_t* tmp = ptr + stringlen;
    for (size_t i = 1; i < num; i++) {
        wmemcpy(tmp, ptr, stringlen);
        tmp += stringlen;
    }
    stringlen = stringlen * num;
    ptr[stringlen] = 0;
    return *this;
}

/*! \brief Füllt den String mit einem Zeichen
 *
 * Der String wird mit einem gewünschten Zeichen gefüllt
 * \param unicode Der Unicode des Zeichens, mit dem der String gefüllt werden soll
 * \param num Die Länge des gewünschten Strings
 * \return Referenz auf den neuen String
 */
WideString& WideString::repeat(wchar_t unicode, size_t num)
{
    if (!unicode || !num) {
        clear();
        return *this;
    }
    reserve(num);
    for (size_t i = 0; i < num; i++)
        ptr[i] = unicode;
    stringlen = num;
    ptr[stringlen] = 0;
    return *this;
}

/*!\brief String wiederholen
 *
 * \desc
 * Mit dieser Funktion wird der übergebene String \p str \p num mal wiederholt und
 * das Ergebnis in diesem String gespeichert.
 *
 * @param str Der zu wiederholende String
 * @param num Anzahl wiederholungen
 * @return Referenz auf den String
 */
WideString& WideString::repeat(const WideString& str, size_t num)
{
    if (str.stringlen == 0 || num == 0) {
        clear();
        return *this;
    }

    // Schutz vor Self-Repeat
    WideString temp_holder;
    const wchar_t* src_ptr = str.ptr;
    if (str.ptr >= ptr && str.ptr < ptr + stringlen) {
        temp_holder = str;
        src_ptr = temp_holder.getPtr();
    }
    reserve(str.stringlen * num);
    wchar_t* dst = ptr;
    for (size_t i = 0; i < num; i++) {
        wmemcpy(dst, src_ptr, str.stringlen);
        dst += str.stringlen;
    }
    stringlen = str.stringlen * num;
    ptr[stringlen] = 0;
    return *this;
}

/*!\brief String multiplizieren
 *
 * \desc
 * Der aktuelle String wird \p count mal hintereinander wiederholt und
 * als neuer String zurückgegeben.
 *
 * @param[in] count Anzahl wiederholungen
 * @return Neuer String
 * \exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
 */
WideString WideString::repeated(size_t count) const
{
    WideString ret;
    for (size_t i = 0; i < count; i++)
        ret.append(ptr, stringlen);
    return ret;
}

WideString& WideString::replace(const WideString& search, const WideString& replacement)
//! \brief Ersetzt einen Teilstring durch einen anderen
{
    if (stringlen == 0 || search.stringlen == 0) return *this;
    size_t start = 0, slen = search.stringlen;
    ssize_t end;
    // collect the result
    WideString ms;
    // Do while str is found in the contained string
    while ((end = find(search, start)) >= 0) {
        // The result is built from the parts that don't match str and the replacement string
        ms += mid(start, end - start);
        ms += replacement;
        // New start for search is behind the replaced part
        start = end + slen;
    }
    // Add the remaining part of the contained string to the result
    ms += mid(start);
    // The result is assigned to this mstring
    return set(ms);
}

/*!\brief Kleiner als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
 * rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator<(const WideString& str) const
{
    if (strcmp(str) < 0) return true;
    return false;
}

/*!\brief Kleiner oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator<=(const WideString& str) const
{
    if (strcmp(str) <= 0) return true;
    return false;
}

/*!\brief Gleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
 * rechten identisch ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator==(const WideString& str) const
{
    if (strcmp(str) == 0) return true;
    return false;
}

/*!\brief Ungleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
 * rechten entspricht.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator!=(const WideString& str) const
{
    if (strcmp(str) == 0) return false;
    return true;
}

/*!\brief Größer oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
 * gleich dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator>=(const WideString& str) const
{
    if (strcmp(str) >= 0) return true;
    return false;
}

/*!\brief Größer als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator>(const WideString& str) const
{
    if (strcmp(str) > 0) return true;
    return false;
}

/*!\brief Kleiner als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
 * rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator<(const wchar_t* str) const
{
    if (strcmp(str) < 0) return true;
    return false;
}

/*!\brief Kleiner oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator<=(const wchar_t* str) const
{
    if (strcmp(str) <= 0) return true;
    return false;
}

/*!\brief Gleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
 * rechten identisch ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator==(const wchar_t* str) const
{
    if (strcmp(str) == 0) return true;
    return false;
}

/*!\brief Ungleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
 * rechten entspricht.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator!=(const wchar_t* str) const
{
    if (strcmp(str) == 0) return false;
    return true;
}

/*!\brief Größer oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
 * gleich dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator>=(const wchar_t* str) const
{
    if (strcmp(str) >= 0) return true;
    return false;
}

/*!\brief Größer als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool WideString::operator>(const wchar_t* str) const
{
    if (strcmp(str) > 0) return true;
    return false;
}

/*!\brief %Pointer auf den internen Unicode-String
 *
 * \copydetails WideString::getPtr
 */
const wchar_t* WideString::toWchart() const
{
    if (stringlen == 0) return L"";
    return (const wchar_t*)ptr;
}

/*!\brief %Pointer auf den internen Unicode-String
 *
 * \copydetails WideString::getPtr
 */
WideString::operator const wchar_t*() const
{
    if (stringlen == 0) return L"";
    return (wchar_t*)ptr;
}

WideString::operator bool() const
{
    if (isTrue()) return true;
    return false;
}

WideString::operator int() const
{
    if (!stringlen) return 0;
    return wcstol(ptr, NULL, 0);
}

WideString::operator unsigned int() const
{
    if (!stringlen) return 0;
    return wcstol(ptr, NULL, 0);
}

WideString::operator int64_t() const
{
    return toInt64();
}

WideString::operator uint64_t() const
{
    return toUnsignedInt64();
}

WideString::operator float() const
{
    return toFloat();
}

WideString::operator double() const
{
    return toDouble();
}

WideString::operator std::string() const
{
    String s((const wchar_t*)ptr, stringlen);
    return std::string(s.c_str(), s.len());
}

WideString::operator std::wstring() const
{
    return std::wstring(ptr, stringlen);
}

int WideString::toInt() const
{
    if (!stringlen) return 0;
    return wcstol(ptr, NULL, 10);
}

unsigned int WideString::toUnsignedInt() const
{
    if (!stringlen) return 0;
    return wcstoul(ptr, NULL, 10);
}

int64_t WideString::toInt64() const
{
    if (!stringlen) return 0;
#ifndef _WIN32
    return (int64_t)wcstoll(ptr, NULL, 10);
#else
    return (int64_t)_wcstoi64(ptr, NULL, 10);
#endif
}

uint64_t WideString::toUnsignedInt64() const
{
    if (!stringlen) return 0;
#ifndef _WIN32
    return (uint64_t)wcstoull(ptr, NULL, 10);
#else
    return (uint64_t)_wcstoui64(ptr, NULL, 10);
#endif
}

bool WideString::toBool() const
{
    if (isTrue()) return true;
    return false;
}

long WideString::toLong() const
{
    if (!stringlen) return 0;
    return wcstol(ptr, NULL, 10);
}

unsigned long WideString::toUnsignedLong() const
{
    if (!stringlen) return 0;
    return wcstoul(ptr, NULL, 10);
}

long long WideString::toLongLong() const
{
    if (!stringlen) return 0;
#ifndef _WIN32
    return (long long)wcstoll(ptr, NULL, 10);
#else
    return (long long)_wcstoi64(ptr, NULL, 10);
#endif
}

unsigned long long WideString::toUnsignedLongLong() const
{
    if (!stringlen) return 0;
#ifndef _WIN32
    return (unsigned long long)wcstoull(ptr, NULL, 10);
#else
    return (unsigned long long)_wcstoui64(ptr, NULL, 10);
#endif
}

float WideString::toFloat() const
{
    if (!stringlen) return 0;
#ifndef _WIN32
    return (float)wcstof(ptr, NULL);
#else
    return (float)wcstod(ptr, NULL);
#endif
}

double WideString::toDouble() const
{
    if (!stringlen) return 0;
#ifndef _WIN32
    return wcstod(ptr, NULL);
#else
    return wcstod(ptr, NULL);
#endif
}

bool WideString::has(const WideString& needle, bool ignoreCase) const
{
    if (ignoreCase) {
        return findCase(needle) != WideString::npos;
    } else {
        return find(needle) != WideString::npos;
    }
}

bool WideString::startsWith(const WideString& prefix, size_t start, size_t end) const
{
    WideString part;
    if (start > 0 || end != (size_t)-1) {
        part = mid(start, end).left(prefix.size());
    } else {
        part = left(prefix.size());
    }
    return part == prefix;
}

bool WideString::endsWith(const WideString& suffix, size_t start, size_t end) const
{
    WideString part;
    if (start > 0 || end != (size_t)-1) {
        part = mid(start, end).right(suffix.size());
    } else {
        part = right(suffix.size());
    }
    return part == suffix;
}

WideString WideString::join(const pplib::Array& iterable) const
{
    return WideString(iterable.implode(*this));
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const WideString& str1, const WideString& str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const char* str1, const WideString& str2)
{
    WideString s;
    s.set(str1);
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const WideString& str1, const char* str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const wchar_t* str1, const WideString& str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const WideString& str1, const wchar_t* str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const std::string& str1, const WideString& str2)
{
    WideString s(str1);
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const WideString& str1, const std::string& str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const std::wstring& str1, const WideString& str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

/*!\brief String addieren
 *
 * \relates pplib::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
WideString operator+(const WideString& str1, const std::wstring& str2)
{
    WideString s = str1;
    s.append(str2);
    return s;
}

WideString operator+(const WideString& str1, const wchar_t c)
{
    WideString s = str1;
    s.append(c);
    return s;
}

WideString operator+(const wchar_t c, const WideString& str2)
{
    WideString s;
    s.set(c);
    s.append(str2);
    return s;
}

std::ostream& operator<<(std::ostream& s, const WideString& str)
{
    String a(str);
    return s.write((const char*)a.c_str(), a.size());
}

WideString::iterator WideString::begin() noexcept
{
    return ptr;
}

WideString::const_iterator WideString::begin() const noexcept
{
    return ptr;
}

WideString::const_iterator WideString::cbegin() const noexcept
{
    return ptr;
}

WideString::iterator WideString::end() noexcept
{
    return ptr + stringlen;
}

WideString::const_iterator WideString::end() const noexcept
{
    return ptr + stringlen;
}

WideString::const_iterator WideString::cend() const noexcept
{
    return ptr + stringlen;
}

WideString::reverse_iterator WideString::rbegin() noexcept
{
    return reverse_iterator(end());
}

WideString::const_reverse_iterator WideString::rbegin() const noexcept
{
    return const_reverse_iterator(end());
}

WideString::const_reverse_iterator WideString::crbegin() const noexcept
{
    return const_reverse_iterator(end());
}

WideString::reverse_iterator WideString::rend() noexcept
{
    return reverse_iterator(begin());
}

WideString::const_reverse_iterator WideString::rend() const noexcept
{
    return const_reverse_iterator(begin());
}

WideString::const_reverse_iterator WideString::crend() const noexcept
{
    return const_reverse_iterator(begin());
}

} // namespace pplib
