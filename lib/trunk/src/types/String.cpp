/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.18 $
 * $Date: 2011/03/30 05:56:13 $
 * $Id: String.cpp,v 1.18 2011/03/30 05:56:13 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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
#define _ISOC99_SOURCE
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif

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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "ppl7.h"
#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#ifdef HAVE_PCRE
//#define PCRE_STATIC
#include <pcre.h>
#endif

namespace ppl7 {

static size_t InitialBuffersize=128;
static char GlobalEncoding[64]="UTF-8";


/*!\brief Zeichenkodierung festlegen
 *
 * Standardmäßig erwartet die String-Klasse bei Übergabe von "const char *", dass
 * die darin enthaltenen Strings \b UTF-8 kodiert sind. Dieses Verhalten kann man
 * mit dieser Funktion ändern.
 *
 * \param encoding
 *
 * \attention
 * Die Funktion ist nicht Thread-sicher und sollte daher nur einmal am Anfang des
 * Programms aufgerufen werden.
 */
void String::setGlobalEncoding(const char *encoding) throw(NullPointerException,UnsupportedCharacterEncodingException)
{
	char enc[64]="";
	if (!encoding) throw NullPointerException();
	size_t size=strlen(encoding);
	if (size>63 || size==0) throw UnsupportedCharacterEncodingException();
	for (size_t i=0;i<size;i++) enc[i]=toupper(encoding[i]);
	enc[size]=0;

	// Sofern wir die Multibyte-Funktionen haben, unterstützen wir
	// UTF-8 und US-ASCII auch ohne iconv
#ifdef HAVE_MBSTOWCS
	if (::strcmp(enc,"utf8")==0
			|| ::strcmp(enc,"utf-8")==0
			|| ::strcmp(enc,"us-ascii")==0
			|| ::strcmp(enc,"usascii")==0
			|| ::strcmp(enc,"ascii")==0
			) {
		sprintf(GlobalEncoding,"UTF-8");
		return;
	}
#endif
	// Für alles andere brauchen wir ICONV
#ifndef HAVE_ICONV
	throw UnsupportedCharacterEncodingException();
#else
	// Wir prüfen, ob das angegebene Encoding von Iconv unterstützt wird
	iconv_t iconvimport=iconv_open(ICONV_UNICODE,enc);
	if ((iconv_t)(-1)==iconvimport) {
		throw UnsupportedCharacterEncodingException();
	}
	iconv_close(iconvimport);
	strcpy(GlobalEncoding,enc);
#endif


}


/*!\class String
 * \ingroup PPLGroupDataTypes
 * \ingroup PPLGroupStrings
 * \brief String-Klasse
 *
 * Diese Klasse kann verwendet werden, um beliebige Strings zu speichern und zu verarbeiten. Dabei
 * braucht sich der Anwender keine Gedanken um den verwendeten Speicher zu machen.
 * Die einzelnen Zeichen des Strings werden intern im Unicode-Format gespeichert. Bei Übernahme eines
 * C-Strings wird erwartet, dass dieser im UTF-8 Format vorliegt, mit der statischen Funktion
 * String::setGlobalEncoding kann jedoch auch eine andere Kodierung vorgegeben werden.
 *
 */


/*!\brief Konstruktor
 *
 * @return
 */
String::String() throw()
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
}

String::String(const char *str) throw(OutOfMemoryException, UnsupportedFeatureException, UnsupportedCharacterEncodingException, CharacterEncodingException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str);
}

String::String(const wchar_t *str) throw(OutOfMemoryException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str);
}

String::String(const wchar_t *str, size_t size) throw(OutOfMemoryException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str,size);
}

String::String(const String *str) throw(OutOfMemoryException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str);
}

String::String(const String &str) throw(OutOfMemoryException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str);
}

String::String(const std::string &str) throw(OutOfMemoryException, UnsupportedFeatureException, UnsupportedCharacterEncodingException, CharacterEncodingException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str.data(),str.size());
}

String::String(const std::wstring &str) throw(OutOfMemoryException, UnsupportedFeatureException, UnsupportedCharacterEncodingException, CharacterEncodingException)
{
	type=STRING;
	ptr=NULL;
	stringlen=0;
	s=0;
	set(str.data(),str.size());
}

/*!\brief Destruktor
 *
 * \desc
 * Der Destructor gibt den durch den String belegten Speicher wieder frei.
 *
 */
String::~String() throw()
{
	free(ptr);
}

/*!\brief String leeren
 *
 * \desc
 * Mit dieser Funktion wird der String geleert und der bisher allokierte Speicher wieder
 * freigegeben.
 */
void String::clear() throw()
{
	free(ptr);
	ptr=NULL;
	stringlen=0;
	s=0;
}

/*!\brief Anzahl Zeichen, die in den bereits allokierten Speicher passen
 *
 * \desc
 * Diese Funktion liefert die Anzahl Zeichen zurück, die in den derzeitig allokierten
 * Puffer passen, ohne dass neuer Speicher allokiert werden muss.
 *
 * @return Anzahl Zeichen
 */
size_t String::capacity ( ) const
{
	if (!s) return 0;
	return (s/sizeof(wchar_t))-1;
}

/*!\brief Reserviert Speicher für den String
 *
 * \desc
 * Mit dieser Funktion kann vor Verwendung des Strings vorgegeben werden, wieviel
 * Speicher initial reserviert werden soll. Dies ist insbesondere dann sinnvoll,
 * wenn der String während seiner Lebenszeit häufig verlängert wird.
 *
 * @param[in] size Anzahl Zeichen, für die Speicher reserviert werden soll.
 *
 * \note
 * Enthält der String bereits Zeichen, gehen diese nicht verloren, der existierende
 * Speicherbereich kann aber zwecks Vergrößerung umkopiert werden. Der Aufruf
 * der Funktion String::clear führt dazu, dass der Speicher wieder freigegeben wird.
 *
 */
void String::reserve(size_t size)
{
	size_t bytes=(size+1)*sizeof(wchar_t);
	if (s>=bytes) return; // Nothing to do
	wchar_t *p=(wchar_t*)realloc(ptr,bytes);
	if (!p) throw OutOfMemoryException();
	ptr=p;
	s=bytes;
}

/*!\brief Länge des Strings
 *
 * \desc
 * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
 *
 * \note
 * Die Funktionen String::len, String::length und String::size sind identisch.
 * \see
 * String::capacity
 *
 * @return Anzahl Zeichen
 */
size_t String::len() const
{
	return stringlen;
}

/*!\brief Länge des Strings
 *
 * \desc
 * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
 *
 * \note
 * Die Funktionen String::len, String::length und String::size sind identisch.
 * \see
 * String::capacity
 *
 * @return Anzahl Zeichen
 */
size_t String::length() const
{
	return stringlen;
}

/*!\brief Länge des Strings
 *
 * \desc
 * Diese Funktion gibt die Anzahl Zeichen zurück, aus denen der String besteht.
 *
 * \note
 * Die Funktionen String::len, String::length und String::size sind identisch.
 * \see
 * String::capacity
 *
 * @return Anzahl Zeichen
 */
size_t String::size() const
{
	return stringlen;
}


/*! \brief Prüft, ob der String leer ist.
 *
 * \desc
 * Diese Funktion prüft, ob der String leer ist.
 *
 * \returns Ist der String leer, liefert die Funktion \c true zurück, sonst \c false.
 * \see String::notEmpty
 */
bool String::isEmpty() const
{
	if (stringlen==0) return true;
	return false;
}

/*! \brief Prüft, ob der String Zeichen enthält
 *
 * \desc
 * Diese Funktion prüft, ob der String Zeichen enthält.
 *
 * \returns Enthält der String Zeichen, liefert die Funktion \c true zurück, sonst \c false.
 * \see String::isEmpty
 */
bool String::notEmpty() const
{
	if (stringlen==0) return false;
	return true;
}

/*!\brief Prüft, ob der String nummerisch ist
 *
 * \desc
 * Diese Funktion prüft, ob im String nur nummerische Zeichen vorhanden sind, also die Ziffern
 * 0-9, Punkt, Komma und Minus.
 *
 * @return Ist der String nummerisch, wird 1 zurückgegeben. Ist er es nicht oder ist der String
 * leer, wird 0 zurückgegeben.
 */
bool String::isNumeric() const
{
	wchar_t c;
	if (!stringlen) return false;
	for (size_t i=0;i<stringlen;i++) {
		c=((wchar_t*)ptr)[i];
		if (c<'0' || c>'9') {
			if (c!='.' && c!=',' && c!='-') return 0;
		}
	}
	return 1;
}

