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

#ifndef _PPLIB_INCLUDE_GRAFIX
#define _PPLIB_INCLUDE_GRAFIX
#include <pplib.h>

#include <map>
#include <list>

#include <pplib/grafix/color.h>
#include <pplib/grafix/point.h>
#include <pplib/grafix/size.h>
#include <pplib/grafix/rect.h>
#include <pplib/grafix/rgbformat.h>
#include <pplib/grafix/fonts.h>
#include <pplib/core/baseexception.h>

namespace pplib
{
namespace grafix
{

PPLIBEXCEPTION(UnsupportedColorFormatException, Exception);

PPLIBEXCEPTION(EmptyDrawableException, Exception);
PPLIBEXCEPTION(UnknownBltMethodException, Exception);

PPLIBEXCEPTION(FunctionUnavailableException, Exception);
PPLIBEXCEPTION(InvalidImageSizeException, Exception);
PPLIBEXCEPTION(UnknownImageFormatException, Exception);
PPLIBEXCEPTION(InvalidSpriteException, Exception);

} // namespace grafix
} // end of namespace pplib

#endif // _PPLIB_INCLUDE_GRAFIX
