
// --------------------------------------------------------------------------
// Filename:	Gallery.cpp
// Class:		Gallery
// Purpose:		This class holds a series of bitmaps for an entity or a
//				background.  			
//			
//				
//
// Description: In the case of entities the bitmaps are a series
//				of animations.  They can either be compressed or uncompressed.
//				By default they share pixel information with other galleries of
//				the same name.  Memeory mapped files are used for these galleries.
//				Cloned galleries need to hold all their bitmap data locally in 
//				an uncompressed state so that they can alter pixel data.  
//				These do not use memory mapped files.  Also we must be provided
//				with the base image and the number of images that belong to this
//				particular gallery since there may be many sets of related sprites
//				in a sprite file.
//				
//				In the case of backgrounds each bitmap is a tile.
//
// History:
// -------  
// 23Feb2000	Alima			Created
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Gallery.h"
#include "../../common/C2eTypes.h"
#include <stdio.h>
#include "ErrorMessageHandler.h"
#ifdef _WIN32
#include "../RegistryHandler.h"
#endif // _WIN32
#include	"../App.h"
#include	"Bitmap.h"
#include	"../File.h"
#include "../../common/StringFuncs.h"
#include	"CompressedBitmap.h"
#include "ClonedGallery.h"
#include "DisplayEngine.h"
#include "Sprite.h"
#include "SharedGallery.h"
#include "DisplayErrorConstants.h"


const std::string S16 = ".s16";
const std::string C16 = ".c16";
const std::string BLK = ".blk";

Gallery::Gallery()
:myBitmaps(0),
myCompressedBitmaps(0),
myNumberOfBitmaps(0),
myBaseImage(0),
myAmIClonedFlag(false),
myFileFormat(IAmUncompressed),
myFileSpec(0),
myReferenceCount(1),
myPixelFormat(0)
{

}

// build yourself from an existing cloned gallery
Gallery::Gallery(ClonedGallery& gallery)
:myBitmaps(0),
myCompressedBitmaps(0)
{
	myReferenceCount = 1;
	myAmIClonedFlag = gallery.AreYouCloned();
	myNumberOfBitmaps = gallery.GetCount();
	myPixelFormat = gallery.GetPixelFormat();
	myFileFormat = gallery.GetFileFormat();
	myName = gallery.GetName();
	myBaseImage = gallery.ClonedAbsoluteBase();
	myFileSpec = gallery.GetFileSpec();

	_ASSERT(myAmIClonedFlag == true);

	myBitmaps = new Bitmap[myNumberOfBitmaps];

	for(int32 i= 0; i < myNumberOfBitmaps; ++i )
	{
		myBitmaps[i].Copy(*gallery.GetBitmap(i));
	}

}

Gallery::Gallery(std::string const &name,
								uint32 baseimage,
								uint32 numImages,
								bool clone,
								uint32 defaultBitmapWidth,
								uint32 defaultBitmapHeight,
								bool writeAccess)
									:myBitmaps(0),
									myCompressedBitmaps(0),
									myNumberOfBitmaps(numImages),
									myBaseImage(baseimage),
									myAmIClonedFlag(clone),
									myFileFormat(IAmUncompressed),
									myName(name),
									myFileSpec(0),
									myReferenceCount(1),
									myPixelFormat(0)
{
	if(myName.empty())
	{
			std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
								(int)sidEmptyGalleryString,
								std::string("DisplayEngine::CreateGallery"));
				throw(GalleryException(string,__LINE__));
	}

	// First decide what type of file this if from the extension
	// if there is a mix up it will be detected later.
	std::string ext;

	int x = myName.find_last_of(".");

	ext =  myName.substr( x, myName.length() );
	
	// lower case for testing the extension
	LowerCase(ext);

	if(ext == C16)
	{
		myFileFormat = IAmCompressed;
	}
	else if(ext == S16)
	{
		myFileFormat = IAmUncompressed;
	}
	else if(ext == BLK)
	{
		myFileFormat = IAmTiled;
	}
	else
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
			(int)sidGalleryUnknownType,
			std::string("SharedGallery::ConvertGallery"),
			myName.c_str());

			throw(GalleryException(string,__LINE__));
	}
	
	if(!InitialiseBitmaps(writeAccess))
	{
		// "The following sprite gallery was not created %1"
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidNoGalleryCreated,
									std::string("Gallery::Gallery"),
									myName.c_str());


		throw(GalleryException(string,__LINE__));
	}
}


