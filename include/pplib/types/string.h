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
#ifndef PPLIB_TYPES_STRING_H_
#define PPLIB_TYPES_STRING_H_

#include <string>
#include <stdint.h>

#ifdef PPL_WITH_QT6
#include <QAnyStringView>
#ifndef WITH_QT
#define WITH_QT
#endif
#endif

#ifdef WITH_QT
#include <QString>
#include <QVariant>
#include <QByteArray>
#endif

namespace pplib
{

class WideString;
class ByteArrayPtr;
class ByteArray;
class Array;

/** @class String
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
    static constexpr size_t npos = static_cast<size_t>(-1); // Ergebnis von find, wenn nichts gefunden wurde

    //! @name Konstruktoren und Destruktor
    //@{

    /** @brief Default Konstruktor mit leeren String
     *
     * Es wird ein leerer String erstellt.
     */
    String() noexcept;

    /** @brief Konstruktor mit C-String
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

    /** @brief Konstruktor mit C-String und Länge
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

    /** @brief Konstruktor mit Wide-Character-String
     *
     * Ein String wird aus einem Wide-Character-String erstellt. Dabei wird der String
     * in die globale Kodierung konvertiert.
     *
     * @param str Wide-Character-String mit 0-Wert am Ende
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    explicit String(const wchar_t* str, size_t size = (size_t)-1);

    /** @brief Konstruktor mit anderem String
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

    /** @brief Konstruktor mit WideString
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

    /** @brief Konstruktor mit ByteArrayPtr
     *
     * Ein String wird aus einem ByteArrayPtr erstellt.
     *
     * @param str Referenz auf einen ByteArrayPtr
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    explicit String(const ByteArrayPtr& str);

    /** @brief Konstruktor aus Standard-Template String
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

    /** @brief Konstruktor aus Standard-Template Wide-String
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

    /** @brief Move-Konstruktor
     *
     * Ein String wird aus einem anderen String erstellt. Dabei wird der Speicher des anderen Strings übernommen.
     *
     * @param other Rvalue-Referenz auf einen anderen String
     * @exception Keine
     */
    String(String&& other) noexcept;

    /** @brief Destruktor
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
        QByteArray a = q.toUtf8();
        set((const char*)a);
    }
    String(QString* q)
    {
        ptr = NULL;
        stringlen = 0;
        s = 0;
        QByteArray a = q->toUtf8();
        set((const char*)a);
    }
#endif
    //@}

    /** @brief String löschen
     *
     * Mit dieser Funktion wird der String gelöscht und der Speicher freigegeben.
     */
    void clear() noexcept;

    /** @brief Kapazität des Strings abfragen
     *
     * Mit dieser Funktion kann die Kapazität des Strings abgefragt werden. Die Kapazität ist die
     * Anzahl Zeichen, die der String aufnehmen kann, ohne dass der Speicherbereich vergrößert werden muss.
     *
     * \return
     * Liefert die Anzahl Zeichen, die der String aufnehmen kann.
     */
    size_t capacity() const;

    /** @brief Speicher für den String reservieren
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

    /** @brief Länge des Strings abfragen
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

    /** @brief Länge des Strings abfragen
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

    /** @brief Länge des Strings abfragen
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

    /** @brief Prüft, ob der String leer ist.
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

    /** @brief Prüft, ob der String Zeichen enthält
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

    /** @brief Prüft, ob der String numerisch ist
     *
     * Diese Funktion prüft, ob der String numerisch ist. Ein String ist numerisch, wenn er nur aus Ziffern besteht.
     * Ein Minuszeichen am Anfang ist erlaubt, ebenso ein Dezimalpunkt oder Komma.
     *
     * @return Ist der String numerisch, wird 1 zurückgegeben. Ist er es nicht oder ist der String
     * leer, wird 0 zurückgegeben.
     */
    bool isNumeric() const;

    /** @brief Prüft, ob der String einen Integer Wert enthält
     *
     * Diese Funktion prüft, ob im String einen integer Wert enthält, also nur die Ziffern
     * 0-9 und optional ein Minus am Anfang enthalten sind
     *
     * @return Ist der String ein Integer, wird true zurückgegeben. Ist er es nicht oder ist der String
     * leer, wird false zurückgegeben.
     */
    bool isInteger() const;

