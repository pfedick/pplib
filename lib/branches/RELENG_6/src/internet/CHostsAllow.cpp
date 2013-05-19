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
#include "ppl6.h"

namespace ppl6 {

#define WMLOG(facility,debuglevel) if(Log) Log->Printf(facility,debuglevel,__FILE__,__LINE__,

static ppluint32 netmasks[] = {
		0x00000000,
		0x80000000,
		0xc0000000,
		0xe0000000,
		0xf0000000,
		0xf8000000,
		0xfc000000,
		0xfe000000,
		0xff000000,
		0xff800000,
		0xffc00000,
		0xffe00000,
		0xfff00000,
		0xfff80000,
		0xfffc0000,
		0xfffe0000,
		0xffff0000,
		0xffff8000,
		0xffffc000,
		0xffffe000,
		0xfffff000,
		0xfffff800,
		0xfffffc00,
		0xfffffe00,
		0xffffff00,
		0xffffff80,
		0xffffffc0,
		0xffffffe0,
		0xfffffff0,
		0xfffffff8,
		0xfffffffc,
		0xfffffffe,
		0xffffffff
};


ppluint32 IP2Int(ppl6::CString &ip)
/*!\brief Wandelt eine IPv4-Adresse in einen Integerwert um
 * Diese Funktion wandelt ein in einem String enthaltene IPv4-Adresse in einen
 * Integer-Wert um.
 *
 * \param ip String mit der IP-Adresse. Diese muss das Format "a.b.c.d"
 * aufweisen
 * \return Liefert den Integer-Wert der IP-Adresse zurück. Dabei wird die erste Ziffer
 * im höherwertigsten Byte gespeichert, die vierte Ziffer im niedrigsten. Handelte es
 * sich um eine gültige IP-Adresse, wird ausserdem das Fehlerflag auf 0 gesetzt, so
 * dass zwischen dem Wert der IP-Adresse "0.0.0.0" (=0) und einem Fehlerfall unterschieden
 * werden kann. Im Fehlerfall liefert die Funktion 0 zurück und setzt einen entsprechenden
 * Fehlercode.
 *
 * \since Diese Funktion wurde mit Version 6.0.23 eingeführt
 */
{
	if (ip.PregMatch("/^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$/")) {
		ppluint32 v=0;
		v|=(ppl6::atoi(ip.GetMatch(1)))<<24;
		v|=(ppl6::atoi(ip.GetMatch(2)))<<16;
		v|=(ppl6::atoi(ip.GetMatch(3)))<<8;
		v|=(ppl6::atoi(ip.GetMatch(4)));
		ppl6::SetError(0);
		return v;
	}
	ppl6::SetError(149);	// Ungültige IP-Adresse;
	return 0;
}


/*!\class CHostsAllow
 * \ingroup PPLGroupInternet
 * \brief Klasse zum Prüfen, ob eine IP-Adresse in einer Liste enthalten ist
 *
 * \desc
 * Diese Klasse kann verwendet werden, um zu überprüfen, ob eine IP-Adresse in einer
 * Liste enthalten ist. Dazu wird zunächst eine sogenannte "hostsallow"-Liste geladen.
 * Die Liste kann sowohl einzelne IP-Adressen, IP-Adressen mit Netzmasken als auch
 * Hostnamen enthalten.
 *
 * \example
 * \verbatim
# Spezifische IPs
127.0.0.1
# Subnetze können angegeben werden
62.40.10.192/29
# Oder Hostnamen. Dabei erfolgt jedoch bei jedem Connect eine Hostauflösung
server.pfp.de
thunder.pfp.de
\endverbatim
 *
 * \since Diese Klasse wurde mit Version 6.0.23 eingeführt
 */

CHostsAllow::CHostsAllow()
{
	enabled=true;
	Log=NULL;
}

CHostsAllow::~CHostsAllow()
{
	Hosts.Clear();
}

void CHostsAllow::Clear()
{
	Mutex.Lock();
	Hosts.Clear();
	Mutex.Unlock();
}

void CHostsAllow::SetLogfile(CLog *log)
/*!\brief Aktivierung der Debug-Funktionen
 */
{
	Mutex.Lock();
	Log=log;
	Mutex.Unlock();
}

int CHostsAllow::LoadFromFile(const char *filename, ...)
{
	CString s;
	va_list args;
	va_start(args, filename);
	s.VaSprintf(filename,args);
	va_end(args);
	return LoadFromFile(&s);
}

int CHostsAllow::LoadFromFile(const CString &filename)
{
	CFile ff;
	if (!ff.Open(filename,"rb")) return 0;
	return LoadFromFile(ff);
}

int CHostsAllow::LoadFromFile(CFileObject &file)
{
	const char *map=file.Map();
	if (!map) return 0;
	return Load(map);
}


int CHostsAllow::Load(const char *text)
{
	Mutex.Lock();
	Hosts.Clear();
	const char *tmp;
	ppl6::CAssocArray aa;
	// Zuerst lesen wir den Text in ein temporäres Array
	ppl6::CArray a;
	a.Text2Array(text,"\n");
	// Das gehen wir dann Zeilenweise durch und prüfen den Inhalt auf Gültigkeit.
	// Nur gültige Zeilen werden in das HostsAllow-Array kopiert
	ppl6::CString Tmp;
	a.Reset();
	while ((tmp=a.GetNext())) {
		Tmp=tmp;
		Tmp.Trim();
		if (Tmp.Len()==0) continue;		// Leerzeilen werden ignoriert
		if (Tmp[0]=='#') continue;		// Kommentarzeilen werden ignoriert
		// Einzelne IP?
		if (Tmp.PregMatch("/^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$/")) {
			aa.Clear();
			aa.Set("type","ip");
			aa.Set("ip",Tmp);
			Hosts.Set("[]",aa);
			continue;
		}
		// Subnetz?
		if (Tmp.PregMatch("/^([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)\\/([0-9]+)$/")) {
			// IP in einen 32-Bit-Wert umrechnen
			ppl6::CString IP=Tmp.GetMatch(1);
			int mask=ppl6::atoi(Tmp.GetMatch(2));
			// Maske muss zwischen 0 und 32 liegen
			if (mask>=0 && mask<=32) {
				ppluint32 v=IP2Int(IP);
				// ErrorCode muss 0 sein
				if (ppl6::GetErrorCode()==0) {
					v=v&netmasks[mask];
					aa.Clear();
					aa.Set("type","subnet");
					aa.Set("ip",Tmp.GetMatch(1));
					aa.Setf("value","%u",v);
					aa.Setf("mask","%i",mask);
					Hosts.Set("[]",&aa);
				}
			}
			continue;
		}
		// Hostname?
		if (Tmp.PregMatch("/^[a-z0-9\\-\\.]+$/")) {
			aa.Clear();
			aa.Set("type","hostname");
			aa.Set("host",Tmp);
			Hosts.Set("[]",aa);
			continue;
		}
	}
	Mutex.Unlock();
	return 1;
}

int CHostsAllow::Count()
{
	return Hosts.Count();
}

int CHostsAllow::Num()
{
	return Hosts.Count();
}

int CHostsAllow::IsEnabled()
{
	if (enabled) return 1;
	return 0;
}

int CHostsAllow::IsAllowed(const char *ip)
{
	if (!enabled) {
		WMLOG(ppl6::LOG::DEBUG,2) "HostsAllow is not enabled, all connects are allowed.");
		return 1;
	}
	if (Hosts.Count()>0) {
		WMLOG(ppl6::LOG::DEBUG,2) "Checking HostsAllow...");
		// Wir legen zuerst eine lokale Kopie der HostsAllow an, damit wir
		// durchtraversieren können, ohne andere Threads zu blockieren
		Mutex.Lock();
		ppl6::CAssocArray a, *line;
		ppl6::CString Type;
		a=Hosts;
		Mutex.Unlock();
		bool Allowed=false;
		a.Reset();
		while ((line=a.GetNextArray())) {
			Type=line->Get("type");
			if (Type=="ip") {
				ppl6::CString IP=line->Get("ip");
				if (IP.StrCmp(ip)==0) {
					WMLOG(ppl6::LOG::DEBUG,10) "Matching \"%s\" against IP \"%s\" => match", ip,(const char*)IP);
					Allowed=true;
					break;
				} else {
					WMLOG(ppl6::LOG::DEBUG,10) "Matching \"%s\" against IP \"%s\" => mismatch", ip,(const char*)IP);
				}

			} else if (Type=="subnet") {
				ppl6::CString HostIP=ip;
				ppl6::CString Subnet=line->Get("value");
				int Mask=ppl6::atoi(line->Get("mask"));
				ppluint32 ipval=(ppluint32)ppl6::atoi(Subnet);
				// IP in einen 32-Bit-Wert umrechnen
				ppluint32 hostip=IP2Int(HostIP);
				if (ppl6::GetErrorCode()==0) {
					hostip=hostip&netmasks[Mask];
					if (hostip==ipval) Allowed=true;
				}
				if (!Allowed) {
					WMLOG(ppl6::LOG::DEBUG,10) "Matching \"%s\" against Subnet \"%s/%i\" => mismatch", ip,line->Get("ip"),Mask);
				} else {
					WMLOG(ppl6::LOG::DEBUG,10) "Matching \"%s\" against Subnet \"%s/%i\" => match", ip,line->Get("ip"),Mask);
					break;
				}
			} else if (Type=="hostname") {
				ppl6::CString HostIP=ip;
				ppluint32 hostip=IP2Int(HostIP);
				// Wir müssen zuerst den Hostnamen resolven
				ppl6::CAssocArray r;
				if (GetHostByName(line->Get("host"),&r)) {
					ppl6::CAssocArray *a;
					r.Reset();
					while ((a=r.GetNextArray())) {
						ppl6::CString Type=a->Get("type");
						if (Type=="AF_INET") {
							ppl6::CString IP=a->Get("ip");
							ppluint32 ipval=IP2Int(IP);
							if (ipval==hostip) Allowed=true;
						}
					}
					if (!Allowed) {
						WMLOG(ppl6::LOG::DEBUG,10) "Matching \"%s\" against Hostname \"%s\" => mismatch", ip,line->Get("host"));
					} else {
						WMLOG(ppl6::LOG::DEBUG,10) "Matching \"%s\" against Hostname \"%s\" => match", ip,line->Get("host"));
						break;
					}
				} else {
					WMLOG(ppl6::LOG::DEBUG,2) "ERROR: Cannot resolve Hostname in hosts_allow: \"%s\"", line->Get("host"));
				}
			}
		}
		if (Allowed) {
			WMLOG(ppl6::LOG::DEBUG,2) "Client \"%s\" is in hosts_allow",ip);
			return 1;
		}
		WMLOG(ppl6::LOG::DEBUG,2) "Client \"%s\" is not in hosts_allow",ip);
	} else {
		WMLOG(ppl6::LOG::DEBUG,2) "HostsAllow is empty.");
	}
	SetError(410,"%s",ip);
	return 0;
}



} // EOF namespace ppl6