Gallery::Gallery(std::string const &name,
				MemoryMappedFile::FileMapping memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap)
				 :myMemoryMappedFile( memoryFile, 
							highOffset,
							lowOffset,
							numBytesToMap),
					myBitmaps(0),
					myCompressedBitmaps(0),
					myNumberOfBitmaps(0),
					myBaseImage(0),
					myAmIClonedFlag(false),
					myFileFormat(IAmUncompressed),
					myFileSpec(0),
					myReferenceCount(1),
					myPixelFormat(0),
					myName(name)
{
	if(!InitialiseBitmaps())
	{
		// "The following sprite gallery was not created %1"
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidNoGalleryCreated,
									std::string("Gallery::Gallery"),
									myName.c_str());


		throw(GalleryException(string,__LINE__));
	}

}

void Gallery::SetUpGalleryShellForCloning(std::string path,
									uint32 baseImage,
									uint32 pixelFormat,
									uint32 numBitmaps)
{
	myName = path;
	myBaseImage = baseImage;
	myPixelFormat = pixelFormat;
	myNumberOfBitmaps = numBitmaps;

	myBitmaps = new Bitmap[myNumberOfBitmaps];

	myFileFormat = IAmUncompressed;
}


Gallery::~Gallery()
{
	Trash();
}

bool Gallery::InitialiseBitmaps(bool writeAccess)
{
	if(myAmIClonedFlag)
	{
		return InitialiseClonedBitmaps();
	}

	if(myFileFormat == IAmTiled)
	{
		return InitialiseTiledBitmaps(writeAccess);
	}

	// if we are not cloned then we are using memory mapped files
	if(!(myMemoryMappedFile.Valid() || myMemoryMappedFile.MappedToExistingFile()))
	{
		if (writeAccess)
			myMemoryMappedFile.Open(myName, GENERIC_READ | GENERIC_WRITE);
		else
			myMemoryMappedFile.Open(myName, GENERIC_READ);
	}

	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	myNumberOfBitmaps = myMemoryMappedFile.ReadUINT16();

	// this serves to check if the file is compressed
	if(myPixelFormat & C16_16BITFLAG)
	{
		return InitialiseCompressedBitmaps();
	}

	// we shouldn't be flitting between formats!
	ASSERT(myCompressedBitmaps == NULL);

	if(myBitmaps)
	{
		delete [] myBitmaps;
		myBitmaps = NULL;
	}

	//create the correct number of bitmaps
	myBitmaps = new Bitmap[myNumberOfBitmaps];

	if(!myBitmaps)
		return false;

	// ok we have the all clear
	uint32 i;
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		myBitmaps[i].SetName(myName);
		myBitmaps[i].SetOffset(myMemoryMappedFile.ReadUINT32());

		myBitmaps[i].SetWidth(myMemoryMappedFile.ReadUINT16());
		myBitmaps[i].SetHeight(myMemoryMappedFile.ReadUINT16());

	}

	//now point to your data in the sprite file
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		myBitmaps[i].SetData(myMemoryMappedFile);
	}

	myFileFormat = IAmUncompressed;
	return true;
}

// ----------------------------------------------------------------------
// Method:      InitialiseClonedBitmaps 
// Arguments:   None
//			
// Returns:     true if everything was fine, false otherwise
//
// Description: Creates a set of cloned bitmaps depending on how many 
//				are in the file and initialises them with their widths
//				and heights.  Each bitmap's will actually create its data
//				on the heap. 
//			
// ----------------------------------------------------------------------
bool Gallery::InitialiseClonedBitmaps()
{
	if (myBitmaps) delete [] myBitmaps;

	File file(myName,
			GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE);

	// First four bytes is a set of flags to say whther this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file
	file.Read(&myPixelFormat,4);

	file.SeekToBegin();

	if((myPixelFormat & C16_16BITFLAG))
	{
		return DecompressC16(file);
	}

	int x = myName.find_last_of(".");

	if(x == -1)
	{
		return false;
	}

	std::string ext = myName.substr(x, 3);

	// check for a preferred C16 version
	if(ext[1] == 'C' || ext[1] == 'c')	
	{
		return DecompressC16(file);
	}

	uint32 size = file.GetSize();

	ASSERT(size);
	
	if(size)
	{
		// First four bytes is a set of flags to say whther this is
		// a 565 or 555 sprite file. I am assuming this is a 565 file
		file.Read(&myPixelFormat,4);

		// Now the number of images in the file
		uint16 numImages =0;
		file.Read(&numImages,2);

		// but we really want to use our count as we were 
		// initialised
		if(myNumberOfBitmaps == 0)
		{
			myNumberOfBitmaps = numImages;
		}

		//create the correct number of bitmaps
		myBitmaps=new Bitmap[myNumberOfBitmaps];

		if(myBitmaps)
		{
			int16 bitmap_width;
			int16 bitmap_height;
			uint32 offset = 0;
			uint32 actualBitmap =0;

			// only read in data specific to you
			uint32 i;
			for(i = 0; i < numImages; i++)
			{
			
				// Set the offset
				file.Read(&offset,4);
				// Now read in the width and height as shorts
				file.Read(&bitmap_width,2);
				file.Read(&bitmap_height,2);

				if(i >= myBaseImage && i < myBaseImage + myNumberOfBitmaps)
				{	
					myBitmaps[actualBitmap].SetOffset(offset);
					myBitmaps[actualBitmap].SetWidth(bitmap_width);
					myBitmaps[actualBitmap].SetHeight(bitmap_height);
					actualBitmap++;
				}
			}

			//now read in each bit
			// actualBitmap =0;
			uint32 dest = DisplayEngine::theRenderer().GetMyPixelFormat();
			uint32 from = RGB_555;

			if( myPixelFormat & C16_FLAG_565 )
				from = RGB_565;

			for(i = 0; i < myNumberOfBitmaps; i++)
			{
			// look for your data in the file
				file.Seek(myBitmaps[i].GetOffset(),File::Start);
					
				myBitmaps[i].LoadFromS16(file);

				myBitmaps[i].Convert(from, dest);
				if( dest == RGB_555 ) // Clear the 565 bit
					myPixelFormat = myPixelFormat & ~1;
				else // Set the 565 bit
					myPixelFormat = myPixelFormat | 1;
			}

			file.Close();
			myFileFormat = IAmUncompressed;
			return true;
		}
	}
	
	//!!! if the file didn't exist it wouldn't get
	// this far

	return false;
}

