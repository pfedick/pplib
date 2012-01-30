/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.5 $
 * $Date: 2011/01/04 10:49:58 $
 * $Id: WikiParser.cpp,v 1.5 2011/01/04 10:49:58 pafe Exp $
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

#include "ppl7.h"
#include "ppl7-inet.h"

namespace ppl7 {

/*!\class WikiParser
 * \ingroup PPLGroupInternet
 * \brief Klasse zum Übersetzen von Wiki-Quelltext in HTML
 *
 * \desc
 * Mit dieser Klasse kann ein Wiki-ähnlicher Quelltext in HTML übersetzt werden.
 * \par Text-Formatierung:
 * <table>
 * <tr><th>Beschreibung</th><th>Wiki-Code</th><th>Ausgabe</th></tr>
 * <tr><td>Italic (kursiver) Text</td><td><tt>''italic''</tt></td><td><i>italic</i></td></tr>
 * <tr><td>Bold (fetter) Text</td><td><tt>'''bold'''</tt></td><td><b>bold</b></td></tr>
 * <tr><td>Bold und italic (fett und kursiv)</td><td><tt>'''''bold & italic'''''</tt></td><td><b><i>bold & italic</i></b></td></tr>
 * <tr><td>Unterbinden des Wiki-Markup</td><td><tt><nowiki>kein ''Markup''</nowiki></tt></td><td>kein ''Markup''</td></tr>
 * <tr><td>Überschriften in verschiedenen Grössen</td><td><pre>=Level 1=
==Level 2==
===Level 3===
====Level 4====
=====Level 5=====
======Level 6======
</pre></td><td>
<h1>Level 1</h1>
<h2>Level 2</h2>
<h3>Level 3</h3>
<h4>Level 4</h4>
<h5>Level 5</h5>
<h6>Level 6</h6>
</td></tr>
<tr><td>Horizontale Linie</td><td><tt>----</tt></td><td><hr></td></tr>
<tr><td>ungeordnete Liste</td><td><tt>* erste Zeile
<br>* zweite Zeile
<br>* dritte Zeile
<br>** dritte Zeile,erster Unterpunkt
<br>** dritte Zeile,zweiter Unterpunkt</tt></td><td><ul><li>erste Zeile</li>
<li>zweite Zeile</li>
<li>dritte Zeile</li>
<ul><li>dritte Zeile,erster Unterpunkt</li>
<li>dritte Zeile,zweiter Unterpunkt</li>
</ul></ul>
</td></tr>
<tr><td>geordnete Liste</td><td><pre># erste Zeile
# zweite Zeile
# dritte Zeile
## dritte Zeile,erster Unterpunkt
## dritte Zeile,zweiter Unterpunkt</pre></td><td>
<ol><li>erste Zeile</li>
<li>zweite Zeile</li>
<li>dritte Zeile</li>
<ol><li>dritte Zeile,erster Unterpunkt</li>
<li>dritte Zeile,zweiter Unterpunkt</li>
</ol></ol>
</td></tr>
<tr><td>Definitionsliste</td><td><pre>;Definition
:Beschreibung 1
:Beschreibung 2</pre></td><td><b>Definition</b><br>&nbsp;&nbsp;&nbsp;&nbsp;Beschreibung 1<br>
&nbsp;&nbsp;&nbsp;&nbsp;Beschreibung 2
<tr><td>Einrückungen</td><td><pre>Normaler Text
: Ein bischen eingerückt
:: Noch mehr eingerückt
::::::: ganz viel eingerückt
: nur einmal eingerückt</pre></td><td>Normaler Text
<br>&nbsp;&nbsp;&nbsp;&nbsp;Ein bischen eingerückt
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Noch mehr eingerückt
<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ganz viel eingerückt
<br>&nbsp;&nbsp;&nbsp;&nbsp;nur einmal eingerückt
</td></tr>
</table>

 * \par Forformatierter Text und Quellcode:
 * Ein Leerzeichen am Anfang einer Zeile führt dazu, dass der nachfolgende Text als vorformatiert
 * interpretiert wird. Alternativ dazu kann der Text aber auch in die HTML-Tags &lt;pre&gt; und &lt;/pre&gt;,
 * &lt;source&gt; und &lt;/source&gt; oder &lt;sourcecode&gt; und &lt;/sourcecode&gt; eingebettet werden.
 * Es ist geplant, dass bei Verwendung der beiden letzten Tags der darin enthaltene Quellcode optisch
 * aufbereitet wird. Dies ist jedoch noch nicht implementiert.
 *
 * \par Tabellen:
<table>
<tr>
<th>Wiki Code</th>
<th>vgl. HTML Code</th>
<th>Beschreibung</th>
</tr>
<tr>
<td>{|</td>
<td>&lt;table&gt;</td>
<td><b>Tabellenanfang</b></td>
</tr>
<tr>
<td>|+</td>

<td>&nbsp;</td>
<td><b>Tabellenüberschrift</b>, <i>optional;</i> nur einmal pro Tabelle zwischen Tabellenanfang und erster Zeile definierbar</td>
</tr>
<tr>
<td>|-</td>
<td>&lt;tr&gt;</td>
<td>neue <b>Tabellenzeile</b></td>
</tr>

<tr>
<td>!</td>
<td>&lt;th&gt;</td>
<td><b>Tabellenkopfzelle</b>, <i>optional.</i> Aufeinanderfolgende Kopfzellen können, durch doppelte Ausrufezeichen (!!) getrennt, in der gleichen Zeile folgen oder mit einfachen Ausrufezeichen (!) auf einer neuen Zeile beginnen.</td>
</tr>
<tr>
<td>|</td>
<td>&lt;td&gt;</td>

<td><b>Tabellenzelle</b>, <i>Pflicht!</i> Aufeinanderfolgende Zellen können, durch doppelte Striche (||) getrennt, in der gleichen Zeile folgen oder mit einfachem Strich (|) auf einer neuen Zeile beginnen.</td>
</tr>
<tr>
<td>|}</td>
<td>&lt;/table&gt;</td>
<td><b>Tabellenende</b></td>
</tr>
</table>
- Die angegebenen Zeichen müssen am Zeilenanfang stehen, ausgenommen die Doppelten || und !! für optional
  aufeinanderfolgende Zellen in einer Zeile.
- XHTML - Eigenschaften. Jede Markierung, außer das Tabellenende, kann optional eine oder mehrere
  XHTML Eigenschaften haben. Die Eigenschaften müssen in der gleichen Zeile eingegeben werden, für
  die sie auch gültig sind. Mehrere Eingaben müssen mit einem Leerzeichen getrennt eingegeben werden.

 * \par Links:
 * URLs, die mit "http://", "https://" oder "ftp://" beginnen, werden automatisch erkannt und als Link hinterlegt.
 * Es können aber auch Links im Wiki-Style verwendet werden, die in eckigen Klammern eingeschlossen werden. Diese erlauben
 * auch die Angabe eines Textes, der anstelle der URL angezeigt werden soll. Das Auftauchen von RFC-Nummern wird
 * automatisch verlinkt. Beispiele:
 <table><tr><th>Wiki-Code</th><th>Ergebnis</th></tr>
 <tr><td>
 \verbatim http://www.pfp.de/ \endverbatim
 </td><td><a href="http://www.pfp.de/">http://www.pfp.de/</a></td></tr>
 <tr><td>
 \verbatim ftp://www.pfp.de/ \endverbatim
 </td><td><a href="ftp://www.pfp.de/">ftp://www.pfp.de/</a></td></tr>
  <tr><td>
 \verbatim Post bitte an [gibtsnicht@pfp.de] schicken \endverbatim
 </td><td>Post bitte an <a href="mailto:gibtsnicht@pfp.de">gibtsnicht@pfp.de</a> schicken</td></tr>
  <tr><td>
 \verbatim Post bitte an [gibtsnicht@pfp.de diese Adresse] schicken \endverbatim
 </td><td>Post bitte an <a href="mailto:gibtsnicht@pfp.de">diese Adresse</a> schicken</td></tr>
 <tr><td>
 \verbatim [http://www.pfp.de/ Meine Webseite] \endverbatim
 </td><td><a href="http://www.pfp.de/">Meine Webseite</a></td></tr>
 <tr><td>
 \verbatim Die genaue Funktion ist in RFC 1234 beschrieben. \endverbatim
 </td><td>Die genaue Funktion ist in <a href="http://tools.ietf.org/html/rfc1234/">RFC 1234</a> beschrieben.</td></tr>
 </table>
 Der Parser kann um weitere Link-Erkennungen erweitert werden, in dem die virtuelle Funktion WikiParser::customParseLinks
 implementiert wird.
 *
 * \par Diagramme:
 * Dies ist kein Wiki-Feature, sondern eine Erweiterung des WikiParsers der PPL-Library.
 * \verbatim
<diagram name="Ein Beispieldiagramm">
<options>
<smallscale>4000</smallscale>
<bigscale>0</bigscale>
</options>
<items>
<item id="item1" color="#007000" name="Ergebnis Frage 1"/>
<item id="item2" color="#ff0000" name="Ergebnis Frage 2"/>
<item id="item3" color="#ffa000" name="Ergebnis Frage 3"/>
<item id="item4" color="#0000ff" name="Ergebnis Frage 4"/>
</items>
<section name="Gruppe 1">
<row id="item1" value="312"/>
<row id="item2" value="382"/>
<row id="item3" value="3345"/>
<row id="item4" value="60"/>
</section>
<section name="Gruppe 2">
<row id="item1" value="2714"/>
<row id="item2" value="1075"/>
<row id="item3" value="8310"/>
<row id="item4" value="100"/>
</section>
</diagram>
\endverbatim
ergibt:
\image html WikiParser_diagram.png
 */


WikiParser::WikiParser()
{
	init();
	indexenabled=true;
}

WikiParser::~WikiParser()
{

}

/*!\brief Interne Funktion zur Initialisierung des Parsers
 *
 * \desc
 * Diese Funktion wird intern vom Konstruktor und einigen anderen Funktionen aufgerufen,
 * um alle internen Variablen zu initialisieren.
 *
 */
void WikiParser::init()
{
	ispre=0;
	ullevel=0;
	ollevel=0;
	indexcount=0;
	intable=0;
	inrow=0;
	nowikicount=0;
	index.clear();
	nowiki.clear();
	ret.clear();
	precount=0;
	pre.clear();
	sourcecount=0;
	source.clear();
	indentlevel=0;
	doxyparamsStarted=false;
}


/*!\brief Komplette HTML-Seite mit Header und Stylesheet erstellen
 *
 * \desc
 * Mit dieser Funktion wird der Wiki-Quelltext in eine vollständige HTML-Seite mit Header
 * und Stylesheet übersetzt.
 *
 * @param[in] Source String mit dem Wiki Quelltext
 * @param[out] Html String mit der HTML-Übersetzung
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 */
int WikiParser::render(const String &Source, String &Html)
{
	init();
	if (!getHeader(Html)) return 0;
	int ret=renderInternal(Source,Html);
	Html+="</body></html>\n";
	//Html.Print(true);
	return ret;
}

/*!\brief Komplette HTML-Seite mit Header und Stylesheet erstellen
 *
 * \desc
 * Mit dieser Funktion wird der Wiki-Quelltext in eine vollständige HTML-Seite mit Header
 * und Stylesheet übersetzt.
 *
 * @param[in] Source String mit dem Wiki Quelltext
 * @return Die Funktion gibt einen String mit der HTML-Übersetzung zurück
 */
String WikiParser::render(const String &Source)
{
	String res;
	render(Source,res);
	return res;
}

/*!\brief Nur den HTML-Body erstellen
 *
 * \desc
 * Mit dieser Funktion wird nur der Wiki-Quelltext in HTML übersetzt, es wird kein
 * HTML-Header oder Stylesheet eingebaut.
 *
 * @param[in] Source String mit dem Wiki Quelltext
 * @param[out] Html String mit der HTML-Übersetzung
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0
 */
int WikiParser::renderBody(const String &Source, String &Html)
{
	init();
	int ret=renderInternal(Source,Html);
	return ret;
}

/*!\brief Nur den HTML-Body erstellen
 *
 * \desc
 * Mit dieser Funktion wird nur der Wiki-Quelltext in HTML übersetzt, es wird kein
 * HTML-Header oder Stylesheet eingebaut.
 *
 * @param[in] Source String mit dem Wiki Quelltext
 * @return Die Funktion gibt einen String mit der HTML-Übersetzung zurück
 */
String WikiParser::renderBody(const String &Source)
{
	String res;
	renderBody(Source,res);
	return res;
}


int WikiParser::getHeader(String &Html)
{
	Html="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd\">\n";
	Html+="<html><head><style type=\"text/css\">\n";
	Resource *res=GetPPLResource();
	if (res) {
		ByteArrayPtr mem=res->getMemory(50);
		if (!mem.isEmpty()) Html.append((const char*)mem.adr(),mem.size());
	}
	Html+="</style>\n";
	Html+="</head><body>\n";
	return 1;
}

String WikiParser::header()
{
	String Html;
	getHeader(Html);
	return Html;
}

void WikiParser::setIndexEnabled(bool enabled)
{
	indexenabled=enabled;
}

void WikiParser::setBaseURI(const String &Uri)
{
	BaseURI=Uri;
}

void WikiParser::extractNoWiki(String &Text)
{
	Array matches;
	while (Text.pregMatch("/^(.*)<nowiki>[\\n]*(.*?)<\\/nowiki>(.*)$/ism",matches)) {
		nowikicount++;
		nowiki.set(nowikicount,matches[2]);
		Text.setf("%ls<nowiki %i>%ls",matches.getPtr(1),nowikicount,matches.getPtr(3));
	}

}

void WikiParser::extractSourcecode(String &Text)
{
	String Tmp;
	Text.pregReplace("/^\\\\code$/im","<source>");
	Text.pregReplace("/^\\\\endcode$/im","</source>");

	Text.pregReplace("/<sourcecode>/i","<source>");
	Text.pregReplace("/</sourcecode>/i","</source>");
	Array matches;
	//while (Text.PregMatch("/^(.*)<source.*?>[\\n]*(.*?)<\\/source>(.*)$/ism")) {
	// Die Verwendung von .*? hinter source führt dazu, dass das Parsen ewig lange dauert!
	while (Text.pregMatch("/^(.*)<source>[\\n]*(.*?)<\\/source>(.*)$/ism",matches)) {
		sourcecount++;
		Tmp=matches[2];
		Tmp.replace("\t","    ");
		source.set(sourcecount,Tmp);
		Text.setf("%ls<source %i>%ls",matches.getPtr(1),sourcecount,matches.getPtr(3));
	}
}

void WikiParser::extractDiagrams(String &Text)
{
	String d;
	Array matches;
	while (Text.pregMatch("/^(.*?)(<diagram.*?>.*?<\\/diagram>)(.*)$/ism",matches)) {
		d=xmlDiagram2HTML(matches[2]);
		diagrams.add(d);
		//printf ("Diagramm: >>>%s<<<\n",(const char*)d);
		Text.setf("%ls<tmp_diagram %i>%ls",matches.getPtr(1),diagrams.size(),matches.getPtr(3));
	}
}

void WikiParser::parseHeadlines(String &Line)
{
	String Key;
	// Auf die Reihenfolge kommt es an
	Line.pregReplace("/'''''(.*?)'''''/","<b><i>$1</i></b>");
	Line.pregReplace("/'''(.*?)'''/","<b>$1</b>");
	Line.pregReplace("/''(.*?)''/","<i>$1</i>");
	Line.pregReplace("/^----$/","<hr>");
	Line.pregReplace("/^====== (.*?) ======$/","<h6>$1</h6>");
	Line.pregReplace("/^===== (.*?) =====$/","<h5>$1</h5>");
	Line.pregReplace("/^==== (.*?) ====$/","<h4>$1</h4>");
	Line.pregReplace("/^=== (.*?) ===$/","<h3>$1</h3>");
	Line.pregReplace("/^== (.*?) ==$/","<h2>$1</h2>");
	Line.pregReplace("/^= (.*?) =$/","<h1>$1</h1>");

	// Index aufbauen
	Array matches;
	if (Line.pregMatch("/\\<h([0-9]+)\\>(.*?)\\<\\/h[0-9]+\\>/i",matches)) {
		indexcount++;
		Line=ToString("<a name=\"index_%i\"></a>%ls",indexcount,(const wchar_t*)Line);
		Key.setf("%i/ebene",indexcount);
		index.set(Key,matches[1]);
		Key.setf("%i/link",indexcount);
		index.setf(Key,"<a href=\"#index_%i\">%ls</a>",indexcount,matches.getPtr(2));
		//echo "match $match[1]<br>";
	}
}

void WikiParser::doxygenChapter(String &Line, const String &Name, const Array &Matches)
{
	String Tmp;
	Line="";
	for (int i=indentlevel;i>0;i--) Line+="</div>";
	indentlevel=0;
	Line+="<b>";
	Line+=Name;
	Line+="</b><div style=\"margin-left: 30px;\">";
	Tmp=Matches[1];
	Tmp.trim();
	if (Tmp.notEmpty()) Line+=Tmp;
	else nobr=true;
	indentlevel++;
}

void WikiParser::parseDoxygen(String &Line)
{
	String Tmp;
	Array matches;
	Line.pregReplace("/\\\\p\\s([a-z_A-Z_0-9]+)/","<b style=\"color: #005000;\">$1</b>");
	Line.pregReplace("/\\\\b\\s([^\\s]+)/","<b>$1</b>");
	Line.pregReplace("/^\\\\brief\\s(.*)$/","$1<br>");
	if (Line.pregMatch("/^\\\\desc(.*)$/i", matches)) {
		doxygenChapter(Line,"Beschreibung:",matches);
	}
	if (Line.pregMatch("/^\\\\return(.*)$/i",matches)) {
		doxygenChapter(Line,"Rückgabe:",matches);
	}
	if (Line.pregMatch("/^\\\\example(.*)$/i",matches)) {
		doxygenChapter(Line,"Beispiel:",matches);
	}
	if (Line.pregMatch("/^\\\\history(.*)$/i",matches)) {
		doxygenChapter(Line,"Historie:",matches);
	}

	if (Line.pregMatch("/^\\\\sourcecode(.*)$/i",matches)) {
		doxygenChapter(Line,"Quellcode:",matches);
	}

	if (Line.pregMatch("/^\\\\note(.*)$/i",matches)) {
		doxygenChapter(Line,"Hinweis:",matches);
	}
	if (Line.pregMatch("/^\\\\attention(.*)$/i",matches)) {
		doxygenChapter(Line,"Achtung:",matches);
	}

	if (Line.pregMatch("/^\\\\par$/i")) {
		Line="";
		for (int i=indentlevel;i>0;i--) Line+="</div>";
		indentlevel=0;
		Line+="<div style=\"margin-left: 30px;\">";
		indentlevel++;
	}
	if (Line.pregMatch("/^\\\\syntax\\s+(.*)$/i",matches)) {
		Line="";
		for (int i=indentlevel;i>0;i--) Line+="</div>";
		indentlevel=0;
		Line="<b>Syntax:</b><div style=\"margin-left: 30px;\">";
		String s=matches[1];
		if (s.pregMatch("/^(.+)\\s+(.+)\\s*\\((.*)\\)$/",matches)) {
			Line.appendf("<span style=\"color: #400000;\">%ls</span> <b>%ls</b>(",
					matches.getPtr(1), matches.getPtr(2));
			Array Tok;
			StrTok(Tok,matches[3],",");
			// TODO
			const char *t;
			int c=0;
			while ((t=Tok.GetNext())) {
				String tt=t;
				if (tt.PregMatch("/^(.*)\\s+(.*)$/")) {
					if (c) Line+=", ";
					Line.Concatf("<span style=\"color: #400000;\">%s</span> <b style=\"color: #005000;\">%s</b>",
							tt.GetMatch(1),tt.GetMatch(2));
					c++;
				}

			}
		}
		Line+=")</div>";
	}

	if (Line.PregMatch("/^\\\\param\\s*\\[(.+)\\]\\s{0}(.+?)\\s+(.*)$/i")) {
		Line="";
		if (!doxyparamsStarted) {
			for (int i=indentlevel;i>0;i--) Line+="</div>";
			indentlevel=0;
			doxyparamsStarted=true;
			Line+="<b>Parameter:</b><div style=\"margin-left: 30px;\">\n";
			indentlevel++;
		}

		Line+="<ul><li>[";
		Line+=Line.GetMatch(1);
		Line+="] <b style=\"color: #005000;\">";
		Line+=Line.GetMatch(2);
		Line+="</b> ";
		Line+=Line.GetMatch(3);
		Line+="</li></ul>";
		nobr=true;
	}
	if (Line.PregMatch("/^\\\\param\\s+(.+?)\\s+(.*)$/i")) {
		Line="";
		if (!doxyparamsStarted) {
			for (int i=indentlevel;i>0;i--) Line+="</div>";
			indentlevel=0;
			doxyparamsStarted=true;
			Line+="<b>Parameter:</b><div style=\"margin-left: 30px;\">\n";
			indentlevel++;
		}
		Line+="<ul><li><b style=\"color: #005000;\">";
		Line+=Line.GetMatch(1);
		Line+="</b> ";
		Line+=Line.GetMatch(2);
		Line+="</li></ul>";
	}

}

int WikiParser::parseUL(String &Line)
{
	// Aufzählung UL?
	if (Line[0]=='*') {
		if (!ullevel) {
			ullevel=1;
			ret+="<ul>";
		}
		// Wie tief?
		int c=1;
		//$ul="";
		while (Line[c]=='*') c++;
		Line=Line.SubStr(c);
		Line.Trim();
		ret.Concatf("<li style=\"margin-left: %ipx;\">%s</li>\n",((c-1)*20),(const char*)Line);
		return 1;
	} else if (ullevel>0) {
		ret+="</ul>\n";
		ullevel=0;
	}
	return 0;
}

int WikiParser::parseIndent(String &Line)
{
	int c=0;
	while (Line[c]==':') c++;
	if (c>0) {
		Line=Line.SubStr(c);
		Line.Trim();
		ret.Concatf("<div style=\"margin-left: %ipx;\">%s</div>\n",(c*20),(const char*)Line);
		return 1;
	}
	if (Line[0]==';') {
		Line=Line.SubStr(1);
		Line.Trim();
		Line="<b class=\"definition\">"+Line+"</b>";
		return 0;
	}
	return 0;
}

int WikiParser::parseOL(String &Line)
{
	// Aufzählung OL?
	//printf ("OL-Prüfung auf: >>>%s\n",(const char*)Line);
	if (Line[0]=='#') {
		// Wie tief?
		int c=1;
		while (Line[c]=='#') c++;
		//printf("OL-Match, ollevel=%i, c=%i\n",ollevel,c);
		Line=Line.SubStr(c);
		Line.Trim();
		if (c>ollevel) for (int i=ollevel;i<c;i++) ret+="<ol>";
		if (c<ollevel) for (int i=ollevel;i>c;i--) ret+="</ol>";
		ollevel=c;
		ret.Concatf("<li style=\"margin-left: %ipx;\">%s</li>\n",((c-1)*20),(const char*)Line);
		//ret.Print(true);
		return 1;
	} else if (ollevel>0) {
		//printf("OL-Failed, ollevel=%i\n",ollevel);
		for (int i=ollevel;i>0;i--) ret+="</ol>";
		ret+="\n";
		//ret.Print(true);
		ollevel=0;
	}
	return 0;
}

void WikiParser::parseAutoPRE(String &Line)
{
	// Zeilen mit Space am Anfang?
	if (Line[0]==' ') {
		if (!ispre) ret+="<pre>";
		ispre++;
		nobr=true;
	} else if (ispre) {
		ret+="</pre>";
		ispre=0;
	}
}

void WikiParser::parseTable(String &Line)
{
	CArray Match;
	String Tmp;
	if (Line.PregMatch("/^\\{\\|(.*)$/",&Match)) {
		//printf ("Table match: %s\n",(const char*)Line);
		intable=1;
		Line.Setf("<table %s>\n",Match[1]);
		nobr=true;
	}
	if (intable) {
		//printf ("Table-Line vorher: >>%s<<\n",(const char *)Line);
		if (Line.PregMatch("/^\\|\\}(.*)$/", &Match)) {
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			if (inrow) Line+="</tr>\n";
			intable=0;
			incol="";
			inrow=0;
			Line+="</table>\n";
			Line+=Match[1];
			nobr=true;
		}
		if (Line.PregMatch("/^\\|-(.*)$/u",&Match)) {
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			if (inrow) Line+="</tr>\n";
			incol="";
			inrow=1;
			Line.Concatf("<tr %s>\n",Match[1]);
			nobr=true;
		}
		if (Line.PregMatch("/^\\|([^\\|]+)\\|([^\\|].*)$/us",&Match)) {		// Spalte mit HTML-Parameter: |parameter=value...|Inhalt
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="td";
			Line.Concatf("<td %s>",Match[1]);
			Tmp=Match[2];
			Tmp.Trim();
			if (Tmp.NotEmpty()) Line+=Tmp; // Falls Inhalt leer ist, unterdrücken wir den Zeilenumbruch
			else nobr=true;
		} else if (Line.PregMatch("/^\\|([^\\|]+)\\|$/us",&Match)) {		// Spalte mit HTML-Parameter: |parameter=value...|Inhalt
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="td";
			Line.Concatf("<td %s>",Match[1]);
			nobr=true;
		} else if (Line.PregMatch("/^\\|(.*)$/us",&Match)) {
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="td";
			Line+="<td>";
			Line+=Match[1];
		}
		while (Line.PregMatch("/^(.*)\\|\\|(.*)$/us",&Match)) {
			Line=Match[1];
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="td";
			Line+="<td>";
			Line+=Match[2];
		}
		//printf ("Table-Line nachher: >>%s<<\n",(const char *)Line);
		if (Line.PregMatch("/^\\!([^\\|\\!]+)[\\|\\!]([^\\|\\!].*)$/u",&Match)) {		// Spalte mit HTML-Parameter: |parameter=value...|Inhalt
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="th";
			Line.Concatf("<th %s>",Match[1]);
			Tmp=Match[2];
			Tmp.Trim();
			if (Tmp.NotEmpty()) Line+=Tmp; // Falls Inhalt leer ist, unterdrücken wir den Zeilenumbruch
			else nobr=true;
		} else if (Line.PregMatch("/^\\!([^\\|]+)\\|$/u",&Match)) {		// Spalte mit HTML-Parameter: |parameter=value...|Inhalt
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="th";
			Line.Concatf("<th %s>",Match[1]);
			nobr=true;
		} else if (Line.PregMatch("/^\\!(.*)$/u",&Match)) {
			Line="";
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="th";
			Line.Concatf("<th>%s",Match[1]);
		}
		while (Line.PregMatch("/^(.*)\\!\\!(.*)$/us",&Match)) {
			Line=Match[1];
			if (incol.NotEmpty()) Line.Concatf("</%s>",(const char*)incol);
			incol="th";
			Line.Concatf("<th>%s",Match[2]);
		}
	}
}

void WikiParser::parseLinks(String &Line)
{
	String Tmp;
	// Escapete Klammern merken
	Line.Replace("\\[","___@1___");
	Line.Replace("\\]","___@2___");

	// RFC-Links
	Line.PregReplace("/RFC ([0-9]+)/i","[http://tools.ietf.org/html/rfc$1 RFC $1]");

	// Externe Links ohne Klammern
	Line.PregReplace("/([^\\[]+)(https?:\\/\\/[\\S]+)/i","$1[$2]");
	Line.PregReplace("/^(https?:\\/\\/[\\S]+)/i","[$1]");

	Line.PregReplace("/([^\\[]+)(ftp:\\/\\/[\\S]+)/i","$1[$2]");
	Line.PregReplace("/^(ftp:\\/\\/[\\S]+)/i","[$1]");

	// Mail-Links
	//Line.PregReplace("/([^\\[]*)([^\\s]+\\@[^\\s]+)/","$1<a class=\"maillink\" href=\"mailto:$2\">$2</a>");
	Line.PregReplace("/\\[([^\\s\\@]+\\@.*?)\\s+(.*?)\\]/","<a class=\"maillink\" href=\"mailto:$1\">$2</a>");
	Line.PregReplace("/\\[([^\\s\\@]+\\@.*?)\\]/","<a class=\"maillink\" href=\"mailto:$1\">$1</a>");

	// Benutzer-individuelle Links
	customParseLinks(Line);

	// Wiki HTTP[S]-Links
	Line.PregReplace("/\\[(https?:\\/\\/.*?)\\s(.*?)\\]/i","<a href=\"$1\">$2</a>");
	Line.PregReplace("/\\[(https?:\\/\\/.*?)\\]/i","<a href=\"$1\">$1</a>");

	Line.PregReplace("/\\[(ftp:\\/\\/.*?)\\s(.*?)\\]/i","<a href=\"$1\">$2</a>");
	Line.PregReplace("/\\[(ftp:\\/\\/.*?)\\]/i","<a href=\"$1\">$1</a>");

	// Wiki-Links
	while (Line.PregMatch("/^(.*?)\\[\\[(.*?)\\s+(.*?)\\]\\](.*?)$/")) {
		if (BaseURI.NotEmpty()) {
			Tmp.Setf("%s<a href=%s%s>%s</a>%s",Line.GetMatch(1),(const char*)BaseURI,Line.GetMatch(2),Line.GetMatch(3),Line.GetMatch(4));
		} else {
			Tmp.Setf("%s<a href=%s>%s</a>%s",Line.GetMatch(1),Line.GetMatch(2),Line.GetMatch(3),Line.GetMatch(4));
		}
		Line=Tmp;
	}

	// Escapete Klammern zurückwandeln
	Line.Replace("___@1___","[");
	Line.Replace("___@2___","]");
}

void WikiParser::customParseLinks(String &Line)
{

}

void WikiParser::buildIndex(String &Html)
{
	String Tmp;
	// Index hinzufügen
	if (index.Count()>3 && indexenabled==true) {
		Html+="<table class=index><tr><td>";
		Html+="<b>Inhaltsverzeichnis:</b><br>\n";
		Tmp="";
		//index.List("index");
		index.Reset();
		CAssocArray *r;
		String a;
		while ((r=index.GetNextArray())) {
			a.Repeat('*',r->ToInt("ebene"));
			a.Concatf(" %s\n",r->Get("link"));
			Tmp+=a;
		}
		a.Clear();
		WikiParser Wiki;
		Wiki.renderInternal(Tmp,a);
		Html+=a;
		Html+="</td></tr></table>\n";
	}
}

void WikiParser::finalize()
{
	CArray Match;
	for (int i=0;i<ullevel;i++) ret+="</ul>\n";
	for (int i=0;i<ollevel;i++) ret+="</ol>\n";
	for (int i=0;i<ispre;i++) ret+="</pre>\n";

	for (int i=indentlevel;i>0;i--) ret+="</div>";
	indentlevel=0;


	// <br> am Ende wollen wir nicht
	ret.PregReplace("/<br>$/i","");
	// <br> vor einem </td> oder </th> wollen wir nicht
	//ret.PregReplace("/<br>\\n(<\\/t[dh]>)/i","$1");		// TODO: BUG in PPL6!!!! $1 scheint Random-Daten einzufügen
	ret.PregReplace("/<br>\\n(<\\/td>)/i","</td>");
	ret.PregReplace("/<br>\\n(<\\/th>)/i","</th>");
	// <br> vor einem <ul> oder <ol> wollen wir nicht
	ret.PregReplace("/<br>\\n+<ul>/i","<ul>");
	ret.PregReplace("/<br>\\n+<ol>/i","<ol>");

	ret.PregReplace("/<pre><br>/i","<pre>");


	// Leere Tabellen-Zellen wollen wir sehen
	while (ret.PregMatch("/(.*)(<t[dh]>)\\s*(<\\/t[dh]>)(.*)/i",&Match)) {
		ret.Setf("%s%s&nbsp;%s%s",Match[1],Match[2],Match[3],Match[4]);
	}
	finalizeNoWiki();
	finalizePRE();
	finalizeSource();
	finalizeDiagrams();
	ret.Replace("\\n","<br>\n");
	// Nach einem </div> wollen wir maximal 1 <br>
	ret.PregReplace("/<\\/div>\\n<br>\\n<br>/i","</div>\n<br>\n");
}

void WikiParser::finalizeNoWiki()
{
	if (nowikicount>0) {
		while (ret.PregMatch("/^(.*)<nowiki ([0-9]+)>(.*)$/ims")) {
			ret.Setf("%s%s%s",ret.GetMatch(1),nowiki[atoi(ret.GetMatch(2))],ret.GetMatch(3));
		}
	}
}

void WikiParser::finalizePRE()
{
	CArray Match;
	CAssocArray *p;
	if (precount>0) {
		while (ret.PregMatch("/^(.*)<pre ([0-9]+)>(.*)$/sim",&Match)) {
			p=pre.GetArray(Match[2]);
			if (p) {
				if (p->Get("class")) {
					// if ($pre[$n]["class"]) $ret=$match[1]."<pre class=\"".$pre[$n]["class"]."\">\n".$pre[$n][content]."</pre>\n".$match[3];
					ret.Setf("%s<pre class=\"%s\">\n%s</pre>\n%s",Match[1],p->Get("class"),p->Get("content"),Match[3]);
				} else {
					//else $ret=$match[1]."<pre>\n".$pre[$n][content]."</pre>\n".$match[3];
					ret.Setf("%s<pre>\n%s</pre>\n%s",Match[1],p->Get("content"),Match[3]);
				}
			} else {
				ret=Match[1];
				ret+=Match[3];
			}
		}
	}
}

void WikiParser::finalizeSource()
{
	CArray Match;
	if (sourcecount>0) {
		while (ret.PregMatch("/^(.*)<source ([0-9]+)>(.*)$/sim",&Match)) {
			ret=Match[1];
			ret+="<div class=\"source\">"+EscapeHTMLTags(source[atoi(Match[2])]);
			ret+="</div>";
			ret+=Match[3];
		}
	}
}

void WikiParser::finalizeDiagrams()
{
	CArray Match;
	if (diagrams.Num()) {
		while (ret.PregMatch("/^(.*)<tmp_diagram ([0-9]+)>(.*)$/sim",&Match)) {
			ret.Setf("%s\n%s\n%s",Match[1],diagrams[atoi(Match[2])-1],Match[3]);
		}
	}
}


int WikiParser::renderInternal(const String &Source, String &Html)
{
	String Tmp;
	String Text=Source;
	Text.Replace("\r\n","\n");

	extractNoWiki(Text);
	extractSourcecode(Text);
	extractDiagrams(Text);

	String Line;

	CArray Rows;
	Rows.Explode(Text,"\n");
	const char *line;
	CArray Match;



	while((line=Rows.GetNext())) {
		Line=line;
		nobr=false;
		parseHeadlines(Line);
		parseLinks(Line);
		parseDoxygen(Line);

		if (parseOL(Line)) continue;
		if (parseUL(Line)) continue;
		if (parseIndent(Line)) continue;


		parseTable(Line);
		parseAutoPRE(Line);		// Zeilen mit Space am Anfang?

		// An bestimmten Stellen wollen wir am Ende kein <br>

		if (Line.PregMatch("/^.*<\\/li>$/i")) nobr=true;
		if (Line.PregMatch("/^.*<nobr>$/i")) {
			Line.PregReplace("/<nobr>$/i","");
			nobr=true;
		}

		Line.Trim();
		ret+=Line;
		if (!nobr) ret+="<br>";
		ret+="\n";
	}
	finalize();

	if (indexenabled) buildIndex(Html);	// Index hinzufügen
	Html+=ret;
	return 1;

}




String WikiParser::xmlDiagram2HTML(const String &xml)
{
	String Body,Tmp, Param, d, Row, Text;
	CAssocArray opt, items, a;
	int smallscale=50;
	int bigscale=100;
	Text=xml;
	if (Text.PregMatch("/^(.*?)<diagram(.*?)>[\\n]*(.*?)<\\/diagram>(.*)$/ism")) {
		Param=Text.GetMatch(2);
		Body=Text.GetMatch(3);
		d.Clear();
		if (Param.PregMatch("/name=\"(.*?)\"/i")) {
			d+="<b>";
			d+=Param.GetMatch(1);
			d+="</b><br>\n";
		}
		if (Body.PregMatch("/<smallscale>(.*?)<\\/smallscale>/is")) smallscale=atoi(Body.GetMatch(1));
		if (Body.PregMatch("/<bigscale>(.*?)<\\/bigscale>/is")) bigscale=atoi(Body.GetMatch(1));

		if (bigscale==0) {
			String Backup=Body;
			while (Body.PregMatch("/^(.*)<section(.*?)>(.*?)<\\/section>(.*)$/is")) {
				Param=Body.GetMatch(2);
				Tmp=Body.GetMatch(3);
				Body=Body.GetMatch(1);
				Body+=Body.GetMatch(4);
				while (Tmp.PregMatch("/^(.*)(<row.*?)\\/>(.*)$/is")) {
					Row=Tmp.GetMatch(2);
					Tmp=Tmp.GetMatch(1);
					Tmp+=Tmp.GetMatch(3);
					String value;
					if (Row.PregMatch("/value=\"(.*?)\"/is")) value=Row.GetMatch(1);
					if (value.ToInt()>bigscale) bigscale=value.ToInt();
				}
			}
			Body=Backup;
			if (bigscale==0) bigscale=100;
		}

		if (Body.PregMatch("/<items>(.*?)<\\/items>/is")) {
			Tmp=Body.GetMatch(1);
			while (Tmp.PregMatch("/^(.*)(<item.*?)\\/>(.*)$/is")) {
				Row=Tmp.GetMatch(2);
				Tmp=Tmp.GetMatch(1);
				Tmp+=Tmp.GetMatch(3);
				//printf ("row: %s\n",(const char*)Row);
				String id, color,name;
				if (Row.PregMatch("/id=\"(.*?)\"/is")) id=Row.GetMatch(1);
				if (Row.PregMatch("/color=\"(.*?)\"/is")) color=Row.GetMatch(1);
				if (Row.PregMatch("/name=\"(.*?)\"/is")) name=Row.GetMatch(1);
				if (id.NotEmpty()) {
					a.Clear();
					a.Set("color",color);
					a.Set("name",name);
					a.Set("id",id);
					items.Set(id,a);
				}

			}
			//items.List("items");
		}

		while (Body.PregMatch("/^(.*?)<section(.*?)>(.*?)<\\/section>(.*)$/is")) {
			Param=Body.GetMatch(2);
			Tmp=Body.GetMatch(3);
			Body=Body.GetMatch(1);
			Body+=Body.GetMatch(4);
			//printf ("Section: >>>%s<<<\n",(const char*)Row);
			//printf ("Param: >>>%s<<<\n",(const char*)Param);
			d+="<div style=\"border: 1px solid black; margin-bottom: 6px;\">\n";
			if (Param.PregMatch("/name=\"(.*?)\"/is")) d+="<div style=\"background: white; color: black; font-size: 80%; font-weight: bold;\">"+ToString("%s",Param.GetMatch(1))+"</div>\n";
			d+="<table style=\"width: 100%; background: #f0f0f0;\">\n";
			while (Tmp.PregMatch("/^(.*?)(<row.*?)\\/>(.*)$/is")) {
				Row=Tmp.GetMatch(2);
				Tmp=Tmp.GetMatch(1);
				Tmp+=Tmp.GetMatch(3);
				String id,value;
				if (Row.PregMatch("/id=\"(.*?)\"/is")) id=Row.GetMatch(1);
				if (Row.PregMatch("/value=\"(.*?)\"/is")) value=Row.GetMatch(1);
				if (id.NotEmpty()) {
					CAssocArray *i=items.GetArray(id);
					if (i!=NULL && i->Count()>0) {
						d+="<tr><td style=\"font-size: 80%;\">"+i->ToString("name").Replace(" ","&nbsp;")+"</td>";
						d+="<td style=\"width: 100%; font-size: 80%;\">";
						int v=0;
						if (smallscale==0) v=value.ToInt()*80/bigscale;
						else {
							if (value.ToInt()<=smallscale) {
								v=value.ToInt()*40/smallscale;
							} else {
								int diff=bigscale-smallscale;
								if (diff==0) diff=1;
								v=40+(value.ToInt()-smallscale)*40/(diff);
							}
						}
						v++;
						if (v>80) v=80;
						d+="<div style=\"margin-right: 10px; border-bottom: 1px solid #000000; "
								"border-right: 1px solid #000000; border-top: 1px solid #ffffff; "
								"border-left: 1px solid #ffffff; "
								"background: "+i->ToString("color")+"; float: left; width: ";
						d.Concatf("%i",v);
						d+="%;\">&nbsp;</div>";
						d+=value;
						d+="</td></tr>\n";
					}
				}
			}
			d+="</table></div>\n";
		}

	}


	return d;
}

} // namespace ppl7
