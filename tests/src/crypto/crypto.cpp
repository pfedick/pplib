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

#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/types/widestring.h>
#include <pplib/types/array.h>
#include <pplib/exceptions.h>
#include <pplib/core/file.h>
#include <pplib/core/dir.h>
#include <pplib-crypto.h>
#include <pplib/core/functions.h>
// #include <pplib/core/regex.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class CryptoTest : public ::testing::Test
{
protected:
    CryptoTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~CryptoTest()
    {
    }
};

static unsigned char key[] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                              17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
static unsigned char iv[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

TEST_F(CryptoTest, EncryptDecrypt_AES_128_CFB)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_128, pplib::Crypt::Mode_CFB);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_AES_128, pplib::Crypt::Mode_CFB);

    ASSERT_EQ(16, encrypt.keyLength());
    ASSERT_EQ(16, encrypt.ivLength());
    ASSERT_EQ(1, encrypt.blockSize());

    ASSERT_EQ(16, decrypt.keyLength());
    ASSERT_EQ(16, decrypt.ivLength());
    ASSERT_EQ(1, decrypt.blockSize());

    encrypt.setKey(pplib::ByteArrayPtr(key, 16));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    decrypt.setKey(pplib::ByteArrayPtr(key, 16));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String clearData("this is an unencrypted string");
    pplib::ByteArray crypteddata;

    encrypt.encrypt(pplib::ByteArrayPtr(clearData), crypteddata);

    ASSERT_EQ(29, crypteddata.size());

    pplib::ByteArray decrypteddata;
    decrypt.decrypt(crypteddata, decrypteddata);

    ASSERT_EQ(29, decrypteddata.size());
    ASSERT_EQ(clearData, pplib::String(decrypteddata));
}

TEST_F(CryptoTest, EncryptDecrypt_AES_256_CFB)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CFB);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CFB);

    ASSERT_EQ(32, encrypt.keyLength());
    ASSERT_EQ(16, encrypt.ivLength());
    ASSERT_EQ(1, encrypt.blockSize());

    ASSERT_EQ(32, decrypt.keyLength());
    ASSERT_EQ(16, decrypt.ivLength());
    ASSERT_EQ(1, decrypt.blockSize());

    encrypt.setKey(pplib::ByteArrayPtr(key, 32));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    decrypt.setKey(pplib::ByteArrayPtr(key, 32));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String clearData("this is an unencrypted string");
    pplib::ByteArray crypteddata;

    encrypt.encrypt(pplib::ByteArrayPtr(clearData), crypteddata);

    ASSERT_EQ(29, crypteddata.size());

    pplib::ByteArray decrypteddata;
    decrypt.decrypt(crypteddata, decrypteddata);

    ASSERT_EQ(29, decrypteddata.size());
    ASSERT_EQ(clearData, pplib::String(decrypteddata));
}

TEST_F(CryptoTest, EncryptDecrypt_TRIPLE_DES_CFB)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_TRIPLE_DES, pplib::Crypt::Mode_CFB);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_TRIPLE_DES, pplib::Crypt::Mode_CFB);

    ASSERT_EQ(24, encrypt.keyLength());
    ASSERT_EQ(8, encrypt.ivLength());
    ASSERT_EQ(1, encrypt.blockSize());

    ASSERT_EQ(24, decrypt.keyLength());
    ASSERT_EQ(8, decrypt.ivLength());
    ASSERT_EQ(1, decrypt.blockSize());

    encrypt.setKey(pplib::ByteArrayPtr(key, 24));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    decrypt.setKey(pplib::ByteArrayPtr(key, 24));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String clearData("this is an unencrypted string");
    pplib::ByteArray crypteddata;

    encrypt.encrypt(pplib::ByteArrayPtr(clearData), crypteddata);

    ASSERT_EQ(29, crypteddata.size());

    pplib::ByteArray decrypteddata;
    decrypt.decrypt(crypteddata, decrypteddata);

    ASSERT_EQ(29, decrypteddata.size());
    ASSERT_EQ(clearData, pplib::String(decrypteddata));
}

TEST_F(CryptoTest, EncryptDecrypt_TRIPLE_DES_CBC)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_TRIPLE_DES, pplib::Crypt::Mode_CBC);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_TRIPLE_DES, pplib::Crypt::Mode_CBC);

    ASSERT_EQ(24, encrypt.keyLength());
    ASSERT_EQ(8, encrypt.ivLength());
    ASSERT_EQ(8, encrypt.blockSize());

    ASSERT_EQ(24, decrypt.keyLength());
    ASSERT_EQ(8, decrypt.ivLength());
    ASSERT_EQ(8, decrypt.blockSize());

    encrypt.setKey(pplib::ByteArrayPtr(key, 24));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    decrypt.setKey(pplib::ByteArrayPtr(key, 24));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String clearData("this is an unencrypted string");
    pplib::ByteArray crypteddata;

    encrypt.encrypt(pplib::ByteArrayPtr(clearData), crypteddata);

    ASSERT_EQ(32, crypteddata.size());

    pplib::ByteArray decrypteddata;
    decrypt.decrypt(crypteddata, decrypteddata);
    // crypteddata.hexDump();
    // decrypteddata.hexDump();
    ASSERT_EQ(29, decrypteddata.size());
    ASSERT_EQ(clearData, pplib::String(decrypteddata));
}

TEST_F(CryptoTest, EncryptDecryptWithBase64_AES_256_CFB)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CFB);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CFB);

    ASSERT_EQ(32, encrypt.keyLength());
    ASSERT_EQ(16, encrypt.ivLength());
    ASSERT_EQ(1, encrypt.blockSize());

    ASSERT_EQ(32, decrypt.keyLength());
    ASSERT_EQ(16, decrypt.ivLength());
    ASSERT_EQ(1, decrypt.blockSize());

    encrypt.setKey(pplib::ByteArrayPtr(key, 32));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    decrypt.setKey(pplib::ByteArrayPtr(key, 32));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String clearData("this is an unencrypted string");
    pplib::String crypteddata;

    crypteddata = encrypt.encrypt(pplib::ByteArrayPtr(clearData)).toBase64();

    ASSERT_EQ(40, crypteddata.size());
    ASSERT_EQ(pplib::String("O2k5VBdaAV/4KXSLjjGxHexJoIdy/P148zvHAR7="), crypteddata);

    pplib::ByteArray decrypteddata;
    decrypt.decrypt(pplib::FromBase64(crypteddata), decrypteddata);

    ASSERT_EQ(29, decrypteddata.size());
    ASSERT_EQ(clearData, pplib::String(decrypteddata));
}

} // namespace
