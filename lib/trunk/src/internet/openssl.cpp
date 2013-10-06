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
	#include <process.h>
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

#ifdef NO_SSL
#undef HAVE_OPENSSL
#endif

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/x509v3.h>
#endif


#include "ppl7.h"
#include "ppl7-inet.h"
#include "socket.h"

namespace ppl7 {

typedef struct {
	ppl7::Mutex *mutex;
} MUTEX_STRUCT;

#ifdef HAVE_OPENSSL
static bool SSLisInitialized=false;
static bool PRNGIsSeed=false;
//static int  SSLRefCount=0;
static Mutex	SSLMutex;
static MUTEX_STRUCT *mutex_buf=NULL;


// seed PRNG (Pseudo Random Number Generator)
static void SeedPRNG()
{
	if (PRNGIsSeed) return;		// Muss nur einmal gemacht werden
	char *buf;
	time_t  t;
#ifdef HAVE_GETUID
	uid_t   uid;
#endif
#ifdef _WIN32
	int		pid;
#else
	pid_t   pid;
#endif

	int p=0;

	// allocate buffer
	#ifdef HAVE_GETUID
		buf=(char*) malloc(sizeof(t)+sizeof(uid)*2+sizeof(pid)*2+32);
	#else
		buf=(char*) malloc(sizeof(t)+sizeof(pid)*2+32);
	#endif

	time(&t);
	memcpy(buf+p,&t,sizeof(time_t));
	p+=sizeof(time_t);

	#ifdef HAVE_GETUID
		uid=getuid();
		memcpy(buf+p,&uid,sizeof(uid_t));
		p+=sizeof(uid_t);
	#endif
	#ifdef HAVE_GETEUID
        uid=geteuid();
		memcpy(buf+p,&uid,sizeof(uid_t));
		p+=sizeof(uid_t);
	#endif

	#ifdef HAVE_GETPID
		pid=getpid();
		memcpy(buf+p,&pid,sizeof(pid));
		p+=sizeof(pid);
	#else
		#ifdef _WIN32
			pid=_getpid();
			memcpy(buf+p,&pid,sizeof(pid));
			p+=sizeof(pid);
		#endif
	#endif

	#ifdef HAVE_GETPPID
		pid=getppid();
		memcpy(buf+p,&pid,sizeof(pid));
		p+=sizeof(pid);
	#endif

	::MD5((const unsigned char*)buf,p,(unsigned char*)buf+p);
	p+=32;
	RAND_seed(buf,p);
	free(buf);
}

static void locking_function(int mode, int n, const char *file, int line)
{
	if (!mutex_buf) return;
	if (mode & CRYPTO_LOCK)
		mutex_buf[n].mutex->lock();
	else
		mutex_buf[n].mutex->unlock();
}

static unsigned long id_function(void)
{
	return (unsigned long) ppl7::ThreadID();
}


/*
static void ssl_exit()
{
	SSL_Exit();
}
*/

static const char * ssl_geterror(SSL *ssl, int code)
{
	int e=SSL_get_error(ssl,code);
	switch (e) {
		case SSL_ERROR_NONE: return "SSL_ERROR_NONE";
		case SSL_ERROR_ZERO_RETURN: return "SSL_ERROR_ZERO_RETURN";
		case SSL_ERROR_WANT_READ: return "SSL_ERROR_WANT_READ";
		case SSL_ERROR_WANT_WRITE: return "SSL_ERROR_WANT_WRITE";
		case SSL_ERROR_WANT_CONNECT: return "SSL_ERROR_WANT_CONNECT";
		case SSL_ERROR_WANT_ACCEPT: return "SSL_ERROR_WANT_ACCEPT";
		case SSL_ERROR_WANT_X509_LOOKUP: return "SSL_ERROR_WANT_X509_LOOKUP";
		case SSL_ERROR_SYSCALL: return "SSL_ERROR_SYSCALL";
		case SSL_ERROR_SSL: return "SSL_ERROR_SSL";
	};
	return "SSL_ERROR_UNKNOWN";
}


#endif



/*!\ingroup PPLGroupInternet
 * \brief OpenSSL-Schnittstelle initialisieren
 *
 * \desc
 * Bevor eine der SSL-Funktionen in TCPSocket und die Klasse SSLContext verwendet werden kann, muss
 * nicht nur OpenSSL beim Kompilieren der Library eingebunden werden, sondern diese bei
 * Programmstart auch initialisiert werden. Dazu muss diese Funktion aufgerufen werden.
 * \par
 * Sie initialisiert den Zufallszahlengenerator, OpenSSL für die Verwendung in einer
 * multithreaded Umgebung, sowie die verschiedenen verschlüsselungs Algorithmen.
 * \par
 * Da durch Aufruf dieser Funktion Speicher allokiert wird, muss irgendwann, wenn OpenSSL
 * nicht mehr benötigt wird, die Funktion SSL_Exit aufgerufen werden. Dies geschieht
 * bei Programmende jedoch auch automatisch.
 *
 * @exception UnsupportedFeatureException
 * @exception OutOfMemoryException
 * @exception InitializationFailedException
 *
 * \see
 * - SSL_Exit
 * - SSLContext
 * - CTCPSocket
 *
 * \relates SSLContext
 * \relates CTCPSocket
 */
void SSL_Init()
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	SSLMutex.lock();
	if (SSLisInitialized) {
		SSLMutex.unlock();
		return;
	}
	SeedPRNG();
	int max_locks=CRYPTO_num_locks();
	mutex_buf=(MUTEX_STRUCT*)malloc(max_locks * sizeof(MUTEX_STRUCT));
	if (!mutex_buf) {
		SSLMutex.unlock();
		throw OutOfMemoryException();
	}
	if (!SSL_library_init()) {
		free(mutex_buf);
		SSLMutex.unlock();
		throw InitializationFailedException("OpenSSL");
	}
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
	for (int i=0;i<max_locks;i++) {
		mutex_buf[i].mutex=new Mutex;
	}
	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	SSLisInitialized=true;
	atexit(SSL_Exit);
	SSLMutex.unlock();
#endif
}

