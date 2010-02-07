/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: patrick $
 * $Revision: 1.14 $
 * $Date: 2009/08/24 09:10:05 $
 * $Id: CFilter_PPM.cpp,v 1.14 2009/08/24 09:10:05 patrick Exp $
 *
 *******************************************************************************
 * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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
#include "ppl6.h"
#include "ppl6-grafix.h"


namespace ppl6 {
namespace grafix {

/*!\class CFilter_PPM
 * \ingroup PPLGroupGrafik
 * \brief Import-/Export-Filter für PPM-Dateien
 */


CFilter_PPM::CFilter_PPM()
{
	//SaveAsASCII=false;
}

CFilter_PPM::~CFilter_PPM()
{
}


int CFilter_PPM::Ident(CFileObject *file, IMAGE *img)
{
	CTok tok;
	char tmpc[65], *tmp;
	if (file==NULL) { SetError(136); return false;}
	file->Seek((ppldd)0);
	file->Gets(tmpc,64);
	if (strncmp(tmpc,"P6\n",3)==0) {			// Ja, ist ein PPM
		file->Gets(tmpc,64);			// Breite und Hoehe holen
		tmp=trim(tmpc);					// Es koennte sein, dass hier Kommentare drin sind
		while (tmp[0]=='#') {
			file->Gets(tmpc,64);
			tmp=trim(tmpc);
		}
		img->width=atol(tok.GetFirst(tmpc," "));
		img->height=atol(tok.GetNext());
		//file->Gets(tmpc,64);			// Farbtiefe holen, normalerweise 255
		img->bitdepth=24;
		img->colors=0x1000000;
		img->pitch=3*img->width;
		return true;
	}
	SetError(1018);
	return false;
}

int CFilter_PPM::Load(CFileObject * file, CSurface *surface, IMAGE *img)
{
	CTok tok;
	ppldd	 farbtiefe,pp;
	const char *adresse,*tmp;
	char tmpc[64];
	int x,y;
	ppldb r,g,b;

	if (file==NULL || surface==NULL ||img==NULL) {
		SetError(136);
		return false;
	}

	file->Seek((ppldd)0);
	file->Gets(tmpc,64);
	if (strncmp(tmpc,"P6\n",3)==0) {			// Ja, ist ein PPM
		file->Gets(tmpc,64);			// Breite und Hoehe holen
		tmp=trim(tmpc);					// Es koennte sein, dass hier Kommentare drin sind
		while (tmp[0]=='#') {
			file->Gets(tmpc,64);
			tmp=trim(tmpc);
		}
		file->Gets(tmpc,64);			// Farbtiefe holen, normalerweise 255
		farbtiefe=atol(tmpc);
		pp=(ppldd)file->Ftell();

		if (surface->Lock()) {
			for (y=0;y<surface->GetHeight();y++) {
				adresse=file->Map(pp,img->pitch);	// Zeile fuer Zeile einlesen
				pp+=img->pitch;
				for (x=0;x<surface->GetWidth();x++) {
					r=adresse[x*3];
					g=adresse[x*3+1];
					b=adresse[x*3+2];
					surface->PutPixel(x,y,surface->RGB(r,g,b));
				}
			}
			surface->Unlock();
			SetError(0);
		} else {
			SetError(145);
		}
		if (GetErrorCode()==0) return true;
		return false;
	}
	SetError(88);
	return false;
}

int CFilter_PPM::Save (CSurface * surface, CFileObject * file, RECT *area, CAssocArray *param)
{
	int x,y;
	ppldd farbe;
	int haupt,unter,build;
	ppldb r,g,b;
	ppldd c;
	bool SaveAsASCII=false;


	if (surface==NULL || file==NULL) {
		SetError(136);
		return false;
	}
	if (param) {
		const char *tmp;
		if ((tmp=param->Get("ascii"))) SaveAsASCII=(bool)IsTrue(tmp);
	}


	if (surface->GetBitdepth()>8) {
		if (SaveAsASCII==false) file->Puts("P6\n");
		else file->Puts("P3\n");
		Cppl6Core *core=PPLCore;
		if (!core) core=new Cppl6Core;
		core->GetVersion (&haupt, &unter, &build);
		file->Putsf("# Generated by PPL %u.%u.%u\n",haupt,unter,build);
		file->Putsf("# %s\n",(const char*)core->GetCopyright());
		file->Putsf("%d %d\n",surface->GetWidth(),surface->GetHeight());
		file->Putsf("%d\n",255);
		if (core!=PPLCore) delete core;
		if (surface->Lock()) {
			c=0;
			for (y=0;y<surface->GetHeight();y++) {
				for (x=0;x<surface->GetWidth();x++) {
					farbe=surface->Surface2RGB(surface->GetPixel(x,y));
					r=(ppldb)(farbe&255);
					g=(ppldb)((farbe>>8)&255);
					b=(ppldb)((farbe>>16)&255);
					if (SaveAsASCII==false) {
						file->Write((char*)&r,1);
						file->Write((char*)&g,1);
						file->Write((char*)&b,1);
					} else {
						file->Putsf("%u %u %u ",r,g,b);
						c++;
						if (c>7) {
							file->Puts("\n");
							c=0;
						}
					}
				}
			}
			if (SaveAsASCII==true) {
				file->Puts("\n");
			}

			surface->Unlock();
			SetError(0);
		} else {
			SetError(145);
		}
	} else {			// Farbpalette wurde benutzt
		SetError(88);
	}
	if (GetErrorCode()==0) return true;
	return false;
}

const char *CFilter_PPM::GetName()
{
	return "PPM";
}
const char *CFilter_PPM::GetDescription()
{
	return "PPM";
}


} // EOF namespace grafix
} // EOF namespace ppl6
