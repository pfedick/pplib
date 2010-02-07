/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.13 $
 * $Date: 2009/08/19 22:33:12 $
 * $Id: Sybase.cpp,v 1.13 2009/08/19 22:33:12 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2009, Patrick Fedick <patrick@pfp.de>
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
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include "ppl6.h"
#include "ppl6-db.h"

#ifdef HAVE_SYBASE
#include <ctpublic.h>
#endif

namespace ppl6 {
namespace db {

/*!\class Sybase
 * \ingroup PPLGroupDatabases
 * \brief Implementierung einer Sybase-Datenbank
 *
 * \header \#include <ppl6-db.h>
 *
 * \descr
 * Mit dieser Klasse kann eine Verbindung zu einer Sybase-Datenbank aufgebaut werden, um darüber
 * SQL-Queries durchzuführen.
 *
 * \example
 * \dontinclude db_examples.cpp
 * \skip DB_Sybase_Example1
 * \until EOF
 */


#define PPLSYB_DONE 1
#define PPLSYB_FAILURE 2
#define PPLSYB_RESULTS 3

#ifdef HAVE_SYBASE
static int syb_refcount=0;
static unsigned int ctx_version=CS_VERSION_100;
static CMutex refmutex;
static int maxconnects=0;
static CString default_locale;
static CString default_dateformat;
#endif


#ifdef HAVE_SYBASE
static CS_CONTEXT				*context=NULL;
static CS_LOCALE				*locale=NULL;

typedef struct tagSybConnect {
	struct tagSybConnect	*previous;
	struct tagSybConnect	*next;
	CS_CONNECTION			*conn;
	CS_COMMAND				*cmd;
	Sybase					*c;
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
		CLog *log=sc->c->GetLogfile();
		if (log) log->Print(LOG::DEBUG,5,"ppl6::db::Sybase","servermsg_callback",__FILE__,__LINE__,(const char*)msg.GetPtr());
		sc->c->lastServerMsgNumber=msgp->msgnumber;
		sc->c->syberrorLong=msg;
		sc->c->syberrorLong.Replace("\n"," ");
		sc->c->syberror.Sprintf("%s",msgp->text);
		sc->c->syberror.Replace("\n"," ");
		//printf ("syberrorLong: >>%s<<, syberror: >>%s<<\n",(const char*)sc->c->syberrorLong, (const char*)sc->c->syberror);
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
		CLog *log=sc->c->GetLogfile();
		if (log) log->Print(LOG::DEBUG,5,"ppl6::db::Sybase","clientmsg_callback",__FILE__,__LINE__,(const char*)msg.GetPtr());
		sc->c->lastClientMsgNumber=CS_NUMBER(msgp->msgnumber);
		sc->c->syberrorLong=msg;
		sc->c->syberrorLong.Replace("\n"," ");
		sc->c->syberror.Sprintf("%s",msgp->msgstring);
		sc->c->syberror.Replace("\n"," ");
		//printf ("syberrorLong: >>%s<<, syberror: >>%s<<\n",(const char*)sc->c->syberrorLong, (const char*)sc->c->syberror);
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


#endif

Sybase::Sybase()
{
#ifdef HAVE_SYBASE
	lastServerMsgNumber=0;
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
		if (default_locale.NotEmpty()) {
			// CS_LC_ALL ist in FreeTDS 0.82 nicht implementiert, und liefert immer CS_FAIL zurück.
			// Daher prüfen wir den erstmal Returncode nicht.
			// TODO: Configure soll ein Define zur Erkennung von FreeTDS liefern, dann kann die
			// Prüfung abhängig vom Flag erfolgen
			ret=cs_locale(context,CS_SET,locale,CS_LC_ALL,(CS_VOID*)default_locale.GetPtr(),CS_NULLTERM,NULL);
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
#endif
}

Sybase::~Sybase()
{
#ifdef HAVE_SYBASE
	PushError();
	Close();
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","~Sybase",__FILE__,__LINE__,"Objekt wird gelöscht");
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
#endif
}

int Sybase::Connect(const CAssocArray &params)
/*!\brief Verbindung zur Datenbank herstellen
 *
 * \desc
 * Mit dieser Funktion wird eine Verbindung zu einer Sybase-Datenbank hergestellt.
 *
 * \param[in] params Ein Assoziatives Array, das sämtliche Connect-Parameter enthält.
 * Folgende Paramater sind anzugeben:
 * <table border=1 cellspacing=0 cellpadding=2>
 * <tr class=table-row><td class=table-header>interface</td><td>optional</td><td>Der Name des Datenbank-Servers, wie er in der freetds.conf oder der Sybase Interface-Datei
 * definiert ist. Statt des Interface-Namens kann auch eine Kombination aus \p host und \p port verwendet werden.
 * Wird weder \p host noch \p interface angegeben, wird mit dem in der Interface-Datei angegebenen Default-Server verbunden oder
 * es kommt zu einer Fehlermeldung</td></tr>
 * <tr class=table-row><td class=table-header>host</td><td>optional</td><td>Falls der Interface-Name des Datenbank-Servers nicht mit \p interface angegeben wird,
 * kann hiermit alternativ ein FQDN oder die IP des Servers angegeben werden. In diesem Fall muss auch die Variable \p port
 * angegeben werden</td></tr>
 * <tr class=table-row><td class=table-header>port</td><td>optional</td><td>Falls \p host verwendet wird, muss auch \p port angegeben werden</td></tr>
 * <tr class=table-row><td class=table-header>user</td><td>erforderlich</td><td>Der Benutzername (Login), mit dem man sich am Sybase-Server anmelden möchte</td></tr>
 * <tr class=table-row><td class=table-header>password</td><td>erforderlich</td><td>Das Passwort des Benutzers</td></tr>
 * <tr class=table-row><td class=table-header>dbname</td><td>optional</td><td>Name der Datenbank. Kann auch später noch mit Sybase::SelectDB
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
 *
 * \note
 * Das Verhalten des Parameters \p host hat sich im Vergleich zu der bisherigen Klasse CDBSybase verändert. Früher wurde hier der Name
 * des Interfaces erwartet (dieser muss nun mit dem Parameter \p interface angegeben werden), jetzt ist es der Hostname bzw. IP-Adresse
 * des Servers. Diese Änderung wurde gemacht, um die Connect-Parameter zu vereinheitlichen.
 */
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	refmutex.Lock();
	if (syb_refcount==0) {
		SetError(296);
		refmutex.Unlock();
		return 0;
	}
	refmutex.Unlock();
	Close();
	condata=params;
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!conn) {
		SetError(2);
		return 0;
	}
	const char *tmp;
	const char *interface=params.Get("interface");
	const char *host=params.Get("host");
	tmp=params.Get("port");
	unsigned int port=0;
	if (tmp) port=atoi(tmp);
	const char *user=params.Get("user");
	const char *password=params.Get("password");
	const char *dbname=params.Get("dbname");
	const char *appname=params.Get("appname");
	int packetsize=ppl6::atoi(params.Get("packetsize"));
	const char *tdsversion=params.Get("tdsversion");
	const char *language=params.Get("language");
	const char *dateformat=params.Get("dateformat");


	//char *charset=data->Get("charset");
	CS_RETCODE ret;
	mutex.Lock();
	sc->conn=NULL;
	// Zuerst eine connection structure allokieren
	lastServerMsgNumber=0;
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
	if (host!=NULL && port>0) {
		CString t;
		t.Setf("%s %i",host,port);
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
	lastServerMsgNumber=0;
	lastClientMsgNumber=0;
	ret = ct_connect(sc->conn, (CS_CHAR*)interface, CS_NULLTERM);
	if (ret!=CS_SUCCEED) {
		int e=77;
		switch (lastServerMsgNumber) {
			case 4002:
				e=542;
				syberror="Login failed";
				break;
			default:
				e=77;
		};
		//printf ("conn 3\n");
		SetError(e,"Host: %s, Port: %i, Interface: %s, Sybase-Error: %s",host,port,interface,(const char*)syberror);
		mutex.Unlock();
		return 0;
	}
	sc->cmd=NULL;
	lastServerMsgNumber=0;
	lastClientMsgNumber=0;
	ret = ct_cmd_alloc(sc->conn, &sc->cmd);
	if (ret!=CS_SUCCEED) {
		Close();
		SetError(77,"Command Buffer ct_cmd_alloc");
		mutex.Unlock();
		return 0;
	}
	CLog *log=GetLogfile();
	if (log) log->Printf(LOG::DEBUG,3,__FILE__,__LINE__,"SYBASE Connected to %s",host);
	connected=true;
	UpdateLastUse();
	mutex.Unlock();
	if (language) Execf("set language %s",language);
	else if (default_locale.NotEmpty()) Execf("set language %s",(const char*)default_locale);
	if (dateformat) Execf("set dateformat %s",dateformat);
	else if (default_dateformat.NotEmpty()) Execf("set dateformat %s", (const char*)default_dateformat);

	if (dbname) return SelectDB(dbname);
	return 1;
#endif
}

int Sybase::Reconnect()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	Close();
	CAssocArray a;
	a.Copy(&condata);
	if (Connect(a)) return 1;
	SetError(299,"Reconnect failed");
	return 0;
#endif
}

int Sybase::Disconnect()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	mutex.Lock();
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Disconnect",__FILE__,__LINE__,"Trenne Verbindung");

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
		if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","Disconnect",__FILE__,__LINE__,"Verbindung wurde getrennt");
	} else {
		if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","Disconnect",__FILE__,__LINE__,"Es war keine Verbindung vorhanden");
	}
	connected=false;
	lastinsertid=0;
	ClearLastUse();
	mutex.Unlock();
	return 1;
#endif
}

