/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 600 $
 * $Date: 2013-04-26 21:37:49 +0200 (Fr, 26 Apr 2013) $
 * $Id: memorygroup.cpp 600 2013-04-26 19:37:49Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <ppl7-crypto.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"

#include <openssl/opensslv.h>

namespace {

// The fixture for testing class Foo.
class DigestTest : public ::testing::Test {
	protected:
	DigestTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~DigestTest() {

	}
};

TEST_F(DigestTest, ConstructorSimple) {
	ASSERT_NO_THROW(ppl7::Digest dig);
}


TEST_F(DigestTest, ConstructorWithMD5) {
	ASSERT_NO_THROW(ppl7::Digest dig(ppl7::Digest::Algo_MD5));
}

TEST_F(DigestTest, ConstructorWithName) {
	ASSERT_NO_THROW(ppl7::Digest dig("md5"));

}

TEST_F(DigestTest, ConstructorWithUnknownAlgorithm) {
	ASSERT_THROW({
		ppl7::Digest dig((ppl7::Digest::Algorithm)65555);
	},ppl7::InvalidAlgorithmException);
}

TEST_F(DigestTest, SetAlgorithmById) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_THROW(dig.setAlgorithm((ppl7::Digest::Algorithm)777),ppl7::InvalidAlgorithmException);
}

TEST_F(DigestTest, SetAlgorithmByName) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm("sha256"));
	ASSERT_THROW(dig.setAlgorithm("patrickcrypt"),ppl7::InvalidAlgorithmException);
}


TEST_F(DigestTest, TestMD4) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD4));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("0a5520947c0bca5b9e2cff7ae0df67c7"),result.toHex());
}


TEST_F(DigestTest, TestMD5) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result.toHex());
}

TEST_F(DigestTest, TestSHA1) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA1));
	ASSERT_EQ((ppluint64)0,dig.bytesHashed());
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((ppluint64)591,dig.bytesHashed());
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result.toHex());
}

TEST_F(DigestTest, TestSHA224) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA224));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"),result.toHex());
}

TEST_F(DigestTest, TestSHA256) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result.toHex());
}

TEST_F(DigestTest, TestSHA384) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA384));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("18a703d3f092003da0443174dbc977516335596a14db4090abe37f2106103f20c4669e3dc3d301c9339bf8913c94ec3e"),result.toHex());
}

TEST_F(DigestTest, TestSHA512) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA512));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("9b75ddb74674b45ab738f84f73ef25c833d7d33d7c72d2556f13274d753259187386bf91dadf8e6a735e6111d703d3ffbabf64d827aaec64d5c6c33259260ce9"),result.toHex());
}

#if OPENSSL_VERSION_NUMBER >= 0x10001000L
TEST_F(DigestTest, TestWhirlpool) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_WHIRLPOOL));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("5df2f2942a455dae96896be1ca9507cbfbf51d284fccc65f59dcb15081035ab3cf4f975d3d8ce68c63dc19132cebeaa6d281f33a9dcfa3e0493e33b121d8420a"),result.toHex());
}
#endif

TEST_F(DigestTest, TestRipeMD160) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_RIPEMD160));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("7cba3ff0d404943790e16e95b52d78ab502989dc"),result.toHex());
}

TEST_F(DigestTest, TestECDSA) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_ECDSA));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result.toHex());
}

TEST_F(DigestTest, TestChangeAlgorithm) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA1));
	ASSERT_EQ((ppluint64)0,dig.bytesHashed());
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((ppluint64)591,dig.bytesHashed());
	ppl7::ByteArray result;
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result.toHex());

	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA224));
	ASSERT_EQ((ppluint64)0,dig.bytesHashed());
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((ppluint64)591,dig.bytesHashed());
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"),result.toHex());

	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(result=dig.getDigest());
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result.toHex());
}


TEST_F(DigestTest, TestSaveDigestToString) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(dig.saveDigest(result));
	ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result);
}

TEST_F(DigestTest, TestSaveDigestToWideString) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::WideString result;
	ASSERT_NO_THROW(dig.saveDigest(result));
	ASSERT_EQ(ppl7::WideString(L"901736df3fbc807121c46f9eaed8ff28"),result);
}

TEST_F(DigestTest, TestSaveDigestToByteArray) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray result;
	ASSERT_NO_THROW(dig.saveDigest(result));
	ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result.toHex());
}

TEST_F(DigestTest, TestSaveDigestToByteArrayPtr) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::ByteArray buffer(20);
	ppl7::ByteArrayPtr result=buffer;
	ASSERT_THROW(dig.saveDigest(result),ppl7::UnsupportedDataTypeException);
}

TEST_F(DigestTest, TestSaveDigestToArray) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::Array result;
	ASSERT_THROW(dig.saveDigest(result),ppl7::UnsupportedDataTypeException);
}

