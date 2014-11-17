/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
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

#define PPL6TESTSUITEMAIN
#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "../include/ppl6.h"
#include <gtest/gtest.h>
#include "ppl6-tests.h"

extern const char *wordlist;

ppl6::CArray Wordlist;
ppl6::CConfig *PPL6TestConfig=NULL;

void setDefaultConfigParams()
{
	PPL6TestConfig->CreateSection("tcpsocket");
	PPL6TestConfig->Add("echoserver","localhost");
	PPL6TestConfig->Add("unknownserver","unknown.server.pfp.de");
	PPL6TestConfig->Add("tcpserver_host","localhost");
	PPL6TestConfig->Add("tcpserver_port","50001");
}

void help()
{
	printf ("PPL6 Testsuite configuration options:\n"
			"-c CONFIG   Configuration-file for ppl6 testsuite\n"
			"\n"
			"Test-Framework options:\n");

}

int main (int argc, char**argv)
{
	const char *tmp;
	PPL6TestConfig=new ppl6::CConfig;
	if (ppl6::getargv(argc,argv,"-h")!=NULL || ppl6::getargv(argc,argv,"--help")!=NULL) help();
	if ((tmp=ppl6::getargv(argc,argv,"-c"))) {
		PPL6TestConfig->Load(tmp);
	} else {
		setDefaultConfigParams();
		PPL6TestConfig->Load("test.conf");
	}
	::testing::InitGoogleTest(&argc, argv);
	if (ppl6::getargv(argc,argv,"-h")!=NULL || ppl6::getargv(argc,argv,"--help")!=NULL) return 0;

	ppl6::PrintDebugTime ("Wortliste in String laden\n");
	ppl6::CString w(wordlist);
	Wordlist.Reserve(130000);
	ppl6::PrintDebugTime ("Wortliste in Array laden\n");
	Wordlist.Explode(w,"\n");
	ppl6::PrintDebugTime ("done\n");

	int ret=0;
	try {
		ret=RUN_ALL_TESTS();
	} catch (const ppl6::Exception &e) {
		printf ("ppl6::Exception: %s\n",e.what());
	} catch (...) {
		printf ("Unbekannte Exception\n");
	}
	delete PPL6TestConfig;
	PPL6TestConfig=NULL;
	ppl6::CleanupThreadData();
	return ret;
}
