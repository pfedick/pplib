/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.4 $
 * $Date: 2011/04/21 19:49:03 $
 * $Id: Exceptions.cpp,v 1.4 2011/04/21 19:49:03 patrick Exp $
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

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "ppl7.h"

namespace ppl7 {

Exception::Exception() throw()
{
	ErrorText=NULL;
}

Exception::~Exception() throw()
{
	if (ErrorText) free(ErrorText);
}

const char* Exception::what() const throw()
{
	return "PPLException";
}

Exception::Exception(const Exception &other) throw()
{
	if (other.ErrorText) {
		ErrorText=strdup(other.ErrorText);
	} else {
		ErrorText=NULL;
	}
}

Exception& Exception::operator= (const Exception &other) throw()
{
	if (other.ErrorText) {
		ErrorText=strdup(other.ErrorText);
	} else {
		ErrorText=NULL;
	}
	return *this;
}

Exception::Exception(const char *msg, ...)
{
	if (msg) {
		String Msg;
		va_list args;
		va_start(args, msg);
		Msg.vasprintf(msg,args);
		va_end(args);
		ErrorText=strdup((const char*)Msg.toUtf8());
	} else {
		ErrorText=NULL;
	}
}

void Exception::copyText(const char *str) throw()
{
	free(ErrorText);
	ErrorText=strdup(str);
}

void Exception::copyText(const char *fmt, va_list args) throw()
{
	free(ErrorText);
	String Msg;
	Msg.vasprintf(fmt,args);
	ErrorText=strdup((const char*)Msg.toUtf8());
}

const char* Exception::text() const throw()
{
	if (ErrorText) return ErrorText;
	else return "";
}

String Exception::toString() const throw()
{
	String str;
	str.setf("%s",what());
	if (ErrorText) str.appendf(" [%s]",(const char*)ErrorText);
	return str;
}

void Exception::print() const
{
	PrintDebug("Exception: %s",what());
	if (ErrorText) PrintDebug(" [%s]",(const char*)ErrorText);
	PrintDebug ("\n");
}



}
