// --------------------------------------------------------------------------
// Filename:	Bitmap.cpp
// Class:		Bitmap
// Purpose:		Bitmaps hold their width height and data
//				
//				
//				
//
// Description: Bitmaps can be either compressed or normal.  If they are
//				compressed then they have pointers to the first pixel
//				in every line.  
//
//				Bitmaps either create their own data or point to 
//				their data in a memory mapped file.
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added LoadFromS16
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Bitmap.h"
#include	"DisplayEngine.h"
#include	"MemoryMappedFile.h"
#include	"../File.h"


#ifndef C2E_SDL
// why is this included here?
// All the inlines in the .h file are members of DisplayEngine!
// Addendum: It really does seem to need to be here
// for things to link.  Moving to DisplayEngine just doesn't cut it
#include    "DisplayEnginePlotFunctions.h"
#endif

// Externs for Nasm
int32 bitmapWidth;
int32 bitmapHeight;
uint16* data_ptr;
uint32* screen_ptr;
uint32 data_step;
uint32 screen_step;
int dwordWidth;
uint16* compressedData_ptr;


Bitmap::Bitmap()
:myWidth(0),
myHeight(0),
myDataFlag(0),
myOffset(0),
myData(0),
myXOffset(0),
myYOffset(0),
myPosition(0,0),
myClippedWidth(0),
myClippedHeight(0)
{
}

Bitmap::~Bitmap()
	{
		// only delete the bits if you created them
		if(myDataFlag)
		{
		if (myData) delete [] myData;
		myData = NULL;
		}
	}





// ----------------------------------------------------------------------
// Method:      LoadFromS16 
// Arguments:   file - file to read the data from.  File must be in the
//					   correct position!!!
// Returns:     None
//
// Description: Loads bitmap from given file and copies the bits
//				
// ----------------------------------------------------------------------
void Bitmap::LoadFromS16(File& file)
{
	myGalleryName = file.GetName();
	myDataFlag = true;

	const int bitsPerPixel = 2;
	myData=new uint16[myWidth*myHeight];

	ASSERT(myData);
	if(myData)
	{

	file.Read(myData,myWidth * myHeight * bitsPerPixel);
	}
	// for testing purposes
	myXOffset=0;
	myYOffset=0;
}


// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   file - file to save my data to
//			
// Returns:     None
//
// Description: Saves pixel data to the given file
//			
// ----------------------------------------------------------------------
void Bitmap::Save(File& file)
{
	file.Write(myData,myWidth*myHeight);
}

