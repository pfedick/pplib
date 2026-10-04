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

#ifndef PPLIB_TYPES_VARIANTARRAY_H_
#define PPLIB_TYPES_VARIANTARRAY_H_

#include <stdint.h>
#include <iterator>
#include <vector>

#include "pplib/types/string.h"
#include "pplib/types/array.h"
#include "pplib/types/variant.h"

namespace pplib
{

/** @class VariantArray
 * @ingroup PPLGroupDataTypes
 * @brief Ein Array mit beliebigen Datentypen (Variant).
 *
 * Diese Klasse repräsentiert ein Array aus Variants, also beliebigen anderen Datentypen,
 * die über einen Index angesprochen werden
 * können. Die Zählung der Elemente beginnt dabei bei 0, das heisst das erste Element hat den
 * Index 0 (vergleichbar mit Arrays in C/C++).
 *
 * Beim Einfügen oder Lesen von Elementen können auch negative Indizes verwendet werden,
 * um vom Ende des Arrays zu zählen. Bei einem Array mit 10 Elementen entspricht der Index
 * -1 dem letzten Element, -2 dem vorletzten Element usw.
 *
 * Ein Array kann maximal \c SSIZE_MAX Elemente enthalten. Auf 32-Bit-Systemen entspricht
 * dies typischerweise 2^31-1 Elementen, auf 64-Bit-Systemen 2^63-1 Elementen.
 */
class VariantArray
{
private:
    std::vector<Variant> elements;

public:
    static constexpr ssize_t npos = static_cast<ssize_t>(-1); // Ergebnis von find, wenn nichts gefunden wurde

    VariantArray() = default;
    ~VariantArray() = default;

    VariantArray(const VariantArray& other) = default;
    VariantArray& operator=(const VariantArray& other) = default;

    VariantArray(VariantArray&& other) noexcept = default;
    VariantArray& operator=(VariantArray&& other) noexcept = default;

    VariantArray(const Array& other);
    VariantArray(Array&& other);
    VariantArray& operator=(const Array& other);
    VariantArray& operator=(Array&& other);

    /** @brief Alle Elemente aus dem Array löschen
     *
     * Diese Methode entfernt alle Elemente aus dem Array.
     */
    void clear()
    {
        elements.clear();
    }

    /** @brief Anzahl der Elemente im Array
     *
     * @return Anzahl der Elemente im Array
     */
    std::size_t size() const
    {
        return elements.size();
    }

    /** @brief Kapazität des Arrays
     *
     * @return Kapazität des Arrays
     */
    std::size_t capacity() const
    {
        return elements.capacity();
    }

    /** @brief Kapazität des Arrays auf mindestens \p newCapacity erhöhen
     *
     * @param newCapacity Neue Kapazität des Arrays
     */
    void reserve(std::size_t newCapacity);

    /** @brief Element zum Array hinzufügen
     *
     * Fügt ein neues Element am Ende des Arrays hinzu.
     *
     * @param value Das hinzuzufügende Element
     */
    void add(Variant value);

    /** @brief Element am Ende des Arrays anhängen
     *
     * @param value Das hinzuzufügende Element
     * @note Diese Methode ist ein Alias für \c add().
     */
    inline void append(Variant value)
    {
        add(std::move(value));
    }

    /** @brief Array an das aktuelle Array anhängen
     *
     * Fügt alle Elemente des übergebenen Arrays am Ende des aktuellen Arrays hinzu.
     * Es wird eine Kopie der Elemente erstellt.
     *
     * @param other Das anzuhängende Array
     */
    void extend(const VariantArray& other);

    /** @brief Array an das aktuelle Array anhängen (Move)
     *
     * Fügt alle Elemente des übergebenen Arrays am Ende des aktuellen Arrays hinzu, indem die Ressourcen der übergebenen Elemente
     * übernommen werden.
     *
     * @param other Das anzuhängende Array
     */
    void extend(VariantArray&& other);

    /** @brief Elemente eines Arrays hinzufügen
     *
     * Fügt alle Elemente des übergebenen Arrays am Ende des aktuellen Arrays hinzu.
     * Es wird eine Kopie der Elemente erstellt.
     *
     * @param other Das anzuhängende Array
     */
    void extend(const Array& other);

    /** @brief Elemente eines Arrays hinzufügen (Move)
     *
     * Fügt alle Elemente des übergebenen Arrays am Ende des aktuellen Arrays hinzu, indem die Ressourcen der übergebenen Elemente
     * übernommen werden.
     *
     * @param other Das anzuhängende Array
     */
    void extend(Array&& other);