    /** @brief Prüft, ob der String "wahr" ist
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

    /** @brief Prüft, ob der String "unwahr" ist
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

    /** @brief Vergleicht den String mit einem anderen String
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

    /** @brief Vergleicht den String mit einem anderen String, ohne die Gross-/Kleinschreibung zu berücksichtigen
     *
     * Mit dieser Funktion kann der aktuelle String mit einem anderen String verglichen werden, ohne die Gross-/Kleinschreibung zu
     * berücksichtigen.
     *
     * @param str Referenz auf einen anderen String
     * @param size Optionaler Parameter, der die Anzahl zu vergleichender Zeichen angibt. Ist der Wert nicht angegeben, wird der komplette
     * String verglichen. Ist der Wert größer als der angegebene String, wird er ignoriert und der komplette String verglichen.
     * @return Liefert 0 zurück, wenn die Strings gleich sind. Ist der aktuelle String kleiner als \p str, wird ein Wert kleiner 0
     * zurückgegeben. Ist er größer, wird ein Wert größer 0 zurückgegeben.
     */
    int strCaseCmp(const String& str, size_t size = (size_t)-1) const;

    /** @brief Vergleicht den String mit einem C-String
     *
     * Mit dieser Funktion kann der aktuelle String mit einem C-String verglichen werden.
     *
     * @param str Pointer auf einen C-String
     * @param size Optionaler Parameter, der die Anzahl zu vergleichender Zeichen angibt. Ist der Wert nicht angegeben, wird der komplette
     * String verglichen. Ist der Wert größer als der angegebene String, wird er ignoriert und der komplette String verglichen.
     * @return Liefert 0 zurück, wenn die Strings gleich sind. Ist der aktuelle String kleiner als \p str, wird ein Wert kleiner 0
     * zurückgegeben. Ist er größer, wird ein Wert größer 0 zurückgegeben.
     */
    int strcmp(const char* str, size_t size = (size_t)-1) const;

    /** @brief Vergleicht den String mit einem C-String, ohne die Gross-/Kleinschreibung zu berücksichtigen
     *
     * Mit dieser Funktion kann der aktuelle String mit einem C-String verglichen werden, ohne die Gross-/Kleinschreibung zu
     * berücksichtigen.
     *
     * @param str Pointer auf einen C-String
     * @param size Optionaler Parameter, der die Anzahl zu vergleichender Zeichen angibt. Ist der Wert nicht angegeben, wird der komplette
     * String verglichen. Ist der Wert größer als der angegebene String, wird er ignoriert und der komplette String verglichen.
     * @return Liefert 0 zurück, wenn die Strings gleich sind. Ist der aktuelle String kleiner als \p str, wird ein Wert kleiner 0
     * zurückgegeben. Ist er größer, wird ein Wert größer 0 zurückgegeben.
     */
    int strCaseCmp(const char* str, size_t size = (size_t)-1) const;

    /** @brief Linken Teilstring zurückgeben
     *
     * Gibt die ersten \p len Zeichen des Strings als neuen zurück.
     *
     * @param len Länge des Teilstrings
     * @return Neuer String
     */
    String left(size_t len) const;

    /** @brief Rechten Teilstring zurückgeben
     *
     * \desc
     * Gibt die letzten \p len Zeichen des Strings als neuen zurück.
     *
     * @param len Länge des Teilstrings
     * @return Neuer String
     */
    String right(size_t len) const;

    /** @brief Teilstring zurückgeben
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
    String mid(size_t start, size_t len = (size_t)-1) const;

    /** @brief Teilstring zurückgeben
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
    String substr(size_t start, size_t len = (size_t)-1) const;

    //! @name String setzen und verändern
    //@{

    /** @brief String anhand eines C-Strings setzen
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

    /** @brief String anhand eines anderen Strings setzen
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

    /** @brief String anhand eines ByteArrayPtr setzen
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

    /** @brief String anhand eines WideStrings setzen
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

    /** @brief String anhand eines STL-Strings setzen
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

    /** @brief String anhand eines STL-WideStrings setzen
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

    /** @brief String anhand eines wchar_t* setzen
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

    /** @brief Einzelnes ASCII-Zeichen übernehmen
     *
     * Ein einzelnes ASCII-Zeichen \p c wird in den String übernommen.
     *
     * @param c ASCII-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    String& set(char c);

    /** @brief Einzelnes Zeichen ersetzen
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

    /** @brief Erzeugt einen String anhand eines Formatstrings und beliebiger Parameter
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

    /** @brief Statische Methode zur erstellung eines Strings anhand eines Formatstrings und beliebiger Parameter
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

    /** @brief Fügt einen C-String an das Ende des bestehenden an
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

    /** @brief String an das Ende des bestehenden anhängen
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

    /** @brief Wide-String an das Ende des bestehenden anhängen
     *
     * Mit dieser Funktion wird der Wide-String \p str an das Ende des bestehenden Strings angehangen.
     *
     * @param[in] str Referenz auf einen anderen Wide-String
     * @param[in] size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
     * der angegebene String, wird er ignoriert und der komplette String importiert.
     * @return Referenz auf den String
     */
    String& append(const WideString& str, size_t size = (size_t)-1);

