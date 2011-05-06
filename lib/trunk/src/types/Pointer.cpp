/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 75 $
 * $Date: 2011-05-04 19:35:00 +0000 (Mi, 04 Mai 2011) $
 * $Id: Variant.cpp 75 2011-05-04 19:35:00Z pafe $
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


#include "ppl7.h"

namespace ppl7 {


/*!\class Pointer
 * \ingroup PPLGroupDataTypes
 * \brief %Pointer-Klasse
 *
 * \desc
 * Diese Klasse kann zur Darstellung eines beliebigen Pointers verwendet werden.
 *
 */

Pointer::Pointer()
{
	type=POINTER;
	myptr=NULL;
}

Pointer::Pointer(const Pointer &other)
{
	type=POINTER;
	myptr=other.myptr;
}

Pointer::Pointer(void *ptr)
{
	type=POINTER;
	myptr=ptr;
}

bool Pointer::isNull() const
{
	if (myptr==NULL) return true;
	return false;
}

const void *Pointer::ptr() const
{
	return myptr;
}

Pointer::operator const void*() const
{
	return (const void*)myptr;
}

Pointer::operator const char*() const
{
	return (const char*)myptr;
}

Pointer::operator const unsigned char*() const
{
	return (const unsigned char*)myptr;
}

void Pointer::set(const void *ptr)
{
	myptr=(void*)ptr;
}

Pointer &Pointer::operator=(const Pointer &other)
{
	myptr=other.myptr;
	return *this;
}

Pointer &Pointer::operator=(const void *ptr)
{
	myptr=(void*)ptr;
	return *this;
}

} // EOF namespace ppl7