// ----------------------------------------------------------------------
// Method:      InitialiseCompressedBitmaps 
// Arguments:   None
//			
// Returns:     true if everything was fine, false otherwise
//
// Description: Creates a set of compressed bitmaps depending on how many 
//				are in the file and initialises them with their widths
//				and heights.  Each bitmap's data is set to the start of the
//				the bitmaps data in the memory mapped file. 
//			
// ----------------------------------------------------------------------
bool Gallery::InitialiseCompressedBitmaps()
{
	delete [] myCompressedBitmaps;

	//create the correct number of bitmaps
	myCompressedBitmaps= new CompressedBitmap[myNumberOfBitmaps];

	
	uint32 pos =0;
	for(uint32 i = 0; i < myNumberOfBitmaps; i++)
	{

#ifdef _DEBUG
		// set the name and offset
		myCompressedBitmaps[i].SetName(myName);
#endif

		myCompressedBitmaps[i].InitHeader(myMemoryMappedFile);

		pos = myMemoryMappedFile.GetPosition();

		myCompressedBitmaps[i].SetData(myMemoryMappedFile);	

		myMemoryMappedFile.Seek(pos,File::Start);
	
	}
	myFileFormat = IAmCompressed;
	return true;
}

// ----------------------------------------------------------------------
// Method:      InitialiseTiledBitmaps 
// Arguments:   None
//			
// Returns:     None
//
// Description: Creates a set of bitmaps depending on how many are in the 
//				file and initialises them with their tile widths
//				and heights.  Each bitmaps data is set to the start of the
//				the bitmaps data in the file. 
//			
//			
// ----------------------------------------------------------------------
bool Gallery::InitialiseTiledBitmaps(bool writeAccess)
{
	if (writeAccess)
		myMemoryMappedFile.Open(myName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ);
	else
		myMemoryMappedFile.Open(myName,GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ);

	// skip to the number of images in the file
	// get my 16 bit pixel format flag 565 or 555
	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	// put here the tilewidth and tileheight of the background
	// offset from the world position
	myTileWidth =  myMemoryMappedFile.ReadUINT16();
	ASSERT(myTileWidth);


	myTileHeight =  myMemoryMappedFile.ReadUINT16();
	ASSERT(myTileHeight);


	myNumberOfBitmaps = myMemoryMappedFile.ReadUINT16();
	_ASSERT(myNumberOfBitmaps);

	//create the correct number of bitmaps
	myBitmaps=new Bitmap[myNumberOfBitmaps];

	uint32 i;	
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
#ifdef _DEBUG
		myBitmaps[i].SetName(myName);
#endif
		try {
		myBitmaps[i].InitHeader(myMemoryMappedFile);
		} catch (BasicException &e) {
			std::cout << "An error initing bitmap: " << myName << e.what() << std::endl;
		} catch (...) {
			std::cout << "An error initing bitmap: " << myName << std::endl;
		}
	}

	//now point to your data in the sprite file
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		myBitmaps[i].SetData(myMemoryMappedFile);
	}
	return true;

}

