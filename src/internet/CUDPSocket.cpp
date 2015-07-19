/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CUDPSocket.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
	#include <ws2tcpip.h>
	//#include <Ws2tcpip.h>
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


/*!\class CUDPSocket
 * \ingroup PPLGroupInternet
 * \brief UDP-Socket-Klasse
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Klasse können Pakete per UDP verschickt und empfangen werden.
 *
 * \since Diese Klasse wurde mit Version 6.0.19 eingeführt
 */

CUDPSocket::CUDPSocket()
/*! \brief Konstruktor der Klasse
 *
 * \header \#include <ppl6.h>
 * \desc
 * Initialisiert interne Daten der Klasse
 *
 * \since Wurde mit Version 6.0.19 eingeführt
 */
{
#ifdef _WIN32
	InitWSA();
#endif
	timeout_sec=0;
	timeout_usec=0;
	socket=NULL;
}

CUDPSocket::~CUDPSocket()
/*! \brief Destruktor der Klasse
 *
 * \header \#include <ppl6.h>
 * \desc
 * De-Initialisiert interne Daten der Klasse
 *
 * \since Wurde mit Version 6.0.19 eingeführt
 */
{
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) {
		return;
	}
    if (s->sd>-1) ppl_closesocket(s->sd);
	free(s);
}

void CUDPSocket::SetTimeout(int seconds, int useconds)
/*! \brief Timeout setzen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird der Timeout für das Empfangen von Daten gesetzt.
 *
 * \param seconds Anzahl Sekunden
 * \param useconds Anzahl Millisekunden
 * \note Diese Funktion hat zur Zeit noch keine Auswirkungen
 * \since Wurde mit Version 6.0.19 eingeführt
 */
{
	timeout_sec=seconds;
	timeout_usec=useconds;
}

int CUDPSocket::SendTo(const char *host, int port, const CString &buffer)
/*! \brief UDP-Packet verschicken
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird ein UDP-Paket an den angegebenen Host verschickt.
 *
 * \param host Der Name oder die IP-Adresse des Zielrechners
 * \param port Der Port des Zielrechners
 * \param buffer Ein Pointer auf eine String-Klasse, die die zu sendenden Daten enthält
 * \returns Im Erfolgsfall liefert die Funktion die Anzahl gesendeter Bytes zurück, im Fehlerfall -1.
 * Der Fehlercode kann über die üblichen Fehler-Funktionen ausgelesen werden
 *
 * \since Wurde mit Version 6.0.19 eingeführt
 */
 {
	return SendTo(host,port,(const void *)buffer.GetPtr(),buffer.Len());
}


int CUDPSocket::SendTo(const char *host, int port, const void *buffer, int bytes)
/*! \brief UDP-Packet verschicken
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird ein UDP-Paket an den angegebenen Host verschickt.
 *
 * \param host Der Name oder die IP-Adresse des Zielrechners
 * \param port Der Port des Zielrechners
 * \param buffer Ein Pointer auf den Puffer, der die zu sendenden Daten enthält
 * \param bytes Die Anzahl Bytes im Puffer, die gesendet werden sollen
 * \returns Im Erfolgsfall liefert die Funktion die Anzahl gesendeter Bytes zurück, im Fehlerfall -1.
 * Der Fehlercode kann über die üblichen Fehler-Funktionen ausgelesen werden
 *
 * \since Wurde mit Version 6.0.19 eingeführt
 */
{
	if (!host) {
		ppl6::SetError(194,"int CUDPSocket::SendTo(==> char *host <== , int port, void *buffer, int bytes)");
		return -1;
	}
	if (!port) {
		ppl6::SetError(194,"int CUDPSocket::SendTo(char *host, ==> int port <== , void *buffer, int bytes)");
		return -1;
	}
	if (!buffer) {
		ppl6::SetError(194,"int CUDPSocket::SendTo(char *host, int port, ==> void *buffer <== , int bytes)");
		return -1;
	}
	if (bytes<0) {
		ppl6::SetError(194,"int CUDPSocket::SendTo(char *host, int port, void *buffer, ==> int bytes <== )");
		return -1;
	}
	ppl6::CAssocArray res, *a;
	ppl6::CBinary *bin;
	if (!ppl6::GetHostByName(host,&res)) return -1;
	a=res.GetFirstArray();
	//a->List();


	int domain, type, protocol;
	domain=ppl6::atoi(a->Get("ai_family"));
	type=ppl6::atoi(a->Get("ai_socktype"));
	//protocol=ppl6::atoi(a->Get("ai_protocol"));
	struct protoent *proto=getprotobyname("UDP");
	if (!proto) {
		ppl6::SetError(395);
		return -1;
	}
	protocol=proto->p_proto;
	bin=a->GetBinary("ai_addr");
	//a->List();
	const struct sockaddr *to=(const struct sockaddr *)bin->GetPtr();
	((sockaddr_in*)to)->sin_port=htons(port);
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) {
		socket=malloc(sizeof(PPLSOCKET));
		s=(PPLSOCKET*)socket;
		s->sd=-1;
	} else if (s->sd>-1) {
		ppl_closesocket(s->sd);
	}
	s->sd=::socket(domain,SOCK_DGRAM,protocol);
	//sockfd=::socket(AF_INET,SOCK_DGRAM,0);
	if (s->sd<0) {
		SetSocketError();
		return -1;
	}
