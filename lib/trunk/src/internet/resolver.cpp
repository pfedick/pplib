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

#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif


#include <list>
#include <set>

#include "ppl7.h"
#include "ppl7-inet.h"
//#include "socket.h"

namespace ppl7 {


IPAddress::IPAddress()
{
	ai_addr=NULL;
	ai_addrlen=0;
	ai_family=0;
	ai_socktype=0;
	ai_protocol=0;
}

IPAddress::IPAddress(const IPAddress &other)
{
	name=other.name;
	ip=other.ip;
	ai_addr=NULL;
	ai_addrlen=other.ai_addrlen;
	ai_family=other.ai_family;
	ai_socktype=other.ai_socktype;
	ai_protocol=other.ai_protocol;
	ai_canonname=other.ai_canonname;
	copyAddr(other.ai_addr, other.ai_addrlen);
}

IPAddress::~IPAddress()
{
	free(ai_addr);
}

void IPAddress::copyAddr(void *ai_addr, size_t ai_addrlen)
{
	free(this->ai_addr);
	this->ai_addrlen=ai_addrlen;
	if (ai_addrlen>0) {
		this->ai_addr=malloc(ai_addrlen);
		if (!this->ai_addr) throw OutOfMemoryException();
		memcpy(this->ai_addr,ai_addr,ai_addrlen);
	} else {
		this->ai_addr=NULL;
	}
}

IPAddress &IPAddress::operator=(const IPAddress &other)
{
	name=other.name;
	ip=other.ip;
	ai_addr=NULL;
	ai_addrlen=other.ai_addrlen;
	ai_family=other.ai_family;
	ai_socktype=other.ai_socktype;
	ai_protocol=other.ai_protocol;
	ai_canonname=other.ai_canonname;
	copyAddr(other.ai_addr, other.ai_addrlen);
	return *this;
}


bool IPAddress::operator<(const IPAddress &other) const
{
	if (ip<other.ip) return true;
	return false;
}

bool IPAddress::operator<=(const IPAddress &other) const
{
	if (ip<=other.ip) return true;
	return false;
}

bool IPAddress::operator==(const IPAddress &other) const
{
	if (ip==other.ip) return true;
	return false;
}

bool IPAddress::operator!=(const IPAddress &other) const
{
	if (ip!=other.ip) return true;
	return false;
}

bool IPAddress::operator>=(const IPAddress &other) const
{
	if (ip>=other.ip) return true;
	return false;
}

bool IPAddress::operator>(const IPAddress &other) const
{
	if (ip>other.ip) return true;
	return false;
}



String GetHostname()
/*!\brief Liefert den Hostnamen des Systems zurück
 *
 * Diese Funktion liefert den Hostnamen des Systems als String zurück.
 *
 * @return Bei Erfolg befindet sich der Hostname im String, bei Misserfolg ist der
 * String leer. Es wird kein Fehlercode gesetzt.
 */
{
	String s;
#ifdef HAVE_GETHOSTNAME
	char h[256];
	if (gethostname(h,255)==0) {
		s=h;
		return s;
	}
	return s;
#else
	s=getenv("HOSTNAME");
	return s;
#endif

}

static size_t GetHostByNameInternal(const String &name, std::list<IPAddress> &result, int flags)
{
	int n;
	struct addrinfo hints, *res, *ressave;
	bzero(&hints,sizeof(struct addrinfo));
	int family=flags&3;
	switch (family) {
		case af_inet: hints.ai_family=AF_INET; break;
		case af_inet6: hints.ai_family=AF_INET6; break;
		default: hints.ai_family=AF_UNSPEC; break;
	}
	hints.ai_socktype=SOCK_STREAM;
	if ((n=getaddrinfo((const char*)name,NULL,&hints,&res))!=0) {
#ifdef EAI_NODATA
		if (n==EAI_NODATA) return 0;
#endif
		if (n==EAI_NONAME) return 0;
		throw NetworkException("getaddrinfo(%s) returned %i: %s",(const char*)name,n,gai_strerror(n));
	}
	ressave=res;
	IPAddress ip;
	char hbuf[NI_MAXHOST];
	do {
		if (getnameinfo(res->ai_addr,res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST) == 0) {
			ip.ip.set(hbuf);
			ip.name=name;
			ip.copyAddr(res->ai_addr,res->ai_addrlen);
			ip.ai_addrlen=res->ai_addrlen;
			ip.ai_family=res->ai_family;
			ip.ai_socktype=res->ai_socktype;
			ip.ai_protocol=res->ai_protocol;
			ip.ai_canonname=res->ai_canonname;
			result.push_back(ip);
		}
	} while ((res=res->ai_next)!=NULL);
	freeaddrinfo(ressave);
	return result.size();
}


/*!\brief Hostauflösung anhand des Namens
 * \ingroup PPLGroupInternet
 *
 * \header \#include <ppl7-inet.h>
 * \desc
 * Diese Funktion führt eine Namensauflösung durch. Dabei werden alle IPs zurückgegeben, die
 * auf den angegebenen Namen passen, einschließlich IPv6.
 *
 * \param name Der gesuchte Hostname oder die IP-Adresse, wobei sowohl IPv4- als auch IPv6-Adressen
 * unterstützt werden
 * \param result Liste vom Typ IPAddress, in dem die gefundenen IP-Adressen gespeichert werden. Der
 * Datentyp IPAddress hat folgenden Inhalt:
 * 	- \b ip: IP-Adresse als String
 *  - \b name: String mit dem FQDN
 *  - \b type: Bei einer IPv4 Adresse ist dieser Wert immer AF_INET, bei IPv6 AF_INET6
 *  - \b ai_addr: Pointer auf eine Datenstruktur vom Typ "struct sockaddr", wie sie z.B. in einer Socket-Funktion
 *       verwendet werden kann (z.B. connect).
 *	- \b ai_addrlen: Die Länge der ai_addr-Struktur in Bytes
 * 	- \b ai_family:
 * 	- \b ai_protocol:
 *  - \b ai_socktype:
 *  - \b ai_canonname:
 * \param flags Bitmaske mit folgender Bedeutung:
 * - af_unspec: Das Betriebssystem entscheidet. Hier ist es häufig so, dass IPv6-Adressen nur
 *   dann zurückgegeben werden, wenn das System auch über eine IPv6-Anbindung verfügt.
 * - af_inet: nur nach IPv4 Adressen suchen
 * - af_inet6: nur nach IPv6 Adressen suchen
 * - af_all: IPv4 und IPv6 Adressen suchen
 *
 * \result Im Erfolgsfall, das heisst der angegebene Name konnte aufgelöst werden, liefert die
 * Funktion die Anzahl gefundener IP-Adressen zurück. Ausserdem werden die Adressen in die
 * Liste \p result kopiert. Wurde der Name \p name nicht gefunden, liefert die Funktion 0
 * zurück. Ist ein anderer Fehler aufgetreten (z.B. Netzwerkprobleme) wird eine Exception
 * geworfern
 *
 * \exception NetworkException Wird geworfen, wenn ein Netzwerkproblem aufgetreten ist.
 *
 * \note Es ist zu beachten, dass die Liste \p result erst gelöscht und dann mit den
 * gefundenen Daten gefüllt wird. Vorher vorhandene Daten gehen also verloren, bzw. müssen
 * bei Bedarf vom Anwender vorher gesichert werden.
 *
 */
size_t GetHostByName(const String &name, std::list<IPAddress> &result,ResolverFlags flags)
{
	#ifdef _WIN32
		InitWSA();
	#endif
	result.clear();
	if (flags!=af_all) return GetHostByNameInternal(name,result,flags);
	int ret=GetHostByNameInternal(name,result,af_inet);
	std::list<IPAddress> additional;
	int ret2=GetHostByNameInternal(name,additional,af_inet6);
	// Hier könnten Duplikate entstanden sein, die wir nicht wollen (FreeBSD)
	if (ret2>0 && ret>0) {
		// Wir bauen uns erst ein Set aus den vorhandenen Adresen auf
		std::set<String> have;
		std::set<String>::iterator haveIt;
		std::list<ppl7::IPAddress>::iterator it;
		for (it=result.begin();it!=result.end();++it) have.insert(it->ip);
		// Dann gleichen wir die zusätzlichen Adressen mit den vorhandenen ab
		// und fügen nur das ins result hinzu, was noch nicht da ist
		for (it=additional.begin();it!=additional.end();++it) {
			haveIt=have.find(it->ip);
			if (haveIt==have.end()) {
				result.push_back(*it);
				ret++;
			}
		}
	} else if (ret2>0 && ret==0) {
		result=additional;
		return ret2;
	}
	return ret;
}


size_t GetHostByAddr(const String &addr, std::list<IPAddress> &result)
/*!\brief Reverse-Lookup anhand einer IP-Adresse
 * \ingroup PPLGroupInternet
 *
 * \header \#include <ppl6.h>
 * \desc
 * Diese Funktion führt eine Reverse-Abfrage einer IP-Adresse durch.
 *
 * \param addr Die gesuchte IP-Adresse, wobei sowohl IPv4- als auch IPv6-Adressen
 * unterstützt werden
 * \param Result Ein Pointer auf ein Assoziatives Array, in dem das Ergebnis gespeichert werden
 * soll. Diese Parameter ist optional. Wird er nicht angegeben, bzw. ist er NULL, prüft die
 * Funktion lediglich, ob die angegebene IP-Adresse auflösbar ist und liefert true (1) oder false (0)
 * zurück.
 *
 * \result Im Erfolgsfall, das heisst die angegebene IP konnte aufgelöst werden, liefert die
 * Funktion true (1) zurück. Wurde der Parameter \p Result angegeben, wird das Array mit dem
 * Ergebnis gefüllt. Das Ergebnis hat folgendes Format:
 * 	- \b 0/ip IP-Adresse im lesbaren Format
 *  - \b 0/name Der FQDN
 *  - \b 0/type Bei einer IPv4 Adresse ist dieser Wert immer AF_INET, bei IPv6 AF_INET6
 *  - \b 0/ai_addr Ein Binäres Objekt vom Typ CBinary, was eine Systemspezifische Struktur
 *    vom Typ "struct sockaddr" enthält, die direkt in den Socket-Funktionen des Systems
 *    verwendet werden kann (z.B. connect).
 *	- \b 0/ai_addrlen Die Länge der ai_addr-Struktur in Bytes
 * 	- \b 0/ai_family
 * 	- \b 0/ai_protocol
 *  - \b 0/ai_socktype
 *
 * Wurden mehrere Namen-Adressen gefunden, wiederholt sich der Block und die Ziffer auf der
 * obersten Ebene des Arrays wird hochgezählt.
 *
 * Im Fehlerfall, das heisst die angegebene IP konnte nicht aufgelöst werden, liefert die Funktion
 * false (0) zurück und des optionale Array \p Result bleibt unverändert.
 *
 * \note Es ist zu beachte, dass das Array \p Result im Erfolgsfall erst gelöscht und dann mit den
 * gefundenen Daten gefüllt wird. Vorher vorhandene Daten im Array gehen also verloren, bzw. müssen
 * bei Bedarf vom Anwender vorher gesichert werden.
 *
 * \since Diese Klasse wurde mit Version 6.0.12 eingeführt
 * \since Ab Version 6.0.19 werden die ai_*-Parameter zurückgegeben
 *
 */
{
	#ifdef _WIN32
		InitWSA();
	#endif
		int n;
		struct addrinfo hints, *res, *ressave;
		result.clear();
		bzero(&hints,sizeof(struct addrinfo));
		hints.ai_family=AF_UNSPEC;
		hints.ai_socktype=SOCK_STREAM;
		if ((n=getaddrinfo((const char*)addr,NULL,&hints,&res))!=0) {
			throw NetworkException("getaddrinfo(%s) returned: %s",(const char*)addr,gai_strerror(n));
		}
		ressave=res;
		IPAddress ip;
		char hbuf[NI_MAXHOST];
		do {
			if (getnameinfo(res->ai_addr,res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST) == 0) {
				ip.ip=hbuf;
				ip.name=hbuf;
				ip.copyAddr(res->ai_addr,res->ai_addrlen);
				ip.ai_addrlen=res->ai_addrlen;
				ip.ai_family=res->ai_family;
				ip.ai_socktype=res->ai_socktype;
				ip.ai_protocol=res->ai_protocol;
				ip.ai_canonname=res->ai_canonname;
				if (getnameinfo(res->ai_addr,res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD) == 0) {
					ip.name=hbuf;
				}
				result.push_back(ip);
			}
		} while ((res=res->ai_next)!=NULL);
		freeaddrinfo(ressave);
		return result.size();
}



}	// namespace ppl7

