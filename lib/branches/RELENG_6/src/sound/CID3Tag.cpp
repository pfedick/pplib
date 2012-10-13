/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.10 $
 * $Date: 2010/10/10 05:24:36 $
 * $Id: CID3Tag.cpp,v 1.10 2010/10/10 05:24:36 pafe Exp $
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
#ifdef HAVE_STRING_H
#include <string.h>
#endif


#ifdef DeleteFile
#undef DeleteFile
#endif

#include "ppl6.h"
#include "ppl6-sound.h"


//#define ID3DEBUG

namespace ppl6 {

static const char * genres[] = {
	"Blues",					// 0
	"Classic Rock",
	"Country",
	"Dance",
	"Disco",
	"Funk",
	"Grunge",
	"Hip-Hop",
	"Jazz",
	"Metal",
	"New Age",
	"Oldies",
	"Other",
	"Pop",
	"R&B",
	"Rap",
	"Reggae",
	"Rock",
	"Techno",
	"Industrial",
	"Alternative",
	"Ska",
	"Death Metal",
	"Pranks",
	"Soundtrack",
	"Euro-Techno",
	"Ambient",
	"Trip-Hop",
	"Vocal",
	"Jazz+Funk",				// 29
 	"Fusion",					// 30
	"Trance",					// 31
	"Classical",				// 32
	"Instrumental",				// 33
	"Acid",						// 34
	"House",					// 35
	"Game",
	"Sound Clip",
	"Gospel",
	"Noise",
	"AlternRock",				// 40
	"Bass",
	"Soul",
	"Punk",
	"Space",
	"Meditative",
	"Instrumental Pop",
	"Instrumental Rock",
	"Ethnic",
	"Gothic",
	"Darkwave",
	"Techno-Industrial",
	"Electronic",
	"Pop-Folk",
	"Eurodance",
	"Dream",
	"Southern Rock",
	"Comedy",
	"Cult",
	"Gangsta",
	"Top 40",
	"Christian Rap",
	"Pop/Funk",
	"Jungle",
	"Native American",
	"Cabaret",
	"New Wave",
	"Psychadelic",
	"Rave",
	"Showtunes",
	"Trailer",
	"Lo-Fi",
	"Tribal",
	"Acid Punk",
	"Acid Jazz",
	"Polka",
	"Retro",
	"Musical",
	"Rock & Roll",
	"Hard Rock",
	"Folk",
	"Folk-Rock",
	"National Folk",
	"Swing",
	"Fast Fusion",
	"Bebob",
	"Latin",
	"Revival",
	"Celtic",
	"Bluegrass",
	"Avantgarde",
	"Gothic Rock",
	"Progressive Rock",
	"Psychedelic Rock",
	"Symphonic Rock",
	"Slow Rock",
	"Big Band",
	"Chorus",
	"Easy Listening",
	"Acoustic",
	"Humour",
	"Speech",
	"Chanson",
	"Opera",
	"Chamber Music",
	"Sonata",
	"Symphony",
	"Booty Bass",
	"Primus",
	"Porn Groove",
	"Satire",
	"Slow Jam",
	"Club",
	"Tango",
	"Samba",
	"Folklore",
	"Ballad",
	"Power Ballad",
	"Rhythmic Soul",
	"Freestyle",
	"Duet",
	"Punk Rock",
	"Drum Solo",
	"A capella",
	"Euro-House",
	"Dance Hall",			// 125
	"Goa",
	"Drum’n’Bass",			// 127
	"Club-House",			// 128
	"Hardcore",
	"Terror",
	"Indie",
	"BritPop",
	"Negerpunk",
	"Polsk Punk",
	"Beat",
	"Christian Gangsta",
	"Heavy Metal",
	"Black Metal",
	"Crossover",
	"Contemporary Christian",
	"Christian Rock",
	"Merengue",
	"Salsa",
	"Thrash Metal",
	"Anime",
	"JPop",
	"SynthPop",				// 147
	NULL
};

static size_t strlen16(const char *data)
{
	size_t l=0;
	ppluint16 *d=(ppluint16*)data;
	while (d[l]!=0) l++;
	return l;
}

static void Bom(CBinary &bin)
{
	unsigned char str[2];
	str[0]=0xfe;
	str[1]=0xff;
	bin.Copy(str,2);
}


/*!\ingroup PPLGroupSound
 * \brief Genre-Bezeichnung
 *
 * \desc
 * Mit dieser Funktion kann die Bezeichnung eines Genres anhand der Genre-ID ausgelesen werden.
 * Die Werte 0-79 wurden durch die ID3v1-Spezifikation definiert, die Werte 80-147 sind eine
 * Erweiterung von WinAmp (siehe http://www.mpx.cz/mp3manager/tags.htm).
 *
 * @param id Zahl von 0 bis 147, die den Index des Genres darstellt
 * @return Pointer auf einen mit 0-terminierten String mit der englischen Genre-Bezeichnung.
 * Falls die \p id ungültig ist, gibt die Funktion \c NULL zurück, es wird aber kein Fehlercode
 * gesetzt.
 */
const char *GetID3GenreName(int id)
{
	int max=sizeof(genres) / sizeof(char *)-2;
	if (id<0 || id>max) return NULL;
	return genres[id];
}

/*!\class CID3Frame
 * \ingroup PPLGroupSound
 * \brief Klasse zum Speichern eines einzelnen MP3-Frames
 */

/*!\brief Konstruktor ohne Parameter
 *
 */
CID3Frame::CID3Frame()
{
	poke32(ID,0);
	ID[4]=0;
	Flags=0;
	Size=0;
	data=NULL;
	nextFrame=previousFrame=NULL;
}

/*!\brief Konstruktor mit Framenamen
 *
 * \desc
 * Konstruktor mit Framenamen
 * @param name Der 4-stellige Name des Frames
 */
CID3Frame::CID3Frame(const char *name)
{
	poke32(ID,0);
	::strncpy(ID,name,4);
	ID[4]=0;
	Flags=0;
	Size=0;
	data=NULL;
	nextFrame=previousFrame=NULL;
}


/*!\brief Destruktor
 *
 */
CID3Frame::~CID3Frame()
{
	if (previousFrame) previousFrame->nextFrame=nextFrame;
	if (nextFrame) nextFrame->previousFrame=previousFrame;
	if(data) free(data);
}


/*!\class CID3Tag
 * \ingroup PPLGroupSound
 * \brief Klasse zum Parsen und Verändern von MP3 ID3 v1 und v2 Tags
 *
 * \desc
 * Mit dieser Klasse können ID3v2-Tags aus MP3-Dateien ausgelesen und geschrieben werden.
 * Beim Schreiben kann zusätzlich noch ein ID3v1-Tag erzeugt werden.
 * \par
 * Dazu muß zunächst die gewünschte Datei mit CID3Tag::Load geöffnet werden. Mit den
 * verschiedenen "Get"-Befehlen können verschiedene vorgegebene Frames aus dem ID3v2-Tag
 * ausgelesen werden (z.B. CID3Tag::GetArtist um den Namen des Interpreten auszulesen),
 * mit "Set" werden sie verändert (z.B. CID3Tag::SetArtist). Mit der Funktionen
 * CID3Tag::FindFrame kann auch jedes beliebige andere Frame ausgelesen werden, jedoch
 * gibt es zur Zeit keine generische Funktion zum Speichern von beliebigen Frames.
 * \par
 * Alle Operationen finden nur im Hauptspeicher statt. Erst durch Aufruf der
 * Funktion CID3Tag::Save werden sie zurück in die Datei geschrieben.
 *
 *
 */

/*!\brief Konstruktor ohne Dateinamen
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird die Klasse initialisiert, ohne dass
 * eine Datei geladen wird. Dies kann mit CID3Tag::Load später nachgeholt werden.
 *
 */
CID3Tag::CID3Tag()
{
	firstFrame=lastFrame=NULL;
	PaddingSize=1024;
	PaddingSpace=128;
	MaxPaddingSpace=1024;
}

/*!\brief Konstruktor mit Dateinamen
 *
 * \desc
 * Durch Verwendung dieses Konstruktors wird die Klasse initialisiert und die
 * angegebene Date \p filename geladen. Dazu wird die Funktion CID3Tag::Load
 * aufgerufen.
 *
 * \param filename Name der MP3-Datei, deren Tags geladen werden soll.
 *
 */
CID3Tag::CID3Tag(const char *filename)
{
	firstFrame=lastFrame=NULL;
	PaddingSize=1024;
	PaddingSpace=128;
	MaxPaddingSpace=1024;
	Load(filename);
}

/*!\brief Destruktor der Klasse
 *
 * \desc
 * Der Destruktor ruft die Funktion CID3Tag::Clear auf, wodurch sämtlicher durch die
 * Klasse reservierter Speicher wieder freigegeben wird.
 *
 */
CID3Tag::~CID3Tag()
{
	Clear();
}
/*!\brief To be done
 *
 * \desc
 * Default=1024
 *
 * @param bytes Anzahl Bytes
 */
void CID3Tag::SetPaddingSize(int bytes)
{
	PaddingSize=bytes;
}

void CID3Tag::SetPaddingSpace(int bytes)
{
	PaddingSpace=bytes;
}

void CID3Tag::SetMaxPaddingSpace(int bytes)
{
	MaxPaddingSpace=bytes;
}

void CID3Tag::SetLogfile(CLog *log)
{
	Log=log;
}

/*!\brief Speicher freigeben und Klasse in den Ausgangszustand versetzen
 *
 * \desc
 * Durch Aufruf dieser Funktion wird der komplette durch diese Klasse reservierte
 * Speicher freigegeben und die Klasse in den Ausgangszustand zurückversetzt.
 * Die Funktion wird automatisch vor dem Laden einer Datei mit CID3Tag::Load
 * aufgerufen.
 */
void CID3Tag::Clear()
{
	CID3Frame *frame=NULL;
	while ((frame=firstFrame)) {
		firstFrame=frame->nextFrame;
		delete frame;
	}
	firstFrame=lastFrame=NULL;
	Filename.Clear();
}

/*!\brief Alle Tags löschen
 *
 * \desc
 * Durch Aufruf dieser Funktion werden alle Frames des geladenen Titels im Speicher gelöscht.
 * Die Datei bleibt unverändert, erst durch Aufruf der Funktion CID3Tag::Save werden
 * auch die ID3-Tags in der Datei aktualisiert und somit gelöscht.
 */
void CID3Tag::ClearTags()
{
	CID3Frame *frame=NULL;
	while ((frame=firstFrame)) {
		firstFrame=frame->nextFrame;
		delete frame;
	}
	firstFrame=lastFrame=NULL;
}

/*!\brief ID3-Tags aus einer MP3-Datei laden
 *
 * \desc
 * Mit dieser Funktion werden die ID3-Tags aus der MP3-Datei \p file geladen.
 *
 * @param file Dateiname
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Wird dabei
 * der Fehlercode 402 zurückgegeben, enthält die Datei keine ID3v2-Tags. Es kann
 * aber trotzdem die Funktion CID3Tag::Save aufgerufen werden, um Tags zu
 * speichern.
 */
int CID3Tag::Load(const CString &File)
{
	Clear();
	CFile ff;
	if (!ff.Open(File,"rb")) return 0;
	Filename=File;
	return Load(&ff);
}

/*!\brief ID3-Tags aus einer MP3-Datei laden
 *
 * \desc
 * Mit dieser Funktion werden die ID3-Tags aus der MP3-Datei \p file geladen.
 *
 * @param file Dateiname
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Wird dabei
 * der Fehlercode 402 zurückgegeben, enthält die Datei keine ID3v2-Tags. Es kann
 * aber trotzdem die Funktion CID3Tag::Save aufgerufen werden, um Tags zu
 * speichern.
 */
int CID3Tag::Load(const char *file)
{
	Clear();
	CFile ff;
	if (!ff.Open(file,"rb")) return 0;
	Filename=file;
	return Load(&ff);
}

/*!\brief MP3-File aus einem CFileObject laden
 *
 * \desc
 * Mit dieser Funktion werden die ID3-Tags einer bereits geöffneten MP3-Datei,
 * die durch das CFileObject \p File repräsentiert wird, in den Hauptspeicher geladen.
 *
 * @param[in] File Pointer auf ein CFileObject mit der geöffneten Datei
 * @return Konnten die ID3-Tags erfolgreich geladen werden, gibt die Funktion 1 zurück,
 * im Fehlerfall 0. Wird dabei der Fehlercode 402 zurückgegeben, enthält die Datei keine
 * ID3v2-Tags.
 */
int CID3Tag::Load(CFileObject *File)
{
	// ID3V2 Header einlesen (10 Byte)
	const char *adr=File->Map(0,10);
	if (!adr) {
		SetError(401);
		return 0;
	}
	#ifdef ID3DEBUG
		printf ("Lade File: %s\n",(const char*)Filename);
	#endif
	if (strncmp(adr,"ID3",3)!=0) {
		SetError(402,"Kein \"ID3\"-Header");
		return 0;
	}
	int version=0;
	if (peek8(adr+3)==3 && peek8(adr+4)==0) version=3;			// Version 2.3
	else if (peek8(adr+3)==4 && peek8(adr+4)==0) version=4;		// Version 2.4
	if (version==0) {
		SetError(403);
		return 0;
	}
	Flags=peek8(adr+5);
	if (Flags&128) {		// Unsynchonisation-Flag gesetzt
		SetError(404);
		return 0;
	}
	if (Flags&64) {			// Extended Header Flag
		SetError(405);
		return 0;
	}
	Size=peek8(adr+9);
	int s=peek8(adr+8);
	Size|=s<<7;
	s=peek8(adr+7);
	Size|=s<<14;
	s=peek8(adr+6);
	Size|=s<<21;

	ppluint32 p=10;

	#ifdef ID3DEBUG
		printf ("ID3 V2-Tag gefunden, Flags: %i, Länge: %i Bytes\n",Flags,Size);
	#endif
	CID3Frame *Frame;
	// Jetzt lesen wir alle Frames in den Speicher
	while (1) {
		adr=File->Map(p,10);
		if (!adr) break;
#ifdef ID3DEBUG
		HexDump((void*)adr,10);
#endif
		if (peek32(adr)==0) break;
		Frame=new CID3Frame;
		Frame->ID[0]=adr[0];
		Frame->ID[1]=adr[1];
		Frame->ID[2]=adr[2];
		Frame->ID[3]=adr[3];
		Frame->ID[4]=0;
		Frame->Flags=PeekN16(adr+8);
		if (version==4) {
			Frame->Size=peek8(adr+7)|(peek8(adr+6)<<7)|(peek8(adr+5)<<14)|(peek8(adr+4)<<21);
		} else {
			Frame->Size=peek8(adr+7)|(peek8(adr+6)<<8)|(peek8(adr+5)<<16)|(peek8(adr+4)<<24);
		}
		if (!Frame->Size) {
			delete Frame;
			break;
		}
		adr=File->Map(p+10,Frame->Size);
		if (adr) {
			//HexDump(adr,Frame->Size);
			Frame->data=(char*)malloc(Frame->Size);
			if (Frame->data) memcpy(Frame->data,adr,Frame->Size);
			AddFrame(Frame);
		} else {
			delete Frame;
		}
		p=p+10+Frame->Size;
		//adr=File->Map(p,256);
		//HexDump(adr,256);
	}


	return 1;
}

/*!\brief Frame hinzufügen
 *
 * \desc
 * Mit dieser internen Funktion wird ein neues Frame zu den ID3v2-Tags
 * hinzugefügt.
 *
 * @param Frame Pointer auf ein neues CID3Frame
 * @return Die Funktion gibt immer 1 zurück.
 */
int CID3Tag::AddFrame(CID3Frame *Frame)
{
	if (!lastFrame) {		// Erster Eintrag
		firstFrame=lastFrame=Frame;
		Frame->previousFrame=NULL;
		Frame->nextFrame=NULL;
	} else {
		lastFrame->nextFrame=Frame;
		Frame->previousFrame=lastFrame;
		Frame->nextFrame=NULL;
		lastFrame=Frame;
	}
	#ifdef ID3DEBUG
		char *content="";
		char *content2="";
		if (strcmp(Frame->ID,"TXXX")==0) {
			content=((char*)Frame->data)+1;
			content2=((char*)Frame->data)+1+strlen(content)+1;
		} else if (strcmp(Frame->ID,"COMM")==0) {
			content=((char*)Frame->data)+4;
			content2=((char*)Frame->data)+4+strlen(content)+1;

		}

		printf ("Adding Frame %s, %i Bytes, Flags: %i: %s=%s\n",Frame->ID, Frame->Size, Frame->Flags, content, content2);
	#endif

	return 1;
}

int CID3Tag::RemoveFrame(CID3Frame *frame)
{
	if (!frame) return 0;
	if (frame->previousFrame) frame->previousFrame->nextFrame=frame->nextFrame;
	if (frame->nextFrame) frame->nextFrame->previousFrame=frame->previousFrame;
	if (frame==firstFrame) firstFrame=frame->nextFrame;
	if (frame==lastFrame) lastFrame=frame->previousFrame;
	return 1;
}

int CID3Tag::DeleteFrame(CID3Frame *frame)
{
	if (!frame) return 0;
	if (!RemoveFrame(frame)) return 0;
	delete frame;
	return 1;
}


/*!\brief Frames auf STDOUT auflisten
 *
 * \desc
 * Mit dieser Funktion werden alle Frames auf STDOUT ausgegeben. Sie ist zu
 * Debug-Zwecken gedacht.
 */
void CID3Tag::ListFrames(int hexdump) const
{
	CID3Frame *frame=firstFrame;
	while (frame) {
		printf ("Frame: %s, Size: %u, Flags: %u\n",frame->ID, frame->Size, frame->Flags);
		if (hexdump) ppl6::HexDump(frame->data,frame->Size);
		frame=frame->nextFrame;
	}
}

/*!\brief Bestimmtes Frame finden
 *
 * \desc
 * Mit dieser Funktion wird nach einem Frame mit der ID \p name
 * gesucht und ein Pointer darauf zurückgegeben.
 *
 * @param name String mit der 4-stelligen ID des gesuchten Frames.
 * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
 * Pointer auf die CID3Frame-Klasse zurück, im Fehlerfall \c NULL.
 */
CID3Frame *CID3Tag::FindFrame(const char *name) const
{
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(strcmp(frame->ID,name)==0) return frame;
		frame=frame->nextFrame;
	}
	return NULL;
}

