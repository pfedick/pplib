;/*******************************************************************************
;/* This file is part of "Patrick's Programming Library", Version 7 (PPL7).
; * Web: http://www.pfp.de/ppl/
; *
; * $Author$
; * $Revision$
; * $Date$
; * $Id$
; * $Url$
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


%include "src/asm/common.asm"


global PPL7_GetCpuCaps
global _PPL7_GetCpuCaps


SECTION .data
SECTION .text

;/*********************************************************************
;/** GetASMBits                                                      **
;/**                                                                 **
;/** ppluint32 GetASMBits()                                          **
;/*********************************************************************
%ifidn __OUTPUT_FORMAT__, elf64
	global GetASMBits
	GetASMBits:
		xor rax,rax
		mov al, __BITS__
		ret

%elifidn __OUTPUT_FORMAT__, win64
	global _GetASMBits
	_GetASMBits:
		xor rax,rax
		mov al, __BITS__
		ret
%elifidn __OUTPUT_FORMAT__, win32
	global _GetASMBits
	_GetASMBits:
		xor eax,eax
		mov al, __BITS__
		ret

%elifidn __OUTPUT_FORMAT__, elf32
	global GetASMBits
	GetASMBits:
		xor eax,eax
		mov al, __BITS__
		ret

%endif

;/*********************************************************************
;/** HaveCPUID                                                       **
;/**                                                                 **
;/** ppluint32 HaveCPUID()                                           **
;/*********************************************************************

%ifidn __OUTPUT_FORMAT__, elf64
	global HaveCPUID
	HaveCPUID:
		; check whether CPUID is supported
		; (bit 21 of Eflags can be toggled)
		; // cli							// Interupts abschalten
		pushfq						; save Eflags
		pop rax						; transfers Eflags into EAX
		mov rdx, rax				; save original Flags
		xor eax, 00200000h			; toggle bit 21
		push rax					; put new value of stack
		popfq						; transfer new value to Eflags
		pushfq						; save updated Eflags
		pop rax						; transfers Eflags into EAX
		xor eax,edx					; updated Eflags and original differ?
		jz NO_CPUID					; no differ, bit 21 can't be toggled
			xor rax,rax
			mov al,1
			ret
		NO_CPUID:
			xor rax,rax
			ret

%else
	global HaveCPUID
	global _HaveCPUID

	HaveCPUID:
	_HaveCPUID:
		; check whether CPUID is supported
		; (bit 21 of Eflags can be toggled)
		; // cli							// Interupts abschalten
		pushfd						; save Eflags
		pop eax						; transfers Eflags into EAX
		mov edx, eax				; save original Flags
		xor eax, 00200000h			; toggle bit 21
		push eax					; put new value of stack
		popfd						; transfer new value to Eflags
		pushfd						; save updated Eflags
		pop eax						; transfers Eflags into EAX
		xor eax,edx					; updated Eflags and original differ?
		jz NO_CPUID					; no differ, bit 21 can't be toggled
			mov eax,1
			ret
		NO_CPUID:
			xor eax,eax
			ret
%endif





GetCpuCaps_Vendor_AMD:
	mov eax, 80000001h		; call extended function 80000001h
	cpuid					; reports back extended feature flags
	; SSE4a
	test ecx, BIT6			; bit 6 in extended features
	jz .NO_SSE4a			; if set, SSE4a is supported
		or edi,0x0400
	.NO_SSE4a:
	; 3DNow
	test edx, BIT31			; bit 31 in extended features
	jz .NO_3DNow			; if set, 3DNow is supported
		or edi,0x08
	.NO_3DNow:
	; 3DNowExt
	test edx, BIT30			; bit 30 in extended features
	jz .NO_3DNowExt			; if set, 3DNowExt is supported
		or edi,0x010
	.NO_3DNowExt:
	; MMX
	test edx, BIT23			; bit 23 in extended features
	jz .NO_MMX				; if set, MMX is supported
		or edi,0x02
	.NO_MMX:

	; MMX2
	test edx, BIT22			; bit 22 in extended features
	jz .NO_MMX2				; if set, MMX2 is supported
		or edi,0x04
	.NO_MMX2:


	ret


PPL7_GetCpuCaps:
_PPL7_GetCpuCaps:
	%ifidn __OUTPUT_FORMAT__, elf64
		push rbx
		push rdi
		xor rdi,rdi
		or edi,0x80
	%elifidn __OUTPUT_FORMAT__, win64
		push rbx
		push rdi
		xor rdi,rdi
		or edi,0x80
	%elifidn __OUTPUT_FORMAT__, win32
		push ebx
		push edi
		xor edi,edi
	%elifidn __OUTPUT_FORMAT__, elf32
		push ebx
		push edi
		xor edi,edi
	%endif
	; In EDI bauen wir den Returnwert zusammen
	call HaveCPUID		; Als erstes prüfen wir, ob wir den Befehl CPUID haben
	cmp al,1
	jne .end			; Nein, dann gibt es nichts mehr zu tun
	or edi,1

	mov eax, 1			; CPUID Funktion Fn0000_0001 aufrufen
	cpuid

	; MMX
	test edx, BIT23		; bit 23 in extended features
	jz .NO_MMX			; if set, MMX is supported
		or edi,0x02
	.NO_MMX:
	; SSE
	test edx, BIT25		; bit 25 in extended features
	jz .NO_SSE			; if set, SSE is supported
		or edi,0x20
	.NO_SSE:
	; SSE2
	test edx, BIT26		; bit 26 in extended features
	jz .NO_SSE2			; if set, SSE2 is supported
		or edi,0x40
	.NO_SSE2:
	; SSE3
	test ecx, BIT0		; bit 0 in extended features
	jz .NO_SSE3			; if set, SSE3 is supported
		or edi,0x100
	.NO_SSE3:
	; SSSE3
	test ecx, BIT9		; bit 9 in extended features
	jz .NO_SSSE3		; if set, SSSE3 is supported
		or edi,0x200
	.NO_SSSE3:
	; SSE4.1
	test ecx, BIT19		; bit 19 in extended features
	jz .NO_SSE41		; if set, SSE4.1 is supported
		or edi,0x400
	.NO_SSE41:
	; SSE4.2
	test ecx, BIT20		; bit 15 in extended features
	jz .NO_SSE42		; if set, SSE4.2 is supported
		or edi,0x800
	.NO_SSE42:
	; AES
	test ecx, BIT25		; bit 25 in extended features
	jz .NO_AES		; if set, AES is supported
		or edi,0x2000
	.NO_AES:

	; Extended Function vorhanden?
	mov eax, 80000000h		; call extended function 80000000h
	cpuid
	cmp eax, 80000000h		; supports functions > 80000000h?
	jbe .end				; keine Vorhanden, wir sind fertig
	; Prozessor Vendor AMD?
		cmp ebx,6874_7541h
		jne .noAMD
		cmp ecx,444D_4163h
		jne .noAMD
		cmp edx,6974_6E65h
		jne .noAMD
		call GetCpuCaps_Vendor_AMD
	.noAMD:

	.end:
	mov eax,edi
	%ifidn __OUTPUT_FORMAT__, elf64
		pop rdi
		pop rbx
		xor rdx, rdx
	%elifidn __OUTPUT_FORMAT__, win64
		pop rdi
		pop rbx
		xor rdx, rdx
	%elifidn __OUTPUT_FORMAT__, win32
		pop edi
		pop ebx
		xor edx,edx
	%elifidn __OUTPUT_FORMAT__, elf32
		pop edi
		pop ebx
		xor edx,edx
	%endif
	ret
