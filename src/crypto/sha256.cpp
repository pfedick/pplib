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

#include <pplib/core/functions.h>
#include <pplib/types/bytearray.h>
#include <pplib/exceptions.h>

namespace pplib
{
// Bitwise rotation right
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

// SHA-256 Logical Functions
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define SIGMA0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIGMA1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

class Sha256Context
{
public:
    uint32_t state[8];
    uint64_t bitcount;
    unsigned char buffer[64];
};

static void SHA256Init(Sha256Context& ctx);
static void SHA256Update(Sha256Context& ctx, const unsigned char* data, size_t len);
static void SHA256Final(Sha256Context& ctx, unsigned char digest[32]);
static void SHA256Transform(Sha256Context& ctx, const unsigned char data[64]);

static void SHA256Init(Sha256Context& ctx)
{
    ctx.bitcount = 0;
    ctx.state[0] = 0x6a09e667;
    ctx.state[1] = 0xbb67ae85;
    ctx.state[2] = 0x3c6ef372;
    ctx.state[3] = 0xa54ff53a;
    ctx.state[4] = 0x510e527f;
    ctx.state[5] = 0x9b05688c;
    ctx.state[6] = 0x1f83d9ab;
    ctx.state[7] = 0x5be0cd19;
}

static void SHA256Update(Sha256Context& ctx, const unsigned char* data, size_t len)
{
    size_t i;
    size_t index = (ctx.bitcount / 8) % 64;
    ctx.bitcount += len * 8;

    size_t partlen = 64 - index;

    if (len >= partlen) {
        memcpy(&ctx.buffer[index], data, partlen);
        SHA256Transform(ctx, ctx.buffer);

        for (i = partlen; i + 63 < len; i += 64) {
            SHA256Transform(ctx, &data[i]);
        }
        index = 0;
    } else {
        i = 0;
    }

    memcpy(&ctx.buffer[index], &data[i], len - i);
}

static void SHA256Final(Sha256Context& ctx, unsigned char digest[32])
{
    unsigned char bits[8];
    for (int i = 0; i < 8; i++) {
        bits[i] = (unsigned char)((ctx.bitcount >> ((7 - i) * 8)) & 0xff);
    }

    size_t index = (ctx.bitcount / 8) % 64;
    size_t padlen = (index < 56) ? (56 - index) : (120 - index);
    static const unsigned char PADDING[64] = {0x80};
    SHA256Update(ctx, PADDING, padlen);
    SHA256Update(ctx, bits, 8);

    for (int i = 0; i < 8; i++) {
        digest[i * 4] = (unsigned char)((ctx.state[i] >> 24) & 0xff);
        digest[i * 4 + 1] = (unsigned char)((ctx.state[i] >> 16) & 0xff);
        digest[i * 4 + 2] = (unsigned char)((ctx.state[i] >> 8) & 0xff);
        digest[i * 4 + 3] = (unsigned char)(ctx.state[i] & 0xff);
    }
}

static void SHA256Transform(Sha256Context& ctx, const unsigned char data[64])
{
    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be,
        0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa,
        0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85,
        0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa98, 0x5b9cca4f,
        0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    uint32_t W[64];
    for (int t = 0; t < 16; t++) {
        W[t] = ((uint32_t)data[t * 4] << 24) | ((uint32_t)data[t * 4 + 1] << 16) | ((uint32_t)data[t * 4 + 2] << 8) |
               ((uint32_t)data[t * 4 + 3]);
    }
    for (int t = 16; t < 64; t++) {
        W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];
    }
    uint32_t a = ctx.state[0];
    uint32_t b = ctx.state[1];
    uint32_t c = ctx.state[2];
    uint32_t d = ctx.state[3];
    uint32_t e = ctx.state[4];
    uint32_t f = ctx.state[5];
    uint32_t g = ctx.state[6];
    uint32_t h = ctx.state[7];

    for (int t = 0; t < 64; t++) {
        uint32_t T1 = h + SIGMA1(e) + CH(e, f, g) + K[t] + W[t];
        uint32_t T2 = SIGMA0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }
    ctx.state[0] += a;
    ctx.state[1] += b;
    ctx.state[2] += c;
    ctx.state[3] += d;
    ctx.state[4] += e;
    ctx.state[5] += f;
    ctx.state[6] += g;
    ctx.state[7] += h;
}

// Public functions
String Sha256(const void* buffer, size_t size)
{
    if (buffer == NULL || size == 0) throw EmptyDataException();
    unsigned char digest[32];
    Sha256Context ctx;
    SHA256Init(ctx);
    SHA256Update(ctx, (const unsigned char*)buffer, size);
    SHA256Final(ctx, digest);

    static const char hex[] = "0123456789abcdef";
    char hexbuf[65];
    for (int i = 0; i < 32; i++) {
        hexbuf[i * 2] = hex[digest[i] >> 4];
        hexbuf[i * 2 + 1] = hex[digest[i] & 0x0f];
    }
    hexbuf[64] = '\0';
    return String(hexbuf);
}

String Sha256(const ByteArrayPtr& buffer)
{
    return Sha256(buffer.ptr(), buffer.size());
}

} // namespace pplib
