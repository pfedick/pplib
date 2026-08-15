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

#ifndef PPLIB_TYPES_ARRAY_H_
#define PPLIB_TYPES_ARRAY_H_

#include <stdint.h>
#include <iterator>

#include "pplib/types/string.h"

namespace pplib
{

class String;

/** @class Array
 * @ingroup PPLGroupDataTypes
 * @brief Ein Array mit Strings
 *
 * Diese Klasse repräsentiert ein Array aus Strings, die über einen Index angesprochen werden
 * können. Die Zählung der Elemente beginnt dabei bei 0, das heisst das erste Element hat den
 * Index 0 (vergleichbar mit Arrays in C/C++).
 */
class Array
{
private:
    typedef struct
    {
        String* value;
    } ROW;

    /// Anzahl Elemente im Array
    size_t numElements;

    /// Maximale Anzahl Elemente im Array, für die bereits Speicher reserviert wurde
    size_t numCapacity;

    /** @brief Array mit den Datenelementen
     *
     * Diese Variable enthält einen Pointer auf die interne Datenstruktur, die die Werte
     * der einzelnen Elemente enthält. Die Größe des Speicherbereichs ergibt sich aus der
     * maximalen Anzahl Elemente im Array (Array::numCapacity) multipliziert mit der größe der
     * Datenstruktur für jedes Element.
     */
    void* rows;

public:
    class Iterator
    {
        friend class Array;

    private:
        size_t pos;

    public:
        Iterator();
    };

    //! @name Konstruktoren und Destruktor
    //@{
    Array();
    Array(const Array& other);
    Array(const String& str, const String& delimiter = String("\n"), size_t limit = 0, bool skipemptylines = false);
    ~Array();
    //@}

    //! @name Elemente hinzufügen/verändern
    //@{
    void add(const String& value);
    void add(const String& value, size_t size);
    void add(const char* value, size_t size);
    void add(const Array& other);
    void addf(const char* fmt, ...);
    void copy(const Array& other);
    void set(size_t index, const String& value);
    void setf(size_t index, const char* fmt, ...);
    void insert(size_t index, const String& value);
    void insert(size_t index, const Array& other);
    void insertf(size_t index, const char* fmt, ...);
    Array& fromArgs(int argc, const char** argv);
    Array& fromArgs(const String& args);
    Array& explode(const String& text, const String& delimiter = "\n", size_t limit = 0, bool skipemptylines = false);
    //@}

    //! @name Elemente löschen
    //@{
    void clear();
    String erase(size_t index);
    String shift();
    String pop();
    //@}

    //! @name Elemente auslesen
    //@{
    const String& get(ssize_t index) const;
    const String& getRandom() const;
    String& get(ssize_t index);
    String& getRandom();
    String getRest(size_t index, const String& delimiter = " ");
    const char* getPtr(ssize_t index) const;
    const char* getRandomPtr() const;
    void reset(Iterator& it) const;
    const String& getFirst(Iterator& it) const;
    const String& getNext(Iterator& it) const;
    String implode(const String& delimiter = "\n") const;
    //@}

    //! @name Sonstiges
    //@{
    size_t count() const;
    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    void reserve(size_t size);
    void list(const String& prefix = String()) const;
    void sort();
    void sortReverse();
    void sortUnique();
    void makeUnique();

    size_t indexOf(const String& search);
    bool has(const String& search);
    //@}

    //! @name Operatoren
    //@{
    String& operator[](ssize_t index);
    const String& operator[](ssize_t index) const;
    Array& operator=(const Array& other);
    Array& operator+=(const Array& other);
    bool operator==(const Array& other) const;
    bool operator!=(const Array& other) const;
    //@}

    //! @name Iteratoren
    //@{
    class ptr_iterator
    {
        void* ptr;

    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef String value_type;
        typedef std::ptrdiff_t difference_type;
        typedef String* pointer;
        typedef String& reference;
        ptr_iterator(void* p)
            : ptr(p)
        {
        }
        reference operator*() const
        {
            return *((*(ROW*)ptr).value);
        }
        pointer operator->() const
        {
            return (*(ROW*)ptr).value;
        }
        ptr_iterator& operator++()
        {
            ptr = (ROW*)ptr + 1;
            return *this;
        }
        ptr_iterator operator++(int)
        {
            ptr_iterator tmp = *this;
            ptr = (ROW*)ptr + 1;
            return tmp;
        }
        bool operator==(const ptr_iterator& other) const
        {
            return ptr == other.ptr;
        }
        bool operator!=(const ptr_iterator& other) const
        {
            return ptr != other.ptr;
        }
    };

    typedef ptr_iterator iterator;
    typedef const ptr_iterator const_iterator;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    //@}
};

Array operator+(const Array& a1, const Array& a2);

} // namespace pplib

#endif /* PPLIB_TYPES_ARRAY_H_ */