    /** @brief String an das Ende des bestehenden anhängen
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

    /** @brief Wide-String an das Ende des bestehenden anhängen
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

    /** @brief Fügt einen Wide-Character String an das Ende des bestehenden an
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

    /** @brief Fügt einen Formatierten String an das Ende des bestehenden an
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

    /** @brief Einzelnes ASCII-Zeichen anhängen
     *
     * Ein einzelnes ASCII-Zeichen \p c wird in an den String angehangen.
     *
     * @param c ASCII-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    String& append(char c);

    /** @brief Fügt einen C-String am Anfang des bestehenden Strings ein
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

    /** @brief Fügt einen String am Anfang des bestehenden Strings ein
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

    /** @brief Fügt einen WideString am Anfang des bestehenden Strings ein
     *
     * Fügt einen WideString am Anfang des bestehenden Strings ein
     *
     * @param[in] str Referenz auf einen anderen WideString
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    String& prepend(const WideString& str, size_t size = (size_t)-1);

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

    /**
     * @brief String wiederholen
     *
     * Mit dieser Funktion wird der Inhalt des Strings mehrfach wiederholt.
     *
     * @param num Anzahl Wiederholungen. Falls \p num 0 ist, ist der String anschließend leer.
     * @return Referenz auf den verlängerten String.
     * @exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
     */
    String& repeat(size_t num);

    /**
     * @brief Füllt den String mit einem Zeichen
     *
     * Der String wird mit einem gewünschten Zeichen gefüllt
     * @param unicode Der Unicode des Zeichens, mit dem der String gefüllt werden soll
     * @param num Die Länge des gewünschten Strings
     * @return Referenz auf den String
     * @exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
     */
    String& repeat(char code, size_t num);

    /**
     * @brief Füllt den String mit einem Zeichen
     *
     * Der String wird mit einem gewünschten Zeichen gefüllt
     * @param unicode Der Unicode des Zeichens, mit dem der String gefüllt werden soll
     * @param num Die Länge des gewünschten Strings
     * @return Referenz auf den neuen String
     * @exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
     */
    String& repeat(const String& str, size_t num);

    /**@brief String multiplizieren
     *
     * Der aktuelle String wird \p count mal hintereinander wiederholt und
     * als neuer String zurückgegeben.
     *
     * @param[in] count Anzahl wiederholungen
     * @return Neuer String
     * @exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
     */
    String repeated(size_t num) const;

    /**
     * @brief Wandelt alle Zeichen des Strings in Kleinbuchstaben um
     *
     * Diese Funktion wandelt alle Zeichen des Strings in Kleinbuchstaben um. Die genaue Funktionsweise hängt davon ab,
     * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
     *
     * @return Referenz auf den String
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
    String& lowerCase();

    /**
     * @brief Wandelt alle Zeichen des Strings in Großbuchstaben um
     *
     * Diese Funktion wandelt alle Zeichen des Strings in Großbuchstaben um. Die genaue Funktionsweise hängt davon ab,
     * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
     *
     * @return Referenz auf den String
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
    String& upperCase();

    /** @brief Schneidet Leerzeichen, Tabs, Returns und Linefeeds am Anfang und Ende des Strings ab
     *
     * Leerzeichen, Tabs, Returns und Linefeeds werden am Anfang und Ende des Strings abgeschnitten
     * und der String wird entsprechend verkürzt.
     *
     * @return Referenz auf den String
     */
    String& trim();