/*!\ingroup PPLGroupInternet
 * \brief OpenSSL-Schnittstelle deinitialisieren
 *
 * \desc
 * Hat die Anwendung OpenSSL verwendet, muss nach Gebrauch diese Funktion aufgerufen werden,
 * um den durch OpenSSL belegten Speicher wieder freizugeben. Dies geschieht zu Programmende
 * jedoch automatisch, so dass man sich den Aufruf in der Regel sparen kann.
 *
 * @return Die Funktion gibt 1 zurück, wenn die OpenSSL-Schnittstelle erfolgreich
 * deinitialisiert wurde, im Fehlerfall 0.
 *
 * \see
 * - SSL_Init
 * - SSLContext
 * - CTCPSocket
 *
 * \relates SSLContext
 * \relates CTCPSocket
 *
 */
void SSL_Exit()
{
	#ifdef HAVE_OPENSSL
		SSLMutex.lock();
		if (!SSLisInitialized) {
			SSLMutex.unlock();
			return;
		}
		CRYPTO_set_id_callback(NULL);
		CRYPTO_set_locking_callback(NULL);
		if (mutex_buf) {
			int max_locks=CRYPTO_num_locks();
			for (int i=0;i<max_locks;i++) {
				delete mutex_buf[i].mutex;
			}
			free(mutex_buf);
			mutex_buf=NULL;
		}
		SSLisInitialized=false;
		EVP_cleanup();
		ERR_free_strings();
		SSLMutex.unlock();
	#endif
}


void ClearSSLErrorStack()
{
#ifdef HAVE_OPENSSL
	while ((ERR_get_error()));
#endif
}

int GetSSLError(SSLError &e)
{
#ifdef HAVE_OPENSSL
	unsigned long ec;
	const char *file, *data;
	char ebuffer[256];
	ec=ERR_get_error_line_data(&file,&e.Line,&data,&e.Flags);
	if (ec==0) return 0;
	ERR_error_string_n(ec,ebuffer,255);
	e.Text.set(ebuffer);
	e.Filename.set(file);
	e.Data.set(data);
	return 1;
#else
	return 0;
#endif
}

int GetSSLErrors(std::list<SSLError> &e)
{
#ifdef HAVE_OPENSSL
	e.clear();
	unsigned long ec;
	const char *file, *data;
	char ebuffer[256];
	SSLError se;

	while ((ec=ERR_get_error_line_data(&file,&se.Line,&data,&se.Flags))) {
		ERR_error_string_n(ec,ebuffer,255);
		se.Text.set(ebuffer);
		se.Filename.set(file);
		se.Data.set(data);
		e.push_back(se);
	}
	return (int) e.size();
#else
	return 0;
#endif
}


/*
 * SSLContext-Klasse
 */

/*!\class SSLContext
 * \ingroup PPLGroupInternet
 * \relates CTCPSocket
 *
 * \brief SSL-Schnittstelle
 *
 * \example TCP-Server mit SSL-Verschlüsselung
 * \dontinclude socket_examples.cpp
 * \skip Socket_Example3
 * \until EOF
 *
 */


SSLContext::SSLContext()
{
	ctx=NULL;
	references=0;
}

/*!\brief SSL-Kontext initialisieren
 *
 * \desc
 *
 * @exception UnsupportedFeatureException
 * @exception OutOfMemoryException
 * @exception InitializationFailedException
 * @exception IllegalArgumentException
 */
SSLContext::SSLContext(int method)
{
	ctx=NULL;
	references=0;
	init(method);
}

SSLContext::~SSLContext()
{
	clear();
}

void SSLContext::clear()
{
	shutdown();
}

/*!\brief SSL-Kontext initialisieren
 *
 * \desc
 *
 * @exception UnsupportedFeatureException
 * @exception OutOfMemoryException
 * @exception InitializationFailedException
 * @exception IllegalArgumentException
 */
