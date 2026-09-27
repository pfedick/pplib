/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <gtest/gtest.h>

#include <pplib/types/string.h>
#include <pplib/core/args.h>
#include <pplib/exceptions.h>

#include "pplib-tests.h"

namespace
{
TEST(ArgvTest, HaveArgvBasic)
{
    char* argv[] = {(char*)"myprog", (char*)"-h", (char*)"--verbose", (char*)"-c", (char*)"test.conf", nullptr};
    int argc = 5;

    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "-h"));
    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "--verbose"));
    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "-c"));
    EXPECT_FALSE(pplib::HaveArgv(argc, argv, "-x"));
    EXPECT_FALSE(pplib::HaveArgv(argc, argv, "--other"));
}

TEST(ArgvTest, HaveArgvPrefixAndEqual)
{
    char* argv[] = {(char*)"myprog", (char*)"--config=test.conf", (char*)"--prefix-path=/usr", (char*)"-ctest", nullptr};
    int argc = 4;

    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "--config"));
    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "--config="));
    EXPECT_FALSE(pplib::HaveArgv(argc, argv, "--prefix"));
    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "--prefix-path"));
    EXPECT_TRUE(pplib::HaveArgv(argc, argv, "-c"));
}

TEST(ArgvTest, HaveArgvEdgeCases)
{
    char* argv[] = {(char*)"myprog", nullptr, (char*)"-h", nullptr};
    EXPECT_FALSE(pplib::HaveArgv(0, argv, "-h"));
    EXPECT_FALSE(pplib::HaveArgv(1, argv, "-h"));
    EXPECT_FALSE(pplib::HaveArgv(3, nullptr, "-h"));
    EXPECT_FALSE(pplib::HaveArgv(3, argv, ""));
    EXPECT_TRUE(pplib::HaveArgv(3, argv, "-h"));
}

TEST(ArgvTest, GetArgvBasicAndAttached)
{
    char* argv[] = {(char*)"myprog",           (char*)"-c",      (char*)"my.conf", (char*)"-ofile.txt",
                    (char*)"--input=data.bin", (char*)"--mode=", (char*)"fast",    nullptr};
    int argc = 7;

    EXPECT_EQ(pplib::String("my.conf"), pplib::GetArgv(argc, argv, "-c"));
    EXPECT_EQ(pplib::String("file.txt"), pplib::GetArgv(argc, argv, "-o"));
    EXPECT_EQ(pplib::String("data.bin"), pplib::GetArgv(argc, argv, "--input"));
    EXPECT_EQ(pplib::String("data.bin"), pplib::GetArgv(argc, argv, "--input="));
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(argc, argv, "--mode="));
}

TEST(ArgvTest, GetArgvDashHandlingAndEscaping)
{
    char* argv[] = {(char*)"myprog", (char*)"-a", (char*)"-b", (char*)"-t", (char*)"\\-10", nullptr};
    int argc = 5;

    EXPECT_EQ(pplib::String(""), pplib::GetArgv(argc, argv, "-a"));
    EXPECT_EQ(pplib::String("-10"), pplib::GetArgv(argc, argv, "-t"));
}

TEST(ArgvTest, GetArgvLastArgWithoutValue)
{
    char* argv[2] = {(char*)"myprog", (char*)"-c"};
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(2, argv, "-c"));

    char* argvNullTerm[] = {(char*)"myprog", (char*)"-c", nullptr};
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(2, argvNullTerm, "-c"));
}

TEST(ArgvTest, GetArgvEdgeCases)
{
    char* argv[] = {(char*)"myprog", (char*)"--prefix-path=/usr", nullptr, (char*)"-c", (char*)"test.conf", nullptr};
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(0, argv, "-c"));
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(1, argv, "-c"));
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(5, nullptr, "-c"));
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(5, argv, ""));
    EXPECT_EQ(pplib::String(""), pplib::GetArgv(5, argv, "--prefix"));
    EXPECT_EQ(pplib::String("/usr"), pplib::GetArgv(5, argv, "--prefix-path"));
    EXPECT_EQ(pplib::String("test.conf"), pplib::GetArgv(5, argv, "-c"));
}

TEST(ClassArgsTest, BasicUsage)
{
    char* argv[] = {(char*)"myprog", (char*)"-c", (char*)"my.conf", nullptr};
    int argc = 3;

    pplib::Args args(argc, argv);
    EXPECT_EQ(args.argc(), argc);
    EXPECT_EQ(args.argv(), argv);
    EXPECT_TRUE(args.has("-c"));
    EXPECT_EQ(args.get("-c"), "my.conf");
}

TEST(ClassArgsTest, FallBackToDefaultValue)
{
    char* argv[] = {(char*)"myprog", (char*)"-c", nullptr};
    int argc = 2;

    pplib::Args args(argc, argv);
    EXPECT_EQ(args.get("-c", "default.conf"), "default.conf");
    EXPECT_EQ(args.get("-d", "default.conf"), "default.conf");
}

} // namespace