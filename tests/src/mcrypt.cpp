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
class MCryptTest : public ::testing::Test {
	protected:
		MCryptTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~MCryptTest() {

	}
};

TEST_F(MCryptTest, ConstructorSimple) {
	ASSERT_NO_THROW({ppl7::MCrypt mc;});
}

TEST_F(MCryptTest, ConstructorWithKnownAlgorithm) {
	ASSERT_NO_THROW({ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH,ppl7::MCrypt::Mode_CFB);});
}

TEST_F(MCryptTest, setAlgorithmWithDefaults) {
	ppl7::MCrypt mc;
	ASSERT_NO_THROW(mc.setAlgorithm());
}

TEST_F(MCryptTest, setAlgorithm) {
	ppl7::MCrypt mc;
	ASSERT_NO_THROW(mc.setAlgorithm(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB));
}

TEST_F(MCryptTest, setInvalidAlgorithm) {
	ppl7::MCrypt mc;
	ASSERT_THROW(mc.setAlgorithm((ppl7::MCrypt::Algorithm)9999, ppl7::MCrypt::Mode_CFB), ppl7::InvalidAlgorithmException);
}

TEST_F(MCryptTest, getIVSizeForTwofishCfb) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ASSERT_EQ(16,mc.getIVSize());
}

TEST_F(MCryptTest, getIVSizeForBlowfishCfb) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_BLOWFISH, ppl7::MCrypt::Mode_CFB);
	ASSERT_EQ(8,mc.getIVSize());
}

TEST_F(MCryptTest, getMaxKeySizeForTwofishCfb) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ASSERT_EQ(32,mc.getMaxKeySize());
}

TEST_F(MCryptTest, setIVwithPointer) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	const char *myIV="TestInitializationVector";
	ASSERT_NO_THROW(mc.setIV(myIV,strlen(myIV)));
}

TEST_F(MCryptTest, setIVwithByteArray) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::ByteArray myIV("TestInitializationVector",24);
	ASSERT_NO_THROW(mc.setIV(myIV,strlen(myIV)));
}

TEST_F(MCryptTest, setIVwithString) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::String myIV="TestInitializationVector";
	ASSERT_NO_THROW(mc.setIV(myIV));
}

TEST_F(MCryptTest, setIVwithWide) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::WideString myIV=L"TestInitializationVector";
	ASSERT_NO_THROW(mc.setIV(myIV));
}

TEST_F(MCryptTest, setIVwithArray) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::Array myIV;
	ASSERT_THROW(mc.setIV(myIV), ppl7::UnsupportedDataTypeException);
}

TEST_F(MCryptTest, setKeywithPointer) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	const char *myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ASSERT_NO_THROW(mc.setKey(myKey,strlen(myKey)));
}

TEST_F(MCryptTest, setKeywithByteArray) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::ByteArray myKey("tG63N-kQ#23=d?b!v39wRtcu",24);
	ASSERT_NO_THROW(mc.setKey(myKey,strlen(myKey)));
}

TEST_F(MCryptTest, setKeywithString) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::String myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ASSERT_NO_THROW(mc.setKey(myKey));
}

TEST_F(MCryptTest, setKeywithWide) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::WideString myKey=L"tG63N-kQ#23=d?b!v39wRtcu";
	ASSERT_NO_THROW(mc.setKey(myKey));
}

TEST_F(MCryptTest, setKeywithArray) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ppl7::Array myKey;
	ASSERT_THROW(mc.setKey(myKey), ppl7::UnsupportedDataTypeException);
}

TEST_F(MCryptTest, needIV) {
	ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
	ASSERT_EQ(true,mc.needIV());
}

TEST_F(MCryptTest, cryptAndDecryptWithPtr) {
	const char cryptedBytes[]={
			(char)0xc5, (char)0xab, (char)0x39, (char)0xaf, (char)0xcc,
			(char)0xe6, (char)0x2b, (char)0xf3, (char)0xf4, (char)0x39, (char)0x2e };

	ppl7::ByteArray uncrypted("Hello World",11);
	ppl7::ByteArray myText=uncrypted;
	// Wir verwenden einen festen Key und Initialisierungsvektor, damit
	// die verschlüsselten Daten immer gleich sind und wir sie vergleichen können
	ppl7::String myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ppl7::String myIV="TestInitializationVector";
	ppl7::ByteArray crypted(cryptedBytes,sizeof(cryptedBytes));

	ASSERT_NO_THROW({
		ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
		mc.setIV(myIV);
		mc.setKey(myKey);
		mc.crypt((void*)myText.ptr(),myText.size());
		ASSERT_EQ(crypted,myText);
		mc.decrypt((void*)myText.ptr(),myText.size());
		ASSERT_EQ(uncrypted,myText);
	});
}

