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

#define PPLIBTESTSUITEMAIN
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <pplib.h>
#include <pplib/core/dir.h>
#include <pplib/core/functions.h>
#include <gtest/gtest.h>
#include "pplib-tests.h"

extern const char* wordlist;

pplib::Array Wordlist;
pplib::ConfigParser PPLIBTestConfig;
pplib::AssocArray TestAssocArray;

void help()
{
    printf("PPLIB Testsuite configuration options:\n"
           "-c CONFIG   Configuration-file for pplib testsuite\n"
           "\n"
           "Test-Framework options:\n");
}

static void setupTestAssocArray()
{
    TestAssocArray.set("key1", "Dieser Wert geht über\nmehrere Zeilen");
    TestAssocArray.set("key2", "value6");
    TestAssocArray.set("array1/unterkey1", "value2");
    TestAssocArray.set("array1/unterkey2", "value3");
    TestAssocArray.set("array1/noch ein array/unterkey1", "value4");
    TestAssocArray.set("array1/unterkey2", "value5");
    TestAssocArray.set("key2", "value7");
    TestAssocArray.set("array2/unterkey1", "value7");
    TestAssocArray.set("array2/unterkey2", "value8");
    TestAssocArray.set("array2/unterkey1", "value9");
}

bool findTestConfig(const std::map<pplib::String, pplib::String>& pathfinder)
{
    for (const auto& [file, p] : pathfinder) {
        if (pplib::File::exists(file)) {
            if (!p.isEmpty()) {
                chdir((const char*)p);
                return true;
            }
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    if (setlocale(LC_ALL, DEFAULT_LOCALE) == NULL) {
        printf("setlocale fehlgeschlagen\n");
        throw std::exception();
    }
    setlocale(LC_NUMERIC, "C"); // Wir wollen immer '.' als Dezimaltrennzeichen haben, egal welche Locale gesetzt ist

    if (pplib::HaveArgv(argc, argv, "-h") || pplib::HaveArgv(argc, argv, "--help")) {
        help();
    }
    ::testing::InitGoogleTest(&argc, argv);
    if (pplib::HaveArgv(argc, argv, "-h") || pplib::HaveArgv(argc, argv, "--help")) return 0;

    try {
        if ((pplib::HaveArgv(argc, argv, "-c"))) {
            PPLIBTestConfig.load(pplib::GetArgv(argc, argv, "-c"));
        } else {
            // TODO: config selber finden und in den richtigen pfad wechseln
            std::map<pplib::String, pplib::String> pathfinder;
            pathfinder["test.conf"] = "";
            pathfinder["tests/test.conf"] = "tests";

            PPLIBTestConfig.load("test.conf");
        }
    }
    catch (const pplib::Exception& e) {
        printf("ERROR: Failed to load test configuration-file\n");
        e.print();
        throw;
        return 1;
    }
    ::testing::InitGoogleTest(&argc, argv);
    if (pplib::HaveArgv(argc, argv, "-h") || pplib::HaveArgv(argc, argv, "--help")) return 0;

    pplib::PrintDebugTime("Wortliste in String laden\n");
    pplib::String w(wordlist);
    Wordlist.reserve(130000);
    pplib::PrintDebugTime("Wortliste in Array laden\n");
    Wordlist.explode(w, "\n");
    pplib::PrintDebugTime("done\n");

    setupTestAssocArray();
    pplib::Dir::rmDir("tmp", true);
    pplib::Dir::mkDir("tmp");

    try {
        return RUN_ALL_TESTS();
    }
    catch (const pplib::Exception& e) {
        printf("pplib::Exception: %s\n", e.what());
    }
    catch (...) {
        printf("Unbekannte Exception\n");
    }

    return 1;
}
