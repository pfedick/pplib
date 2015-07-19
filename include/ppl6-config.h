/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: pafe $
 * $Revision: 1.4 $
 * $Date: 2010/04/07 16:47:20 $
 * $Id: ppl6-config.h,v 1.4 2010/04/07 16:47:20 pafe Exp $
 * 
 ******************************************************************************* 
 * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

/*
 * Windows-Spezifische Konfiguration
 * Wird unter UNIX nicht verwendet!
 */
#ifndef _PPL6_CONFIG
#define _PPL6_CONFIG

//#define HAVE_NASM
#define HAVE_LIBZ
#define HAVE_BZIP2

#define HAVE_MYSQL
#define HAVE_PNG
#define HAVE_JPEG

#define HAVE_FREETYPE2
#define HAVE_LAME
#define HAVE_LIBMAD
#define HAVE_DX9
#define HAVE_PCRE
#define HAVE_ICONV
#define ICONV_CONST const

#define HAVE_OPENSSL
//#define HAVE_LIBCURL
//#define HAVE_SDL
#define HAVE_X86_ASSEMBLER


#ifndef HAVE_SYS_STAT_H
#define HAVE_SYS_STAT_H
#endif

#ifndef HAVE_SYS_TYPES_H
#define HAVE_SYS_TYPES_H
#endif

#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H
#endif

#ifndef HAVE_STDIO_H
#define HAVE_STDIO_H
#endif

#ifndef HAVE_SIGNAL_H
#define HAVE_SIGNAL_H
#endif

#ifndef HAVE_WCHAR_H
#define HAVE_WCHAR_H
#endif

#ifndef HAVE_ERRNO_H
#define HAVE_ERRNO_H
#endif

#ifndef HAVE_MATH_H
#define HAVE_MATH_H
#endif


#ifndef MB_CUR_MAX
#define MB_CUR_MAX 6
#endif

#define ppluint8	unsigned char
#define ppluint16	unsigned short
#define ppluint32	unsigned int
#define ppluint64	unsigned __int64

#define pplint8		signed char
#define pplint16	signed short
#define pplint32	signed int
#define pplint64	signed __int64

#define pplfloat	float
#define ppldouble	double
#define pplbool		bool
#define pplchar		char
#define ppluchar	unsigned char
#define pplwchar	unsigned short

#define ppliptr ppluint32

#define HAVE_GETADDRINFO

#ifdef _WIN32

	#ifdef _M_ALPHA
		#define __BIG_ENDIAN__
		#define ICONV_UNICODE	"UTF-16BE"
	#else
		#define __LITTLE_ENDIAN__
		#define ICONV_UNICODE	"UTF-16LE"
	#endif

	#define WIN32_LEAN_AND_MEAN		// Keine MFCs
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x501
	#endif
	// Ab Visual Studio 2005 (Visual C++ 8.0) gibt es einige Änderungen
	#if _MSC_VER >= 1400
		#define _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_DEPRECATE 
		#define _AFX_SECURE_NO_DEPRECATE 
		#define _ATL_SECURE_NO_DEPRECATE
		#pragma warning(disable : 4996)
		#pragma warning(disable : 4800)

		/*
		#define unlink _unlink
		#define strdup _strdup
		#define fileno _fileno
		#define mkdir _mkdir
		#define open _open
		#define close _close
		#define read _read
		*/
	#endif
	#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
	#include <windows.h>
	#ifdef DeleteFile
		#undef DeleteFile
	#endif
	#ifdef CopyFile
		#undef CopyFile
	#endif
	#ifdef MoveFile
		#undef MoveFile
	#endif


#endif


#endif
