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
#ifdef  HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

// ERRORSFIXED=1


#include "ppl6.h"

/*
Format der Archiv-Dateien:

	Byte 0-4:	"PARC" + Nullbyte						(5 Bytes)
	Byte   5:	Hauptversion 3							(1 Byte)
	Byte   6:	Unterversion 0							(1 Byte)
	Byte   7:	reserved								(1 Byte)
	Byte   8:	Gesamtgröße des Inhalts unkomprimiert	(8 Byte)
	Byte  16:	Gesamtgröße des Inhalts komprimiert		(8 Byte)
	Byte  24:	Anzahl Dateien							(4 Byte)
	Byte  28:	Pointer auf Beginn der Chunks			(4 Byte)

Chunks:
	Byte  0:	Groesse des Chunks ohne Header			(4 Byte)
	Byte  4:	Chunk-ID								(1 Byte)
				0=ungueltig
				1=Name des Archivs (darf nur einmal vorkommen)
				2=Autor des Archivs (darf nur einmal vorkommen)
				3=Beschreibung (darf nur einmal vorkommen)
				4=Daten
	Byte  5:	Flags:
				Bit 0: geloescht
				Bit 1: komprimiert
	Byte  6:	Komprimierung
				0=unkomprimiert
				1=zlib
				2=bzip2
	Byte  7:	reserved
	Byte  8:	Beginn der Daten

	Bei Name, Autor, Beschreibung: String mit abschlie�endem Nullbyte

	Bei Daten:
		Byte  0: Groesse der Datei im Archiv (komprimiert)	(4 Byte)
		Byte  4: Groesse der Datei unkomprimiert			(4 Byte)
		Byte  8: Groesse des Dateinamens					(2 Byte)
		Byte 10: Dateiname, evtl. inkl. Pfad

		danach: Beginn der Daten


*/

