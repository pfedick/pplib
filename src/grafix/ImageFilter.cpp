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
#include <pplib/core/file.h>
#include <pplib/grafix/imagefilter.h>
#include <pplib/grafix/grafix.h>

namespace pplib::grafix
{

void Grafix::addImageFilter(ImageFilter* filter)
{
    MutexLock lock(myMutex);
    // filter darf nicht schon registriert sein
    for (auto it = ImageFilterList.begin(); it != ImageFilterList.end(); ++it) {
        if (*it == filter) throw DuplicateImageFilterException();
    }
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

ImageFilter* Grafix::findImageFilter(FileObject& ff, IMAGE& img) noexcept
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

void ImageFilter::saveFile(const String& filename, const Drawable& surface, const AssocArray& param)
{
    File ff;
    ff.open(filename, File::FileMode::WRITE);
    save(surface, ff, param);
}

} // namespace pplib::grafix
