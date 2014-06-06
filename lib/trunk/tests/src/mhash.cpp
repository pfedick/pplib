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

namespace {

// The fixture for testing class Foo.
class MHashTest : public ::testing::Test {
	protected:
		MHashTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~MHashTest() {

	}
};

TEST_F(MHashTest, ConstructorSimple) {
	ASSERT_NO_THROW(ppl7::MHash hash);

}

TEST_F(MHashTest, ConstructorWithMD5) {
	ASSERT_NO_THROW(ppl7::MHash hash(ppl7::MHash::Algo_MD5));

}

TEST_F(MHashTest, ConstructorWithUnknownAlgorithm) {
	ASSERT_THROW({
		ppl7::MHash hash((ppl7::MHash::Algorithm)65555);
	},ppl7::UnknownAlgorithmException);
}


TEST_F(MHashTest, TestMD5) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_MD5));
	ASSERT_EQ(16,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result);
}

TEST_F(MHashTest, TestMultipleMD5) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_MD5));
	ASSERT_EQ(16,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result);

	const char *teststring="Hello World";
	ASSERT_NO_THROW(hash.addData(teststring,strlen(teststring)));
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("b10a8db164e0754105b7a99be72e3fe5"),result);

	teststring="This is a third test";
	ASSERT_NO_THROW(hash.addData(teststring,strlen(teststring)));
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("50e9ccd0e8fc5d8c774404ed3e8c4e8c"),result);

}


TEST_F(MHashTest, TestMD4) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_MD4));
	ASSERT_EQ(16,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("0a5520947c0bca5b9e2cff7ae0df67c7"),result);
}

TEST_F(MHashTest, TestCRC32B) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_CRC32B));
	ASSERT_EQ(4,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW({
		ASSERT_EQ((int)0xaa00473c, hash.getIntResult());
	});
}

TEST_F(MHashTest, TestSHA1) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA1));
	ASSERT_EQ(20,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result);
}

TEST_F(MHashTest, TestSHA224) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA224));
	ASSERT_EQ(28,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"),result);
}

TEST_F(MHashTest, TestSHA256) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_EQ(32,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(MHashTest, TestSHA384) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA384));
	ASSERT_EQ(48,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("18a703d3f092003da0443174dbc977516335596a14db4090abe37f2106103f20c4669e3dc3d301c9339bf8913c94ec3e"),result);
}

TEST_F(MHashTest, TestSHA512) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA512));
	ASSERT_EQ(64,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("9b75ddb74674b45ab738f84f73ef25c833d7d33d7c72d2556f13274d753259187386bf91dadf8e6a735e6111d703d3ffbabf64d827aaec64d5c6c33259260ce9"),result);
}

TEST_F(MHashTest, TestRipeMD160) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_RIPEMD160));
	ASSERT_EQ(20,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("7cba3ff0d404943790e16e95b52d78ab502989dc"),result);
}


TEST_F(MHashTest, TestAddDataFromPtr) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_EQ(32,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(MHashTest, TestAddDataFromVariantString) {
	ppl7::MHash hash;
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(MHashTest, TestAddDataFromVariantByteArray) {
	ppl7::MHash hash;
	ppl7::ByteArray data((void*)loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(MHashTest, TestAddDataFromVariantByteArrayPtr) {
	ppl7::MHash hash;
	ppl7::ByteArrayPtr data((void*)loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_NO_THROW(hash.addData(data));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(MHashTest, TestAddDataFromVariantArray) {
	ppl7::MHash hash;
	ppl7::Array data;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_THROW(hash.addData(data),ppl7::UnsupportedDataTypeException);
}

TEST_F(MHashTest, TestAddDataFromClosedFile) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ppl7::File ff;
	ASSERT_THROW(hash.addData(ff),ppl7::FileNotOpenException);
}

TEST_F(MHashTest, TestAddDataFromOpenFile) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ppl7::File ff("testdata/dirwalk/testfile.txt");
	ASSERT_NO_THROW(hash.addData(ff));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("e69bd11b9705bd3f3702d943fb7b6f9b640ca212c43e1dfc823b20e820930c52"),result);
}

TEST_F(MHashTest, TestAddDataFromFilename) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/testfile.txt"));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("e69bd11b9705bd3f3702d943fb7b6f9b640ca212c43e1dfc823b20e820930c52"),result);
}

TEST_F(MHashTest, TestAddDataFromSeveralFiles) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/testfile.txt"));
	ASSERT_NO_THROW(hash.addFile("testdata/test.gif"));
	ASSERT_NO_THROW(hash.addFile("testdata/dirwalk/afile.txt"));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("327b824bbe2b201c0a59c34e430d0a1523b20683c58309064e14426f53c1b703"),result);
}

