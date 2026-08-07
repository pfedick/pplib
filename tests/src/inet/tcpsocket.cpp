/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 600 $
 * $Date: 2013-04-26 21:37:49 +0200 (Fr, 26 Apr 2013) $
 * $Id: inet.cpp 600 2013-04-26 19:37:49Z pafe $
 *
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <pplib.h>
#include <pplib-inet.h>
#include <gtest/gtest.h>
#include <list>
#include "pplib-tests.h"

namespace
{

// The fixture for testing class Foo.
class TcpSocketTest : public ::testing::Test
{
protected:
    TcpSocketTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
        pplib::InitSockets();
    }
    virtual ~TcpSocketTest()
    {
    }
};

TEST_F(TcpSocketTest, Constructor)
{
    ASSERT_NO_THROW({ pplib::TCPSocket socket; });
}

TEST_F(TcpSocketTest, getDescriptorWithoutConnection)
{
    pplib::TCPSocket socket;
    ASSERT_THROW({ socket.getDescriptor(); }, pplib::NotConnectedException);
}

TEST_F(TcpSocketTest, disconnectWithoutConnection)
{
    pplib::TCPSocket socket;
    ASSERT_NO_THROW({ socket.disconnect(); });
}

TEST_F(TcpSocketTest, shutdownWithoutConnection)
{
    pplib::TCPSocket socket;
    ASSERT_NO_THROW({ socket.disconnect(); });
}

TEST_F(TcpSocketTest, connectUnresolveableHostname)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "unknownserver", "unresolveable.server.pfp.de");
    ASSERT_THROW(
        { socket.connect(Hostname, 80); }, pplib::ResolverException)
        << "Connect on unresolveable address throws ResolverException";
}

TEST_F(TcpSocketTest, connectOnServerNotRunningWithTimeout)
{
    pplib::TCPSocket socket;
    socket.setTimeoutConnect(1, 0);
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "notrunningserver", "unexistingserver.ppl.pfp.de");
    double start = pplib::GetMicrotime();
    ASSERT_THROW({ socket.connect(Hostname, 80); }, pplib::TimeoutException);
    double duration = pplib::GetMicrotime() - start;
    ASSERT_LT(duration, 1.1) << "Timeout too long";
}

TEST_F(TcpSocketTest, connectOnEchoServerNonexistingPort)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    int port = PPLIBTestConfig.getIntFromSection("tcpsocket", "echoserver_nonexistingport", 11111);
    ASSERT_THROW({ socket.connect(Hostname, port); }, pplib::ConnectionRefusedException);
}

TEST_F(TcpSocketTest, connectOnEchoServer)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
}

TEST_F(TcpSocketTest, connectWithOneParameter)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    Hostname += ":7";
    ASSERT_NO_THROW({ socket.connect(Hostname); });
}

TEST_F(TcpSocketTest, connectAndDisconnect)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
    ASSERT_NO_THROW({ socket.disconnect(); });
}

TEST_F(TcpSocketTest, connectAndDisconnectAndReconnect)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
    ASSERT_NO_THROW({ socket.disconnect(); });
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
}

TEST_F(TcpSocketTest, connectAndReconnect)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
}

TEST_F(TcpSocketTest, writeAndReadString)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
    pplib::String Expected, OutString, InString;
    Expected = "PPLIB Unittest Teststring";
    OutString = Expected;
    ASSERT_NO_THROW({
        ASSERT_EQ((size_t)24, socket.write(OutString));
        ASSERT_EQ((size_t)24, socket.read(InString, 1024));
    });
    ASSERT_EQ(Expected, InString);
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
}

TEST_F(TcpSocketTest, writeAndReadRandomByteArray)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
    pplib::ByteArray Expected, Out, In;
    pplib::Random(Expected, 1234);
    Out = Expected;
    ASSERT_NO_THROW({
        ASSERT_EQ((size_t)1234, socket.write(Out));
        ASSERT_EQ((size_t)1234, socket.read(In, 4000));
    });
    ASSERT_EQ(Expected, In);
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
}

TEST_F(TcpSocketTest, repetetiveReadAndwaitForIncomingData)
{
    pplib::TCPSocket socket;
    pplib::String Hostname = PPLIBTestConfig.getFromSection("tcpsocket", "echoserver", "localhost");
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
    pplib::ByteArray Expected, Out, In, TotalIn;
    size_t totalBytes = 200000;
    pplib::Random(Expected, totalBytes);
    Out = Expected;
    ASSERT_NO_THROW({ ASSERT_EQ((size_t)totalBytes, socket.write(Out)); });
    size_t bytesRead = 0;
    while (socket.waitForIncomingData(0, 1000)) {
        bytesRead += socket.read(In, 20000);
        TotalIn.append(In);
    }
    ASSERT_EQ((size_t)totalBytes, bytesRead);
    ASSERT_EQ((size_t)totalBytes, TotalIn.size());
    ASSERT_EQ(Expected, TotalIn);
    ASSERT_NO_THROW({ socket.connect(Hostname, 7); });
}

} // namespace
