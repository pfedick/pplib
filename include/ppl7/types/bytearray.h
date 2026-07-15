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

#ifndef PPL7_TYPES_BYTEARRAY_H_
#define PPL7_TYPES_BYTEARRAY_H_

#include <iostream>
#include <ppl7/types/bytearrayptr.h>

namespace ppl7
{
class String;
class WideString;

/**@class ByteArray
 * @ingroup PPLGroupDataTypes
 * @brief Verwaltung eines einzelnen Speicherblocks
 *
 * Mit dieser Klasse kann ein einzelner Speicherblock verwaltet werden.
 * Dieser besteht immer aus einer Speicheraddresse und der Größe des
 * Speicherbereichs in Bytes. Die Klasse stellt sicher, dass am Ende des
 * allokierten Speicherbereichs immer 4 0-Bytes angehangen werden. Dadurch kann bei
 * Verwendung der Pointer-Operatoren der Speicher direkt als Eingabe für C-Strings verwendet werden.
 * Bei Verwendung der Funktion ByteArray::useadr ist dies nicht der Fall, da die Klasse in diesem
 * Fall den Speicherbereich nicht selbst allokiert.
 *
 * Der Speicherbereich kann mit ByteArray::set gesetzt werden und mit ByteArray::adr und ByteArray::size
 * ausgelesen werden. Außerdem kann mit dem Operator [] ein bestimmtes Byte aus dem
 * Speicherbereich ausgelesen oder verändert werden.
 *
 * Im Gegensatz zu ByteArrayPtr verwaltet diese Klasse den Speicherbereich selbst. Das heisst
 * der verwaltete Speicher wird bei Veränderungen oder Beendigung der Klasse automatisch freigegeben.
 *
 * Mit ByteArray::malloc oder ByteArray::calloc wird Speicher allokiert, mit ByteArray::free kann er
 * vorzeitig wieder freigegeben werden. Mit ByteArray::copy wird der Speicher eines anderen ByteArray-
 * oder ByteArrayPtr-Objekts oder aus einer angegebenen Speicheradresse dupliziert. Mit ByteArray::append
 * oder ByteArray::prepend kann der Speicher eines anderen ByteArray- oder ByteArrayPtr-Objekts oder aus
 * einer angegebenen Speicheradresse an den bestehenden Speicherbereich an- oder davorgehangen werden.
 *
 * Mit ByteArray::useadr angegebener Speicherbereich wird nicht dupliziert,
 * wird aber fortan von der Klasse verwaltet, das heisst die Anwendung darf diesen nicht selbst
 * freigeben.
 *
 * @see
 * - ByteArrayPtr
 */
class ByteArray : public ByteArrayPtr
{
public:
    /**@brief Konstruktor ohne Argumente
     *
     * Dieser Konstruktor erstellt eine Inszanz der Klasse ohne einen zugeweisenen Speicherbereich
     */
    ByteArray();

    /**@brief Copy-Konstruktor
     *
     * Mit diesem Konstruktor wird der Speicherbereich eines anderen ByteArray- oder ByteArrayPtr-
     * Objekts kopiert.
     *
     * @param[in] other Referenz auf eine andere ByteArray- oder ByteArrayPtr-Klasse
     */
    ByteArray(const ByteArrayPtr& other);

    /**@brief Move-Konstruktor
     *
     * Mit diesem Konstruktor wird der Speicherbereich eines anderen ByteArray-Objekts übernommen.
     * Das andere Objekt enthält danach keinen Speicherbereich mehr.
     *
     * @param[in] other Referenz auf eine andere ByteArray-Klasse
     */
    ByteArray(ByteArray&& other);

    /**@brief Konstruktor mit Angabe einer Speicheradresse und Größe
     *
     * Mit diesem Konstruktor wird eine neue Instanz der Klasse erzeugt, und der
     * mit \p adr und \p size angegebenen Speicherbereich kopiert.
     *
     * @param[in] adr Pointer auf den Beginn des Speicherbereichs
     * @param[in] size Größe des Speicherbereichs in Bytes
     */
    ByteArray(const void* adr, size_t size);

    /**@brief Konstruktor durch String
     *
     * Mit diesem Konstruktor wird der Speicherbereich eines Strings in das Objekt kopiert.
     *
     * @param[in] other Referenz auf eine String-Klasse
     */
    explicit ByteArray(const String& str);

    /**@brief Konstruktor durch WideString
     *
     * Mit diesem Konstruktor wird der Speicherbereich eines WideStrings in das Objekt kopiert.
     *
     * @param[in] other Referenz auf eine WideString-Klasse
     */
    explicit ByteArray(const WideString& str);

    /**@brief Konstruktor mit Speicher-Allokierung
     *
     * Mit diesem Konstruktor wird eine Instanz der Klasse erzeugt, die einen Speicherblock mit
     * der Größe \p size allokiert und diesen selbst verwaltet.
     *
     * @param[in] size Gewünschte Größe des Speicherblocks in Bytes
     */
    ByteArray(size_t size);