int Sybase::SelectDB(const char *databasename)
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
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
		if (Exec(cmdbuf.GetPtr())) {
			ret=1;
		}
		else SetError(295,"%s",databasename);
	}
	return ret;
#endif
}

int Sybase::Exec(const CString &query)
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	Result *res=Query(query);
	if (!res) return 0;
	delete res;
	return 1;
#endif
}

Result *Sybase::Query(const CString &query)
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	double t_start;
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Query",__FILE__,__LINE__,"Sybase::Query(const char *query=%s)",(const char *)query);

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
	if (ct_command(sc->cmd, CS_LANG_CMD, (CS_CHAR*)query.GetPtr(), CS_NULLTERM, CS_UNUSED)!=CS_SUCCEED) {
		SetError(138,"ct_command: %s, syberror: %s",(const char*)query,(const char*)syberror);
		mutex.Unlock();
		if (Log) Log->LogError(4);
		return NULL;
	}
	if (ct_send(sc->cmd)!=CS_SUCCEED) {
		SetError(138,"ct_send: %s, syberror: %s",(const char*)query,(const char*)syberror);
		mutex.Unlock();
		if (Log) Log->LogError(4);
		return NULL;
	}
	UpdateLastUse();
	CS_RETCODE retcode;
	CS_INT restype;
	int status=0;
	GenericResult *ret=new GenericResult;
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
						affectedrows = (pplint64)row_count;
						ret->SetAffectedRows((pplint64)row_count);
					}
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
				if (!FetchResult(ret,query)) {
					ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL);
				}
				status = PPLSYB_RESULTS;
				break;
		}

		if (status==PPLSYB_FAILURE) {
            ct_cancel(NULL, sc->cmd, CS_CANCEL_ALL);
			SetError(138,"ct_results: %s, syberror: %s",(const char*)query,(const char*)syberror);
			mutex.Unlock();
			if (Log) Log->LogError(4);
			return NULL;
        }
    }
	LogQuery(query,(float)(getmicrotime()-t_start));
	UpdateLastUse();
	mutex.Unlock();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Query",__FILE__,__LINE__,"Sybase::Query OK)");
	return (Result*) ret;

