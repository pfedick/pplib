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

#ifndef PPLIB_TYPES_VARIANT_H_
#define PPLIB_TYPES_VARIANT_H_

#include <utility>
namespace pplib
{

class String;
class WideString;
class Array;
class AssocArray;
class ByteArray;
class ByteArrayPtr;
class DateTime;
class Date;
class Time;
class TimeDelta;
class TimeZone;

/** @class Variant
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
 * - Date
 * - Time
 * - TimeDelta
 * - TimeZone
 */
class Variant
{
public:
    /// @brief Mögliche Datentypen, die in einem Variant gespeichert werden können
    enum DataType // TODO: Das sollte eine enum class werden, aber das würde die Kompatibilität zu älteren Versionen brechen
    {
        TYPE_UNKNOWN = 0,    /// @brief Unbekannter Datentyp
        TYPE_STRING = 4,     /// @brief Datentyp ist String
        TYPE_ASSOCARRAY = 5, /// @brief Datentyp ist AssocArray
        TYPE_BYTEARRAY = 6,  /// @brief Datentyp ist ByteArray
        // TYPE_POINTER = 7,     // obsolete, wird ab Version 8 nicht mehr unterstützt
        TYPE_WIDESTRING = 8,    /// @brief Datentyp ist WideString
        TYPE_ARRAY = 9,         /// @brief Datentyp ist Array
        TYPE_DATETIME = 10,     /// @brief Datentyp ist DateTime
        TYPE_BYTEARRAYPTR = 12, /// @brief Datentyp ist ByteArrayPtr
        TYPE_DATE = 13,         /// @brief Datentyp ist Date
        TYPE_TIME = 14,         /// @brief Datentyp ist Time
        TYPE_TIMEDELTA = 15,    /// @brief Datentyp ist TimeDelta
        TYPE_TIMEZONE = 16      /// @brief Datentyp ist TimeZone
    };

private:
    void* value = nullptr;     /// @brief Pointer auf den Inhalt des Datentyps
    DataType t = TYPE_UNKNOWN; /// @brief Variable, zum Speichern des Datentyps

public:
    /** @brief Konstruktor der Klasse
     *
     * Der Konstruktor initialisiert den Typ der Klasse mit Variant::TYPE_UNKNOWN.
     * Es ist aufgabe der abgeleiteten Klasse den korrekten Datentyp zu setzen.
     */
    Variant();

    /** @brief Destruktor
     *
     * Gibt den durch das Objekt belegten Speicher wieder frei.
     */
    ~Variant();

    /** @brief Copy-Konstruktor der Klasse
     *
     * Der Inhalt des anderen Variant-Objekts \p value wird kopiert.
     */
    Variant(const Variant& other);

    /** @brief Move-Konstruktor der Klasse
     *
     * Der Inhalt des anderen Variant-Objekts \p value wird übernommen.
     */
    Variant(Variant&& other);

    /** @brief Konstruktor mit Datentyp String
     *
     * Der Inhalt des Strings \p value wird kopiert.
     *
     * @param value
     */
    Variant(const String& value);

    /** @brief Move Konstruktor mit Datentyp String
     *
     * Der Inhalt des Strings \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(String&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp WideString
     *
     * Der Inhalt des WideStrings \p value wird kopiert.
     *
     * @param value
     */
    Variant(const WideString& value);

    /** @brief Move Konstruktor mit Datentyp WideString
     *
     * Der Inhalt des WideStrings \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(WideString&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp Array
     *
     * Der Inhalt des Arrays \p value wird kopiert.
     *
     * @param value
     */
    Variant(const Array& value);

    /** @brief Move Konstruktor mit Datentyp Array
     *
     * Der Inhalt des Arrays \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(Array&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp AssocArray
     *
     * Der Inhalt des AssocArrays \p value wird kopiert.
     *
     * @param value
     */
    Variant(const AssocArray& value);

    /** @brief Move Konstruktor mit Datentyp AssocArray
     *
     * Der Inhalt des AssocArrays \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(AssocArray&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp ByteArray
     *
     * Der Inhalt des ByteArrays \p value wird kopiert.
     *
     * @param value
     */
    Variant(const ByteArray& value);

    /** @brief Move Konstruktor mit Datentyp ByteArray
     *
     * Der Inhalt des ByteArrays \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(ByteArray&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp ByteArrayPtr
     *
     * Der Inhalt des ByteArrayPtrs \p value wird kopiert.
     *
     * @param value
     */
    Variant(const ByteArrayPtr& value);

