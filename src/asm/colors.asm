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
	global RGBBlend_32_255
	RGBBlend_32_255:
		;; Register ALT => NEU
		;; rdx => ecx		: Intensity
		;; mm6 => xmm1		: für PUNPCKLBW unpacking
		;; mm2 => eax,xmm2  : ground
		;; mm3 => xmm3      : top
		;; mm0 => xmm0      : Inensity

	%if arch_elf64=1
		; Linux ABI: ground=rdi, top=rsi, intensity=rdx
		mov eax,edi					; rax = ground (Backup für schnellen Return)
		mov ecx, edx  		        ; ecx = intensity
		movd xmm3, esi       		; r10d = top
	%elif arch_win64=1	
		; Windows ABI: ground=rcx, top=rdx, intensity=r8
		mov eax, ecx        ; rax = ground
		mov ecx, r8d        ; ecx = intensity
        movd xmm3, edx       ; r10d = top
    %endif
	and ecx, 255
	jz .useBackground       ; Wenn intensity == 0, ground zurückgeben
		cmp ecx, 255
		jge .useForeground      ; Wenn intensity == 255, top zurückgeben
			pxor xmm1, xmm1         ; xmm1 = 0 für Unpacking
			movd xmm2, eax          ; xmm2 = ground (lädt exakt 32 Bit, nullt den Rest)

			; Bytes in 16-Bit Wörter entpacken (00aa 00rr 00gg 00bb)
			punpcklbw xmm2, xmm1    ; ground: 00aa 00rr 00gg 00bb (Wörter)
			punpcklbw xmm3, xmm1    ; top:    00aa 00rr 00gg 00bb (Wörter)

			; intensity (i2) vorbereiten
    		movd xmm0, ecx
    		pshuflw xmm0, xmm0, 0   ; xmm0 = intensity (i2) in allen Wörtern
			; umgekehrte_intensity (i1 = 255 - intensity) vorbereiten
    		mov edx, 255
    		sub edx, ecx
    		movd xmm4, edx
    		pshuflw xmm4, xmm4, 0   ; xmm4 = 255 - intensity (i1) in allen Wörtern

			; Multiplikationen (Beide Teilergebnisse passen jeweils in uint16_t)
			; Da wir getrennt multiplizieren, wird das Maximum pro Kanal 255 * 255 = 65025.
			; Das passt exakt in ein vorzeichenloses 16-Bit Wort (max 65535)!
			pmullw xmm2, xmm4       ; xmm2 = ground * i1
			pmullw xmm3, xmm0       ; xmm3 = top * i2

			; Die korrekte Division durch 255 direkt für die Summanden anwenden,
			; um den additiven Überlauf (über 65535) zu verhindern.
			; Formel: (V + 128 + ((V + 128) >> 8)) >> 8
			
			; Konstante 128 erzeugen
			pcmpeqw xmm5, xmm5
			psrlw xmm5, 15
			psllw xmm5, 7           ; xmm5 = 128 in allen Wörtern

			; Division für (ground * i1)
			paddw xmm2, xmm5        ; V + 128
			movdqa xmm4, xmm2
			psrlw xmm4, 8           ; >> 8
			paddw xmm2, xmm4
			psrlw xmm2, 8           ; xmm2 = (ground * i1) / 255

			; Division für (top * i2)
			paddw xmm3, xmm5        ; V + 128
			movdqa xmm4, xmm3
			psrlw xmm4, 8           ; >> 8
			paddw xmm3, xmm4
			psrlw xmm3, 8           ; xmm3 = (top * i2) / 255

			; Jetzt die beiden fertig dividierten Werte addieren
			paddw xmm2, xmm3        ; xmm2 = Endergebnis
			
			
			; Zurück in Bytes packen und ausgeben
    		packuswb xmm2, xmm1     ; Packen nach Bytes
    		movd eax, xmm2          ; Zurück in Allzweckregister
    		ret


	.useBackground:
		; eax enthält bereits ground
    	ret

	.useForeground:
		%if arch_elf64=1
    		mov eax, esi        ; esi hält top unter Linux
		%elif arch_win64=1
    		mov eax, edx        ; edx hält top unter Windows
		%endif
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
