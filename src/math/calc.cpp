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
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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
#include <stdio.h>
#include <string.h>
#include "ppl7.h"

namespace ppl7 {


static void ResolveAddition(String &expression)
{
	Array matches;
	while (expression.pregMatch("/([-+]{0,1}[0-9\\.,]+)\\+([-+]{0,1}[0-9\\.,]+)/",matches)) {
		double tmp=matches[1].toDouble()+matches[2].toDouble();
		String pattern=matches[1]+"+"+matches[2];
		pattern.pregEscape();
		pattern="/"+pattern+"/";
		String rep;
		if (tmp<0.0) rep.setf("%f",tmp);
		else rep.setf("+%f",tmp);
		expression.pregReplace(pattern,rep);
	}
}

static void ResolveSubtraction(String &expression)
{
	Array matches;
	while (expression.pregMatch("/([-+]{0,1}[0-9\\.,]+)\\-([-+]{0,1}[0-9\\.,]+)/",matches)) {
		double tmp=matches[1].toDouble()-matches[2].toDouble();
		String pattern=matches[1]+"-"+matches[2];
		pattern.pregEscape();
		pattern="/"+pattern+"/";
		String rep;
		if (tmp<0.0) rep.setf("%f",tmp);
		else rep.setf("+%f",tmp);
		expression.pregReplace(pattern,rep);
	}
}

static double Resolve(String &current)
{
	printf ("current=%s\n",(const char*)current);
	while (1) {
		size_t size=current.size();
		size_t start=size+1;
		size_t end=size;
		const char *ptr=current.c_str();
		for (size_t i=0;i<size;i++) {
			if (ptr[i]=='(' && start>size) start=i;
			if (ptr[i]==')') end=i;
		}
		if (start<size) {
			printf ("Loese Klammer auf...\n");
			double tmp=Calc(current.mid(start+1,end-start-1));
			current=current.left(start)+ToString("%f",tmp)+current.mid(end+1);
			printf ("neues current: %s\n",(const char*)current);
		} else {
			break;
		}
	}
	printf ("Berechne: %s\n",(const char*)current);
	ResolveAddition(current);
	ResolveSubtraction(current);
	return current.toDouble();
}

double Calc(const String &expression)
{
	String current=expression;
	current.replace(String(" "),String(""));
	current.replace(String(","),String("."));
	Resolve(current);
	return current.toDouble();
}

}	// EOF namespace ppl6
