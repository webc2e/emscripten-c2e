; Nasm versions of the assembler blitting routines
; These can be used on any X86 platform with Nasm 
; available (i.e. most of them!).

; Under Windows the Visual C++ inline assembly is used
; - you can find it in DisplayEnginePlotFunctions.h
; and DisplayEngine.h/SDL_DisplayEngine.h
; Nasm could be used under Windows, but it wasn't.

; Ugh.  We pass all our parameters in using global
; variables.  Perhaps they should use the stack, but
; this way was more similar to the inline assembly
; used with Visual C++.
extern bitmapHeight
extern data_ptr
extern screen_ptr
extern data_step
extern screen_step
extern dwordWidth
extern data_step
extern bitmapWidth
extern compressedData_ptr

; Export all the functions so C++ can call them
global NasmDrawBitmapFirstPart
global NasmDrawBitmapSecondPart
global NasmDrawWholeBitmapRegardless
global NasmDrawSprite
global NasmDrawCompressedSprite

NasmDrawBitmapFirstPart:
	mov eax,dword [bitmapHeight]	; eax is now the number of lines to plot.
	mov esi,dword [data_ptr]		; esi points to sprite data now
	mov edi,dword [screen_ptr]		; edi points to screen data now
	mov ebx,dword [data_step]		; ebx is now the data step
	mov edx,dword [screen_step]		; edx is now the screen step
lineloop:
	mov ecx,dword [dwordWidth]

	rep movsd 						; copy the dwords from data to screen

	; This is the only line different from NasmDrawBitmapSecondPart below
	movsw							; copy the straggling Pixel

	lea esi,[esi+ebx*2]				; esi += ebx  (data_ptr += data_step)
	lea edi,[edi+edx*2]				; edi += edx  (screen_ptr += screen_step)
	dec eax
	jne lineloop					; If not zero, process another line

	ret

NasmDrawBitmapSecondPart:
	mov eax,dword [bitmapHeight]	; eax is now the number of lines to plot.
	mov esi,dword [data_ptr]		; esi points to sprite data now
	mov edi,dword [screen_ptr]		; edi points to screen data now
	mov ebx,dword [data_step]		; ebx is now the data step
	mov edx,dword [screen_step]		; edx is now the screen step
lineloop2:
	mov ecx,dword [dwordWidth]

	rep movsd 						; copy the dwords from data to screen

	lea esi,[esi+ebx*2]				; esi += ebx  (data_ptr += data_step)
	lea edi,[edi+edx*2]				; edi += edx  (screen_ptr += screen_step)
	dec eax
	jne lineloop2					; If not zero, process another line
	
	ret

NasmDrawWholeBitmapRegardless:
	mov edx,dword [screen_step]		; edx is now the screen step
	mov edi,dword [screen_ptr]		; edi points to screen data now
	mov esi,dword [data_ptr]		; esi points to sprite data now
	mov eax,128							; eax is now the number of lines to plot.
	add esi,32
lineloop3:

	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]
	add edi,32
	fld qword [esi - 32]
	fld qword [esi - 24]
	fld qword [esi - 16]
	fld qword [esi - 8]
	add esi,32
	fstp qword [edi - 8]
	fstp qword [edi - 16]
	fstp qword [edi - 24]
	fstp qword [edi - 32]

	lea edi,[edi+edx]					; edi += edx  (screen_ptr += screen_step)
	dec eax
	jne near lineloop3						; If not zero, process another line

	ret


NasmDrawSprite:
	mov esi,dword [data_ptr]
	mov edi,dword [screen_ptr]
	mov ebx,dword [data_step]
	mov edx,dword [screen_step]
	
	push dword [bitmapHeight]		;Stack up bitmapHeight for later 

topOfOuterLoop:
	mov ecx,dword [bitmapWidth]		;Get the number of pixels we are playing with
topOfInnerLoop:
	;Load pixel into AX
	lodsw
	test ax,ax
	je dontstore
	mov word [edi],ax				;Store screen pixel
dontstore:
	add edi,2							;Increment screen pointer one word

	;Inner loop epilogue
	dec ecx
	jne topOfInnerLoop					;If not done with line, jump to top of inner loop

	; Deal with the widths :)


	;Outer loop epilogue
	pop ecx								;Destack bitmapHeight

	lea esi,[esi+ebx*2]					;data_ptr += data_step
	
	dec ecx
	
	lea edi,[edi+edx*2]					;screen_ptr += screen_step
	
	push ecx							;Restack bitmapHeight
	
	jne topOfOuterLoop

	;asm epilogue

	pop ecx								;clean stack pre popa

	ret

NasmDrawCompressedSprite:
	mov esi,dword [compressedData_ptr]
	mov edi,dword [screen_ptr]
	mov edx,dword [screen_step]
	mov ebx,dword [bitmapHeight]

topOfLineLoop:
	lodsw								;Get tag and increment ptr lodsb
	and eax,0ffffh
	test eax,eax
	je lineLoopEpilogue
	test ax,1
	je skipCopy
	mov ecx,eax
	shr ecx,1
	rep movsw 							;dword ptr [edi],dword ptr [esi]
	jmp topOfLineLoop
skipCopy:	
	lea edi,[edi+eax]
	jmp topOfLineLoop
lineLoopEpilogue:
	lea edi,[edi+edx*2]
	dec ebx
	jne topOfLineLoop

	ret

