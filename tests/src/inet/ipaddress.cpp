/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 600 $
 * $Date: 2013-04-26 21:37:49 +0200 (Fr, 26. Apr 2013) $
 * $Id: resolver.cpp 600 2013-04-26 19:37:49Z pafe $
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
#include "pplib-tests.h"
#ifndef WIN32
#include <netinet/in.h>
#endif
#include <set>

namespace
{

class InetIPAddressTest : public ::testing::Test
{
protected:
    InetIPAddressTest()
    {
        if (setlocale(LC_CTYPE, DEFAULT_LOCALE) == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~InetIPAddressTest()
    {
    }
};

TEST_F(InetIPAddressTest, Constructor)
{
    ASSERT_NO_THROW({ pplib::IPAddress adr; });
}

TEST_F(InetIPAddressTest, setIPv4FromString)
{
    pplib::IPAddress adr;
    ASSERT_NO_THROW({ adr.set("194.77.8.1"); });
    // pplib::HexDump(adr.addr(), adr.addr_len());
    ASSERT_EQ(pplib::IPAddress::IPv4, adr.family());
    ASSERT_EQ((size_t)4, adr.addr_len());

    const unsigned char* addr = (const unsigned char*)adr.addr();
    ASSERT_EQ(194, addr[0]);
    ASSERT_EQ(77, addr[1]);
    ASSERT_EQ(8, addr[2]);
    ASSERT_EQ(1, addr[3]);
}

TEST_F(InetIPAddressTest, setIPv6FromString)
{
    pplib::IPAddress adr;
    ASSERT_NO_THROW({ adr.set("2001:678:2a::53"); });
    // pplib::HexDump(adr.addr(), adr.addr_len());
    ASSERT_EQ(pplib::IPAddress::IPv6, adr.family());
    ASSERT_EQ((size_t)16, adr.addr_len());

    const unsigned char* addr = (const unsigned char*)adr.addr();
    ASSERT_EQ(0x20, addr[0]);
    ASSERT_EQ(0x01, addr[1]);
    ASSERT_EQ(0x06, addr[2]);
    ASSERT_EQ(0x78, addr[3]);
    ASSERT_EQ(0, addr[4]);
    ASSERT_EQ(0x2a, addr[5]);
    ASSERT_EQ(0, addr[6]);
    ASSERT_EQ(0, addr[7]);
    ASSERT_EQ(0, addr[8]);
    ASSERT_EQ(0, addr[9]);
    ASSERT_EQ(0, addr[10]);
    ASSERT_EQ(0, addr[11]);
    ASSERT_EQ(0, addr[12]);
    ASSERT_EQ(0, addr[13]);
    ASSERT_EQ(0, addr[14]);
    ASSERT_EQ(0x53, addr[15]);
}

TEST_F(InetIPAddressTest, setInvalidIpAddressThrowsException)
{
    pplib::IPAddress adr;
    ASSERT_THROW({ adr.set("555.333.2.1"); }, pplib::InvalidIpAddressException);
    ASSERT_THROW({ adr.set("2001:678:2a::53::1"); }, pplib::InvalidIpAddressException);
}

TEST_F(InetIPAddressTest, IPv4toString)
{
    pplib::IPAddress adr;
    adr.set("194.77.8.1");
    ASSERT_EQ(pplib::String("194.77.8.1"), adr.toString());
}

TEST_F(InetIPAddressTest, IPv6toString)
{
    pplib::IPAddress adr;
    adr.set("2001:678:2A::53");
    ASSERT_EQ(pplib::String("2001:678:2a::53"), adr.toString());
    adr.set("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
    ASSERT_EQ(pplib::String("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), adr.toString());
}

TEST_F(InetIPAddressTest, UnsetIpAddressThrowsException)
{
    pplib::IPAddress adr;
    ASSERT_THROW({ ASSERT_EQ(NULL, adr.addr()); }, pplib::InvalidIpAddressException);
    ASSERT_THROW({ ASSERT_EQ((size_t)0, adr.addr_len()); }, pplib::InvalidIpAddressException);

    ASSERT_THROW({ ASSERT_EQ(pplib::String(), adr.toString()); }, pplib::InvalidIpAddressException);
}

TEST_F(InetIPAddressTest, OperatorString)
{
    pplib::IPAddress adr;
    adr.set("2001:678:2A::53");
    ASSERT_EQ(pplib::String("2001:678:2a::53"), (pplib::String)adr);
}

TEST_F(InetIPAddressTest, ConstructorWithString)
{
    pplib::IPAddress adr1("194.77.8.1");
    ASSERT_EQ(pplib::String("194.77.8.1"), adr1.toString());

    pplib::IPAddress adr2("2001:678:2A::53");
    ASSERT_EQ(pplib::String("2001:678:2a::53"), adr2.toString());
}

TEST_F(InetIPAddressTest, CopyConstructor)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr(adr1);
    ASSERT_EQ(pplib::IPAddress::IPv4, adr.family());
    ASSERT_EQ((size_t)4, adr.addr_len());

    const unsigned char* addr = (const unsigned char*)adr.addr();
    ASSERT_EQ(194, addr[0]);
    ASSERT_EQ(77, addr[1]);
    ASSERT_EQ(8, addr[2]);
    ASSERT_EQ(1, addr[3]);
}

TEST_F(InetIPAddressTest, OperatorSetWithIPAddress)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr;
    adr = adr1;
    ASSERT_EQ(pplib::IPAddress::IPv4, adr.family());
    ASSERT_EQ((size_t)4, adr.addr_len());

    const unsigned char* addr = (const unsigned char*)adr.addr();
    ASSERT_EQ(194, addr[0]);
    ASSERT_EQ(77, addr[1]);
    ASSERT_EQ(8, addr[2]);
    ASSERT_EQ(1, addr[3]);
}

TEST_F(InetIPAddressTest, OperatorSetWithString)
{
    pplib::String adr1("194.77.8.1");
    pplib::IPAddress adr;
    adr = adr1;
    ASSERT_EQ(pplib::IPAddress::IPv4, adr.family());
    ASSERT_EQ((size_t)4, adr.addr_len());

    const unsigned char* addr = (const unsigned char*)adr.addr();
    ASSERT_EQ(194, addr[0]);
    ASSERT_EQ(77, addr[1]);
    ASSERT_EQ(8, addr[2]);
    ASSERT_EQ(1, addr[3]);
}

TEST_F(InetIPAddressTest, setFromBinary)
{
    unsigned char data[] = {194, 77, 8, 1};

    pplib::IPAddress adr;
    adr.set(pplib::IPAddress::IPv4, &data, 4);
    ASSERT_EQ(pplib::IPAddress::IPv4, adr.family());
    ASSERT_EQ((size_t)4, adr.addr_len());

    const unsigned char* addr = (const unsigned char*)adr.addr();
    ASSERT_EQ(194, addr[0]);
    ASSERT_EQ(77, addr[1]);
    ASSERT_EQ(8, addr[2]);
    ASSERT_EQ(1, addr[3]);
}

TEST_F(InetIPAddressTest, compare)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("2001:678:2A::53");
    pplib::IPAddress adr3("194.77.8.2");
    pplib::IPAddress adr4("195.77.8.2");
    ASSERT_TRUE(adr1.compare(adr2) < 0);
    ASSERT_TRUE(adr2.compare(adr1) > 0);
    ASSERT_TRUE(adr1.compare(adr3) < 0);
    ASSERT_TRUE(adr1.compare(adr1) == 0);
    ASSERT_TRUE(adr2.compare(adr2) == 0);
    ASSERT_TRUE(adr1.compare(adr4) < 0);
    ASSERT_TRUE(adr4.compare(adr1) > 0);
}

TEST_F(InetIPAddressTest, operator_LT)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("195.77.8.2");

    ASSERT_TRUE(adr1 < adr2);
    ASSERT_FALSE(adr2 < adr1);
}

TEST_F(InetIPAddressTest, operator_LTEQ)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("195.77.8.2");
    pplib::IPAddress adr3("194.77.8.1");