/*!\brief Prüft, ob der String einen Integer Wert enthält
 *
 * \desc
 * Diese Funktion prüft, ob im String einen integer Wert enthält, also nur die Ziffern
 * 0-9 und optional ein Minus am Anfang enthalten sind
 *
 * @return Ist der String ein Integer, wird 1 zurückgegeben. Ist er es nicht oder ist der String
 * leer, wird 0 zurückgegeben.
 */
bool String::isInteger() const
{
	wchar_t c;
	if (!stringlen) return false;
	for (size_t i=0;i<stringlen;i++) {
		c=((wchar_t*)ptr)[i];
		if (c<'0' || c>'9') {
			if (c=='-' && i==0) continue;		// Minus am Anfang ist erlaubt
			return 0;
		}
	}
	return 1;
}

/*!\brief Prüft, ob der String "wahr" ist
 *
 * Diese Funktion überprüft den aktuellen String, ob er "wahr" ist. Dies ist der Fall,
 * wenn eine der folgenden Bedingungen erfüllt ist:
 * - Der String enthält eine Ziffer ungleich 0
 * - Der String enthält das Wort "true" (Gross- oder Kleingeschrieben)
 * - Der String enthält das Wort "wahr" (Gross- oder Kleingeschrieben)
 * - Der String enthält das Wort "yes" (Gross- oder Kleingeschrieben)
 * - Der String enthält das Wort "ja" (Gross- oder Kleingeschrieben)
 *
 * \returns Liefert true (1) zurück, wenn der String "wahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
 * \see CWString::IsFalse()
 */
bool String::isTrue() const
{
	if (!stringlen) return false;
	if (wcstol(ptr,NULL,0)!=0) return true;
	if (strcasecmp("true")==0) return true;
	if (strcasecmp("wahr")==0) return true;
	if (strcasecmp("ja")==0) return true;
	if (strcasecmp("yes")==0) return true;
	return false;
}

/*!\brief Prüft, ob der String "unwahr" ist
 *
 * Diese Funktion überprüft den aktuellen String, ob er "unwahr" ist. Dies ist der Fall,
 * wenn eine der folgenden Bedingungen erfüllt ist:
 * - Der String zeigt auf NULL
 * - Die Länge des Strings ist 0
 * - Der String enthält die Ziffer 0
 * - Der String enthält nicht das Wort "true", "wahr", "yes" oder "ja" (Gross-/Kleinschreibung egal)
 *
 * \returns Liefert true (1) zurück, wenn der String "unwahr" ist, sonst false (0). Ein Fehlercode wird nicht gesetzt
 * \see CWString::IsTrue()
 */
bool String::isFalse() const
{
	if (isTrue()) return false;
	return true;
}



/*!\brief String anhand eines C-Strings setzen
 *
 * \desc
 * Mit dieser Funktion wird der String anhand eines char * gesetzt. Dabei wird er
 * intern nach Unicode konvertiert.
 *
 * \param str Pointer auf einen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 * \exception UnsupportedFeatureException
 * \exception UnsupportedCharacterEncodingException
 * \exception CharacterEncodingException
 *
 */
String & String::set(const char *str, size_t size) throw(OutOfMemoryException, UnsupportedFeatureException, UnsupportedCharacterEncodingException, CharacterEncodingException)
{
	if (!str) {
		clear();
		return *this;
	}
	size_t inbytes;
	if (size!=(size_t)-1) inbytes=size;
	else inbytes=strlen(str);
	size_t outbytes=inbytes*sizeof(wchar_t)+4;
	if (outbytes>=s) {
		if (ptr) free(ptr);
		stringlen=0;
		s=InitialBuffersize;
		if (s<=outbytes) s=((outbytes/InitialBuffersize)+1)*InitialBuffersize+4;
		ptr=(wchar_t*)malloc(s);
		if (!ptr) {
			s=0;
			throw OutOfMemoryException();
		}
	}
#ifdef HAVE_MBSTOWCS
	if (::strcasecmp(GlobalEncoding,"UTF-8")==0
			|| ::strcasecmp(GlobalEncoding,"UTF8")==0
			|| ::strcasecmp(GlobalEncoding,"USASCII")==0
			|| ::strcasecmp(GlobalEncoding,"US-ASCII")==0
			) {
		size_t ret=mbstowcs((wchar_t*)ptr, str, inbytes);
		if (ret==(size_t) -1) {
			((wchar_t*)ptr)[0]=0;
			stringlen=0;
			throw CharacterEncodingException();
		}
		((wchar_t*)ptr)[ret]=0;
		stringlen=ret;
		return *this;
	}
#endif
#ifdef HAVE_ICONV
	iconv_t iconvimport=iconv_open(ICONV_UNICODE,GlobalEncoding);
	if ((iconv_t)(-1)==iconvimport) {
		throw UnsupportedCharacterEncodingException();
	}
	char *outbuf=(char*)ptr;
	HexDump(str,inbytes);
	size_t res=iconv(iconvimport, (ICONV_CONST char **)&str, &inbytes,
				(char**)&outbuf, &outbytes);
	iconv_close(iconvimport);
	if (res==(size_t)(-1)) {
		((wchar_t*)ptr)[0]=0;
		stringlen=0;
		//SetError(289,"%s",strerror(errno));
		throw CharacterEncodingException();
	}
	((wchar_t*)outbuf)[0]=0;
	stringlen=wcslen((wchar_t*)ptr);
	return *this;
#else
	throw UnsupportedFeatureException();
#endif
}

/*!\brief String anhand eines wchar_t* setzen
 *
 * \desc
 * Mit dieser Funktion wird der String anhand eines wchar_t * gesetzt.
 *
 * \param str Pointer auf einen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
String & String::set(const wchar_t *str, size_t size) throw(OutOfMemoryException)
{
	if (!str) {
		clear();
		return *this;
	}
	size_t inbytes;
	if (size!=(size_t)-1) inbytes=size;
	else inbytes=wcslen(str);
	size_t outbytes=inbytes*sizeof(wchar_t)+4;
	if (outbytes>=s) {
		if (ptr) free(ptr);
		stringlen=0;
		s=InitialBuffersize;
		if (s<=outbytes) s=((outbytes/InitialBuffersize)+1)*InitialBuffersize+4;
		ptr=(wchar_t*)malloc(s);
		if (!ptr) {
			s=0;
			throw OutOfMemoryException();
		}
	}
	wcsncpy((wchar_t*)ptr,str,inbytes);
	stringlen=inbytes;
	((wchar_t*)ptr)[stringlen]=0;
	return *this;
}

/*!\brief String anhand eines String-Pointers setzen
 *
 * \desc
 * Mit dieser Funktion wird der String anhand des Pointers \p str eines anderen
 * Strings gesetzt.
 *
 * \param str Pointer auf einen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
String & String::set(const String *str, size_t size) throw(OutOfMemoryException)
{
	if (!str) {
		clear();
		return *this;
	}
	size_t inbytes;
	if (size!=(size_t)-1) inbytes=size;
	else inbytes=str->stringlen;
	if (inbytes>str->stringlen) inbytes=str->stringlen;
	return set((wchar_t*)str->ptr,inbytes);
}

/*!\brief Wert eines anderen Strings übernehmen
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings \p str übernommen.
 *
 * \param str Referenz auf einen anderen String
 * \param size Optionaler Parameter, der die Anzahl zu importierender Zeichen angibt.
 * Ist der Wert nicht angegeben, wird der komplette String übernommen. Ist der Wert größer als
 * der angegebene String, wird er ignoriert und der komplette String importiert.
 * \return Referenz auf den String
 * \exception OutOfMemoryException
 */
String & String::set(const String &str, size_t size) throw(OutOfMemoryException)
{
	size_t inbytes;
	if (size!=(size_t)-1) inbytes=size;
	else inbytes=str.stringlen;
	if (inbytes>str.stringlen) inbytes=str.stringlen;
	return set((wchar_t*)str.ptr,inbytes);
}

/*! \brief Erzeugt einen formatierten String
 *
 * \desc
 * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
 * und den optionalen Parametern \p ...
 *
 * \param fmt Der Formatstring
 * \param ... Optionale Parameter
 *
 * @return Referenz auf den String
 *
 * \par Example:
 * \code
int main(int argc, char **argv)
{
	String s;
	s.setf ("Anzahl Parameter: %i\n",argc);
	s.printnl();
	return 0;
}
\endcode
 *
 * \copydoc sprintf.dox
 */
String & String::setf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *buff=NULL;
	if (::vasprintf (&buff, (char*)fmt, args)>0 && buff!=NULL) {
		try {
			set(buff);
			free(buff);
		} catch(...) {
			free(buff);
			va_end(args);
			throw;
		}
		return *this;
	}
	va_end(args);
    free(buff);
    throw Exception();
}

/*!\brief Einzelnes Unicode-Zeichen übernehmen
 *
 * \desc
 * Ein einzelnes Unicode-Zeichen \p c wird in den String übernommen.
 *
 * @param c Unicode-Wert des gewünschten Zeichens
 *
 * @return Referenz auf den String
 */