    /** @brief Move Konstruktor mit Datentyp ByteArrayPtr
     *
     * Der Inhalt des ByteArrayPtrs \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(ByteArrayPtr&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp DateTime
     *
     * Der Inhalt des DateTime-Objekts \p value wird kopiert.
     *
     * @param value
     */
    Variant(const DateTime& value);

    /** @brief Move Konstruktor mit Datentyp DateTime
     *
     * Der Inhalt des DateTime-Objekts \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(DateTime&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp Date
     *
     * Der Inhalt des Date-Objekts \p value wird kopiert.
     *
     * @param value
     */
    Variant(const Date& value);

    /** @brief Move Konstruktor mit Datentyp Date
     *
     * Der Inhalt des Date-Objekts \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(Date&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp Time
     *
     * Der Inhalt des Time-Objekts \p value wird kopiert.
     *
     * @param value
     */
    Variant(const Time& value);

    /** @brief Move Konstruktor mit Datentyp Time
     *
     * Der Inhalt des Time-Objekts \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(Time&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp TimeDelta
     *
     * Der Inhalt des TimeDelta-Objekts \p value wird kopiert.
     *
     * @param value
     */
    Variant(const TimeDelta& value);

    /** @brief Move Konstruktor mit Datentyp TimeDelta
     *
     * Der Inhalt des TimeDelta-Objekts \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(TimeDelta&& value)
    {
        set(std::move(value));
    }

    /** @brief Konstruktor mit Datentyp TimeZone
     *
     * Der Inhalt des TimeZone-Objekts \p value wird kopiert.
     *
     * @param value
     */
    Variant(const TimeZone& value);

    /** @brief Move Konstruktor mit Datentyp TimeZone
     *
     * Der Inhalt des TimeZone-Objekts \p value wird übernommen.
     *
     * @param value
     */
    inline Variant(TimeZone&& value)
    {
        set(std::move(value));
    }

    /** @brief Inhalt des Objekts löschen
     *
     * Der im Objekte gespeicherte Datentyp wird gelöscht und sein Speicher
     * freigegeben.
     */
    void clear();

    /** @brief Wert eines anderen Variant kopieren
     *
     * Der Wert des anderen Variant \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const Variant& value);

    /** @brief Wert eines anderen Variant übernehmen
     *
     * Der Wert des anderen Variant \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(Variant&& value);

    /** @brief Wert eines Strings kopieren
     *
     * Der Wert des Strings \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const String& value);

    /** @brief Wert eines Strings übernehmen
     *
     * Der Wert des Strings \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(String&& value);

    /** @brief Wert eines WideStrings kopieren
     *
     * Der Wert des WideStrings \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const WideString& value);

    /** @brief Wert eines WideStrings übernehmen
     *
     * Der Wert des WideStrings \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(WideString&& value);

    /** @brief Wert eines Arrays kopieren
     *
     * Der Wert des Arrays \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const Array& value);

    /** @brief Wert eines Arrays übernehmen
     *
     * Der Wert des Arrays \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(Array&& value);

    /** @brief Wert eines AssocArrays kopieren
     *
     * Der Wert des AssocArrays \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const AssocArray& value);

    /** @brief Wert eines AssocArrays übernehmen
     *
     * Der Wert des AssocArrays \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(AssocArray&& value);

    /** @brief Wert eines ByteArrays kopieren
     *
     * Der Wert des ByteArrays \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const ByteArray& value);

    /** @brief Wert eines ByteArrays übernehmen
     *
     * Der Wert des ByteArrays \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(ByteArray&& value);

    /** @brief Wert eines ByteArrayPtrs kopieren
     *
     * Der Wert des ByteArrayPtrs \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const ByteArrayPtr& value);

    /** @brief Wert eines ByteArrayPtrs übernehmen
     *
     * Der Wert des ByteArrayPtrs \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(ByteArrayPtr&& value);

    /** @brief Wert eines DateTimes kopieren
     *
     * Der Wert des DateTimes \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const DateTime& value);

    /** @brief Wert eines DateTimes übernehmen
     *
     * Der Wert des DateTimes \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(DateTime&& value);

    /** @brief Wert eines Dates kopieren
     *
     * Der Wert des Dates \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const Date& value);

    /** @brief Wert eines Dates übernehmen
     *
     * Der Wert des Dates \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(Date&& value);

    /** @brief Wert eines Times kopieren
     *
     * Der Wert des Times \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const Time& value);

    /** @brief Wert eines Times übernehmen
     *
     * Der Wert des Times \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(Time&& value);

    /** @brief Wert eines TimeDeltas kopieren
     *
     * Der Wert des TimeDeltas \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const TimeDelta& value);

    /** @brief Wert eines TimeDeltas übernehmen
     *
     * Der Wert des TimeDeltas \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(TimeDelta&& value);

    /** @brief Wert eines TimeZones kopieren
     *
     * Der Wert des TimeZones \p value wird kopiert.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(const TimeZone& value);

    /** @brief Wert eines TimeZones übernehmen
     *
     * Der Wert des TimeZones \p value wird übernommen.
     *
     * @param value
     * @return Referenz auf dieses Objekt
     */
    Variant& set(TimeZone&& value);

