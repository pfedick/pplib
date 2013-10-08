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

#ifndef PPL7INET_H_
#define PPL7INET_H_

#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif

#include <list>

namespace ppl7 {

PPLPARAMETERISEDEXCEPTION(NetworkException);
PPLNORMALEXCEPTION(WinsockInitialisationFailed);
PPLPARAMETERISEDEXCEPTION(IdnConversionException);
PPLPARAMETERISEDEXCEPTION(QueryFailedException);

// TODO
//int GetHostByName(const char *name, CAssocArray *Result);
//int GetHostByAddr(const char *addr, CAssocArray *Result);
String GetHostname();

enum ResolverFlags {
	af_unspec=0,
	af_inet=1,
	af_inet6=2,
	af_all=3
};

class IPAddress
{
	public:
		IPAddress();
		IPAddress(const IPAddress &other);
		~IPAddress();
		IPAddress &operator=(const IPAddress &other);
		void copyAddr(void *ai_addr, size_t ai_addrlen);
		String		name;
		String		ip;
		void		*ai_addr;
		size_t		ai_addrlen;
        int			ai_family;
        int			ai_socktype;
        int			ai_protocol;
        String		ai_canonname;

		bool operator<(const IPAddress &other) const;
		bool operator<=(const IPAddress &other) const;
		bool operator==(const IPAddress &other) const;
		bool operator!=(const IPAddress &other) const;
		bool operator>=(const IPAddress &other) const;
		bool operator>(const IPAddress &other) const;

};

void InitSockets();
size_t GetHostByName(const String &name, std::list<IPAddress> &result,ResolverFlags flags=af_unspec);
size_t GetHostByAddr(const String &addr, std::list<IPAddress> &result);


class Resolver
{
        private:

        public:
                // Quelle: http://en.wikipedia.org/wiki/List_of_DNS_record_types
                enum Type {
                        A=1,
                        NS=2,
                        CNAME=5,
                        MX=15,
                        SOA=6,
                        PTR=12,
                        TXT=16,
                        AAAA=28,
                        NAPTR=35,
                        SRV=33,
                        DS=43,
                        DNSKEY=48,
                        NSEC=47,
                        NSEC3=50,
                        RRSIG=46,
                        OPT=41,
                        TSIG=250
                };

                enum Class {
                        /** the Internet */
                        CLASS_IN    = 1,
                        /** Chaos class */
                        CLASS_CH    = 3,
                        /** Hesiod (Dyer 87) */
                        CLASS_HS    = 4,
                        /** None class, dynamic update */
                        CLASS_NONE      = 254,
                        /** Any class */
                        CLASS_ANY   = 255,
                        CLASS_FIRST     = 0,
                        CLASS_LAST      = 65535,
                        CLASS_COUNT     = CLASS_LAST - CLASS_FIRST + 1
                };

                static String typeName(Type t);
                static String className(Class c);
                static void query(Array &r, const String &label, Type t=A, Class c=CLASS_IN);
};




String Idn2Ace(const String &idn);
WideString Idn2Ace(const WideString &idn);
String Ace2Idn(const String &ace);
WideString Ace2Idn(const WideString &ace);

String ToQuotedPrintable (const String &source);


class TCPSocket;

void SSL_Init();
void SSL_Exit();

//! \brief SSL-Verschlüsselung
class SSLContext
{
	friend class CTCPSocket;
	private:
		Mutex		mutex;
		void		*ctx;
		int			references;
		void 		clear();
	public:
		enum SSL_METHOD {
			SSLv2	= 1,
			SSLv2client,
			SSLv2server,
			SSLv3,
			SSLv3client,
			SSLv3server,
			TLSv1,
			TLSv1client,
			TLSv1server,
			SSLv23,				// Unterstuetzt SSLv2, v3 und TLSv1
			SSLv23client,		// Unterstuetzt SSLv2, v3 und TLSv1
			SSLv23server		// Unterstuetzt SSLv2, v3 und TLSv1
		};