/*!\brief Bestimmtes Frame finden
 *
 * \desc
 * Mit dieser Funktion wird nach einem Frame mit der ID \p name
 * gesucht und ein Pointer darauf zurückgegeben.
 *
 * @param name String mit der 4-stelligen ID des gesuchten Frames.
 * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
 * Pointer auf die CID3Frame-Klasse zurück, im Fehlerfall \c NULL.
 */
CID3Frame *CID3Tag::FindFrame(const CString &name) const
{
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(name.StrCmp(frame->ID)==0) return frame;
		frame=frame->nextFrame;
	}
	return NULL;
}


/*!\brief Benutzerdefinierten Text in einem TXXX-Frame finden
 *
 * \desc
 * Mit dieser Funktion wird nach einem benutzerdefinierten Text
 * in einem TXXX-Frame mit dem Namen \p description gesucht
 * und ein Pointer darauf zurückgegeben.
 *
 * @param name String mit der Description des gesuchten Frames.
 * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
 * Pointer auf die CID3Frame-Klasse zurück, im Fehlerfall \c NULL.
 */
CID3Frame *CID3Tag::FindUserDefinedText(const char *description) const
{
	CString c;
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(strcmp(frame->ID,"TXXX")==0) {
			Decode(frame,+1,Peek8(frame->data),c);
			if (c.StrCmp(description)==0) return frame;
		}
		frame=frame->nextFrame;
	}
	return NULL;
}

