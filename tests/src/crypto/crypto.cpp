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

// ------------------------------------------------------------------------------------------------------
// Zusaetzliche Tests zur Verbesserung der Codeabdeckung von Crypt, Encrypt und Decrypt
// ------------------------------------------------------------------------------------------------------

static const pplib::Crypt::Algorithm ALL_ALGORITHMS[] = {
    pplib::Crypt::Algo_AES_128,
    pplib::Crypt::Algo_AES_192,
    pplib::Crypt::Algo_AES_256,
    pplib::Crypt::Algo_ARIA_128,
    pplib::Crypt::Algo_ARIA_192,
    pplib::Crypt::Algo_ARIA_256,
    pplib::Crypt::Algo_BLOWFISH,
    pplib::Crypt::Algo_CAMELLIA_128,
    pplib::Crypt::Algo_CAMELLIA_192,
    pplib::Crypt::Algo_CAMELLIA_256,
    pplib::Crypt::Algo_CAST5,
    pplib::Crypt::Algo_DES,
    pplib::Crypt::Algo_TRIPLE_DES,
    pplib::Crypt::Algo_IDEA,
    pplib::Crypt::Algo_RC2,
    pplib::Crypt::Algo_RC5,
};

static const pplib::Crypt::Mode ALL_MODES[] = {
    pplib::Crypt::Mode_ECB,
    pplib::Crypt::Mode_CBC,
    pplib::Crypt::Mode_CFB,
    pplib::Crypt::Mode_OFB,
};

// Manche Algorithmen sind je nach OpenSSL-Build zur Compile-Zeit deaktiviert (dann wirft
// schon der Konstruktor UnsupportedAlgorithmException), andere stecken unter OpenSSL 3.x
// im "legacy"-Provider, der nicht automatisch geladen wird (dann schlaegt EVP_EncryptInit_ex
// zur Laufzeit fehl, InitializationFailedException). Welche Algorithmen betroffen sind,
// unterscheidet sich zwischen den Zielsystemen (Fedora/Debian/FreeBSD/mingw64 unter Windows),
// deshalb wird die Verfuegbarkeit hier zur Laufzeit geprueft statt fest anzunehmen.
static bool algorithmIsUsable(pplib::Crypt::Algorithm algo, pplib::Crypt::Mode mode)
{
    try {
        pplib::Encrypt e(algo, mode);
        return true;
    } catch (const pplib::UnsupportedAlgorithmException&) {
        return false;
    } catch (const pplib::InitializationFailedException&) {
        return false;
    }
}

TEST_F(CryptoTest, EncryptDecryptAllAlgorithmsAndModes)
{
    pplib::String clearData("this is an unencrypted string, long enough for multiple blocks!");
    int usableCount = 0;

    for (auto algo : ALL_ALGORITHMS) {
        for (auto mode : ALL_MODES) {
            if (!algorithmIsUsable(algo, mode)) continue;
            usableCount++;
            SCOPED_TRACE(::testing::Message() << "algo=" << algo << " mode=" << mode);

            pplib::Encrypt encrypt(algo, mode);
            pplib::Decrypt decrypt(algo, mode);

            // Schluessel-/IV-Laenge direkt vom Objekt erfragen, damit der Test unabhaengig
            // vom konkreten Algorithmus und der OpenSSL-Version bleibt.
            int keylen = encrypt.keyLength();
            int ivlen = encrypt.ivLength();
            ASSERT_LE(keylen, (int)sizeof(key));
            ASSERT_LE(ivlen, (int)sizeof(iv));

            encrypt.setKey(pplib::ByteArrayPtr(key, keylen));
            encrypt.setIV(pplib::ByteArrayPtr(iv, ivlen));
            decrypt.setKey(pplib::ByteArrayPtr(key, keylen));
            decrypt.setIV(pplib::ByteArrayPtr(iv, ivlen));

            pplib::ByteArray crypted = encrypt.encrypt(pplib::ByteArrayPtr(clearData));
            pplib::ByteArray decrypted = decrypt.decrypt(crypted);
            EXPECT_EQ(clearData, pplib::String(decrypted));
        }
    }
    EXPECT_GT(usableCount, 0) << "Kein einziger Algorithmus/Modus war auf diesem System nutzbar";
}

TEST_F(CryptoTest, ConstructWithInvalidAlgorithmThrows)
{
    ASSERT_THROW(pplib::Encrypt((pplib::Crypt::Algorithm)999, pplib::Crypt::Mode_CBC),
                 pplib::UnsupportedAlgorithmException);
    ASSERT_THROW(pplib::Decrypt((pplib::Crypt::Algorithm)999, pplib::Crypt::Mode_CBC),
                 pplib::UnsupportedAlgorithmException);
}

