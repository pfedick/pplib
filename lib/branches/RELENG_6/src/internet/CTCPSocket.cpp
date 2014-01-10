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

//#define DEBUGOUT

#include "ppl6.h"
#include "socket.h"

namespace ppl6 {

/*!\class CTCPSocket
 * \ingroup PPLGroupInternet
 *
 * \brief Socket-Klasse für TCP-Verbindungen
 *
 * \header \#include <ppl6.h>
 *
 * \desc
 * Mit dieser Klasse kann eine TCP-Verbindung (Transmission Control Protocol) zu einem über IP
 * erreichbaren Server aufgebaut oder ein eigener Server gestartet werden. Über eine
 * bestehende Verbindung können Daten gesenden und empfangen werden. Mit Hilfe der Klasse CSSL
 * und der OpenSSL-Bibliothek können die Daten auch verschlüsselt übertragen werden.
 *
 * \example TCP-Client
 * Das nachfolgende Beispiel zeigt einen einfachen Client, der eine Verbindung
 * zu einem Server öffnet, ein Datenpaket sendet und auf ein Datenpaket
 * wartet.
 * \dontinclude socket_examples.cpp
 * \skip Socket_Example1
 * \until EOF
 *
 * \example TCP-Server
 * Das zweite Beispiel zeigt, wie ein einfacher TCP-Server programmiert
 * wird. Zunächst wird ein Server gestartet, der auf eingehende
 * Verbindungen wartet. Nach Verbindungseingang wird auf Datenpakete
 * gewartet, die sofort wieder an den Client zurückgeschickt werden. Durch Abbruch
 * der Verbindung wird die Schleife wieder beendet.
 * \dontinclude socket_examples.cpp
 * \skip Socket_Example2
 * \until EOF
 * \par
 * Ein weiteres Beispiel mit einer verschlüsselten Datenübertragung ist in der Dokumentation
 * der Klasse CSSL zu finden.
 *
 */

// Compat-Funktionen
#ifdef _WIN32
#define socklen_t	int
#endif


/*!\brief Konstruktor der Klasse
 *
 *
 */
CTCPSocket::CTCPSocket()
{
#ifdef _WIN32
	InitWSA();
#endif
	socket=NULL;
    connected=0;
	islisten=0;
	stoplisten=false;
	ssl=NULL;
	sslclass=NULL;
	sslreference=NULL;
	thread=NULL;
	log=NULL;
	BytesWritten=0;
	BytesRead=0;
	connect_timeout_sec=0;
	connect_timeout_usec=0;
	SourcePort=0;

#ifndef _WIN32
	// signal SIGPIPE ignorieren
	signal(SIGPIPE,SIG_IGN);
#endif
}

/*!\brief Destruktor der Klasse
 *
 *
 */
CTCPSocket::~CTCPSocket()
{
	PushError();
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::~CTCPSocket()");
	SSL_Shutdown();
    if (connected) Disconnect();
    if (islisten) Disconnect();
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) {
		PopError();
		return;
	}
    if (s->sd) Disconnect();
	if (s->ipname) free(s->ipname);
	free(s);
	socket=NULL;
	PopError();
}

/*!\brief Überwachungsthread definieren
 *
 * \desc
 * Mit dieser Funktion kann der Klasse ein Überwachungsthread mitgeteilt werden. Verschiedene Funktionen
 * der Socket-Klasse, die üblicherweise in einer Endlosschleife laufen oder erst wiederkehren, wenn
 * bestimmte Voraussetzungen erfüllt sind, prüfen diese Threadklasse, ob Ihr ein Stop signalisiert wurde.
 * Ist dies der Fall, beenden sich diese Funktionen ebenfalls.
 * \remarks
 * Eventuell wird diese Funktionalität nochmal überarbeitet.
 *
 * @param[in] thread Pointer auf die zu überwachende Thread-Klasse
 * @return Liefert immer 1 zurück.
 */
int CTCPSocket::WatchThread(CThread *thread)
{
	this->thread=thread;
	return 1;
}

/*!\brief Logfile definieren
 *
 * \desc
 * Einige Funktionen können zusätzliche Debug-Informationen in ein Logfile schreiben. Durch Aufruf
 * dieser Funktion wird dieses Feature aktiviert oder deaktiviert.
 *
 * @param[in] log Pointer auf eine CLog-Klasse oder NULL, um das Logging zu deaktivieren
 *
 */
void CTCPSocket::SetLogfile(CLog *log)
{
	this->log=log;
}

/*!\brief Fehlercode des Betriebssystems in PPL-Fehler übersetzen
 *
 * \desc
 * Diese Funktion wird intern immer dann aufgerufen, wenn eine Socket-Funktion des Betriebssystems
 * einen Fehler signalisiert. Sie übersetzt die Betriebssystem-Anghängigen Fehlercodes in einheitliche
 * PPL-Fehlercodes.
 * \par
 * Unter Unix wird dazu die globale Fehlervariable "errno" ausgewertet, unter Windows wird
 * der Fehler über die Socketfunktion WSAGetLastError ausgelesen.
 */
void CTCPSocket::DispatchErrno()
{
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::DispatchErrno()");
	int e=TranslateErrno(errno);
	if (e==3) {
		SetError(312,errno,strerror(errno));
	} else {
		SetError(e,errno,strerror(errno));
	}
#ifdef OLDDISPATCH
	#ifdef WIN32
	int e=WSAGetLastError();
	switch (e) {
		case WSANOTINITIALISED: SetError(312,"WSANOTINITIALISED");
			break;
		case WSAENETDOWN: SetError(312,"WSAENETDOWN");
			break;
		case WSAEACCES: SetError(312,"WSAEACCES");
			break;
		case WSAEINTR: SetError(312,"WSAEINTR");
			break;
		case WSAEINPROGRESS: SetError(312,"WSAEINPROGRESS");
			break;
		case WSAEFAULT: SetError(312,"WSAEFAULT");
			break;
		case WSAENETRESET: SetError(312,"WSAENETRESET");
			break;
		case WSAENOBUFS: SetError(312,"WSAENOBUFS");
			break;
		case WSAENOTCONN: SetError(312,"WSAENOTCONN");
			break;
		case WSAENOTSOCK: SetError(312,"WSAENOTSOCK");
			break;
		case WSAEOPNOTSUPP: SetError(312,"WSAEOPNOTSUPP");
			break;
		case WSAESHUTDOWN: SetError(308,"The socket has been shut down");
			break;
		case WSAEWOULDBLOCK: SetError(309,errno,"Mit  O_NONBLOCK wurde nicht-blockierende Ein-/Ausgabe gewaehlt, aber in der mit fd verbundenen pipe oder dem Socket war kein Platz mehr, um die Daten sofort zu schreiben/lesen.");
			break;
		case WSAEMSGSIZE: SetError(312,"WSAEMSGSIZE");
			break;
		case WSAEHOSTUNREACH: SetError(312,"WSAEHOSTUNREACH");
			break;
		case WSAEINVAL: SetError(312,"WSAEINVAL");
			break;
		case WSAECONNABORTED: SetError(312,"WSAECONNRESET");
			break;
		case WSAECONNRESET: SetError(312,"WSAECONNRESET");
			break;
		case WSAETIMEDOUT: SetError(312,"WSAETIMEDOUT");
			break;
		default:
			SetError(312,e);
	}
	#else
	switch (errno) {
		case EBADF:	SetError(72,errno,"fd ist keine gueltige Dateikennzahl oder die Datei wurde nicht zum Schreiben geoeffnet.");
			break;
		case EINVAL: SetError(306,errno,"fd gehoert zu einer Datei, welche nicht zum Schreiben geeignet ist.");
			break;
		case EFAULT: SetError(307,errno,"buf ist ausserhalb Deines ansprechbaren Adressraumes.");
			break;
		case EPIPE: SetError(308,errno,"fd  ist  mit einer pipe oder einem socket verbunden, deren lesendes Ende geschlossen ist. In diesem Falle empfaengt der schreibende Prozess ein SIGPIPE Signal. Wird  dieses  abgefangen, blockiert oder ignoriert, wird EPIPE zurueckgegeben.");
			break;
		case EAGAIN: SetError(309,errno,"Mit  O_NONBLOCK wurde nicht-blockierende Ein-/Ausgabe gewaehlt, aber in der mit fd verbundenen pipe oder dem Socket war kein Platz mehr, um die Daten sofort zu schreiben/lesen.");
			break;
		case EINTR: SetError(310,errno,"Der Aufruf wurde durch ein Signal unterbrochen, bevor Daten geschrieben werden konnten.");
			break;
		case ENOSPC: SetError(311,errno,"Das Geraet, welches die Datei enthaelt, welche durch fd referenziert ist, hat keinen  Platz fuer die Daten.");
			break;
		case EISDIR: SetError(313,errno,"fd referenziert ein Verzeichnis.");
			break;
		//case EWOULDBLOCK: SetError(174,errno,"Timeout bei Socket-Operation");
			break;
		default:
			SetError(312,errno,strerror(errno));
	};
	#endif
#endif
	if (log) {
		PushError();
		log->LogError(4);
		PopError();
	}
}

/*!\brief Descriptor des Sockets auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Betriebssystem-spezifische Socket-Descriptor ausgelesen werden.
 * Unter Unix ist dies ein File-Descriptor vom Typ Integer, unter Windows ein Socket-Descriptor
 * vom Typ SOCKET.
 *
 * @return Betriebsystem-spezifischer Descriptor.
 */
int CTCPSocket::GetDescriptor()
{
	if (!connected) return 0;
	PPLSOCKET *s=(PPLSOCKET*)socket;
#ifdef _WIN32
	return (int)s->sd;
#else
	return s->sd;
#endif
}

/*!\brief Timeout für Connect-Funktion definieren
 *
 * \desc
 * Mit dieser Funktion kann ein Timeout für die Connect-Funktionen definiert werden. Ist es nicht
 * möglich innerhalb der vorgegebenen Zeit eine Verbindung mit dem Ziel herzustellen, brechen die
 * Connect-Funktionen mit einer Timeout-Fehlermeldung ab.
 * \par
 * Um den Timeout wieder abzustellen, kann die Funktion mit 0 als Wert für seconds und useconds aufgerufen werden.
 *
 * @param[in] seconds Anzahl Sekunden
 * @param[in] useconds Anzahl Mikrosekunden (1000 Mikrosekunden=1 Millisekunde, 1000 Millisekunden = 1 Sekunde)
 */
