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

#include "../include/prolog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "../include/ppl6.h"
#include <gtest/gtest.h>
#include "ppl6-tests.h"

namespace {

class LogThread : public ppl6::CThread
{
	public:
		virtual void ThreadMain(void *param);
};

ppl6::CLog *log=NULL;

void LogThread::ThreadMain(void *param)
{
	log->Print(ppl6::LOG::INFO,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Thread start");
	for (int i=0;i<10;i++) {
		log->Printf(ppl6::LOG::INFO,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Logiteration %i",i);
		ppl6::MSleep(1);
		log->Print(ppl6::LOG::WARNING,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Test WARNING");
		ppl6::MSleep(0);
		log->Print(ppl6::LOG::INFO,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Test INFO");
		log->Print(ppl6::LOG::ERROR,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Test ERROR");
		ppl6::MSleep(1);
		log->Print(ppl6::LOG::NOTICE,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Test NOTICE");

		ppl6::CAssocArray a;
		ppl6::CDateTime now;
		a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
		a.Set("key2","value6");
		now.setCurrentTime();
		a.Set("time",now);
		ppl6::CBinary ba(1234);
		a.Set("bytearray",ba);
		ppl6::CArray a1("red green blue yellow black white"," ");
		a.Set("array1",a1);
		ppl6::MSleep(0);
		log->PrintArraySingleLine(ppl6::LOG::DEBUG,1,"test::CLogTest","LogThread::ThreadMain",__FILE__,__LINE__,&a,"Array output multiline");
		ppl6::MSleep(2);
	}
	log->Print(ppl6::LOG::INFO,1,"ppl6::test::loggertest","LogThread::ThreadMain",__FILE__,__LINE__,"Thread end");
}


class CLogTest : public ::testing::Test {
	protected:
	CLogTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
	}
	virtual ~CLogTest() {

	}
};

TEST_F(CLogTest, ConstructorNew) {
	ASSERT_NO_THROW({
		log=new ppl6::CLog();
	});
	ASSERT_TRUE(log!=NULL)	<< "log ist nicht mehr NULL";
}

TEST_F(CLogTest, SetLogLevelWarning) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->SetLogLevel(ppl6::LOG::WARNING,10);
	}) << "setze Loglevel fuer WARNING auf 10";
	ASSERT_EQ(10,log->GetLogLevel(ppl6::LOG::WARNING)) << "Pruefen: Loglevel fuer WARNING muss 10 sein";
}

TEST_F(CLogTest, SetLogLevelInfo) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->SetLogLevel(ppl6::LOG::INFO,9);
	}) <<"setze Loglevel fuer INFO auf 9";
	ASSERT_EQ(9,log->GetLogLevel(ppl6::LOG::INFO)) << "Pruefen: Loglevel fuer INFO muss 9 sein";
}

TEST_F(CLogTest, SetLogLevelError) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->SetLogLevel(ppl6::LOG::ERROR,8);
	}) << "setze Loglevel fuer ERROR auf 8";
	ASSERT_EQ(8,log->GetLogLevel(ppl6::LOG::ERROR)) << "Pruefen: Loglevel fuer ERROR muss 8 sein";
}

TEST_F(CLogTest, SetLogLevelDebug) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->SetLogLevel(ppl6::LOG::DEBUG,7);
	}) << "setze Loglevel fuer DEBUG auf 7";
	ASSERT_EQ(7,log->GetLogLevel(ppl6::LOG::DEBUG)) << "Pruefen: Loglevel fuer DEBUG muss 7 sein";
}

TEST_F(CLogTest, SetLogLevelNotice) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->SetLogLevel(ppl6::LOG::NOTICE,6);
	})  << "setze Loglevel fuer NOTICE auf 6";
	ASSERT_EQ(6,log->GetLogLevel(ppl6::LOG::NOTICE)) << "Pruefen: Loglevel fuer NOTICE muss 6 sein";
	ASSERT_EQ(7,log->GetLogLevel(ppl6::LOG::DEBUG)) << "Pruefen: Loglevel fuer DEBUG muss 7 sein";
	ASSERT_EQ(8,log->GetLogLevel(ppl6::LOG::ERROR)) << "Pruefen: Loglevel fuer ERROR muss 8 sein";
	ASSERT_EQ(9,log->GetLogLevel(ppl6::LOG::INFO)) << "Pruefen: Loglevel fuer INFO muss 9 sein";
	ASSERT_EQ(10,log->GetLogLevel(ppl6::LOG::WARNING)) << "Pruefen: Loglevel fuer WARNING muss 10 sein";
}

TEST_F(CLogTest, DeleteExistingLogfile) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ppl6::CFile::DeleteFile("clogtest.log.1");
	ppl6::CFile::DeleteFile("clogtest.log.2");
	ppl6::CFile::DeleteFile("clogtest.log.3");
	ppl6::CFile::DeleteFile("clogtest.log.4");
	ppl6::CFile::DeleteFile("clogtest.log.5");
	int ret=ppl6::CFile::DeleteFile("clogtest.log");
	bool ok=false;
	if (ret==1) ok=true;
	else if (ppl6::GetErrorCode()==364) ok=true;
	ASSERT_EQ(true,ok) << "Kein Logfile vorhanden oder Logfile wurde geloescht";
}