// ----------------------------------------------------------------------
// Method:      LoadFromC16 
// Arguments:   name - name of the bitmap file to load from
//			
// Returns:     true if the file loads OK false otherwise
//
// Description: Splits the file into the given amount of bitmaps.  Copies
//				the bits for each bitmap.
//			
//			
// ----------------------------------------------------------------------
bool Gallery::LoadFromC16(std::string const& fileName)
{

	if(myName == fileName)
		return true;

	myName = fileName;
	myNumberOfBitmaps = 0;
	myPixelFormat =0;
	myMemoryMappedFile.Open(fileName, GENERIC_READ);

	if (myCompressedBitmaps)
		delete [] myCompressedBitmaps;

	// get my 16 bit pixel format flag 565 or 555
	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	// for new 16 bit tag check the tag!  because this could be an old c16
	// using the 8 bit tag.
	if(!(myPixelFormat & C16_16BITFLAG))
	{
		// note that this is a developer message and should be removed
		// before release
		const std::string message(myName + "\n\nThis file is using the old compressed file format.\nYou need Dan's sprite builder!");
#ifdef _WIN32
		MessageBox(NULL, message.c_str(), "CompressedGallery::InitBitmaps", MB_APPLMODAL);
#else
		std::cout << message;
#endif
		return false;
	}

	myNumberOfBitmaps = myMemoryMappedFile.ReadUINT16();
//gtb!temp!	ASSERT((myNumberOfBitmaps==0) || (myNumberOfBitmaps>=192));

	ASSERT(myNumberOfBitmaps);

	if(myNumberOfBitmaps == 0)
	{

		ErrorMessageHandler::Show("gallery_error",
									1,
									std::string("CompressedGallery::CompressedGallery"),
									myName.c_str());
	}

	//create the correct number of bitmaps
	myCompressedBitmaps= new CompressedBitmap[myNumberOfBitmaps];
	uint32 pos =0;
	for(uint32 i = 0; i < myNumberOfBitmaps; i++)
	{
#ifdef _DEBUG
		// set the name and offset
		myCompressedBitmaps[i].SetName(myName);
#endif

		myCompressedBitmaps[i].InitHeader(myMemoryMappedFile);
		pos = myMemoryMappedFile.GetPosition();

		myCompressedBitmaps[i].LoadFromC16(myMemoryMappedFile);	
		myMemoryMappedFile.Seek(pos,File::Start);	
	}

	return true;

}


// ----------------------------------------------------------------------
// Method:      ReloadFromC16 
// Arguments:   name - name of the bitmap file to load from
//			
// Returns:     true if the file loads OK false otherwise
//
// Description: Splits the file into the given amount of bitmaps.  Copies
//				the bits for each bitmap in the given range.  This is useful
//				when wanting to retint a few sprites from the original colour
//			
//			
// ----------------------------------------------------------------------
bool Gallery::ReloadFromC16( uint32 range_start,
									uint32 range_end)
{

	myMemoryMappedFile.Reset();

	// get my 16 bit pixel format flag 565 or 555
	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	// for new 16 bit tag check the tag!  because this could be an old c16
	// using the 8 bit tag.
	if(!(myPixelFormat & C16_16BITFLAG))
	{
		// note that this is a developer message and should be removed
		// before release
		const std::string message(myName + "\n\nThis file is using the old compressed file format.\nYou need Dan's sprite builder!");
#ifdef _WIN32
		MessageBox(NULL, message.c_str(), "CompressedGallery::InitBitmaps", MB_APPLMODAL);
#else
		std::cout << message;
#endif
		return false;
	}

	myNumberOfBitmaps = myMemoryMappedFile.ReadUINT16();

	ASSERT(myNumberOfBitmaps);

	if(myNumberOfBitmaps == 0)
	{

		ErrorMessageHandler::Show("gallery_error",
									1,
									std::string("CompressedGallery::CompressedGallery"),
									myName.c_str());
	}


	uint32 pos =0;
	for(uint32 i = 0; i < myNumberOfBitmaps; i++)
	{
#ifdef _DEBUG
		// set the name and offset
		myCompressedBitmaps[i].SetName(myName);
#endif

		myCompressedBitmaps[i].InitHeader(myMemoryMappedFile);
		pos = myMemoryMappedFile.GetPosition();

		if(i >= range_start && i <= range_end )
			myCompressedBitmaps[i].LoadFromC16(myMemoryMappedFile);	
		else
			myCompressedBitmaps[i].DummyLoadFromC16(myMemoryMappedFile);	
		myMemoryMappedFile.Seek(pos,File::Start);	
	}

	return true;
}

