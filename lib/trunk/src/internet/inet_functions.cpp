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
		free(a);
		return ace;
    }
	free(a);
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
		free(a);
		return idn;
    }
	free(a);
	throw IdnConversionException(ace);
#else
	throw UnsupportedFeatureException("libidn");
#endif
}




}	// namespace ppl7