TEST_F(MHashTest, TestReset) {
	ppl7::MHash hash;
	ASSERT_NO_THROW(hash.setAlgorithm(ppl7::MHash::Algo_SHA256));
	ASSERT_EQ(32,hash.getBlockSize());
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	hash.reset();
	ASSERT_NO_THROW(hash.addData(loremipsum,strlen(loremipsum)));
	ppl7::String result;
	ASSERT_NO_THROW(hash.getResult(result));
	ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
}

TEST_F(MHashTest, TestGetAlgorithmFromName) {
	ASSERT_EQ(ppl7::MHash::Algo_CRC32,ppl7::MHash::getAlgorithmFromName("crc32"));
	ASSERT_EQ(ppl7::MHash::Algo_CRC32B,ppl7::MHash::getAlgorithmFromName("crc32b"));
	ASSERT_EQ(ppl7::MHash::Algo_ADLER32,ppl7::MHash::getAlgorithmFromName("adler32"));
	ASSERT_EQ(ppl7::MHash::Algo_MD2,ppl7::MHash::getAlgorithmFromName("md2"));
	ASSERT_EQ(ppl7::MHash::Algo_MD4,ppl7::MHash::getAlgorithmFromName("md4"));
	ASSERT_EQ(ppl7::MHash::Algo_MD5,ppl7::MHash::getAlgorithmFromName("md5"));
	ASSERT_EQ(ppl7::MHash::Algo_SHA1,ppl7::MHash::getAlgorithmFromName("sha1"));
	ASSERT_EQ(ppl7::MHash::Algo_SHA224,ppl7::MHash::getAlgorithmFromName("sha224"));
	ASSERT_EQ(ppl7::MHash::Algo_SHA256,ppl7::MHash::getAlgorithmFromName("sha256"));
	ASSERT_EQ(ppl7::MHash::Algo_SHA384,ppl7::MHash::getAlgorithmFromName("sha384"));
	ASSERT_EQ(ppl7::MHash::Algo_SHA512,ppl7::MHash::getAlgorithmFromName("sha512"));
	ASSERT_EQ(ppl7::MHash::Algo_WHIRLPOOL,ppl7::MHash::getAlgorithmFromName("whirlpool"));
	ASSERT_EQ(ppl7::MHash::Algo_HAVAL256,ppl7::MHash::getAlgorithmFromName("haval256"));
	ASSERT_EQ(ppl7::MHash::Algo_HAVAL224,ppl7::MHash::getAlgorithmFromName("haval224"));
	ASSERT_EQ(ppl7::MHash::Algo_HAVAL192,ppl7::MHash::getAlgorithmFromName("haval192"));
	ASSERT_EQ(ppl7::MHash::Algo_HAVAL160,ppl7::MHash::getAlgorithmFromName("haval160"));
	ASSERT_EQ(ppl7::MHash::Algo_HAVAL128,ppl7::MHash::getAlgorithmFromName("haval128"));
	ASSERT_EQ(ppl7::MHash::Algo_RIPEMD128,ppl7::MHash::getAlgorithmFromName("ripemd128"));
	ASSERT_EQ(ppl7::MHash::Algo_RIPEMD160,ppl7::MHash::getAlgorithmFromName("ripemd160"));
	ASSERT_EQ(ppl7::MHash::Algo_RIPEMD256,ppl7::MHash::getAlgorithmFromName("ripemd256"));
	ASSERT_EQ(ppl7::MHash::Algo_RIPEMD320,ppl7::MHash::getAlgorithmFromName("ripemd320"));
	ASSERT_EQ(ppl7::MHash::Algo_TIGER128,ppl7::MHash::getAlgorithmFromName("tiger128"));
	ASSERT_EQ(ppl7::MHash::Algo_TIGER160,ppl7::MHash::getAlgorithmFromName("tiger160"));
	ASSERT_EQ(ppl7::MHash::Algo_TIGER192,ppl7::MHash::getAlgorithmFromName("tiger192"));
	ASSERT_EQ(ppl7::MHash::Algo_GOST,ppl7::MHash::getAlgorithmFromName("gost"));
	ASSERT_EQ(ppl7::MHash::Algo_SNEFRU128,ppl7::MHash::getAlgorithmFromName("snefru128"));
	ASSERT_EQ(ppl7::MHash::Algo_SNEFRU256,ppl7::MHash::getAlgorithmFromName("snefru256"));
}