void CTCPSocket::SetConnectTimeout(int seconds, int useconds)
{
	connect_timeout_sec=seconds;
	connect_timeout_usec=useconds;
}

/*!\brief Quell-Interface und Port festlegen
 *
 * \desc
 * Diese Funktion kann aufgerufen werden, wenn der Rechner über mehrere Netzwerkinterfaces verfügt.
 * Normalerweise entscheidet das Betriebssytem, welches Interface für eine ausgehende Verbindung
 * verwendet werden soll, aber manchmal kann es sinnvoll sein, dies manuell zu machen.
 *
 * @param[in] host Hostname oder IP-Adresse des Quellinterfaces. Wird NULL angegeben, wird nur der
 * \p port beachtet
 * @param[in] port Port-Nummer des Quellinterfaces. Wird 0 angegeben, wird nur der \p host
 * beachtet
 *
 * \attention
 * Diese Funktionalität wird derzeit nicht unter Windows unterstützt! Falls trotzdem ein
 * \p host oder \p port definiert wurden, wird die Connect-Funktion fehlschlagen!
 *
 * \remarks
 * Sind beide Parameter NULL bzw. 0, wird das Quellinterface und Port vom Betriebssystem
 * festgelegt.
 *
 */
void CTCPSocket::SetSource(const char *host, int port)
{
	SourceHost=host;
	SourcePort=port;
}

const CString& CTCPSocket::getHostname() const
{
	return HostName;
}

int CTCPSocket::getPort() const
{
	return PortNum;
}


#ifdef WIN32
static int out_bind(SOCKET sockfd, const char *host, int port)
{
	SetError(219,"Connect mit vorherigem Bind");
	return 0;
}

#else
/*!\brief Socket auf ausgehendes Interface legen
 *
 * \desc
 * Diese Funktion wird intern durch die Connect-Funktionen aufgerufen, um einen ausgehenden Socket auf
 * ein bestimmtes Netzwerk-Interface zu binden. Die Funktion wird nur dann verwendet, wenn
 * mit CTCPSocket::SetSource ein Quellinterface definiert wurde.
 *
 * @param[in] host Hostname oder IP des lokalen Interfaces
 * @param[in] port Port auf dem lokalen Interface
 * @return Bei Erfolg liefert die Funktion die File-Nummer des Sockets zurück,
 * im Fehlerfall 0.
 *
 * \relates ppl6::CTCPSocket
 */
static int out_bind(const char *host, int port)
{
	struct addrinfo hints, *res, *ressave;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags=AI_PASSIVE;
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	int listenfd;
	int n;
	int on=1;
	// Prüfen, ob host ein Wildcard ist
	if (host!=NULL && strlen(host)==0) host=NULL;
	if (host!=NULL && host[0]=='*') host=NULL;

	if (host!=NULL && port>0) {
		char portstr[10];
		sprintf(portstr,"%i",port);
		n=getaddrinfo(host,portstr,&hints,&res);
	} else if (host) {
		n=getaddrinfo(host,NULL,&hints,&res);
	} else if (port) {
		char portstr[10];
		sprintf(portstr,"%i",port);
		n=getaddrinfo(NULL,portstr,&hints,&res);
	} else {
		SetError(194);
		return 0;
	}
	if (n!=0) {
		SetError(273,"Sourceport: %s:%i, %s",host,port,gai_strerror(n));
		return 0;
	}
	ressave=res;
	do {
		listenfd=::socket(res->ai_family,res->ai_socktype,res->ai_protocol);
		if (listenfd<0) continue;		// Error, try next one

		if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))!=0) {
			freeaddrinfo(ressave);
			SetError(334,"Sourceport: %s",host);
			return 0;
		}

		//HexDump(res->ai_addr,res->ai_addrlen);
		if (::bind(listenfd,res->ai_addr,res->ai_addrlen)==0) {

			break;
		}
		shutdown(listenfd,2);
		close(listenfd);
		listenfd=0;
	} while ((res=res->ai_next)!=NULL);
	freeaddrinfo(ressave);
	if (listenfd<=0) {
		SetError(393,"Host: %s, Port: %d",host,port);
		return 0;
	}
	if (res==NULL) {
		SetError(394,"Host: %s, Port: %d",host,port);
		return 0;
	}
	return listenfd;
}
#endif


/*!\brief Verbindung aufbauen
 *
 * Mit dieser Funktion wird eine Verbindung zur angegebenen Adresse aufgebaut.
 *
 * \param host Der Parameter "host" muss das Format "hostname:port" haben, wobei
 * "hostname" auch eine IP-Adresse sein kann. Der Port kann entweder als Zahl oder
 * als Servicename angegeben werden, z.B. "smtp" für Port 25.
 * \return Konnte die Verbindung erfolgreich aufgebaut werden, wird true (1) zurückgegeben,
 * im Fehlerfall false (0).
 * \since Seit Version 6.0.18 kann der Port statt als Nummer auch als Servicenamen angegeben
 * werden.
 */
int CTCPSocket::Connect(const char *host)
{
    if (!host) {
        SetError(270);
        return 0;
    }
	CTok hostname;
	hostname.Split(host,":");
	if (hostname.Num()!=2) {
		SetError(291);
		return 0;
	}
	const char *portname=hostname.Get(1);
	int port=ppl6::atoi(portname);
	//printf ("Debug: host=%s, portname=%s, port=%i\n",host,portname,port);
	if (port<=0 && strlen(portname)>0) {
		// Vielleicht wurde ein Service-Namen angegeben?
		struct servent *s=getservbyname(portname, "tcp");
		if (s) {
			unsigned short int p=s->s_port;
			port=(int) ntohs(p);
		}
	}
	//printf ("Debug: host=%s, hostname=%s, portname=%s, port=%i\n",host,hostname.Get(0),portname,port);
	if (port<=0) {
        SetError(271,"%s",host);
        return 0;
	}
	return Connect(hostname.Get(0),port);
}

#ifdef _WIN32
#else

/*!\brief Non-Blocking-Connect
 *
 * \desc
 * Diese Funktion wird intern durch die Connect-Funktionen aufgerufen, um einen nicht blockierenden
 * Connect auf die Zieladresse durchzuführen. Normalerweise würde ein Connect solange
 * blockieren, bis entweder ein Fehler festgestellt wird, die Verbindung zustande kommt oder
 * das Betriebssystem einen Timeout auslöst. Letzteres kann aber mehrere Minuten dauern.
 * Mit dieser Funktion wird ein non-blocking-connect durchgeführt, bei dem der Timeout
 * mikrosekundengenau angegeben werden kann.
 *
 * @param[in] sockfd File-ID des Sockets
 * @param[in] serv_addr IP-Adressenstruktur mit der Ziel-IP un dem Ziel-Port
 * @param[in] addrlen Die Länge der Adressenstruktur
 * @param[in] sec Timeout in Sekunden
 * @param[in] usec Timeout in Mikrosekunden. Der tatsächliche Timeout errechnet sich aus \p sec + \p usec
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \relates ppl6::CTCPSocket
 * \note
 * Zur Zeit wird diese Funktion nur unter Unix unterstützt.
 */
static int ppl_connect_nb(int sockfd, struct sockaddr *serv_addr, int  addrlen, int sec, int usec)
{
	int flags,n,error;
	struct timeval tval;
	socklen_t len;
	fd_set rset,wset;
	flags=fcntl(sockfd,F_GETFL,0);
	fcntl(sockfd,F_SETFL,flags|O_NONBLOCK);
	error=0;
	if ((n=::connect(sockfd,serv_addr,addrlen))<0)
		if (errno!=EINPROGRESS)
			return (-1);
	if (n==0)			// Connect completed immediately
		goto done;
	FD_ZERO(&rset);
	FD_SET(sockfd,&rset);
	wset=rset;
	tval.tv_sec=sec;
	tval.tv_usec=usec;
	if ((n=select(sockfd+1,&rset,&wset,NULL,&tval))==0) {
		errno=ETIMEDOUT;
		return -1;
	}
	if (FD_ISSET(sockfd,&rset) || FD_ISSET(sockfd,&wset)) {
		len=sizeof(error);
		if (getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len)<0) {
			return -1;		// Solaris pending error
		}
	}
	done:
		fcntl(sockfd,F_SETFL,flags);
		if (error) {
			errno=error;
			return (-1);
		}
		return 0;
}
#endif

/*!\fn CTCPSocket::Connect(const char *host, int port)
 * \brief Verbindung aufbauen
 *
 * \desc
 * Mit dieser Funktion wird eine Verbindung zur angegebenen Adresse aufgebaut.
 *
 * \param[in] host Der Hostname oder die IP-Adresse des Zielrechners
 * \param[in] port Der gewünschte Zielport
 * \return Konnte die Verbindung erfolgreich aufgebaut werden, wird true (1) zurückgegeben,
 * im Fehlerfall false (0).
 */
#ifdef _WIN32
int CTCPSocket::Connect(const char *host, int port)
{
    if (connected) Disconnect();
    if (islisten) Disconnect();
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
		s->port=0;
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (s->ipname) free(s->ipname);
	s->ipname=NULL;

    if (s->sd) Disconnect();
    if (!host) {
        SetError(270);
        return 0;
    }
    if (!port) {
        SetError(271);
        return 0;
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));

    // convert host to in_addr
    struct in_addr in;
	if (inet_aton(host,&in)) {
        addr.sin_addr.s_addr = in.s_addr;
    } else {                // failed, perhaps it's a hostname we have to resolve first?
        struct hostent *h=gethostbyname(host);
        if (!h) {
        	ppl6::SetError(TranslateSocketError(),"%s:%i",host,port);
			//int e=WSAGetLastError();
           	//SetError(273,e);
			return 0;
        }
		bcopy((void*)h->h_addr,(void*)&addr.sin_addr.s_addr,h->h_length);
    }
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

	// Socket erstellen
    s->sd = ::socket(PF_INET,SOCK_STREAM,s->proto);
    if (s->sd<=0) {
    	//ppl6::SetError(TranslateSocketError(),"%s:%i",host,port);
        SetError(272,"Return-Value of socket: %i, %s",s->sd,strerror(errno));
        return 0;
    }

    // Try to connect
    if (connect(s->sd, (struct sockaddr *)&addr, sizeof(addr) )!=0) {
    	ppl6::SetError(TranslateSocketError(),"%s:%i",host,port);
    	PushError();
		closesocket (s->sd);
		s->sd=0;
		// SetError(274,e,"errno=%i, %s",e,strerror(e));
		PopError();
        return 0;
    }
    SetError(0);
    connected=1;
    return 1;
}