// ----------------------------------------------------------------------
// Method:      DecompressC16 
// Arguments:   file - a compressed file to read the data from
//			
// Returns:     true if the file decompresses OK false otherwise
//
// Description: Splits the file into the given amount of bitmaps.  And 
//				decompresses the data.  Each bitmap will hold a pointer
//				to its data on the heap.
//						
// ----------------------------------------------------------------------
bool Gallery::DecompressC16(File& file)
{
	if (myBitmaps) delete [] myBitmaps;

	uint32 size = file.GetSize();
	
	if(size)
	{
		// First four bytes is a set of flags to say whther this is
		// a 565 or 555 sprite file. I am assuming this is a 565 file
		file.Read(&myPixelFormat,4);
		myPixelFormat = myPixelFormat & ~C16_16BITFLAG;
		// Now the number of images in the file

		//create the correct number of bitmaps
		uint16 numImages = 0;
		file.Read(&numImages,2);

		if( myNumberOfBitmaps == 0 )
		{
			myNumberOfBitmaps = numImages;
		}

		myBitmaps=new Bitmap[myNumberOfBitmaps];

		int16 bitmap_width;
		int16 bitmap_height;
		uint32 offset = 0;
		int actualBitmap = 0;
		uint32 i;
		for(i = 0; i < numImages; i++)
		{
			// read the offset

			file.Read(&offset,4);

			// Now read in the width and height as shorts
			file.Read(&bitmap_width,2);
			file.Read(&bitmap_height,2);

			if( i >= myBaseImage && i < myBaseImage + myNumberOfBitmaps )
			{
				myBitmaps[actualBitmap].SetOffset(offset);
				myBitmaps[actualBitmap].SetWidth(bitmap_width);
				myBitmaps[actualBitmap].SetHeight(bitmap_height);
				++actualBitmap;
			}

		// skip scan lines
				uint32 scanValue = 0;
			// do one less scan line because we have the first offset
			// value in myOffset in the header
			for(int h = bitmap_height-1; h--;)
			{
				file.Read(&scanValue,sizeof(scanValue));
			}


		}


	//now read in each bit
		uint32 dest = DisplayEngine::theRenderer().GetMyPixelFormat();
		uint32 from = RGB_555;
		if( myPixelFormat & C16_FLAG_565 )
			from = RGB_565;
		for(i = 0; i < myNumberOfBitmaps; i++)
		{
			file.Seek(myBitmaps[i].GetOffset(),File::Start);
			myBitmaps[i].DecompressC16(file);

			if (dest != from)
				myBitmaps[i].Convert(from, dest);
		}
		if( dest == RGB_555 ) // Clear the 565 bit
			myPixelFormat = myPixelFormat & ~1;
		else // Set the 565 bit
			myPixelFormat = myPixelFormat | 1;

		myFileFormat = IAmUncompressed;
		return true;
	}

	//!!! if the file didn't exist it wouldn't get
	// this far

	return false;
}


// ----------------------------------------------------------------------
// Method:      Recolour 
// Arguments:   tintable - a table of tint values
//			
// Returns:     none
//
// Description: recolours the pixel data according to the entires in the 
//				tint table.
//						
// ----------------------------------------------------------------------
void Gallery::Recolour(const uint16 tintTable[65536])
{
	if(myCompressedBitmaps)
	{
		for(int i = 0; i < myNumberOfBitmaps; i++)
		{
			myCompressedBitmaps[i].Recolour(tintTable);
		}
	}
	else if(myBitmaps)
	{
		for(int i = 0; i < myNumberOfBitmaps; i++)
		{
			myBitmaps[i].Recolour(tintTable);
		}
	}
}

// ----------------------------------------------------------------------
// Method:      Recolour 
// Arguments:   tintable - a table of tint values.
//				range_start
//				range_end
//			
// Returns:     none
//
// Description: recolours the pixel data according to the entires in the 
//				tint table.  The client specifies the subset of bitmaps
//				to recolour.
//						
// ----------------------------------------------------------------------
void Gallery::Recolour(const uint16 tintTable[65536], 
								 uint32 range_start,
								 uint32 range_end)
{
	if(range_start >= myNumberOfBitmaps ||  range_end >= myNumberOfBitmaps)
	{
		return;
	}

	if(myCompressedBitmaps)
	{
		for(int i = range_start; i <= range_end; i++)
		{
			myCompressedBitmaps[i].Recolour(tintTable);
		}
	}
	else if(myBitmaps)
	{
		for(int i = range_start; i < range_end; i++)
		{
			myBitmaps[i].Recolour(tintTable);
		}
	}
}

int32 Gallery::ClonedAbsoluteBase() const
{
	if(myAmIClonedFlag)
		return myBaseImage;
	else 
		return 0;
}