    /**
     * @brief Schneidet Leerzeichen, Tabs, Returns und Linefeeds am Anfang und Ende des Strings ab
     *
     * Es wird eine Kopie des Strings angelegt und bei dieser alle Leerzeichen, Tabs, Returns und
     * Linefeeds am Anfang und Ende des Strings abgeschnitten. Das Ergebnis wird als Returnwert
     * zurückgegeben.
     * @return Neuer, gekürzter String
     */
    String trimmed() const;

    /** @brief Gibt eine Kopie des Strings zurück, bei der alle Zeichen in Kleinbuchstaben umgewandelt wurden
     *
     * @return Neuer String mit allen Zeichen in Kleinbuchstaben
     */
    String toLowerCase() const;

    /** @brief Gibt eine Kopie des Strings zurück, bei der alle Zeichen in Großbuchstaben umgewandelt wurden
     *
     * @return Neuer String mit allen Zeichen in Großbuchstaben
     */
    String toUpperCase() const;

    /** @brief Schneidet Leerzeichen, Tabs, Returns und Linefeeds am Anfang des Strings ab
     *
     * Leerzeichen, Tabs, Returns und Linefeeds werden am Anfang des Strings abgeschnitten
     * und der String wird entsprechend verkürzt.
     *
     * @return Referenz auf den String
     */
    String& trimLeft();

    /** @brief Schneidet Leerzeichen, Tabs, Returns und Linefeeds am Ende des Strings ab
     *
     * Leerzeichen, Tabs, Returns und Linefeeds werden am Ende des Strings abgeschnitten
     * und der String wird entsprechend verkürzt.
     *
     * @return Referenz auf den String
     */
    String& trimRight();

    /** @brief Schneidet bestimmte Zeichen am Anfang und Ende des Strings ab
     *
     * Die Funktion schneidet alle Zeichen, die in \p chars enthalten sind, am Anfang und Ende des Strings ab.
     *
     * @param chars String mit den Zeichen, die abgeschnitten werden sollen
     * @return Referenz auf den String
     */
    String& trim(const String& chars);

    /** @brief Schneidet bestimmte Zeichen am Anfang des Strings ab
     *
     * Die Funktion schneidet alle Zeichen, die in \p chars enthalten sind, am Anfang des Strings ab.
     *
     * @param chars String mit den Zeichen, die abgeschnitten werden sollen
     * @return Referenz auf den String
     */
    String& trimLeft(const String& chars);

    /** @brief Schneidet bestimmte Zeichen am Ende des Strings ab
     *
     * Die Funktion schneidet alle Zeichen, die in \p chars enthalten sind, am Ende des Strings ab.
     *
     * @param chars String mit den Zeichen, die abgeschnitten werden sollen
     * @return Referenz auf den String
     */
    String& trimRight(const String& chars);

    /** @brief Schneidet Zeichen am Ende des Strings ab
     *
     * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
     * größer als der String ist, bleibt ein leerer String zurück.
     *
     * @param num Anzahl Zeichen, die abgeschnitten werden sollen
     * @return Referenz auf den String
     */
    String& chopRight(size_t num = 1);

    /** @brief Schneidet Zeichen am Ende des Strings ab
     *
     * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
     * größer als der String ist, bleibt ein leerer String zurück.
     *
     * @param num Anzahl Zeichen, die abgeschnitten werden sollen
     * @return Referenz auf den String
     *
     * @see
     * Die Funktion ist identisch zu String::chopRight
     */
    inline String& chop(size_t num = 1)
    {
        return chopRight(num);
    }

    /** @brief Schneidet Zeichen am Anfang des Strings ab
     *
     * Diese Funktion schneidet \p num Zeichen vom Anfang des Strings ab. Falls \p num
     * größer als der String ist, bleibt ein leerer String zurück.
     *
     * @param num Anzahl Zeichen, die abgeschnitten werden sollen
     *
     * @return Referenz auf den String
     */
    String& chopLeft(size_t num = 1);

    /** @brief Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
     *
     * @return Referenz auf den String
     */
    String& chomp();

    /** @brief Schneidet den String an einer bestimmten Stelle ab
     *
     * Der String wird an einer bestimmten Stelle einfach abgeschnitten
     * @param pos Die Position, an der der String abgeschnitten wird. Bei Angabe von 0 ist der String anschließend
     * komplett leer. Ist \c pos größer als die Länge des Strings, passiert nichts.
     *
     * @return Referenz auf den String
     */
    String& cut(size_t pos);

