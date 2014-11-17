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
#include <list>

namespace {

class ServerThread : private ppl6::CThread, private ppl6::CTCPSocket, private ppl6::CSignal
{
	private:
		ppl6::CMutex	Mutex;
		ppl6::CSSL		SSLServerContext;

	public:
		ServerThread();
		virtual ~ServerThread();
		virtual void ThreadMain(void *param);
		virtual void Signal(int sig);
		virtual int ReceiveConnect(ppl6::CTCPSocket *socket, const char *host, int port);
		bool setupSSL();
		bool start();
		void stop();
		bool isRunning();
		bool isListening();
		void resetTestFlags();
		int ProcessMessage(ppl6::CTCPSocket *Socket, const ppl6::CAssocArray &a);
		int AnswerError(ppl6::CTCPSocket *Socket);
		int Answer(ppl6::CTCPSocket *Socket, const ppl6::CAssocArray &a);
		int AnswerSuccess(ppl6::CTCPSocket *Socket, ppl6::CAssocArray &a);
		int AnswerSuccess(ppl6::CTCPSocket *Socket);

		int CMD_Ping(ppl6::CTCPSocket *Socket);
		int CMD_StartTLS(ppl6::CTCPSocket *Socket);


		// Testing Flags
		bool bConnectionReceived;
		bool bInMessageLoop;
		int msgCount;
		int exitPos;
		std::list <ppl6::CSocketMessage> msgList;

};


ServerThread::ServerThread()
{
	resetTestFlags();
}

void ServerThread::resetTestFlags()
{
	bConnectionReceived=false;
	bInMessageLoop=false;
	msgCount=0;
	msgList.clear();
	exitPos=0;
}

ServerThread::~ServerThread()
{
	SSL_Shutdown();
	Disconnect();
}

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
	//this->ThreadDeleteOnExit(true);

	this->WatchThread(this);
	if (!Bind(PPL6TestConfig->Get("tcpsocket","tcpserver_host","localhost"),
			PPL6TestConfig->GetInt("tcpsocket","tcpserver_port",50001))) {
		return;
	}
	if (!Listen()) return;

	printf ("ServerThread wird beendet\n");
}

int ServerThread::ReceiveConnect(ppl6::CTCPSocket *socket, const char *host, int port)
{
	bConnectionReceived=true;
	ppl6::CAssocArray Data;
	printf ("DEBUG: Connect start: %s:%i\n",host,port);
	socket->WatchThread(this);
	socket->SetBlocking(false);
	ppl6::CSocketMessage Msg;
	while (1) {
		bInMessageLoop=true;
		if (ThreadShouldStop()) {
			exitPos=1;
			break;
		}
		if (!socket->WaitForMessage(Msg)) {
			exitPos=2;
			break;
		}
		msgCount++;
		msgList.push_back(Msg);
		if(Msg.GetType()==PPL_ASSOCARRAY) {
			if (!Msg.GetData(Data)) {
				exitPos=3;
				break;
			}
			if (!ProcessMessage(socket, Data)) {
				if (!AnswerError(socket)) {
					exitPos=4;
					break;
				}
			}
		} else {
			exitPos=5;
			break;
		}
	}
	printf ("DEBUG: Connect end: %s:%i\n",host,port);
	socket->Disconnect();
	delete socket;
	return 1;
}

int ServerThread::AnswerError(ppl6::CTCPSocket *Socket)
{
	ppl6::CAssocArray a;
	a.Set("result","failed");
	a.Setf("errorcode","%i",ppl6::GetErrorCode());
	a.Set("errortext",ppl6::GetError());
	a.Set("extendederror",ppl6::GetExtendedError());
	a.Setf("suberror","%i",ppl6::GetSubError());
	return Answer(Socket, a);
}