/*!\brief Benutzerdefinierten Text in einem TXXX-Frame finden
 *
 * \desc
 * Mit dieser Funktion wird nach einem benutzerdefinierten Text
 * in einem TXXX-Frame mit dem Namen \p description gesucht
 * und ein Pointer darauf zurückgegeben.
 *
 * @param name String mit der Description des gesuchten Frames.
 * @return Wurde das gewünschte Frame gefunden, gibt die Funktion einen
 * Pointer auf die CID3Frame-Klasse zurück, im Fehlerfall \c NULL.
 */
CID3Frame *CID3Tag::FindUserDefinedText(const CString &description) const
{
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(strcmp(frame->ID,"TXXX")==0) {
			if (description.StrCmp(frame->data+1)==0) return frame;
		}
		frame=frame->nextFrame;
	}
	return NULL;
}


int CID3Tag::SetTextFrame(const char *framename, const CString &text)
{
	bool exists=false;
	CBinary enc;
	if (!text.Transcode("UTF-8","UTF-16",enc)) return 0;
	if (enc.Len()==0) Bom(enc);
	CID3Frame *frame=FindFrame(framename);
	if (frame) {
		exists=true;
	} else {
		frame=new CID3Frame(framename);
		if (!frame) {
			SetError(2);
			return 0;
		}
	}
	frame->Flags=0;
	frame->Size=enc.Len()+3;
	//printf ("Frame-Size: %i\n",frame->Size);
	if (frame->data) free(frame->data);
	frame->data=(char*)malloc(frame->Size);
	if (!frame->data)  {
		frame->Size=0;
		if (!exists) delete frame;
		SetError(2);
		return 0;
	}

	Poke8(frame->data,1);
	Poke16(frame->data+1+enc.Len(),0);	// Terminierendes 0-Byte
	memcpy(frame->data+1,enc.GetPtr(),enc.Len());
	if (!exists) {
		AddFrame(frame);
	}
	return 1;
}

