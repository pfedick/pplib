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

#ifndef PPL7_TYPES_BYTEARRAYPTR_H_
#define PPL7_TYPES_BYTEARRAYPTR_H_

#include <stdint.h>
#include <iostream>
namespace ppl7
{

class String;
class WideString;

/**@class ByteArrayPtr
 * @ingroup PPLGroupDataTypes
 * @brief Referenz auf einen Speicherbereich
 *
 * Mit dieser Klasse kann ein einzelner Speicherblock repräsentiert werden.
 * Dieser besteht immer aus einer Speicheraddresse und der Größe des
 * Speicherbereichs in Bytes.
 *
 * Der Speicherbereich kann mit ByteArray::set gesetzt werden und mit ByteArray::adr und ByteArray::size
 * ausgelesen werden. Außerdem kann mit dem Operator [] ein bestimmtes Byte aus dem
 * Speicherbereich ausgelesen werden.
 *
 * Die Klasse enthält immer nur eine Referenz auf einen bestehenen Speicherbereich,
 * nicht den Speicher selbst. Soll auch der Speicher verwaltet werden, kann man
 * stattdessen die abgeleitete Klasse ByteArray verwenden.
 *
 * @see
 * - ByteArray
 */
class ByteArrayPtr
{
    friend class ByteArray;

private:
    void* ptradr;   /// Pointer auf den referenzierten Speicherbereich
    size_t ptrsize; /// Größe des referenzierten Speicherbereichs

public:
    /**@brief Konstruktor ohne Argumente
     *
     * Dieser Konstruktor erstellt eine Inszanz der Klasse ohne einen zugeweisenen Speicherbereich
     */
    ByteArrayPtr();

    /**@brief Copy-Konstruktor
     *
     * Mit diesem Konstruktor wird eine Referenz auf den Speicherbereich einer anderen
     * ByteArrayPtr- oder ByteArray-Klasse übernommen.
     *
     * @param[in] other Referenz auf eine andere ByteArrayReferenz-Klasse
     */
    ByteArrayPtr(const ByteArrayPtr& other);

    /**@brief Konstruktor mit Angabe einer String-Instanz
     *
     * Mit diesem Konstruktor wird eine Referenz auf den Speicherbereich einer String-Instanz übernommen.
     *
     * @param[in] data Referenz auf eine String-Instanz
     */
    explicit ByteArrayPtr(const String& data);

    /**@brief Konstruktor mit Angabe einer WideString-Instanz
     *
     * Mit diesem Konstruktor wird eine Referenz auf den Speicherbereich einer WideString-Instanz übernommen.
     *
     * @param[in] data Referenz auf eine WideString-Instanz
     */
    explicit ByteArrayPtr(const WideString& data);

    /**@brief Konstruktor mit Angabe einer Speicheradresse und Größe
     *
     * Mit diesem Konstruktor wird eine neue Instanz der Klasse erzeugt, die eine Referenz auf den mit
     * \p adr und \p size angegebenen Speicherbereich enthält.
     *
     * @param[in] adr Pointer auf den Beginn des Speicherbereichs
     * @param[in] size Größe des Speicherbereichs in Bytes
     */
    ByteArrayPtr(const void* adr, size_t size);

    /**@brief Prüfen, ob Speicher referenziert ist und größer als 0 Byte ist
     *
     * Mit dieser Funktion kann geprüft werden, ob die Klasse zur Zeit eine Referenz auf einen
     * Speicherbereich enthält und dieser größer als 0 Byte ist.
     *
     * @return
     * Enthält die Klasse keine Referenz auf einen Speicherbereich, der mindestens 1 Byte
     * gross ist, liefert die Funktion \c true zurück, andernfalls \c false.
     */
    bool isNull() const;

    /**@brief Prüfen, ob Speicher referenziert ist und größer als 0 Byte ist
     *
     * Mit dieser Funktion kann geprüft werden, ob die Klasse zur Zeit eine Referenz auf einen
     * Speicherbereich enthält und dieser größer als 0 Byte ist.
     *
     * @return
     * Enthält die Klasse keine Referenz auf einen Speicherbereich, der mindestens 1 Byte
     * gross ist, liefert die Funktion \c true zurück, andernfalls \c false.
     */
    bool isEmpty() const;

