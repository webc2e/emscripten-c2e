// $Id: DisplayEnginePlotFunctions.cpp,v 1.7 2001/04/24 15:01:22 firving Exp $

// Plot functions which are shared between SDL and DirectX display engines

#include "DisplayEngine.h"
#include "CompressedBitmap.h"

// ----------------------------------------------------------------------
// Method:      DrawAlphaCompressedSprite 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
// Returns:     None
//
// Description: Draw the sprite to the back buffer taking account of
//				Transparency encoding.  this is the longest set of code
//				because I have different routines for whether there is
//				no clipping.  right.bottom clipping. left/top clipping.
//						
// ----------------------------------------------------------------------
void DisplayEngine::DrawAlphaCompressedSprite(Position& position,
										 CompressedBitmap* bitmap,
										 int intensity)
{
	return;

// 	// Invisible
// 	if (intensity == 256)
// 		return;

// //	OutputDebugString("get drawing parameters\n");
// 	// assume that we will do left or top clip
// 	bool rightClip= false;
// 	bool bottomClip = false;
// 	bool topClip = false;
// 	bool leftClip =false;
// 	int32 bytes =0;

// 	int32	x=position.GetX();
// 	int32	y=position.GetY();

// 	int32 bitmapWidth = bitmap->GetWidth();
// 	int32 bitmapHeight = bitmap->GetHeight();

// 	// work out how much to increase the data and sreen pointers
// 	// on when drawing
// 	uint32 data_step=bitmapWidth;
// 	// when both left and right clipped
// 	// top and bottom clipped
// 	uint32 leftClipped = bitmapWidth;
// 	uint32 topClipped = bitmapHeight;
	
// 	uint16* compressedData_ptr=bitmap->GetData();
// 	ASSERT(compressedData_ptr);
	

// 	uint32 screen_step = myPitch;
// 	//	OutputDebugString("getting back buffer\n");
// 	uint32*	 screen_ptr=GetBackBufferPtr();
// 	ASSERT(screen_ptr);

// 	//	OutputDebugString("got back buffer\n");


// 	// determine whether we have to clip the
// 	// sprite
// 	if (x<0)
// 	{
// 		bitmapWidth+=x;
// 		if (bitmapWidth<0)
// 			return;
// 		x=0;
// 		leftClip = true;
// 		// this is the position in the bitmap to
// 		// start drawing at
// 		leftClipped-=bitmapWidth;
// 	}
// 	else
// 	{
// 		leftClipped = 0;
// 	}

// 	if (y<0)
// 	{
// 		bitmapHeight+=y;
// 		if (bitmapHeight<0)
// 			return;
// 		topClip = true;
// 		//!!!!!!!!
// 		compressedData_ptr = (uint16*)bitmap->GetScanLine(0-y);
// 		y=0;
// 		topClipped -= bitmapHeight; 
		
// 	}
// 	else
// 	{
// 		topClipped = 0;
// 	}

// 	int32 t=(x+bitmapWidth)-myPitch;

// 	// if the bitmap needs clipping to the right
// 	if (t>=0)
// 	{
// 		bitmapWidth-=t;
// 		if (bitmapWidth<=0)
// 			return;
// 		rightClip = true;
// 	}


// 	t = (y+bitmapHeight)-mySurfaceArea.bottom;

// 	// if the bitmap needs clipping at the bottom
// 	if (t>=0)
// 	{
// 		bitmapHeight-=t;
// 		if (bitmapHeight<=0)
// 			return;
// 		bottomClip = true;
// 	}

// 	data_step-=bitmapWidth;
// 	screen_step-=bitmapWidth;

// 	screen_ptr+=(y*myPitch)+x;


	
// 	uint16 tag = 0;
// 	int32 thisStep =0;
// 	uint16 count = 0;
// 	uint32 pixelsDrawnThisLine =0;


// 	uint16 sTemp =0;
// 	uint16 dTemp =0;
// 	uint16 sr,sg,sb,dr,dg,db =0;

	

// 	//	OutputDebugString("start actual drawing\n");
// 	// if no clipping is required
// 	if(data_step == 0)
// 	{
// 		//	OutputDebugString("start no clip\n");
// 		// draw taking account of transparent pixels
		
		
// 		for (;bitmapHeight--;)
// 		{
			
// 			tag = *compressedData_ptr++;
// 			while(tag)
// 			{
				
// 				// find the number of colours to plot
// 				count = tag >>1;
				
// 				// check whether the run is transparent or colour
// 				if(tag & 0x01)
// 				{
// 					bytes = count << 1;

// 					if(myPixelFormat == RGB_565)
// 					{
// 					for(int i = 0; i < count ; ++i)
// 						{
// 							sTemp = *compressedData_ptr++;
// 							dTemp = *screen_ptr;

// 							sb = sTemp & 0x1f;
// 							db = dTemp & 0x1f;
// 							sg = (sTemp >> 5) & 0x3f;
// 							dg = (dTemp >> 5) & 0x3f;
// 							sr = (sTemp >> 11) & 0x1f;
// 							dr = (dTemp >> 11) & 0x1f;

// 							// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 							// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 							// 	((intensity * (dr - sr) >> 8) + sr) << 11);

// 						}
// 					}
// 					else
// 					{
// 						for(int i = 0; i < count ; ++i)
// 						{
// 							sTemp = *compressedData_ptr++;
// 							dTemp = *screen_ptr;

// 							sb = sTemp & 0x1f;
// 							db = dTemp & 0x1f;
// 							sg = (sTemp >> 5) & 0x1f;
// 							dg = (dTemp >> 5) & 0x1f;
// 							sr = (sTemp >> 10) & 0x1f;
// 							dr = (dTemp >> 10) & 0x1f;
							
// 							// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 							// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 							// 	((intensity * (dr - sr) >> 8) + sr) << 10);
// 							}
// 					}

// 				}
// 				else
// 				{
// 					screen_ptr+= count;
// 				}
// 				tag = *compressedData_ptr++;
// 			}// end  while tag
			
// 			screen_ptr+=screen_step;
// 		}//end for bitmap height--
		
// 	}// end if datastep ==0
// 	else // some clipping is required
// 	{
// 		if((rightClip && !leftClip) ||(bottomClip && !leftClip))
// 		{
// 		//	OutputDebugString("start r b not left\n");

// 			uint32 i=0;
// 			int32 drawHeight = bitmapHeight;
// 			if(topClip)
// 			{
// 				i = topClipped;
// 				drawHeight = bitmapHeight + topClipped;
// 			}
// 			// for each line
// 			for (; i < drawHeight; i++)
// 			{
// 				thisStep = bitmapWidth;
// 				// find out what type of pixel we have
// 				tag = *compressedData_ptr++;
// 				while(tag)
// 				{
// 					// find the number of pixels to plot
// 					count = tag >>1;
					
// 					// we have a run of colours 
// 					if(tag & 0x01)
// 					{
// 						// if we are not at our stopping point yet
// 						if(count <= thisStep)
// 						{
// 							thisStep -= count;
// 							// we need to keep drawing unless
// 							// this is the end of the
// 							// line in which case there will be a tag
// 							// anyway
// 							bytes = count << 1;

// 							if(myPixelFormat == RGB_565)
// 							{
// 								for(int i = 0; i < count ; ++i)
// 								{
// 									sTemp = *compressedData_ptr++;
// 									dTemp = *screen_ptr;

// 									sb = sTemp & 0x1f;
// 									db = dTemp & 0x1f;
// 									sg = (sTemp >> 5) & 0x3f;
// 									dg = (dTemp >> 5) & 0x3f;
// 									sr = (sTemp >> 11) & 0x1f;
// 									dr = (dTemp >> 11) & 0x1f;

// 									// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 									// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 									// 	((intensity * (dr - sr) >> 8) + sr) << 11);

// 								}
// 							}
// 							else
// 							{
// 								for(int i = 0; i < count ; ++i)
// 								{
// 									sTemp = *compressedData_ptr++;
// 									dTemp = *screen_ptr;

// 									sb = sTemp & 0x1f;
// 									db = dTemp & 0x1f;
// 									sg = (sTemp >> 5) & 0x1f;
// 									dg = (dTemp >> 5) & 0x1f;
// 									sr = (sTemp >> 10) & 0x1f;
// 									dr = (dTemp >> 10) & 0x1f;

// 									// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 									// ((intensity * (dg - sg) >> 8) + sg) << 5 |
// 									// ((intensity * (dr - sr) >> 8) + sr) << 10);
									
// 								}

// 							}
// 						}
// 						else // pixel count has over run
// 						{
// 						//	uint32 over = count - thisStep;
// 							// then draw the remaining amount of pixels 
// 							// and set the step to zero
// 							// draw all the colours 
// 							// draw what you will
// 							bytes = thisStep << 1;
							
// 							if(myPixelFormat == RGB_565)
// 							{
// 								for(int i = 0; i < thisStep ; ++i)
// 								{

// 									sTemp = *compressedData_ptr++;
// 									dTemp = *screen_ptr;
// 									sb = sTemp & 0x1f;
// 									db = dTemp & 0x1f;
// 									sg = (sTemp >> 5) & 0x3f;
// 									dg = (dTemp >> 5) & 0x3f;
// 									sr = (sTemp >> 11) & 0x1f;
// 									dr = (dTemp >> 11) & 0x1f;

// 									// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 									// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 									// 	((intensity * (dr - sr) >> 8) + sr) << 11);

// 								}
// 							}
// 							else
// 							{
// 								for(int i = 0; i < thisStep ; ++i)
// 								{
// 									sTemp = *compressedData_ptr++;
// 									dTemp = *screen_ptr;
// 									sb = sTemp & 0x1f;
// 									db = dTemp & 0x1f;
// 									sg = (sTemp >> 5) & 0x1f;
// 									dg = (dTemp >> 5) & 0x1f;
// 									sr = (sTemp >> 10) & 0x1f;
// 									dr = (dTemp >> 10) & 0x1f;

// 									// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 									// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 									// 	((intensity * (dr - sr) >> 8) + sr) << 10);

// 								}
// 							}
// 							// don't worry about moving past the overrun
// 							// in the data file
// 							// since thisStep = 0 will take care of it
// 							thisStep = 0;
// 						}
// 					}// end if colour
// 					else
// 					{
// 						// calculate which pixel in this line
// 						// to start from
// 						//	thisStep += count;
// 						// if we are not at our starting point yet
// 						if(count <= thisStep)
// 						{
// 							// skip past the transparent pixels on the screen
// 							// part of the bitmap
// 							thisStep-=count;
// 							screen_ptr+=count;
// 							tag = *compressedData_ptr++;
// 							continue;
// 						}
// 						else // pixel count has over run
// 						{
// 							screen_ptr+=thisStep;
// 							// don't worry about moving past the overrun
// 							// in the data
// 							// since thisStep = 0 will take care of it
// 							thisStep = 0;
// 						}
// 					}// end else black
					
// 					if(thisStep == 0)
// 					{
// 						// no more to draw
// 						//move the data ptr on to the end of this line
// 						//
// 						if(i+1 < drawHeight )
// 						{
// 							//!!!!!!!!!
// 							compressedData_ptr = (uint16*)bitmap->GetScanLine(i+1);

// 							if(!compressedData_ptr)
// 								return;
							
// 							compressedData_ptr--;
// 						}
// 						else
// 						{
// 							// force the tag to zero so that everything stops
// 							tag = 0;
// 							continue;
// 						}
						
						
// 					}// end this step over
					
// 					tag = *compressedData_ptr++;
// 				}// end while tag
// 				screen_ptr+=screen_step;
// 			}// bitmap height
// 		//	OutputDebugString("end r b not left\n");
// 		}// end if right/bottom
// 		else
// 		{
// 			//	OutputDebugString("start left\n");
// 			// left clip
// 			uint32 nextline = topClipped;
// 			uint32 leftSkip = 0;
// 			for (;bitmapHeight--;)
// 			{
// 				nextline++;
// 				thisStep = 0;
// 				pixelsDrawnThisLine =0;
// 				leftSkip = 0;

// 				// find out what type of pixel we have
// 				tag = *compressedData_ptr++;
// 				while(tag)
// 				{
// 					// find the number of colours to plot
// 					count = tag >>1;
// 					// calculate which pixel in this line
// 					// to start from
// 					thisStep += count;
					
// 					if(tag & 0x01) // colour pixel
// 					{
// 						// if we are not at our starting point yet
// 						if(thisStep <= leftClipped)
// 						{
// 							// move the data on to the next tag
// 							compressedData_ptr+=count;
// 							leftSkip += count;
// 						}
// 						else
// 						{

// 							// find out how many pixels we are over and
// 							// draw them to the screen
// 							uint32 over = thisStep - leftClipped;
							
// 							// draw the amount we are over

// 							thisStep-=over;
// 							if(leftSkip < leftClipped)
// 							{
// 								compressedData_ptr+= leftClipped - leftSkip;
// 								leftSkip = leftClipped;
// 							}

// 							// if we are still supposed to be drawing pixels
// 							if(over + pixelsDrawnThisLine <= bitmapWidth )
// 							{		
// 								bytes = over << 1;

// 								if(myPixelFormat == RGB_565)
// 								{

// 									for(int i = 0; i < over ; ++i)
// 									{
// 										sTemp = *compressedData_ptr++;
// 										dTemp = *screen_ptr;
// 										sb = sTemp & 0x1f;
// 										db = dTemp & 0x1f;
// 										sg = (sTemp >> 5) & 0x3f;
// 										dg = (dTemp >> 5) & 0x3f;
// 										sr = (sTemp >> 11) & 0x1f;
// 										dr = (dTemp >> 11) & 0x1f;

// 										// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 										// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 										// 	((intensity * (dr - sr) >> 8) + sr) << 11);

// 									}
// 								}
// 								else
// 								{
// 									for(int i = 0; i < over ; ++i)
// 									{
// 										sTemp = *compressedData_ptr++;
// 										dTemp = *screen_ptr;
// 										sb = sTemp & 0x1f;
// 										db = dTemp & 0x1f;
// 										sg = (sTemp >> 5) & 0x1f;
// 										dg = (dTemp >> 5) & 0x1f;
// 										sr = (sTemp >> 10) & 0x1f;
// 										dr = (dTemp >> 10) & 0x1f;

// 										// *screen_ptr++ = (uint16)((intensity * (db - sb) >> 8) + sb |
// 										// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 										// 	((intensity * (dr - sr) >> 8) + sr) << 10);
// 									}

// 								}
// 								pixelsDrawnThisLine += over;
// 							}
// 							else
// 							{
// 								// too many pixels in this run
// 								// any portion to draw?
// 								uint32 therest = bitmapWidth - pixelsDrawnThisLine;
// 								if(therest)
// 								{
// 									bytes = therest << 1;
						
// 									pixelsDrawnThisLine += therest;

// 								if(myPixelFormat == RGB_565)
// 								{
// 									for(int i = 0; i < therest ; ++i)
// 									{
// 										sTemp = *compressedData_ptr++;
// 										dTemp = *screen_ptr;
// 										sb = sTemp & 0x1f;
// 										db = dTemp & 0x1f;
// 										sg = (sTemp >> 5) & 0x3f;
// 										dg = (dTemp >> 5) & 0x3f;
// 										sr = (sTemp >> 11) & 0x1f;
// 										dr = (dTemp >> 11) & 0x1f;

// 										// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 										// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 										// 	((intensity * (dr - sr) >> 8) + sr) << 11);

// 									}
// 								}
// 								else
// 								{
// 									for(int i = 0; i < therest ; ++i)
// 									{
// 										sTemp = *compressedData_ptr++;
// 										dTemp = *screen_ptr;
// 										sb = sTemp & 0x1f;
// 										db = dTemp & 0x1f;
// 										sg = (sTemp >> 5) & 0x1f;
// 										dg = (dTemp >> 5) & 0x1f;
// 										sr = (sTemp >> 10) & 0x1f;
// 										dr = (dTemp >> 10) & 0x1f;

// 										// *screen_ptr++ = (uint32)((intensity * (db - sb) >> 8) + sb |
// 										// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
// 										// 	((intensity * (dr - sr) >> 8) + sr) << 10);
// 									}
// 								}

// 									// now skip to the end of the bitmap data
// 									compressedData_ptr = (uint16*)bitmap->GetScanLine(nextline);
// 									if(!compressedData_ptr)
// 									return;
							
// 									compressedData_ptr--;

// 								}

// 							}

// 						}
// 					}// end if colour
// 					else
// 					{// transparent
// 						// calculate which pixel in this line
// 						// to start from
// 						//	thisStep += count;
// 						// if we are not at our starting point yet
// 						if(thisStep <= leftClipped)
// 						{
// 							// do nothing for we don't need to draw this
// 							// part of the bitmap
// 							leftSkip += count;
// 							//	continue;
// 						}
// 						else // thisStep is larger than data_step
// 						{
// 							// find out how many pixels we are over and
// 							// draw them to the screen
// 							uint32 over = thisStep - leftClipped;
							
// 							// draw the amount we are over

// 							thisStep-=over;

// 							// find out how many pixels we are over and
// 							// draw them to the screen
// 							// if we are still supposed to be drawing pixels
// 							if(leftSkip < leftClipped)
// 							{
// 								leftSkip = leftClipped;
// 							}

// 							if(over + pixelsDrawnThisLine <= bitmapWidth )
// 							{		
// 								// move the screen ptr on by the required amount of steps
// 								// because these are transparent
// 								screen_ptr += over;
// 								//pretend that we didn't go over so that
// 								// we can draw to the end of the line
								
// 								pixelsDrawnThisLine += over;
// 							}
// 							else
// 							{
// 								// too many pixels in this run
// 								// any portion to draw?
// 								uint32 therest = bitmapWidth - pixelsDrawnThisLine;
// 								if(therest)
// 								{
// 								screen_ptr+=therest;

// 								pixelsDrawnThisLine+= therest;
// 								// finally skip past the right clipped area
// 								compressedData_ptr = (uint16*)bitmap->GetScanLine(nextline);
// 								if(!compressedData_ptr)
// 								return;
							
// 								compressedData_ptr--;

// 								}
// 							}
// 						}
						
// 					}// end colour check
// 					tag = *compressedData_ptr++;
// 				}// end while tag
// 				screen_ptr+=screen_step;
// 			}// bitmap height
// 			//	OutputDebugString("end left\n");
// 		}// end left clip
// 	}// end else clipping required
}