/*!\brief Name des Interpreten setzen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Interpreten (Artist) festgelegt.
 * Der Wert wird im Frame "TPE1" gespeichert.
 *
 * @param[in] artist String mit dem Namen des Interpreten
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetArtist(const CString &artist)
{
	return SetTextFrame("TPE1",artist);
}

/*!\brief Titel des Songs setzen
 *
 * \desc
 * Mit dieser Funktion wird der Titel des Songs festgelegt.
 * Der Wert wird im Frame "TIT2" gespeichert.
 *
 * @param[in] title String mit dem Titel des Songs
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */

int CID3Tag::SetTitle(const CString &title)
{
	return SetTextFrame("TIT2",title);
}

/*!\brief Name des Albums setzen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Albums festgelegt, aus dem der Titel stammt.
 * Der Wert wird im Frame "TALB" gespeichert.
 *
 * @param[in] album String mit dem Namen des Albums
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetAlbum(const CString &album)
{
	return SetTextFrame("TALB",album);
}

/*!\brief Name des Remixers oder des Mixes
 *
 * \desc
 * Mit dieser Funktion wird der Name des Remixers oder des Mixes festgelegt.
 * Der Wert wird im Frame "TPE4" gespeichert, sowie in einem benutzerdefinierten
 * "TXXX"-Frame mit dem Namen "TraktorRemixer". Letzterer wird von Traktor
 * von Native Instruments verwendet.
 *
 * @param[in] remixer String mit dem Namen des Remixers oder des Mixes.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetRemixer(const CString &remixer)
{
	if (!SetTextFrame("TPE4",remixer)) return 0;
	bool exists=false;
	CBinary enc, udf;
	CString udfstring="TraktorRemixer";
	if (!remixer.Transcode("UTF-8","UTF-16",enc)) return 0;
	if (!udfstring.Transcode("UTF-8","UTF-16",udf)) return 0;
	if (enc.Len()==0) Bom(enc);
	if (udf.Len()==0) Bom(udf);
	CID3Frame *frame;
	frame=FindUserDefinedText("TraktorRemixer");
	if (frame) {
		exists=true;
	} else {
		frame=new CID3Frame("TXXX");
		if(!frame) {
			SetError(2);
			return 0;
		}
	}
	frame->Flags=0;
	frame->Size=enc.Len()+5+udf.Len();
	if (frame->data) free(frame->data);
	frame->data=(char*)malloc(frame->Size);
	if (!frame->data) {
		if(!exists) delete frame;
		SetError(2);
		return 0;
	}
	Poke8(frame->data,1);
	memcpy(frame->data+1,udf.GetPtr(),udf.Len());
	Poke16(frame->data+1+udf.Len(),0);	// Terminierendes 0-Byte
	memcpy(frame->data+3+udf.Len(),enc.GetPtr(),enc.Len());
	Poke16(frame->data+3+udf.Len()+enc.Len(),0);	// Terminierendes 0-Byte
	if (!exists) {
		AddFrame(frame);
	}
	return 1;
}

/*!\brief Name des Plattenlabels setzen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Plattenlabels festgelegt,
 * auf dem der Titel erschienen ist.
 * Der Wert wird im Frame "TPUB" gespeichert.
 *
 * @param[in] label String mit dem Namen des Plattenlabels
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetLabel(const CString &label)
{
	return SetTextFrame("TPUB",label);
}

/*!\brief Nummer des Tracks setzen
 *
 * \desc
 * Mit dieser Funktion wird die Tracknummer des Titels innerhalb des
 * Datenträgers oder Sets gesetzt. Der String muss eine Ziffer enthalten,
 * die die Position des Tracks innerhalb des Datenträgers oder Sets angibt.
 * Optional kann durch Slash getrennt noch die Gesamtzahl der Tracks folgen,
 * also z.B. "4/9".

 * Der Wert wird im Frame "TRCK" gespeichert.
 *
 * @param[in] track String mit der Nummer des Tracks.
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetTrack(const CString &track)
{
	return SetTextFrame("TRCK",track);
}

int CID3Tag::SetBPM(const CString &bpm)
{
	return SetTextFrame("TBPM",bpm);
}

/*!\brief Erscheinungsjahr des Titels setzen
 *
 * \desc
 * Mit dieser Funktion wird das Erscheinungsjahr des Titel festgelegt.
 * Der Wert wird im Frame "TYER" gespeichert.
 *
 * @param[in] year String mit dem Erscheinungsjahr des Titels
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetYear(const CString &year)
{
	return SetTextFrame("TYER",year);
}

/*!\brief Musikgenre setzen
 *
 * \desc
 * Mit dieser Funktion wird das Musikgenre des Titels festgelegt. Falls
 * das Genre eine offizielle ID hat, wird diese im Frame in Klammern vorangestellt
 * (z.B. <tt>(13)Pop</tt>).
 * Der Wert wird im Frame "TCON" gespeichert.
 *
 * @param[in] genre String mit dem Namen des Musikgenres
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetGenre(const CString &genre)
{
	CString gen=genre;
	// Gibt es in der Genre-Liste einen passenden Eintrag?
	int genreid=0;
	while(genres[genreid]!=NULL) {
		if (genre.StrCmp((char*)genres[genreid])==0) {
			gen.Setf("(%i)",genreid);
			gen+=genre;
			break;
		}
		genreid++;
	}
	return SetTextFrame("TCON",gen);
}

/*!\brief Kommentar setzen
 *
 * \desc
 * Mit dieser Funktion wird ein Kommentar zum Titel festgelegt.
 * Der Wert wird im Frame "COMM" gespeichert.
 *
 * @param[in] comment String mit dem Kommentar
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetComment(const CString &comment)
{
	return SetComment("",comment);
}

/*!\brief Kommentar setzen
 *
 * \desc
 * Mit dieser Funktion wird ein Kommentar zum Titel festgelegt.
 * Der Wert wird im Frame "COMM" gespeichert.
 *
 * @param[in] comment String mit dem Kommentar
 * @param[in] shortcomment String mit dem Kommentar
 * @return Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 */
