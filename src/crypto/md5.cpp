/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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

#include <ppl7/core/functions.h>
#include <ppl7/core/fileobject.h>
#include <ppl7/types/bytearray.h>
#include <ppl7/exceptions.h>

namespace ppl7
{

// MD5 Berechnung
typedef struct MD5Context
{
    uint32_t state[4];        /* state (ABCD) */
    uint32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64]; /* input buffer */
} MD5_CTX;

static void MD5Init(MD5_CTX* context);
static void MD5Update(MD5_CTX* context, const unsigned char* input, size_t inputLen);
static void MD5Final(unsigned char digest[16], MD5_CTX* context);
static void MD5Transform(uint32_t state[4], const unsigned char block[64]);

/*
 * Encodes input (uint32_t) into output (unsigned char). Assumes len is
 * a multiple of 4.
 */
static void Encode(unsigned char* output, const uint32_t* input, size_t len)
{
    for (size_t i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (unsigned char)(input[i] & 0xff);
        output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

/*
 * Decodes input (unsigned char) into output (uint32_t). Assumes len is
 * a multiple of 4.
 */
static void Decode(uint32_t* output, const unsigned char* input, size_t len)
{
    for (size_t i = 0, j = 0; j < len; i++, j += 4) {
        output[i] =
            ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) | (((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
    }
}

static const unsigned char PADDING[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                          0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                          0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/* F, G, H and I are basic MD5 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/*
 * FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
 */
#define FF(a, b, c, d, x, s, ac)                                                                                                           \
    {                                                                                                                                      \
        (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                                    \
        (a) = ROTATE_LEFT((a), (s));                                                                                                       \
        (a) += (b);                                                                                                                        \
    }
#define GG(a, b, c, d, x, s, ac)                                                                                                           \
    {                                                                                                                                      \
        (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                                    \
        (a) = ROTATE_LEFT((a), (s));                                                                                                       \
        (a) += (b);                                                                                                                        \
    }
#define HH(a, b, c, d, x, s, ac)                                                                                                           \
    {                                                                                                                                      \
        (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                                    \
        (a) = ROTATE_LEFT((a), (s));                                                                                                       \
        (a) += (b);                                                                                                                        \
    }

#define II(a, b, c, d, x, s, ac)                                                                                                           \
    {                                                                                                                                      \
        (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac);                                                                                    \
        (a) = ROTATE_LEFT((a), (s));                                                                                                       \
        (a) += (b);                                                                                                                        \
    }

static void MD5Init(MD5_CTX* context)
{
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

static void MD5Update(MD5_CTX* context, const unsigned char* input, size_t inputLen)
{
    size_t i, partLen;
    unsigned int index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    if ((context->count[0] += ((uint32_t)inputLen << 3)) < ((uint32_t)inputLen << 3)) {
        context->count[1]++;
    }
    context->count[1] += ((uint32_t)inputLen >> 29);

    partLen = 64 - index;

    if (inputLen >= partLen) {
        memcpy(&context->buffer[index], input, partLen);
        MD5Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64) {
            MD5Transform(context->state, &input[i]);
        }
        index = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[index], &input[i], inputLen - i);
}

static void MD5Pad(MD5_CTX* context)
{
    unsigned char bits[8];
    Encode(bits, context->count, 8);

    unsigned int index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    unsigned int padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(context, PADDING, padLen);
    MD5Update(context, bits, 8);
}

static void MD5Final(unsigned char digest[16], MD5_CTX* context)
{
    MD5Pad(context);
    Encode(digest, context->state, 16);
    memset(context, 0, sizeof(*context));
}

static void MD5Transform(uint32_t state[4], const unsigned char block[64])
{
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    Decode(x, block, 64);

    /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
    FF(a, b, c, d, x[0], S11, 0xd76aa478);
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);
    FF(c, d, a, b, x[2], S13, 0x242070db);
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);
    FF(d, a, b, c, x[5], S12, 0x4787c62a);
    FF(c, d, a, b, x[6], S13, 0xa8304613);
    FF(b, c, d, a, x[7], S14, 0xfd469501);
    FF(a, b, c, d, x[8], S11, 0x698098d8);
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);
    FF(b, c, d, a, x[11], S14, 0x895cd7be);
    FF(a, b, c, d, x[12], S11, 0x6b901122);
    FF(d, a, b, c, x[13], S12, 0xfd987193);
    FF(c, d, a, b, x[14], S13, 0xa679438e);
    FF(b, c, d, a, x[15], S14, 0x49b40821);

    /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
    GG(a, b, c, d, x[1], S21, 0xf61e2562);
    GG(d, a, b, c, x[6], S22, 0xc040b340);
    GG(c, d, a, b, x[11], S23, 0x265e5a51);
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);
    GG(a, b, c, d, x[5], S21, 0xd62f105d);
    GG(d, a, b, c, x[10], S22, 0x2441453);
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);
    GG(d, a, b, c, x[14], S22, 0xc33707d6);
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);
    GG(b, c, d, a, x[8], S24, 0x455a14ed);
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);
    GG(c, d, a, b, x[7], S23, 0x676f02d9);
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);

    /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
    HH(a, b, c, d, x[5], S31, 0xfffa3942);
    HH(d, a, b, c, x[8], S32, 0x8771f681);
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);
    HH(b, c, d, a, x[14], S34, 0xfde5380c);
    HH(a, b, c, d, x[1], S31, 0xa4beea44);
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);
    HH(b, c, d, a, x[6], S34, 0x4881d05);
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);

    /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
    II(a, b, c, d, x[0], S41, 0xf4292244);
    II(d, a, b, c, x[7], S42, 0x432aff97);
    II(c, d, a, b, x[14], S43, 0xab9423a7);
    II(b, c, d, a, x[5], S44, 0xfc93a039);
    II(a, b, c, d, x[12], S41, 0x655b59c3);
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);
    II(c, d, a, b, x[10], S43, 0xffeff47d);
    II(b, c, d, a, x[1], S44, 0x85845dd1);
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
    II(c, d, a, b, x[6], S43, 0xa3014314);
    II(b, c, d, a, x[13], S44, 0x4e0811a1);
    II(a, b, c, d, x[4], S41, 0xf7537e82);
    II(d, a, b, c, x[11], S42, 0xbd3af235);
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);
    II(b, c, d, a, x[9], S44, 0xeb86d391);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    memset(x, 0, sizeof(x));
}