    /** @brief Liefert den Datentyp des Objekts zurück
     *
     * Diese Funktion liefert den Datentyp des Objekts zurück.
     *
     * @return ID aus der Enumeration Variant::DataType
     */
    inline constexpr DataType type() const
    {
        return t;
    }

    /** @brief Prüft auf einen bestimmten Datentyp
     *
     * Prüft, ob es sich bei diesem Objekt um den Datentyp \p type handelt.
     *
     * @param type Zu überprüfender Typ. Wert aus der Enumerationr Variant::DataType
     * @return Liefert \c true zurück, wenn es sich um den angegebenen Datentyp \p type handelt,
     * sonst \c false.
     */
    inline bool isType(DataType type) const
    {
        return (this->t == type);
    }

    /** @brief Prüft, ob es sich um den Datentyp String handelt
     *
     * Prüft, ob es sich um den Datentyp String handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp String handelt, sonst \c false.
     */
    inline bool isString() const
    {
        return t == TYPE_STRING;
    }

    /** @brief Prüft, ob es sich um den Datentyp WideString handelt
     *
     * Prüft, ob es sich um den Datentyp WideString handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp WideString handelt, sonst \c false.
     */
    inline bool isWideString() const
    {
        return t == TYPE_WIDESTRING;
    }

    /** @brief Prüft, ob es sich um den Datentyp Array handelt
     *
     * Prüft, ob es sich um den Datentyp Array handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp Array handelt, sonst \c false.
     */
    inline bool isArray() const
    {
        return t == TYPE_ARRAY;
    }

    /** @brief Prüft, ob es sich um den Datentyp AssocArray handelt
     *
     * Prüft, ob es sich um den Datentyp AssocArray handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp AssocArray handelt, sonst \c false.
     */
    inline bool isAssocArray() const
    {
        return t == TYPE_ASSOCARRAY;
    }

    /** @brief Prüft, ob es sich um den Datentyp ByteArray handelt
     *
     * Prüft, ob es sich um den Datentyp ByteArray handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp ByteArray handelt, sonst \c false.
     */
    inline bool isByteArray() const
    {
        return t == TYPE_BYTEARRAY;
    }

    /** @brief Prüft, ob es sich um den Datentyp ByteArrayPtr handelt
     *
     * Prüft, ob es sich um den Datentyp ByteArrayPtr handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp ByteArrayPtr handelt, sonst \c false.
     */
    inline bool isByteArrayPtr() const
    {
        return t == TYPE_BYTEARRAYPTR;
    }

    /** @brief Prüft, ob es sich um den Datentyp DateTime handelt
     *
     * Prüft, ob es sich um den Datentyp DateTime handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp DateTime handelt, sonst \c false.
     */
    inline bool isDateTime() const
    {
        return t == TYPE_DATETIME;
    }

    /** @brief Prüft, ob es sich um den Datentyp Date handelt
     *
     * Prüft, ob es sich um den Datentyp Date handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp Date handelt, sonst \c false.
     */
    inline bool isDate() const
    {

        return t == TYPE_DATE;
    }

    /** @brief Prüft, ob es sich um den Datentyp Time handelt
     *
     * Prüft, ob es sich um den Datentyp Time handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp Time handelt, sonst \c false.
     */
    inline bool isTime() const
    {
        return t == TYPE_TIME;
    }

    /** @brief Prüft, ob es sich um den Datentyp TimeDelta handelt
     *
     * Prüft, ob es sich um den Datentyp TimeDelta handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp TimeDelta handelt, sonst \c false.
     */
    inline bool isTimeDelta() const
    {
        return t == TYPE_TIMEDELTA;
    }

    /** @brief Prüft, ob es sich um den Datentyp TimeZone handelt
     *
     * Prüft, ob es sich um den Datentyp TimeZone handelt
     *
     * @return Liefert \c true zurück, wenn es sich um den Datentyp TimeZone handelt, sonst \c false.
     */
    inline bool isTimeZone() const
    {
        return t == TYPE_TIMEZONE;
    }

