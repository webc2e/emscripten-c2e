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
//				We have a scanline value for each loine except the top one
//				which is covered by the original S16 format Header member:
//				myOffset
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added LoadFromS16
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////
#ifndef		BITMAP_H
#define		BITMAP_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"../../common/C2eTypes.h"
#include	"Position.h"
#include	<vector>



const int BitsPerPixel = 2;

class MemoryMappedFile;
class File;

class Bitmap 
{
public:
// ----------------------------------------------------------------------
// Method:      Constructors 
// Arguments:   None
//						
// ----------------------------------------------------------------------
	Bitmap();

	Bitmap(int32 width,int32 height)
	{
		myWidth=width;
		myHeight=height;
		myDataFlag = true;
		myData=new uint16[myWidth*myHeight];
	}
// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   none
//
// Description: Deletes the created bitmaps and unmaps and closes any
//				memory mapped files
//						
// ----------------------------------------------------------------------
	virtual ~Bitmap();
////////////////////////////////////////////////////////////////////////////
// Load Bitmap Data from specified file formats...
///////////////////////////////////////////////////////////////////////////
	void LoadFromS16(File& name);

////////////////////////////////////////////////////////////////////////////
// Get and Set Methods...
////////////////////////////////////////////////////////////////////////////

	int32& GetXOffset()
	{
		return myXOffset;
	}

	int32& GetYOffset()
	{
		return myYOffset;
	}

	int32 GetWidth() const
	{
		return myWidth;
	}

	int32 GetHeight() const
	{
		return myHeight;
	}

	void SetWidth(int32 width)
	{
		myWidth = width;
	}

	void SetHeight(int32 height)
	{
		myHeight = height;
	}

// ----------------------------------------------------------------------
// Method:      Recolour 
// Arguments:   a complete lookup table assigning a
//				replacement to each colour.
//			
// Returns:     None
//
// Description: Recolours the bitmap.
//			
// ----------------------------------------------------------------------
	virtual void Recolour(const uint16*& tintTable);

// ----------------------------------------------------------------------
// Method:      SetData 
// Arguments:   dataPtr - pointer to the start of the bitmaps data in the
//				file.
//			
// Returns:     None
//
// Description: Marks the point in the sprite file where this bitmaps
//				data begins.
//			
// ----------------------------------------------------------------------
	virtual void SetData(uint8*& dataPtr);

	virtual void SetData(MemoryMappedFile& file);


	void SetData(uint16* dataPtr)
	{
		// just point to your data in the file
		myData = dataPtr;
	}

	uint16* GetData()
	{
		return myData;
	}

	void SetOffset(uint8*& dataPtr)
	{
		myOffset = GetUINT32At(dataPtr);
		dataPtr +=4;
	}

	void SetOffset(uint32 offset)
	{
		myOffset = offset;
	}

	uint32 GetOffset() const
	{
		return	myOffset;
	}

	bool UpdateBoundIfLarger(RECT& rect);
// ----------------------------------------------------------------------
// Method:      SetWidthHeight 
// Arguments:   dataPtr - start of the width and height data
//                         in the file
//			
// Returns:     None
//
// Description: Reads the width and height from the file pointer
//				
//			
// ----------------------------------------------------------------------
virtual bool Read(CreaturesArchive &archive);



	void SetPosition(const Position pos)
	{
		myPosition = pos;
	}

	Position  GetPosition() const
	{
		return myPosition;
	}

	void SetName(const std::string name)
	{
		myGalleryName = name;
	}

	std::string GetName()
	{
		return myGalleryName;
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
	virtual void Save(File& file);

	virtual void Convert(uint32 from,
							uint32 to);

	void Scale(float scaleBy);

	virtual uint32 SaveData(File& file);
	uint32 SaveHeader(File& file);

	uint32 SaveHeader(MemoryMappedFile& file);
	virtual uint32 SaveData(MemoryMappedFile& file);

	virtual void Draw();
	virtual void DrawWholeBitmapRegardless();

	virtual bool IsPixelTransparent(Position pixelPos);

	void CreateBlankCanvas();
	void ResetCanvas();

	virtual void InitHeader(MemoryMappedFile& file);
	void DecompressC16(File& file);

	virtual void Copy(Bitmap & bitmap);

	void CreatePixelData(uint16* data);

	void SetClippedDimensions(int32& w, int32& h)
	{myClippedWidth = w;
	myClippedHeight = h;}

	int32 GetClippedWidth(){return myClippedWidth;}
	int32 GetClippedHeight(){return myClippedHeight;}


protected:
	// Copy constructor and assignment operator
	// Declared but not implemented
	Bitmap (const Bitmap&);
	Bitmap& operator= (const Bitmap&);

	std::string myGalleryName;
	int32	myWidth;
	int32	myHeight;
	int32	myClippedWidth;
	int32	myClippedHeight;

	bool	myDataFlag;

	// pointer to the place in the sprite file that holds
	// your width and height
	uint32 myOffset;
	uint16*	myData;
	int32	myXOffset;
	int32	myYOffset;

	Position myPosition;
};

#endif		// BITMAP_H