TEST_F(MHashTest, TestStaticHashWithSha256) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::hash(data,ppl7::MHash::Algo_SHA256).toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}

TEST_F(MHashTest, TestStaticHashWithSha256String) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::hash(data,"sha256").toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}

TEST_F(MHashTest, TestStaticCrc32) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ASSERT_EQ(-1942889117,ppl7::MHash::crc32(data));
	});
}

TEST_F(MHashTest, TestStaticCrc32b) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ASSERT_EQ(-1442822340,ppl7::MHash::crc32b(data));
	});
}

TEST_F(MHashTest, TestStaticAdler32) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ASSERT_EQ(1871173577, ppl7::MHash::adler32(data));
	});
}

TEST_F(MHashTest, TestStaticMd4) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::md4(data).toHex();
		ASSERT_EQ(ppl7::String("0a5520947c0bca5b9e2cff7ae0df67c7"),result);
	});
}

TEST_F(MHashTest, TestStaticMd5) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::md5(data).toHex();
		ASSERT_EQ(ppl7::String("901736df3fbc807121c46f9eaed8ff28"),result);
	});
}
TEST_F(MHashTest, TestStaticSha1) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::sha1(data).toHex();
		ASSERT_EQ(ppl7::String("5bad3910a14b84999677b58528bd3d96500f1f94"),result);
	});
}
TEST_F(MHashTest, TestStaticSha2_256Bits) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::sha2(data,ppl7::MHash::Bits_256).toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}
TEST_F(MHashTest, TestStaticSha224) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::sha224(data).toHex();
		ASSERT_EQ(ppl7::String("026070b0071d458b383f5b91b73b8160c1dbd7e4e49468532c93b2f8"),result);
	});
}
TEST_F(MHashTest, TestStaticSha256) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::sha256(data).toHex();
		ASSERT_EQ(ppl7::String("ff4ef4245da5b09786e3d3de8b430292fa081984db272d2b13ed404b45353d28"),result);
	});
}
TEST_F(MHashTest, TestStaticSha384) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::sha384(data).toHex();
		ASSERT_EQ(ppl7::String("18a703d3f092003da0443174dbc977516335596a14db4090abe37f2106103f20c4669e3dc3d301c9339bf8913c94ec3e"),result);
	});
}
TEST_F(MHashTest, TestStaticSha512) {
	ppl7::String data(loremipsum,strlen(loremipsum));
	ASSERT_NO_THROW({
		ppl7::String result=ppl7::MHash::sha512(data).toHex();
		ASSERT_EQ(ppl7::String("9b75ddb74674b45ab738f84f73ef25c833d7d33d7c72d2556f13274d753259187386bf91dadf8e6a735e6111d703d3ffbabf64d827aaec64d5c6c33259260ce9"),result);
	});
}

}	// EOF namespace

