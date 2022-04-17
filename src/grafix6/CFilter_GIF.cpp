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

/*!\class CFilter_GIF
 * \ingroup PPLGroupGrafik
 * \brief Import-Filter für GIF-Dateien
 *
 * Dieser Import Filter erlaubt das Laden von GIF-Dateien. Der Code basiert
 * auf einer Library von David Koblas.
 *
 * \code
+-------------------------------------------------------------------+
| Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    |
|   Permission to use, copy, modify, and distribute this software   |
|   and its documentation for any purpose and without fee is hereby |
|   granted, provided that the above copyright notice appear in all |
|   copies and that both that copyright notice and this permission  |
|   notice appear in supporting documentation.  This software is    |
|   provided "as is" without express or implied warranty.           |
+-------------------------------------------------------------------+
 * \endcode
 */

#define        MAXCOLORMAPSIZE         256

#define        TRUE    1
#define        FALSE   0

#define CM_RED         0
#define CM_GREEN       1
#define CM_BLUE                2

#define        MAX_LWZ_BITS            12

#define INTERLACE              0x40
#define LOCALCOLORMAP  0x80
#define BitSet(byte, bit)      (((byte) & (bit)) == (bit))

//#define        ReadOK(file,buffer,len) (fread(buffer, len, 1, file) != 0)
#define        ReadOK(file,buffer,len) (file->Read((char *)buffer, len) != 0)

#define LM_to_uint(a,b)                        (((b)<<8)|(a))

/* We may eventually want to use this information, but def it out for now */
#if 0
static struct {
       unsigned int    Width;
       unsigned int    Height;
       unsigned char   ColorMap[3][MAXCOLORMAPSIZE];
       unsigned int    BitPixel;
       unsigned int    ColorResolution;
       unsigned int    Background;
       unsigned int    AspectRatio;
} GifScreen;
#endif

static struct {
       int     transparent;
       int     delayTime;
       int     inputFlag;
       int     disposal;
} Gif89 = { -1, -1, -1, 0 };

static int ReadColorMap (CFileObject *fd, int number, unsigned char (*buffer)[256]);
static int DoExtension (CFileObject *fd, int label, int *Transparent);
static int GetDataBlock (CFileObject *fd, unsigned char *buf);
static int GetCode (CFileObject *fd, int code_size, int flag);
static int LWZReadByte (CFileObject *fd, int flag, int input_code_size);
//static void ReadImage (gdImagePtr im, FILE *fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore);
static void ReadImage (CDrawable &surface, CFileObject *fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore);

static int ZeroDataBlock;


static int ReadColorMap(CFileObject *fd, int number, unsigned char (*buffer)[256])
{
       int             i;
       unsigned char   rgb[3];


       for (i = 0; i < number; ++i) {
               if (! ReadOK(fd, rgb, sizeof(rgb))) {
                       return TRUE;
               }
               buffer[CM_RED][i] = rgb[0] ;
               buffer[CM_GREEN][i] = rgb[1] ;
               buffer[CM_BLUE][i] = rgb[2] ;
       }


       return FALSE;
}

static int DoExtension(CFileObject *fd, int label, int *Transparent)
{
       static unsigned char     buf[256];

       switch (label) {
       case 0xf9:              /* Graphic Control Extension */
               (void) GetDataBlock(fd, (unsigned char*) buf);
               Gif89.disposal    = (buf[0] >> 2) & 0x7;
               Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
               Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
               if ((buf[0] & 0x1) != 0)
                       *Transparent = buf[3];

               while (GetDataBlock(fd, (unsigned char*) buf) != 0)
                       ;
               return FALSE;
       default:
               break;
       }
       while (GetDataBlock(fd, (unsigned char*) buf) != 0)
               ;

       return FALSE;
}