    /**@brief Prüfen, ob Speicher referenziert ist und größer als 0 Byte ist
     *
     * Mit dieser Funktion kann geprüft werden, ob die Klasse zur Zeit eine Referenz auf einen
     * Speicherbereich enthält und dieser größer als 0 Byte ist.
     *
     * @return
     * Enthält die Klasse eine Referenz auf einen Speicherbereich, der mindestens 1 Byte
     * gross ist, liefert die Funktion \c true zurück, andernfalls \c false.
     */
    bool notEmpty() const;

    /**@brief Größe des Speicherblocks auslesen
     *
     * Mit dieser Funktion kann die Größe des Speicherblocks ausgelesen werden.
     *
     * @return Größe des Speicherblocks oder 0, wenn kein Speicher zugeordnet ist.
     */
    size_t size() const;

    /**@brief Adresse des Speicherblocks auslesen
     *
     * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
     *
     * @return Adresse des Speicherblocks
     */
    const void* adr() const;

    /**@brief Adresse des Speicherblocks auslesen
     *
     * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
     *
     * @return Adresse des Speicherblocks
     */
    const void* ptr() const;

    /**@brief Referenz auf Speicherbereich setzen
     *
     * Mit dieser Funktion wird der Klasse der Speicherbereich mit der Adresse \p adr und der
     * Größe \p size zugeordnet. Der Speicherbereich selbst wird von der Klasse nicht verwaltet,
     * sondern nur referenziert.
     *
     * @param[in] adr Pointer auf den Beginn des Speicherbereichs
     * @param[in] size Größe des Speicherbereichs in Bytes
     */
    void use(void* adr, size_t size);

    /**@brief Referenz auf Speicherbereich von einer anderen ByteArrayPtr-Instanz kopieren
     *
     * Mit dieser Funktion wird eine Referenz auf einen Speicherbereich von einer anderen
     * ByteArrayPtr- oder ByteArray- Instanz kopiert.
     *
     * @param[in] other Referenz auf ein anderes ByteArray-Objekt.
     */
    void use(const ByteArrayPtr& other);

    /**@brief Speicherreferenz von anderem ByteArrayPtr-Objekt übernehmen
     *
     * Mit diesem Operator wird eine Referenz auf einen Speicherbereich von einer anderen
     * ByteArrayPtr- oder ByteArray Instanz übernommen.
     *
     * @param[in] other Referenz auf ein anderes ByteArrayPtr-Objekt.
     * @return Referenz auf das Objekt
     */
    ByteArrayPtr& operator=(const ByteArrayPtr& other);

    /**@brief Adresse des Speicherblocks als "const void *" auslesen
     *
     * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
     *
     * @return Adresse des Speicherblocks
     */
    operator const void*() const;

    /**@brief Adresse des Speicherblocks als "const char *" auslesen
     *
     * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
     *
     * @return Adresse des Speicherblocks
     */
    operator const char*() const;

    /**@brief Adresse des Speicherblocks als "const unsigned char *" auslesen
     *
     * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
     *
     * @return Adresse des Speicherblocks
     */
    operator const unsigned char*() const;

    /**@brief Speicherbereich als Hexwerte in einen String exportieren
     *
     * Der referenzierte Speicherbereich wird als String mit Hexadezimalwerten
     * exportiert, wobei jedes Byte als zwei Zeichen langer Hexadezimalwert
     * dargestellt wird.
     *
     * @return String mit Hexadezimal-Werten
     */
    String toHex() const;

    /**@brief Speicherbereich als Base64 in einen String exportieren
     *
     * Der referenzierte Speicherbereich wird im Base64-Format als String
     * exportiert.
     *
     * @return Inhalt des Speicherbereichs als Base64-String.
     */
    String toBase64() const;

    /**@brief Speicherbereich als String exportieren
     *
     * Der referenzierte Speicherbereich wird als String exportiert.
     *
     * @return Inhalt des Speicherbereichs als String.
     */
    String toString() const;

    /**@brief Speicherbereich als WideString exportieren
     *
     * Der referenzierte Speicherbereich wird als WideString exportiert.
     *
     * @return Inhalt des Speicherbereichs als WideString.
     */
    WideString toWideString() const;

    /**@brief Adresse des Speicherblocks auslesen
     *
     * Mit diesem Operator wird die Adresse des Speicherblocks ausgelesen
     *
     * @return Adresse des Speicherblocks
     */
    const char* toCharPtr() const;

