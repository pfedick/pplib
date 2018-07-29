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
#ifdef _WIN32
    #include <winsock2.h>
	#include <Ws2tcpip.h>
	#include <windows.h>
namespace ppl7 { namespace compat {
	int inet_pton(int af, const char *src, void *dst);
} }
	using ppl7::compat::inet_pton;
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


namespace {

class InetSockAddrTest : public ::testing::Test {
	protected:
		InetSockAddrTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~InetSockAddrTest() {

	}
};


TEST_F(InetSockAddrTest, Constructor) {
	ASSERT_NO_THROW ({
		ppl7::SockAddr saddr;
	});
	/*
	printf ("sizeof(struct sockaddr): %d\n", sizeof(struct sockaddr));
	printf ("sizeof(struct sockaddr_in): %d\n", sizeof(struct sockaddr_in));
	printf ("sizeof(struct sockaddr_in6): %d\n", sizeof(struct sockaddr_in6));
	*/
}

TEST_F(InetSockAddrTest, setAddrFromIPv4Address) {
	ppl7::SockAddr saddr;
	ASSERT_NO_THROW ({
		saddr.setAddr(ppl7::IPAddress("172.16.102.1"),4711);
	});
	ASSERT_EQ(ppl7::IPAddress("172.16.102.1"),saddr.toIPAddress());
	ASSERT_EQ((int)4711,saddr.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in),saddr.size());
}

TEST_F(InetSockAddrTest, setAddrFromIPv6Address) {
	ppl7::SockAddr saddr;
	ASSERT_NO_THROW ({
		saddr.setAddr(ppl7::IPAddress("2001:dead:cafe::42"),4711);
	});
	ASSERT_EQ(ppl7::IPAddress("2001:dead:cafe::42"),saddr.toIPAddress());
	ASSERT_EQ((int)4711,saddr.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in6),saddr.size());
}

TEST_F(InetSockAddrTest, setAddrFromMemory) {
	struct sockaddr_in6 sin;
	const char *ipv6="2001:dead:cafe::42";
	memset(&sin,0,sizeof(sin));
	inet_pton(AF_INET6, ipv6, &sin.sin6_addr);
	sin.sin6_port=htons(4711);
	//sin.sin6_len=sizeof(sin);
	sin.sin6_family=AF_INET6;

	ppl7::SockAddr saddr;
	ASSERT_NO_THROW ({
		saddr.setAddr(&sin,sizeof(sin));
	});
	ASSERT_EQ(ppl7::IPAddress("2001:dead:cafe::42"),saddr.toIPAddress());
	ASSERT_EQ((int)4711,saddr.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in6),saddr.size());

	//ppl7::HexDump(saddr.addr(),saddr.size());
}

TEST_F(InetSockAddrTest, CopyConstructor) {
	ppl7::SockAddr saddr1(ppl7::IPAddress("2001:dead:cafe::42"),4711);
	ppl7::SockAddr saddr2(saddr1);
	ASSERT_EQ(ppl7::IPAddress("2001:dead:cafe::42"),saddr2.toIPAddress());
	ASSERT_EQ((int)4711,saddr2.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in6),saddr2.size());
	ASSERT_EQ((int)0,memcmp(saddr1.addr(),saddr2.addr(),saddr1.size()));
}

TEST_F(InetSockAddrTest, ConstructorWithIPAddressAndPort) {
	ppl7::SockAddr saddr(ppl7::IPAddress("2001:dead:cafe::42"),4711);
	ASSERT_EQ(ppl7::IPAddress("2001:dead:cafe::42"),saddr.toIPAddress());
	ASSERT_EQ((int)4711,saddr.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in6),saddr.size());
}


TEST_F(InetSockAddrTest, ConstructorWithMemoryAddress) {
	struct sockaddr_in6 sin;
	const char *ipv6="2001:dead:cafe::42";
	memset(&sin,0,sizeof(sin));
	inet_pton(AF_INET6, ipv6, &sin.sin6_addr);
	sin.sin6_port=htons(4711);
	//sin.sin6_len=sizeof(sin);
	sin.sin6_family=AF_INET6;

	ppl7::SockAddr saddr(&sin,sizeof(sin));
	ASSERT_EQ(ppl7::IPAddress("2001:dead:cafe::42"),saddr.toIPAddress());
	ASSERT_EQ((int)4711,saddr.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in6),saddr.size());
}


TEST_F(InetSockAddrTest, OperatorAssign) {
	ppl7::SockAddr saddr1(ppl7::IPAddress("2001:dead:cafe::42"),4711);
	ppl7::SockAddr saddr2;
	saddr2=saddr1;
	ASSERT_EQ(ppl7::IPAddress("2001:dead:cafe::42"),saddr2.toIPAddress());
	ASSERT_EQ((int)4711,saddr2.port());
	ASSERT_EQ((size_t)sizeof(sockaddr_in6),saddr2.size());
}



}	// EOF namespace


