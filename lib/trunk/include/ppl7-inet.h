/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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
PPLPARAMETERISEDEXCEPTION(IdnConversionException);

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
};

size_t GetHostByName(const String &name, std::list<IPAddress> &result,ResolverFlags flags=af_unspec);
size_t GetHostByAddr(const String &addr, std::list<IPAddress> &result);
String Ace2Idn(const String &ace);
String Idn2Ace(const String &idn);

String ToQuotedPrintable (const String &source);





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
