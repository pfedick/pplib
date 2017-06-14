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
 * Copyright (c) 2017, Patrick Fedick <patrick@pfp.de>
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

#include <time.h>
#ifdef _WIN32
    #include <winsock2.h>
	#include <Ws2tcpip.h>
	#include <windows.h>
#else
	#ifdef HAVE_UNISTD_H
    #include <unistd.h>
	#endif
	#ifdef HAVE_SYS_SOCKET_H
    #include <sys/socket.h>
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


#include "ppl7.h"
#include "ppl7-inet.h"
//#include "socket.h"

namespace ppl7 {

IPNetwork::IPNetwork()
{
	_prefixlen=-1;
}

IPNetwork::IPNetwork(const IPNetwork &other)
{
	_prefixlen=-1;
	set(other);
}

IPNetwork::IPNetwork(const String &other)
{
	_prefixlen=-1;
	set(other);
}

IPNetwork &IPNetwork::operator=(const IPNetwork &other)
{
	set(other);
	return *this;
}

IPNetwork &IPNetwork::operator=(const String &other)
{
	set(other);
	return *this;
}


void IPNetwork::set(const IPAddress &other, int prefixlen)
{
	_addr=other.mask(prefixlen);
	_prefixlen=prefixlen;
}

void IPNetwork::set(const IPNetwork &other)
{
	_prefixlen=other._prefixlen;
	_addr=other._addr;
}

void IPNetwork::set(const String &network)
{
	int t=network.instr("/");
	if (t<0) throw InvalidNetworkAddressException(network);
	IPAddress addr=network.left(t);
	String mask=network.mid(t+1);
	int prefixlen=0;
	if (mask.instr(".")>=0) {
		// netzmaske
	} else {
		prefixlen=mask.toInt();
	}
	_addr=addr.mask(prefixlen);
	_prefixlen=prefixlen;
}

IPAddress::IP_FAMILY IPNetwork::family() const
{
	return _addr.family();
}

IPAddress IPNetwork::first() const
{
	if (_prefixlen<0) throw InvalidNetworkAddressException();
	return _addr;
}

IPAddress IPNetwork::last() const
{
	if (_prefixlen<0) throw InvalidNetworkAddressException();
	// TODO
	return _addr;
}

int IPNetwork::prefixlen() const
{
	if (_prefixlen<0) throw InvalidNetworkAddressException();
	return _prefixlen;
}

String IPNetwork::toString() const
{
	if (_prefixlen<0) throw InvalidNetworkAddressException();
	return ppl7::ToString("%s/%d",(const char*)_addr.toString(),_prefixlen);
}

IPNetwork::operator String() const
{
	return toString();
}


}	// namespace ppl7

