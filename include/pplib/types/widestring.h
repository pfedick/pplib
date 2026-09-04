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
#ifndef PPLIB_TYPES_WIDESTRING_H_
#define PPLIB_TYPES_WIDESTRING_H_

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

class String;
class ByteArrayPtr;
class ByteArray;
class Array;

/** @class WideString
 * @ingroup PPLGroupDataTypes
 * @ingroup PPLGroupStrings
 * @brief WideString-Klasse
 *
 * Diese Klasse repräsentiert einen Wide-String. Dieser wird unter Unix intern als Unicode in einem
 * 32-Bit-Wide-Character-Format gespeichert, unter Windows als UTF-16 in einem 16-Bit-Wide-Character-Format
 * (wchar_t).
 *
 * Aufgrund der unterschiedlichen internen Darstellung von Wide-Strings auf verschiedenen Plattformen
 * (32-Bit unter Unix, 16-Bit unter Windows) kann es zu plattformspezifischen Unterschieden bei der
 * Verarbeitung und Konvertierung kommen. Insbesondere beziehen sich alle Längenangaben und Indizes
 * unter Unix tatsächlich auf die Zeichen im String (Unicode), gemessen in 32-Bit-Wide-Characters,
 * unter Windows jedoch auf die 16-Bit-Wide-Characters (UTF-16).
 *
 * Der String kann beliebige Zeichen enthalten, auch 0-Bytes. Allerdings ist nicht garantiert,
 * dass alle Funktionen diese auch korrekt verarbeiten. Insbesondere bei der Ausgabe nach stdout
 * oder bei der Konvertierung in einen String wird beim ersten 0-Byte abgeschnitten.
 *
 * @note Bei der Konvertierung zwischen String und WideString wird die von der Anwendung konfigurierte
 * Zeichenkodierung verwendet (setlocale).
 * @note Diese Klasse ist nicht threadsicher.
 *
 * @see String
 */
class WideString
{
private:
    wchar_t* ptr;     // Pointer auf den Speicherbereich, in dem der String gespeichert ist
    size_t s;         // Anzahl Zeichen, für die Speicher reserviert wurde, exkl. 0-Byte am Ende
    size_t stringlen; // Länge des Strings, exkl. 0-Byte am Ende

public:
    static constexpr ssize_t npos = static_cast<ssize_t>(-1); // Ergebnis von find, wenn nichts gefunden wurde

    //! @name Konstruktoren und Destruktor
    //@{

    /** @brief Konstruktor für leeren String
     *
     * Es wird ein leerer String erstellt.
     */
    WideString() noexcept;

    /** @brief Konstruktor aus Wide-Character-String
     *
     * Ein String wird aus einem Wide-Character-String erstellt.
     * @param str Wide-Character-String, der mit einem 0-Wert Endet
     * @exception OutOfMemoryException
     */
    WideString(const wchar_t* str);

    /** @brief Konstruktor aus Wide-Character-String mit bestimmer Länge
     *
     * Ein String wird aus dem Wide-Character-String \p str erstellt, von dem maximal
     * @p size Zeichen übernommen werden.
     *
     * @param str Wide-Character-String, der mit einem 0-Wert Endet
     * @param size Maximale Anzahl Zeichen, die übernommen werden sollen
     * @exception OutOfMemoryException
     */
    WideString(const wchar_t* str, size_t size);
    explicit WideString(const char* str, size_t size = (size_t)-1);
    WideString(const WideString& str);

    /** @brief Konstruktor aus anderem String (Copy-Konstruktor)
     *
     * Ein String wird aus einem anderen String erstellt.
     * @param str Referenz auf einen anderen String
     * @exception OutOfMemoryException
     */
    explicit WideString(const String& str);

    /** @brief Konstruktor aus Standard-Template String
     *
     * Ein String wird aus einem String der Standard-Template-Library (STL) erstellt.
     * @param str Referenz auf String der STL
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     */
    explicit WideString(const std::string& str);

    /** @brief Konstruktor aus Standard-Template Wide-String
     *
     * Ein String wird aus einem Wide-String der Standard-Template-Library (STL) erstellt.
     * @param str Referenz auf Wide-String der STL
     * @exception OutOfMemoryException
     */
    WideString(const std::wstring& str);

    /** @brief Konstruktor mit ByteArrayPtr
     *
     * Ein String wird aus einem ByteArrayPtr erstellt.
     *
     * Es wird davon ausgegangen,das ByteArray als Array von wchar_t interpretiert wird. Falls dies nicht der Fall ist, muss
     * der Aufrufer gegebenenfalls zunächst Konvertieren, zum Beispiel mit Iconv.
     *
     * @param str Referenz auf einen ByteArrayPtr
     */
    explicit WideString(const ByteArrayPtr& str);

    /** @brief Konstruktor aus anderem String (Copy-Konstruktor)
     *
     * Ein String wird aus einem anderen String erstellt.
     * @param str Referenz auf einen anderen String
     * @exception OutOfMemoryException
     */
    WideString(WideString&& other) noexcept;

    /** @brief Destruktor
     *
     * Der Destructor gibt den durch den String belegten Speicher wieder frei.
     */
    ~WideString() noexcept;
#ifdef WITH_QT
    WideString(const QString& q)
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
    WideString(QString* q)
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

    //! @name Statische Funktionen
    //@{
    static WideString format(const char* fmt, ...);
    //@}

    /** @brief String leeren
     *
     * Mit dieser Funktion wird der String geleert und der bisher allokierte Speicher wieder
     * freigegeben.
     */
    void clear() noexcept;

    /** @brief Anzahl Zeichen, die in den bereits allokierten Speicher passen
     *
     * Diese Funktion liefert die Anzahl Zeichen zurück, die in den derzeitig allokierten
     * Puffer passen, ohne dass neuer Speicher allokiert werden muss.
     * @return Anzahl Zeichen
     */
    size_t capacity() const;

