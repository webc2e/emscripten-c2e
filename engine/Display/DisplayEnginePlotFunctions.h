#ifndef DISPLAYENGINEPLOTFUNCTIONS_H
#define DISPLAYENGINEPLOTFUNCTIONS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Bitmap.h"

// Nasm declarations (the variables are
// defined in Bitmap.cpp, the functions 
// in Plot.asm).  For now we use these
// global variables even in the non-asm
// version - hopefully won't slow it down!
extern int32 bitmapHeight;
extern uint16* data_ptr;
extern uint32* screen_ptr;
extern uint32 data_step;
extern uint32 screen_step;
extern int dwordWidth;

#ifdef C2E_NASM_RENDERING
extern "C"
{
	void NasmDrawBitmapFirstPart();
	void NasmDrawBitmapSecondPart();
	void NasmDrawWholeBitmapRegardless();
}
#endif


inline int Get555Red(int colour)
{	return (colour>>7)&0xf8;}

inline int Get555Green(int colour)
{	return (colour>>2)&0xf8;}

inline int Get555Blue(int colour)
{	return (colour<<3)&0xf8;}

inline int Get565Red(int colour)
{	return (colour>>8)&0xf8;}

inline int Get565Green(int colour)
{	return (colour>>3)&0xfc;}

inline int Get565Blue(int colour)
{	return (colour<<3)&0xf8;}

// ----------------------------------------------------------------------
// Method:      DrawBitmap 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap to draw
// Returns:     None
//
// Description: The interface to the directdraw object this method creates
//				surface.
//			
// ----------------------------------------------------------------------

inline void DisplayEngine::DrawBitmap(Position& position,Bitmap& bitmap) {
	int32 bitmapWidth = bitmap.GetWidth();
	bitmapHeight = bitmap.GetHeight();

	int32 originalBitmapWidth = bitmap.GetWidth();
	int32 originalBitmapHeight = bitmap.GetHeight();

	// work out how much to increase the data and screen pointers
	// on when drawing
	data_step = bitmapWidth;
	
	data_ptr = bitmap.GetData();
	
	screen_step = myPitch;
	screen_ptr = GetBackBufferPtr();

	if (screen_ptr == NULL)
		return;

	int32	x=position.GetX();
	int32	y=position.GetY();


	ASSERT(data_ptr);
	
	ASSERT(screen_ptr);


	// determine whether we have to clip the
	// sprite
	if (x<0)
	{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return;
		// only modify the dataptr (not the compressed one)
		data_ptr-=x;
		x=0;
	}

	
	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
	// int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
	{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return;
	}
	


	
	if (y<0)
	{
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return;

		// only modify the dataptr (not the compressed one)
		data_ptr-=(y*bitmap.GetWidth());
		y=0;
	}


	// only bother to do this for really large bitmaps
		// they will be the only ones that go right off
		// both ends of the screen
	t = (y+bitmapHeight)-mySurfaceArea.bottom;

		// if the bitmap needs clipping at the bottom
		if (t>=0)
			{
			bitmapHeight-=t;
			if (bitmapHeight<=0)
				return;
			}


	


	screen_ptr+=(y*myPitch)+x;


	// draw for your life
#ifdef C2E_NO_INLINE_ASM

	int i;
	for( i=0; i<bitmapHeight; ++i )
	{
		int j;
		for (j = 0; j < originalBitmapWidth; ++j) {
			if (j < bitmapWidth) {
				uint16 byte = *data_ptr;

				int r = Get565Red(byte);
				int g = Get565Green(byte);
				int b = Get565Blue(byte);

				uint32 pixel = (b << 16) + (g << 8) + r;

				*screen_ptr = pixel;
				screen_ptr++;
			}
			data_ptr++;
		} 
			// memcpy(screen_ptr, data_ptr, 128 * (sizeof(uint32)));
		// Additions to get to next row (pixels, not bytes!)
		// data_ptr += 128;
		screen_ptr += myPitch - bitmapWidth;	// screen_step;
	}
#else
	// when using memcpy don't do this
	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	// New Inline Assembly version (Dan)

	dwordWidth = bitmapWidth>>1;
	if (bitmapWidth == 0 || bitmapHeight == 0)
		return;
	//If we have a straggling pixel, do extra copy...
	if (bitmapWidth & 1)
#ifdef C2E_NASM_RENDERING
	{
		NasmDrawBitmapFirstPart();
	}
	else
	{
		NasmDrawBitmapSecondPart();
	}
#else
	_asm
	{
		mov eax,dword ptr [bitmapHeight]	; eax is now the number of lines to plot.
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		;mov ebp,dword ptr [dwordWidth]		; ebp is the width of the bitmap - already shifted to DWORD size
		mov ebx,dword ptr [data_step]		; ebx is now the data step
		mov edx,dword ptr [screen_step]		; edx is now the screen step
	lineloop:
		;mov ecx,ebp							; ecx gets the number of dwords to copy per loop
		mov ecx,dword ptr [dwordWidth]
		rep movs dword ptr [edi],dword ptr [esi]
											; copy the dwords from data to screen
		movsw
											; copy the straggling Pixel
		lea esi,[esi+ebx*2]					; esi += ebx  (data_ptr += data_step)
		lea edi,[edi+edx*2]					; edi += edx  (screen_ptr += screen_step)
;		sub eax,1							; decrement bitmap height properly
		dec eax
		jne lineloop						; If not zero, process another line
	}
	else
	_asm
	{
		mov eax,dword ptr [bitmapHeight]	; eax is now the number of lines to plot.
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		;mov ebp,dword ptr [dwordWidth]		; ebp is the width of the bitmap - already shifted to DWORD size
		mov ebx,dword ptr [data_step]		; ebx is now the data step
		mov edx,dword ptr [screen_step]		; edx is now the screen step
	lineloop2:
		;mov ecx,ebp							; ecx gets the number of dwords to copy per loop
		mov ecx,dword ptr [dwordWidth]
		rep movs dword ptr [edi],dword ptr [esi]
											; copy the dwords from data to screen
		lea esi,[esi+ebx*2]					; esi += ebx  (data_ptr += data_step)
		lea edi,[edi+edx*2]					; edi += edx  (screen_ptr += screen_step)
;		sub eax,1							; decrement bitmap height properly
		dec eax
		jne lineloop2						; If not zero, process another line
	}
#endif
#endif
}



