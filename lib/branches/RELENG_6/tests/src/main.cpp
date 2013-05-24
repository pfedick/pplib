/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 653 $
 * $Date: 2013-05-19 10:57:32 +0200 (So, 19 Mai 2013) $
 * $Id: strings.cpp 653 2013-05-19 08:57:32Z pafe $
 * $URL: https://svn.code.sf.net/p/pplib/code/lib/branches/RELENG_6/tests/src/strings.cpp $
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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
#include "ppl6-tests.h"

extern const char *wordlist;

ppl6::CArray Wordlist;


int main (int argc, char**argv)
{
	::testing::InitGoogleTest(&argc, argv);
	ppl6::PrintDebugTime ("Wortliste in String laden\n");
	ppl6::CString w(wordlist);
	Wordlist.Reserve(130000);
	ppl6::PrintDebugTime ("Wortliste in Array laden\n");
	Wordlist.Explode(w,"\n");
	ppl6::PrintDebugTime ("done\n");

	try {
		return RUN_ALL_TESTS();
	} catch (const ppl6::Exception &e) {
		printf ("ppl6::Exception: %s\n",e.what());
	} catch (...) {
		printf ("Unbekannte Exception\n");
	}

	ppl6::CleanupThreadData();
	return 1;
}