int CID3Tag::SetComment(const CString &description, const CString &comment)
{
	const char *framename="COMM";
	bool exists=false;
	CBinary enc;
	CBinary shortenc;
	if (!description.Transcode("UTF-8","UTF-16",shortenc)) return 0;
	if (!comment.Transcode("UTF-8","UTF-16",enc)) return 0;
	if (shortenc.Len()==0) Bom(shortenc);
	if (enc.Len()==0) Bom(enc);
	CID3Frame *frame=FindFrame(framename);
	if (frame) {
		exists=true;
		// Es könnte andere COMM-Frames geben, die löschen wir
		CID3Frame *nf,*f=firstFrame;
		while (f) {
			if(strcmp(f->ID,framename)==0 && f!=frame) {
				// Frame löschen
				nf=f->nextFrame;
				DeleteFrame(f);
				f=nf;
			} else {
				f=f->nextFrame;
			}
		}

	} else {
		frame=new CID3Frame(framename);
		if (!frame) {
			SetError(2);
			return 0;
		}
	}
	frame->Flags=0;
	frame->Size=enc.Len()+3+3+shortenc.Len()+2;
	//printf ("Frame-Size: %i\n",frame->Size);
	if (frame->data) free(frame->data);
	frame->data=(char*)malloc(frame->Size);
	if (!frame->data)  {
		frame->Size=0;
		if (!exists) delete frame;
		SetError(2);
		return 0;
	}

	Poke8(frame->data,1);

	//strncpy(frame->data+1,"eng",3);
	memset(frame->data+1,0,3);
	memcpy(frame->data+4,shortenc.GetPtr(),shortenc.Len());
	Poke16(frame->data+4+shortenc.Len(),0);	// Terminierendes 0-Byte
	memcpy(frame->data+6+shortenc.Len(),enc.GetPtr(),enc.Len());
	Poke16(frame->data+6+shortenc.Len()+enc.Len(),0);	// Terminierendes 0-Byte
	if (!exists) {
		AddFrame(frame);
	}
	return 1;
}

