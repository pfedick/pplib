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

#include <climits>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <algorithm>

#include <pplib/types/variantarray.h>
#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/exceptions.h>

#ifndef SSIZE_MAX
#define SSIZE_MAX (std::numeric_limits<ssize_t>::max())
#endif

namespace pplib
{

template <typename Func> static void runAndCatchOperation(Func&& op)
{
    try {
        op(); // Führt den übergebenen Codeblock aus
    }
    // LCOV_EXCL_START
    catch (const std::bad_alloc&) {
        throw OutOfMemoryException();
    }
    catch (const std::length_error&) {
        throw OutOfMemoryException();
    }
    // LCOV_EXCL_STOP
}

static inline size_t correctNegativeIndex(ssize_t index, size_t array_size)
{
    ssize_t size = static_cast<ssize_t>(array_size);
    if (index < 0) {
        index = size + index;
        if (index < 0) throw OutOfBoundsException();
    }
    return static_cast<size_t>(index);
}

VariantArray::VariantArray(const Array& other)
{
    extend(other);
}

VariantArray::VariantArray(Array&& other)
{
    extend(std::move(other));
}

void VariantArray::add(Variant value)
{
    if (elements.size() >= static_cast<size_t>(SSIZE_MAX)) {
        // LCOV_EXCL_START
        throw OutOfBoundsException();
        // LCOV_EXCL_STOP
    }
    runAndCatchOperation([&]() { elements.push_back(std::move(value)); });
}

void VariantArray::extend(const VariantArray& other)
{
    size_t base = elements.size();
    if (other.elements.size() > static_cast<size_t>(SSIZE_MAX) - base) {
        // LCOV_EXCL_START
        throw OutOfBoundsException();
        // LCOV_EXCL_STOP
    }
    if (this == &other) {
        VariantArray copy(other);
        extend(copy);
        return;
    }
    runAndCatchOperation([&]() { elements.insert(elements.end(), other.elements.begin(), other.elements.end()); });
}

void VariantArray::extend(VariantArray&& other)
{
    size_t base = elements.size();
    if (other.elements.size() > static_cast<size_t>(SSIZE_MAX) - base) {
        // LCOV_EXCL_START
        throw OutOfBoundsException();
        // LCOV_EXCL_STOP
    }
    if (this == &other) {
        VariantArray copy(other);
        extend(std::move(copy));
        return;
    }
    runAndCatchOperation([&]() {
        elements.insert(elements.end(), std::make_move_iterator(other.elements.begin()), std::make_move_iterator(other.elements.end()));
    });
    other.clear();
}

void VariantArray::extend(const Array& other)
{
    size_t base = elements.size();
    if (other.size() > static_cast<size_t>(SSIZE_MAX) - base) {
        // LCOV_EXCL_START
        throw OutOfBoundsException();
        // LCOV_EXCL_STOP
    }
    reserve(base + other.size());
    runAndCatchOperation([&]() {
        for (const auto& str : other) {
            elements.emplace_back(str);
        }
    });
}

void VariantArray::extend(Array&& other)
{
    size_t base = elements.size();
    if (other.size() > static_cast<size_t>(SSIZE_MAX) - base) {
        // LCOV_EXCL_START
        throw OutOfBoundsException();
        // LCOV_EXCL_STOP
    }
    reserve(base + other.size());
    runAndCatchOperation([&]() {
        for (auto& str : other) {
            elements.emplace_back(std::move(str));
        }
    });
    other.clear();
}

void VariantArray::reserve(std::size_t newCapacity)
{
    if (newCapacity > static_cast<size_t>(SSIZE_MAX)) throw OutOfBoundsException();
    runAndCatchOperation([&]() { elements.reserve(newCapacity); });
}

Variant& VariantArray::get(ssize_t index)
{
    size_t correctedIndex = correctNegativeIndex(index, elements.size());
    if (correctedIndex >= elements.size()) throw OutOfBoundsException();
    return elements[correctedIndex];
}

const Variant& VariantArray::get(ssize_t index) const
{
    size_t correctedIndex = correctNegativeIndex(index, elements.size());
    if (correctedIndex >= elements.size()) throw OutOfBoundsException();
    return elements[correctedIndex];
}

void VariantArray::set(ssize_t index, Variant value)
{
    size_t real_index = correctNegativeIndex(index, elements.size());
    if (real_index >= static_cast<size_t>(SSIZE_MAX)) throw OutOfBoundsException();
    runAndCatchOperation([&]() {
        if (real_index >= elements.size()) {
            elements.resize(real_index + 1);
        }
        elements[real_index] = std::move(value);
    });
}

void VariantArray::insert(ssize_t index, Variant value)
{
    size_t real_index = correctNegativeIndex(index, elements.size());
    if (elements.size() >= static_cast<size_t>(SSIZE_MAX) || real_index >= static_cast<size_t>(SSIZE_MAX)) throw OutOfBoundsException();
    runAndCatchOperation([&]() {
        if (real_index > elements.size()) {
            elements.resize(real_index);
        }
        elements.insert(elements.begin() + real_index, std::move(value));
    });
}

bool VariantArray::has(const Variant& value) const
{
    return std::find(elements.begin(), elements.end(), value) != elements.end();
}

ssize_t VariantArray::indexOf(const Variant& value) const
{
    auto it = std::find(elements.begin(), elements.end(), value);
    if (it != elements.end()) {
        return std::distance(elements.begin(), it);
    }
    return npos;
}

Variant VariantArray::erase(ssize_t index)
{
    size_t real_index = correctNegativeIndex(index, elements.size());
    if (real_index >= elements.size()) throw OutOfBoundsException();
    Variant removedElement = std::move(elements[real_index]);
    runAndCatchOperation([&]() { elements.erase(elements.begin() + real_index); });
    return removedElement;
}

Variant VariantArray::pop()
{
    if (elements.empty()) throw EmptyDataException();
    return erase(static_cast<ssize_t>(elements.size() - 1));
}

Variant VariantArray::shift()
{
    if (elements.empty()) throw EmptyDataException();
    return erase(0);
}

Array VariantArray::toArray(bool strict) const
{
    Array result;
    result.reserve(elements.size());
    for (const auto& var : elements) {
        if (var.isString()) {
            result.add(var.toString());
        } else if (var.isWideString()) {
            result.add(String(var.toWideString()));
        } else if (strict) {
            throw TypeConversionException();
        } else {
            result.add(String());
        }
    }
    return result;
}

VariantArray& VariantArray::operator=(const Array& other)
{
    elements.clear();
    extend(other);
    return *this;
}

VariantArray& VariantArray::operator=(Array&& other)
{
    elements.clear();
    extend(std::move(other));
    return *this;
}

size_t VariantArray::exportBinary(void* buffer, size_t buffersize) const
{
    return 0;
}

size_t VariantArray::importBinary(const void* buffer, size_t buffersize)
{
    return 0;
}

size_t VariantArray::binarySize() const
{
    return 0;
}

ByteArray VariantArray::exportBinary() const
{
    ByteArray buffer;
    size_t size = binarySize();
    buffer.malloc(size);
    exportBinary((void*)buffer.adr(), buffer.size());
    return buffer;
}
void VariantArray::importBinary(const ByteArrayPtr& buffer)
{
}

} // namespace pplib