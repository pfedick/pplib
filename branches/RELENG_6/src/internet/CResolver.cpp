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
	#include <Ws2tcpip.h>
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
	#include <netdb.h>
#endif

#ifdef HAVE_LIBLDNS
extern "C" {
	#include <ldns/ldns.h>
	#include <ldns/keys.h>
}
#endif

#include "ppl6.h"
#include "socket.h"

namespace ppl6 {

#ifdef HAVE_LIBLDNS

static int FindRecords(CArray &records, const char *label, ldns_rr_list *add, ldns_rr_type type)
{
	CString s;
	int c=0;
	ldns_rr *ar;
	if (!add) return 0;
	for (size_t n=0;n<add->_rr_count;n++) {
		ar=add->_rrs[n];
		if (ar->_rr_type==type) {
			char *str2=ldns_rdf2str(ar->_owner);
			if (str2) {
				if (strcmp(label,str2)==0) {
					LDNS_FREE(str2);
					char *str2=ldns_rdf2str(ar->_rdata_fields[0]);
					if (str2) {
						records.Add(str2);
						c++;
						LDNS_FREE(str2);
					}
				} else {
					LDNS_FREE(str2);
				}
			}
		}
	}
	return c;
}



static void Translate_NS(ppl6::CAssocArray &a, ldns_rr *rr, ldns_rr_list *add, ldns_rr_list *ipv6, ldns_rr_list *sig)
{
	char *str=ldns_rdf2str(rr->_rdata_fields[0]);
	if (str) {
		ppl6::CString s=str;
		CArray arecords;
		if (FindRecords(arecords,str,add,LDNS_RR_TYPE_A)) {
			a.Set("A",arecords);
		}
		if (FindRecords(arecords,str,add,LDNS_RR_TYPE_AAAA)) {
			a.Set("AAAA",arecords);
		}
		if (FindRecords(arecords,str,sig,LDNS_RR_TYPE_RRSIG)) {
			a.Set("RRSIG",arecords);
		}


		s.RTrim(".");
		a.Setf("NS",s);
		LDNS_FREE(str);
	}
}

static void Translate_A(ppl6::CAssocArray &a, ldns_rr *rr, const CString &type, ldns_rr_list *sig)
{
	CString s;
	char *str=ldns_rdf2str(rr->_rdata_fields[0]);
	if (str) {
		s=str;
		s.RTrim(".");
		a.Set(type,s);
		CArray arecords;
		if (FindRecords(arecords,str,sig,LDNS_RR_TYPE_RRSIG)) {
			a.Set("RRSIG",arecords);
		}

		LDNS_FREE(str);
	}
}

static void Translate_MX(ppl6::CAssocArray &a, ldns_rr *rr, ldns_rr_list *add, ldns_rr_list *ipv6, ldns_rr_list *sig)
{
	CString s;
	char *str=ldns_rdf2str(rr->_rdata_fields[0]);
	if (str) {
		a.Set("prio",str);
		LDNS_FREE(str);
	}
	str=ldns_rdf2str(rr->_rdata_fields[1]);
	if (str) {
		s=str;
		CArray arecords;
		if (FindRecords(arecords,str,add,LDNS_RR_TYPE_A)) {
			a.Set("A",arecords);
		}
		if (FindRecords(arecords,str,add,LDNS_RR_TYPE_AAAA)) {
			a.Set("AAAA",arecords);
		}

		if (FindRecords(arecords,str,sig,LDNS_RR_TYPE_RRSIG)) {
			a.Set("RRSIG",arecords);
		}

		s.RTrim(".");
		a.Set("MX",s);
		LDNS_FREE(str);
	}
}

#endif


/*!\class CResolver
 * \ingroup PPLGroupInternet
 * \brief DNS Namensauflösung, Abfrage verschiedener Resource Records
 *
 * \desc
 * Mit dieser Klasse können verschiedene Nameserver-spezifischen Resource Records abgefragt werden.
 * Die unterstützten Typen finden sich in der Enumeration CResolver::Type.
 *
 */


/*!\enum CResolver::Type
 * \brief Die unterstützen Resource Records (RR)
 *
 */


CResolver::CResolver()
{
	res=NULL;
	packet=NULL;
}

CResolver::~CResolver()
{
#ifdef HAVE_LIBLDNS
	if (res) ldns_resolver_deep_free((ldns_resolver*)res);
	if (packet) ldns_pkt_free((ldns_pkt *)packet);

#endif
	res=NULL;
}

CString CResolver::typeName(Type t)
{
	switch (t)  {
		case A: return "A";
		case NS: return "NS";
		case CNAME: return "CNAME";
		case MX: return "MX";
		case SOA: return "SOA";
		case PTR: return "PTR";
		case TXT: return "TXT";
		case AAAA: return "AAAA";
		case NAPTR: return "NAPTR";
		case SRV: return "SRV";
		case DS: return "DS";
		case DNSKEY: return "DNSKEY";
		case NSEC: return "NSEC";
		case NSEC3: return "NSEC3";
		case RRSIG: return "RRSIG";
		case OPT: return "OPT";
		case TSIG: return "TSIG";
		default: return "UNKNOWN";
	}
}

CString CResolver::className(Class c)
{
	switch (c)  {
		case CLASS_IN: return "IN";
		case CLASS_CH: return "CH";
		case CLASS_HS: return "HS";
		case CLASS_NONE: return "NONE";
		case CLASS_ANY: return "ANY";
		default: return "UNKNOWN";
	}
}

int CResolver::InitInternal()
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return 0;
#else
	if (!res) {
		res=ldns_resolver_new();
		if (!res) {
			SetError(2);
			return 0;
		}
	}
	return 1;
#endif
}

