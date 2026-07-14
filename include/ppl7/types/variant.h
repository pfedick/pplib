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

#ifndef PPL7_TYPES_VARIANT_H_
#define PPL7_TYPES_VARIANT_H_
namespace ppl7
{

class String;
class WideString;
class Array;
class AssocArray;
class ByteArray;
class ByteArrayPtr;
class DateTime;
class Pointer;

/**@class Variant
 * @ingroup PPLGroupDataTypes
 * @brief Flexibler Datentyp, der verschiedene Datentypen aufnehmen kann
 *
 * Dieser Datentyp kann andere Datentypen aufnehmen. Unterstützt werden:
 *
 * - String
 * - WideString
 * - Array
 * - AssocArray
 * - ByteArray
 * - ByteArrayPtr
 * - DateTime
 *
 */
class Variant
{
public:
    /// @brief Mögliche Datentypen, die in einem Variant gespeichert werden können
    enum DataType // TODO: Das sollte eine enum class werden, aber das würde die Kompatibilität zu älteren Versionen brechen
    {
        TYPE_UNKNOWN = 0,      /// @brief Unbekannter Datentyp
        TYPE_STRING = 4,       /// @brief Datentyp ist String
        TYPE_ASSOCARRAY = 5,   /// @brief Datentyp ist AssocArray
        TYPE_BYTEARRAY = 6,    /// @brief Datentyp ist ByteArray
        TYPE_POINTER = 7,      /// @brief Datentyp ist Pointer
        TYPE_WIDESTRING = 8,   /// @brief Datentyp ist WideString
        TYPE_ARRAY = 9,        /// @brief Datentyp ist Array
        TYPE_DATETIME = 10,    /// @brief Datentyp ist DateTime
        TYPE_BYTEARRAYPTR = 12 /// @brief Datentyp ist ByteArrayPtr
    };

private:
    void* value; /// @brief Pointer auf den Inhalt des Datentyps
    DataType t;  /// @brief Variable, zum Speichern des Datentyps

public:
    /**@brief Konstruktor der Klasse
     *
     * Der Konstruktor initialisiert den Typ der Klasse mit Variant::TYPE_UNKNOWN.
     * Es ist aufgabe der abgeleiteten Klasse den korrekten Datentyp zu setzen.
     */
    Variant();

    /**@brief Destruktor
     *
     * Gibt den durch das Objekt belegten Speicher wieder frei.
     */
    ~Variant();

    /**@brief Copy-Konstruktor der Klasse
     *
     * Der Inhalt des anderen Variant-Objekts \p value wird kopiert.
     */
    Variant(const Variant& other);

    /**@brief Move-Konstruktor der Klasse
     *
     * Der Inhalt des anderen Variant-Objekts \p value wird kopiert.
     */
    Variant(Variant&& other);

    /**@brief Konstruktor mit Datentyp String
     *
     * Der Inhalt des Strings \p value wird kopiert.
     *
     * @param value
     */
    Variant(const String& value);

    /**@brief Konstruktor mit Datentyp WideString
     *
     * Der Inhalt des WideStrings \p value wird kopiert.
     *
     * @param value
     */
    Variant(const WideString& value);

    /**@brief Konstruktor mit Datentyp Array
     *
     * Der Inhalt des Arrays \p value wird kopiert.
     *
     * @param value
     */
    Variant(const Array& value);

    /**@brief Konstruktor mit Datentyp AssocArray
     *
     * Der Inhalt des AssocArrays \p value wird kopiert.
     *
     * @param value
     */
    Variant(const AssocArray& value);

    /**@brief Konstruktor mit Datentyp ByteArray
     *
     * Der Inhalt des ByteArrays \p value wird kopiert.
     *
     * @param value
     */
    Variant(const ByteArray& value);

    /**@brief Konstruktor mit Datentyp ByteArrayPtr
     *
     * Der Inhalt des ByteArrayPtrs \p value wird kopiert.
     *
     * @param value
     */
    Variant(const ByteArrayPtr& value);

    /**@brief Konstruktor mit Datentyp DateTime
     *
     * Der Inhalt des DateTime-Objekts \p value wird kopiert.
     *
     * @param value
     */
    Variant(const DateTime& value);

    /**@brief Konstruktor mit Datentyp Pointer
     *
     * Der Inhalt des Pointer-Objekts \p value wird kopiert.
     *
     * @param value
     * @deprecated Pointer ist veraltet und wird demnächst entfernt.
     */
    [[deprecated]] Variant(const Pointer& value);