    ASSERT_TRUE(adr1 <= adr2);
    ASSERT_TRUE(adr1 <= adr3);
    ASSERT_FALSE(adr2 <= adr1);
}

TEST_F(InetIPAddressTest, operator_EQ)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("195.77.8.2");
    pplib::IPAddress adr3("194.77.8.1");
    ASSERT_TRUE(adr1 == adr3);
    ASSERT_FALSE(adr1 == adr2);
}

TEST_F(InetIPAddressTest, operator_NE)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("195.77.8.2");
    pplib::IPAddress adr3("194.77.8.1");
    ASSERT_TRUE(adr1 != adr2);
    ASSERT_FALSE(adr1 != adr3);
}

TEST_F(InetIPAddressTest, operator_GT)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("195.77.8.2");

    ASSERT_TRUE(adr2 > adr1);
    ASSERT_FALSE(adr1 > adr2);
}

TEST_F(InetIPAddressTest, operator_GTEQ)
{
    pplib::IPAddress adr1("194.77.8.1");
    pplib::IPAddress adr2("195.77.8.2");
    pplib::IPAddress adr3("194.77.8.1");

    ASSERT_TRUE(adr2 >= adr1);
    ASSERT_TRUE(adr3 >= adr1);
    ASSERT_FALSE(adr1 >= adr2);
}

