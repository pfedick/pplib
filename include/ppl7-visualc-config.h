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
#pragma once
#ifndef PPL7_VISUALC_CONFIG
#define PPL7_VISUALC_CONFIG

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// define HAVE_STRING_H 0
#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDARG_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_CRTDEFS_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_CTYPE_H 1
#define HAVE_WCHAR_H 1
#define HAVE_WCTYPE_H 1
#define HAVE_LOCALE_H 1
#define HAVE_ERRNO_H 1
//#define HAVE_WIDEC_H 0
#define HAVE_SYS_TIME_H 1
#define HAVE_FCNTL_H 1
//#define HAVE_SYS_MMAN_H 0
//#define HAVE_UNISTD_H 0
//#define HAVE_DIRENT_H 0
//#define HAVE_SYS_FILE_H 0
#define HAVE_MATH_H
#define HAVE_SIGNAL_H

#define HAVE_STAT


#define HAVE_STRNCPY_S 1
#define HAVE_WCSNCPY_S 1
#define HAVE_WCSTOMBS_S 1
#define HAVE_MBSTOWCS_S 1
#define HAVE_STRERROR_S 1

/*
* External Features
*/
#define HAVE_LIBZ 1
#define HAVE_BZIP2 1
#define HAVE_X86_ASSEMBLER 1
#define HAVE_FREETYPE2 1
#define HAVE_JPEG 1
#define HAVE_PNG 1
#define HAVE_TIFF 1




#define uint8_t unsigned char
#define int8_t signed char
#define uint16_t unsigned short
#define int16_t signed short
#define uint32_t unsigned int
#define int32_t signed int
#define uint64_t unsigned long long
#define int64_t signed long long

#define ppliptr uint64_t

#define pplfloat	float
#define ppldouble	double
#define pplbool		bool
#define pplchar		char
#define ppluchar	unsigned char


#define ssize_t long long
#define mode_t int


#define ICONV_UNICODE "UTF-16LE"

#endif