    /** @brief Typkonvertierung zu: const String
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

    /** @brief Typkonvertierung zu: String
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

    /** @brief Typkonvertierung zu: const WideString
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

    /** @brief Typkonvertierung zu: WideString
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

    /** @brief Typkonvertierung zu: const Array
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

    /** @brief Typkonvertierung zu: Array
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

    /** @brief Typkonvertierung zu: const AssocArray
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

    /** @brief Typkonvertierung zu: AssocArray
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

    /** @brief Typkonvertierung zu: const ByteArray
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

    /** @brief Typkonvertierung zu: ByteArray
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

    /** @brief Typkonvertierung zu: const ByteArrayPtr
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

    /** @brief Typkonvertierung zu: ByteArrayPtr
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

    /** @brief Typkonvertierung zu: const DateTime
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

    /** @brief Typkonvertierung zu: DateTime
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

    /** @brief Typkonvertierung zu: const Date
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * Date zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf Date
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Date handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const Date& toDate() const;

    /** @brief Typkonvertierung zu: Date
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * Date zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf Date
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Date handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    Date& toDate();

    /** @brief Typkonvertierung zu: const Time
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * Time zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf Time
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Time handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const Time& toTime() const;

    /** @brief Typkonvertierung zu: Time
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * Time zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf Time
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Time handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    Time& toTime();

    /** @brief Typkonvertierung zu: const TimeDelta
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * TimeDelta zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf TimeDelta
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeDelta handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const TimeDelta& toTimeDelta() const;

    /** @brief Typkonvertierung zu: TimeDelta
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * TimeDelta zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf TimeDelta
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeDelta handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    TimeDelta& toTimeDelta();

    /** @brief Typkonvertierung zu: const TimeZone
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz auf den gespeicherten
     * TimeZone zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return const Referenz auf TimeZone
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeZone handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    const TimeZone& toTimeZone() const;

    /** @brief Typkonvertierung zu: TimeZone
     *
     * Der Aufruf dieser Funktion liefert eine Referenz auf den gespeicherten
     * TimeZone zurück, sofern der Variant diesen Datentyp enthält. Ist dies nicht der Fall,
     * wird eine Exception geworfen.
     *
     * @return Referenz auf TimeZone
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeZone handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    TimeZone& toTimeZone();

    /** @brief Typkonvertierung zu: String
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten Strings zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten Strings
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator String&()
    {
        return toString();
    }

    /** @brief Typkonvertierung zu: const String
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten Strings zurück, sofern der Variant diesen Datentyp
     * enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten Strings
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen String handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const String&() const
    {
        return toString();
    }

    /** @brief Typkonvertierung zu: WideString
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten WideStrings zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten WideStrings
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator WideString&()
    {
        return toWideString();
    }

    /** @brief Typkonvertierung zu: const WideString
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten WideStrings zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten WideStrings
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen WideString handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const WideString&() const
    {
        return toWideString();
    }

    /** @brief Typkonvertierung zu: Array
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten Arrays zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten Arrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Array handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator Array&()
    {
        return toArray();
    }

    /** @brief Typkonvertierung zu: const Array
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten Arrays zurück, sofern der Variant diesen Datentyp
     * enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten Arrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Array handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const Array&() const
    {
        return toArray();
    }

    /** @brief Typkonvertierung zu: AssocArray
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten AssocArrays zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten AssocArrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen AssocArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator AssocArray&()
    {
        return toAssocArray();
    }

    /** @brief Typkonvertierung zu: const AssocArray
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten AssocArrays zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten AssocArrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen AssocArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const AssocArray&() const
    {
        return toAssocArray();
    }

    /** @brief Typkonvertierung zu: ByteArray
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten ByteArrays zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten ByteArrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator ByteArray&()
    {
        return toByteArray();
    }

    /** @brief Typkonvertierung zu: const ByteArray
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten ByteArrays zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten ByteArrays
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArray handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const ByteArray&() const
    {
        return toByteArray();
    }

    /** @brief Typkonvertierung zu: ByteArrayPtr
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten ByteArrayPtrs zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten ByteArrayPtrs
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator ByteArrayPtr&()
    {
        return toByteArrayPtr();
    }

    /** @brief Typkonvertierung zu: const ByteArrayPtr
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten ByteArrayPtrs zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten ByteArrayPtrs
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen ByteArrayPtr handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const ByteArrayPtr&() const
    {
        return toByteArrayPtr();
    }

    /** @brief Typkonvertierung zu: DateTime
     *
     * Der Aufruf dieser Funktion liefert eine Referenz des gespeicherten DateTimes zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten DateTimes
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator DateTime&()
    {
        return toDateTime();
    }

    /** @brief Typkonvertierung zu: const DateTime
     *
     * Der Aufruf dieser Funktion liefert eine unveränderliche Referenz des gespeicherten DateTimes zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten DateTimes
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen DateTime handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const DateTime&() const
    {
        return toDateTime();
    }

    /** @brief Typkonvertierung zu: Date
     *
     * Der Aufruf dieser Operators liefert eine Referenz des gespeicherten Dates zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten Dates
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Date handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator Date&()
    {
        return toDate();
    }

    /** @brief Typkonvertierung zu: const Date
     *
     * Der Aufruf dieser Operators liefert eine unveränderliche Referenz des gespeicherten Dates zurück, sofern der Variant diesen Datentyp
     * enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten Dates
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Date handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const Date&() const
    {
        return toDate();
    }

    /** @brief Typkonvertierung zu: Time
     *
     * Der Aufruf dieser Operators liefert eine Referenz des gespeicherten Times zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten Times
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Time handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator Time&()
    {
        return toTime();
    }

    /** @brief Typkonvertierung zu: const Time
     *
     * Der Aufruf dieser Operators liefert eine unveränderliche Referenz des gespeicherten Times zurück, sofern der Variant diesen Datentyp
     * enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten Times
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen Time handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const Time&() const
    {
        return toTime();
    }

    /** @brief Typkonvertierung zu: TimeDelta
     *
     * Der Aufruf dieser Operators liefert eine Referenz des gespeicherten TimeDeltas zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten TimeDeltas
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeDelta handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator TimeDelta&()
    {
        return toTimeDelta();
    }

    /** @brief Typkonvertierung zu: const TimeDelta
     *
     * Der Aufruf dieser Operators liefert eine unveränderliche Referenz des gespeicherten TimeDeltas zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten TimeDeltas
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeDelta handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const TimeDelta&() const
    {
        return toTimeDelta();
    }

    /** @brief Typkonvertierung zu: TimeZone
     *
     * Der Aufruf dieser Operators liefert eine Referenz des gespeicherten TimeZones zurück, sofern der Variant diesen Datentyp enthält.
     * Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return Referenz des gespeicherten TimeZones
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeZone handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator TimeZone&()
    {
        return toTimeZone();
    }

    /** @brief Typkonvertierung zu: const TimeZone
     *
     * Der Aufruf dieser Operators liefert eine unveränderliche Referenz des gespeicherten TimeZones zurück, sofern der Variant diesen
     * Datentyp enthält. Ist dies nicht der Fall, wird eine Exception geworfen.
     *
     * @return const Referenz des gespeicherten TimeZones
     * @exception TypeConversionException: Wird geworfen, wenn es sich nicht um einen TimeZone handelt.
     * @exception EmptyDataException: Wird geworfen, wenn keine Daten in diesem Variant hinterlegt sind.
     */
    inline operator const TimeZone&() const
    {
        return toTimeZone();
    }