#else

int CTCPSocket::Connect(const char *host, int port)
{
    if (connected) Disconnect();
    if (islisten) Disconnect();
	BytesWritten=0;
	BytesRead=0;
	if (!socket) {
		socket=malloc(sizeof(PPLSOCKET));
		if (!socket) {
			SetError(2);
			return 0;
		}
		PPLSOCKET *s=(PPLSOCKET*)socket;
    	s->sd=0;
		//s->proto=6;
		s->proto=0;
		s->ipname=NULL;
		s->port=0;
		//s->addrlen=0;
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (s->ipname) free(s->ipname);
	s->ipname=NULL;

    if (s->sd) Disconnect();
    if (!host) {
        SetError(270);
        return 0;
    }
    if (!port) {
        SetError(271);
        return 0;
    }
	#ifdef _WIN32
		SOCKET	sockfd;
	#else
		int sockfd;
	#endif
    int n;
    struct addrinfo hints, *res, *ressave;
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
	char portstr[10];
	sprintf(portstr,"%i",port);
	if ((n=getaddrinfo(host,portstr,&hints,&res))!=0) {
		SetError(273,"%s, %s",host,gai_strerror(n));
		return 0;
	}
	ressave=res;
	int e=0, conres=0;
	do {
		if (SourceHost.Len()>0 || SourcePort>0) {
			if ((sockfd=out_bind((const char*)SourceHost,SourcePort))<=0) {
				shutdown(sockfd,2);
				close(sockfd);
				freeaddrinfo(ressave);
				return 0;
			}
		} else {
			sockfd=::socket(res->ai_family,res->ai_socktype,res->ai_protocol);
			if (sockfd<0) continue;		// Error, try next one
		}
		if (log) {
			log->Printf(LOG::DEBUG,6,__FILE__,__LINE__,"CTCPSocket::Connect(%s,%i) => Got Descriptor: %i\n",host,port,sockfd);
			log->HexDump(LOG::DEBUG,10,res->ai_addr,res->ai_addrlen);
		}
		if (connect_timeout_sec>0 || connect_timeout_usec) {
			#ifdef _WIN32
				conres=::connect(sockfd,res->ai_addr,res->ai_addrlen);
			#else
				conres=ppl_connect_nb(sockfd,res->ai_addr,res->ai_addrlen,connect_timeout_sec,connect_timeout_usec);
				e=errno;
			#endif
		} else {
			conres=::connect(sockfd,res->ai_addr,res->ai_addrlen);
			e=errno;
			if (conres<0) {
				if (log) log->HexDump(LOG::DEBUG,6,res->ai_addr,res->ai_addrlen);
			}

		}
		if (conres==0) {
			if (log) log->Printf(LOG::DEBUG,6,__FILE__,__LINE__,"CTCPSocket::Connect(%s,%i) => Connected with Descriptor: %i\n",host,port,sockfd);
			break;
		}
		#ifdef _WIN32
			e=WSAGetLastError();
			shutdown(sockfd,2);
			closesocket(sockfd);
		#else
			shutdown(sockfd,2);
			close(sockfd);
		#endif
		if (log) log->Printf(LOG::DEBUG,6,__FILE__,__LINE__,"CTCPSocket::Connect(%s,%i) => Close Descriptor: %i\n",host,port,sockfd);
		sockfd=0;
	} while ((res=res->ai_next)!=NULL);
	if (conres<0) {
		res=NULL;
	}
	if (sockfd<0) {
		freeaddrinfo(ressave);
		SetError(265,e,"Host: %s, Port: %d",host,port);
		return 0;
	}
	if (res==NULL) {
		int pple=TranslateErrno(e);
		if (pple==174) {
			SetError(174,e,"Host: %s, Port: %d, errno=%i, %s",host,port,e,strerror(e));
		} else {
			SetError(274,e,"Host: %s, Port: %d, errno=%i, %s >>%s<<",host,port,e,strerror(e), GetError(TranslateErrno(e)));
		}
		freeaddrinfo(ressave);
		return 0;
	}
	if (log) log->Printf(LOG::DEBUG,6,__FILE__,__LINE__,"CTCPSocket::Connect(%s,%i) => Connect ok, Descriptor: %i\n",host,port,sockfd);
	s->sd=sockfd;
	//s->addrlen=res->ai_addrlen;
	HostName=host;
	PortNum=port;
	connected=1;
	freeaddrinfo(ressave);
	return 1;
}
#endif



/*!\brief Verbindung trennen
 *
 * \desc
 * Durch Aufruf dieser Funktion wird eine bestehende Verbindung beendet. Unter Windows ist die Funktion
 * identisch mit CTCPSocket::Shutdown, unter Unix unterscheiden sie sich.
 * \par
 * CTCPSocket::Disconnect schließt den Socket-Descriptor für den Prozess, die Verbindung bleibt
 * jedoch weiterhin bestehen, wenn ein anderer Prozess den gleichen Descriptor verwendet.
 * Die Verbindung bleibt dann sowohl zum lesen als auch zum schreiben geöffnet.
 *
 * \par
 * CTCPSocket::Shutdown trennt die Verbindung für alle Prozesse, die den gleichen Descriptor
 * verwenden. Falls ein anderer Prozess noch versucht auf den Socket zuzzugreifen, bekommt er eine
 * Fehlermeldung. Beim Lesen ist dies meist ein EOF, beim Schreiben ein SIGPIPE, der möglicherweise
 * Aufgrund von Puffern im Kernel nicht sofort auftreten muss.
 * @return Die Funktion liefert immer 1 zurück.
 */
int CTCPSocket::Disconnect()
{
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::Disconnect()");
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) return 1;
	if (islisten) {
		StopListen();
	}
	SSL_Stop();
    //if (s->sd>0) shutdown(s->sd,2);
    connected=false;
    if (s->sd>0) {
    	if (log) log->Printf(LOG::DEBUG,6,__FILE__,__LINE__,"CTCPSocket::Disconnect() => Close Descriptor: %i\n",(const char*)HostName,PortNum,s->sd);
    	//printf ("Socket Disconnect Descriptor: %i\n",s->sd);
		#ifdef _WIN32
			closesocket(s->sd);
		#else
			close (s->sd);
		#endif
        s->sd=0;
    }
    HostName="";
    PortNum=0;
	islisten=false;
	BytesWritten=0;
	BytesRead=0;
    return 1;
}

/*!\brief Verbindung trennen
 *
 * \desc
 * Durch Aufruf dieser Funktion wird eine bestehende Verbindung beendet. Unter Windows ist die Funktion
 * identisch mit CTCPSocket::Shutdown, unter Unix unterscheiden sie sich.
 * \par
 * CTCPSocket::Disconnect schließt den Socket-Descriptor für den Prozess, die Verbindung bleibt
 * jedoch weiterhin bestehen, wenn ein anderer Prozess den gleichen Descriptor verwendet.
 * Die Verbindung bleibt dann sowohl zum lesen als auch zum schreiben geöffnet.
 *
 * \par
 * CTCPSocket::Shutdown trennt die Verbindung für alle Prozesse, die den gleichen Descriptor
 * verwenden. Falls ein anderer Prozess noch versucht auf den Socket zuzzugreifen, bekommt er eine
 * Fehlermeldung. Beim Lesen ist dies meist ein EOF, beim Schreiben ein SIGPIPE, der möglicherweise
 * Aufgrund von Puffern im Kernel nicht sofort auftreten muss.
 * @return Die Funktion liefert immer 1 zurück.
 */
int CTCPSocket::Shutdown()
{
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::Disconnect()");
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!s) return 1;
	if (islisten) {
		StopListen();
	}
	SSL_Stop();
    //if (s->sd>0) shutdown(s->sd,2);
    connected=false;
    if (s->sd>0) {
    	if (log) log->Printf(LOG::DEBUG,6,__FILE__,__LINE__,"CTCPSocket::Disconnect() => Close Descriptor: %i\n",(const char*)HostName,PortNum,s->sd);
    	//printf ("Socket Disconnect Descriptor: %i\n",s->sd);
		#ifdef _WIN32
			closesocket(s->sd);
		#else
			shutdown (s->sd,SHUT_RDWR);
		#endif
        s->sd=0;
    }
    HostName="";
    PortNum=0;
	islisten=false;
	BytesWritten=0;
	BytesRead=0;
    return 1;
}

/*!\brief Prüfen, ob eine Verbindung besteht
 *
 * \desc
 * Mit dieser Funktion kann überprüft werden, ob eine TCP-Verbindung besteht.
 *
 * @return Liefert 1 zurück, wenn eine Verbindung besteht, sonst 0. Es wird kein Fehlercode gesetzt.
 */
int CTCPSocket::IsConnected()
{
	return connected;
}

/*!\brief Prüfen, ob der Socket auf eingehende Verbindungen wartet
 *
 * \desc
 * Mit dieser Funktion kann überprüft werden, ob der Socket auf eingehende TCP-Verbindungen wartet.
 *
 * @return Liefert 1 zurück, wenn der Socket wartet, sonst 0. Es wird kein Fehlercode gesetzt.
 */
int CTCPSocket::IsListening()
{
	return islisten;
}

/*!\brief Anzahl gesendeter Bytes
 *
 * \desc
 * Diese Funktion liefert die Anzahl gesendeter Bytes während der letzten Schreiboperation.
 *
 * @return Anzahl Bytes
 */
int CTCPSocket::GetBytesWritten()
{
	return BytesWritten;
}

