/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
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
#ifdef HAVE_STRINGS_H
	#include <strings.h>
#endif


#include "ppl6.h"


namespace ppl6 {



CString PerlHelper::escapeString(const CString &s)
{
	CString ret=s;
	ret.Replace("\\","\\\\");
	ret.Replace("\"","\\\"");
	ret.Replace("@","\\@");
	return ret;
}

CString PerlHelper::escapeRegExp(const CString &s)
{
	CString ret=s;
	ret.PregEscape();
	return ret;
}


static ppl6::CString toHashRecurse(const CAssocArray &a, const CString &name)
{
	CString r;
	CString key;
	CTreeWalker walk;
	a.Reset(walk);
	ARRAY_RESULT res;
	while ((res=a.GetNext(walk))) {
		a.GetKey(res,key);
		if (a.IsArray(res)) {
			CString newName;
			newName.Setf("%s{%s}",(const char*)name,(const char*)key);
			r+=toHashRecurse(*a.GetArray(res),newName);
		} else {
			r.Concatf("%s{%s}=\"%s\";\n",(const char*)name,(const char*)key,(const char*)PerlHelper::escapeString(a.GetChar(res)));
		}
	}
	return r;
}

CString PerlHelper::toHash(const CAssocArray &a, const CString &name)
{
	CString ret;
	if (name.IsEmpty()) return ret;
	ret.Setf("my %%%s;\n",(const char*)name);
	CString n;
	n.Setf("$%s",(const char*)name);
	ret+=toHashRecurse(a,n);
	return ret;
}

}	// EOF namespace ppl6
