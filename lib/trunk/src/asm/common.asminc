;/*******************************************************************************
;/* This file is part of "Patrick's Programming Library", Version 7 (PPL7).
; * Web: http://www.pfp.de/ppl/
; *
; * $Author: pafe $
; * $Revision: 1.2 $
; * $Date: 2010/02/12 19:43:56 $
; * $Id: common.asminc,v 1.2 2010/02/12 19:43:56 pafe Exp $
; * $URL$
; *
;/*******************************************************************************
; * Copyright (c) 2011, Patrick Fedick <patrick@pfp.de>
; * All rights reserved.
; *
; * Redistribution and use in source and binary forms, with or without
; * modification, are permitted provided that the following conditions are met:
; *     * Redistributions of source code must retain the above copyright
; *       notice, this list of conditions and the following disclaimer.
; *     * Redistributions in binary form must reproduce the above copyright
; *       notice, this list of conditions and the following disclaimer in the
; *       documentation and/or other materials provided with the distribution.
; *     * Neither the name of the copyright holder nor the names of its
; *       contributors may be used to endorse or promote products derived
; *       from this software without specific prior written permission.
; *
; * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
; * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
; * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
; * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
; * THE POSSIBILITY OF SUCH DAMAGE.
; *******************************************************************************/


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



struc DRAWABLE_DATA
	.GRAFIX_FUNCTIONS:		PTR		1
	.base:					PTR		1
	.pitch:					resd	1
	.width:					resd	1
	.height:				resd	1
endstruc