void SSLContext::init(int method)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	SSLMutex.lock();
	if (!SSLisInitialized) {
		SSLMutex.unlock();
		SSL_Init();
	} else {
		SSLMutex.unlock();
	}
	shutdown();
	mutex.lock();
	if (!method) method=SSLContext::SSLv23;
	while ((ERR_get_error()));	// Clear Error-Stack
	switch (method) {
#ifndef OPENSSL_NO_SSL2
		case SSLContext::SSLv2:
			ctx=SSL_CTX_new(SSLv2_method());
			break;
		case SSLContext::SSLv2client:
			ctx=SSL_CTX_new(SSLv2_client_method());
			break;
		case SSLContext::SSLv2server:
			ctx=SSL_CTX_new(SSLv2_server_method());
			break;
#endif
		case SSLContext::SSLv3:
			ctx=SSL_CTX_new(SSLv3_method());
			break;
		case SSLContext::SSLv3client:
			ctx=SSL_CTX_new(SSLv3_client_method());
			break;
		case SSLContext::SSLv3server:
			ctx=SSL_CTX_new(SSLv3_server_method());
			break;
		case SSLContext::SSLv23:
			ctx=SSL_CTX_new(SSLv23_method());
			break;
		case SSLContext::SSLv23client:
			ctx=SSL_CTX_new(SSLv23_client_method());
			break;
		case SSLContext::SSLv23server:
			ctx=SSL_CTX_new(SSLv23_server_method());
			break;
		case SSLContext::TLSv1:
			ctx=SSL_CTX_new(TLSv1_method());
			break;
		case SSLContext::TLSv1client:
			ctx=SSL_CTX_new(TLSv1_client_method());
			break;
		case SSLContext::TLSv1server:
			ctx=SSL_CTX_new(TLSv1_server_method());
			break;
		default:
			mutex.unlock();
			throw IllegalArgumentException("SSLContext::Init(int method=%i)",method);
			break;
	};
	if (!ctx) {
		mutex.unlock();
		throw InitializationFailedException("SSL_CTX_new");
	}
	mutex.unlock();
#endif
}

bool SSLContext::isInit()
{
	if (ctx) return true;
	return false;
}

/*!\brief SSL-Kontext deinitialisieren
 *
 * \desc
 *
 * @exception SSLContextInUseException
 */
void SSLContext::shutdown()
{
	#ifdef HAVE_OPENSSL
		if (!SSLisInitialized) return;
		mutex.lock();
		if (references) {
			mutex.unlock();
			throw SSLContextInUseException("%i",references);
		}
    	if (ctx) {
			SSL_CTX_free((SSL_CTX*)ctx);
			ctx=NULL;
    	}
    	while ((ERR_get_error()));	// Clear Error-Stack
    	mutex.unlock();
	#endif
}

/*!\brief create a new SSL structure for a connection
 *
 * \desc
 * creates a new SSL structure which is needed to hold the data for a TLS/SSL connection.
 * The new structure inherits the settings of the underlying SSLContext:
 * - connection method (SSLv2/v3/TLSv1)
 * - options
 * - verification settings
 * - timeout settings.
 * \return The function allocates memory to hold a SSL-Structure. It must be freed by
 * using SSLContext::releaseSSL
 *
 * @exception UnsupportedFeatureException
 * @exception SSLContextUninitializedException
 * @exception SSLException
 */
void *SSLContext::newSSL()
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	mutex.lock();
	if (!ctx) {
		mutex.unlock();
		throw SSLContextUninitializedException();
	}
	while ((ERR_get_error()));	// Clear Error-Stack
	void *ssl=SSL_new((SSL_CTX*)ctx);
	if (!ssl) {
		mutex.unlock();
		throw SSLException("SSL_new failed");
	}
	references++;
	mutex.unlock();
	return ssl;
#endif
}

/*!\brief free an allocated SSL structure
 *
 * \desc
 * Decrements the reference count of \p ssl, and removes the SSL
 * structure pointed to by \p ssl and frees up the allocated memory if the
 * reference count has reached 0.
 * @param ssl Pointer to SSL structure received by SSLContext::newSSL
 *
 * @exception UnsupportedFeatureException is thrown, when OpenSSL is not supported
 * @exception NullPointerException is thrown when \p ssl points to NULL
 * @exception SSLContextReferenceCounterMismatchException is thrown, when the internal reference
 * counter is already 0 when calling this function
 */
void SSLContext::releaseSSL(void *ssl)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	if (!ssl) throw NullPointerException();
	mutex.lock();
	SSL_free((SSL*)ssl);
	if (references<=0) throw SSLContextReferenceCounterMismatchException();
	references--;
	mutex.unlock();
#endif
}

void SSLContext::loadTrustedCAfromFile(const String &filename)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	mutex.lock();
	if (!ctx) {
		mutex.unlock();
		throw SSLContextUninitializedException();
	}
	while ((ERR_get_error()));	// Clear Error-Stack
	if (SSL_CTX_load_verify_locations((SSL_CTX*)ctx,filename,NULL)!=1) {
		mutex.unlock();
		throw InvalidSSLCertificateException(filename);
	}
	mutex.unlock();
#endif
}

void SSLContext::loadTrustedCAfromPath(const String &path)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else

	mutex.lock();
	if (!ctx) {
		mutex.unlock();
		throw SSLContextUninitializedException();
	}
	while ((ERR_get_error()));	// Clear Error-Stack
	if (SSL_CTX_load_verify_locations((SSL_CTX*)ctx,NULL,path)!=1) {
		mutex.unlock();
		throw InvalidSSLCertificateException(path);
	}
	mutex.unlock();
#endif
}


