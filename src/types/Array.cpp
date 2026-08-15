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
#include <unordered_set>
#include <algorithm>

#include <pplib/types/array.h>
#include <pplib/types/string.h>
#include <pplib/exceptions.h>
#include <pplib/core/functions.h>

namespace pplib
{

Array::Array(const String& str, const String& delimiter, size_t limit, bool skipemptylines)
{
    explode(str, delimiter, limit, skipemptylines);
}

void Array::reserve(size_t size)
{
    try {
        elements.reserve(size);
    }
    catch (const std::bad_alloc&) {
        throw OutOfMemoryException();
    }
    catch (const std::length_error&) {
        throw OutOfMemoryException();
    }
}

void Array::copy(const Array& other)
{
    elements = other.elements;
}

void Array::add(const Array& other)
{
    elements.insert(elements.end(), other.elements.begin(), other.elements.end());
}

void Array::add(const String& value)
{
    elements.push_back(value);
}

void Array::add(const String& value, size_t size)
{
    String str;
    str.set(value, size);
    elements.push_back(str);
}

void Array::addf(const char* fmt, ...)
{
    String value;
    va_list args;
    va_start(args, fmt);
    value.vasprintf(fmt, args);
    va_end(args);
    elements.push_back(value);
}

void Array::set(size_t index, const String& value)
{
    if (index >= elements.size()) {
        elements.resize(index + 1);
    }
    elements[index] = value;
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
    if (index >= elements.size()) {
        set(index, value);
        return;
    }
    elements.insert(elements.begin() + index, value);
}

void Array::insert(size_t index, const Array& other)
{
    if (other.elements.empty()) return; // Anderes Array ist leer
    if (index >= elements.size()) {
        elements.resize(index);
        add(other);
        return;
    }
    elements.insert(elements.begin() + index, other.elements.begin(), other.elements.end());
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

void Array::list(const String& prefix) const
{
    if (prefix.isEmpty()) {
        if (elements.empty()) {
            PrintDebug("Array ist leer\n");
        }
        for (size_t i = 0; i < elements.size(); i++) {
            PrintDebug("%6zu: %s\n", i, (const char*)elements[i]);
        }
    } else {
        if (elements.empty()) {
            PrintDebug("Array \"%s\" ist leer\n", (const char*)prefix);
        }
        for (size_t i = 0; i < elements.size(); i++) {
            PrintDebug("%s, %6zu: %s\n", (const char*)prefix, i, (const char*)elements[i]);
        }
    }
}

const String& Array::get(ssize_t index) const
{
    ssize_t size = static_cast<ssize_t>(elements.size());
    if (index < 0) {
        index = size + index;
    }
    if (index < 0 || index >= size) {
        throw OutOfBoundsException();
    }
    return elements[index];
}

String& Array::get(ssize_t index)
{
    ssize_t size = static_cast<ssize_t>(elements.size());
    if (index < 0) {
        index = size + index;
    }
    if (index < 0 || index >= size) {
        throw OutOfBoundsException();
    }
    return elements[index];
}

const String& Array::getRandom() const
{
    if (elements.empty()) throw EmptyDataException();
    size_t index = pplib::rand(0, elements.size() - 1);
    return elements[index];
}

String& Array::getRandom()
{
    if (elements.empty()) throw EmptyDataException();
    size_t index = pplib::rand(0, elements.size() - 1);
    return elements[index];
}

String Array::getRest(size_t index, const String& delimiter)
{
    String rest;
    for (size_t i = index; i < elements.size(); i++) {
        if (i > index) rest += delimiter;
        rest += elements[i];
    }
    return rest;
}

String Array::erase(size_t index)
{
    if (index >= elements.size()) throw OutOfBoundsException();
    String ret = std::move(elements[index]);
    elements.erase(elements.begin() + index);
    return ret;
}

String Array::shift()
{
    if (elements.empty()) throw EmptyDataException();
    String ret = std::move(elements.front());
    elements.erase(elements.begin());
    return ret;
}

String Array::pop()
{
    if (elements.empty()) throw EmptyDataException();
    String ret = std::move(elements.back());
    elements.pop_back();
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
            add(str);
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
    for (size_t i = 0; i < elements.size(); i++) {
        if (i) ret += delimiter;
        ret += elements[i];
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

Array& Array::operator+=(const Array& other)
{
    add(other);
    return *this;
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
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        s.insert(*it);
    }
    elements.clear();
    elements.reserve(s.size());
    for (std::multiset<pplib::String>::const_iterator it = s.begin(); it != s.end(); ++it) {
        add(*it);
    }
}

void Array::sortReverse()
{
    std::multiset<pplib::String> s;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        s.insert(*it);
    }
    elements.clear();
    elements.reserve(s.size());
    for (std::multiset<pplib::String>::const_reverse_iterator it = s.rbegin(); it != s.rend(); ++it) {
        add(*it);
    }
}

void Array::sortUnique()
{
    std::set<pplib::String> s;
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        s.insert(*it);
    }
    elements.clear();
    elements.reserve(s.size());
    for (std::multiset<pplib::String>::const_iterator it = s.begin(); it != s.end(); ++it) {
        add(*it);
    }
}

void Array::makeUnique()
{
    std::set<String> seen;
    // std::remove_if schiebt alle "Duplikate" ans Ende des Vectors
    auto new_end = std::remove_if(elements.begin(), elements.end(), [&seen](const String& str) {
        // insert().second ist true, wenn der String NEU war (noch nicht im Set)
        return !seen.insert(str).second;
    });

    // Ein einziges erase schneidet am Ende alle Duplikate auf einmal ab
    elements.erase(new_end, elements.end());
}

ssize_t Array::indexOf(const String& search) const
{
    if (elements.empty()) return -1;
    for (size_t i = 0; i < elements.size(); i++) {
        if (elements[i] == search) return i;
    }
    return -1;
}

bool Array::has(const String& search) const
{
    if (elements.empty()) return false;
    for (size_t i = 0; i < elements.size(); i++) {
        if (elements[i] == search) return true;
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

} // namespace pplib
