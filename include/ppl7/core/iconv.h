/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
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

#ifndef PPL7_CORE_ICONV_H_
#define PPL7_CORE_ICONV_H_

#include <list>
#include <ppl7/types/string.h>

namespace ppl7
{
class Iconv
{
private:
    void* iconv_handle;

public:
    Iconv();
    Iconv(const String& fromEncoding, const String& toEncoding);
    ~Iconv();
    void init(const String& fromEncoding, const String& toEncoding);
    void transcode(const ByteArrayPtr& from, ByteArray& to);
    void transcode(const String& from, String& to);
    String transcode(const String& from);
    static String fromUnicode(const WideString& from, const String& toEncoding);
    static WideString toUnicode(const String& from, const String& fromEncoding);
    static void enumerateCharsets(Array& list);
    static void enumerateCharsets(std::list<ppl7::String>& list);
    static String getLocalCharset();
    static String Utf8ToLocal(const String& text);
    static String LocalToUtf8(const String& text);
};

}; // namespace ppl7

#endif /* PPL7_CORE_ICONV_H_ */