/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CDBSybase.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef _WIN32
#include <winsock.h>
#endif

#include "ppl6.h"

#ifdef HAVE_SYBASE
#include <ctpublic.h>
#endif

#define PPLSYB_DONE 1
#define PPLSYB_FAILURE 2
#define PPLSYB_RESULTS 3

namespace ppl6 {

#ifdef HAVE_SYBASE
static int syb_refcount=0;
static unsigned int ctx_version=CS_VERSION_100;
#endif

static CMutex refmutex;
static int maxconnects=0;
static char *default_locale=NULL;
static char *default_dateformat=NULL;



int SybaseGetMaxConnects()
/*!\brief Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken auslesen
 *
 * \desc
 *
 * \ingroup PPLGroupOldDatabases
 */
{
	return maxconnects;
}

int SybaseSetMaxConnects(int max)
/*!\brief Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken festlegen
 *
 * \ingroup PPLGroupOldDatabases
 *
 * \remarks
 * Die Funktion muss aufgerufen werden, bevor die erste CDBSybase-Klasse instantiiert wird!
 */
{
	if (maxconnects>0) return 0;
	maxconnects=max;
	return 1;
}

int SybaseSetLocale(const char *locale, const char *dateformat)
/*!\brief Legt Ländereinstellungen fest
 *
 * \ingroup PPLGroupOldDatabases
 * \desc
 * Mit dieser Funktion kann festgelegt werden, welche Ländereinstellungen als
 * Default verwendet werden sollen.
 *
 * @param[in] locale Pointer auf den Namen der Lokalisierung, z.B. "us_english" oder "german". Genaueres
 * ist dem Sybase-Handbuch zu entnehmen.
 * @param[in] dateformat Gibt an, in welcher Reihenfolge bei einem Datum Tag, Monat und Jahr
 * erwartet werden. Mögliche Werte sind: mdy, dmy, ymd, ydm, myd, und dym.
 *
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0.
 *
 * \remarks
 * Die Funktion muss aufgerufen werden, bevor die erste CDBSybase-Klasse instantiiert wird!
 *
 *
 * \attention
 * Die Funktion wird von FreeTDS nicht unterstützt. Hier empfiehlt es sich eine Default-Einstellung in
 * der freetds.conf vorzunehmen:
 * \code
 * [global]
 * language=german
 * ...
 * \endcode
 */
{
	if (default_locale) free(default_locale);
	default_locale=NULL;
	if (locale) default_locale=strdup(locale);
	return 1;
}

int SybaseSetVersion(int version)
/*!\brief Zu verwendende Version definieren
 *
 * \ingroup PPLGroupOldDatabases
 *
 *\desc
 * Legt das grundlegende Verhalten der Sybase CS-Library fest.
 *
 * @param[in] version Kann folgende Werte annehmen:
 * <table border=1 cellspacing=0 cellpadding=2><tr class=table-header><th>Wert</th><th>Verhalten</th><th>Features</th></tr>
 * <tr class=table-row><td>100</td><td>10.0 behavior</td><td>Initial version</td></tr>
 * <tr class=table-row><td>110</td><td>11.1 behavior</td><td>Unicode character set support,
 * Use of external configuration files to control Client-Library property settings.</td></tr>
 * <tr class=table-row><td>120</td><td>12.0 behavior</td><td>All above features</td></tr>
 * <tr class=table-row><td>125</td><td>12.5 behavior</td><td>unichar support, wide data and columns, SSL</td></tr>
 * <tr class=table-row><td>150</td><td>15.0 behavior</td><td>BCP partitions, BCP computed columns,
 * large identifiers, Unilib, ASE default packet size, scrollable cursors,
 * and clusters support. Also, support for unitext, xml, bigint, usmallint,
 * uint, and ubigint datatypes. Note the Sybase library name change.</td></tr>
 * </table>
 * Der Default ist 100
 *
 * @return Bei Erfolg liefert die Funktion 1 zurück, sonst 0.
 *
 * \remarks Die Verfügbarkeit der Versionen hängt von der installierten Library-
 * Version ab. Eine Version 12-Library wird den Wert 150 nicht kennen und
 * FreeTDS bietet gegenwärtig nur Version 10.0 und 11.1 an.
 *
 * Die Funktion muss aufgerufen werden, bevor die erste CDBSybase-Klasse instantiiert wird!
 */
{
#ifdef HAVE_SYBASE
#ifdef CS_VERSION_100
	if (version==100) {
		ctx_version=CS_VERSION_100;
		return 1;
	}
#endif
#ifdef CS_VERSION_110
	if (version==110) {
		ctx_version=CS_VERSION_110;
		return 1;
	}
#endif
#ifdef CS_VERSION_120
	if (version==120) {
		ctx_version=CS_VERSION_120;
		return 1;
	}
#endif
#ifdef CS_VERSION_125
	if (version==125) {
		ctx_version=CS_VERSION_125;
		return 1;
	}
#endif
#ifdef CS_VERSION_150
	if (version==150) {
		ctx_version=CS_VERSION_150;
		return 1;
	}
#endif
	SetError(510,"%i",version);
	return 0;
#else
	SetError(511,"Sybase");
	return 0;
#endif
}


#ifdef HAVE_SYBASE
static CS_CONTEXT				*context=NULL;
static CS_LOCALE				*locale=NULL;

typedef struct tagSybConnect {
	struct tagSybConnect	*previous;
	struct tagSybConnect	*next;
	CS_CONNECTION			*conn;
	CS_COMMAND				*cmd;
	CDBSybase			*c;
} SYBCONNECT;

typedef struct {
    CS_INT *lengths;
    CS_SMALLINT *indicators;
    char **tmp_buffer;
    unsigned char *numerics;
	int *ppl_type;
    CS_INT *types;
    CS_DATAFMT *datafmt;
    CS_RETCODE last_retcode;
} sybase_result;

static SYBCONNECT *first=NULL;
static SYBCONNECT *last=NULL;

static SYBCONNECT *FindSybClass(CS_CONNECTION *conn)
{
	if (!conn) return NULL;
	refmutex.Lock();
	SYBCONNECT *s=first;
	while (s) {
		if (s->conn==conn) {
			refmutex.Unlock();
			return s;
		}
		s=s->next;
	}
	refmutex.Unlock();
	return NULL;
}

/* Handler for server messages. Client-Library will call this
 * routine when it receives a messages from the server.
 */

CS_RETCODE servermsg_callback(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_SERVERMSG *msgp)
{
	SYBCONNECT *sc=FindSybClass(chp);
	CString msg;
	msg.Sprintf("SYBASE Server Msg %ld, Severity %ld, State %ld, Line %ld: %s",
		(long)msgp->msgnumber,
		(long)msgp->severity,
		(long)msgp->state,
		(long)msgp->line,
		msgp->text);
	if (msgp->svrnlen>0) {
		msg.Concatf(", Server ’%s’, ", msgp->svrname);
	}
	if (msgp->proclen>0) {
		msg.Concatf(", Procedure ’%s’, ", msgp->proc);
	}
	if (sc) {
		if (sc->c->Log)	sc->c->Log->Print(LOG::DEBUG,5,(char*)msg.GetPtr());
		sc->c->syberror=msg;
	}

	return (CS_SUCCEED);
}

/* Client-Library error handler. This function will be invoked
 * when a Client-Library has detected an error. Before Client-
 * Library routines return CS_FAIL, this handler will be called
 * with additional error information.
 */

CS_RETCODE clientmsg_callback(CS_CONTEXT *cp, CS_CONNECTION *chp, CS_CLIENTMSG *msgp)
{
	SYBCONNECT *sc=FindSybClass(chp);
	CString msg;
	msg.Sprintf("SYBASE Client Msg %ld, Severity %ld, Origin %ld, Layer %ld: %s",
		(long) CS_NUMBER(msgp->msgnumber),
		(long) CS_SEVERITY(msgp->severity),
		(long) CS_ORIGIN(msgp->msgnumber),
		(long) CS_LAYER(msgp->msgnumber),
		msgp->msgstring);
	if (msgp->osstringlen>0) {
		msg.Concatf(", Operation System error number %ld: ’%s’, ", (long)msgp->osnumber, msgp->osstring);
	}
	if (sc) {
		if (sc->c->Log) sc->c->Log->Print(LOG::DEBUG,5,(char*)msg.GetPtr());
		sc->c->syberrorLong=msg;
		sc->c->syberror.Sprintf("%s",msgp->msgstring);
	}
	return (CS_SUCCEED);
}

/* CS-Library error handler. This function will be invoked
 * when CS-Library has detected an error.
 */

CS_RETCODE csmsg_callback(CS_CONTEXT *cp, CS_CLIENTMSG *msgp)
{
	CString msg;
	msg.Sprintf("SYBASE Client Msg %ld, Severity %ld, Origin %ld, Layer %ld: %s",
		(long) CS_NUMBER(msgp->msgnumber),
		(long) CS_SEVERITY(msgp->severity),
		(long) CS_ORIGIN(msgp->msgnumber),
		(long) CS_LAYER(msgp->msgnumber),
		msgp->msgstring);
	if (msgp->osstringlen>0) {
		msg.Concatf(", Operation System error number %ld: ’%s’, ", (long)msgp->osnumber, msgp->osstring);
	}
	msg.Print();
	return (CS_SUCCEED);
}

/*!\class CDBSybase
 * \brief Klasse zur Verwendung einer Sybase Datenbank
 *
 * \ingroup PPLGroupOldDatabases
 */


CDBSybase::CDBSybase()
/*!\brief Konstruktor der Klasse
 *
 */
{
	connected=false;
	maxrows=0;
	conn=malloc(sizeof(SYBCONNECT));
	if (!conn) {
		SetError(2,"malloc conn");
		return;
	}
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	sc->previous=NULL;
	sc->next=NULL;
	sc->conn=NULL;
	sc->cmd=NULL;
	sc->c=this;

	refmutex.Lock();
	if (syb_refcount==0) {
		CS_RETCODE ret;
		// eine Context-Struktur allokieren
		context=NULL;
		ret=cs_ctx_alloc(ctx_version,&context);
		if (ret!=CS_SUCCEED) {
			SetError(2,"cs_ctx_alloc");
			refmutex.Unlock();
			return;
		};
		// Install a callback function to handle CS-Library errors
		ret=cs_config(context, CS_SET, CS_MESSAGE_CB,(CS_VOID*)csmsg_callback, CS_UNUSED, NULL);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"cs_config, csmsg_callback");
			return;
		}
		// Install a callback function to handle Client-Library errors
		ret=ct_callback(context, NULL, CS_SET, CS_CLIENTMSG_CB,(CS_VOID*) clientmsg_callback);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"ct_callback, clientmsg_callback");
			return;
		}
		// Install a callback function to handle server messages
		ret=ct_callback(context, NULL, CS_SET, CS_SERVERMSG_CB,(CS_VOID*) servermsg_callback);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"ct_callback, servermsg_callback");
			return;
		}

		/*
		 * Locales setzen
		 */

		ret=cs_loc_alloc(context, &locale);
		if (ret!=CS_SUCCEED) {
			cs_ctx_drop(context);
			context=NULL;
			refmutex.Unlock();
			SetError(296,"cs_loc_alloc");
			return;
		}
		if (default_locale) {
			// CS_LC_ALL ist in FreeTDS 0.82 nicht implementiert, und liefert immer CS_FAIL zurück.
			// Daher prüfen wir den erstmal Returncode nicht.
			// TODO: Configure soll ein Define zur Erkennung von FreeTDS liefern, dann kann die
			// Prüfung abhängig vom Flag erfolgen
			ret=cs_locale(context,CS_SET,locale,CS_LC_ALL,default_locale,CS_NULLTERM,NULL);
			/*
			if (ret!=CS_SUCCEED) {
				cs_loc_drop(context,locale);
				locale=NULL;
				cs_ctx_drop(context);
				refmutex.Unlock();
				SetError(296,"cs_locale: %s",default_locale);
				return;
			}
			*/
		}


		/* Set datetime conversion format to "Nov  3 1998  8:06PM".
		 * This is the default format for the ct-lib that comes with
		 * Sybase ASE 11.5.1 for Solaris, but the Linux libraries that
		 * come with 11.0.3.3 default to "03/11/98" which is singularly
		 * useless.  This levels the playing field for all platforms.
		 */
		CS_INT dt_convfmt = CS_DATES_SHORT;
		//CS_INT dt_convfmt = CS_DATES_MDYHMS;

		// Auch diese Funktion ist in FreeTDS nicht implementiert, liefert aber immer SUCCEED zurück
		ret=cs_dt_info(context, CS_SET, NULL, CS_DT_CONVFMT, CS_UNUSED, &dt_convfmt, sizeof(dt_convfmt), NULL);
		if (ret!=CS_SUCCEED) {
			//ct_exit(context, CS_FORCE_EXIT);
			cs_loc_drop(context,locale);
			cs_ctx_drop(context);
			refmutex.Unlock();
			context=NULL;
			SetError(296,"Setting date format");
			return;
		}


		// Datenbank-Context initialisieren
		ret=ct_init(context, ctx_version);
		if (ret!=CS_SUCCEED) {
			cs_loc_drop(context,locale);
			cs_ctx_drop(context);
			context=NULL;
			locale=NULL;
			refmutex.Unlock();
			SetError(296,"ct_init");
			return;
		}

		#ifdef CS_MAX_CONNECT
		if (maxconnects==0) {
			ret=ct_config(context,CS_GET,CS_MAX_CONNECT,&maxconnects,CS_UNUSED,NULL);
		} else {
			ret=ct_config(context,CS_SET,CS_MAX_CONNECT,&maxconnects,CS_UNUSED,NULL);
		}
		#endif
		int timeout=5;
		ret=ct_config(context,CS_SET,CS_LOGIN_TIMEOUT,&timeout,CS_UNUSED,NULL);
		/*
		if (ret==CS_FAIL) {
			printf ("Error\n");
		} else {
			printf ("ok\n");
		}
		ret=ct_config(context,CS_GET,CS_LOGIN_TIMEOUT,&timeout,CS_UNUSED,NULL);
		if (ret==CS_FAIL) {
			printf ("Error\n");
		} else {
			printf ("ok: %i\n",timeout);
		}
		*/

	}
	SetError(0);
	syb_refcount++;
	refmutex.Unlock();
	lastinsertid=0;
}

