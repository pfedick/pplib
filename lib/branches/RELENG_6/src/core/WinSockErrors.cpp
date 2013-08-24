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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
	#include <errno.h>
#endif
#ifdef HAVE_NETDB_H
	#include <netdb.h>
#endif

#include "ppl6.h"
#ifdef _WIN32
#include <winsock2.h>
#include <winerror.h>
#endif

namespace ppl6 {
void SetSocketError()
{
	SetError(TranslateSocketError());
}

int TranslateSocketError()
{
#ifdef _WIN32
	int e=WSAGetLastError();
	switch (e) {
		case 0: return 0;
		case WSAEBADF: return 72;
		case WSAEACCES: return 367;
		case WSAEFAULT: return 307;
		case WSAEINVAL: return 279;
		case WSAEMFILE: return 379;
		case WSAEWOULDBLOCK: return 277;
		case WSAEINPROGRESS: return 377;
		case WSAEALREADY: return 454;
		case WSAENOTSOCK: return 455;
		case WSAEDESTADDRREQ: return 456;
		case WSAEMSGSIZE: return 381;
		case WSAEPROTOTYPE: return 457;
		case WSAENOPROTOOPT: return 458;
		case WSAEPROTONOSUPPORT: return 459;
		case WSAESOCKTNOSUPPORT: return 460;
		case WSAEOPNOTSUPP: return 278;
		case WSAEPFNOSUPPORT: return 461;
		case WSAEAFNOSUPPORT: return 462;
		case WSAEADDRINUSE: return 463;
		case WSAEADDRNOTAVAIL: return 406;
		case WSAENETDOWN: return 464;
		case WSAENETUNREACH: return 465;
		case WSAENETRESET: return 466;
		case WSAECONNABORTED: return 467;
		case WSAECONNRESET: return 409;
		case WSAENOBUFS: return 468;
		case WSAEISCONN: return 469;
		case WSAENOTCONN: return 408;
		case WSAESHUTDOWN: return 470;
		case WSAETOOMANYREFS: return 471;
		case WSAETIMEDOUT: return 174;
		case WSAECONNREFUSED: return 407;
		case WSAELOOP: return 368;
		case WSAENAMETOOLONG: return 363;
		case WSAEHOSTDOWN: return 472;
		case WSAEHOSTUNREACH: return 473;
		case WSAENOTEMPTY: return 386;
		case WSAEPROCLIM: return 474;
		case WSAEUSERS: return 475;
		case WSAEDQUOT: return 476;
		case WSAESTALE: return 477;
		case WSAEREMOTE: return 478;
		case WSASYSNOTREADY: return 490;
		case WSAVERNOTSUPPORTED: return 491;
		case WSANOTINITIALISED: return 492;
		case WSAEDISCON: return 493;
		case WSAENOMORE: return 494;
		case WSAECANCELLED: return 495;
		case WSAEINVALIDPROCTABLE: return 496;
		case WSAEINVALIDPROVIDER: return 497;
		case WSAEPROVIDERFAILEDINIT: return 498;
		case WSASYSCALLFAILURE: return 499;
		case WSASERVICE_NOT_FOUND: return 500;
		case WSATYPE_NOT_FOUND: return 501;
		case WSA_E_NO_MORE: return 502;
		case WSA_E_CANCELLED: return 503;
		case WSAEREFUSED: return 504;
		case WSAHOST_NOT_FOUND: return 505;
		case WSATRY_AGAIN: return 506;
		case WSANO_RECOVERY: return 507;
		case WSANO_DATA: return 508;
		case WSAEINTR: return 310;
	}
#else
	int e=TranslateErrno(errno);
	if (e==3) e=312;
	return e;
#endif
	return 312;
}

int TranslateGaiError(int e)
{
#ifdef _WIN32
	return TranslateSocketError();
#else
#ifndef CPPCHECK
	switch (e) {
#ifdef EAI_AGAIN
		case EAI_AGAIN: return 479;	// temporary failure in name resolution
#endif
#ifdef EAI_ADDRFAMILY
		case EAI_ADDRFAMILY: return 483;
#endif
#ifdef EAI_BADFLAGS
		case EAI_BADFLAGS: return 480;	// invalid value for ai_flags
#endif
#ifdef EAI_BADHINTS
		case EAI_BADHINTS: return 481;	// invalid value for hints
#endif
#ifdef EAI_FAIL
		case EAI_FAIL: return 482;		// non-recoverable failure in name resolution
#endif
#ifdef EAI_FAMILY
		case EAI_FAMILY: return 483;	// ai_family not supported
#endif
#ifdef EAI_MEMORY
		case EAI_MEMORY: return 484;	// memory allocation failure
#endif
#ifdef EAI_NONAME
		case EAI_NONAME: return 485;	// hostname or servname not provided, or not known
#endif
#ifdef EAI_OVERFLOW
		case EAI_OVERFLOW: return 486;	// argument buffer overflow
#endif
#ifdef EAI_PROTOCOL
		case EAI_PROTOCOL: return 487;	// resolved protocol is unknown
#endif
#ifdef EAI_SERVICE
		case EAI_SERVICE: return 488;	// servname not supported for ai_socktype
#endif
#ifdef EAI_SOCKTYPE
		case EAI_SOCKTYPE: return 489;	// ai_socktype not supported
#endif
#ifdef EAI_SYSTEM
		case EAI_SYSTEM: return TranslateSocketError();	//  system error returned in errno
#endif
#ifdef EAI_NODATA
		case EAI_NODATA: return 509;
#endif
	
	}
#endif

#endif //#ifndef CPPCHECK
	return 312;
}

}	// EOF namespace ppl6