#endif
}

int Sybase::FetchResult(Result *res, const char *query)
/*!\brief Interne Funktion zum Auslesen der Server-Antwort
 *
 */
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	if (!res) {
		SetError(2);
		return 0;
	}
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	GenericResult *sybres=(GenericResult *)res;
	int num_fields;
	CS_INT retcode;
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,5,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Sybase::FetchResult(Result *res, const char *query=%s)",query);
	/* The following (if unbuffered) is more or less the equivalent of mysql_store_result().
	 * fetch all rows from the server into the row buffer, thus:
	 * 1)  Being able to fire up another query without explicitly reading all rows
	 * 2)  Having numrows accessible
	 */
	if (ct_res_info(sc->cmd, CS_NUMDATA, &num_fields, CS_UNUSED, NULL)!=CS_SUCCEED) {
		return 0;
	}

	sybase_result *result;
	result = (sybase_result *) sybres->Malloc(sizeof(sybase_result));

	sybres->SetNumFields(num_fields);
	result->tmp_buffer = (char **) sybres->Calloc(sizeof(char *)*num_fields);
	result->datafmt = (CS_DATAFMT *) sybres->Calloc(sizeof(CS_DATAFMT)*num_fields);
	result->ppl_type = (int *) sybres->Calloc(sizeof(int)*num_fields);
	result->lengths = (CS_INT *) sybres->Calloc(sizeof(CS_INT)*num_fields);
	result->indicators = (CS_SMALLINT *) sybres->Calloc(sizeof(CS_INT)*num_fields);
	result->numerics = (unsigned char *) sybres->Calloc(sizeof(unsigned char)*num_fields);

	for (int i=0; i<num_fields; i++) {
		ct_describe(sc->cmd, i+1, &result->datafmt[i]);
		result->ppl_type[i]=Result::Unknown;
        switch (result->datafmt[i].datatype) {
            case CS_CHAR_TYPE:
            case CS_VARCHAR_TYPE:
            case CS_TEXT_TYPE:
            case CS_IMAGE_TYPE:
                result->datafmt[i].maxlength++;
				result->ppl_type[i]=Result::String;
                //result->numerics[i] = 0;
                break;
            case CS_BINARY_TYPE:
            case CS_VARBINARY_TYPE:
                result->datafmt[i].maxlength *= 2;
                result->datafmt[i].maxlength++;
                //result->numerics[i] = 0;
				result->ppl_type[i]=Result::Binary;
                break;
            case CS_BIT_TYPE:
            case CS_TINYINT_TYPE:
                result->datafmt[i].maxlength = 4;
				result->ppl_type[i]=Result::Integer;
                //result->numerics[i] = 1;
                break;
            case CS_SMALLINT_TYPE:
                result->datafmt[i].maxlength = 7;
				result->ppl_type[i]=Result::Integer;
                //result->numerics[i] = 1;
                break;
            case CS_INT_TYPE:
                result->datafmt[i].maxlength = 12;
				result->ppl_type[i]=Result::Integer;
                //result->numerics[i] = 1;
                break;
            case CS_REAL_TYPE:
            case CS_FLOAT_TYPE:
                result->datafmt[i].maxlength = 24;
				result->ppl_type[i]=Result::Decimal;
                //result->numerics[i] = 2;
                break;
            case CS_MONEY_TYPE:
            case CS_MONEY4_TYPE:
                result->datafmt[i].maxlength = 24;
				result->ppl_type[i]=Result::Decimal;
                //result->numerics[i] = 2;
                break;
            case CS_DATETIME_TYPE:
            case CS_DATETIME4_TYPE:
                result->datafmt[i].maxlength = 30;
				result->ppl_type[i]=Result::Integer;
                //result->numerics[i] = 0;
                break;
            case CS_NUMERIC_TYPE:
            case CS_DECIMAL_TYPE:
                result->datafmt[i].maxlength = result->datafmt[i].precision + 3;
				result->ppl_type[i]=Result::Decimal;
                /* numeric(10) vs numeric(10, 1) */
                //result->numerics[i] = (result->datafmt[i].scale == 0) ? 3 : 2;
                break;
            default:
                result->datafmt[i].maxlength++;
				result->ppl_type[i]=Result::String;
                //result->numerics[i] = 0;
                break;
        }
		result->tmp_buffer[i] = (char *)sybres->Malloc(result->datafmt[i].maxlength);
        result->datafmt[i].datatype = CS_CHAR_TYPE;
        result->datafmt[i].format = CS_FMT_NULLTERM;
        ct_bind(sc->cmd, i+1, &result->datafmt[i], result->tmp_buffer[i], &result->lengths[i], &result->indicators[i]);
		//res->SetFieldName(i,result->datafmt[i].name,result->datafmt[i].namelen,result->ppl_type[i]);
		sybres->SetFieldName(i,result->datafmt[i].name,result->datafmt[i].namelen,Result::String);


		if (Log) Log->Printf(LOG::DEBUG,5,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Field %i: Type: %i, Name: %s, Maxlength: %i",
			i,result->ppl_type[i],sybres->FieldName(i),result->datafmt[i].maxlength);
	}

	int z=0;
	while ((retcode=ct_fetch(sc->cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL))==CS_SUCCEED) {
		sybres->NewRow();
		for (int i=0; i<num_fields; i++) {
			if (Log) Log->Printf(LOG::DEBUG,10,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Feld %i, Length: %i: >>>%s<<<",i,result->lengths[i],result->tmp_buffer[i]);
			sybres->StoreField(i,result->tmp_buffer[i],result->lengths[i]);
		}
		z++;
		if (maxrows>0 && (ppldd)z>=maxrows) break;
	}
	sybres->SetAffectedRows(z);
	sybres->BuildIndex();
	if (Log) Log->Printf(LOG::DEBUG,5,"ppl6::db::Sybase","FetchResult",__FILE__,__LINE__,"Allocated Memory: %llu Bytes",sybres->GetUsedMem());
	return 1;
#endif
}


