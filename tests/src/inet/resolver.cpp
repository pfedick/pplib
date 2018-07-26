/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
#include <list>
#include "ppl7-tests.h"

namespace {

class GetHostByName : public ::testing::Test {
	protected:
		GetHostByName() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~GetHostByName() {

	}
};



class Resolver : public ::testing::Test {
	protected:
	Resolver() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
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


TEST_F(GetHostByName, SingleIPv4_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv4.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::IPAddress("192.168.0.1"),adr) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, SingleIPv4_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv4.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::IPAddress("192.168.0.1"),adr) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, SingleIPv4_af_inet6) {
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

TEST_F(GetHostByName, ThreeIPv4_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv4.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::IPAddress("192.168.0.1"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("192.168.10.4"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("192.168.200.56"),*lit) << "Unexpected IP-Address";

		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, ThreeIPv4_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv4.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::IPAddress("192.168.0.1"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("192.168.10.4"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("192.168.200.56"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, ThreeIPv4_af_inet6) {
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

TEST_F(GetHostByName, SingleIPv6_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv6.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0b:3e4::1"),adr) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, SingleIPv6_af_inet) {
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

TEST_F(GetHostByName, SingleIPv6_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv6.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0b:3e4::1"),adr) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, ThreeIPv6_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv6.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0a:3e4::a000"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0a:3e4::99:1b4c"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0b:3e4::1"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, ThreeIPv6_af_inet) {
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

TEST_F(GetHostByName, ThreeIPv6_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("threeipv6.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)3,num) << "Unexpected return value";
			ASSERT_EQ((size_t)3,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::String("2001:470:1f0a:3e4::a000"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String("2001:470:1f0a:3e4::99:1b4c"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::String("2001:470:1f0b:3e4::1"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, MixedIPv4v6_af_all) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("mixedv4v6.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)4,num) << "Unexpected return value";
			ASSERT_EQ((size_t)4,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::IPAddress("192.168.16.76"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("192.168.22.98"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0a:3e4::a016"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0a:3e4::99:1b93"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, MixedIPv4v6_af_inet) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("mixedv4v6.ppl.pfp.de",result,ppl7::af_inet);
			ASSERT_EQ((size_t)2,num) << "Unexpected return value";
			ASSERT_EQ((size_t)2,result.size()) << "Unexpected number of results";

			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::IPAddress("192.168.16.76"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("192.168.22.98"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(GetHostByName, MixedIPv4v6_af_inet6) {
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("mixedv4v6.ppl.pfp.de",result,ppl7::af_inet6);
			ASSERT_EQ((size_t)2,num) << "Unexpected return value";
			ASSERT_EQ((size_t)2,result.size()) << "Unexpected number of results";
			// Sort addresses for better comparing
			std::set<ppl7::IPAddress> sorted;
			std::list<ppl7::IPAddress>::iterator it;
			for (it=result.begin();it!=result.end();it++) sorted.insert(*it);

			std::set<ppl7::IPAddress>::iterator lit;
			lit=sorted.begin();
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0a:3e4::a016"),*lit) << "Unexpected IP-Address";
			lit++;
			ASSERT_EQ(ppl7::IPAddress("2001:470:1f0a:3e4::99:1b93"),*lit) << "Unexpected IP-Address";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(Resolver, query_NS) {
	ppl7::Array a;
	ASSERT_NO_THROW ({
		ppl7::Resolver::query(a,"de.",ppl7::Resolver::NS);
	});
	a.sortUnique();
	EXPECT_EQ((size_t)6,a.size()) << "Unexpected number of result records";
	EXPECT_EQ(ppl7::String("a.nic.de"),a[0]) << "Unexpected value of result record 0";
	EXPECT_EQ(ppl7::String("f.nic.de"),a[1]) << "Unexpected value of result record 1";
	EXPECT_EQ(ppl7::String("l.de.net"),a[2]) << "Unexpected value of result record 2";
	EXPECT_EQ(ppl7::String("n.de.net"),a[3]) << "Unexpected value of result record 3";
	EXPECT_EQ(ppl7::String("s.de.net"),a[4]) << "Unexpected value of result record 4";
	EXPECT_EQ(ppl7::String("z.nic.de"),a[5]) << "Unexpected value of result record 5";


}

TEST_F(Resolver, query_A_singleResult) {
	ppl7::Array a;
	ASSERT_NO_THROW ({
		ppl7::Resolver::query(a,"singleipv4.ppl.pfp.de.",ppl7::Resolver::A);
	});
	a.sortUnique();
	EXPECT_EQ((size_t)1,a.size()) << "Unexpected number of result records";
	EXPECT_EQ(ppl7::String("192.168.0.1"),a[0]) << "Unexpected value of result record 0";
}

TEST_F(Resolver, query_A_ThreeResult) {
	ppl7::Array a;
	ASSERT_NO_THROW ({
		ppl7::Resolver::query(a,"threeipv4.ppl.pfp.de.",ppl7::Resolver::A);
	});
	a.sortUnique();
	EXPECT_EQ((size_t)3,a.size()) << "Unexpected number of result records";
	EXPECT_EQ(ppl7::String("192.168.0.1"),a[0]) << "Unexpected value of result record 0";
	EXPECT_EQ(ppl7::String("192.168.10.4"),a[1]) << "Unexpected value of result record 1";
	EXPECT_EQ(ppl7::String("192.168.200.56"),a[2]) << "Unexpected value of result record 2";
}

TEST_F(Resolver, query_AAAA_singleResult) {
	ppl7::Array a;
	ASSERT_NO_THROW ({
		ppl7::Resolver::query(a,"singleipv6.ppl.pfp.de.",ppl7::Resolver::AAAA);
	});
	a.sortUnique();
	EXPECT_EQ((size_t)1,a.size()) << "Unexpected number of result records";
	EXPECT_EQ(ppl7::String("2001:470:1f0b:3e4::1"),a[0]) << "Unexpected value of result record 0";
}

TEST_F(Resolver, query_AAAA_ThreeResult) {
	ppl7::Array a;
	ASSERT_NO_THROW ({
		ppl7::Resolver::query(a,"threeipv6.ppl.pfp.de.",ppl7::Resolver::AAAA);
	});
	a.sortUnique();
	EXPECT_EQ((size_t)3,a.size()) << "Unexpected number of result records";
	EXPECT_EQ(ppl7::String("2001:470:1f0a:3e4::99:1b4c"),a[0]) << "Unexpected value of result record 0";
	EXPECT_EQ(ppl7::String("2001:470:1f0a:3e4::a000"),a[1]) << "Unexpected value of result record 1";
	EXPECT_EQ(ppl7::String("2001:470:1f0b:3e4::1"),a[2]) << "Unexpected value of result record 2";
}

TEST_F(Resolver, query_A_noResult) {
	ppl7::Array a;
	ASSERT_THROW({
			ppl7::Resolver::query(a,"unknown.ppl.pfp.de.",ppl7::Resolver::A);
		},ppl7::HostNotFoundException);
	EXPECT_EQ((size_t)0,a.size()) << "Unexpected number of result records";
}

TEST_F(Resolver, query_NS_EmptyResult) {
	ppl7::Array a;
	ASSERT_THROW({
			ppl7::Resolver::query(a,"ppl.pfp.de.",ppl7::Resolver::NS);
		},ppl7::NoResultException);
	EXPECT_EQ((size_t)0,a.size()) << "Unexpected number of result records";
}

TEST_F(Resolver, query_InvalidQuery) {
	ppl7::Array a;
	ASSERT_THROW({
			ppl7::Resolver::query(a,"don't L:ke it!...",ppl7::Resolver::A);
		},ppl7::QueryFailedException);
	EXPECT_EQ((size_t)0,a.size()) << "Unexpected number of result records";
}

/*
TEST_F(Resolver, query_SOA) {
	ppl7::Array a;
	ASSERT_NO_THROW ({
		ppl7::Resolver::query(a,"de.",ppl7::Resolver::SOA);
	});
	a.sortUnique();
	EXPECT_EQ((size_t)1,a.size()) << "Unexpected number of result records";
	EXPECT_EQ(ppl7::String("2001:470:1f0a:3e4::99:1b4c"),a[0]) << "Unexpected value of result record 0";
}
*/

}

