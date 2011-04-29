/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#ifdef HAVE_PCRE
//#define PCRE_STATIC
#include <pcre.h>
#endif

namespace ppl7 {

Variant::Variant()
{
	type=UNKNOWN;
}


int Variant::dataType() const
{
	return type;
}

bool Variant::isType(int type) const
{
	if (this->type==type) return true;
	return false;
}

bool Variant::isString() const
{
	if (type==STRING) return true;
	return false;
}

bool Variant::isArray() const
{
	if (type==ARRAY) return true;
	return false;
}

bool Variant::isAssocArray() const
{
	if (type==ASSOCARRAY) return true;
	return false;
}

bool Variant::isByteArray() const
{
	if (type==BYTEARRAY) return true;
	return false;
}

bool Variant::isByteArrayPtr() const
{
	if (type==BYTEARRAYPTR) return true;
	return false;
}


const String& Variant::toString() const
{
	if (type!=STRING) throw TypeConversionException();
	return static_cast<const String&>(*this);
}

const Array& Variant::toArray() const
{
	if (type!=ARRAY) throw TypeConversionException();
	return static_cast<const Array&>(*this);
}

/*
AssocArray& Variant::toAssocArray()
{
	if (type!=ASSOCARRAY) throw TypeConversionException();
	//return static_cast<AssocArray&>(*this);
}
*/

const ByteArray& Variant::toByteArray() const
{
	if (type!=BYTEARRAY) throw TypeConversionException();
	return static_cast<const ByteArray&>(*this);
}

const ByteArrayPtr& Variant::toByteArrayPtr() const
{
	if (type!=BYTEARRAYPTR && type!=BYTEARRAY) throw TypeConversionException();
	return static_cast<const ByteArrayPtr&>(*this);
}


} // EOF namespace ppl7


