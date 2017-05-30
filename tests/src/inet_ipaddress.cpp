/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 600 $
 * $Date: 2013-04-26 21:37:49 +0200 (Fr, 26. Apr 2013) $
 * $Id: resolver.cpp 600 2013-04-26 19:37:49Z pafe $
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
#include <ppl7-inet.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"

#include <netinet/in.h>
#include <set>

namespace {

class InetIPAddressTest : public ::testing::Test {
	protected:
		InetIPAddressTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~InetIPAddressTest() {

	}
};


TEST_F(InetIPAddressTest, Constructor) {
	ASSERT_NO_THROW ({
		ppl7::IPAddress adr;
	});
}

TEST_F(InetIPAddressTest, setIPv4FromString) {
	ppl7::IPAddress adr;
	ASSERT_NO_THROW ({
		adr.set("194.77.8.1");
	});
	//ppl7::HexDump(adr.addr(), adr.addr_len());
	ASSERT_EQ(ppl7::IPAddress::IPv4, adr.family());
	ASSERT_EQ((size_t)4, adr.addr_len());

	const unsigned char* addr=(const unsigned char*)adr.addr();
	ASSERT_EQ(194,addr[0]);
	ASSERT_EQ(77,addr[1]);
	ASSERT_EQ(8,addr[2]);
	ASSERT_EQ(1,addr[3]);
}

TEST_F(InetIPAddressTest, setIPv6FromString) {
	ppl7::IPAddress adr;
	ASSERT_NO_THROW ({
		adr.set("2001:678:2a::53");
	});
	//ppl7::HexDump(adr.addr(), adr.addr_len());
	ASSERT_EQ(ppl7::IPAddress::IPv6, adr.family());
	ASSERT_EQ((size_t)16, adr.addr_len());

	const unsigned char* addr=(const unsigned char*)adr.addr();
	ASSERT_EQ(0x20,addr[0]);
	ASSERT_EQ(0x01,addr[1]);
	ASSERT_EQ(0x06,addr[2]);
	ASSERT_EQ(0x78,addr[3]);
	ASSERT_EQ(0,addr[4]);
	ASSERT_EQ(0x2a,addr[5]);
	ASSERT_EQ(0,addr[6]);
	ASSERT_EQ(0,addr[7]);
	ASSERT_EQ(0,addr[8]);
	ASSERT_EQ(0,addr[9]);
	ASSERT_EQ(0,addr[10]);
	ASSERT_EQ(0,addr[11]);
	ASSERT_EQ(0,addr[12]);
	ASSERT_EQ(0,addr[13]);
	ASSERT_EQ(0,addr[14]);
	ASSERT_EQ(0x53,addr[15]);
}

TEST_F(InetIPAddressTest, setInvalidIpAddressThrowsException) {
	ppl7::IPAddress adr;
	ASSERT_THROW({
		adr.set("555.333.2.1");
	},ppl7::InvalidIpAddressException);
	ASSERT_THROW({
		adr.set("2001:678:2a::53::1");
	},ppl7::InvalidIpAddressException);

}

TEST_F(InetIPAddressTest, IPv4toString) {
	ppl7::IPAddress adr;
	adr.set("194.77.8.1");
	ASSERT_EQ(ppl7::String("194.77.8.1"), adr.toString());
}

TEST_F(InetIPAddressTest, IPv6toString) {
	ppl7::IPAddress adr;
	adr.set("2001:678:2A::53");
	ASSERT_EQ(ppl7::String("2001:678:2a::53"), adr.toString());
}

TEST_F(InetIPAddressTest, UnsetIpAddressThrowsException) {
	ppl7::IPAddress adr;
	ASSERT_THROW({
		ASSERT_EQ(NULL, adr.addr());
	},ppl7::InvalidIpAddressException);
	ASSERT_THROW({
		ASSERT_EQ((size_t)0, adr.addr_len());
	},ppl7::InvalidIpAddressException);

	ASSERT_THROW({
		ASSERT_EQ(ppl7::String(),adr.toString());
	},ppl7::InvalidIpAddressException);
}

TEST_F(InetIPAddressTest, OperatorString) {
	ppl7::IPAddress adr;
	adr.set("2001:678:2A::53");
	ASSERT_EQ(ppl7::String("2001:678:2a::53"), (ppl7::String)adr);
}

TEST_F(InetIPAddressTest, ConstructorWithString) {
	ppl7::IPAddress adr1("194.77.8.1");
	ASSERT_EQ(ppl7::String("194.77.8.1"), adr1.toString());

	ppl7::IPAddress adr2("2001:678:2A::53");
	ASSERT_EQ(ppl7::String("2001:678:2a::53"), adr2.toString());
}

TEST_F(InetIPAddressTest, CopyConstructor) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr(adr1);
	ASSERT_EQ(ppl7::IPAddress::IPv4, adr.family());
	ASSERT_EQ((size_t)4, adr.addr_len());

	const unsigned char* addr=(const unsigned char*)adr.addr();
	ASSERT_EQ(194,addr[0]);
	ASSERT_EQ(77,addr[1]);
	ASSERT_EQ(8,addr[2]);
	ASSERT_EQ(1,addr[3]);
}

