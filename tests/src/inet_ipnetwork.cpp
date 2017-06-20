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

namespace {

class InetIPNetworkTest : public ::testing::Test {
	protected:
		InetIPNetworkTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~InetIPNetworkTest() {

	}
};


TEST_F(InetIPNetworkTest, Constructor) {
	ASSERT_NO_THROW ({
		ppl7::IPNetwork net;
	});
}


TEST_F(InetIPNetworkTest, ConstructorWithString) {
	ppl7::IPNetwork net(ppl7::String("172.16.102.66/22"));
	ASSERT_EQ(ppl7::IPAddress::IPv4, net.family());
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),net.addr());
	ASSERT_EQ((int)22,net.prefixlen());
}

TEST_F(InetIPNetworkTest, CopyConstructor) {
	ppl7::IPNetwork net1("172.16.102.66/22");
	ppl7::IPNetwork net2(net1);
	ASSERT_EQ(ppl7::IPAddress::IPv4, net2.family());
	ASSERT_EQ(22, net2.prefixlen());
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),net2.addr());
}

TEST_F(InetIPNetworkTest, setFromString) {
	ppl7::IPNetwork net;
	ASSERT_NO_THROW ({
		net.set("172.16.102.13/22");
	});
	ASSERT_EQ(ppl7::IPAddress::IPv4, net.family());
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),net.addr());
	ASSERT_EQ((int)22,net.prefixlen());

	ASSERT_NO_THROW ({
		net.set("2001:678:2a::53/64");
	});
	ASSERT_EQ(ppl7::IPAddress::IPv6, net.family());
	ASSERT_EQ(ppl7::IPAddress("2001:678:2a::"),net.addr());
	ASSERT_EQ((int)64,net.prefixlen());

	ASSERT_THROW({
		net.set("257.3.4.1/24");
	},ppl7::InvalidIpAddressException);
	ASSERT_THROW({
		net.set("194.3.4.1/33");
	},ppl7::InvalidNetmaskOrPrefixlenException);
	ASSERT_THROW({
		net.set("194.3.4.1/255.255.255.0");
	},ppl7::InvalidNetmaskOrPrefixlenException);

	ASSERT_THROW({
		net.set("194.3.4.1");
	},ppl7::InvalidNetworkAddressException);
}

TEST_F(InetIPNetworkTest, setFromIPAddressAndPrefixlen) {
	ppl7::IPAddress adr("172.16.102.66");
	ppl7::IPNetwork net;
	net.set(adr,22);
	ASSERT_EQ(ppl7::IPAddress::IPv4, net.family());
	ASSERT_EQ(22, net.prefixlen());
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),net.addr());

	ASSERT_THROW({
		net.set(adr,33);
	},ppl7::InvalidNetmaskOrPrefixlenException);

	ASSERT_THROW({
		net.set(ppl7::IPAddress(),24);
	},ppl7::InvalidIpAddressException);
}

TEST_F(InetIPNetworkTest, OperatorSetWithNetwork) {
	ppl7::IPNetwork net1("172.16.102.66/22");
	ppl7::IPNetwork net2;
	net2=net1;
	ASSERT_EQ(ppl7::IPAddress::IPv4, net2.family());
	ASSERT_EQ(22, net2.prefixlen());
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),net2.addr());
}

TEST_F(InetIPNetworkTest, OperatorSetWithString) {
	ppl7::String net1("172.16.102.66/22");
	ppl7::IPNetwork net2;
	net2=net1;
	ASSERT_EQ(ppl7::IPAddress::IPv4, net2.family());
	ASSERT_EQ(22, net2.prefixlen());
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),net2.addr());
}


TEST_F(InetIPNetworkTest, toString) {
	ASSERT_EQ(ppl7::String("172.16.100.0/22"), ppl7::IPNetwork("172.16.102.54/22").toString());
	ASSERT_EQ(ppl7::String("2001:678:2a::/64"), ppl7::IPNetwork("2001:678:2A::53/64").toString());
	ASSERT_EQ(ppl7::String("2001:678:2a::/64"), (ppl7::String)ppl7::IPNetwork("2001:678:2A::53/64"));
	ASSERT_THROW({
		ppl7::IPNetwork().toString();
	},ppl7::InvalidNetworkAddressException);
}

