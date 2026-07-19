/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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
#ifndef PPL7_TYPES_STRING_H_
#define PPL7_TYPES_STRING_H_

#include <string>
#include <stdint.h>

namespace ppl7
{

class WideString;
class ByteArrayPtr;
class ByteArray;
class Array;

/**@class String
 * @ingroup PPLGroupDataTypes
 * @ingroup PPLGroupStrings
 * @brief String-Klasse
 *
 * Diese Klasse repräsentiert einen String, der beliebige Zeichen enthalten kann.
 * Der String wird intern als C-String mit 0-Byte am Ende gespeichert. Dabei kann es sich
 * um einen UTF-8 String, einen ASCII-String oder einen String in einer anderen Kodierung handeln.
 *
 * @note Bei der Konvertierung zwischen String und WideString wird die eingestellte Kodierung
 * verwendet, die mit der Funktion setlocale() aus der C-Standardbibliothek festgelegt wurde.
 * In der Regel ist der Defaultwert "C", daher sollte beim Start des Programms setlocale()
 * mit der gewünschten Kodierung aufgerufen werden.
 *
 * @see WideString
 */
class String
{
private:
    char* ptr;
    size_t s, stringlen;

public:
    //! @name Konstruktoren und Destruktor
    //@{

    /**@brief Default Konstruktor mit leeren String
     *
     * Es wird ein leerer String erstellt.
     */
    String() noexcept;

    /**@brief Konstruktor mit C-String
     *
     * Ein String wird aus einem C-String erstellt.
     *
     * @param str C-String mit 0-Byte am Ende
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String(const char* str);

    /**@brief Konstruktor mit C-String und Länge
     *
     * Ein String wird aus einem C-String erstellt. Es werden maximal \p size Zeichen übernommen.
     *
     * @param str C-String mit 0-Byte am Ende
     * @param size Maximale Anzahl Zeichen, die übernommen werden sollen
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String(const char* str, size_t size);

    explicit String(const wchar_t* str, size_t size = (size_t)-1);

    /**@brief Konstruktor mit anderem String
     *
     * Ein String wird aus einem anderen String erstellt.
     *
     * @param str Referenz auf einen anderen String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String(const String& str);

    /**@brief Konstruktor mit WideString
     *
     * Ein String wird aus einem WideString erstellt. Dabei wird der String
     * in die globale Kodierung konvertiert, die mit String::setGlobalEncoding festgelegt wurde.
     * Der Defaultwert ist UTF-8.
     *
     * @param str Referenz auf einen WideString
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String(const WideString& str);

    /**@brief Konstruktor mit ByteArrayPtr
     *
     * Ein String wird aus einem ByteArrayPtr erstellt. Dabei wird der String
     * in die globale Kodierung konvertiert, die mit String::setGlobalEncoding festgelegt wurde.
     * Der Defaultwert ist UTF-8.
     *
     * @param str Referenz auf einen ByteArrayPtr
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    explicit String(const ByteArrayPtr& str);

    /**@brief Konstruktor aus Standard-Template String
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
    String(const std::string& str);

    /**@brief Konstruktor aus Standard-Template Wide-String
     *
     * \desc
     * Ein String wird aus einem Wide-String der Standard-Template-Library (STL) erstellt.
     *
     * @param str Referenz auf Wide-String der STL
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String(const std::wstring& str);

    /**@brief Move-Konstruktor
     *
     * Ein String wird aus einem anderen String erstellt. Dabei wird der Speicher des anderen Strings übernommen.
     *
     * @param other Rvalue-Referenz auf einen anderen String
     * @exception Keine
     */
    String(String&& other) noexcept;

    /**@brief Destruktor
     *
     * Der String wird gelöscht und der Speicher freigegeben.
     */
    ~String() noexcept;
#ifdef WITH_QT
    String(const QString& q)
    {
        ptr = NULL;
        stringlen = 0;
        s = 0;
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        set((const char*)a);
    }
    String(QString* q)
    {
        ptr = NULL;
        stringlen = 0;
        s = 0;
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q->toUtf8();
#else
        QByteArray a = q->toLocal8Bit();
#endif
        set((const char*)a);
    }
#endif
    //@}