void CResolver::setTimeout(int seconds, int microseconds)
{
#ifdef HAVE_LIBLDNS
	if (!InitInternal()) throw Exception();
	struct timeval t;
	t.tv_sec=seconds;
	t.tv_usec=microseconds;
	ldns_resolver_set_timeout((ldns_resolver*)res,t);
#endif
}

int CResolver::setNameserver(const CString &server)
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return 0;
#else
	if (!InitInternal()) return 0;
	CArray a;
	a.Add(server);
	return setNameservers(a);
#endif
}

int CResolver::setNameservers(const CArray &list)
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return 0;
#else
	if (!InitInternal()) return 0;
	int p;
	int num=list.Num();
	ldns_status ret;
	ldns_rdf *server;
	ppl6::CString name;
	CAssocArray hosts;
	for (int i=0;i<num;i++) {
		name=list[i];
		p=name.Instr(":");
		if (p>=0) {
			CString port=name.Mid(p+1);
			printf ("setPort: %i\n",port.ToInt());
			ldns_resolver_set_port((ldns_resolver*)res,(uint16_t)port.ToInt() );
			name=name.Left(p);
		}
		// Der Name könnte ein Hostname statt einer IP sein, daher resolven wir sie zuerst
		if (!GetHostByName(name,&hosts)) return 0;
		name=hosts["0/ip"];
		//printf ("setName: %s\n",(const char*)name);
		server=ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A,name);
		ret=ldns_resolver_push_nameserver((ldns_resolver*)res,server);
		if (ret!=LDNS_STATUS_OK) {
			ppl6::SetError(548,"%s",ldns_get_errorstr_by_id(ret));
			return 0;
		}

		//ldns_rdf_deep_free(server);
	}

	printf ("Anzahl Nameserver: %zu\n",ldns_resolver_nameserver_count((ldns_resolver*)res));
	return 1;
#endif
}

int CResolver::setDNSSECAnchorFromFile(const CString &filename)
{
	CString s;
	if (!CFile::LoadFile(s,filename)) return 0;
	return setDNSSECAnchor(s);
}

int CResolver::setDNSSECAnchorFromFile(ppl6::CFileObject &file)
{
	CString s;
	if (!file.Load(s)) return 0;
	return setDNSSECAnchor(s);
}

void CResolver::setDnssecEnabled(bool flag)
{
#ifdef HAVE_LIBLDNS
	if (!InitInternal()) return;
	ldns_resolver_set_dnssec((ldns_resolver*)res,flag);
#endif
}

bool CResolver::dnssecEnabled() const
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return false;
#else
	if (!res) return false;
	return ldns_resolver_dnssec((ldns_resolver*)res);
#endif
}

void CResolver::setEdnsUdpSize(size_t s)
{
#ifdef HAVE_LIBLDNS
	if (!InitInternal()) return;
	ldns_resolver_set_edns_udp_size ((ldns_resolver*)res, (uint16_t) s);
#endif
}

size_t CResolver::ednsUdpSize() const
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return 0;
#else
	if (!res) return 0;
	return ldns_resolver_edns_udp_size((ldns_resolver*)res);
#endif

}


int CResolver::setDNSSECAnchor(const CString &anchor)
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return 0;
#else
	if (!InitInternal()) return 0;

	ldns_rr *r=NULL;
	ldns_status ret;
	// TODO: Zeilenweise bearbeitung, damit mehrere Keys eingelesen werden können

    ret = ldns_rr_new_frm_str(&r, anchor, 0, NULL, NULL);
    if (ret == LDNS_STATUS_OK && (ldns_rr_get_type(r) == LDNS_RR_TYPE_DNSKEY || ldns_rr_get_type(r) == LDNS_RR_TYPE_DS)) {
        (void) ldns_resolver_push_dnssec_anchor((ldns_resolver*)res, r);
        ldns_rr_free(r);
        ldns_resolver_set_dnssec((ldns_resolver*)res,true);
        return 1;
    } else {
    	ppl6::SetError(551,"%s",ldns_get_errorstr_by_id(ret));
        return 0;
    }