    /** @brief Schneidet den String beim ersten Auftauchen eines Zeichens/Strings ab
     *
     * Der String wird beim ersten Auftauchen eines Zeichens oder eines Strings abgeschnitten.
     * @param[in] letter Buchstabe oder Buchstabenkombination, an der der String abgeschnitten werden
     * soll. Zeigt der Pointer auf NULL oder ist der String leer, passiert nichts.
     *
     * @return Referenz auf den String
     */
    String& cut(const String& letter);

    /** @brief Schiebt den String nach links
     *
     * Der String wird um die mit \c size angegebenen Zeichen nach links verschoben und rechts mit dem durch \c c angegebenen
     * Zeichen aufgefüllt.
     * @param c Das Zeichen, mit dem der String auf der rechten Seite aufgefüllt werden soll. Wird der Wert 0 verwendet, findet keine
     * Auffüllung statt, d.h. der String verkürzt sich einfach.
     * @param size Die Anzahl Zeichen, um die der String nach links verschoben werden soll. Ist \c size größer als die Länge
     * des Strings, wird der String komplett geleert und ist anschließend so groß wie size, sofern c>0 war.
     *
     * @return Referenz auf den String
     */
    String& shl(char c, size_t size);

    /** @brief Schiebt den String nach rechts
     *
     * Der String wird um die mit \c size angegebenen Zeichen nach rechts verschoben und links mit dem durch \c c angegebenen
     * Zeichen aufgefüllt.
     * @param c Das Zeichen, mit dem der String auf der linken Seite aufgefüllt werden soll. Wird der Wert 0 verwendet, findet keine
     * Auffüllung statt, d.h. der String verkürzt sich einfach.
     * @param size Die Anzahl Zeichen, um die der String nach rechts verschoben werden soll. Ist \c size größer als die Länge
     * des Strings, wird der String komplett geleert und ist anschließend so groß wie size, sofern c>0 war.
     *
     * @return Referenz auf den String
     */
    String& shr(char c, size_t size);

    /** @brief Findet das erste Vorkommen eines Zeichens im String und gibt den Teilstring ab diesem Zeichen zurück.
     *
     * @param c Das zu suchende Zeichen.
     * @return Ein String, der das erste Vorkommen des Zeichens und alles danach enthält.
     * Wenn das Zeichen nicht gefunden wird, wird ein leerer String zurückgegeben.
     */
    String strchr(char c) const;

    /** @brief Findet das letzte Vorkommen eines Zeichens im String und gibt den Teilstring ab diesem Zeichen zurück.
     *
     * @param c Das zu suchende Zeichen.
     * @return Ein String, der das letzte Vorkommen des Zeichens und alles danach enthält.
     * Wenn das Zeichen nicht gefunden wird, wird ein leerer String zurückgegeben.
     */
    String strrchr(char c) const;

    /** @brief Teilstring finden
     *
     * Diese Funktion findet die erste Position der Zeichenfolge \p needle
     * innerhalb des Strings. Abschließende `\0'-Zeichen werden nicht
     * miteinander verglichen.
     *
     * @param needle
     * @return Die Funktion gibt einen String zurück, der mit der gefundenen
     * Zeichenkette beginnt und den Rest des Strings bis zum Ende enthält.
     * Wurde die Zeichenkette nicht gefunden, wird ein leerer String
     * zurückgegeben.
     * \note
     * Ein Sonderfall besteht, wenn \p needle leer ist. In diesem Fall wird
     * der komplette String zurückgegeben.
     */
    String strstr(const String& needle) const;

    /*! @brief Sucht nach einem String
     *
     * Find sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
     *
     * \param[in] needle Gesuchter Teilstring
     * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
     * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
     * Ende des Strings gesucht.
     *
     * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder String::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
     * Funktion immer String::npos zurück.
     */
    size_t find(const String& needle, ssize_t start = 0) const;

    /*! @brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
     *
     * Diese Funktion sucht innerhalb des aktuellen String nach \p needle ab der
     * gewünschten Position \a start. Gross-/Kleinschreibung wird dabei ignoriert.
     *
     * \param[in] needle Gesuchter Teilstring
     * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
     * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
     * Ende des Strings gesucht.
     *
     * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder String::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
     * Funktion immer 0 zurück.
     */
    size_t findCase(const String& needle, ssize_t start = 0) const;

