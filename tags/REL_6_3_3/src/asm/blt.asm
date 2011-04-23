;/*******************************************************************************
;/* This file is part of "Patrick's Programming Library", Version 6 (PPL6).
; * Web: http://www.pfp.de/ppl/
; *
; * $Author: patrick $
; * $Revision: 1.22 $
; * $Date: 2009/02/06 21:55:47 $
; * $Id: blt.asm,v 1.22 2009/02/06 21:55:47 patrick Exp $
; *
;/*******************************************************************************
; * Copyright (c) 2008, Patrick Fedick <patrick@pfp.de>
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

%define elf64 0
%define win64 0
%define elf32 0
%define win32 0

%ifidn __OUTPUT_FORMAT__, elf64
%ifidn __BITS__, 64
%define elf64 1
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
%endif
%endif

%ifidn __OUTPUT_FORMAT__, win32
%ifidn __BITS__, 32
%define win32 1
%endif
%endif


SECTION .data
SECTION .text

struc BLTDATA
%ifidn __BITS__, 32
	src:		resd	1			; ebp+0
	tgt:		resd	1			; ebp+4
%elifidn __BITS__,  64
	src:		resq	1			; ebp+0
	tgt:		resq	1			; ebp+4
%endif
	width:		resd	1			; ebp+8
	height:		resd	1			; ebp+12
	pitchsrc:	resd	1			; ebp+16
	pitchtgt:	resd	1			; ebp+20
	color:		resd	1			; ebp+24
endstruc

;/*********************************************************************
;/** ASM_AlphaBlt32                                                  **
;/**                                                                 **
;/** void ASM_AlphaBlt32(BLTDATA *data)                              **
;/*********************************************************************
%if elf64=1 || win64=1
	%if elf64=1
		global ASM_AlphaBlt32
		ASM_AlphaBlt32:				; Pointer auf data ist in rdi
		mov r8,rdi					; Pointer nach r8 schieben
	%elif win64=1
		global _ASM_AlphaBlt32
		_ASM_AlphaBlt32:			; Pointer auf data ist zunächst in rcx
		mov r8,rcx					; Pointer nach r8 schieben
		push rdi					; rdi und rsi müssen unter WIN64 gerettet werden
		push rsi
	%endif
	push rbx					; rbx, r10 und r11 retten
	push r10
	push r11
	mov eax, 0xff000000
	pxor mm6,mm6
	movd mm3,eax				;// Fuer Or-Maske, damit Alpha-Channel immer ff ist
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
	jmp near .aYLoop
	ALIGN 16
	.aYLoop:
		mov ecx,r9d
		jmp near .aXLoop
		ALIGN 16
		.aXLoop:
			mov eax,[rsi+rcx*4]
			test eax,0xff000000
			jz .askip1
				movd mm0,eax				; src-Farbe nach mm0
				shr eax,24					; Alpha-Channel nach al
				inc al
				jz .noalpha
					movd mm2,[rdi+rcx*4]
					movd mm4,eax			; Um 1 erhöhten Alphawert nach mm4
					movq mm5,mm7
					punpcklbw mm0,mm6
					pshufw mm4,mm4,0
					punpcklbw mm2,mm6
					psubusw mm5,mm4
					pmullw mm0,mm4
					pmullw mm2,mm5
					psrlq mm0,8			;// Erst shiften und maskieren
					psrlq mm2,8			;// geht schneller als...
					pand mm0,mm7
					pand mm2,mm7
					paddusw mm0,mm2
					;//psrlq mm0,8		;// ...es nachträglich zu machen
					;//pand mm0,mm7

					packuswb mm0,mm6
					por mm0, mm3		;// Alpha-Channel auf ff
				.noalpha:
					movd [rdi+rcx*4],mm0
			.askip1:
			dec ecx
			jnz .aXLoop
		add rsi,r10
		add rdi,r11
		dec edx
		jnz .aYLoop
	emms
	xor rax,rax
	pop r11
	pop r10
	inc rax					; Returnwert auf 1 setzen
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret

%else								; 32-Bit-Version
	global ASM_AlphaBlt32
	global _ASM_AlphaBlt32

	ASM_AlphaBlt32:
	_ASM_AlphaBlt32:
		push ebp
		mov ebp,[esp+8]				; Pointer auf data nach ebp
		pushad
		mov eax, 0xff000000
		pxor mm6,mm6
		movd mm3,eax				;// Fuer Or-Maske, damit Alpha-Channel immer ff ist
		mov eax,0x00ff00ff
		mov edi,[ebp+tgt]
		movd mm7,eax
		mov esi,[ebp+src]

		sub edi,4					;// ?? Wofür soll das gut sein???
		sub esi,4					;// ?? Dito
		mov edx,[ebp+height]
		pshufw mm7,mm7, 0
		jmp near .aYLoop
		ALIGN 16
		.aYLoop:
			mov ecx, [ebp+width]
			jmp near .aXLoop
			ALIGN 16
			.aXLoop:
				mov eax,[esi+ecx*4]
				test eax,0xff000000
				jz .askip1
					movd mm0,eax				; src-Farbe nach mm0
					shr eax,24					; Alpha-Channel nach al
					inc al
					jz .noalpha
						movd mm2,[edi+ecx*4]
						movd mm4,eax			; Um 1 erhöhten Alphawert nach mm4
						movq mm5,mm7
						punpcklbw mm0,mm6
						pshufw mm4,mm4,0
						punpcklbw mm2,mm6
						psubusw mm5,mm4
						pmullw mm0,mm4
						pmullw mm2,mm5
						psrlq mm0,8			;// Erst shiften und maskieren
						psrlq mm2,8			;// geht schneller als...
						pand mm0,mm7
						pand mm2,mm7
						paddusw mm0,mm2
						;//psrlq mm0,8		;// ...es nachträglich zu machen
						;//pand mm0,mm7

						packuswb mm0,mm6
						por mm0, mm3		;// Alpha-Channel auf ff
					.noalpha:
						movd [edi+ecx*4],mm0
				.askip1:
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
;/** ASM_Blt32                                                       **
;/**                                                                 **
;/** void ASM_Blt32(BLTDATA *data)                                   **
;/*********************************************************************
%if elf64=1 || win64=1
	%if elf64=1
		global ASM_Blt32
		ASM_Blt32:					; Pointer auf data ist in rdi
		mov rcx,rdi					; Pointer nach rcx schieben
	%elif win64=1
		global _ASM_Blt32
		_ASM_Blt32:					; Pointer auf data ist in ecx
		push rdi
		push rsi
	%endif
		;push rbx				; rbx wird nicht benutzt
		mov r8d,[rcx+width]		; Breite nach r8d
		mov eax,[rcx+pitchsrc]	; Pitch der Quelle nach eax
		mov r9d,r8d				; Breite nach r9 kopieren
		mov edx,[rcx+pitchtgt]	; Pitch des Ziels nach edx
		shl r9d,2
		mov rdi,[rcx+tgt]		; Zieladresse nach rdi
		sub eax,r9d				; Breite*4 muss von SourcePitch abgezogen werden
		sub edx,r9d				; Breite*4 muss von TargetPitch abgezogen werden
		mov rsi,[rcx+src]		; Quelladresse nach rsi
		mov r9d,[rcx+height]	; Höhe nach r9d
		.loop:
			mov ecx,r8d			; Breite aus r8d holen
			rep movsd
			add rdi,rdx
			add rsi,rax
			dec r9d
		jnz .loop
		;emms					; Wir haben keine MMX-Register verwendet
		;pop rbx
		xor rax,rax
	%if win64=1
		pop rsi
		pop rdi
	%endif
		inc rax					; Returnwert auf 1 setzen
		ret

%else			; Die 32-Bit Variante
	global ASM_Blt32
	global _ASM_Blt32

	ASM_Blt32:
	_ASM_Blt32:
		push ebp
		mov ebp,[esp+8]			; Pointer auf data nach ebp
		pushad
		movd mm1,[ebp+width]	; Breite nach mm1
		mov eax,[ebp+pitchsrc]	; Pitch der Quelle nach eax
		movd ecx,mm1			; Breite nach ecx kopieren
		mov edx,[ebp+pitchtgt]	; Pitch des Ziels nach edx
		shl ecx,2
		mov edi,[ebp+tgt]		; Zieladresse nach edi
		sub eax,ecx				; Breite*4 muss von SourcePitch abgezogen werden
		sub edx,ecx				; Breite*4 muss von TargetPitch abgezogen werden
		mov esi,[ebp+src]		; Quelladresse nach esi
		mov ebx,[ebp+height]	; Höhe nach ebx
		.loop:
			movd ecx,mm1		; width
			rep movsd
			add edi,edx
			add esi,eax
			dec ebx
			jnz .loop
		emms
		popad
		xor eax,eax
		pop ebp
		inc eax					; Returnwert auf 1 setzen
		ret
%endif


;/*********************************************************************
;/** ASM_BltColorKey32                                               **
;/**                                                                 **
;/** void ASM_BltColorKey32(BLTDATA *data)                           **
;/*********************************************************************
%if elf64=1 || win64=1
	%if elf64=1
		global ASM_BltColorKey32
		ASM_BltColorKey32:			; Pointer auf data ist in rdi
		mov rcx,rdi					; Pointer nach rcx schieben
	%elif win64=1
		global _ASM_BltColorKey32
		_ASM_BltColorKey32:			; Pointer auf data ist in ecx
		push rdi
		push rsi
	%endif
		push rbx
		push r10
		push r11
		mov r8d,[rcx+width]		; Breite nach r8d
		mov eax,4
		mov r10d,[rcx+pitchsrc]	; Pitch der Quelle nach r10d
		mov r11d,[rcx+pitchtgt]	; Pitch des Ziels nach r11d
		mov rdi,[rcx+tgt]		; Zieladresse nach rdi
		mov rsi,[rcx+src]		; Quelladresse nach rsi
		sub rdi,rax
		mov ebx,[rcx+color]		; Colorkey nach ebx
		sub rsi,rax
		mov r9d,[rcx+height]	; Höhe nach r9d
		.loop:
			mov ecx,r8d			; Breite aus r8d holen
			.loopx:
				mov eax,[rsi+rcx*4]
				cmp eax,ebx		; Colorkey?
				je .skip1
					mov[rdi+rcx*4],eax
				.skip1:
				dec ecx
				jnz .loopx

			add rdi,r11
			add rsi,r10
			dec r9d
		jnz .loop
		;emms
		xor rax,rax
		pop r11
		pop r10
		pop rbx
		inc rax			; Returnwert auf 1 setzen
	%if win64=1
		pop rsi
		pop rdi
	%endif
		ret

%else			; Die 32-Bit Variante

	global ASM_BltColorKey32
	global _ASM_BltColorKey32

	ASM_BltColorKey32:
	_ASM_BltColorKey32:
		push ebp
		mov ebp,[esp+8]			; Pointer auf data nach ebp
		pushad
		movd mm1,[ebp+width]	; Breite nach mm1
		mov eax,4
		movd mm3,[ebp+pitchsrc]	; Pitch der Quelle nach mm3
		movd mm4,[ebp+pitchtgt]	; Pitch des Ziels nach mm4
		mov edi,[ebp+tgt]		; Zieladresse nach rdi
		mov esi,[ebp+src]		; Quelladresse nach rsi
		sub edi,eax
		mov ebx,[ebp+color]		; Colorkey nach ebx
		sub esi,eax
		mov edx,[ebp+height]	; Höhe nach edx
		.loop:
			movd ecx,mm1		; Breite aus mm1 holen
			.loopx:
				mov eax,[esi+ecx*4]
				cmp eax,ebx		; Colorkey?
				je .skip1
					mov[edi+ecx*4],eax
				.skip1:
				dec ecx
				jnz .loopx
			movd eax,mm4
			movd ecx,mm3
			add edi,eax
			add esi,ecx
			dec edx
		jnz .loop
		emms
		popad
		xor eax,eax
		pop ebp
		inc eax
		ret
%endif


;/*********************************************************************
;/** ASM_BltDiffuse32                                                **
;/**                                                                 **
;/** void ASM_BltDiffuse32(BLTDATA *data)                            **
;/*********************************************************************
%if elf64=1 || win64=1
	%if elf64=1
		global ASM_BltDiffuse32
		ASM_BltDiffuse32:			; Pointer auf data ist in rdi
		mov r8,rdi					; Pointer nach r8 schieben
	%elif win64=1
		global _ASM_BltDiffuse32
		_ASM_BltDiffuse32:			; Pointer auf data ist in ecx
		mov r8,ecx
		push rdi
		push rsi
	%endif
		push rbx
		push r10
		push r11
		mov r9d,[r8+width]		; Breite nach r9
		mov r10d,[r8+pitchsrc]	; Pitch der Quelle nach r10d
		pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
		mov r11d,[r8+pitchtgt]	; Pitch des Ziels nach r11d
		mov rdi,[r8+tgt]		; Zieladresse nach rdi
		mov eax,4
		mov rsi,[r8+src]		; Quelladresse nach rsi
		movd mm4,[r8+color]		; Color nach mm4
		sub rdi,rax				; Zieladresse 1 Pixel nach links, da wir von 1 bis Breite arbeiten
		dec rsi					; Quelladresse dito
		test r9d,0xffff			; Haben wir überhaupt eine Breite?
		mov ebx,[r8+height]		; Höhe nach ebx
		jz .end					; Keine Breite, nix zu tun
			mov eax,0xff
			mov edx, 0xff000000
			movd mm7,eax
			movd mm5,edx		; Alphachannel 0xff000000 nach mm5
			pshufw mm7,mm7,0	; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
								; zum Maskieren
			por mm4, mm5		; Alpha-Channel der Farbe auf ff
			.loopHoehe:
				mov ecx,r9d		; Breite aus r9d holen
				ALIGN 16
				.loopBreite:
					movzx eax, byte [rsi+rcx]		; Pixel holen
					test al,255						; Muss der Pixel gezeichnet werden?
					jz .skip						; Nein
						movq mm3,mm4				; Farbe nach mm3
													; Blenden müssen wir nur, wenn der Wert < 255 ist
						inc al						; Ist dies der Fall, wird al durch diesen INC Zero
						jz .noblend					; und das Zero-Flag gesetzt...
							movd mm2,[rdi+rcx*4]	; Hintergrundfarbe holen
							movd mm0,eax			; Alphachannel nach mm0
							por mm2, mm5			; Alphawert des Hintergrunds auf 0xff
							punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
							punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
							pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
							psubsw mm3,mm2			; src-dst mit Vorzeichen
							pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
							psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
							paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
							pand mm3,mm7			; Die oberen Bytes ausmaskieren
							packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
							;por mm3, mm5			; Alpha-Channel auf ff
						.noblend:
						dec ecx
						movd [rdi+rcx*4+4],mm3		; Farbe schreiben
						jnz .loopBreite
						jz .nextrow
					.skip:
					dec ecx							; Zähler für Breite verringern
				jnz .loopBreite
				.nextrow:
				add rdi,r11				; Zielpitch addieren
				add rsi,r10				; Quellpitch addieren
				dec ebx					; Höhe verringern
			jnz .loopHoehe
		.end:
		emms
		xor rax,rax
		pop r11
		pop r10
		pop rbx
		inc rax			; Returnwert auf 1 setzen
	%if win64=1
		pop rsi
		pop rdi
	%endif
		ret

%else			; Die 32-Bit Variante

	global ASM_BltDiffuse32
	global _ASM_BltDiffuse32

	ASM_BltDiffuse32:
	_ASM_BltDiffuse32:
		push ebp
		xor eax,eax				; eax löschen wir schonmal als Vorbereitung für den Return-Wert
		mov ebp,[esp+8]			; Pointer auf data nach ebp
		pushad
		mov edx,[ebp+width]		; Breite nach edx
		pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
		mov edi,[ebp+tgt]		; Zieladresse nach edi
		mov esi,[ebp+src]		; Quelladresse nach esi
		movd mm4,[ebp+color]	; Color nach mm4
		sub edi,4				; Zieladresse 1 Pixel nach links, da wir von 1 bis Breite arbeiten
		dec esi					; Quelladresse dito
		test edx,0xffff			; Haben wir überhaupt eine Breite?
		mov ebx,[ebp+height]	; Höhe nach ebx
		jz .end					; Keine Breite, nix zu tun
			mov eax,0xff
			mov ecx, 0xff000000
			movd mm7,eax
			movd mm5,ecx		; Alphachannel 0xff000000 nach mm5
			pshufw mm7,mm7,0	; 0x00ff00ff00ff00ff in mm7 für Bildung der Differenz und
								; zum Maskieren
			por mm4, mm5		; Alpha-Channel der Farbe auf ff
			.loopHoehe:
				mov ecx,edx		; Breite aus edx holen
				.loopBreite:
					movzx eax, byte [esi+ecx]		; Pixel holen
					test al,255						; Muss der Pixel gezeichnet werden?
					jz .skip						; Nein
						movq mm3,mm4				; Farbe nach mm3
													; Blenden müssen wir nur, wenn der Wert < 255 ist
						inc al						; Ist dies der Fall, wird al durch diesen INC Zero
						jz .noblend					; und das Zero-Flag gesetzt...
							movd mm2,[edi+ecx*4]	; Hintergrundfarbe holen
							movd mm0,eax			; Alphachannel nach mm0
							por mm2, mm5			; Alphawert des Hintergrunds auf 0xff
							punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
							punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
							pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
							psubsw mm3,mm2			; src-dst mit Vorzeichen
							pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
							psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
							paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
							pand mm3,mm7			; Die oberen Bytes ausmaskieren
							packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
							;por mm3, mm5			; Alpha-Channel auf ff
						.noblend:
						movd [edi+ecx*4],mm3		; Farbe schreiben
					.skip:
					dec ecx							; Zähler für Breite verringern
				jnz .loopBreite
				add edi,[ebp+pitchtgt]	; Zielpitch addieren
				add esi,[ebp+pitchsrc]	; Quellpitch addieren
				dec ebx					; Höhe verringern
			jnz .loopHoehe
		.end:
		emms
		popad
		inc eax
		pop ebp
		ret
%endif
