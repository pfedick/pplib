/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_LIBMICROHTTPD
#include <microhttpd.h>
#endif

#include "ppl6.h"



namespace ppl6 {

#ifdef HAVE_LIBMICROHTTPD
static int KeyValueIterator (void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	ppl6::CAssocArray *a=(ppl6::CAssocArray *)cls;
	a->Set(key,value);
	return MHD_YES;
}


int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{
	Webserver::Request * r = (Webserver::Request*)*con_cls;
	//printf ("answer_to_connection, url=%s, method=%s, versimn=%s\n",url,method,version);
	//HexDump((void*)upload_data, *upload_data_size);
	if (!r) {
		//printf ("answer_to_connection, new Request\n");
		r=new Webserver::Request;
		r->connection=connection;
		r->url=url;
		r->method=method;
		r->version=version;
		*con_cls=r;
		return MHD_YES;

	} else {
		//printf ("answer_to_connection, bestehender Request\n");
	}
	Webserver *w=(Webserver*) cls;
	if (w->useBasicAuthentication()) {
		char *pass=NULL;
		char *user = MHD_basic_auth_get_username_password (connection, &pass);
		int auth=0;
		if (user!=NULL) {
			auth=w->authenticate(user,pass, *r);
			free(user);
			if (pass) free(pass);
		}
		if (!auth) {
			return w->queueBasicAuthFailedResponse(*r);
		}

	}


	MHD_get_connection_values (connection,MHD_GET_ARGUMENT_KIND,KeyValueIterator,&r->data);
	MHD_get_connection_values (connection,MHD_HEADER_KIND,KeyValueIterator,&r->header);



	//r->data.List("data");



	//printf ("Upload Size: %i\n",(int)*upload_data_size);
	//ppl6::HexDump((void*)upload_data,*upload_data_size);


	w->request(*r);
	delete (r);
	CleanupThreadData();

	return MHD_YES;
}

#endif

Webserver::Webserver()
{
	daemon=NULL;
	port=80;
	basicAuthentication=false;
	SSLEnabled=false;
}

Webserver::~Webserver()
{
#ifdef HAVE_LIBMICROHTTPD
	if (daemon) MHD_stop_daemon((struct MHD_Daemon*)daemon);
#endif
}

void Webserver::bind(const ppl6::CString &adr, int port)
{
#ifdef HAVE_LIBMICROHTTPD
	if (!Socket.Bind(adr,port)) throw CouldNotBindToSocket(Error2String());
#else
	throw UnsupportedFeatureException("libmicrohttpd");
#endif
}

const CString & Webserver::getHostname() const
{
	return Socket.getHostname();
}

int Webserver::getPort() const
{
	return Socket.getPort();
}


void Webserver::loadCertificate(const CString &certificate, const CString &privatekey, const CString &password)
{
	CString e;
	/*
	if (!SSL.LoadCertificate(certificate,privatekey,password)) {
		Error2String(e);
		throw InvalidSSLCertificate(e);
	}
	*/
	if (!CFile::LoadFile(sslkey,privatekey)) {
		Error2String(e);
		throw InvalidSSLCertificate(e);
	}
	if (!CFile::LoadFile(sslcert,certificate)) {
		Error2String(e);
		throw InvalidSSLCertificate(e);
	}

}

void Webserver::enableSSL(bool enable)
{
	/*
	if (enable) {
		if (!SSL.Init(ppl6::CSSL::SSLv3)) throw SSLInitializationFailed();
	}
	*/
	SSLEnabled=enable;
}

bool Webserver::sslEnabled() const
{
	return SSLEnabled;
}

void Webserver::start()
{
#ifdef HAVE_LIBMICROHTTPD
	int sd=Socket.GetDescriptor();
	if (!sd) throw NoAddressSpecified();
	if (::listen(sd,5)!=0) {
		int e=TranslateErrno(errno);
		throw CouldNotStartDaemon(ppl6::ToString("listen => %i: %s [%s:%i]",e,GetError(e),(const char*)Socket.getHostname(),Socket.getPort()));
	}

	if (!SSLEnabled) {
		daemon=MHD_start_daemon (
				MHD_USE_THREAD_PER_CONNECTION, 8090, NULL,NULL, answer_to_connection,this,
				MHD_OPTION_LISTEN_SOCKET,
				sd,
				MHD_OPTION_END
		);
	} else {
		daemon=MHD_start_daemon (
				MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL,
				8090, NULL,NULL, answer_to_connection,this,
				MHD_OPTION_LISTEN_SOCKET,
				sd,
				MHD_OPTION_HTTPS_MEM_KEY,
				(const char*)sslkey,
				MHD_OPTION_HTTPS_MEM_CERT,
				(const char*)sslcert,
				MHD_OPTION_END
		);

	}
	if (!daemon) throw CouldNotStartDaemon("MHD_start_daemon failed [%s:%i]",(const char*)Socket.getHostname(),Socket.getPort());
#else
	throw UnsupportedFeatureException("libmicrohttpd");
#endif
}

void Webserver::stop()
{
#ifdef HAVE_LIBMICROHTTPD
	if (daemon) MHD_stop_daemon((struct MHD_Daemon*)daemon);
	daemon=NULL;
#else
	throw UnsupportedFeatureException("libmicrohttpd");
#endif

}


int Webserver::queueResponse(const Request &req, const ppl6::CString &text, int httpStatus)
{
#ifdef HAVE_LIBMICROHTTPD
	struct MHD_Response *response;
	int ret;
	response = MHD_create_response_from_buffer (text.Size(),
			(void *) text.GetPtr(), MHD_RESPMEM_MUST_COPY);
	MHD_add_response_header (response, "Content-Type", "text/html");

	ret = MHD_queue_response ((struct MHD_Connection *)req.connection, httpStatus, response);
	MHD_destroy_response (response);
	return ret;
#else
	throw UnsupportedFeatureException("libmicrohttpd");
#endif

}

int Webserver::queueBasicAuthFailedResponse(const Request &req)
{
#ifdef HAVE_LIBMICROHTTPD
	struct MHD_Response *response;
	int ret;
	CString msg=getDenyMessage();
	response = MHD_create_response_from_buffer (msg.Size(),
			(void *) msg.GetPtr(), MHD_RESPMEM_MUST_COPY);
	ret = MHD_queue_basic_auth_fail_response ((struct MHD_Connection *)req.connection, (const char*)realm, response);
	MHD_destroy_response (response);
	return ret;
#else
	throw UnsupportedFeatureException("libmicrohttpd");
#endif

}

int Webserver::request(Request &req)
{
	ppl6::CString Answer,text;
	ppl6::CWikiParser wiki;
	text="Hallo Browser\n== Connection ==\n* URL: "+req.url+"\n";
	text+="* Method: "+req.method+"\n";
	text+="* Version: "+req.version+"\n";


	ppl6::CString Key,Value;
	text+="== Header ==\n";
	req.header.Reset();
	while (req.header.GetNext(Key,Value)) {
		text+="* "+Key+": "+Value+"\n";
	}
	text+="== Data ==\n";
	req.data.Reset();
	while (req.data.GetNext(Key,Value)) {
		text+="* "+Key+": "+Value+"\n";
	}
	Answer=wiki.render(text);
	return queueResponse(req,Answer);
}

void Webserver::requireBasicAuthentication(bool enable, const CString &realm)
{
	basicAuthentication=enable;
	this->realm=realm;
}

bool Webserver::useBasicAuthentication() const
{
	return basicAuthentication;
}


/*!\brief Callback: Benutzer authentisieren
 *
 * \desc
 * Diese Funktion wird bei jedem Seitenaufruf aufgerufen, wenn Passwort-Authentisierung aktiviert wurde (siehe
 * Webserver::requireBasicAuthentication). Der Benutzername und das Passwort werden als Parameter übergeben,
 * der Returnwert der Funktion zeigt an, ob der Zugriff gewährt wird oder nicht.
 *
 * @param username String mit dem Benutzernamen
 * @param password String mit dem Passwort des Benutzers im Klartext
 *
 * @return Die Funktion muss 1 zurückgeben, wenn der Zugriff gewährt wird, andernfalls 0.
 */
int Webserver::authenticate(const CString &username, const CString &password, Request &req)
{
	return 0;
}


CString Webserver::getDenyMessage()
{
	CString text;
	text="<html><body>\n<h1>Permission Denied</h1>\n";
	text+="</body></html>\n";
	return text;
}

} // EOF namespace ppl6