String & String::set(wchar_t c) throw(OutOfMemoryException)
{
	wchar_t buffer[2];
	buffer[0]=c;
	buffer[1]=0;
	return set(buffer,1);
}

/*! \brief Erzeugt einen formatierten String
 *
 * Erzeugt einen String anhand des übergebenen Formatstrings \p fmt
 * und den optionalen Parametern in \p args.
 *
 * \param fmt Der Formatstring
 * \param args Pointer auf Liste der Parameter. Muss zuvor mit va_start initialisiert worden sein.
 * @return Referenz auf den String
 * \par Example:
 * \code
void MyFunction(const char *fmt, ...)
{
	String s;
	va_list args;
	va_start(args, fmt);
	s.vasprintf(fmt,args);
	va_end(args);
	printf ("Ergebnis: %ls",(const wchar_t*)s);
}
\endcode
 *
 * \copydoc sprintf.dox
 */
String & String::vasprintf(const char *fmt, va_list args) throw(OutOfMemoryException, UnsupportedFeatureException)
{
	char *buff=NULL;
	if (::vasprintf (&buff, (char*)fmt, args)>0 && buff!=NULL) {
		try {
			set(buff);
			free(buff);
		} catch(...) {
			free(buff);
			throw;
		}
		return *this;
	}
    free(buff);
    throw Exception();
}

/*!\brief Fügt einen Wide-Character String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen Wide-Character String an das Ende des bestehenden an
 *
 * \param[in] str Pointer auf einen Wide-Character String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::append(const wchar_t *str, size_t size) throw(OutOfMemoryException)
{
	if (!str) return *this;
	if (!ptr) {
		set(str,size);
		return *this;
	}
	size_t inchars;
	if (size!=(size_t)-1) inchars=size;
	else inchars=wcslen(str);
	size_t outbytes=(inchars+stringlen)*sizeof(wchar_t)+4;
	if (outbytes>=s) {
		size_t newbuffersize=((outbytes/InitialBuffersize)+1)*InitialBuffersize+16;
		wchar_t *t=(wchar_t*)realloc(ptr,newbuffersize);
		if (!t) throw OutOfMemoryException();
		ptr=t;
		s=newbuffersize;
	}
	wmemcpy(((wchar_t*)ptr)+stringlen,str,inchars);
	stringlen+=inchars;
	((wchar_t*)ptr)[stringlen]=0;
	return *this;
}

/*!\brief Fügt einen C-String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen C-String an das Ende des bestehenden an. Der String muss entweder
 * UTF-8 kodiert sein, oder es muss mit der statischen Funktion String::setGlobalEncoding
 * zuvor eine andere Kodierung gesetzt worden sein.
 *
 * \param[in] str Pointer auf einen Wide-Character String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 * \exception UnsupportedFeatureException
 * \exception UnsupportedCharacterEncodingException
 * \exception CharacterEncodingException
 *
 */
String & String::append(const char *str, size_t size) throw(OutOfMemoryException, UnsupportedFeatureException, UnsupportedCharacterEncodingException, CharacterEncodingException)
{
	String a;
	a.set(str,size);
	return append((wchar_t*)a.ptr,a.stringlen);
}

/*!\brief Fügt einen als Pointer übergebenen String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen als Pointer übergebenen String an das Ende des bestehenden an. Ist der Pointer
 * NULL oder der Inhalt des Strings leer, wird der bisherige String beibehalten, es erfolgt
 * keine Exception.
 *
 * \param[in] str Pointer auf ein String-Objekt
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::append(const String *str, size_t size) throw(OutOfMemoryException)
{
	if (!str) return *this;
	return append((wchar_t*)str->ptr,str->stringlen);
}

/*!\brief Fügt einen String an das Ende des bestehenden an
 *
 * \desc
 * Fügt einen String an das Ende des bestehenden an.
 *
 * \param[in] str Referenz auf ein String-Objekt
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::append(const String &str, size_t size) throw(OutOfMemoryException)
{
	return append((wchar_t*)str.ptr,str.stringlen);
}


/*!\brief Fügt einen Formatierten String an das Ende des bestehenden an
 *
 * Anhand des übergebenen Formatstrings \p fmt und den optionalen Parametern \p ...
 * wird ein neuer String gebildet, der an das Ende des bestehenden angehangen wird
 *
 * \param fmt Der Formatstring
 * \param ... Optionale Parameter
  * @return Referenz auf den String
 *
 * \par Example:
 * \code
int main(int argc, char **argv)
{
	String s;
	s="Hallo Welt!";
	s.appendf (" Es wurden %i Parameter übergeben\n",argc);
	s.printnl();
	return 0;
}
\endcode
 *
 * \copydoc sprintf.dox
 */
String & String::appendf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *buff=NULL;
	if (::vasprintf (&buff, (const char*)fmt, args)>0 && buff!=NULL) {
		try {
			String a;
			a.set(buff);
			free(buff);
			append((wchar_t*)a.ptr,a.stringlen);
		} catch(...) {
			free(buff);
			va_end(args);
			throw;
		}
		return *this;
	}
	va_end(args);
    free(buff);
    throw Exception();
}

/*!\brief Einzelnes Unicode-Zeichen anhängen
 *
 * \desc
 * Ein einzelnes Unicode-Zeichen \p c wird in an den String angehangen.
 *
 * @param c Unicode-Wert des gewünschten Zeichens
 *
 * @return Referenz auf den String
 */
String & String::append(wchar_t c) throw(OutOfMemoryException)
{
	wchar_t buffer[2];
	buffer[0]=c;
	buffer[1]=0;
	return append(buffer,1);
}


/*!\brief Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen Wide-Character String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Pointer auf einen Wide-Character String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::prepend(const wchar_t *str, size_t size) throw(OutOfMemoryException)
{
	if (!str) return *this;
	if (!ptr) {
		set(str,size);
		return *this;
	}
	size_t inchars;
	if (size!=(size_t)-1) inchars=size;
	else inchars=wcslen(str);
	size_t outbytes=(inchars+stringlen)*sizeof(wchar_t)+4;
	if (outbytes>=s) {
		size_t newbuffersize=((outbytes/InitialBuffersize)+1)*InitialBuffersize+16;
		wchar_t *t=(wchar_t*)realloc(ptr,newbuffersize);
		if (!t) throw OutOfMemoryException();
		ptr=t;
		s=newbuffersize;
	}
	// Bestehenden Speicherblock nach hinten moven
	wmemmove(((wchar_t*)ptr)+inchars,ptr,stringlen);
	// Neuen Speicherblock davor kopieren
	wmemcpy(ptr,str,inchars);
	stringlen+=inchars;
	((wchar_t*)ptr)[stringlen]=0;
	return *this;
}

/*!\brief Fügt einen String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Pointer auf einen String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::prepend(const String *str, size_t size) throw(OutOfMemoryException)
{
	if (!str) return *this;
	if (!ptr) {
		set(str,size);
		return *this;
	}
	return prepend(str->ptr,str->stringlen);
}

/*!\brief Fügt einen String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Referenz auf einen String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::prepend(const String &str, size_t size) throw(OutOfMemoryException)
{
	if (!ptr) {
		set(str,size);
		return *this;
	}
	return prepend(str.ptr,str.stringlen);
}

/*!\brief Fügt einen C-String am Anfang des bestehenden Strings ein
 *
 * \desc
 * Fügt einen C-String am Anfang des bestehenden Strings ein
 *
 * \param[in] str Pointer auf einen C-String
 * \param[in] size Optional die Anzahl Zeichen (nicht Bytes) im String, die kopiert werden sollen.
 *
 * \return Referenz auf den String
 *
 * \exception OutOfMemoryException
 */
String & String::prepend(const char *str, size_t size) throw(OutOfMemoryException, UnsupportedFeatureException, UnsupportedCharacterEncodingException, CharacterEncodingException)
{
	String a;
	a.set(str,size);
	return prepend((wchar_t*)a.ptr,a.stringlen);
}

/*!\brief Fügt einen Formatierten String am Anfang bestehenden ein
 *
 * Anhand des übergebenen Formatstrings \p fmt und den optionalen Parametern \p ...
 * wird ein neuer String gebildet, der am Anfang des bestehenden eingehangen wird.
 *
 * \param fmt Der Formatstring
 * \param ... Optionale Parameter
 * \par Example:
 * \code
int main(int argc, char **argv)
{
	String s;
	s="Vielen Dank!\n";
	s.prependf ("Es wurden %i Parameter übergeben. ",argc);
	s.printnl();
	return 0;
}
\endcode
 *
 * \copydoc sprintf.dox
 */
