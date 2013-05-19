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




#include "ppl6.h"
#include "socket.h"

namespace ppl6 {

CString GetHostname()
/*!\brief Liefert den Hostnamen des Systems zurück
 *
 * Diese Funktion liefert den Hostnamen des Systems als String zurück.
 *
 * @return Bei Erfolg befindet sich der Hostname im String, bei Mißerfolg ist der
 * String leer. Es wird kein Fehlercode gesetzt.
 */
{
	CString s;
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

static int GetHostByNameInternal(const char *name, CAssocArray *Result, int family)
{
		int n;
		struct addrinfo hints, *res, *ressave;
		CBinary bin;
		bzero(&hints,sizeof(struct addrinfo));
		hints.ai_family=family;
		hints.ai_socktype=SOCK_STREAM;
		if ((n=getaddrinfo(name,NULL,&hints,&res))!=0) {
			SetError(273,"%s, %s",name,gai_strerror(n));
			return 0;
		}
		ressave=res;
		if (Result) {
			Result->Clear();
			CAssocArray ip;
			//int e, conres;
			char hbuf[NI_MAXHOST];
			do {
				if (getnameinfo(res->ai_addr,res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST) == 0) {
					ip.Clear();
					bin.Clear();
					bin.Set(res->ai_addr,res->ai_addrlen);
					ip.Set("ip",hbuf);
					ip.Set("name",name);
					if (res->ai_family==AF_INET) {
						ip.Set("type","AF_INET");
					} else if (res->ai_family==AF_INET6) {
						ip.Set("type","AF_INET6");
					}
					ip.Set("ai_addr",&bin);
					ip.Setf("ai_addrlen","%i",res->ai_addrlen);
					ip.Setf("ai_family","%i",res->ai_family);
					ip.Setf("ai_protocol","%i",res->ai_protocol);
					ip.Setf("ai_socktype","%i",res->ai_socktype);
					Result->Set("[]",&ip);
				}
			} while ((res=res->ai_next)!=NULL);
		}
		freeaddrinfo(ressave);
		return 1;
	//#endif
}


int GetHostByName(const char *name, CAssocArray *Result)
/*!\brief Hostauflösung anhand des Namens
 * \ingroup PPLGroupInternet
 *
 * \header \#include <ppl6.h>
 * \desc
 * Diese Funktion führt eine Namensauflösung durch. Dabei werden alle IPs zurückgegeben, die
 * auf den angegebenen Namen passen, einschließlich IPv6.
 *
 * \param name Der gesuchte Hostname oder die IP-Adresse, wobei sowohl IPv4- als auch IPv6-Adressen
 * unterstützt werden
 * \param Result Ein Pointer auf ein Assoziatives Array, in dem das Ergebnis gespeichert werden
 * soll. Diese Parameter ist optional. Wird er nicht angegeben, bzw. ist er NULL, prüft die
 * Funktion lediglich, ob der angegebene Hostname auflösbar ist und liefert true (1) oder false (0)
 * zurück.
 *
 * \result Im Erfolgsfall, das heisst der angegebene Name konnte aufgelöst werden, liefert die
 * Funktion true (1) zurück. Wurde der Parameter \p Result angegeben, wird das Array mit dem
 * Ergebnis gefüllt. Das Ergebnis hat folgendes Format:
 * 	- \b 0/ip IP-Adresse im lesbaren Format
 *  - \b 0/name Der FQDN
 *  - \b 0/type Bei einer IPv4 Adresse ist dieser Wert immer AF_INET, bei IPv6 AF_INET6
 *  - \b 0/ai_addr Ein Binäres Objekt vom Typ CBinary, was eine Systemspezifische Struktur vom Typ "struct sockaddr" enthält, die direkt in den Socket-Funktionen des Systems verwendet werden kann (z.B. connect).
 *	- \b 0/ai_addrlen Die Länge der ai_addr-Struktur in Bytes
 * 	- \b 0/ai_family
 * 	- \b 0/ai_protocol
 *  - \b 0/ai_socktype
 *
 * Wurden mehrere IP-Adressen gefunden, wiederholt sich der Block und die Ziffer auf der
 * obersten Ebene des Arrays wird hochgezählt.
 *
 * Im Fehlerfall, das heisst der angegebene Name konnte nicht aufgelöst werden, liefert die Funktion
 * false (0) zurück und des optionale Array \p Result bleibt unverändert.
 *
 * \note Es ist zu beachte, dass das Array \p Result im Erfolgsfall erst gelöscht und dann mit den
 * gefundenen Daten gefüllt wird. Vorher vorhandene Daten im Array gehen also verloren, bzw. müssen
 * bei Bedarf vom Anwender vorher gesichert werden.
 *
 * \since Diese Klasse wurde mit Version 6.0.12 eingeführt
 * \since Ab Version 6.0.19 werden die ai_*-Parameter zurückgegeben
 *
 * \note
 * Ursprünglich hat die Funktion intern einen einzelnen Aufruf der Systemfunktion getaddrinfo gemacht.
 * In neueren libc Bibliotheken scheint sich jedoch das Verhalten geändert zu haben.
 * Hier werden AAAA-Records nur dann zurückgegeben, wenn auf dem lokalen host auch eine globales
 * IPv6-Interface konfiguriert ist. Zu beobachten auf Ubuntu 9.10 nach Einspielen der Updates am 13.01.2009.
 * Die Funktion wurde daher geändert und ruft getaddrinfo nun zweimal auf, einmal für IPv4/INET und
 * einmal für IPv6/INET6.
 *
 */
{
	if (!name) {
		SetError(194,"GetHostByName(==> const char *name <==, CAssocArray *Result)");
		return 0;
	}

	#ifdef _WIN32
		InitWSA();
	#endif
	int ret=GetHostByNameInternal(name,Result,AF_INET);
	ret+=GetHostByNameInternal(name,Result,AF_INET6);
	return ret;
}


int GetHostByAddr(const char *addr, CAssocArray *Result)
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
	if (!addr) {
		SetError(194,"GetHostByAddr(==> const char *addr <==, CAssocArray *Result)");
		return 0;
	}
	#ifdef _WIN32
		InitWSA();
	#endif
		int n;
		struct addrinfo hints, *res, *ressave;
		CBinary bin;
		bzero(&hints,sizeof(struct addrinfo));
		hints.ai_family=AF_UNSPEC;
		hints.ai_socktype=SOCK_STREAM;
		if ((n=getaddrinfo(addr,NULL,&hints,&res))!=0) {
			SetError(273,"%s, %s",addr,gai_strerror(n));
			return 0;
		}
		ressave=res;
		if (Result) {
			Result->Clear();
			CAssocArray ip;
			//int e, conres;
			char hbuf[NI_MAXHOST];
			do {
				if (getnameinfo(res->ai_addr,res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NUMERICHOST) == 0) {
					ip.Clear();
					bin.Clear();
					bin.Set(res->ai_addr,res->ai_addrlen);
					ip.Set("ip",hbuf);
					if (getnameinfo(res->ai_addr,res->ai_addrlen, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD) == 0) {
						ip.Set("name",hbuf);
					} else {
						ip.Set("name",ip.Get("ip"));
					}
					if (res->ai_family==AF_INET) {
						ip.Set("type","AF_INET");
					} else if (res->ai_family==AF_INET6) {
						ip.Set("type","AF_INET6");
					}
					ip.Set("ai_addr",&bin);
					ip.Setf("ai_addrlen","%i",res->ai_addrlen);
					ip.Setf("ai_family","%i",res->ai_family);
					ip.Setf("ai_protocol","%i",res->ai_protocol);
					ip.Setf("ai_socktype","%i",res->ai_socktype);
					Result->Set("[]",&ip);
				}
			} while ((res=res->ai_next)!=NULL);
		}
		freeaddrinfo(ressave);
		return 1;
	//#endif

}



}	// namespace ppl6

