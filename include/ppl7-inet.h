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

PPLEXCEPTION(NetworkException,Exception);
PPLEXCEPTION(WinsockInitialisationFailed, Exception);
PPLEXCEPTION(IdnConversionException, Exception);
PPLEXCEPTION(QueryFailedException, Exception);


PPLEXCEPTION(NotConnectedException,NetworkException);
PPLEXCEPTION(CouldNotOpenSocketException,NetworkException);				// 393
PPLEXCEPTION(IllegalPortException,NetworkException);				// 393
PPLEXCEPTION(CouldNotBindToSourceInterfaceException,NetworkException);	// 394
PPLEXCEPTION(ConnectionRefusedException,NetworkException);
PPLEXCEPTION(NetworkDownException,NetworkException);
PPLEXCEPTION(NetworkUnreachableException,NetworkException);
PPLEXCEPTION(NetworkDroppedConnectionOnResetException,NetworkException);
PPLEXCEPTION(SoftwareCausedConnectionAbortException,NetworkException);
PPLEXCEPTION(ConnectionResetByPeerException,NetworkException);
PPLEXCEPTION(NoBufferSpaceException,NetworkException);
PPLEXCEPTION(SocketIsAlreadyConnectedException,NetworkException);
PPLEXCEPTION(SocketIsNotConnectedException,NetworkException);
PPLEXCEPTION(CantSendAfterSocketShutdownException,NetworkException);
PPLEXCEPTION(TooManyReferencesException,NetworkException);
PPLEXCEPTION(HostDownException,NetworkException);
PPLEXCEPTION(NoRouteToHostException,NetworkException);
PPLEXCEPTION(InvalidSocketException,NetworkException);
PPLEXCEPTION(UnknownOptionException,NetworkException);
PPLEXCEPTION(OutOfBandDataReceivedException,NetworkException);
PPLEXCEPTION(BrokenPipeException,NetworkException);





PPLEXCEPTION(ResolverException,Exception);



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
ppluint32 Ntohl(ppluint32 net);
ppluint32 Htonl(ppluint32 host);
ppluint16 Ntohs(ppluint16 net);
ppluint16 Htons(ppluint16 host);
bool IsBigEndian();
bool IsLittleEndian();


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
		bool	connected;
		bool	islisten;
		//int		BytesWritten;
		//int		BytesRead;
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
		void connect(const String &host_and_port);
		void connect(const String &host, int port);
		void setTimeoutConnect(int seconds, int useconds);
		bool isConnected() const;
		//@}

		//! @name TCP-Server functions
		//@{
		void bind(const char *ip, int port);
		virtual int receiveConnect(TCPSocket *socket, const String &host, int port);
		bool isListening() const;
        void stopListen();
		void signalStopListen();
        void listen(int timeout=100);

		//@}

		//! @name Common functions for client and server
		//@{
		void setTimeoutRead(int seconds, int useconds);
		void setTimeoutWrite(int seconds, int useconds);

        void disconnect();
        void shutdown();
		size_t write(const String &str, size_t bytes=0);
		size_t write(const WideString &str, size_t bytes=0);
		size_t write(const ByteArrayPtr &bin, size_t bytes=0);
		size_t write(const void *buffer, size_t bytes);
		size_t writef(const char *fmt, ...);
		//int write(SocketMessage &msg);
		size_t read(void *buffer, size_t bytes);
		size_t read(String &buffer, size_t bytes);
		size_t read(ByteArray &buffer, size_t bytes);

		int getDescriptor();
		void setBlocking(bool value);
		bool isWriteable();
		bool isReadable();
		void waitForIncomingData(int seconds, int useconds);
		void waitForOutgoingData(int seconds, int useconds);
		//@}

		//! @name SSL Encryption
		//@{

		int sslInit(SSLContext *ssl);
		int sslShutdown();
		int sslInitClient();
		int sslInit_Server();
		int sslStart();
		int sslStop();
		int sslCheckCertificate(const char *hostname, bool AcceptSelfSignedCert=false);
		int sslAccept();
		int sslWaitForAccept(int timeout=0);

		//@}


		//! @name TODO
		//@{
        //int	waitForMessage(SocketMessage &msg, int timeout=0);
		//@}

		//! @name Depreceated
		//@{
		//int connectSSL(const char *host_and_port, SSLContext *ssl=NULL);
		//int connectSSL(const char *host, int port, SSLContext *ssl=NULL);
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




		PPLEXCEPTION(CouldNotBindToSocket, Exception);
		PPLEXCEPTION(InvalidSSLCertificate, Exception);
		PPLEXCEPTION(NoAddressSpecified, Exception);
		PPLEXCEPTION(CouldNotStartDaemon, Exception);
		PPLEXCEPTION(SSLInitializationFailed, Exception);

};

//! \brief CURL-Klasse
class Curl
{
	private:
		void *handle;
		void *httppost, *last_httppost;
		void *headers;
		char *resultbuffer;
		Logger *log;
		size_t resultbuffer_size;
		String Header;
		String Browser, Url, UserPassword, Referer, Proxy;
		String GetCall;
		char *errorbuffer;
		bool	aboard;

		void curlResultOk(int ret) const;



	public:

		PPLEXCEPTION(InitializationFailedException, Exception);
		PPLEXCEPTION(MiscException, Exception);
		PPLEXCEPTION(StringEscapeException, Exception);
		PPLEXCEPTION(InvalidURLException, Exception);
		PPLEXCEPTION(NoResultException, Exception);
		PPLEXCEPTION(OperationFailedException, Exception);


		enum HTTPVERSION {
			HTTP_1_0=1,
			HTTP_1_1,
			HTTP_2_0,
		};

		static bool isSupported();

		Curl();
		~Curl();

		//TODO: CNotifyHandler call_receive;
		//TODO: CNotifyHandler call_send;
		//TODO: CNotifyHandler call_done;

		void	debugHandler(int type, const char *data, size_t size);
		size_t	storeResult(void *ptr, size_t bytes, int type);
		void	setLogger(Logger *log);

		void	setBrowser(const String &browser);
		void	setURL(const String &url);
		void	setReferer(const String &url);
		void	setUserPassword(const String &username, const String &password);
		void	setUsername(const String &username);
		void	setPassword(const String &password);
		void	setUserPassword(const String &userpassword);	// Format: username:password
		void	setProxy(const String &proxy, int port);
		void	setHttpVersion(Curl::HTTPVERSION version);
		void	setTimeout(int seconds);
		void	setHeader(const String &name, const String &value);
		void	setMaximumPersistantConnects(int value);
		void	enableSignals(bool enable);
		void	reset();
		void	clearHeader();
		void	get();
		void	get(const String &parameter);
		void	get(const AssocArray &param);
		void	getResultBuffer(void **buffer, size_t *size) const;
		ByteArrayPtr getResultBuffer() const;
		void	copyResultBuffer(ByteArray &bin) const;
		String	getResultBufferAsString() const;
		String	getHeader() const;
		String	getURL() const;
		String	getLastURL() const;
		void	escape(String &target, const AssocArray &source);
		void	escape(String &string);
		void	addPostVar(const String &name, const String &data, const String &contenttype=String());
		void	addPostVar(const String &name, int val, const String &contenttype=String());
		void	addPostVar(const AssocArray &param, const String &prefix=String());
		void	post();
		void	post(const AssocArray &param);
		void 	postFields(const AssocArray &param);
		void	clear();

		void	addCAFile(const String &filename);
		void	verifyPeer(bool verify);

		void *	getCurlHandle() const;

		static String getUri(const String &uri);
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