String & String::prependf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char *buff=NULL;
	if (::vasprintf (&buff, (const char*)fmt, args)>0 && buff!=NULL) {
		try {
			String a;
			a.set(buff);
			free(buff);
			prepend((wchar_t*)a.ptr,a.stringlen);
		} catch(...) {
			free(buff);
			va_end(args);
			throw;
		}
		return *this;
	}
	va_end(args);
    free(buff);
    throw Exception();
}

/*!\brief Einzelnes Unicode-Zeichen am Anfang einfügen
 *
 * \desc
 * Ein einzelnes Unicode-Zeichen \p c wird in am Anfang des Strings eingefügt.
 * Die nachfolgenden Zeichen des Strings verschieben sich nach rechts.
 *
 * @param c Unicode-Wert des gewünschten Zeichens
 *
 * @return Referenz auf den String
 */
String & String::prepend(wchar_t c) throw(OutOfMemoryException)
{
	wchar_t buffer[2];
	buffer[0]=c;
	buffer[1]=0;
	return prepend(buffer,1);
}

/*!\brief String in UTF8 umwandeln
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings in UTF8 umgewandelt und als
 * ByteArray zurückgegeben.
 *
 * @return ByteArray mit der UTF8-Repräsentation des Strings.
 */
ByteArray String::toUtf8() const
{
	if (stringlen==0) return ByteArray();
#ifdef HAVE_WCSTOMBS
	size_t buffersize=stringlen*4+8;
	char *buffer=(char*)malloc(buffersize);
	if (!buffer) throw OutOfMemoryException();

	size_t ret=wcstombs(buffer, (const wchar_t*)ptr, buffersize);
	if (ret==(size_t)-1) {
		free (buffer);
		throw CharacterEncodingException();
	}
	ByteArray a;
	try {
		a.copy(buffer,ret);
	} catch (...) {
		free(buffer);
		throw;
	}
	free (buffer);
	return a;
#else
	return toEncoding("UTF-8");
#endif
}

/*!\brief String in eine beliebige lokale Kodierung umwandeln
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings in eine beliebige lokale
 * Kodierung umgewandelt und als ByteArray zurückgegeben.
 *
 * \param[in] encoding Das gewünschte Encoding
 *
 * @return ByteArray mit der UTF8-Repräsentation des Strings.
 *
 * \attention
 * Für diese Funktion wird "Iconv" benötigt. Ist keine Iconv-Bibliothek auf dem
 * System vorhanden, wird eine UnsupportedFeatureException geworfen.
 */
ByteArray String::toEncoding(const char *encoding) const
{
#ifndef HAVE_ICONV
	throw UnsupportedFeatureException();
#else
	iconv_t iconv_handle=iconv_open(encoding,ICONV_UNICODE);
	if ((iconv_t)(-1)==iconv_handle) {
		throw UnsupportedCharacterEncodingException();
	}

	size_t buffersize=(stringlen+4)*sizeof(wchar_t);
	char *buffer=(char*)malloc(buffersize);
	if (!buffer) {
		iconv_close(iconv_handle);
		throw OutOfMemoryException();
	}
	size_t outbytes=buffersize;
	char *b=buffer;
	char *inbuffer=(char*)ptr;
	size_t inbytes=stringlen*sizeof(wchar_t);

	hexDump();

	size_t res=iconv((iconv_t)iconv_handle, (ICONV_CONST char **)&inbuffer, &inbytes,
			(char**)&b, &outbytes);
	iconv_close(iconv_handle);
	if (res==(size_t)(-1)) {
		free(buffer);
		throw CharacterEncodingException();
	}
	b[0]=0;
	ByteArray ret(buffer,buffersize-outbytes);
	free(buffer);
	return ret;
#endif
}

String String::getUtf8MD5() const
{
	if (stringlen==0 || ptr==NULL) throw EmptyDataException();
	return toUtf8().md5();
}

String String::getMD5() const
{
	if (stringlen==0 || ptr==NULL) throw EmptyDataException();
	ByteArrayPtr ba(ptr,s);
	return ba.md5();
}


/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Unicode-Wert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden, wobei 0 dem ersten Zeichen entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Unicode-Wert des Zeichens
 * \exception OutOfBoundsEception Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
wchar_t String::get(size_t pos) const
{
	if (stringlen>pos) return ((wchar_t*)ptr)[pos];
	throw OutOfBoundsEception();
}

/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit dieser Funktion kann der Unicode-Wert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
 * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
 * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
 * dem letzten Zeichen des Strings entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Unicode-Wert des Zeichens
 * \exception OutOfBoundsEception Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
wchar_t String::get(ssize_t pos) const
{
	if (pos>0 && stringlen>(size_t)pos) return ((wchar_t*)ptr)[pos];
	if (pos<0 && (size_t)(0-pos)<stringlen) return ((wchar_t*)ptr)[stringlen+pos];
	throw OutOfBoundsEception();
}


/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit diesem Operator kann der Unicode-Wert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden, wobei 0 dem ersten Zeichen entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Unicode-Wert des Zeichens
 * \exception OutOfBoundsEception Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
wchar_t String::operator[](size_t pos) const
{
	if (stringlen>pos) return ((wchar_t*)ptr)[pos];
	throw OutOfBoundsEception();
}


/*!\brief Einzelnes Zeichen auslesen
 *
 * \desc
 * Mit diesem Operator kann der Unicode-Wert eines einzelnen Zeichens an der Position
 * \p pos ausgelesen werden. Enthält \p pos einen positiven Wert, wird die Position des
 * Zeichens vom Anfang des Strings ermittelt, wobei 0 dem ersten Zeichen entspricht.
 * Ist der Wert negativ, wird das Zeichen vom Ende des Strings ermittelt, wobei -1
 * dem letzten Zeichen des Strings entspricht.
 *
 * @param pos Position des Zeichens innerhalb des Strings
 * @return Unicode-Wert des Zeichens
 * \exception OutOfBoundsEception Wird geworfen, wenn die angegebene Position \p pos
 * ausserhalb des Strings liegt oder der String leer ist.
 */
wchar_t String::operator[](ssize_t pos) const
{
	if (pos>0 && stringlen>(size_t)pos) return ((wchar_t*)ptr)[pos];
	if (pos<0 && (size_t)(0-pos)<stringlen) return ((wchar_t*)ptr)[stringlen+pos];
	throw OutOfBoundsEception();
}


/*!\brief String auf STDOUT ausgeben
 *
 * Diese Funktion gibt den aktuellen String auf STDOUT aus. Dazu ist es notwendig den String vom internen
 * Unicode-Format in das Encoding des Betriebssystems umzurechnen. Da dieses von den lokalen Einstellungen
 * des Betriebssystems und des Users abhängig ist, wird die Environment-Variable "LANG" ausgewertet.
 * Ist diese nicht gesetzt oder enthält ein unbekanntes Encoding, wird der String immer in UTF-8 ausgegeben.
 *
 * \param withNewline Ein optionaler Parameter, der angibt, ob nach der Ausgabe ein Zeilenumbruch
 * angehangen werden soll (default=false)
 * \par Exceptions:
 * Keine
 *
 */
void String::print(bool withNewline) const throw()
{
	if (ptr!=NULL && stringlen>0) {
		if (withNewline) printf("%ls\n",(wchar_t*)ptr);
		else printf("%ls",(wchar_t*)ptr);
	} else if (withNewline) {
		printf("\n");
	}
}

/*!\brief String auf STDOUT mit abschließendem Zeilenumbruch ausgeben
 *
 * Diese Funktion gibt den aktuellen String mit abschließendem Zeilenumbruch auf STDOUT aus.
 * Dazu ist es notwendig den String vom internen
 * Unicode-Format in das Encoding des Betriebssystems umzurechnen. Da dieses von den lokalen Einstellungen
 * des Betriebssystems und des Users abhängig ist, wird die Environment-Variable "LANG" ausgewertet.
 * Ist diese nicht gesetzt oder enthält ein unbekanntes Encoding, wird der String immer in UTF-8 ausgegeben.
 *
 * \par Exceptions:
 * Keine
 *
 */
void String::printnl() const throw()
{
	print(true);
}

void String::hexDump() const
{
	if (stringlen==0) throw EmptyDataException();
	printf ("HEXDUMP of String: %zi Bytes starting at Address %p:\n",stringlen*sizeof(wchar_t),ptr);
	HexDump(ptr,stringlen*sizeof(wchar_t),true);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const char* str)
{
	return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const wchar_t* str)
{
	return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const String *str)
{
	return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const String &str)
{
	return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const std::string &str)
{
	return set(str);
}

