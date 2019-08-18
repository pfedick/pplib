/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 782 $
 * $Date: 2013-10-10 11:35:46 +0200 (Do, 10 Okt 2013) $
 * $Id: PerlHelper.cpp 782 2013-10-10 09:35:46Z pafe $
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



CString PythonHelper::escapeString(const CString &s)
{
	CString ret=s;
	ret.Replace("\\","\\\\");
	ret.Replace("\"","\\\"");
	ret.Replace("\n","\\n");
	return ret;
}

CString PythonHelper::escapeRegExp(const CString &s)
{
	CString ret=s;
	ret.PregEscape();
	return ret;
}



static ppl6::CString getValue(const ppl6::CString str)
{
	ppl6::CString lstr=str;
	lstr.LCase();
	if (str.IsNumeric() && (str.Instr(",")<0)) return str;
	else if(lstr=="true") return "True";
	else if(lstr=="false") return "False";
	else if(lstr=="null" || lstr=="none") return "None";
	else return ppl6::ToString("\"%s\"",(const char*)PythonHelper::escapeString(str));
}

static ppl6::CString toHashRecurse(const CAssocArray &a, int indention)
{
	CString r;
	CString key;
	CTreeWalker walk;
	a.Reset(walk);
	ARRAY_RESULT res;
	CString indent;
	indent.Repeat(' ',indention);
	while ((res=a.GetNext(walk))) {
		a.GetKey(res,key);
		if (a.IsArray(res)) {
			r.Concatf("%s\"%s\": {",(const char*)indent,(const char*)key);
			CAssocArray *child=a.GetArray(res);
			if (child!=NULL && child->Count()>0) {
				r+="\n";
				r+=toHashRecurse(*child,indention+4);
			}
			r.Concatf("%s},\n",(const char*)indent);
		} else {
			r.Concatf("%s\"%s\": ",(const char*)indent,(const char*)key);
			r+=getValue(a.GetCString(key));
			r+=",\n";
		}
	}
	r.RTrim(",\n");
	r+="\n";
	return r;
}

CString PythonHelper::toHash(const CAssocArray &a, const CString &name, int indention)
{
	CString ret;
	CString indent;
	indent.Repeat(' ',indention);

	if (name.IsEmpty()) return ret;
	ret.Setf("%s%s = {",(const char*)indent,(const char*)name);
	if (a.Count()) {
		ret+="\n";
		ret+=toHashRecurse(a,indention+4);
		ret+=indent;
	}
	ret+="}\n";
	return ret;
}

}	// EOF namespace ppl6