/*!\brief Anzahl gelesender Bytes
 *
 * \desc
 * Diese Funktion liefert die Anzahl gelesener Bytes während der letzten Leseoperation.
 *
 * @return Anzahl Bytes
 */
int CTCPSocket::GetBytesRead()
{
	return BytesRead;
}

/*!\brief Daten schreiben
 *
 * \desc
 * Mit dieser Funktionen werden \p bytes Bytes aus dem Speicherbereich \p buffer an die Gegenstelle
 * gesendet.
 *
 * @param[in] buffer Pointer auf die zu sendenden Daten
 * @param[in] bytes Anzahl zu sendender Bytes
 * @return Wenn die Daten erfolgreich geschrieben wurden, gibt die Funktion 1 zurück, im Fehlerfall 0.
 * Falls ein Fehler aufgetreten ist, kann mit der Funktion CTCPSocket::GetBytesWritten abgefragt werden,
 * wieviele Bytes wirklich geschrieben wurden.
 */
int CTCPSocket::Write(const void *buffer, int bytes)
{
	if (log) {
		log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Write(void *buffer, int bytes=%i",bytes);
		//log->HexDump(LOG::DEBUG,11,buffer,bytes);
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
		if (log) log->LogError(5);
        return 0;
    }
	if (!buffer) {
		SetError(194,"void *buffer");
		if (log) log->LogError(5);
		return 0;
	}
	BytesWritten=0;
	if (bytes) {
		int rest=bytes;
		int b=0;
		while (rest) {
			if (ssl) b=SSL_Write(buffer,rest);
			else b=::send(s->sd,(char *)buffer,rest,0);
			if (b>0) {
				BytesWritten+=b;
				rest-=b;
				buffer=((char*)buffer)+b;
			}
			if (log) log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Write: ::write, bytes=%i, b=%i, rest=%i",bytes,b,rest);
			#ifdef WIN32
			if (b==SOCKET_ERROR) {
			#else
			if (b<0) {
			#endif
				if (errno==EAGAIN) {
					WaitForOutgoingData(0,100000);
				} else  {
					if (log) log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Write: ERROR");
					DispatchErrno();
					return 0;
				}
			}
		}
	}
	if (log) log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Write: OK");
	return 1;
}


/*!\brief Lese-Timeout festlegen
 *
 * Mit dieser Funktion kann ein Timeout für Lesezugriffe gesetzt werden. Normalerweise
 * blockiert eine Leseoperation mit "Read" solange, bis die angeforderten Daten
 * eingegangen sind (ausser der Socket wurde mit CTCPSocket::SetBlocking auf "Non-Blocking"
 * gesetzt). Mit dieser Funktion kann jedoch ein beliebiger mikrosekunden genauer
 * Timeout festgelegt werden. Der Timeout errechnet sich dabei aus
 * \p seconds + \p useconds.
 * \par
 * Um den Timeout wieder abzustellen, kann die Funktion mit 0 als
 * Wert für \p seconds und \p useconds aufgerufen werden.
 *
 * @param[in] seconds Anzahl Sekunden
 * @param[in] useconds Anzahl Mikrosekunden (1000000 Mikrosekunden = 1 Sekunde)
 * @return Konnte der Timeout errfolgreich gesetzt werden, gibt die Funktion
 * 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SetReadTimeout(int seconds, int useconds)
{
	struct timeval tv;
	tv.tv_sec=seconds;
	tv.tv_usec=useconds;
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
		if (log) log->LogError(5);
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

/*!\brief Schreib-Timeout festlegen
 *
 * Mit dieser Funktion kann ein Timeout für Schreibzugriffe gesetzt werden. Normalerweise
 * blockiert eine Schreiboperation mit "Write" solange, bis alle Daten gesendet wurden.
 * Mit dieser Funktion kann jedoch ein beliebiger mikrosekunden genauer
 * Timeout festgelegt werden. Der Timeout errechnet sich dabei aus
 * \p seconds + \p useconds.
 * \par
 * Um den Timeout wieder abzustellen, kann die Funktion mit 0 als
 * Wert für \p seconds und \p useconds aufgerufen werden.
 *
 * @param[in] seconds Anzahl Sekunden
 * @param[in] useconds Anzahl Mikrosekunden (1000000 Mikrosekunden = 1 Sekunde)
 * @return Konnte der Timeout errfolgreich gesetzt werden, gibt die Funktion
 * 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SetWriteTimeout(int seconds, int useconds)
{
	struct timeval tv;
	tv.tv_sec=seconds;
	tv.tv_usec=useconds;
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (!connected) {
		SetError(275);
		if (log) log->LogError(5);
		return 0;
	}
#ifdef WIN32
	if (setsockopt(s->sd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&tv,sizeof(tv))!=0) {
#else
	if (setsockopt(s->sd,SOL_SOCKET,SO_SNDTIMEO,(void*)&tv,sizeof(tv))!=0) {
#endif
		SetError(350);
		return 0;
	}
	return 1;
}


/*!\brief Daten lesen
 *
 * \desc
 * Diese Funktion liest genau \p bytes vom Socket in den von der Anwendung
 * angegebenen Puffer \p buffer. Falls der Socket auf "non-blocking" eingestellt
 * wurde und mit CTCPSocket::WatchThread ein Überwachungsthread angegeben wurde,
 * prüft die Funktion regelmäßig, ob ein Stop signalisiert wurde.
 *
 * @param[in] buffer Speicherbereich, in dem die gelesenen Daten abgelegt
 * werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Ist ein Fehler aufgetreten, kann mit der Funktion
 * CTCPSocket::GetBytesRead abgefragt werden, wieviel Bytes tatsächlich gelesen wurden.
 */
int CTCPSocket::Read(void *buffer, int bytes)
{
	if (log) {
		log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Read(void *buffer, int bytes=%i",bytes);
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
		if (log) log->LogError(5);
        return 0;
    }
	if (!buffer) {
		SetError(194,"void *buffer");
		if (log) log->LogError(5);
		return 0;
	}
	BytesRead=0;
	if (bytes) {
		int rest=bytes;
		int b=0;
		while (rest) {
			if (thread) {
				if (thread->ThreadShouldStop()) {
					SetError(336);
					return 0;
				}
			}
			WaitForIncomingData(0,100000);
			if (ssl) {
				b=SSL_Read(buffer,rest);
			} else {
				//b=::recv(s->sd,(char*)buffer,rest,0);
				b=::recv(s->sd,(char*)buffer,rest,0);
			}
			if (b>0) {
				BytesRead+=b;
				rest-=b;
				buffer=((char*)buffer)+b;
			}
			if (log) log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Read: ::read, bytes=%i, b=%i, rest=%i",bytes,b,rest);
			if (b==0) {
				if (log) log->Printf(LOG::DEBUG,5,"CTCPSocket::Read: 0-Byte gelesen, ERROR %i: %s",errno,strerror(errno));
				SetError(308,errno,"0-Byte gelesen");
				return 0;
			}
			#ifdef WIN32
			if (b==SOCKET_ERROR) {
					if (log) log->Printf(LOG::DEBUG,5,"CTCPSocket::Read: ERROR %i: %s",errno,strerror(errno));
					DispatchErrno();
					return 0;
			}
			#else
			if (b<0) {
				if (errno==EAGAIN || (errno==EINTR && ssl==true) ) {
					WaitForIncomingData(0,100000);
				} else {
					if (log) log->Printf(LOG::DEBUG,5,"CTCPSocket::Read: ERROR %i: %s",errno,strerror(errno));
					DispatchErrno();
					return 0;
				}
			}
			#endif
		}
	}
	if (log) {
		log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::Read: OK");
		//log->HexDump(LOG::DEBUG,11,buffer,bytes);
	}
	return 1;
}

/*!\brief Daten lesen
 *
 * \desc
 * Diese Funktion liest maximal \p bytes vom Socket in den von der Anwendung
 * angegebenen Puffer \p buffer. Falls der Socket auf "blocking" eingestellt ist (=Default)
 * wartet die Funktion solange, bis die ersten Daten eingegangen sind und liefert diese zurück.
 * Ist der Socket auf "non blocking" eingestellt, kehrt die Funktion sofort zurück, wenn keine
 * Daten zur Verfügung stehen. Die Anzahl tatsächlich gelesener Bytes kann mit der Funktion
 * CTCPSocket::GetBytesRead ausgelesen werden.
 *
 * @param[in] buffer Speicherbereich, in dem die gelesenen Daten abgelegt
 * werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Die Anzahl tatsächlich gelesener Bytes kann mit
 * CTCPSocket::GetBytesRead abgefragt werden.
 */
int CTCPSocket::ReadOnce(void *buffer, int bytes)
{
	if (log) {
		log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::ReadOnce(void *buffer, int bytes=%i",bytes);
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
		if (log) log->LogError(5);
        return 0;
    }
	if (!buffer) {
		SetError(194,"void *buffer");
		if (log) log->LogError(5);
		return 0;
	}
	BytesRead=0;
	if (bytes) {
		if (ssl) BytesRead=SSL_Read(buffer,bytes);
		else BytesRead=::recv(s->sd,(char*)buffer,bytes,0);
		if (log) log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::ReadOnce: ::read, bytes=%i, b=%i",bytes,BytesRead);
		if (BytesRead==0) {
			if (!errno) {
				// Das dürfte nicht vorkommen, es sei denn, die Verbindung wurde getrennt/unterbrochen
				if (!IsReadable ()) return 0;
				SetError(312);
				return 0;
			}
			DispatchErrno();
			if (log) log->Printf(LOG::DEBUG,5,"CTCPSocket::ReadOnce: 0-Byte gelesen, ERROR %i: %s",errno,strerror(errno));
			//SetError(308,errno,"0-Byte gelesen");
			return 0;
		}
		#ifdef WIN32
			if (BytesRead==SOCKET_ERROR) {
				if (log) log->Printf(LOG::DEBUG,5,"CTCPSocket::ReadOnce: ERROR %i: %s",errno,strerror(errno));
				DispatchErrno();
				return 0;
			}
		#else
			if (BytesRead<0) {
				//if (errno=EAGAIN) return 1;
				//else  {
					//if (log) log->Printf(LOG::DEBUG,5,"CTCPSocket::ReadOnce: ERROR %i: %s",errno,strerror(errno));
					DispatchErrno();
					return 0;
				//}
			}
		#endif
	}
	if (log) {
		log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CTCPSocket::ReadOnce: OK");
		//log->HexDump(LOG::DEBUG,11,buffer,BytesRead);
	}
	return 1;
}

/*!\brief Daten schreiben
 *
 * \desc
 * Mit dieser Funktionen werden \p bytes Bytes aus dem Speicherbereich \p buffer an die Gegenstelle
 * gesendet.
 *
 * @param[in] buffer Pointer auf die zu sendenden Daten
 * @param[in] bytes Anzahl zu sendender Bytes
 * @return Wenn die Daten erfolgreich geschrieben wurden, gibt die Funktion 1 zurück, im Fehlerfall 0.
 * Falls ein Fehler aufgetreten ist, kann mit der Funktion CTCPSocket::GetBytesWritten abgefragt werden,
 * wieviele Bytes wirklich geschrieben wurden.
 */
int CTCPSocket::WriteBuffer(const void *buffer, int bytes)
{
	return Write(buffer,bytes);
}

/*!\brief String schreiben
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings \p str an die Gegenstelle
 * gesendet.
 *
 * @param[in] str Referenz auf eine CString-Klasse
 * @return Wenn die Daten erfolgreich geschrieben wurden, gibt die Funktion 1 zurück, im Fehlerfall 0.
 * Falls ein Fehler aufgetreten ist, kann mit der Funktion CTCPSocket::GetBytesWritten abgefragt werden,
 * wieviele Bytes wirklich geschrieben wurden.
 */
int CTCPSocket::Write(const CString &str)
{
	return Write((const void*)str.GetPtr(),str.Len());
}

/*!\brief String schreiben
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings \p str an die Gegenstelle
 * gesendet.
 *
 * @param[in] str Pointer auf eine CString-Klasse
 * @return Wenn die Daten erfolgreich geschrieben wurden, gibt die Funktion 1 zurück, im Fehlerfall 0.
 * Falls ein Fehler aufgetreten ist, kann mit der Funktion CTCPSocket::GetBytesWritten abgefragt werden,
 * wieviele Bytes wirklich geschrieben wurden.
 */
int CTCPSocket::Write(const CString *str)
{
	if (!str) {
		SetError(194);
		return 0;
	}
	return Write((const void*)str->GetPtr(),str->Len());
}

/*!\brief Formatierten String schreiben
 *
 * \desc
 * Mit dieser Funktion wird zunächst ein String anhand der des angegebenen
 * Formatierungsstrings \p fmt und den zusätzlichen optionalen Parametern gebildet und dieser
 * an die Gegenstelle gesendet.
 *
 * @param[in] fmt Formatierungsstring
 * @param[in] ... Optionale Parameter für den Formatierungsstring
 * @return Wenn die Daten erfolgreich geschrieben wurden, gibt die Funktion 1 zurück, im Fehlerfall 0.
 * Falls ein Fehler aufgetreten ist, kann mit der Funktion CTCPSocket::GetBytesWritten abgefragt werden,
 * wieviele Bytes wirklich geschrieben wurden.
 */
int CTCPSocket::Writef(const char *fmt, ...)
{
	if (!fmt) {
		SetError(194,"char *fmt");
		return 0;
	}
	CString str;
	va_list args;
	va_start(args, fmt);
	str.Vasprintf(fmt,args);
	va_end(args);
	return Write(str);
}

/*!\brief Binäre Daten schreiben
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des CBinary-Objekts \p bin an die Gegenstelle
 * gesendet.
 *
 * @param[in] bin Referenz auf eine CBinary-Klasse
 * @return Wenn die Daten erfolgreich geschrieben wurden, gibt die Funktion 1 zurück, im Fehlerfall 0.
 * Falls ein Fehler aufgetreten ist, kann mit der Funktion CTCPSocket::GetBytesWritten abgefragt werden,
 * wieviele Bytes wirklich geschrieben wurden.
 */
int CTCPSocket::Write(const CBinary &bin)
{
	return Write((const void*)bin.GetPtr(),bin.GetSize());
}


/*!\brief Daten in einen String einlesen
 *
 * \desc
 * Diese Funktion liest genau \p bytes vom Socket in den Buffer des Strings \p buffer ein.
 * Falls der Socket auf "blocking" eingestellt ist (=Default), wartet die Funktion, bis die
 * gewünschte Anzahl Bytes eingegangen ist. Ist er auf "non-blocking" eingestellt,
 * und es wurde mit CTCPSocket::WatchThread ein Überwachungsthread angegeben,
 * prüft die Funktion regelmäßig, ob ein Stop signalisiert wurde.
 *
 * @param[out] buffer String, in dem die gelesenen Daten abgelegt werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Ist ein Fehler aufgetreten, kann mit der Funktion
 * CTCPSocket::GetBytesRead abgefragt werden, wieviel Bytes tatsächlich gelesen wurden.
 */
int CTCPSocket::Read(CString &buffer, int bytes)
{
	char *b=(char*)malloc(bytes+1);
	if (!b) {
		SetError(2);
		return 0;
	}
	int ret=Read(b,bytes);
	if (!ret) {
		bytes=GetBytesRead();
	}
	b[bytes]=0;
	buffer.ImportBuffer(b,bytes+1);
	return ret;
}

/*!\brief Daten in einen String einlesen
 *
 * \desc
 * Diese Funktion liest genau \p bytes vom Socket in den Buffer des Strings \p buffer ein.
 * Falls der Socket auf "blocking" eingestellt ist (=Default), wartet die Funktion, bis die
 * gewünschte Anzahl Bytes eingegangen ist. Ist er auf "non-blocking" eingestellt,
 * und es wurde mit CTCPSocket::WatchThread ein Überwachungsthread angegeben,
 * prüft die Funktion regelmäßig, ob ein Stop signalisiert wurde.
 *
 * @param[out] buffer String, in dem die gelesenen Daten abgelegt werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Ist ein Fehler aufgetreten, kann mit der Funktion
 * CTCPSocket::GetBytesRead abgefragt werden, wieviel Bytes tatsächlich gelesen wurden.
 */
int CTCPSocket::Read(CString *buffer, int bytes)
{
	if (!buffer) {
		SetError(194);
		return 0;
	}
	char *b=(char*)malloc(bytes+1);
	if (!b) {
		SetError(2);
		return 0;
	}
	int ret=Read(b,bytes);
	if (!ret) {
		bytes=BytesRead;
	}
	b[bytes]=0;
	buffer->ImportBuffer(b,bytes+1);
	return ret;
}

/*!\brief Daten in ein CBinary-Objekt einlesen
 *
 * \desc
 * Diese Funktion liest genau \p bytes vom Socket in das CBinary-Objekt \p buffer ein.
 * Falls der Socket auf "blocking" eingestellt ist (=Default), wartet die Funktion, bis die
 * gewünschte Anzahl Bytes eingegangen ist. Ist er auf "non-blocking" eingestellt,
 * und es wurde mit CTCPSocket::WatchThread ein Überwachungsthread angegeben,
 * prüft die Funktion regelmäßig, ob ein Stop signalisiert wurde.
 *
 * @param[out] buffer CBinary-Objekt, in dem die gelesenen Daten abgelegt werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Ist ein Fehler aufgetreten, kann mit der Funktion
 * CTCPSocket::GetBytesRead abgefragt werden, wieviel Bytes tatsächlich gelesen wurden.
 */
int CTCPSocket::Read(CBinary &buffer, int bytes)
{
	char *b=(char*)malloc(bytes);
	if (!b) {
		SetError(2);
		return 0;
	}
	int ret=Read(b,bytes);
	if (!ret) {
		bytes=GetBytesRead();
	}
	b[bytes]=0;
	buffer.Set(b,bytes);
	buffer.ManageMemory();
	return ret;
}


/*!\brief Daten lesen
 *
 * \desc
 * Diese Funktion liest genau \p bytes vom Socket. Falls der Socket nicht
 * auf Non-Blocking gestellt wurde, wartet die Funktion solange, bis mindestens
 * \p bytes Bytes eingegangen sind.
 * \par
 * Die Funktion allokiert Speicher mit "malloc", um die eingegangenen Daten zu
 * speichern. Die Anwendung muss diesen daher selbst mit "free" wieder
 * freigeben. Mit der Funktion CTCPSocket::GetBytesRead kann abgefragt werden,
 * wieviele Bytes gelesen wurden.
 *
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Liefert einen Pointer auf den Speicherbereich zurück, in dem die
 * gelesenen Daten abgelegt wurden. Dieser muss von der Anwendung mit "free"
 * wieder freigegeben werden. Im Fehlerfall wird NULL zurückgegeben.
 */
char *CTCPSocket::Read(int bytes)
{
	if (!bytes) return NULL;
	char *str=(char*)malloc(bytes+1);
	if (!str) return NULL;
	str[bytes]=0;
	int ret=Read(str,bytes);
	if (!ret) {
		free(str);
		return NULL;
	}
	return str;
}

/*!\brief Daten lesen
 *
 * \desc
 * Diese Funktion liest maximal \p bytes vom Socket in den String \p buffer.
 * Falls der Socket auf "blocking" eingestellt ist (=Default)
 * wartet die Funktion solange, bis die ersten Daten eingegangen sind und liefert diese zurück.
 * Ist der Socket auf "non blocking" eingestellt, kehrt die Funktion sofort zurück, wenn keine
 * Daten zur Verfügung stehen. Die Anzahl tatsächlich gelesener Bytes kann mit der Funktion
 * CTCPSocket::GetBytesRead ausgelesen werden.
 *
 * @param[in] buffer String, in dem die gelesenen Daten abgelegt werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Die Anzahl tatsächlich gelesener Bytes kann mit
 * CTCPSocket::GetBytesRead abgefragt werden.
 */
int CTCPSocket::ReadOnce(CString &buffer, int bytes)
{
	char *b=(char*)malloc(bytes+1);
	if (!b) {
		SetError(2);
		return 0;
	}
	int ret=ReadOnce(b,bytes);
	if (ret) {
		b[BytesRead]=0;
		buffer.ImportBuffer(b,bytes+1);
	}
	return ret;
}

/*!\brief Daten lesen
 *
 * \desc
 * Diese Funktion liest maximal \p bytes vom Socket in den String \p buffer.
 * Falls der Socket auf "blocking" eingestellt ist (=Default)
 * wartet die Funktion solange, bis die ersten Daten eingegangen sind und liefert diese zurück.
 * Ist der Socket auf "non blocking" eingestellt, kehrt die Funktion sofort zurück, wenn keine
 * Daten zur Verfügung stehen. Die Anzahl tatsächlich gelesener Bytes kann mit der Funktion
 * CTCPSocket::GetBytesRead ausgelesen werden.
 *
 * @param[in] buffer String, in dem die gelesenen Daten abgelegt werden sollen.
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Die Funktion gibt 1 zurück, wenn die Bytes erfolgreich
 * gelesen wurden, im Fehlerfall 0. Die Anzahl tatsächlich gelesener Bytes kann mit
 * CTCPSocket::GetBytesRead abgefragt werden.
 */
int CTCPSocket::ReadOnce(CString *buffer, int bytes)
{
	if (!buffer) {
		SetError(194);
		return 0;
	}
	char *b=(char*)malloc(bytes+1);
	if (!b) {
		SetError(2);
		return 0;
	}
	int ret=ReadOnce(b,bytes);
	if (ret) {
		b[BytesRead]=0;
		buffer->ImportBuffer(b,bytes+1);
	}
	return ret;
}

/*!\brief Daten lesen
 *
 * \desc
 * Diese Funktion liest maximal \p bytes vom Socket und gibt einen Pointer auf
 * den Speicherbereich zurück.
 * Falls der Socket auf "blocking" eingestellt ist (=Default)
 * wartet die Funktion solange, bis die ersten Daten eingegangen sind und liefert diese zurück.
 * Ist der Socket auf "non blocking" eingestellt, kehrt die Funktion sofort zurück, wenn keine
 * Daten zur Verfügung stehen. Die Anzahl tatsächlich gelesener Bytes kann mit der Funktion
 * CTCPSocket::GetBytesRead ausgelesen werden.
 * \par
 * Die Funktion allokiert Speicher mit "malloc", um die eingegangenen Daten zu
 * speichern. Die Anwendung muss diesen daher selbst mit "free" wieder
 * freigeben. Mit der Funktion CTCPSocket::GetBytesRead kann abgefragt werden,
 * wieviele Bytes gelesen wurden.
 *
 * @param[in] bytes Anzahl zu lesender Bytes
 * @return Liefert einen Pointer auf den Speicherbereich zurück, in dem die
 * gelesenen Daten abgelegt wurden. Dieser muss von der Anwendung mit "free"
 * wieder freigegeben werden. Im Fehlerfall wird NULL zurückgegeben.
 * Die Anzahl tatsächlich gelesener Bytes kann mit CTCPSocket::GetBytesRead abgefragt werden.
 */
char *CTCPSocket::ReadOnce(int bytes)
{
	if (!bytes) return NULL;
	char *str=(char*)malloc(bytes+1);
	if (!str) return NULL;
	str[bytes]=0;
	int ret=ReadOnce(str,bytes);
	if (!ret) {
		free(str);
		return NULL;
	}
	str[BytesRead]=0;
	return str;
}


/*!\brief TCP-Server anhalten
 *
 * \desc
 * Durch Aufruf dieser Funktion innerhalb eines anderen Threads wird dem Server mitgeteilt, dass er nicht
 * weiter auf Verbindungseingänge warten soll. Die Funktion kehrt dabei erst zurück, wenn der Server sich
 * beendet hat. Falls man darauf nicht warten möchte, kann stattdessen auch CTCPSocket::SignalStopListen
 * aufgerufen werden.
 *
 * @return Die Funktion liefert immer 1 zurück.
 */
int CTCPSocket::StopListen()
{
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::StopListen()");
	mutex.Lock();
	stoplisten=true;
	while (islisten) {
		mutex.Unlock();
		MSleep(1);
		mutex.Lock();
	}
	stoplisten=false;
	mutex.Unlock();
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::StopListen() DONE");
	return 1;
}

/*!\brief TCP-Server signalisieren, dass er stoppen soll
 *
 * \desc
 * Durch Aufruf dieser Funktion innerhalb eines anderen Threads wird dem Server mitgeteilt, dass er nicht
 * weiter auf Verbindungseingänge warten soll. Die Funktion kehrt dabei sofort zurück und es liegt an der
 * Anwendung später zu prüfen, ob der Server wirklich gestoppt wurde (z.B. mit CTCPSocket::StopListen).
 *
 * @return Die Funktion liefert immer 1 zurück.
 */
int CTCPSocket::SignalStopListen()
{
	if (log) log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CTCPSocket::SignalStopListen()");
	stoplisten=true;
	return 1;
}

#ifdef _WIN32
int CTCPSocket::Bind(const char *host, int port)
{
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
		s->port=0;
	}
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (s->sd) Disconnect();
	if (s->ipname) free(s->ipname);
	s->ipname=NULL;
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));

	PortNum=port;

	if (!host) {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		HostName="*";
	} else {
		HostName=host;
		// convert host to in_addr
		struct in_addr in;
		if (inet_aton(host,&in)) {
			addr.sin_addr.s_addr = in.s_addr;
		} else {                // failed, perhaps it's a hostname we have to resolve first?
			struct hostent *h=gethostbyname(host);
			if (!h) {
				SetError(273);
				return 0;
			}
			// Copy hostaddress
			bcopy((void*)h->h_addr,(void*)&addr.sin_addr.s_addr,h->h_length);
		}
	}
	addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    /* create socket */
    s->sd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(s->sd<0) {
        SetError(265);
        return 0;
    }

    /* bind server port */
    if(bind(s->sd, (struct sockaddr *) &addr, sizeof(addr))<0) {
		shutdown(s->sd,2);
		closesocket(s->sd);
		s->sd=0;
		SetError(266);
		return 0;
	}
	connected=1;
	SetError(0);
	return 1;
}

#else

/*!\brief Socket auf eine IP-Adresse und Port binden
 *
 * \desc
 * Diese Funktion muss aufgerufen werden, bevor man mit CTCPSocket::Listen einen TCP-Server starten kann. Dabei wird mit \p host
 * die IP-Adresse festgelegt, auf die sich der Server binden soll und mit \p port der TCP-Port.
 * Es ist nicht möglich einen Socket auf mehrere Adressen zu binden.
 *
 * @param[in] host IP-Adresse, Hostname, "*" oder NULL. Bei Angabe von "*" oder NULL bindet sich der Socket auf alle
 * Interfaces des Servers.
 * @param[in] port Der gewünschte TCP-Port
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::Bind(const char *host, int port)
{
	//int addrlen=0;
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
		//s->addrlen=0;
	}
#ifdef _WIN32
	SOCKET	listenfd=0;
#else
	int listenfd=0;
#endif

	PPLSOCKET *s=(PPLSOCKET*)socket;
	if (s->sd) Disconnect();
	if (s->ipname) free(s->ipname);
	s->ipname=NULL;

	struct addrinfo hints, *res, *ressave;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags=AI_PASSIVE;
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	int n;
	int on=1;
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
			if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))!=0) {
				freeaddrinfo(ressave);
				SetError(334);
				return 0;
			}
			if (::bind(listenfd,res->ai_addr,res->ai_addrlen)==0) {
				//addrlen=res->ai_addrlen;
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
		listenfd=::socket(AF_INET, SOCK_STREAM, 0);
		if (listenfd>=0) {
			struct sockaddr_in addr;
			memset(&addr,0,sizeof(addr));
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
    		//s->addrlen=0;
    		connected=1;
    		HostName="*";
    		PortNum=port;
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
	//s->addrlen=addrlen;
	connected=1;
	HostName=host;
	PortNum=port;
	return 1;
}

#endif

/*!\brief Bei Lesezugriffen blockieren
 *
 * \desc
 * Durch Aufruf dieser Funktion kann festgelegt werden, ob der Socket bei Lesezugriffen
 * blockieren soll. Nach dem Öffnen des Sockets ist dieser defaultmäßig so
 * eingestellt, dass er bei Lesezugriffen solange blockiert (wartet), bis Daten zur
 * Verfügung stehen. Wird er auf "Non-Blocking" gestellt, kehren die Lese-Funktionen
 * sofort mit einer Fehlermeldung zurück, wenn noch keine Daten bereitstehen.
 *
 * @param[in] value Der Wert "true" setzt den Socket in den Blocking-Modus, was auch der
 * Default ist. Durch den Wert "false" wird er in den Non-Blocking-Modus gesetzt.
 * @return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SetBlocking(bool value)
{
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if((!s) || (!s->sd)) {
		SetError(267);
		return 0;
	}
#ifdef _WIN32
	int ret=0;
	u_long v;
	if (value) {
		v=0;
		ret=ioctlsocket(s->sd,FIONBIO,NULL);
	} else {
		v=1;
		ret=ioctlsocket(s->sd,FIONBIO,&v);
	}
	if (ret==0) return 1;
	return 0;
#else
	if (value)
		fcntl(s->sd,F_SETFL,fcntl(s->sd,F_GETFL,0)&(~O_NONBLOCK));    // Blocking
	else
		fcntl(s->sd,F_SETFL,fcntl(s->sd,F_GETFL,0)|O_NONBLOCK);   // NON-Blocking
	//if (ret==0) return 1;
	return 1;
#endif
}


/*!\brief Server starten und auf eingehende Verbindung warten
 *
 * \desc
 * Der Aufruf dieser Funktion führt dazu, dass auf eingehende TCP-Verbindungen gewartet wird.
 * Es wird also ein TCP-Server gestartet. Wird eine Verbindung geöffnet, ruft sie die virtuelle
 * Funktion CTCPSocket::ReceiveConnect auf. Diese muss entsprechend reagieren, indem Sie bei
 * Annahme der Verbindung einen neuen Thread startet und 1 zurückgibt, bei Ablehnung 0.
 * \par
 * Vor Aufruf der Funktion muss der Klasse über die Funktion CTCPSocket::Bind zuerst mitgeteilt
 * werden, auf welche IP-Adresse und Port sich der Server binden soll. Sofern kein kritischer
 * Fehler auftritt, läuft die Funktion in einer Endlosschleife. Alle \p timeout Millisekunden
 * wird jedoch geprüft, ob die Funktion sich beenden soll. Dies wird durch Aufruf der Funktion
 * CTCPSocket::SignalStopListen oder CTCPSocket::StopListen signalisiert.
 *
 * \param[in] timeout Intervall in Millisekunden, nachdem geprüpft werden soll, ob
 * weiter auf eingehende Verbindungen gewartet werden soll. Je niedriger der Wert, desto
 * schneller wird auf einen Stop-Befehl reagiert, jedoch steigt dadurch auch die
 * CPU-Auslastung. Der Default ist 100 Millisekunden (1000 Millisekunden = 1 Sekunde).
 *
 * @return Konnte der Server erfolgreich gestartet werden, kehrt die Funktion erst dann wieder zurück, wenn er wieder gestoppt wird.
 * In diesem Fall liefert sie den Wert 1 zurück. Konnte der Server nicht gestartet werden, wird 0 zurückgegeben.
 *
 */
int CTCPSocket::Listen(int timeout)
{
	#ifdef _WIN32
		struct sockaddr_in cliAddr;
	#else
		struct sockaddr cliAddr;
		fd_set rset;
		struct timeval tv;
	#endif
	PPLSOCKET *s=(PPLSOCKET*)socket;
	if((!s) || (!s->sd)) {
		SetError(267);
		return 0;
	}
	StopListen();
	mutex.Lock();
	stoplisten=false;
	islisten=true;
	mutex.Unlock();

	// Listen
	if (listen(s->sd,5)!=0) {
		mutex.Lock();
		islisten=false;
		mutex.Unlock();
		SetError(268);
		return 0;
	}
	SetBlocking(true);

	while (1) {
		//printf ("Listen...\n");
		mutex.Lock();
		if (stoplisten) {
			//printf ("stoplisten detected\n");
			mutex.Unlock();
			break;
		}
		mutex.Unlock();
		socklen_t cliLen=sizeof(cliAddr);
		#ifdef _WIN32
			SOCKET newSd;
			newSd = accept(s->sd, (struct sockaddr *) &cliAddr, &cliLen);
			if (newSd!=INVALID_SOCKET) {
				CTCPSocket *newsocket=new CTCPSocket();
				if (newsocket) newsocket->socket=malloc(sizeof(PPLSOCKET));
				if (newsocket==NULL || newsocket->socket==NULL) {
					if (newsocket) delete newsocket;
					if (log) log->Printf (LOG::ERROR,1,__FILE__,__LINE__,"Out of memory, could not handle connect from: %s:%d\n",inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port));
					SetError(2);
					continue;
				}
				newsocket->connected=1;
				PPLSOCKET *ns=(PPLSOCKET*)newsocket->socket;
				ns->proto=6;
				ns->sd=newSd;
				ns->ipname=strdup(inet_ntoa(cliAddr.sin_addr));
				ns->port=ntohs(cliAddr.sin_port);
				if (!ReceiveConnect(newsocket,ns->ipname,ns->port)) {
					delete newsocket;
				}
			} else {
				MSleep(100);
			}
		#else
			int newSd;
			// Prüfen, ob der Socket lesbar ist
			FD_ZERO(&rset);
			FD_SET(s->sd,&rset);
			tv.tv_sec=0;
			tv.tv_usec=timeout*1000; // Timeout für select setzen
			if (select(s->sd+1,&rset,NULL,NULL,&tv)<=0) continue;
			newSd = accept(s->sd, (struct sockaddr *) &cliAddr, &cliLen);
			if(newSd>0) {
#ifdef DEBUGOUT
				printf ("%010.6f Verbindung angenommen\n",ppl6::GetMicrotime());
#endif
				char hostname[1024];
				char servname[32];
				memset(hostname,0,1024);
				memset(servname,0,32);
				if (getnameinfo((const sockaddr*)&cliAddr,sizeof(cliAddr),
						hostname,1023, servname,31,NI_NUMERICHOST|NI_NUMERICSERV)!=0) {
#ifdef _WIN32
					closesocket(newSd);
#else
					close (newSd);
#endif
					continue;
				}
				CTCPSocket *newsocket=new CTCPSocket();
				if (newsocket) newsocket->socket=malloc(sizeof(PPLSOCKET));
				if (newsocket==NULL || newsocket->socket==NULL) {
					if (newsocket) delete newsocket;
					if (log) log->Printf (LOG::ERROR,1,__FILE__,__LINE__,"Out of memory, could not handle connect from: %s:%d\n",hostname,atoi(servname));
					SetError(2);
					continue;
				}
				newsocket->connected=1;
				PPLSOCKET *ns=(PPLSOCKET*)newsocket->socket;
				ns->proto=6;
				ns->sd=newSd;
				ns->ipname=strdup(hostname);
				ns->port=atoi(servname);
				if (log) log->Printf (LOG::DEBUG,3,__FILE__,__LINE__,"Connect from: %s:%d\n",ns->ipname, ns->port);

#ifdef DEBUGOUT
				printf ("%010.6f Dispatch an ReceiveConnect\n",ppl6::GetMicrotime());
#endif
				if (!ReceiveConnect(newsocket,ns->ipname,ns->port)) {
					if (log) log->Printf (LOG::DEBUG,3,__FILE__,__LINE__,"Connect from: %s:%d refused by CTCPSocket::ReceiveConnect\n",ns->ipname, ns->port);
					delete newsocket;
				}

#ifdef _WIN32
			} else {
				MSleep(100);
#endif
			}
		#endif
	}
	mutex.Lock();
	islisten=false;
	mutex.Unlock();
	return 1;
}