void Sybase::SetMaxRows(ppluint64 rows)
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
#else
	maxrows=rows;
#endif
}

int Sybase::Ping()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	CString query="select 1 as result";
	CLog *Log=GetLogfile();
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Ping",__FILE__,__LINE__,"Prüfe Verbindung");
	double t_start;
	mutex.Lock();
	SYBCONNECT *sc=(SYBCONNECT*)conn;
	if (!sc->conn) {
		mutex.Unlock();
		SetError(181);
		if (Log) Log->LogError(4);
		return 0;
	}
	syberror="";
	t_start=getmicrotime();
	if (ct_command(sc->cmd, CS_LANG_CMD, (CS_CHAR*)query.GetPtr(), CS_NULLTERM, CS_UNUSED)!=CS_SUCCEED) {
		SetError(138,"ct_command: %s, syberror: %s",(const char*)query,(const char*)syberror);
		mutex.Unlock();
		if (Log) Log->LogError(4);
		return 0;
	}
	if (ct_send(sc->cmd)!=CS_SUCCEED) {
		SetError(138,"ct_send: %s, syberror: %s",(const char*)query,(const char*)syberror);
		mutex.Unlock();
		if (Log) Log->LogError(4);
		return 0;
	}
	if (Log) Log->Printf(LOG::DEBUG,4,"ppl6::db::Sybase","Ping",__FILE__,__LINE__,"Ok");
	mutex.Unlock();
	return 1;