TEST_F(InetIPNetworkTest, prefixlen) {
	ppl7::IPNetwork net("172.16.102.54/22");
	ASSERT_EQ((int) 22, net.prefixlen());
	ASSERT_THROW({
		ppl7::IPNetwork().prefixlen();
	},ppl7::InvalidNetworkAddressException);
}


TEST_F(InetIPNetworkTest, netmask) {
	ppl7::IPNetwork net;
	net.set("172.16.102.1/22");
	ASSERT_EQ(ppl7::String("172.16.100.0/22"),net.toString());
	ASSERT_EQ(ppl7::IPAddress("255.255.252.0"),net.netmask());
	net.set("2a01:4f8:202:109a::2/64");
	ASSERT_EQ(ppl7::IPAddress("ffff:ffff:ffff:ffff::"),net.netmask());
	ASSERT_THROW({
			ppl7::IPNetwork().netmask();
	},ppl7::InvalidNetworkAddressException);
}

TEST_F(InetIPNetworkTest, addr) {
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),ppl7::IPNetwork("172.16.102.54/22").addr());
	ASSERT_EQ(ppl7::IPAddress("2a01:4f8:202:109a::"),ppl7::IPNetwork("2a01:4f8:202:109a::2/64").addr());
	ASSERT_THROW({
		ppl7::IPNetwork().addr();
	},ppl7::InvalidNetworkAddressException);
}

TEST_F(InetIPNetworkTest, first) {
	ASSERT_EQ(ppl7::IPAddress("172.16.100.0"),ppl7::IPNetwork("172.16.102.54/22").first());
	ASSERT_EQ(ppl7::IPAddress("2a01:4f8:202:109a::"),ppl7::IPNetwork("2a01:4f8:202:109a::2/64").first());
	ASSERT_THROW({
		ppl7::IPNetwork().first();
	},ppl7::InvalidNetworkAddressException);
}

TEST_F(InetIPNetworkTest, last) {
	ASSERT_EQ(ppl7::IPAddress("172.16.103.255"),ppl7::IPNetwork("172.16.102.54/22").last());
	ASSERT_EQ(ppl7::IPAddress("2a01:4f8:202:109a:ffff:ffff:ffff:ffff"),ppl7::IPNetwork("2a01:4f8:202:109a::2/64").last());
	ASSERT_THROW({
		ppl7::IPNetwork().last();
	},ppl7::InvalidNetworkAddressException);
}

TEST_F(InetIPNetworkTest, contains) {
	ASSERT_TRUE(ppl7::IPNetwork("172.16.102.54/22").contains(ppl7::IPAddress("172.16.103.255")));
	ASSERT_TRUE(ppl7::IPNetwork("172.16.100.0/22").contains(ppl7::IPAddress("172.16.100.0")));
	ASSERT_FALSE(ppl7::IPNetwork("172.16.100.0/22").contains(ppl7::IPAddress("172.16.104.0")));
	ASSERT_FALSE(ppl7::IPNetwork("172.16.100.0/22").contains(ppl7::IPAddress("172.16.99.255")));
	ASSERT_TRUE(ppl7::IPNetwork("2a01:4f8:202::/48").contains(ppl7::IPAddress("2a01:4f8:202::")));
	ASSERT_TRUE(ppl7::IPNetwork("2a01:4f8:202::/48").contains(ppl7::IPAddress("2a01:4f8:202:ffff:ffff:ffff:ffff:ffff")));
	ASSERT_FALSE(ppl7::IPNetwork("2a01:4f8:202::/48").contains(ppl7::IPAddress("2a01:4f8:203::")));
	ASSERT_FALSE(ppl7::IPNetwork("2a01:4f8:202::/48").contains(ppl7::IPAddress("2a01:4f8:201:ffff:ffff:ffff:ffff:ffff")));
}