    /** @brief Reserviert Speicher für den String
     *
     * Mit dieser Funktion kann vor Verwendung des Strings vorgegeben werden, wieviel
     * Speicher initial reserviert werden soll. Dies ist insbesondere dann sinnvoll,
     * wenn der String während seiner Lebenszeit häufig verlängert wird.
     *
     * @param[in] size Anzahl Zeichen, für die Speicher reserviert werden soll.
     *
     * @note
     * Enthält der String bereits Zeichen, gehen diese nicht verloren, der existierende
     * Speicherbereich kann aber zwecks Vergrößerung umkopiert werden. Der Aufruf
     * der Funktion WideString::clear führt dazu, dass der Speicher wieder freigegeben wird.
     */
    void reserve(size_t size);

    /** @brief Länge des Strings
     *
     * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
     * @note
     * Die Funktionen WideString::len, WideString::length und WideString::size sind identisch.
     * @see
     * WideString::capacity
     *
     * @return Anzahl Zeichen
     */
    size_t len() const;

    /** @brief Länge des Strings
     *
     * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
     * @note
     * Die Funktionen WideString::len, WideString::length und WideString::size sind identisch.
     * @see
     * WideString::capacity
     *
     * @return Anzahl Zeichen
     */
    size_t length() const;

    /** @brief Länge des Strings
     *
     * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
     * @note
     * Die Funktionen WideString::len, WideString::length und WideString::size sind identisch.
     * @see
     * WideString::capacity
     *
     * @return Anzahl Zeichen
     */
    size_t size() const;

    /** @brief Länge des Strings in Byte
     *
     * Diese Funktion gibt die Anzahl Byte zurück, die durch den String belegt werden.
     * @return Anzahl Bytes
     */
    size_t byteLength() const;

    /** @brief Prüft, ob der String leer ist.
     *
     * Diese Funktion prüft, ob der String leer ist.
     * @returns Ist der String leer, liefert die Funktion @c true zurück, sonst @c false.
     * @see WideString::notEmpty
     */
    bool isEmpty() const;

    /** @brief Prüft, ob der String Zeichen enthält
     *
     * Diese Funktion prüft, ob der String Zeichen enthält.
     * @returns Enthält der String Zeichen, liefert die Funktion @c true zurück, sonst @c false.
     * @see WideString::isEmpty
     */
    bool notEmpty() const;

    /** @brief Prüft, ob der String nummerisch ist
     *
     * Diese Funktion prüft, ob im String nur nummerische Zeichen vorhanden sind, also die Ziffern
     * 0-9, Punkt, Komma und Minus.
     * @return Ist der String nummerisch, wird 1 zurückgegeben. Ist er es nicht oder ist der String
     * leer, wird 0 zurückgegeben.
     */
    bool isNumeric() const;

    /** @brief Prüft, ob der String einen Integer Wert enthält
     *
     * Diese Funktion prüft, ob im String einen integer Wert enthält, also nur die Ziffern
     * 0-9 und optional ein Minus am Anfang enthalten sind
     * @return Ist der String ein Integer, wird 1 zurückgegeben. Ist er es nicht oder ist der String
     * leer, wird 0 zurückgegeben.
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
     * @returns Liefert true (1) zurück, wenn der String "wahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
     * @see CWWideString::IsFalse()
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
     *
     * @returns Liefert true (1) zurück, wenn der String "unwahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
     * @see CWWideString::IsTrue()
     */
    bool isFalse() const;

    /** @brief Führt einen Vergleich mit einem anderen String durch
     *
     * Führt einen Vergleich mit einem anderen String durch.
     * @param str String, mit dem verglichen werden soll
     * @param size Optionaler Parameter, der die Anzahl zu berücksichtigender Zeichen innerhalb des
     * Strings @p str angibt. Wird er nicht angegeben, wird ein vergleich mit dem kompletten String
     * @p str durchgeführt.
     *
     * @return Ist der String innerhalb dieses Objekts kleiner als der mit @a str angegebene, wird ein
     * negativer Wert zurückgegeben, ist er größer, erfolgt ein positiver Return-Wert,
     * sind beide identisch, wird 0 zurückgegeben.
     *
     * @see strCaseCmp Vergleich zweier Strings unter Ignorierung der Gross-/Kleinschreibung
     */
    int strcmp(const WideString& str, size_t size = (size_t)-1) const;

    /** @brief Stringvergleich mit Ignorierung von Gross-/Kleinschreibung
     *
     * Führt einen Vergleich mit einem anderen String durch, unter Ignorierung der
     * Gross-/Kleinschreibung.
     * @param str String, mit dem verglichen werden soll
     * @param size Optionaler Parameter, der die Anzahl zu berücksichtigender Zeichen innerhalb des
     * Strings @p str angibt. Wird er nicht angegeben, wird ein vergleich mit dem kompletten String
     * @p str durchgeführt.
     *
     *
     * @return Ist der String innerhalb dieses Objekts kleiner als der mit @a str angegebene, wird ein
     * negativer Wert zurückgegeben, ist er größer, erfolgt ein positiver Return-Wert,
     * sind beide identisch, wird 0 zurückgegeben.
     *
     * @see strcmp Vergleich zweier Strings unter Berücksichtigung der Gross-/Kleinschreibung
     */
    int strCaseCmp(const WideString& str, size_t size = (size_t)-1) const;

    /** @brief Linken Teilstring zurückgeben
     *
     * Gibt die ersten \p len Zeichen des Strings als neuen zurück.
     * @param len Länge des Teilstrings
     * @return Neuer String
     */
    WideString left(size_t len) const;

    /** @brief Rechten Teilstring zurückgeben
     *
     * Gibt die letzten \p len Zeichen des Strings als neuen zurück.
     * @param len Länge des Teilstrings
     * @return Neuer String
     */
    WideString right(size_t len) const;