TEST_F(DigestTest, TestMultipleMD5) {
	ppl7::Digest dig;
	ASSERT_NO_THROW(dig.setAlgorithm(ppl7::Digest::Algo_MD5));
	ASSERT_NO_THROW(dig.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(dig.saveDigest(result));
	ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result);

	const char *teststring="Hello World";
	ASSERT_NO_THROW(dig.addData(teststring,strlen(teststring)));
	ASSERT_NO_THROW(dig.saveDigest(result));
	ASSERT_EQ(ppl7::String("b10a8db164e0754105b7a99be72e3fe5"),result);

	teststring="This is a third test";
	ASSERT_NO_THROW(dig.addData(teststring,strlen(teststring)));
	ASSERT_NO_THROW(dig.saveDigest(result));
	ASSERT_EQ(ppl7::String("50e9ccd0e8fc5d8c774404ed3e8c4e8c"),result);

}

TEST_F(DigestTest, AddDataWithoutAlgorithm) {
	ppl7::Digest hash;
	ASSERT_THROW(hash.addData(loremipsum,strlen(loremipsum)),ppl7::NoAlgorithmSpecifiedException);
}


TEST_F(DigestTest, TestAddDataFromPtr) {
	ppl7::Digest hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(DigestTest, TestAddDataFromVariantString) {
	ppl7::Digest hash;
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(DigestTest, TestAddDataFromVariantWideString) {
	ppl7::Digest hash;
	ppl7::String original(loremipsum,strlen(loremipsum));
	ppl7::WideString data=original;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("1780fcdeb19fa0435ad20e02720d96dd73f1f417452b8dab0bb4d433ee01531a"),result);
}

TEST_F(DigestTest, TestAddDataFromVariantByteArray) {
	ppl7::Digest hash;
	ppl7::ByteArray data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(DigestTest, TestAddDataFromVariantByteArrayPtr) {
	ppl7::Digest hash;
	ppl7::ByteArrayPtr data((void*)loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(DigestTest, TestAddDataFromVariantArray) {
	ppl7::Digest hash;
	ppl7::Array data;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_THROW(hash.addData(data),ppl7::UnsupportedDataTypeException);
}

TEST_F(DigestTest, TestAddDataFromClosedFile) {
	ppl7::Digest hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ppl7::File ff;
	ASSERT_THROW(hash.addData(ff),ppl7::FileNotOpenException);
}

TEST_F(DigestTest, TestAddDataFromOpenFile) {
	ppl7::Digest hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ppl7::File ff("testdata/dirwalk/testfile.txt");
	ASSERT_NO_THROW(hash.addData(ff));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("e69bd11b9705bd3f3702d943fb7b6f9b640ca212c43e1dfc823b20e820930c52"),result);
}

TEST_F(DigestTest, TestAddDataFromFilename) {
	ppl7::Digest hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/testfile.txt"));
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("e69bd11b9705bd3f3702d943fb7b6f9b640ca212c43e1dfc823b20e820930c52"),result);
}

TEST_F(DigestTest, TestAddDataFromSeveralFiles) {
	ppl7::Digest hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/testfile.txt"));
	ASSERT_NO_THROW(hash.addFile("testdata/test.gif"));
	ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/afile.txt"));
	ASSERT_EQ((ppluint64)1615253,hash.bytesHashed());
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("327b824bbe2b201c0a59c34e430d0a1523b20683c58309064e14426f53c1b703"),result);
}

TEST_F(DigestTest, TestReset) {
	ppl7::Digest hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::Digest::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((ppluint64)2364,hash.bytesHashed());
	hash.reset();
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_EQ((ppluint64)591,hash.bytesHashed());
	ppl7::String result;
	ASSERT_NO_THROW(hash.saveDigest(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(DigestTest, ResetWithoutAlgorithm) {
	ppl7::Digest hash;
	ASSERT_THROW(hash.reset(),ppl7::NoAlgorithmSpecifiedException);
}



TEST_F(DigestTest, TestStaticHashWithAlgorithmId) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::hash(data,ppl7::Digest::Algo_SHA256).toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}

TEST_F(DigestTest, TestStaticHashWithAlgorithmName) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::hash(data,"sha256").toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}

TEST_F(DigestTest, TestStaticMd4) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::md4(data).toHex();
		ASSERT_EQ(ppl7::String("0a5520947c0bca5b9e2cff7ae0df67c7"),result);
	});
}

TEST_F(DigestTest, TestStaticMd5) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::md5(data).toHex();
		ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result);
	});
}

TEST_F(DigestTest, TestStaticSha1) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::sha1(data).toHex();
		ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result);
	});
}

TEST_F(DigestTest, TestStaticSha224) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::sha224(data).toHex();
		ASSERT_EQ(ppl7::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"),result);
	});
}

TEST_F(DigestTest, TestStaticSha256) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::sha256(data).toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}

TEST_F(DigestTest, TestStaticSha384) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::sha384(data).toHex();
		ASSERT_EQ(ppl7::String("18a703d3f092003da0443174dbc977516335596a14db4090abe37f2106103f20c4669e3dc3d301c9339bf8913c94ec3e"),result);
	});
}

TEST_F(DigestTest, TestStaticSha512) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::sha512(data).toHex();
		ASSERT_EQ(ppl7::String("9b75ddb74674b45ab738f84f73ef25c833d7d33d7c72d2556f13274d753259187386bf91dadf8e6a735e6111d703d3ffbabf64d827aaec64d5c6c33259260ce9"),result);
	});
}

TEST_F(DigestTest, TestStaticEcdsa) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::Digest::ecdsa(data).toHex();
		ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result);
	});
}


}	// EOF namespace