    /**@brief Speicherbereich auslesen
     *
     * Mit dieser Funktion kann ein bestimmter Speicherbereich ausgelesen werden.
     * Der Speicherbereich wird als Pointer auf einen char-Array zurückgegeben.
     * Die Größe des Speicherbereichs wird in \p size zurückgegeben.
     *
     * @param[in] position Position innerhalb des referenzierten Speicherbereichs, ab der der neue Speicherbereich beginnt
     * @param[in] size Größe des neuen Speicherbereichs in Bytes
     * @return Pointer auf den neuen Speicherbereich
     */
    const char* map(size_t position, size_t size) const;

    /**@brief Speicherbereich kürzen
     *
     * Mit dieser Funktion kann der referenzierte Speicherbereich auf die Größe \p position gekürzt werden.
     * Der Speicherbereich selbst wird von der Klasse nicht verwaltet, sondern nur referenziert.
     *
     * @param[in] position Neue Größe des Speicherbereichs in Bytes
     */
    void truncate(size_t position);

    /**@brief CRC32-Prüfsumme berechnen
     *
     * Mit dieser Funktion wird die CRC32-Prüfsumme des referenzierten Speicherbereichs berechnet.
     *
     * @return CRC32-Prüfsumme
     */
    uint32_t crc32() const;

    /*!\brief Einzelnes Byte aus dem Speicherbereich auslesen
     *
     * \desc
     * Mit dem Operator [] kann ein bestimmtes Byte \p pos aus dem Speicherbereich
     * ausgelesen werden. Ist kein Speicher referenziert oder ist \p pos größer als
     * der Speicherblock, wird eine Exception ausgelöst.
     *
     * @param [in] pos Auszulesendes Byte, beginnend mit 0.
     * @return Wert der Speicherstelle
     * \exception OutOfBoundsException Diese Exception wird geworfen, wenn die mit
     * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
     * kein Speicher referenziert ist.
     */
    unsigned char operator[](size_t pos) const;

    /*!\brief Einzelnes Byte aus dem Speicherbereich referenzieren
     *
     * \desc
     * Mit dem Operator [] kann eine Referenz auf ein bestimmtes Byte \p pos aus dem Speicherbereich
     * erhalten werden. Dies ermöglicht Lese- und Schreibzugriff. Ist kein Speicher referenziert oder ist \p pos größer als
     * der Speicherblock, wird eine Exception ausgelöst.
     *
     * @param [in] pos Auszulesendes Byte, beginnend mit 0.
     * @return Referenz auf das Byte
     * \exception OutOfBoundsException Diese Exception wird geworfen, wenn die mit
     * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
     * kein Speicher referenziert ist.
     */
    unsigned char& operator[](size_t pos);

    /**@brief Einzelnes Byte aus dem Speicherbereich auslesen
     *
     * Mit dieser Funktion kann ein bestimmtes Byte \p pos aus dem Speicherbereich
     * ausgelesen werden. Ist kein Speicher referenziert oder ist \p pos größer als
     * der Speicherblock, wird eine Exception ausgelöst.
     *
     * @param [in] pos Auszulesendes Byte, beginnend mit 0.
     * @return Wert der Speicherstelle
     * \exception OutOfBoundsException Diese Exception wird geworfen, wenn die mit
     * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
     * kein Speicher referenziert ist.
     */
    unsigned char operator[](int pos) const;

    /**@brief Einzelnes Byte aus dem Speicherbereich referenzieren
     *
     * Mit dieser Funktion kann eine Referenz auf ein bestimmtes Byte \p pos aus dem Speicherbereich
     * erhalten werden. Dies ermöglicht Lese- und Schreibzugriff. Ist kein Speicher referenziert oder ist \p pos größer als
     * der Speicherblock, wird eine Exception ausgelöst.
     *
     * @param [in] pos Auszulesendes Byte, beginnend mit 0.
     * @return Referenz auf das Byte
     * \exception OutOfBoundsException Diese Exception wird geworfen, wenn die mit
     * \p pos angegebene Speicherstelle ausseralb des referenzierten Speichers liegt oder
     * kein Speicher referenziert ist.
     */
    unsigned char& operator[](int pos);