CDBSybase::~CDBSybase()
/*!\brief Destruktor der Klasse
 *
 */
{
	PushError();
	Close();
	if (conn) free(conn);
	conn=NULL;
	refmutex.Lock();
	syb_refcount--;
	if (syb_refcount==0) {
		ct_exit(context, CS_FORCE_EXIT);
		cs_loc_drop(context,locale);
		cs_ctx_drop(context);
		context=NULL;
		locale=NULL;
	}
	refmutex.Unlock();
	PopError();
}

int CDBSybase::Connect(CAssocArray *data)
/*!\brief Verbindung zur Datenbank herstellen
 *
 * \desc
 * Mit dieser Funktion wird eine Verbindung zur Datenbank hergestellt.
 *
 * \param[in] data Pointer auf ein Assoziatives Array, das sämtliche Connect-Parameter enthält.
 * Folgende Paramater sind anzugeben:
 * <table border=1 cellspacing=0 cellpadding=2>
 * <tr class=table-row><td class=table-header>host</td><td>optional</td><td>Name des Datenbank-Servers. Dieser muss in der Interface-Datei von Sybase bzw.
 * freetds.conf bei FreeTDS definiert sein. Ab Version 6.2.4 der PPl-Library kann alternativ auch eine Kombination
 * aus \b serveraddr und \b port verwendet werden. Wird weder \p host noch \p serveraddr angegeben, wird mit dem in der
 * Interface-Datei angegebenen Default-Server verbunden oder es kommt zu einer Fehlermeldung</td></tr>
 * <tr class=table-row><td class=table-header>serveraddr</td><td>optional</td><td>Falls der Name des Datenbank-Servers nicht mit \b host angegeben wird,
 * kann hiermit alternativ ein FQDN oder die IP des Servers angegeben werden. In diesem Fall muss auch die Variable \p port
 * angegeben werden</td></tr>
 * <tr class=table-row><td class=table-header>port</td><td>optional</td><td>Falls \b serveraddr verwendet wird, muss auch \b port angegeben werden</td></tr>
 * <tr class=table-row><td class=table-header>user</td><td>erforderlich</td><td>Der Benutzername (Login), mit dem man sich am Sybase-Server anmelden möchte</td></tr>
 * <tr class=table-row><td class=table-header>password</td><td>erforderlich</td><td>Das Passwort des Benutzers</td></tr>
 * <tr class=table-row><td class=table-header>dbname</td><td>optional</td><td>Name der Datenbank. Kann auch später noch mit CDBSybase::SelectDB
 * oder per SQL Query "use DATENBANKNAME" gewählt werden.</td></tr>
 * <tr class=table-row><td class=table-header>appname</td><td>optional</td><td>Name der Applikation. Dieser wird an den Sybase-Server
 * übermittelt und kann dort eventuell zu einer einfacheren identifizierung der Prozesse beitragen</td></tr>
 * <tr class=table-row><td class=table-header>packetsize</td><td>optional</td><td>Legt die Maximale Größe für Datenpakete fest, die der Server
 * dem Client schicken darf. Muss ein Wert zwischen 512 und 65535 sein, je nach Konfiguration des Servers kann das
 * Limit jedoch auch weitaus niedriger sein.</td></tr>
 * <tr class=table-row><td class=table-header>tdsversion</td><td>optional</td><td>Überschreibt die in der Interface-Datei bzw. freetds.conf
 * angegebene TDS-Version. Kann folgende Werte enthalten: 4.0, 4.2, 4.6, 4.95 oder 5.0 enthalten</td></tr>
 * <tr class=table-row><td class=table-header>language</td><td>optional</td><td>Legt fest, welche Sprache verwendet
 * werden soll, beispielswiese "us_english" oder "german". Mit der Funktion ppl6::SybaseSetLocale kann aber auch ein
 * Prozessweiter Default gesetzt werden.</td></tr>
 * <tr class=table-row><td class=table-header>dateformat</td><td>optional</td><td>Legt fest, in welcher Reihenfolge
 * Tag, Monat und Jahr angegeben werden. Mögliche Werte sind: mdy, dmy, ymd, ydm, myd, und dym.
 * Mit der Funktion ppl6::SybaseSetLocale kann aber auch ein
 * Prozessweiter Default gesetzt werden.</td></tr>
 *
 * </table>
 *
 * \returns Bei einem erfolgreichen Connect gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
{
	refmutex.Lock();
	if (syb_refcount==0) {
		SetError(296);
		refmutex.Unlock();
		return 0;
	}
	refmutex.Unlock();
	Close();
	if (!data) {
		SetError(194,"Connect(CAssocArray *data)");
		return 0;
	}
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!conn) {
		SetError(2);
		return 0;
	}
	const char *tmp;
	const char *host=data->Get("host");
	tmp=data->Get("port");
	unsigned int port=0;
	if (tmp) port=atoi(tmp);
	const char *user=data->Get("user");
	const char *password=data->Get("password");
	const char *dbname=data->Get("dbname");
	const char *appname=data->Get("appname");
	int packetsize=ppl6::atoi(data->Get("packetsize"));
	const char *serveraddr=data->Get("serveraddr");
	const char *tdsversion=data->Get("tdsversion");
	const char *language=data->Get("language");
	const char *dateformat=data->Get("dateformat");


	//char *charset=data->Get("charset");
	CS_RETCODE ret;
	mutex.Lock();
	sc->conn=NULL;
	// Zuerst eine connection structure allokieren
	ret=ct_con_alloc(context,&sc->conn);
	if (ret!=CS_SUCCEED) {
		SetError(77,"ct_con_alloc failed");
		mutex.Unlock();
		return 0;
	}

	if (user) {
		ret=ct_con_props(sc->conn,CS_SET,CS_USERNAME,(CS_VOID*)user,CS_NULLTERM,NULL);
	}
	if (password) {
		ret=ct_con_props(sc->conn,CS_SET,CS_PASSWORD,(CS_VOID*)password,CS_NULLTERM,NULL);
	}
	//if (host) DBSETLHOST(login, host);
	if (appname) {
		ret=ct_con_props(sc->conn,CS_SET,CS_APPNAME,(CS_VOID*)appname,CS_NULLTERM,NULL);
	}
#ifdef CS_PACKETSIZE
	if (packetsize>=512) {
		ret=ct_con_props(sc->conn,CS_SET,CS_PACKETSIZE,&packetsize,CS_UNUSED,NULL);
	} else {
		int p=2048;
		ret=ct_con_props(sc->conn,CS_SET,CS_PACKETSIZE,&p,CS_UNUSED,NULL);
	}
#endif

#ifdef CS_TDS_VERSION
	if (tdsversion) {
		CString t=tdsversion;
		CS_INT v=0;
		if (t=="4.0") v=CS_TDS_40;
		if (t=="4.2") v=CS_TDS_42;
		if (t=="4.6") v=CS_TDS_46;
		if (t=="4.95") v=CS_TDS_495;
		if (t=="5.0") v=CS_TDS_50;
		if (v) {
			ret=ct_con_props(sc->conn,CS_SET,CS_TDS_VERSION,&v,CS_UNUSED,NULL);
		}

	}

#endif

#ifdef CS_SERVERADDR
	if (serveraddr!=NULL && port>0) {
		CString t;
		t.Setf("%s %i",serveraddr,port);
		ret=ct_con_props(sc->conn,CS_SET,CS_SERVERADDR,(CS_VOID*)t.GetPtr(),CS_NULLTERM,NULL);
	}
#endif

	refmutex.Lock();
	if (sc) {
		if (last) last->next=sc;
		sc->next=NULL;
		sc->previous=last;
		last=sc;
		if (!first) first=sc;
	}
	refmutex.Unlock();

	ret = ct_connect(sc->conn, (CS_CHAR*)host, CS_NULLTERM);
	if (ret!=CS_SUCCEED) {
		//printf ("conn 3\n");
		SetError(77,"Interface: %s, Sybase-Error: %s",host,(const char*)syberror);
		mutex.Unlock();
		return 0;
	}
	sc->cmd=NULL;
	ret = ct_cmd_alloc(sc->conn, &sc->cmd);
	if (ret!=CS_SUCCEED) {
		Close();
		SetError(77,"Command Buffer ct_cmd_alloc");
		mutex.Unlock();
		return 0;
	}

	if (Log) Log->Printf(LOG::DEBUG,3,__FILE__,__LINE__,"SYBASE Connected to %s",host);
	connected=true;
	UpdateLastUse();
	mutex.Unlock();
	if (language) Exec("set language %s",language);
	else if (default_locale) Exec("set language %s",default_locale);
	if (dateformat) Exec("set dateformat %s",dateformat);
	else if (default_dateformat) Exec("set dateformat %s", default_dateformat);

	if (dbname) return SelectDB(dbname);
	return 1;
}

int CDBSybase::Close()
/*\brief Verbindung zur Datenbank trennen
 */
{
	mutex.Lock();
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (sc) {
		refmutex.Lock();
		if (sc->previous) sc->previous->next=sc->next;
		if (sc->next) sc->next->previous=sc->previous;
		if (sc==first) first=sc->next;
		if (sc==last) last=sc->previous;
		refmutex.Unlock();
		sc->next=NULL;
		sc->previous=NULL;
		if (sc->cmd) {
			ct_cmd_drop(sc->cmd);
			sc->cmd=NULL;
		}
		if (sc->conn) {
			ct_close(sc->conn,CS_UNUSED);
			ct_con_drop(sc->conn);
			sc->conn=NULL;
		}
	}
	connected=false;
	lastinsertid=0;
	ClearLastUse();
	mutex.Unlock();
	return 1;
}