    /** @brief Zuweisungsoperator
     *
     * Der Aufruf dieses Operators weist dem Variant den Wert des übergebenen Variants zu.
     *
     * @param other Variant, dessen Wert zugewiesen werden soll.
     * @return Referenz auf das aktuelle Objekt.
     */
    inline Variant& operator=(const Variant& other)
    {
        set(other);
        return *this;
    }

    /** @brief Move-Zuweisungsoperator
     *
     * Der Aufruf dieses Operators weist dem Variant den Wert des übergebenen Variants zu.
     *
     * @param other Variant, dessen Wert zugewiesen werden soll.
     * @return Referenz auf das aktuelle Objekt.
     */
    inline Variant& operator=(Variant&& other)
    {
        if (this != &other) {
            clear();
            value = other.value;
            t = other.t;
            other.value = nullptr;
            other.t = TYPE_UNKNOWN;
        }
        return *this;
    }

    inline Variant& operator=(const String& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(String&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const WideString& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(WideString&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const Array& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(Array&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const AssocArray& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(AssocArray&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const ByteArray& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(ByteArray&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const ByteArrayPtr& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(ByteArrayPtr&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const DateTime& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(DateTime&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const Date& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(Date&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const Time& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(Time&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const TimeDelta& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(TimeDelta&& other)
    {
        set(std::move(other));
        return *this;
    }

    inline Variant& operator=(const TimeZone& other)
    {
        set(other);
        return *this;
    }

    inline Variant& operator=(TimeZone&& other)
    {
        set(std::move(other));
        return *this;
    }

    bool operator==(const Variant& other) const;
    bool operator!=(const Variant& other) const;
};

} // namespace pplib

#endif // PPLIB_TYPES_VARIANT_H_