    /**@brief Inhalt des Objekts löschen
     *
     * Der im Objekte gespeicherte Datentyp wird gelöscht und sein Speicher
     * freigegeben.
     */
    void clear();

    /**@brief Wert eines anderen Variant kopieren
     *
     * Der Wert des anderen Variant \p value wird kopiert.
     *
     * \param value
     */
    void set(const Variant& value);

    /**@brief Wert eines Strings kopieren
     *
     * Der Wert des Strings \p value wird kopiert.
     *
     * \param value
     */
    void set(const String& value);

    /**@brief Wert eines WideStrings kopieren
     *
     * Der Wert des WideStrings \p value wird kopiert.
     *
     * \param value
     */
    void set(const WideString& value);

    /**@brief Wert eines Arrays kopieren
     *
     * Der Wert des Arrays \p value wird kopiert.
     *
     * \param value
     */
    void set(const Array& value);

    /**@brief Wert eines AssocArrays kopieren
     *
     * Der Wert des AssocArrays \p value wird kopiert.
     *
     * \param value
     */
    void set(const AssocArray& value);

    /**@brief Wert eines ByteArrays kopieren
     *
     * Der Wert des ByteArrays \p value wird kopiert.
     *
     * \param value
     */
    void set(const ByteArray& value);

    /**@brief Wert eines ByteArrayPtrs kopieren
     *
     * Der Wert des ByteArrayPtrs \p value wird kopiert.
     *
     * \param value
     */
    void set(const ByteArrayPtr& value);

    /**@brief Wert eines DateTimes kopieren
     *
     * Der Wert des DateTimes \p value wird kopiert.
     *
     * \param value
     */
    void set(const DateTime& value);

    /**@brief Wert eines Pointers kopieren
     *
     * Der Wert des Pointers \p value wird kopiert.
     *
     * @param value
     * @deprecated Pointer ist veraltet und wird demnächst entfernt.
     */
    [[deprecated("Use a different pointer type instead.")]]
    void set(const Pointer& value);

    /**@brief Liefert den Datentyp des Objekts zurück
     *
     * Diese Funktion liefert den Datentyp des Objekts zurück.
     *
     * @return ID aus der Enumeration Variant::DataType
     */
    inline constexpr DataType type() const
    {
        return t;
    }

    /**@brief Prüft auf einen bestimmten Datentyp
     *
     * Prüft, ob es sich bei diesem Objekt um den Datentyp \p type handelt.
     *
     * @param type Zu überprüfender Typ. Wert aus der Enumerationr Variant::DataType
     * @return Liefert \c true zurück, wenn es sich um den angegebenen Datentyp \p type handelt,
     * sonst \c false.
     */
    bool isType(DataType type) const;

    /**@brief Prüft, ob es sich um den Datentyp String handelt
     *
     * Prüft, ob es sich um den Datentyp String handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp String handelt, sonst \c false.
     */
    bool isString() const;

    /**@brief Prüft, ob es sich um den Datentyp WideString handelt
     *
     * Prüft, ob es sich um den Datentyp WideString handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp WideString handelt, sonst \c false.
     */
    bool isWideString() const;

    /**@brief Prüft, ob es sich um den Datentyp Array handelt
     *
     * Prüft, ob es sich um den Datentyp Array handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp Array handelt, sonst \c false.
     */
    bool isArray() const;

    /**@brief Prüft, ob es sich um den Datentyp AssocArray handelt
     *
     * Prüft, ob es sich um den Datentyp AssocArray handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp AssocArray handelt, sonst \c false.
     */
    bool isAssocArray() const;

    /**@brief Prüft, ob es sich um den Datentyp ByteArray handelt
     *
     * Prüft, ob es sich um den Datentyp ByteArray handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp ByteArray handelt, sonst \c false.
     */
    bool isByteArray() const;

    /**@brief Prüft, ob es sich um den Datentyp ByteArrayPtr handelt
     *
     * Prüft, ob es sich um den Datentyp ByteArrayPtr handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp ByteArrayPtr handelt, sonst \c false.
     */
    bool isByteArrayPtr() const;

    /**@brief Prüft, ob es sich um den Datentyp DateTime handelt
     *
     * Prüft, ob es sich um den Datentyp DateTime handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp DateTime handelt, sonst \c false.
     */
    bool isDateTime() const;

    /**@brief Prüft, ob es sich um den Datentyp Pointer handelt
     *
     * Prüft, ob es sich um den Datentyp Pointer handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp Pointer handelt, sonst \c false.
     * @deprecated Pointer ist veraltet und wird demnächst entfernt.
     */
    [[deprecated("Use a different pointer type instead.")]]
    bool isPointer() const;