    /*! @brief Sucht nach einem String
     *
     * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
     *
     * \param[in] needle Gesuchter Teilstring
     * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
     * oder nicht angegeben, wird der String vom Anfang an gesucht. Ist der Wert jedoch negativ, wird rückwärts vom Ende des Strings
     * gesucht.
     *
     * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder String::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
     * Funktion immer String::npos zurück.
     */
    size_t instr(const String& needle, ssize_t start = 0) const
    {
        return find(needle, start);
    }

    /*! @brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
     *
     * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
     * Gross-/Kleinschreibung wird dabei ignoriert.
     *
     * \param[in] needle Gesuchter Teilstring
     * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
     * oder nicht angegeben, wird der String vom Anfang an gesucht. Ist der Wert jedoch negativ, wird rückwärts vom Ende des Strings
     *
     * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder String::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
     * Funktion immer String::npos zurück.
     */
    size_t instrCase(const String& needle, ssize_t start = 0) const
    {
        return findCase(needle, start);
    }

    /** @brief Prüft, ob der String einen bestimmten Teilstring enthält
     *
     * Diese Funktion prüft, ob der String den angegebenen Teilstring enthält.
     *
     * @param needle Teilstring, der überprüft werden soll
     * @param ignoreCase Optionaler Parameter, der angibt, ob die Groß-/Kleinschreibung ignoriert werden soll (default=false)
     *
     * @return Liefert true zurück, wenn der String den Teilstring enthält, andernfalls false.
     */
    bool has(const String& needle, bool ignoreCase = false) const;

    /** @brief Prüft, ob der String einen bestimmten Teilstring enthält
     *
     * Diese Funktion prüft, ob der String den angegebenen Teilstring enthält.
     *
     * @param needle Teilstring, der überprüft werden soll
     * @param ignoreCase Optionaler Parameter, der angibt, ob die Groß-/Kleinschreibung ignoriert werden soll (default=false)
     *
     * @return Liefert true zurück, wenn der String den Teilstring enthält, andernfalls false.
     */
    inline bool contains(const String& needle, bool ignoreCase = false) const
    {
        return has(needle, ignoreCase);
    }

    /** @brief Prüft, ob der String mit einem bestimmten Präfix beginnt
     *
     * Diese Funktion prüft, ob der String mit dem angegebenen Präfix beginnt.
     *
     * @param prefix Präfix, das überprüft werden soll
     * @param start Startposition innerhalb des Strings, ab der die Prüfung erfolgen soll
     * @param end Endposition innerhalb des Strings, bis zu der die Prüfung erfolgen soll
     *
     * @return Liefert true zurück, wenn der String mit dem Präfix beginnt, andernfalls false.
     */
    bool startsWith(const String& prefix, size_t start = 0, size_t end = (size_t)-1) const;

    /** @brief Prüft, ob der String mit einem bestimmten Suffix endet
     *
     * Diese Funktion prüft, ob der String mit dem angegebenen Suffix endet.
     *
     * @param suffix Suffix, das überprüft werden soll
     * @param start Startposition innerhalb des Strings, ab der die Prüfung erfolgen soll
     * @param end Endposition innerhalb des Strings, bis zu der die Prüfung erfolgen soll
     *
     * @return Liefert true zurück, wenn der String mit dem Suffix endet, andernfalls false.
     */
    bool endsWith(const String& suffix, size_t start = 0, size_t end = (size_t)-1) const;

    /** @brief Inhalt eines Arrays zu einem String zusammenfügen
     *
     * Mit dieser Funktion wird der Inhalt eines Arrays zu einem String zusammengefügt.
     * Dabei werden die einzelnen Elemente des Arrays durch den aktuellen String getrennt.
     *
     * @param iterable Referenz auf ein Array, dessen Inhalt zusammengefügt werden soll
     * @return Neuer String, der den zusammengefügten Inhalt des Arrays enthält
     */
    String join(const pplib::Array& iterable) const;

    /** @brief Teilstring durch einen anderen String ersetzen
     *
     * Mit dieser Funktion wird der Teilstring \p search durch den String \p replacement ersetzt.
     *
     * @param search Der zu ersetzende Teilstring
     * @param replacement Der String, der anstelle des Teilstrings eingefügt werden soll
     * @return Referenz auf den String
     */
    String& replace(const String& search, const String& replacement);

    //@}

    //! @name String ausgeben und auslesen
    //@{