    /**@brief Destruktor der Klasse
     *
     * Der Destruktor sort dafür, dass der allokierte Speicher mittes "free" wieder freigegeben wird.
     */
    ~ByteArray();

    /**@brief Speicherverwaltung übernehmen
     *
     * Mit dieser Funktion wird der Klasse die Verwaltung des Speicherbereich mit der Adresse \p adr und der
     * Größe \p size übergeben. Der Speicher muss zuvor mit "malloc" bzw. "calloc" allokiert worden sein
     * und darf von der Anwendung selbst nicht mehr freigegeben werden.
     *
     * @param[in] adr Startadresse des Speicherbereichs
     * @param[in] size Größe des Speicherbereichs in Bytes
     *
     * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
     *
     * @attention
     * Bei Verwendung dieser Funktion werden keine zusätzlichen 0-Byte am Ende des Speicherbereichs
     * hinzugefügt. Fall das Objekt als Eingabe für eine C-String-Funktion verwendet werden soll, muss die
     * Anwendung selbst dafür sorgen, dass am Ende ein 0-Byte vorhanden ist.
     */
    void useadr(void* adr, size_t size);

    /**@brief Speicherbereich kopieren
     *
     * Mit dieser Funktion wird der durch \p adr und \p size angegebene Speicherbereich in
     * die ByteArray-Klasse kopiert. Die Klasse allokiert dafür zunächst den erforderlichen
     * Speicher und übernimmt dessen Verwaltung.
     *
     * @param[in] adr
     * @param[in] size
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     * @note
     * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
     */
    void* copy(const void* adr, size_t size);

    /**@brief Speicherbereich kopieren
     *
     * Mit dieser Funktion wird der durch \p other referenzierte Speicherbereich in
     * die ByteArray-Klasse kopiert. Die Klasse allokiert dafür zunächst den erforderlichen
     * Speicher und übernimmt dessen Verwaltung.
     *
     * @param[in] other Referenz auf das ByteArray- oder ByteArrayPtr-Objekt, von dem der
     * Speicher kopiert werden soll.
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    void* copy(const ByteArrayPtr& other);

    /**@brief Speicherbereich anhängen
     *
     * Mit dieser Funktion wird der durch \p adr und \p size angegebene Speicherbereich an die
     * bereits in der Klasse vorhandenen Daten angehangen. Die Klasse allokiert dafür zunächst
     * den erforderlichen Speicher und kopiert dann die Daten.
     *
     * @param[in] adr
     * @param[in] size
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     * @note
     * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
     */
    void* append(const void* adr, size_t size);

    /**@brief Speicherbereich aus einem ByteArray oder ByteArrayPtr-Objekt anhängen
     *
     * Mit dieser Funktion wird der durch \p other referenzierte Speicherbereich an die
     * bereits in der Klasse vorhandenen Daten angehangen. Die Klasse allokiert dafür zunächst
     * den erforderlichen Speicher und kopiert dann die Daten.
     *
     * @param[in] other Referenz auf das ByteArray- oder ByteArrayPtr-Objekt, von dem der
     * Speicher kopiert werden soll.
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    void* append(const ByteArrayPtr& other);

    /**@brief Speicherbereich davorhängen
     *
     * Mit dieser Funktion wird der durch \p adr und \p size angegebene Speicherbereich vor die
     * bereits in der Klasse vorhandenen Daten gehangen. Die Klasse allokiert dafür zunächst
     * den erforderlichen Speicher und kopiert dann die Daten.
     *
     * @param[in] adr
     * @param[in] size
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     *
     * @note
     * Falls die Klasse vor Aufruf bereits Speicher verwaltet hat, wird dieser zuerst freigegeben.
     */
    void* prepend(const void* adr, size_t size);

