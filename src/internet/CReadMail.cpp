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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef _WIN32
#else
	#ifdef HAVE_NETDB_H
    #include <netdb.h>
	#endif
#endif

#include "ppl6.h"

namespace ppl6 {

/*!\class CReadMail
 * \ingroup PPLGroupInternet
 * \brief Klasse zum Lesen und Dekodieren von Emails
 *
 * \header \#include <ppl6.h>
 * \desc
 * Mit dieser Klasse können Emails eingelesen und Dekodiert werden. Anschließend kann man auf
 * die einzelnen Bestandteile des Headers und den Body zugreifen.
 *
 * \since Diese Klasse wurde mit Version 6.1.3 eingeführt.
 */

/*!\var CReadMail::HeaderLines
 * \brief Assoziatives Array mit den einzelnen Keyworten des Headers
 *
 * Dieses Array beinhaltet die einzelnen Keywörter des Mail-Headers. Dabei ist
 * sichergestellt, dass mehrzeilige Werte zu einer Zeile zusammengefasst sind. Bei Keywörtern,
 * die mehrfach im Header auftauchen (Received-Zeile) werden die einzelnen Vorkommen mit
 * Newline voneinander getrennt.
 *
 * Das komplette Array kann mit der Funktion CReadMail::GetHeaderArray ausgelesen/kopiert
 * werden, einzelne Elemente mit CReadMail::GetHeader(const char *key)
 */

/*!\var CReadMail::Mail
 * \brief String mit der kompletten unverarbeiteten Mail
 *
 * String mit der kompletten unverarbeiteten Mail. Der Inhalt kann über die GetMail-Funktionen
 * ausgelesen werden:
 * - const char *CReadMail::GetMail();
 * - int CReadMail::GetMail(CString *Buffer);
 */

/*!\var CReadMail::Header
 * \brief String mit dem kompletten unveränderten Header
 *
 * String mit dem kompletten unveränderten Header. Der Inhalt kann über die GetHeader-Funktionen
 * ausgelesen werden:
 * - const char *CReadMail::GetHeader();
 * - int CReadMail::GetHeader(CString *Buffer);
 *
 */

/*!\var CReadMail::Body
 * \brief String mit dem kompletten unveränderten Body
 *
 * String mit dem kompletten unveränderten Body. Der Inhalt kann über die GetBody-Funktionen
 * ausgelesen werden:
 * - const char *CReadMail::GetBody();
 * - int CReadMail::GetBody(CString *Buffer);
 *
 */

/*!\var CReadMail::Decoded
 * \brief String mit dem kompletten dekodierten Body
 *
 * String mit dem kompletten dekodierten Body. Eine Mail im Quoted-Printable oder Base64-Format
 * wurde hier bereits in lesbare Zeichen umgewandelt. Der Inhalt kann über die
 * GetDecodedBody-Funktionen ausgelesen werden:
 * - const char *CReadMail::GetDecodedBody();
 * - int CReadMail::GetDecodedBody(CString *Buffer);
 *
 */

/*!\var CReadMail::Transcoded
 * \brief String mit dem kompletten dekodierten und Transcodierten Body
 *
 * Dies ist ein Behelfs-String, der nur gefüllt wird, wenn die Funktion
 * CReadMail::GetTranscodedBody(CString *Buffer, const char *charset) ohne Buffer
 * aufgerufen wird. Der Body ist dabei bereits Dekodiert und in das angegebene Charset
 * umgewandelt.
 *
 */

/*!\var CReadMail::Loaded
 * \brief Flag was anzeigt, ob eine Mail erfolgreich geladen wurde
 */


CReadMail::CReadMail()
/*!\brief Konstruktor der Klasse
 */
{
	Loaded=false;
}

CReadMail::~CReadMail()
/*!\brief Destruktor der Klasse
 */
{
	Clear();
}

void CReadMail::Clear()
/*!\brief Inhalt der Klasse wird gelöscht
 *
 * Diese Funktion wird automatisch vom Destruktor und zu Beginn der Load-Funktion aufgerufen,
 * um den Inhalt der Klasse zu leeren und den allokierten Speicher freizugeben,
 */
{
	HeaderLines.Clear();
	Mail.Clear();
	Header.Clear();
	Body.Clear();
	Decoded.Clear();
	Transcoded.Clear();
	Loaded=false;
}

int CReadMail::LoadFile(const char *filename, ...)
/*!\brief Mail aus Datei laden
 */
{
	if (!filename) {
		SetError(194,"int CReadMail::LoadFile(==> const char *filename <==, ...)");
		return 0;
	}
	CString String;
	va_list args;
	va_start(args, filename);
	String.VaSprintf(filename,args);
	va_end(args);
	CFile ff;
	if (!ff.Open(String,"rb")) return 0;
	return LoadFile(&ff);
}

int CReadMail::LoadFile(CFileObject *file)
/*!\brief Mail aus Datei laden
 */
{
	if (!file) {
		SetError(194,"int CReadMail::LoadFile(==> CFile *file <==)");
		return 0;
	}
	const char *buffer=(const char*)file->Map();
	if (!buffer) return 0;
	return LoadString(buffer);
}

int CReadMail::LoadFile(CFileObject &file)
/*!\brief Mail aus Datei laden
 */
{
	const char *buffer=(const char*)file.Map();
	if (!buffer) return 0;
	return LoadString(buffer);
}

int CReadMail::LoadString(const char *string)
/*!\brief Mail aus String laden
 */
{
	if (!string) {
		SetError(194,"int CReadMail::LoadString(==> const char *string <==)");
		return 0;
	}
	Clear();
	Mail=string;
	// CRLF oder LFCR durch einfache LF ersetzen
	if (!RemoveCR()) return 0;
	// Mail in Header und Body aufteilen
	if (!SplitHeaderAndBody()) return 0;
	if (!ParseHeader()) return 0;
	if (!DecodeBody()) return 0;
	//printf ("Body:>>>\n%s<<<\n",(char*)Decoded);
	Loaded=true;
	return 1;
}

int CReadMail::LoadString(const CString &string)
/*!\brief Mail aus String laden
 */
{
	Clear();
	Mail=string;
	// CRLF oder LFCR durch einfache LF ersetzen
	if (!RemoveCR()) return 0;
	// Mail in Header und Body aufteilen
	if (!SplitHeaderAndBody()) return 0;
	if (!ParseHeader()) return 0;
	if (!DecodeBody()) return 0;
	//printf ("Body:>>>\n%s<<<\n",(char*)Decoded);
	Loaded=true;
	return 1;
}


int CReadMail::RemoveCR()
/*!\brief Eventuell vorhandene Carriage-Returns (CR) entfernen oder durch Newline (NL) ersetzen
 */
{
	Mail.Replace("\r\n","\n");
	Mail.Replace("\n\r","\n");
	Mail.Replace("\r","\n");
	return 1;
}

int CReadMail::SplitHeaderAndBody()
/*!\brief Mail in Header und Body zerlegen
 */
{
	// Wir suchen das erste Auftauchen von zwei Newlines hintereinander
	int p=Mail.Instr("\n\n");
	if (!p) {
		SetError(445);
		return 0;
	}
	Header=Mail.SubStr(0,p+1);
	Body=Mail.SubStr(p+2);
	//printf ("Header:>>>\n%s<<<\n",(char*)Header);
	//printf ("Body:>>>\n%s<<<\n",(char*)Body);
	return 1;
}

int CReadMail::ParseHeader()
/*!\brief Header der Mail in ein Assoziatives Array parsen
 */
{
	if (Header.Len()==0) {
		SetError(446);
		return 0;
	}
	CArray a;
	CString Key, Line;
	if (!a.Explode(&Header,"\n")) return 0;
	const char *line=a.GetFirst();
	CString Value;
	int trenn;
	while (line) {
		Line=line;
		line=a.GetNext();
		// Leere Zeilen werden ignoriert
		if (Line.Len()==0) continue;
		// Beginnt die Zeile mit "From ", wird diese überlesen (=Mbox-Header)
		if (Line.StrCmp("From ",5)==0) continue;
		trenn=Line.Instr(":");
		// Falls "line" mit Space beginnt, wird die Zeile an das letzte Keywort angehangen
		if (Line[0]==9 || Line[0]==32 || trenn<0) {
			Value=Line;
			Value.Trim();
			// Leere Zeilen werden ignoriert
			if (Value.Len()==0) continue;
			HeaderLines.Concat (Key, Value, " ");
		} else {
			Key=Line.SubStr(0,trenn);
			Key.Trim();
			//Key.LCase();
			Value=Line.SubStr(trenn+1);
			Value.Trim();
			// Wenn es das Keywort schon gibt, hängen wir den Wert mit Newline an
			HeaderLines.Concat (Key, Value, "\n");
		}
	}
	//HeaderLines.List("header");
	return 1;
}

int CReadMail::DecodeBody()
/*!\brief Body der Mail Dekodieren
 */
{
	if (Body.Len()==0) {
		SetError(445);
		return 0;
	}
	const char *enc=HeaderLines.Get("Content-Transfer-Encoding");
	if (!enc) enc="7bit";
	if (::strcasecmp(enc,"7bit")==0
		|| ::strcasecmp(enc,"8bit")==0) {
		// Es muss nichts gemacht werden
		Decoded=Body;
		return 1;
	}
	if (::strcasecmp(enc,"quoted-printable")==0) {
		return Decode_QuotedPrintable();
	}
	if (::strcasecmp(enc,"base64")==0) {
		printf ("Debug 2\n");
		//return Decode_Base64();
	}
	SetError(447,"Content-Transfer-Encoding: %s",enc);
	return 0;
}

int CReadMail::Decode_QuotedPrintable()
/*!\brief Body im "Quoted Printable"-Format dekodieren
 */
{
	const unsigned char *str=(const unsigned char *)Body.GetPtr();
	size_t length=(size_t)Body.Len();
    unsigned int i;
    unsigned const char *p1;
    unsigned char *p2;
    unsigned int h_nbl, l_nbl;

    size_t decoded_len, buf_size;
    unsigned char *retval;

    static unsigned int hexval_tbl[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 32, 16, 64, 64, 16, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        32, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64, 64, 64, 64, 64, 64,
        64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    i = (int)length, p1 = str; buf_size = length;

    while (i > 1 && *p1 != '\0') {
        if (*p1 == '=') {
            buf_size -= 2;
            p1++;
            i--;
        }
        p1++;
        i--;
    }

    retval = (unsigned char*)malloc(buf_size + 1);
    if (!retval) {
    	SetError(2);
    	return 0;
    }
    i = (int)length; p1 = str; p2 = retval;
    decoded_len = 0;

    while (i > 0 && *p1 != '\0') {
        if (*p1 == '=') {
            i--, p1++;
            if (i == 0 || *p1 == '\0') {
                break;
            }
            h_nbl = hexval_tbl[*p1];
            if (h_nbl < 16) {
                /* next char should be a hexadecimal digit */
                if ((--i) == 0 || (l_nbl = hexval_tbl[*(++p1)]) >= 16) {
                    free(retval);
                    SetError(448,"Quoted Printable");
                    return 0;
                }
                *(p2++) = (h_nbl << 4) | l_nbl, decoded_len++;
                i--, p1++;
            } else if (h_nbl < 64) {
                /* soft line break */
                while (h_nbl == 32) {
                    if (--i == 0 || (h_nbl = hexval_tbl[*(++p1)]) == 64) {
                        free(retval);
                        SetError(448,"Quoted Printable");
                        return 0;
                    }
                }
                if (p1[0] == '\r' && i >= 2 && p1[1] == '\n') {
                    i--, p1++;
                }
                i--, p1++;
            } else {
                free(retval);
                SetError(448,"Quoted Printable");
                return 0;
            }
        } else {
            //*(p2++) = (replace_us_by_ws == *p1 ? '\x20': *p1);
			*(p2++)=*p1;
            i--, p1++, decoded_len++;
        }
    }
    *p2 = '\0';
    Decoded=(char*)retval;
    free(retval);
    return 1;
}

int CReadMail::Decode_Base64()
/*!\brief Body im "Base64"-Format dekodieren
 */
{
	/*
	static const char base64_table[] =
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
			'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
			'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
	};
	*/
	static const char base64_pad = '=';

	static const short base64_reverse_table[256] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
			-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
			-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};
	int length=Body.Len();
	const unsigned char *current = (const unsigned char *)Body.GetPtr();
	int ch, i = 0, j = 0, k;
    /* this sucks for threaded environments */
    unsigned char *result;

    result = (unsigned char *)malloc(length + 1);
    if (result == NULL) {
    	SetError(2);
        return 0;
    }

    /* run through the whole string, converting as we go */
    while ((ch = *current++) != '\0' && length-- > 0) {
        if (ch == base64_pad) break;

        /* When Base64 gets POSTed, all pluses are interpreted as spaces.
           This line changes them back.  It's not exactly the Base64 spec,
           but it is completely compatible with it (the spec says that
           spaces are invalid).  This will also save many people considerable
           headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>
        */

        if (ch == ' ') ch = '+';

        ch = base64_reverse_table[ch];
        if (ch < 0) continue;

        switch(i % 4) {
        case 0:
            result[j] = ch << 2;
            break;
        case 1:
            result[j++] |= ch >> 4;
            result[j] = (ch & 0x0f) << 4;
            break;
        case 2:
            result[j++] |= ch >>2;
            result[j] = (ch & 0x03) << 6;
            break;
        case 3:
            result[j++] |= ch;
            break;
        }
        i++;
    }

    k = j;
    /* mop things up if we ended on a boundary */
    if (ch == base64_pad) {
        switch(i % 4) {
        case 1:
            free(result);
            SetError(448,"Base64");
            return 0;
        case 2:
            k++;
        case 3:
            result[k++] = 0;
        }
    }
    result[j] = '\0';
    Decoded=(char*)result;
    free(result);
    return 1;
}

const char *CReadMail::GetMail()
/*!\brief Pointer auf komplette Mail zurückgeben
 *
 * Diese Funktion gibt einen Pointer auf den kompletten unveränderten Inhalt der Mail zurück.
 *
 * \returns Bei Erfolg liefert die Funktion einen Pointer auf den Inhakt der Mail zurück,
 * im Fehlerfall NULL,
 */
{
	if (!Loaded) {
		SetError(444);
		return NULL;
	}
	return (const char*)Mail.GetPtr();
}

int CReadMail::GetMail(CString &Buffer)
/*!\brief Komplette Mail in einen Puffer kopieren
 *
 * Diese Funktion kopiert die komplette unveränderte Mail in den angegebenen Buffer.
 *
 * \param[out] Buffer Pointer auf einen String, in den der Inhalt der Mail kopiert werden soll.
 *
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall false (0).
 */
{
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	Buffer.Set(Mail);
	return 1;
}

CAssocArray *CReadMail::GetHeaderArray(CAssocArray *Buffer)
/*!\brief Pointer auf Assoziatives Array mit Header-Zeilen
 *
 * Diese Funktion gibt einen Pointer auf ein Assoziatives Array zurück, dass die einzelnen Zeilen des
 * Headers enthält.
 *
 * \param[out] Buffer Ein optionaler Pointer auf ein Assoziatives Array, in das die Headerzeilen
 * kopiert werden sollen.
 *
 * \returns Wurde der \p Buffer-Parameter angegeben, wird bei Erfolg ein Pointer auf diesen zurückgegeben.
 * Wurde \p Buffer nicht angegeben, wird ein Pointer auf ein internes Array zurückgegeben, dessen Speicher von
 * der CReadMail-Klasse verwaltet wird und von der Anwendung nicht freigegeben werden darf.
 */
{
	if (!Loaded) {
		SetError(444);
		return NULL;
	}
	if (Header.Len()==0) {
		SetError(446);
		return NULL;
	}
	if (!Buffer) return &HeaderLines;
	if (Buffer->Copy(HeaderLines)) return Buffer;
	return NULL;
}

int CReadMail::GetHeaderArray(CAssocArray &Buffer)
/*!\brief Header-Zeilen in ein Assoziatives Array kopieren
 *
 * \desc
 * Mit dieser Funktion werden die Header-Zeilen der Mail als Key-Value-Paare
 * in das Assoziative Array \p Buffer kopiert.
 *
 * \param[out] Buffer Referenz auf ein Assoziatives Array, in das die Headerzeilen
 * kopiert werden sollen.
 *
 * \returns Wurden die Header-Zeilen erfolgreich kopiert, gibt die Funktion 1 zurück,
 * im Fehlerfall 0.
 */
{
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	if (Header.Len()==0) {
		SetError(446);
		return 0;
	}
	Buffer.Clear();
	return Buffer.Copy(HeaderLines);
}


const char *CReadMail::GetHeader(const char *key)
/*!\brief Gibt den Wert eines Keywortes im Header zurück
 *
 * Diese Funktion prüft, ob es im Header eine zum \p key passende Zeile gibt, und gibt dessen Wert
 * zurück.
 *
 * \param[in] key Pointer auf den Namen der Header-Zeile (z.B. "Subject" oder "From").
 * \returns Bei Erfolg liefert die Funktion einen Pointer auf den Wert des Schlüssels zurück, im
 * Fehlerfall NULL.
 *
 * \note Werte, die im Mail-Header über mehrere Zeilen gingen (z.B. Received-Zeilen) werden zu einer
 * Zeile zusammengefasst, wobei ein Space als Trennzeichen benutzt wird. Keyworte, die mehrfach im
 * Header vorkommen (z.B. Received-Zeilen), werden in einem String zusammengefasst, der durch
 * Newline getrennt wird. Mit Hilfe von CArray::Explode lassen sich so z.B. die einzelnen Received-Zeilen
 * einer Mail wieder aufsplitten.
 */
{
	if (!Loaded) {
		SetError(444);
		return NULL;
	}
	if (!key) {
		SetError(194,"const char *CReadMail::GetHeader(==> const char *key <==)");
		return NULL;
	}
	if (Header.Len()==0) {
		SetError(446);
		return NULL;
	}
	return (const char *)HeaderLines.Get(key);
}

const char *CReadMail::GetHeader()
/*!\brief Gibt einen Pointer auf den kompletten Header zurück
 *
 * Gibt einen Pointer auf den kompletten Header zurück
 *
 * \note Der Speicherbereich wird von der CReadMail-Klasse verwaltet und darf von der
 * Anwendung nicht verändert oder freigegeben werden!
 */
{
	if (!Loaded) {
		SetError(444);
		return NULL;
	}
	if (Header.Len()==0) {
		SetError(446);
		return NULL;
	}
	return (const char*)Header.GetPtr();
}

int CReadMail::GetHeader(CString &Buffer)
/*!\brief Kompletten Header in einen Puffer kopieren
 *
 * Diese Funktion kopiert den kompletten Header der Mail in den angegebenen Buffer.
 *
 * \param[out] Buffer Pointer auf einen String, in den der Inhalt des Headers kopiert werden soll.
 *
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall false (0).
 */
{
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	if (Header.Len()==0) {
		SetError(446);
		return 0;
	}
	Buffer.Set(Header);
	return 1;
}

int CReadMail::GetHeader(CString &Buffer, const char *key)
/*!\brief Kopiert den Wert eines Keywortes im Header in einen Puffer
 *
 * Diese Funktion prüft, ob es im Header eine zum \p key passende Zeile gibt, und kopiert dessen Wert
 * in den angegebenen Buffer.
 *
 * \param[out] Buffer Pointer auf einen String, in dem der Wert gespeichert werden soll.
 * \param[in] key Pointer auf den Namen der Header-Zeile (z.B. "Subject" oder "From").
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, im Fehlerfall false (0).
 *
 * \note Werte, die im Mail-Header über mehrere Zeilen gingen (z.B. Received-Zeilen) werden zu einer
 * Zeile zusammengefasst, wobei ein Space als Trennzeichen benutzt wird. Keyworte, die mehrfach im
 * Header vorkommen (z.B. Received-Zeilen), werden in einem String zusammengefasst, der durch
 * Newline getrennt wird. Mit Hilfe von CArray::Explode lassen sich so z.B. die einzelnen Received-Zeilen
 * einer Mail wieder aufsplitten.
 */
{
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	if (!key) {
		SetError(194,"int CReadMail::GetHeader(CString *Buffer, ==> const char *key <==)");
		return 0;
	}
	if (Header.Len()==0) {
		SetError(446);
		return 0;
	}
	const char *value=HeaderLines.Get(key);
	if (!value) return 0;
	Buffer.Set(value);
	return 1;
}


const char *CReadMail::GetBody()
/*!\brief Gibt einen Pointer auf den kompletten undekodierten Body zurück
 *
 * Gibt einen Pointer auf den kompletten undekodierten Body zurück
 *
 * \note Der Speicherbereich wird von der CReadMail-Klasse verwaltet und darf von der
 * Anwendung nicht verändert oder freigegeben werden!
 */
{
	if (!Loaded) {
		SetError(444);
		return NULL;
	}
	return (const char*)Body.GetPtr();
}

int CReadMail::GetBody(CString &Buffer)
/*!\brief Unveränderten Body in einen Puffer kopieren
 *
 * Diese Funktion kopiert den unveränderten original Body in den angegebenen Buffer.
 *
 * \param[out] Buffer Pointer auf einen String, in den der Inhalt des Bodies kopiert werden soll.
 *
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall false (0).
 */
{
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	if (Body.Len()==0) {
		SetError(445);
		return 0;
	}
	Buffer.Set(Body);
	return 1;
}


const char *CReadMail::GetDecodedBody()
/*!\brief Pointer auf dekodierten Body zurückgeben
 *
 * Diese Funktion liefert einen Pointer auf den Dekodierten Body zurück. Das Charset ist
 * dabei noch unverändert und entspricht der in der Mail angegebenen Kodierung.
 *
 * \returns Wenn der Body zuvor erfolgreich dekodiert werden konnte, gibt
 * die Funktion einen Pointer darauf zurück. Im Fehlerfall wird NULL zurückgegeben und
 * ein Fehlercode gesetzt.
 *
 * \note Der Speicherbereich wird von der CReadMail-Klasse verwaltet und darf von der
 * Anwendung nicht verändert oder freigegeben werden!
 */
{
	if (!Loaded) {
		SetError(444);
		return NULL;
	}
	if (Body.Len()==0) {
		SetError(445);
		return NULL;
	}
	return Decoded.GetPtr();
}

int CReadMail::GetDecodedBody(CString &Buffer)
/*!\brief Dekodierten Body in einen Puffer schreiben
 *
 * Diese Funktion kopiert den dekodierten Body in den angegebenen Buffer. Das Charset ist
 * dabei noch unverändert und entspricht der in der Mail angegebenen Kodierung.
 *
 * \param[out] Buffer Pointer auf einen String, in den der Inhalt des Bodies kopiert werden soll.
 *
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, im Fehlerfall false (0).
 */
{
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	if (Body.Len()==0) {
		SetError(445);
		return 0;
	}
	Buffer.Set(Decoded);
	return 1;
}



const char *CReadMail::GetTranscodedBody(const char *charset)
/*!\brief Pointer auf dekodierten Body zurückgeben
 *
 * Diese Funktion liefert einen Pointer auf Body zurück, der dekodiert und in das gewünschte
 * Charset gewandelt wurde.
 *
 * \param[in] charset Name des gewünschten Charsets. Der Default ist "UTF-8".
 * \returns Wenn der Body erfolgreich in das gewünschte Charset kodiert werden konnte, gibt
 * die Funktion einen Pointer darauf zurück. Im Fehlerfall wird NULL zurückgegeben und
 * ein Fehlercode gesetzt.
 *
 * \note Der Speicherbereich wird von der CReadMail-Klasse verwaltet und darf von der
 * Anwendung nicht verändert oder freigegeben werden! Bei wiederholtem Aufruf der
 * Funktion wird der alte Speicherbereich überschrieben.
 */
{
	if (!charset) {
		SetError(194,"const char *CReadMail::GetTranscodedBody(==> const char *charset <==)");
		return NULL;
	}
	if (!GetTranscodedBody(Transcoded,charset)) return NULL;
	return Transcoded.GetPtr();
}

int CReadMail::GetTranscodedBody(CString &Buffer, const char *charset)
/*!\brief Dekodierten und Transkodierten Body in einen Puffer kopieren
 *
 * Diese Funktion versucht den Body der Mail in das gewünschte \p charset zu transkodieren und
 * kopiert diesen dann in den angegebenen Buffer.
 *
 * \param[out] Buffer String, in dem das Ergebnis gespeichert werden soll.
 * \param[in] charset Name des gewünschten Charsets. Der Default ist "UTF-8".
 * \returns Wenn der Body erfolgreich in das gewünschte Charset kodiert werden konnte, gibt
 * die Funktion true(1) zurück und kopiert den Body in den Buffer. Im Fehlerfall wird
 * false (0) zurückgegeben und ein Fehlercode gesetzt.
 *
 */
{
	if (!charset) {
		SetError(194,"int CReadMail::GetTranscodedBody(CString *Buffer, ==> const char *charset <==)");
		return 0;
	}
	if (!Loaded) {
		SetError(444);
		return 0;
	}
	if (Body.Len()==0) {
		SetError(445);
		return 0;
	}
	// Wir benötigen das Encoding der Mail
	const char *fromcharset;
	CString Match;
	CString ContentType=HeaderLines.Get("Content-Type");
	if (ContentType.Len()==0) ContentType="text/plain; CHARSET=US-ASCII";
	if (ContentType.PregMatch("/charset\\=(.+)$/i")) {
		Match=ContentType.GetMatch(1);
		// An einem eventuell vorhandenen Space oder Semikolon abschneiden
		Match.Cut((char*)" ");
		Match.Cut((char*)";");
		// Eventuell vorhandene Anführungszeichen abschneiden
		Match.Trim("\"");
		fromcharset=ContentType.GetMatch(1);
	}
	else fromcharset="US-ASCII";
	return Buffer.TranscodeText(Decoded,fromcharset,charset);
}


}	// EOF namespace ppl6
