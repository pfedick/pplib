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

#ifndef PPLIB_CORE_REGEX_H_
#define PPLIB_CORE_REGEX_H_

#include <vector>
#include <pplib/types/string.h>

namespace pplib
{
class RegEx
{
public:
    class Flags
    {
    public:
        enum
        {
            NONE = 0,
            CASELESS = 1,
            ANCHORED = 2,
            MULTILINE = 4,
            EXTENDED = 8,
            DOTALL = 16,
            UNGREEDY = 32
        };
    };

    class Pattern
    {
        friend class RegEx;

    private:
        void* p;
        char bits;

    public:
        Pattern();
        Pattern(const Pattern& other);
        Pattern(const Pattern&& other);
        ~Pattern();
    };

    typedef std::vector<String> MatchVector;

    static Pattern compile(const String& regex, int flags = Flags::NONE);
    static Pattern compile(const WideString& regex, int flags = Flags::NONE);

    static bool match(const String& regex, const String& subject, int flags = Flags::NONE);
    static bool match(const WideString& regex, const WideString& subject, int flags = Flags::NONE);
    static bool match(const Pattern& pattern, const String& subject);
    static bool match(const Pattern& pattern, const WideString& subject);

    static bool capture(const String& regex, const String& subject, std::vector<String>& matches, int flags = Flags::NONE);
    static bool capture(const Pattern& pattern, const String& subject, std::vector<String>& matches);
    static bool capture(const WideString& regex, const WideString& subject, std::vector<WideString>& matches, int flags = Flags::NONE);
    static bool capture(const Pattern& pattern, const WideString& subject, std::vector<WideString>& matches);

    static String replace(const String& regex, const String& subject, const String& replacement, int flags = Flags::NONE, int max = 0);
    static String replace(const Pattern& pattern, const String& subject, const String& replacement, int max = 0);
    static WideString replace(
        const WideString& regex, const WideString& subject, const WideString& replacement, int flags = Flags::NONE, int max = 0);
    static WideString replace(const Pattern& pattern, const WideString& subject, const WideString& replacement, int max = 0);

    static String escape(const String& subject);
    static WideString escape(const WideString& subject);
};
} // namespace pplib

#endif // PPLIB_CORE_MEMORYHEAP_H_
