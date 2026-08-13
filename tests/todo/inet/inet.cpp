/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
class InetTest : public ::testing::Test
{
protected:
    InetTest()
    {
        if (setlocale(LC_CTYPE, "") == NULL) {
            printf("setlocale fehlgeschlagen\n");
            throw std::exception();
        }
    }
    virtual ~InetTest()
    {
    }
};

TEST_F(InetTest, Idn2AceUtf8)
{
    ASSERT_NO_THROW({
        pplib::String idn = pplib::Iconv::Utf8ToLocal("tästmé.de");
        try {
            pplib::String ace = pplib::Idn2Ace(idn);
            ASSERT_EQ(pplib::String("xn--tstm-loa7a.de"), ace) << "Convertion failed";
        }
        catch (const pplib::Exception& exp) {
            exp.print();
            throw;
        }
    });
}

TEST_F(InetTest, Ace2IdnUtf8)
{
    ASSERT_NO_THROW({
        pplib::String ace = "xn--tstm-loa7a.de";
        pplib::String idn = pplib::Ace2Idn(ace);
        ASSERT_EQ(pplib::String(pplib::Iconv::Utf8ToLocal("tästmé.de")), idn) << "Convertion failed";
    });
}

TEST_F(InetTest, Idn2AceWideString)
{
    ASSERT_NO_THROW({
        pplib::WideString idn = L"tästmé.de";
        pplib::WideString ace = pplib::Idn2Ace(idn);
        ASSERT_EQ(pplib::WideString(L"xn--tstm-loa7a.de"), ace) << "Convertion failed";
    });
}

TEST_F(InetTest, Ace2IdnWideString)
{
    ASSERT_NO_THROW({
        pplib::WideString ace = L"xn--tstm-loa7a.de";
        pplib::WideString idn = pplib::Ace2Idn(ace);
        ASSERT_EQ(pplib::WideString(L"tästmé.de"), idn) << "Convertion failed";
    });
}

TEST_F(InetTest, Ntohl)
{
    if (pplib::IsLittleEndian()) {
        ASSERT_EQ((uint32_t)0x44332211, pplib::Ntohl(0x11223344));
    } else {
        ASSERT_EQ((uint32_t)0x11223344, pplib::Ntohl(0x11223344));
    }
}

TEST_F(InetTest, Ntohs)
{
    if (pplib::IsLittleEndian()) {
        ASSERT_EQ((uint32_t)0x2211, pplib::Ntohs(0x1122));
    } else {
        ASSERT_EQ((uint32_t)0x1122, pplib::Ntohs(0x1122));
    }
}

TEST_F(InetTest, Htonl)
{
    if (pplib::IsLittleEndian()) {
        ASSERT_EQ((uint32_t)0x44332211, pplib::Htonl(0x11223344));
    } else {
        ASSERT_EQ((uint32_t)0x11223344, pplib::Htonl(0x11223344));
    }
}

TEST_F(InetTest, Htons)
{
    if (pplib::IsLittleEndian()) {
        ASSERT_EQ((uint32_t)0x2211, pplib::Htons(0x1122));
    } else {
        ASSERT_EQ((uint32_t)0x1122, pplib::Htons(0x1122));
    }
}

TEST_F(InetTest, IsBigEndian)
{
    bool isBigE = false;
    if (pplib::Htonl(0x11223344) == 0x11223344) isBigE = true;
    ASSERT_EQ(isBigE, pplib::IsBigEndian());
    ASSERT_EQ(!isBigE, pplib::IsLittleEndian());
}

TEST_F(InetTest, IsLittleEndian)
{
    bool isLittleE = true;
    if (pplib::Htonl(0x11223344) == 0x11223344) isLittleE = false;
    ASSERT_EQ(isLittleE, pplib::IsLittleEndian());
    ASSERT_EQ(!isLittleE, pplib::IsBigEndian());
}

} // namespace