    /** @brief Teilstring zurückgeben
     *
     * Gibt \p len Zeichen des Strings, beginnend ab Position \p start als
     * neuen String zurück.
     * @param start Startposition
     * @param len Optionale Länge des Teilstrings. Ist der Parameter nicht angegeben, wird
     * der komplette String ab Position @p start zurückgegeben.
     * @return Neuer String
     */
    WideString mid(size_t start, size_t len = (size_t)-1) const;

    /** @brief Teilstring zurückgeben
     *
     * Gibt \p len Zeichen des Strings, beginnend ab Position \p start als
     * neuen String zurück.
     * @param start Startposition
     * @param len Optionale Länge des Teilstrings. Ist der Parameter nicht angegeben, wird
     * der komplette String ab Position @p start zurückgegeben.
     * @return Neuer String
     */
    WideString substr(size_t start, size_t len = (size_t)-1) const;

    //! @name String setzen und verändern
    //@{

    /** @brief String anhand eines C-Strings setzen
     *
     * Mit dieser Funktion wird der String anhand eines char * gesetzt. Dabei wird er
     * intern nach Unicode konvertiert.
     * @param str Pointer auf einen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Bytes angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Bytes an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Bytes gelesen werden können.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     * @exception UnsupportedFeatureException
     * @exception UnsupportedCharacterEncodingException
     * @exception CharacterEncodingException
     *
     * @note
     * Multibyte-Characters zählen als ein Zeichen.
     *
     * @attention
     * Die Konvertierung erfolgt über die C-Funktion ::mbstowcs, die laut Definition an einem
     * eingebetteten 0-Byte im Quell-Puffer abbricht. Enthält \p str ein 0-Byte innerhalb der ersten
     * \p size Bytes, wird der String an dieser Stelle abgeschnitten, auch wenn \p size mehr Bytes
     * vorgibt. Anders als beim reinen wchar_t*-Pendant (WideString::set(const wchar_t*, size_t))
     * können über diesen Weg (und transitiv set(const std::string&, size_t), set(const String&,
     * size_t)) daher keine eingebetteten 0-Bytes übernommen werden.
     */
    WideString& set(const char* str, size_t size = (size_t)-1);

    /** @brief String anhand eines wchar_t* setzen
     *
     * Mit dieser Funktion wird der String anhand eines wchar_t * gesetzt.
     * @param str Pointer auf einen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Zeichen an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Zeichen gelesen werden können. Enthaltene 0-Werte
     * werden dabei nicht als Ende interpretiert, sondern als Teil des Inhalts übernommen.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& set(const wchar_t* str, size_t size = (size_t)-1);

    /** @brief Wert eines anderen Strings übernehmen
     *
     * Mit dieser Funktion wird der Inhalt des Strings \p str übernommen.
     * @param str Referenz auf einen anderen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Zeichen an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Zeichen gelesen werden können. Enthaltene 0-Werte
     * werden dabei nicht als Ende interpretiert, sondern als Teil des Inhalts übernommen.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& set(const WideString& str, size_t size = (size_t)-1);

    /** @brief Wert eines Strings übernehmen
     *
     * Mit dieser Funktion wird der Inhalt des Strings \p str übernommen.
     * @param str Referenz auf einen String
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Zeichen an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Zeichen gelesen werden können. Enthaltene 0-Werte
     * werden dabei nicht als Ende interpretiert, sondern als Teil des Inhalts übernommen.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& set(const String& str, size_t size = (size_t)-1);

    /** @brief WideString aus einem ByteArray übernehmen
     *
     * Mit dieser Funktion wird der Inhalt des ByteArrays \p str übernommen. Es wird davon ausgegangen,
     * das ByteArray als Array von wchar_t interpretiert wird. Falls dies nicht der Fall ist, muss
     * der Aufrufer gegebenenfalls zunächst Konvertieren, zum Beispiel mit Iconv.
     * @param str Referenz auf ein Byte-Array
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird das komplette ByteArray übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Zeichen an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Zeichen gelesen werden können. Enthaltene 0-Werte
     * werden dabei nicht als Ende interpretiert, sondern als Teil des Inhalts übernommen.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& set(const ByteArrayPtr& str, size_t size = (size_t)-1);

    /** @brief Wert eines Strings der STL übernehmen
     *
     * Mit dieser Funktion wird der Inhalt des STL-Strings \p str übernommen.
     * @param str Referenz auf einen String der Standard Template Library (STL)
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Zeichen an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Zeichen gelesen werden können. Enthaltene 0-Werte
     * werden dabei nicht als Ende interpretiert, sondern als Teil des Inhalts übernommen.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& set(const std::string& str, size_t size = (size_t)-1);

    /** @brief Wert eines Wide-Strings der STL übernehmen
     *
     * Mit dieser Funktion wird der Inhalt des STL-Wide-Strings \p str übernommen.
     * @param str Referenz auf einen Wide-String der Standard Template Library (STL)
     * @param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
     * Ist der Wert nicht angegeben, wird der komplette String übernommen. Andernfalls gibt \p size
     * die exakte Anzahl an Zeichen an, die übernommen werden soll – der Aufrufer muss sicherstellen,
     * dass ab \p str mindestens so viele gültige Zeichen gelesen werden können. Enthaltene 0-Werte
     * werden dabei nicht als Ende interpretiert, sondern als Teil des Inhalts übernommen.
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& set(const std::wstring& str, size_t size = (size_t)-1);

    /** @brief Einzelnes Unicode-Zeichen übernehmen
     *
     * Ein einzelnes Unicode-Zeichen \p c wird in den String übernommen.
     * @param c Unicode-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    WideString& set(wchar_t c);

    /** @brief Einzelnes Zeichen ersetzen
     *
     * Mit dieser Funktion wird ein einzelnes Zeichen eines Strings an der Position
     * @p position durch das Zeichen @p c ersetzt.
     *
     * @param position Position innerhalb des Strings (Zählung beginnt bei 0)
     * @param c Unicode-Wert, der gesetzt werden soll
     * @return Referenz auf den String
     * @throw OutOfBoundsException: Wird geworfen, wenn @p position größer ist, als die
     * Länge des Strings
     */
    WideString& set(size_t position, wchar_t c);

