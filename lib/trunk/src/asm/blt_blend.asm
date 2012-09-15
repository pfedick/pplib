;/*******************************************************************************
;/* This file is part of "Patrick's Programming Library", Version 7 (PPL7).
; * Web: http://www.pfp.de/ppl/
; *
; * $Author: pafe $
; * $Revision: 446 $
; * $Date: 2012-06-09 15:19:11 +0200 (Sa, 09 Jun 2012) $
; * $Id: blt.asm 446 2012-06-09 13:19:11Z pafe $
; * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/src/asm/blt.asm $
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


;/*********************************************************************
;/** blt.asm                                                         **
;/*********************************************************************

%include "src/asm/common.asm"


SECTION .text

struc BLTDATA
	src:		PTR		1			; ebp+0
	tgt:		PTR		1			; ebp+4
	width:		resd	1			; ebp+8
	height:		resd	1			; ebp+12
	pitchsrc:	resd	1			; ebp+16
	pitchtgt:	resd	1			; ebp+20
	color:		resd	1			; ebp+24
endstruc



;/*********************************************************************
;/** ASM_BltBlend32_MMX                                              **
;/**                                                                 **
;/** void ASM_BltBlend32_MMX(BLTDATA *data, int factor)              **
;/*********************************************************************
global ASM_BltBlend32_MMX
global _ASM_BltBlend32_MMX
ASM_BltBlend32_MMX:
_ASM_BltBlend32_MMX:

%if elf64=1 || win64=1
	%if elf64=1
		mov r8,rdi					; Pointer nach r8 schieben
		movd mm4,esi				; Faktor ist in rsi, wovon uns aber nur das untere Byte interessiert
	%elif win64=1
		mov r8,rcx
		movd mm4,edx				;// Faktor nach mm4
		push rdi
		push rsi
	%endif
	; In: r8=Pointer auf BLTDATA-Struktur
	;     mm4=Faktor
	push rbx
	pxor mm6,mm6
	mov eax,0x00ff00ff
	mov rdi,[r8+tgt]			;// Zieladresse nach r8 => rdi
	movd mm7,eax
	mov rsi,[r8+src]
	mov r10d,[r8+pitchsrc]
	mov r11d,[r8+pitchtgt]
	sub rdi,4					;// Wir gehen von rechts nach links, von Breite bis 1 (nicht 0),
	sub rsi,4					;// daher müssen wir bei der Basisadresse eine Position nach links gehen
	mov edx,[r8+height]
	pshufw mm7,mm7, 0
	mov r9d,[r8+width]			; Breite nach r9
	movq mm5,mm7
	pshufw mm4,mm4,0
	psubusw mm5,mm4

	jmp near .aYLoop
	ALIGN 16
	.aYLoop:
		mov ecx,r9d
		jmp near .aXLoop
		ALIGN 16
		.aXLoop:
			movd mm0,[rdi+rcx*4]	;// Quellfarbe
			movd mm2,[rsi+rcx*4]	;// Blendfarbe
			punpcklbw mm0,mm6
			punpcklbw mm2,mm6
			pmullw mm0,mm5
			pmullw mm2,mm4
			psrlw mm0,8
			psrlw mm2,8
			paddusw mm0,mm2
			packuswb mm0,mm6
			movd [rdi+rcx*4],mm0
			dec ecx
			jnz .aXLoop
		add rsi,r10
		add rdi,r11
		dec edx
		jnz .aYLoop
	emms
	xor rax,rax
	inc rax					; Returnwert auf 1 setzen
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret


%else			; Die 32-Bit Variante
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp
		movd mm4,[esp+12];			; Faktor nach mm4
		pushad
		pxor mm6,mm6
		mov eax,0x00ff00ff
		mov edi,[ebp+tgt]
		movd mm7,eax
		mov esi,[ebp+src]
		pshufw mm7,mm7, 0
		pshufw mm4,mm4,0

		sub edi,4					;// ?? Wofür soll das gut sein???
		sub esi,4					;// ?? Dito
		movq mm5,mm7
		mov edx,[ebp+height]
		psubusw mm5,mm4

		jmp near .aYLoop
		ALIGN 16
		.aYLoop:
			mov ecx, [ebp+width]
			jmp near .aXLoop
			ALIGN 16
			.aXLoop:
				movd mm0,[edi+ecx*4]
				movd mm2,[esi+ecx*4]
				punpcklbw mm0,mm6
				punpcklbw mm2,mm6
				pmullw mm0,mm5
				pmullw mm2,mm4
				psrlq mm0,8
				psrlq mm2,8
				paddusw mm0,mm2
				packuswb mm0,mm6
				movd [edi+ecx*4],mm0
				dec ecx
				jnz .aXLoop
			add esi,[ebp+pitchsrc]
			add edi,[ebp+pitchtgt]
			dec edx
			jnz .aYLoop
		emms
		popad
		xor eax,eax
		pop ebp
		inc eax						; Returnwert auf 1 setzen
		ret
%endif


;/*********************************************************************
;/** ASM_BltBlend32_SSE1                                                **
;/**                                                                 **
;/** void ASM_Blend32_PPL6(BLTDATA *data, int factor)                **
;/*********************************************************************
ALIGN 16
blendmask:		dd	0x00ff00ff
				dd	0x00ff00ff
				dd	0x00ff00ff
				dd	0x00ff00ff

ALIGN 16
global ASM_BltBlend32_SSE_Align1
global _ASM_BltBlend32_SSE_Align1

ASM_BltBlend32_SSE_Align1:
_ASM_BltBlend32_SSE_Align1:
%if elf64=1 || win64=1
	%if elf64=1
		mov r8,rdi					; Pointer nach r8 schieben
		movd xmm4,esi				; Faktor ist in rsi, wovon uns aber nur das untere Byte interessiert
	%elif win64=1
		mov r8,rcx
		movd xmm4,edx				;// Faktor nach mm4
		push rdi
		push rsi
	%endif
	; In: r8=Pointer auf BLTDATA-Struktur
	;     mm4=Faktor
	push rbx
	pxor xmm1,xmm1
	mov rdi,[r8+tgt]			;// Zieladresse nach r8 => rdi
	movdqa xmm5,[blendmask]
	mov rsi,[r8+src]
	mov r10d,[r8+pitchsrc]
	mov r11d,[r8+pitchtgt]
	sub rdi,4					;// Wir gehen von rechts nach links, von Breite bis 1 (nicht 0),
	sub rsi,4					;// daher müssen wir bei der Basisadresse eine Position nach links gehen
	mov edx,[r8+height]
	pshuflw xmm7,xmm7,0
	pshuflw xmm4,xmm4,0
	mov r9d,[r8+width]			; Breite nach r9
	psubusw xmm5,xmm4
	mov ecx,r9d

	jmp near .aXLoop
		ALIGN 16
		.aXLoop:
			movd xmm0,[rdi+rcx*4]	;// Quellfarbe
			movd xmm2,[rsi+rcx*4]	;// Blendfarbe
			punpcklbw xmm0,xmm1
			punpcklbw xmm2,xmm1
			pmullw xmm0,xmm5
			pmullw xmm2,xmm4
			psrlw xmm0,8
			psrlw xmm2,8
			paddusw xmm0,xmm2
			packuswb xmm0,xmm1
			movd [rdi+rcx*4],xmm0
			;sub rcx,1
			;dec rcx
			;sub ecx,1
			dec ecx
			jnz .aXLoop
		add rsi,r10
		add rdi,r11
		mov ecx,r9d
		dec edx
		jnz .aXLoop
	emms
	xor rax,rax
	inc rax					; Returnwert auf 1 setzen
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret


%else			; Die 32-Bit Variante
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp
		movd xmm4,[esp+12];			; Faktor nach mm4
		pushad
		mov eax, 0xff000000
		pxor xmm1,xmm1
		movd xmm3,eax				;// Fuer Or-Maske, damit Alpha-Channel immer ff ist
		mov eax,0x00ff00ff
		mov edi,[ebp+tgt]
		movd xmm7,eax
		mov esi,[ebp+src]
		pshuflw xmm7,xmm7, 0
		pshuflw xmm4,xmm4,0

		sub edi,4					;// ?? Wofür soll das gut sein???
		sub esi,4					;// ?? Dito
		movq xmm5,xmm7
		mov edx,[ebp+height]
		psubusw xmm5,xmm4

		jmp near .aYLoop
		ALIGN 16
		.aYLoop:
			mov ecx, [ebp+width]
			jmp near .aXLoop
			ALIGN 16
			.aXLoop:
				movd xmm0,[edi+ecx*4]
				movd xmm2,[esi+ecx*4]
				punpcklbw xmm0,xmm1
				punpcklbw xmm2,xmm1
				pmullw xmm0,xmm5
				pmullw xmm2,xmm4
				psrlq xmm0,8
				psrlq xmm2,8
				pand xmm0,xmm7
				pand xmm2,xmm7
				paddusw xmm0,xmm2
				packuswb xmm0,xmm1
				por xmm0, xmm3		;// Alpha-Channel auf ff
				movd [edi+ecx*4],xmm0
				dec ecx
				jnz .aXLoop
			add esi,[ebp+pitchsrc]
			add edi,[ebp+pitchtgt]
			dec edx
			jnz .aYLoop
		emms
		popad
		xor eax,eax
		pop ebp
		inc eax						; Returnwert auf 1 setzen
		ret
%endif


;/*********************************************************************
;/** ASM_BltBlend32_SSE2                                             **
;/**                                                                 **
;/** void ASM_BltBlend32_SSE2(BLTDATA *data, int factor)             **
;/*********************************************************************
ALIGN 16
global ASM_BltBlend32_SSE_Align2
global _ASM_BltBlend32_SSE_Align2

ASM_BltBlend32_SSE_Align2:
_ASM_BltBlend32_SSE_Align2:
%if elf64=1 || win64=1
	%if elf64=1
		mov r8,rdi					; Pointer nach r8 schieben
		movd xmm4,esi				; Faktor ist in rsi, wovon uns aber nur das untere Byte interessiert
	%elif win64=1
		mov r8,rcx
		movd xmm4,edx				;// Faktor nach mm4
		push rdi
		push rsi
	%endif
	; In: r8=Pointer auf BLTDATA-Struktur
	;     mm4=Faktor
	push rbx
	pxor xmm1,xmm1
	pshuflw xmm4,xmm4,0
	mov rdi,[r8+tgt]			;// Zieladresse nach r8 => rdi
	movdqa xmm5,[blendmask]
	mov rsi,[r8+src]
	mov r10d,[r8+pitchsrc]
	pshufd xmm4,xmm4,0
	mov r11d,[r8+pitchtgt]
	sub rdi,8					;// Wir gehen von rechts nach links, von Breite bis 1 (nicht 0),
	sub rsi,8					;// daher müssen wir bei der Basisadresse eine Position nach links gehen
	mov edx,[r8+height]
	mov r9d,[r8+width]			; Breite nach r9
	shr r9,1
	psubusw xmm5,xmm4
	mov rcx,r9

	jmp near .aXLoop
		ALIGN 16
		.aXLoop:
			movq xmm0,[rdi+rcx*8]	;// Quellfarbe
			movq xmm2,[rsi+rcx*8]	;// Blendfarbe
			punpcklbw xmm0,xmm1
			punpcklbw xmm2,xmm1
			pmullw xmm0,xmm5
			pmullw xmm2,xmm4
			psrlw xmm0,8
			psrlw xmm2,8
			paddusw xmm0,xmm2
			packuswb xmm0,xmm1
			movq [rdi+rcx*8],xmm0
			sub rcx,1
			jnz .aXLoop
		add rsi,r10
		add rdi,r11
		mov ecx,r9d
		dec edx
		jnz .aXLoop
	emms
	xor rax,rax
	inc rax					; Returnwert auf 1 setzen
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret


%else			; Die 32-Bit Variante
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp
		movd xmm4,[esp+12];			; Faktor nach mm4
		pushad
		pxor xmm1,xmm1
		pshuflw xmm4,xmm4,0
		movdqa xmm5,[blendmask]
		mov edi,[ebp+tgt]

		mov esi,[ebp+src]
		pshufd xmm4,xmm4,0

		sub edi,8
		sub esi,8
		shr [ebp+width],1			;// Breite durch 2 teilen

		mov edx,[ebp+height]
		psubusw xmm5,xmm4
		mov ecx, [ebp+width]

		jmp near .aXLoop
		ALIGN 16
			ALIGN 16
			.aXLoop:
				movq xmm0,[edi+ecx*8]
				movq xmm2,[esi+ecx*8]
				punpcklbw xmm0,xmm1
				punpcklbw xmm2,xmm1
				pmullw xmm0,xmm5
				pmullw xmm2,xmm4
				psrlw xmm0,8
				psrlw xmm2,8
				paddusw xmm0,xmm2
				packuswb xmm0,xmm1
				movq [edi+ecx*8],xmm0
				dec ecx
				jnz .aXLoop
			add esi,[ebp+pitchsrc]
			add edi,[ebp+pitchtgt]
			mov ecx, [ebp+width]
			dec edx
			jnz .aXLoop
		emms
		popad
		xor eax,eax
		pop ebp
		inc eax						; Returnwert auf 1 setzen
		ret
%endif