/*!\brief String übernehmen
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str kopiert. Der Operator
 * ist identisch mit der Funktion String::set
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(const std::wstring &str)
{
	return set(str);
}

/*!\brief Zeichen übernehmen
 *
 * \desc
 * Mit diesem Operator wird ein einzelnes Zeichen in den String kopiert.
 *
 * @param[in] c Unicode Wert des zu übernehmenden Zeichens
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator=(wchar_t c)
{
	return set(c);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const char* str)
{
	return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const wchar_t* str)
{
	return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const String& str)
{
	return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const std::string &str)
{
	return append(str);
}

/*!\brief String addieren
 *
 * \desc
 * Mit diesem Operator wird der Angegebene String \p str an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] str Zu kopierender String
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(const std::wstring &str)
{
	return append(str);
}

/*!\brief Zeichen anhängen
 *
 * \desc
 * Mit diesem Operator wird das angegebene Zeichen \p c an den bisher vorhandenen
 * String angehangen. Der Operator ist identisch mit der Funktion String::append.
 *
 * @param[in] c Unicode-Wert des anzuhängenden Zeichens
 * @return Referenz auf diese Instanz der Klasse
 */
String& String::operator+=(wchar_t c)
{
	return append(c);
}

/*!\brief Führt einen Vergleich mit einem anderen String durch
 *
 * \desc
 * Führt einen Vergleich mit einem anderen String durch.
 *
 * \return Ist der String kleiner als der mit \a str angegebene, wird ein negativer Wert
 * zurückgegeben, ist er identisch, wird 0 zurückgegeben, ist er größer, erfolgt ein positiver
 * Return-Wert.
 */
int String::strcmp(const String &str, size_t size) const
{
	const wchar_t *mystr=ptr;
	const wchar_t *otherstr=str.ptr;
	if (!mystr) mystr=L"";
	if (!otherstr) otherstr=L"";
	if (size!=(size_t)-1) return wcsncmp(mystr,otherstr,size);
	return wcscmp(mystr,otherstr);
}

/*!\brief Stringvergleich mit Ignorierung von Gross-/Kleinschreibung
 *
 * \desc
 * Führt einen Vergleich mit einem anderen String durch, unter Ignorierung der
 * Gross-/Kleinschreibung.
 *
 * \return Ist der String kleiner als der mit \a str angegebene, wird ein negativer Wert
 * zurückgegeben, ist er identisch, wird 0 zurückgegeben, ist er größer, erfolgt ein positiver
 * Return-Wert.
 */
int String::strcasecmp(const String &str, size_t size) const
{
	const wchar_t *mystr=ptr;
	const wchar_t *otherstr=str.ptr;
	if (!mystr) mystr=L"";
	if (!otherstr) otherstr=L"";
#ifdef HAVE_WCSCASECMP
	if (size) return wcsncasecmp(mystr,otherstr,size);
	return wcscasecmp(mystr,otherstr);
#elif WIN32
	if (size) return wcsnicmp(mystr,otherstr,size);
	return wcsicmp(mystr,otherstr);
#else
	String b=mystr;
	String s=otherstr;
	s.LCase();
	b.LCase();
	if (size) return wcsncmp(b.ptr,s.ptr,size);
	return wcscmp(b.ptr,s.ptr);
#endif
}


String String::left(size_t len) const
{
	String ret;
	if(ptr != NULL && stringlen > 0) {
		if(len > stringlen) len = stringlen;
		ret.set(ptr,len);
	}
	return ret;
}

String String::right(size_t len) const
{
	String ret;
	if(ptr != NULL && stringlen > 0) {
		if(len > stringlen) len = stringlen;
		ret.set(ptr+stringlen-len,len);
	}
	return ret;
}

String String::mid(size_t start, size_t len) const
{
	String ret;
	if (len==(size_t)-1) len=stringlen;
	if (start<stringlen && ptr!=NULL && len>0) {
		if (start+len>stringlen) len=stringlen-start;
		ret.set(ptr+start,len);
	}
	return ret;
}

String String::substr(size_t start, size_t len) const
{
	String ret;
	if (len==(size_t)-1) len=stringlen;
	if (start<stringlen && ptr!=NULL && len>0) {
		if (start+len>stringlen) len=stringlen-start;
		ret.set(ptr+start,len);
	}
	return ret;
}

/*! \brief Wandelt alle Zeichen des Strings in Kleinbuchstaben um
 *
 * Diese Funktion wandelt alle Zeichen des Strings in Kleinbuchstaben um. Die genaue Funktionsweise hängt davon ab,
 * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
 *
 * \attention Unter UNIX (und möglicherweise anderen Betriebssystemen) werden die Lokalisationseinstellungen der
 * Umgebung nicht automatisch übernommen, sondern stehen standardmäßig auf "C". Dadurch werden nur US-ASCII
 * (ASCII 32 bis 127) umgewandelt. Man sollte daher nach Programmstart mit "setlocale" die gewünschte
 * Spracheinstellung vornehmen.
 *
 * \example
 * \code
 * #include <locale.h>
 * ...
 * setlocale(LC_CTYPE,"de_DE.UTF-8");
 * \endcode
 */
void String::lowerCase()
{
	if (ptr!=NULL && stringlen>0) {
		wchar_t c,wc;
		for (size_t i=0;i<stringlen;i++) {
			wc=ptr[i];
			c=towlower(wc);
			if (c!=(wchar_t)WEOF) {
				ptr[i]=c;
			}
		}
	}
}

/*! \brief Wandelt alle Zeichen des Strings in Grossbuchstaben um
 *
 * Diese Funktion wandelt alle Zeichen des Strings in Großbuchstaben um. Die genaue Funktionsweise hängt davon ab,
 * welche Spracheinstellungen aktiv sind, genauer vom Wert "LC_CTYPE".
 *
 * \attention Unter UNIX (und möglicherweise anderen Betriebssystemen) werden die Lokalisationseinstellungen der
 * Umgebung nicht automatisch übernommen, sondern stehen standardmäßig auf "C". Dadurch werden nur US-ASCII
 * (ASCII 32 bis 127) umgewandelt. Man sollte daher nach Programmstart mit "setlocale" die gewünschte
 * Spracheinstellung vornehmen.
 *
 * \example
 * \code
 * #include <locale.h>
 * ...
 * setlocale(LC_CTYPE,"de_DE.UTF-8");
 * \endcode
 */
void String::upperCase()
{
	if (ptr!=NULL && stringlen>0) {
		wchar_t c,wc;
		for (size_t i=0;i<stringlen;i++) {
			wc=ptr[i];
			c=towupper(wc);
			if (c!=(wchar_t)WEOF) {
				ptr[i]=c;
			}
		}
	}
}

/*!\brief Anfangsbuchstaben der Wörter groß
 *
 * \desc
 * Diese Funktion wandelt die Anfangsbuchstaben aller im String enthaltenen Wörter in
 * Großbuchstaben um.
 */
