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
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class InetTest : public ::testing::Test {
	protected:
	InetTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~InetTest() {

	}
};

TEST_F(InetTest, Idn2AceUtf8) {
	ASSERT_NO_THROW({
		ppl7::String idn="tästmé.de";
		//printf ("String: >>%s<<\n",(const char*)idn);
		ppl7::String ace=ppl7::Idn2Ace(idn);
		ASSERT_EQ(ppl7::String("xn--tstm-loa7a.de"),ace) << "Convertion failed";
	}
	);
}

TEST_F(InetTest, Ace2IdnUtf8) {
	ASSERT_NO_THROW({
		ppl7::String ace="xn--tstm-loa7a.de";
		ppl7::String idn=ppl7::Ace2Idn(ace);
		ASSERT_EQ(ppl7::String("tästmé.de"),idn) << "Convertion failed";
	}
	);
}

TEST_F(InetTest, Idn2AceWideString) {
	ASSERT_NO_THROW({
		ppl7::WideString idn=L"tästmé.de";
		ppl7::WideString ace=ppl7::Idn2Ace(idn);
		ASSERT_EQ(ppl7::WideString(L"xn--tstm-loa7a.de"),ace) << "Convertion failed";
	}
	);
}

TEST_F(InetTest, Ace2IdnWideString) {
	ASSERT_NO_THROW({
		ppl7::WideString ace=L"xn--tstm-loa7a.de";
		ppl7::WideString idn=ppl7::Ace2Idn(ace);
		ASSERT_EQ(ppl7::WideString(L"tästmé.de"),idn) << "Convertion failed";
	}
	);
}


}

