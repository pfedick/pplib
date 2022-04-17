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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <strings.h>
#include <ctype.h>
#endif

#include "ppl6.h"

namespace ppl6 {

static const char * identtxt[] = {
	"unknown",							//  0
	"PCX Version 3",					//  1
	"GIF 89a",							//  2
	"GIF 87a",							//  3
	"BMP RGB-encoded",					//  4
	"BMP RLE-encoded",					//  5
	"PFP-Sprite Version 2.0",			//  6
	"PFP-Sprite Version 3.0",			//  7
	"PFP-Sprite Version 4.x",			//  8
	"PFP-Font Version 3.0",				//  9
	"PFP-Font Version 4.x",				// 10
	"PFP-Cursor Version 1.0",			// 11
	"PFP-Cursor Version 2.0",			// 12
	"PFP-Cursor Version 3.0",			// 13
	"PFP-Picture Version 1.0",			// 14
	"PFP-Picture Version 2.0",			// 15
	"PFP-Picture Version 3.0",			// 16
	"PFP-Picture Version 4.0",			// 17
	"PFP-Pattern Version 1.0",			// 18
	"PFP-Sprite Version 1.0",			// 19
	"PFP-Pattern Version 2.0",			// 20
	"JASC Farbpalette Version 1.0",		// 21
	"PFP-Cursor Version 4.0",			// 22
	"PFP-Sprite Version 5.x",			// 23
	"Language Sprachdatei Version 2.0",	// 24
	"Language Sprachdatei Version 3.0",	// 25
	"PAR-Archiv Version 2.0",			// 26
	"WAVE",								// 27
	"MPEG File",						// 28
	"MPEG 1, Audio Layer 3 File",		// 29
	"JPEG JFIF-Datei",					// 30
	"PFP-Font Version 5.0",				// 31
	"DNS File",							// 32
	"PFP-Cursor Version 4.1",			// 33
	"WinMusik WMT-File",				// 34
	"Language Sprachdatei Version 5.0",	// 35
	"PFPFile Version 3",				// 36
	"unknown"

};

const char * identstring (int id)
{
	int max=sizeof(identtxt) / sizeof(char *)-2;
	if (id<=max) return identtxt[id];
	return identtxt[0];
}

int is_pfp (const char * adresse, PFPSTRUCT *pfs)
{
	char ispfp=0;
	if (adresse==NULL) {
		//err=37;
		return (0);
	}
	ispfp=0;
	if (strncmp(adresse+4,"File",4)==0) {
		if (strncmp(adresse,"PFS-",4)==0) {
			ispfp=1;
		} else if (strncmp(adresse,"PFP-",4)==0) {
			ispfp=2;
			if (peek8(adresse+8)==3) ispfp=3;
		}

	}
	//err=0;
	if (ispfp==1) {
		if (pfs!=NULL) {			// Header fuer PFP-File 1.0 fuellen
			memcpy ((char *)pfs->header,"PFS-File",8);
			pfs->header_version=1;
			pfs->header_length=30;
			pfs->file_id[3]=' ';
			memcpy ((char *)pfs->file_id,(char *) adresse+10,3);
			pfs->subversion=(ppldb) peek8(adresse+9);
			pfs->mainversion=(ppldb) peek8(adresse+8);
			pfs->offset_name=(ppldw) peek16(adresse+13);
			pfs->offset_autor=(ppldw) peek16(adresse+13);
			pfs->date_creation=(ppldd) peek32(adresse+15);
			pfs->time_creation=(ppldd) peekt(adresse+19);
			pfs->date_update=(ppldd) peek32(adresse+22);
			pfs->time_update=(ppldd) peekt(adresse+26);
			pfs->compression=(ppldb) peek8(adresse+29);
		}
		return (1);
	} else if (ispfp==2) {
		if (pfs!=NULL) {			// Header fuer PFP-File 2.0 fuellen
			memcpy ((char *)pfs->header,"PFP-File",8);
			pfs->header_version=2;
			pfs->header_length=(ppldb) peek8(adresse+9);
			memcpy ((char *)pfs->file_id, adresse+10,4);
			pfs->subversion=(ppldb) peek8(adresse+14);
			pfs->mainversion=(ppldb) peek8(adresse+15);
			pfs->offset_name=(ppldw) peek16(adresse+16);
			pfs->offset_autor=(ppldw) peek16(adresse+18);
			pfs->date_creation=(ppldd) peek32(adresse+20);
			pfs->time_creation=(ppldd) peek32(adresse+24);
			pfs->date_update=(ppldd) peek32(adresse+28);
			pfs->time_update=(ppldd) peek32(adresse+32);
			pfs->compression=(ppldb) peek16(adresse+36);
		}
		return (1);
	} else if (ispfp==3) {
		if (pfs!=NULL) {			// Header fuer PFP-File 2.0 fuellen
			memcpy ((char *)pfs->header,"PFP-File",8);
			pfs->header_version=3;
			pfs->header_length=(ppldb) peek8(adresse+9);
			memcpy ((char *)pfs->file_id, adresse+10,4);
			pfs->subversion=(ppldb) peek8(adresse+14);
			pfs->mainversion=(ppldb) peek8(adresse+15);
			pfs->offset_name=0;
			pfs->offset_autor=0;
			pfs->date_creation=0;
			pfs->time_creation=0;
			pfs->date_update=0;
			pfs->time_update=0;
			pfs->compression=(ppldb) peek8(adresse+16);
		}
		return (1);
	} else {
		//err=16;
		return (0);
	}

}

int ident (const char * adresse, PFPSTRUCT * pfp)
{
	PFPSTRUCT * pfs,pf;
	//MPEG mpg;

	if (adresse==NULL) {
		//err=37;
		return (0);
	}
	if (pfp!=NULL) pfs=pfp;
	else pfs=&pf;

	if (is_pfp(adresse,pfs)==TRUE) {
		/*
		 * Okay, wir haben ein PFS-File, finden wir heraus, was fuer eins...
		 */
		if (pfs->header_version==1) {
			if (strncmp((char *)pfs->file_id,"SPR",3)==0) {		// Sprite?
				if (pfs->mainversion==1) return (19);	//		Version 1.0
				if (pfs->mainversion==1) return (6);	//		Version 2.0
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"FNT",3)==0) {		// Font?
				if (pfs->mainversion==3) return (9);	//		Version 3.0
				return (0);
			}
			//err=108;
			return (0);

		} else if (pfs->header_version==2) {
			if (strncmp((char *)pfs->file_id,"SPRI",4)==0) {	// Sprite?
				if (pfs->mainversion==3) return (7);	//		Version 3.0
				if (pfs->mainversion==4) return (8);	//		Version 4.0
				if (pfs->mainversion==5) return (23);	//		Version 5.0
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"PICT",4)==0) {	// Picture?
				if (pfs->mainversion==4) return (17);	//		Version 4.0
				return(0);
			}
			if (strncmp((char *)pfs->file_id,"FONT",4)==0) {	// Font?
				if (pfs->mainversion==4) return (10);	//		Version 4.x
				if (pfs->mainversion==5) return (31);	//		Version 5.x
				return(0);
			}
			if (strncmp((char *)pfs->file_id,"PATT",4)==0) {	// Pattern?
				if (pfs->mainversion==1) return (18);	//		Version 1.0
				if (pfs->mainversion==2) return (20);	//		Version 2.0
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"MAUS",4)==0) {	// Cursor-Datei?
				if (pfs->mainversion==3) return (13);	//		Version 3.0
				if (pfs->mainversion==4 && pfs->subversion==0) return (22);	//		Version 4.0
				if (pfs->mainversion==4 && pfs->subversion==1) return (33);	//		Version 4.1
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"LANG",4)==0) {	// Sprachdatei?
				if (pfs->mainversion==2) return (24);	//		Version 2.0
				if (pfs->mainversion==3) return (25);	//		Version 3.0
				if (pfs->mainversion==5) return (35);	//		Version 5.0
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"PARC",4)==0) {	// PAR-Archiv?
				if (pfs->mainversion==2) return (26);	//		Version 2.0
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"DNS ",4)==0) {	// DNS-File?
				if (pfs->mainversion==1) return (32);	//		Version 2.0
				return (0);
			}
			if (strncmp((char *)pfs->file_id,"WM2i",4)==0) {	// WinMusik 2.0 WMT-File
				if (pfs->mainversion==2) return (34);	//		Version 2.0
				return (0);
			}
			//err=108;
			return (0);
		} else if (pfs->header_version==3) {
			return(36);
		}

	} else {
		/*
		 * Kein PFS-File. Hmm, was koennte es denn sonst sein?...
		 */
		//err=0;
		if (strncmp(adresse,"JASC-PAL",8)==0) {				// JASC-Palettenfile?
			return (21);
		}

		if (peek16(adresse)==0x050a) return (1);		// PCX?
		if (strncmp ((char *)adresse, "GI",2)==0) {					// GIF?
			if (strncmp((char *)adresse+2,"F89a",4)==0) return (2);	// GIF89a
			if (strncmp((char *)adresse+2,"F87a",4)==0) return (3);	// GIF87a
		}
		if (strncmp ((char *)adresse, "BM",2)==0) {					// BMP?
			return (4);
		}
		if (strncmp ((char *)adresse,"RIFF",4)==0) {		// Es ist eine RIFF-Datei
			if (peek32(adresse+4)>0) {
				if (strncmp (adresse+8,"WAVE",4)==0) {		// Es ist sogar eine WAVE-Datei
					return (27);
				}
			}
		}
		if (peek32(adresse)==0xe0ffd8ff && peek16(adresse+4)==0x1000 ){
			if (strncmp ((char *)adresse+6,"JFIF",4)==0) {		// Es ist eine JPEG-Datei
				return (30);
			}
		}

		/*
		if (IsMPEG(adresse,64,&mpg)==true) {
			if (mpg.version==0 && mpg.lay==3) return (29);
			return 28;
		}
		*/
	}
	//err=20;
	return (0);

}

int ident (CFileObject &ff, PFPSTRUCT * pfp)
{
	const char *adresse;
	adresse=ff.Map(0,256);
	if (!adresse) return 0;
	int id=ident(adresse,pfp);
	if (id) return id;
	return 0;
}

} // end of namespace ppl6

