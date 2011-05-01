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

static int	printdebug=0;

/*!\brief Ausgabe für Debug-Output angeben
 *
 * Mit dieser Funktion wird festgelegt, an welcher Stelle die Textausgabe erfolgen soll.
 * Einige Funktionen (z.B. CAssocArray::List) geben Informationen aus. Standardmäßig werden diese
 * per STDOUT auf die Konsole ausgegeben. Unter Windows gibt es aber manchmal keine Konsole. Hier kann
 * es dann ganz hilfreich sein die Ausgabe im Debugger von Visual Studio zu sehen.
 *
 * \param[in] type 0=STDOUT, 1=VisualStudio Debugger
 */
void SetGlobalOutput(int type)
{
	printdebug=type;
}

/*!\brief Interne Funktion zur Ausgabe von Text
 *
 * Diese Funktion dient als Ersatz für "printf" und wird intern von einigen Funktionen/Klasse zur
 * Ausgabe von Text verwendet. Über die Funktion SetGlobalOutput kann bestimmt werden, ob dieser
 * Text per STDOUT auf die Konsole ausgegeben werden soll oder beispielsweise im Debugger von
 * VisualStudio unter Windows.
 *
 * \param[in] format Formatstring für den Text
 * \param[in] ...    Optionale Parameter, die im Formatstring eingesetzt werden sollen
 */
void PrintDebug(const char *format, ...)
{
	if (!format) return;
	char *buff=NULL;
	va_list args;
	va_start(args, format);
	if (vasprintf (&buff, format, args)<0) {
		va_end(args);
		return;
	}
	va_end(args);
	if (!buff) return;
	if (printdebug==1) {
#ifdef WIN32
		OutputDebugString(buff);
#endif
	} else {
		printf("%s",buff);
	}
	free(buff);
}

/*!\brief Interne Funktion zur Ausgabe von Text
 *
 * Diese Funktion dient als Ersatz für "printf" und wird intern von einigen Funktionen/Klasse zur
 * Ausgabe von Text verwendet. Über die Funktion SetGlobalOutput kann bestimmt werden, ob dieser
 * Text per STDOUT auf die Konsole ausgegeben werden soll oder beispielsweise im Debugger von
 * VisualStudio unter Windows.
 *
 * \param[in] format Formatstring für den Text
 * \param[in] ...    Optionale Parameter, die im Formatstring eingesetzt werden sollen
 */
void PrintDebugTime(const char *format, ...)
{
	if (!format) return;
	char *buff=NULL;
	va_list args;
	va_start(args, format);
	if (vasprintf (&buff, format, args)<0) {
		va_end(args);
		return;
	}
	va_end(args);
	if (!buff) return;
	DateTime now;
	now.setCurrentTime();
	String Time=now.getISO8601withMsec();
	Time+=": ";

	if (printdebug==1) {
#ifdef WIN32
		OutputDebugString((const char*)Time.toLocalEncoding());
		OutputDebugString(buff);
#endif
	} else {
		printf("%ls%s",(const wchar_t*)Time,buff);
	}
	free(buff);
}

void HexDump(const void *address, size_t bytes, bool skipheader)
{
    char buff[1024], tmp[10], cleartext[20];
    if (!skipheader) {
    	sprintf (buff,"HEXDUMP: %zi Bytes starting at Address %p:",bytes,address);
    	printf("%s\n",buff);
    }

    char *_adresse=(char*)address;
    int spalte=0;
    sprintf (buff,"%p: ",_adresse);
    bzero(cleartext,20);
    for (size_t i=0;i<bytes;i++) {
        if (spalte>15) {
            strcat(buff,"                                                               ");
            buff[60]=0;
            strcat (buff,": ");
            strcat (buff,cleartext);
            printf("%s\n",buff);
            sprintf (buff,"%p: ",_adresse+i);
            bzero(cleartext,20);
            spalte=0;
        }
        sprintf (tmp,"%02X ",(ppluint8)_adresse[i]);
        strcat (buff,tmp);
        if ((ppluint8)_adresse[i]>31 && (ppluint8)_adresse[i]<128)  cleartext[spalte]=(ppluint8)_adresse[i];
        else cleartext[spalte]='.';
        spalte++;
    }

    if (spalte>0) {
        strcat(buff,"                                                               ");
        buff[60]=0;
        strcat (buff,": ");
        strcat (buff,cleartext);
        printf("%s\n",buff);
    }
    if (!skipheader) printf("\n");
}

void HexDump(const void *address, size_t bytes)
{
	HexDump(address,bytes,false);
}


} // EOF namespace ppl7


