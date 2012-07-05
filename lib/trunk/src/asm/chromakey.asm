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
; * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
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

struc BLTCHROMADATA
	sadr:		PTR		1
	bgadr:		PTR		1
	tgadr:		PTR		1
	spitch:		resd	1
	bgpitch:	resd	1
	tgpitch:	resd	1
	width:		resd	1
	height:		resd	1
	cb_key:		resd	1
	cr_key:		resd	1
	tola:		resd	1
	tolb:		resd	1
endstruc



ALIGN 16
%if elf64=1 || win64=1
	%if elf64=1
		global ASM_BltChromaKey32
		ASM_BltChromaKey32:					; Pointer auf data ist in rdi
		mov r8,rdi							; Pointer nach r8 schieben
	%elif win64=1
		global _ASM_BltChromaKey32
		_ASM_BltChromaKey32:				; Pointer auf data ist zunächst in rcx
		mov r8,rcx							; Pointer nach r8 schieben
	%endif
	; Prüfen, ob Breite durch 4 teilbar ist und Speicheradressen der Grafiken durch 16
	mov eax, [r8+width]
	and al,3
	jnz ASM_BltChromaKey32_1
	mov rax, [r8+sadr]
	and al,15
	jnz ASM_BltChromaKey32_1
	mov rax, [r8+bgadr]
	and al,15
	jnz ASM_BltChromaKey32_1
	jmp ASM_BltChromaKey32_4
%else								; 32-Bit-Version
	global ASM_BltChromaKey32
	global _ASM_BltChromaKey32

	ASM_BltChromaKey32:
	_ASM_BltChromaKey32:
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp

		xor eax,eax					; Returnwert auf 0 setzen
		pop ebp
		ret
%endif


;*******************************************************************************************************
;* Die nachfolgende Funktion ist zwar SSE2 optimiert, verarbeitet aber immer nur einen Pixel gleichzeitig.
;* Sie kann daher verwendet werden, wenn die Breite nicht durch 4 oder die Bitmap-Adressen nicht durch
;* 16 teilbar sind
;*******************************************************************************************************

ALIGN 16
sflt1:	dd	0.5
sflt2:	dd	-0.168736
sflt3:	dd	-0.331264
sflt4:	dd	-0.418688
sflt5:	dd	-0.081312
sflt6:	dd	128.0
sflt7:	dd	1.0
sflt8:	dd	255.0
sflt9:	dd	0.0

ALIGN 16
akehr:  dq 0x00ff00ff00ff00ff
		dq 0x00ff00ff00ff00ff