		SSLContext();
		SSLContext(int method);
		~SSLContext();
		void 	init(int method=SSLv3);
		bool	isInit();
		void 	shutdown();
		void 	*newSSL();
		void	releaseSSL(void *ssl);
		void	loadTrustedCAfromFile(const String &filename);
		void	loadTrustedCAfromPath(const String &path);
		void	loadCertificate(const String &certificate, const String &privatekey=String(), const String &password=String());
		void	setCipherList(const String &cipherlist);		// "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
};

class SSLError
{
	public:
		SSLError() {
			Code=0;
			Line=Flags=0;
		}
		String			Text;
		String			Filename;
		String			Data;
		unsigned long	Code;
		int				Line;
		int				Flags;
};

int GetSSLError(SSLError &e);
int GetSSLErrors(std::list<SSLError> &e);
void ClearSSLErrorStack();


//! \brief TCP-Socket-Nachrichten
class SocketMessage
{
	friend class TCPSocket;
	private:
		int			datatype;
		void		*data;
		void		*incoming_data;
		int			size;
		int			incoming_size;
		int			incoming_type;
		int			commandId;
		int			Id;
		int			Version;
		bool		UseCompression;
		bool		SupportMsgChannel;

	public:
		bool		ClientSupportsCompression;

		SocketMessage();
		SocketMessage(const SocketMessage &other);

		virtual ~SocketMessage();
		void Copy(const SocketMessage &other);
		void Copy(const SocketMessage *other);

		void Clear();
		void SetCommandId(int id);
		void SetId(int id);
		int SetData(const String &msg);
		int SetData(const char *msg);
		int SetData(const AssocArray &msg);
		int GetData(String &msg);
		const char*GetData();
		int GetData(AssocArray &msg);
		int GetId();
		int GetCommandId();
		int GetType();
		void Dump(String &buffer);
		void Dump();
		void Dump(Logger *Log, int facility=Logger::DEBUG, int level=1);
		int SetVersion(int version);
		void EnableCompression();
		void DisableCompression();
		void EnableMsgChannel();
		void DisableMsgChannel();
		bool isCompressionSupported() const;
		bool isMsgChannelSupported() const;
};

//! \brief TCP-Socket-Klasse
class TCPSocket
{
	private:
		Mutex	mutex;
		SSLContext		*sslclass;
		Thread	*thread;
		void		*sslreference;
		void *socket;
		void *ssl;
		int connected;
		int islisten;
		int		BytesWritten;
		int		BytesRead;
		int		connect_timeout_sec;
		int		connect_timeout_usec;
        bool    stoplisten;
        String	HostName;
        int			PortNum;
        String		SourceInterface;
        int			SourcePort;

        int		SSL_Write(const void *buffer, int size);
        int		SSL_Read(void *buffer, int bytes);

	public:
		TCPSocket();
		virtual ~TCPSocket();
		//! @name TCP-Client functions
		//@{
		void setSource(const String &interface, int port=0);
		int connect(const String &host_and_port);
		int connect(const String &host, int port);
		//@}

		//! @name TCP-Server functions
		//@{
		int Bind(const char *ip, int port);
		virtual int ReceiveConnect(TCPSocket *socket, const char *host, int port);
		int IsListening();
        int StopListen();
		int SignalStopListen();
        int Listen(int timeout=100);

		//@}

		//! @name Common functions for client and server
		//@{
        int Disconnect();
		int Write(const String &str);
		int Write(const ByteArrayPtr &bin);
		int Write(const String *str);
		int Write(const void *buffer, int bytes);
		int WriteBuffer(const void *buffer, int bytes);
		int Writef(const char *fmt, ...);
		int Write(SocketMessage &msg);
		int Read(void *buffer, int bytes);
		int Read(String &buffer, int bytes);
		int Read(ByteArray &buffer, int bytes);
		char *Read(int bytes);
		int ReadOnce(void *buffer, int bytes);
		int ReadOnce(String &buffer, int bytes);
		char *ReadOnce(int bytes);

		int getDescriptor();
		int SetBlocking(bool value);
		int IsWriteable();
		int IsReadable();
		int WaitForIncomingData(int seconds, int useconds);
		int WaitForOutgoingData(int seconds, int useconds);
		//@}

		//! @name SSL Encryption
		//@{

		int SSL_Init(SSLContext *ssl);
		int SSL_Shutdown();
		int SSL_Init_Client();
		int SSL_Init_Server();
		int SSL_Start();
		int SSL_Stop();
		int SSL_CheckCertificate(const char *hostname, bool AcceptSelfSignedCert=false);
		int SSL_Accept();
		int SSL_WaitForAccept(int timeout=0);

		//@}


		//! @name TODO
		//@{

		void SetConnectTimeout(int seconds, int useconds);
		int SetReadTimeout(int seconds, int useconds);
		int SetWriteTimeout(int seconds, int useconds);


		void DispatchErrno();
		int GetBytesWritten();
		int GetBytesRead();





