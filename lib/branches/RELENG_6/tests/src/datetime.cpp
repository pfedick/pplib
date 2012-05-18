/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 89 $
 * $Date: 2011-05-07 12:03:12 +0200 (Sa, 07 Mai 2011) $
 * $Id: datetime.cpp 89 2011-05-07 10:03:12Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/tests/src/datetime.cpp $
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
#include <ppl6.h>
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class DateTimeTest : public ::testing::Test {
	protected:
	DateTimeTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~DateTimeTest() {

	}
};

TEST_F(DateTimeTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1;
		ASSERT_TRUE(d1.isEmpty()==true) << "Class is not Empty";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithDate) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 00:00:00"),d1.get()) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithDateTime) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithDateTimeMsec1) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.1");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.001000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithDateTimeMsec2) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.15");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.015000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithDateTimeMsec3) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithDateTimeUsec4) {
	ASSERT_THROW(ppl6::CDateTime d1("2012-05-18 11:50:11.1594"),ppl6::IllegalArgumentException);
}

TEST_F(DateTimeTest, ConstructorWithDateTimeUsec5) {
	ASSERT_THROW(ppl6::CDateTime d1("2012-05-18 11:50:11.15947"),ppl6::IllegalArgumentException);
}


TEST_F(DateTimeTest, ConstructorWithDateTimeUsec6) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159473");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeMsec1) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.1");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.001000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeMsec2) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.15");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.015000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeMsec3) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeUsec6) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159473");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeUsec6TZ) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159473+0200");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithISO8601DateTimeUsec6TZ1) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159473-0200");
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18"),d1.getDate()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11"),d1.get()) << "Unexpected date";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}


TEST_F(DateTimeTest, CopyConstructor) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159473");
		ppl6::CDateTime d2=d1;
		ASSERT_TRUE(d2.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ConstructorWithTime_t) {
	ppluint64 t=1337335350; // 2012-05-18 12:02:30
	ASSERT_NO_THROW({
		ppl6::CDateTime d1(t);
		ASSERT_TRUE(d1.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18 12:02:30.000000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}


TEST_F(DateTimeTest, Assignment) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159473");
		ppl6::CDateTime d2;
		d2=d1;
		ASSERT_TRUE(d2.notEmpty()==true) << "Class is empty";
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d2.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}


TEST_F(DateTimeTest, getLongInt) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159473");
		ppluint64 t=d1.longInt();
		ASSERT_EQ(64679514611159473,t) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, setLongInt) {
	ppluint64 v=64679514611159473;
	ASSERT_NO_THROW({
		ppl6::CDateTime d1;
		d1.setLongInt(v);
		ASSERT_EQ(ppl6::CString("2012-05-18 11:50:11.159473"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, getISO8601) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159473");
		ASSERT_EQ(ppl6::CString("2012-05-18T11:50:11+02:00"),d1.getISO8601()) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, getISO8601withMsec) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159473");
		ASSERT_EQ(ppl6::CString("2012-05-18T11:50:11.159+02:00"),d1.getISO8601withMsec()) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, getISO8601withMsecNoRoundUp) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18 11:50:11.159999");
		ASSERT_EQ(ppl6::CString("2012-05-18T11:50:11.159+02:00"),d1.getISO8601withMsec()) << "Unexpected date";
	}
	);
}


TEST_F(DateTimeTest, ReassignmentWithoutMsec) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159473");
		d1.set("2011-11-23T22:14:39");
		ASSERT_EQ(ppl6::CString("2011-11-23 22:14:39.000000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, ReassignmentWithMsec) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159473");
		d1.set("2011-11-23T22:14:39.946210");
		ASSERT_EQ(ppl6::CString("2011-11-23 22:14:39.946210"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}

TEST_F(DateTimeTest, clear) {
	ASSERT_NO_THROW({
		ppl6::CDateTime d1("2012-05-18T11:50:11.159473");
		d1.clear();
		ASSERT_EQ(ppl6::CString("0000-00-00 00:00:00.000000"),d1.get("%Y-%m-%d %H:%M:%S.%u")) << "Unexpected date";
	}
	);
}



}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