ALIGN 16
ASM_BltChromaKey32_1:
%if elf64=1 || win64=1
	; IN: r8 => Pointer auf Datenstruktur
	%if win64=1
		push rdi							; rdi und rsi müssen unter WIN64 gerettet werden
		push rsi
	%endif
	push rbx
	push r12
	push r13
	mov rsi,[r8+bgadr]			;// rsi: Pointer auf Background
	mov r13,[r8+tgadr]			;// r13: Pointer auf Ziel
	mov r10d,[r8+spitch]		;// r10: Pitch von Quelle
	mov r11d,[r8+bgpitch]		;// r11: Pitch vom Background
	mov r12d,[r8+tgpitch]		;// r12: Pitch vom ziel
	mov r9d,[r8+width]			;// r9:  Breite
	mov ebx,[r8+height]			;// ebx: Hoehe
	cvtsi2ss xmm8,[r8+cb_key]	; // xmm8=Cb_key
	cvtsi2ss xmm9,[r8+cr_key]	; // xmm8=Cr_key
	cvtsi2ss xmm2,[r8+tola]		; // xmm2=tola
	cvtsi2ss xmm3,[r8+tolb]		; // xmm2=tolb

	mov r8,[r8+sadr]			;// r8: Pointer auf Quelle/Ziel
	;movss xmm8, [sflt7]	; 1
	;pxor xmm9,xmm9

	jmp near .aYLoop
	ALIGN 16
	.aYLoop:
		mov ecx,r9d					; Breite nach ecx laden
		xor eax,eax
		jmp near .aXLoop
		ALIGN 16
		.aXLoop:
			mov edi,[r8+rcx*4]	; Farbe nach edi
			movss xmm0,xmm8
			movss xmm1,xmm9

			; *************************************************************************
			; ** ColorClose berechnen
			; **
			; ** IN: edi=farbe, xmm0=Cb_key, xmm1=Cr_key, xmm2=tola, xmm3=tolb
			; ** OUT: xmm1
			; **
			; ** UNCHANGED: edi, xmm2, xmm3, r8, r9...
			; ** LOST: eax, edx, xmm0, xmm1, xmm4, xmm5, xmm6, xmm7
			mov eax,edi
			movzx edx,di			; edx=gruen
			shr eax,16				; eax=blau
			shr edx,8
			movzx eax,al
			cvtsi2ss xmm4,edx		; // xmm4=gruen
			cvtsi2ss xmm5,eax		; // xmm5=rot
			cvtsi2ss xmm7,eax		; // xmm7=rot
			cvtsi2ss xmm6,edx		; // xmm6=gruen
			mulss  xmm5, [sflt2]	; rot * -0.168736
			mulss  xmm7, [sflt1]	; rot * 0.5
			movzx edx,dil			; edx=blau
			mulss  xmm4, [sflt3]	; gruen * 0.331264
			mulss  xmm6, [sflt4]	; gruen * 0.418688
			addss  xmm5, [sflt6]	; gruen +128
			addss  xmm7, [sflt6]	; gruen +128
			addss  xmm5,xmm4		; gruen+rot
			addss  xmm7,xmm6		; gruen+rot
			cvtsi2ss xmm4,edx		; // xmm4=blau
			cvtsi2ss xmm6,edx		; // xmm6=blau
			mulss  xmm4, [sflt1]	; blau * 0.5
			mulss  xmm6, [sflt5]	; blau * 0.081312
			addss  xmm5,xmm4		; dazuaddieren
			addss  xmm7,xmm6		; dazuaddieren
			; xmm5=Cr_p, xmm7=Cb_p
			; Differenz berechnen
			subss xmm0,xmm5
			subss xmm1,xmm7
			mulss xmm0,xmm0
			mulss xmm1,xmm1
			addss xmm0,xmm1
			sqrtss xmm1,xmm0		; Wurzel ziehen
			movd xmm7,[rsi+rcx*4]	; Background schonmal nach xmm7 laden
			;if (temp < tola) {return (0.0);}
			comiss xmm1,xmm2
			jc .useBackground
			;if (temp < tolb) {return ((temp-tola)/(tolb-tola));}
			comiss xmm1,xmm3
			jc .blendWithBackground
				mov [r13+rcx*4], edi
				dec ecx
				jnz .aXLoop
				jmp .endaXLoop
	ALIGN 16
		.useBackground:
			movd [r13+rcx*4], xmm7
			dec ecx
			jnz .aXLoop
			jmp near .endaXLoop
	ALIGN 16
		.blendWithBackground:
			movss xmm4,xmm3		; tolb nach xmm4
			subss xmm1,xmm2		; temp-tola
			subss xmm4,xmm2		; tolb-tola
			movd xmm5,edi		; Vordergrund nach xmm5
			divss xmm1,xmm4		; Alphawert ist nun als float in xmm1
			pxor xmm6,xmm6
			mulss xmm1,[sflt8]	; und jetzt als float 0..255
			movq xmm0,[akehr]
			punpcklbw xmm7,xmm6	; Background in xmm7 in Worte umgewandelt
			cvtps2dq xmm4,xmm1	; und jetzt als integer 0..255 in xmm4
			punpcklbw xmm5,xmm6 ; Vordergrund in xmm5 in Worte umgewandelt
			pshuflw xmm4,xmm4,0	; nun haben wir den Alpha-Wert 4 mal als Word in xmm4
			psubw xmm0,xmm4		; Umkehrwert für Alpha berechnen (255-alpha)
			pmullw xmm5, xmm4
			pmullw xmm7, xmm0
			psrlw xmm5,8
			psrlw xmm7,8
			paddusw xmm7,xmm5
			packuswb xmm7,xmm6
			movd [r13+rcx*4], xmm7

			dec ecx
			jnz .aXLoop
	.endaXLoop:
		add rsi,r11
		add r13,r12
		add r8,r10
		dec ebx
		jnz .aYLoop

	pop r13
	xor rax,rax
	pop r12
	inc al
	pop rbx							; rbx, r10 und r11 retten
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret
%else								; 32-Bit-Version
	global ASM_BltChromaKey32_1
	global _ASM_BltChromaKey32_1

	ASM_BltChromaKey32_1:
	_ASM_BltChromaKey32_1:
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp

		xor eax,eax					; Returnwert auf 0 setzen
		pop ebp
		ret
%endif