    /** @brief Erzeugt einen formatierten String
     *
     * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
     * und den optionalen Parametern \p ...
     * @param fmt Der Formatstring
     * @param ... Optionale Parameter
     *
     * @return Referenz auf den String
     *
     *
     * @code
     * int main(int argc, char **argv)
     * {
     * String s;
     * s.setf ("Anzahl Parameter: %i@n",argc);
     * s.printnl();
     * return 0;
     * }
     * @endcode
     *
     * @copydoc sprintf.dox
     */
    WideString& setf(const char* fmt, ...);

    /** @brief Fügt einen C-String an das Ende des bestehenden an
     *
     * Fügt einen C-String an das Ende des bestehenden an. Der String muss entweder
     * UTF-8 kodiert sein, oder es muss mit der statischen Funktion WideString::setGlobalEncoding
     * zuvor eine andere Kodierung gesetzt worden sein.
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
    WideString& append(const char* str, size_t size = (size_t)-1);

    /** @brief Fügt einen Wide-Character String an das Ende des bestehenden an
     *
     * Fügt einen Wide-Character String an das Ende des bestehenden an
     * @param[in] str Pointer auf einen Wide-Character String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& append(const wchar_t* str, size_t size = (size_t)-1);

    /** @brief Fügt einen String an das Ende des bestehenden an
     *
     * Fügt einen String an das Ende des bestehenden an.
     * @param[in] str Referenz auf ein String-Objekt
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& append(const WideString& str, size_t size = (size_t)-1);
    WideString& append(const String& str, size_t size = (size_t)-1);

    /** @brief Fügt einen std::string an das Ende des bestehenden an
     *
     * Fügt einen std::string an das Ende des bestehenden an.
     * @param[in] str Referenz auf ein String-Objekt der STL
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& append(const std::string& str, size_t size = (size_t)-1);

    /** @brief Fügt einen std::wstring an das Ende des bestehenden an
     *
     * Fügt einen std::wstring an das Ende des bestehenden an.
     * @param[in] str Referenz auf ein Wide-String-Objekt der STL
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& append(const std::wstring& str, size_t size = (size_t)-1);

    /** @brief Fügt einen Formatierten String an das Ende des bestehenden an
     *
     * Anhand des übergebenen Formatstrings @p fmt und den optionalen Parametern @p ...
     * wird ein neuer String gebildet, der an das Ende des bestehenden angehangen wird
     *
     * @param fmt Der Formatstring
     * @param ... Optionale Parameter
     * @return Referenz auf den String
     *
     *
     * @code
     * int main(int argc, char **argv)
     * {
     * String s;
     * s="Hallo Welt!";
     * s.appendf (" Es wurden %i Parameter übergeben@n",argc);
     * s.printnl();
     * return 0;
     * }
     * @endcode
     *
     * @copydoc sprintf.dox
     */
    WideString& appendf(const char* fmt, ...);

    /** @brief Einzelnes Unicode-Zeichen anhängen
     *
     * Ein einzelnes Unicode-Zeichen \p c wird in an den String angehangen.
     * @param c Unicode-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    WideString& append(wchar_t c);

    /** @brief Fügt einen C-String am Anfang des bestehenden Strings ein
     *
     * Fügt einen C-String am Anfang des bestehenden Strings ein
     * @param[in] str Pointer auf einen C-String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& prepend(const char* str, size_t size = (size_t)-1);

    /** @brief Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
     *
     * Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
     * @param[in] str Pointer auf einen Wide-Character String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& prepend(const wchar_t* str, size_t size = (size_t)-1);

    /** @brief Fügt einen String am Anfang des bestehenden Strings ein
     *
     * Fügt einen String am Anfang des bestehenden Strings ein
     * @param[in] str Referenz auf einen String
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& prepend(const WideString& str, size_t size = (size_t)-1);
    WideString& prepend(const String& str, size_t size = (size_t)-1);

    /** @brief Fügt einen std::string der STL am Anfang des bestehenden Strings ein
     *
     * Fügt einen std::string der Standard Template Library am Anfang des bestehenden Strings ein
     * @param[in] str Referenz auf einen std::string
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& prepend(const std::string& str, size_t size = (size_t)-1);

    /** @brief Fügt einen std::wstring der STL am Anfang des bestehenden Strings ein
     *
     * Fügt einen std::wstring der Standard Template Library am Anfang des bestehenden Strings ein
     * @param[in] str Referenz auf einen std::wstring
     * @param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
     *
     * @return Referenz auf den String
     *
     * @exception OutOfMemoryException
     */
    WideString& prepend(const std::wstring& str, size_t size = (size_t)-1);

    /** @brief Fügt einen Formatierten String am Anfang bestehenden ein
     *
     * Anhand des übergebenen Formatstrings @p fmt und den optionalen Parametern @p ...
     * wird ein neuer String gebildet, der am Anfang des bestehenden eingehangen wird.
     *
     * @param fmt Der Formatstring
     * @param ... Optionale Parameter
     *
     * @code
     * int main(int argc, char **argv)
     * {
     * String s;
     * s="Vielen Dank!@n";
     * s.prependf ("Es wurden %i Parameter übergeben. ",argc);
     * s.printnl();
     * return 0;
     * }
     * @endcode
     *
     * @copydoc sprintf.dox
     */
    WideString& prependf(const char* fmt, ...);

