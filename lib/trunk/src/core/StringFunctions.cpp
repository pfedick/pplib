/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.4 $
 * $Date: 2011/04/23 10:34:49 $
 * $Id: StringFunctions.cpp,v 1.4 2011/04/23 10:34:49 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#include "prolog.h"
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif

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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "ppl7.h"


namespace ppl7 {

/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
String ToBase64(const ByteArrayPtr &bin)
{
	String res;
    unsigned char in[3], out[4];
    int i, len;
    size_t p=0, filelen=bin.size();

    while( p<filelen ) {
        len = 0;
        for( i = 0; i < 3; i++ ) {
            in[i] = (unsigned char) bin.get(p++);
            if( p<=filelen) {
                len++;
            }
        }
        if( len ) {
            encodeblock( in, out, len );
            for( i = 0; i < 4; i++ ) {
            	res.appendf("%c",out[i]);
            }
        }
    }
    return res;
}

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void decodeblock( unsigned char in[4], unsigned char out[3] )
{
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
ByteArray FromBase64(const String &str)
{
	ByteArray res;
    unsigned char in[4], out[3];
    wchar_t v;
    int i, len;
    size_t p=0, filelen=str.len();
    while( p<filelen) {
        for( len = 0, i = 0; i < 4 && p<filelen; i++ ) {
            v = 0;
            while( p<filelen+1 && v == 0 ) {
                v = str.get(p++);
                v = ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if( v ) {
                    v = ((v == '$') ? 0 : v - 61);
                }
            }
            if( p<filelen+1 ) {
                len++;
                if( v ) {
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else {
                in[i] = 0;
            }
        }
        if( len) {
            decodeblock( in, out );
            res.append(out,len-1);
            len=0;
        }
    }
    return res;
}

String StripSlashes(const String &str)
{
	String ret=str;
	ret.stripSlashes();
	return ret;
}

String Trim(const String &str)
{
	String ret=str;
	ret.trim();
	return ret;
}

String UpperCase(const String &str)
{
	String ret=str;
	ret.upperCase();
	return ret;
}

String LowerCase(const String &str)
{
	String ret=str;
	ret.lowerCase();
	return ret;
}

int StrCmp(const String &s1, const String &s2)
{
	int cmp=s1.strcmp(s2);
	if (cmp<0) return -1;
	if (cmp>0) return 1;
	return 0;
}

int StrCaseCmp(const String &s1, const String &s2)
{
	int cmp=s1.strcasecmp(s2);
	if (cmp<0) return -1;
	if (cmp>0) return 1;
	return 0;
}


/*! \brief Sucht nach Zeichen in einem String
 *
 * \code
int instr (char * string, char * such, unsigned int start);

\endcode
 * Diese Funktionen suchen nach einer Zeichenkette innerhalb eines Strings, wobei
 * die Funktion \c instr zwischen Gross- und Kleinschreibung unterschiedet, und
 * die Funktion \c instrcase nicht.
 *
 * \param haystack ist ein Pointer auf einen Null-terminierten String, der
 * den zu durchsuchenden String enthält.
 * \param needle ist ein Pointer auf einen Null-terminierten String, der
 * den zu suchenden String enthält.
 * \param start ist ein optionaler Parameter, der die Startposition innerhalb des Suchstrings angibt.
 * Der Default ist 0, das heißt der String wurd vom Anfang an durchsucht.
 * \return Wird der String \p needle gefunden, wird seine Position zurückgegeben. Befindet sich der
 * String gleich an erster Stelle des Suchstrings, wird 0 zurückgegeben. Wird der String nicht gefunden,
 * wird -1 zurückgegeben.
 */
ssize_t Instr (const char * haystack, const char * needle, size_t start)
{
	if (!haystack) return -1;
	if (!needle) return -1;
	const char * _t;
	if (start<strlen(haystack)) {
		_t=strstr((haystack+start),needle);
		if (_t!=NULL) {
			return ((ssize_t)(_t-haystack));
		}
	}
	return (-1);
}


/*! \brief Sucht nach Zeichen in einem String und ignoriert Gross-/Kleinschreibung
 *
 * \copydoc Instr (const char * haystack, const char * needle, size_t start)
 */
ssize_t Instrcase (const char * haystack, const char * needle, size_t start)
{
	if (!haystack) return -1;
	if (!needle) return -1;
	const char * _t;
	if (start<strlen(haystack)) {
		_t=strcasestr((haystack+start),needle);
		if (_t!=NULL) {
			return ((long)(_t-haystack));
		}
	}
	return (-1);
}

/*! \brief Sucht nach Zeichen in einem String
 *
 * \copydoc Instr (const char * haystack, const char * needle, size_t start)
 */
ssize_t Instr (const wchar_t * haystack, const wchar_t * needle, size_t start)
{
	if (!haystack) return -1;
	if (!needle) return -1;
	const wchar_t * _t;
	if (start<wcslen(haystack)) {
		_t=wcsstr((haystack+start),needle);
		if (_t!=NULL) {
			return ((ssize_t)(_t-haystack));
		}
	}
	return (-1);
}

/*! \brief Sucht nach Zeichen in einem String und ignoriert Gross-/Kleinschreibung
 *
 * \copydoc Instr (const char * haystack, const char * needle, size_t start)
 */
ssize_t Instrcase (const wchar_t * haystack, const wchar_t * needle, size_t start)
{
	if (!haystack) return -1;
	if (!needle) return -1;
	wchar_t * myHaystack=wcsdup(haystack);
	if (!myHaystack) throw OutOfMemoryException();

	wchar_t * myNeedle=wcsdup(needle);
	if (!myNeedle) throw OutOfMemoryException();

	size_t len=wcslen(myHaystack);
	if (start<len) {
		// String in Kleinbuchstaben umwandeln
		wchar_t wc;
		for (size_t i=0;i<len;i++) {
			wc=myHaystack[i];
			wc=towlower(wc);
			if (wc!=(wchar_t)WEOF) {
				myHaystack[i]=wc;
			}
		}
		// Needle in Kleinbuchstaben umwandeln
		len=wcslen(myNeedle);
		for (size_t i=0;i<len;i++) {
			wc=myNeedle[i];
			wc=towlower(wc);
			if (wc!=(wchar_t)WEOF) {
				myNeedle[i]=wc;
			}
		}

		const wchar_t * _t;
		_t=wcsstr((myHaystack+start),myNeedle);
		if (_t!=NULL) {
			free(myHaystack);
			free(myNeedle);
			return ((ssize_t)(_t-myHaystack));
		}
	}
	free(myHaystack);
	free(myNeedle);
	return (-1);
}

ssize_t Instr (const String &haystack, const String &needle, size_t start)
{
	return haystack.instr(needle,start);
}

ssize_t InstrCase (const String &haystack, const String &needle, size_t start)
{
	return haystack.instrCase(needle,start);
}

String Left(const String &str, size_t num)
{
	return str.left(num);
}

String Right(const String &str, size_t num)
{
	return str.right(num);
}

String Mid(const String &str, size_t start, size_t num)
{
	return str.mid(start,num);
}

String SubStr(const String &str, size_t start, size_t num)
{
	return str.substr(start,num);
}

String ToString(const char *fmt, ...)
{
	String str;
	va_list args;
	va_start(args, fmt);
	str.vasprintf(fmt,args);
	va_end(args);
	return str;
}

String Replace(const String &string, const String &search, const String &replace)
{
	String Tmp=string;
	Tmp.replace(search,replace);
	return Tmp;
}

bool IsTrue(const String &str)
{
	return str.isTrue();
}

bool PregMatch(const String &expression, const String &subject)
{
	return subject.pregMatch(expression);
}

bool PregMatch(const String &expression, const String &subject, Array &matches)
{
	return subject.pregMatch(expression,matches);
}


} // EOF namespace ppl7