static void MD5End(MD5_CTX* ctx, char buf[33])
{
    unsigned char digest[16];
    static const char hex[] = "0123456789abcdef";

    MD5Final(digest, ctx);
    for (int i = 0; i < 16; i++) {
        buf[i * 2] = hex[digest[i] >> 4];
        buf[i * 2 + 1] = hex[digest[i] & 0x0f];
    }
    buf[32] = '\0';
}

String Md5(const void* buffer, size_t size)
/*!\ingroup PPLGroupMath
 */
{
    if (buffer == NULL || size == 0) throw EmptyDataException();
    char tmp[33];
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, (const unsigned char*)buffer, size);
    MD5End(&ctx, tmp);
    tmp[32] = 0;
    return String(tmp);
}

String Md5(const ByteArrayPtr& buffer)
{
    if (buffer.isNull() || buffer.size() == 0) throw EmptyDataException();
    return Md5(buffer.ptr(), buffer.size());
}

/*!\brief MD5-Summe berechnen
 *
 * Diese Funktion berechnet die MD5-Summe des Strings und gibt diese als
 * Hexadezimalwert in einem String zurück.
 *
 * \return Die Funktion liefert ein String-Objekt zurück. Bei Erfolg enthält dieses
 * die MD5-Summe, im Fehlerfall wird eine Exception geworfen.
 * Ein Fehler kann auftreten, wenn die Klasse keine Daten enthalten hat oder
 * bei der Berechnung der MD5-Summe ein Fehler aufgetreten ist.
 *
 */
String String::md5() const
{
    if (stringlen == 0 || ptr == nullptr) throw EmptyDataException();
    return Md5(ptr, stringlen);
}

String FileObject::md5()
{
    if (!isOpen()) throw FileNotOpenException();

    ByteArray buffer(1024 * 1024);
    char tmp[33];
    MD5_CTX ctx;
    MD5Init(&ctx);

    uint64_t oldpos = tell();
    seek(0);

    try {
        while (!eof()) {
            size_t bytes_read = fread((void*)buffer.ptr(), 1, buffer.size());
            if (bytes_read == 0) break;
            MD5Update(&ctx, (const unsigned char*)buffer.ptr(), bytes_read);
        }
    }
    catch (...) {
        seek(oldpos);
        throw;
    }

    seek(oldpos);
    MD5End(&ctx, tmp);
    return String(tmp);
}

} // namespace ppl7