int ServerThread::Answer(ppl6::CTCPSocket *Socket, const ppl6::CAssocArray &a)
{
	if (a.Count()==0) {
		ppl6::CAssocArray b;
		b.Set("result","success");
		return Answer(Socket, b);
	}
	ppl6::CSocketMessage Msg;
	Msg.SetData(a);
	int ret=0;
	if (Socket) ret=Socket->Write(Msg);
	else ppl6::SetError(299);
	return ret;
}

int ServerThread::AnswerSuccess(ppl6::CTCPSocket *Socket, ppl6::CAssocArray &a)
{
	ppl6::CAssocArray b;
	b.Set("result","success");
	b.Set("answer",a);
	return Answer(Socket, b);
}

int ServerThread::AnswerSuccess(ppl6::CTCPSocket *Socket)
{
	ppl6::CAssocArray b;
	b.Set("result","success");
	return Answer(Socket, b);
}


int ServerThread::ProcessMessage(ppl6::CTCPSocket *Socket, const ppl6::CAssocArray &a)
{
	ppl6::CString Action=a.ToCString("action");

	if (Action=="ping") {
		return CMD_Ping(Socket);
	} else if (Action=="starttls") {
		return CMD_StartTLS(Socket);
	}
	ppl6::SetError(10003,"%s",(const char*)Action);
	return 0;
}

int ServerThread::CMD_Ping(ppl6::CTCPSocket *Socket)
{
	ppl6::CAssocArray a;
	a.Set("result","success");
	a.Set("version","1");
	a.Setf("servertime","%llu",ppl6::GetTime());
	return Answer(Socket, a);
}

int ServerThread::CMD_StartTLS(ppl6::CTCPSocket *Socket)
{
	if (!Socket->SSL_Init(&SSLServerContext)) {
		return 0;
	}
	ppl6::CAssocArray a;
	a.Set("result","success");
	if (!Answer(Socket, a)) return 0;


	if (!Socket->SSL_Init_Server()) {
		return 0;
	}
	printf ("Wait for SSL-Accept\n");
	if (!Socket->SSL_WaitForAccept(1000)) {
		printf ("SSL-Handshake failed\n");
		return 0;
	}
	printf ("SSL-Handshake done\n");
	return 1;
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
			delete server;
			server=NULL;
			printf ("TearDownTestCase done\n");
		}
	}



};


TEST_F(ClientServerTest, ConnectWaitAndDisconnect) {
	server->resetTestFlags();
	ppl6::CTCPSocket Socket;
	ASSERT_TRUE(Socket.Connect(PPL6TestConfig->Get("tcpsocket","tcpserver_host","localhost"),
			PPL6TestConfig->GetInt("tcpsocket","tcpserver_port",50001))) << "Connect failed [" << ppl6::Error2String() << "]";
	ppl6::SSleep(1);
	ASSERT_TRUE(server->bConnectionReceived) << "No connection received";
	ASSERT_TRUE(server->bInMessageLoop) << "Not in Message-Loop";
	ASSERT_EQ(0,server->msgCount) << "Unexpected amount of Messages";
	ppl6::SSleep(1);
	printf ("Disconnect\n");
	Socket.Disconnect();
	ppl6::SSleep(1);
	printf ("Disconnect\n");
	ASSERT_EQ(0,server->msgCount) << "Unexpected amount of Messages";
	ASSERT_EQ(2,server->exitPos) << "Unexpected exitCode";
}

