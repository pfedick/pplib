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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <pplib.h>
#include <pplib-crypto.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

#include <openssl/opensslv.h>

namespace
{

// The fixture for testing class Foo.
class DigestTest : public ::testing::Test
{
protected:
    DigestTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~DigestTest()
    {
    }
};

TEST_F(DigestTest, ConstructorSimple)
{
    ASSERT_NO_THROW(pplib::Digest dig);
}

TEST_F(DigestTest, ConstructorWithMD5)
{
    ASSERT_NO_THROW(pplib::Digest dig(pplib::Digest::Algo_MD5));
}

TEST_F(DigestTest, ConstructorWithName)
{
    ASSERT_NO_THROW(pplib::Digest dig("md5"));
}

TEST_F(DigestTest, ConstructorWithUnknownAlgorithm)
{
    ASSERT_THROW({ pplib::Digest dig((pplib::Digest::Algorithm)65555); }, pplib::InvalidAlgorithmException);
}

TEST_F(DigestTest, SetAlgorithmById)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_MD5));
    ASSERT_THROW(dig.setAlgorithm((pplib::Digest::Algorithm)777), pplib::InvalidAlgorithmException);
}

TEST_F(DigestTest, SetAlgorithmByName)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm("sha256"));
    ASSERT_THROW(dig.setAlgorithm("patrickcrypt"), pplib::InvalidAlgorithmException);
}

TEST_F(DigestTest, TestMD5)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_MD5));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("901736df3fbc807121c46f9eaed8ff28"), result.toHex());
}

TEST_F(DigestTest, TestSHA1)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA1));
    ASSERT_EQ((uint64_t)0, dig.bytesHashed());
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    ASSERT_EQ((uint64_t)591, dig.bytesHashed());
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("5bad3910a14b84999677b58528bd3d96500f1f94"), result.toHex());
}

TEST_F(DigestTest, TestSHA224)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA224));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"), result.toHex());
}

TEST_F(DigestTest, TestSHA256)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result.toHex());
}

TEST_F(DigestTest, TestSHA384)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA384));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("18a703d3f092003da0443174dbc977516335596a14db4090abe37f2106103f20c4669e3dc3d301c9339bf8913c94ec3e"),
              result.toHex());
}

TEST_F(DigestTest, TestSHA512)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA512));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(
        pplib::String("9b75ddb74674b45ab738f84f73ef25c833d7d33d7c72d2556f13274d753259187386bf91dadf8e6a735e6111d703d3ffbabf64d827aaec6"
                      "4d5c6c33259260ce9"),
        result.toHex());
}

TEST_F(DigestTest, TestRipeMD160)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_RIPEMD160));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("7cba3ff0d404943790e16e95b52d78ab502989dc"), result.toHex());
}

TEST_F(DigestTest, TestChangeAlgorithm)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA1));
    ASSERT_EQ((uint64_t)0, dig.bytesHashed());
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    ASSERT_EQ((uint64_t)591, dig.bytesHashed());
    pplib::ByteArray result;
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("5bad3910a14b84999677b58528bd3d96500f1f94"), result.toHex());

    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA224));
    ASSERT_EQ((uint64_t)0, dig.bytesHashed());
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    ASSERT_EQ((uint64_t)591, dig.bytesHashed());
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"), result.toHex());

    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    ASSERT_NO_THROW(result = dig.getDigest());
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result.toHex());
}

TEST_F(DigestTest, TestSaveDigestToString)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_MD5));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::String result;
    ASSERT_NO_THROW(dig.saveDigest(result));
    ASSERT_EQ(pplib::String("901736df3fbc807121c46f9eaed8ff28"), result);
}

TEST_F(DigestTest, TestSaveDigestToWideString)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_MD5));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::WideString result;
    ASSERT_NO_THROW(dig.saveDigest(result));
    ASSERT_EQ(pplib::WideString(L"901736df3fbc807121c46f9eaed8ff28"), result);
}

