/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 108 $
 * $Date: 2011-05-20 16:55:16 +0200 (Fr, 20. Mai 2011) $
 * $Id: file.cpp 108 2011-05-20 14:55:16Z pafe $
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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/ppl7.h"
#include <gtest/gtest.h>

namespace {

// The fixture for testing class Foo.
class DirTest : public ::testing::Test {
	protected:
	DirTest() {
		if (setlocale(LC_CTYPE,"de_DE.UTF-8")==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl7::String::setGlobalEncoding("UTF-8");
	}
	virtual ~DirTest() {

	}
};


TEST_F(DirTest, ConstructorSimple) {
	ASSERT_NO_THROW({
		ppl7::Dir d1;
	}
	);
}

TEST_F(DirTest, ConstructorWithDir) {
	ASSERT_NO_THROW({
		ppl7::Dir d1("src");
	}
	);
}

TEST_F(DirTest, print) {
	ppl7::Dir d1("src");
	ASSERT_NO_THROW({
		d1.print();
	}
	);
}

TEST_F(DirTest, resortByFilenameIgnoreCase) {
	ppl7::Dir d1("src");
	d1.resort(ppl7::Dir::SORT_FILENAME);
	ASSERT_NO_THROW({
		d1.print();
	}
	);
}

TEST_F(DirTest, resortByMTime) {
	ppl7::Dir d1("src");
	d1.resort(ppl7::Dir::SORT_MTIME);
	ASSERT_NO_THROW({
		d1.print();
	}
	);
}

TEST_F(DirTest, resortBySize) {
	ppl7::Dir d1("src");
	d1.resort(ppl7::Dir::SORT_SIZE);
	ASSERT_NO_THROW({
		d1.print();
	}
	);
}



}


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