int CDBSybase::SelectDB(const char *databasename)
/*!\brief Bestimmte Datenbank selektieren
 *
 */
{
	if (!databasename) {
		SetError(194,"SelectDB(CString *databasename)");
		return 0;
	}
	int ret=0;
	mutex.Lock();
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!sc->conn) {
		SetError(181);
		mutex.Unlock();
		return 0;
	} else {
		mutex.Unlock();
		CString cmdbuf;
		cmdbuf.Sprintf("use %s", databasename);
		PPL_DB_RESULT res;
		if ((res=DoExec(cmdbuf.GetPtr()))!=NULL) {
			ret=1;
			FreeResult(res);
		}
		else SetError(295,"SelectDB(%s)",databasename);
	}
	return ret;
}

int CDBSybase::SelectDB(CString *databasename)
/*!\brief Bestimmte Datenbank selektieren
 *
 */
{
	if (!databasename) {
		SetError(194,"SelectDB(CString *databasename)");
		return 0;
	}
	return SelectDB((const char*)databasename->GetPtr());
}

PPL_DB_RESULT CDBSybase::DoExec(const char *query)
/*!\brief Interne Funktion zur Ausführung eines Queries
 *
 */
{
	double t_start;
	if (Log) Log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CDBSybase::DoExec(const char *query=%s)",query);
	PPL_DB_RESULT ret=NULL;
	mutex.Lock();
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!sc->conn) {
		mutex.Unlock();
		SetError(181);
		if (Log) Log->LogError(4);
		return NULL;
	}
	syberror="";
	t_start=getmicrotime();
	if (ct_command(sc->cmd, CS_LANG_CMD, (CS_CHAR*)query, CS_NULLTERM, CS_UNUSED)!=CS_SUCCEED) {
		SetError(138,"ct_command: %s, syberror: %s",query,(const char*)syberror);
		mutex.Unlock();
		if (Log) Log->LogError(4);
		return NULL;
	}
	if (ct_send(sc->cmd)!=CS_SUCCEED) {
		SetError(138,"ct_send: %s, syberror: %s",query,(const char*)syberror);
		mutex.Unlock();
		if (Log) Log->LogError(4);
		return NULL;
	}
	UpdateLastUse();
	CS_RETCODE retcode;
	CS_INT restype;
	int status=0;
	while ((retcode = ct_results(sc->cmd, &restype))==CS_SUCCEED) {
		switch ((int) restype) {
			case CS_CMD_FAIL:
			default:
                status = PPLSYB_FAILURE;
				ret=NULL;
                break;
            case CS_CMD_SUCCEED:
            case CS_CMD_DONE: {
					CS_INT row_count;
					if (ct_res_info(sc->cmd, CS_ROW_COUNT, &row_count, CS_UNUSED, NULL)==CS_SUCCEED) {
						affected_rows = (int)row_count;
					}
					if (!ret) ret=(PPL_DB_RESULT)1;
				}
				/* Fall through */
			case CS_COMPUTEFMT_RESULT:
			case CS_ROWFMT_RESULT:
			case CS_DESCRIBE_RESULT:
			case CS_MSG_RESULT:
				//buffered= 0;                /* These queries have no need for buffering */
				status = PPLSYB_DONE;
				break;
			case CS_COMPUTE_RESULT:
			case CS_CURSOR_RESULT:
			case CS_PARAM_RESULT:
			case CS_ROW_RESULT:
			case CS_STATUS_RESULT:
				ret = FetchResult(query);
				if (ret == NULL) {
					ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL);
				}
				status = PPLSYB_RESULTS;
				break;
		}

		if (status==PPLSYB_FAILURE) {
            ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL);
			SetError(138,"ct_results: %s, syberror: %s",query,(const char*)syberror);
			mutex.Unlock();
			if (Log) Log->LogError(4);
			return NULL;
        }
    }
	LogQuery(query,(float)(getmicrotime()-t_start));
	UpdateLastUse();
	mutex.Unlock();
	if (Log) Log->Printf(LOG::DEBUG,4,__FILE__,__LINE__,"CDBSybase::DoExec OK)");
	return ret;
}

