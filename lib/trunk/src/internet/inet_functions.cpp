/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: resolver.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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

#include <time.h>
#ifdef _WIN32
    #include <winsock2.h>
	#include <Ws2tcpip.h>
#else
	#ifdef HAVE_UNISTD_H
    #include <unistd.h>
	#endif
	#ifdef HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
	#endif
	#ifdef HAVE_SYS_POLL_H
    #include <sys/poll.h>
	#endif
	#ifdef HAVE_NETINET_IN_H
    #include <netinet/in.h>
	#endif
	#ifdef HAVE_NETDB_H
    #include <netdb.h>
	#endif
	#ifdef HAVE_ARPA_INET_H
    #include <arpa/inet.h>
	#endif
	#include <netdb.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SIGNAL_H
    #include <signal.h>
#endif
#ifdef HAVE_LIBIDN
#include <idna.h>
#ifdef _WIN32
#include <idn-free.h>
#endif
#endif



#include "ppl7.h"
#include "ppl7-inet.h"
//#include "socket.h"

namespace ppl7 {


/*!\brief Ace-Form aus einem IDN-String berechnen
 *
 * \desc
 * Wandelt einen IDN-String in seine Ace-Form um.
 *
 * \param[in] idn Der IDN-String
 * \return Liefert einen neuen String mit der Ace-Form des Domainnamens zurück.
 * \exception IdnConversionException Wird geworfen, wenn der Domainname im String \p idn
 * nicht konvertiert werden kann.
 */
String Idn2Ace(const String &idn)
{
#ifdef HAVE_LIBIDN
	String ace;
	char *a=NULL;
	ByteArray ucs4=idn.toUCS4();
	if (IDNA_SUCCESS==idna_to_ascii_4z((const uint32_t*)ucs4.ptr(),&a,0) && a!=NULL) {
		ace.set(a);
#ifdef _WIN32
		idn_free(a);
#else
		free(a);
#endif
		return ace;
    }
#ifdef _WIN32
		idn_free(a);
#else
		free(a);
#endif

	throw IdnConversionException(idn);
#else
	throw UnsupportedFeatureException("libidn");
#endif
}

/*!\brief Ace-Form einer Domain in die IDN-Form umwandeln
 *
 * \desc
 * Wandelt einen ACE-String in seine lesbare IDN-Form um.
 *
 * \param[in] ace Der ACE-String
 * \return Liefert einen neuen String mit der IDN-Form des Domainnamens zurück.
 * \exception IdnConversionException Wird geworfen, wenn der ACE-String nicht
 * umgewandelt werden kann.
 */
String Ace2Idn(const String &ace)
{
#ifdef HAVE_LIBIDN
	String idn;
	uint32_t *a=NULL;
	ByteArray ucs4=ace.toUCS4();
    if (IDNA_SUCCESS==idna_to_unicode_4z4z((const uint32_t*)ucs4.ptr(), &a,0) && a!=NULL) {
		idn.fromUCS4(a);
#ifdef _WIN32
		idn_free(a);
#else
		free(a);
#endif
		return idn;
    }
#ifdef _WIN32
		idn_free(a);
#else
		free(a);
#endif

	throw IdnConversionException(ace);
#else
	throw UnsupportedFeatureException("libidn");
#endif
}

#define MAXL (size_t) 75    /* 76th position only used by continuation = */

/*!\brief Konvertiert einen 8Bit-String in Quoted Printable (RFC-822)
 *
 * \desc
 * Diese Funktion wandelt einen String in Quoted Printable um.
 *
 * \param[in] source Der zu konvertierende String
 * \return Gibt den String als Quoted Printable zurück
 */
String ToQuotedPrintable (const String &source)
{
	unsigned char *src=(unsigned char *)source.getPtr();
	size_t srcl=source.size();
	unsigned long lp = 0;
	unsigned char *ret = (unsigned char *) malloc ((size_t) (3*srcl + 3*(((3*srcl)/MAXL) + 1)));
	unsigned char *d = ret;
	const char *hex = (const char*)"0123456789ABCDEF";
	unsigned char c;
	while (srcl--) {      /* for each character */
		/* true line break? */
		if (((c = *src++) == '\012') && srcl) {
			*d++ = '\012'; *d++ = *src++; srcl--;
			lp = 0;           /* reset line count */
		}
		else {          /* not a line break */
			/* quoting required? */
			if (iscntrl (c) || (c == 0x7f) || (c & 0x80) || (c == '=') ||
					((c == ' ') && (*src == '\012'))) {
				if ((lp += 3) > MAXL) { /* yes, would line overflow? */
					*d++ = '='; *d++ = '\012';
					lp = 2;       /* set line count */
				}
				*d++ = '=';     /* quote character */
				*d++ = hex[c >> 4]; /* high order 4 bits */
				*d++ = hex[c & 0xf];    /* low order 4 bits */
			}
			else {            /* ordinary character */
				if ((++lp) > MAXL) {    /* would line overflow? */
					*d++ = '='; *d++ = '\012';
					lp = 1;       /* set line count */
				}
				*d++ = c;       /* ordinary character */
			}
		}
	}
	*d = '\0';            /* tie off destination */
	size_t len = d - ret;       /* calculate true size */
	String rr;
	rr.set((const char*)ret,len);
	//target->Replace("=0A","=0A\n");
	free(ret);
	return rr;
}




}	// namespace ppl7