namespace ppl6 {

#define CHUNK_HEADER_SIZE	8

typedef struct tagCHUNK
{
	struct tagCHUNK *previous, *next;
	ppluint64	filepos;
	char		*filename;
	ppluint32	chunksize;
	ppluint32	file_uncompressed;
	ppluint32	file_compressed;
	ppluint8	chunkId;
	ppluint8	flags;
	ppluint8	compression;
} CHUNK;

CArchiv::CArchiv()
{
	ff=NULL;
	size_uncompressed=0;
	size_compressed=0;
	filecount=0;
	nextptr=0;
	first_chunk=last_chunk=NULL;
}

CArchiv::~CArchiv()
{
	if (ff) {
		PushError();
		Close();
		PopError();
	}
}

int CArchiv::Create(char *filename)
{
	if (ff) Close();
	ff=new CFile(filename, "w+");
	if (GetErrorCode()>0) {
		return 0;
	}
	size_uncompressed=0;
	size_compressed=0;
	filecount=0;
	nextptr=32;
	SetError (0);
	return WriteHeader();
}

int CArchiv::Open(char *filename)
{
	if (ff) Close();
	ff=new CFile(filename, "r+");
	if (GetErrorCode()>0) {
		return 0;
	}
	nextptr=ff->Size();
	if (ReadHeader()) return 1;
	delete ff;
	return 0;
}

int CArchiv::WriteHeader()
{
	if (!ff) {
		SetError(239);
		return 0;
	}
	if (ff->Seek(0)!=0) {
		SetError(11);
		return 0;
	}
	if (GetErrorCode()>0) {
		return 0;
	}
	char *buffer=(char*) malloc(32);
	if (!buffer) {
		SetError(2);
		return 0;
	}
	pokesz(buffer+0,(char*)"PARC");
	poke8(buffer+5,3);			// Hauptversion
	poke8(buffer+6,0);			// Unterversion
	poke8(buffer+7,0);			// reserved
	poke64(buffer+8,size_uncompressed);
	poke64(buffer+16,size_compressed);
	poke32(buffer+24,filecount);
	poke32(buffer+28,32);
	ff->Write(buffer,32);

	free(buffer);
	return 1;
}

int CArchiv::ReadHeader()
{
	if (!ff) {
		SetError(239);
		return 0;
	}
	if (ff->Seek(0)!=0) {
		SetError(11);
		return 0;
	}
	if (GetErrorCode()>0) {
		return 0;
	}
	char *buffer=(char*) calloc(32,1);
	if (!buffer) {
		SetError(2);
		return 0;
	}
	ff->Read(buffer,32);
	if (peek8(buffer+4)!=0) {
		SetError(240);
		return 0;
	}
	if (strncmp("PARC",buffer,4)!=0) {
		SetError(240);
		return 0;
	}
	if (peek8(buffer+5)!=3 || peek8(buffer+6)!=0 || peek32(buffer+28)!=32) {
		SetError(241);
		return 0;
	}
	size_uncompressed=peek64(buffer+8);
	size_compressed=peek64(buffer+16);
	filecount=peek32(buffer+24);
	free(buffer);
	return 1;
}

int CArchiv::Close()
{
	if (!ff) return 1;
	CHUNK *chunk=(CHUNK*)last_chunk;
	CHUNK *tmp;
	while (chunk) {
		tmp=chunk;
		chunk=chunk->previous;
		free(tmp);
	}

	first_chunk=last_chunk=NULL;
	nextptr=0;
	delete ff;
	ff=NULL;
	return 1;
}

int CArchiv::SetName(char *name)
{
	return SetText(name,1);
}

int CArchiv::SetAutor(char *name)
{
	return SetText(name,2);
}

int CArchiv::SetDescription(char *name)
{
	return SetText(name,3);
}

int CArchiv::SetText(char *name, int chunkid)
{
	if (!ff) {
		SetError(153);
		return 0;
	}
	CHUNK *c=NULL;
	CHUNK *chunk=(CHUNK*) FindChunk(chunkid);
	if (!chunk) {
		if (!name) return 1;
		c=(CHUNK*)malloc(sizeof(CHUNK)+strlen(name));
	} else {
		if (!name) {		// Name loeschen
			chunk->flags|=1;
			return SaveChunk(chunk);
		}
		if (chunk->chunksize<=strlen(name)+1) {
			chunk->file_compressed=(ppluint32)strlen(name)+1;
			chunk->file_uncompressed=(ppluint32)strlen(name)+1;
			ff->Write(name,(ppluint32)strlen(name)+1,chunk->filepos+CHUNK_HEADER_SIZE);
			return SaveChunk(chunk);
		}
		chunk->flags|=1;		// Alten Chunk loeschen
		SaveChunk(chunk);
		c=(CHUNK*)malloc(sizeof(CHUNK)+strlen(name));
	}
	if (!c) {
		SetError(2);
		return 0;
	}
	memset(c,0,sizeof(CHUNK));
	c->chunkId=chunkid;
	c->chunksize=(ppluint32)strlen(name)+1;
	c->compression=0;
	c->file_compressed=c->chunksize;
	c->file_uncompressed=c->chunksize;
	c->filename=NULL;
	c->filepos=nextptr;
	c->flags=0;
	c->next=NULL;
	c->previous=(CHUNK*)last_chunk;
	if (c->previous) c->previous->next=c;
	SaveChunk(c);
	ff->Write(name,(ppluint32)c->file_compressed,c->filepos+CHUNK_HEADER_SIZE);
	nextptr+=CHUNK_HEADER_SIZE+c->chunksize;
	size_uncompressed+=c->file_uncompressed;
	size_compressed+=c->file_compressed;
	WriteHeader();


	last_chunk=c;
	if (!first_chunk) first_chunk=c;
	return 1;
}

int CArchiv::SaveChunk(void *data)
{
	if (!ff) {
		SetError(153);
		return 0;
	}
	CHUNK *chunk=(CHUNK*)data;
	if (!chunk) {
		SetError(194);
		return 0;
	}
	char d[8];
	poke32(d,chunk->chunksize);
	poke8(d+4,chunk->chunkId);
	poke8(d+5,chunk->flags);
	poke8(d+6,chunk->compression);
	poke8(d+7,0);
	ff->Write(d,8,chunk->filepos);
	return 1;
}

void *CArchiv::FindChunk(int chunkid, char *filename)
{
	if (!ff) {
		SetError(153);
		return NULL;
	}
	if (chunkid==4 && filename==NULL) {
		SetError(243);
		return NULL;
	}
	if (chunkid<1 || chunkid>4) {
		SetError(242);
		return NULL;
	}
	CHUNK *chunk=(CHUNK*)first_chunk;
	while (chunk) {
		if (chunk->chunkId==chunkid && (chunk->flags&1)==0) {
			if (chunkid==1 || chunkid==2 || chunkid==3) return chunk;
			if (chunkid==4 && strcmp(chunk->filename,filename)==0) return chunk;
		}
		chunk=chunk->next;
	}
	return NULL;
}

int CArchiv::Add(char *filename, bool includepath, int compression, int level)
{
	CFile f;
	if (!f.Open(filename,"rb")) {
		return 0;
	}
	return Add(&f,filename,includepath,compression,level);
}

int	CArchiv::Add(void *adr, ppluint32 size, char *filename, bool includepath, int compression, int level)
{
	CMemFile f;
	if (!f.Open(adr,size)) {
		return 0;
	}
	return Add(&f,filename,includepath,compression,level);
}

int	CArchiv::Add(CFileObject *file, char *filename, bool includepath, int compression, int level)
{
	if (!ff) {
		SetError(153);
		return 0;
	}
	if (!file) {
		SetError(194,"CFileObject *file");
		return 0;
	}
	if (!filename) {
		SetError(194,"char *filename");
		return 0;
	}

	CHUNK *c=(CHUNK*) FindChunk(4,filename);
	if (c) {			// Die Datei gibt es schon
		SetError(244,"File: %s",filename);
		return 0;
	}
	CString fname=filename;
	if (!includepath) fname=GetFilename(filename);
	ppluint32 l_fname=fname.Size()+1;

	c=(CHUNK*)malloc(sizeof(CHUNK)+l_fname);
	if (!c) {
		SetError(2);
		return 0;
	}

	memset(c,0,sizeof(CHUNK));
	c->chunkId=4;
	c->chunksize=(ppluint32)file->Size()+10+l_fname;		// TODO
	c->compression=0;
	c->file_compressed=(ppluint32)file->Size();			// TODO
	c->file_uncompressed=(ppluint32)file->Size();
	c->filename=((char*)c)+sizeof(CHUNK);
	c->filepos=nextptr;
	c->flags=0;
	c->next=NULL;
	c->previous=(CHUNK*)last_chunk;
	if (c->previous) c->previous->next=c;
	SaveChunk(c);
	char d[10];
	/*
		Byte  0: Groesse der Datei im Archiv (komprimiert)	(4 Byte)
		Byte  4: Groesse der Datei unkomprimiert			(4 Byte)
		Byte  8: Groesse des Dateinamens					(2 Byte)
		Byte 10: Dateiname, evtl. inkl. Pfad
		*/
	poke32(d,c->file_compressed);
	poke32(d+4,c->file_uncompressed);
	poke16(d+8,l_fname);
	ff->Seek(c->filepos+CHUNK_HEADER_SIZE);
	ff->Write(d,10);			// Header speichern
	ff->Write(fname,l_fname);	// Dateiname speichern
	ff->Copy(*file,file->Size());

	nextptr+=CHUNK_HEADER_SIZE+c->chunksize;
	size_uncompressed+=c->file_uncompressed;
	size_compressed+=c->file_compressed;
	WriteHeader();


	last_chunk=c;
	if (!first_chunk) first_chunk=c;
	return 1;
}





}	// EOF namespace ppl6

