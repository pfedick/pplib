/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 847 $
 * $Date: 2014-01-26 10:29:53 +0100 (So, 26 Jan 2014) $
 * $Id: clog.cpp 847 2014-01-26 09:29:53Z pafe $
 * $URL: https://svn.code.sf.net/p/pplib/code/lib/branches/RELENG_6/tests/src/clog.cpp $
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
#include <signal.h>
#include "ppl6-tests.h"

namespace {

class ServerThread : private ppl6::CThread, private ppl6::CTCPSocket, private ppl6::CSignal
{
	private:
		ppl6::CMutex	Mutex;
		ppl6::CSSL		SSLServerContext;


	public:
		virtual void ThreadMain(void *param);
		virtual void Signal(int sig);
		virtual int ReceiveConnect(ppl6::CTCPSocket *socket, const char *host, int port);
		bool setupSSL();
		bool start();
		void stop();
		bool isRunning();
		bool isListening();


};

void ServerThread::Signal(int sig)
{
	switch (sig) {
		case SIGINT:
			printf ("SIGINT => Shutdown\n");
			SignalStopListen();
			break;
		default:
			printf ("Unknown Signal => Shutdown\n");
			SignalStopListen();
			break;

	}
}


void ServerThread::ThreadMain(void *param)
{
	CatchSignal(SIGINT);

	this->WatchThread(this);
	if (!Bind(PPL6TestConfig.Get("tcpsocket","tcpserver_host","localhost"),
			PPL6TestConfig.GetInt("tcpsocket","tcpserver_port",50001))) {
		return;
	}
	if (!Listen()) return;

	printf ("ServerThread wird beendet\n");
}

int ServerThread::ReceiveConnect(ppl6::CTCPSocket *socket, const char *host, int port)
{
	printf ("Connect: %s:%i\n",host,port);
	return 0;
}

bool ServerThread::setupSSL()
{
	if (!SSLServerContext.Init(ppl6::CSSL::TLSv1server)) return false;
	if (!SSLServerContext.LoadCertificate("testdata/ssl/ppl6.cert",
			"testdata/ssl/ppl6.key","bGl7R_3JkaRT=1+A")) return false;

	if (!this->SSL_Init(&SSLServerContext)) return false;


	return true;
}

bool ServerThread::start()
{
	return this->ThreadStart(NULL);
}

void ServerThread::stop()
{
	printf ("Stop\n");
	this->SignalStopListen();
	this->StopListen();
	this->ThreadStop();
}

bool ServerThread::isListening()
{
	return this->IsListening();
}

bool ServerThread::isRunning()
{
	return this->ThreadIsRunning();
}

static ServerThread * server;

class ClientServerTest : public ::testing::Test {
	protected:
	ClientServerTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
		ppl6::SetGlobalEncoding("UTF-8");
		//printf ("debug\n");
	}
	virtual ~ClientServerTest() {

	}



	static void SetUpTestCase() {
		printf ("SetupOnce");
		ASSERT_TRUE(ppl6::SSL_Init()) << "SSL-Initialisation failed [" << ppl6::Error2String() << "]";
		server=new ServerThread;
		ASSERT_TRUE(server->setupSSL()) << "Could not setup SSL-Context [" << ppl6::Error2String() << "]";
		ASSERT_TRUE(server->start()) << "Could not start Server-Thread [" << ppl6::Error2String() << "]";
		for (int i=0;i<20;i++) {
			if (server->isListening()) break;
			ppl6::MSleep(100);
		}
		ASSERT_TRUE(server->isListening()) << "Server Thread is not running [" << ppl6::Error2String() << "]";
	}

	static void TearDownTestCase() {
		if (server) {
			server->stop();
			ASSERT_FALSE(server->isRunning()) << "Could not stop Server-Thread";
			//delete server;
			//server=NULL;
			printf ("TearDownTestCase done\n");
		}

	}



};


TEST_F(ClientServerTest, Aaa) {
	ppl6::SSleep(2);
}

TEST_F(ClientServerTest, Zzz) {

}



}	// EOF namespace