void String::upperCaseWords()
{
	if (ptr!=NULL && stringlen>0) {
		wchar_t c,wc;
		bool wordstart=true;
		for (size_t i=0;i<stringlen;i++) {
			wc=ptr[i];
			if (wordstart) {
				c=towupper(wc);
				if (c!=(wchar_t)WEOF) {
					ptr[i]=c;
				}
			}
			if (wc<48 || (wc>57 && wc<65) || (wc>90 && wc<97) || (wc>122 && wc<127) ) {
				wordstart=true;
			} else {
				wordstart=false;
			}
		}
	}
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang und Ende des Strings ab
void String::trim()
{
	if (ptr!=NULL && stringlen>0) {
		size_t i,start,ende,s;
		start=0; s=0;
		ende=stringlen;
		for (i=0;i<stringlen;i++) {
			if (ptr[i]==13 || ptr[i]==10 || ptr[i]==32 || ptr[i]=='\t') {
				if (s==0) start=i+1;
			} else {
				s=1; ende=i;
			}
		}
		ptr[ende+1]=0;
		if (start>0)
			memmove(ptr,ptr+start,(ende-start+2)*sizeof(wchar_t));
		stringlen=wcslen(ptr);
		ptr[stringlen]=0;
	}
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Anfang des Strings ab
void String::trimLeft()
{
	if (ptr!=NULL && stringlen>0) {
		size_t i,start,s;
		start=0; s=0;
		//ende=stringlen;
		for (i=0;i<stringlen;i++) {
			if (ptr[i]==13 || ptr[i]==10 || ptr[i]==32 || ptr[i]=='\t') {
				if (s==0) start=i+1;
			} else {
				s=1; // ende=i;
			}
		}
		if (start>0)
			memmove(ptr,ptr+start,(stringlen-start+2)*sizeof(wchar_t));
		stringlen=wcslen(ptr);
		ptr[stringlen]=0;
	}
}

//! \brief Schneidet Leerzeichen, Tabs Returns und Linefeeds am Ende des Strings ab
void String::trimRight()
{
	if (ptr!=NULL && stringlen>0) {
		size_t i,ende;
		ende=stringlen;
		for (i=0;i<stringlen;i++) {
			if (ptr[i]==13 || ptr[i]==10 || ptr[i]==32 || ptr[i]=='\t') {
				//if (s==0) start=i+1;
			} else {
				ende=i;
			}
		}
		ptr[ende+1]=0;
		stringlen=wcslen(ptr);
		ptr[stringlen]=0;
	}
}

//! \brief Schneidet die definierten Zeichen am Anfang des Strings ab
void String::trimLeft(const String &chars)
{
	if (ptr!=NULL && stringlen>0 && chars.stringlen>0) {
		size_t i,start,s,z;
		int match;
		start=0; s=0;
		for (i=0;i<stringlen;i++) {
			match=0;
			for (z=0;z<chars.stringlen;z++) {
				if (ptr[i]==chars.ptr[z]) {
					if (s==0) start=i+1;
					match=1;
					break;
				}
			}
			if (!match) {
				s=1;
			}
		}
		if (start>0) {
			memmove(ptr,ptr+start,(stringlen-start+2)*sizeof(wchar_t));
			stringlen=wcslen(ptr);
		}
	}
}

//! \brief Schneidet die definierten Zeichen am Ende des Strings ab
void String::trimRight(const String &chars)
{
	if (ptr!=NULL && stringlen>0 && chars.stringlen>0) {
		size_t i,ende,z;
		int match;
		ende=stringlen;
		for (i=0;i<stringlen;i++) {
			match=0;
			for (z=0;z<chars.stringlen;z++) {
				if (ptr[i]==chars.ptr[z]) {
					//if (s==0) start=i+1;
					match=1;
					break;
				}
			}
			if (!match) {
				ende=i;
			}
		}
		ptr[ende+1]=0;
		stringlen=wcslen(ptr);
	}
}

//! \brief Schneidet die definierten Zeichen am Anfang und Ende des Strings ab
void String::trim(const String &chars)
{
	trimLeft(chars);
	trimRight(chars);
}

/*!\brief Schneidet Zeichen am Ende des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 */
void String::chopRight(size_t num)
{
	if (ptr!=NULL && stringlen>0) {
		if (stringlen<num) num=stringlen;
		stringlen-=num;
		ptr[stringlen]=0;
	}
}

/*!\brief Schneidet Zeichen am Ende des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Ende des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 *
 * \see
 * Die Funktion ist identisch zu String::chopRight
 */
void String::chop(size_t num)
{
	if (ptr!=NULL && stringlen>0) {
		if (stringlen<num) num=stringlen;
		stringlen-=num;
		ptr[stringlen]=0;
	}
}

/*!\brief Schneidet Zeichen am Anfang des Strings ab
 *
 * \desc
 * Diese Funktion schneidet \p num Zeichen vom Anfang des Strings ab. Falls \p num
 * größer als der String ist, bleibt ein leerer String zurück.
 *
 * @param num Anzahl Zeichen, die abgeschnitten werden sollen
 */
void String::chopLeft(size_t num)
{
	if (ptr!=NULL && stringlen>0) {
		if (stringlen<num) num=stringlen;
		memmove(ptr,ptr+num,(stringlen-num)*sizeof(wchar_t));
		stringlen-=num;
		ptr[stringlen]=0;
	}
}

/*!\brief Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
 *
 * \desc
 * Schneidet Returns und Linefeeds am Anfanng und Ende des Strings ab
 */
void String::chomp()
{
	trim("\n\r");
}

/*!\brief Schneidet den String an einer bestimmten Stelle ab
 *
 * \desc
 * Der String wird an einer bestimmten Stelle einfach abgeschnitten
 * \param pos Die Position, an der der String abgeschnitten wird. Bei Angabe von 0 ist der String anschließend
 * komplett leer. Ist \c pos größer als die Länge des Strings, passiert nichts.
 */
void String::cut(size_t pos)
{
	if (ptr==NULL &&stringlen==0) return;
	if (pos>stringlen) return;
	ptr[pos]=0;
	stringlen=pos;
}

/*! \brief Schneidet den String beim ersten Auftauchen eines Zeichens/Strings ab
 *
 * Der String wird beim ersten Auftauchen eines Zeichens oder eines Strings abgeschnitten.
 * \param[in] letter Buchstabe oder Buchstabenkombination, an der der String abgeschnitten werden
 * soll. Zeigt der Pointer auf NULL oder ist der String leer, passiert nichts.
 */
void String::cut(const String &letter)
{
	if (ptr==NULL &&stringlen==0) return;
	if (letter.isEmpty()) return;
	ssize_t p=instr(letter,0);
	if (p>=0) {
		ptr[p]=0;
		stringlen=p;
	}
}


String String::strchr(wchar_t c) const
{
	String ret;
	if (ptr!=NULL && stringlen>0) {
		wchar_t *p=wcschr(ptr, c);
		if (p) ret.set(p);
	}
	return ret;
}

String String::strrchr(wchar_t c) const
{
	String ret;
	if (ptr!=NULL && stringlen>0) {
		wchar_t *p=wcsrchr(ptr, c);
		if (p) ret.set(p);
	}
	return ret;
}

/*!\brief Teilstring finden
 *
 * \desc
 * Diese Funktion findet die erste Position der Zeichenfolge \p needle
 * innerhalb des Strings. Abschließende `\0'-Zeichen werden nicht
 * miteinander verglichen.
 *
 * @param needle
 * @return
 * Die Funktion gibt einen String zurück, der mit der gefundenen
 * Zeichenkette beginnt und den Rest des Strings bis zum Ende enthält.
 * Wurde die Zeichenkette nicht gefunden, wird ein leerer String
 * zurückgegeben.
 * \note
 * Ein Sonderfall besteht, wenn \p needle leer ist. In diesem Fall wird
 * der komplette String zurückgegeben.
 */
String String::strstr(const String &needle) const
{
	String ret;
	if (ptr!=NULL && stringlen>0) {
		if (needle.len()==0) return *this;
		wchar_t *p=wcsstr(ptr, needle.ptr);
		if (p) ret.set(p);
	}
	return ret;
}

/*! \brief Sucht nach einem String
 *
 * Find sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
 * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
 * Ende des Strings gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::find(const String &needle, ssize_t start) const
{
	if (ptr==NULL || stringlen==0) return -1;
	if (needle.stringlen==0) return 0;
	if (start>0 && (size_t)start>=stringlen) return -1;
	if (start<0 && ((size_t)((ssize_t)stringlen+start))>=stringlen) return -1;


	//Position to return
	size_t p = -1;
	//Length of the string to search for
	size_t lstr = needle.stringlen;
	//Current position to search from and position of found string
	wchar_t *found = NULL, *tmp = NULL;

	//Search forward
	if(start >= 0) {
		//Search first occurence, starting at the given position...
		found = wcsstr(ptr + start, needle.ptr);
		//...and calculate the position to return if str was found
		if (found != NULL) {
			p = found - ptr;
		}
	} else {
		//Search backward
		//Start counting from behind at 0, not -1
		start++;

		/* Beginning at the start of the contained string, start searching for
			   every occurence of the str and make it the position last found as long
			   as the found string doesn't exceed the defined end of the search */
		while((found = wcsstr((tmp == NULL ? ptr : tmp + 1), needle.ptr)) != NULL && found - ptr + lstr <= stringlen + start)
			tmp = found;

		//Calculate the position to return if str was found
		if(tmp != NULL) {
			p = tmp - ptr;
		}
	}
	return p;
}


/*! \brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
 *
 * \desc
 * Diese Funktion sucht innerhalb des aktuellen String nach \p needle ab der
 * gewünschten Position \a start. Gross-/Kleinschreibung wird dabei ignoriert.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0 oder wird er weggelassen,
 * wird der String vom Anfang an durchsucht. Ist der Wert jedoch negativ, wird rückwärts vom
 * Ende des Strings gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::findCase(const String &needle, ssize_t start) const
{
	String CaseNeedle(needle);
	String CaseSearch(ptr,stringlen);
	CaseNeedle.lowerCase();
	CaseSearch.lowerCase();
	return CaseSearch.find(CaseNeedle,start);
}

/*! \brief Sucht nach einem String
 *
 * \desc
 * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
 * oder nicht angegeben, wird der String vom Anfang an gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::instr(const String &needle, size_t start) const
{
	if (ptr==NULL || stringlen==0) return -1;
	if (needle.stringlen==0) return 0;
	if (start>=stringlen) return -1;
	const wchar_t * p;
	p=wcsstr((ptr+start),needle.ptr);
	if (p!=NULL) {
		return ((ssize_t)(p-ptr));
	}
	return -1;
}

/*! \brief Sucht nach einem String, Gross-/Kleinschreibung wird ignoriert
 *
 * \desc
 * Diese Funktion sucht nach dem Suchstring \a needle ab der gewünschten Position \a start.
 * Gross-/Kleinschreibung wird dabei ignoriert.
 *
 * \param[in] needle Gesuchter Teilstring
 * \param[in] start Optionale Startposition innerhalb des Suchstrings. Ist der Parameter 0
 * oder nicht angegeben, wird der String vom Anfang an gesucht.
 *
 * \return Liefert die Position innerhalb des Strings, an der der Suchstring gefunden wurde
 * oder -1 wenn er nicht gefunden wurde. Ist \p needle ein leerer String, liefert die
 * Funktion immer 0 zurück.
 */