static int GetDataBlock(CFileObject *fd, unsigned char *buf)
{
       unsigned char   count;

       if (! ReadOK(fd,&count,1)) {
               return -1;
       }

       ZeroDataBlock = count == 0;

       if ((count != 0) && (! ReadOK(fd, buf, count))) {
               return -1;
       }

       return count;
}

static int GetCode(CFileObject *fd, int code_size, int flag)
{
       static unsigned char    buf[280];
       static int              curbit, lastbit, done, last_byte;
       int                     i, j, ret;
       unsigned char           count;

       if (flag) {
               curbit = 0;
               lastbit = 0;
               done = FALSE;
               return 0;
       }

       if ( (curbit+code_size) >= lastbit) {
               if (done) {
                       if (curbit >= lastbit) {
                                /* Oh well */
                       }
                       return -1;
               }
               buf[0] = buf[last_byte-2];
               buf[1] = buf[last_byte-1];

               if ((count = GetDataBlock(fd, &buf[2])) == 0)
                       done = TRUE;

               last_byte = 2 + count;
               curbit = (curbit - lastbit) + 16;
               lastbit = (2+count)*8 ;
       }

       ret = 0;
       for (i = curbit, j = 0; j < code_size; ++i, ++j)
               ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;

       curbit += code_size;

       return ret;
}

static int LWZReadByte(CFileObject *fd, int flag, int input_code_size)
{
       static int      fresh = FALSE;
       int             code, incode;
       static int      code_size, set_code_size;
       static int      max_code, max_code_size;
       static int      firstcode, oldcode;
       static int      clear_code, end_code;
       static int      table[2][(1<< MAX_LWZ_BITS)];
       static int      stack[(1<<(MAX_LWZ_BITS))*2], *sp;
       int    i;

       if (flag) {
               set_code_size = input_code_size;
               code_size = set_code_size+1;
               clear_code = 1 << set_code_size ;
               end_code = clear_code + 1;
               max_code_size = 2*clear_code;
               max_code = clear_code+2;

               GetCode(fd, 0, TRUE);

               fresh = TRUE;

               for (i = 0; i < clear_code; ++i) {
                       table[0][i] = 0;
                       table[1][i] = i;
               }
               for (; i < (1<<MAX_LWZ_BITS); ++i)
                       table[0][i] = table[1][0] = 0;

               sp = stack;

               return 0;
       } else if (fresh) {
               fresh = FALSE;
               do {
                       firstcode = oldcode =
                               GetCode(fd, code_size, FALSE);
               } while (firstcode == clear_code);
               return firstcode;
       }

       if (sp > stack)
               return *--sp;

	   while ((code = GetCode(fd, code_size, FALSE)) >= 0) {
               if (code == clear_code) {
                       for (i = 0; i < clear_code; ++i) {
                               table[0][i] = 0;
                               table[1][i] = i;
                       }
                       for (; i < (1<<MAX_LWZ_BITS); ++i)
                               table[0][i] = table[1][i] = 0;
                       code_size = set_code_size+1;
                       max_code_size = 2*clear_code;
                       max_code = clear_code+2;
                       sp = stack;
                       firstcode = oldcode =
                                       GetCode(fd, code_size, FALSE);
                       return firstcode;
               } else if (code == end_code) {
                       int             count;
                       unsigned char   buf[260];

                       if (ZeroDataBlock)
                               return -2;

                       while ((count = GetDataBlock(fd, buf)) > 0)
                               ;

                       if (count != 0)
                       return -2;
               }

               incode = code;

               if (code >= max_code) {
                       *sp++ = firstcode;
                       code = oldcode;
               }

               while (code >= clear_code) {
                       *sp++ = table[1][code];
                       if (code == table[0][code]) {
                               /* Oh well */
                       }
                       code = table[0][code];
               }

               *sp++ = firstcode = table[1][code];

               if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
                       table[0][code] = oldcode;
                       table[1][code] = firstcode;
                       ++max_code;
                       if ((max_code >= max_code_size) &&
                               (max_code_size < (1<<MAX_LWZ_BITS))) {
                               max_code_size *= 2;
                               ++code_size;
                       }
               }

               oldcode = incode;

               if (sp > stack)
                       return *--sp;
       }
       return code;
}