#ifdef HAVE_OPENSSL
static int pem_passwd_cb(char *buf, int size, int rwflag, void *password)
{
	strncpy(buf, (char *)(password), size);
	buf[size - 1] = 0;
	return(strlen(buf));
}
#endif

/*!\brief
 *
 * \desc
 * LoadCertificate wird benoetigt, wenn ein SSL-Server gestartet werden soll.
 * LoadCertificate laed ein Zertifikat im PEM-Format oder eine komplette Trustchain im
 * PEM-Format aus dem File "certificate". Wird "privatekey" angegeben, wird daraus der
 * Private Key geladen. Wenn nicht, wird der Private Key ebenfalls in der "certificate"-
 * Datei erwartet. Ist der Key durch ein Passwort geschuetzt, muss dieses als "password"
 * angegeben werden.
 *
 */
void SSLContext::loadCertificate(const String &certificate, const String &privatekey, const String &password)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	mutex.lock();
	if (!ctx) {
		mutex.unlock();
		throw SSLContextUninitializedException();
	}
	while ((ERR_get_error()));	// Clear Error-Stack

	//if (!SSL_CTX_use_certificate_file((SSL_CTX*)ctx,keyfile,SSL_FILETYPE_PEM)) {
	if (!SSL_CTX_use_certificate_chain_file((SSL_CTX*)ctx,certificate)) {
		mutex.unlock();
		throw SSLException("SSL_CTX_use_certificate_chain_file");
	}
	if (password.notEmpty()) {
		SSL_CTX_set_default_passwd_cb((SSL_CTX*)ctx,pem_passwd_cb);
		SSL_CTX_set_default_passwd_cb_userdata((SSL_CTX*)ctx,(void*)password.getPtr());
	}
	String key=privatekey;
	if (key.isEmpty()) key=certificate;
	if (!SSL_CTX_use_PrivateKey_file((SSL_CTX*)ctx,key,SSL_FILETYPE_PEM)) {
		mutex.unlock();
		throw SSLPrivatKeyException(key);
	}
	mutex.unlock();
#endif
}


void SSLContext::setCipherList(const String &cipherlist)
{
#ifndef HAVE_OPENSSL
	throw UnsupportedFeatureException("OpenSSL");
#else
	mutex.lock();
	if (!ctx) {
		mutex.unlock();
		throw SSLContextUninitializedException();
	}
	while ((ERR_get_error()));	// Clear Error-Stack
	if (SSL_CTX_set_cipher_list((SSL_CTX*)ctx, cipherlist)!=1) {
		mutex.unlock();
		throw InvalidSSLCipherException(cipherlist);
	}
	mutex.unlock();
#endif
}


#ifdef TODO

/** @name SSL-Verschlüsselung
 *  Die nachfolgenden Befehle werden benötigt, wenn die Kommunikation zwischen Client
 *  und Server mit SSL verschlüsselt werden soll. Voraussetzung dafür ist, dass die PPL-Library
 *  mit OpenSSL-Unterstützung kompiliert wurde.
 *
 */
//@{