/*!\brief Prüfen, ob Daten auf den Socket geschrieben werden können
 *
 * \desc
 * Diese Funktion prüft, ob der bereits verbundene Socket beschreibbar ist.
 *
 * @return Liefert 1 zurück, wenn Daten auf den Socket geschrieben werden können,
 * ansonsten 0.
 */
int CTCPSocket::IsWriteable()
{
#ifndef _WIN32
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);
	FD_SET(s->sd,&wset);
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
	if (ret<0) {
		DispatchErrno();
		return 0;
	}
	if (FD_ISSET(s->sd,&eset)) {
		SetError(453);
		return 0;
	}

	if (FD_ISSET(s->sd,&wset)) {
		return 1;
	}
	/*
	// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
	if (FD_ISSET(s->sd,&rset)) {
		char buf[2];
		ret=recv(s->sd, &buf,1, MSG_PEEK|MSG_DONTWAIT);
		// Kommt hier ein Fehler zurück?
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
		if (ret==0) {
			SetError(308);	// EPIPE
			return 0;
		}
	}
	*/
	SetError(451);
	return 0;

#else
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);
	FD_SET(s->sd,&wset);
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
	if (ret<0) {
		DispatchErrno();
		return 0;
	}
	if (FD_ISSET(s->sd,&eset)) {
		SetError(453);
		return 0;
	}

	if (FD_ISSET(s->sd,&wset)) {
		return 1;
	}
	/*
	// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
	if (FD_ISSET(s->sd,&rset)) {
		char buf[2];
		ret=recv(s->sd, &buf,1, MSG_PEEK|MSG_DONTWAIT);
		// Kommt hier ein Fehler zurück?
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
		if (ret==0) {
			SetError(308);	// EPIPE
			return 0;
		}
	}
	*/
	SetError(451);
	return 0;
