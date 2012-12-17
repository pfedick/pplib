/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 89 $
 * $Date: 2011-05-07 12:03:12 +0200 (Sa, 07. Mai 2011) $
 * $Id: Mutex.cpp 89 2011-05-07 10:03:12Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/core/Mutex.cpp $
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


#include "ppl7.h"


namespace ppl7 {



String PerlHelper::escapeString(const String &s)
{
	String ret=s;
	ret.replace("\"","\\\"");
	ret.replace("@","\\@");
	return ret;
}

String PerlHelper::escapeRegExp(const String &s)
{
	String ret=s;
	ret.pregEscape();
	return ret;
}


static String toHashRecurse(const AssocArray &a, const String &name)
{
	String r;
	String key;
	AssocArray::Iterator it;
	a.reset(it);
	while (a.getNext(it)) {
		const String &key=it.key();
		const Variant &res=it.value();
		if (res.isAssocArray()) {
			String newName;
			newName=name+"{"+key+"}";
			r+=toHashRecurse(res.toAssocArray(),newName);
		} else {
			r+=name+"{"+key+"}=\""+PerlHelper::escapeString(a.toString())+"\";\n";
		}
	}
	return r;
}

String PerlHelper::toHash(const AssocArray &a, const String &name)
{
	String ret;
	ret="my %"+name+";\n";
	String n;
	n="$"+name;
	ret+=toHashRecurse(a,n);
	return ret;
}

}	// EOF namespace ppl7