/*!\brief ID3 Version 1 Tag erstellen
 *
 * \desc
 * Diese Funktion erstellt einen 128 Byte langen ID3v1-Tag im Speicher und gibt einen
 * Pointer darauf zurück.
 *
 * @return Bei Erfolg gibt die Funktion einen Pointer auf einen 128-Byte großen Speicherblock
 * zurück, der nach Gebrauch von der aufrufenden Anwendung mit \c free freigegeben werden muß.
 * Im Fehlerfall gibt sie \c NULL zurück.
 */
char* CID3Tag::MakeIdV1Tag()
{
	CString text;
	char *tag=(char*)malloc(128);
	memset(tag,0,128);
	strcpy(tag,"TAG");
	CID3Frame *frame=FindFrame("TIT2");
	if (frame) {
		text.Clear();
		if (frame->data[0]==0) text.Set(frame->data+1,frame->Size-1);
		if (frame->data[0]==1) text.TranscodeText(frame->data+1,frame->Size-1,"UTF-16","ISO-8859-1");
		if (text.Len()) strncpy(tag+3,text,30);
	}
	frame=FindFrame("TPE1");
	if (frame) {
		text.Clear();
		if (frame->data[0]==0) text.Set(frame->data+1,frame->Size-1);
		if (frame->data[0]==1) text.TranscodeText(frame->data+1,frame->Size-1,"UTF-16","ISO-8859-1");
		if (text.Len()) strncpy(tag+33,text,30);
	}
	frame=FindFrame("TYER");
	if (frame) {
		text.Clear();
		if (frame->data[0]==0) text.Set(frame->data+1,frame->Size-1);
		if (frame->data[0]==1) text.TranscodeText(frame->data+1,frame->Size-1,"UTF-16","ISO-8859-1");
		if (text.Len()) strncpy(tag+93,text,4);
	}

	frame=FindFrame("TPE4");
	if (frame) {
		text.Clear();
		if (frame->data[0]==0) text.Set(frame->data+1,frame->Size-1);
		if (frame->data[0]==1) text.TranscodeText(frame->data+1,frame->Size-1,"UTF-16","ISO-8859-1");
		if (text.Len()) strncpy(tag+97,text,30);
	}
	frame=FindFrame("TCON");
	if (frame) {
		text.Clear();
		if (frame->data[0]==0) text.Set(frame->data+1,frame->Size-1);
		if (frame->data[0]==1) text.TranscodeText(frame->data+1,frame->Size-1,"UTF-16","ISO-8859-1");
		if (text.PregMatch("/^\\(([0-9]+)\\).*$/")) {
			int g=ppl6::atoi(text.GetMatch(1));
			Poke8(tag+127,g);
			//printf ("Genre: %s: %i\n",(char*)text,g);
		} else {
			Poke8(tag+127,255);
		}
	}

	return tag;
}


/*!\brief Tag speichern
 *
 * \desc
 * Bei Aufruf dieser Funktion wird der ID3-Tag in der zuvor mit CID3Tag::Load
 * geladenen Datei gespeichert. Ist das
 * Flag \p writev1 gesetzt, wird ein ID3v1-Tag am Ende der Datei angehangen, bzw.
 * ein bereits vorhandener Tag überschrieben. Ist das Flag \p writev2 gesetzt, wird
 * ein ID3v2-Tag am Anfang der Datei geschrieben. Dabei werden die nachfolgenden
 * Daten gegebenenfalls nach hinten verschoben.
 *
 * @param writev1 ID3v1-Tag schreiben
 * @param writev2 ID3v2-Tag schreiben.
 *
 * @return Konnten die Tags erfolgreich geschrieben werden, gibt die Funktion 1
 * zurück, im Fehlerfall 0.
 *
 * \attention
 * Zur Zeit hat der Parameter \p writev2 keine Auswirkung auf das Verhalten. Es wird
 * immer ein ID3v2-Tag geschrieben.
 */