Bitmap* Gallery::GetBitmap(uint32 index)
{	
	if (index>=myNumberOfBitmaps)
		throw GalleryException(theCatalogue.Get("gallery_error", 0),__LINE__);

	if(myBitmaps)
	{
		return myBitmaps+index;	
	}
	else if(myCompressedBitmaps)
	{
		return myCompressedBitmaps+index;
	}

	return NULL;
}

int32 Gallery::GetBitmapWidth(uint32 index)
{
	_ASSERT(index<myNumberOfBitmaps);

	if(myBitmaps)
		return index<myNumberOfBitmaps ? myBitmaps[index].GetWidth() :  myBitmaps[0].GetWidth();
	else if(myCompressedBitmaps)
		return index<myNumberOfBitmaps ? myCompressedBitmaps[index].GetWidth() :  myCompressedBitmaps[0].GetWidth();

	return 0;
}

int32 Gallery::GetBitmapHeight(uint32 index)
{
		ASSERT(index<myNumberOfBitmaps);

		if(myBitmaps)
			return index<myNumberOfBitmaps ? 
				myBitmaps[index].GetHeight() : myBitmaps[0].GetHeight();
		else if(myCompressedBitmaps)
			return index<myNumberOfBitmaps ? 
				myCompressedBitmaps[index].GetHeight() : myCompressedBitmaps[0].GetHeight();

		return 0;
}

bool Gallery::IsPixelTransparent(uint32 x,
							uint32 y,
							int32 imageIndex)
{
	if(imageIndex > myNumberOfBitmaps)
		return false;

	// this should get any type of bitmap
	Bitmap* bitmap = GetBitmap(imageIndex);

	if(bitmap)
		return	bitmap->IsPixelTransparent(Position(x,y));

	return true;
}

bool Gallery::IsValid()
{
	return (myBitmaps || myCompressedBitmaps)?true:false;
}

bool Gallery::ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight)
{
	if(myBitmaps)
	{
		for (int32 i = 0; i< myNumberOfBitmaps; i++)
		{
			// as soon as you catch just one bitmap which is under 3x3 bail
			if(myBitmaps[i].GetWidth() < minimumWidth ||
				myBitmaps[i].GetHeight() < minimumHeight )
				return false;
		}
	}
	else if(myCompressedBitmaps)
	{
		for (int32 i = 0; i< myNumberOfBitmaps; i++)
		{
			// as soon as you catch just one bitmap which is under 3x3 bail
			if(myCompressedBitmaps[i].GetWidth() < minimumWidth || 
				myCompressedBitmaps[i].GetHeight() < minimumHeight )
				return false;
		}

	}

	// all bitmaps conform
	return true;
}


bool Gallery::ConvertTo(uint32 to)
{
	uint32 from;
	switch(to)
	{
	case RGB_555:
		{
		if(!(myPixelFormat & C16_FLAG_565))
			return false;
		from = RGB_565;
		break;
		}
	case RGB_565:
		{
		if(myPixelFormat & C16_FLAG_565)
			return false;
		from = RGB_555;
		break;
		}
	default:
		return false;
	}
	
	myPixelFormat ^= C16_FLAG_565;

	// set the memory mapped file back to the beginning
	myMemoryMappedFile.Seek(0,File::Start);


	if(myBitmaps)
	{
// TODO: is this assert correct? if so, .blk files don't get converted here...
//		_ASSERT(myFileFormat == IAmUncompressed);
		for(uint32 i = 0; i < myNumberOfBitmaps; i++)
		{
			myBitmaps[i].Convert(from,to);
		}
		switch(myFileFormat)
		{
			case IAmTiled:
				{
					SaveTiledFile(myMemoryMappedFile);
					break;
				}
			default:
				{
					SaveUncompressedFile(myMemoryMappedFile);
					break;
				}
		}
	}
	else if(myCompressedBitmaps)
	{
		for(uint32 i = 0; i < myNumberOfBitmaps; i++)
		{
			myCompressedBitmaps[i].Convert(from,to);
		}
			
		SaveCompressedFile(myMemoryMappedFile);
	}

//	myPixelFormat ^= C16_FLAG_565;

	return true;
}


uint32 Gallery::Save(MemoryMappedFile& file)
{
	switch(myFileFormat)
	{
	case IAmUncompressed:
		{
			return SaveUncompressedFile( file);
		}
	case IAmCompressed:
		{
			return SaveCompressedFile(file);
		}
	case IAmTiled:
		{
			return SaveTiledFile(file);
		}
	}
return 0;
}