PPL_DB_RESULT CDBSybase::FetchResult(const char *query)
/*!\brief Interne Funktion zum Auslesen der Server-Antwort
 *
 */
{
	//char *fname;
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	int num_fields;
	CS_INT retcode;
	if (Log) Log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"CDBSybase::FetchResult(const char *query=%s)",query);
	/* The following (if unbuffered) is more or less the equivalent of mysql_store_result().
	 * fetch all rows from the server into the row buffer, thus:
	 * 1)  Being able to fire up another query without explicitly reading all rows
	 * 2)  Having numrows accessible
	 */
	if (ct_res_info(sc->cmd, CS_NUMDATA, &num_fields, CS_UNUSED, NULL)!=CS_SUCCEED) {
		return NULL;
	}
	CDBResult *res=new CDBResult();
	if (!res) {
		SetError(2);
		return NULL;
	}

	sybase_result *result;
	result = (sybase_result *) res->Malloc(sizeof(sybase_result));

	res->SetNumFields(num_fields);
	result->tmp_buffer = (char **) res->Calloc(sizeof(char *)*num_fields);
	result->datafmt = (CS_DATAFMT *) res->Calloc(sizeof(CS_DATAFMT)*num_fields);
	result->ppl_type = (int *) res->Calloc(sizeof(int)*num_fields);
	result->lengths = (CS_INT *) res->Calloc(sizeof(CS_INT)*num_fields);
	result->indicators = (CS_SMALLINT *) res->Calloc(sizeof(CS_INT)*num_fields);
	result->numerics = (unsigned char *) res->Calloc(sizeof(unsigned char)*num_fields);

	for (int i=0; i<num_fields; i++) {
		ct_describe(sc->cmd, i+1, &result->datafmt[i]);
		result->ppl_type[i]=DBFIELDTYPE::UNKNOWN;
        switch (result->datafmt[i].datatype) {
            case CS_CHAR_TYPE:
            case CS_VARCHAR_TYPE:
            case CS_TEXT_TYPE:
            case CS_IMAGE_TYPE:
                result->datafmt[i].maxlength++;
				result->ppl_type[i]=DBFIELDTYPE::STRING;
                //result->numerics[i] = 0;
                break;
            case CS_BINARY_TYPE:
            case CS_VARBINARY_TYPE:
                result->datafmt[i].maxlength *= 2;
                result->datafmt[i].maxlength++;
                //result->numerics[i] = 0;
				result->ppl_type[i]=DBFIELDTYPE::BINARY;
                break;
            case CS_BIT_TYPE:
            case CS_TINYINT_TYPE:
                result->datafmt[i].maxlength = 4;
				result->ppl_type[i]=DBFIELDTYPE::INT;
                //result->numerics[i] = 1;
                break;
            case CS_SMALLINT_TYPE:
                result->datafmt[i].maxlength = 7;
				result->ppl_type[i]=DBFIELDTYPE::INT;
                //result->numerics[i] = 1;
                break;
            case CS_INT_TYPE:
                result->datafmt[i].maxlength = 12;
				result->ppl_type[i]=DBFIELDTYPE::INT;
                //result->numerics[i] = 1;
                break;
            case CS_REAL_TYPE:
            case CS_FLOAT_TYPE:
                result->datafmt[i].maxlength = 24;
				result->ppl_type[i]=DBFIELDTYPE::FLOAT;
                //result->numerics[i] = 2;
                break;
            case CS_MONEY_TYPE:
            case CS_MONEY4_TYPE:
                result->datafmt[i].maxlength = 24;
				result->ppl_type[i]=DBFIELDTYPE::FLOAT;
                //result->numerics[i] = 2;
                break;
            case CS_DATETIME_TYPE:
            case CS_DATETIME4_TYPE:
                result->datafmt[i].maxlength = 30;
				result->ppl_type[i]=DBFIELDTYPE::INT;
                //result->numerics[i] = 0;
                break;
            case CS_NUMERIC_TYPE:
            case CS_DECIMAL_TYPE:
                result->datafmt[i].maxlength = result->datafmt[i].precision + 3;
				result->ppl_type[i]=DBFIELDTYPE::FLOAT;
                /* numeric(10) vs numeric(10, 1) */
                //result->numerics[i] = (result->datafmt[i].scale == 0) ? 3 : 2;
                break;
            default:
                result->datafmt[i].maxlength++;
				result->ppl_type[i]=DBFIELDTYPE::STRING;
                //result->numerics[i] = 0;
                break;
        }
		result->tmp_buffer[i] = (char *)res->Malloc(result->datafmt[i].maxlength);
        result->datafmt[i].datatype = CS_CHAR_TYPE;
        result->datafmt[i].format = CS_FMT_NULLTERM;
        ct_bind(sc->cmd, i+1, &result->datafmt[i], result->tmp_buffer[i], &result->lengths[i], &result->indicators[i]);
		//res->SetFieldName(i,result->datafmt[i].name,result->datafmt[i].namelen,result->ppl_type[i]);
		res->SetFieldName(i,result->datafmt[i].name,result->datafmt[i].namelen,DBFIELDTYPE::STRING);


		if (Log) Log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"Field %i: Type: %i, Name: %s, Maxlength: %i",
			i,result->ppl_type[i],res->GetFieldName(i),result->datafmt[i].maxlength);
	}

	int z=0;
	while ((retcode=ct_fetch(sc->cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL))==CS_SUCCEED) {
		res->NewRow();
		for (int i=0; i<num_fields; i++) {
			if (Log) Log->Printf(LOG::DEBUG,10,__FILE__,__LINE__,"Feld %i, Length: %i: >>>%s<<<",i,result->lengths[i],result->tmp_buffer[i]);
			res->StoreField(i,result->tmp_buffer[i],result->lengths[i]);
		}
		z++;
		if (maxrows>0 && (ppldd)z>=maxrows) break;
	}
	res->SetAffectedRows(z);
	res->BuildIndex();
	if (Log) Log->Printf(LOG::DEBUG,5,__FILE__,__LINE__,"Allocated Memory: %llu Bytes",res->GetUsedMem());
	return (PPL_DB_RESULT*)res;
}

