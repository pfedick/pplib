/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 407 $
 * $Date: 2012-06-06 22:31:39 +0200 (Mi, 06. Jun 2012) $
 * $Id: Functions.cpp 407 2012-06-06 20:31:39Z pafe $
 * $URL: svn+ssh://pafe@svn.code.sf.net/p/pplib/code/lib/trunk/src/core/Functions.cpp $
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// Keine MFCs
#include <windows.h>
#endif

#include "ppl7.h"

namespace ppl7 {

typedef struct tagSections {
	struct tagSections *last, *next;
	String name;
	AssocArray values;
} SECTION;

/*!\class ConfigParser
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Klasse können Konfigurationsdateien mit mehreren Sektionen geladen und
 * geschrieben werden.
 *
 * \example
 * Beispiel einer Konfigurationsdatei:
\code
[server]
ip=127.0.0.1
port=8080
path=/var/tmp

[user]
# In dieser Sektion werden Username und Passwort gespeichert
root=fhrquofhqufhqo
patrick=kngbhwloh

[allowedhosts]
# In dieser Sektion befinden sich IP-Adressen oder Hostnamen von den Rechnern,
# denen der Zugriff auf das Programm erlaubt ist.
192.168.0.1
192.168.0.2
192.168.0.3
192.168.0.4
\endcode
 */

ConfigParser::ConfigParser()
{
	init();
}

ConfigParser::ConfigParser(const String &filename)
{
	init();
	load(filename);
}

ConfigParser::ConfigParser(FileObject &file)
{
	init();
	load(file);
}

ConfigParser::~ConfigParser()
{
	unload();
}


void ConfigParser::init()
{
	setSeparator("=");
	isused=false;
	first=last=section=NULL;
}

void ConfigParser::unload()
{
	sections.clear();
	SECTION *s=(SECTION *)first;
	while (s) {
		s=s->next;
		delete ((SECTION *)first);
		first=s;
	}
	section=first=last=NULL;
	if (isused) {
		isused=false;
	}
	currentsection.clear();
}

void ConfigParser::setSeparator(const String &string)
{
	separator=string;
	if (separator.isEmpty()) separator="=";
	separatorLength=separator.length();
}

void ConfigParser::selectSection(const String &sectionname)
{
	section=findSection(sectionname);
	if (!section) throw KeyNotFoundException(sectionname);
}

void *ConfigParser::findSection(const String &sectionname) const
{
	SECTION *s=(SECTION *)first;
	while (s) {
		if (sectionname.strcasecmp(s->name)==0) return (void *) s;
		s=s->next;
	}
	return NULL;
}

int ConfigParser::firstSection()
{
	section=first;
	if (section) return 1;
	return 0;
}

int ConfigParser::nextSection()
{
	SECTION *s=(SECTION *)section;
	if (!s) return 0;
	section=s->next;
	if (!section) return 0;
	return 1;
}

const String & ConfigParser::getSectionName() const
{
	if (!section) throw NoSectionSelectedException();
	return ((SECTION*)section)->name;
}


void ConfigParser::createSection(const String &sectionname)
{
	if (findSection(sectionname)) return;		// Section existiert bereits
	SECTION *s;
	s=new SECTION;
	if (!s) {
		throw OutOfMemoryException();
	}
	s->name=sectionname;
	s->next=NULL;
	s->last=(SECTION *) last;				// In die Kette haengen
	if (last) ((SECTION *)last)->next=s;
	last=s;
	if (!first) first=s;
	section=s;
	currentsection=sectionname;
}

void ConfigParser::deleteSection(const String &sectionname)
{
	SECTION *s=(SECTION *)findSection(sectionname);
	if (!s) return;
	if (s->last) s->last->next=s->next;		// aus der Kette nehmen
	if (s->next) s->next->last=s->last;
	if (s==(SECTION *)last) last=s->last;
	if (s==(SECTION *)first) first=s->next;
	delete(s);
}

void ConfigParser::reset()
{
	if (!section) return;
	return ((SECTION *)section)->values.reset(it);
}

bool ConfigParser::getFirst(String &key, String &value)
{
	if (!section) return false;
	return ((SECTION *)section)->values.getFirst(it,key,value);
}

bool ConfigParser::getNext(String &key, String &value)
{
	if (!section) return false;
	return ((SECTION *)section)->values.getNext(it,key,value);
}

void ConfigParser::add(const String &section, const String &key, const String &value)
{
	SECTION *s=(SECTION *)findSection(section);
	if (!s) {
		createSection(section);
		s=(SECTION *)findSection(section);
		if (!s) throw UnknownSectionException(section);
	}
	s->values.append(key,value);
}

void ConfigParser::add(const String &section, const String &key, int value)
{
	add(section,key,ToString("%i",value));
}

void ConfigParser::add(const String &section, const String &key, bool value)
{
	add(section,key,ToString("%s",(value?"yes":"no")));
}


void ConfigParser::add(const String &key, const String &value)
{
	if (!section) throw NoSectionSelectedException();
	add(((SECTION *)section)->name,key,value);
}

void ConfigParser::add(const String &key, int value)
{
	if (!section) throw NoSectionSelectedException();
	add(((SECTION *)section)->name,key,value);
}

void ConfigParser::add(const String &key, bool value)
{
	if (!section) throw NoSectionSelectedException();
	add(((SECTION *)section)->name,key,value);
}

void ConfigParser::deleteKey(const String &key)
{
	if (!section) throw NoSectionSelectedException();
	return deleteKey(((SECTION *)section)->name,key);
}

void ConfigParser::deleteKey(const String &section, const String &key)
{
	SECTION *s=(SECTION *)findSection(section);
	if (!s) return;
	return s->values.remove(key);
}

String ConfigParser::get(const String &key, const String &defaultvalue)
{
	if (!section) throw NoSectionSelectedException();
	String ret;
	try {
		ret=((SECTION *)section)->values.get(key);
	} catch (...) {
		return defaultvalue;
	}
	return ret;
}

String ConfigParser::get(const String &section, const String &key, const String &defaultvalue)
{
	SECTION *s=(SECTION *)findSection(section);
	if (!s) return defaultvalue;
	String ret;
	try {
		ret=s->values.get(key);
	} catch (...) {
		return defaultvalue;
	}
	return ret;
}

bool ConfigParser::getBool(const String &section, const String &key, bool defaultvalue)
{
	SECTION *s=(SECTION *)findSection(section);
	if (!s) return defaultvalue;
	String ret;
	try {
		ret=s->values.get(key);
	} catch (...) {
		return defaultvalue;
	}
	return IsTrue(ret);
}

bool ConfigParser::getBool(const String &key, bool defaultvalue)
{
	if (!section) throw NoSectionSelectedException();
	String ret;
	try {
		ret=((SECTION*)section)->values.get(key);
	} catch (...) {
		return defaultvalue;
	}
	return IsTrue(ret);
}

int ConfigParser::getInt(const String &section, const String &key, int defaultvalue)
{
	SECTION *s=(SECTION *)findSection(section);
	if (!s) return defaultvalue;
	String ret;
	try {
		ret=s->values.get(key);
	} catch (...) {
		return defaultvalue;
	}
	return ret.toInt();
}

int ConfigParser::getInt(const String &key, int defaultvalue)
{
	if (!section) throw NoSectionSelectedException();
	String ret;
	try {
		ret=((SECTION*)section)->values.get(key);
	} catch (...) {
		return defaultvalue;
	}
	return ret.toInt();
}


const String& ConfigParser::getSection(const String &name) const
/*!\brief Inhalt einer Sektion als char* auslesen
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird der komplette Inhalt einer Sektion als char*
 * zurückgegeben
 *
 * \param section Der Name der Sektion ohne Eckige Klammern
 * \returns Im Erfolgsfall liefert die Funktion einen char-Pointer auf den Inhalt
 * der Sektion zurück. Im Fehlerfall wird NULL zurückgegeben und der Fehlercode
 * kann wie üblich mit den \link Errors Fehlerfunktionen \endlink abgefragt werden.
 */
{
	//sections.List("configSections");
	return sections.getString(name);
}

void ConfigParser::copySection(AssocArray &target, const String &section) const
/*!\brief Inhalt einer Sektion in einem Array speichern
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird der Inhalt einer Sektion ein ein Assoziatives Array kopiert.
 *
 * \param target Assoziatives Array, in dem die Sektion gespeichert werden soll
 * \param section Der Name der Sektion ohne Eckige Klammern
 */
{
	SECTION *s=(SECTION *)findSection(section);
	if (!s) throw UnknownSectionException(section);
	target.add(s->values);
}

void ConfigParser::print() const
/*!\brief Konfiguration auf STDOUT ausgeben
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird die geladene Konfiguration auf STDOUT ausgegeben.
 *
 */
{
	SECTION *s=(SECTION *)first;
	while (s) {
		printf ("[%s]\n",s->name.getPtr());
		s->values.list();
		printf ("\n");
		s=s->next;
	}
}

void ConfigParser::load(const String &filename)
/*!\brief Konfiguration aus einer Datei laden
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird eine Konfiguration aus einer Datei geladen.
 *
 * \param filename Dateiname
 */
{
	File ff;
	ff.open(filename,File::READ);
	load(ff);
}

void ConfigParser::loadFromMemory(const void *buffer, size_t bytes)
/*!\brief Konfiguration aus dem Speicher laden
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird eine bereits im Speicher befindliche Konfiguration in das
 * ConfigParser-Objekt geladen.
 *
 * \param buffer Ein Pointer auf den Beginn des Speicherbereichs
 * \param bytes Die Größe des Speicherbereichs
 *
 * \since Diese Funktion wurde mit Version 6.0.19 eingeführt
 */
{
	if (!buffer) throw IllegalArgumentException("buffer");
	if (!bytes) throw IllegalArgumentException("bytes");
	MemFile ff;
	ff.open((void*)buffer,bytes);
	ff.load();
}

void ConfigParser::loadFromString(const String &string)
/*!\brief Konfiguration aus einem String laden
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird eine in einem String enthaltene Konfiguration in das
 * ConfigParser-Objekt geladen.
 *
 * \param string Ein Pointer auf die String-Klasse
 *
 * \since Diese Funktion wurde mit Version 6.0.19 eingeführt
 */
{

	MemFile ff;
	ff.open((void*)string.getPtr(),string.size());
	ff.load();
}

void ConfigParser::load(FileObject &file)
/*!\brief Konfiguration aus einem CFileObject-Objekt laden
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Funktion wird eine Konfiguration aus einem CFileObject-Objekt geladen.
 * Die Funktion wird intern von den anderen Load-Funktionen verwendet.
 *
 * \param file Ein Pointer auf eine CFileObject-Klasse
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall false (0). Der genaue
 * Fehlercode kann wie üblich mit den \link Errors Fehlerfunktionen \endlink abgefragt werden.
 *
 * \see loadFromString(const String &string)
 * \see load(const String &filename);
 * \see loadFromMemory(const void *buffer, size_t bytes)
 */
{
	unload();
	char *cursect=NULL, *line;
	String key,value;
	String Line;
	String buffer;
	String sectionname;

	size_t l;
	int trenn;

	while (!file.eof()) {			// Zeilen lesen, bis keine mehr kommt
		if (!file.gets(buffer,65536)) break;
		Line=buffer;
		Line.rTrim();
		line=trim(buffer);		// überflüssige Zeichen wegschneiden
		l=strlen(line);
		if (cursect) {
			if (l==0 || (l>0 && line[0]!='[' && line[l-1]!=']')) {
				sections.Concat(cursect,buffer);
			}
		}
		if (l>0) {
			if (line[0]=='[' && line[l-1]==']') {		// Neue [Sektion] erkannt
				cursect=NULL;
				if (l<1024) {							// nur gültig, wenn < 1024 Zeichen
					strncpy(sectionname,line+1,l-2);	// Ohne die Klammern in den Buffer
					sectionname[l-2]=0;					// kopieren und 0-Byte anhängen
					cursect=trim(sectionname);			// eventuelle Spaces wegschneiden
					sections.Concat(cursect,"","");
				}
			} else if ((cursect) && line[0]!='#' && line[0]!=';') {		// Kommentare ignorieren
				trenn=instr(line,separator,0);			// Trennzeichen suchen
				if (trenn>0) {							// Wenn eins gefunden wurde, dann
					line[trenn]=0;						// durch 0-Byte ersetzen
					key=line;							// Key ist alles vor dem Trennzeichen
					key.Trim();
					value=(line+trenn+separatorLength);	// Value der rest danach
					value.Trim();
					//value.HexDump();
					Add(cursect,(const char*)key,(const char*)value);				// Und das ganze dann hinzuf�gen ins Array
				}
			}
		}
	}

	free (sectionname);
	free (buffer);
	return 1;
}

int ConfigParser::Save(const char *filename)
{
	if (!filename) { SetError(6); return 0; }
	CFile ff;
	if (ff.Open(filename,"w")) {
		return Save(&ff);
	}
	return 0;
}

int ConfigParser::Save(CFileObject *file)
{
	char *key, *value;
	int res;
	SECTION *s=(SECTION *)first;
	while (s) {
		file->Putsf("\n[%s]\n",s->name);
		res=s->values->GetFirst(&key,&value);
		while (res) {
			//printf ("[%s]%s=%s\n",s->name,key,value);
			file->Putsf("%s%s%s\n",key,separator,value);
			res=s->values->GetNext(&key, &value);
		}
		s=s->next;
	}
	return 1;
}


} // end of namespace ppl7

