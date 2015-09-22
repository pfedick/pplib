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
#include <list>
#include "ppl7-tests.h"

namespace {

class SockAddr : public ::testing::Test {
	protected:
		SockAddr() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~SockAddr() {

	}
};

TEST_F(SockAddr, toString) {
	/*
	ASSERT_NO_THROW ({
		try {
			std::list<ppl7::IPAddress> result;
			size_t num=ppl7::GetHostByName("singleipv4.ppl.pfp.de",result,ppl7::af_all);
			ASSERT_EQ((size_t)1,num) << "Unexpected return value";
			ASSERT_EQ((size_t)1,result.size()) << "Unexpected number of results";
			ppl7::IPAddress adr;
			adr=result.front();
			ASSERT_EQ(ppl7::String("192.168.0.1"),adr.ip) << "Unexpected IP-Address";
			ASSERT_EQ(ppl7::String("singleipv4.ppl.pfp.de"),adr.name) << "Unexpected Name";
		} catch (ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
	*/

}

TEST_F(SockAddr, fromIPv4StringToString) {
	ASSERT_NO_THROW ({
		ppl7::SockAddr adr=ppl7::SockAddr::fromString("127.0.0.1");
		ASSERT_EQ((size_t)16,adr.size()) << "Unexpected Size of IP-Address";
		//ppl7::HexDump(adr.addr(),adr.size());
		ASSERT_EQ(ppl7::String("127.0.0.1"),adr.toString()) << "Unexpected IP-Address";

	});

}

TEST_F(SockAddr, fromIPv6StringToString) {
	ASSERT_NO_THROW ({
		ppl7::SockAddr adr=ppl7::SockAddr::fromString("2a01:4f8:202:109a::2");
		ASSERT_EQ((size_t)28,adr.size()) << "Unexpected Size of IP-Address";
		//ppl7::HexDump(adr.addr(),adr.size());
		ASSERT_EQ(ppl7::String("2a01:4f8:202:109a::2"),adr.toString()) << "Unexpected IP-Address";
	});

}


}	// EOF namespace