int CDBSybase::StoreField(CDBResult *res, int field, int type, void *data, int size)
/*!\brief Hat keine Funktionalität
 *
 */
{
	return 0;
}



ppldd CDBSybase::NumRows(PPL_DB_RESULT result)
/*!\brief Liefert die Anzahl Ergebnisse nach einem Query zurück
 *
 */
{
	if (result==(void*)1) return 0;
	CDBResult *res=(CDBResult*)result;
	return res->NumRows();
}

ppldd CDBSybase::NumFields(PPL_DB_RESULT result)
/*!\brief Liefert die Anzahl Felder/Spalten eines Results zurück
 *
 */
{
	if (result==(void*)1) return 0;
	CDBResult *res=(CDBResult*)result;
	return res->NumFields();
}

int CDBSybase::FreeResult(PPL_DB_RESULT result)
/*!\brief Ergebnis eines Results freigeben
 *
 * \desc
 * Diese Funktion muss aufgerufen werden, um den durch die CDBSybase::Exec-Funktion
 * reservierten Speicehr für das Ergebnis wieder freizugeben.
 */
{
	if (result==(void*)1) return 1;
	CDBResult *res=(CDBResult*)result;
	delete res;
	return 1;
}

char *CDBSybase::Result(PPL_DB_RESULT result, ppldd row, char *field)
/*!\brief Wert aus einem Query-Ergebnis auslesen
 *
 */
{
	if (result==(void*)1) return NULL;
	CDBResult *res=(CDBResult*)result;
	return (char*)res->Result(row,(const char*)field);
}

