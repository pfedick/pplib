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
#include <list>
#include <ppl7.h>
#include "ppl7-tests.h"

extern "C" {
	int A_strlen (const char * s);
}

extern const char *wordlist;

ppl7::Array Wordlist;
ppl7::ConfigParser PPL7TestConfig;
ppl7::AssocArray TestAssocArray;

void create_empty_strings()
{
	std::list<ppl7::String> slist;
	for (int i=0;i<10000000;i++) {
		slist.push_back(ppl7::String());
	}
}

void create_strings()
{
	std::list<ppl7::String> slist;
	for (int i=0;i<10000000;i++) {
		slist.push_back(ppl7::String("Hello World"));
	}
}

void create_large_strings()
{
	std::list<ppl7::String> slist;
	for (int i=0;i<10000000;i++) {
		slist.push_back(ppl7::String("The big brown fox jumps over the lazy dog in a huge hello world."));
	}
}

void create_ordered_set_with_strings_from_wordlist()
{
	std::set<ppl7::String> sset;
	size_t wlist_size=Wordlist.size();
	ppl7::String Suffix;
	for (int iter=0;iter<10;iter++) {
		Suffix.setf("_%05d");
		for (size_t i=0;i<wlist_size;i++) {
			sset.insert(Wordlist[i]+Suffix);
		}
	}
}


double timer(const char *descr, void (*fn)())
{
	double start=ppl7::GetMicrotime();
	fn();
	double duration=ppl7::GetMicrotime()-start;
	printf ("%-30s: %0.3f\n",descr, duration);
	fflush(NULL);
	return duration;
}


int main (int argc, char**argv)
{
	double start=ppl7::GetMicrotime();
	ppl7::PrintDebugTime ("Wortliste in String laden\n");
	ppl7::String w(wordlist);
	Wordlist.reserve(130000);
	ppl7::PrintDebugTime ("Wortliste in Array laden\n");
	Wordlist.explode(w,"\n");
	ppl7::PrintDebugTime ("done\n");

	timer ("create empty strings", create_empty_strings);
	timer ("create short strings", create_strings);
	timer ("create large strings", create_large_strings);
	timer ("ordered set with strings", create_ordered_set_with_strings_from_wordlist);
	double duration=ppl7::GetMicrotime()-start;
	printf ("%-30s: %0.3f\n","Totaltime",duration);
	return 1;
}

