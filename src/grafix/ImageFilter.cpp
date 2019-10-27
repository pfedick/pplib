/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
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

#include "../../include/prolog_ppl7.h"
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include "ppl7.h"
#include "ppl7-grafix.h"


namespace ppl7 {
namespace grafix {


/*!\brief Image-Filter registrieren
 *
 * \desc
 * Mit dieser Funktion wird ein neuer Image-Filter registriert. Ein Image-Filter ist
 * eine von ImageFilter abgeleitete Klasse, die in der Lage ist ein bestimmtes Grafikformat
 * zu lesen und optional auch zu schreiben.
 * Der Filter muss mit "new" angelegt worden sein. Grafix übernimmt dessen
 * Verwaltung, dass heisst die Klasse kümmert sich auch um das Löschen. Mit
 * Grafix::UnloadFilter kann ein Filter von der Anwendung manuell wieder entfernt
 * werden.
 *
 * \param[in] filter Pointer auf den zu registrierenden Filter
 * \return Konnte der Filter erfolgreich registriert werden, liefert die Funktion
 * 1 zurück, im Fehlerfall 0. Ein entsprechender Fehlercode wird gesetzt.
 *
 * \see
 * - ImageFilter
 * - Grafix::UnloadFilter
 * - Grafix::FindFilter
 */
void Grafix::addImageFilter(ImageFilter *filter)
{
	myMutex.lock();
	try {
		ImageFilterList.add(filter);
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	myMutex.unlock();
}

/*!\brief Image-Filter entfernen
 *
 * \desc
 * Mit dieser Funktion wird ein zuvor mit Grafix::AddFilter registrierter Image-Filter
 * aus der Grafik-Engine entfernt. Der Filter selbst wird jedoch nicht gelöscht, darum
 * muss sich die Anwendung kümmern.
 *
 * \param[in] filter Pointer auf den zu entfernenden Image-Filter
 * \return Bei Erfolg liefert die Funktion 1 zurück, im Fehlerfall 0.
 */
void Grafix::unloadImageFilter(ImageFilter *filter)
{
	myMutex.lock();
	try {
		ImageFilterList.erase(filter);
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	myMutex.unlock();
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
ImageFilter *Grafix::findImageFilter(const String &name)
{
	List<ImageFilter*>::Iterator it;
	myMutex.lock();
	// Wir gehen die Liste rückwärts durch
	ImageFilterList.reset(it);
	try {
		while (ImageFilterList.getPrevious(it)) {
			ImageFilter *f=it.value();
			if (name.strCaseCmp(f->name())==0) {
				myMutex.unlock();
				return f;
			}
		}
	} catch (EndOfListException &) {
		myMutex.unlock();
		throw UnknownImageFormatException();
	} catch (...) {
		myMutex.unlock();
		throw;
	}
	myMutex.unlock();
	throw UnknownImageFormatException();

}

/*!\brief Filter anhand des Inhalts einer geöffneten Datei finden
 *
 * \desc
 * Mit dieser Funktion kann ein registrierter Filter anhand des Inhalts einer
 * bereits geöffneten Datei gefunden werden. Dazu wird die Funktion ImageFilter::Ident
 * von jedem registrierten Filter aufgerufen, bis einer signalisiert, dass er das
 * Format verarbeiten kann.
 *
 * \param[in] ff Referenz auf die geöffnete Datei
 * \return Bei Erfolg liefert die Funktion einen Pointer auf den gefundenen Filter
 * zurück, im Fehlerfall NULL.
 */
ImageFilter *Grafix::findImageFilter(FileObject &ff, IMAGE &img)
{
	List<ImageFilter*>::Iterator it;
	myMutex.lock();
	// Wir gehen die Liste rückwärts durch
	ImageFilterList.reset(it);
	try {
		while (ImageFilterList.getPrevious(it)) {
			ImageFilter *f=it.value();
			if (f->ident(ff,img)==1) {
				myMutex.unlock();
				return f;
			}
		}
	} catch (EndOfListException &) {
		myMutex.unlock();
		throw UnknownImageFormatException();
	} catch (...) {
		myMutex.unlock();
		throw;
	}

	myMutex.unlock();
	throw UnknownImageFormatException();
}




/*!\class ImageFilter
 * \ingroup PPLGroupGrafik
 * \brief Basisklasse für Import-/Export-Filter verschiedener Grafikformate
 *
 * \desc
 * Dies ist die Basisklasse für alle Import- und Export-Filter für verschiedene
 * Grafikformate.
 * eine von ImageFilter abgeleitete Klasse, die in der Lage ist ein bestimmtes Grafikformat
 * zu lesen und optional auch zu schreiben.
 * Eine Instanz des Filters muss mit "new" angelegt und mit der Funktion
 * Grafix::addFilter in der Grafik-Engine registriert werden. Die Grafik-Engine
 * kümmert sich bei Programmende selbst um das Löschen des Filters.
 * Mit Grafix::unloadFilter kann ein Filter aber auch manuell von der Anwendung
 * wieder entfernt werden. Mit Grafix::findFilter kann ein Filter anhand seines
 * Namens gefunden werden.
 */

/*!\brief Konstruktor
 *
 * \desc
 * Im Konstruktor werden interne Daten der Klasse initialisiert
 */
ImageFilter::ImageFilter()
{
}

/*!\brief Destruktor
 *
 * \desc
 * Der Destruktor sorgt dafür, dass durch die Klasse allokierter Speicher
 * wieder freigegeben wird.
 */
ImageFilter::~ImageFilter()
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
int ImageFilter::ident(FileObject &file, IMAGE &img)
{
	throw UnimplementedVirtualFunctionException();
}

void ImageFilter::load(FileObject &file, Drawable &surface, IMAGE &img)
{
	throw UnimplementedVirtualFunctionException();
}

void ImageFilter::save (const Drawable &surface, FileObject &file, const Rect &area, const AssocArray &param)
{
	Drawable draw=surface.getDrawable(area);
	save(draw,file,param);
}

void ImageFilter::save (const Drawable &surface, FileObject &file, const AssocArray &param)
{
	throw UnimplementedVirtualFunctionException();
}

void ImageFilter::saveFile (const String &filename, const Drawable &surface, const AssocArray &param)
{
	File ff;
	ff.open(filename,File::WRITE);
	save(surface,ff,param);
}

void ImageFilter::saveFile (const String &filename, const Drawable &surface, const Rect &area, const AssocArray &param)
{
	Drawable draw=surface.getDrawable(area);
	File ff;
	ff.open(filename,File::WRITE);
	save(draw,ff,param);
}

String ImageFilter::name()
{
	return "unknown";
}

String ImageFilter::description()
{
	return "Baseclass for image filter";
}



} // EOF namespace grafix
} // EOF namespace ppl7