/*!\brief Socket als SSL-Client vorbereiten
 *
 * \desc
 * Diese Funktion muss aufgerufen werden, wenn über eine bereits bestehende TCP-Verbindung
 * SSL-Verschlüsselung aktiviert werden soll. Dazu muss nach dem Connect zunächst CTCPSocket::SSL_Init_Client
 * gefolgt von CTCPSocket::SSL_Start aufgerufen werden.
 *
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SSL_Init_Client()
{
	#ifdef HAVE_OPENSSL
		if (!SSLisInitialized) {
			SetError(317);
			return 0;
		}
 		if (!connected) {
			SetError(275);
			if (log) log->LogError(5);
        	return 0;
    	}
		if (!sslclass) {
			SetError(321);
			return 0;
		}
		if (!ssl) {
			PPLSOCKET *s=(PPLSOCKET*)socket;
			//ssl=SSL_new((SSL_CTX*)ctx);
			ssl=sslclass->NewSSL();
			if (!ssl) {
				//SetError(322,"SSL_new");
				return 0;
			}
			SSL_set_fd((SSL*)ssl,s->sd);
			//SSL_set_mode((SSL*)ssl, SSL_MODE_AUTO_RETRY);
			SSL_set_connect_state((SSL*)ssl);
		}
		return 1;
	#else
		SetError(292);
		return 0;
	#endif
}

/*!\brief SSL-Kommunikation starten
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die SSL-Kommunikation über eine bereits bestehende TCP-Verbindung
 * zu einem Server gestartet. Um wieder unverschlüsselt kommunizieren zu können, muss CTCPSocket::SSL_Stop
 * aufgerufen werden.
 *
 * @return Wenn die SSL-Kommunikation mit dem Server erfolgreich gestartet werden konnte, liefert die
 * Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SSL_Start()
{
	#ifdef HAVE_OPENSSL
		if (!ssl) {
			SetError(331);
			return 0;
		}
		if (connect_timeout_sec>0 || connect_timeout_usec>0) {
			SetBlocking(false);
			struct timeval tval;
			fd_set rset, wset;
			int n;
			int sockfd=((PPLSOCKET*)socket)->sd;
			tval.tv_sec=connect_timeout_sec;
			tval.tv_usec=connect_timeout_usec;
			FD_ZERO(&rset);
			FD_SET(sockfd, &rset);
			wset=rset;
			int res;
			while ((res=SSL_connect((SSL*)ssl))<1) {
				if (thread!=NULL && thread->ThreadShouldStop()) {
					SSL_shutdown((SSL*)ssl);
					SSL_free((SSL*)ssl);
					ssl=NULL;
					SetError(336);
					return 0;
				}
				if (res==0) {
					SetError(322,"SSL_connect: %s, State: 0x%X %s",ssl_geterror((SSL*)ssl,res),SSL_state((SSL*)ssl), SSL_state_string_long((SSL*)ssl));
					SSL_shutdown((SSL*)ssl);
					SSL_free((SSL*)ssl);
					ssl=NULL;
					return 0;
				}
				int e=SSL_get_error((SSL*)ssl,res);
				//printf ("res=%i, e=%i, state=%x: %s\n",res,e,SSL_state((SSL*)ssl), SSL_state_string_long((SSL*)ssl));
				if (e==SSL_ERROR_WANT_READ) {
					if ((n=select(sockfd+1,&rset,NULL,NULL,&tval))==0) {
						SetError(174,"SSL_connect: %s, State: 0x%X %s",ssl_geterror((SSL*)ssl,res),SSL_state((SSL*)ssl), SSL_state_string_long((SSL*)ssl));
						SSL_shutdown((SSL*)ssl);
						SSL_free((SSL*)ssl);
						ssl=NULL;
						return 0;
					}
				} else if (e==SSL_ERROR_WANT_WRITE) {
					if ((n=select(sockfd+1,NULL,&wset,NULL,&tval))==0) {
						SetError(174,"SSL_connect: %s, State: 0x%X %s",ssl_geterror((SSL*)ssl,res),SSL_state((SSL*)ssl), SSL_state_string_long((SSL*)ssl));
						SSL_shutdown((SSL*)ssl);
						SSL_free((SSL*)ssl);
						ssl=NULL;
						return 0;
					}
				} else {
					SetError(174,"SSL_connect: %s, State: 0x%X %s",ssl_geterror((SSL*)ssl,res),SSL_state((SSL*)ssl), SSL_state_string_long((SSL*)ssl));
					SSL_shutdown((SSL*)ssl);
					SSL_free((SSL*)ssl);
					ssl=NULL;
					return 0;
				}
			}
			SetBlocking(true);
		} else {
			int res=SSL_connect((SSL*)ssl);
			if (res<1) {
				SetError(322,"SSL_connect: %s",ssl_geterror((SSL*)ssl,res));
				SSL_shutdown((SSL*)ssl);
				SSL_free((SSL*)ssl);
				ssl=NULL;
				return 0;
			}
		}
		return 1;
	#else
		SetError(292);
		return 0;
	#endif
}


/*!\brief SSL-Kommunikation stoppen
 *
 * \desc
 * Durch Aufruf dieser Funktion wird die SSL-Kommunikation zu einem Server wieder gestoppt. Sofern
 * die Verbindung dadurch nicht geschlossen wird, kann mit der Gegenstelle wieder unverschlüsselt
 * kommuniziert werden.
 *
 * @return Wenn die SSL-Kommunikation erfolgreich beendet werden konnte, liefert die
 * Funktion 1 zurück, im Fehlerfall 0.
 *
 * @return
 */
int CTCPSocket::SSL_Stop()
{
	#ifdef HAVE_OPENSSL
		if (!SSLisInitialized) {
			SetError(317);
			return 0;
		}
		if (ssl) {
			SSL_shutdown((SSL*)ssl);
			SSL_free((SSL*)ssl);
			ssl=NULL;
		}
		//mutex.Unlock();
		return 1;
	#else
		SetError(292);
		return 0;
	#endif

}

/*!\brief Verwendung von OpenSSL initialisieren
 *
 * \desc
 * Falls beabsichtigt wird mit diesem Socket verschlüsselt zu kommunizieren, muss diese Funktion noch
 * vor Verbindungsaufbau aufgerufen werden. Als Parameter \p ssl bekommt sie einen Pointer auf eine SSLContext-Klasse.
 * Diese muss ebenfalls vor Verwendung initialisiert worden sein.
 *
 * @param[in] ssl Pointer auf eine SSLContext-Klasse
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SSL_Init(SSLContext *ssl)
{
	if (!ssl) {
		SetError(194);
		return 0;
	}
	SSL_Shutdown();
	sslreference=ssl->RegisterSocket(this);
	if (!sslreference) return 0;
	sslclass=ssl;
	return 1;
}

/*!\brief SSL für diese Klasse deinitialisieren
 *
 * \desc
 * Wurde mit dieser Socket-Klasse SSL-Verschlüsselung verwendet, muss nach beendigung der Verbindung diese Funktion
 * aufgerufen werde. Dies wird com Destruktor der Klasse automatisch durchgeführt, so dass ein manueller Aufruf
 * durch die Anwendung in der Regel nicht erforderlich ist.
 *
 * @return Die Funktion gibt zur Zeit immer 1 zurück.
 */

int CTCPSocket::SSL_Shutdown()
{
	SSL_Stop();
	if (sslclass) {
		sslclass->ReleaseSocket(this,sslreference);
		sslclass=NULL;
		sslreference=NULL;
	}
	return 1;
}


