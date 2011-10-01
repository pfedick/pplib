/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.5 $
 * $Date: 2010/09/25 07:58:08 $
 * $Id: CTCPSocket.cpp,v 1.5 2010/09/25 07:58:08 pafe Exp $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/core/AVLTree.cpp $
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
	if (!r) {
		r=new Webserver::Request;
		r->connection=connection;
		r->url=url;
		r->method=method;
		r->version=version;

		MHD_get_connection_values (connection,MHD_HEADER_KIND,KeyValueIterator,&r->header);


	}
	Webserver *w=(Webserver*) cls;
	MHD_get_connection_values (connection,MHD_GET_ARGUMENT_KIND,KeyValueIterator,&r->data);



	//printf ("Upload Size: %i\n",(int)*upload_data_size);
	//ppl6::HexDump((void*)upload_data,*upload_data_size);


	return w->request(*r);
	//return MHD_YES;
}

#endif

Webserver::Webserver()
{
	daemon=NULL;
	port=80;


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

void Webserver::start()
{
#ifdef HAVE_LIBMICROHTTPD
	int sd=Socket.GetDescriptor();
	if (!sd) throw NoAddressSpecified();
	::listen(sd,5);
	daemon=MHD_start_daemon (
		MHD_USE_THREAD_PER_CONNECTION, 8090, NULL,NULL, answer_to_connection,this,
		MHD_OPTION_LISTEN_SOCKET,
		sd,
		MHD_OPTION_END
		);

	if (!daemon) throw CouldNotStartDaemon();
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


int Webserver::queueResponse(const Request &req, ppl6::CString &text)
{
#ifdef HAVE_LIBMICROHTTPD
	struct MHD_Response *response;
	int ret;
	response = MHD_create_response_from_buffer (text.Size(),
			(void *) text.GetPtr(), MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response ((struct MHD_Connection *)req.connection, MHD_HTTP_OK, response);
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


} // EOF namespace ppl6
