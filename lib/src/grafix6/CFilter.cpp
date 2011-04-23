/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 1.2 $
 * $Date: 2010/02/12 19:43:48 $
 * $Id: CFilter.cpp,v 1.2 2010/02/12 19:43:48 pafe Exp $
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


/*!\brief Image-Filter registrieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Image-Filter registriert. Ein Image-Filter ist
 * eine von CFilter abgeleitete Klasse, die in der Lage ist ein bestimmtes Grafikformat
 * zu lesen und optional auch zu schreiben.
 * Der Filter muss mit "new" angelegt worden sein. CGrafix übernimmt dessen
 * Verwaltung, dass heisst die Klasse kümmert sich auch um das Löschen. Mit
 * CGrafix::UnloadFilter kann ein Filter von der Anwendung manuell wieder entfernt
 * werden.
 *
 * \param[in] filter Pointer auf den zu registrierenden Filter
 * \return Konnte der Filter erfolgreich registriert werden, liefert die Funktion
 * 1 zurück, im Fehlerfall 0. Ein entsprechender Fehlercode wird gesetzt.
 *
 * \see
 * - CFilter
 * - CGrafix::UnloadFilter
 * - CGrafix::FindFilter
 */
int CGrafix::AddFilter(CFilter *filter)
{
	int ret;
	Mutex.Lock();
	ret=ImageFilter.Add(filter);
	Mutex.Unlock();
	if (!ret) {
		ExtendError(1003);
	}
	return ret;
}

/*!\brief Image-Filter entfernen
 *
 * \desc
 * Mit dieser Funktion wird ein zuvor mit CGrafix::AddFilter registrierter Image-Filter
 * aus der Grafik-Engine entfernt. Der Filter selbst wird jedoch nicht gelöscht, darum
 * muss sich die Anwendung kümmern.
 *
 * \param[in] filter Pointer auf den zu entfernenden Image-Filter
 * \return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
int CGrafix::UnloadFilter(CFilter *filter)
{
	int ret;
	Mutex.Lock();
	ret=ImageFilter.Delete(filter);
	Mutex.Unlock();
	if (!ret) {
		ExtendError(1004);
	}
	return ret;
}

/*!\brief Filter anhand seines Namens finden
 *
 * \desc
 * Jeder Filter muss einen eindeutigen Namen haben. Mit dieser Funktion kann ein
 * registrierter Filter anhand dieses Namens gefunden werden.
 *
 * \param[in] name Der gesuchte Filter-Name
 * \return Bei Erfolg liefert die Funktion einen Pointer auf den gefundenen Filter
 * zurück, im Fehlerfall NULL.
 */
CFilter *CGrafix::FindFilter(const CString &name)
{
	CFilter *f;
	Mutex.Lock();
	// Wir gehen die Liste rückwärts durch
	ImageFilter.Reset();
	while ((f=(CFilter*)ImageFilter.GetPrevious())) {
		if (StrCaseCmp(name,f->Name())==0) {
			Mutex.Unlock();
			return f;
		}
	}
	Mutex.Unlock();
	SetError(1006,"%s", (const char*)name);
	return NULL;
}

/*!\brief Filter anhand des Inhalts einer geöffneten Datei finden
 *
 * \desc
 * Mit dieser Funktion kann ein registrierter Filter anhand des Inhalts einer
 * bereits geöffneten Datei gefunden werden. Dazu wird die Funktion CFilter::Ident
 * von jedem registrierten Filter aufgerufen, bis einer signalisiert, dass er das
 * Format verarbeiten kann.
 *
 * \param[in] ff Referenz auf die geöffnete Datei
 * \return Bei Erfolg liefert die Funktion einen Pointer auf den gefundenen Filter
 * zurück, im Fehlerfall NULL.
 */
