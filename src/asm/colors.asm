;###############################################################################
;# This file is part of "Patrick's Programming Library", Version 7 (PPL7).
;# Web: http://www.pfp.de/ppl/
;#
;###############################################################################
;#
;# Copyright (c) 2023, Patrick Fedick <patrick@pfp.de>
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

%include "src/asm/common.asm"



SECTION .data
SECTION .text

;/*********************************************************************
;/** ASM_RGB_A8R8G8B8                                                **
;/**                                                                 **
;/** void ASM_RGB_A8R8G8B8(int red, int green, int blue, int alpha)  **
;/*********************************************************************
%ifidn __OUTPUT_FORMAT__, elf64
		global RGB_A8R8G8B8
		RGB_A8R8G8B8:	; red=rdi, green=rsi, blue=rdx, alpha=rcx
			mov r8d,edi
			mov ah,cl
			mov al,r8b
			mov ecx,esi
			shl eax,16
			mov ah,cl
			mov al,dl
			ret
%elifidn __OUTPUT_FORMAT__, win64
		global RGB_A8R8G8B8
		RGB_A8R8G8B8:	; red=rcx, green=rdx, blue=r8, alpha=r9
			; r9,rcx,rdx,r8
			xor rax,rax
			mov al,r9b
			shl rax,8
			mov al,cl
			shl rax,16
			mov ah,dl
			mov al,r8b
			ret
%else
	global RGB_A8R8G8B8
	global _RGB_A8R8G8B8
	RGB_A8R8G8B8:
	_RGB_A8R8G8B8:
		push ebp
		mov ebp,esp
		mov ecx,[ebp+20]	;	alpha
		mov eax,[ebp+8]		;	red
		mov ah,cl
		mov edx,[ebp+12]	;	green
		shl eax,16
		mov ecx,[ebp+16]	;	blue
		mov ah,dl
		mov al,cl
		pop ebp
		ret
%endif

;/*********************************************************************
;/** RGBBlend_32_255                                                 **
;/**                                                                 **
;/** void RGBBlend_32_255(int ground, int top, int intensity         **
;/*********************************************************************
%if arch_elf64=1 || arch_win64=1
	%if arch_elf64=1
		global RGBBlend_32_255
		RGBBlend_32_255:			; ground=rdi, top=rsi, intensity=rdx
		or rdx,rdx					; Alpha=0?
		jz .useBackground			; Wenn intensity=0 ist, dann geben wir ground zurück
			inc dl
			jz .useForeground		; Wenn intensity=255 ist, dann geben wir top zurück
			pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
			movq mm2,rdi			; Hintergrund nach mm2
			movq mm3,rsi			; Color nach mm3
			movq mm0,rdx			; Alphachannel nach mm0

	%elif arch_win64=1
		global RGBBlend_32_255
		RGBBlend_32_255:			; ground=rcx, top=rdx, intensity=r8
		or r8,r8					; Alpha=0?
		jz .useBackground			; Wenn Alpha=0 ist, dann geben wir ground zurück
			inc r8b
			jz .useForeground		; Wenn Alpha=255 ist, dann geben wir top zurück
			pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
			movq mm2,rcx			; Hintergrund nach mm2
			movq mm3,rdx			; Color nach mm3
			movq mm0,r8 			; Alphachannel nach mm0
	%endif
	; Der Rest ist identisch auf beiden ABIs
			punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
			punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
			pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
			psubsw mm3,mm2			; src-dst mit Vorzeichen
			mov edx, 0xff
			pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
			movq mm7,rdx
			psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
			pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 zum Maskieren
			paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
			pand mm3,mm7			; Die oberen Bytes ausmaskieren
			packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
			movq rax,mm3
			emms
			ret

		.useBackground:
		mov rax,INT_PARAM1
		ret

		.useForeground:
		mov rax,INT_PARAM2
		ret


%else
	global RGBBlend_32_255
	global _RGBBlend_32_255
	RGBBlend_32_255:
	_RGBBlend_32_255:
		push ebp
		mov ebp,esp
		mov eax,[ebp+16]			; Alpha nach eax
		and eax,0xff
		jz .useBackground			; Wenn Alpha=0 ist, dann geben wir ground zurück
			inc al
			jz .useForeground		; Wenn Alpha=255 ist, dann geben wir top zurück
			pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
			movd mm2,[ebp+8]		; Hintergrund nach mm2
			movd mm3,[ebp+12]		; Color nach mm3
			movd mm0,eax			; Alphachannel nach mm0
			punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
			punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
			pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
			psubsw mm3,mm2			; src-dst mit Vorzeichen
			mov edx,0xff
			pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
			movd mm7,edx
			psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
			pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 zum Maskieren
			paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
			pand mm3,mm7			; Die oberen Bytes ausmaskieren
			packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
			movd eax,mm3
			emms
			;or eax,0xff000000
			pop ebp
			ret
	.useBackground:
		mov eax,[ebp+8]
		;or eax,0xff000000
		pop ebp
		ret
	.useForeground:
		mov eax,[ebp+12]
		;or eax,0xff000000
		pop ebp
		ret
%endif
