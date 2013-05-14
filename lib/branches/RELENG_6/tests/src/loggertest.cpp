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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include "../include/ppl6.h"
#include "ppl6-tests.h"


int main(int agrc, char ** argv)
{

	if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
		printf ("setlocale fehlgeschlagen\n");
		throw std::exception();
	}
	ppl6::SetGlobalEncoding("UTF-8");

	ppl6::CLog log;
	log.SetLogLevel(ppl6::LOG::WARNING,10);
	log.SetLogLevel(ppl6::LOG::INFO,10);
	log.SetLogLevel(ppl6::LOG::DEBUG,10);
	log.SetLogLevel(ppl6::LOG::ERROR,10);
	log.SetLogLevel(ppl6::LOG::NOTICE,10);
	log.openSyslog("ppl6-loggertest",ppl6::CLog::SYSLOG_USER);
	log.Print(ppl6::LOG::WARNING,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test");
	log.Print(ppl6::LOG::INFO,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test Info");
	log.Print(ppl6::LOG::ERROR,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test ERROR");
	log.Print(ppl6::LOG::NOTICE,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test NOTICE");
	ppl6::SetError(2,"Test");
	log.LogError();



	return 0;
}
