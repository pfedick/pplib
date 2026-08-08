/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <mutex>

#include <pplib/types/string.h>
#include <pplib/grafix/imagefilter.h>
#include <pplib/grafix/grafix.h>

namespace pplib::grafix
{

void Grafix::addImageFilter(ImageFilter* filter)
{
    MutexLock lock(myMutex);
    ImageFilterList.push_back(filter);
}

void Grafix::unloadImageFilter(ImageFilter* filter) noexcept
{
    MutexLock lock(myMutex);
    for (auto it = ImageFilterList.begin(); it != ImageFilterList.end(); ++it) {
        if (*it == filter) {
            ImageFilterList.erase(it);
            break;
        }
    }
}

ImageFilter* Grafix::findImageFilter(const String& name) noexcept
{
    MutexLock lock(myMutex);
    // Wir gehen die Liste rückwärts durch
    for (auto it = ImageFilterList.rbegin(); it != ImageFilterList.rend(); ++it) {
        if (name.strCaseCmp((*it)->name()) == 0) {
            return *it;
        }
    }
    return nullptr;
}

ImageFilter* Grafix::findImageFilter(FileObject& ff, IMAGE& img)
{
    MutexLock lock(myMutex);
    // Wir gehen die Liste rückwärts durch
    for (auto it = ImageFilterList.rbegin(); it != ImageFilterList.rend(); ++it) {
        if ((*it)->ident(ff, img) == 1) {
            return *it;
        }
    }
    return nullptr;
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
int ImageFilter::ident(FileObject& file, IMAGE& img)
{
    throw UnimplementedVirtualFunctionException();
}

void ImageFilter::load(FileObject& file, Drawable& surface, IMAGE& img)
{
    throw UnimplementedVirtualFunctionException();
}

void ImageFilter::save(const Drawable& surface, FileObject& file, const Rect& area, const AssocArray& param)
{
    Drawable draw = surface.getDrawable(area);
    save(draw, file, param);
}

void ImageFilter::save(const Drawable& surface, FileObject& file, const AssocArray& param)
{
    throw UnimplementedVirtualFunctionException();
}

void ImageFilter::saveFile(const String& filename, const Drawable& surface, const AssocArray& param)
{
    File ff;
    ff.open(filename, File::WRITE);
    save(surface, ff, param);
}

void ImageFilter::saveFile(const String& filename, const Drawable& surface, const Rect& area, const AssocArray& param)
{
    Drawable draw = surface.getDrawable(area);
    File ff;
    ff.open(filename, File::WRITE);
    save(draw, ff, param);
}

String ImageFilter::name()
{
    return "unknown";
}

String ImageFilter::description()
{
    return "Baseclass for image filter";
}

} // namespace grafix
} // namespace pplib