TEST_F(DigestTest, TestSaveDigestToByteArray)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_MD5));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::ByteArray result;
    ASSERT_NO_THROW(dig.saveDigest(result));
    ASSERT_EQ(pplib::String("901736df3fbc807121c46f9eaed8ff28"), result.toHex());
}

TEST_F(DigestTest, TestMultipleMD5)
{
    pplib::Digest dig;
    ASSERT_NO_THROW(dig.setAlgorithm(pplib::Digest::Algo_MD5));
    ASSERT_NO_THROW(dig.addData(loremipsum, strlen(loremipsum)));
    pplib::String result;
    ASSERT_NO_THROW(dig.saveDigest(result));
    ASSERT_EQ(pplib::String("901736df3fbc807121c46f9eaed8ff28"), result);

    const char* teststring = "Hello World";
    ASSERT_NO_THROW(dig.addData(teststring, strlen(teststring)));
    ASSERT_NO_THROW(dig.saveDigest(result));
    ASSERT_EQ(pplib::String("b10a8db164e0754105b7a99be72e3fe5"), result);

    teststring = "This is a third test";
    ASSERT_NO_THROW(dig.addData(teststring, strlen(teststring)));
    ASSERT_NO_THROW(dig.saveDigest(result));
    ASSERT_EQ(pplib::String("50e9ccd0e8fc5d8c774404ed3e8c4e8c"), result);
}

TEST_F(DigestTest, AddDataWithoutAlgorithm)
{
    pplib::Digest hash;
    ASSERT_THROW(hash.addData(loremipsum, strlen(loremipsum)), pplib::NoAlgorithmSpecifiedException);
}

TEST_F(DigestTest, TestAddDataFromPtr)
{
    pplib::Digest hash;
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addData(loremipsum, strlen(loremipsum)));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
}

TEST_F(DigestTest, TestAddDataFromString)
{
    pplib::Digest hash;
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addData(data));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
}

TEST_F(DigestTest, TestAddDataFromWideString)
{
    pplib::Digest hash;
    pplib::String original(loremipsum, strlen(loremipsum));
    pplib::WideString data(original);
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addData(data));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));

#ifdef WIN32
    // wchar_t is 2 byte on windows
    ASSERT_EQ(pplib::String("8c6d8979fd6c6de310f794f0c5d40b4efc15218d335e1b9c777825845c2342ad"), result);
#else
    // wchar_t is 4 byte on unix
    ASSERT_EQ(pplib::String("1780fcdeb19fa0435ad20e02720d96dd73f1f417452b8dab0bb4d433ee01531a"), result);
#endif
}

TEST_F(DigestTest, TestAddDataFromByteArray)
{
    pplib::Digest hash;
    pplib::ByteArray data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addData(data));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
}

TEST_F(DigestTest, TestAddDataFromByteArrayPtr)
{
    pplib::Digest hash;
    pplib::ByteArrayPtr data((void*)loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addData(data));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
}

TEST_F(DigestTest, TestAddDataFromClosedFile)
{
    pplib::Digest hash;
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    pplib::File ff;
    ASSERT_THROW(hash.addData(ff), pplib::FileNotOpenException);
}

TEST_F(DigestTest, TestAddDataFromOpenFile)
{
    pplib::Digest hash;
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    pplib::File ff("testdata/dirwalk/testfile.txt");
    ASSERT_NO_THROW(hash.addData(ff));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("e69bd11b9705bd3f3702d943fb7b6f9b640ca212c43e1dfc823b20e820930c52"), result);
}

TEST_F(DigestTest, TestAddDataFromFilename)
{
    pplib::Digest hash;
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/testfile.txt"));
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("e69bd11b9705bd3f3702d943fb7b6f9b640ca212c43e1dfc823b20e820930c52"), result);
}

