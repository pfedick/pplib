/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2024, Patrick Fedick <patrick@pfp.de>
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

#include "prolog_ppl7.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Keine MFCs
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x600
#endif
#define _WINSOCKAPI_ /* Prevent inclusion of winsock.h in windows.h */
#include <windows.h>
#endif

#include "ppl7.h"
#include "ppl7-inet.h"

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

namespace ppl7
{
#ifdef HAVE_LIBCURL
static void addParamsToUrl(const ppl7::HttpRequest& req, CURL* curl, ppl7::String& finalUrl);
static curl_mime* addPostParams(const ppl7::HttpRequest& req, CURL* curl);
#endif

static String BrowserString =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36";
static int http_timeout = 30; // Sekunden
static bool verifySsl = true;
static String CaBundlePath;
static String CaPath;
static String ProxyUrl;
static String ProxyUser;
static String ProxyPassword;
static volatile bool CurlInitialized = false;
static Mutex InitMutex;

struct CurlData
{
    HttpResponse* response;
};

#ifdef HAVE_LIBCURL
static size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t totalSize = size * nmemb;
    CurlData* data = static_cast<CurlData*>(userdata);
    data->response->body.append(ptr, totalSize);
    return totalSize;
}

static size_t header_callback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t totalSize = size * nmemb;
    CurlData* data = static_cast<CurlData*>(userdata);
    ppl7::String headerLine((const char*)ptr, totalSize);

    // Einfaches Parsing: "Key: Value"
    int pos = headerLine.instr(":");
    if (pos > 0) {
        ppl7::String key = headerLine.left(pos).trimmed();
        ppl7::String value = headerLine.mid(pos + 1).trimmed();
        data->response->headers[key] = value;
    }
    return totalSize;
}

#endif

HttpRequest::HttpRequest()
{
    timeout = http_timeout;
    this->verifySsl = verifySsl;
    browserString = BrowserString;
}

extern "C" void curl_cleanup_handler()
{
    if (CurlInitialized) {
        curl_global_cleanup();
        CurlInitialized = false;
    }
}

void HttpClient::Init()
{
#ifdef HAVE_LIBCURL
    InitMutex.lock();
    if (!CurlInitialized) {
        if (curl_global_init(CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL) != 0) {
            InitMutex.unlock();
            throw Curl::InitializationFailedException();
        }
        CurlInitialized = true;
        atexit(curl_cleanup_handler);
    }
    InitMutex.unlock();
#endif
}

void HttpClient::setBrowserString(const String& browser)
{
    BrowserString = browser;
}

void HttpClient::setTimeout(int timeout_seconds)
{
    http_timeout = timeout_seconds;
}

void HttpClient::setVerifySsl(bool verify)
{
    verifySsl = verify;
}

void HttpClient::setCaBundle(const String& path)
{
    CaBundlePath = path;
}

void HttpClient::setCaPath(const String& path)
{
    CaPath = path;
}

void HttpClient::setProxy(const String& proxy)
{
    ProxyUrl = proxy;
}

void HttpClient::setProxyAuth(const String& user, const String& password)
{
    ProxyUser = user;
    ProxyPassword = password;
}

enum class HttpMethod
{
    HTTP_GET,
    HTTP_POST,
    HTTP_HEAD,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE
};