static void ReadImage (CDrawable &surface, CFileObject *fd, int len, int height, unsigned char (*cmap)[256], int interlace, int ignore)
{
	unsigned char   c;
	int             v;
	int             xpos = 0, ypos = 0, pass = 0;

	/* Stash the color map into the image */
	/*
       for (i=0; (i<gdMaxColors); i++) {
               im->red[i] = cmap[CM_RED][i];
               im->green[i] = cmap[CM_GREEN][i];
               im->blue[i] = cmap[CM_BLUE][i];
               im->open[i] = 1;
       }
	 */
	/* Many (perhaps most) of these colors will remain marked open. */
	//im->colorsTotal = gdMaxColors;
	/*
	 **  Initialize the Compression routines
	 */
	if (! ReadOK(fd,&c,1)) {
		return;
	}
	if (LWZReadByte(fd, TRUE, c) < 0) {
		return;
	}

	/*
	 **  If this is an "uninteresting picture" ignore it.
	 */
	if (ignore) {
		while (LWZReadByte(fd, FALSE, c) >= 0)
			;
		return;
	}
	while ((v = LWZReadByte(fd,FALSE,c)) >= 0 ) {
		/* This how we recognize which colors are actually used. */
		/*
               if (im->open[v]) {
                       im->open[v] = 0;
               }
		 */
		surface.putPixel(xpos,ypos,Color(cmap[CM_RED][v],cmap[CM_GREEN][v],cmap[CM_BLUE][v]));
		//gdImageSetPixel(im, xpos, ypos, v);
		++xpos;
		if (xpos == len) {
			xpos = 0;
			if (interlace) {
				switch (pass) {
					case 0:
					case 1:
						ypos += 8; break;
					case 2:
						ypos += 4; break;
					case 3:
						ypos += 2; break;
				}

				if (ypos >= height) {
					++pass;
					switch (pass) {
						case 1:
							ypos = 4; break;
						case 2:
							ypos = 2; break;
						case 3:
							ypos = 1; break;
						default:
							goto fini;
					}
				}
			} else {
				++ypos;
			}
		}
		if (ypos >= height)
			break;
	}
	fini:
	if (LWZReadByte(fd,FALSE,c)>=0) {
		/* Ignore extra */
	}
}


CFilter_GIF::CFilter_GIF()
{
}

CFilter_GIF::~CFilter_GIF()
{
}

int CFilter_GIF::Ident(CFileObject &file, IMAGE &img)
{
	const char *address=file.Map(0,256);
	const char *lsdb;
	char b;
	if (address==NULL) {
		SetError(1018);
		return 0;
	}

	if (strncmp (address, "GIF",3)!=0) {		// GIF ?
		SetError(1018);
		return 0;
	}
	lsdb=address+6;

	img.width=peekw(lsdb);
	img.height=peekw(lsdb+2);
	b=(ppldb)peekb(lsdb+4);
	img.bitdepth=((b>>4)&7)+1;
	img.format=RGBFormat::Palette;
	img.colors=256;
	img.pitch=0;
	//img->pfp.header_version=0;
	if (strncmp(address+3,"89a",3)==0) {							// GIF89a
		return 1;
	}
	if (strncmp(address+3,"87a",3)==0) {							// GIF87a
		return 1;
	}
	SetError(1018);
	return 0;
}