TEST_F(InetIPAddressTest, sortIPAddresses)
{
    std::set<pplib::IPAddress> list;
    list.insert(pplib::IPAddress("194.2.3.4"));
    list.insert(pplib::IPAddress("2001:678:2A::53"));
    list.insert(pplib::IPAddress("1.2.3.4"));
    list.insert(pplib::IPAddress("255.0.0.0"));
    list.insert(pplib::IPAddress("2001:678:2::1"));
    list.insert(pplib::IPAddress("66.7.1.0"));
    list.insert(pplib::IPAddress("2002:dead:2::1"));

    ASSERT_EQ((size_t)7, list.size());

    std::set<pplib::IPAddress>::const_iterator it;
    it = list.begin();
    ASSERT_EQ(pplib::IPAddress("1.2.3.4"), (*it++));
    ASSERT_EQ(pplib::IPAddress("66.7.1.0"), (*it++));
    ASSERT_EQ(pplib::IPAddress("194.2.3.4"), (*it++));
    ASSERT_EQ(pplib::IPAddress("255.0.0.0"), (*it++));
    ASSERT_EQ(pplib::IPAddress("2001:678:2::1"), (*it++));
    ASSERT_EQ(pplib::IPAddress("2001:678:2A::53"), (*it++));
    ASSERT_EQ(pplib::IPAddress("2002:dead:2::1"), (*it++));
}

