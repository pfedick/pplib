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
#include <stdio.h>
#include "ppl6.h"

namespace ppl6 {

static ppluint32 SinusTabelle[]={
     0x00000000,0x00000006,0x0000000c,0x00000012,0x00000019,0x0000001f,0x00000025,0x0000002b,
     0x00000032,0x00000038,0x0000003e,0x00000045,0x0000004b,0x00000051,0x00000057,0x0000005e,
     0x00000064,0x0000006a,0x00000070,0x00000077,0x0000007d,0x00000083,0x00000089,0x00000090,
     0x00000096,0x0000009c,0x000000a2,0x000000a8,0x000000af,0x000000b5,0x000000bb,0x000000c1,
     0x000000c7,0x000000cd,0x000000d4,0x000000da,0x000000e0,0x000000e6,0x000000ec,0x000000f2,
     0x000000f8,0x000000fe,0x00000104,0x0000010b,0x00000111,0x00000117,0x0000011d,0x00000123,
     0x00000129,0x0000012f,0x00000135,0x0000013b,0x00000141,0x00000147,0x0000014d,0x00000153,
     0x00000158,0x0000015e,0x00000164,0x0000016a,0x00000170,0x00000176,0x0000017c,0x00000182,
     0x00000187,0x0000018d,0x00000193,0x00000199,0x0000019e,0x000001a4,0x000001aa,0x000001b0,
     0x000001b5,0x000001bb,0x000001c1,0x000001c6,0x000001cc,0x000001d2,0x000001d7,0x000001dd,
     0x000001e2,0x000001e8,0x000001ed,0x000001f3,0x000001f8,0x000001fe,0x00000203,0x00000209,
     0x0000020e,0x00000213,0x00000219,0x0000021e,0x00000223,0x00000229,0x0000022e,0x00000233,
     0x00000238,0x0000023e,0x00000243,0x00000248,0x0000024d,0x00000252,0x00000257,0x0000025c,
     0x00000261,0x00000267,0x0000026c,0x00000271,0x00000275,0x0000027a,0x0000027f,0x00000284,
     0x00000289,0x0000028e,0x00000293,0x00000298,0x0000029c,0x000002a1,0x000002a6,0x000002ab,
     0x000002af,0x000002b4,0x000002b8,0x000002bd,0x000002c2,0x000002c6,0x000002cb,0x000002cf,
     0x000002d4,0x000002d8,0x000002dc,0x000002e1,0x000002e5,0x000002e9,0x000002ee,0x000002f2,
     0x000002f6,0x000002fa,0x000002ff,0x00000303,0x00000307,0x0000030b,0x0000030f,0x00000313,
     0x00000317,0x0000031b,0x0000031f,0x00000323,0x00000327,0x0000032b,0x0000032e,0x00000332,
     0x00000336,0x0000033a,0x0000033d,0x00000341,0x00000345,0x00000348,0x0000034c,0x0000034f,
     0x00000353,0x00000356,0x0000035a,0x0000035d,0x00000361,0x00000364,0x00000367,0x0000036b,
     0x0000036e,0x00000371,0x00000374,0x00000377,0x0000037a,0x0000037e,0x00000381,0x00000384,
     0x00000387,0x0000038a,0x0000038c,0x0000038f,0x00000392,0x00000395,0x00000398,0x0000039a,
     0x0000039d,0x000003a0,0x000003a2,0x000003a5,0x000003a8,0x000003aa,0x000003ad,0x000003af,
     0x000003b2,0x000003b4,0x000003b6,0x000003b9,0x000003bb,0x000003bd,0x000003bf,0x000003c2,
     0x000003c4,0x000003c6,0x000003c8,0x000003ca,0x000003cc,0x000003ce,0x000003d0,0x000003d2,
     0x000003d3,0x000003d5,0x000003d7,0x000003d9,0x000003da,0x000003dc,0x000003de,0x000003df,
     0x000003e1,0x000003e2,0x000003e4,0x000003e5,0x000003e7,0x000003e8,0x000003e9,0x000003eb,
     0x000003ec,0x000003ed,0x000003ee,0x000003ef,0x000003f0,0x000003f1,0x000003f2,0x000003f3,
     0x000003f4,0x000003f5,0x000003f6,0x000003f7,0x000003f8,0x000003f9,0x000003f9,0x000003fa,
     0x000003fb,0x000003fb,0x000003fc,0x000003fc,0x000003fd,0x000003fd,0x000003fe,0x000003fe,
     0x000003fe,0x000003ff,0x000003ff,0x000003ff,0x000003ff,0x000003ff,0x000003ff,0x000003ff,
     0x00000400,0x000003ff,0x000003ff,0x000003ff,0x000003ff,0x000003ff,0x000003ff,0x000003ff,
     0x000003fe,0x000003fe,0x000003fe,0x000003fd,0x000003fd,0x000003fc,0x000003fc,0x000003fb,
     0x000003fb,0x000003fa,0x000003f9,0x000003f9,0x000003f8,0x000003f7,0x000003f6,0x000003f5,
     0x000003f4,0x000003f3,0x000003f2,0x000003f1,0x000003f0,0x000003ef,0x000003ee,0x000003ed,
     0x000003ec,0x000003eb,0x000003e9,0x000003e8,0x000003e7,0x000003e5,0x000003e4,0x000003e2,
     0x000003e1,0x000003df,0x000003de,0x000003dc,0x000003da,0x000003d9,0x000003d7,0x000003d5,
     0x000003d3,0x000003d2,0x000003d0,0x000003ce,0x000003cc,0x000003ca,0x000003c8,0x000003c6,
     0x000003c4,0x000003c2,0x000003bf,0x000003bd,0x000003bb,0x000003b9,0x000003b6,0x000003b4,
     0x000003b2,0x000003af,0x000003ad,0x000003aa,0x000003a8,0x000003a5,0x000003a2,0x000003a0,
     0x0000039d,0x0000039a,0x00000398,0x00000395,0x00000392,0x0000038f,0x0000038c,0x0000038a,
     0x00000387,0x00000384,0x00000381,0x0000037e,0x0000037a,0x00000377,0x00000374,0x00000371,
     0x0000036e,0x0000036b,0x00000367,0x00000364,0x00000361,0x0000035d,0x0000035a,0x00000356,
     0x00000353,0x0000034f,0x0000034c,0x00000348,0x00000345,0x00000341,0x0000033d,0x0000033a,
     0x00000336,0x00000332,0x0000032e,0x0000032b,0x00000327,0x00000323,0x0000031f,0x0000031b,
     0x00000317,0x00000313,0x0000030f,0x0000030b,0x00000307,0x00000303,0x000002ff,0x000002fa,
     0x000002f6,0x000002f2,0x000002ee,0x000002e9,0x000002e5,0x000002e1,0x000002dc,0x000002d8,
     0x000002d4,0x000002cf,0x000002cb,0x000002c6,0x000002c2,0x000002bd,0x000002b8,0x000002b4,
     0x000002af,0x000002ab,0x000002a6,0x000002a1,0x0000029c,0x00000298,0x00000293,0x0000028e,
     0x00000289,0x00000284,0x0000027f,0x0000027a,0x00000275,0x00000271,0x0000026c,0x00000267,
     0x00000261,0x0000025c,0x00000257,0x00000252,0x0000024d,0x00000248,0x00000243,0x0000023e,
     0x00000238,0x00000233,0x0000022e,0x00000229,0x00000223,0x0000021e,0x00000219,0x00000213,
     0x0000020e,0x00000209,0x00000203,0x000001fe,0x000001f8,0x000001f3,0x000001ed,0x000001e8,
     0x000001e2,0x000001dd,0x000001d7,0x000001d2,0x000001cc,0x000001c6,0x000001c1,0x000001bb,
     0x000001b5,0x000001b0,0x000001aa,0x000001a4,0x0000019e,0x00000199,0x00000193,0x0000018d,
     0x00000187,0x00000182,0x0000017c,0x00000176,0x00000170,0x0000016a,0x00000164,0x0000015e,
     0x00000158,0x00000153,0x0000014d,0x00000147,0x00000141,0x0000013b,0x00000135,0x0000012f,
     0x00000129,0x00000123,0x0000011d,0x00000117,0x00000111,0x0000010b,0x00000104,0x000000fe,
     0x000000f8,0x000000f2,0x000000ec,0x000000e6,0x000000e0,0x000000da,0x000000d4,0x000000cd,
     0x000000c7,0x000000c1,0x000000bb,0x000000b5,0x000000af,0x000000a8,0x000000a2,0x0000009c,
     0x00000096,0x00000090,0x00000089,0x00000083,0x0000007d,0x00000077,0x00000070,0x0000006a,
     0x00000064,0x0000005e,0x00000057,0x00000051,0x0000004b,0x00000045,0x0000003e,0x00000038,
     0x00000032,0x0000002b,0x00000025,0x0000001f,0x00000019,0x00000012,0x0000000c,0x00000006,
     0x00000000,0xfffffffa,0xfffffff4,0xffffffee,0xffffffe7,0xffffffe1,0xffffffdb,0xffffffd5,
     0xffffffce,0xffffffc8,0xffffffc2,0xffffffbb,0xffffffb5,0xffffffaf,0xffffffa9,0xffffffa2,
     0xffffff9c,0xffffff96,0xffffff90,0xffffff89,0xffffff83,0xffffff7d,0xffffff77,0xffffff70,
     0xffffff6a,0xffffff64,0xffffff5e,0xffffff58,0xffffff51,0xffffff4b,0xffffff45,0xffffff3f,
     0xffffff39,0xffffff33,0xffffff2c,0xffffff26,0xffffff20,0xffffff1a,0xffffff14,0xffffff0e,
     0xffffff08,0xffffff02,0xfffffefc,0xfffffef5,0xfffffeef,0xfffffee9,0xfffffee3,0xfffffedd,
     0xfffffed7,0xfffffed1,0xfffffecb,0xfffffec5,0xfffffebf,0xfffffeb9,0xfffffeb3,0xfffffead,
     0xfffffea8,0xfffffea2,0xfffffe9c,0xfffffe96,0xfffffe90,0xfffffe8a,0xfffffe84,0xfffffe7e,
     0xfffffe79,0xfffffe73,0xfffffe6d,0xfffffe67,0xfffffe62,0xfffffe5c,0xfffffe56,0xfffffe50,
     0xfffffe4b,0xfffffe45,0xfffffe3f,0xfffffe3a,0xfffffe34,0xfffffe2e,0xfffffe29,0xfffffe23,
     0xfffffe1e,0xfffffe18,0xfffffe13,0xfffffe0d,0xfffffe08,0xfffffe02,0xfffffdfd,0xfffffdf7,
     0xfffffdf2,0xfffffded,0xfffffde7,0xfffffde2,0xfffffddd,0xfffffdd7,0xfffffdd2,0xfffffdcd,
     0xfffffdc8,0xfffffdc2,0xfffffdbd,0xfffffdb8,0xfffffdb3,0xfffffdae,0xfffffda9,0xfffffda4,
     0xfffffd9f,0xfffffd99,0xfffffd94,0xfffffd8f,0xfffffd8b,0xfffffd86,0xfffffd81,0xfffffd7c,
     0xfffffd77,0xfffffd72,0xfffffd6d,0xfffffd68,0xfffffd64,0xfffffd5f,0xfffffd5a,0xfffffd55,
     0xfffffd51,0xfffffd4c,0xfffffd48,0xfffffd43,0xfffffd3e,0xfffffd3a,0xfffffd35,0xfffffd31,
     0xfffffd2c,0xfffffd28,0xfffffd24,0xfffffd1f,0xfffffd1b,0xfffffd17,0xfffffd12,0xfffffd0e,
     0xfffffd0a,0xfffffd06,0xfffffd01,0xfffffcfd,0xfffffcf9,0xfffffcf5,0xfffffcf1,0xfffffced,
     0xfffffce9,0xfffffce5,0xfffffce1,0xfffffcdd,0xfffffcd9,0xfffffcd5,0xfffffcd2,0xfffffcce,
     0xfffffcca,0xfffffcc6,0xfffffcc3,0xfffffcbf,0xfffffcbb,0xfffffcb8,0xfffffcb4,0xfffffcb1,
     0xfffffcad,0xfffffcaa,0xfffffca6,0xfffffca3,0xfffffc9f,0xfffffc9c,0xfffffc99,0xfffffc95,
     0xfffffc92,0xfffffc8f,0xfffffc8c,0xfffffc89,0xfffffc86,0xfffffc82,0xfffffc7f,0xfffffc7c,
     0xfffffc79,0xfffffc76,0xfffffc74,0xfffffc71,0xfffffc6e,0xfffffc6b,0xfffffc68,0xfffffc66,
     0xfffffc63,0xfffffc60,0xfffffc5e,0xfffffc5b,0xfffffc58,0xfffffc56,0xfffffc53,0xfffffc51,
     0xfffffc4e,0xfffffc4c,0xfffffc4a,0xfffffc47,0xfffffc45,0xfffffc43,0xfffffc41,0xfffffc3e,
     0xfffffc3c,0xfffffc3a,0xfffffc38,0xfffffc36,0xfffffc34,0xfffffc32,0xfffffc30,0xfffffc2e,
     0xfffffc2d,0xfffffc2b,0xfffffc29,0xfffffc27,0xfffffc26,0xfffffc24,0xfffffc22,0xfffffc21,
     0xfffffc1f,0xfffffc1e,0xfffffc1c,0xfffffc1b,0xfffffc19,0xfffffc18,0xfffffc17,0xfffffc15,
     0xfffffc14,0xfffffc13,0xfffffc12,0xfffffc11,0xfffffc10,0xfffffc0f,0xfffffc0e,0xfffffc0d,
     0xfffffc0c,0xfffffc0b,0xfffffc0a,0xfffffc09,0xfffffc08,0xfffffc07,0xfffffc07,0xfffffc06,
     0xfffffc05,0xfffffc05,0xfffffc04,0xfffffc04,0xfffffc03,0xfffffc03,0xfffffc02,0xfffffc02,
     0xfffffc02,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,
     0xfffffc00,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,0xfffffc01,
     0xfffffc02,0xfffffc02,0xfffffc02,0xfffffc03,0xfffffc03,0xfffffc04,0xfffffc04,0xfffffc05,
     0xfffffc05,0xfffffc06,0xfffffc07,0xfffffc07,0xfffffc08,0xfffffc09,0xfffffc0a,0xfffffc0b,
     0xfffffc0c,0xfffffc0d,0xfffffc0e,0xfffffc0f,0xfffffc10,0xfffffc11,0xfffffc12,0xfffffc13,
     0xfffffc14,0xfffffc15,0xfffffc17,0xfffffc18,0xfffffc19,0xfffffc1b,0xfffffc1c,0xfffffc1e,
     0xfffffc1f,0xfffffc21,0xfffffc22,0xfffffc24,0xfffffc26,0xfffffc27,0xfffffc29,0xfffffc2b,
     0xfffffc2d,0xfffffc2e,0xfffffc30,0xfffffc32,0xfffffc34,0xfffffc36,0xfffffc38,0xfffffc3a,
     0xfffffc3c,0xfffffc3e,0xfffffc41,0xfffffc43,0xfffffc45,0xfffffc47,0xfffffc4a,0xfffffc4c,
     0xfffffc4e,0xfffffc51,0xfffffc53,0xfffffc56,0xfffffc58,0xfffffc5b,0xfffffc5e,0xfffffc60,
     0xfffffc63,0xfffffc66,0xfffffc68,0xfffffc6b,0xfffffc6e,0xfffffc71,0xfffffc74,0xfffffc76,
     0xfffffc79,0xfffffc7c,0xfffffc7f,0xfffffc82,0xfffffc86,0xfffffc89,0xfffffc8c,0xfffffc8f,
     0xfffffc92,0xfffffc95,0xfffffc99,0xfffffc9c,0xfffffc9f,0xfffffca3,0xfffffca6,0xfffffcaa,
     0xfffffcad,0xfffffcb1,0xfffffcb4,0xfffffcb8,0xfffffcbb,0xfffffcbf,0xfffffcc3,0xfffffcc6,
     0xfffffcca,0xfffffcce,0xfffffcd2,0xfffffcd5,0xfffffcd9,0xfffffcdd,0xfffffce1,0xfffffce5,
     0xfffffce9,0xfffffced,0xfffffcf1,0xfffffcf5,0xfffffcf9,0xfffffcfd,0xfffffd01,0xfffffd06,
     0xfffffd0a,0xfffffd0e,0xfffffd12,0xfffffd17,0xfffffd1b,0xfffffd1f,0xfffffd24,0xfffffd28,
     0xfffffd2c,0xfffffd31,0xfffffd35,0xfffffd3a,0xfffffd3e,0xfffffd43,0xfffffd48,0xfffffd4c,
     0xfffffd51,0xfffffd55,0xfffffd5a,0xfffffd5f,0xfffffd64,0xfffffd68,0xfffffd6d,0xfffffd72,
     0xfffffd77,0xfffffd7c,0xfffffd81,0xfffffd86,0xfffffd8b,0xfffffd8f,0xfffffd94,0xfffffd99,
     0xfffffd9f,0xfffffda4,0xfffffda9,0xfffffdae,0xfffffdb3,0xfffffdb8,0xfffffdbd,0xfffffdc2,
     0xfffffdc8,0xfffffdcd,0xfffffdd2,0xfffffdd7,0xfffffddd,0xfffffde2,0xfffffde7,0xfffffded,
     0xfffffdf2,0xfffffdf7,0xfffffdfd,0xfffffe02,0xfffffe08,0xfffffe0d,0xfffffe13,0xfffffe18,
     0xfffffe1e,0xfffffe23,0xfffffe29,0xfffffe2e,0xfffffe34,0xfffffe3a,0xfffffe3f,0xfffffe45,
     0xfffffe4b,0xfffffe50,0xfffffe56,0xfffffe5c,0xfffffe62,0xfffffe67,0xfffffe6d,0xfffffe73,
     0xfffffe79,0xfffffe7e,0xfffffe84,0xfffffe8a,0xfffffe90,0xfffffe96,0xfffffe9c,0xfffffea2,
     0xfffffea8,0xfffffead,0xfffffeb3,0xfffffeb9,0xfffffebf,0xfffffec5,0xfffffecb,0xfffffed1,
     0xfffffed7,0xfffffedd,0xfffffee3,0xfffffee9,0xfffffeef,0xfffffef5,0xfffffefc,0xffffff02,
     0xffffff08,0xffffff0e,0xffffff14,0xffffff1a,0xffffff20,0xffffff26,0xffffff2c,0xffffff33,
     0xffffff39,0xffffff3f,0xffffff45,0xffffff4b,0xffffff51,0xffffff58,0xffffff5e,0xffffff64,
     0xffffff6a,0xffffff70,0xffffff77,0xffffff7d,0xffffff83,0xffffff89,0xffffff90,0xffffff96,
     0xffffff9c,0xffffffa2,0xffffffa9,0xffffffaf,0xffffffb5,0xffffffbb,0xffffffc2,0xffffffc8,
     0xffffffce,0xffffffd5,0xffffffdb,0xffffffe1,0xffffffe7,0xffffffee,0xfffffff4,0xfffffffa,
};


pplint32 sinus1024 (pplint32 wert)
/*!\ingroup PPLGroupMath
 */
{
  pplint32 pp;
  pp=(wert+256)&1023;
  return (SinusTabelle[pp]);
}

pplint32 cosinus1024 (pplint32 wert)
/*!\ingroup PPLGroupMath
 */
{
  pplint32 pp;
  pp=wert&1023;
  return (SinusTabelle[pp]);
}

pplint32 lowest (pplint32 wert1, pplint32 wert2)
/*!\ingroup PPLGroupMath
 */
{
	if (wert1<wert2) return (wert1);
	return wert2;
}

pplint32 highest (pplint32 wert1, pplint32 wert2)
/*!\ingroup PPLGroupMath
 */
{
	if (wert1>wert2) return (wert1);
	return wert2;
}

pplint32	saturate(pplint32 wert, pplint32 min, pplint32 max)
/*!\ingroup PPLGroupMath
 */
{
	if (wert<min) wert=min;
	if (wert>max) wert=max;
	return wert;
}

} // end of namespace ppl
