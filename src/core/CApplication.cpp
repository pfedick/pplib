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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ppl6.h"


namespace ppl6 {


#ifndef MINGW32
#ifdef _WIN32
int PPL6MainWin (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, CApplication *app)
{
	if (!app) {
		return 251;
	}
	PPL6APPPARAMS p;
	memset(&p,0,sizeof(p));
	p.hInstance=hInstance;
	p.hPrevInstance=hPrevInstance;
	LPTSTR cmd=GetCommandLine();
	/*
	size_t l=strlen(cmd)+10;

	int len=::WideCharToMultiByte(CP_UTF8,0,(LPWSTR)cmd,-1,
		NULL,0,NULL,NULL);
	char *buffer=(char*)malloc(len+1);
	if (!::WideCharToMultiByte(CP_UTF8,0,(LPWSTR)cmd,-1,
		buffer,len,NULL,NULL)) {
		free(buffer);
		return 1;
	}

	free(buffer);
	*/
	// "e:\cvs\ppl\ppl6\test\test_dx9\Debug\test_dx9.exe" -h server.pfp.de:8080 --name "Patrick Fedick" -a -t -lpafe -f


	// Kommandozeile in argc und argv[] umwandeln
	char *buffer=strdup(cmd);
	size_t l=strlen(buffer);
	int argc=0;
	char *argv[255];
	for (int a=0;a<255;a++) {
		argv[a]=NULL;
	}
	argv[0]=buffer;
	bool inQuote=false;
	for (size_t i=0;i<l;i++) {
		if(buffer[i]==34 && inQuote==false) {
			if (i==0) {
				inQuote=true;
				argv[argc]=buffer+i+1;
			}
			else if (buffer[i-1]!='\\') {
				inQuote=true;
				argv[argc]=buffer+i+1;
			}
		} else if(buffer[i]==34 && inQuote==true && buffer[i-1]!='\\') {
			inQuote=false;
			buffer[i]=0;
			argv[argc]=trim(argv[argc]);
			if(argv[argc][0]!=0) {
				argc++;
			}
			argv[argc]=buffer+i+1;
		} else if((buffer[i]==' ' || buffer[i]=='\t') && inQuote==false) {
			buffer[i]=0;
			argv[argc]=trim(argv[argc]);
			if(argv[argc][0]!=0) {
				argc++;
			}
			argv[argc]=buffer+i+1;
		}
		if (argc>253) {
			free(buffer);
			return(252);
		}
	}
	argv[argc]=trim(argv[argc]);
	if(argv[argc][0]!=0) argc++;

	p.argc=argc;
	p.argv=(char**)&argv;
	int ret=app->Init(&p);
	free(buffer);
	return ret;
}
#endif
#endif

int PPL6Main (int argc, char **argv, CApplication *app)
{

	PPL6APPPARAMS p;
	memset(&p,0,sizeof(p));
	p.argc=argc;
	p.argv=argv;
	if (!app) {
		return 251;
	}
	return app->Init(&p);
}

/************************************************************
 * CApplication
 ************************************************************/

/*!\class CApplication
 * \brief Application Framework
 *
 * \desc
 * Diese Klasse kann als Framework für eigene Applikationen verwendet
 * werden. Sie abstrahiert einige Betriebssystem- und Komprimierungsabhängige
 * Unterschiede (main vs. WinMain) und stellt eine einheitliche Schnittstelle
 * zum Zugriff auf die übergebenen Parameter zur Verfügung.
 *
 * Zur Verwendung der Klasse muss lediglich eine Instanz der Klasse
 * abgeleitet werden.
 *
 * \example
 * \include CApplication_1.cpp
 *
 */

/*!\var CApplication::PPLCore
 * \brief Pointer auf die Programmweite ppl6::Cppl6Core Klasse
 *
 * \desc
 * Pointer auf die Programmweite ppl6::Cppl6Core Klasse. Dies wird bei Programmstart automatisch angelegt.
 */

CApplication::CApplication()
/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Dies ist der Konstruktor der Klasse. Er initialisiert einige interne Variablen.
 */
{
	argc=0;
	argv=NULL;
	PPLCore=PPLInit();
}

CApplication::~CApplication()
/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor der Klasse.
 */
{

}

int CApplication::Init(PPL6APPPARAMS *params)
/*!\brief Interne Initialisierung
 *
 * \desc
 * Diese Funktion wird nur intern aufgerufen. Hier werden Betriebssystem-unabhängig die Startparameter initialisiert
 * und anschließend die virtuelle Funktion CApplication::Main aufgerufen.
 *
 * @param[in] params Pointer auf eine ppl6::PPL6APPPARAMS Struktur, die je nach Betriebssystem unterschiedlich
 * definiert sein kann.
 *
 * @return Die Funktion liefert den Return-Wert der Main-Funktion zurück, der letztenendlich auch als
 * Returncode des Programms zurückgegeben wird.
 */
{
	startparam=params;
	argc=params->argc;
	argv=params->argv;
	return Main();
}

int CApplication::Main()
/*!\brief Virtuelle Hauptfunktion
 *
 * \desc
 * Dies ist die Hauptfunktion der Klasse, die beim Starten der Anwendung aufgerufen wird. Sie ist als
 * virtuell definiert und hat selbst keine Funktion. Sie muss daher von der abgeleiteten Klasse
 * der Anwendung überschrieben werden.
 * @return Ein Wert, der als Returncode der Anwendung zurückgegeben werden soll.
 */
{
	return 0;
}

const char *CApplication::GetArg(const char *argument)
/*!\brief Wert eines Aufrufparameters zurückgeben
 *
 * \desc
 * Diese Funktion prüft, ob das angegebene Argument bei den Aufrufparametern des Programms dabei war und liefert
 * einen Pointer auf dessen Wert zurück.
 *
 * @param[in] argument Pointer auf einen String mit dem gesuchten Parameter. Dieser enthält in der Regel sowas
 * wie "-h" oder "--version"
 * @return Wenn das Argument nicht bei den Aufrufparametern vorhanden war, liefert die Funktion NULL zurück.
 * Falls doch, wird ein Pointer auf den String zurückgeleifert, der dem Argument folgt. Dies kann auch ein
 * Pointer auf einen leeren String sein.
 */
{
	return getargv(argc,argv,(char*)argument);
}

bool CApplication::HaveArg(const char *argument)
/*!\brief Prüft, ob ein bestimmter Parameter vorhanden ist
 *
 * \desc
 * Diese Funktion prüft, ob das angegebene Argument bei den Aufrufparametern des Programms dabei war.
 *
 * @param[in] argument Pointer auf einen String mit dem gesuchten Parameter. Dieser enthält in der Regel sowas
 * wie "-h" oder "--version"
 * @return Wenn das Argument bei den Aufrufparametern vorhanden war, liefert die Funktion true zurück, andernfalls
 * false.
 */
{
	if (getargv(argc,argv,(char*)argument)!=NULL) return true;
	return false;
}

const char *CApplication::GetArg(int num)
/*!\brief Bestimmten Parameter zurückgeben
 *
 * \desc
 * Mit dieser Funktion kannein bestimmer Parameter anhand seiner Nummer ausgelesen werden. Die Anzahl
 * übergebener Paraneter kann mit der Funktion CApplication::GetArgc ausgelesen werden.
 *
 * @param[in] num Nummer des Parameters
 *
 * @return Pointer auf den Inhalt des Parameters oder NULL, wenn die angegebene Nummer \p num nicht
 * vorhanden ist.
 */
{
	if (num>=0 && num<=argc) return argv[num];
	return NULL;
}

int CApplication::GetArgc()
/*!\brief Anzahl Parameter
 *
 * \desc
 * Diese Funktion liefert die Anzahl übergebener Parameter zurück.
 *
 * @return Anzahl Parameter
 */
{
	return argc;
}

const char **CApplication::GetArgv()
/*!\brief Pointer auf Array mit Parametern
 *
 * \desc
 * Diese Funktion liefert einen Pointer auf ein Array mit Parametern zurück. Die Anzahl Parameter kann
 * mit der Funktion CApplication::GetArgc abgefragt werden. Das Array ist identisch wie der Parameter \p argv bei
 * der C-main-Funktion.
 *
 * @return Pointer auf ein Array mit den Parametern.
 */
{
	return (const char**)argv;
}

PPL6APPPARAMS *CApplication::GetSystemParameter()
/*!\brief Pointer auf interne Systemparameter
 *
 * \desc
 * Diese Funktion liefert einen Pointer auf eine Struktur mit internen Systemparametern zurück.
 *
 * @return Pointer auf ppl6::PPL6APPPARAMS
 */
{
	return startparam;
}


}	// EOF namespace ppl6