// ----------------------------------------------------------------------
// Method:      DrawWholeBitmapRegardless 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap to draw
// Returns:     None
//
// Description: The interface to the directdraw object this method creates
//				surface.
//			
// ----------------------------------------------------------------------
inline void DisplayEngine::DrawWholeBitmapRegardless(Position& position,Bitmap& bitmap)
{
	// Here we assume certain information about the Bitmaps.
	// 1. Size is always 128x128
	// 2. Therefore is always even
	// 3. We are drawing the whole bitmap, so DataStep is zero
	// 4. myPitchForBackgroundTiles is valid (myPitch-128)

	data_ptr = bitmap.GetData();
	screen_ptr = GetBackBufferPtr();
	screen_step = myPitchForBackgroundTiles;
	screen_ptr+=(position.GetY()*myPitch)+position.GetX();
#ifdef C2E_NO_INLINE_ASM
	int i;
	for( i=0; i<128; ++i )
	{
		int j;
		for (j = 0; j < 128; ++j) {
			uint16 byte = *data_ptr;

			int r = Get565Red(byte);
			int g = Get565Green(byte);
			int b = Get565Blue(byte);

			uint32 pixel = (b << 16) + (g << 8) + r;

			*screen_ptr = (uint32) pixel;
			data_ptr++;
			screen_ptr++;
		} 
			// memcpy(screen_ptr, data_ptr, 128 * (sizeof(uint32)));
		// Additions to get to next row (pixels, not bytes!)
		// data_ptr += 128;
		screen_ptr += myPitch - 128;	// screen_step;
	}

#else
	
#ifdef C2E_NASM_RENDERING
	{
		NasmDrawWholeBitmapRegardless();
	}
#else
	_asm
	{
		mov edx,dword ptr [screen_step]		; edx is now the screen step
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov eax,128							; eax is now the number of lines to plot.
		add esi,32
	lineloop2:
		;mov ecx,128
		;rep movsw                           ; copy the dwords from data to screen

		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]

		lea edi,[edi+edx]					; edi += edx  (screen_ptr += screen_step)
		dec eax
		jne lineloop2						; If not zero, process another line
	}
#endif // C2E_NASM_RENDERING
#endif // C2E_NO_INLINE_ASM
}


#endif //DISPLAYENGINEPLOTFUNCTIONS_H