TEST_F(CLogTest, SetLogfileDebug) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		ASSERT_EQ(1,log->SetLogfile(ppl6::LOG::DEBUG,"clogtest.log")) << "Setze Logfile fuer Debug-Output";
	})  << "setze Loglevel fuer NOTICE auf 6";
}


TEST_F(CLogTest, OpenSyslog) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->openSyslog("ppl6-loggertest",ppl6::CLog::SYSLOG_USER);
	})  << "Oeffne Syslog";
}

TEST_F(CLogTest, SetFilterModule) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_EQ(1,log->SetFilter("ppl6::test::loggertest","supress1",1)) << "Setze Filter 1 auf Modul";
	ASSERT_EQ(1,log->SetFilter("ppl6::test::loggertest","supress2",1)) << "Setze Filter 2 auf Modul";
	ASSERT_EQ(1,log->SetFilter("ppl6::test::loggertest","supress3",1)) << "Setze Filter 3 auf Modul";
}

TEST_F(CLogTest, SetFilterFile) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_EQ(1,log->SetFilter(__FILE__,193,1)) << "Setze Filter 1 auf File";
	ASSERT_EQ(1,log->SetFilter(__FILE__,1024,1)) << "Setze Filter 2 auf File";
	ASSERT_EQ(1,log->SetFilter(__FILE__,1399,1)) << "Setze Filter 3 auf File";
}


TEST_F(CLogTest, SimpleLogging) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->Print(ppl6::LOG::WARNING,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test WARNING");
		log->Print(ppl6::LOG::INFO,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test INFO");
		log->Print(ppl6::LOG::ERROR,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test ERROR");
		log->Print(ppl6::LOG::NOTICE,1,"ppl6::test::loggertest","main",__FILE__,__LINE__,"Test NOTICE");
	});
}

TEST_F(CLogTest, SuppressedLogging) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->Print(ppl6::LOG::DEBUG,1,"ppl6::test::loggertest","supress1",__FILE__,__LINE__,"Test SUPPRESSED 1");
		log->Print(ppl6::LOG::DEBUG,1,"ppl6::test::loggertest","supress2",__FILE__,__LINE__,"Test SUPPRESSED 2");
		log->Print(ppl6::LOG::DEBUG,1,"ppl6::test::loggertest","supress3",__FILE__,__LINE__,"Test SUPPRESSED 3");
		log->Print(ppl6::LOG::DEBUG,1,"ppl6::test::loggertest","supress4",__FILE__,__LINE__,"Test NOT SUPPRESSED 4");
	});
}



TEST_F(CLogTest, LogError) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ppl6::SetError(2,"Test");
	ASSERT_NO_THROW({
		log->LogError();
	}) << "Logge Fehlerzustand";
}

TEST_F(CLogTest, PrintArray) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ppl6::CAssocArray a;
	ppl6::CDateTime now;
	a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.Set("key2","value6");
	now.setCurrentTime();
	a.Set("time",now);
	ppl6::CBinary ba(1234);
	a.Set("bytearray",ba);
	ppl6::CArray a1("red green blue yellow black white"," ");
	a.Set("array1",a1);

	ASSERT_NO_THROW({
		log->PrintArray(ppl6::LOG::DEBUG,1,"test::CLogTest","PrintArray",__FILE__,__LINE__,&a,"Array output multiline");
	}) << "Logge AssocArray-Inhalt";
}

TEST_F(CLogTest, PrintArraySingleLine) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ppl6::CAssocArray a;
	ppl6::CDateTime now;
	a.Set("key1","Dieser Wert geht über\nmehrere Zeilen");
	a.Set("key2","value6");
	now.setCurrentTime();
	a.Set("time",now);
	ppl6::CBinary ba(1234);
	a.Set("bytearray",ba);
	ppl6::CArray a1("red green blue yellow black white"," ");
	a.Set("array1",a1);

	ASSERT_NO_THROW({
		log->PrintArraySingleLine(ppl6::LOG::DEBUG,1,"test::CLogTest","PrintArray",__FILE__,__LINE__,&a,"Array output multiline");
	}) << "Logge AssocArray-Inhalt";
}

TEST_F(CLogTest, SetLogRotate) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_EQ(1,log->SetLogRotate(20000,5)) << "SetLogRotate aufrufen";
}

TEST_F(CLogTest, MultithreadedTests) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ppl6::CThreadPool Pool;
	LogThread *thread;

	for (int i=0;i<10;i++) {
		thread=new LogThread;
		ASSERT_EQ(1,thread->ThreadStart(NULL)) << "Starte Thread";
		ASSERT_EQ(1,Pool.AddThread(thread)) << "Gebe Thread in den Threadpool";
	}
	Pool.Stop();
	Pool.DestroyThreads();
}


TEST_F(CLogTest, CloseSyslog) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	ASSERT_NO_THROW({
		log->closeSyslog();
	})  << "Schliesse Syslog";
}

TEST_F(CLogTest, CallDestructor) {
	ASSERT_TRUE(log!=NULL)	<< "log darf nicht auf NULL zeigen";
	delete log;
}

}
