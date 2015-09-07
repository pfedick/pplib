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

#define PPL7TESTSUITEMAIN
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"

extern const char *wordlist;

ppl7::Array Wordlist;
ppl7::ConfigParser PPL7TestConfig;

void help()
{
	printf ("PPL7 Testsuite configuration options:\n"
			"-c CONFIG   Configuration-file for ppl7 testsuite\n"
			"\n"
			"Test-Framework options:\n");

}

void setDefaultConfigParams()
{
	printf ("   DEBUG 1\n");
	PPL7TestConfig.createSection("tcpsocket");
	printf ("   DEBUG 2\n");
	PPL7TestConfig.add("echoserver","localhost");
	printf ("   DEBUG 3\n");
	PPL7TestConfig.add("unknownserver","unknown.server.pfp.de");
	printf ("   DEBUG 4\n");
}

int main (int argc, char**argv)
{
	printf ("DEBUG 1\n");
	const char *tmp;
	if (ppl7::GetArgv(argc,argv,"-h")!=NULL || ppl7::GetArgv(argc,argv,"--help")!=NULL) help();
	if ((tmp=ppl7::GetArgv(argc,argv,"-c"))) {
		printf ("DEBUG 2\n");
		PPL7TestConfig.load(tmp);
	} else {
		printf ("DEBUG 3\n");
		setDefaultConfigParams();
	}
	printf ("DEBUG 4\n");
	::testing::InitGoogleTest(&argc, argv);
	if (ppl7::GetArgv(argc,argv,"-h")!=NULL || ppl7::GetArgv(argc,argv,"--help")!=NULL) return 0;

	printf ("DEBUG 5\n");

	ppl7::PrintDebugTime ("Wortliste in String laden\n");
	ppl7::String w(wordlist);
	Wordlist.reserve(130000);
	ppl7::PrintDebugTime ("Wortliste in Array laden\n");
	Wordlist.explode(w,"\n");
	ppl7::PrintDebugTime ("done\n");

	printf ("DEBUG 6\n");
	try {
		return RUN_ALL_TESTS();
	} catch (const ppl7::Exception &e) {
		printf ("ppl7::Exception: %s\n",e.what());
	} catch (...) {
		printf ("Unbekannte Exception\n");
	}

	return 1;
}
