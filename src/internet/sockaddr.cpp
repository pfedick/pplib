/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 595 $
 * $Date: 2013-04-26 21:18:42 +0200 (Fr, 26. Apr 2013) $
 * $Id: resolver.cpp 595 2013-04-26 19:18:42Z pafe $
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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif


#include "ppl7.h"
#include "ppl7-inet.h"
//#include "socket.h"

namespace ppl7 {

SockAddr::SockAddr()
{
	saddr=NULL;
	addrlen=0;
}

SockAddr::SockAddr(const SockAddr &other)
{
	if (other.saddr!=NULL) {
		saddr=malloc(other.addrlen);
		if (!saddr) throw OutOfMemoryException();
		memcpy(saddr,other.saddr,other.addrlen);
		addrlen=other.addrlen;
	} else {
		saddr=NULL;
		addrlen=0;
	}
}

SockAddr::SockAddr(const void *addr, size_t addrlen)
{
	if (!addr) throw ppl7::IllegalArgumentException();
	saddr=malloc(addrlen);
	if (!saddr) throw OutOfMemoryException();
	memcpy(saddr,addr,addrlen);
	this->addrlen=addrlen;
}

SockAddr::~SockAddr()
{
	free(saddr);
}

void *SockAddr::addr() const
{
	return saddr;
}

size_t SockAddr::size() const
{
	return addrlen;
}

SockAddr &SockAddr::operator=(const SockAddr &other)
{
	free(saddr);
	if (other.saddr!=NULL) {
		saddr=malloc(other.addrlen);
		if (!saddr) throw OutOfMemoryException();
		memcpy(saddr,other.saddr,other.addrlen);
		addrlen=other.addrlen;
	} else {
		saddr=NULL;
		addrlen=0;
	}
	return *this;
}

String SockAddr::toString() const
{
	if (!saddr) throw InvalidIpAddressException("No IP-Address stored");
	char buffer[128];
	const char *res=NULL;
	if (((struct sockaddr_in*)saddr)->sin_family==AF_INET) {
		res=inet_ntop(((struct sockaddr_in*)saddr)->sin_family,
			&((struct sockaddr_in*)saddr)->sin_addr,
			buffer,
			((struct sockaddr_in*)saddr)->sin_len
			);
	} else if (((struct sockaddr_in*)saddr)->sin_family==AF_INET6) {
		res=inet_ntop(((struct sockaddr_in6*)saddr)->sin6_family,
			&((struct sockaddr_in6*)saddr)->sin6_addr,
			buffer,
			((struct sockaddr_in6*)saddr)->sin6_len
			);
	} else {
		throw InvalidIpAddressException();
	}
	if (!res) throw InvalidIpAddressException();
	return String(res);
}

SockAddr SockAddr::fromString(const String &ip)
{
	struct sockaddr_storage sa;
	memset(&sa,0,sizeof(sa));

	if (ip.instr(":") >= 0) {
		sa.ss_family = AF_INET6;
		sa.ss_len = sizeof(struct sockaddr_in6);
		struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &sa;
		if (inet_pton(AF_INET6, (const char*) ip, &addr->sin6_addr) <= 0)
			throw InvalidIpAddressException(ip);
		return SockAddr(addr, addr->sin6_len);
	}
	sa.ss_family = AF_INET;
	sa.ss_len = sizeof(struct sockaddr_in);
	struct sockaddr_in *addr = (struct sockaddr_in *) &sa;
	if (inet_pton(AF_INET, (const char*) ip, &addr->sin_addr) <= 0)
		throw InvalidIpAddressException(ip);
	return SockAddr(addr, addr->sin_len);
}

SockAddr::operator String() const
{
	return toString();
}

void SockAddr::setAddr(const void *addr, size_t addrlen)
{
	free(saddr);
	saddr=NULL;
	addrlen=0;
	if (!addr) throw ppl7::IllegalArgumentException();
	saddr=malloc(addrlen);
	if (!saddr) throw OutOfMemoryException();
	memcpy(saddr,addr,addrlen);
	this->addrlen=addrlen;
}

void SockAddr::setAddr(const String &addr)
{
	SockAddr other=SockAddr::fromString(addr);
	if (other.saddr!=NULL) {
		saddr=malloc(other.addrlen);
		if (!saddr) throw OutOfMemoryException();
		memcpy(saddr,other.saddr,other.addrlen);
		addrlen=other.addrlen;
	} else {
		saddr=NULL;
		addrlen=0;
	}
}




}	// EOF namespace ppl7