CFilter *CGrafix::FindFilter(CFileObject &ff, IMAGE &img)
{
	CFilter *filter;
	Mutex.Lock();
	// Wir gehen die Liste rückwärts durch
	ImageFilter.Reset();
	while ((filter=(CFilter*)ImageFilter.GetPrevious())) {
		if (filter->Ident(ff,img)==1) break;
	}
	Mutex.Unlock();
	if (!filter) {
		SetError(1017);
		return NULL;
	}
	return filter;
}

/*!\brief Filter anhand des Inhalts einer geöffneten Datei finden
 *
 * \desc
 * Mit dieser Funktion kann ein registrierter Filter anhand des Inhalts einer
 * bereits geöffneten Datei gefunden werden. Dazu wird die Funktion CFilter::Ident
 * von jedem registrierten Filter aufgerufen, bis einer signalisiert, dass er das
 * Format verarbeiten kann.
 *
 * \param[in] ff Pointer auf die geöffnete Datei
 * \return Bei Erfolg liefert die Funktion einen Pointer auf den gefundenen Filter
 * zurück, im Fehlerfall NULL.
 */
CFilter *CGrafix::FindFilter(CFileObject *ff, IMAGE &img)
{
	if (!ff) {
		SetError(194);
		return NULL;
	}
	return FindFilter(*ff, img);
}

/*
CSurface *CGrafix::Load(CResource *res, int id, int flags, const RGBFormat &rgbformat, CFilter *filter)
{
	if (!res) {
		SetError(194);
		return NULL;
	}
	CFileObject *ff;
	ff=res->GetFile(id);
	if (!ff) return NULL;
	CSurface *s=Load(ff,flags,rgbformat,filter);
	PushError();
	delete ff;
	PopError();
	return s;
}

CSurface *CGrafix::Load(CResource *res, const char *name, int flags, const RGBFormat &rgbformat, CFilter *filter)
{
	if (!res) {
		SetError(194);
		return NULL;
	}
	if (!name) {
		SetError(194);
		return NULL;
	}
	CFileObject *ff;
	ff=res->GetFile(name);
	if (!ff) return NULL;
	CSurface *s=Load(ff,flags,rgbformat,filter);
	PushError();
	delete ff;
	PopError();
	return s;
}

CSurface *CGrafix::Load(const CString &filename, int flags, const RGBFormat &rgbformat, CFilter *filter)
{
	CFile ff;
	if (!ff.Open(filename,"rb")) return NULL;
	return Load(&ff,flags,rgbformat, filter);
}

CSurface *CGrafix::Load(const CMemory &memory, int flags, const RGBFormat &rgbformat, CFilter *filter)
{
	CMemFile ff;
	if (!ff.Open(memory)) return NULL;
	return Load(&ff,flags,rgbformat, filter);
}

CSurface *CGrafix::Load(CFileObject &ff, int flags, const RGBFormat &rgbformat, CFilter *filter)
{
	IMAGE img;
	if (!filter) {
		filter=FindFilter(ff);
		if (!filter) return NULL;
	} else {
		if (!filter->Ident(ff,img)) return 0;
	}
	RGBFormat format=rgbformat;
	if ((flags&Surface::Flags::ConvertColors) && rgbformat==RGBFormat::unknown) format=PrimaryRGBFormat;
	if (format==RGBFormat::unknown) format=img.format;
	// TODO:
	// CSurface *surface=CreateSurface("img",img.width,img.height,format,flags);
	CSurface *surface=NULL;
	if (!surface) return NULL;
	if (!filter->Load(ff,surface,img)) {
		PushError();
		delete surface;
		PopError();
		return NULL;
	}
	return surface;
}

CSurface *CGrafix::Load(CFileObject *ff, int flags, const RGBFormat &rgbformat, CFilter *filter)
{
	if (!ff) {
		SetError(194);
		return NULL;
	}
	IMAGE img;
	if (!filter) {
		filter=FindFilter(ff);
		if (!filter) return NULL;
	} else {
		if (!filter->Ident(*ff,img)) return 0;
	}
	RGBFormat format=rgbformat;
	if ((flags&Surface::Flags::ConvertColors) && rgbformat==RGBFormat::unknown) format=PrimaryRGBFormat;
	if (format==RGBFormat::unknown) format=img.format;
	// TODO:
	// CSurface *surface=CreateSurface("img",img.width,img.height,format,flags);
	CSurface *surface=NULL;
	if (!surface) return NULL;
	if (!filter->Load(*ff,surface,img)) {
		PushError();
		delete surface;
		PopError();
		return NULL;
	}
	return surface;
}
*/