int CID3Tag::Save(bool writev1, bool writev2)
{
	if (Filename.IsEmpty()) {
		SetError(543);
		return 0;
	}
	CString tmpfile=Filename;
	tmpfile+=".rename.tmp";
	CFile n;
	CFile o;
	if (!n.Open(&tmpfile,"w+b")) return 0;
	if (!o.Open(Filename,"r+b")) return 0;
	// Wir benötigen exklusiven Zugriff auf das File
	//if (!o.LockExclusive(false)) return 0;		// TODO: Hat unter Windows keine Wirkung
	PPL_MPEG_HEADER mpg;

	if (!IdentMPEG(&o, &mpg)) {
		ppl6::PushError();
		//o.Unlock();
		ppl6::PopError();
		return 0;
	}

	//printf ("Daten-Anfang: %u, Ende: %u, Size: %u\n",mpg.start, mpg.end, mpg.size);

	ppluint32 pn=0;

	if (firstFrame) {
		pn=10;
		// Leeren ID3v2-Header schreiben
		char header[10];
		char frame[10];
		header[0]='I';
		header[1]='D';
		header[2]='3';
		header[3]=4;
		header[4]=0;
		header[5]=0;

		n.Write(header,10,0);
		CID3Frame *Frame=firstFrame;
		ppluint32 s;
		while (Frame) {
			//printf ("Save: %s, %u Bytes\n",Frame->ID,Frame->Size);
			frame[0]=Frame->ID[0];
			frame[1]=Frame->ID[1];
			frame[2]=Frame->ID[2];
			frame[3]=Frame->ID[3];
			// Descynchronisieren
			s=Frame->Size;
			Poke8(frame+7,(s&127));
			Poke8(frame+6,((s>>7)&127));
			Poke8(frame+5,((s>>14)&127));
			Poke8(frame+4,((s>>21)&127));

			PokeN16(frame+8,Frame->Flags);
			n.Write(frame,10,pn);
			n.Write(Frame->data,Frame->Size,pn+10);
			pn=pn+10+Frame->Size;
			Frame=Frame->nextFrame;
		}
		memset(frame,0,10);
		n.Write(frame,10,pn);
		pn+=10;
		ppluint32 size=pn-10;
		header[9]=size&127;
		header[8]=(size>>7)&127;
		header[7]=(size>>14)&127;
		header[6]=(size>>21)&127;
		n.Write(header,10,0);
	}
	char *v1=NULL;
	if (writev1) v1=MakeIdV1Tag();
	ppluint32 rest=0;
	bool useoldfile=false;
	// Falls der neue Tag noch vor den ersten Frame passt und MaxPaddingSpace nicht
	// überschritten wird, schreiben wir ihn in das Originalfile
	if (pn<=mpg.start && mpg.start-pn<MaxPaddingSpace) {
		o.Copy(n,0,pn,0);
		// Mit Nullen auffüllen bis zum Start der Frames
		rest=mpg.start-pn;
		useoldfile=true;
		//printf ("Keine Verschiebung nötig, verwende altes File\n");
	} else {
		if (pn<PaddingSize) {
			// Wir füllen mit Nullen auf
			rest=PaddingSize-pn;
		} else {
			// Wir lassen mindestens "PaddingSpace" frei
			rest=PaddingSpace;
		}
	}
	if (rest>0) {
		char *space=(char*)calloc(1024,1);
		ppluint32 bytes;
		while (rest) {
			bytes=rest;
			if (bytes>1024) bytes=1024;
			if (useoldfile)	o.Write(space,bytes,pn);
			else n.Write(space,bytes,pn);
			pn+=bytes;
			rest-=bytes;
		}
		free(space);
	}
	if (useoldfile) {
		// Am Ende noch den v1-Tag reinschreiben
		if (v1) o.Write(v1,128,mpg.end);
		n.Close();
		o.Close();
		CFile::DeleteFile(tmpfile);
	} else {
		// Nun kopieren wir die Musikframes
		n.Copy(o,(ppluint64)mpg.start,(ppluint64)mpg.size,(ppluint64)pn);
		// Und am Ende noch den v1-Tag
		if (v1) {
			n.Write(v1,128);
		}
		n.Close();
		o.Close();
		if (!CFile::DeleteFile(Filename)) {
			if (v1) free(v1);
			return 0;
		}
		CFile::RenameFile(tmpfile,Filename);
	}
	if (v1)free(v1);
	return 1;
}

/*!\brief Name des Interpreten auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Interpreten aus dem Frame "TPE1"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Namen des Interpreten zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetArtist() const
{
	CString r;
	CID3Frame *frame=FindFrame("TPE1");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Name des Titels auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Titels aus dem Frame "TIT2"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Titel des Songs zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetTitle() const
{
	CString r;
	CID3Frame *frame=FindFrame("TIT2");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Genre auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Musikgenres aus dem Frame "TCON"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Namen des Genres zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetGenre() const
{
	CString r;
	CID3Frame *frame=FindFrame("TCON");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Kommentar auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Kommentar des Titels aus dem Frame "COMM"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Kommentar zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetComment() const
{
	return GetComment("");
}

/*!\brief Kommentar auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Kommentar des Titels aus dem Frame "COMM"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Kommentar zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetComment(const CString &description) const
{
	CString r, desc, best;
	CID3Frame *frame=firstFrame;
	CID3Frame *bestframe=NULL;
	size_t size;
	while (frame) {
		if(strcmp(frame->ID,"COMM")==0) {
			// Wir haben ein Comment-Feld
			int encoding=Peek8(frame->data);
			if (encoding<4) {
				if (encoding==0 || encoding==3) {
					size=strlen(frame->data+4);
					Decode(frame,4,encoding,desc);
					Decode(frame,5+size,encoding,r);
				} else {
					size=strlen16(frame->data+4)*2;
					Decode(frame,4,encoding,desc);
					Decode(frame,6+size,encoding,r);
				}
				//printf ("Found COMM-Frame with desc=%s => %s\n",(const char *)desc,(const char*)r);
				if (description.Len()>0 && description==desc) return r;
				if (!bestframe) bestframe=frame;
				if (description.Len()==0 && desc.Len()==0) best=r;
			}


		}
		frame=frame->nextFrame;
	}
	if (bestframe) {
		return best;
	} else {
		r.Clear();
	}
	return r;
}

/*!\brief Name des Remixers oder des Mixes auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Names des Remixers oder des Mixes aus dem Frame "TPE4"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Namen des Remixers oder Mixes zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetRemixer() const
{
	CString r;
	CID3Frame *frame=FindFrame("TPE4");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Erscheinungsjahr auslesen
 *
 * \desc
 * Mit dieser Funktion wird das Erscheinungsjahr des Titels aus dem Frame "TYER"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Erscheinungsjahr des Titels zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetYear() const
{
	CString r;
	CID3Frame *frame=FindFrame("TYER");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Name des Plattenlabels auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Name des Plattenlabels aus dem Frame "TPUB"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Namen des Plattenlabels zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetLabel() const
{
	CString r;
	CID3Frame *frame=FindFrame("TPUB");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Name des Albums auslesen
 *
 * \desc
 * Mit dieser Funktion wird der Names des Albums aus dem Frame "TALB"
 * ausgelesen.
 *
 * @return Bei Erfolg wird ein String mit dem Namen des Albums zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetAlbum() const
{
	CString r;
	CID3Frame *frame=FindFrame("TALB");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

/*!\brief Track-Nummer auslesen
 *
 * \desc
 * Mit dieser Funktion wird die Tracknummer aus dem Frame "TRCK"
 * ausgelesen. Der String enthält eine Ziffer, die die Position des Tracks
 * innerhalb des Datenträgers oder Sets angibt. Optional kann durch Slash getrennt
 * noch die Gesamtzahl der Tracks folgen, also z.B. "4/9".
 *
 * @return Bei Erfolg wird ein String mit der Tracknummer zurückgegeben,
 * im Fehlerfall ein leerer String.
 */