/*!\brief Verschlüsselte Verbindung aufbauen
 *
 * \desc
 * Mit dieser Funktion wird zunächst eine Verbindung zur gewünschten Zieladresse \p host
 * aufgebaut und dann die SSL-Verschlüsselung gestartet.
 *
 * \param[in] host Der Parameter "host" muss das Format "hostname:port" haben, wobei
 * "hostname" auch eine IP-Adresse sein kann. Der Port kann entweder als Zahl oder
 * als Servicename angegeben werden, z.B. "smtp" für Port 25.
 * \param[in] ssl Optionaler Pointer auf ein SSLContext-Objekt. Wird es nicht angegeben, erstellt
 * die Klasse selbst eins mit Default-Parametern. Es ist flexibler, ein eigenes Objekt
 * zu erstellen, da man damit auch beliebige zusätzliche Zertifikate laden oder bestimmte
 * Verschlüsselungs-Algorithmen definieren kann.
 * \return Konnte die Verbindung erfolgreich aufgebaut werden, wird true (1) zurückgegeben,
 * im Fehlerfall false (0).
 * \since Seit Version 6.0.18 kann der Port statt als Nummer auch als Servicenamen angegeben
 * werden.
 */
int CTCPSocket::ConnectSSL(const char *host, SSLContext *ssl)
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
	int port=atoi(hostname.Get(1));
	if (port<=0) {
        SetError(271);
        return 0;
	}
	return ConnectSSL(hostname.Get(0),port, ssl);
}

/*!\brief Verschlüsselte Verbindung aufbauen
 *
 * \desc
 * Mit dieser Funktion wird zunächst eine Verbindung zur gewünschten Zieladresse \p host
 * aufgebaut und dann die SSL-Verschlüsselung gestartet.
 *
 * \param[in] host Der Hostname oder die IP-Adresse des Zielrechners
 * \param[in] port Der gewünschte Zielport
 * \param[in] ssl Optionaler Pointer auf ein SSLContext-Objekt. Wird es nicht angegeben, erstellt
 * die Klasse selbst eins mit Default-Parametern. Es ist flexibler, ein eigenes Objekt
 * zu erstellen, da man damit auch beliebige zusätzliche Zertifikate laden oder bestimmte
 * Verschlüsselungs-Algorithmen definieren kann.
 * \return Konnte die Verbindung erfolgreich aufgebaut werden, wird true (1) zurückgegeben,
 * im Fehlerfall false (0).
 * \since Seit Version 6.0.18 kann der Port statt als Nummer auch als Servicenamen angegeben
 * werden.
 */
int CTCPSocket::ConnectSSL(const char *host, int port, SSLContext *ssl)
{
	if (ssl) {
		if (!SSL_Init(ssl)) return 0;
	}
	if (!Connect(host,port)) return 0;
	if (!SSL_Init_Client()) return 0;
	if (!SSL_Start()) return 0;
	return 1;
}


/*!\brief Verschlüsselte Daten schreiben
 *
 * \desc
 * Diese private Funktion wird intern automatisch von den Write-Funktionen der Socketklasse
 * aufgerufen, wenn SSL-Verschlüsselung aktiviert wurde. Anstelle der Write-Funktionen der
 * Socket-Schnittstelle des Betriebssystems wird dann diese Funktion verwendet.
 * Sie schickt \p size Bytes aus dem Speicherbereich \p buffer verschlüsselt an die
 * Gegenstelle.
 *
 * @param[in] buffer Beginn des zu sendenden Speicherbereichs
 * @param[in] size Anzahl zu sendender Bytes.
 * @return Die Funktion gibt die Anzahl erfolgreich geschriebener Bytes zurück, im Fehlerfall
 * 0 oder einen negativen Wert. Aus der Manpage zu SSL_write(3):
 * - 0: The write operation was not successful. Probably the underlying
 *     connection was closed. Call SSL_get_error() with the return value
 *     ret to find out, whether an error occurred or the connection was
 *     shut down cleanly (SSL_ERROR_ZERO_RETURN).
 *     \par
 *     SSLv2 (deprecated) does not support a shutdown alert protocol, so
 *     it can only be detected, whether the underlying connection was
 *     closed. It cannot be checked, why the closure happened.
 *
 * - <0: The write operation was not successful, because either an error
 *       occurred or action must be taken by the calling process. Call
 *       SSL_get_error() with the return value ret to find out the reason.
 */
int CTCPSocket::SSL_Write(const void *buffer, int size)
{
	#ifdef HAVE_OPENSSL
		int bytes=::SSL_write((SSL*)ssl,buffer,size);
		return bytes;
	#else
		SetError(292);
		return 0;
	#endif
}

/*!\brief Verschlüsselte Daten lesen
 *
 * \desc
 * Diese private Funktion wird intern automatisch von den Read-Funktionen der Socketklasse
 * aufgerufen, wenn SSL-Verschlüsselung aktiviert wurde. Anstelle der Read-Funktionen der
 * Socket-Schnittstelle des Betriebssystems wird dann diese Funktion verwendet.
 * Sie liest \p size verschlüsselte Bytes von der Gegenstelle in den bereits allokierten
 * Speicherbereich \p buffer und entschlüsselt sie dabei.
 *
 * @param[in] buffer Bereits allokierter Speicherbereich, in den die gelesenen Daten
 * geschrieben werden sollen
 * @param[in] size Anzahl zu lesender Bytes.
 * @return Die Funktion gibt die Anzahl erfolgreich gelesener Bytes zurück, im Fehlefall
 * 0 oder einen negativen Wert.
 */