ssize_t String::instrCase(const String &needle, size_t start) const
{
	String CaseNeedle(needle);
	String CaseSearch(ptr,stringlen);
	CaseNeedle.lowerCase();
	CaseSearch.lowerCase();
	return CaseSearch.instrCase(CaseNeedle,start);
}

String &String::stripSlashes()
{
	if (ptr==NULL || stringlen==0) return *this;
	size_t p=0, np=0;
	wchar_t a, lastchar=0;
	while ((a=ptr[p])) {
		if (lastchar!='\\' && p>0) {
			ptr[np]=lastchar;
			np++;
		}
		lastchar=a;
		p++;
	}
	if (lastchar) {
		ptr[np]=lastchar;
		np++;
	}
	ptr[np]=0;
	if (stringlen!=np) {
		stringlen=np;
	}
	return *this;
}

/*!\brief String wiederholen
 *
 * \desc
 * Mit dieser Funktion wird der Inhalt des Strings mehrfach wiederholt.
 *
 * @param num Anzahl Wiederholungen. Falls \p num 0 ist, ist der String anschließend leer.
 *
 * @return Referenz auf den verlängerten String.
 */
String& String::repeat(size_t num)
{
	if (ptr==NULL || stringlen==0) return *this;
	if (num==0) {
		clear();
		return *this;
	}
	size_t newsize=(stringlen*num+16)*sizeof(wchar_t);
	wchar_t *buf=(wchar_t*)malloc(newsize);
	if (!buf) throw OutOfMemoryException();
	wchar_t *tmp=buf;
	for (size_t i=0;i<num;i++) {
		wcsncpy(tmp,ptr,stringlen);
		tmp+=stringlen;
	}
	free(ptr);
	ptr=buf;
	stringlen=stringlen*num;
	ptr[stringlen]=0;
	s=newsize;
	return *this;
}

/*! \brief Füllt den String mit einem Zeichen
 *
 * Der String wird mit einem gewünschten Zeichen gefüllt
 * \param unicode Der Unicode des Zeichens, mit dem der String gefüllt werden soll
 * \param num Die Länge des gewünschten Strings
 * \return Referenz auf den neuen String
 */
String& String::repeat(wchar_t unicode, size_t num)
{
	if (!unicode) {
		throw IllegalArgumentException();
	}
	if (!num) {
		clear();
		return *this;
	}
	size_t newsize=(num+16)*sizeof(wchar_t);
	wchar_t *buf=(wchar_t*)malloc(newsize);
	if (!buf) throw OutOfMemoryException();
	for (size_t i=0;i<num;i++) buf[i]=unicode;
	free(ptr);
	ptr=buf;
	stringlen=num;
	ptr[stringlen]=0;
	s=newsize;
	return *this;
}

/*!\brief String wiederholen
 *
 * \desc
 * Mit dieser Funktion wird der übergebene String \p str \p num mal wiederholt und
 * das Ergebnis in diesem String gespeichert.
 *
 * @param str Der zu wiederholende String
 * @param num Anzahl wiederholungen
 * @return Referenz auf den String
 */
String& String::repeat(const String& str, size_t num)
{
	if (str.stringlen==0 || num==0 || str.ptr==NULL) {
		clear();
		return *this;
	}
	size_t newsize=(str.stringlen*num+16)*sizeof(wchar_t);
	wchar_t *buf=(wchar_t*)malloc(newsize);
	if (!buf) throw OutOfMemoryException();
	wchar_t *tmp=buf;
	for (size_t i=0;i<num;i++) {
		wcsncpy(tmp,str.ptr,str.stringlen);
		tmp+=str.stringlen;
	}
	free(ptr);
	ptr=buf;
	stringlen=num;
	ptr[stringlen]=0;
	s=newsize;
	return *this;
}

/*!\brief String multiplizieren
 *
 * \desc
 * Der aktuelle String wird \p count mal hintereinander wiederholt und
 * als neuer String zurückgegeben.
 *
 * @param[in] count Anzahl wiederholungen
 * @return Neuer String
 * \exception OutOfMemoryException Tritt auf, wenn kein Speicher mehr verfügbar ist.
 */
String String::repeated(size_t count) const
{
	String ret;
	for (size_t i=0;i<count;i++) ret.append(ptr,stringlen);
	return ret;
}


String& String::replace(const String &search, const String &replacement)
//! \brief Ersetzt einen Teilstring durch einen anderen
{
	if (ptr==NULL || stringlen==0 || search.ptr==NULL || search.stringlen==0) return *this;
	size_t start = 0, slen = search.stringlen;
	ssize_t end;
	// collect the result
	String ms;
	//Do while str is found in the contained string
	while((end = find(search, start)) >= 0) {
		//The result is built from the parts that don't match str and the replacement string
		ms += mid(start, end - start);
		ms += replacement;
		//New start for search is behind the replaced part
		start = end + slen;
	}
	//Add the remaining part of the contained string to the result
	ms += mid(start);
	//The result is assigned to this mstring
	return set(ms);
}

String &String::pregEscape()
/*! \brief Fügt dem String Escape-Zeichen zu, zur Verwendung in einem Match
 *
 * \desc
 * Der Befehl scannt den String nach Zeichen mit besonderer Bedeutung in einer Perl-Regular-Expression und
 * escaped diese mit einem Slash. Das Ergebnis kann dann in einer Regular Expression verwendet werden.
 *
 * Folgende Zeichen werden escaped: - + \ * /
 */
{
	if (ptr==NULL || stringlen==0) return *this;
	String t;
	String compare="-+\\*/";
	String letter;
	for (size_t i=0;i<stringlen;i++) {
		letter.set(ptr[i]);
		if (compare.instr(letter,0)>=0) t+="\\";
		t+=letter;
	}
	if (strcmp(t)!=0) set(t);
	return *this;
}

/*! \brief Der String wird anhand einer Regular Expression durchsucht
 *
 * Durchsucht den String anhand einer Perl-Expression
\param[in] expression Ist eine Perl-kompatible Regular Expression, die mit Slash (/) beginnt und
endet. Optional können nach dem schließenden Slash folgende Optionen angegeben werden:
\copydoc pregexpr.dox
\return Liefert \c true(1) zurück, wenn ein Match gefunden wurde, ansonsten \c false(0)
\remarks
Der String wird intern zuerst nach UTF-8 kodiert, bevor die pcre-Funktionen aufgerufen werden.
\note

\copydoc pcrenote.dox
*/
bool String::pregMatch(const String &expression) const
{
	#ifndef HAVE_PCRE
		throw UnsupportedFeatureException("PCRE");
	#else
		if (ptr==NULL || stringlen==0 || expression.ptr==NULL || expression.stringlen==0) return false;
		ByteArray utf8=toUtf8();
		ByteArray expr=expression.toUtf8();
		int flags=PCRE_UTF8;
		// letzten Slash in regex finden

		const char *options=::strrchr((const char*)expr,'/');
		if (options) {
			expr.set(options-(const char*)expr,0);
			options++;
			if (::strchr(options,'i')) flags|=PCRE_CASELESS;
			if (::strchr(options,'m')) flags|=PCRE_MULTILINE;
			if (::strchr(options,'x')) flags|=PCRE_EXTENDED;
			if (::strchr(options,'s')) flags|=PCRE_DOTALL;
			if (::strchr(options,'a')) flags|=PCRE_ANCHORED;
			if (::strchr(options,'u')) flags|=PCRE_UNGREEDY;
		}
		const char *perr;
		int re,erroffset, ovector[32];
		int perrorcode;
		pcre *reg;
		//printf ("r=%s, flags=%i\n",r,flags);
		reg=pcre_compile2((const char*)expr,flags,&perrorcode,&perr, &erroffset, NULL);
		if (!reg) throw IllegalRegularExpressionException();
		memset(ovector,0,30*sizeof(int));
		if ((re=pcre_exec(reg, NULL, (const char*) utf8,utf8.size(),0, 0, ovector, 30))>=0) {
			return true;
		}
		return false;
#endif
}

