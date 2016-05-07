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
 * Copyright (c) 2015, Patrick Fedick <patrick@pfp.de>
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

#ifdef _WIN32
    #include <winsock2.h>
	#include <Ws2tcpip.h>
	#include <windows.h>
#else

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

#endif


#include "ppl7.h"
#include "ppl7-inet.h"
//#include "socket.h"

namespace ppl7 {

/*!\class SockAddr
 * \ingroup PPLGroupInternet
 *
 * \brief Klasse zum Speichern eine Socket-Adresse
 *
 * \header \#include <ppl7-inet.h>
 *
 * \desc
 * Diese Klasse kann zum Speichern eine Socket-Adresse verwendet werden und unterstützt sowohl IPv4
 * als auch IPv6. Die Adresse wird dabei in einem Format gespeichert, wie es auch von den
 * Socket-Funktionen des Betriebssystems verstanden wird.
 *
 * \example Verwendung in der Socket-Funktion "connect"
 * \code
 * ppl7::SockAddr addr=ppl7::SockAddr::fromString("127.0.0.1")
 * struct sockaddr_in* in_addr=(struct sockaddr_in*)addr.addr();
 * in_addr->sin_port=7;
 * connect(sockfd,(struct sockaddr *)in_addr, in_addr->sin_len);
 * \endcode
 */

/*!\var SockAddr::saddr
 * \brief %Pointer auf den Speicherbereich mit der Socket-Struktur
 */

/*!\var SockAddr::addrlen
 * \brief Länge der Socket-Struktur in Bytes
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Konstruktor der Klasse
 */
SockAddr::SockAddr()
{
	saddr=NULL;
	addrlen=0;
}

/*!\brief Copy-Konstruktor
 * \desc
 * Copy-Konstruktor
 * @param other Anderes SockAddr-Objekt, von dem Kopiert werden soll
 */
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

/*!\brief Kopieren aus einer sockaddr-Structure
 *
 * \desc
 * Kopiert die Adresse aus einer sockaddr-Struktur
 * @param addr Muss ein Pointer auf eine struct sockaddr, sockaddr_in oder sockaddr_in6 sein
 * @param addrlen Länge der Struktur
 */
SockAddr::SockAddr(const void *addr, size_t addrlen)
{
	this->addrlen=0;
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

/*!\brief Adresse der Socket-Struktur auslesen
 *
 * \desc
 * Adresse der Socket-Struktur auslesen
 * @return Pointer auf eine Socket-Struktur
 */
void *SockAddr::addr() const
{
	return saddr;
}

/*!\brief Länge der Socket-Struktur auslesen
 *
 * \desc
 * Länge der Socket-Struktur auslesen
 *
 * @return Länge der Socket-Struktur in Byte
 */
size_t SockAddr::size() const
{
	return addrlen;
}

/*!\brief Inhalt einer anderen Variablen zuweisen
 *
 * \desc
 * Inhalt einer anderen Variablen zuweisen
 *
 * @param other
 * @return
 */
SockAddr &SockAddr::operator=(const SockAddr &other)
{
	free(saddr);
	addrlen=0;
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

/*!\brief Wandelt die IP-Adresse in der Struktur in einen Lesbaren String um
 *
 * \desc
 * Wandelt die IP-Adresse in der Struktur in einen Lesbaren String um
 *
 * @return String mit der IPv4- oder IPv6-Adresse
 */
String SockAddr::toString() const
{
	if (!saddr) throw InvalidIpAddressException("No IP-Address stored");
	char buffer[128];
	const char *res=NULL;
	if (((struct sockaddr_in*)saddr)->sin_family==AF_INET) {
		res=inet_ntop(((struct sockaddr_in*)saddr)->sin_family,
			&((struct sockaddr_in*)saddr)->sin_addr,
			buffer,
			sizeof(struct sockaddr_in)
			);
	} else if (((struct sockaddr_in*)saddr)->sin_family==AF_INET6) {
		res=inet_ntop(((struct sockaddr_in6*)saddr)->sin6_family,
			&((struct sockaddr_in6*)saddr)->sin6_addr,
			buffer,
			sizeof(struct sockaddr_in6)
			);
	} else {
		throw InvalidIpAddressException();
	}
	if (!res) throw InvalidIpAddressException();
	return String(res);
}

/*!\brief String mit einer IPv4- oder IPv6-Adresse in eine Socket-Struktur umwandeln
 *
 * \desc
 * Wandelt einen String mit einer IPv4- oder IPv6-Adresse in eine Socket-Struktur um.
 *
 * @param ip String mit der IP-Adresse
 * @return Gibt ein neues SockAddr-Objekt zurück
 */
SockAddr SockAddr::fromString(const String &ip)
{
	struct sockaddr_storage sa;
	memset(&sa,0,sizeof(sa));

	if (ip.instr(":") >= 0) {
		sa.ss_family = AF_INET6;
		//sa.ss_len = sizeof(struct sockaddr_in6);
		struct sockaddr_in6 *addr = (struct sockaddr_in6 *) &sa;
		if (inet_pton(AF_INET6, (const char*) ip, &addr->sin6_addr) <= 0)
			throw InvalidIpAddressException(ip);
		return SockAddr(addr, sizeof(struct sockaddr_in6));
	}
	sa.ss_family = AF_INET;
	//sa.ss_len = sizeof(struct sockaddr_in);
	struct sockaddr_in *addr = (struct sockaddr_in *) &sa;
	if (inet_pton(AF_INET, (const char*) ip, &addr->sin_addr) <= 0)
		throw InvalidIpAddressException(ip);
	return SockAddr(addr, sizeof(struct sockaddr_in));
}

/*!\brief Wandelt die IP-Adresse in der Struktur in einen Lesbaren String um
 *
 * \desc
 * Wandelt die IP-Adresse in der Struktur in einen Lesbaren String um
 *
 * @return String mit der IPv4- oder IPv6-Adresse
 */
SockAddr::operator String() const
{
	return toString();
}


/*!\brief Kopieren aus einer sockaddr-Structure
 *
 * \desc
 * Kopiert die Adresse aus einer sockaddr-Struktur
 *
 * @param addr Muss ein Pointer auf eine struct sockaddr, sockaddr_in oder sockaddr_in6 sein
 * @param addrlen Länge der Struktur
 */
void SockAddr::setAddr(const void *addr, size_t addrlen)
{
	free(saddr);
	saddr=NULL;
	this->addrlen=0;
	if (!addr) throw ppl7::IllegalArgumentException();
	saddr=malloc(addrlen);
	if (!saddr) throw OutOfMemoryException();
	memcpy(saddr,addr,addrlen);
	this->addrlen=addrlen;
}

/*!\brief Setzt den Inhalt der Socket.Struktur anhand eines Strings mkit einer IP-Adresse
 *
 * \desc
 * Wandelt einen String mit einer IPv4- oder IPv6-Adresse in eine Socket-Struktur um.
 *
 * @param ip String mit der IP-Adresse
 * @return Gibt ein neues SockAddr-Objekt zurück
 */
void SockAddr::setAddr(const String &ip)
{
	SockAddr other=SockAddr::fromString(ip);
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


/*!\brief Port setzen
 *
 * \desc
 * Setzt die Portnummer in der Socket-Struktur
 *
 * @param port Portnummer
 */
void SockAddr::setPort(int port)
{
	if (!saddr) throw InvalidIpAddressException("No IP-Address stored");
	if (((struct sockaddr_in*)saddr)->sin_family==AF_INET) {
		((struct sockaddr_in*)saddr)->sin_port=htons(port);
	} else if (((struct sockaddr_in*)saddr)->sin_family==AF_INET6) {
		((struct sockaddr_in6*)saddr)->sin6_port=htons(port);
	}
}


/*!\brief Port auslesen
 *
 * \desc
 * Gibt die in der Socket-Struktur gesetzte Portnummer zurück.
 * @return Portnummer
 */
int SockAddr::port() const
{
	if (!saddr) throw InvalidIpAddressException("No IP-Address stored");
	if (((struct sockaddr_in*)saddr)->sin_family==AF_INET) {
		return ntohs(((struct sockaddr_in*)saddr)->sin_port);
	} else if (((struct sockaddr_in*)saddr)->sin_family==AF_INET6) {
		return ntohs(((struct sockaddr_in6*)saddr)->sin6_port);
	}
	throw InvalidIpAddressException("No valid IP-Address");
}


}	// EOF namespace ppl7