TEST_F(CryptoTest, ConstructWithInvalidModeThrows)
{
    ASSERT_THROW(pplib::Encrypt(pplib::Crypt::Algo_AES_128, (pplib::Crypt::Mode)99),
                 pplib::UnsupportedAlgorithmException);
    ASSERT_THROW(pplib::Decrypt(pplib::Crypt::Algo_AES_128, (pplib::Crypt::Mode)99),
                 pplib::UnsupportedAlgorithmException);
}

TEST_F(CryptoTest, MethodsThrowWhenNotInitialized)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_128, pplib::Crypt::Mode_CBC);
    ASSERT_THROW(encrypt.setAlgorithm((pplib::Crypt::Algorithm)999, pplib::Crypt::Mode_CBC),
                 pplib::UnsupportedAlgorithmException);
    ASSERT_THROW(encrypt.keyLength(), pplib::NotInitializedException);
    ASSERT_THROW(encrypt.ivLength(), pplib::NotInitializedException);
    ASSERT_THROW(encrypt.blockSize(), pplib::NotInitializedException);
    ASSERT_THROW(encrypt.setPadding(true), pplib::NotInitializedException);
    ASSERT_THROW(encrypt.setKeyLength(16), pplib::NotInitializedException);

    pplib::Decrypt decrypt(pplib::Crypt::Algo_AES_128, pplib::Crypt::Mode_CBC);
    ASSERT_THROW(decrypt.setAlgorithm((pplib::Crypt::Algorithm)999, pplib::Crypt::Mode_CBC),
                 pplib::UnsupportedAlgorithmException);
    ASSERT_THROW(decrypt.keyLength(), pplib::NotInitializedException);
    ASSERT_THROW(decrypt.ivLength(), pplib::NotInitializedException);
    ASSERT_THROW(decrypt.blockSize(), pplib::NotInitializedException);
    ASSERT_THROW(decrypt.setPadding(true), pplib::NotInitializedException);
    ASSERT_THROW(decrypt.setKeyLength(16), pplib::NotInitializedException);
}

TEST_F(CryptoTest, MaxKeyLength)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    ASSERT_EQ(64, encrypt.maxKeyLength());
}

TEST_F(CryptoTest, SetPadding)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    ASSERT_NO_THROW(encrypt.setPadding(false));
    ASSERT_NO_THROW(encrypt.setPadding(true));
}

TEST_F(CryptoTest, SetKeyLength)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    ASSERT_NO_THROW(encrypt.setKeyLength(32));
    ASSERT_THROW(encrypt.setKeyLength(999), pplib::InvalidKeyLengthException);
}

TEST_F(CryptoTest, EncryptDecryptStreaming)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    encrypt.setKey(pplib::ByteArrayPtr(key, 32));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));
    decrypt.setKey(pplib::ByteArrayPtr(key, 32));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String part1("this is the first part, ");
    pplib::String part2("and this is the second part.");

    pplib::ByteArray crypted, tmp;
    encrypt.update(pplib::ByteArrayPtr(part1), tmp);
    crypted.append(tmp);
    encrypt.update(pplib::ByteArrayPtr(part2), tmp);
    crypted.append(tmp);
    encrypt.final(tmp);
    crypted.append(tmp);

    // Ciphertext in zwei Haelften aufteilen, um auch bei Decrypt::update() mehrere
    // Aufrufe (mit unvollstaendigen Bloecken) abzudecken.
    size_t half = crypted.size() / 2;
    pplib::ByteArrayPtr firstHalf(crypted.ptr(), half);
    pplib::ByteArrayPtr secondHalf((const char*)crypted.ptr() + half, crypted.size() - half);

    pplib::ByteArray decrypted, dtmp;
    decrypt.update(firstHalf, dtmp);
    decrypted.append(dtmp);
    decrypt.update(secondHalf, dtmp);
    decrypted.append(dtmp);
    decrypt.final(dtmp);
    decrypted.append(dtmp);

    ASSERT_EQ(part1 + part2, pplib::String(decrypted));
}

TEST_F(CryptoTest, DecryptWithCorruptedCiphertextThrows)
{
    pplib::Encrypt encrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    pplib::Decrypt decrypt(pplib::Crypt::Algo_AES_256, pplib::Crypt::Mode_CBC);
    encrypt.setKey(pplib::ByteArrayPtr(key, 32));
    encrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));
    decrypt.setKey(pplib::ByteArrayPtr(key, 32));
    decrypt.setIV(pplib::ByteArrayPtr(iv, sizeof(iv)));

    pplib::String clearData("this is an unencrypted string");
    pplib::ByteArray crypted = encrypt.encrypt(pplib::ByteArrayPtr(clearData));

    // Letztes Byte kaputt machen -> ungueltiges PKCS#7-Padding beim Entschluesseln
    pplib::ByteArray corrupted;
    unsigned char* raw = static_cast<unsigned char*>(corrupted.copy(crypted));
    raw[corrupted.size() - 1] ^= 0xFF;

    ASSERT_THROW(decrypt.decrypt(corrupted), pplib::OperationFailedException);
}

} // namespace
