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
#include <pplib/core/functions.h>
#include <pplib/exceptions.h>

#ifndef SSIZE_MAX
#define SSIZE_MAX (std::numeric_limits<ssize_t>::max())
#endif

namespace pplib
{

extern size_t exportVariantBinary(const Variant& v, char* buffer, size_t buffersize);
extern size_t importVariantBinary(Variant& v, const char* ptr, size_t buffersize);

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
    char* ptr = (char*)buffer;
    size_t p = 0;
    // ByteArray ba;
    if (!buffer) buffersize = 0;
    if (p + 8 <= buffersize) memcpy(ptr, "PPL8VAAR", 8);
    p += 8;
    if (p + 1 <= buffersize) PokeN8(ptr + p, 1); // Version 1
    p++;
    if (p + 8 <= buffersize) PokeN64(ptr + p, elements.size());
    p += 8;
    VariantArray::const_iterator it;
    for (it = elements.begin(); it != elements.end(); ++it) {
        const Variant& a = *it;
        size_t remaining = (buffersize > p) ? (buffersize - p) : 0;
        p += exportVariantBinary(a, ptr + p, remaining);
    }
    if (buffersize == 0 || p <= buffersize) return p;
    throw ExportBufferToSmallException("%zd < %zd", buffersize, p);
}

size_t VariantArray::importBinary(const void* buffer, size_t buffersize)
{
    if (!buffer) throw IllegalArgumentException();
    if (buffersize == 0) throw IllegalArgumentException();
    const char* ptr = (const char*)buffer;
    size_t p = 0;
    if (buffersize < 8 || strncmp((const char*)ptr, "PPL8VAAR", 8) != 0) {
        throw ImportFailedException("Not an PPL8 VariantArray binary export");
    }
    p += 8;
    if (p + 1 > buffersize) throw ImportFailedException("Invalid PPL8 VariantArray binary export");
    int version = PeekN8(ptr + p);
    p++;
    if (version != 1) throw ImportFailedException("Invalid PPL8 VariantArray binary export version %d", version);

    auto requireBytes = [&](size_t n) {
        if (buffersize - p < n) {
            throw ImportFailedException("Buffer too small for import");
        }
    };
    requireBytes(8);
    size_t elementCount = PeekN64(ptr + p);
    p += 8;
    // Jedes Element muss mindestens 1 Byte im Buffer belegen
    if (elementCount > static_cast<size_t>(SSIZE_MAX) || buffersize - p < elementCount) {
        throw ImportFailedException("Invalid element count (%zu) or buffer too small", elementCount);
    }
    clear();
    reserve(elementCount);
    for (size_t i = 0; i < elementCount; i++) {
        Variant var;
        p += importVariantBinary(var, ptr + p, buffersize - p);
        add(std::move(var));
    }
    return p;
}

size_t VariantArray::binarySize() const
{
    return exportBinary(NULL, 0);
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
    importBinary(buffer.adr(), buffer.size());
}

} // namespace pplib