    /**\brief String löschen
     *
     * Mit dieser Funktion wird der String gelöscht und der Speicher freigegeben.
     */
    void clear() noexcept;

    /**\brief Kapazität des Strings abfragen
     *
     * Mit dieser Funktion kann die Kapazität des Strings abgefragt werden. Die Kapazität ist die
     * Anzahl Zeichen, die der String aufnehmen kann, ohne dass der Speicherbereich vergrößert werden muss.
     *
     * \return
     * Liefert die Anzahl Zeichen, die der String aufnehmen kann.
     */
    size_t capacity() const;

    /**\brief Speicher für den String reservieren
     *
     * Mit dieser Funktion kann Speicher für den String vorab reserviert werden. Dies ist insbesondere dann sinnvoll,
     * wenn der String während seiner Lebenszeit häufig verlängert wird.
     *
     * @param size
     * Anzahl Zeichen, für die Speicher reserviert werden soll.
     *
     * @note
     * Enthält der String bereits Zeichen, gehen diese nicht verloren, der existierende Speicherbereich kann aber zwecks Vergrößerung
     * umkopiert werden.
     */
    void reserve(size_t size);

    /**\brief Länge des Strings abfragen
     *
     * Mit dieser Funktion kann die Länge des Strings abgefragt werden. Die Länge ist die Anzahl Zeichen, die der String aktuell enthält.
     *
     * @return
     * Liefert die Anzahl Zeichen, die der String aktuell enthält.
     */
    inline constexpr size_t len() const
    {
        return stringlen;
    }

    /**\brief Länge des Strings abfragen
     *
     * Mit dieser Funktion kann die Länge des Strings abgefragt werden. Die Länge ist die Anzahl Zeichen, die der String aktuell enthält.
     *
     * @return
     * Liefert die Anzahl Zeichen, die der String aktuell enthält.
     */
    inline constexpr size_t length() const
    {
        return stringlen;
    }

    /**\brief Länge des Strings abfragen
     *
     * Mit dieser Funktion kann die Länge des Strings abgefragt werden. Die Länge ist die Anzahl Zeichen, die der String aktuell enthält.
     *
     * @return
     * Liefert die Anzahl Zeichen, die der String aktuell enthält.
     */
    inline constexpr size_t size() const
    {
        return stringlen;
    }

    /**@brief Prüft, ob der String leer ist.
     *
     * Diese Funktion prüft, ob der String leer ist.
     *
     * @returns Ist der String leer, liefert die Funktion \c true zurück, sonst \c false.
     * @see String::notEmpty
     */
    inline constexpr bool isEmpty() const
    {
        return (stringlen == 0);
    }

    /**@brief Prüft, ob der String Zeichen enthält
     *
     * Diese Funktion prüft, ob der String Zeichen enthält.
     *
     * @returns Enthält der String Zeichen, liefert die Funktion \c true zurück, sonst \c false.
     * @see String::isEmpty
     */
    inline constexpr bool notEmpty() const
    {
        return (stringlen != 0);
    }

    /**@brief Prüft, ob der String numerisch ist
     *
     * Diese Funktion prüft, ob der String numerisch ist. Ein String ist numerisch, wenn er nur aus Ziffern besteht.
     * Ein Minuszeichen am Anfang ist erlaubt, ebenso ein Dezimalpunkt oder Komma.
     *
     * @return Ist der String numerisch, wird 1 zurückgegeben. Ist er es nicht oder ist der String
     * leer, wird 0 zurückgegeben.
     */
    bool isNumeric() const;

    /**@brief Prüft, ob der String einen Integer Wert enthält
     *
     * Diese Funktion prüft, ob im String einen integer Wert enthält, also nur die Ziffern
     * 0-9 und optional ein Minus am Anfang enthalten sind
     *
     * @return Ist der String ein Integer, wird true zurückgegeben. Ist er es nicht oder ist der String
     * leer, wird false zurückgegeben.
     */
    bool isInteger() const;

