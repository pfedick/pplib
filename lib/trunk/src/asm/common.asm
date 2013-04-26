;###############################################################################
;# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
;# Web: http://www.pfp.de/ppl/
;#
;# $Author$
;# $Revision$
;# $Date$
;# $Id$
;#
;###############################################################################
;#
;# Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
;# All rights reserved.
;#
;# Redistribution and use in source and binary forms, with or without
;# modification, are permitted provided that the following conditions are met:
;#
;#   1. Redistributions of source code must retain the above copyright notice,
;#      this list of conditions and the following disclaimer.
;#   2. Redistributions in binary form must reproduce the above copyright notice,
;#      this list of conditions and the following disclaimer in the documentation
;#      and/or other materials provided with the distribution.
;#
;# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
;# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;# POSSIBILITY OF SUCH DAMAGE.
;#
;###############################################################################


%define elf64 0
%define win64 0
%define elf32 0
%define win32 0
%define PTR resd


%ifidn __OUTPUT_FORMAT__, elf64
%ifidn __BITS__, 64
%define elf64 1
%define PTR resq
%endif
%endif

%ifidn __OUTPUT_FORMAT__, elf32
%ifidn __BITS__, 32
%define elf32 1
%endif
%endif

%ifidn __OUTPUT_FORMAT__, win64
%ifidn __BITS__, 64
%define win64 1
%define PTR resq
%endif
%endif

%ifidn __OUTPUT_FORMAT__, win32
%ifidn __BITS__, 32
%define win32 1
%endif
%endif


%define BIT0	0x00000001
%define BIT1	0x00000002
%define BIT2	0x00000004
%define BIT3	0x00000008
%define BIT4	0x00000010
%define BIT5	0x00000020
%define BIT6	0x00000040
%define BIT7	0x00000080
%define BIT8	0x00000100
%define BIT9	0x00000200
%define BIT10	0x00000400
%define BIT11	0x00000800
%define BIT12	0x00001000
%define BIT13	0x00002000
%define BIT14	0x00004000
%define BIT15	0x00008000
%define BIT16	0x00010000
%define BIT17	0x00020000
%define BIT18	0x00040000
%define BIT19	0x00080000
%define BIT20	0x00100000
%define BIT21	0x00200000
%define BIT22	0x00400000
%define BIT23	0x00800000
%define BIT24	0x01000000
%define BIT25	0x02000000
%define BIT26	0x04000000
%define BIT27	0x08000000
%define BIT28	0x10000000
%define BIT29	0x20000000
%define BIT30	0x40000000
%define BIT31	0x80000000



struc DRAWABLE_DATA
	.GRAFIX_FUNCTIONS:		PTR		1
	.base:					PTR		1
	.pitch:					resd	1
	.width:					resd	1
	.height:				resd	1
endstruc
