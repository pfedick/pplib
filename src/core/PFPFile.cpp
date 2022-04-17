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

namespace ppl6 {

/*!\class PFPChunk
 * \brief Daten-Chunk eines PFP-File Version 3
 *
 * In dieser Klasse werden die Daten eines einzelnen Chunks eines PFP-Files Version 3
 * gespeichert. Um einen Chunk zu erstellen, wird mit "new" eine neue Instanz
 * der Klasse generiert und mit der Funktion PFPFile::AddChunk einem PFP-File
 * hinzugefügt. Um einen existierenden Chunk zu löschen, kann einfach ein
 * delete auf das Objekt gemacht werden, oder dir Funktion PFPFile::DeleteChunk
 * aufgerufen werden.
 *
 * \see PFPFile
 *
 * \since Diese Klasse wurde in Version 6.1.0 eingeführt
 */

PFPChunk::PFPChunk()
/*!\brief Konstruktor des PFPChunk
 *
 * Der Konstruktor initialisiert den Datenbereich mit NULL und setzt als
 * Name "UNKN" ein. Ein derartiger Chunk würde durch die PFPFile-Klasse
 * nicht gespeichert.
 *
 * \since Version 6.1.0
 */
{
	strcpy(chunkname,"UNKN");
	data=NULL;
	size=0;
}

PFPChunk::~PFPChunk()
/*!\brief Destruktor des PFPChunk
 *
 * Der Destruktor sorgt dafür, dass der Datenbereich des Chunks freigegeben
 * wird, sofern er definiert wurde, und wenn er einem PFPFile zugeordnet
 * wurde, wird er daraus entfernt.
 *
 * \since Version 6.1.0
 */
{
	if (data) free(data);
	data=NULL;
}

int PFPChunk::SetName(const char *chunkname)
/*!\brief Name des Chunks setzen
 *
 * \desc
 * Mit dieser Funktion wird der Name eines Chunks definiert. Der Name muss
 * exakt 4 Byte lang sein und darf nur Großbuchstaben enthalten (es wird
 * eine automatische Konvertierung durchgeführt).
 *
 * \param chunkname Pointer auf einen 0-Terminierten String, der den Namen
 * des Chunks enthält.
 * \returns Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0.
 *
 * \since Version 6.1.0
 */
{
	if (!chunkname) {
		SetError(194,"int PFPChunk::SetName(const char *chunkname)");
		return 0;
	}
	int l=strlen(chunkname);
	if (l!=4) {
		SetError(423,"%i",l);
		return 0;
	}
	CString s=chunkname;
	s.UCase();
	strcpy(this->chunkname,s);
	return 1;
}

int PFPChunk::SetData(const void *ptr, int size)
/*!\brief Nutzdaten des Chunks setzen
 *
 * Mit dieser Funktion werden die Nutzdaten des Chunks angegeben. Die
 * Daten werden dabei in einen eigenen Speicherbereich kopiert.
 *
 * \param ptr Ein Pointer auf den Beginn der Daten
 * \param size Größe der Daten in Byte
 * \returns Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Ein entsprechender
 * Fehlercode wird gesetzt.
 * Ein Fehler könnte auftreten, wenn nicht mehr genug Speicher zur
 * Verfügung steht, um die Daten zu kopieren.
 *
 * \since Version 6.1.0
 */
{
	if (!ptr) {
		SetError(194,"int PFPChunk::SetData(==> void *ptr <==, int size)");
		return 0;
	}
	if (data) free(data);
	data=NULL;
	this->size=0;
	data=malloc(size);
	if (!data) {
		SetError(2,"int PFPChunk::SetData(void *ptr, int size), %i Bytes",size);
		return 0;
	}
	memcpy(data,ptr,size);
	this->size=size;
	return 1;
}

int PFPChunk::SetData(const char *s)
/*!\brief String als Nutzdaten des Chunks setzen
 *
 * Mit dieser Funktion werden die Nutzdaten des Chunks angegeben. Die
 * Daten werden dabei in einen eigenen Speicherbereich kopiert.
 *
 * \param s Pointer auf einen 0-Terminierten String, der als
 * Nutzdaten übernommen werden soll.
 * \returns Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Ein entsprechender
 * Fehlercode wird gesetzt.
 * Ein Fehler könnte auftreten, wenn nicht mehr genug Speicher zur
 * Verfügung steht, um die Daten zu kopieren.
 *
 * \since Version 6.1.0
 */
{
	if (!s) {
		SetError(194,"int PFPChunk::SetData(const char *s)");
		return 0;
	}
	int l=(int) strlen(s);
	return SetData((void*)s,l+1);
}

int PFPChunk::SetData(const CString &s)
/*!\brief String als Nutzdaten des Chunks setzen
 *
 * Mit dieser Funktion werden die Nutzdaten des Chunks angegeben. Die
 * Daten werden dabei in einen eigenen Speicherbereich kopiert.
 *
 * \param s Ein CString, dessen Inhalt als
 * Nutzdaten übernommen werden soll.
 * \returns Bei Erfolg gibt die Funktion 1 zurück, im Fehlerfall 0. Ein entsprechender
 * Fehlercode wird gesetzt.
 * Ein Fehler könnte auftreten, wenn nicht mehr genug Speicher zur
 * Verfügung steht, um die Daten zu kopieren.
 *
 * \since Version 6.1.0
 */
{
	return SetData((void*)s.GetPtr(),s.Size()+1);
}

int PFPChunk::Size()
/*!\brief Größe des Chunks auslesen
 *
 * Mit dieser Funktion kann die Größe des Chunks ausgelesen werden.
 *
 * \returns Enthält der Chunk Daten, wird die Anzahl Bytes zurückgegeben.
 * Ist er leer, wird 0 zurückgegeben und der Fehlercode 424 gesetzt.
 *
 * \since Version 6.1.0
 */
{
	if (!size) SetError(424);
	return size;
}

const void *PFPChunk::Data()
/*!\brief Pointer auf die Daten des Chunks holen
 *
 * Mit dieser Funktion wir der Pointer auf die Nutzdaten des Chunks
 * ausgelesen.
 *
 * \returns Enthält der Chunk Daten, wird ein Pointer darauf zurückgegeben.
 * Wenn nicht, wird NULL zurückgegeben und der Fehlercode 424 gesetzt.
 *
 * \since Version 6.1.0
 */
{
	if (!data) SetError(424);
	return data;
}

const char *PFPChunk::Name()
/*!\brief Pointer auf den Namen des Chunks holen
 *
 * Diese Funktion liefert einen Pointer auf den Namen des Chunks zurück. Dieser ist
 * immer 4 Byte groß und mit einem Nullbyte abgeschlossen.
 *
 * \returns Pointer auf den Namen des Chunks.
 *
 * \since Version 6.1.0
 */
{
	return (const char*)chunkname;
}




/*!\class PFPFile
 * \brief Klasse zum Lesen und schreiben von PFP-Files Version 3
 *
 * Mit dieser Klasse können Dateien mit "PFP-File"-Header der Version 3 gelesen und geschrieben werden.
 * Mit Version 3 wurde ein mehr generisches Format definiert, als in den beiden Vorgängerversionen.
 * Jedes File, ganz gleich welchen Inhalt es hat, hat bis zum Ende den gleichen Aufbau. Wichtigste
 * Neuerung dabei sind die sogenannten Chunks. Ein File kann aus bliebig vielen Chunks bestehen.
 * Diese werden von der Klasse PFPChunk abgeleitet, bekommen einen Namen und einen beliebigen
 * Inhalt. Diese können dann mit PFPFile::Add in das File hinzugefügt werden.
 *
 * \copydoc PFPFileVersion3
 *
 * \since Version 6.1.0
 *
 */

/*!\var PFPFile::Chunks
 * \brief Verwaltung aller Chunks in einer Liste
 *
 * \since Version 6.1.0
 */

/*!\var PFPFile::id
 * \brief enthält die ID des Chunks. Die ID ist immer 4 Byte lang, gefolgt von einem 0-Byte
 *
 * \since Version 6.1.0
 */

/*!\var PFPFile::mainversion
 * \brief Hier wird die Hauptversion des Files gespeichert
 *
 * \since Version 6.1.0
 */

/*!\var PFPFile::subversion
 * \brief Hier wird die Unterversion des Files gespeichert
 *
 * \since Version 6.1.0
 */

/*!\var PFPFile::comp
 * \brief Kompressions-Flag
 *
 * In dieser Variable wird die Art der Komprimierung gespeichert:
 * - 0 = keine Komprimierung
 * - 1 = Zlib
 * - 2 = Bzip2
 *
 * \since Version 6.1.0
 */

/*!\var PFPFile::findchunk
 * \brief Chunkname für FindFirstChunk und FindNextChunk
 *
 * \since Version 6.1.0
 */


PFPFile::PFPFile()
/*!\brief Konstruktor der Klasse
 *
 * Hier werden einige interne Variablen initialisert, die ID wird auf "UNKN" gesetzt, Version
 * auf 0 und Kompression abgeschaltet
 *
 * \since Version 6.1.0
 */
{
	strcpy(id,"UNKN");
	mainversion=subversion=0;
	comp=CCompression::Algo_NONE;
}

PFPFile::~PFPFile()
/*!\brief Destruktor der Klasse
 *
 * Der Destruktor sorgt dafür, dass sämtlicher von der Klasse allokierter Speicher einschließlich
 * aller geladener Chunks freigegeben wird.
 *
 * \since Version 6.1.0
 */
{
	Clear();
}

void PFPFile::Clear()
/*!\brief Inhalt der Klasse löschen
 *
 * Mit dieser Funktion werden alle Chunks im Speicher freigegeben und die Klasse auf den
 * Ursprungszustand zurückgesetzt, das heisst sie ist anschließend leer
 *
 * \since Version 6.1.0
 */
{
	Chunks.Clear(true);
	strcpy(id,"UNKN");
	mainversion=subversion=0;
	comp=CCompression::Algo_NONE;
}

int PFPFile::SetVersion(int main, int sub)
/*!\brief Version setzen
 *
 * Mit dieser Funktion wird die Version des PFP-Files gesetzt
 *
 * \param main Hauptversion
 * \param sub Unterversion
 * \returns Die Funktion liefert 1 zurück, wenn die Version erfolgreich gesetzt werden konnte,
 * oder 0 bei einem Fehler. Ein Fehler kann nur auftreten, wenn eine der Versionskomponenten
 * kleiner 0 oder größer 255 ist.
 *
 * \remarks
 * Haupt- und Unterversion werden jeweils in einem einzelnen Byte gespeichert. Daher darf die
 * Version nicht größer als 255 werden.
 *
 * \since Version 6.1.0
 */
{
	if (main<0 || main>255 || sub<0 || sub>255) {
		SetError(431);
		return 0;
	}
	mainversion=(ppluint8)(main&0xff);
	subversion=(ppluint8)(sub&0xff);
	return 1;
}

int PFPFile::SetId(const char *id)
/*!\brief ID des PFP-Files setzen
 *
 * Mit dieser Version wird die ID des PFP-Files festgelegt. Eine ID muss zwingend 4 Byte lang
 * sein und darf nur US-ASCII-Zeichen enthalten.
 *
 * \param id Pointer auf einen 4-Byte langen String, der mit 0 terminiert ist.
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, sonst false (0).
 *
 * \since Version 6.1.0
 */
{
	if (!id) {
		SetError(194,"int PFPFile::SetId(const char *id)");
		return 0;
	}
	int l=strlen(id);
	if (l!=4) {
		SetError(429,"%i",l);
		return 0;
	}
	strcpy(this->id,id);
	return 1;
}

int PFPFile::Save(const char *filename)
/*!\brief PFP-File speichern
 *
 * Mit dieser Funktion wird der Inhalt der PFPFile-Klasse in eine Datei geschrieben.
 * Dabei wird der Header und sämtliche Chunks zusammengefasst, gegebenenfalls komprimiert
 * (siehe PFPFile::SetCompression) und im Filesystem gespeichert. Der genaue Aufbau der Datei
 * wird weiter unten beschrieben.
 *
 * \param filename Die Funktion bekommt als einzigen Parameter einen Pointer auf den Dateinamen.
 * Es ist zu beachten, dass eine eventuell vorhandene gleichnamige Datei überschrieben wird.
 * \returns Konnte die Datei erfolgreich gespeichert werden, gibt die Funktion true (1) zurück,
 * sonst false (0). Ein entsprechender Fehlercode, der Auskunft über die Art des Fehlers gibt,
 * wird gesetzt.
 *
 * \remarks
 * Die Funktion stellt sicher, dass die Chunks in einer bestimmten Reihenfolge geschrieben
 * werden. Die vordefinierten Chunks mit Name, Author, Copyright und Beschreibung werden in
 * jedem Fall zuerst gespeichert, dann die restlichen Chunks.
 *
 * \par Aufbau der PFP-Datei
 *
 * \copydoc PFPFileVersion3
 *
 * \since Version 6.1.0
 */
{
	// Gespeichert wird nur, wenn die Datei Chunks enthält
	if (Chunks.Num()==0) {
		SetError(425,"Datei kann nicht gespeichert werden");
		return 0;
	}
	CFile ff;
	// Wir benötigen zuerst die Gesamtgröße aller Chunks
	int size=24;
	Chunks.Reset();
	PFPChunk *chunk;
	while((chunk=(PFPChunk*)Chunks.GetNext())) {
		size+=8;
		size+=chunk->size;
	}
	// plus ENDF-Chunk
	size+=8;

	// Datei zusammenbauen
	char *p=(char*)malloc(size);
	if (!p) {
		SetError(2);
		return 0;
	}
	int hsize=24;
	memcpy(p,"PFP-File",8);
	poke8(p+8,3);
	poke8(p+9,hsize);
	memcpy(p+10,id,4);
	poke8(p+15,mainversion);
	poke8(p+14,subversion);
	poke8(p+16,comp);
	poke8(p+17,0);
	poke8(p+18,0);
	poke8(p+19,0);
	poke32(p+20,(ppluint32)GetTime());

	int pp=hsize;
	// Chunks zusammenfassen
	// Zuerst die vordefinierten, die wir am Anfang des Files wollen
	Chunks.Reset();
	chunk=FindFirstChunk("NAME");
	if (chunk) {
		memcpy(p+pp,chunk->chunkname,4);
		poke32(p+pp+4,chunk->size+8);
		memcpy(p+pp+8,chunk->data,chunk->size);
		pp+=8;
		pp+=chunk->size;
	}
	chunk=FindFirstChunk("AUTH");
	if (chunk) {
		memcpy(p+pp,chunk->chunkname,4);
		poke32(p+pp+4,chunk->size+8);
		memcpy(p+pp+8,chunk->data,chunk->size);
		pp+=8;
		pp+=chunk->size;
	}
	chunk=FindFirstChunk("DESC");
	if (chunk) {
		memcpy(p+pp,chunk->chunkname,4);
		poke32(p+pp+4,chunk->size+8);
		memcpy(p+pp+8,chunk->data,chunk->size);
		pp+=8;
		pp+=chunk->size;
	}
	chunk=FindFirstChunk("COPY");
	if (chunk) {
		memcpy(p+pp,chunk->chunkname,4);
		poke32(p+pp+4,chunk->size+8);
		memcpy(p+pp+8,chunk->data,chunk->size);
		pp+=8;
		pp+=chunk->size;
	}


	// Restliche Chunks
	Chunks.Reset();
	while((chunk=(PFPChunk*)Chunks.GetNext())) {
		CString cn=chunk->chunkname;
		// Vordefinierte Chunks müssen übergangen werden, da diese weiter oben schon
		// ausgelesen wurden
		if (cn!="NAME" && cn!="AUTH" && cn!="DESC" && cn!="COPY") {
			memcpy(p+pp,chunk->chunkname,4);
			poke32(p+pp+4,chunk->size+8);
			if (chunk->size) {
				memcpy(p+pp+8,chunk->data,chunk->size);
				pp+=chunk->size;
			}
			pp+=8;
		}
	}
	memcpy(p+pp,"ENDF",4);
	poke32(p+pp+4,0);
	pp+=8;

	void *save=NULL;
	int savesize=pp-hsize;
	// Komprimierung?
	CCompression c;
	if (comp) {
		size_t dstlen=savesize+64;
		save=malloc(dstlen);
		if (!save) {
			free(p);
			SetError(2);
			return 0;
		}
		c.Init(comp,CCompression::Level_High);
		if (!c.Compress(save,&dstlen,p+hsize,savesize)) {
			free(save);
			free(p);
			return 0;
		}
		savesize=dstlen;
	}
	if (!ff.Open((char*)filename,"wb")) {
		PushError();
		if (save) free(save);
		free(p);
		PopError();
		return 0;
	}
	ff.Write(p,hsize);
	if (comp) {
		char t[8];
		poke32(t,pp-hsize);
		poke32(t+4,savesize);
		ff.Write(t,8);
		ff.Write(save,savesize);
	} else {
		ff.Write(p+hsize,pp-hsize);
	}
	ff.Close();
	if (save) free(save);
	free(p);
	return 1;
}

int PFPFile::AddChunk(PFPChunk *chunk)
/*!\brief Chunk hinzufügen
 *
 * Mit dieser Funktion wird ein neuer Chunk in die Klasse hinzugefügt. Der Chunk muss von der
 * Anwendung mit "new" erstellt worden sein, einen Namen haben. Ist dies nicht der Fall,
 * gibt die Funktion eine Fehlermeldung zurück.
 *
 * Sobald der Chunk mit AddChunk an die PFPFile-Klasse übergeben wurde, wird er von der Klasse
 * verwaltet und gegebenenfalls auch gelöscht. Die Anwendung braucht kein "delete" darauf zu
 * machen.
 *
 * \param chunk Pointer auf den hinzuzufügenden Chunk
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, sonst false (0). Ein passender Fehlercode
 * wird gesetzt.
 * \remarks Es ist möglich mehrere Chunks mit gleichem Namen hinzuzufügen. Der Chunk wird nur in der
 * Klasse hinzugefügt, nicht aber in die Datei geschrieben. Zum Speichern muss explizit die Funktion
 * PFPFile::Save aufgerufen werden.
 *
 * \example
 * \code
 * void *ptr=xxxxx;    // Pointer auf die Daten
 * int size=xxxx;      // Größe der Daten in Byte
 * ppl6::PFPFile file;
 * ppl6::PFPChunk *chunk=new ppl6::PFPChunk;
 * chunk->SetName("DATA");
 * chunk->SetData(ptr,size);
 * file.AddChunk(chunk);
 * \endcode
 *
 * \since Version 6.1.0
 */
{
	if (!chunk) {
		SetError(194,"int PFPFile::AddChunk(PFPChunk *chunk)");
		return 0;
	}
	if (strcmp(chunk->chunkname,"UNKN")==0) {
		SetError(426);
		return 0;
	}
	if (!Chunks.Add(chunk)) {
		ExtendError(427);
		return 0;
	}
	return 1;
}

int PFPFile::DeleteChunk(PFPChunk *chunk)
/*!\brief Bestimmten Chunk löschen
 *
 * Mit dieser Funktion wird ein bestimmter Chunk aus der Klasse gelöscht.
 *
 * \param chunk Pointer auf den zu löschenden Chunk
 * \returns Die Funktion liefert bei Erfolg true (1) zurück, sonst false. Ein Fehler kann nur
 * auftreten, wenn der Chunk garnicht im PFPFile vorhanden war.
 * \remarks
 * Alternativ kann auch einfach ein "delete" auf den Chunk gemacht werden.
 *
 * \see PFPFile::DeleteChunk(const char *name)
 *
 * \since Version 6.1.0
 */
{
	if (!chunk) {
		SetError(194,"int PFPFile::DeleteChunk(PFPChunk *chunk)");
		return 0;
	}
	return Chunks.Delete(chunk);
}

int PFPFile::DeleteChunk(const char *chunkname)
/*!\brief Chunk nach Namen löschen
 *
 * Mit dieser Funktion werden alle Chunks gelöscht, die den angegebenen Namen haben
 *
 * \param chunkname Pointer auf den Namen des Chunks
 * \returns Die Funktion liefert true (1) zurück, wenn mindestens 1 Chunk gelöscht
 * wurde, sonst false (0).
 * \remarks
 *
 * \see PFPFile::DeleteChunk(PFPChunk *chunk)
 *
 * \since Version 6.1.0
 */
{
	if (!chunkname) {
		SetError(194,"int PFPFile::DeleteChunk(const char *chunkname)");
		return 0;
	}
	PFPChunk *chunk;
	int ret=0;
	while ((chunk=FindFirstChunk(chunkname))) {
		delete chunk;
		ret=1;
	}
	if (!ret) {
		SetError(428);
		return 0;
	}
	return 1;
}

PFPChunk *PFPFile::FindFirstChunk(const char *chunkname)
/*!\brief Ersten Chunk mit einem bestimmten Namen finden
 *
 * Mit dieser und der Funktion PFPFile::FindNextChunk kann man sich durch alle Chunks mit einem
 * bestimmten Namen durchhangeln.
 *
 * \param chunkname Pointer auf den Namen des Chunks
 * \returns Bei Erfolg liefert die Funktion einen Pointer auf den ersten gefundenen Chunk zurück.
 * Wurde kein passender Chunk gefunden, wird NULL zurückgegeben und ein entsprechender Fehlercode
 * gesetzt.
 * \see
 * - PFPFile::FindFirstChunk
 * - PFPFile::FindNextChunk
 * - PFPFile::Reset
 * - PFPFile::GetFirst
 * - PFPFile::GetNext
 *
 * \since Version 6.1.0
 */
{
	if (!chunkname) {
		SetError(194,"int PFPFile::DeleteChunk(const char *chunkname)");
		return 0;
	}
	Chunks.Reset();
	return FindNextChunk(chunkname);
}

PFPChunk *PFPFile::FindNextChunk(const char *chunkname)
/*!\brief Nächsten Chunk mit einem bestimmten Namen finden
 *
 * Mit dieser und der Funktion PFPFile::FindFirstChunk kann man sich durch alle Chunks mit einem
 * bestimmten Namen durchhangeln.
 *
 * \param chunkname Optionaler Pointer auf den Namen des Chunks. Wurde zuvor bereits
 * PFPFile::GetFirstChunk aufgerufen, muss kein Name angegeben werden.
 * \returns Bei Erfolg liefert die Funktion einen Pointer auf den nächsten gefundenen Chunk zurück.
 * Wurde kein passender Chunk gefunden, wird NULL zurückgegeben und ein entsprechender Fehlercode
 * gesetzt.
 * \see
 * - PFPFile::FindFirstChunk
 * - PFPFile::FindNextChunk
 * - PFPFile::Reset
 * - PFPFile::GetFirst
 * - PFPFile::GetNext
 *
 * \since Version 6.1.0
 */
{
	PFPChunk *chunk;
	if (chunkname) {
		if (findchunk.StrCmp((char*)chunkname)!=0) {
			findchunk=chunkname;
			findchunk.UCase();
			Chunks.Reset();
		}
	}
	if (findchunk.Len()==0) {
		SetError(430,"PFPChunk *PFPFile::FindNextChunk(const char *chunkname)");
		return NULL;
	}
	while ((chunk=(PFPChunk*)Chunks.GetNext())) {
		if (findchunk.StrCmp(chunk->chunkname)==0) return chunk;
	}
	ExtendError(428);
	return NULL;
}

void PFPFile::Reset()
/*!\brief Zeiger zum Durchwandern der Chunks zurücksetzen
 *
 * Mit dieser Funktion wird der Zeiger, der beim Durchwandern der Chunks mit den Funktionen
 * FindNextChunk und GetNext verwendet wird, wieder auf den Anfang gesetzt.
 *
 * \see
 * - PFPFile::FindFirstChunk
 * - PFPFile::FindNextChunk
 * - PFPFile::Reset
 * - PFPFile::GetFirst
 * - PFPFile::GetNext
 *
 * \since Version 6.1.0
 */
{
	Chunks.Reset();
}

PFPChunk *PFPFile::GetFirst()
/*!\brief Pointer auf ersten Chunk holen
 *
 * Diese Funktion liefert einen Pointer auf den ersten Chunk in der Datei zurück.
 *
 * \returns Pointer auf den ersten Chunk oder NULL, wenn es keine Chunks gibt.
 * \see
 * - PFPFile::FindFirstChunk
 * - PFPFile::FindNextChunk
 * - PFPFile::Reset
 * - PFPFile::GetFirst
 * - PFPFile::GetNext
 *
 * \since Version 6.1.0
 */
{
	Chunks.Reset();
	return GetNext();
}

PFPChunk *PFPFile::GetNext()
/*!\brief Pointer auf nächsten Chunk holen
 *
 * Diese Funktion liefert einen Pointer auf den nächsten Chunk in der Datei zurück.
 *
 * \returns Pointer auf den nächsten Chunk oder NULL, wenn es keine weiteren Chunks gibt.
 *
 * \see
 * - PFPFile::FindFirstChunk
 * - PFPFile::FindNextChunk
 * - PFPFile::Reset
 * - PFPFile::GetFirst
 * - PFPFile::GetNext
 *
 * \since Version 6.1.0
 */
{
	PFPChunk *c=(PFPChunk*)Chunks.GetNext();
	if (c) return c;
	ExtendError(428);
	return NULL;
}

void PFPFile::List()
/*!\brief Chunks auf STDOUT auflisten
 *
 * Diese Funktion listet die Namen und Größen aller Chunks auf STDOUT aus.
 *
 * \since Version 6.1.0
 */
{
	printf("PFP-File Version 3 ============================================\n");
	printf("ID: %s, Version: %i.%i, Komprimierung: ",id,mainversion,subversion);
	switch(comp) {
		case 0: printf ("keine\n");
			break;
		case 1: printf ("Zlib\n");
			break;
		case 2: printf ("Bzip2\n");
			break;
		default: printf ("unbekannt\n");
			break;
	}
	const char *tmp;
	if ((tmp=GetName())) printf("Name:        %s\n",tmp);
	if ((tmp=GetAuthor())) printf("Author:      %s\n",tmp);
	if ((tmp=GetDescription())) printf("Description: %s\n",tmp);
	if ((tmp=GetCopyright())) printf("Copyright:   %s\n",tmp);

	Chunks.Reset();
	PFPChunk *chunk;
	printf ("\nChunks:\n");
	while((chunk=(PFPChunk*)Chunks.GetNext())) {
		printf ("  %s: %i Bytes\n",chunk->chunkname,chunk->size);
	}
	printf("===============================================================\n");
}

int PFPFile::SetCompression(CCompression::Algorithm type)
/*!\brief Kompression einstellen
 *
 * Mit dieser Funktion wird festgelegt ob und welche Kompression beim Speichern verwendet werden
 * soll.
 *
 * \param type Ein Wert, der die Art der Kompression angibt. Mögliche Werte sind:
 * - CCompression::Algo_NONE - Keine Komprimierung
 * - CCompression::Algo_ZLIB - Komprimierung mit Zlib
 * - CCompression::Algo_BZIP2 - Komprimierung mit Bzip2
 * \returns Die Funktion liefert true(1) zurück, wenn die Komprimierungsmethode erfolgreich
 * gesetzt werden konnt, sonst false (0)
 *
 * \since Version 6.1.0
 */
{
	if (type>2 || type<0) {
		SetError(67,"%i",type);
		return 0;
	}
	comp=type;
	return 1;
}

int PFPFile::SetParam(const char *chunkname, const char *data)
/*!\brief Interne Funktion zum Speichern von vordefinierten Chunks
 *
 * Diese Funktion wird intern verwendet, um die vordefinierten Text-Chunks zu speichern. Sie
 * stellt sicher, dass jeder Chunk nur einmal vorkommt.
 *
 * \param chunkname Pointer auf den Namen des Chunks.
 * \param data Pointer auf den zu setzenden Text-String
 * \returns Bei Erfolg gibt die Funktion true (1) zurück, sonst false (0).
 * \see Die Funktion wird intern von folgenden Funktionen aufgerufen:
 * - PFPFile::SetName
 * - PFPFile::SetAuthor
 * - PFPFile::SetDescription
 * - PFPFile::SetCopyright
 *
 * \since Version 6.1.0
 */
{
	DeleteChunk(chunkname);
	PFPChunk *c=new PFPChunk;
	if (!c->SetName(chunkname)) {
		PushError();
		delete c;
		PopError();
		return 0;
	}
	if (!c->SetData(data)) {
		PushError();
		delete c;
		PopError();
		return 0;
	}
	if (!AddChunk(c)) {
		PushError();
		delete c;
		PopError();
		return 0;
	}
	return 1;
}

int PFPFile::SetAuthor(const char *author)
/*!\brief Author setzen
 *
 * Mit dieser Funktion wird automatisch ein Author-Chunk ("AUTH") angelegt. Dabei ist sichergestellt,
 * dass der Chunk nur ein einziges mal in der Datei vorkommt.
 *
 * \param author Pointer auf einen Null-terminierten String mit dem Namen des Authors
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, sonst false (0). Ein entsprechender
 * Fehlercode wird gesetzt.
 * \see
 * - PFPFile::SetAuthor
 * - PFPFile::SetCopyright
 * - PFPFile::SetDescription
 * - PFPFile::SetName
 *
 * \since Version 6.1.0
 */
{
	if (!author) {
		SetError(194,"int PFPFile::SetAuthor(const char *author)");
		return 0;
	}
	return SetParam("AUTH",author);
}

int PFPFile::SetCopyright(const char *copy)
/*!\brief Copyright setzen
 *
 * Mit dieser Funktion wird automatisch ein Copyright-Chunk ("COPY") angelegt. Dabei ist sichergestellt,
 * dass der Chunk nur ein einziges mal in der Datei vorkommt.
 *
 * \param copy Pointer auf einen Null-terminierten String mit dem Copyright-Text
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, sonst false (0). Ein entsprechender
 * Fehlercode wird gesetzt.
 *
 * \see
 * - PFPFile::SetAuthor
 * - PFPFile::SetCopyright
 * - PFPFile::SetDescription
 * - PFPFile::SetName
 *
 * \since Version 6.1.0
 */
{
	if (!copy) {
		SetError(194,"int PFPFile::SetCopyright(const char *copy)");
		return 0;
	}
	return SetParam("COPY",copy);
}

int PFPFile::SetDescription(const char *descr)
/*!\brief Description setzen
 *
 * Mit dieser Funktion wird automatisch ein Description-Chunk ("DESC") angelegt. Dabei ist sichergestellt,
 * dass der Chunk nur ein einziges mal in der Datei vorkommt.
 *
 * \param descr Pointer auf einen Null-terminierten String mit der Beschreibung
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, sonst false (0). Ein entsprechender
 * Fehlercode wird gesetzt.
 * \see
 * - PFPFile::SetAuthor
 * - PFPFile::SetCopyright
 * - PFPFile::SetDescription
 * - PFPFile::SetName
 *
 * \since Version 6.1.0
 */
{
	if (!descr) {
		SetError(194,"int PFPFile::SetDescription(const char *descr)");
		return 0;
	}
	return SetParam("DESC",descr);
}

int PFPFile::SetName(const char *name)
/*!\brief Name
 *
 * Mit dieser Funktion wird automatisch ein Namens-Chunk ("NAME") angelegt. Dabei ist sichergestellt,
 * dass der Chunk nur ein einziges mal in der Datei vorkommt.
 *
 * \param name Pointer auf einen Null-terminierten String mit dem Namen des Files
 * \returns Bei Erfolg liefert die Funktion true (1) zurück, sonst false (0). Ein entsprechender
 * Fehlercode wird gesetzt.
 * \see
 * - PFPFile::SetAuthor
 * - PFPFile::SetCopyright
 * - PFPFile::SetDescription
 * - PFPFile::SetName
 *
 * \since Version 6.1.0
 */
{
	if (!name) {
		SetError(194,"int PFPFile::SetName(const char *name)");
		return 0;
	}
	return SetParam("NAME",name);
}

const char *PFPFile::GetName()
/*!\brief Pointer auf den Namen holen
 *
 * Diese Funktion liefert einen Pointer auf den Namen des Files zurück.
 * \returns Pointer auf den Namen oder NULL, wenn es keinen "NAME"-Chunk in der Datei gibt.
 * \see
 * - PFPFile::GetAuthor
 * - PFPFile::GetCopyright
 * - PFPFile::GetDescription
 * - PFPFile::GetName
 *
 * \since Version 6.1.0
 */
{
	PFPChunk *chunk=FindFirstChunk("NAME");
	if (!chunk) {
		SetError(428,"NAME");
		return NULL;
	}
	return (const char*) chunk->data;
}

const char *PFPFile::GetDescription()
/*!\brief Pointer auf die Description holen
 *
 * Diese Funktion liefert einen Pointer auf die Beschreibung zurück.
 * \returns Pointer auf die Beschreibung oder NULL, wenn es keinen "DESC"-Chunk in der Datei gibt.
 *
 * \see
 * - PFPFile::GetAuthor
 * - PFPFile::GetCopyright
 * - PFPFile::GetDescription
 * - PFPFile::GetName
 *
 * \since Version 6.1.0
 */
{
	PFPChunk *chunk=FindFirstChunk("DESC");
	if (!chunk) {
		SetError(428,"DESC");
		return NULL;
	}
	return (const char*) chunk->data;
}

const char *PFPFile::GetAuthor()
/*!\brief Pointer auf den Author holen
 *
 * Diese Funktion liefert einen Pointer auf den Author zurück
 * \returns Pointer auf den Author oder NULL, wenn es keinen "AUTH"-Chunk in der Datei gibt.
 * \see
 * - PFPFile::GetAuthor
 * - PFPFile::GetCopyright
 * - PFPFile::GetDescription
 * - PFPFile::GetName
 *
 * \since Version 6.1.0
 */
{
	PFPChunk *chunk=FindFirstChunk("AUTH");
	if (!chunk) {
		SetError(428,"AUTH");
		return NULL;
	}
	return (const char*) chunk->data;
}

const char *PFPFile::GetCopyright()
/*!\brief Pointer auf den Copyright-String holen
 *
 * Diese Funktion liefert einen Pointer auf den Copyright-Text des Files zurück.
 * \returns Pointer auf das Copyright oder NULL, wenn es keinen "COPY"-Chunk in der Datei gibt.
 * \see
 * - PFPFile::GetAuthor
 * - PFPFile::GetCopyright
 * - PFPFile::GetDescription
 * - PFPFile::GetName
 *
 * \since Version 6.1.0
 */
{
	PFPChunk *chunk=FindFirstChunk("COPY");
	if (!chunk) {
		SetError(428,"COPY");
		return NULL;
	}
	return (const char*) chunk->data;
}

void PFPFile::GetVersion(int *main, int *sub)
/*!\brief Version auslesen
 *
 * Mit dieser Funktion wird die Version der Datei in die beiden Parameter kopiert
 * \param[out] main Pointer auf eine Integer-Variable, in der die Hauptversion geschrieben
 * werden soll
 * \param[out] sub Pointer auf eine Integer-Variable, in der die Unterversion geschrieben
 * werden soll
 *
 * \since Version 6.1.0
 *
 */
{
	if (main) *main=mainversion;
	if (sub) *sub=subversion;
}

const char *PFPFile::GetID()
/*!\brief ID auslesen
 *
 * Diese Funktion liefert einen Pointer auf die ID der Datei zurück
 * \returns Pointer auf die ID der Datei. Diese ist immer 4 Byte groß und mit einem 0-Byte
 * terminiert
 *
 * \since Version 6.1.0
 */
{
	return id;
}

int PFPFile::GetMainVersion()
/*!\brief Hauptversion auslesen
 *
 * Mit dieser Funktion wird die Hauptversion der Datei ausgelesen.
 * \returns Hauptversion als Interger
 *
 * \since Version 6.1.0
 */
{
	return mainversion;
}

int PFPFile::GetSubVersion()
/*!\brief Unterversion auslesen
 *
 * Mit dieser Funktion wird die Unterversion der Datei ausgelesen.
 * \returns Unterversion als Interger
 *
 * \since Version 6.1.0
 *
 */
{
	return subversion;
}

CCompression::Algorithm PFPFile::GetCompression()
/*!\brief Kompressionsverfahren auslesen
 *
 * Mit dieser Funktion wird das eingestellte Kompressionsverfahren
 * ausgelesen.
 * \returns ID des Kompressionsverfahrens
 *
 * \see PFPFile::SetCompression
 * \since Version 6.1.0
 *
 */
{
	return comp;
}

/*!\brief Prüfen, ob es sich um ein PFP-File handelt
 *
 * \desc
 * Diese Funktion prüft, ob es sich bei der Datei mit dem Namen \p file um eine Datei
 * im \ref PFPFileVersion3 PFP-Format Version 3 handelt. Ist dies der Fall, wird deren
 * ID und Version eingelesen.
 *
 * @param file Dateiname
 * @return Gibt \c true zurück, wenn es sich um eine Datei im PFP-Format handelt. Deren
 * ID kann anschließend mit PFPFile::GetID ausgelesen werden, Version mit PFPFile::GetVersion bzw.
 * PFPFile::GetMainVersion und PFPFile::GetSubVersion. Handelt es sich nicht um eine Datei
 * im PFP-Format, gibt die Funktion \c false zurück. Es wird keine Exception geworfen.
 */
bool PFPFile::Ident(const CString &file)
{
	CFile ff;
	try {
		ff.Open(file,"rb");
	} catch (...) {
		return false;
	}
	return Ident(ff);
}

/*!\brief Prüfen, ob es sich um ein PFP-File handelt
 *
 * \desc
 * Diese Funktion prüft, ob es sich bei der geöffneten Datei \p ff um eine Datei
 * im \ref PFPFileVersion3 PFP-Format Version 3 handelt. Ist dies der Fall, wird deren
 * ID und Version eingelesen.
 *
 * @param ff Referenz auf eine geöffnete Datei
 * @return Gibt \c true zurück, wenn es sich um eine Datei im PFP-Format handelt. Deren
 * ID kann anschließend mit PFPFile::GetID ausgelesen werden, Version mit PFPFile::GetVersion bzw.
 * PFPFile::GetMainVersion und PFPFile::GetSubVersion. Handelt es sich nicht um eine Datei
 * im PFP-Format, gibt die Funktion \c false zurück. Es wird keine Exception geworfen.
 */
bool PFPFile::Ident(CFileObject &ff)
{
	try {
		const char *p;
		p=ff.Map(0,24);
		if (strncmp(p,"PFP-File",8)!=0) return false;
		if (Peek8(p+8)!=3) return false;
		id[4]=0;
		strncpy(id,p+10,4);
		mainversion=Peek8(p+15);
		subversion=Peek8(p+14);
		comp=(CCompression::Algorithm)Peek8(p+16);
		return true;
	} catch (...) {
		return false;
	}
	return false;
}


int PFPFile::Load(const char *file)
/*!\brief PFP-File laden
 *
 * Mit dieser Funktion wird ein PFP-File in die Klasse geladen. Dabei wird zuerst der Header geladen
 * und überprüft, ob es sich um ein gültiges PFP-File handelt. Anschließend wird geprüft, ob der
 * Datenbereich komprimiert ist und gegebenenfalls dekomprimiert. Erst danach werden die
 * einzelnen Chunks eingelesen. Kommt es dabei zu Fehlern durch ungültige Chunks, werden diese
 * ignoriert und die Funktion gibt den Fehlercode 434 zurück.
 *
 * \param file Pointer auf den Namen der Datei, die geladen werden soll.
 * \returns Konnte die Datei fehlerfrei eingelesen werden, gibt die Funktion true (1) zurück,
 * im Fehlerfall false (0). Ein entsprechender Fehlercode wird gesetzt.
 *
 * \remarks
 * Vor dem Laden der Datei wird die Funktion PFPFile::Clear aufgerufen, so dass eventuell vorher
 * vorhandene Daten verloren gehen.
 *
 * \since Version 6.1.0
 */
{
	if (!file) {
		SetError(194,"int PFPFile::Load(const char *file)");
		return 0;
	}
	CFile ff;
	if (!ff.Open((char*)file,"rb")) return 0;
	return Load(&ff);
}

int PFPFile::Load(CFileObject *ff)
/*!\brief PFP-File laden
 *
 * Mit dieser Funktion wird ein PFP-File in die Klasse geladen. Dabei wird zuerst der Header geladen
 * und überprüft, ob es sich um ein gültiges PFP-File handelt. Dann wird die virtuelle Funktion
 * PFPFile::LoadRequest mit ID, Haupt- und Unterversion als Parameter aufgerufen. Liefert diese
 * nicht true (1) zurück, wird der Ladevorgang abgebrochen. Andernfalls wird fortgeführt
 * und geprüft, ob der
 * Datenbereich komprimiert ist und gegebenenfalls dekomprimiert. Erst danach werden die
 * einzelnen Chunks eingelesen. Kommt es dabei zu Fehlern durch ungültige Chunks, werden diese
 * ignoriert und die Funktion gibt den Fehlercode 434 zurück.
 *
 * \param ff Pointer auf eine CFile-Klasse, mit der die einzulesende Datei geöffnet wurde.
 * \returns Konnte die Datei fehlerfrei eingelesen werden, gibt die Funktion true (1) zurück,
 * im Fehlerfall false (0). Ein entsprechender Fehlercode wird gesetzt.
 *
 * \remarks
 * Vor dem Laden der Datei wird die Funktion PFPFile::Clear aufgerufen, so dass eventuell vorher
 * vorhandene Daten verloren gehen.
 *
 * \since Version 6.1.0
 */
{
	if (!ff) {
		SetError(194,"int PFPFile::Load(CFile *ff)");
		return 0;
	}
	const char *p=ff->Map(0,24);
	if (!p) {
		ExtendError(432);
		return 0;
	}
	if (strncmp(p,"PFP-File",8)!=0) {
		SetError(432);
		return 0;
	}
	if (Peek8(p+8)!=3) {
		SetError(432);
		return 0;
	}
	ppluint32 z,fsize;
	// Wir haben ein gültiges PFP-File, aber dürfen wir es auch laden?
	char tmpid[5];
	tmpid[4]=0;
	strncpy(tmpid,p+10,4);
	int t1,t2;
	t1=peek8(p+15);
	t2=peek8(p+14);
	SetError(0);
	if (!LoadRequest(tmpid,t1,t2)) {
		if (GetErrorCode()==0) SetError(435);
		return 0;
	}
	Clear();
	strncpy(id,p+10,4);
	mainversion=peek8(p+15);
	subversion=peek8(p+14);
	comp=(CCompression::Algorithm)peek8(p+16);
	int hsize=peek8(p+9);
	char *u=NULL;
	if (comp) {
		p=(char*)ff->Map(hsize,8);
		if (!p) {
			PushError();
			Clear();
			PopError();
			ExtendError(433);
			return 0;
		}
		ppluint32 sizeunk=peek32(p);
		ppluint32 sizecomp=peek32(p+4);
		p=ff->Map(hsize+8,sizecomp);
		if (!p) {
			PushError();
			Clear();
			PopError();
			ExtendError(433);
			return 0;
		}
		u=(char*)malloc(sizeunk+1);
		if (!u) {
			Clear();
			SetError(2);
			return 0;
		}
		size_t dstlen=sizeunk;
		CCompression c;
		if (!c.Uncompress(u,&dstlen,p,sizecomp,comp)) {
			free(u);
			PushError();
			Clear();
			PopError();
			ExtendError(433);
			return 0;
		}
		if (dstlen!=sizeunk) {
			free(u);
			Clear();
			SetError(433,"Längendifferenz nach Dekomprimierung");
			return 0;
		}
		u[dstlen]=0;
		p=u;
		fsize=dstlen;
	} else {
		p=ff->Map();
		p+=hsize;
		fsize=(int)ff->Size()-hsize;
	}
	// Wir haben nun den ersten Chunk ab Pointer p
	z=0;
	char tmp[5];
	tmp[4]=0;
	ppluint32 size=0;
	int e=0;
	while ((z+=size)<fsize) {
		size=peek32(p+z+4);
		if (strncmp(p+z,"ENDF",4)==0) break;
		if (!size) break;
		// Falls z+size über das Ende der Datei geht, stimmt mit diesem Chunk was nicht
		if (z+size>fsize) break;
		strncpy(tmp,p+z,4);
		PFPChunk *chunk=new PFPChunk;
		if (!chunk->SetName(tmp)) { e++; continue; }
		if (!chunk->SetData(p+z+8,size-8)) { e++; continue; }
		if (!AddChunk(chunk)) continue;
	}
	if (u) free(u);
	if (e) {
		SetError(434,"%i",e);
		return 0;
	}
	return 1;
}

int PFPFile::LoadRequest(const char *id, int mainversion ,int subversion)
/*!\brief Ladevorgang bestätigen
 *
 * Diese Funktion wird bei jedem Ladevorgang aufgerufen. Falls die Anwendung eine Klasse definiert
 * hat, die von PFPFile abgeleitet ist, kann sie an dieser Stelle den Ladevorgang abbrechen,
 * wenn die Datei nicht dem unterstützten Format entspricht.
 *
 * \param id Pointer auf die 4-Byte-ID des PFP-Files
 * \param mainversion Die Hauptversion der Datei
 * \param subversion Die Unterversion der Datei
 * \returns Die Funktion muss true (1) zurückliefern, wenn der Ladevorgang fortgesetzt werden
 * darf, oder false (0), wenn er abgebrochen werden soll. Optional kann auch ein Fehlercode
 * gesetzt werden. Wird dies nicht gemacht, setzt die Ladefunktion den Fehlercode 435.
 *
 * \since Version 6.1.0
 */
{
	return 1;
}

}
