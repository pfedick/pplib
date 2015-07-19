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
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "ppl6.h"

namespace ppl6 {

static double Eval(CString &s)
{
	double d1, d2,r;
	CString p1,p2,rep;
	CArray Param;
	//printf ("ppl6::Calc::Eval(%s)\n",(const char*)s);
	while (s.PregMatch("/([-+]{0,1}[0-9\\.,]+)\\^([-+]{0,1}[0-9\\.,]+)/",&Param)) {		// ^
		p1=Param[1];
		p2=Param[2];
		d1=p1.ToDouble();
		d2=p2.ToDouble();
		r=pow(d1,d2);
		//printf ("Match: %s ^ %s = %f\n",Param[1],Param[2],r);
		rep.Sprintf("%s^%s",Param[1],Param[2]);
		rep.PregEscape();
		rep.Shr('/',1);
		rep+="/";
		if (r<0.0) p1.Sprintf("%f",r);
		else p1.Sprintf("+%f",r);
		s.PregReplace((const char*)rep,(const char*)p1);
		//printf ("rep=%s, p1=%s, s=%s\n",(const char*)rep,(const char*)p1,s.GetPtr());
	}

	while (s.PregMatch("/([-+]{0,1}[0-9\\.,]+)\\*([-+]{0,1}[0-9\\.,]+)/",&Param)) {		// *
		p1=Param[1];
		p2=Param[2];
		d1=p1.ToDouble();
		d2=p2.ToDouble();
		r=d1*d2;
		//printf ("Match: %s * %s = %f\n",Param[1],Param[2],r);
		rep.Sprintf("%s*%s",Param[1],Param[2]);
		rep.PregEscape();
		rep.Shr('/',1);
		rep+="/";
		if (r<0.0) p1.Sprintf("%f",r);
		else p1.Sprintf("+%f",r);
		s.PregReplace((const char*)rep,(const char*)p1);
		//printf ("rep=%s, p1=%s, s=%s\n",(const char*)rep,(const char*)p1,s.GetPtr());
	}
	while (s.PregMatch("/([-+]{0,1}[0-9\\.,]+)\\/([-+]{0,1}[0-9\\.,]+)/",&Param)) {		// /
		p1=Param[1];
		p2=Param[2];
		d1=p1.ToDouble();
		d2=p2.ToDouble();
		r=d1/d2;
		//printf ("Match: %s / %s = %f\n",Param[1],Param[2],r);
		rep.Sprintf("%s/%s",Param[1],Param[2]);
		rep.PregEscape();
		rep.Shr('/',1);
		rep+="/";
		if (r<0.0) p1.Sprintf("%f",r);
		else p1.Sprintf("+%f",r);
		s.PregReplace((const char*)rep,(const char*)p1);
		//printf ("rep=%s, p1=%s, s=%s\n",(const char*)rep,(const char*)p1,s.GetPtr());
	}
	while (s.PregMatch("/([-+]{0,1}[0-9\\.,]+)\\+([-+]{0,1}[0-9\\.,]+)/",&Param)) {		// +
		p1=Param[1];
		p2=Param[2];
		d1=p1.ToDouble();
		d2=p2.ToDouble();
		r=d1+d2;
		//printf ("Match: %s + %s = %f\n",Param[1],Param[2],r);
		rep.Sprintf("%s+%s",Param[1],Param[2]);
		rep.PregEscape();
		rep.Shr('/',1);
		rep+="/";
		if (r<0.0) p1.Sprintf("%f",r);
		else p1.Sprintf("+%f",r);
		s.PregReplace((const char*)rep,(const char*)p1);
		//printf ("rep=%s, p1=%s, s=%s\n",(const char*)rep,(const char*)p1,s.GetPtr());
	}
	while (s.PregMatch("/([-+]{0,1}[0-9\\.,]+)\\-([-+]{0,1}[0-9\\.,]+)/",&Param)) {		// -
		p1=Param[1];
		p2=Param[2];
		d1=p1.ToDouble();
		d2=p2.ToDouble();
		r=d1-d2;
		//printf ("Match: %s - %s = %f\n",Param[1],Param[2],r);
		rep.Sprintf("%s-%s",Param[1],Param[2]);
		rep.PregEscape();
		rep.Shr('/',1);
		rep+="/";
		if (r<0.0) p1.Sprintf("%f",r);
		else p1.Sprintf("+%f",r);
		s.PregReplace((const char*)rep,(const char*)p1);
		//printf ("rep=%s, p1=%s, s=%s\n",(const char*)rep,(const char*)p1,s.GetPtr());
	}
	return s.ToDouble();
}

static int ResolveKlammer(CString &s, double &res)
{
	CString sub,t;
	double r;
	char *ptr;
	size_t len;
	int start=-1,end=-1;
	int a;
	bool instring=false;
	startpunkt:
	ptr=(char*)s.GetPtr();
	len=strlen(ptr);
	//printf ("***Formel: %s\n",ptr);
	for (size_t i=0;i<len;i++) {
		a=ptr[i];
		if (a==34 && instring==false) instring=true; // String überspringen
		else if (a==34 && instring==true) instring=false;
		else if (a=='(' && instring==false) start=(int)i+1;
		else if (a==')' && instring==false) {
			if (!start) {
				SetError(349,"Schliessende Klammer ohne oeffnende Klammer an Position %i: %s",i,ptr+i);
				return 0;
			}
			end=i;
			sub=s.Mid(start,end-start);
			//printf ("sub: %s\n",(char*)sub);
			r=Eval(sub);
			sub.Sprintf("%f",r);
			//printf ("sub: %s\n",(char*)sub);
			t=s.Left(start-1);
			t+=sub;
			t+=s.Mid(end+1);
			s.Set(&t);
			goto startpunkt;
		}
	}
	res=s.ToDouble();
	return 1;
}

/*!\ingroup PPLGroupMath
 * \brief Berechnet das Ergebnis eines mathematischen Strings
 *
 * Berechnet das Ergebnis eines mathematischen Strings. Dabei werden die Grundrechenarten
 * +, -, *, / beherscht, sowie die Regel Kammer- vor Punkt- vor Strich-rechnen.
 * \param[in] str Der Parameter \c str kann verschiedene Datentypen darstellen:
 * \li <tt>const char *</tt>
 * \li <tt>CString *</tt>
 * \li <tt>CString</tt>
 *
 * Er beinhaltet jeweils einen String mit der zu berechnenden Formel.
 * \param[out] result In \c result wird das Ergebnis der Rechenoperation gespeichert.
 * \return Liefert 1 zurück, wenn die Berechnung erfolgreich war.
 * oder 0, wenn ein Fehler aufgetreten ist. Der Fehler kann über die Error-Funktionen
 * ausgelesen werden
 *
 * \par Beispiel:
 * In dem folgenden Beispiel wird eine komplexe Formel berechnet und das Ergebnis
 * ausgegeben:
 * \code
void main()
{
	ppl6::CString formel;
	double ergebnis;
	formel="3*(7+4/2-(88-(32*3)*2))+3*5/(2.5+4)";
	printf("Formel: %s\n",(char*)formel);
	if (ppl6::Calc(formel,&ergebnis)) {
		printf("Ergebnis: %f\n",ergebnis);
	} else {
		ppl6::PrintError();
	}
}
 * \endcode
 * \b Ergebnis:
 * \code
Formel: 3*(7+4/2-(88-(32*3)*2))+3*5/(2.5+4)
Ergebnis: 341.307692
\endcode
 */
int Calc(const CString &str, double &result)
{
	CString s;
	s="(";
	s+=str;
	s+=")";
	if (!ResolveKlammer(s,result)) return 0;
	return 1;
}

//! \overload Calc(CString &str, double *res)
int Calc(const char *str, double &result)
/*!\ingroup PPLGroupMath
 */
{
	CString s;
	s="(";
	s+=str;
	s+=")";
	if (!ResolveKlammer(s,result)) return 0;
	return 1;
}


} // end of namespace ppl6
