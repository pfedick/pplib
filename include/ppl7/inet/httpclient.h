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

#ifndef PPL7_INET_HTTPCLIENT_H_
#define PPL7_INET_HTTPCLIENT_H_

#include <map>
#include <ppl7/types/string.h>
#include <ppl7/types/bytearray.h>

namespace ppl7
{

class HttpRequest
{
public:
    std::map<String, String> params;
    std::map<String, String> headers;
    std::map<String, String> cookies;
    ppl7::String body;
    ppl7::String userAgent;
    ppl7::String referer;
    ppl7::String username;
    ppl7::String password;
    int timeout;
    bool verifySsl;
    HttpRequest();
};

class HttpResponse
{
private:
public:
    int statusCode;
    ppl7::String url;
    ByteArray body;
    std::map<String, String> headers;
    ppl7::String error; // Falls Curl-Fehler auftraten
};

class HttpClient
{
public:
    static void Init();
    static void setUserAgent(const String& agent);
    static void setTimeout(int timeout_seconds);
    static void setVerifySsl(bool verify);
    static void setCaBundle(const String& path);
    static void setCaPath(const String& path);
    static void setProxy(const String& proxy);
    static void setProxyAuth(const String& user, const String& password);
    static HttpResponse get(const ppl7::String& url, const HttpRequest& req = HttpRequest());
    static HttpResponse post(const ppl7::String& url, const HttpRequest& req = HttpRequest());
    static HttpResponse put(const ppl7::String& url, const HttpRequest& req = HttpRequest());
    static HttpResponse patch(const ppl7::String& url, const HttpRequest& req = HttpRequest());
    static HttpResponse del(const ppl7::String& url, const HttpRequest& req = HttpRequest());
    static HttpResponse head(const ppl7::String& url, const HttpRequest& req = HttpRequest());
};

} // namespace ppl7

#endif /* PPL7_INET_HTTPCLIENT_H_ */