    /**@brief Prüft, ob der String "wahr" ist
     *
     * Diese Funktion überprüft den aktuellen String, ob er "wahr" ist. Dies ist der Fall,
     * wenn eine der folgenden Bedingungen erfüllt ist:
     * - Der String enthält eine Ziffer ungleich 0
     * - Der String enthält das Wort "true" (Gross- oder Kleingeschrieben)
     * - Der String enthält das Wort "wahr" (Gross- oder Kleingeschrieben)
     * - Der String enthält das Wort "yes" (Gross- oder Kleingeschrieben)
     * - Der String enthält das Wort "ja" (Gross- oder Kleingeschrieben)
     *
     * @returns Liefert true zurück, wenn der String "wahr" ist, sonst false. Ein Fehlercode wird nicht gesetzt
     * @see String::isFalse()
     */
    bool isTrue() const;

    /**@brief Prüft, ob der String "unwahr" ist
     *
     * Diese Funktion überprüft den aktuellen String, ob er "unwahr" ist. Dies ist der Fall,
     * wenn eine der folgenden Bedingungen erfüllt ist:
     * - Der String zeigt auf NULL
     * - Die Länge des Strings ist 0
     * - Der String enthält die Ziffer 0
     * - Der String enthält nicht das Wort "true", "wahr", "yes" oder "ja" (Gross-/Kleinschreibung egal)
     * @returns Liefert true (1) zurück, wenn der String "unwahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
     * @see String::isTrue()
     */
    bool isFalse() const;

    /**@brief Vergleicht den String mit einem anderen String
     *
     * Mit dieser Funktion kann der aktuelle String mit einem anderen String verglichen werden.
     *
     * @param str Referenz auf einen anderen String
     * @param size Optionaler Parameter, der die Anzahl zu vergleichender Zeichen angibt. Ist der Wert nicht angegeben, wird der komplette
     * String verglichen. Ist der Wert größer als der angegebene String, wird er ignoriert und der komplette String verglichen.
     * @return Liefert 0 zurück, wenn die Strings gleich sind. Ist der aktuelle String kleiner als \p str, wird ein Wert kleiner 0
     * zurückgegeben. Ist er größer, wird ein Wert größer 0 zurückgegeben.
     */
    int strcmp(const String& str, size_t size = (size_t)-1) const;
    int strCaseCmp(const String& str, size_t size = (size_t)-1) const;
    int strcmp(const char* str, size_t size = (size_t)-1) const;
    int strCaseCmp(const char* str, size_t size = (size_t)-1) const;
    String left(size_t len) const;
    String right(size_t len) const;
    String mid(size_t start, size_t len = (size_t)-1) const;
    String substr(size_t start, size_t len = (size_t)-1) const;

    //! @name String setzen und verändern
    //@{

    /**@brief String anhand eines C-Strings setzen
     *
     * Mit dieser Funktion wird der String anhand eines char * gesetzt. Dabei wird er
     * intern nach Unicode konvertiert.
     *
     * @param str Pointer auf einen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& set(const char* str, size_t size = (size_t)-1);

    /**@brief String anhand eines anderen Strings setzen
     *
     * Mit dieser Funktion wird der String anhand eines anderen Strings gesetzt.
     *
     * @param str Referenz auf einen anderen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    String& set(const String& str, size_t size = (size_t)-1);

    /**@brief String anhand eines ByteArrayPtr setzen
     *
     * Mit dieser Funktion wird der String anhand eines ByteArrayPtr gesetzt. Dabei wird er
     * intern nach Unicode konvertiert.
     *
     * @param str Referenz auf einen ByteArrayPtr
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& set(const ByteArrayPtr& str, size_t size = (size_t)-1);

    /**@brief String anhand eines WideStrings setzen
     *
     * Mit dieser Funktion wird der String anhand eines WideString gesetzt. Dabei wird er
     * intern nach Unicode konvertiert.
     *
     * @param str Referenz auf einen WideString
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& set(const WideString& str, size_t size = (size_t)-1);

    /**@brief String anhand eines STL-Strings setzen
     *
     * Mit dieser Funktion wird der String anhand eines Strings der Standard-Template-Library (STL) gesetzt.
     *
     * @param str Referenz auf einen String der STL
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& set(const std::string& str, size_t size = (size_t)-1);

    /**@brief String anhand eines STL-WideStrings setzen
     *
     * Mit dieser Funktion wird der String anhand eines WideStrings der Standard-Template-Library (STL) gesetzt.
     *
     * @param str Referenz auf einen WideString der STL
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& set(const std::wstring& str, size_t size = (size_t)-1);

    /**@brief String anhand eines wchar_t* setzen
     *
     * Mit dieser Funktion wird der String anhand eines wchar_t * gesetzt. Dabei wird er
     * intern nach Unicode konvertiert.
     *
     * @param str Pointer auf einen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& set(const wchar_t* str, size_t size = (size_t)-1);

    /**@brief Einzelnes ASCII-Zeichen übernehmen
     *
     * Ein einzelnes ASCII-Zeichen \p c wird in den String übernommen.
     *
     * @param c ASCII-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    String& set(char c);

    /**@brief Einzelnes Zeichen ersetzen
     *
     * Mit dieser Funktion wird ein einzelnes Zeichen eines Strings an der Position
     * \p position durch das Zeichen \p c ersetzt.
     *
     * @param position Position innerhalb des Strings (Zählung beginnt bei 0)
     * @param c Unicode-Wert, der gesetzt werden soll
     * @return Referenz auf den String
     * @throw OutOfBoundsException: Wird geworfen, wenn \p position größer ist, als die
     * Länge des Strings
     */
    String& set(size_t position, char c);

