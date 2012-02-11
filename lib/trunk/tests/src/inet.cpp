/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 89 $
 * $Date: 2011-05-07 12:03:12 +0200 (Sa, 07 Mai 2011) $
 * $Id: array.cpp 89 2011-05-07 10:03:12Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
#include <list>

namespace {

// The fixture for testing class Foo.
class InetTest : public ::testing::Test {
	protected:
	InetTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~InetTest() {

	}
};

TEST_F(InetTest, Idn2Ace) {
	ASSERT_NO_THROW({
		ppl7::String idn="tästmé.de";
		ppl7::String ace=ppl7::Idn2Ace(idn);
		ASSERT_EQ(ppl7::String(L"xn--tstm-loa7a.de"),ace) << "Convertion failed";
	}
	);
}

TEST_F(InetTest, Ace2Idn) {
	ASSERT_NO_THROW({
		ppl7::String ace="xn--tstm-loa7a.de";
		ppl7::String idn=ppl7::Ace2Idn(ace);
		ASSERT_EQ(ppl7::String(L"tästmé.de"),idn) << "Convertion failed";
	}
	);
}


class Resolver : public ::testing::Test {
	protected:
	Resolver() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~Resolver() {

	}
};

/*
 * ;
; PPL7 Testsuite-Adressen
;
singleipv4.ppl      IN  A 192.168.0.1
threeipv4.ppl       IN  A 192.168.0.1
                    IN  A 192.168.10.4
                    IN  A 192.168.200.56
singleipv6.ppl      IN  AAAA 2001:470:1f0b:3e4::1
threeipv6.ppl       IN  AAAA 2001:470:1f0b:3e4::1
                    IN  AAAA 2001:470:1f0a:3e4::a000
                    IN  AAAA 2001:470:1f0a:3e4::99:1b4c
mixedv4v6.ppl       IN  A 192.168.16.76
                    IN  A 192.168.22.98
                    IN  AAAA 2001:470:1f0a:3e4::a016
                    IN  AAAA 2001:470:1f0a:3e4::99:1b93
 *
 */

TEST_F(Resolver, SingleIPv4_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv4.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::String(L"192.168.0.1"),adr.ip) << "Unexpected IP-Address";
			ASSERT_EQ(ppl7::String(L"singleipv4.ppl.pfp.de"),adr.name) << "Unexpected Name";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, SingleIPv4_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv4.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::String(L"192.168.0.1"),adr.ip) << "Unexpected IP-Address";
			ASSERT_EQ(ppl7::String(L"singleipv4.ppl.pfp.de"),adr.name) << "Unexpected Name";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, SingleIPv4_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv4.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)0,num) << "Unexpected return value";
			ASSERT_EQ((size_t)0,result.size()) << "Unexpected number of results";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, ThreeIPv4_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv4.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"192.168.0.1"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"192.168.10.4"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"192.168.200.56"),*lit) << "Unexpected IP-Address";

		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, ThreeIPv4_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv4.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"192.168.0.1"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"192.168.10.4"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"192.168.200.56"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, ThreeIPv4_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv4.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)0,num) << "Unexpected return value";
			ASSERT_EQ((size_t)0,result.size()) << "Unexpected number of results";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, SingleIPv6_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv6.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::String(L"2001:470:1f0b:3e4::1"),adr.ip) << "Unexpected IP-Address";
			ASSERT_EQ(ppl7::String(L"singleipv6.ppl.pfp.de"),adr.name) << "Unexpected Name";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, SingleIPv6_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv6.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)0,num) << "Unexpected return value";
			ASSERT_EQ((size_t)0,result.size()) << "Unexpected number of results";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, SingleIPv6_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv6.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::String(L"2001:470:1f0b:3e4::1"),adr.ip) << "Unexpected IP-Address";
			ASSERT_EQ(ppl7::String(L"singleipv6.ppl.pfp.de"),adr.name) << "Unexpected Name";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, ThreeIPv6_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv6.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::99:1b4c"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::a000"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0b:3e4::1"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, ThreeIPv6_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv6.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)0,num) << "Unexpected return value";
			ASSERT_EQ((size_t)0,result.size()) << "Unexpected number of results";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, ThreeIPv6_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv6.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::99:1b4c"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::a000"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0b:3e4::1"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, MixedIPv4v6_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("mixedv4v6.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)4,num) << "Unexpected return value";
			ASSERT_EQ((size_t)4,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"192.168.16.76"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"192.168.22.98"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::99:1b93"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::a016"),*lit) << "Unexpected IP-Address";

		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, MixedIPv4v6_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("mixedv4v6.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)2,num) << "Unexpected return value";
			ASSERT_EQ((size_t)2,result.size()) << "Unexpected number of results";

			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"192.168.16.76"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"192.168.22.98"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, MixedIPv4v6_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("mixedv4v6.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)2,num) << "Unexpected return value";
			ASSERT_EQ((size_t)2,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::String> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(it->ip);

			std::set<ppl7::String>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::99:1b93"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String(L"2001:470:1f0a:3e4::a016"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

