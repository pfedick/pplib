/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 837 $
 * $Date: 2014-01-23 08:31:34 +0100 (Do, 23 Jan 2014) $
 * $Id: file.cpp 837 2014-01-23 07:31:34Z pafe $
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include <gtest/gtest.h>
#include "ppl7-tests.h"

namespace {

// The fixture for testing class Foo.
class TimeTest : public ::testing::Test {
	protected:
	TimeTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen: LC_CTYPE\n");
			throw std::exception();
		}
		if (setlocale(LC_TIME,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen: LC_TIME\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~TimeTest() {

	}
};


TEST_F(TimeTest, MkTime_withIso8601String_withoutTimezone) {
	EXPECT_EQ((ppl7::ppl_time_t)1378388624,ppl7::MkTime(ppl7::String("2013-09-05T14:43:44"))) << "MkTime returns unexpected value";
}

TEST_F(TimeTest, MkTime_withIso8601String_withZeroTimeoffset) {
	EXPECT_EQ((ppl7::ppl_time_t)1378388624,ppl7::MkTime(ppl7::String("2013-09-05T14:43:44+00:00"))) << "MkTime returns unexpected value";
	EXPECT_EQ((ppl7::ppl_time_t)1378388624,ppl7::MkTime(ppl7::String("2013-09-05T14:43:44-00:00"))) << "MkTime returns unexpected value";
}

TEST_F(TimeTest, MkTime_withIso8601String_withPositiveTimeoffset) {
	EXPECT_EQ((ppl7::ppl_time_t)1378388624,ppl7::MkTime(ppl7::String("2013-09-05T16:43:44+02:00"))) << "MkTime returns unexpected value";
	EXPECT_EQ((ppl7::ppl_time_t)1378388624,ppl7::MkTime(ppl7::String("2013-09-05T12:43:44-02:00"))) << "MkTime returns unexpected value";
}

TEST_F(TimeTest, MkTime_withIso8601String_withNegativeTimeoffset) {
	EXPECT_EQ((ppl7::ppl_time_t)1378388624,ppl7::MkTime(ppl7::String("2013-09-05T12:43:44-02:00"))) << "MkTime returns unexpected value";
}




}