TEST_F(MCryptTest, cryptAndDecryptWithByteArrayPtr) {
	const char cryptedBytes[]={
			(char)0xc5, (char)0xab, (char)0x39, (char)0xaf, (char)0xcc,
			(char)0xe6, (char)0x2b, (char)0xf3, (char)0xf4, (char)0x39, (char)0x2e };

	ppl7::ByteArray uncrypted("Hello World",11);
	ppl7::ByteArray myText=uncrypted;
	// Wir verwenden einen festen Key und Initialisierungsvektor, damit
	// die verschlüsselten Daten immer gleich sind und wir sie vergleichen können
	ppl7::String myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ppl7::String myIV="TestInitializationVector";
	ppl7::ByteArray crypted(cryptedBytes,sizeof(cryptedBytes));

	ASSERT_NO_THROW({
		ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
		mc.setIV(myIV);
		mc.setKey(myKey);
		mc.crypt(myText);
		ASSERT_EQ(crypted,myText);
		mc.decrypt(myText);
		ASSERT_EQ(uncrypted,myText);
	});
}

TEST_F(MCryptTest, cryptAndDecryptWithStringVariant) {
	const char cryptedBytes[]={
			(char)0xc5, (char)0xab, (char)0x39, (char)0xaf, (char)0xcc,
			(char)0xe6, (char)0x2b, (char)0xf3, (char)0xf4, (char)0x39, (char)0x2e };

	ppl7::ByteArray uncrypted("Hello World",11);
	ppl7::String myText="Hello World";
	ppl7::ByteArray tmp, out;

	// Wir verwenden einen festen Key und Initialisierungsvektor, damit
	// die verschlüsselten Daten immer gleich sind und wir sie vergleichen können
	ppl7::String myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ppl7::String myIV="TestInitializationVector";
	ppl7::ByteArray crypted(cryptedBytes,sizeof(cryptedBytes));

	ASSERT_NO_THROW({
		ppl7::MCrypt mc(ppl7::MCrypt::Algo_TWOFISH, ppl7::MCrypt::Mode_CFB);
		mc.setIV(myIV);
		mc.setKey(myKey);
		mc.crypt(myText,tmp);
		ASSERT_EQ(crypted,tmp);

		mc.decrypt(tmp,out);
		ASSERT_EQ(uncrypted,out);
	});
}

TEST_F(MCryptTest, staticCryptWithoutIV) {
	ppl7::ByteArray uncrypted("Hello World",11);
	ppl7::ByteArray myText=uncrypted;
	// Wir verwenden einen festen Key und Initialisierungsvektor, damit
	// die verschlüsselten Daten immer gleich sind und wir sie vergleichen können
	ppl7::String myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ASSERT_NO_THROW({
		ppl7::MCrypt::crypt(myText,myKey,ppl7::MCrypt::Algo_TWOFISH,ppl7::MCrypt::Mode_CFB);
		ASSERT_NE(uncrypted,myText);
		ppl7::MCrypt::decrypt(myText,myKey,ppl7::MCrypt::Algo_TWOFISH,ppl7::MCrypt::Mode_CFB);
		ASSERT_EQ(uncrypted,myText);
	});
}

TEST_F(MCryptTest, staticCryptWithIV) {
	const char cryptedBytes[]={
			(char)0xc5, (char)0xab, (char)0x39, (char)0xaf, (char)0xcc,
			(char)0xe6, (char)0x2b, (char)0xf3, (char)0xf4, (char)0x39, (char)0x2e };
	ppl7::ByteArray crypted(cryptedBytes,sizeof(cryptedBytes));
	ppl7::ByteArray uncrypted("Hello World",11);
	ppl7::ByteArray myText=uncrypted;
	// Wir verwenden einen festen Key und Initialisierungsvektor, damit
	// die verschlüsselten Daten immer gleich sind und wir sie vergleichen können
	ppl7::String myKey="tG63N-kQ#23=d?b!v39wRtcu";
	ppl7::String myIV="TestInitializationVector";
	ASSERT_NO_THROW({
		ppl7::MCrypt::crypt(myText,myKey,myIV,ppl7::MCrypt::Algo_TWOFISH,ppl7::MCrypt::Mode_CFB);
		ASSERT_NE(uncrypted,myText);
		ASSERT_EQ(crypted,myText);
		ppl7::MCrypt::decrypt(myText,myKey,myIV,ppl7::MCrypt::Algo_TWOFISH,ppl7::MCrypt::Mode_CFB);
		ASSERT_EQ(uncrypted,myText);
	});
}



}	// EOF namespace

