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


;/*********************************************************************
;/** fonts.asm                                                       **
;/*********************************************************************

%include "src/asm/common.asm"

SECTION .data
SECTION .text

struc GLYPH
	data:		PTR		1			; ebp+0
	target:		PTR		1			; ebp+4
	pitch:		resd	1			; ebp+8
	color:		resd	1			; ebp+12
endstruc

;/*********************************************************************
;/** BltGlyph_M8_32                                                  **
;/**                                                                 **
;/** void BltGlyph_M8_32(GLYPH *glyph)                               **
;/*********************************************************************

%if elf64=1 || win64=1
	%if elf64=1
		global BltGlyph_M8_32
		BltGlyph_M8_32:
			mov r8,rdi		; Pointer auf Glyph-Data nach r8
	%elif win64=1
		global _BltGlyph_M8_32
		_BltGlyph_M8_32:
			push rdi
			push rsi
			mov r8,rcx		; Pointer auf Glyph-Data nach r8
	%endif
	push rbx

	mov rsi,[r8+data]		; Source nach esi
	mov rdi,[r8+target]		; Traget nach edi
	movzx ecx, word [rsi]	; Breite erst nach ecx;
	movzx ebx, word [rsi+2]	; Höhe erst nach ebx;
	test ecx,0xffff
	mov eax,[r8+color]		; Color nach eax
	jz .end
	add esi,10
	mov r9d,[r8+pitch]		; Pitch nach r9d
	mov r8d,ecx				; Breite nach r8

	.loopHoehe:
		xor rdx,rdx
		.loopBreite:
			test byte [rsi],1
			jz .skip
				mov [rdi+rdx*4],eax
			.skip:
			inc rsi
			inc rdx
			dec rcx
		jnz .loopBreite
		add rdi,r9
		mov rcx,r8
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	pop rbx
	xor rax,rax
	%if win64=1
		pop rsi
		pop rdi
	%endif
	inc rax
	ret


%else						; 32-Bit-Version
global BltGlyph_M8_32
global _BltGlyph_M8_32

BltGlyph_M8_32:
_BltGlyph_M8_32:
	push ebp
	mov ebp,[esp+8]			; Pointer auf data nach ebp
	pushad
	mov esi,[ebp+data]		; Source nach esi
	mov edi,[ebp+target]	; Traget nach edi
	movzx ecx, word [esi]		; Breite erst nach ecx;
	movzx ebx, word [esi+2]	; Höhe erst nach ebx;
	test ecx,0xffff
	mov eax,[ebp+color]		; Color nach eax
	jz .end
	add esi,10
	movd mm0,ecx			; Breite nach mm0

	.loopHoehe:
		xor edx,edx
		.loopBreite:
			test byte [esi],1
			jz .skip
				mov [edi+edx*4],eax
			.skip:
			inc esi
			inc edx
			dec ecx
		jnz .loopBreite
		add edi,[ebp+pitch]
		movd ecx,mm0
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	popad
	xor eax,eax
	pop ebp
	inc eax
	ret
%endif

;/*********************************************************************
;/** BltGlyph_M1_32                                                  **
;/**                                                                 **
;/** void BltGlyph_M1_32(GLYPH *glyph)                               **
;/*********************************************************************

%if elf64=1 || win64=1
	%if elf64=1
		global BltGlyph_M1_32
		BltGlyph_M1_32:
			mov r8,rdi		; Pointer auf Glyph-Data nach r8
	%elif win64=1
		global _BltGlyph_M1_32
		_BltGlyph_M1_32:
			mov r8,rcx		; Pointer auf Glyph-Data nach r8
			push rdi
			push rsi
	%endif
	push rbx
	mov rsi,[r8+data]		; Source nach esi
	mov rdi,[r8+target]		; Traget nach edi
	movzx ecx, word [rsi]	; Breite erst nach ecx;
	movzx ebx, word [rsi+2]	; Höhe erst nach ebx;
	mov eax,[r8+color]		; Color nach eax
	mov r9d,[r8+pitch]		; Pitch nach r9d
	test ecx,0xffff
	movd mm1,eax			; Color nach mm1
	jz .end
	add rsi,10
	movd mm0,ecx			; Breite nach mm0
	mov ah, byte [rsi]		; Erstes Byte nach ah
	mov al,8				; Anzahl Restbits in al=8
	inc rsi
	.loopHoehe:
		xor rdx,rdx
		.loopBreite:
			test ah,128
			jz .skip
				movd [rdi+rdx*4],mm1
			.skip:
			shl ah,1
			dec al
			jnz .genugBits
				mov ah, byte [rsi]		; nächstes Byte nach ah
				mov al,8
				inc rsi
			.genugBits:
			inc edx
			dec ecx
		jnz .loopBreite
		add rdi,r9
		movd ecx,mm0
		dec ebx
	jnz .loopHoehe

	.end:
	emms
	pop rbx
	xor eax,eax
	%if win64=1
		pop rsi
		pop rdi
	%endif
	inc eax
	ret


%else						; 32-Bit-Version
	global BltGlyph_M1_32
	global _BltGlyph_M1_32
BltGlyph_M1_32:
_BltGlyph_M1_32:
	push ebp
	mov ebp,[esp+8]			; Pointer auf data nach ebp
	pushad
	mov esi,[ebp+data]		; Source nach esi
	mov edi,[ebp+target]	; Traget nach edi
	movzx ecx, word [esi]	; Breite erst nach ecx;
	movzx ebx, word [esi+2]	; Höhe erst nach ebx;
	mov eax,[ebp+color]		; Color nach eax
	test ecx,0xffff
	movd mm1,eax			; Color nach mm1
	jz .end
	add esi,10
	movd mm0,ecx			; Breite nach mm0
	mov ah, byte [esi]		; Erstes Byte nach ah
	mov al,8				; Anzahl Restbits in al=8
	inc esi
	.loopHoehe:
		xor edx,edx
		.loopBreite:
			test ah,128
			jz .skip
				movd [edi+edx*4],mm1
			.skip:
			shl ah,1
			dec al
			jnz .genugBits
				mov ah, byte [esi]		; nächstes Byte nach ah
				mov al,8
				inc esi
			.genugBits:
			inc edx
			dec ecx
		jnz .loopBreite
		add edi,[ebp+pitch]
		movd ecx,mm0
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	popad
	xor eax,eax
	pop ebp
	inc eax
	ret
%endif
;/*********************************************************************
;/** BltGlyph_AA8_32                                                 **
;/**                                                                 **
;/** void BltGlyph_AA8_32(GLYPH *glyph)                              **
;/*********************************************************************
%if elf64=1 || win64=1
	%if elf64=1
		global BltGlyph_AA8_32
		BltGlyph_AA8_32:
			mov r8,rdi			; Pointer auf Glyph-Data nach r8
	%elif win64=1
		global _BltGlyph_AA8_32
		_BltGlyph_AA8_32:
			mov r8,rcx			; Pointer auf Glyph-Data nach r8
			push rdi
			push rsi
	%endif
	push rbx
	mov rsi,[r8+data]			; Source nach esi
	pxor mm6,mm6				; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
	mov rdi,[r8+target]			; Traget nach rdi
	movzx ecx, word [rsi]		; Breite erst nach ecx;
	movzx ebx, word [rsi+2]		; Höhe erst nach ebx;
	mov r10,rcx					; Breite nach r10 kopieren
	test ecx,0xffff
	movd mm4,[r8+color]			; Color nach mm4
	mov r9d,[r8+pitch]			; Pitch nach r9
	jz .end
		mov eax,0xff
		add rsi,10
		movd mm7,eax
		pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
								; zum Maskieren
		mov eax, 0xff000000
		movd mm5,eax			; Alphachannel 0xff000000 nach mm5
		;por mm4, mm5			; Alpha-Channel auf ff
		.loopHoehe:
			xor rdx,rdx
			.loopBreite:
				movzx eax,byte [rsi]
				test eax,255					; Muss der Pixel gezeichnet werden?
				jz .skip						; Nein
					movd mm0,eax				; Alphachannel nach mm0
					movq mm3,mm4				; Fontfarbe nach mm3
												; Blenden müssen wir nur, wenn der Wert < 255 ist
					inc al						; Ist dies der Fall, wird al durch diesen INC Zero
					jz .noblend					; und das Zero-Flag gesetzt...
						pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
						punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
						movd mm2,[rdi+rdx*4]	; Hintergrundfarbe holen
						movq mm1,mm7			; 0x00ff00ff00ff00ff nach mm1
						punpcklbw mm2, mm6		; ergibt in mm2: 0a0r0g0b
						psubusw mm1,mm0			; Differenz bilden: 0xff-Alpha
						; Wir haben nun:
						;   mm0: Multiplikator für Fontfarbe
						;   mm1: Multiplikator für Hintergrundfarbe
						;   mm2: Farbe hintergrund
						;   mm3: Fontfarbe
						pmullw mm3,mm0			; Fontfarbe mit Multiplikator multiplizieren
						pmullw mm2,mm1			; Hintergrundfarbe mit Multiplikator multiplizieren
						psrlq mm3,8				; Das Ergebnis müssen wir durch 255 teilen...
						psrlq mm2,8				; dito
						pand mm3,mm7			; Die oberen Bytes ausmaskieren
						pand mm2,mm7			; dito
						paddusw mm3,mm2			; Beide Farben addieren
						packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
						por mm3, mm5			; Alpha-Channel auf ff
					.noblend:
					movd [rdi+rdx*4],mm3		; Farbe schreiben
				.skip:
				inc rsi
				inc rdx
				dec ecx
			jnz .loopBreite
			add rdi,r9						; Pitch zum Target addieren
			mov rcx,r10						; Breite wieder nach ecx
			dec ebx							; Höhe -1
		jnz .loopHoehe
	.end:
	emms
	xor rax,rax
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	inc rax
	ret


%else						; 32-Bit-Version
global BltGlyph_AA8_32
global _BltGlyph_AA8_32

BltGlyph_AA8_32:
_BltGlyph_AA8_32:
	push ebp
	mov ebp,[esp+8]			; Pointer auf data nach ebp
	pushad
	mov esi,[ebp+data]		; Source nach esi
	pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
	mov edi,[ebp+target]	; Traget nach edi
	movzx ecx, word [esi]		; Breite erst nach ecx;
	movzx ebx, word [esi+2]	; Höhe erst nach ebx;
	test ecx,0xffff
	movd mm4,[ebp+color]	; Color nach mm4
	jz .end
	add esi,10
	mov eax,0xff
	movd mm7,eax
	pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
							; zum Maskieren
	mov eax, 0xff000000
	movd mm5,eax			; Alphachannel 0xff000000 nach mm5
	;por mm4, mm5			; Alpha-Channel auf ff

	.loopHoehe:
		xor edx,edx
		push ecx			; Breite auf den Stack
		.loopBreite:
			movzx eax,byte [esi]
			test eax,255					; Muss der Pixel gezeichnet werden?
			jz .skip						; Nein
				movd mm0,eax				; Alphachannel nach mm0
				movq mm3,mm4				; Fontfarbe nach mm3
											; Blenden müssen wir nur, wenn der Wert < 255 ist
				inc al						; Ist dies der Fall, wird al durch diesen INC Zero
				jz .noblend					; und das Zero-Flag gesetzt...
					pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
					punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
					movd mm2,[edi+edx*4]	; Hintergrundfarbe holen
					movq mm1,mm7			; 0x00ff00ff00ff00ff nach mm1
					punpcklbw mm2, mm6		; ergibt in mm2: 0a0r0g0b
					psubusw mm1,mm0			; Differenz bilden: 0xff-Alpha
					; Wir haben nun:
					;   mm0: Multiplikator für Fontfarbe
					;   mm1: Multiplikator für Hintergrundfarbe
					;   mm2: Farbe hintergrund
					;   mm3: Fontfarbe
					pmullw mm3,mm0			; Fontfarbe mit Multiplikator multiplizieren
					pmullw mm2,mm1			; Hintergrundfarbe mit Multiplikator multiplizieren
					psrlq mm3,8				; Das Ergebnis müssen wir durch 255 teilen...
					psrlq mm2,8				; dito
					pand mm3,mm7			; Die oberen Bytes ausmaskieren
					pand mm2,mm7			; dito
					paddusw mm3,mm2			; Beide Farben addieren
					packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
					por mm3, mm5			; Alpha-Channel auf ff
				.noblend:
				movd [edi+edx*4],mm3		; Farbe schreiben
			.skip:
			inc esi
			inc edx
			dec ecx
		jnz .loopBreite
		add edi,[ebp+pitch]
		pop ecx							; Breite vom Stack nach ecx
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	popad
	xor eax,eax
	pop ebp
	inc eax
	ret
%endif

;/*********************************************************************
;/** BltGlyph_AA2_32                                                 **
;/**                                                                 **
;/** void BltGlyph_AA2_32(GLYPH *glyph)                              **
;/*********************************************************************

BlendAA2_128:		dd	200
BlendAA2_64:		dd	100

%if elf64=1 || win64=1
	%if elf64=1
		global BltGlyph_AA2_32
		BltGlyph_AA2_32:
			mov r8,rdi		; Pointer auf Glyph-Data nach r8
	%elif win64=1
		global _BltGlyph_AA2_32
		_BltGlyph_AA2_32:
			push rdi
			push rsi
			mov r8,rcx		; Pointer auf Glyph-Data nach r8
	%endif
	push rbx
	push r10
	mov rsi,[r8+data]		; Source nach esi
	pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
	mov rdi,[r8+target]		; Traget nach edi
	movzx ecx, word [rsi]	; Breite erst nach ecx;
	movzx ebx, word [rsi+2]	; Höhe erst nach ebx;
	mov r10,rcx				; Breite nach r10 kopieren
	test ecx,0xffff
	movd mm4,[r8+color]		; Color nach mm4
	mov r9d, dword [r8+pitch]		; Pitch nach r9d
	jnz .draw
		jmp .end
	.draw:
	add rsi,10
	mov eax,0xff
	movd mm7,eax
	pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
							; zum Maskieren
	mov eax, 0xff000000
	movd mm5,eax			; Alphachannel 0xff000000 nach mm5

	mov ah, byte [rsi]		; Erstes Byte nach ah
	mov al,4				; Anzahl Rest-Doppelbits in al=4
	inc rsi
	.loopHoehe:
		xor rdx,rdx
		.loopBreite:
			movq mm3,mm4					; Fontfarbe nach mm3
			test ah,192						; Muss der Pixel gezeichnet werden?
			jz .skip						; Nein
				test ah,128
				jz .blend64
				test ah,64
				jnz .noblend
					movd mm0,[BlendAA2_128]	; Alphachannel nach mm0
					jmp .blend
				.blend64:
					movd mm0,[BlendAA2_64]	; Alphachannel nach mm0

				.blend:
					punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
					movd mm2,[rdi+rdx*4]	; Hintergrundfarbe holen
					pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte


					movq mm1,mm7			; 0x00ff00ff00ff00ff nach mm1
					punpcklbw mm2, mm6		; ergibt in mm2: 0a0r0g0b
					psubusw mm1,mm0			; Differenz bilden: 0xff-Alpha
					; Wir haben nun:
					;   mm0: Multiplikator für Fontfarbe
					;   mm1: Multiplikator für Hintergrundfarbe
					;   mm2: Farbe hintergrund
					;   mm3: Fontfarbe
					pmullw mm3,mm0			; Fontfarbe mit Multiplikator multiplizieren
					pmullw mm2,mm1			; Hintergrundfarbe mit Multiplikator multiplizieren
					psrlq mm3,8				; Das Ergebnis müssen wir durch 255 teilen...
					psrlq mm2,8				; dito
					pand mm3,mm7			; Die oberen Bytes ausmaskieren
					pand mm2,mm7			; dito
					paddusw mm3,mm2			; Beide Farben addieren
					packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
					;por mm3, mm5			; Alpha-Channel auf ff
				.noblend:
				movd [rdi+rdx*4],mm3		; Farbe schreiben
			.skip:
			shl ah,2
			dec al
			jnz .genugBits
				mov ah, byte [rsi]		; nächstes Byte nach ah
				mov al,4
				inc rsi
			.genugBits:
			inc edx
			dec ecx
		jnz .loopBreite
		add rdi,r9							; Pitch addieren
		mov rcx,r10							; Breite wieder nach ecx kopieren
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	xor eax,eax
	pop r10
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	inc eax
	ret

%else						; 32-Bit-Version

global BltGlyph_AA2_32
global _BltGlyph_AA2_32

BltGlyph_AA2_32:
_BltGlyph_AA2_32:
	push ebp
	mov ebp,[esp+8]			; Pointer auf data nach ebp
	pushad
	mov esi,[ebp+data]		; Source nach esi
	pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
	mov edi,[ebp+target]	; Traget nach edi
	movzx ecx, word [esi]	; Breite erst nach ecx;
	movzx ebx, word [esi+2]	; Höhe erst nach ebx;
	test ecx,0xffff
	movd mm4,[ebp+color]	; Color nach mm4
	jnz .draw
		jmp .end
	.draw:
	add esi,10
	mov eax,0xff
	movd mm7,eax
	pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
							; zum Maskieren
	mov eax, 0xff000000
	movd mm5,eax			; Alphachannel 0xff000000 nach mm5

	mov ah, byte [esi]		; Erstes Byte nach ah
	mov al,4				; Anzahl Rest-Doppelbits in al=4
	inc esi

	.loopHoehe:
		xor edx,edx
		push ecx			; Breite auf den Stack
		.loopBreite:
			movq mm3,mm4					; Fontfarbe nach mm3
			test ah,192						; Muss der Pixel gezeichnet werden?
			jz .skip						; Nein
				test ah,128
				jz .blend64
				test ah,64
				jnz .noblend
					movd mm0,[BlendAA2_128]	; Alphachannel nach mm0
					jmp .blend
				.blend64:
					movd mm0,[BlendAA2_64]	; Alphachannel nach mm0

				.blend:
					punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
					movd mm2,[edi+edx*4]	; Hintergrundfarbe holen
					pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte


					movq mm1,mm7			; 0x00ff00ff00ff00ff nach mm1
					punpcklbw mm2, mm6		; ergibt in mm2: 0a0r0g0b
					psubusw mm1,mm0			; Differenz bilden: 0xff-Alpha
					; Wir haben nun:
					;   mm0: Multiplikator für Fontfarbe
					;   mm1: Multiplikator für Hintergrundfarbe
					;   mm2: Farbe hintergrund
					;   mm3: Fontfarbe
					pmullw mm3,mm0			; Fontfarbe mit Multiplikator multiplizieren
					pmullw mm2,mm1			; Hintergrundfarbe mit Multiplikator multiplizieren
					psrlq mm3,8				; Das Ergebnis müssen wir durch 255 teilen...
					psrlq mm2,8				; dito
					pand mm3,mm7			; Die oberen Bytes ausmaskieren
					pand mm2,mm7			; dito
					paddusw mm3,mm2			; Beide Farben addieren
					packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
					;por mm3, mm5			; Alpha-Channel auf ff
				.noblend:
				movd [edi+edx*4],mm3		; Farbe schreiben
			.skip:
			shl ah,2
			dec al
			jnz .genugBits
				mov ah, byte [esi]		; nächstes Byte nach ah
				mov al,4
				inc esi
			.genugBits:
			inc edx
			dec ecx
		jnz .loopBreite
		add edi,[ebp+pitch]
		pop ecx							; Breite vom Stack nach ecx
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	popad
	xor eax,eax
	pop ebp
	inc eax
	ret
%endif



;/*********************************************************************
;/** BltGlyph_AA4_32                                                 **
;/**                                                                 **
;/** void BltGlyph_AA4_32(GLYPH *glyph)                              **
;/*********************************************************************

BlendAA4:	dd	0			; 0
			dd	1*255/15	; 1
			dd	2*255/15	; 2
			dd	3*255/15	; 3
			dd	4*255/15	; 4
			dd	5*255/15	; 5
			dd	6*255/15	; 6
			dd	7*255/15	; 7
			dd	8*255/15	; 8
			dd	9*255/15	; 9
			dd	10*255/15	; 10
			dd	11*255/15	; 11
			dd	12*255/15	; 12
			dd	13*255/15	; 13
			dd	14*255/15	; 14
			dd	15*255/15	; 15


%if elf64=1 || win64=1
	%if elf64=1
		global BltGlyph_AA4_32
		BltGlyph_AA4_32:
			mov r8,rdi		; Pointer auf Glyph-Data nach r8
	%elif win64=1
		global _BltGlyph_AA4_32
		_BltGlyph_AA4_32:
			push rdi
			push rsi
			mov r8,rcx		; Pointer auf Glyph-Data nach r8
	%endif
	push rbx
	push r10
	mov rsi,[r8+data]		; Source nach esi
	pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
	mov rdi,[r8+target]		; Traget nach edi
	movzx ecx, word [rsi]	; Breite erst nach ecx;
	movzx ebx, word [rsi+2]	; Höhe erst nach ebx;
	mov r10,rcx				; Breite nach r10 kopieren
	test ecx,0xffff
	movd mm4,[r8+color]		; Color nach mm4
	mov r9d,[r8+pitch]		; Pitch nach r9
	jnz .draw
		jmp .end
	.draw:
	add rsi,10
	mov eax,0xff
	movd mm7,eax
	pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
							; zum Maskieren
	mov eax, 0xff000000
	movd mm5,eax			; Alphachannel 0xff000000 nach mm5
	xor rax,rax
	mov ah, byte [rsi]		; Erstes Byte nach ah
	mov al,2				; Anzahl Rest-Doppelbits in al=4
	inc rsi
	.loopHoehe:
		xor rdx,rdx
		.loopBreite:
			movq mm3,mm4					; Fontfarbe nach mm3
			test ah,240						; Muss der Pixel gezeichnet werden?
			jz .skip						; Nein
				movd mm2,eax
				shr eax,12
				cmp al,15
				jb .blend
				movd eax,mm2
				jmp near .noblend
				.blend:
					movd mm0,[BlendAA4+rax*4]
					movd eax,mm2

					punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
					movd mm2,[rdi+rdx*4]	; Hintergrundfarbe holen
					pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte


					movq mm1,mm7			; 0x00ff00ff00ff00ff nach mm1
					punpcklbw mm2, mm6		; ergibt in mm2: 0a0r0g0b
					psubusw mm1,mm0			; Differenz bilden: 0xff-Alpha
					; Wir haben nun:
					;   mm0: Multiplikator für Fontfarbe
					;   mm1: Multiplikator für Hintergrundfarbe
					;   mm2: Farbe hintergrund
					;   mm3: Fontfarbe
					pmullw mm3,mm0			; Fontfarbe mit Multiplikator multiplizieren
					pmullw mm2,mm1			; Hintergrundfarbe mit Multiplikator multiplizieren
					psrlq mm3,8				; Das Ergebnis müssen wir durch 255 teilen...
					psrlq mm2,8				; dito
					pand mm3,mm7			; Die oberen Bytes ausmaskieren
					pand mm2,mm7			; dito
					paddusw mm3,mm2			; Beide Farben addieren
					packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
					;por mm3, mm5			; Alpha-Channel auf ff
				.noblend:
				movd [rdi+rdx*4],mm3		; Farbe schreiben
			.skip:
			shl ah,4
			dec al
			jnz .genugBits
				mov ah, byte [rsi]		; nächstes Byte nach ah
				mov al,2
				inc rsi
			.genugBits:
			inc edx
			dec ecx
		jnz .loopBreite
		add rdi,r9
		mov rcx,r10							; Breite wieder nach ecx kopieren
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	xor eax,eax
	pop r10
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	inc eax
	ret

%else						; 32-Bit-Version

global BltGlyph_AA4_32
global _BltGlyph_AA4_32

BltGlyph_AA4_32:
_BltGlyph_AA4_32:
	push ebp
	mov ebp,[esp+8]			; Pointer auf data nach ebp
	pushad
	mov esi,[ebp+data]		; Source nach esi
	pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
	mov edi,[ebp+target]	; Traget nach edi
	movzx ecx, word [esi]	; Breite erst nach ecx;
	movzx ebx, word [esi+2]	; Höhe erst nach ebx;
	test ecx,0xffff
	movd mm4,[ebp+color]	; Color nach mm4
	jnz .draw
		jmp .end
	.draw:
	add esi,10
	mov eax,0xff
	movd mm7,eax
	pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
							; zum Maskieren
	mov eax, 0xff000000
	movd mm5,eax			; Alphachannel 0xff000000 nach mm5
	xor eax,eax

	mov ah, byte [esi]		; Erstes Byte nach ah
	mov al,2				; Anzahl Rest-Doppelbits in al=4
	inc esi

	.loopHoehe:
		xor edx,edx
		push ecx			; Breite auf den Stack
		.loopBreite:
			movq mm3,mm4					; Fontfarbe nach mm3
			test ah,240						; Muss der Pixel gezeichnet werden?
			jz .skip						; Nein
				movd mm2,eax
				shr eax,12
				cmp al,15
				jb .blend
				movd eax,mm2
				jmp near .noblend
				.blend:
					movd mm0,[BlendAA4+eax*4]
					movd eax,mm2

					punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
					movd mm2,[edi+edx*4]	; Hintergrundfarbe holen
					pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte


					movq mm1,mm7			; 0x00ff00ff00ff00ff nach mm1
					punpcklbw mm2, mm6		; ergibt in mm2: 0a0r0g0b
					psubusw mm1,mm0			; Differenz bilden: 0xff-Alpha
					; Wir haben nun:
					;   mm0: Multiplikator für Fontfarbe
					;   mm1: Multiplikator für Hintergrundfarbe
					;   mm2: Farbe hintergrund
					;   mm3: Fontfarbe
					pmullw mm3,mm0			; Fontfarbe mit Multiplikator multiplizieren
					pmullw mm2,mm1			; Hintergrundfarbe mit Multiplikator multiplizieren
					psrlq mm3,8				; Das Ergebnis müssen wir durch 255 teilen...
					psrlq mm2,8				; dito
					pand mm3,mm7			; Die oberen Bytes ausmaskieren
					pand mm2,mm7			; dito
					paddusw mm3,mm2			; Beide Farben addieren
					packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
					;por mm3, mm5			; Alpha-Channel auf ff
				.noblend:
				movd [edi+edx*4],mm3		; Farbe schreiben
			.skip:
			shl ah,4
			dec al
			jnz .genugBits
				mov ah, byte [esi]		; nächstes Byte nach ah
				mov al,2
				inc esi
			.genugBits:
			inc edx
			dec ecx
		jnz .loopBreite
		add edi,[ebp+pitch]
		pop ecx							; Breite vom Stack nach ecx
		dec ebx
	jnz .loopHoehe
	.end:
	emms
	popad
	xor eax,eax
	pop ebp
	inc eax
	ret
%endif