/*!\class CFilter
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Import-/Export-Filter verschiedener Grafikformate
 *
 * \desc
 * Dies ist die Basisklasse für alle Import- und Export-Filter für verschiedene
 * Grafikformate.
 * eine von CFilter abgeleitete Klasse, die in der Lage ist ein bestimmtes Grafikformat
 * zu lesen und optional auch zu schreiben.
 * Eine Instanz des Filters muss mit "new" angelegt und mit der Funktion
 * CGrafix::AddFilter in der Grafik-Engine registriert werden. Die Grafik-Engine
 * kümmert sich bei Programmende selbst um das Löschen des Filters.
 * Mit CGrafix::UnloadFilter kann ein Filter aber auch manuell von der Anwendung
 * wieder entfernt werden. Mit CGrafix::FindFilter kann ein Filter anhand seines
 * Namens gefunden werden.
 */

/*!\brief Konstruktor
 *
 * \desc
 * Im Konstruktor werden interne Daten der Klasse initialisiert
 */
CFilter::CFilter()
{
}

/*!\brief Destruktor
 *
 * \desc
 * Der Destruktor sorgt dafür, dass durch die Klasse allokierter Speicher
 * wieder freigegeben wird.
 */
CFilter::~CFilter()
{
}


/*!\brief Grafikformat identifizieren
 *
 * \desc
 * Diese Funktion wird aufgerufen, um herauszufinden, ob ein Grafikformat durch diesen
 * Filter gelesen werden kann. Ist dieser der Fall, muss die die Funktion 1 zurückgeben
 * und die IMAGE-Struktur \p img füllen. Wird das Format nicht unterstützt, muss 0
 * zurückgegeben werden.
 *
 * @param[in] file Eine geöffnete Datei
 * @param[out] img Eine IMAGE-Struktur
 * @return Wird das Grafikformat unterstützt, muss die Funktion 1 zurückgegen,
 * andernfalls 0.
 */
int CFilter::Ident(CFileObject &file, IMAGE &img)
{
	SetError(1005);
	return 0;
}

int CFilter::Load(CFileObject &file, CDrawable &surface, IMAGE &img)
{
	SetError(1005);
	return 0;
}

int CFilter::Save (const CDrawable &surface, CFileObject &file, const Rect &area, CAssocArray *param)
{
	CDrawable draw=surface.getDrawable(area);
	return Save(draw,file,param);
}

int CFilter::Save (const CDrawable &surface, CFileObject &file, CAssocArray *param)
{
	SetError(1005);
	return 0;
}

int CFilter::SaveFile (const CDrawable &surface, const CString &filename, CAssocArray *param)
{
	CFile ff;
	if (!ff.Open(filename,"wb")) return 0;
	return Save(surface,ff,param);
}

int CFilter::SaveFile (const CDrawable &surface, const CString &filename, const Rect &area, CAssocArray *param)
{
	CDrawable draw=surface.getDrawable(area);
	CFile ff;
	if (!ff.Open(filename,"wb")) return 0;
	return Save(draw,ff,param);
}

CString CFilter::Name()
{
	return "unknown";
}

CString CFilter::Description()
{
	return "Basisklasse für Grafikfilter";
}



} // EOF namespace grafix
} // EOF namespace ppl6

