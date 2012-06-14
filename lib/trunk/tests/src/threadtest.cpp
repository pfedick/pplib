/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 435 $
 * $Date: 2012-06-08 23:58:34 +0200 (Fr, 08 Jun 2012) $
 * $Id: strings.cpp 435 2012-06-08 21:58:34Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/tests/src/strings.cpp $
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
#include "ppl7-tests.h"


class TestThread : private ppl7::Thread
{
	private:
		ppl7::Mutex mutex;
		ppl7::Mutex signal;
	public:
		void run();
		void start();


};


void TestThread::start()
{
	threadStart();
}

void TestThread::run()
{
	ppl7::PrintDebugTime("Thread %u: gestartet\n",threadGetID());
	ppl7::MSleep(500);
	ppl7::PrintDebugTime("Thread %u: wird beendet\n",threadGetID());
}

int main(int argc, char**argv)
{
	ppl7::PrintDebugTime("Hauptprogramm gestartet\n");
	TestThread t1,t2;
	t1.start();
	t2.start();
	ppl7::MSleep(1000);
	ppl7::PrintDebugTime("Hauptprogramm wird beendet\n");
	return 0;
}