int CTCPSocket::SSL_Read(void *buffer, int size)
{
	#ifdef HAVE_OPENSSL
		int bytes=::SSL_read((SSL*)ssl,buffer,size);
		return bytes;
	#else
		SetError(292);
		return 0;
	#endif
}

/*!\brief Socket als SSL-Server vorbereiten
 *
 * \desc
 * Falls mit diesem Socket ein SSL-fähiger Server gestartet werden soll, muss nach Verbindungseingang CTCPSocket::Init_SSL
 * gefolgt von dieser Funktion aufgerufen werden. Damit wird dem Client mitgeteilt, dass der Server nun verschlüsselt
 * kommunizieren möchte. Mit CTCPSocket::SSL_Accept kann geprüft werden, ob der Client bereits zugestimmt hat, mit
 * CTCPSocket::SSL_WaitForAccept wird gewartet, bis der Client zustimmt oder ein Timeout auftritt.
 *
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SSL_Init_Server()
{
	#ifdef HAVE_OPENSSL
		if (!SSLisInitialized) {
			SetError(317);
			return 0;
		}
 		if (!connected) {
			SetError(275);
			if (log) log->LogError(5);
        	return 0;
    	}
		if (!sslclass) {
			SetError(321);
			return 0;
		}
		if (!ssl) {
			PPLSOCKET *s=(PPLSOCKET*)socket;
			//ssl=SSL_new((SSL_CTX*)ctx);
			ssl=sslclass->NewSSL();
			if (!ssl) {
				//SetError(322,"SSL_new");
				return 0;
			}
			SSL_set_fd((SSL*)ssl,s->sd);
			//SSL_set_mode((SSL*)ssl, SSL_MODE_AUTO_RETRY);
			SSL_set_accept_state((SSL*)ssl);
		}
		return 1;
	#else
		SetError(292);
		return 0;
	#endif
}

/*!\brief Auf eine TLS/SSL-Handshake warten
 *
 * \desc
 * SSL_Accept wartet darauf, dass der mit dem Socket verbundene Client eine TLS/SSL Verbindung
 * startet. Ist der Socket auf "blocking" eingestellt, wartet die Funktion solange, bis
 * ein Handshake erfolgt, die Verbindung getrennt wird oder ein Timeout auftritt.
 * Die verwandte Funktion CTCPSocket::SSL_WaitForAccept wartet ebenfalls auf ein TLS/SSL Handshake,
 * jedoch kann hier zusätzlich ein Timeout angegeben werden und die Funktion beendet sich, wenn
 * der Thread beendet werden soll.
 *
 * @return Bei erfolgreichem Handshake liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SSL_Accept()
{
	#ifdef HAVE_OPENSSL
		if (!ssl) {
			SetError(331);
			return 0;
		}
		/*
		 *
       If the underlying BIO is non-blocking, SSL_accept() will also return when the underlying BIO could not satisfy the needs of
       SSL_accept() to continue the handshake, indicating the problem by the return value -1.  In this case a call to SSL_get_error() with
       the return value of SSL_accept() will yield SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE. The calling process then must repeat the
       call after taking appropriate action to satisfy the needs of SSL_accept().  The action depends on the underlying BIO. When using a
       non-blocking socket, nothing is to be done, but select() can be used to check for the required condition. When using a buffering BIO,
       like a BIO pair, data must be written into or retrieved out of the BIO before being able to continue.
		 */
		int res=SSL_accept((SSL*)ssl);
		if (res<1) {
			int e=SSL_get_error((SSL*)ssl,res);
			if (e==SSL_ERROR_WANT_READ || e==SSL_ERROR_WANT_WRITE) {
				// Non-Blocking
				SetError(309);
				return 0;
			} else {
				SetError(332,"%s, %s",ssl_geterror((SSL*)ssl,res), ERR_error_string(e,NULL));
				/*
		        char *ERR_error_string(unsigned long e, char *buf);
        const char *ERR_lib_error_string(unsigned long e);
        const char *ERR_func_error_string(unsigned long e);
        const char *ERR_reason_error_string(unsigned long e);
        */

				SSL_shutdown((SSL*)ssl);
				SSL_free((SSL*)ssl);
				ssl=NULL;
				return 0;
			}
		}
		return 1;
	#else
		SetError(292);
		return 0;
	#endif
}

/*!\brief SSL-Zertifikat der Gegenstelle prüfen
 *
 * \desc
 * Mit dieser Funktion kann nach Herstellung einer SSL-Verbindung das Zertifikat der Gegenstelle
 * geprüft werden. Dabei werden insbesondere auch die Signierungsketten bis zum Root-Zertifikat überprüft
 * und dabei nicht nur die gängigen Root-Zertifikate, die in der OpenSSL-Library enthalten sind, berücksichtigt,
 * sondern auch die, die zuvor manuell mit SSLContext::LoadTrustedCAfromFile oder SSLContext::LoadTrustedCAfromPath
 * geladen wurden.
 *
 * @param[in] hostname Der erwartete Name des Zertifikats. Wird NULL übergeben, wird der Name des Zertifikats
 * nicht überprüft
 * @param[in] AcceptSelfSignedCert Wird hier "true" angegeben, werden auch selbst-signierte Zertifikate
 * akzeptiert.
 * @return Ist das Zertifikat gültig und alle Prüfungen erfolgreich, gibt die Funktion 1 zurück,
 * ansonsten 0.
 */