    /** @brief String auf STDOUT ausgeben
     *
     * Diese Funktion gibt den aktuellen String auf STDOUT aus.
     *
     * @param withNewline Ein optionaler Parameter, der angibt, ob nach der Ausgabe ein Zeilenumbruch
     * angehangen werden soll (default=false)
     */
    void print(bool withNewline = false) const noexcept;

    /** @brief String auf STDOUT mit abschließendem Zeilenumbruch ausgeben
     *
     * Diese Funktion gibt den aktuellen String mit abschließendem Zeilenumbruch auf STDOUT aus.
     */
    void printnl() const noexcept;

    /** @brief Hexdump des Strings ausgeben
     *
     * Mit dieser zu Debug-Zwecken gedachten Funktion wird der Inhalt des
     * Strings als HexDump auf der Konsole ausgegeben.
     */
    void hexDump() const;

    /** @brief Einzelnes Zeichen auslesen
     *
     * Mit dieser Funktion kann Bytewert eines einzelnen Zeichens an der Position
     * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
     * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
     * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
     * dem letzten Zeichen des Strings entspricht.
     *
     * @param pos Position des Zeichens innerhalb des Strings
     * @return Bytewert des Zeichens
     * @exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
     * ausserhalb des Strings liegt oder der String leer ist.
     */
    char get(ssize_t pos) const;

    /** @brief Pointer auf den internen C-String
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
        return stringlen > 0 ? ptr : "";
    }

    /** @brief Pointer auf den internen C-String
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
        return stringlen > 0 ? ptr : "";
    }

    /** @brief Konvertiert den String in einen Integer-Wert
     * @return Integer-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    int toInt() const;

    /** @brief Konvertiert den String in einen unsigned Integer-Wert
     * @return unsigned Integer-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    unsigned int toUnsignedInt() const;

    /** @brief Konvertiert den String in einen 64-Bit Integer-Wert
     * @return 64-Bit Integer-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    int64_t toInt64() const;

    /** @brief Konvertiert den String in einen unsigned 64-Bit Integer-Wert
     * @return unsigned 64-Bit Integer-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    uint64_t toUnsignedInt64() const;

    /** @brief Konvertiert den String in einen WideString
     * @return WideString, der dem Inhalt des Strings entspricht. Bei einem leeren String wird ein leerer WideString zurückgegeben.
     */
    WideString toWideString() const;

    /** @brief Konvertiert den String in einen Boolean-Wert
     * @return Boolean-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird false zurückgegeben.
     */
    bool toBool() const;