    /** @brief Element an einer bestimmten Position im Array abrufen
     *
     * @param index Position des abzurufenden Elements. Bei einem nagativen Wert wird vom Ende des Arrays gezählt.
     * @return Referenz auf das Element an der angegebenen Position
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    Variant& get(ssize_t index);

    /** @brief Element an einer bestimmten Position im Array abrufen (const)
     *
     * @param index Position des abzurufenden Elements. Bei einem nagativen Wert wird vom Ende des Arrays gezählt.
     * @return Referenz auf das Element an der angegebenen Position
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    const Variant& get(ssize_t index) const;

    /** @brief Element an einer bestimmten Position im Array abrufen (Operator[])
     *
     * @param index Position des abzurufenden Elements. Bei einem nagativen Wert wird vom Ende des Arrays gezählt.
     * @return Referenz auf das Element an der angegebenen Position
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    Variant& operator[](ssize_t index)
    {
        return get(index);
    }

    /** @brief Element an einer bestimmten Position im Array abrufen (Operator[] const)
     *
     * @param index Position des abzurufenden Elements. Bei einem nagativen Wert wird vom Ende des Arrays gezählt.
     * @return Referenz auf das Element an der angegebenen Position
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    const Variant& operator[](ssize_t index) const
    {
        return get(index);
    }

    /** @brief Element an einer bestimmten Position im Array setzen
     *
     * @param index Position des zu setzenden Elements. Bei einem nagativen Wert wird vom Ende des Arrays gezählt.
     * @param value Neuer Wert des Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    void set(ssize_t index, Variant value);

    /** @brief Element an einer bestimmten Position im Array einfügen
     *
     * Diese Methode fügt ein neues Element an der angegebenen Position im Array ein.
     * Alle nachfolgenden Elemente werden um eins nach hinten verschoben.
     *
     * @param index Position, an der das Element eingefügt werden soll. Bei einem nagativen Wert wird vom Ende des Arrays gezählt.
     * @param value Wert des einzufügenden Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    void insert(ssize_t index, Variant value);

    /** @brief Prüfen, ob das Array leer ist
     *
     * @return \c true, wenn das Array keine Elemente enthält, sonst \c false
     */
    bool isEmpty() const
    {
        return elements.empty();
    }

    /** @brief Prüfen, ob das Array leer ist (Alternative zu isEmpty)
     *
     * @return \c true, wenn das Array keine Elemente enthält, sonst \c false
     */
    bool empty() const
    {
        return elements.empty();
    }

    /** @brief Element löschen
     *
     * Das Element an Position \p index wird gelöscht. Alle nachfolgenden Elemente werden um eins nach vorne verschoben.
     * Ist \p index größer als die Anzahl Elemente des Arrays, wird eine Exception geworfen.
     *
     * @param index Position des zu löschenden Elements
     * @return Wert des gelöschten Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn \p index größer als die Anzahl Elemente des Arrays ist
     */
    Variant erase(ssize_t index);

    /** @brief Letztes Element aus dem Array entfernen und zurückgeben
     *
     * Diese Methode entfernt das letzte Element aus dem Array und gibt dessen Wert zurück.
     *
     * @return Wert des entfernten Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn das Array leer ist
     */
    Variant pop();

    /** @brief Erstes Element aus dem Array entfernen und zurückgeben
     *
     * Diese Methode entfernt das erste Element aus dem Array und gibt dessen Wert zurück.
     *
     * @return Wert des entfernten Elements
     * @exception OutOfBoundsException: Wird geworfen, wenn das Array leer ist
     */
    Variant shift();

    /** @brief Prüfen, ob das Array ein bestimmtes Element enthält
     *
     * @param value Wert, nach dem gesucht werden soll
     * @return \c true, wenn das Array das Element enthält, sonst \c false
     */
    bool has(const Variant& value) const;

    /** @brief Index eines bestimmten Elements im Array ermitteln
     *
     * @param value Wert, dessen Index ermittelt werden soll
     * @return Index des Elements, oder \c -1, wenn das Element nicht gefunden wurde
     */
    ssize_t indexOf(const Variant& value) const;

    /** @brief Konvertieren des VariantArrays in ein Array
     *
     * @param strict Wenn \c true (Default), wird eine strikte Konvertierung durchgeführt. Elemente,
     * die nicht konvertiert werden können, führen zu einer Exception.
     *
     * Es ist garantiert, dass die Indizes der Elemente im resultierenden Array mit denen im VariantArray übereinstimmen.
     * Wenn \p strict auf \c false gesetzt ist, werden Elemente, die nicht konvertiert werden können, als leere Strings behandelt.
     *
     * @return Ein Array, das die gleichen Elemente wie das VariantArray enthält
     */
    Array toArray(bool strict = true) const;

    size_t exportBinary(void* buffer, size_t buffersize) const;
    size_t importBinary(const void* buffer, size_t buffersize);
    size_t binarySize() const;
    ByteArray exportBinary() const;
    void importBinary(const ByteArrayPtr& buffer);

    VariantArray& operator+=(const VariantArray& other)
    {
        extend(other);
        return *this;
    }

    VariantArray& operator+=(VariantArray&& other) noexcept
    {
        extend(std::move(other));
        return *this;
    }

    inline bool operator==(const VariantArray& other) const
    {
        return elements == other.elements;
    }

    inline bool operator!=(const VariantArray& other) const
    {
        return !(*this == other);
    }

    typedef std::vector<Variant>::iterator iterator;
    typedef std::vector<Variant>::const_iterator const_iterator;
    typedef std::vector<Variant>::reverse_iterator reverse_iterator;
    typedef std::vector<Variant>::const_reverse_iterator const_reverse_iterator;

    iterator begin() noexcept
    {
        return elements.begin();
    }
    iterator end() noexcept
    {
        return elements.end();
    }
    const_iterator begin() const noexcept
    {
        return elements.begin();
    }
    const_iterator end() const noexcept
    {
        return elements.end();
    }
    reverse_iterator rbegin() noexcept
    {
        return elements.rbegin();
    }
    reverse_iterator rend() noexcept
    {
        return elements.rend();
    }
    const_reverse_iterator rbegin() const noexcept
    {
        return elements.rbegin();
    }
    const_reverse_iterator rend() const noexcept
    {
        return elements.rend();
    }
};

} // namespace pplib

#endif // PPLIB_TYPES_VARIANTARRAY_H_