int CFilter_GIF::Load(CFileObject &file, CDrawable &surface, IMAGE &img)
{
	int imageNumber;
	int BitPixel;
	int Transparent = (-1);
	unsigned char   buf[16];
	unsigned char   c;
	unsigned char   ColorMap[3][MAXCOLORMAPSIZE];
	unsigned char   localColorMap[3][MAXCOLORMAPSIZE];
	int             imw, imh;
	int             useGlobalColormap;
	int             bitPixel;
	int             imageCount = 0;
	char            version[4];
	ZeroDataBlock = FALSE;
	CFileObject *fd=&file;
	imageNumber = 1;
	file.Seek(0);

	if (!ReadOK(fd,buf,6)) {
		SetError(1021);
		return false;
	}
	if (strncmp((char *)buf,"GIF",3) != 0) {
		SetError(1021);
		return false;
	}
	strncpy(version, (char *)buf + 3, 3);
	version[3] = '\0';

	if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0)) {
		SetError(1021);
		return false;
	}
	if (! ReadOK(fd,buf,7)) {
		SetError(1021);
		return false;
	}
	BitPixel        = 2<<(buf[4]&0x07);
	/*
	int ColorResolution = (int) (((buf[4]&0x70)>>3)+1);
	int Background      = buf[5];
	int AspectRatio     = buf[6];
	*/

	if (BitSet(buf[4], LOCALCOLORMAP)) {    /* Global Colormap */
		if (ReadColorMap(fd, BitPixel, ColorMap)) {
			SetError(1021);
			return false;
		}
	}
	for (;;) {
		if (! ReadOK(fd,&c,1)) {
			SetError(1021);
			return false;
		}
		if (c == ';') {         /* GIF terminator */
			if (imageCount < imageNumber) {
				SetError(1021);
				return false;
			}
			/* Terminator before any image was declared! */
			/*
                       if (!im) {
                              return;
                       }
			 */
			/* Check for open colors at the end, so
                          we can reduce colorsTotal and ultimately
                          BitsPerPixel */
			/*
                       for (i=((im->colorsTotal-1)); (i>=0); i--) {
                               if (im->open[i]) {
                                       im->colorsTotal--;
                               } else {
                                       break;
                               }
                       }
			 */
			SetError(0);
			return 1;		// Erfolgreich geladen
		}

		if (c == '!') {         /* Extension */
			if (! ReadOK(fd,&c,1)) {
				SetError(1021);
				return false;
			}
			DoExtension(fd, c, &Transparent);
			continue;
		}

		if (c != ',') {         /* Not a valid start character */
			continue;
		}

		++imageCount;

		if (! ReadOK(fd,buf,9)) {
			SetError(1021);
			return false;
		}

		useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);

		bitPixel = 1<<((buf[8]&0x07)+1);

		imw = LM_to_uint(buf[4],buf[5]);
		imh = LM_to_uint(buf[6],buf[7]);
		/*
               if (!(im = gdImageCreate(imw, imh))) {
                         return;
               }
               im->interlace = BitSet(buf[8], INTERLACE);
		 */
		if (! useGlobalColormap) {
			if (ReadColorMap(fd, bitPixel, localColorMap)) {
				return 1;
			}
			ReadImage(surface, fd, imw, imh, localColorMap,
					BitSet(buf[8], INTERLACE),
					imageCount != imageNumber);
			if (Transparent != (-1)) {
				/*
				surface.SetColorKey(Color(localColorMap[CM_RED][Transparent],
						localColorMap[CM_GREEN][Transparent],
						localColorMap[CM_BLUE][Transparent]));
						*/
			}

		} else {
			ReadImage(surface, fd, imw, imh,
					ColorMap,
					BitSet(buf[8], INTERLACE),
					imageCount != imageNumber);
			if (Transparent != (-1)) {
				//surface.SetColorKey(surface->RGB(0,0,0));
			}
		}
	}
	return 1;
}

int CFilter_GIF::Save (const CDrawable &surface, CFileObject &file, CAssocArray *param)
{
	SetError(1020);
	return false;
}

CString CFilter_GIF::Name()
{
	return "GIF";
}

CString CFilter_GIF::Description()
{
	return "GIF";
}


} // EOF namespace grafix
} // EOF namespace ppl6