    /**@brief Typkonvertierung zu: const String
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * String zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf String
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const String& toString() const;

    /**@brief Typkonvertierung zu: String
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * String zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf String
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    String& toString();

    /**@brief Typkonvertierung zu: const WideString
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * WideString zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf WideString
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const WideString& toWideString() const;

    /**@brief Typkonvertierung zu: WideString
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * WideString zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf WideString
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    WideString& toWideString();

    /**@brief Typkonvertierung zu: const Array
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * Array zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf Array
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Array handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const Array& toArray() const;

    /**@brief Typkonvertierung zu: Array
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * Array zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf Array
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Array handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    Array& toArray();

    /**@brief Typkonvertierung zu: const AssocArray
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * AssocArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf AssocArray
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen AssocArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const AssocArray& toAssocArray() const;

    /**@brief Typkonvertierung zu: AssocArray
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * AssocArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf AssocArray
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen AssocArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    AssocArray& toAssocArray();

    /**@brief Typkonvertierung zu: const ByteArray
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * ByteArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf ByteArray
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const ByteArray& toByteArray() const;

    /**@brief Typkonvertierung zu: ByteArray
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * ByteArray zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf ByteArray
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    ByteArray& toByteArray();

    /**@brief Typkonvertierung zu: const ByteArrayPtr
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * ByteArrayPtr zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf ByteArrayPtr
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const ByteArrayPtr& toByteArrayPtr() const;

    /**@brief Typkonvertierung zu: ByteArrayPtr
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * ByteArrayPtr zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf ByteArrayPtr
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    ByteArrayPtr& toByteArrayPtr();

    /**@brief Typkonvertierung zu: const DateTime
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * DateTime zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf DateTime
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const DateTime& toDateTime() const;

    /**@brief Typkonvertierung zu: DateTime
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * DateTime zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf DateTime
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    DateTime& toDateTime();

    /**@brief Typkonvertierung zu: const Pointer
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * Pointer zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf Pointer
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Pointer handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     * @deprecated Pointer ist veraltet und wird demnächst entfernt.
     */
    [[deprecated("Use a different pointer type instead.")]]
    const Pointer& toPointer() const;

    /**@brief Typkonvertierung zu: Pointer
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * Pointer zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf Pointer
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Pointer handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     * @deprecated Pointer ist veraltet und wird demnächst entfernt.
     */
    [[deprecated("Use a different pointer type instead.")]]
    Pointer& toPointer();

    /**@brief Typkonvertierung zu: String
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten Strings zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten Strings
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator String() const;

    /**@brief Typkonvertierung zu: WideString
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten WideStrings zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten WideStrings
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator WideString() const;

    /**@brief Typkonvertierung zu: Array
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten Arrays zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten Arrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Array handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator Array() const;

    /**@brief Typkonvertierung zu: AssocArray
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten AssocArrays zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten AssocArrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen AssocArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator AssocArray() const;

    /**@brief Typkonvertierung zu: ByteArray
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten ByteArrays zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten ByteArrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator ByteArray() const;

    /**@brief Typkonvertierung zu: ByteArrayPtr
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten ByteArrayPtrs zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten ByteArrayPtrs
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator ByteArrayPtr() const;

    /**@brief Typkonvertierung zu: DateTime
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten DateTimes zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten DateTimes
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    operator DateTime() const;

    /**@brief Typkonvertierung zu: Pointer
     *
     * Der Aufruf dieser Funktion liefert eine Kopie des gespeicherten Pointers zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Kopie des gespeicherten Pointers
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Pointer handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     * @deprecated Pointer ist veraltet und wird demnächst entfernt.
     */
    [[deprecated("Use a different pointer type instead.")]]
    operator Pointer() const;

    Variant& operator=(const Variant& other);
    Variant& operator=(const String& other);
    Variant& operator=(const WideString& other);
    Variant& operator=(const Array& other);
    Variant& operator=(const AssocArray& other);
    Variant& operator=(const ByteArray& other);
    Variant& operator=(const ByteArrayPtr& other);
    Variant& operator=(const DateTime& other);

    [[deprecated("Use a different pointer type instead.")]] Variant& operator=(const Pointer& other);

    bool operator==(const Variant& other) const;
    bool operator!=(const Variant& other) const;
};

} // namespace ppl7

#endif // PPL7_TYPES_VARIANT_H_