TEST_F(InetIPNetworkTest, compare) {
	ppl7::IPNetwork net1("172.16.100.0/22");
	ppl7::IPNetwork net2("2001:678:2A::/64");
	ppl7::IPNetwork net3("172.16.103.0/24");
	ppl7::IPNetwork net4("172.16.99.0/21");
	ppl7::IPNetwork net5("172.16.100.0/24");

	ASSERT_TRUE(net1.compare(net2)<0);
	ASSERT_TRUE(net1.compare(net1)==0);
	ASSERT_TRUE(net3.compare(net1)>0);
	ASSERT_TRUE(net4.compare(net1)<0);
	ASSERT_TRUE(net1.compare(net5)>0);
}

TEST_F(InetIPNetworkTest, operator_LT) {
	ppl7::IPNetwork net1("194.77.8.1/24");
	ppl7::IPNetwork net2("195.77.9.2/24");

	ASSERT_TRUE(net1<net2);
	ASSERT_FALSE(net2<net1);
}

TEST_F(InetIPNetworkTest, operator_LTEQ) {
	ppl7::IPNetwork adr1("194.77.8.1/24");
	ppl7::IPNetwork adr2("195.77.9.2/24");
	ppl7::IPNetwork adr3("194.77.8.0/24");

	ASSERT_TRUE(adr1<=adr2);
	ASSERT_TRUE(adr1<=adr3);
	ASSERT_FALSE(adr2<=adr1);
}

TEST_F(InetIPNetworkTest, operator_EQ) {
	ppl7::IPNetwork adr1("194.77.8.1/24");
	ppl7::IPNetwork adr2("195.77.9.2/24");
	ppl7::IPNetwork adr3("194.77.8.1/24");
	ASSERT_TRUE(adr1==adr3);
	ASSERT_FALSE(adr1==adr2);
}

TEST_F(InetIPNetworkTest, operator_NE) {
	ppl7::IPNetwork adr1("194.77.8.1/24");
	ppl7::IPNetwork adr2("195.77.9.2/24");
	ppl7::IPNetwork adr3("194.77.8.1/24");
	ASSERT_TRUE(adr1!=adr2);
	ASSERT_FALSE(adr1!=adr3);
}

TEST_F(InetIPNetworkTest, operator_GT) {
	ppl7::IPNetwork adr1("194.77.8.1/24");
	ppl7::IPNetwork adr2("195.77.9.2/24");

	ASSERT_TRUE(adr2>adr1);
	ASSERT_FALSE(adr1>adr2);
}

TEST_F(InetIPNetworkTest, operator_GTEQ) {
	ppl7::IPNetwork adr1("194.77.8.1/24");
	ppl7::IPNetwork adr2("195.77.9.2/24");
	ppl7::IPNetwork adr3("194.77.8.1/24");

	ASSERT_TRUE(adr2>=adr1);
	ASSERT_TRUE(adr3>=adr1);
	ASSERT_FALSE(adr1>=adr2);
}

TEST_F(InetIPNetworkTest, sortIPNetworks) {
	std::set<ppl7::IPNetwork> list;
	list.insert(ppl7::IPNetwork("194.2.3.4/24"));
	list.insert(ppl7::IPNetwork("2001:678:2A::53/48"));
	list.insert(ppl7::IPNetwork("1.2.3.4/22"));
	list.insert(ppl7::IPNetwork("255.0.0.0/24"));
	list.insert(ppl7::IPNetwork("2001:678:2::1/64"));
	list.insert(ppl7::IPNetwork("66.7.1.0/25"));
	list.insert(ppl7::IPNetwork("2002:dead:2::1/48"));

	ASSERT_EQ((size_t)7,list.size());

	std::set<ppl7::IPNetwork>::const_iterator it;
	it=list.begin();
	ASSERT_EQ(ppl7::IPNetwork("1.2.3.4/22"),(*it++));
	ASSERT_EQ(ppl7::IPNetwork("66.7.1.0/25"),(*it++));
	ASSERT_EQ(ppl7::IPNetwork("194.2.3.4/24"),(*it++));
	ASSERT_EQ(ppl7::IPNetwork("255.0.0.0/24"),(*it++));
	ASSERT_EQ(ppl7::IPNetwork("2001:678:2::1/64"),(*it++));
	ASSERT_EQ(ppl7::IPNetwork("2001:678:2A::53/48"),(*it++));
	ASSERT_EQ(ppl7::IPNetwork("2002:dead:2::1/48"),(*it++));
}





}	// EOF namespace