    /**@brief Erzeugt einen String anhand eines Formatstrings und beliebiger Parameter
     *
     * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
     * und den optionalen Parametern \p ...
     *
     * @param fmt Der Formatstring
     * @param ... Optionale Parameter
     *
     * @return Referenz auf den String
     *
     * @copydoc sprintf.dox
     */
    String& setf(const char* fmt, ...);

    /**@brief Statische Methode zur erstellung eines Strings anhand eines Formatstrings und beliebiger Parameter
     *
     * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
     * und den optionalen Parametern in \p args.
     *
     * @param fmt Der Formatstring
     * @param args Liste der optionalen Parameter
     *
     * @return Neuer String
     *
     * @copydoc sprintf.dox
     */
    static String format(const char* fmt, ...);

    /**@brief Fügt einen C-String an das Ende des bestehenden an
     *
     * Fügt einen C-String an das Ende des bestehenden an. Der String muss entweder
     * UTF-8 kodiert sein, oder es muss mit der statischen Funktion String::setGlobalEncoding
     * zuvor eine andere Kodierung gesetzt worden sein.
     *
     * @param[in] str Pointer auf einen Wide-Character String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    String& append(const char* str, size_t size = (size_t)-1);

    /**@brief String an das Ende des bestehenden anhängen
     *
     * Mit dieser Funktion wird der String \p str an das Ende des bestehenden Strings angehangen.
     *
     * @param[in] str Referenz auf einen anderen String
     * @param[in] size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     */
    String& append(const String& str, size_t size = (size_t)-1);

    /**@brief String an das Ende des bestehenden anhängen
     *
     * Mit dieser Funktion wird der String \p str an das Ende des bestehenden Strings angehangen.
     *
     * @param[in] str Referenz auf einen anderen String
     * @param[in] size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     */
    String& append(const std::string& str, size_t size = (size_t)-1);

    /**@brief Wide-String an das Ende des bestehenden anhängen
     *
     * Mit dieser Funktion wird der Wide-String \p str an das Ende des bestehenden Strings angehangen.
     *
     * @param[in] str Referenz auf einen anderen Wide-String
     * @param[in] size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     */
    String& append(const std::wstring& str, size_t size = (size_t)-1);

    /**@brief Fügt einen Wide-Character String an das Ende des bestehenden an
     *
     * Fügt einen Wide-Character String an das Ende des bestehenden an
     *
     * @param[in] str Pointer auf einen Wide-Character String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& append(const wchar_t* str, size_t size = (size_t)-1);

    /**@brief Fügt einen Formatierten String an das Ende des bestehenden an
     *
     * Anhand des übergebenen Formatstrings \p fmt und den optionalen Parametern \p ...
     * wird ein neuer String gebildet, der an das Ende des bestehenden angehangen wird
     *
     * \param fmt Der Formatstring
     * \param ... Optionale Parameter
     * @return Referenz auf den String
     *
     * \copydoc sprintf.dox
     */
    String& appendf(const char* fmt, ...);