#ifdef _WIN32
	int ret=::sendto(s->sd,(const char*)buffer,bytes,0,to,bin->GetSize());
#else
	ssize_t ret=::sendto(s->sd,(const void*)buffer,(size_t)bytes,0,to,(socklen_t) bin->GetSize());
#endif
	if (ret<0) {
		printf ("ret: %i\n",(int)ret);
		SetSocketError();
		return -1;
	}
	//close(sockfd);
	return ret;
}

int CUDPSocket::RecvFrom(CString &buffer, int maxlen)
/*! \brief UDP-Packet empfangen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Diese Funktion wartet auf ein UDP-Packet
 *
 * \param buffer Ein Pointer auf eine String-Klasse, in die die Daten geschrieben werden sollen
 * \param maxlen Die maximale Anzahl Bytes, die in den Puffer geschrieben werden können
 * \returns Im Erfolgsfall liefert die Funktion die Anzahl gelesener Bytes zurück, im Fehlerfall -1.
 * Der Fehlercode kann über die üblichen Fehler-Funktionen ausgelesen werden
 *
 * \since Wurde mit Version 6.0.19 eingeführt
 */
{
	char *b=(char*)malloc(maxlen+1);
	if (!b) {
		SetError(2);
		return 0;
	}
	int ret=RecvFrom((void*)b,maxlen);
	buffer.ImportBuffer(b,maxlen);
	return ret;
}

int CUDPSocket::SetReadTimeout(int seconds, int useconds)
{
	struct timeval tv;
	tv.tv_sec=seconds;
	tv.tv_usec=useconds;
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!s) {
        SetError(275);
        return 0;
    }
#ifdef WIN32
	if (setsockopt(s->sd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof(tv))!=0) {
#else
	if (setsockopt(s->sd,SOL_SOCKET,SO_RCVTIMEO,(void*)&tv,sizeof(tv))!=0) {
#endif
		SetError(350);
		return 0;
	}
	return 1;
}


int CUDPSocket::RecvFrom(void *buffer, int maxlen)
/*! \brief UDP-Packet empfangen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Diese Funktion wartet auf ein UDP-Packet
 *
 * \param buffer Ein Pointer auf den Puffer, in den die Daten geschrieben werden sollen
 * \param maxlen Die maximale Anzahl Bytes, die in den Puffer geschrieben werden können
 * \returns Im Erfolgsfall liefert die Funktion die Anzahl gelesener Bytes zurück, im Fehlerfall -1.
 * Der Fehlercode kann über die üblichen Fehler-Funktionen ausgelesen werden
 *
 * \since Wurde mit Version 6.0.19 eingeführt
 */
{
	struct sockaddr from;
#ifdef _WIN32
	int len;
#else
	socklen_t len=0;
#endif
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) {
		socket=malloc(sizeof(PPLSOCKET));
		s=(PPLSOCKET*)socket;
		s->sd=-1;
	}
	if (s->sd<0) s->sd=::socket(AF_INET,SOCK_DGRAM,0);

#ifdef _WIN32
	len=sizeof(struct sockaddr);
	int ret=::recvfrom(s->sd,(char*)buffer,maxlen,0,&from,&len);
	//int ret=::recvfrom(s->sd,(char*)buffer,maxlen,0,NULL,0);
#else
	int ret=::recvfrom(s->sd,buffer,maxlen,0,&from,&len);
#endif
	if (ret<0) {
		SetSocketError();
		return -1;
	}

	return ret;
}

int CUDPSocket::GetDescriptor()
{
	if (!socket) return 0;
	PPLSOCKET *s=(PPLSOCKET*)socket;
#ifdef _WIN32
	return (int)s->sd;
#else
	return s->sd;
#endif
}