// ----------------------------------------------------------------------
// Method:      SaveHeader 
// Arguments:   data - raw memory to write to
//			
// Returns:     None
//
// Description: Saves S16 header data to the given memory location
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveHeader(File& file)
{
	uint32 bytesWritten = 0;
	
	file.Write(&myOffset,sizeof(myOffset));
	bytesWritten += sizeof(uint32);

	file.Write(&myWidth,sizeof(uint16));
	bytesWritten += sizeof(uint16);

	file.Write(&myHeight,sizeof(uint16));
	bytesWritten += sizeof(uint16);

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveHeader 
// Arguments:   file - memory mapped to save my data to
//			
// Returns:     None
//
// Description: Saves S16 header data to the given file
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveHeader(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	
	file.WriteUINT32(myOffset);
	bytesWritten += sizeof(uint32);

	file.WriteUINT16(myWidth);
	bytesWritten += sizeof(uint16);

	file.WriteUINT16(myHeight);
	bytesWritten += sizeof(uint16);

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveData
// Arguments:   data - memory to save my data to
//			
// Returns:     None
//
// Description: Saves pixel data to the given memory location
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveData(File& file)
{
	uint32 bytes = myWidth*myHeight * sizeof(uint16);
	file.Write(myData,bytes);
	return bytes;
}
// ----------------------------------------------------------------------
// Method:      SaveData 
// Arguments:   file - memory mapped to save my data to
//			
// Returns:     None
//
// Description: Saves S16 pixel data to the given file
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveData(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	uint16* pixels = myData;

	for(uint32 i = 0; i < myWidth*myHeight;i++)
	{
		file.WriteUINT16(*pixels);
		bytesWritten+=sizeof(uint16);
		pixels++;
	}
	
	return bytesWritten;
}

bool Bitmap::UpdateBoundIfLarger(RECT& rect)
{
	uint32 rectWidth = rect.right - rect.left;
	uint32 rectHeight = rect.bottom - rect.top;

	bool updated = false;
	if(myWidth > rectWidth)
	{
		rect.right = myWidth;
		updated = true;
	}

	if(myHeight > rectHeight)
	{
		rect.bottom = myHeight;
		updated = true;
	}
	return updated;
}

void Bitmap::Draw()
{
	DisplayEngine::theRenderer().DrawBitmap(myPosition,*this);
}

void Bitmap::DrawWholeBitmapRegardless()
{
	DisplayEngine::theRenderer().DrawWholeBitmapRegardless(myPosition,*this);
}

void Bitmap::Recolour(const uint16*& tintTable)
{
	// Replace each pixel with its tinted equivalent
	uint16* ptr = myData;
	for (int i=myWidth*myHeight;i--;)
	{
		*ptr ++ = tintTable[ *ptr ];
	}
}


// ----------------------------------------------------------------------
// Method:      IsPixelTransparent 
// Arguments:   imageIndex - the sprite image to test
//								if this is -1 then use the current image
//				x - x coordinate of the pixel to test
//				y - y coordinate of the pixel to test
//
// Returns:     true if the pixel at the given coordinates is transparent
//				false otherwise. Note no checks for out of bounds
//			
// ----------------------------------------------------------------------
bool Bitmap::IsPixelTransparent(Position pixelPos)
{
	if( pixelPos.GetY() < 0 || pixelPos.GetX() < 0 ||
		pixelPos.GetY() >= myHeight || pixelPos.GetX() >= myWidth )
		return true;

	uint32 actualPixel = (myWidth*pixelPos.GetY()) + pixelPos.GetX(); 

	ASSERT(actualPixel < myWidth*myHeight);

	return (myData[actualPixel] == 0 ? true : false );
}

void Bitmap::SetData(MemoryMappedFile& file)
{
	myDataFlag = false;

	// point to your dfata in the gievn file
	uint8* data;
	file.ReadUINT8Ptr(data);
	myData = (uint16*)data;
	file.Seek((myWidth * myHeight * BitsPerPixel)-1,File::Current);
}


void Bitmap::Convert(uint32 from, uint32 to)
{
	uint16* pixels = myData;
	uint16 	s = 0;
	uint16 r=0;
	uint16 g=0;
	uint16 b=0;

	switch(from)
	{
		case RGB_555:
		{
			switch(to)
			{
				case RGB_565:
				{
					for(uint32 i = 0; i<myWidth*myHeight;i++)
					{
						P555_TO_RGB(*pixels, r, g, b);
						RGB_TO_565(r, g, b, *pixels);

						// Preserve non-transparency.
						if (*pixels == 0x0000 && (r != 0x00 || g != 0x00 || b != 0x00))
							*pixels = 0x0821; // Dark grey.

						pixels++;
					}
					break;
				}
		
				default:
				break;
			}
	
			break;
		}
		case RGB_565:
		{
			switch(to)
			{

				case RGB_555:
				{
					for(uint32 i = 0; i<myWidth*myHeight;i++)
					{
						P565_TO_RGB(*pixels, r, g, b);
						RGB_TO_555(r, g, b, *pixels);

						// Preserve non-transparency.
						if (*pixels == 0x0000 && (r != 0x00 || g != 0x00 || b != 0x00))
							*pixels = 0x0421; // Dark grey.

						pixels++;
					}
				}
				default:
					break;
			}
		}
		default:
			break;
	}
}


void Bitmap::CreateBlankCanvas()
{
	if(myData)
		delete [] myData;

	myData = new uint16[myWidth*myHeight];

	ZeroMemory(myData,myWidth*myHeight*2);
}

void Bitmap::ResetCanvas()
{
	if(myData)
		ZeroMemory(myData,myWidth*myHeight*2);
}

void Bitmap::SetData(uint8*& dataPtr)
{

	myDataFlag = false;

	// just point to your data in the file
	myData = (uint16*)dataPtr;

	//move the dataPtr on to the next bitmaps details
	dataPtr += myWidth * myHeight * BitsPerPixel;
}


void Bitmap::InitHeader(MemoryMappedFile& file)
{
	myOffset = file.ReadUINT32();

	myWidth = file.ReadUINT16();

	myHeight = file.ReadUINT16();

}

/*
bool Bitmap::Write(CreaturesArchive &archive) const
{
	archive << myGalleryName;
	archive << myWidth;
	archive << myHeight;
	archive << myDataFlag;

	archive << myOffset;
	archive << myXOffset;
	archive << myYOffset;

	archive << myPosition;
	archive.Write( (void *)myData, myWidth * myHeight * 2 );
	return true;
}
*/

bool Bitmap::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		archive >> myGalleryName;
		archive >> myWidth;
		archive >> myHeight;
		archive >> myDataFlag;

		archive >> myOffset;
		archive >> myXOffset;
		archive >> myYOffset;

		archive >> myPosition;
		myData = new uint16[myWidth * myHeight];
		archive.Read( (void *)myData, myWidth * myHeight * 2 );
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


void Bitmap::CreatePixelData(uint16* data)
{
	myData = new uint16[myWidth * myHeight];
	memcpy(myData,data,myWidth*myHeight*sizeof(uint16));
	myDataFlag = true;
}

void Bitmap::DecompressC16(File& file)
{
#ifdef _DEBUG
	// see whether we are dealing with a compressed file by looking
			// at the file extension
	std::string galleryName(file.GetName());
	int x = galleryName.find_last_of(".");
	
	std::string ext = galleryName.substr(x, 3);

	ASSERT(ext[1] == 'C' || ext[1] == 'c');	
#endif

	myDataFlag = true;
	uint16 tag =0;

	const int bitsPerPixel = 2;

	delete myData;

	// this will be far too much space but at least it is
	// guaranteed to cover the whole set of data
	myData = new uint16[myWidth*myHeight];

	if(myData)
	{
		// read int all the
		uint16* pixels = (uint16*)myData;

		uint16 data = 255;
		uint32 count=0;
		file.Read(&tag,2);
		count = tag >> 1;

		for(uint32 i = 0; i < myHeight; i++)
		{
			while(tag)
			{
		
				if(tag & 0x01)
				{
					count = tag >> 1;
					for(int i = 0; i < count ; i++)
					{
						file.Read(&tag,2);
						*pixels++ = tag;
					}
				}
				else
				{
					count = tag >> 1;
					for(int i = 0; i < count ; i++)
					{
						*pixels++ = 0;
					}
				}

			file.Read(&tag,2);
			}
			// read the last zero for end of file
			file.Read(&tag,2);

		}

		
		
	}
}

void Bitmap::Scale(float scaleBy)
{ 
	int32 newWidth = myWidth;
	int32 newHeight = myHeight;

	// if we are shrinking the bitmap...
	int32 hstep =0;

	int32 vstep =0;

	uint16* newData =NULL;
	uint16* oldpixels = myData;
	uint16* newpixels = NULL;

	if(scaleBy < 1.0f)
	{
		newWidth = int32(float(myWidth) * float(scaleBy));
		newHeight = int32(float(myHeight) * float(scaleBy));
		
		hstep = int32(float(myWidth)/float(newWidth));

		vstep = int32(float(myHeight)/float(newHeight));

		// the step must be an integer so we will have
		// some percentages coming out at the same proportions
		newWidth = myWidth / hstep;
		newHeight = myHeight / vstep;


		newData = new uint16[newWidth * newHeight];

	
		if(newData)
		{
			newpixels = newData;


			int counth = 0;
			int countv = 0;

			for(int i = 0; i < myHeight;++i)
			{

				if(i % vstep == 0 && countv < newHeight)
				{
					counth=0;
					for(int j = 0; j < myWidth; ++j )
					{
						if(j % hstep == 0 && counth < newWidth)
						{
							counth++;
							*newpixels++ = *oldpixels++;
						}
						else
						{
							oldpixels++;
						}
					}

				countv++;
				}
				else
				{
					oldpixels+=myWidth;
				}
				
				
			}
		}

	}
	else
	{
		oldpixels = myData;

		newWidth = int32(float(myWidth) * (scaleBy));
		newHeight = int32(float(myHeight) * (scaleBy));

		hstep = int32(float(newWidth)/float(myWidth));

		vstep = int32(float(newHeight)/float(myHeight));

		// make sure that the new dimensions fit properly
		newWidth = hstep * myWidth;
		newHeight = vstep * myHeight;

		newData = new uint16[newWidth * newHeight];

		if(newData)
		{
			newpixels = newData;

			for(int i = 0; i < myHeight; ++i)
			{
				// do as many duplicates as we need
				for(int s = 0; s < vstep; ++s)
				{
					for(int j = 0; j < myWidth; ++j )
					{
						uint16 value = *oldpixels++;
						for(int t = 0; t < hstep; ++t)
						{
							*newpixels++ = value; 
						}
						
					}

					// go back one line
					if(s != vstep-1)
						oldpixels-=myWidth;
					
				}
			}
		}
	}


	delete [] myData;
	myData = newData; 

	myWidth = newWidth;
	myHeight = newHeight;

}

void Bitmap::Copy(Bitmap& bitmap)
{
	myOffset = bitmap.GetOffset();
	myWidth = bitmap.GetWidth();
	myHeight = bitmap.GetHeight();
	myDataFlag = true;
	myGalleryName = bitmap.GetName();

	myPosition = bitmap.GetPosition();
	myClippedWidth = bitmap.GetClippedWidth();
	myClippedHeight = bitmap.GetClippedHeight();

	delete [] myData;
	myData = new uint16[myWidth * myHeight];

	if(myData)
	{
		memcpy(myData,bitmap.GetData(),myWidth*myHeight*sizeof(uint16));
	}
}

