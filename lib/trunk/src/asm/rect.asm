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
;/** rect.asm                                                         **
;/*********************************************************************

%include "src/asm/common.asm"


SECTION .data
SECTION .text



struc RECTDATA
	tgt:		PTR		1			; ebp+0
	width:		resd	1			; ebp+4
	height:		resd	1			; ebp+8
	pitch:		resd	1			; ebp+12
	color:		resd	1			; ebp+16
endstruc

;/*********************************************************************
;/** ASM_FillRect32                                                  **
;/**                                                                 **
;/** void ASM_FillRect32(RECTDATA *data)                             **
;/*********************************************************************
%if elf64=1 || win64=1
	%if elf64=1
		global ASM_FillRect32
		ASM_FillRect32:
			mov r8,rdi		; Pointer auf data nach r8
	%elif win64=1
		global _ASM_FillRect32
		_ASM_FillRect32:
			push rdi
			push rsi
			mov r8,rcx		; Pointer auf data nach r8
	%endif
	push rbx
	mov ebx,[r8+width]		; Breite nach ebx
	mov rdi,[r8+tgt]		; Zieladresse nach edi
	mov ecx,ebx				; Breite nach ecx kopieren für Abzug non Pitch
	mov esi,[r8+pitch]		; Pitch nach esi
	shl ecx,2
	sub rsi,rcx				; Breite*4 muss von Pitch abgezogen werden

	; Wenn das Rechteck kleiner als 16 Pixel ist, benutzen wir eine gesonderte Routine mit stosd
	cmp ebx,16
	ja .BiggerRect
		mov eax,[r8+color]
		mov edx,[r8+height]	; Höhe nach edx
		.box1_loop:
			mov ecx,ebx		; width
			rep stosd
			add rdi,rsi
			dec edx
			jnz .box1_loop
			jmp near .end

	.BiggerRect:
	test ebx,15		; Restpixel
	jz .NoRest		; Breite hat keine Restpixel
		mov ecx,ebx					; In ecx brauchen wir die Breite geteilt durch 16
		movd mm0,[r8+color]
		and ebx,15
		movd mm2,ebx				; Restpixel nach mm2
		movq mm1,mm0
		shr ecx,4
		psllq mm1,32				; Farbe um 32 Bit nach links schieben
		mov ebx,[r8+height]			; Höhe nach ebx
		por mm0,mm1
		mov r9d,64					; Wert 64 in r9 speichern
		.box1_a_loop4:
			mov eax,ecx
			jmp near .box1_a_loop5
			ALIGN 16
			.box1_a_loop5:
				movntq [rdi], mm0
				movntq [rdi+8], mm0
				movntq [rdi+16], mm0
				movntq [rdi+24], mm0
				movntq [rdi+32], mm0
				movntq [rdi+40], mm0
				movntq [rdi+48], mm0
				movntq [rdi+56], mm0
				add rdi,r9				; 64 Byte addieren
				dec eax
				jnz .box1_a_loop5
			movd eax,mm2				; Restpixel nach eax
			.box1_loop6:
				movd [rdi],mm0
				add rdi,4
				dec eax
				jnz .box1_loop6

			add rdi,rsi
			dec ebx
			jnz .box1_a_loop4
		sfence
		emms
		jmp near .end
	.NoRest:
		movd mm0,[r8+color]
		mov ecx,ebx					; In ecx brauchen wir die Breite geteilt durch 16
		mov rdi,[r8+tgt]
		movq mm1,mm0
		shr ecx,4
		psllq mm1,32
		mov ebx,[r8+height]		; Höhe nach ebx
		por mm0,mm1
		mov r9d,64					; Wert 64 in r9 speichern
		.box1_a_loop2:
			mov eax,ecx
			jmp near .box1_a_loop3
			ALIGN 16
			.box1_a_loop3:
				movntq [rdi], mm0
				movntq [rdi+8], mm0
				movntq [rdi+16], mm0
				movntq [rdi+24], mm0
				movntq [rdi+32], mm0
				movntq [rdi+40], mm0
				movntq [rdi+48], mm0
				movntq [rdi+56], mm0
				add rdi,r9
				dec eax
				jnz .box1_a_loop3
			add rdi,rsi
			dec ebx
			jnz .box1_a_loop2
	.end:
	pop rbx
	%if win64=1
		pop rsi
		pop rdi
	%endif
	ret

%else	; 32-Bit Assembler
global ASM_FillRect32
global _ASM_FillRect32

ASM_FillRect32:
_ASM_FillRect32:
	push ebp
	mov ebp,[esp+8]			; Pointer auf data nach ebp
	pushad
	mov ebx,[ebp+width]		; Breite nach ebx
	mov edi,[ebp+tgt]		; Zieladresse nach edi
	mov ecx,ebx				; Breite nach ecx kopieren für Abzug non Pitch
	mov esi,[ebp+pitch]		; Pitch nach esi
	shl ecx,2
	sub esi,ecx				; Breite*4 muss von Pitch abgezogen werden

	; Wenn das Rechteck kleiner als 16 Pixel ist, benutzen wir eine gesonderte Routine mit stosd
	cmp ebx,16
	ja .BiggerRect
		mov eax,[ebp+color]
		mov edx,[ebp+height]
		.box1_loop:
			mov ecx,ebx		; width
			rep stosd
			add edi,esi
			dec edx
			jnz .box1_loop
			popad
			pop ebp
			ret

	.BiggerRect:
	test ebx,15		; Restpixel
	jz .NoRest		; Breite hat keine Restpixel
		mov ecx,ebx					; In ecx brauchen wir die Breite geteilt durch 16
		movd mm0,[ebp+color]
		and ebx,15
		mov edx,[ebp+tgt]
		movd mm2,ebx				; Restpixel nach mm2
		movq mm1,mm0
		shr ecx,4
		mov ebx,[ebp+height]		; Höhe nach ebx
		psllq mm1,32
		por mm0,mm1

		.box1_a_loop4:
			mov eax,ecx
			ALIGN 16
			.box1_a_loop5:
				movntq [edx], mm0
				movntq [edx+8], mm0
				movntq [edx+16], mm0
				movntq [edx+24], mm0
				movntq [edx+32], mm0
				movntq [edx+40], mm0
				movntq [edx+48], mm0
				movntq [edx+56], mm0
				add edx,64
				dec eax
				jnz .box1_a_loop5
			movd eax,mm2				; Restpixel nach eax
			.box1_loop6:
				movd [edx],mm0
				add edx,4
				dec eax
				jnz .box1_loop6

			add edx,esi
			dec ebx
			jnz .box1_a_loop4
		sfence
		emms
		popad
		pop ebp
		ret

	.NoRest:
		movd mm0,[ebp+color]
		mov ecx,ebx					; In ecx brauchen wir die Breite geteilt durch 16
		mov edx,[ebp+tgt]
		movq mm1,mm0
		shr ecx,4
		mov ebx,[ebp+height]		; Höhe nach ebx
		psllq mm1,32
		por mm0,mm1

		.box1_a_loop2:
			mov eax,ecx
			ALIGN 16
			.box1_a_loop3:
				movntq [edx], mm0
				movntq [edx+8], mm0
				movntq [edx+16], mm0
				movntq [edx+24], mm0
				movntq [edx+32], mm0
				movntq [edx+40], mm0
				movntq [edx+48], mm0
				movntq [edx+56], mm0
				add edx,64
				dec eax
				jnz .box1_a_loop3
			add edx,esi
			dec ebx
			jnz .box1_a_loop2
		.end:
		sfence
		emms
		popad
		pop ebp
		ret

%endif

