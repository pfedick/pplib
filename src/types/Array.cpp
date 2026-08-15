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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <set>

#include <pplib/types/array.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

namespace pplib
{

static String EmptyString;

Array::Array()
{
    numElements = 0;
    numCapacity = 0;
    rows = NULL;
}

Array::Array(const Array& other)
{
    numElements = 0;
    numCapacity = 0;
    rows = NULL;
    add(other);
}

Array::Array(Array&& other)
{
    numElements = other.numElements;
    numCapacity = other.numCapacity;
    rows = other.rows;
    other.numElements = 0;
    other.numCapacity = 0;
    other.rows = NULL;
}

Array::Array(const String& str, const String& delimiter, size_t limit, bool skipemptylines)
{
    numElements = 0;
    numCapacity = 0;
    rows = NULL;
    explode(str, delimiter, limit, skipemptylines);
}

Array::~Array()
{
    clear();
}

void Array::clear()
{
    ROW* r = (ROW*)rows;
    if (r) {
        for (size_t i = 0; i < numCapacity; i++) {
            if (r[i].value) {
                delete (r[i].value);
                r[i].value = NULL;
            }
        }
        free(rows);
    }
    rows = NULL;
    numCapacity = 0;
    numElements = 0;
}

void Array::copy(const Array& other)
{
    clear();
    reserve(other.numElements);
    ROW* r_other = (ROW*)other.rows;
    ROW* r = (ROW*)rows;
    for (size_t i = 0; i < other.numElements; i++) {
        if (r_other[i].value != NULL) {
            if (r[i].value == NULL) {
                r[i].value = new String;
                if (!r[i].value) throw OutOfMemoryException();
            }
            r[i].value->set(*r_other[i].value);
        }
    }
    numElements = other.numElements;
}

void Array::add(const Array& other)
{
    ROW* r_other = (ROW*)other.rows;
    size_t first = numElements;
    reserve(numElements + other.numElements);
    ROW* r = (ROW*)rows;
    for (size_t i = 0; i < other.numElements; i++) {
        if (r_other[i].value != NULL) {
            if (r[first + i].value == NULL) {
                r[first + i].value = new String;
                if (!r[first + i].value) throw OutOfMemoryException();
            }
            r[first + i].value->set(*r_other[i].value);
        }
    }
    numElements += other.numElements;
}

void Array::add(const String& value)
{
    set(numElements, value);
}

void Array::add(const String& value, size_t size)
{
    String str;
    str.set(value, size);
    set(numElements, str);
}

void Array::addf(const char* fmt, ...)
{
    String value;
    va_list args;
    va_start(args, fmt);
    value.vasprintf(fmt, args);
    va_end(args);
    set(numElements, value);
}

void Array::set(size_t index, const String& value)
{
    ROW* r;
    if (index >= numCapacity) {
        // Array muss vergroessert werden
        size_t newsize = numCapacity / 3;
        if (newsize < 10) newsize = 10;
        if (newsize < index + 10) newsize = index + 10;
        reserve(newsize);
    }
    r = (ROW*)rows;
    if (index >= numElements) numElements = index + 1;
    if (value.notEmpty()) {
        if (r[index].value == NULL) {
            r[index].value = new String;
            if (!r[index].value) throw OutOfMemoryException();
        }
        r[index].value->set(value);
    } else {
        if (r[index].value != NULL) {
            delete r[index].value;
            r[index].value = NULL;
        }
    }
}

void Array::setf(size_t index, const char* fmt, ...)
{
    String value;
    va_list args;
    va_start(args, fmt);
    value.vasprintf(fmt, args);
    va_end(args);
    set(index, value);
}

void Array::insert(size_t index, const String& value)
{
    if (index >= numElements) {
        set(index, value);
        return;
    }
    ROW* r = (ROW*)rows;
    // Zunächst sorgen wir dafür, dass im Array genug Platz ist
    reserve(numElements + 2);
    // Nun verschieben wir alle Elemente ab Position index um eins nach hinten
    if (numElements > index) {
        for (size_t i = numElements; i > index; i--) {
            r[i].value = r[i - 1].value;
        }
        numElements++;
        r[index].value = NULL;
    }
    // Den neuen Wert einfügen
    set(index, value);
}

void Array::insert(size_t index, const Array& other)
{
    if (other.numElements == 0) return; // Anderes Array ist leer
    // Zunächst sorgen wir dafür, dass im Array genug Platz ist
    reserve(index + other.numElements + 2);

    if (index > numElements) {
        numElements = index;
    }
    ROW* r = (ROW*)rows;
    // Nun verschieben wir alle Elemente ab Position index um die größe des anderen
    // Arrays nach hinten
    if (numElements > index) {
        for (size_t i = numElements; i > index; --i) {
            size_t ii = i - 1;
            r[ii + other.numElements].value = r[ii].value;
            r[ii].value = NULL;
        }
    }
    // Die neuen Werte einfügen
    ROW* r2 = (ROW*)other.rows;
    for (size_t i = 0; i < other.numElements; i++) {
        if (r2[i].value != NULL) {
            r[index + i].value = new String;
            if (!r[index + i].value) throw OutOfMemoryException();
            r[index + i].value->set(*r2[i].value);
        } else {
            r[index + i].value = NULL;
        }
    }
    numElements += other.numElements;
}

void Array::insertf(size_t index, const char* fmt, ...)
{
    String value;
    va_list args;
    va_start(args, fmt);
    value.vasprintf(fmt, args);
    va_end(args);
    insert(index, value);
}

void Array::reserve(size_t size)
{
    if (size > numCapacity) {
        // PrintDebugTime ("Array::reserve von %zu auf %zu Elemente\n",numCapacity,size);
        ROW* r;
        // Array muss vergroessert werden
        void* newrows = realloc(rows, (size) * sizeof(ROW));
        if (!newrows) {
            throw OutOfMemoryException();
        }
        r = (ROW*)newrows;
        for (size_t i = numCapacity; i < size; i++) {
            r[i].value = NULL;
        }
        rows = newrows;
        numCapacity = size;
    }
}
void Array::list(const String& prefix) const
{
    ROW* r = (ROW*)rows;
    if (prefix.isEmpty()) {
        if ((!rows) || numElements == 0) {
            PrintDebug("Array ist leer\n");
        }
        for (size_t i = 0; i < numElements; i++) {
            if (r[i].value != NULL) PrintDebug("%6zu: %s\n", i, (const char*)r[i].value->getPtr());
        }
    } else {
        if ((!rows) || numElements == 0) {
            PrintDebug("Array \"%s\" ist leer\n", (const char*)prefix);
        }
        for (size_t i = 0; i < numElements; i++) {
            if (r[i].value != NULL) PrintDebug("%s, %6zu: %s\n", (const char*)prefix, i, (const char*)r[i].value->getPtr());
        }
    }
}

const String& Array::get(ssize_t index) const
{
    if (index < 0) {
        index = numElements + index;
        if (index < 0) throw OutOfBoundsException();
    }
    ROW* r = (ROW*)rows;
    if ((size_t)index >= numElements) throw OutOfBoundsException();
    if (r[index].value != NULL) return *r[index].value;
    return EmptyString;
}

String& Array::get(ssize_t index)
{
    if (index < 0) {
        index = numElements + index;
        if (index < 0) throw OutOfBoundsException();
    }
    ROW* r = (ROW*)rows;
    if ((size_t)index >= numElements) throw OutOfBoundsException();
    if (r[index].value != NULL) return *r[index].value;
    return EmptyString;
}

const String& Array::getRandom() const
{
    if (!numElements) return EmptyString;
    ROW* r = (ROW*)rows;
    size_t index = pplib::rand(0, numElements - 1);
    if (index < numElements && r[index].value != NULL) return *r[index].value;
    return EmptyString;
}

String& Array::getRandom()
{
    if (!numElements) return EmptyString;
    ROW* r = (ROW*)rows;
    size_t index = pplib::rand(0, numElements - 1);
    if (index < numElements && r[index].value != NULL) return *r[index].value;
    return EmptyString;
}

String Array::getRest(size_t index, const String& delimiter)
{
    String rest;
    ROW* r = (ROW*)rows;
    for (size_t i = index; i < numElements; i++) {
        if (i > index) rest += delimiter;
        if (r[i].value != NULL) rest += *r[i].value;
    }
    return rest;
}

String Array::erase(size_t index)
{
    if (index >= numElements) throw OutOfBoundsException();
    String ret;
    ROW* r = (ROW*)rows;
    if (r[index].value != NULL) {
        ret = *r[index].value;
        delete r[index].value;
    }
    for (size_t i = index; i < numElements - 1; i++) {
        r[i].value = r[i + 1].value;
    }
    numElements--;
    r[numElements].value = NULL;
    return ret;
}

String Array::shift()
{
    if (!numElements) throw EmptyDataException();
    String ret;
    ROW* r = (ROW*)rows;
    if (r[0].value != NULL) {
        ret = *r[0].value;
        delete r[0].value;
    }
    for (size_t i = 0; i < numElements - 1; i++) {
        r[i].value = r[i + 1].value;
    }
    numElements--;
    r[numElements].value = NULL;
    return ret;
}

String Array::pop()
{
    if (!numElements) throw EmptyDataException();
    ROW* r = (ROW*)rows;
    String ret;
    if (r[numElements - 1].value != NULL) {
        ret = *r[numElements - 1].value;
        delete r[numElements - 1].value;
        r[numElements - 1].value = NULL;
    }
    numElements--;
    return ret;
}

Array& Array::explode(const String& text, const String& delimiter, size_t limit, bool skipemptylines)
{
    if (text.isEmpty()) return *this;
    if (delimiter.isEmpty()) return *this;
    ssize_t p;
    size_t t = delimiter.len();
    size_t count = 0;
    const char* del = (const char*)delimiter;
    char* etext = (char*)text.getPtr();
    char* _t;
    String str;
    while (1) {
        _t = strstr(etext, del);
        if (_t) {
            p = _t - etext;
            if (p == 0 && skipemptylines == true) {
                etext += t;
                continue;
            }
            if (limit > 0 && count >= limit) {
                return *this;
            }
            str.set(etext, p);
            // add(etext,p);
            set(numElements, str);
            etext = etext + p + t;
            count++;
        } else {
            if (skipemptylines == false || strlen(etext) > 0) {
                count++;
                if (limit == 0 || count <= limit) {
                    add(etext);
                }
            }
            return *this;
        }
    }
    return *this;
}

String Array::implode(const String& delimiter) const
{
    String ret;
    for (size_t i = 0; i < numElements; i++) {
        if (i) ret += delimiter;
        ret += get(i);
    }
    return ret;
}

String& Array::operator[](ssize_t index)
{
    return get(index);
}

const String& Array::operator[](ssize_t index) const
{
    return get(index);
}

Array& Array::operator=(const Array& other)
{
    copy(other);
    return *this;
}

Array& Array::operator=(Array&& other)
{
    if (this != &other) {
        clear();
        numElements = other.numElements;
        numCapacity = other.numCapacity;
        rows = other.rows;
        other.numElements = 0;
        other.numCapacity = 0;
        other.rows = NULL;
    }
    return *this;
}

Array& Array::operator+=(const Array& other)
{
    add(other);
    return *this;
}
bool Array::operator==(const Array& other) const
{
    if (numElements != other.numElements) return false;
    for (size_t i = 0; i < numElements; i++)
        if (get(i) != other.get(i)) return false;
    return true;
}

bool Array::operator!=(const Array& other) const
{
    if (numElements != other.numElements) return true;
    for (size_t i = 0; i < numElements; i++)
        if (get(i) != other.get(i)) return true;
    return false;
}

Array operator+(const Array& a1, const Array& a2)
{
    Array ret(a1);
    ret.add(a2);
    return ret;
}

void Array::sort()
{
    std::multiset<pplib::String> s;
    for (size_t i = 0; i < numElements; i++) {
        s.insert(get(i));
    }
    clear();
    for (std::multiset<pplib::String>::const_iterator it = s.begin(); it != s.end(); ++it) {
        add(*it);
    }
}

void Array::sortReverse()
{
    std::multiset<pplib::String> s;
    for (size_t i = 0; i < numElements; i++) {
        s.insert(get(i));
    }
    clear();
    for (std::multiset<pplib::String>::const_reverse_iterator it = s.rbegin(); it != s.rend(); ++it) {
        add(*it);
    }
}

void Array::sortUnique()
{
    std::set<pplib::String> s;
    for (size_t i = 0; i < numElements; i++) {
        s.insert(get(i));
    }
    clear();
    for (std::multiset<pplib::String>::const_iterator it = s.begin(); it != s.end(); ++it) {
        add(*it);
    }
}

void Array::makeUnique()
{
    std::set<String> s;
    for (size_t i = 0; i < numElements; i++) {
        const String& value = get(i);
        auto it = s.find(value);
        if (it != s.end()) {
            erase(i);
            i--;
        } else {
            s.insert(value);
        }
    }
}

ssize_t Array::indexOf(const String& search) const
{
    if (!numElements) return -1;
    ROW* r = (ROW*)rows;
    for (size_t i = 0; i < numElements; i++) {
        if (r[i].value && *r[i].value == search) return i;
    }
    return -1;
}

bool Array::has(const String& search) const
{
    if (!numElements) return false;
    ROW* r = (ROW*)rows;
    for (size_t i = 0; i < numElements; i++) {
        if (r[i].value && *r[i].value == search) return true;
    }
    return false;
}

Array Sort(const Array& array, bool unique)
{
    Array ret;
    if (unique) {
        std::set<pplib::String> s;
        for (size_t i = 0; i < array.size(); i++) {
            s.insert(array.get(i));
        }
        for (std::multiset<pplib::String>::const_iterator it = s.begin(); it != s.end(); ++it) {
            ret.add(*it);
        }
    } else {
        std::multiset<pplib::String> s;
        for (size_t i = 0; i < array.size(); i++) {
            s.insert(array.get(i));
        }
        for (std::multiset<pplib::String>::const_iterator it = s.begin(); it != s.end(); ++it) {
            ret.add(*it);
        }
    }
    return (ret);
}

Array SortReverse(const Array& array, bool unique)
{
    Array ret;
    if (unique) {
        std::set<pplib::String> s;
        for (size_t i = 0; i < array.size(); i++) {
            s.insert(array.get(i));
        }
        for (std::multiset<pplib::String>::const_reverse_iterator it = s.rbegin(); it != s.rend(); ++it) {
            ret.add(*it);
        }
    } else {
        std::multiset<pplib::String> s;
        for (size_t i = 0; i < array.size(); i++) {
            s.insert(array.get(i));
        }
        for (std::multiset<pplib::String>::const_reverse_iterator it = s.rbegin(); it != s.rend(); ++it) {
            ret.add(*it);
        }
    }
    return (ret);
}

Array Array::fromArgs(int argc, const char* argv[])
{
    Array result;
    if (argc <= 0 || argv == NULL) return result;
    for (int i = 0; i < argc; i++) {
        result.add(argv[i]);
    }
    return result;
}

Array Array::fromArgs(const String& args)
{
    Array result;
    if (args.isEmpty()) return result;

    String currentArg;
    bool inDoubleQuote = false;
    bool inSingleQuote = false;
    bool inArg = false;
    size_t len = args.len();

    for (size_t i = 0; i < len; i++) {
        char c = args[i];

        if (c == '\\' && !inSingleQuote && i + 1 < len) {
            i++;
            currentArg.append(args[i]);
            inArg = true;
        } else if (c == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
            inArg = true;
        } else if (c == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
            inArg = true;
        } else if ((c == ' ' || c == '\t' || c == '\r' || c == '\n') && !inDoubleQuote && !inSingleQuote) {
            if (inArg) {
                result.add(currentArg);
                currentArg.clear();
                inArg = false;
            }
        } else {
            currentArg.append(c);
            inArg = true;
        }
    }

    if (inArg) {
        result.add(currentArg);
    }

    return result;
}

Array::iterator Array::begin() noexcept
{
    return iterator(rows);
}

Array::const_iterator Array::begin() const noexcept
{
    return const_iterator(rows);
}

Array::iterator Array::end() noexcept
{
    if (!rows) return iterator(NULL);
    return iterator((Array::ROW*)rows + numElements);
}

Array::const_iterator Array::end() const noexcept
{
    if (!rows) return const_iterator(NULL);
    return const_iterator((Array::ROW*)rows + numElements);
}

} // namespace pplib