#endif
}

/*!\brief Prüfen, ob Daten vom Socket gelesen werden können
 *
 * \desc
 * Diese Funktion prüft, ob auf dem bereits geöffneten Socket Daten zum
 * Lesen vorhanden sind.
 *
 * @return Liefert 1 zurück, wenn Daten auf dem Socket zum Lesen
 * bereitstehen.
 */
int CTCPSocket::IsReadable()
{
#ifndef _WIN32
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);
	FD_SET(s->sd,&rset);
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
	if (ret<0) {
		DispatchErrno();
		return 0;
	}
	if (FD_ISSET(s->sd,&eset)) {
		SetError(453);
		return 0;
	}
	// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
	if (FD_ISSET(s->sd,&rset)) {
		char buf[2];
		ret=recv(s->sd, &buf,1, MSG_PEEK|MSG_DONTWAIT);
		// Kommt hier ein Fehler zurück?
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
		if (ret==0) {
			SetError(308);	// EPIPE
			return 0;
		}
	}
	return 1;

#else
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);
	FD_SET(s->sd,&rset);
	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=0;
	int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
	if (ret<0) {
		DispatchErrno();
		return 0;
	}
	if (FD_ISSET(s->sd,&eset)) {
		SetError(453);
		return 0;
	}
	// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
	if (FD_ISSET(s->sd,&rset)) {
		char buf[2];
		ret=recv(s->sd, buf,1, MSG_PEEK);
		// Kommt hier ein Fehler zurück?
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
		if (ret==0) {
			SetError(308);	// EPIPE
			return 0;
		}
	}
	return 1;
