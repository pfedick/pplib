/* include/ppl7-config.h.  Generated from ppl7-config.h.in by configure.  */
/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7). 
 * Web: http://www.pfp.de/ppl/
 *  
 * $Author: pafe $
 * $Revision: 396 $
 * $Date: 2012-06-02 15:06:05 +0200 (Sa, 02 Jun 2012) $
 * $Id: ppl7-config.h.in 396 2012-06-02 13:06:05Z pafe $
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
 
#ifndef _PPL7_CONFIG
#define _PPL7_CONFIG

#define HAVE_SYS_TYPES_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STDARG_H 1

/* #undef MINGW32 */

/* #undef size_t */


/*
 * Type definitions
 */

#define ppluint8 unsigned char
#define pplint8 signed char
#define ppluint16 unsigned short
#define pplint16 signed short
#define ppluint32 unsigned int
#define pplint32 signed int
#define ppluint64 unsigned long long
#define pplint64 signed long long

#define pplfloat	float
#define ppldouble	double
#define pplbool		bool
#define pplchar		char
#define ppluchar	unsigned char
#define ppliptr ppluint32


#endif
