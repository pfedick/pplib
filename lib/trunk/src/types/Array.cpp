/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.7 $
 * $Date: 2011/04/23 10:34:49 $
 * $Id: Array.cpp,v 1.7 2011/04/23 10:34:49 patrick Exp $
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include "ppl7.h"


namespace ppl7 {


/*!\class Array
 * \ingroup PPLGroupDataTypes
 *
 */


typedef struct {
	String *value;
} ROW;


Array::Array()
{
	type=ARRAY;
	num=0;
	maxnum=0;
	pos=0;
	rows=NULL;
}

Array::Array(const Array &other)
{
	type=ARRAY;
	num=0;
	maxnum=0;
	pos=0;
	rows=NULL;
	add(other);
}

Array::Array(const String &str, const String &delimiter, size_t limit, bool skipemptylines)
{
	type=ARRAY;
	num=0;
	maxnum=0;
	pos=0;
	rows=NULL;
	explode(str,delimiter,limit,skipemptylines);
}

Array::~Array()
{
	clear();
}


void Array::clear()
{
	ROW *r=(ROW*)rows;
	if (r) {
		for (size_t i=0;i<maxnum;i++) {
			if (r[i].value) {
				delete(r[i].value);
				r[i].value=NULL;
			}
		}
		free(rows);
		rows=NULL;
		maxnum=0;
		num=0;
		pos=0;
	}
}

void Array::copy(const Array &other)
{
	clear();
	ROW *r=(ROW*)other.rows;
	for (size_t i=0;i<other.num;i++) {
		if (r[i].value!=NULL) set(i,*r[i].value);
	}
}

void Array::add(const Array &other)
{
	ROW *r=(ROW*)other.rows;
	size_t first=num;
	for (size_t i=0;i<other.num;i++) {
		if (r[i].value!=NULL) set(first+i,*r[i].value);
	}
}

void Array::add(const String &value)
{
	set(num,value);
}

void Array::add(const wchar_t *value, size_t size)
{
	String s;
	s.set(value,size);
	set(num,s);
}

void Array::add(const String &value, size_t size)
{
	set(num,value.left(size));
}

void Array::addf(const char *fmt, ...)
{
	String value;
	va_list args;
	va_start(args, fmt);
	value.vasprintf(fmt,args);
	va_end(args);
	set(num,value);
}

void Array::set(size_t index, const String &value)
{
	ROW *r;
	if (index>=maxnum) {
		// Array muss vergroessert werden
		void *newrows=realloc(rows,(index+10)*sizeof(ROW));
		if (!newrows) {
			throw OutOfMemoryException();
		}
		r=(ROW*)newrows;
		for (size_t i=maxnum;i<index+10;i++) {
			r[i].value=NULL;
		}
		rows=newrows;
		maxnum=index+10;
	}
	r=(ROW*)rows;
	if (value.notEmpty()) {
		if (r[index].value==NULL) {
			r[index].value=new String;
			if (!r[index].value) throw OutOfMemoryException();
		}
		r[index].value->set(value);
	} else {
		if (r[index].value==NULL) {
			delete r[index].value;
			r[index].value=NULL;
		}
	}
}

void Array::setf(size_t index, const char *fmt, ...)
{
	String value;
	va_list args;
	va_start(args, fmt);
	value.vasprintf(fmt,args);
	va_end(args);
	set(index,value);
}


size_t Array::count() const
{
	return num;
}

void Array::list(const String &prefix) const
{
	ROW *r=(ROW*)rows;
	if (prefix.isEmpty()) {
		if ((!rows) || num==0) {
			printf("Array ist leer\n");
		}
		for (size_t i=0;i<num;i++) {
			if (r[i].value!=NULL) printf ("%6zu: %ls\n",i,(const wchar_t*)r[i].value);

		}
	} else {
		if ((!rows) || num==0) {
			printf("Array %ls ist leer\n", (const wchar_t*)prefix);
		}
		for (size_t i=0;i<num;i++) {
			if (r[i].value!=NULL) printf ("%ls, %6zu: %ls\n",(const wchar_t*)prefix, i,(const wchar_t*)r[i].value);
		}

	}
}


String Array::get(size_t index) const
{
	ROW *r=(ROW*)rows;
	if (index<num && r[index].value!=NULL) return r[index].value;
	return String();
}

String Array::operator[](size_t index) const
{
	ROW *r=(ROW*)rows;
	if (index<num && r[index].value!=NULL) return r[index].value;
	return String();
}

String Array::getRandom() const
{
	if (!num) return String();
	ROW *r=(ROW*)rows;
	size_t index=ppl7::rand(0,num-1);
	if (index<num && r[index].value!=NULL) return r[index].value;
	return String();
}

Array &Array::explode(const String &text, const String &delimiter, size_t limit, bool skipemptylines)
{
	if (text.isEmpty()) return *this;
	if (delimiter.isEmpty()) return *this;
	ssize_t p;
	size_t t=delimiter.len();
	size_t count=0;
	wchar_t *etext=(wchar_t*)text.getPtr();
	while (1) {
		count++;
		p=Instr(etext,(const wchar_t*)delimiter,0);
		if (p>=0) {
			if (p==0 && skipemptylines==true) {
				etext+=t;
				continue;
			}
			if (limit>0 && count==limit) {
				add((wchar_t*)etext);
				return *this;
			}
			add((wchar_t*)etext,p);
			etext=etext+p+t;
		} else {
			if (skipemptylines==false || wcslen(etext)>0) add(etext);
			return *this;
		}
	}

	return *this;
}


String Array::implode(const String &trenn) const
{
	String ret;
	for (size_t i=0;i<num;i++) {
		if (i) ret+=trenn;
		ret+=get(i);
	}
	return ret;
}


Array &Array::fromArgs(int argc, const char **argv)
{
	clear();
	for (int i=0;i<argc;i++) {
		add(argv[i]);
	}
	return *this;
}

Array &Array::fromArgs(const String &args)
{
	clear();
	String buffer(args);
	String arg;
	// Kommandozeile in argc und argv[] umwandeln

	size_t l=buffer.len();
	add(args);
	bool inDoubleQuote=false;
	bool inSingleQuote=false;
	size_t start=0;
	for (size_t i=0;i<l;i++) {
		if(buffer[i]==34 && inDoubleQuote==false && inSingleQuote==false) {
			if (i==0) {
				inDoubleQuote=true;
				start=i+1;
			}
			else if (buffer[i-1]!='\\') {
				inDoubleQuote=true;
				start=i+1;
			}
		} else if(buffer[i]=='\'' && inDoubleQuote==false && inSingleQuote==false) {
				if (i==0) {
					inSingleQuote=true;
					start=i+1;
				}
				else if (buffer[i-1]!='\\') {
					inSingleQuote=true;
					start=i+1;
				}

		} else if(buffer[i]==34 && inDoubleQuote==true && buffer[i-1]!='\\') {
			inDoubleQuote=false;
			arg=buffer.mid(start,i-start);
			if (arg.notEmpty()) add(arg);

			//if(argv[argc][0]!=0) argc++;
			start=i+1;
		} else if(buffer[i]=='\'' && inSingleQuote==true && buffer[i-1]!='\\') {
			inSingleQuote=false;
			arg=buffer.mid(start,i-start);
			if (arg.notEmpty()) add(arg);

			//if(argv[argc][0]!=0) argc++;
			start=i+1;
		} else if((buffer[i]==' ' || buffer[i]=='\t') && inDoubleQuote==false && inSingleQuote==false) {
			arg=Trim(buffer.mid(start,i-start));
			if (arg.notEmpty()) add(arg);
			start=i+1;
		}
	}
	if (start<l) {
		arg=Trim(buffer.mid(start,l-start));
		if (arg.notEmpty()) add(arg);
	}
	return *this;
}






} // EOF namespace ppl7