char *CDBSybase::ResultByNo(PPL_DB_RESULT result, ppldd row, ppldd field)
/*!\brief Gegenwärtig nicht implementiert!
 *
 */
{
	if (result==(void*)1) return NULL;
	CDBResult *res=(CDBResult*)result;
	return (char*)res->Result(row,field);
}

ppldds CDBSybase::FieldNum(PPL_DB_RESULT result, char *field)
{
	if (result==(void*)1) return -1;
	CDBResult *res=(CDBResult*)result;
	return res->GetFieldNum((const char*)field);
}

const char *CDBSybase::FieldName(PPL_DB_RESULT result, int field)
{
	if (result==(void*)1) return NULL;
	CDBResult *res=(CDBResult*)result;
	return res->GetFieldName(field);
}

int CDBSybase::FieldType(PPL_DB_RESULT result, int field)
{
	if (result==(void*)1) return 0;
	CDBResult *res=(CDBResult*)result;
	return res->GetFieldType(field);
}

int CDBSybase::FieldType(PPL_DB_RESULT result, char *field)
{
	if (result==(void*)1) return 0;
	CDBResult *res=(CDBResult*)result;
	return res->GetFieldType(field);
}

CAssocArray *CDBSybase::FetchArray(PPL_DB_RESULT res, ppldds nr)
{
	CAssocArray *result=new CAssocArray;
	if (!result) {
		SetError(2);
		return NULL;
	}
	if (FetchArray(result,res,nr)) return result;
	delete result;
	return NULL;
}

