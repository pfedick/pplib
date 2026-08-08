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

#ifndef PPLIB_BASEEXCEPTION_H_
#define PPLIB_BASEEXCEPTION_H_
#include <exception>
#include <ostream>
#include <stdarg.h>
#include <string>

#include <pplib/types/string.h>

namespace pplib
{

class Exception : public std::exception
{
private:
    char* ErrorText;

public:
    Exception() noexcept;
    Exception(const Exception& other) noexcept;
    Exception& operator=(const Exception& other) noexcept;
    Exception(const char* msg, ...) noexcept;
    Exception(const String& msg) noexcept;
    virtual ~Exception() noexcept;
    virtual const char* what() const noexcept;
    const char* text() const noexcept;
    String toString() const noexcept;
    void print() const;
    void copyText(const char* str) noexcept;
    void copyText(const char* fmt, va_list args) noexcept;
};

std::ostream& operator<<(std::ostream& s, const Exception& e);

#define STR_VALUE(arg) #arg
#define PPLIBEXCEPTION(name, inherit)                                                                                                      \
    class name : public pplib::inherit                                                                                                     \
    {                                                                                                                                      \
    public:                                                                                                                                \
        name() noexcept                                                                                                                    \
        {                                                                                                                                  \
        }                                                                                                                                  \
        name(const char* msg, ...) noexcept                                                                                                \
        {                                                                                                                                  \
            va_list args;                                                                                                                  \
            va_start(args, msg);                                                                                                           \
            copyText(msg, args);                                                                                                           \
            va_end(args);                                                                                                                  \
        }                                                                                                                                  \
        virtual const char* what() const noexcept                                                                                          \
        {                                                                                                                                  \
            return (STR_VALUE(name));                                                                                                      \
        }                                                                                                                                  \
    };

} // namespace pplib

#endif /* PPLIB_BASEEXCEPTION_H_ */
