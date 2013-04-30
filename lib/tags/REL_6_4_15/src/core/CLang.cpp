/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CLang.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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

#include <stdlib.h>
#include <string.h>
#include "ppl6.h"


#define MAX_VAL_LENGTH	256
#define MAX_VALUES		10

typedef struct {
	char *text;
} SPRUNGTABELLE;

namespace ppl6 {

CLang::CLang()
{
	header=NULL;
	data=NULL;
	table=NULL;
	values=(char*)malloc(MAX_VAL_LENGTH*MAX_VALUES);
	for (int i=0;i<MAX_VALUES;i++) val[i]=NULL;
}

CLang::~CLang()
{
	Unload();
	if (values) free(values);
}

void CLang::Unload()
{
	if (header) free(header);
	header=NULL;
	if (data) free(data);
	data=NULL;
	if (table) free(table);
	table=NULL;
	for (int i=0;i<10;i++) val[i]=NULL;
}


int CLang::Compile(char *lang, char *file)
{
	CFile src;
	CFile dst;
	PFPSTRUCT pfp;

	if (!src.Open(lang,"rb")) return 0;
	if (!dst.Open(file,"wb")) return 0;

	CConfig conf;
	conf.Load(&src);
	//conf.Print();

	if (!conf.SelectSection("PFP-Language-File")) {
		conf.Unload();
		src.Close();
		return 0;
	}



	const char *name=conf.Get("Name");
	const char *autor=conf.Get("Autor");
	const char *country=conf.Get("Country");
	const char *language=conf.Get("Language");
	const char *international=conf.Get("International");
	const char *choose=conf.Get("Choose");

	if (!name) name=(char*)"unknown";
	if (!autor) autor=(char*)"unknown";
	if (!country) country=(char*)"en,us,au";
	if (!language) language=(char*)"unknown";
	if (!international) international=(char*)"unknown";
	if (!choose) choose=(char*)"Please choose your language";

	if (!conf.SelectSection("Words")) {
		conf.Unload();
		src.Close();
		return 0;
	}
	long minid=0x10000;
	long maxid=0;
	long count=0;

	// Wir reservieren Speicher f�r die Worte
	ppldd buffersize=(ppldd)src.Size()*2;
	if (buffersize<8192) buffersize=8192;
	char *lbuffer=(char*)malloc(buffersize*2);
	if (!lbuffer) {
		conf.Unload();
		src.Close();
		return 0;
	}
	char *cbuffer=lbuffer+buffersize;		// Buffer f�r Komprimierung
	ppldd p=0;

	// Jetzt lesen wir s�mtliche Worte ein
	char *key, *value;
	int cont=conf.GetFirst(&key,&value);
	long id;
	ppldd len;
	while (cont) {
		id=atol(key);
		if (id<0xffff && id>=0 && strlen(key)>0) {
			count++;
			if (id>maxid) maxid=id;
			if (id<minid) minid=id;
			len=(ppldd)strlen(value);
			poke16(lbuffer+p,id);
			poke16(lbuffer+p+2,len);
			strcpy(lbuffer+p+4,value);
			p+=5+len;

		} else {

		}
		cont=conf.GetNext(&key,&value);
	}

	// Der letzte Eintrag mit 0 als Laenge
	poke16(lbuffer+p,0xffff);
	poke16(lbuffer+p+2,0);
	p+=4;

	poke32(cbuffer+4,p);

	// Daten komprimieren
	CCompression comp(CCompression::Algo_ZLIB, CCompression::Level_High);

	size_t complen=buffersize-9;
	//if (compress(cbuffer+9,&complen,lbuffer,p,PPLCOMP_ZLIB,PPLCOMPLEVEL_HIGH)) {
	if (!comp.Compress(cbuffer+9,&complen,lbuffer,p)) {
		conf.Unload();
		free(lbuffer);
		src.Close();
		return 0;
	}
	poke8(cbuffer+8,CCompression::Algo_ZLIB);
	poke32(cbuffer,complen);

	// Jetzt die fertige Datei zusammenbauen
	PresetPFPHeader(&pfp);
	strncpy((char*)pfp.file_id,"LANG",4);
	pfp.mainversion=5;
	pfp.subversion=0;


	poke32(lbuffer+2,count);
	poke32(lbuffer+6,maxid);

	p=18;
	pfp.offset_name=(ppldw)(40+p);
	strcpy(lbuffer+p,name);
	p+=(ppldd)strlen(name)+1;
	pfp.offset_autor=(ppldw)(40+p);
	strcpy(lbuffer+p,autor);
	p+=(ppldd)strlen(autor)+1;

	// Auf volle 2 Byte aufrufen
	if (p&1) p++;
	poke16(lbuffer+10,p);				// Country-Tabelle erzeugen
	const char *tmp;
	CTok tok;
	tmp=tok.GetFirst(country,",");
	if (!tmp) tmp=(char*)"en";
	while (tmp) {
		strncpy(lbuffer+p,tmp,2);
		p+=2;
		tmp=tok.GetNext();
	}
	poke16(lbuffer+p,0);
	p+=2;

	// Default-Texte schreiben
	poke16(lbuffer+12,p);
	strcpy(lbuffer+p,language);
	p+=(ppldd)strlen(language)+1;
	poke16(lbuffer+14,p);
	strcpy(lbuffer+p,international);
	p+=(ppldd)strlen(international)+1;
	poke16(lbuffer+16,p);
	strcpy(lbuffer+p,choose);
	p+=(ppldd)strlen(choose)+1;

	poke16(lbuffer+0,p);

	// Fertig, jetzt müssen wir nurnoch speichern
	SavePFPHeader(&dst,&pfp);
	dst.Write((void*)lbuffer,(size_t)p,(ppluint64)40);
	dst.Write(cbuffer,complen+9,p+40);

	dst.Close();
	free(lbuffer);
	src.Close();
	return 1;
}


int CLang::Load(const char *file)
{
	CFile ff;
	if (!ff.Open(file,"rb")) return 0;
	int ret=Load(ff);
	ff.Close();
	return ret;
}

int CLang::Load(CFileObject &file)
{
	PFPSTRUCT pfp;
	if (ident(file,&pfp)!=35) return 0;
	Unload();

	ppldd ptr_lh=pfp.header_length;
	// Wie gross ist der Header?
	ppldw ptr_db=0;
	file.Read((char*)&ptr_db,2,ptr_lh);
	header=(char*)malloc(ptr_db+10);
	if (!header) return 0;
	file.Read(header,ptr_db+10,ptr_lh);

	country=header+peek16(header+10);
	language=header+peek16(header+12);
	international=header+peek16(header+14);
	choose=header+peek16(header+16);
	count=peek32(header+2);
	maxid=peek32(header+6);

	CCompression comp;
	ppldd bytes_compressed, bytes_uncompressed;
	int comp_type;
	bytes_compressed=peek32(header+ptr_db);
	bytes_uncompressed=peek32(header+ptr_db+4);
	comp_type=peek8(header+ptr_db+8);

	if (!bytes_uncompressed) return 0;
	data=(char*)malloc(bytes_uncompressed);
	table=calloc(maxid+1,4);
	if (!data) return 0;
	if (!table) return 0;

	char *buffer=(char*)malloc(bytes_compressed);
	if (!buffer) {
		free (table);
		table=NULL;
		return 0;
	}

	if (file.Read(buffer,bytes_compressed,ptr_lh+ptr_db+9)<bytes_compressed) {
		free (buffer);
		free (table);
		table=NULL;
		return 0;
	}
	size_t bytes=bytes_uncompressed;
	//if (uncompress(data,&bytes,buffer,bytes_compressed,comp_type)) {
	if (comp.Uncompress(data,&bytes,buffer,bytes_compressed,(CCompression::Algorithm)comp_type)) {
		free (buffer);
		free (table);
		table=NULL;
		return 0;
	}
	free(buffer);

	// Nun liegen die W�rter unkomprimiert im Speicher ab data
	SPRUNGTABELLE *jump=(SPRUNGTABELLE*) table;
	ppldd p=0,id,len;

	for (ppldd i=0;i<count;i++) {
		id=peek16(data+p);
		len=peek16(data+p+2);
		if (id==0xffff) break;
		else {
			jump[id].text=data+4+p;
		}
		p+=5+len;
	}
	return 1;
}

bool CLang::HasCountry(char *cc)
{
	if (!table) return false;
	if (!country) return false;

	ppldw *stack=(ppldw*)country;
	ppldw val=((ppldw*)cc)[0];
	while (1) {
		if (stack[0]==val) return true;
		if (stack[0]==0) break;
		stack++;
	}
	return false;
}

const char *CLang::GetRaw(ppldd id)
{
	if (!table) return "Error";
	SPRUNGTABELLE *jump=(SPRUNGTABELLE*) table;
	char *e=jump[0].text;
	if (!e) e=(char*)"unknown";
	if (id>maxid) return e;
	if (!jump[id].text) return e;
	return jump[id].text;
}

void CLang::SetValue(ppldd id, ppldd value)
{
	char tmp[20];
	sprintf(tmp,"%u",value);
	SetValue(id,tmp);
}

void CLang::SetValue(ppldd id, float value, int dezimalstellen)
{
	char mask[20];
	char tmp[40];
	if (dezimalstellen>10) dezimalstellen=10;
	sprintf(mask,"%%s0.%u",dezimalstellen);
	sprintf(tmp,mask,value);
	SetValue(id,tmp);
}

void CLang::SetValue(ppldd id, char *value)
{
	if (id<1 || id>MAX_VALUES) return;
	if (!value) {
		val[id-1]=NULL;
	} else {
		val[id-1]=values+((id-1)*MAX_VAL_LENGTH);
		strncpy(val[id-1],value,MAX_VAL_LENGTH-1);
		val[id-1][MAX_VAL_LENGTH-1]=0;
	}
}

void CLang::SetReplace(char *str, char *replacement)
{
	replace.Add(str,replacement);
}

char *CLang::GetValue(ppldd id)
{
	if (id<1 || id>MAX_VALUES) return NULL;
	return val[id-1];
}

void CLang::Replace(char *alt, char *neu, char *buffer, ppldd size)
{
	int altlen=(int)strlen(alt);
	int neulen=(int)strlen(neu);
	int textlen=(int)strlen(buffer);
	int p,np,ap,diff;
	while (1) {
		p=instr(buffer,alt,0);
		if (p<0) break;
		// Nachfolgende Zeichen verschieben
		if (neulen<altlen) {			// Text nach links verschieben
			np=p+neulen;
			ap=p+altlen;
			while (buffer[ap]) {
				buffer[np]=buffer[ap];
				ap++;
				np++;
			}
			buffer[np]=0;
			textlen=textlen-(altlen-neulen);

		} else if (neulen>altlen) {		// Text nach rechts verschieben
			diff=neulen-altlen;
			np=textlen+diff+1;
			ap=textlen+1;
			diff=np-size;
			if (diff>0) {				// Passt nicht in den Buffer
				np-=diff;
				ap-=diff;
			}
			while (1) {
				buffer[np]=buffer[ap];
				ap--;
				np--;
				if (ap<1 || ap<p+altlen) break;
			}
			textlen=textlen+(neulen-altlen);
		}
		strncpy(buffer+p,neu,neulen);

	}

}

char *CLang::Get(ppldd id, char *buffer, ppldd buffersize)
{
	const char *text=GetRaw(id);
	// internen Buffer anlegen
	ppldd ibuffersize=(ppldd)strlen(text)*3;
	if (ibuffersize<2048) ibuffersize=2048;
	char *ibuffer=(char*)malloc(ibuffersize);
	if (!ibuffer) return NULL;
	strcpy(ibuffer,text);
	long p,e;
	ppldd valid;
	char *val;
	char alt[20];
	while (1) {		// Platzhalter austauschen
		p=instr(ibuffer,"%par",0);
		if (p<0) break;
		e=instr(ibuffer,"%",p+3);
		valid=atol(ibuffer+p+4);
		if (!e) break;		// Keine Endmarkierung
		sprintf(alt,"%%par%u%%",valid);
		val=GetValue(valid);
		if (!val) Replace(alt,(char*)"",ibuffer,ibuffersize);
		else Replace(alt,val,ibuffer,ibuffersize);
	}

	// Jetzt die Replacement-Characters
	char *repalt, *repneu;
	p=replace.GetFirst(&repalt,&repneu);
	while (p) {
		Replace(repalt,repneu,ibuffer,ibuffersize);
		p=replace.GetNext(&repalt,&repneu);
	}

	if (!buffer) return ibuffer;
	if (buffersize<1) return NULL;
	strncpy(buffer,ibuffer,buffersize);
	buffer[buffersize-1]=0;
	free(ibuffer);
	return buffer;
}

} // EOF Namespace ppl