// Nasm declarations (the variables are
// defined in Bitmap.cpp, the functions 
// in Plot.asm).  For now we use these
// global variables even in the non-asm
// version - hopefully won't slow it down!
extern int32 bitmapHeight;
extern int32 bitmapWidth;
extern uint16* data_ptr;
extern uint32* screen_ptr;
extern uint32 data_step;
extern uint32 screen_step;
extern int dwordWidth;
extern uint16* compressedData_ptr;

void DisplayEngine::DrawAlphaSprite(Position& position,Bitmap& bitmap, int intensity)
{
	// work out how much to increase the data and sreen pointers
	// on when drawing

	return;

	// if(!GetDrawingParameters(position,
	// 					&bitmap,
	// 					data_step,
	// 					data_ptr,
	// 					screen_step,
	// 					screen_ptr,
	// 					bitmapWidth,
	// 					bitmapHeight))
	// 					return;

	// // draw taking account of transparent pixels and
	// // alpha value
	// uint16 sr,sg,sb,dr,dg,db =0;
	// uint16 sTemp;
	// uint16 dTemp;
	// uint16 pixel = 0;
	// for (;bitmapHeight--;)
	// {
	// 	for (int32 k=bitmapWidth;k--;)
	// 	{
	// 		sTemp = *data_ptr++;
	// 		if (sTemp)
	// 		{
	// 			dTemp = *screen_ptr;
	// 			if (myPixelFormat == RGB_565)
	// 			{
	// 				sb = sTemp & 0x1f;
	// 				db = dTemp & 0x1f;
	// 				sg = (sTemp >> 5) & 0x3f;
	// 				dg = (dTemp >> 5) & 0x3f;
	// 				sr = (sTemp >> 11) & 0x1f;
	// 				dr = (dTemp >> 11) & 0x1f;

	// 				// *screen_ptr = (uint32)((intensity * (db - sb) >> 8) + sb |
	// 				// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
	// 				// 	((intensity * (dr - sr) >> 8) + sr) << 11);
	// 			}
	// 			else
	// 			{
	// 				sb = sTemp & 0x1f;
	// 				db = dTemp & 0x1f;
	// 				sg = (sTemp >> 5) & 0x1f;
	// 				dg = (dTemp >> 5) & 0x1f;
	// 				sr = (sTemp >> 10) & 0x1f;
	// 				dr = (dTemp >> 10) & 0x1f;
					
	// 				// *screen_ptr = (uint32)((intensity * (db - sb) >> 8) + sb |
	// 				// 	((intensity * (dg - sg) >> 8) + sg) << 5 |
	// 				// 	((intensity * (dr - sr) >> 8) + sr) << 10);
	// 			}
	// 		}
	// 		screen_ptr++;
	// 	}
	// 	data_ptr+=data_step;
	// 	screen_ptr+=screen_step;
	// }
}