#endif

}


/*!\brief Auf eingehende Daten warten
 *
 * \desc
 * Diese Funktion prüft, ob Daten eingegangen sind. Ist dies der Fall,
 * kehrt sie sofort wieder zurück. Andernfalls wartet sie solange, bis
 * Daten eingehen, maximal aber die mit \p seconds und \p useconds
 * angegebene Zeitspanne. Falls \p seconds und \p useconds Null sind, und
 * keine Daten bereitstehen, kehrt die Funktion sofort zurück.
 * \par
 * Falls über die Funktion CTCPSocket::WatchThread ein Thread angegeben
 * wurde, kehrt die Funktion nicht nach erreichen des Timeouts zurück,
 * sondern erst wenn dem Thread ein Stopsignal gegeben wird, bzw. wenn Daten
 * bereitstehen.
 *
 * @param[in] seconds Anzahl Sekunden, die gewartet werden soll
 * @param[in] useconds Anzahl Mikrosekunden, die gewartet werden soll
 * @return Die Funktion gibt 1 zurück, wenn Daten zum Lesen bereitstehen,
 * sonst 0. Der Fehlercode 174 zeigt an, dass ein Timeout aufgetreten ist,
 * und man die Funktion nochmal aufrufen kann. Alle anderen Fehlercodes
 * weisen auf einen richtigen Fehler hin, meist einen Verbindungsabbruch.
 *
 */
