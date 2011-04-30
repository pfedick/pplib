/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 * $URL$
 *
 *******************************************************************************
 * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
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

#include "ppl7.h"


namespace ppl7 {

/*!\class AssocArray
 * \ingroup PPLGroupDataTypes
 * \brief Komplexes mehrdimensionales Array mit Strings als Schlüssel
 *
 * \desc
 * Die Klasse AssocArray dient als Container für beliebige Key-Value-Paare. Ein Schlüssel
 * (Key) besteht aus einem String, der aus beliebigen Zeichen bestehen kann. Ein Value kann
 * veschiedene Datentypen enthalten. Gegenwärtig werden neben Strings auch Arrays, Binaries und
 * Pointer unterstützt.
 * \par
 * Die Schlüssel werden sortiert in einem AVL-Baum verwaltet (siehe CTree, CTreeItem, CArrayItem), so dass auch bei
 * sehr großen Arrays eine schnelle Verarbeitung gewährleistet ist. Gross-/Kleinschreibung wird dabei
 * ignoriert, der Schlüssel "TEST" wäre also identisch mit "test" oder "Test".
 * \par
 * Mehrdimensionale Arrays sind möglich, indem einem Schlüssel als Wert einfach ein anderes Array
 * zugeordnet wird. In einem solchen Array kann jedes Element direkt angesprochen werden, indem man
 * die einzelnen Schlüssel durch Slash (/) getrennt zu einem einzigen Schlüssel zusammenfasst.
 * \par
 * Mehrdimensionale Arrays werden automatisch generiert. Gibt man bei einem leeren Array dem Schlüssel
 * <tt>"ebene1/ebene2/key"</tt> einen Wert, werden automatisch folgende Aktionen ausgeführt:
 * - Es wird ein neues Array generiert und mit dem Schlüssel "ebene1" in das Array eingefügt
 * - In das Array "ebene1" wird ein weiteres neues Array mit dem Schlüssel "ebene2" eingefügt
 * - In das Array "ebene2" wird der eigentliche Wert unter dem Schlüssel "key" eingefügt
 *
 * \par Beispiel:
 * \code
 * ppl7::AssocArray a;
 * // Wert setzen
 * a.set("ebene1/ebene2/key","Ein Wert");
 * // Wert auslesen
 * a.get("ebene1/ebene2/key").toString().printnl();
 * \endcode
 *
 */


} // EOF namespace ppl7