int CUDPSocket::RecvFrom(void *buffer, int maxlen, CString &host, int *port)
/*! \brief UDP-Packet empfangen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Diese Funktion wartet auf ein UDP-Packet
 *
 * \param buffer Ein Pointer auf den Puffer, in den die Daten geschrieben werden sollen
 * \param maxlen Die maximale Anzahl Bytes, die in den Puffer geschrieben werden können
 * \param host Pointer auf einen String, in dem die Absender-IP des Pakets gespeichert wird
 * \param port Port des Absenders
 * \returns Im Erfolgsfall liefert die Funktion die Anzahl gelesener Bytes zurück, im Fehlerfall -1.
 * Der Fehlercode kann über die üblichen Fehler-Funktionen ausgelesen werden
 *
 * \since Wurde mit Version 6.2.3 eingeführt
 */
{
	struct sockaddr_in from;
#ifdef _WIN32
	int len;
#else
	socklen_t len=0;
#endif
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) {
		socket=malloc(sizeof(PPLSOCKET));
		s=(PPLSOCKET*)socket;
		s->sd=-1;
	}
	if (s->sd<0) s->sd=::socket(AF_INET,SOCK_DGRAM,0);

#ifdef _WIN32
	len=sizeof(struct sockaddr);
	int ret=::recvfrom(s->sd,(char*)buffer,maxlen,0,(sockaddr*)&from,&len);
	//int ret=::recvfrom(s->sd,(char*)buffer,maxlen,0,NULL,0);
#else
	int ret=::recvfrom(s->sd,buffer,maxlen,0,(struct sockaddr *)&from,&len);
#endif
	if (ret<0) {
		SetSocketError();
		return -1;
	}
	char hostname[1024];
	char servname[32];
	bzero(hostname,1024);
	bzero(servname,32);
	ppl6::HexDump(&from,sizeof(struct sockaddr));
	if (getnameinfo((const sockaddr*)&from,len,
		hostname,1023, servname,31,NI_NUMERICHOST|NI_NUMERICSERV)!=0) {
		host.Set("");
		*port=0;
	} else {
		host.Set(hostname);
		*port=atoi(servname);
	}

	return ret;
}


int CUDPSocket::Bind(const char *host, int port)
{
	int addrlen=0;
	if (!socket) {
		socket=malloc(sizeof(PPLSOCKET));
		if (!socket) {
			SetError(2);
			return 0;
		}
		PPLSOCKET *s=(PPLSOCKET*)socket;
    	s->sd=0;
		s->proto=6;
		s->ipname=NULL;
		s->port=port;
		s->addrlen=0;
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
	//if (s->sd) Disconnect();
	if (s->ipname) free(s->ipname);
	s->ipname=NULL;

	struct addrinfo hints, *res, *ressave;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags=AI_PASSIVE;
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	int n;
	int on=1;
	#ifdef _WIN32
		SOCKET	listenfd;
	#else
		int listenfd;
	#endif
	// Prüfen, ob host ein Wildcard ist
	if (host!=NULL && strlen(host)==0) host=NULL;
	if (host!=NULL && host[0]=='*') host=NULL;

	if (host) {
		char portstr[10];
		sprintf(portstr,"%i",port);
		if ((n=getaddrinfo(host,portstr,&hints,&res))!=0) {
			SetError(273,"%s, %s",host,gai_strerror(n));
			return 0;
		}
		ressave=res;
		do {
			listenfd=::socket(res->ai_family,res->ai_socktype,res->ai_protocol);
			if (listenfd<0) continue;		// Error, try next one
			if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on))!=0) {
				freeaddrinfo(ressave);
				SetError(334);
				return 0;
			}
			if (::bind(listenfd,res->ai_addr,res->ai_addrlen)==0) {
				addrlen=res->ai_addrlen;
				break;
			}
			shutdown(listenfd,2);
			#ifdef _WIN32
				closesocket(listenfd);
			#else
				close(listenfd);
			#endif
			listenfd=0;

		} while ((res=res->ai_next)!=NULL);
		freeaddrinfo(ressave);
	} else {
		listenfd=::socket(AF_INET, SOCK_DGRAM, 0);
		if (listenfd>=0) {
			struct sockaddr_in addr;
			bzero(&addr,sizeof(addr));
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(port);
			addr.sin_family = AF_INET;
		    /* bind server port */
    		if(::bind(listenfd, (struct sockaddr *) &addr, sizeof(addr))!=0) {
				shutdown(listenfd,2);
				#ifdef _WIN32
					closesocket(listenfd);
				#else
					close(listenfd);
				#endif
				SetError(266);
				return 0;
    		}
    		s->sd=listenfd;
    		s->addrlen=0;
    		//connected=1;
    		return 1;
		}
	}
	if (listenfd<0) {
		SetError(265,"Host: %s, Port: %d",host,port);
		return 0;
	}
	if (res==NULL) {
		SetError(266,"Host: %s, Port: %d",host,port);
		return 0;
	}
	s->sd=listenfd;
	s->addrlen=addrlen;
	//connected=1;
	return 1;
}


} // EOF namespace ppl6