TEST_F(InetIPAddressTest, mask_v4)
{
    ASSERT_EQ(pplib::IPAddress("255.255.255.255"), pplib::IPAddress("255.255.255.255").mask(32));
    ASSERT_EQ(pplib::IPAddress("255.255.255.254"), pplib::IPAddress("255.255.255.255").mask(31));
    ASSERT_EQ(pplib::IPAddress("255.255.255.252"), pplib::IPAddress("255.255.255.253").mask(30));
    ASSERT_EQ(pplib::IPAddress("255.255.255.248"), pplib::IPAddress("255.255.255.249").mask(29));
    ASSERT_EQ(pplib::IPAddress("255.255.255.240"), pplib::IPAddress("255.255.255.241").mask(28));
    ASSERT_EQ(pplib::IPAddress("255.255.255.224"), pplib::IPAddress("255.255.255.225").mask(27));
    ASSERT_EQ(pplib::IPAddress("255.255.255.192"), pplib::IPAddress("255.255.255.193").mask(26));
    ASSERT_EQ(pplib::IPAddress("255.255.255.128"), pplib::IPAddress("255.255.255.129").mask(25));
    ASSERT_EQ(pplib::IPAddress("255.255.255.0"), pplib::IPAddress("255.255.255.1").mask(24));

    ASSERT_EQ(pplib::IPAddress("255.255.254.0"), pplib::IPAddress("255.255.255.255").mask(23));
    ASSERT_EQ(pplib::IPAddress("255.255.252.0"), pplib::IPAddress("255.255.253.255").mask(22));
    ASSERT_EQ(pplib::IPAddress("255.255.248.0"), pplib::IPAddress("255.255.255.255").mask(21));
    ASSERT_EQ(pplib::IPAddress("255.255.240.0"), pplib::IPAddress("255.255.255.255").mask(20));
    ASSERT_EQ(pplib::IPAddress("255.255.224.0"), pplib::IPAddress("255.255.255.255").mask(19));
    ASSERT_EQ(pplib::IPAddress("255.255.192.0"), pplib::IPAddress("255.255.255.255").mask(18));
    ASSERT_EQ(pplib::IPAddress("255.255.128.0"), pplib::IPAddress("255.255.255.255").mask(17));
    ASSERT_EQ(pplib::IPAddress("255.255.0.0"), pplib::IPAddress("255.255.255.255").mask(16));

    ASSERT_EQ(pplib::IPAddress("255.254.0.0"), pplib::IPAddress("255.255.255.255").mask(15));
    ASSERT_EQ(pplib::IPAddress("255.252.0.0"), pplib::IPAddress("255.255.255.255").mask(14));
    ASSERT_EQ(pplib::IPAddress("255.248.0.0"), pplib::IPAddress("255.255.255.255").mask(13));
    ASSERT_EQ(pplib::IPAddress("255.240.0.0"), pplib::IPAddress("255.255.255.255").mask(12));
    ASSERT_EQ(pplib::IPAddress("255.224.0.0"), pplib::IPAddress("255.255.255.255").mask(11));
    ASSERT_EQ(pplib::IPAddress("255.192.0.0"), pplib::IPAddress("255.255.255.255").mask(10));
    ASSERT_EQ(pplib::IPAddress("255.128.0.0"), pplib::IPAddress("255.255.255.255").mask(9));
    ASSERT_EQ(pplib::IPAddress("255.0.0.0"), pplib::IPAddress("255.255.255.255").mask(8));

    ASSERT_EQ(pplib::IPAddress("254.0.0.0"), pplib::IPAddress("255.255.255.255").mask(7));
    ASSERT_EQ(pplib::IPAddress("252.0.0.0"), pplib::IPAddress("255.255.255.255").mask(6));
    ASSERT_EQ(pplib::IPAddress("248.0.0.0"), pplib::IPAddress("255.255.255.255").mask(5));
    ASSERT_EQ(pplib::IPAddress("240.0.0.0"), pplib::IPAddress("255.255.255.255").mask(4));
    ASSERT_EQ(pplib::IPAddress("224.0.0.0"), pplib::IPAddress("255.255.255.255").mask(3));
    ASSERT_EQ(pplib::IPAddress("192.0.0.0"), pplib::IPAddress("255.255.255.255").mask(2));
    ASSERT_EQ(pplib::IPAddress("128.0.0.0"), pplib::IPAddress("255.255.255.255").mask(1));
    ASSERT_EQ(pplib::IPAddress("0.0.0.0"), pplib::IPAddress("255.255.255.255").mask(0));
    ASSERT_EQ(pplib::IPAddress("172.16.100.0"), pplib::IPAddress("172.16.102.1").mask(22));
}

TEST_F(InetIPAddressTest, mask_v6)
{
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(128));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(127));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffc"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(126));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fff8"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(125));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fff0"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(124));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffe0"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(123));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffc0"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(122));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(112));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(96));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:ffff:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(64));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:ffff:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(48));
    ASSERT_EQ(pplib::IPAddress("ffff:ffff:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(32));
    ASSERT_EQ(pplib::IPAddress("ffff:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(16));
    ASSERT_EQ(pplib::IPAddress("ff00:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(8));
    ASSERT_EQ(pplib::IPAddress("fe00:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(7));
    ASSERT_EQ(pplib::IPAddress("f000:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(4));
    ASSERT_EQ(pplib::IPAddress("e000:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(3));
    ASSERT_EQ(pplib::IPAddress("c000:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(2));
    ASSERT_EQ(pplib::IPAddress("8000:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(1));
    ASSERT_EQ(pplib::IPAddress("0000:0000:0000:0000:0000:0000:0000:0000"),
              pplib::IPAddress("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff").mask(0));
}

} // namespace