uint32 Gallery::Save(File& file)
{
	switch(myFileFormat)
	{
	case IAmUncompressed:
		{
			return SaveUncompressedFile(file);

		}
	case IAmCompressed:
		{
			return SaveCompressedFile(file);
		}
	case IAmTiled:
		{
			// not implemented
			_ASSERT(false);
			break;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////
// Saving Uncompressed format files
/////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      SaveCompressedFile 
// Arguments:   file - a memory mapped file to save to
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to a given memory mapped file
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveUncompressedFile(MemoryMappedFile& file)
{

	uint32 bytesWritten = 0;
	//	flag = 0x01 for
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file

	file.WriteUINT32(myPixelFormat);

	// Now the number of images in the file
	file.WriteUINT16(myNumberOfBitmaps);

	bytesWritten +=6;
		
	int i;
	for( i=0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myBitmaps[i].SaveHeader(file);
	}

	//now read in each bit

	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveCompressedFile 
// Arguments:   file - a memory mapped file to save to
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to a given file
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveUncompressedFile(File& file)
{

	uint32 bytesWritten = 0;
	//	flag = 0x01 for
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file
	file.Write(&myPixelFormat,sizeof(myPixelFormat));

	// Now the number of images in the file
	file.Write(&myNumberOfBitmaps,sizeof(uint16));

	bytesWritten +=6;
		
	int i;
	for( i=0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myBitmaps[i].SaveHeader(file);
	}

	//now read in each bit

	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}

// user must now give local pathname - function unused anyway?
uint32 Gallery::SaveUncompressedFileInALocalWorldFolder(std::string& path)
{

	int x = path.find_last_of(".");
	if( x == -1 )
		path += ".s16";
	else
		path = path.substr( 0, x + 1 ) + "s16";

	// I know that I am an S16
//	path.SetExtension("s16");

//	// force the file to be created locally
//	std::string filename;
//	path.GetWorldDirectoryVersionOfTheFile(filename, true);

	File file;
	file.Create(path);

	return SaveCompressedFile(file);
}


// ----------------------------------------------------------------------
// Method:      SaveUncompressedFile 
// Arguments:   None
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to your own memorymapped file.
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveUncompressedFile()
{
	myMemoryMappedFile.Seek(0,File::Start);
	return SaveUncompressedFile(myMemoryMappedFile);
}

/////////////////////////////////////////////////////////////////////////
// Saving Compressed format files
/////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      SaveCompressedFile 
// Arguments:   file - a memory mapped file to save to
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to a given memory mapped file
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveCompressedFile(MemoryMappedFile& file)
{

	uint32 bytesWritten = 0;

	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file

	file.WriteUINT32(myPixelFormat);

	// Now the number of images in the file

	file.WriteUINT16(myNumberOfBitmaps);

	
	bytesWritten +=6;
		
	uint32 i;
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myCompressedBitmaps[i].SaveHeaderAndOffsetData(file);
	}

	// now save out each bit
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten +=  myCompressedBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveCompressedFile 
// Arguments:   file - a file to save to
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to a given file
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveCompressedFile(File& file)
{
	uint32 bytesWritten = 0;

	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file

	file.Write(&myPixelFormat,sizeof(myPixelFormat));

	// Now the number of images in the file

	file.Write(&myNumberOfBitmaps,sizeof(uint16));
	
	bytesWritten +=6;
		
	uint32 i;
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myCompressedBitmaps[i].SaveHeaderAndOffsetData(file);
	} 


	// now save out each bit
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten +=  myCompressedBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}

// function not used? - would specify whole path anyway
// ----------------------------------------------------------------------
// Method:      SaveCompressedFile 
// Arguments:   path - name and path of the file
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to a new file in the world directory
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveCompressedFileInALocalWorldFolder(std::string& path)
{
	// I know that I am a compressed gallery 
//	path.SetExtension("c16");

	// force the file to be created locally
//	std::string filename;
//	path.GetWorldDirectoryVersionOfTheFile(filename, true);

		int x = path.find_last_of(".");
	if( x == -1 )
		path += ".c16";
	else
		path = path.substr( 0, x + 1 ) + "c16";

	File file;
	file.Create(path);

	return SaveCompressedFile(file);
}


// ----------------------------------------------------------------------
// Method:      SaveCompressedFile 
// Arguments:   None
//			
// Returns:     the number of bytes written
//
// Description: Saves file data to your own memorymapped file.
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveCompressedFile()
{
	myMemoryMappedFile.Seek(0,File::Start);
	return SaveCompressedFile(myMemoryMappedFile);
}

/////////////////////////////////////////////////////////////////////////
// Saving Tiled format files
/////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   dataPtr - point in file to write to
//			
// Returns:     true if the file writes OK false otherwise
//
// Description: Saves the bits and headers for each bitmap.
//				This routine will always save the file out as a
//				656 file
//			
//			
// ----------------------------------------------------------------------
uint32 Gallery::SaveTiledFile(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;

	file.WriteUINT32(myPixelFormat);

	file.WriteUINT16(myTileWidth);
	file.WriteUINT16(myTileHeight);
	file.WriteUINT16(myNumberOfBitmaps);

	bytesWritten +=8;
		
	uint32 i;
	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten +=myBitmaps[i].SaveHeader(file);
	}

	//now read in each bit

	for(i = 0; i < myNumberOfBitmaps; i++)
	{
		bytesWritten += myBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}


void Gallery::ScaleGallery(float scaleby)
{
	if(myPixelFormat != IAmTiled)
	{
		if(myBitmaps)
		{
			_ASSERT(myPixelFormat == IAmUncompressed);

			for(int i = 0; i < myNumberOfBitmaps; i++)
			{
				myBitmaps[i].Scale(scaleby);
			}
		}
		else if(myCompressedBitmaps)
		{
		// not implemented
		}
	}
}

Bitmap* Gallery::GetTile(uint32 index)
{
	
	if(myDrawnTilesMap.find(index) == myDrawnTilesMap.end())
	{
//		myBitmaps->SetData((uint16*)(myPixelData + 
		//	(index * GetBitmapWidth(0) * GetBitmapWidth(0) << 1)) );
		
		myDrawnTilesMap[index] = true;

		return index<myNumberOfBitmaps ? myBitmaps + index: NULL;
	}

	return NULL;
}


void Gallery::Trash()
{
	delete [] myCompressedBitmaps;
	delete [] myBitmaps;
	myCompressedBitmaps = NULL;
	myBitmaps = NULL;

	myName+=".bollocksed"; // This is __NECESSARY__ - Dan
}

/*
// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool Gallery::Write(CreaturesArchive &archive) const
{
	_ASSERT(myAmIClonedFlag == true);

	if(myAmIClonedFlag)
	{
		// we only seriliase out cloned sprites
		archive << myName;
		archive << myBaseImage;
		archive << myPixelFormat;
		archive << int(myFileFormat);
		archive << myAmIClonedFlag;
		archive << myNumberOfBitmaps;
		archive << myReferenceCount;
		archive << myFileSpec;

		if((myFileFormat == IAmUncompressed) && myAmIClonedFlag)
		{
			for	(int i=0; i<myNumberOfBitmaps; i++)
				myBitmaps[i].Write( archive );
		}

	}
	else
	{
		return false;
	}

	return true;
}
*/

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Gallery::Read(CreaturesArchive &archive)
{
			
	int32 version = archive.GetFileVersion();

	if(version >= 13)
	{
		archive >> myName;
		archive >> myBaseImage;
		archive >> myPixelFormat;

		int temp;
		archive >> temp;
		myFileFormat = FileFormat(temp);

		archive >> myAmIClonedFlag;

		_ASSERT(myAmIClonedFlag);
		archive >> myNumberOfBitmaps;
		archive >> myReferenceCount;
		archive >> myFileSpec;

		myBitmaps = new Bitmap[myNumberOfBitmaps];

		if((myFileFormat == IAmUncompressed) && myAmIClonedFlag)
		{
			for	(int i = 0; i < myNumberOfBitmaps; i++) 
			{
				if(!myBitmaps[i].Read( archive ))
					return false;
			}
		}

		//success
		SharedGallery::theSharedGallery().AddClonedGallery( this );

	}
	else if(version >= 3)
	{

		archive >> myNumberOfBitmaps;

		myBitmaps = new Bitmap[myNumberOfBitmaps];

		for	(int i = 0; i < myNumberOfBitmaps; i++) 
		{
			if(!myBitmaps[i].Read( archive ))
				return false;
		}

		archive >> myReferenceCount;
	//	archive >> myName;
		int directory =0;
		bool useLocalWorldDirectoryVersionOfTheFile;

		archive >> directory >> myName 
		>> useLocalWorldDirectoryVersionOfTheFile;

		archive >> myFileSpec;

		archive >> myPixelFormat;

		myFileFormat = IAmUncompressed;
		myAmIClonedFlag = true;
	}

	else
	{
		_ASSERT(false);
		return false;
	}

	_ASSERT(myAmIClonedFlag == true);

	if(myAmIClonedFlag)
	{
		
		// Destination format is...
		uint32 dest = DisplayEngine::theRenderer().GetMyPixelFormat();
		uint32 from = RGB_555;
		if( myPixelFormat & C16_FLAG_565 )
			from = RGB_565;
		int i;
		for( i=0; i < myNumberOfBitmaps; i++)
		{
			myBitmaps[i].Convert(from, dest);
		}

		if( dest == RGB_555 ) // Clear the 565 bit
			myPixelFormat = myPixelFormat & ~1;
		else // Set the 565 bit
			myPixelFormat = myPixelFormat | 1;
		}

	return true;
}