    /**@brief Speicherbereich aus einem ByteArray-Objekt davorhängen
     *
     * Mit dieser Funktion wird der durch \p other referenzierte Speicherbereich
     * vor die Daten dieser Instanz der ByteArray-Klasse gehangen. Dazu wird
     * Speicher reallokiert und kopiert.
     *
     * @param[in] other Referenz auf das ByteArray- oder ByteArrayPtr-Objekt, von dem der
     * Speicher kopiert werden soll.
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception
     * geworfen.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    void* prepend(const ByteArrayPtr& other);

    /**@brief Speicherbereich aus einem Hex-String kopieren
     *
     * Mit dieser Funktion werden die im String \p hex kodierten Hexadezimal-Werte
     * dekodiert und in den Speicherbereich der ByteArray-Klasse kopiert. Dabei wird davon
     * ausgegangen, dass die Hexadezimalwerte im String fortlaufend sind und jeweils zwei
     * Zeichen ein Byte repräsentieren. Der durch die Klasse allokierte Speicherbereich wird daher
     * halb so gross sein, wie der Eingabe-String.
     *
     * @param[in] hex Referenz auf einen Hex-String, der den Speicherbereich beschreibt.
     * @return Bei erfolgreichem Kopieren liefert die Funktion einen Pointer auf den
     * neu allokierten Speicherbereich zurück. Im Fehlerfall wird eine Exception geworfen.
     *
     * @exception IllegalArgumentException Der Eingabe-String enthält eine ungerade Anzahl an Zeichen oder ist leer
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    void* fromHex(const String& hex);

    /**@brief Speicherbereich allokieren
     *
     * Mit dieser Funktion wird ein neuer Speicherblock mit der Größe von \p size Bytes allokiert
     * und dessen Adresse als Ergebnis zurückgeliefert. Das ByteArray-Objekt übernimmt die Verwaltung
     * des Speicherblocks.
     *
     * @param[in] size Gewünschte Größe des Speicherblocks.
     * @return Pointer auf den Beginn des allokierten Speichers, oder NULL, wenn ein Fehler beim
     * Allokieren aufgetreten ist. Ein entsprechender Fehlercode wird gesetzt.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    void* malloc(size_t size);

    /**@brief Speicherbereich allokieren und mit 0 initialisieren
     *
     * Mit dieser Funktion wird ein neuer Speicherblock mit der Größe von \p size Bytes allokiert,
     * der Inhalt mit 0 initialisiert und dessen Adresse als Ergebnis zurückgeliefert.
     * Das ByteArray-Objekt übernimmt die Verwaltung des Speicherblocks.
     *
     * @param[in] size Gewünschte Größe des Speicherblocks.
     * @return Pointer auf den Beginn des allokierten Speichers, oder NULL, wenn ein Fehler beim
     * Allokieren aufgetreten ist. Ein entsprechender Fehlercode wird gesetzt.
     *
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    void* calloc(size_t size);

    /**@brief Speicher freigeben
     *
     * Mit dieser Funktion wird der von der ByteArray-Klasse verwaltete Speicherbereich wieder freigegeben.
     * Die Klasse enthält danach keinen Speicherbereich mehr.
     */
    void free();

    /**@brief Speicher freigeben
     *
     * Mit dieser Funktion wird der von der ByteArray-Klasse verwaltete Speicherbereich wieder freigegeben.
     * Die Klasse enthält danach keinen Speicherbereich mehr.
     */
    void clear();

    /**@brief Speicherreferenz von anderem ByteArray- oder ByteArrayPtr-Objekt übernehmen
     *
     * Mit diesem Operator wird der Speicherbereich eines anderen ByteArray- oder ByteArrayPtr-Objekts
     * kopiert. Er ist identisch zur Funktion ByteArray::copy.
     *
     * @param[in] other Referenz auf ein anderes ByteArray- oder ByteArrayPtr-Objekt.
     * @return Referenz auf das Objekt
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    ByteArray& operator=(const ByteArrayPtr& other);

    /**@brief Move-Operator
     *
     * Mit diesem Operator wird der Speicherbereich eines anderen ByteArray-Objekts übernommen.
     * Das andere Objekt enthält danach keinen Speicherbereich mehr.
     *
     * @param[in] other Referenz auf ein anderes ByteArray-Objekt.
     * @return Referenz auf das Objekt
     */
    ByteArray& operator=(ByteArray&& other);

    /**@brief Inhalt eines Strings in den Speicherbereich kopieren
     *
     * Mit diesem Operator wird der Speicherbereich eines Strings in das Objekt kopiert.
     *
     * @param[in] str Referenz auf eine String-Klasse
     * @return Referenz auf das Objekt
     */
    ByteArray& operator=(const String& str);

    /**@brief Inhalt eines WideStrings in den Speicherbereich kopieren
     *
     * Mit diesem Operator wird der Speicherbereich eines WideStrings in das Objekt kopiert.
     *
     * @param[in] str Referenz auf eine WideString-Klasse
     * @return Referenz auf das Objekt
     */
    ByteArray& operator=(const WideString& str);

    /**@brief Speicherbereich anfügen
     *
     * Mit diesem Operator wird der Speicherbereich eines anderen ByteArray- oder ByteArrayPtr-Objekts
     * an den Speicherbereich dieser Instanz angehangen. Er ist identisch zur Funktion ByteArray::append.
     *
     * @param[in] other Referenz auf ein anderes ByteArray- oder ByteArrayPtr-Objekt.
     * @return Referenz auf das Objekt
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    ByteArray& operator+=(const ByteArrayPtr& other);

    /**@brief Speicherbereich eines Strings anfügen
     *
     * Mit diesem Operator wird der Speicherbereich eines Strings an den Speicherbereich dieser Instanz angehangen.
     *
     * @param[in] other Referenz auf eine String-Klasse
     * @return Referenz auf das Objekt
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    ByteArray& operator+=(const String& other);

    /**@brief Speicherbereich eines WideStrings anfügen
     *
     * Mit diesem Operator wird der Speicherbereich eines WideStrings an den Speicherbereich dieser Instanz angehangen.
     *
     * @param[in] other Referenz auf eine WideString-Klasse
     * @return Referenz auf das Objekt
     * @exception OutOfMemoryException Speicher konnte nicht allokiert werden
     */
    ByteArray& operator+=(const WideString& other);
};

std::ostream& operator<<(std::ostream& s, const ByteArray& ba);

} // namespace ppl7

#endif // PPL7_TYPES_BYTEARRAY_H_