static HttpResponse performHttpRequest(const ppl7::String& url, const HttpRequest& req, HttpMethod method)
{
    HttpClient::Init(); // Sicherstellen, dass global init durch ist
    HttpResponse response;
    response.url = url;
    response.statusCode = 0;
#ifdef HAVE_LIBCURL
    CURL* curl = curl_easy_init();
    if (!curl) return response;

    char errorBuffer[CURL_ERROR_SIZE];
    errorBuffer[0] = 0;
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

    CurlData curlData = {&response};
    ppl7::String finalUrl = url;

    // 1. GET-Parameter an URL anhängen
    if (method == HttpMethod::HTTP_GET && !req.params.empty()) {
        addParamsToUrl(req, curl, finalUrl);
    }

    // 2. Custom Header aufbauen
    struct curl_slist* headerList = nullptr;
    for (auto const& [key, value] : req.headers) {
        ppl7::String h = key + ": " + value;
        headerList = curl_slist_append(headerList, (const char*)h);
    }

    // 3. Curl-Optionen setzen
    curl_easy_setopt(curl, CURLOPT_URL, (const char*)finalUrl);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, (const char*)req.browserString);
    if (req.referer.notEmpty()) curl_easy_setopt(curl, CURLOPT_REFERER, (const char*)req.referer);

    if (req.username.notEmpty()) {
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
        curl_easy_setopt(curl, CURLOPT_USERNAME, (const char*)req.username);
        if (req.password.notEmpty()) curl_easy_setopt(curl, CURLOPT_PASSWORD, (const char*)req.password);
    }

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)req.timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, req.verifySsl ? 1L : 0L);
    if (CaBundlePath.notEmpty()) curl_easy_setopt(curl, CURLOPT_CAINFO, (const char*)CaBundlePath);
    if (CaPath.notEmpty()) curl_easy_setopt(curl, CURLOPT_CAPATH, (const char*)CaPath);

    if (ProxyUrl.notEmpty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, (const char*)ProxyUrl);
        if (ProxyUser.notEmpty()) {
            curl_easy_setopt(curl, CURLOPT_PROXYAUTH, (long)CURLAUTH_ANY);
            curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, (const char*)ProxyUser);
            if (ProxyPassword.notEmpty()) curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, (const char*)ProxyPassword);
        }
    }

    if (headerList) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    // Callbacks
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlData);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &curlData);

    // 4. Methoden-spezifische Logik
    curl_mime* mime = nullptr;
    if (method == HttpMethod::HTTP_POST) {
        mime = addPostParams(req, curl);
    } else if (method == HttpMethod::HTTP_HEAD) {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    } else if (method == HttpMethod::HTTP_PUT) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (!req.body.isEmpty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)req.body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req.body.size());
        }
    } else if (method == HttpMethod::HTTP_PATCH) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (!req.body.isEmpty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)req.body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req.body.size());
        }
    } else if (method == HttpMethod::HTTP_DELETE) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    // 5. PERFORM
    CURLcode res = curl_easy_perform(curl);

    // 6. Ergebnis einsammeln
    if (res != CURLE_OK) {
        if (errorBuffer[0] != 0) {
            response.error = errorBuffer;
        } else {
            response.error = curl_easy_strerror(res);
        }
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        response.statusCode = (int)response_code;

        char* effective_url = nullptr;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective_url);
        if (effective_url) response.url = effective_url;
    }

    // Cleanup
    if (headerList) curl_slist_free_all(headerList);
    if (mime) curl_mime_free(mime);
    curl_easy_cleanup(curl);
#endif
    return response;
}

#ifdef HAVE_LIBCURL
static curl_mime* addPostParams(const ppl7::HttpRequest& req, CURL* curl)
{
    curl_mime* mime = nullptr;
    if (!req.body.isEmpty()) {
        // Roher Body hat Vorrang
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)req.body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req.body.size());
    } else if (!req.params.empty()) {
        // Multipart Form-Data via mime API
        mime = curl_mime_init(curl);
        if (mime) {
            for (auto const& [key, value] : req.params) {
                curl_mimepart* part = curl_mime_addpart(mime);
                curl_mime_name(part, (const char*)key);
                curl_mime_data(part, (const char*)value, CURL_ZERO_TERMINATED);
            }
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        }
    } else {
        // Einfacher POST ohne Daten
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
    }
    return mime;
}

static void addParamsToUrl(const ppl7::HttpRequest& req, CURL* curl, ppl7::String& finalUrl)
{
    ppl7::String queryParams;
    for (auto const& [key, value] : req.params) {
        if (!queryParams.isEmpty()) queryParams += "&";
        char* escapedKey = curl_easy_escape(curl, (const char*)key, key.size());
        char* escapedValue = curl_easy_escape(curl, (const char*)value, value.size());
        queryParams.appendf("%s=%s", escapedKey, escapedValue);
        curl_free(escapedKey);
        curl_free(escapedValue);
    }
    if (finalUrl.instr("?") > 0)
        finalUrl += "&";
    else
        finalUrl += "?";
    finalUrl += queryParams;
}

#endif

HttpResponse HttpClient::get(const ppl7::String& url, const HttpRequest& req)
{
    return performHttpRequest(url, req, HttpMethod::HTTP_GET);
}

HttpResponse HttpClient::post(const ppl7::String& url, const HttpRequest& req)
{
    return performHttpRequest(url, req, HttpMethod::HTTP_POST);
}

HttpResponse HttpClient::put(const ppl7::String& url, const HttpRequest& req)
{
    return performHttpRequest(url, req, HttpMethod::HTTP_PUT);
}

HttpResponse HttpClient::patch(const ppl7::String& url, const HttpRequest& req)
{
    return performHttpRequest(url, req, HttpMethod::HTTP_PATCH);
}

HttpResponse HttpClient::del(const ppl7::String& url, const HttpRequest& req)
{
    return performHttpRequest(url, req, HttpMethod::HTTP_DELETE);
}

HttpResponse HttpClient::head(const ppl7::String& url, const HttpRequest& req)
{
    return performHttpRequest(url, req, HttpMethod::HTTP_HEAD);
}

} // namespace ppl7