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

#include <random>
#include <pplib/core/functions.h>
#include <pplib/types/bytearray.h>
#include <pplib/exceptions.h>

#include "config_pplib.h"

#ifdef HAVE_OPENSSL
#include <openssl/rand.h>
#endif

namespace pplib
{

// Thread-lokaler Mersenne-Twister-Generator (C++11/17)
static thread_local std::mt19937_64 rng(std::random_device{}());

void Random::seed(uint64_t x)
{
    rng.seed(x);
}

size_t Random::get(size_t min, size_t max)
{
    if (min > max) std::swap(min, max);
    if (min == max) return min;
    std::uniform_int_distribution<size_t> dist(min, max);
    return dist(rng);
}

float Random::getFloat(float min, float max)
{
    if (min > max) std::swap(min, max);
    if (min == max) return min;
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

double Random::getDouble(double min, double max)
{
    if (min > max) std::swap(min, max);
    if (min == max) return min;
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

ByteArray Random::bytes(size_t bytes)
{
    ByteArray bin;
    if (bytes == 0) return bin;

    char* buffer = (char*)bin.malloc(bytes);
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    for (size_t i = 0; i < bytes; i++) {
        buffer[i] = static_cast<char>(dist(rng));
    }
    return bin;
}

ByteArray& Random::fill(ByteArray& buffer, size_t bytes)
{
    if (bytes == 0) {
        buffer.clear();
        return buffer;
    }
    if (bytes != buffer.size()) buffer.realloc(bytes);
    char* b = static_cast<char*>(buffer.ptr());
    std::uniform_int_distribution<unsigned int> dist(0, 255);
    for (size_t i = 0; i < bytes; i++) {
        b[i] = static_cast<char>(dist(rng));
    }
    return buffer;
}

static void randBytes(char* buf, size_t n)
{
    while (n > 0) {
        int chunk = (n > 65536) ? 65536 : static_cast<int>(n);
        if (RAND_bytes(reinterpret_cast<unsigned char*>(buf), chunk) != 1) {
            throw OperationFailedException("RAND_bytes");
        }
        buf += chunk;
        n -= chunk;
    }
}

ByteArray Random::Secure::bytes(size_t num_bytes)
{
#ifdef HAVE_OPENSSL
    ByteArray bin;
    if (num_bytes == 0) return bin;
    randBytes(static_cast<char*>(bin.malloc(num_bytes)), num_bytes);
    return bin;
#else
    throw UnsupportedFeatureException("OpenSSL");
#endif
}

ByteArray& Random::Secure::fill(ByteArray& buffer, size_t num_bytes)
{
#ifdef HAVE_OPENSSL
    if (num_bytes == 0) {
        buffer.clear();
        return buffer;
    }
    if (num_bytes != buffer.size()) buffer.realloc(num_bytes);
    randBytes(static_cast<char*>(buffer.ptr()), num_bytes);
    return buffer;
#else
    throw UnsupportedFeatureException("OpenSSL");
#endif
}

ByteArray& Random::Secure::fill(ByteArray& buffer)
{
    return fill(buffer, buffer.size());
}

size_t Random::Secure::get(size_t min, size_t max)
{
#ifdef HAVE_OPENSSL
    if (min > max) std::swap(min, max);
    if (min == max) return min;

    size_t span = max - min + 1;                 // Bereichsgröße
    size_t limit = SIZE_MAX - (SIZE_MAX % span); // Schwelle gegen Modulo-Bias
    size_t v;
    do {
        randBytes(reinterpret_cast<char*>(&v), sizeof(v));
    } while (v >= limit); // Rejection Sampling
    return min + (v % span);
#else
    throw UnsupportedFeatureException("OpenSSL");
#endif
}

} // namespace pplib
