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

namespace pplib
{

// Thread-lokaler Mersenne-Twister-Generator (C++11/17)
static thread_local std::mt19937_64 rng(std::random_device{}());

void srand(uint32_t x)
{
    rng.seed(x);
}

size_t rand(size_t min, size_t max)
{
    if (min >= max) return min;
    std::uniform_int_distribution<size_t> dist(min, max);
    return dist(rng);
}

float randf(float min, float max)
{
    if (min >= max) return min;
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

double randd(double min, double max)
{
    if (min >= max) return min;
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

ByteArray Random(size_t bytes)
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

ByteArray& Random(ByteArray& buffer, size_t bytes)
{
    if (bytes == 0) {
        buffer.clear();
        return buffer;
    }
    char* b = (char*)buffer.malloc(bytes);
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    for (size_t i = 0; i < bytes; i++) {
        b[i] = static_cast<char>(dist(rng));
    }
    return buffer;
}

} // namespace pplib
