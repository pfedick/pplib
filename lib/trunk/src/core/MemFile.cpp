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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
	#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
	#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_FILE_H
	#include <sys/file.h>
#endif
#ifdef HAVE_STDARG_H
	#include <stdarg.h>
#endif
#ifdef HAVE_ERRNO_H
	#include <errno.h>
#endif
#ifdef _WIN32
#include <io.h>
#define WIN32_LEAN_AND_MEAN		// Keine MFCs
#define popen _popen
#define pclose _pclose

#include <windows.h>
#ifdef DeleteFile
#undef DeleteFile
#endif
#endif
#include "ppl7.h"

namespace ppl7 {

#define COPYBYTES_BUFFERSIZE 1024*1024

/*!\class MemFile
 * \ingroup PPLGroupFileIO
 * \brief Simulation von Dateizugriffen im Hauptspeicher
 *
 * \desc
 * Mit dieser von CFileObject abgeleiteten Klasse können Dateizugriffe im Hauptspeicher simuliert werden.
 * Sie ist hauptsächlich zum Lesen von Daten gedacht, z.B. aus einer Resourcendatei. Zwar können auch
 * Daten geschrieben werden, jedoch nicht über die initiale Größe des Speicherbereichs hinaus.
 * \par
 * Der zu verwendende Speicherbereich kann entweder über den Konstruktor abgegeben werden (siehe
 * MemFile::MemFile (void * adresse, size_t size) ) oder über die Funktion MemFile::Open.
 *
 */

MemFile::MemFile ()
{
	mysize=0;
	pos=0;
	buffer=NULL;
	MemBase=NULL;
}


MemFile::MemFile (void * adresse, size_t size)
/*!\brief Konstruktor der Klasse mit Angabe eines Speicherbereichs
 *
 * Mit diesem Konstruktor wird gleichzeitig ein Pointer auf den Speicherbereich \p adresse mit einer
 * Größe von \p size Bytes übergeben. Sämtliche Dateizugriffe werden in diesem Speicherbereich
 * simuliert.
 *
 * @param adresse Pointer auf den zu verwendenden Speicherbereich
 * @param size Größe des Speicherbereichs
 */
{
	#ifdef DEBUGLOG	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> DEBUG
		Debug.Log ("%u->MemFile::MemFile (adresse=%u, size=%u)",this,adresse,size);
	#endif

	MemBase=NULL;
	buffer=NULL;
	pos=0;
	open(adresse,size);
}

/*!\brief Konstruktor der Klasse mit Angabe eines Speicherbereichs
 *
 * Mit diesem Konstruktor wird gleichzeitig ein Pointer auf den Speicherbereich \p adresse mit einer
 * Größe von \p size Bytes übergeben. Sämtliche Dateizugriffe werden in diesem Speicherbereich
 * simuliert.
 *
 * @param adresse Pointer auf den zu verwendenden Speicherbereich
 * @param size Größe des Speicherbereichs
 */
MemFile::MemFile (const ByteArrayPtr &memory)
{
	if (memory.isEmpty()) {
		MemBase=NULL;
		buffer=NULL;
		pos=0;
		return;
	}
	MemBase=(char*)memory.adr();
	mysize=memory.size();
	pos=0;
}

MemFile::~MemFile()
{
	if (buffer) free(buffer);
}

int MemFile::open (void * adresse, size_t size)
/*!\brief Speicherbereich öffnen
 *
 * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher geöffnet. Dazu muss mit
 * \p adresse ein Pointer auf den Beginn des zu verwendenden Hauptspeichers angegeben werden,
 * sowie mit \p size seine Größe. Sämtliche nachfolgenden Dateizugriffe werden dann in diesem
 * Speicherbereich simuliert.
 *
 * @param adresse Pointer auf den zu verwendenden Speicherbereich
 * @param size Größe des Speicherbereichs
 * \return Die Funktion gibt bei Erfolg 1 zurück, im Fehlerfall 0. Ein Fehler kann nur auftreten,
 * wenn einer der Parameter \p adresse oder \p size einen Nullwert aufweisen.
 */

{
	if (adresse==NULL || size==0) throw IllegalArgumentException();
	MemBase=(char*)adresse;
	mysize=size;
	pos=0;
	return 1;
}

/*!\brief Speicherbereich öffnen
 *
 * Mit dieser Funktion wird die simulierte Datei im Hauptspeicher geöffnet. Dazu muss mit
 * \p adresse ein Pointer auf den Beginn des zu verwendenden Hauptspeichers angegeben werden,
 * sowie mit \p size seine Größe. Sämtliche nachfolgenden Dateizugriffe werden dann in diesem
 * Speicherbereich simuliert.
 *
 * @param memory Referenz auf eine ByteArrayPtr-Klasse, die den zu verwendenden Speicherbereich enthält
 * \return Die Funktion gibt bei Erfolg 1 zurück, im Fehlerfall 0. Ein Fehler kann nur auftreten,
 * wenn einer der Parameter \p adresse oder \p size einen Nullwert aufweisen.
 */
int MemFile::open(const ByteArrayPtr &memory)
{
	if (memory.isEmpty()) throw IllegalArgumentException();
	MemBase=(char*)memory.adr();
	mysize=memory.size();
	pos=0;
	return 1;
}

bool MemFile::isOpen() const
{
	if (MemBase!=NULL) return true;
	return false;
}

int MemFile::close()
{
	MemBase=NULL;
	mysize=0;
	pos=0;
	if (buffer!=0) {
		free (buffer);
		buffer=0;
	}
	return 1;
}

pplint64 MemFile::size () const
{
	if (MemBase!=NULL) {
		return (pplint64)mysize;
	}
	throw FileNotOpenException();
}

int MemFile::seek(ppluint64 position)
{
	if (MemBase!=NULL) {
		if (position<mysize) {
			pos=position;
		} else {
			return 0;
		}
		return 1;
	}
	throw FileNotOpenException();
}

int	 MemFile::fseek (ppluint64 offset, SeekOrigin origin )
{
	if (MemBase!=NULL) {
		//suberr=::fseek(ff,offset,origin);
		//if (suberr==0) {
		switch (origin) {
			case SEEKCUR:
				pos+=offset;
				if (pos>mysize) return 1;
				if (pos<0) {pos=0;return 1; }
				break;
			case SEEKEND:
				pos=mysize-offset;
				if (pos>mysize) return 1;
				if (pos<0) {pos=0; return 1;}
				break;
			case SEEKSET:
				pos=offset;
				if (pos<0) {pos=0; return 1;}
				if (pos>mysize) return 1;
		}
		throw FileSeekException();
	}
	throw FileNotOpenException();
}

pplint64 MemFile::ftell()
{
	if (MemBase!=NULL) {
		return pos;
	}
	throw FileNotOpenException();
}


size_t MemFile::fread(void *ptr, size_t size, size_t nmemb)
{
	if (MemBase==NULL) throw FileNotOpenException();
	size_t by=nmemb;
	if (pos+(by*size)>mysize) by=(size_t)(mysize-pos)/size;
	memmove(ptr,MemBase+pos,by*size);
	pos+=(by*size);
	if (by<nmemb) throw ReadException();
	return by;
}

size_t MemFile::fwrite(const void *ptr, size_t size, size_t nmemb)
{
	if (MemBase==NULL) throw FileNotOpenException();
	size_t by=nmemb;
	if (pos+(by*size)>mysize) by=(size_t)(mysize-pos)/size;
	memmove(MemBase+pos,ptr,by*size);
	pos+=(by*size);
	if (by<nmemb) throw WriteException();
	return by;
}


ppluint64 MemFile::doCopy (FileObject &quellfile, ppluint64 bytes)
{
	if (MemBase==NULL) throw FileNotOpenException();
	if (buffer==NULL) {
		buffer=(char *)malloc(COPYBYTES_BUFFERSIZE);
		if (buffer==NULL) throw OutOfMemoryException();
	}
	if (quellfile.size()>quellfile.ftell()) {
		if (quellfile.ftell()+(pplint64)bytes>quellfile.size()) {
			bytes=(ppluint64)quellfile.lof()-(ppluint64)quellfile.ftell();
		}
		ppluint64 rest=bytes;
		ppluint32 by;

		while (rest>0) {
			by=quellfile.fread (buffer,1,COPYBYTES_BUFFERSIZE);
			write (buffer,by);
			rest-=by;
		}
	}
	return bytes;
}

char *MemFile::fgets (char *buffer1, size_t num)
{
	if (MemBase!=NULL) {
		if (pos>=mysize) throw EndOfFileException();
		ppluint64 by;
		by=num-1;
		if (pos+by>mysize) by=(ppluint64)(mysize-pos);
		char *ptr=MemBase+pos;
		ppluint64 i;
		for (i=0;i<by;i++) {
			if ((buffer1[i]=ptr[i])=='\n') {
				i++;
				break;
			}
		}
		buffer1[i]=0;
		pos+=i;
		if (pos>=mysize) throw EndOfFileException();
		return buffer1;
	}
	throw FileNotOpenException();
}

wchar_t *MemFile::fgetws (wchar_t *buffer1, size_t num)
{
	if (MemBase!=NULL) {
		if (pos>=mysize) throw EndOfFileException();
		ppluint64 by;
		by=(num-1)*sizeof(wchar_t);
		if (pos+by>mysize) by=(ppluint64)(mysize-pos);
		wchar_t *ptr=(wchar_t*)(MemBase+pos);
		ppluint64 i;
		for (i=0;i<(num-1);i++) {
			if ((buffer1[i]=ptr[i])==L'\n') {
				i++;
				break;
			}
		}
		buffer1[i]=0;
		pos+=(i*sizeof(wchar_t));
		if (pos>=mysize) throw EndOfFileException();
		return buffer1;
	}
	throw FileNotOpenException();
}

int MemFile::puts (const char *str)
{
	if (MemBase!=NULL) {
		fwrite ((void*)str,1,(ppluint32)strlen(str));
		return 1;
	}
	throw FileNotOpenException();
}

int MemFile::putws (const wchar_t *str)
{
	if (MemBase!=NULL) {
		fwrite (str,1,(ppluint32)wcslen(str)*sizeof(wchar_t));
		return 1;
	}
	throw FileNotOpenException();
}

int MemFile::putc(int c)
{
	if (MemBase!=NULL) {
		MemBase[pos++]=(ppluint8)c;
		return c;
	}
	throw FileNotOpenException();
}

int MemFile::putwc(wchar_t c)
{
	if (MemBase!=NULL) {
		wchar_t *a=(wchar_t*)(MemBase+pos);
		pos++;
		a[0]=c;
		return c;
	}
	throw FileNotOpenException();
}

int MemFile::getc()
{
	if (MemBase!=NULL) {
		return MemBase[pos++];
	}
	throw FileNotOpenException();
}

wchar_t MemFile::getwc()
{
	if (MemBase!=NULL) {
		wchar_t *a=(wchar_t*)(MemBase+pos);
		pos++;
		return a[0];
	}
	throw FileNotOpenException();
}


bool MemFile::eof() const
{
	if (MemBase!=NULL) {
		if (pos>=mysize) return true;
		return false;

	}
	throw FileNotOpenException();
}

char *MemFile::adr(size_t adresse)
{
	if (MemBase!=NULL) {
		return (MemBase+adresse);
	}
	throw FileNotOpenException();
}

const char *MemFile::map(ppluint64 position, size_t bytes)
{
	if (MemBase==NULL) throw FileNotOpenException();
	if (position+bytes<=mysize) {
		return (MemBase+position);
	}
	return NULL;
}

char *MemFile::mapRW(ppluint64 position, size_t bytes)
{
	if (MemBase==NULL) throw FileNotOpenException();
	if (position+bytes<=mysize) {
		return (MemBase+position);
	}
	return NULL;
}

void MemFile::unmap()
{
	return;
}


int MemFile::flush()
{
	return 1;
}

int MemFile::sync()
{
	return 1;
}


} // end of namespace ppl7