    /** @brief Einzelnes Unicode-Zeichen am Anfang einfügen
     *
     * Ein einzelnes Unicode-Zeichen \p c wird in am Anfang des Strings eingefügt.
     * Die nachfolgenden Zeichen des Strings verschieben sich nach rechts.
     * @param c Unicode-Wert des gewünschten Zeichens
     *
     * @return Referenz auf den String
     */
    WideString& prepend(wchar_t c);

    /** @brief Erzeugt einen formatierten String
     *
     * Erzeugt einen String anhand des übergebenen Formatstrings @p fmt
     * und den optionalen Parametern in @p args.
     *
     * @param fmt Der Formatstring
     * @param args Pointer auf Liste der Parameter. Muss zuvor mit va_start initialisiert worden sein.
     * @return Referenz auf den String
     *
     * @code
     * void MyFunction(const char *fmt, ...)
     * {
     * String s;
     * va_list args;
     * va_start(args, fmt);
     * s.vasprintf(fmt,args);
     * va_end(args);
     * printf ("Ergebnis: %ls",(const wchar_t*)s);
     * }
     * @endcode
     *
     * @copydoc sprintf.dox
     */
    WideString& vasprintf(const char* fmt, va_list args);

    /** @brief String wiederholen
     *
     * Mit dieser Funktion wird der Inhalt des Strings mehrfach wiederholt.
     * @param num Anzahl Wiederholungen. Falls @p num 0 ist, ist der String anschließend leer.
     *
     * @return Referenz auf den verlängerten String.
     */
    WideString& repeat(size_t num);

    /** @brief Füllt den String mit einem Zeichen
     *
     * Der String wird mit einem gewünschten Zeichen gefüllt
     * @param unicode Der Unicode des Zeichens, mit dem der String gefüllt werden soll
     * @param num Die Länge des gewünschten Strings
     * @return Referenz auf den neuen String
     */
    WideString& repeat(wchar_t unicode, size_t num);

    /** @brief String wiederholen
     *
     * Mit dieser Funktion wird der übergebene String \p str \p num mal wiederholt und
     * das Ergebnis in diesem String gespeichert.
     * @param str Der zu wiederholende String
     * @param num Anzahl wiederholungen
     * @return Referenz auf den String
     */
    WideString& repeat(const WideString& str, size_t num);

    /** @brief String multiplizieren
     *
     * Der aktuelle String wird \p count mal hintereinander wiederholt und
     * als neuer String zurückgegeben.
     * @param[in] count Anzahl wiederholungen
     * @return Neuer String
     * @exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
     */
    WideString repeated(size_t num) const;

    /** @brief Wandelt alle Zeichen des Strings in Kleinbuchstaben um
     *
     * Diese Funktion wandelt alle Zeichen des Strings in Kleinbuchstaben um. Die genaue Funktionsweise hängt davon ab,
     * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
     * @attention Unter UNIX (und möglicherweise anderen Betriebssystemen) werden die Lokalisationseinstellungen der
     * Umgebung nicht automatisch übernommen, sondern stehen standardmäßig auf "C". Dadurch werden nur US-ASCII
     * (ASCII 32 bis 127) umgewandelt. Man sollte daher nach Programmstart mit "setlocale" die gewünschte
     * Spracheinstellung vornehmen.
     *
     * @example
     * @code
     * #include <locale.h>
     * ...
     * setlocale(LC_CTYPE,"de_DE.UTF-8");
     * @endcode
     */
    WideString& lowerCase();

    /** @brief Wandelt alle Zeichen des Strings in Grossbuchstaben um
     *
     * Diese Funktion wandelt alle Zeichen des Strings in Großbuchstaben um. Die genaue Funktionsweise hängt davon ab,
     * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
     * @attention Unter UNIX (und möglicherweise anderen Betriebssystemen) werden die Lokalisationseinstellungen der
     * Umgebung nicht automatisch übernommen, sondern stehen standardmäßig auf "C". Dadurch werden nur US-ASCII
     * (ASCII 32 bis 127) umgewandelt. Man sollte daher nach Programmstart mit "setlocale" die gewünschte
     * Spracheinstellung vornehmen.
     *
     * @example
     * @code
     * #include <locale.h>
     * ...
     * setlocale(LC_CTYPE,"de_DE.UTF-8");
     * @endcode
     */
    WideString& upperCase();

    /** @brief Anfangsbuchstaben der Wörter groß
     *
     * Diese Funktion wandelt die Anfangsbuchstaben aller im String enthaltenen Wörter in
     * Großbuchstaben um.
     */
    WideString& upperCaseWords();
    WideString& trim();
    WideString& trimLeft();
    WideString& trimRight();
    WideString& trim(const WideString& chars);
    WideString& trimLeft(const WideString& chars);
    WideString& trimRight(const WideString& chars);

    /** @brief Schneidet Zeichen am Ende des Strings ab
     *
     * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
     * größer als der String ist, bleibt ein leerer String zurück.
     * @param num Anzahl Zeichen, die abgeschnitten werden sollen
     */
    WideString& chopRight(size_t num = 1);

    /** @brief Schneidet Zeichen am Ende des Strings ab
     *
     * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
     * größer als der String ist, bleibt ein leerer String zurück.
     * @param num Anzahl Zeichen, die abgeschnitten werden sollen
     *
     * @see
     * Die Funktion ist identisch zu WideString::chopRight
     */
    inline WideString& chop(size_t num = 1)
    {
        return chopRight(num);
    }

    /** @brief Schneidet Zeichen am Anfang des Strings ab
     *
     * Diese Funktion schneidet \p num Zeichen vom Anfang des Strings ab. Falls \p num
     * größer als der String ist, bleibt ein leerer String zurück.
     * @param num Anzahl Zeichen, die abgeschnitten werden sollen
     */
    WideString& chopLeft(size_t num = 1);

