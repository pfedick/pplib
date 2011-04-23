/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.14 $
 * $Date: 2011/04/23 10:34:49 $
 * $Id: ppl7.h,v 1.14 2011/04/23 10:34:49 patrick Exp $
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


#ifndef _PPL7_INCLUDE
#define _PPL7_INCLUDE

#define PPL_VERSION_MAJOR	7
#define PPL_VERSION_MINOR	0
#define PPL_VERSION_BUILD	0
#define PPL_RELEASEDATE		20110421

// Inlcude PPL7 configuration file
#ifndef _PPL7_CONFIG
	#ifdef PPL7LIB
		#ifdef MINGW32
			#include "config.h"
		#elif defined _WIN32
			#include "ppl7-config.h"
		#else
			#include "config.h"
		#endif
	#else
		#include <ppl7-config.h>
	#endif
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef  HAVE_STRINGS_H
#include <strings.h>
#endif

#include <string>

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef WITH_QT
	#include <QString>
	#include <QVariant>
	#include <QByteArray>
#endif


// Exceptions
#ifdef PPL7LIB
	#include "ppl7-exceptions.h"
#else
	#include <ppl7-exceptions.h>
#endif


namespace ppl7 {


}

// Inlcude PPL7 Data-Types
#ifdef PPL7LIB
	#include "ppl7-types.h"
#else
	#include <ppl7-types.h>
#endif

//#ifndef MINGW32
	#ifdef _WIN32
		#define bzero ZeroMemory
		//#define bcopy CopyMemory   // CopyMemory hat andere Parameter-Reihenfolge als bcopy!
		#define strcasecmp stricmp
		#define mode_t ppluint32
	#endif
//#endif




namespace ppl7 {

// Functions
void PrintDebug(const char *format, ...);
void SetGlobalOutput(int type);


void HexDump(const void *address, size_t bytes, bool skipheader);
void HexDump(const void *address, size_t bytes);
String ToBase64(const ByteArrayPtr &bin);
ByteArray FromBase64(const String &str);
ppluint32 Crc32(const void* buffer, size_t size);

String StripSlashes(const String &str);
String Trim(const String &str);
String UpperCase(const String &str);
String LowerCase(const String &str);
int StrCmp(const String &s1, const String &s2);
int StrCaseCmp(const String &s1, const String &s2);
ssize_t Instr (const String &haystack, const String &needle, size_t start=0);
ssize_t InstrCase (const String &haystack, const String &needle, size_t start=0);
ssize_t Instr (const char * haystack, const char * needle, size_t start=0);
ssize_t Instrcase (const char * haystack, const char * needle, size_t start=0);
ssize_t Instr (const wchar_t * haystack, const wchar_t * needle, size_t start=0);
ssize_t Instrcase (const wchar_t * haystack, const wchar_t * needle, size_t start=0);

String Left(const String &str, size_t num);
String Right(const String &str, size_t num);
String Mid(const String &str, size_t start, size_t num=(size_t)-1);
String SubStr(const String &str, size_t start, size_t num=(size_t)-1);
String ToString(const char *fmt, ...);
String Replace(const String &string, const String &search, const String &replace);
bool IsTrue(const String &str);

int PregMatch(const String &expression, const String &subject);
int PregMatch(const String &expression, const String &subject, Array &matches);


size_t rand(size_t min, size_t max);


 //! \brief Synchronisation von Threads
 class Mutex
 {
 	private:
 		void *handle;

 	public:
 		Mutex() throw(OutOfMemoryException);
 		~Mutex() throw();
 		int lock() throw();
 		int unlock() throw();
 		int tryLock() throw();
 		int wait(int milliseconds=0) throw();
 		int signal() throw();
 };





 };	// EOF namespace ppl7


 #endif	// #ifndef _PPL7_INCLUDE
