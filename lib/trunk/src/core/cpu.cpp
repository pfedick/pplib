/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 137 $
 * $Date: 2011-05-28 12:21:26 +0000 (Sa, 28 Mai 2011) $
 * $Id: Dir.cpp 137 2011-05-28 12:21:26Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/core/Dir.cpp $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <strings.h>
#include <ctype.h>
#endif

#include "ppl7.h"

#ifdef HAVE_X86_ASSEMBLER
extern "C" {
	ppluint32 GetASMBits();
	ppluint32 PPL7_GetCpuCaps();
}
#else
static ppluint32 PPL7_GetCpuCaps()
{
	return 0;
}

static ppluint32 GetASMBits()
{
	return 0;
}
#endif

namespace ppl7 {



String binaryString(ppluint64 value)
{
	String ret;
	for (int i=63;i>=0;i--) {
		if (value&((ppluint64)1<<i)) ret+="1";
		else ret+="0";
	}
	return ret;
}


ppluint32 GetCPUCaps (CPUCaps *cpu)
{
	ppluint32 caps=PPL7_GetCpuCaps();
	printf ("caps=%i = %ls\n",caps,(const wchar_t*)binaryString(caps));
	//int bits=HaveSSE2();
	//printf ("cpuid 1: %x\n",bits);


	if (cpu) {
		cpu->caps=caps;
		cpu->bits=GetASMBits();
	}
	return caps;
}

} // end of namespace ppl