int CTCPSocket::SSL_CheckCertificate(const char *hostname, bool AcceptSelfSignedCert)
{
	#ifdef HAVE_OPENSSL
		if (!ssl) {
			SetError(321);
			return 0;
		}
		if (hostname) {
			// Den Namen Überprüfen
			X509 *peer=SSL_get_peer_certificate((SSL*)ssl);
			char peer_CN[256];
			X509_NAME_get_text_by_NID(X509_get_subject_name(peer),NID_commonName,peer_CN,256);
			if (strcasecmp(peer_CN,(char*)hostname)!=0) {
				SetError(324,"Expected: \"%s\", got: \"%s\"",hostname,peer_CN);
				X509_free(peer);
				return 0;
			}
			X509_free(peer);
		}

		// Zertifikat Überprüfen
		int ret=SSL_get_verify_result((SSL*)ssl);
		//if (ret!=X509_V_OK && ret!=X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
		if (ret!=X509_V_OK) {
			const char *a="unknown";
			switch (ret) {
				case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
					a="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT";
					break;
				case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
					a="X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE";
					break;
				case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
					a="X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY";
					break;
				case X509_V_ERR_CERT_SIGNATURE_FAILURE:
					a="X509_V_ERR_CERT_SIGNATURE_FAILURE";
					break;
				case X509_V_ERR_CERT_NOT_YET_VALID:
					a="X509_V_ERR_CERT_NOT_YET_VALID";
					break;
				case X509_V_ERR_CERT_HAS_EXPIRED:
					a="X509_V_ERR_CERT_HAS_EXPIRED";
					break;
				case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
					a="X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD";
					break;
				case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
					a="X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD";
					break;
				case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
					a="X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT";
					break;
				case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
					a="X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN";
					break;
				case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
					a="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY";
					break;
				case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
					a="X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE";
					break;
				case X509_V_ERR_INVALID_CA:
					a="X509_V_ERR_INVALID_CA";
					break;
				case X509_V_ERR_PATH_LENGTH_EXCEEDED:
					a="X509_V_ERR_PATH_LENGTH_EXCEEDED";
					break;
				case X509_V_ERR_INVALID_PURPOSE:
					a="X509_V_ERR_INVALID_PURPOSE";
					break;
				case X509_V_ERR_CERT_UNTRUSTED:
					a="X509_V_ERR_CERT_UNTRUSTED";
					break;
				case X509_V_ERR_CERT_REJECTED:
					a="X509_V_ERR_CERT_REJECTED";
					break;
				case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
					a="X509_V_ERR_SUBJECT_ISSUER_MISMATCH";
					break;
				case X509_V_ERR_AKID_SKID_MISMATCH:
					a="X509_V_ERR_AKID_SKID_MISMATCH";
					break;
				case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
					a="X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH";
					break;
				case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
					a="X509_V_ERR_KEYUSAGE_NO_CERTSIGN";
					break;
				case X509_V_ERR_CERT_REVOKED:
					a="X509_V_ERR_CERT_REVOKED";
					break;
			}
			SetError(323,"Host: %s, SSL-Error: %s",(const char*)HostName,a);
			if (AcceptSelfSignedCert==false || ret!=X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
				return 0;
			}
		}
		return 1;
	#else
		SetError(292);
		return 0;
	#endif
}

/*!\brief Auf eine TLS/SSL-Handshake warten

 * \desc
 * SSL_WaitForAccept wartet darauf, dass der mit dem Socket verbundene Client eine TLS/SSL Verbindung
 * startet. Vor AUfruf sollte der Socket auf "non-Blocking" gestellt werden (siehe CTCPSocket::SetBlocking).
 * Die Funktion wartet solange, bis entweder ein Handshake zustande kommt, der angegebene Timeout
 * erreicht oder die Verbindung getrennt wurde, oder der Überwachungsthread (siehe CTCPSocket::WatchThread)
 * beendet werden soll.
 *
 * @param timeout Ein Timeout in Sekunden. Bei Angabe von 0, wartet die Funktion unbegenzt lange.
 * @return Bei erfolgreichem Handshake liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CTCPSocket::SSL_WaitForAccept(int timeout)
{
	ppluint64 tt=GetMilliSeconds()+(timeout*1000);
	while (timeout==0 || GetMilliSeconds()<=tt) {
		if (thread) {
			if (thread->ThreadShouldStop()) {
				SetError(336);
				return 0;
			}
		}
		if (!SSL_Accept()) {
			if (ppl6::GetErrorCode()!=309) {	// Non-Blocking
				return 0;
			}
		} else {
			return 1;
		}
		MSleep(10);
	}
	SetError(174);
	return 0;
}

//@}



#endif

} // EOF namespace ppl



