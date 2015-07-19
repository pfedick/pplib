/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.18 $
 * $Date: 2009/10/22 11:25:43 $
 * $Id: CSendMail.cpp,v 1.18 2009/10/22 11:25:43 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
    #include <winsock.h>
	//#include <Ws2tcpip.h>
#else
	#ifdef HAVE_NETDB_H
    #include <netdb.h>
	#endif
#endif

#include "ppl6.h"

namespace ppl6 {


/*!\class CSendMail
 * \ingroup PPLGroupInternet
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Klasse können einfache Emails ohne Attachement per SMTP an einen
 * beliebigen Mailserver zugestellt werden.
 *
 * \example
 * Beispiel zur Verwendung der Klasse:
 * \code
int main(int argc, char **argv)
{
	ppl6::CSendMail Mail;
	ppl6::CString d;
	ppl6::MkISO8601Date(&d);
	Mail.SetHeader("To","patrick@pfp.de");
	Mail.SetHeader("To","irgendwer@example.com");
	Mail.SetHeader("Cc","nochjemand@pfp.de");
	Mail.SetHeader("From","patrick@pfp.de");
	Mail.SetHeader("Subject","Testmail %s",(char*)d);
	Mail.Message.Set("Dies ist eine Testmail");
	Mail.SetServer("mail.pfp.de:smtp");
	if (!Mail.Send()) {
		ppl6::PrintError();
		return 0;
	}
	printf ("ok\n");
	return 0;
}
\endcode
 * \since Diese Klasse wurde mit Version 6.0.18 eingeführt
 */

/*!\var CSendMail::Message
 * \brief String-Variable für den Body der Mail
 *
 * Diese Variable enthält den Body der Mail. Sie kann entweder direkt beschrieben
 * werden oder mittels folgender Funktionen:
 * - CSendMail::SetMessage(const char *message)
 * - CSendMail::SetMessage(CString *message)
 * - CSendMail::SetMessage(CString message)
 */


/*!\brief Konstruktor der Klasse
 * \header \#include <ppl6.h>
 * \desc
 *
 * Der Konstruktor initialisiert gegenwärtig nur zwei private Variablen:
 *
 * - SMTPServer wird mit dem String "localhost:smtp" initialisiert. Der Wert
 *   kann mit derFunktion SetServer überschrieben werden.
 * - MyHostname wird mit dem Wert der Environment-Variablen "HOSTNAME"
 *   initialisiert. Dieser Wert kann mit der Funktion SetLocalHostName
 *   überschrieben werden.
 */
CSendMail::CSendMail()
{
#ifdef HAVE_GETHOSTNAME
	char h[256];
	if (gethostname(h,255)==0) {
		MyHostname=h;
		/* getdomainname funktioniert auf vielen Systemen nicht oder liefert
		 * alles mögliche nur nicht dem Domainnamen zurück (z.B. "(none)").
		 * Daher auskommentiert

#ifdef HAVE_GETDOMAINNAME
		if (getdomainname(h,255)==0 && strlen(h)>0 && strcmp(h,"none")!=0
				&& strcmp(h,"(none)")!=0) {
			MyHostname.Concatf(".%s",h);
		}

#endif
*/
	} else {
		MyHostname=getenv("HOSTNAME");
	}
#else
	MyHostname=getenv("HOSTNAME");
#endif
	SMTPServer="localhost:smtp";
}

/*!\brief Destruktor der Klasse
 * \header \#include <ppl6.h>
 * \desc
 * Gegenwärtig gibt es keine Daten, die deinitialisiert werden müßten, daher
 * führt der Destruktor keine Aktionen aus.
 */
CSendMail::~CSendMail()
{
}

/*!\brief Mail löschen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Diese Funktion löscht Header und Body der letzten Mail
 */
void CSendMail::Clear()
{
	Header.Clear();
	Message.Clear();
}

/*!\brief Werte im Header setzen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion können alle Variablen innerhalb des Headers gesetzt werden.
 *
 * \param name Name der Header-Variablen
 * Beispiel:
 * - \b Subject Das Subject der Mail
 * - \b To Der Empfänger der Mail. Um mehrere Empfänger zu definieren, muss SetHeader
 *   mehrfach aufgerufen werden.
 * - \b Cc Empfänger für Kopien
 * - \b Bcc Empfänger für blinde Kopien
 * - \b Date
 * - \b From
 * \param content Ein Formatstring mit dem Wert der Variablen
 * \param ... Optionale Parameter
 * \return Die Funktion liefert true (1) zurück, wenn die Variable gesetzt werden konnte,
 * im Fehlerfall wird ein enstrechender Fehlercode gesetzt und die Funktion gibt
 * false (0) zurück.
 * \remark Wird die Funktion mit einem Variablen-Namen aufgerufen, der zuvor schonmal
 * gesetzt wurde, wird diese nicht überschrieben, sondern erweitert. In der Mail würde
 * diese Zeile dann mehrfach auftauchen. Um eine Variable aus dem Header zu löschen,
 * kann die Funktion DeleteHeader aufgerufen werden.
 */
int CSendMail::SetHeader(const char *name,const char *content, ...)
{
	CString Value;
	va_list args;
	va_start(args, content);
	Value.VaSprintf(content,args);
	va_end(args);
	return Header.Concat(name,Value,"\n");
}

/*!\brief Header aus einem Array kopieren
 *
 * Diese Funktion kopiert alle Werte aus dem angegebenen Array in den Header der
 * Mail.
 *
 * \param header Ein Assoziatives Array mit den Header-Werten
 * \return Die Funktion liefert im Erfolgsfall true (1) zurück, bei einem Fehler
 * false (0).
 */
int CSendMail::CopyHeader(CAssocArray &header)
{
	return Header.Copy(header);
}

/*!\brief Wert im Header löschen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion kann eine Variable innerhalb des Headers gelöscht werden.
 *
 * \param name Der Name der Header-Variablen
 *
 * \return Die Funktion liefert true (1) zurück, wenn die Variable gesetzt werden konnte,
 * im Fehlerfall wird ein enstrechender Fehlercode gesetzt und die Funktion gibt
 * false (0) zurück.
 */
int CSendMail::DeleteHeader(const char *name)
{
	return Header.Delete(name);
}

/*!\brief SMTP-Server angeben
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird der SMTP-Server festgelegt, an den die Mail übergeben werden
 * soll. Wird diese Funktion nicht aufgerufen, wird defaultmäßig versucht auf den
 * lokalen SMTP-Port zu connecten (localhost:25).
 *
 * \param hostport Hostname:Port des SMTP-Servers. Statt Hostname kann auch die IP
 * angegeben werden und statt einer Portnummer der Servicename.
 * Beispiel: mail.pfp.de:smtp
 * \return Die Funktion führt keine Prüfung des übergebenen Hostnamens zurück
 * und liefert daher immer true (1) zurück.
 */
int CSendMail::SetServer(const char *hostport)
{
	SMTPServer=hostport;
	if (SMTPServer.Instr(":")<0) SMTPServer.Concat(":smtp");
	return 1;
}

/*!\brief Lokalen Rechnernamen definieren
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird der lokale Rechnername festgelegt, der während der
 * Kommunikation mit dem Mailserver im "HELO"-Kommando übergeben wird. Wird diese
 * Funktion nicht aufgerufen, wird defaultmäßig der Hostname des lokalen Servers
 * benutzt.
 *
 * \param name Der Name des Rechners
 * \return Die Funktion führt keine Prüfung des übergebenen Hostnamens zurück. Sie liefert daher
 * immer true (1) zurück, sofern ein Name angegeben wurde. Zeigt \p name auf NULL oder einen
 * leeren String, liefert die Funktion false (0) zurück und setzt einen Fehlercode.
 * und liefert daher immer true (1) zurück.
 */
int CSendMail::SetLocalHostname(const char *name)
{
	if (name==NULL || strlen(name)==0) {
		SetError(194,"int CSendMail::SetLocalHostname(==> const char *name <==), name darf nicht NULL oder leer sein");
		return 0;
	}
	MyHostname=name;
	return 1;
}

/*!\brief Absender definieren
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird der Absender der Email definiert. Diese wird während
 * der Kommunikation mit dem Mailserver im "MAIL"-Kommando übergeben und darf nur eine
 * Emailadresse enthalten. Wird diese Funktion nicht aufgerufen, wird defaultmäßig der
 * Wert aus der "From"-Variablen im Header verwendet.
 *
 * \param name Die Emailadresse des Absenders
 * \return Die Funktion führt keine Prüfung des übergebenen Wertes zurück
 * und liefert daher immer true (1) zurück.
 * \note Falls der String einen Namen und Emailadresse in spitzen Klammern enthält
 * (z.B. "Mein Name <ich@domain.de>"), wird nur der Inhalt der spitzen Klammern
 * verwendet (ab 6.0.23).
 */
int CSendMail::SetSender(const char *name)
{
	Sender=name;
	if (Sender.PregMatch("/<(.*?)>/")) {
		Sender=Sender.GetMatch(1);
	}
	return 1;
}

/*!\brief Nachricht übergeben
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird die zu sendende Nachricht übergeben. Die Funktion
 * existiert mehrfach und unterscheidet sich nur durch den Datentyp des
 * übergebenen Parameters.
 *
 * \param message Der Inhalt der Email, der mit folgenden Datentypen übergeben
 * werden kann:
 * - const char *
 * - CString *
 * - CString
 * \return Die Funktion führt keine Prüfung des übergebenen Wertes zurück
 * und liefert daher immer true (1) zurück.
 */
int CSendMail::SetMessage(const char *message)
{
	Message=message;
	return 1;
}

/*!\brief Nachricht übergeben
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird die zu sendende Nachricht übergeben. Die Funktion
 * existiert mehrfach und unterscheidet sich nur durch den Datentyp des
 * übergebenen Parameters.
 *
 * \param message Der Inhalt der Email, der mit folgenden Datentypen übergeben
 * werden kann:
 * - const char *
 * - CString
 * \return Die Funktion führt keine Prüfung des übergebenen Wertes zurück
 * und liefert daher immer true (1) zurück.
 */
int CSendMail::SetMessage(CString &message)
{
	Message=message;
	return 1;
}

int CSendMail::ParseAnswer(CString &Answer, CString &Text)
/*!\brief Antwort des SMTP-Servers in seine Bestandteile zerlegen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Dies ist eine interne Funktion, die von außen nicht benutzt werden kann.
 * Sie wird verwendet, um die Antwort des SMTP-Servers in die Bestandteile
 * "ReturnCode" und "Text" zu zerlegen.
 * \param Answer Enthält die Antwort des Servers
 * \param Text in dieser Variablen wird der Text-Bestandteil der Antwort abgelegt
 * \return Die Funktion liefert den Return-Code des SMTP-Servers zurück
 */
{
	CString n=Answer.Left(3);
	if (Answer.PregMatch("/^[0-9\\.]+([^0-9\\.].*)$/")) {
		Text.Set(Answer.GetMatch(1));
		Text.Trim();
	} else {
		Text.Set(Answer);
	}
	return n.ToInt();
}

int CSendMail::Talk(CTCPSocket &socket,CString &Reply, const char *msg, ...)
/*!\brief Kommunikation mit dem SMTP-Server
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser internen Funktion wird eine Nachricht an den SMTP-Server geschickt
 * und dessen Antwort gelesen
 * \param[in] socket Pointer auf die Socket-Klasse, die die Verbindung zum SMTP-Server
 * enthält
 * \param[in] Reply Ein String, in dem die Antwort des Servers gespeichert wird
 * \param[in] msg Ein Formatstring, der die Nachricht an den SMTP-Server enthält
 * \param[in] ... Beliebig viele optionale Parameter, die im Formatstring verwendet
 * werden
 * \return Die Funktion liefert im Fehlerfall 0 zurück, sonst den Return-Code des
 * SMTP-Servers (siehe ParseAnswer).
 */
{
	CString Value;
	va_list args;
	va_start(args, msg);
	Value.VaSprintf(msg,args);
	va_end(args);
	return Talk(socket, Reply, Value);
}

int CSendMail::Talk(CTCPSocket &socket,CString &Reply, CString &Msg)
/*!\brief Kommunikation mit dem SMTP-Server
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser internen Funktion wird eine Nachricht an den SMTP-Server geschickt
 * und dessen Antwort gelesen
 * \param[in] socket Pointer auf die Socket-Klasse, die die Verbindung zum SMTP-Server
 * enthält
 * \param[in] Reply Ein String, in dem die Antwort des Servers gespeichert wird
 * \param[in] Msg Die Nachricht, die an den SMTP-Server geschickt werden soll
 * \return Die Funktion liefert im Fehlerfall 0 zurück, sonst den Return-Code des
 * SMTP-Servers (siehe ParseAnswer).
 */
{
	Msg.Trim();
	//printf ("Sende Befehl: %s\n",Msg->GetPtr());
	Msg.Concat("\n");
	CString answer;
	if (!socket.Write(&Msg)) return 0;
	if (!socket.ReadOnce(&answer,4096)) return 0;
	return ParseAnswer(answer,Reply);
}


int CSendMail::Send()
/*!\brief Email versenden
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird die Email an den SMTP-Server zugestellt.
 *
 * \return Konnte die Mail erfolgreich versendet werden, liefert die Funktion
 * true (1) zurück, im Fehlerfall wird ein entsprechender Fehlercode gesetzt und die
 * Funktion liefert false (0) zurück.
 */
{
	CTCPSocket socket;
	CString answer, Text, line;
	CArray rcpt;
	const char *tmp;
	if (Sender.Len()==0) {
		SetSender(Header["From"]);
	}

	// Falls wir keinen Hostnamen haben, können wir nicht weitermachen
	if (MyHostname.Len()==0) {
		SetError(512,"Hostname wird für HELO-Kommando benötigt");
		return 0;
	}

	socket.SetConnectTimeout(5,0);
	if (!socket.Connect((const char*)SMTPServer)) return 0;
	if (!socket.WaitForIncomingData(5,0)) {
		return 0;
	}
	//220 server.pfp.de ESMTP Sendmail 8.13.8/8.13.8; Tue, 6 Mar 2007 10:24:58 +0100 (CET)
	// Solange lesen, bis ein Newline kommt
	answer.Clear();
	while (1) {
		socket.ReadOnce(&Text,4096);
		answer+=Text;
		if (answer[-1]=='\n') break;
	}

	int code=ParseAnswer(answer,Text);
	if (code!=220) {
		SetError(390,"Reading Greeting, Return-Code: %i, Msg: %s",code,(const char*)Text);
		return 0;
	}
	code=Talk(socket,Text,"HELO %s",(const char*)MyHostname);
	if (code!=250) {
		Talk(socket,Text,"QUIT");
		SetError(390,"Sending \"HELO %s\", Return-Code: %i, Msg: %s",(const char*)MyHostname, code,(const char*)Text);
		return 0;
	}
	code=Talk(socket,Text,"MAIL From: %s",(const char*)Sender);
	if (code!=250) {
		Talk(socket,Text,"QUIT");
		SetError(390,"Sending \"MAIL From: %s\", Return-Code: %i, Msg: %s",(const char*)Sender, code,(const char*)Text);
		return 0;
	}
	rcpt.Explode(Header["To"],"\n");
	rcpt.Explode(Header["Cc"],"\n");
	rcpt.Explode(Header["Bcc"],"\n");
	rcpt.Reset();
	while ((tmp=rcpt.GetNext())) {
		code=Talk(socket,Text,"RCPT To: %s",tmp);
		if (code!=250) {
			Talk(socket,Text,"QUIT");
			SetError(390,"Sending \"RCPT To: %s\", Return-Code: %i, Msg: %s",tmp, code,(const char*)Text);
			return 0;
		}
	}

	code=Talk(socket,Text,"DATA");
	if (code!=354) {
		Talk(socket,Text,"QUIT");
		SetError(390,"Sending \"DATA\", Return-Code: %i, Msg: %s",code,(const char*)Text);
		return 0;
	}
	//printf ("code: %i, Text: %s\n",code,(char*)Text);
	Header.Reset();
	CString Key,Value,header;
	CArray a;
	while (Header.GetNext(&Key,&Value)) {
		if (Key.StrCaseCmp((char*)"bcc")==0) continue;
		a.Clear();
		a.Explode(&Value,"\n");
		a.Reset();
		while ((tmp=a.GetNext())) {
			header.Concatf("%s: %s\n",(const char*)Key,tmp);
		}
	}
	header.Concat("\n");
	CompleteMail=header;
	CompleteMail+=Message;
	//printf ("Header:\n%s\n",(char*)header);
	if (!socket.Write(&header)) return 0;
	if (!socket.Write(&Message)) return 0;
	if (!socket.Write((char*)"\n.\n",3)) return 0;
	socket.ReadOnce(&answer,2048);
	code=ParseAnswer(answer,Text);
	//printf ("code: %i, Text: %s\n",code,(char*)Text);
	if (code!=250) {
		Talk(socket,Text,"QUIT");
		SetError(390,"Answer after sending Mail, Return-Code: %i, Msg: %s",code,(const char*)Text);
		return 0;
	}
	code=Talk(socket,Text,"QUIT");
	if (code!=221) {
		SetError(390,"Sending \"QUIT\", Return-Code: %i, Msg: %s",code,(const char*)Text);
		return 0;
	}
	return 1;
}

const char *CSendMail::GetMail()
/*!\brief Komplette Mail
 *
 * Diese Funktion gibt einen Pointer auf die komplette Mail, also Header und Body, zurück.
 *
 * @return Pointer auf Mailinhalt
 */
{
	return CompleteMail.GetPtr();
}


} // EOF namespace ppl6