    /** @brief Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
     *
     * Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
     */
    WideString& chomp();

    /** @brief Schneidet den String an einer bestimmten Stelle ab
     *
     * Der String wird an einer bestimmten Stelle einfach abgeschnitten
     * @param pos Die Position, an der der String abgeschnitten wird. Bei Angabe von 0 ist der String anschließend
     * komplett leer. Ist @c pos größer als die Länge des Strings, passiert nichts.
     */
    WideString& cut(size_t pos);

    /** @brief Schneidet den String beim ersten Auftauchen eines Zeichens/Strings ab
     *
     * Der String wird beim ersten Auftauchen eines Zeichens oder eines Strings abgeschnitten.
     * @param[in] letter Buchstabe oder Buchstabenkombination, an der der String abgeschnitten werden
     * soll. Zeigt der Pointer auf NULL oder ist der String leer, passiert nichts.
     */
    WideString& cut(const WideString& letter);

    /** @brief Schiebt den String nach links
     *
     * Der String wird um die mit \c size angegebenen Anzahl Zeichen nach links verschoben und rechts mit dem durch \c c angegebenen
     * Zeichen aufgefüllt.
     * @param c Das Zeichen, mit dem der String auf der rechten Seite aufgefüllt werden soll. Wird der Wert 0 verwendet, findet keine
     * Auffüllung statt, d.h. der String verkürzt sich einfach.
     * @param size Die Anzahl Zeichen, um die der String nach links verschoben werden soll. Ist \c size größer als die Länge
     * des Strings, wird der String komplett geleert und ist anschließend so groß wie size, sofern c>0 war.
     *
     * @return Referenz auf den String
     */
    WideString& shl(wchar_t c, size_t size);

    /** @brief Schiebt den String nach rechts
     *
     * Der String wird um die mit \c size angegebenen Anzahl Zeichen nach rechts verschoben und links mit dem durch \c c angegebenen
     * Zeichen aufgefüllt.
     * @param c Das Zeichen, mit dem der String auf der linken Seite aufgefüllt werden soll. Wird der Wert 0 verwendet, findet keine
     * Auffüllung statt, d.h. der String verkürzt sich einfach.
     * @param size Die Anzahl Zeichen, um die der String nach rechts verschoben werden soll. Ist \c size größer als die Länge
     * des Strings, wird der String komplett geleert und ist anschließend so groß wie size, sofern c>0 war.
     *
     * @return Referenz auf den String
     */
    WideString& shr(wchar_t c, size_t size);

    WideString trimmed() const;

    WideString toLowerCase() const;
    WideString toUpperCase() const;

    WideString strchr(wchar_t c) const;
    WideString strrchr(wchar_t c) const;

    /** @brief Teilstring finden
     *
     * Diese Funktion findet die erste Position der Zeichenfolge \p needle
     * innerhalb des Strings. Abschließende `\0'-Zeichen werden nicht
     * miteinander verglichen.
     * @param needle
     * @return
     * Die Funktion gibt einen String zurück, der mit der gefundenen
     * Zeichenkette beginnt und den Rest des Strings bis zum Ende enthält.
     * Wurde die Zeichenkette nicht gefunden, wird ein leerer String
     * zurückgegeben.
     * @note
     * Ein Sonderfall besteht, wenn @p needle leer ist. In diesem Fall wird
     * der komplette String zurückgegeben.
     */
    WideString strstr(const WideString& needle) const;

    /** @brief Sucht nach einem String
     *
     * Find sucht nach dem Suchstring @a needle ab der gewünschten Position @a start.
     *
     * @param[in] needle Gesuchter Teilstring
     * @param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
     * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
     * Ende des Strings gesucht.
     *
     * @return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder WideString::npos wenn er nicht gefunden wurde. Ist @p needle ein leerer String, liefert die
     * Funktion immer 0 zurück.
     */
    ssize_t find(const WideString& needle, ssize_t start = 0) const;

    /** @brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
     *
     * Diese Funktion sucht innerhalb des aktuellen String nach \p needle ab der
     * gewünschten Position \a start. Gross-/Kleinschreibung wird dabei ignoriert.
     * @param[in] needle Gesuchter Teilstring
     * @param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
     * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
     * Ende des Strings gesucht.
     *
     * @return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder WideString::npos wenn er nicht gefunden wurde. Ist @p needle ein leerer String, liefert die
     * Funktion immer 0 zurück.
     */
    ssize_t findCase(const WideString& needle, ssize_t start = 0) const;

    /*! @brief Sucht nach einem String
     *
     * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
     *
     * \param[in] needle Gesuchter Teilstring
     * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
     * oder nicht angegeben, wird der String vom Anfang an gesucht.
     *
     * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder WideString::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
     * Funktion immer WideString::npos zurück.
     */
    inline ssize_t instr(const WideString& needle, size_t start = 0) const
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
     * oder nicht angegeben, wird der String vom Anfang an gesucht.
     *
     * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
     * oder WideString::npos wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
     * Funktion immer WideString::npos zurück.
     */
    inline ssize_t instrCase(const WideString& needle, size_t start = 0) const
    {
        return findCase(needle, start);
    }

    bool has(const WideString& needle, bool ignoreCase = false) const;
    inline bool contains(const WideString& needle, bool ignoreCase = false) const
    {
        return has(needle, ignoreCase);
    }

    /** @brief Prüft, ob der String mit einem bestimmten Präfix beginnt
     *
     * Diese Funktion prüft, ob der String mit dem angegebenen Präfix beginnt.
     *
     * @param prefix Präfix, das überprüft werden soll
     * @param start Startposition innerhalb des Strings, ab der die Prüfung erfolgen soll
     * @param len Länge des zu prüfenden Bereichs ab \p start. Ist der Parameter nicht
     *               angegeben, wird bis zum Ende des Strings geprüft.
     * @return Liefert true zurück, wenn der String mit dem Präfix beginnt, andernfalls false.
     */
    bool startsWith(const WideString& prefix, size_t start = 0, size_t len = (size_t)-1) const;