CString CID3Tag::GetTrack() const
{
	CString r;
	CID3Frame *frame=FindFrame("TRCK");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}

CString CID3Tag::GetBPM() const
{
	CString r;
	CID3Frame *frame=FindFrame("TBPM");
	if (frame) CopyAndDecodeText(r,frame,0);
	return r;
}


int CID3Tag::GetPicture(int type, CBinary &bin) const
{
	CString name="APIC";
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(name.StrCmp(frame->ID)==0) {
			// Wir haben ein Picture
			ppl6::CString MimeType;
			int encoding=Peek8(frame->data);
			int offset=Decode(frame,1,0,MimeType);
			//printf ("Offset: %i, Type=%i, encoding=%i\n",offset, (int)Peek8(frame->data+offset),encoding);
			if ((int)Peek8(frame->data+offset)==type) {
				ppl6::CString Description;
				offset=Decode(frame,offset+1,encoding,Description);
				//printf ("Mimetype: >>>%s<<<, offset: %i\n",(const char*)MimeType,offset);
				bin.Copy(frame->data+offset,frame->Size-offset);
				return 1;
			}
			return 1;
		}
		frame=frame->nextFrame;
	}
	SetError(561);
	return 0;
}

int CID3Tag::SetPicture(int type, const CBinary &bin, const CString &MimeType)
{
	bool exists=false;
	CString name="APIC";
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(name.StrCmp(frame->ID)==0) {
			// Wir haben ein Picture
			ppl6::CString MimeType;
			//int encoding=Peek8(frame->data);
			int offset=Decode(frame,1,0,MimeType);
			if ((int)Peek8(frame->data+offset)==type) {
				exists=true;
				delete (frame->data);
				frame->data=NULL;
				frame->Size=0;
				break;
			}
		}
		frame=frame->nextFrame;
	}
	if (!frame) {
		frame=new CID3Frame(name);
		if (!frame) {
			SetError(2);
			return 0;
		}
	}
	frame->Flags=0;
	frame->data=(char*)malloc(bin.Size()+MimeType.Size()+4);
	if (!frame->data) {
		if (!exists) {
			delete frame;
			SetError(2);
			return 0;
		}
	}
	frame->Size=bin.Size()+MimeType.Size()+4;
	Poke8(frame->data,0);
	pokesz(frame->data+1,MimeType.GetPtr());
	Poke8(frame->data+2+MimeType.Size(),type);
	Poke8(frame->data+3+MimeType.Size(),0);
	memcpy(frame->data+4+MimeType.Size(),bin.GetPtr(),bin.Size());

	if (!exists) {
		AddFrame(frame);
	}
	return 1;
}

void CID3Tag::RemovePicture(int type)
{
	CString name="APIC";
	CID3Frame *frame=firstFrame;
	while (frame) {
		if(name.StrCmp(frame->ID)==0) {
			// Wir haben ein Picture
			ppl6::CString MimeType;
			//int encoding=Peek8(frame->data);
			int offset=Decode(frame,1,0,MimeType);
			if ((int)Peek8(frame->data+offset)==type) {
				delete (frame->data);
				frame->data=NULL;
				frame->Size=0;
				DeleteFrame(frame);
				return;
			}
		}
		frame=frame->nextFrame;
	}
}

/*!\brief Text eines Frames dekodieren und in einen String kopieren
 *
 * \desc
 * Mit dieser internen Funktion wird der Text eines Frames ab der Position
 * \p data und einer Länge von \p size Bytes zunächst nach UTF-8
 * konvertiert und dann im String \p s gespeichert.
 *
 * @param[out] s String, in dem der Text gespeichert werden soll
 * @param[in] data Speicherbereich, an dem der Text beginnt
 * @param[in] size Länge des Strings in Bytes
 */
void CID3Tag::CopyAndDecodeText(CString &s, CID3Frame *frame, int offset) const
{
	int encoding=Peek8(frame->data+offset);
	if (encoding<32) {
		Decode(frame,offset+1,encoding,s);
	} else {
		s.TranscodeText(frame->data+offset,frame->Size-offset,"ISO-8859-1","UTF-8");
	}
}


int CID3Tag::Decode(CID3Frame *frame, int offset, int encoding, CString &target) const
{
	size_t size=0;
	const char *data=frame->data+offset;
	if (encoding==0) {
		size=strlen(data);
		if (size+offset>frame->Size) size=frame->Size-offset;
		target.TranscodeText(data,size,"ISO-8859-1","UTF-8");
		return offset+size+1;
	} else if (encoding==1) {
		size=strlen16(data)*2;
		if (size+offset>frame->Size) size=frame->Size-offset;
		target.TranscodeText(data,size,"UTF-16","UTF-8");
		return offset+size+2;
	} else if (encoding==2) {
		size=strlen16(data)*2;
		if (size+offset>frame->Size) size=frame->Size-offset;
		target.TranscodeText(data,size,"UTF-BE","UTF-8");
		return offset+size+2;
	} else if (encoding==3) {
		size=strlen(data);
		if (size+offset>frame->Size) size=frame->Size-offset;
		target.Strncpy(data,size);
		return offset+size+1;
	} else if (encoding>31) {
		size=strlen(data);
		if (size+offset>frame->Size) size=frame->Size-offset;
		target.TranscodeText(data,size,"ISO-8859-1","UTF-8");
		return offset+size+1;
	}
	return offset+size+1;
}

}	// EOF namespace ppl6