#endif
}

int CResolver::query(ppl6::CAssocArray &r, const CString &label, Type t, Class c)
{
#ifndef HAVE_LIBLDNS
	ppl6::SetError(550);
	return 0;
#else
	r.Clear();
	if (packet) ldns_pkt_free((ldns_pkt *)packet);
	packet=NULL;
	ldns_status ret;
	if (!res) {
		// Standardresolver erstellen unter Verwendung von /etc/resolv.conf erstellen
		ret=ldns_resolver_new_frm_file((ldns_resolver**)(&res),NULL);
		if (ret!=LDNS_STATUS_OK) {
			ppl6::SetError(548,"%s",ldns_get_errorstr_by_id(ret));
			return 0;
		}

	}
	ldns_rdf *domain;


	domain=ldns_dname_new_frm_str(label);
	if (!domain) {
		ppl6::SetError(549,"%s",(const char*)label);
		return 0;
	}
	packet = ldns_resolver_query( (ldns_resolver*)res, domain,(ldns_rr_type)t, (ldns_rr_class)c, LDNS_RD);
	if (!packet) {
		ppl6::SetError(546,"%s %s",(const char*)label,(const char*)typeName(t));
		ldns_rdf_deep_free(domain);
		return 0;
	}

	ldns_rr_list *rr, *add=NULL, *sig=NULL, *ipv6=NULL;
	rr = ldns_pkt_rr_list_by_type((ldns_pkt *)packet, (ldns_rr_type)t, LDNS_SECTION_ANSWER);
	if (!rr) {
		ppl6::SetError(546,"%s %s",(const char*)label,(const char*)typeName(t));
		ldns_rdf_deep_free(domain);
		return 0;
	}
	// DNSSEC
	sig = ldns_pkt_rr_list_by_type((ldns_pkt *)packet, LDNS_RR_TYPE_RRSIG, LDNS_SECTION_ANSWER);
	// Additional Section auswerten?
	if (t==NS || t==MX) {
		add = ldns_pkt_rr_list_by_type((ldns_pkt *)packet, LDNS_RR_TYPE_A, LDNS_SECTION_ADDITIONAL);
		ipv6 = ldns_pkt_rr_list_by_type((ldns_pkt *)packet, LDNS_RR_TYPE_AAAA, LDNS_SECTION_ADDITIONAL);
	}

	ldns_rr_list_sort(rr);
	char *str;
	ppl6::CAssocArray a;
	for (size_t n=0;n<rr->_rr_count;n++) {
		ldns_rr *rrs=rr->_rrs[n];
		a.Setf("ttl","%i",rrs->_ttl);
		str=ldns_rdf2str(rrs->_owner);
		if (str) {
			ppl6::CString s=str;
			s.RTrim(".");
			a.Setf("owner",s);
			LDNS_FREE(str);
		}
		a.Setf("type","%s",(const char*)typeName((Type)rrs->_rr_type));
		a.Setf("class","%s",(const char*)className((Class)rrs->_rr_class));

		switch(rrs->_rr_type) {
			case LDNS_RR_TYPE_NS:
					Translate_NS(a,rrs,add,ipv6,sig);
					break;
			case LDNS_RR_TYPE_A:
				Translate_A(a,rrs,"A",sig);
				break;
			case LDNS_RR_TYPE_AAAA:
					Translate_A(a,rrs,"AAAA",sig);
					break;
			case LDNS_RR_TYPE_MX:
					Translate_MX(a,rrs,add,ipv6,sig);
					break;

			default:
				break;

		};
		/*
		str = ldns_rr2str(rr->_rrs[n]);
		if (str) {
			printf ("str=%s\n",str);
			LDNS_FREE(str);
		}
		*/
		r.Set("[]",a);
	}
	//ldns_rr_list_print(stdout, rr);

	if (add) ldns_rr_list_deep_free(add);
	if (ipv6) ldns_rr_list_deep_free(ipv6);
	if (sig) ldns_rr_list_deep_free(sig);
	ldns_rr_list_deep_free(rr);


	//ldns_resolver *ldns=(ldns_resolver*)res;
	//printf ("res ok: %i\n", ldns_resolver_nameserver_count(ldns));
	return 1;
#endif
}


} // namespace ppl6