int CDBSybase::FetchArray(CAssocArray *array, PPL_DB_RESULT result, ppldds row)
{
	if (result==(void*)1) {
		SetError(182);
		return 0;
	}
	if (!array) {
		SetError(194,"CAssocArray *array");
		return 0;
	}
	CDBResult *res=(CDBResult*)result;
	if (row>=0) return res->FetchArray(array,row);
	return res->FetchArray(array);
}

void CDBSybase::PrintResult(PPL_DB_RESULT res)
{
}

int CDBSybase::Ping()
{
	ppluint64 tmp=lastuse;
	PPL_DB_RESULT res=Exec("select 1 as result");
	lastuse=tmp;
	if (!res) return 0;
	FreeResult(res);
	UpdateLastPing();
	return 1;
}

char *CDBSybase::EscapeString(char *string, size_t length)
{
	return NULL;
}

ppld64 CDBSybase::GetInsertID()
{
	return 0;
}

ppldd CDBSybase::Errno()
{
	return 0;
}

const char *CDBSybase::Error()
{
	return 0;
}

int CDBSybase::NumAffectedRows(PPL_DB_RESULT result)
{
	if (result==(void*)1) return 0;
	CDBResult *res=(CDBResult*)result;
	return res->NumAffectedRows();
}

void CDBSybase::SetMaxRows(ppldd rows)
{
	maxrows=rows;
}

#endif		// #ifdef HAVE_SYBASE

}	// EOF namespace ppl6