TEST_F(DigestTest, TestAddDataFromSeveralFiles)
{
    pplib::Digest hash;
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/testfile.txt"));
    ASSERT_NO_THROW(hash.addFile("testdata/test.gif"));
    ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/afile.txt"));
    ASSERT_EQ((uint64_t)1615253, hash.bytesHashed());
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("327b824bbe2b201c0a59c34e430d0a1523b20683c58309064e14426f53c1b703"), result);
}

TEST_F(DigestTest, TestReset)
{
    pplib::Digest hash;
    ASSERT_NO_THROW(hash.setAlgorithm(pplib::Digest::Algo_SHA256));
    ASSERT_NO_THROW(hash.addData(loremipsum, strlen(loremipsum)));
    ASSERT_NO_THROW(hash.addData(loremipsum, strlen(loremipsum)));
    ASSERT_NO_THROW(hash.addData(loremipsum, strlen(loremipsum)));
    ASSERT_NO_THROW(hash.addData(loremipsum, strlen(loremipsum)));
    ASSERT_EQ((uint64_t)2364, hash.bytesHashed());
    hash.reset();
    ASSERT_NO_THROW(hash.addData(loremipsum, strlen(loremipsum)));
    ASSERT_EQ((uint64_t)591, hash.bytesHashed());
    pplib::String result;
    ASSERT_NO_THROW(hash.saveDigest(result));
    ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
}

TEST_F(DigestTest, ResetWithoutAlgorithm)
{
    pplib::Digest hash;
    ASSERT_THROW(hash.reset(), pplib::NoAlgorithmSpecifiedException);
}

TEST_F(DigestTest, TestStaticHashWithAlgorithmId)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::hash(data, pplib::Digest::Algo_SHA256).toHex();
        ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
    });
}

TEST_F(DigestTest, TestStaticHashWithAlgorithmName)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::hash(data, "sha256").toHex();
        ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
    });
}

TEST_F(DigestTest, TestStaticMd5)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::md5(data).toHex();
        ASSERT_EQ(pplib::String("901736df3fbc807121c46f9eaed8ff28"), result);
    });
}

TEST_F(DigestTest, TestStaticSha1)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::sha1(data).toHex();
        ASSERT_EQ(pplib::String("5bad3910a14b84999677b58528bd3d96500f1f94"), result);
    });
}

TEST_F(DigestTest, TestStaticSha224)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::sha224(data).toHex();
        ASSERT_EQ(pplib::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"), result);
    });
}

TEST_F(DigestTest, TestStaticSha256)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::sha256(data).toHex();
        ASSERT_EQ(pplib::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"), result);
    });
}

TEST_F(DigestTest, TestStaticSha384)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::sha384(data).toHex();
        ASSERT_EQ(pplib::String("18a703d3f092003da0443174dbc977516335596a14db4090abe37f2106103f20c4669e3dc3d301c9339bf8913c94ec3e"),
                  result);
    });
}

TEST_F(DigestTest, TestStaticSha512)
{
    pplib::String data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        pplib::String result = pplib::Digest::sha512(data).toHex();
        ASSERT_EQ(
            pplib::String("9b75ddb74674b45ab738f84f73ef25c833d7d33d7c72d2556f13274d753259187386bf91dadf8e6a735e6111d703d3ffbabf64d827a"
                          "aec64d5c6c33259260ce9"),
            result);
    });
}

TEST_F(DigestTest, TestStaticCrc32)
{
    pplib::ByteArrayPtr data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        uint32_t result = pplib::Digest::crc32(data);
        ASSERT_EQ((uint32_t)2852144956, (uint32_t)result);
    });
}

TEST_F(DigestTest, TestStaticAdler32)
{
    pplib::ByteArrayPtr data(loremipsum, strlen(loremipsum));
    ASSERT_NO_THROW({
        uint32_t result = pplib::Digest::adler32(data);
        ASSERT_EQ((uint32_t)1871173577, (uint32_t)result);
    });
}

} // namespace