    /**@brief Einzelnes ASCII-Zeichen anhängen
     *
     * Ein einzelnes ASCII-Zeichen \p c wird in an den String angehangen.
     *
     * @param c ASCII-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    String& append(char c);

    /**@brief Fügt einen C-String am Anfang des bestehenden Strings ein
     *
     * Fügt einen C-String am Anfang des bestehenden Strings ein
     *
     * @param[in] str Pointer auf einen C-String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& prepend(const char* str, size_t size = (size_t)-1);

    /**@brief Fügt einen String am Anfang des bestehenden Strings ein
     *
     * Fügt einen String am Anfang des bestehenden Strings ein
     *
     * @param[in] str Referenz auf einen anderen String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& prepend(const String& str, size_t size = (size_t)-1);

    /**@brief Fügt einen STL-String am Anfang des bestehenden Strings ein
     *
     * Fügt einen STL-String am Anfang des bestehenden Strings ein
     *
     * @param[in] str Referenz auf einen STL-String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& prepend(const std::string& str, size_t size = (size_t)-1);

    /**@brief Fügt einen STL-Wide-String am Anfang des bestehenden Strings ein
     *
     * Fügt einen STL-Wide-String am Anfang des bestehenden Strings ein
     *
     * @param[in] str Referenz auf einen STL-Wide-String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& prepend(const std::wstring& str, size_t size = (size_t)-1);

    /**@brief Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
     *
     * Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
     *
     * @param[in] str Pointer auf einen Wide-Character String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& prepend(const wchar_t* str, size_t size = (size_t)-1);

    /**@brief Fügt einen Formatierten String am Anfang des bestehenden Strings ein
     *
     * Anhand des übergebenen Formatstrings \p fmt und den optionalen Parametern \p ...
     * wird ein neuer String gebildet, der am Anfang des bestehenden eingefügt wird
     *
     * \param fmt Der Formatstring
     * \param ... Optionale Parameter
     * @return Referenz auf den String
     *
     * \copydoc sprintf.dox
     */
    String& prependf(const char* fmt, ...);

    /**@brief Einzelnes ASCII-Zeichen am Anfang des bestehenden Strings einfügen
     *
     * Ein einzelnes ASCII-Zeichen \p c wird am Anfang des Strings eingefügt.
     *
     * @param c ASCII-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    String& prepend(char c);

    /**@brief Erzeugt einen formatierten String
     *
     * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
     * und den optionalen Parametern in \p args.
     *
     * @param[in] fmt Der Formatstring
     * @param[in] args Pointer auf Liste der Parameter. Muss zuvor mit va_start initialisiert worden sein.
     * @return Referenz auf den String
     *
     * @copydoc sprintf.dox
     */
    String& vasprintf(const char* fmt, va_list args);

    String& repeat(size_t num);
    String& repeat(char code, size_t num);
    String& repeat(const String& str, size_t num);
    String repeated(size_t num) const;

    /**@brief Wandelt alle Zeichen des Strings in Kleinbuchstaben um
     *
     * Diese Funktion wandelt alle Zeichen des Strings in Kleinbuchstaben um. Die genaue Funktionsweise hängt davon ab,
     * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
     *
     * @attention Lokalisierungseinstellungen einer Shell werden nicht automatisch übernommen. In der Regel
     * wird standardmäßig die Lokalisierung "C" verwendet, wodurch nur US-ASCII (ASCII 32 bis 127) umgewandelt wird.
     * Man sollte daher bei Programmstart mit "setlocale" die gewünschte Spracheinstellung vornehmen.
     *
     * @example
     * @code
     * #include <locale.h>
     * ...
     * // Lokalisierung explizit setzen
     * setlocale(LC_CTYPE,"de_DE.UTF-8");
     * // oder Lokalisierung von den Systemeinstellungen übernehmen
     * setlocale(LC_CTYPE,"");
     * @endcode
     */
    void lowerCase();

