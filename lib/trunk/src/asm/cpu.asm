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


%include "src/asm/common.asm"


global HaveMMX
global HaveMMX2
global Have3DNow
global Have3DNow2
global HaveSSE
global HaveSSE2
global HaveSSE3
global HaveSSE4a

global _HaveMMX
global _HaveMMX2
global _Have3DNow
global _Have3DNow2
global _HaveSSE
global _HaveSSE2
global _HaveSSE3
global _HaveSSE4a


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



HaveMMX:
_HaveMMX:
	; test whether extended function 80000001h is supported
	mov eax, 80000000h		; call extended function 80000000h
	push rbx
	cpuid
	pop rbx
	cmp eax, 80000000h		; supports functions > 80000000h?
	jbe HaveMMX_NO_EXTENDED
		mov eax,1
		ret
	HaveMMX_NO_EXTENDED:
	xor eax,eax
	ret

HaveMMX2:
_HaveMMX2:
		; test whether extended function 80000001h is supported
		mov eax, 80000000h			; call extended function 80000000h
		push rbx
		cpuid
		cmp eax, 80000000h			; supports functions > 80000000h?
		jbe HaveMMX2_NO_EXTENDED
			; test if function 80000001h indicates MMX Extension support
			mov eax, 80000001h		; call extended function 80000001h
			cpuid					; reports back extended feature flags
			test edx, 400000h		; bit 22 in extended features
			jz HaveMMX2_NO_3DNow	; if set, MMX Extension is supported
				mov eax,1
				pop rbx
				ret
		HaveMMX2_NO_3DNow:
		HaveMMX2_NO_EXTENDED:
		pop rbx
		xor eax,eax
		ret

Have3DNow:
_Have3DNow:
		; test whether extended function 80000001h is supported
		mov eax, 80000000h			; call extended function 80000000h
		push rbx
		cpuid
		cmp eax, 80000000h			; supports functions > 80000000h?
		jbe Have3DNow_NO			; no 3DNow! support either
			; test if function 80000001h indicates 3DNow! support
			mov eax, 80000001h		; call extended function 80000001h
			cpuid					; reports back extended feature flags
			test edx, 80000000h	; bit 31 in extended features
			jz Have3DNow_NO			; if set, 3DNow! is supported
				mov eax,1
				pop rbx
				ret
		Have3DNow_NO:
		xor eax,eax
		pop rbx
		ret

Have3DNow2:
_Have3DNow2:
		;test whether extended function 80000001h is supported
		mov eax, 80000000h			; call extended function 80000000h
		push rbx
		cpuid
		cmp eax, 80000000h			; supports functions > 80000000h?
		jbe Have3DNow2_NO
			; test if function 80000001h indicates 3DNow! Extension support
			mov eax, 80000001h		; call extended function 80000001h
			cpuid					; reports back extended feature flags
			test edx, 40000000h	; bit 30 in extended features
			jz Have3DNow2_NO		; if set, 3DNow! Extension is supported
				mov eax,1
				pop rbx
				ret
		Have3DNow2_NO:
		xor eax,eax
		pop rbx
		ret

HaveSSE:
_HaveSSE:
		xor edx,edx
		push rbx
		mov eax, 1				; standard feature flags auslesen
		cpuid
		test edx, 2000000h		; bit 25 in extended features
		jz NO_SSE				; if set, SSE is supported
			mov eax,1
			pop rbx
			ret
		NO_SSE:
		xor eax,eax
		pop rbx
		ret

HaveSSE2:
_HaveSSE2:
		xor edx,edx
		push rbx
		mov eax, 1				; standard feature flags auslesen
		cpuid
		test edx, 4000000h		; bit 26 in extended features
		jz NO_SSE2				; if set, SSE2 is supported
			mov eax,1
			pop rbx
			ret
		NO_SSE2:
		xor eax,eax
		pop rbx
		ret

HaveSSE3:
_HaveSSE3:
		xor edx,edx
		push rbx
		mov eax, 1				; standard feature flags auslesen
		cpuid
		test ecx, 000001h		; bit 0 in ecx
		jz NO_SSE3				; if set, SSE2 is supported
			mov eax,1
			pop rbx
			ret
		NO_SSE3:
		pop rbx
		xor eax,eax
		ret

HaveSSE4a:
_HaveSSE4a:
		xor edx,edx
		push rbx
		mov eax, 80000001h			; call extended function 80000000h
		cpuid
		test ecx, 000040h		; bit 6 in ecx
		jz NO_SSE4a				; if set, SSE2 is supported
			mov eax,1
			pop rbx
			ret
		NO_SSE4a:
		xor eax,eax
		pop rbx
		ret