int CTCPSocket::WaitForIncomingData(int seconds, int useconds)
{
#ifndef _WIN32
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	struct timeval timeout;
	while (1) {
		timeout.tv_sec=seconds;
		timeout.tv_usec=useconds;

		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);
		FD_SET(s->sd,&rset);	// Wir wollen nur prüfen, ob was zu lesen da ist
		int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		if (FD_ISSET(s->sd,&eset)) {
			SetError(453);
			return 0;
		}
		// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
		if (FD_ISSET(s->sd,&rset)) {
			char buf[2];
			ret=recv(s->sd, &buf,1, MSG_PEEK|MSG_DONTWAIT);
			// Kommt hier ein Fehler zurück?
			if (ret<0) {
				DispatchErrno();
				return 0;
			}
			// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
			if (ret==0) {
				SetError(308);	// EPIPE
				return 0;
			}
			return 1;
		}
		if (thread) {
			if (thread->ThreadShouldStop()) {
				SetError(336);
				return 0;
			}
		} else {
			SetError(174);	// Timeout
			return 0;
		}
	}
	return 0;
#else
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	struct timeval timeout;
	while (1) {
		timeout.tv_sec=seconds;
		timeout.tv_usec=useconds;

		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);
		FD_SET(s->sd,&rset);	// Wir wollen nur prüfen, ob was zu lesen da ist
		int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		if (FD_ISSET(s->sd,&eset)) {
			SetError(453);
			return 0;
		}
		// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
		if (FD_ISSET(s->sd,&rset)) {
			char buf[2];
			ret=recv(s->sd, buf,1, MSG_PEEK);
			// Kommt hier ein Fehler zurück?
			if (ret<0) {
				DispatchErrno();
				return 0;
			}
			// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
			if (ret==0) {
				SetError(308);	// EPIPE
				return 0;
			}
			return 1;
		}
		if (thread) {
			if (thread->ThreadShouldStop()) {
				SetError(336);
				return 0;
			}
		} else {
			SetError(174);	// Timeout
			return 0;
		}
	}
	return 0;
#endif
}

/*!\brief Warten, bis der Socket beschreibbar ist
 *
 * \desc
 * Diese Funktion prüft, ob Daten auf den Socket geschrieben werden können.
 * Ist dies der Fall, kehrt sie sofort wieder zurück. Andernfalls wartet
 * sie solange, bis der Socket beschrieben werden kann, maximal aber die
 * mit \p seconds und \p useconds angegebene Zeitspanne.
 * Falls \p seconds und \p useconds Null sind, und
 * keine Daten gesendet werden können, kehrt die Funktion sofort zurück.
 * \par
 * Falls über die Funktion CTCPSocket::WatchThread ein Thread angegeben
 * wurde, kehrt die Funktion nicht nach erreichen des Timeouts zurück,
 * sondern erst wenn dem Thread ein Stopsignal gegeben wird, bzw. wenn
 * der Socket beschrieben werden kann.
 *
 * @param[in] seconds Anzahl Sekunden, die gewartet werden soll
 * @param[in] useconds Anzahl Mikrosekunden, die gewartet werden soll
 * @return Die Funktion gibt 1 zurück, wenn Daten geschrieben werden können,
 * sonst 0. Der Fehlercode 174 zeigt an, dass ein Timeout aufgetreten ist,
 * und man die Funktion nochmal aufrufen kann. Alle anderen Fehlercodes
 * weisen auf einen richtigen Fehler hin, meist einen Verbindungsabbruch.
 *
 */
int CTCPSocket::WaitForOutgoingData(int seconds, int useconds)
{
#ifndef _WIN32
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	struct timeval timeout;
	while (1) {
		timeout.tv_sec=seconds;
		timeout.tv_usec=useconds;

		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);
		FD_SET(s->sd,&wset);	// Wir wollen nur prüfen, ob wir schreiben können
		int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		if (FD_ISSET(s->sd,&eset)) {
			SetError(453);
			return 0;
		}
		if (FD_ISSET(s->sd,&wset)) {
			return 1;
		}
		/*
		// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
		if (FD_ISSET(s->sd,&rset)) {
			char buf[2];
			ret=recv(s->sd, &buf,1, MSG_PEEK|MSG_DONTWAIT);
			// Kommt hier ein Fehler zurück?
			if (ret<0) {
				DispatchErrno();
				return 0;
			}
			// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
			if (ret==0) {
				SetError(308);	// EPIPE
				return 0;
			}
		}
		*/
		if (thread) {
			if (thread->ThreadShouldStop()) {
				SetError(336);
				return 0;
			}
		} else {
			SetError(174);	// Timeout
			return 0;
		}
	}
	return 0;
#else
	PPLSOCKET *s=(PPLSOCKET*)socket;
    if (!connected) {
        SetError(275);
        return 0;
    }
	fd_set rset, wset, eset;
	struct timeval timeout;
	while (1) {
		timeout.tv_sec=seconds;
		timeout.tv_usec=useconds;

		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);
		FD_SET(s->sd,&wset);	// Wir wollen nur prüfen, ob wir schreiben können
		int ret=select(s->sd+1,&rset,&wset,&eset,&timeout);
		if (ret<0) {
			DispatchErrno();
			return 0;
		}
		if (FD_ISSET(s->sd,&eset)) {
			SetError(453);
			return 0;
		}
		if (FD_ISSET(s->sd,&wset)) {
			return 1;
		}
		/*
		// Falls Daten zum Lesen bereitstehen, könnte dies auch eine Verbindungstrennung anzeigen
		if (FD_ISSET(s->sd,&rset)) {
			char buf[2];
			ret=recv(s->sd, &buf,1, MSG_PEEK|MSG_DONTWAIT);
			// Kommt hier ein Fehler zurück?
			if (ret<0) {
				DispatchErrno();
				return 0;
			}
			// Ein Wert von 0 zeigt an, dass die Verbindung getrennt wurde
			if (ret==0) {
				SetError(308);	// EPIPE
				return 0;
			}
		}
		*/
		if (thread) {
			if (thread->ThreadShouldStop()) {
				SetError(336);
				return 0;
			}
		} else {
			SetError(174);	// Timeout
			return 0;
		}
	}
	return 0;
#endif
}

/*!\brief Eingehende Verbindung verarbeiten
 *
 * \desc
 * Diese virtuelle Funktion wird innerhalb von CTCPSocket::Listen aufgerufen,
 * wenn eine neue Verbindung zum Server aufgebaut wurde. Sie muss daher von
 * der abgeleiteten Klasse reimplementiert werden.
 *
 * @param[in] socket Pointer auf eine neue Socket-Klasse, die die eingegangene
 * Verbindung repräsentiert. Die Anwendung muss die Klasse nach Verbindungsende
 * selbst mit "delete" löschen.
 * @param[in] host Hostname oder IP der Gegenstelle
 * @param[in] port TCP-Port der Gegenstelle
 * @return Wird die Verbindung angenommen und von der Anwendung weiter
 * verarbeitet (z.B. in einem eigenen Thread), muss die Funktion 1
 * zurückgeben. Soll die Verbindung wieder getrennt werden, muss die
 * Funktion 0 zurückgeben.
 */
int CTCPSocket::ReceiveConnect(CTCPSocket *socket, const char *host, int port)
{
	return 0;
}

/*!\brief 32-Bit-Wert von Host-Byteorder in Hostbyteorder wandeln
 *
 * \desc
 * Mit dieser statischen Funktion kann ein 32-Bit-Wert von Netzwerk-Byte-Order
 * (Big Endian) in die Systemspezifische Byte-Reihenfolge umgewandelt werden.
 *
 * @param[in] net 32-Bit-Wert in Netzwerk-Byteorder (Big Endian)
 * @return Liefert den Wert \p net in System-spezifischer Byte-Reihenfolge
 * zurück
 */
ppluint32 CTCPSocket::Ntohl(ppluint32 net)
{
	return ntohl(net);
}

/*!\brief 32-Bit-Wert von Host-Byteorder in Netzwerk-Byteorder wandeln
 *
 * \desc
 * Mit dieser statischen Funktion kann ein 32-Bit-Wert von der
 * Systemspezifische Byte-Reihenfolge in Netzwerk-Byte-Order
 * (Big Endian) umgewandelt werden.
 *
 * @param[in] host 32-Bit-Wert in Systemspezifischer Byte-Reihenfolge
 * @return Liefert den Wert \p host in Netzwerk-Byteorder (Big Endian)
 * zurück
 */
ppluint32 CTCPSocket::Htonl(ppluint32 host)
{
	return htonl(host);
}

/*!\brief 16-Bit-Wert von Netzwerk-Byteorder in Host-Byteorder wandeln
 *
 * \desc
 * Mit dieser statischen Funktion kann ein 16-Bit-Wert von Netzwerk-Byte-Order
 * (Big Endian) in die Systemspezifische Byte-Reihenfolge umgewandelt werden.
 *
 * @param[in] net 16-Bit-Wert in Netzwerk-Byteorder (Big Endian)
 * @return Liefert den Wert \p net in System-spezifischer Byte-Reihenfolge
 * zurück
 */
ppluint16 CTCPSocket::Ntohs(ppluint16 net)
{
	return ntohs(net);
}

/*!\brief 16-Bit-Wert von Host-Byteorder in Netzwerk-Byteorder wandeln
 *
 * \desc
 * Mit dieser statischen Funktion kann ein 16-Bit-Wert von der
 * Systemspezifische Byte-Reihenfolge in Netzwerk-Byte-Order
 * (Big Endian) umgewandelt werden.
 *
 * @param[in] host 16-Bit-Wert in Systemspezifischer Byte-Reihenfolge
 * @return Liefert den Wert \p host in Netzwerk-Byteorder (Big Endian)
 * zurück
 */
ppluint16 CTCPSocket::Htons(ppluint16 host)
{
	return htons(host);
}


} // EOF namespace ppl
