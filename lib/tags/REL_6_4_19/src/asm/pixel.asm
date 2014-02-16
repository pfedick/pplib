;/*******************************************************************************
;/* This file is part of "Patrick's Programming Library", Version 6 (PPL6).
; * Web: http://www.pfp.de/ppl/
; *
; * $Author$
; * $Revision$
; * $Date$
; * $Id$
; *
;/*******************************************************************************
; * Copyright (c) 2010, Patrick Fedick <patrick@pfp.de>
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
;/** pixel.asm                                                       **
;/*********************************************************************

%include "src/asm/common.asminc"



SECTION .data
SECTION .text

;/*****************************************************************************
;/** void PutPixel_32 (DRAWABLE_DATA &data, int x, int y, COLOR color)  **
;/*****************************************************************************

; ELF 64 ************************ ELF 64 ************************ ELF 64 ************************ ELF 64
%if elf64=1
	global PutPixel_32
	PutPixel_32:						; DRAWABLE_DATA=rdi, x=rsi, y=rdx, color=rcx
		; Pixel muss in Grafik passen, d.h....
		; Hier tricksen wir, indem wir einen Vorzeichenlosen vergleich machen, obwohl wir einen Integer haben
		; Ist der Integer negativ, hat er vorzeichenlos betrachtet einen sehr hohen Wert und liegt mit
		; sehr hoher Wahrscheinlichkeit über der Breite, bzw. Höhe. Wir brauchen die Koordinaten daher nur
		; einmal mit der Breite bzw. Höhe zu vergleichen
		; edx muss >= 0 und < DRAWABLE_DATA.width
		cmp edx,[rdi+DRAWABLE_DATA.height]
		jae .end							; Ziel Größer oder Gleich width
		; esi muss >= 0 und < DRAWABLE_DATA.width
		cmp esi,[rdi+DRAWABLE_DATA.width]
		jae .end							; Ziel Größer oder Gleich width

		; Zunächst benötigen wir die Surface-Adresse
		imul edx, [rdi+DRAWABLE_DATA.pitch]	; y*pitch
		add rdx,[rdi+DRAWABLE_DATA.base]	; plus Basisadresse
		mov [rdx+rsi*4],ecx					; Pixel schreiben
		.end:
		ret

; WIN 64 ************************ WIN 64 ************************ WIN 64 ************************ WIN 64
%elif win64=1
	global _PutPixel_32
	_PutPixel_32:						; DRAWABLE_DATA=rcx, x=rdx, y=r8, color=r9
		cmp r8d,[rcx+DRAWABLE_DATA.height]
		jae .end							; Ziel Größer oder Gleich width
		; esi muss >= 0 und < DRAWABLE_DATA.width
		cmp edx,[rcx+DRAWABLE_DATA.width]
		jae .end							; Ziel Größer oder Gleich width

		; Zunächst benötigen wir die Surface-Adresse
		imul r8d, [rcx+DRAWABLE_DATA.pitch]	; y*pitch
		add r8,[rcx+DRAWABLE_DATA.base]		; plus Basisadresse
		mov [r8+rdx*4],ecx					; Pixel schreiben
		.end:
		ret
		
; ELF 32 / WIN 32 *************** ELF 32 / WIN 32 *************** ELF 32 / WIN 32 *************** ELF 32 / WIN 32
%elif elf32=1 || win32=1
	global PutPixel_32
	global _PutPixel_32
	PutPixel_32:
	_PutPixel_32:
		push ebp
		mov eax,[esp+20]				; Color nach eax
		mov edx,[esp+16]				; y nach edx
		mov ecx,[esp+12]					; x nach ecx
		mov ebp,[esp+8]					; DRAWABLE_DATA nach ebp
		cmp edx, [ebp+DRAWABLE_DATA.height]
		jae .end
		cmp ecx, [ebp+DRAWABLE_DATA.width]
		jae .end
		; Zunächst benötigen wir die Surface-Adresse
		imul edx, [ebp+DRAWABLE_DATA.pitch]	; y*pitch
		add edx,[ebp+DRAWABLE_DATA.base]	; plus Basisadresse
		mov [edx+ecx*4],eax					; Pixel schreiben
		.end:
		pop ebp
		ret
%endif

;/*****************************************************************************
;/** SurfaceColor GetPixel_32 (DRAWABLE_DATA &data, int x, int y)            **
;/*****************************************************************************

; ELF 64 ************************ ELF 64 ************************ ELF 64 ************************ ELF 64
%if elf64=1
	global GetPixel_32
	GetPixel_32:							; DRAWABLE_DATA=rdi, x=rsi, y=rdx
		xor eax,eax
		; Pixel muss in Grafik passen, d.h....
		; Hier tricksen wir, indem wir einen Vorzeichenlosen vergleich machen, obwohl wir einen Integer haben
		; Ist der Integer negativ, hat er vorzeichenlos betrachtet einen sehr hohen Wert und liegt mit
		; sehr hoher Wahrscheinlichkeit über der Breite, bzw. Höhe. Wir brauchen die Koordinaten daher nur
		; einmal mit der Breite bzw. Höhe zu vergleichen
		; edx muss >= 0 und < DRAWABLE_DATA.width
		cmp edx,[rdi+DRAWABLE_DATA.height]
		jae .end							; Ziel Größer oder Gleich width
		; esi muss >= 0 und < DRAWABLE_DATA.width
		cmp esi,[rdi+DRAWABLE_DATA.width]
		jae .end							; Ziel Größer oder Gleich width

		; Zunächst benötigen wir die Surface-Adresse
		imul edx, [rdi+DRAWABLE_DATA.pitch]	; y*pitch
		add rdx,[rdi+DRAWABLE_DATA.base]	; plus Basisadresse
		mov eax,[rdx+rsi*4]					; Pixel lesen
		.end:
		ret

; WIN 64 ************************ WIN 64 ************************ WIN 64 ************************ WIN 64
%elif win64=1
	global _GetPixel_32
	_GetPixel_32:						; DRAWABLE_DATA=rcx, x=rdx, y=r8
		xor eax,eax
		cmp r8d,[rcx+DRAWABLE_DATA.height]
		jae .end							; Ziel Größer oder Gleich width
		; esi muss >= 0 und < DRAWABLE_DATA.width
		cmp edx,[rcx+DRAWABLE_DATA.width]
		jae .end							; Ziel Größer oder Gleich width

		; Zunächst benötigen wir die Surface-Adresse
		imul r8d, [rcx+DRAWABLE_DATA.pitch]	; y*pitch
		add r8,[rcx+DRAWABLE_DATA.base]		; plus Basisadresse
		mov eax, [r8+rdx*4]					; Pixel lesen
		.end:
		ret
		
; ELF 32 / WIN 32 *************** ELF 32 / WIN 32 *************** ELF 32 / WIN 32 *************** ELF 32 / WIN 32
%elif elf32=1 || win32=1
	global GetPixel_32
	global _GetPixel_32
	GetPixel_32:
	_GetPixel_32:
		push ebp
		xor eax,eax
		mov edx,[esp+16]				; y nach edx
		mov ecx,[esp+12]					; x nach ecx
		mov ebp,[esp+8]					; DRAWABLE_DATA nach ebp
		cmp edx, [ebp+DRAWABLE_DATA.height]
		jae .end
		cmp ecx, [ebp+DRAWABLE_DATA.width]
		jae .end
		; Zunächst benötigen wir die Surface-Adresse
		imul edx, [ebp+DRAWABLE_DATA.pitch]	; y*pitch
		add edx,[ebp+DRAWABLE_DATA.base]	; plus Basisadresse
		mov eax, [edx+ecx*4]			; Pixel lesen
		.end:
		pop ebp
		ret
%endif

;/*****************************************************************************
;/** int AlphaPixel_32 (DRAWABLE_DATA &data, int x, int y, COLOR color)  **
;/*****************************************************************************

; ELF 64 ************************ ELF 64 ************************ ELF 64 ************************ ELF 64
%if elf64=1
	global AlphaPixel_32
	AlphaPixel_32:						; DRAWABLE_DATA=rdi, x=rsi, y=rdx, color=rcx
		; Pixel muss in Grafik passen...
		; Hier tricksen wir, indem wir einen Vorzeichenlosen vergleich machen, obwohl wir einen Integer haben
		; Ist der Integer negativ, hat er vorzeichenlos betrachtet einen sehr hohen Wert und liegt mit
		; sehr hoher Wahrscheinlichkeit über der Breite, bzw. Höhe. Wir brauchen die Koordinaten daher nur
		; einmal mit der Breite bzw. Höhe zu vergleichen
		; edx muss >= 0 und < DRAWABLE_DATA.height
		cmp edx,[rdi+DRAWABLE_DATA.height]
		jae .end_noemms					; Ziel Größer oder Gleich width
		; esi muss >= 0 und < DRAWABLE_DATA.width
		cmp esi,[rdi+DRAWABLE_DATA.width]
		jae .end_noemms					; Ziel Größer oder Gleich width

		; Zunächst benötigen wir die Surface-Adresse und den Farbwert des Ziels in mm2
		imul edx, [rdi+DRAWABLE_DATA.pitch]	; y*pitch
		; Wir betrachten zunächst den Alpha-Wert
		movd mm3,ecx					; Farbe nach mm3
		add rdx,[rdi+DRAWABLE_DATA.base]; plus Basisadresse
		shr ecx,24						; Alpha jetzt in cl
		mov eax, dword [rdx+rsi*4]		; Pixel einlesen
		jz .end							; AlphaWert 0?
			inc cl
			jz .useForeground			; Wenn Alpha=255 ist, dann Wird der Farbwert ohne Blending geschrieben
				movd mm2,eax			; Hintergrund nach mm2
				pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
				movd mm0,ecx			; Alphachannel nach mm0
				punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
				punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
				pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
				psubsw mm3,mm2			; src-dst mit Vorzeichen
				mov eax,0xff
				pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
				movd mm7,eax
				psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
				pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 zum Maskieren
				paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
				shl eax,24				; Neuen Alphachannel vorbereiten
				pand mm3,mm7			; Die oberen Bytes ausmaskieren
				movd mm0,eax			; Neuen Alphachannel nach mm0
				packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
				por mm3,mm0				; Alpha-Channel auf 0xff
			.useForeground:
			movd dword [rdx+rsi*4], mm3
		.end:
		emms
		.end_noemms:
		ret

; WIN 64 ************************ WIN 64 ************************ WIN 64 ************************ WIN 64
%elif win64=1
	global _AlphaPixel_32
	_AlphaPixel_32:						; DRAWABLE_DATA=rcx, x=rdx, y=r8, color=r9
		; r8d muss >= 0 und < DRAWABLE_DATA.height
		cmp r8d,[rcx+DRAWABLE_DATA.height]
		jae .end_noemms					; Ziel Größer oder Gleich width
		; edx muss >= 0 und < DRAWABLE_DATA.width
		cmp edx,[rcx+DRAWABLE_DATA.width]
		jae .end_noemms					; Ziel Größer oder Gleich width

		; Zunächst benötigen wir die Surface-Adresse und den Farbwert des Ziels in mm2
		imul r8d, [rcx+DRAWABLE_DATA.pitch]	; y*pitch
		; Wir betrachten zunächst den Alpha-Wert
		movd mm3,r9						; Farbe nach mm3
		add r8, [rcx+DRAWABLE_DATA.base]; plus Basisadresse
		shr r9d,24						; Alpha jetzt in r9b
		mov eax, dword [r8+rdx*4]		; Pixel einlesen
		jz .end							; AlphaWert 0?
			inc r9b
			jz .useForeground			; Wenn Alpha=255 ist, dann Wird der Farbwert ohne Blending geschrieben
				movd mm2,eax			; Hintergrund nach mm2
				pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
				movd mm0,r9d			; Alphachannel nach mm0
				punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
				punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
				pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
				psubsw mm3,mm2			; src-dst mit Vorzeichen
				mov eax,0xff
				pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
				movd mm7,eax
				psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
				pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 zum Maskieren
				paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
				shl eax,24				; Neuen Alphachannel vorbereiten
				pand mm3,mm7			; Die oberen Bytes ausmaskieren
				movd mm0,eax			; Neuen Alphachannel nach mm0
				packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
				por mm3,mm0				; Alpha-Channel auf 0xff
			.useForeground:
			movd dword [r8+rdx*4], mm3
		.end:
		emms
		.end_noemms:
		ret

		

; ELF 32 / WIN 32 *************** ELF 32 / WIN 32 *************** ELF 32 / WIN 32 *************** ELF 32 / WIN 32
%elif elf32=1 || win32=1
	global AlphaPixel_32
	global _AlphaPixel_32
	AlphaPixel_32:
	_AlphaPixel_32:						; DRAWABLE_DATA=ebp, x=rsi, y=rdx, color=rcx
		push ebp
		mov eax,[esp+20]				; Color nach eax
		mov edx,[esp+16]				; y nach edx
		mov ecx,[esp+12]				; x nach ecx
		mov ebp,[esp+8]					; DRAWABLE_DATA nach ebp
		push ebx
		cmp edx, [ebp+DRAWABLE_DATA.height]
		jae .end_noemms
		cmp ecx, [ebp+DRAWABLE_DATA.width]
		jae .end_noemms
		; Zunächst benötigen wir die Surface-Adresse
		imul edx, [ebp+DRAWABLE_DATA.pitch]	; y*pitch
		movd mm3,eax					; Farbe nach mm3
		add edx,[ebp+DRAWABLE_DATA.base]	; plus Basisadresse
		shr eax,24						; Alpha jetzt in al
		mov ebx, [edx+ecx*4]			; Pixel nach ebx lesen
		; Wir betrachten zunächst den Alpha-Wert
		jz .end							; AlphaWert 0?
			inc al
			jz .useForeground			; Wenn Alpha=255 ist, dann Wird der Farbwert ohne Blending geschrieben
				movd mm2,ebx			; Hintergrund nach mm2
				pxor mm6,mm6			; mm6 benötigen wir für PUNPCKLBW und muß 0 sein
				movd mm0,eax			; Alphachannel nach mm0
				punpcklbw mm3,mm6		; Farbe in mm3 ist jetzt: 0a0r0g0b
				punpcklbw mm2,mm6		; Hintergrund in mm2: 0a0r0g0b
				pshufw mm0,mm0,0		; Multiplikator in alle 4 16-Bit Werte
				psubsw mm3,mm2			; src-dst mit Vorzeichen
				mov eax,0xff
				pmullw mm3,mm0			; Farbe mit Multiplikator multiplizieren
				movd mm7,eax
				psraw mm3,8				; Das Ergebnis müssen wir unter Berücksichtigung des Vorzeichens durch 256 teilen...
				pshufw mm7,mm7,0		; 0x00ff00ff00ff00ff in mm7 zum Maskieren
				paddsw mm3,mm2			; Und den Hintergrund dazuaddieren
				shl eax,24				; Neuen Alphachannel vorbereiten
				pand mm3,mm7			; Die oberen Bytes ausmaskieren
				movd mm0,eax			; Neuen Alphachannel nach mm0
				packuswb mm3,mm6		; die 4 16-Bit Farbwerte in 32Bit unterbringen
				por mm3,mm0				; Alpha-Channel auf 0xff
			.useForeground:
			movd dword [edx+ecx*4], mm3
		.end:
		emms
		.end_noemms:
		pop ebx
		pop ebp
		ret
%endif