    /** @brief Prüft, ob der String mit einem bestimmten Suffix endet
     *
     * Diese Funktion prüft, ob der String mit dem angegebenen Suffix endet.
     *
     * @param suffix Suffix, das überprüft werden soll
     * @param start Startposition innerhalb des Strings, ab der die Prüfung erfolgen soll
     * @param len Länge des zu prüfenden Bereichs ab \p start. Ist der Parameter nicht
     *               angegeben, wird bis zum Ende des Strings geprüft.
     * @return Liefert true zurück, wenn der String mit dem Suffix endet, andernfalls false.
     */
    bool endsWith(const WideString& suffix, size_t start = 0, size_t len = (size_t)-1) const;
    WideString join(const pplib::Array& iterable) const;

    WideString& replace(const WideString& search, const WideString& replacement);

    //@}

    //! @name String ausgeben und auslesen
    //@{
    void print(bool withNewline = false) const noexcept;
    void printnl() const noexcept;

    /** @brief Hexdump des Strings ausgeben
     *
     * Mit dieser zu Debug-Zwecken gedachten Funktion wird der Inhalt des
     * Strings als HexDump auf der Konsole ausgegeben.
     */
    void hexDump() const;

    /** @brief Einzelnes Zeichen auslesen
     *
     * Mit dieser Funktion kann der Unicode-Wert eines einzelnen Zeichens an der Position
     * @p pos ausgelesen werden. Enthält @p pos einen positiven Wert, wird die Position des
     * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
     * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
     * dem letzten Zeichen des Strings entspricht.
     *
     * @param pos Position des Zeichens innerhalb des Strings
     * @return Unicode-Wert des Zeichens
     * @exception OutOfBoundsException Wird geworfen, wenn die angegebene Position @p pos
     * ausserhalb des Strings liegt oder der String leer ist.
     */
    wchar_t get(ssize_t pos) const;

    inline const wchar_t* getPtr() const
    {
        return stringlen > 0 ? ptr : L"";
    }

    inline const wchar_t* c_str() const
    {
        return stringlen > 0 ? ptr : L"";
    }

    /**@brief String in UTF-8 umwandeln
     *
     * Mit dieser Funktion wird der Inhalt des Strings in UTF-8 umgewandelt und als
     * ByteArray zurückgegeben. Dabei ist garantiert, dass unabhängig von den eingestellten
     * locales immer UTF-8 zurückgegeben wird.
     *
     * @return ByteArray mit der UTF8-Repräsentation des Strings.
     */
    ByteArray toUtf8() const;

    /**@brief String aus UTF-8 kodierten Bytes setzen
     *
     * Mit dieser Funktion wird der Inhalt des Strings anhand des als \p bin übergebenen,
     * UTF-8 kodierten Speicherbereichs gesetzt. Die Dekodierung erfolgt fest verdrahtet und
     * unabhängig von der eingestellten locale (Gegenstück zu WideString::toUtf8), sodass ein
     * Roundtrip über toUtf8()/fromUtf8() unabhängig von setlocale() immer verlustfrei
     * funktioniert.
     *
     * Die Dekodierung ist streng nach RFC 3629: overlong encodings, direkt in UTF-8 kodierte
     * UTF-16-Surrogates (0xD800-0xDFFF) sowie Codepoints jenseits von U+10FFFF gelten als
     * ungültig. Jede ungültige Byte-Sequenz wird durch das Replacement-Zeichen U+FFFD ersetzt,
     * wobei nur das jeweils erste ungültige Byte übersprungen wird, um die Synchronisation mit
     * dem restlichen Datenstrom nicht zu verlieren.
     *
     * @param bin Referenz auf einen Speicherbereich mit UTF-8 kodierten Daten
     * @return Referenz auf den String
     * @exception OutOfMemoryException
     */
    WideString& fromUtf8(const ByteArrayPtr& bin);

    /**@brief String aus UTF-8 kodiertem String setzen
     *
     * @copydetails WideString::fromUtf8(const ByteArrayPtr&)
     * @param str Referenz auf einen String mit UTF-8 kodiertem Inhalt
     */
    WideString& fromUtf8(const String& str);

    /**@brief String in die lokale Kodierung umwandeln
     *
     * Mit dieser Funktion wird der Inhalt des Strings in die lokale Kodierung umgewandelt und als
     * String zurückgegeben. Die lokale Kodierung wird mit der Funktion setlocale() aus der
     * C-Standardbibliothek festgelegt.
     *
     * @return String mit der lokalen Repräsentation des Strings.
     */
    String toString() const;

    ByteArray toUCS4() const;
    WideString& fromUCS4(const uint32_t* str, size_t size = (size_t)-1);
    WideString& fromUCS4(const ByteArrayPtr& bin);

    int toInt() const;
    unsigned int toUnsignedInt() const;
    int64_t toInt64() const;
    uint64_t toUnsignedInt64() const;
    bool toBool() const;
    long toLong() const;
    unsigned long toUnsignedLong() const;
    long long toLongLong() const;
    unsigned long long toUnsignedLongLong() const;
    float toFloat() const;
    double toDouble() const;

    /** @brief %Pointer auf den internen Unicode-String
     *
     * @copydetails WideString::getPtr
     */
    const wchar_t* toWchart() const;

    //@}

    //! @name Operatoren
    //@{
    operator const wchar_t*() const;
    operator int() const;
    operator unsigned int() const;
    operator bool() const;
    operator int64_t() const;
    operator uint64_t() const;
    operator float() const;
    operator double() const;
    operator std::string() const;
    operator std::wstring() const;