    /**@brief Wandelt alle Zeichen des Strings in Großbuchstaben um
     *
     * Diese Funktion wandelt alle Zeichen des Strings in Großbuchstaben um. Die genaue Funktionsweise hängt davon ab,
     * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
     *
     * @attention Lokalisierungseinstellungen einer Shell werden nicht automatisch übernommen. In der Regel
     * wird standardmäßig die Lokalisierung "C" verwendet, wodurch nur US-ASCII (ASCII 32 bis 127) umgewandelt wird.
     * Man sollte daher bei Programmstart mit "setlocale" die gewünschte Spracheinstellung vornehmen.
     *
     * @example
     * @code
     * #include <locale.h>
     * ...
     * // Lokalisierung explizit setzen
     * setlocale(LC_CTYPE,"de_DE.UTF-8");
     * // oder Lokalisierung von den Systemeinstellungen übernehmen
     * setlocale(LC_CTYPE,"");
     * @endcode
     */
    void upperCase();

    void trim();
    String trimmed() const;
    String toLowerCase() const;
    String toUpperCase() const;
    void trimLeft();
    void trimRight();
    void trim(const String& chars);
    void trimLeft(const String& chars);
    void trimRight(const String& chars);
    void chopRight(size_t num = 1);
    void chop(size_t num = 1);
    void chopLeft(size_t num = 1);
    void chomp();
    void cut(size_t pos);
    void cut(const String& letter);

    void shl(char c, size_t size);
    void shr(char c, size_t size);

    String strchr(char c) const;
    String strrchr(char c) const;
    String strstr(const String& needle) const;
    ssize_t find(const String& needle, ssize_t start = 0) const;
    ssize_t findCase(const String& needle, ssize_t start = 0) const;
    ssize_t instr(const String& needle, size_t start = 0) const;
    ssize_t instrCase(const String& needle, size_t start = 0) const;
    bool has(const String& needle, bool ignoreCase = false) const;

    bool startsWith(const String& prefix, size_t start = 0, size_t end = (size_t)-1) const;
    bool endsWith(const String& suffix, size_t start = 0, size_t end = (size_t)-1) const;
    String join(const ppl7::Array& iterable) const;

    String& replace(const String& search, const String& replacement);

    //@}

    //! @name String ausgeben und auslesen
    //@{
    void print(bool withNewline = false) const noexcept;
    void printnl() const noexcept;
    void hexDump() const;
    char get(ssize_t pos) const;

    /**\brief Pointer auf den internen C-String
     *
     * Diese Funktion liefert einen Pointer im Format "const char*" auf den internen
     * C-String der Klasse zurück. Bei einem leeren String ist sichergestellt, dass ein
     * Pointer auf einen leeren mit 0-Byte terminierten String zurückgegeben wird. Die
     * Funktion gibt also niemals NULL zurück.
     *
     * @return Pointer auf den internen C-String der Klasse
     * @see
     * Die folgenden Funktionen erfüllen den gleichen Zweck:
     * - const char * String::getPtr() const
     * - const char * String::c_str() const
     * - const char * String::toChar() const
     * - String::operator const char *() const
     */
    const char* getPtr() const
    {
        return ptr ? ptr : "";
    }

    /**\brief Pointer auf den internen C-String
     *
     * Diese Funktion liefert einen Pointer im Format "const char*" auf den internen
     * C-String der Klasse zurück. Bei einem leeren String ist sichergestellt, dass ein
     * Pointer auf einen leeren mit 0-Byte terminierten String zurückgegeben wird. Die
     * Funktion gibt also niemals NULL zurück.
     *
     * @return Pointer auf den internen C-String der Klasse
     * @see
     * Die folgenden Funktionen erfüllen den gleichen Zweck:
     * - const char * String::getPtr() const
     * - const char * String::c_str() const
     * - const char * String::toChar() const
     * - String::operator const char *() const
     */
    const char* c_str() const
    {
        return ptr ? ptr : "";
    }

    ByteArray toEncoding(const char* encoding) const;

    String md5() const;

    int toInt() const;
    unsigned int toUnsignedInt() const;
    int64_t toInt64() const;
    uint64_t toUnsignedInt64() const;
    WideString toWideString() const;
    bool toBool() const;
    long toLong() const;
    unsigned long toUnsignedLong() const;
    long long toLongLong() const;
    unsigned long long toUnsignedLongLong() const;
    float toFloat() const;
    double toDouble() const;