TEST_F(ClientServerTest, ConnectPingDisconnect) {
	server->resetTestFlags();
	ppl6::CTCPSocket Socket;
	ASSERT_TRUE(Socket.Connect(PPL6TestConfig->Get("tcpsocket","tcpserver_host","localhost"),
			PPL6TestConfig->GetInt("tcpsocket","tcpserver_port",50001))) << "Connect failed [" << ppl6::Error2String() << "]";
	ppl6::SSleep(1);
	ASSERT_TRUE(server->bConnectionReceived) << "No connection received";
	ASSERT_TRUE(server->bInMessageLoop) << "Not in Message-Loop";
	ASSERT_EQ(0,server->msgCount) << "Unexpected amount of Messages";

	ppl6::CSocketMessage Msg;
	ppl6::CAssocArray data;
	data.Set("action","ping");
	Msg.SetData(data);
	Socket.Write(Msg);
	ASSERT_EQ(1,Socket.WaitForMessage(Msg, 1000));
	Msg.GetData(data);

	ASSERT_EQ(ppl6::CString("success"),data.ToCString("result")) << "Unexpected result";
	ASSERT_EQ(ppl6::CString("1"),data.ToCString("version")) << "Unexpected result";
	Socket.Disconnect();
	ppl6::SSleep(1);
	ASSERT_EQ(1,server->msgCount) << "Unexpected amount of Messages";
	ASSERT_EQ(2,server->exitPos) << "Unexpected exitCode";
}

TEST_F(ClientServerTest, ConnectPingStartTLSPingDisconnect) {
	server->resetTestFlags();
	ppl6::CSSL SSLContext;
	ppl6::CTCPSocket Socket;
	ASSERT_TRUE(SSLContext.Init(ppl6::CSSL::TLSclient));
	ASSERT_TRUE(Socket.SSL_Init(&SSLContext));

	ASSERT_TRUE(Socket.Connect(PPL6TestConfig->Get("tcpsocket","tcpserver_host","localhost"),
			PPL6TestConfig->GetInt("tcpsocket","tcpserver_port",50001))) << "Connect failed [" << ppl6::Error2String() << "]";
	ppl6::SSleep(1);
	ASSERT_TRUE(server->bConnectionReceived) << "No connection received";
	ASSERT_TRUE(server->bInMessageLoop) << "Not in Message-Loop";
	ASSERT_EQ(0,server->msgCount) << "Unexpected amount of Messages";

	ppl6::CSocketMessage Msg;
	ppl6::CAssocArray data;

	// Step 1: Ping
	data.Set("action","ping");
	Msg.SetData(data);
	Socket.Write(Msg);
	ASSERT_EQ(1,Socket.WaitForMessage(Msg, 1000));
	Msg.GetData(data);
	ASSERT_EQ(ppl6::CString("success"),data.ToCString("result")) << "Unexpected result Step 1";
	ASSERT_EQ(ppl6::CString("1"),data.ToCString("version")) << "Unexpected result Step 2";

	// Step 2: StartTLS
	data.Clear();
	Msg.Clear();
	data.Set("action","starttls");
	Msg.SetData(data);
	Socket.Write(Msg);
	ASSERT_EQ(1,Socket.WaitForMessage(Msg, 1000));
	Msg.GetData(data);
	ASSERT_EQ(ppl6::CString("success"),data.ToCString("result")) << "Unexpected result, starttls failed";

	// Step 3: Initialize SSL
	ppl6::SSleep(1);
	ASSERT_TRUE(Socket.SSL_Init_Client());
	ASSERT_TRUE(Socket.SSL_Start());
	printf ("Client SSL-Handshake done\n");
	ppl6::SSleep(1);

	// Step 4: Ping again, now encrypted
	data.Clear();
	Msg.Clear();
	data.Set("action","ping");
	Msg.SetData(data);
	Socket.Write(Msg);
	ASSERT_EQ(1,Socket.WaitForMessage(Msg, 1000));
	Msg.GetData(data);
	data.List();
	ASSERT_EQ(ppl6::CString("success"),data.ToCString("result")) << "Unexpected result Encrypted Ping";
	ASSERT_EQ(ppl6::CString("1"),data.ToCString("version")) << "Unexpected result Encrypted Ping";


	Socket.Disconnect();
	ppl6::SSleep(1);
	ASSERT_EQ(3,server->msgCount) << "Unexpected amount of Messages";
	ASSERT_EQ(2,server->exitPos) << "Unexpected exitCode";
}




}	// EOF namespace