    /**@brief Einzelnes Byte im Speicherbereich setzen
     *
     * Mit dieser Funktion kann ein bestimmtes Byte \p pos im Speicherbereich auf den Wert \p value gesetzt werden.
     * Ist kein Speicher referenziert oder ist \p pos größer als der Speicherblock, wird eine Exception ausgelöst.
     *
     * @param [in] pos Zu setzendes Byte, beginnend mit 0.
     * @param [in] value Wert, auf den das Byte gesetzt werden soll.
     * \exception OutOfBoundsException Diese Exception wird geworfen, wenn die mit
     * \p pos angegebene Speicherstelle ausserhalb des referenzierten Speichers liegt oder
     * kein Speicher referenziert ist.
     */
    void set(size_t pos, unsigned char value);

    /**@brief Einzelnes Byte im Speicherbereich auslesen
     *
     * Mit dieser Funktion kann ein bestimmtes Byte \p pos aus dem Speicherbereich
     * ausgelesen werden. Ist kein Speicher referenziert oder ist \p pos größer als
     * der Speicherblock, wird eine Exception ausgelöst.
     *
     * @param [in] pos Auszulesendes Byte, beginnend mit 0.
     * @return Wert der Speicherstelle
     * \exception OutOfBoundsException Diese Exception wird geworfen, wenn die mit
     * \p pos angegebene Speicherstelle ausserhalb des referenzierten Speichers liegt oder
     * kein Speicher referenziert ist.
     */
    unsigned char get(size_t pos) const;

    /**@brief Hexdump des Speicherbereichs auf stdout ausgeben
     *
     * Mit dieser Funktion wird der referenzierte Speicherbereich als Hexdump auf stdout ausgegeben.
     */
    void hexDump() const;

    /**@brief Hexdump des Speicherbereichs auf stdout ausgeben
     *
     * Mit dieser Funktion wird der referenzierte Speicherbereich als Hexdump auf stdout ausgegeben.
     *
     * @param bytes Anzahl der Bytes, die ausgegeben werden sollen. Ist der Wert größer als die Größe des Speicherbereichs, wird nur der
     * Speicherbereich ausgegeben.
     */
    void hexDump(size_t bytes) const;

    /**@brief Hexdump des Speicherbereichs auf stdout ausgeben
     *
     * Mit dieser Funktion wird der referenzierte Speicherbereich als Hexdump auf stdout ausgegeben.
     *
     * @param offset Offset innerhalb des Speicherbereichs, ab dem die Ausgabe beginnen soll.
     * @param bytes Anzahl der Bytes, die ausgegeben werden sollen. Ist der Wert größer als die Größe des Speicherbereichs, wird nur der
     * Speicherbereich ab \p offset ausgegeben.
     */
    void hexDump(size_t offset, size_t bytes) const;

    /**@brief Speicher mit bestimmtem Wert füllen
     *
     * Diese Funktion füllt den gesamten verwalteten Speicher mit dem Bytewert \p value
     *
     * \param[in] value Bytewert
     */
    void memset(int value);

    /**@brief Speicher mit einem anderen Speicherbereich vergleichen
     *
     * Mit dieser Funktion kann der referenzierte Speicherbereich mit einem anderen Speicherbereich verglichen werden.
     * Der Vergleich erfolgt byteweise. Die Funktion liefert einen Wert kleiner 0, wenn der referenzierte Speicherbereich
     * kleiner als der mit \p other angegebene ist, einen Wert größer 0, wenn er größer ist und 0, wenn beide identisch sind.
     *
     * @param[in] other Referenz auf einen anderen Speicherbereich
     * @return Vergleichswert
     */
    int memcmp(const ByteArrayPtr& other) const;

    inline bool operator<(const ByteArrayPtr& other) const
    {
        return memcmp(other) < 0;
    }
    inline bool operator<=(const ByteArrayPtr& other) const
    {
        return memcmp(other) <= 0;
    }
    inline bool operator>(const ByteArrayPtr& other) const
    {
        return memcmp(other) > 0;
    }
    inline bool operator>=(const ByteArrayPtr& other) const
    {
        return memcmp(other) >= 0;
    }
    inline bool operator==(const ByteArrayPtr& other) const
    {
        return memcmp(other) == 0;
    }
    inline bool operator!=(const ByteArrayPtr& other) const
    {
        return memcmp(other) != 0;
    }
};
std::ostream& operator<<(std::ostream& s, const ByteArrayPtr& bap);

} // namespace ppl7

#endif // PPL7_TYPES_BYTEARRAYPTR_H_