    /** @brief Konvertiert den String in einen long-Wert
     * @return long-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    long toLong() const;

    /** @brief Konvertiert den String in einen unsigned long-Wert
     * @return unsigned long-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    unsigned long toUnsignedLong() const;

    /** @brief Konvertiert den String in einen long long-Wert
     * @return long long-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    long long toLongLong() const;

    /** @brief Konvertiert den String in einen unsigned long long-Wert
     * @return unsigned long long-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    unsigned long long toUnsignedLongLong() const;

    /** @brief Konvertiert den String in einen float-Wert
     * @return float-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
    float toFloat() const;

    /** @brief Konvertiert den String in einen double-Wert
     * @return double-Wert, der dem Inhalt des Strings entspricht. Bei einem leeren String wird 0 zurückgegeben.
     */
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
        return stringlen > 0 ? ptr : "";
    }

    //@}

    //! @name Operatoren
    //@{
    operator const char*() const
    {
        return stringlen > 0 ? ptr : "";
    }
    operator const unsigned char*() const
    {
        return (const unsigned char*)(stringlen > 0 ? ptr : "");
    }
    operator int() const;
    operator unsigned int() const;
    operator bool() const;
    operator long() const;
    inline operator int64_t() const
    {
        return toInt64();
    }
    operator uint64_t() const
    {
        return toUnsignedInt64();
    }

    operator unsigned long() const;
    operator float() const;
    operator double() const;
    operator std::string() const;
    operator std::wstring() const;

    /** @brief Einzelnes Zeichen auslesen
     *
     * Mit diesem Operator kann der Bytewert eines einzelnen Zeichens an der Position
     * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
     * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
     * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
     * dem letzten Zeichen des Strings entspricht.
     *
     * @param pos Position des Zeichens innerhalb des Strings
     * @return Bytewert des Zeichens
     * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
     * ausserhalb des Strings liegt oder der String leer ist.
     */
    char operator[](ssize_t pos) const;

    /** @brief Einzelnes Zeichen referenzieren
     *
     * Mit diesem Operator kann eine Referenz auf ein einzelnes Zeichen an der Position
     * \p pos erhalten werden. Dies ermöglicht Lese- und Schreibzugriff.
     * Enthält \p pos einen positiven Wert, wird die Position des
     * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
     * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
     * dem letzten Zeichen des Strings entspricht.
     *
     * @param pos Position des Zeichens innerhalb des Strings
     * @return Referenz auf das Zeichen
     * \exception OutOfBoundsException Wird geworfen, wenn die angegebene Position \p pos
     * ausserhalb des Strings liegt oder der String leer ist.
     */
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
    String& operator+=(const WideString& str);
    String& operator+=(const std::string& str);
    String& operator+=(const std::wstring& str);
    String& operator+=(char c);

    inline bool operator<(const String& str) const
    {
        return strcmp(str) < 0;
    }
    inline bool operator<=(const String& str) const
    {
        return strcmp(str) <= 0;
    }

    inline bool operator==(const String& str) const
    {
        return strcmp(str) == 0;
    }

    inline bool operator!=(const String& str) const
    {
        return strcmp(str) != 0;
    }

    inline bool operator>=(const String& str) const
    {
        return strcmp(str) >= 0;
    }

    inline bool operator>(const String& str) const
    {
        return strcmp(str) > 0;
    }

    inline bool operator<(const char* str) const
    {
        return strcmp(str) < 0;
    }

    inline bool operator<=(const char* str) const
    {
        return strcmp(str) <= 0;
    }

    inline bool operator==(const char* str) const
    {
        return strcmp(str) == 0;
    }

    inline bool operator!=(const char* str) const
    {
        return strcmp(str) != 0;
    }

    inline bool operator>=(const char* str) const
    {
        return strcmp(str) >= 0;
    }

    inline bool operator>(const char* str) const
    {
        return strcmp(str) > 0;
    }

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
        return QAnyStringView(ptr, stringlen);
    }
#endif

#ifdef WITH_QT
    //! @name Operatoren zur Verwendung der Klasse mit Qt
    //@{
    operator const QString() const
    {
        return QString::fromUtf8(ptr, stringlen);
    }

    operator const QVariant() const
    {
        QVariant v = QString::fromUtf8(ptr, stringlen);
        return v;
    }

    String& operator=(const QString& q)
    {
        QByteArray a = q.toUtf8();
        set((const char*)a);
        return *this;
    }
    String& operator=(const QString* q)
    {
        QByteArray a = q->toUtf8();
        set((const char*)a);
        return *this;
    }
    String& operator+=(const QString& q)
    {
        QByteArray a = q.toUtf8();
        append((const char*)a);
        return *this;
    }
//@}
#endif
};

String operator+(const String& str1, const String& str2);
String operator+(const String& str1, const WideString& str2);
String operator+(const String& str1, const char* str2);
String operator+(const String& str1, const wchar_t* str2);
String operator+(const String& str1, const std::string& str2);
String operator+(const String& str1, const std::wstring& str2);
String operator+(const String& str1, const char c);

String operator+(const WideString& str1, const String& str2);
String operator+(const char* str1, const String& str2);
String operator+(const wchar_t* str1, const String& str2);
String operator+(const std::string& str1, const String& str2);
String operator+(const std::wstring& str1, const String& str2);
String operator+(const char c, const String& str2);

std::ostream& operator<<(std::ostream& s, const String& str);

///@name Google Test Integration
inline void PrintTo(const String& str, ::std::ostream* os)
{
    *os << "\"" << str.c_str() << "\"";
}

/** @brief Erstellt einen String aus einem Formatstring und den angegebenen Argumenten
 *
 * Diese Funktion erstellt einen String, der dem angegebenen Formatstring entspricht, wobei die Platzhalter im Formatstring
 * durch die angegebenen Argumente ersetzt werden. Die Funktion verhält sich ähnlich wie printf, jedoch wird der resultierende
 * String als String-Objekt zurückgegeben.
 *
 * @param fmt Formatstring, der die Platzhalter enthält
 * @param ... Argumente, die in den Formatstring eingesetzt werden sollen
 * @return Neuer String, der dem formatierten Ergebnis entspricht
 */
String ToString(const char* fmt, ...);

} // namespace pplib

#endif /* PPLIB_TYPES_STRING_H_ */