    /**\brief Pointer auf den internen C-String
     *
     * Diese Funktion liefert einen Pointer im Format "const char*" auf den internen
     * C-String der Klasse zurück. Bei einem leeren String ist sichergestellt, dass ein
     * Pointer auf einen leeren mit 0-Byte terminierten String zurückgegeben wird. Die
     * Funktion gibt also niemals NULL zurück.
     *
     * @return Pointer auf den internen C-String der Klasse
     * @see
     * Die folgenden Funktionen erfüllen den gleichen Zweck:
     * - const char * String::getPtr() const
     * - const char * String::c_str() const
     * - const char * String::toChar() const
     * - String::operator const char *() const
     */
    const char* toChar() const
    {
        return ptr ? ptr : "";
    }

    //@}

    //! @name Operatoren
    //@{
    operator const char*() const
    {
        return ptr ? ptr : "";
    }
    operator const unsigned char*() const
    {
        return (const unsigned char*)(ptr ? ptr : "");
    }
    operator int() const;
    operator unsigned int() const;
    operator bool() const;
    operator long() const;
    operator unsigned long() const;
    operator long long() const;
    operator unsigned long long() const;
    operator float() const;
    operator double() const;
    operator std::string() const;
    operator std::wstring() const;

    char operator[](ssize_t pos) const;
    char& operator[](ssize_t pos);

    String& operator=(const char* str);
    String& operator=(const wchar_t* str);
    String& operator=(const String& str);
    String& operator=(const WideString& str);
    String& operator=(const std::string& str);
    String& operator=(const std::wstring& str);
    String& operator=(char c);
    String& operator=(String&& other) noexcept;
    String& operator+=(const char* str);
    String& operator+=(const wchar_t* str);
    String& operator+=(const String& str);
    String& operator+=(const std::string& str);
    String& operator+=(const std::wstring& str);
    String& operator+=(char c);
    bool operator<(const String& str) const;
    bool operator<=(const String& str) const;
    bool operator==(const String& str) const;
    bool operator!=(const String& str) const;
    bool operator>=(const String& str) const;
    bool operator>(const String& str) const;

    bool operator<(const char* str) const;
    bool operator<=(const char* str) const;
    bool operator==(const char* str) const;
    bool operator!=(const char* str) const;
    bool operator>=(const char* str) const;
    bool operator>(const char* str) const;

    //@}

    //! @name Iteratoren
    //@{
    typedef char* iterator;
    typedef const char* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;
    //@}

#ifdef PPL_WITH_QT6
    operator QAnyStringView() const
    {
#ifdef PPL_QT_STRING_UTF8
        return QAnyStringView(ptr, stringlen);
#else
        return QAnyStringView(ptr, stringlen);
#endif
    }
#endif

#ifdef WITH_QT
    //! @name Operatoren zur Verwendung der Klasse mit Qt
    //@{
    operator const QString() const
    {
#ifdef PPL_QT_STRING_UTF8
        return QString::fromUtf8(ptr, stringlen);
#else
        return QString::fromLocal8Bit(ptr, stringlen);
#endif
    }

    operator const QVariant() const
    {
#ifdef PPL_QT_STRING_UTF8
        QVariant v = QString::fromUtf8(ptr, stringlen);
#else
        QVariant v = QString::fromLocal8Bit(ptr, stringlen);
#endif
        return v;
    }

    String& operator=(const QString& q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    String& operator=(const QString* q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q->toUtf8();
#else
        QByteArray a = q->toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    String& operator+=(const QString& q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        append((const char*)a);
        return *this;
    }
//@}
#endif
};

String operator+(const String& str1, const String& str2);
String operator+(const char* str1, const String& str2);
String operator+(const String& str1, const char* str2);
String operator+(const wchar_t* str1, const String& str2);
String operator+(const String& str1, const wchar_t* str2);
String operator+(const std::string& str1, const String& str2);
String operator+(const String& str1, const std::string& str2);
String operator+(const std::wstring& str1, const String& str2);
String operator+(const String& str1, const std::wstring& str2);

std::ostream& operator<<(std::ostream& s, const String& str);

///@name Google Test Integration
inline void PrintTo(const String& str, ::std::ostream* os)
{
    *os << "\"" << str.c_str() << "\"";
}

} // namespace ppl7

#endif /* PPL7_TYPES_STRING_H_ */