#endif
}

int Sybase::Escape(CString &str)
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	// Sybase hat keine Escape-Funktion, daher müssen wir das selbst machen
	CString n;
	const char *tmp=str.GetPtr();
	int c;
	while ((c=tmp[0])) {
		if (c=='\'') n+="'";
		n+=c;
		tmp++;
	}
	str=n;
	return 0;
#endif
}

ppluint64 Sybase::GetInsertID()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	return 0;
#endif
}

pplint64 Sybase::GetAffectedRows()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	return 0;
#endif
}

int Sybase::StartTransaction()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	if (Exec("BEGIN TRANSACTION")) return 1;
	return 0;
#endif
}

int Sybase::EndTransaction()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	if (Exec("COMMIT TRANSACTION")) return 1;
	return 0;
#endif
}

int Sybase::CancelTransaction()
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	if (Exec("ROLLBACK TRANSACTION")) return 1;
	return 0;
#endif
}

int Sybase::CreateDatabase(const char *name)
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	return 0;
#endif
}


/*
 * Statische Member-Funktionen
 */

int	Sybase::GetMaxConnects()
/*!\ingroup PPLGroupDatabases
 * \brief Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken auslesen
 *
 * \descr
 * Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken auslesen
 *
 * \return Liefert die maximale Anzahl gleichzeitig erlaubter Connects zurück.
 *
 */
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	return maxconnects;
#endif
}

int	Sybase::SetMaxConnects(int max)
/*!\ingroup PPLGroupDatabases
 * \brief Anzahl maximaler gleichzeitiger Connects auf Sybase-Datenbanken festlegen
 *
 * \descr
 * Mit dieser Funktion wird die maximale Anzahl gleichzeitiger Connects auf Sybase-Datenbanken
 * festgelegt. Der Default ist unlimitiert.
 *
 * \param[in] max
 *
 * \remarks
 * Die Funktion muss aufgerufen werden, bevor die erste Sybase-Klasse instantiiert wird!
 */
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	if (maxconnects>0) return 0;
	maxconnects=max;
	return 1;
#endif
}

int Sybase::SetVersion(int version)
/*!\brief Zu verwendende Version definieren
 *
 * \ingroup PPLGroupDatabases
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
 * Die Funktion muss aufgerufen werden, bevor die erste Sybase-Klasse instantiiert wird!
 */
{
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
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
#endif
}

int Sybase::SetLocale(const char *locale, const char *dateformat)
/*!\brief Legt Ländereinstellungen fest
 *
 * \ingroup PPLGroupDatabases
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
 * Die Funktion muss aufgerufen werden, bevor die erste Sybase-Klasse instantiiert wird!
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
#ifndef HAVE_SYBASE
	SetError(511,"Sybase");
	return 0;
#else
	default_locale.Clear();
	if (locale) default_locale=locale;
	default_dateformat.Clear();
	if (dateformat) default_dateformat=dateformat;
	return 1;
#endif
}


}	// EOF namespace db
}	// EOF namespace ppl6