TEST_F(InetIPAddressTest, OperatorSetWithIPAddress) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr;
	adr=adr1;
	ASSERT_EQ(ppl7::IPAddress::IPv4, adr.family());
	ASSERT_EQ((size_t)4, adr.addr_len());

	const unsigned char* addr=(const unsigned char*)adr.addr();
	ASSERT_EQ(194,addr[0]);
	ASSERT_EQ(77,addr[1]);
	ASSERT_EQ(8,addr[2]);
	ASSERT_EQ(1,addr[3]);
}

TEST_F(InetIPAddressTest, OperatorSetWithString) {
	ppl7::String adr1("194.77.8.1");
	ppl7::IPAddress adr;
	adr=adr1;
	ASSERT_EQ(ppl7::IPAddress::IPv4, adr.family());
	ASSERT_EQ((size_t)4, adr.addr_len());

	const unsigned char* addr=(const unsigned char*)adr.addr();
	ASSERT_EQ(194,addr[0]);
	ASSERT_EQ(77,addr[1]);
	ASSERT_EQ(8,addr[2]);
	ASSERT_EQ(1,addr[3]);
}

TEST_F(InetIPAddressTest, setFromBinary) {
	unsigned char data[]={194,77,8,1};

	ppl7::IPAddress adr;
	adr.set(ppl7::IPAddress::IPv4, &data,4);
	ASSERT_EQ(ppl7::IPAddress::IPv4, adr.family());
	ASSERT_EQ((size_t)4, adr.addr_len());

	const unsigned char* addr=(const unsigned char*)adr.addr();
	ASSERT_EQ(194,addr[0]);
	ASSERT_EQ(77,addr[1]);
	ASSERT_EQ(8,addr[2]);
	ASSERT_EQ(1,addr[3]);
}

TEST_F(InetIPAddressTest, compare) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("2001:678:2A::53");
	ppl7::IPAddress adr3("194.77.8.2");
	ppl7::IPAddress adr4("195.77.8.2");
	ASSERT_TRUE(adr1.compare(adr2)<0);
	ASSERT_TRUE(adr2.compare(adr1)>0);
	ASSERT_TRUE(adr1.compare(adr3)<0);
	ASSERT_TRUE(adr1.compare(adr1)==0);
	ASSERT_TRUE(adr2.compare(adr2)==0);
	ASSERT_TRUE(adr1.compare(adr4)<0);
	ASSERT_TRUE(adr4.compare(adr1)>0);
}

TEST_F(InetIPAddressTest, operator_LT) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("195.77.8.2");

	ASSERT_TRUE(adr1<adr2);
	ASSERT_FALSE(adr2<adr1);
}

TEST_F(InetIPAddressTest, operator_LTEQ) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("195.77.8.2");
	ppl7::IPAddress adr3("194.77.8.1");

	ASSERT_TRUE(adr1<=adr2);
	ASSERT_TRUE(adr1<=adr3);
	ASSERT_FALSE(adr2<=adr1);
}

TEST_F(InetIPAddressTest, operator_EQ) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("195.77.8.2");
	ppl7::IPAddress adr3("194.77.8.1");
	ASSERT_TRUE(adr1==adr3);
	ASSERT_FALSE(adr1==adr2);
}

TEST_F(InetIPAddressTest, operator_NE) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("195.77.8.2");
	ppl7::IPAddress adr3("194.77.8.1");
	ASSERT_TRUE(adr1!=adr2);
	ASSERT_FALSE(adr1!=adr3);
}

TEST_F(InetIPAddressTest, operator_GT) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("195.77.8.2");

	ASSERT_TRUE(adr2>adr1);
	ASSERT_FALSE(adr1>adr2);
}

TEST_F(InetIPAddressTest, operator_GTEQ) {
	ppl7::IPAddress adr1("194.77.8.1");
	ppl7::IPAddress adr2("195.77.8.2");
	ppl7::IPAddress adr3("194.77.8.1");

	ASSERT_TRUE(adr2>=adr1);
	ASSERT_TRUE(adr3>=adr1);
	ASSERT_FALSE(adr1>=adr2);
}


TEST_F(InetIPAddressTest, sortIPAddresses) {
	std::set<ppl7::IPAddress> list;
	list.insert(ppl7::IPAddress("194.2.3.4"));
	list.insert(ppl7::IPAddress("2001:678:2A::53"));
	list.insert(ppl7::IPAddress("1.2.3.4"));
	list.insert(ppl7::IPAddress("255.0.0.0"));
	list.insert(ppl7::IPAddress("2001:678:2::1"));
	list.insert(ppl7::IPAddress("66.7.1.0"));
	list.insert(ppl7::IPAddress("2002:dead:2::1"));

	ASSERT_EQ((size_t)7,list.size());

	std::set<ppl7::IPAddress>::const_iterator it;
	it=list.begin();
	ASSERT_EQ(ppl7::IPAddress("1.2.3.4"),(*it++));
	ASSERT_EQ(ppl7::IPAddress("66.7.1.0"),(*it++));
	ASSERT_EQ(ppl7::IPAddress("194.2.3.4"),(*it++));
	ASSERT_EQ(ppl7::IPAddress("255.0.0.0"),(*it++));
	ASSERT_EQ(ppl7::IPAddress("2001:678:2::1"),(*it++));
	ASSERT_EQ(ppl7::IPAddress("2001:678:2A::53"),(*it++));
	ASSERT_EQ(ppl7::IPAddress("2002:dead:2::1"),(*it++));

}

}	// EOF namespace

