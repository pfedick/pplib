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

#include "ppl7.h"


namespace ppl7 {

typedef struct tagChunk {
	struct tagChunk *next;
	int		id;
	String	name;
	ppluint32	size_u;
	ppluint32	size_c;
	int			compression;
	void	*data;
	const void	*uncompressed;
	int			clearbuffer;
} RESCHUNK;

Resource::Resource()
{
	firstchunk=NULL;
	count=0;
	maxid=minid=0;
	major=0;
	minor=0;
}

Resource::~Resource()
{
	clear();
}

void Resource::clear()
{
	RESCHUNK *res=(RESCHUNK *)firstchunk;
	while (res) {
		if (res->clearbuffer) free(res->data);
		res->clearbuffer=0;
		res->data=NULL;
		res=res->next;
	}
	heap.clear();
	memory.free();
	memref.use(NULL,0);
	firstchunk=NULL;
	minid=0;
	maxid=0;
	count=0;
}


void Resource::load(const String &filename)
{
	File ff;
	ff.open(filename,File::READ);
	load(ff);
}

void Resource::load(FileObject &file)
{
	clear();
	size_t size=(size_t)file.size();
	void *buffer=memory.malloc(size);
	if (!buffer) throw OutOfMemoryException();
	try {
		if (file.read(buffer,size,0)!=size) {
			memory.free();
			throw FailedToLoadResourceException();
		}
		checkResource(memory);
		memref=memory;
		parse();
	} catch (...) {
		memory.free();
		memref.use(NULL,0);
		heap.clear();
		throw;
	}
}

void Resource::load(const ByteArrayPtr &memory)
{
	clear();
	checkResource(memory);
	this->memory.copy(memory);
	memref=this->memory;
	parse();
}

void Resource::useMemory(const ByteArrayPtr &memory)
{
	clear();
	checkResource(memory);
	this->memref=memory;
	parse();
}

void Resource::useMemory(void *ptr, size_t size)
{
	clear();
	memref.use(ptr,size);
	checkResource(memref);
	parse();
}

void Resource::checkResource(const ByteArrayPtr &memory)
{
	if (memory.size()<9) throw InvalidResourceException();
	// Die ersten 9 Bytes müssen folgende Werte haben:
	// 0x50,0x50,0x4c,0x52,0x45,0x53,0x00,0x06,0x00
	if (memory[0]!=0x50) throw InvalidResourceException();
	if (memory[1]!=0x50) throw InvalidResourceException();
	if (memory[2]!=0x4c) throw InvalidResourceException();
	if (memory[3]!=0x52) throw InvalidResourceException();
	if (memory[4]!=0x45) throw InvalidResourceException();
	if (memory[5]!=0x53) throw InvalidResourceException();
	if (memory[6]!=0x00) throw InvalidResourceException();
	if (memory[7]!=0x06) throw InvalidResourceException();
	if (memory[8]!=0x00) throw InvalidResourceException();
}

void Resource::parse()
{
	const char *b=(const char*)memref.adr();
	// Welche Version haben wir?
	major=Peek8(b+7);
	minor=Peek8(b+8);
	b=b+9;
	maxid=0;
	count=0;
	minid=999999;
	int size=Peek32(b);
	while (size) {
		count++;
		b=b+size;
		size=Peek32(b);
	}
	heap.init(sizeof(RESCHUNK),count+1,10);
	b=(const char*)memref.adr()+9;
	size=Peek32(b);
	RESCHUNK *lastres=NULL;
	while (size) {
		int id=Peek16(b+4);
		if(id<minid) minid=id;
		if(id>maxid) maxid=id;
		int size_u=Peek32(b+6);
		int size_c=Peek32(b+10);
		int type=Peek8(b+14);
		int off=Peek8(b+15);
		const char *name=b+16;
		RESCHUNK *res=(RESCHUNK*)heap.malloc();
		if (!res) throw OutOfMemoryException();
		if (!firstchunk) firstchunk=res;
		if(lastres) lastres->next=res;
		res->id=id;
		res->name.set(name);
		res->data=NULL;
		res->size_c=size_c;
		res->size_u=size_u;
		res->compression=type;
		res->uncompressed=b+off;
		res->clearbuffer=0;
		res->next=NULL;
		if(type==0) res->data=(void*)res->uncompressed;	// Unkomprimiert
		lastres=res;
		b=b+size;
		size=Peek32(b);
	}
}

void *Resource::find(int id)
{
	RESCHUNK *res=(RESCHUNK *)firstchunk;
	if (!firstchunk) throw ResourceNotFoundException();
	while (res) {
		if (res->id==id) {
			if (!res->data) {
				uncompress(res);
			}
			return res;
		}
		res=res->next;
	}
	throw ResourceNotFoundException();
}

void *Resource::find(const String &name)
{
	RESCHUNK *res=(RESCHUNK *)firstchunk;
	if (!firstchunk) throw ResourceNotFoundException();
	while (res) {
		if (res->name==name) {
			if (!res->data) {
				uncompress(res);
			}
			return res;
		}
		res=res->next;
	}
	throw ResourceNotFoundException();
}

CMemoryReference Resource::GetMemory(int id)
{
	CMemoryReference m;
	RESCHUNK *res=(RESCHUNK *)Find(id);
	if (!res) return m;
	m.set(res->data,res->size_u);
	return m;
}

CMemoryReference Resource::GetMemory(const char *name)
{
	CMemoryReference m;
	RESCHUNK *res=(RESCHUNK *)Find(name);
	if (!res) return m;
	m.set(res->data,res->size_u);
	return m;
}

CFileObject *Resource::GetFile(int id)
{
	RESCHUNK *res=(RESCHUNK *)Find(id);
	if (!res) return NULL;
	CMemFile *ff=new CMemFile();
	if(!ff->Open(res->data,res->size_u)) {
		delete ff;
		return NULL;
	}
	ff->SetFilename(res->name);
	return ff;
}

CFileObject *Resource::GetFile(const char *name)
{
	RESCHUNK *res=(RESCHUNK *)Find(name);
	if (!res) return NULL;
	CMemFile *ff=new CMemFile();
	if(!ff->Open(res->data,res->size_u)) {
		delete ff;
		return NULL;
	}
	ff->SetFilename(res->name);
	return ff;
}

void Resource::uncompress(void *resource)
{
	Compression comp;
	RESCHUNK *res=(RESCHUNK*)resource;
	size_t bufferlen=res->size_u;
	void *buffer=malloc(bufferlen);
	if (!buffer) {
		SetError(2);
		return 0;
	}

	if (comp.Uncompress(buffer, &bufferlen,res->uncompressed,res->size_c,
		(CCompression::Algorithm)res->compression)) {
		res->data=buffer;
		res->clearbuffer=1;
		return 1;
	}
	free(buffer);
	return 0;

}

/*********************************************************************************
 * Resourcen generieren
 *********************************************************************************/

static void IncludeHelp(CFileObject *out, const char *configfile)
{
	ppl6::Cppl6Core *core=ppl6::PPLInit();
	CString copyright=core->GetCopyright();
	CString version=core->GetVersion();
	CString now;
	MkISO8601Date(now);
	out->Putsf(
		"/*********************************************************\n"
		" * PPL6 Resourcen Generator Version %s\n"
		" * %s\n"
		"",(const char*)version,(const char*)copyright
	);

	out->Putsf(
		" *\n"
		" * File generation: %s\n"
		" * Config: %s\n"
		" *********************************************************/\n\n",
		(const char*)now, configfile
	);
	out->Putsf(
		"/* File-Format:\n"
		" *    Byte 0-5: ID \"PPLRES\"                              (6 Byte)\n"
		" *    Byte 6:   0-Byte                                   (1 Byte)\n"
		" *    Byte 7:   Major Version (6)                        (1 Byte)\n"
		" *    Byte 8:   Minor Version (0)                        (1 Byte)\n"
		" *    Byte 9:   Start of Chunks\n"
		" *\n"
		" * Chunk-Format:\n"
		" *    Byte 0:  Size of Chunk incl. Header                (4 Byte)\n"
		" *    Byte 4:  ID of Resource                            (2 Byte)\n"
		" *    Byte 6:  Size of data uncompressed                 (4 Byte)\n"
		" *    Byte 10: Size of data compressed                   (4 Byte)\n"
		" *    Byte 14: Compression-Type (0=none,1=zlib,2=bzip2)  (1 Byte)\n"
		" *    Byte 15: Offset of Data from Chunk-Start           (1 Byte)\n"
		" *    Byte 16: Name of Resource with 0-Byte              (n Bytes)\n"
		" *    Byte n:  Data\n"
		" *\n"
		" * Chunks are repeated as often as required, followed by\n"
		" * a 4 Byte 0-Value (Chunk size 0), which marks the end of file\n"
		" */\n\n"
		);
}

static void BufferOut(CFileObject *out, const char *buffer, int bytes)
{
	static int c=0;
	static char clear[25]="";
	CString utf8;
	for (int i=0;i<bytes;i++) {
		out->Putsf("0x%02x,",(ppldb)buffer[i]);
		if(buffer[i]>31 && buffer[i]!='\\' && buffer[i]!='/')clear[c]=buffer[i];
		else clear[c]='.';
		c++;
		clear[c]=0;
		if (c>15) {
			c=0;
			utf8=clear;
			utf8.Transcode("ISO-8859-1","UTF-8");
			out->Putsf(" // %s\n    ",(const char*)utf8);
			clear[0]=0;
		}
	}
	if (!bytes) {
		for (int i=c;i<16;i++) {
			out->Puts("     ");
		}
		c=0;
		utf8=clear;
		utf8.Transcode("ISO-8859-1","UTF-8");
		out->Putsf(" // %s\n    ",(const char*)utf8);
		clear[0]=0;
	}
}

static void Output(CFileObject *ff, int resid, const char *name, const char *filename, int size_u, char *buffer, int bytes, int compressiontype)
{
	char *buf=(char*)malloc(64);
	if (!buf) return;
	ppluint32 chunksize=bytes+strlen(name)+17;
	poke32(buf+0,chunksize);
	poke16(buf+4,resid);
	poke32(buf+6,size_u);
	poke32(buf+10,bytes);
	poke8(buf+14,compressiontype);
	poke8(buf+15,17+strlen(name));
	BufferOut(ff,buf,16);
	BufferOut(ff,name,strlen(name)+1);
	//BufferOut(ff,filename,strlen(filename)+1);
	BufferOut(ff,buffer,bytes);
	free(buf);
}

static int Compress(CFileObject *ff, char **buffer, ppluint32 *size, int *type)
{
	CCompression comp;
	ppluint32 size_u=(ppluint32)ff->Size();
	char *source=(char*)ff->Map();			// Komplette Datei mappen

	size_t size_c=size_u+2048;
	size_t size_zlib=size_c;
	size_t size_bz2=size_c;
	char *buf=(char*)malloc(size_c);
	if (!buf) return 0;
	// Zuerst Zlib
	comp.Init(CCompression::Algo_ZLIB,CCompression::Level_High);
	comp.Compress(buf,&size_zlib,source,size_u);
	// Dann Bzip2
	comp.Init(CCompression::Algo_BZIP2,CCompression::Level_High);
	comp.Compress(buf,&size_bz2,source,size_u);
	// Was war kleiner?
	if (size_u<=size_bz2 && size_u<=size_zlib) {
		free(buf);
		printf ("Using no compression: %u Bytes (Zlib: %u, BZ2: %u)\n",(ppluint32)size_u, (ppluint32)size_zlib, (ppluint32)size_bz2);
		*size=size_u;
		*type=0;
		*buffer=(char*)malloc(size_u);
		memcpy(*buffer,source,size_u);
		return 1;
	}
	if (size_bz2<size_zlib) {
		printf ("Using bzip2: %u Bytes von %u Bytes (zlib: %u Bytes)\n",(ppluint32)size_bz2,(ppluint32)size_u,(ppluint32)size_zlib);
		*buffer=buf;
		*size=size_bz2;
		*type=2;
		return 1;
	}
	size_zlib=size_c;
	comp.Init(CCompression::Algo_ZLIB,CCompression::Level_High);
	comp.Compress(buf,&size_zlib,source,size_u);
	printf ("Using zlib: %u Bytes von %u Bytes (bzip2: %u Bytes)\n",(ppluint32)size_zlib,(ppluint32)size_u,(ppluint32)size_bz2);
	*buffer=buf;
	*size=size_zlib;
	*type=1;
	return 1;
}

int Resource::GenerateResourceHeader(const char *basispfad, const char *configfile, const char *targetfile, const char *label)
{
	char section[12];
	if ((!configfile) || strlen(configfile)==0) {
		SetError(194,"configfile ist NULL oder leer");
		return 0;
	}
	CConfig conf;
	if (!conf.Load(configfile)) {
		return 0;
	}
	if (conf.SelectSection("setup")) {
		if (!basispfad) basispfad=conf.Get("path");
		if (!targetfile) targetfile=conf.Get("targetfile");
		if (!label) label=conf.Get("label");
	}

	if ((!basispfad) || strlen(basispfad)==0) {
		SetError(194,"basispfad ist NULL oder leer");
		return 0;
	}
	if ((!targetfile) || strlen(targetfile)==0) {
		SetError(194,"targetfile ist NULL oder leer");
		return 0;
	}

	if ((!label) || strlen(label)==0) {
		SetError(194,"label ist NULL oder leer");
		return 0;
	}
	const char *prefix=conf.GetSection("prefix");
	const char *suffix=conf.GetSection("suffix");
	const char *path=basispfad;
	CFile out;
	if (!out.Open(targetfile,"wb")) {
		return 0;
	}
	printf ("Verarbeite Resourcen in: %s...\n",configfile);
	if (prefix) out.Write((void*)prefix,strlen(prefix));
	IncludeHelp(&out, configfile);

	out.Puts("/**************************************************************\n");
	out.Puts(" * Resourcen:\n");
	out.Puts(" *\n");
	int havesection=conf.FirstSection();
	while(havesection) {
		sprintf(section,"%s",conf.GetSectionName());
		const char *id=conf.Get("ID");
		const char *name=conf.Get("Name");
		const char *filename=conf.Get("File");
		havesection=conf.NextSection();
		if (!filename) continue;
		if (strlen(filename)<2) continue;
		if (!id) continue;
		if (!name) continue;
		CFile ff;
		if (ff.Open("%s/%s","rb",path,filename)) {
			out.Putsf(" * %4i: %-20s (%s)\n",atoi(id),name,filename);
		}
	}
	out.Puts(" **************************************************************/\n\n");
	out.Putsf("static unsigned char %s []={\n    ",label);
	sprintf(section,"PPLRES");
	poke8(section+7,6);
	poke8(section+8,0);
	BufferOut(&out,section,9);

	havesection=conf.FirstSection();
	while(havesection) {
		sprintf(section,"%s",conf.GetSectionName());
		const char *id=conf.Get("ID");
		const char *name=conf.Get("Name");
		const char *filename=conf.Get("File");
		const char *compression=conf.Get("compression");
		havesection=conf.NextSection();
		if (!filename) continue;
		if (strlen(filename)<2) continue;
		if (!id) continue;
		if (!name) continue;
		CFile ff;
		if (!ff.Open("%s/%s","rb",path,filename)) {
			printf ("Konnte Resource %s nicht oeffnen\n",filename);
		} else {
			char *buffer=NULL;
			ppluint32 size=0;
			int type=0;
			printf ("%s: ",filename);
			if (compression) {
				CString forcecomp=LCase(Trim(compression));
				if (forcecomp=="none") {
					buffer=ff.Load();
					printf ("Forced no compression: %u Bytes\n",(ppluint32)ff.Size());
					Output(&out,atoi(id),name,filename,(ppluint32)ff.Size(),buffer,(ppluint32)ff.Size(),0);
					free(buffer);
					continue;
				}
				printf ("Unbekannter Kompressionsalgorithmus für ID %s: >>%s<<\n",id,(const char*)forcecomp);
				return 0;
			}
			if (Compress(&ff,&buffer,&size,&type)) {
				Output(&out,atoi(id),name,filename,(ppluint32)ff.Size(),buffer,size,type);
				free(buffer);
			}
		}
	}
	poke32(section,0);
	BufferOut(&out,section,4);
	BufferOut(&out,NULL,0);
	out.Puts("0\n};\n");
	if (suffix) out.Write((void*)suffix,strlen(suffix));
	return 1;
}

Resource *Resource::getPPLResource()
{
	return ppl6::GetPPLResource();
}


}	// EOF namespace ppl7
