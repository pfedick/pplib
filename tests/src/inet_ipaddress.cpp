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
#ifndef WIN32
#include <netinet/in.h>
#endif
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
	adr.set("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
	ASSERT_EQ(ppl7::String("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), adr.toString());
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

TEST_F(InetIPAddressTest, mask_v4) {
	ASSERT_EQ(ppl7::IPAddress("255.255.255.255"),ppl7::IPAddress("255.255.255.255").mask(32));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.254"),ppl7::IPAddress("255.255.255.255").mask(31));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.252"),ppl7::IPAddress("255.255.255.253").mask(30));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.248"),ppl7::IPAddress("255.255.255.249").mask(29));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.240"),ppl7::IPAddress("255.255.255.241").mask(28));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.224"),ppl7::IPAddress("255.255.255.225").mask(27));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.192"),ppl7::IPAddress("255.255.255.193").mask(26));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.128"),ppl7::IPAddress("255.255.255.129").mask(25));
	ASSERT_EQ(ppl7::IPAddress("255.255.255.0"),ppl7::IPAddress("255.255.255.1").mask(24));

	ASSERT_EQ(ppl7::IPAddress("255.255.254.0"),ppl7::IPAddress("255.255.255.255").mask(23));
	ASSERT_EQ(ppl7::IPAddress("255.255.252.0"),ppl7::IPAddress("255.255.253.255").mask(22));
	ASSERT_EQ(ppl7::IPAddress("255.255.248.0"),ppl7::IPAddress("255.255.255.255").mask(21));
	ASSERT_EQ(ppl7::IPAddress("255.255.240.0"),ppl7::IPAddress("255.255.255.255").mask(20));
	ASSERT_EQ(ppl7::IPAddress("255.255.224.0"),ppl7::IPAddress("255.255.255.255").mask(19));
	ASSERT_EQ(ppl7::IPAddress("255.255.192.0"),ppl7::IPAddress("255.255.255.255").mask(18));
	ASSERT_EQ(ppl7::IPAddress("255.255.128.0"),ppl7::IPAddress("255.255.255.255").mask(17));
	ASSERT_EQ(ppl7::IPAddress("255.255.0.0"),ppl7::IPAddress("255.255.255.255").mask(16));

	ASSERT_EQ(ppl7::IPAddress("255.254.0.0"),ppl7::IPAddress("255.255.255.255").mask(15));
	ASSERT_EQ(ppl7::IPAddress("255.252.0.0"),ppl7::IPAddress("255.255.255.255").mask(14));
	ASSERT_EQ(ppl7::IPAddress("255.248.0.0"),ppl7::IPAddress("255.255.255.255").mask(13));
	ASSERT_EQ(ppl7::IPAddress("255.240.0.0"),ppl7::IPAddress("255.255.255.255").mask(12));
	ASSERT_EQ(ppl7::IPAddress("255.224.0.0"),ppl7::IPAddress("255.255.255.255").mask(11));
	ASSERT_EQ(ppl7::IPAddress("255.192.0.0"),ppl7::IPAddress("255.255.255.255").mask(10));
	ASSERT_EQ(ppl7::IPAddress("255.128.0.0"),ppl7::IPAddress("255.255.255.255").mask(9));
	ASSERT_EQ(ppl7::IPAddress("255.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(8));

	ASSERT_EQ(ppl7::IPAddress("254.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(7));
	ASSERT_EQ(ppl7::IPAddress("252.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(6));
	ASSERT_EQ(ppl7::IPAddress("248.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(5));
	ASSERT_EQ(ppl7::IPAddress("240.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(4));
	ASSERT_EQ(ppl7::IPAddress("224.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(3));
	ASSERT_EQ(ppl7::IPAddress("192.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(2));
	ASSERT_EQ(ppl7::IPAddress("128.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(1));
	ASSERT_EQ(ppl7::IPAddress("0.0.0.0"),ppl7::IPAddress("255.255.255.255").mask(0));
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),ppl7::IPAddress("172.16.102.1").mask(22));

}

TEST_F(InetIPAddressTest, mask_v6) {
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(128));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(127));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffc"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(126));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fff8"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(125));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fff0"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(124));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffe0"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(123));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffc0"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(122));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(112));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(96));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(64));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(48));
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(32));
	ASSERT_EQ(ppl7::IPAddress("ffff:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(16));
	ASSERT_EQ(ppl7::IPAddress("ff00:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(8));
	ASSERT_EQ(ppl7::IPAddress("fe00:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(7));
	ASSERT_EQ(ppl7::IPAddress("f000:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(4));
	ASSERT_EQ(ppl7::IPAddress("e000:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(3));
	ASSERT_EQ(ppl7::IPAddress("c000:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(2));
	ASSERT_EQ(ppl7::IPAddress("8000:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(1));
	ASSERT_EQ(ppl7::IPAddress("0000:0000:0000:0000:0000:0000:0000:0000"),ppl7::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(0));
}


}	// EOF namespace