/*! \brief Der String wird anhand einer Regular Expression durchsucht
 *
 * Durchsucht den String anhand einer Perl-Expression
\param[in] expression Ist eine Perl-kompatible Regular Expression, die mit Slash (/) beginnt und
endet. Optional können nach dem schließenden Slash folgende Optionen angegeben werden:
\copydoc pregexpr.dox
\return Liefert \c true(1) zurück, wenn ein Match gefunden wurde, ansonsten \c false(0)
\remarks
Der String wird intern zuerst nach UTF-8 kodiert, bevor die pcre-Funktionen aufgerufen werden.
\note

\copydoc pcrenote.dox
*/
bool String::pregMatch(const String &expression, Array &matches, size_t maxmatches) const
{
	#ifndef HAVE_PCRE
		throw UnsupportedFeatureException("PCRE");
	#else
		if (ptr==NULL || stringlen==0 || expression.ptr==NULL || expression.stringlen==0) return false;
		ByteArray utf8=toUtf8();
		ByteArray expr=expression.toUtf8();
		int flags=PCRE_UTF8;
		// letzten Slash in regex finden
		const char *options=::strrchr((const char*)expr,'/');
		if (options) {
			expr.set(options-(const char*)expr,0);
			options++;
			if (::strchr(options,'i')) flags|=PCRE_CASELESS;
			if (::strchr(options,'m')) flags|=PCRE_MULTILINE;
			if (::strchr(options,'x')) flags|=PCRE_EXTENDED;
			if (::strchr(options,'s')) flags|=PCRE_DOTALL;
			if (::strchr(options,'a')) flags|=PCRE_ANCHORED;
			if (::strchr(options,'u')) flags|=PCRE_UNGREEDY;
		}
		const char *perr;
		if (maxmatches<16) maxmatches=16;
		int re,erroffset;
		int ovectorsize=(maxmatches+1)*2;
		int *ovector=(int*)malloc(ovectorsize*sizeof(int));
		int perrorcode;
		pcre *reg;
		//printf ("r=%s, flags=%i\n",r,flags);
		reg=pcre_compile2((const char*)expr,flags,&perrorcode,&perr, &erroffset, NULL);
		if (!reg) throw IllegalRegularExpressionException();
		memset(ovector,0,30*sizeof(int));
		if ((re=pcre_exec(reg, NULL, (const char*) utf8,utf8.size(),0, 0, ovector, ovectorsize))>=0) {
			for (int i=0;i<14;i++) {
				const char *tmp=NULL;
				pcre_get_substring((const char*)utf8,ovector,ovectorsize,i,(const char**)&tmp);
				if (tmp) {
					printf("tmp[%i]=%s\n",i,tmp);
					//res->Set(i,tmp);
					pcre_free_substring(tmp);
				}
			}

			return true;
		}
		return false;
#endif
}



/*!\brief Kleiner als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner dem des
 * rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator<(const String &str) const
{
	if (strcmp(str)<0) return true;
	return false;
}

/*!\brief Kleiner oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters kleiner oder gleich
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator<=(const String &str) const
{
	if (strcmp(str)<=0) return true;
	return false;
}

/*!\brief Gleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters mit dem des
 * rechten identisch ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator==(const String &str) const
{
	if (strcmp(str)==0) return true;
	return false;
}

/*!\brief Ungleich
 *
 * \desc
 * Dieser Operator liefert \c true zurück, wenn der Wert des linken Parameters nicht dem des
 * rechten entspricht.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator!=(const String &str) const
{
	if (strcmp(str)==0) return false;
	return true;

}

/*!\brief Größer oder gleich
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer oder
 * gleich dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator>=(const String &str) const
{
	if (strcmp(str)>=0) return true;
	return false;
}

/*!\brief Größer als
 *
 * \desc
 * Dieser Operator liefert true zurück, wenn der Wert des linken Parameters größer
 * dem des rechten ist.
 *
 * @param str Zu vergleichender String
 * @return Liefert \c true oder \c false zurück
 */
bool String::operator>(const String &str) const
{
	if (strcmp(str)>0) return true;
	return false;
}


const wchar_t * String::getPtr() const
{
	if (ptr==NULL || stringlen==0) return L"";
	return (wchar_t*)ptr;
}


String::operator const wchar_t *() const
{
	if (ptr==NULL || stringlen==0) return L"";
	return (wchar_t*)ptr;
}

String::operator bool() const
{
	if (isTrue()) return true;
	return false;
}


String::operator int() const
{
	if (!stringlen) return 0;
	return wcstol(ptr,NULL,0);

}

String::operator unsigned int() const
{
	if (!stringlen) return 0;
	return wcstol(ptr,NULL,0);
}

String::operator long() const
{
	return toLong();
}

String::operator unsigned long() const
{
	return toUnsignedLong();
}

String::operator long long() const
{
	return toLongLong();
}

String::operator unsigned long long() const
{
	return toUnsignedLongLong();
}

String::operator float() const
{
	return toFloat();
}

String::operator double() const
{
	return toDouble();
}


String::operator std::string() const
{
	ByteArray utf8=toUtf8();
	return std::string((const char*)utf8,utf8.size());
}

String::operator std::wstring() const
{
	return std::wstring(ptr,stringlen);
}

int String::toInt() const
{
	if (!stringlen) return 0;
	return wcstol(ptr,NULL,0);
}

unsigned int String::toUnsignedInt() const
{
	if (!stringlen) return 0;
	return wcstoul(ptr,NULL,0);
}

pplint64 String::toInt64() const
{
	if (!stringlen) return 0;
#ifdef HAVE_WCSTOLL
	return (pplint64) wcstoll(ptr,NULL,0);
#elif WIN32
	return (pplint64) _wcstoi64(ptr,NULL,0);
#else
	return (pplint64) pplwcstoll(ptr,NULL,0);
#endif
}

ppluint64 String::toUnsignedInt64() const
{
	if (!stringlen) return 0;
#ifdef HAVE_WCSTOULL
	return (ppluint64) wcstoll(ptr,NULL,0);
#elif WIN32
	return (ppluint64) _wcstoi64(ptr,NULL,0);
#else
	return (ppluint64) pplwcstoll(ptr,NULL,0);
#endif

}

bool String::toBool() const
{
	if (isTrue()) return true;
	return false;
}

long String::toLong() const
{
	if (!stringlen) return 0;
	return wcstol(ptr,NULL,0);
}

unsigned long String::toUnsignedLong() const
{
	if (!stringlen) return 0;
	return wcstoul(ptr,NULL,0);
}


long long String::toLongLong() const
{
	if (!stringlen) return 0;
#ifdef HAVE_WCSTOLL
	return (long long) wcstoll(ptr,NULL,0);
#elif WIN32
	return (long long) _wcstoi64(ptr,NULL,0);
#else
	return (long long) pplwcstoll(ptr,NULL,0);
#endif
}

unsigned long long String::toUnsignedLongLong() const
{
	if (!stringlen) return 0;
#ifdef HAVE_WCSTOULL
	return (unsigned long long) wcstoull(ptr,NULL,0);
#elif HAVE_WCSTOLL
	return (unsigned long long) wcstoll(ptr,NULL,0);
#elif WIN32
	return (unsigned long long) _wcstoi64(ptr,NULL,0);
#else
	return (unsigned long long) pplwcstoll(ptr,NULL,0);
#endif
}

float String::toFloat() const
{
	if (!stringlen) return 0;
#ifdef WIN32
	return (float)wcstod(ptr,NULL);
#elif HAVE_WCSTOF
	return (float)wcstof(ptr,NULL);
#elif HAVE_WCSTOD
	return (float)wcstod(ptr,NULL);
#else
	#error *** Keine Funktion, um einen Wide-String in einen float zu konvertieren!
	return 0;
#endif
}

double String::toDouble() const
{
	if (!stringlen) return 0;
#ifdef WIN32
	return wcstod(ptr,NULL);
#elif HAVE_WCSTOD
	return wcstod(ptr,NULL);
#else
	#error *** Keine Funktion, um einen Wide-String in einen double zu konvertieren!
	return 0;
#endif
}


/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String &str1, const String& str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const char *str1, const String& str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String &str1, const char *str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const wchar_t *str1, const String& str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String &str1, const wchar_t *str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const std::string &str1, const String& str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String &str1, const std::string &str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const std::wstring &str1, const String& str2)
{
	String s=str1;
	s.append(str2);
	return s;
}

/*!\brief String addieren
 *
 * \relates ppl7::String
 *
 * \desc
 * Zwei Strings werden zu einem neuen String zusammengefügt.
 *
 * @param[in] str1 Erster String
 * @param[in] str2 Zweiter String
 * @return Neuer String
 */
String operator+(const String &str1, const std::wstring &str2)
{
	String s=str1;
	s.append(str2);
	return s;
}


std::ostream& operator<<(std::ostream& s, const String &str)
{
	ByteArray a=str.toUtf8();
	return s.write((const char*)a.adr(),a.size());
}

} // EOF namespace ppl7