		int IsConnected();
        int Shutdown();
        int	WaitForMessage(SocketMessage &msg, int timeout=0);
        int WatchThread(Thread *thread);


		static ppluint32 Ntohl(ppluint32 net);
		static ppluint32 Htonl(ppluint32 host);
		static ppluint16 Ntohs(ppluint16 net);
		static ppluint16 Htons(ppluint16 host);

		//@}

		//! @name Depreceated
		//@{
		void SetLogfile(Logger *log);
		int ConnectSSL(const char *host_and_port, SSLContext *ssl=NULL);
		int ConnectSSL(const char *host, int port, SSLContext *ssl=NULL);
		//@}
};

class CUDPSocket
{
	private:
		int		timeout_sec;
		int		timeout_usec;
		void	*socket;

	public:
		CUDPSocket();
		~CUDPSocket();
		void SetTimeout(int seconds, int useconds);
		int SendTo(const char *host, int port, const void *buffer, int bytes);
		int SendTo(const char *host, int port, const String &buffer);
		int RecvFrom(void *buffer, int maxlen);
		int RecvFrom(void *buffer, int maxlen, String &host, int *port);
		int RecvFrom(String &buffer, int maxlen);
		int SetReadTimeout(int seconds, int useconds);
		int Bind(const char *host, int port);
		int GetDescriptor();

};

class Webserver
{
	private:
		SSLContext	SSL;
		void		*daemon;
		int			port;
		AssocArray	res;
		TCPSocket	Socket;
		bool		basicAuthentication;
		bool		SSLEnabled;
		String		realm;
		String		sslkey, sslcert;

	public:
		class Request {
			friend class Webserver;
			private:

			public:
				void *connection;
				void *postprocessor;
				String url;
				String method;
				String version;
				AssocArray header;
				AssocArray data;
				AssocArray auth;
		};
		Webserver();
		virtual ~Webserver();
		void bind(const String &adr, int port);
		void loadCertificate(const String &certificate, const String &privatekey, const String &password);
		void enableSSL(bool enable);
		bool sslEnabled() const;
		void start();
		void stop();
		void requireBasicAuthentication(bool enable, const String &realm);
		bool useBasicAuthentication() const;
		int queueResponse(const Request &req, const String &text, int httpStatus=200);
		int queueBasicAuthFailedResponse(const Request &req);

		virtual int request(Request &req);
		virtual int authenticate(const String &username, const String &password, Request &req);
		virtual String getDenyMessage();




		PPLPARAMETERISEDEXCEPTION(CouldNotBindToSocket);
		PPLPARAMETERISEDEXCEPTION(InvalidSSLCertificate);
		PPLNORMALEXCEPTION(NoAddressSpecified);
		PPLNORMALEXCEPTION(CouldNotStartDaemon);
		PPLNORMALEXCEPTION(SSLInitializationFailed);

};




class WikiParser
{
	private:
		int ispre;
		int ullevel;
		int ollevel;
		int indexcount;
		int intable;
		int inrow;
		int indentlevel;
		bool doxyparamsStarted;
		bool indexenabled;

		AssocArray index;
		String incol;
		String ret;
		String BaseURI;
		Array nowiki;
		int nowikicount;
		bool nobr;

		int precount;
		AssocArray pre;
		int sourcecount;
		Array source;

		Array diagrams;


		void init();
		int renderInternal(const String &Source, String &Html);
		void extractNoWiki(String &Text);
		void extractSourcecode(String &Text);
		void extractDiagrams(String &Text);
		void parseHeadlines(String &Line);
		int parseUL(String &Line);
		int parseOL(String &Line);
		int parseIndent(String &Line);

		void parseDoxygen(String &Line);
		void doxygenChapter(String &Line, const String &Name, const Array &Matches);
		void parseAutoPRE(String &Line);
		void parseTable(String &Line);
		void parseLinks(String &Line);
		void buildIndex(String &Html);
		void finalize();
		void finalizeNoWiki();
		void finalizePRE();
		void finalizeSource();
		void finalizeDiagrams();

	protected:
		virtual void customParseLinks(String &Line);

	public:

		WikiParser();
		virtual ~WikiParser();
		int render(const String &Source, String &Html);
		int renderBody(const String &Source, String &Html);
		String render(const String &Source);
		String renderBody(const String &Source);
		String header();
		void setIndexEnabled(bool enabled);
		void setBaseURI(const String &Uri);
		static String xmlDiagram2HTML(const String &xml);
		virtual int getHeader(String &Html);
};


} // EOF namespace ppl7

#endif /* PPL7INET_H_ */