;*******************************************************************************************************
;* Die nachfolgende Funktion ist optimiert für Breiten und Adressen, die durch 4 teilbar sind.
;* Es werden jeweils 4 Pixel gleichzeitig berechnet
;*******************************************************************************************************


ALIGN 16
s4flt1:	dd	0.5
		dd	0.5
		dd	0.5
		dd	0.5
s4flt2:	dd	-0.168736
		dd	-0.168736
		dd	-0.168736
		dd	-0.168736
s4flt3:	dd	-0.331264
		dd	-0.331264
		dd	-0.331264
		dd	-0.331264
s4flt4:	dd	-0.418688
		dd	-0.418688
		dd	-0.418688
		dd	-0.418688
s4flt5:	dd	-0.081312
		dd	-0.081312
		dd	-0.081312
		dd	-0.081312
s4flt6:	dd	128.0
		dd	128.0
		dd	128.0
		dd	128.0
s4flt7:	dd	1.0
		dd	1.0
		dd	1.0
		dd	1.0
s4flt8:	dd	255.0
		dd	255.0
		dd	255.0
		dd	255.0
s4flt9:	dd	0.0
		dd	0.0
		dd	0.0
		dd	0.0

ALIGN 16
andmask:	dd 0x000000ff
			dd 0x000000ff
			dd 0x000000ff
			dd 0x000000ff

ALIGN 16
ASM_BltChromaKey32_4:
%if elf64=1 || win64=1
	; IN: r8 => Pointer auf Datenstruktur
	%if win64=1
		push rdi							; rdi und rsi müssen unter WIN64 gerettet werden
		push rsi
	%endif
	push rbx
	push r12
	push r13
	mov rsi,[r8+bgadr]			;// rsi: Pointer auf Background
	mov rdi,[r8+tgadr]			;// rsi: Pointer auf Ziel
	mov r10d,[r8+spitch]		;// r10: Pitch von Quelle
	mov r11d,[r8+bgpitch]		;// r11: Pitch vom Background
	mov r12d,[r8+tgpitch]		;// r11: Pitch vom Ziel
	mov r9d,[r8+width]			;// r9:  Breite
	mov ebx,[r8+height]			;// ebx: Hoehe
	shl r9d,2					; Breite * 4
	cvtsi2ss xmm8,[r8+cb_key]	; // xmm8=Cb_key
	cvtsi2ss xmm9,[r8+cr_key]	; // xmm8=Cr_key
	cvtsi2ss xmm2,[r8+tola]		; // xmm2=tola
	cvtsi2ss xmm3,[r8+tolb]		; // xmm2=tolb
	pshufd xmm8,xmm8,0			; // Cb_key ist nun viermal in xmm8 vorhanden
	pshufd xmm9,xmm9,0			; // Cr_key ist nun viermal in xmm9 vorhanden

	mov r8,[r8+sadr]			;// r8: Pointer auf Quelle/Ziel

	;movss xmm8, [sflt7]	; 1
	;pxor xmm9,xmm9

	jmp near .aYLoop
	ALIGN 16
	.aYLoop:
		mov ecx,r9d					; Breite nach ecx laden
		xor eax,eax
		jmp near .aXLoop
		ALIGN 16
		.aXLoop:
			prefetchw [r8+rcx-256]

			movdqa xmm10,[r8+rcx-16]	; 4 Farbwerte nach xmm10
			movdqa xmm11,[andmask];
			movdqa xmm4, xmm10			; xmm4: blau
			movdqa xmm5, xmm10			; xmm5: gruen
			movdqa xmm6, xmm10			; xmm6: rot
			pand xmm4,xmm11
			psrld xmm5,8
			psrld xmm6,16
			pand xmm5,xmm11
			pand xmm6,xmm11
			cvtdq2ps xmm4,xmm4			; blau => float
			cvtdq2ps xmm5,xmm5			; gruen => float
			cvtdq2ps xmm6,xmm6			; rot => float
			; Da wir Cb und Cr gleichzeitig berechnen wollen, kopieren wir die Werte nochmal
			movdqa xmm12,xmm4			; xmm12: blau
			movdqa xmm13,xmm5			; xmm13: gruen
			movdqa xmm14,xmm6			; xmm14: rot

			mulps xmm4,[s4flt1]			; blau * 0.5
			mulps xmm12,[s4flt5]		; blau * 0.081312
			mulps xmm5,[s4flt3]			; gruen * 0.331264
			mulps xmm13,[s4flt4]		; gruen * 0.418688
			mulps xmm6,[s4flt2]			; rot * -0.168736
			mulps xmm14,[s4flt1]		; rot * 0.5
			addps xmm4, [s4flt6]		; Cb = blau + 128
			addps xmm12, [s4flt6]		; Cr = blau + 128
			addps xmm4,xmm5				; Cb = blau + gruen
			addps xmm12,xmm13			; Cr = blau + gruen
			addps xmm4,xmm6				; Cb = Cb + rot
			addps xmm12,xmm14			; Cr = Cb + rot

			; Ergebnis: xmm4 = Cr_p, xmm12 = Cb_p
			; Differenz berechnen
			movdqa xmm0,xmm8
			movdqa xmm1,xmm9
			subps xmm0,xmm4
			subps xmm1,xmm12
			mulps xmm0,xmm0
			mulps xmm1,xmm1
			addps xmm0,xmm1
			movdqa xmm7,[rsi+rcx-16]	; Background schonmal nach xmm7 laden
			sqrtps xmm1,xmm0			; Wurzel ziehen

			; Nun müssen wir jeden der 4 Pixel einzeln durchgehen
			; Pixel 1
			movdqa xmm5,xmm10
			movdqa xmm4,xmm7
			movdqa xmm0,xmm1
			shufps xmm4,xmm4,3
			shufps xmm5,xmm5,3
			shufps xmm0,xmm0,3
			sub ecx,4
			call HandlePixel

			; Pixel 2
			movdqa xmm5,xmm10
			movdqa xmm4,xmm7
			movdqa xmm0,xmm1
			shufps xmm4,xmm4,2
			shufps xmm5,xmm5,2
			shufps xmm0,xmm0,2
			sub ecx,4
			call HandlePixel

			; Pixel 3
			movdqa xmm5,xmm10
			movdqa xmm4,xmm7
			movdqa xmm0,xmm1
			shufps xmm4,xmm4,1
			shufps xmm5,xmm5,1
			shufps xmm0,xmm0,1
			sub ecx,4
			call HandlePixel

			; Pixel 4
			movdqa xmm5,xmm10
			movdqa xmm4,xmm7
			movdqa xmm0,xmm1
			sub ecx,4
			call HandlePixel
			cmp ecx,0
			jnz .aXLoop
	.endaXLoop:
		add rsi,r11
		add rdi,r12
		add r8,r10
		dec ebx
		jnz .aYLoop

	pop r13
	xor eax,eax
	pop r12
	inc al							; Returnwert auf 1 setzen
	pop rbx							; rbx, r10 und r11 retten
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret

ALIGN 16
	HandlePixel:	; IN: xmm0=Differenz, xmm4=Background, xmm5=Foreground
		comiss xmm0,xmm2
		jc .useBackground
		comiss xmm0,xmm3
		jc .blendWithBackground
		movd [rdi+rcx], xmm5
		ret
ALIGN 16
	.useBackground:
		movd [rdi+rcx], xmm4
		ret
ALIGN 16
	.blendWithBackground:	; IN: xmm0=Differenz, xmm4=Background, xmm5=Foreground
		movss xmm13,xmm3	; tolb nach xmm13
		subss xmm0,xmm2		; temp-tola
		subss xmm13,xmm2		; tolb-tola
		divss xmm0,xmm13	; Alphawert ist nun als float in xmm0
		pxor xmm6,xmm6
		mulss xmm0,[s4flt8]	; und jetzt als float 0..255
		movq xmm12,[akehr]
		punpcklbw xmm4,xmm6	; Background in xmm4 in Worte umgewandelt
		cvtps2dq xmm0,xmm0	; und jetzt als integer 0..255 in xmm0
		punpcklbw xmm5,xmm6 ; Vordergrund in xmm5 in Worte umgewandelt
		pshuflw xmm0,xmm0,0	; nun haben wir den Alpha-Wert 4 mal als Word in xmm0
		psubw xmm12,xmm0		; Umkehrwert für Alpha berechnen (255-alpha)
		pmullw xmm5, xmm0
		pmullw xmm4, xmm12
		psrlw xmm5,8
		psrlw xmm4,8
		paddusw xmm4,xmm5
		packuswb xmm4,xmm6
		movd [rdi+rcx], xmm4
		ret

%else								; 32-Bit-Version
	global ASM_BltChromaKey32_4
	global _ASM_BltChromaKey32_4

	ASM_BltChromaKey32:
	_ASM_BltChromaKey32:
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp

		xor eax,eax					; Returnwert auf 0 setzen
		pop ebp
		ret
%endif