    /** @brief Einzelnes Zeichen auslesen
     *
     * Mit diesem Operator kann der Unicode-Wert eines einzelnen Zeichens an der Position
     * @p pos ausgelesen werden. Enthält @p pos einen positiven Wert, wird die Position des
     * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
     * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
     * dem letzten Zeichen des Strings entspricht.
     *
     * @param pos Position des Zeichens innerhalb des Strings
     * @return Unicode-Wert des Zeichens
     * @exception OutOfBoundsException Wird geworfen, wenn die angegebene Position @p pos
     * ausserhalb des Strings liegt oder der String leer ist.
     */
    wchar_t operator[](ssize_t pos) const;
    wchar_t& operator[](ssize_t pos);

    /** @brief String übernehmen
     *
     * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
     * ist identisch mit der Funktion WideString::set
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(const char* str);

    /** @brief String übernehmen
     *
     * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
     * ist identisch mit der Funktion WideString::set
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(const wchar_t* str);
    WideString& operator=(const WideString& str);

    /** @brief String übernehmen
     *
     * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
     * ist identisch mit der Funktion WideString::set
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(const String& str);

    /** @brief String übernehmen
     *
     * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
     * ist identisch mit der Funktion WideString::set
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(const std::string& str);

    /** @brief String übernehmen
     *
     * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
     * ist identisch mit der Funktion WideString::set
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(const std::wstring& str);

    /** @brief Zeichen übernehmen
     *
     * Mit diesem Operator wird ein einzelnes Zeichen in den String kopiert.
     * @param[in] c Unicode Wert des zu übernehmenden Zeichens
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(wchar_t c);

    /** @brief String übernehmen
     *
     * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
     * ist identisch mit der Funktion WideString::set
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator=(WideString&& other) noexcept;

    /** @brief String addieren
     *
     * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
     * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator+=(const char* str);

    /** @brief String addieren
     *
     * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
     * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator+=(const wchar_t* str);
    WideString& operator+=(const String& str);

    /** @brief String addieren
     *
     * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
     * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator+=(const WideString& str);

    /** @brief String addieren
     *
     * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
     * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator+=(const std::string& str);

    /** @brief String addieren
     *
     * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
     * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
     * @param[in] str Zu kopierender String
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator+=(const std::wstring& str);

    /** @brief Zeichen anhängen
     *
     * Mit diesem Operator wird das angegebene Zeichen \p c an den bisher vorhandenen
     * String angehangen. Der Operator ist identisch mit der Funktion WideString::append.
     * @param[in] c Unicode-Wert des anzuhängenden Zeichens
     * @return Referenz auf diese Instanz der Klasse
     */
    WideString& operator+=(wchar_t c);

    /** @brief Kleiner als
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
     * rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator<(const WideString& str) const;

    /** @brief Kleiner oder gleich
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
     * dem des rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator<=(const WideString& str) const;

    /** @brief Gleich
     *
     * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
     * rechten identisch ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator==(const WideString& str) const;

    /** @brief Ungleich
     *
     * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
     * rechten entspricht.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator!=(const WideString& str) const;

    /** @brief Größer oder gleich
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
     * gleich dem des rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator>=(const WideString& str) const;

    /** @brief Größer als
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
     * dem des rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator>(const WideString& str) const;

    /** @brief Kleiner als
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
     * rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator<(const wchar_t* str) const;

    /** @brief Kleiner oder gleich
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
     * dem des rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator<=(const wchar_t* str) const;

    /** @brief Gleich
     *
     * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
     * rechten identisch ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator==(const wchar_t* str) const;

    /** @brief Ungleich
     *
     * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
     * rechten entspricht.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator!=(const wchar_t* str) const;

    /** @brief Größer oder gleich
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
     * gleich dem des rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator>=(const wchar_t* str) const;

    /** @brief Größer als
     *
     * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
     * dem des rechten ist.
     * @param str Zu vergleichender String
     * @return Liefert @c true oder @c false zurück
     */
    bool operator>(const wchar_t* str) const;

    //@}

    //! @name Iteratoren
    //@{
    typedef wchar_t* iterator;
    typedef const wchar_t* const_iterator;
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

#ifdef WITH_QT
    //! @name Operatoren zur Verwendung der Klasse mit Qt
    //@{
    operator const QString() const
    {
        return QString::fromWCharArray(ptr, stringlen);
    }

    operator const QVariant() const
    {
        QVariant v = QString::fromWCharArray(ptr, stringlen);
        return v;
    }

    WideString& operator=(const QString& q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q.toUtf8();
#else
        QByteArray a = q.toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    WideString& operator=(const QString* q)
    {
#ifdef PPL_QT_STRING_UTF8
        QByteArray a = q->toUtf8();
#else
        QByteArray a = q->toLocal8Bit();
#endif
        set((const char*)a);
        return *this;
    }
    //@}
#endif
};

WideString operator+(const WideString& str1, const WideString& str2);
WideString operator+(const char* str1, const WideString& str2);
WideString operator+(const WideString& str1, const char* str2);
WideString operator+(const wchar_t* str1, const WideString& str2);
WideString operator+(const WideString& str1, const wchar_t* str2);
WideString operator+(const std::string& str1, const WideString& str2);
WideString operator+(const WideString& str1, const std::string& str2);
WideString operator+(const std::wstring& str1, const WideString& str2);
WideString operator+(const WideString& str1, const std::wstring& str2);
WideString operator+(const WideString& str1, const wchar_t c);
WideString operator+(const wchar_t, const WideString& str1);

std::ostream& operator<<(std::ostream& s, const WideString& str);

} // namespace pplib

#endif /* PPLIB_TYPES_WIDESTRING_H_ */
