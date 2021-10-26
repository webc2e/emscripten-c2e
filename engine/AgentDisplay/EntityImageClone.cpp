#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "EntityImageClone.h"
#include "../Camera/MainCamera.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Display/DisplayErrorConstants.h"
#include "../../common/StringFuncs.h"
#include <string>

#ifdef C2D_DIRECT_DISPLAY_LIB
#include <C2D.h>
#include <C2DScreen.h>
#include <C2DSprite.h>
#include "../File.h"
#include "../..\common\FileFuncs.h"
#else
	#include "../Display/ClonedGallery.h"
	#include "../Display/DisplayEngine.h"
	#include "../Display/Sprite.h"
#endif //C2D_DIRECT_DISPLAY_LIB

//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( ClonedEntityImage)

#ifdef C2D_DIRECT_DISPLAY_LIB
std::vector<C2DAniStrip*> ClonedEntityImage::ourClonedAnimationStrips;
#endif


ClonedEntityImage::ClonedEntityImage()
{
	myReadArchivedGalleryFlag = false;
}

ClonedEntityImage::ClonedEntityImage(FilePath const& galleryName,
									 int32 plane,//plane
									 int32 worldx, // world x
									 int32 worldy, // world y
									 uint32 baseimage, // base image
									 uint32 numImages,
									 uint32 currentimage )
{
	myBlockWidth = baseimage;
	myBlockHeight =plane;

	myFilePath = galleryName;
	myFilePath.LowerCase();
	myPlane = plane;
	myNumberOfImages = numImages;

	myReadArchivedGalleryFlag = false;

#ifdef C2D_DIRECT_DISPLAY_LIB

	myFontWidth = 8;
	myFontHeight = 8;

	myFontColour = 0;

	myFontType = 400;

		std::string headerFile = galleryName.GetFullPath();

		int x = headerFile.find_last_of(".");

		if(x != -1)
		{
			headerFile = headerFile.substr( 0, x + 1 );
			headerFile += "hed";

			if(FileExists(headerFile.c_str()))
			{
				File file(headerFile);

				int blockWidth,blockHeight,numberOfImages = 0; 

				file.Read(&blockWidth,sizeof(int));
				file.Read(&blockHeight,sizeof(int));
				file.Read(&numberOfImages,sizeof(int));
				file.Close();

				myNumberOfImages = numberOfImages;
				CreateAnimStrip(blockWidth,blockHeight);
			}
			// it's not an error if there is no .hed file it's optional
		}
		

#else
	// check that you now have a gallery
	if(!SetGallery(myFilePath,baseimage,numImages))
		{
		
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidNoGalleryCreated,
									std::string("ClonedEntityImage::ClonedEntityImage"),
									myFilePath.GetFullPath().c_str());

		throw EntityImageException(string,__LINE__);
		}
#endif

	// the base index and current index should always be zero
	// since their galleries contain their bitmaps exclusively
	// and they are not shared by other entity images

	Init(plane,//plane
		 worldx, // world x
		 worldy, // world y
		 0,	// base image
		 0,
		 numImages); // current image
}

#ifdef C2D_DIRECT_DISPLAY_LIB

void ClonedEntityImage::CreateAnimStrip(int blockWidth,int blockHeight)
{
	myBlockWidth=blockWidth;
	myBlockHeight=blockHeight;

	if(!SetGallery(myFilePath,  myCurrentBaseImageIndex,myNumberOfImages))
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidNoGalleryCreated,
									std::string("ClonedEntityImage::ClonedEntityImage"),
									myDefaultGalleryName.GetFullPath().c_str());

		throw EntityImageException(string,__LINE__);
	}
}

#endif

ClonedEntityImage::ClonedEntityImage(
	int32 plane,//plane
	 int32 worldx, // world x
	 int32 worldy, // world y
	 int32 width,
	 int32 height
	 )
{
	myReadArchivedGalleryFlag = false;
}

// ----------------------------------------------------------------------
// Method:      SetGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseImage - the image in the sprite file that this
//							 particular entity will start from 
//				numImages - must know exactly how many images to copy from the
//							file
//							
//				
// Returns:     None
//
// Description: Create a gallery for this entity  and set up a
//				sprite to hold your current image data.
//						
// ----------------------------------------------------------------------
bool ClonedEntityImage::SetGallery(FilePath const& galleryName,
								   uint32 baseImage,
								   uint32 numImages/*= 0*/,
								   bool reset /*= true*/)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	if(!FileExists(galleryName.GetFullPath().c_str()))
	{
		return false;
	}

	if(myAniStrip)
	{
		delete	myAniStrip;
		myAniStrip = NULL;
	}

	myAniStrip = new C2DAniStrip(theMainView.GetSpritePtr(),
								galleryName.GetFullPath().c_str(),
								myBlockWidth,myBlockHeight,myNumberOfImages);


	if(!myAniStrip->IsValid())
	{
		delete	myAniStrip;
		myAniStrip = NULL;
	
		return false;
	}

	ourClonedAnimationStrips.push_back( myAniStrip);

	myAniStrip->SetColorKey(0);

	C2DSprite* oldSprite = mySprite;

//	if(mySprite)
	//	delete mySprite;

	mySprite = theMainView.AddAnimStrip(myAniStrip,myPlane);


	if(oldSprite)
	{
		mySprite->CopyDrawingAttributes(*oldSprite);
				
		delete oldSprite;
	}

	if(!myVisibleFlag && mySprite)
	{
		EntityImage::Unlink();
	}

	#else
		if(!mySprite)
			mySprite = new Sprite();
	#endif

	// the base index and current index should always be zero
	// since their galleries contain their bitmaps exclusively
	// and they are not shared by other entity images
	myAbsoluteBaseImage = 0;
	myCurrentImageIndex = 0;
	myCurrentBaseImageIndex = 0;

	bool ok = true;

	#ifndef C2D_DIRECT_DISPLAY_LIB
		ok = mySprite->SetGallery(galleryName.GetFullPath(),baseImage,numImages);
	#endif

	SetCurrentBitmap();

	return ok;

}

#ifdef C2D_DIRECT_DISPLAY_LIB
void ClonedEntityImage::SetFontAttributes(int width , 
										  int height, 
										  uint32 colour, 
										  uint32 type )
{
	myFontWidth = width;
	myFontHeight = height;

	myFontColour = colour;

	myFontType = type;
}
#endif

int ClonedEntityImage::SelectFontType(  std::string const  & fontName, int r, int g, int b, int rot, int swap )
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	if(myAniStrip)
	{

		myAniStrip->ChangeFont(fontName.c_str(),myFontWidth, myFontHeight);
		myAniStrip->SetFont();
	}
		return -1;
	#else
		return mySprite->SelectFont(fontName, r, g, b, rot, swap);
	#endif
}


void ClonedEntityImage::DrawString( int32 x, int32 y, const std::string& text,
								   int defaultFontIndex, std::vector<Sprite::CharStyle> charStyles, int stylesOffset )
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	if(myAniStrip)
	{

		myAniStrip->GetDC();
		myAniStrip->SetFont();
		myAniStrip->TextXY(x,y,myFontColour,text.c_str());
		myAniStrip->ReleaseDC();
	}

	#else
		mySprite->DrawString(x, y, text, defaultFontIndex, charStyles, stylesOffset);
	#endif
}

void ClonedEntityImage::MeasureString( const std::string &text, int fontIndex,
									  int32 &width, int32 &height )
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		height = 0;
		width = 0;

		uint32 length = text.size();

		for( uint32 i = 0; i < length; ++i )
		{
			int c = text[i];
			if( c < 0 ) c += 256;
			c -= ' ';
			if( c >= 0 )
			{
				width += 10; // estimate font width for now
				int h = 10; // estimate font height for now
				if( h > height ) height = h;
			}
		}

	#else
		mySprite->MeasureString( text, fontIndex, width, height );
	#endif
}

// draw a line to a bitmap/frame
void ClonedEntityImage::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/)  
{

	if(!mySprite)
		return;

	#ifdef C2D_DIRECT_DISPLAY_LIB

	mySprite->DrawLineToCurrentFrame(x1,y1,x2,y2,
									lineColourRed,
									lineColourGreen,
									lineColourBlue);
	
	#else

	mySprite->DrawLine(x1,
					y1,
					x2,
					y2,
					lineColourRed,
					lineColourGreen,
					lineColourBlue);  
	#endif
}


void ClonedEntityImage::DrawSprite(Position& whereToDraw,
							Bitmap* const bitmapToDraw)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	// this function is used for text in C2e and is so not
	// needed for the direct display library because we use
	// GDI for text.
	#else
		mySprite->DrawOnMe(whereToDraw,bitmapToDraw);
	#endif

}

void ClonedEntityImage::DrawSprite(Position& whereToDraw,
					FilePath const& galleryName,
					int32 spriteIndex,
					int32 baseimage,
					int32 numimages)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	// this function is used for text in C2e and is so not
	// needed for the direct display library because we use
	// GDI for text.
	#else
		Gallery* gallery = new Gallery(galleryName.GetFullPath(),
													baseimage,
													numimages);

		Bitmap* bitmap = gallery->GetBitmap(spriteIndex);

		if(bitmap)
		{
		ClonedEntityImage::DrawSprite(whereToDraw,
								bitmap);

		}

		delete gallery;
	#endif 

}

void ClonedEntityImage::Clear()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
// only do this if the anistrip is already in existence
	if(myAniStrip)
	{
		// recreate the strip with a blank canvas
		Unlink();

		delete myAniStrip;
		myAniStrip = NULL;

		CreateAnimStrip(myBlockWidth, myBlockHeight);
		Link(true);
	}
	#else
		mySprite->Clear();
	#endif
}

bool ClonedEntityImage::Write(CreaturesArchive &archive) const
{
	EntityImage::Write( archive );

	#ifndef C2D_DIRECT_DISPLAY_LIB
	// write the gallery out your self
	// we only seriliase out cloned sprites
		archive << myFilePath;
		archive << mySprite->GetGallery()->ClonedAbsoluteBase();
		archive << mySprite->GetGallery()->GetPixelFormat();
		archive << mySprite->GetBitmapCount();

	
		for	(int i=0; i< mySprite->GetBitmapCount(); i++)
		{
			archive << mySprite->GetBitmap(i)->GetWidth();
			archive <<  mySprite->GetBitmap(i)->GetHeight();

			archive <<  mySprite->GetBitmap(i)->GetOffset();

			archive <<  mySprite->GetBitmap(i)->GetPosition();
			archive.Write( (void *) mySprite->GetBitmap(i)->GetData(),
								mySprite->GetBitmap(i)->GetWidth() *
								mySprite->GetBitmap(i)->GetHeight() *
								sizeof(uint16) );

		}
	#endif
	return true;
}

bool ClonedEntityImage::Read(CreaturesArchive &archive)
{	
	int32 version = archive.GetFileVersion();

	if(version >= 13)
	{
		if(!EntityImage::Read( archive ))
			return false;

#ifndef C2D_DIRECT_DISPLAY_LIB
		uint32 baseImage=0;
		uint32 pixelFormat=0;
		int fileformat =0;
		uint32 numBitmaps=0;

		archive >> myFilePath;
		archive >> baseImage;
		archive >> pixelFormat;
		archive >> numBitmaps;

		mySprite->CreateGalleryForCloning(myFilePath.GetFullPath(),
										baseImage,
										pixelFormat,
										numBitmaps);

		for	(int i=0; i< mySprite->GetBitmapCount(); i++)
		{
			uint32 width =0;
			uint32 height =0;
			uint32 offset =0;
			Position pos;

			archive >> width;
			archive >>  height;

			archive >>  offset;

			archive >> pos;

			mySprite->GetBitmap(i)->SetWidth(width);
			mySprite->GetBitmap(i)->SetHeight(height);
			mySprite->GetBitmap(i)->SetOffset(offset);
			mySprite->GetBitmap(i)->SetPosition(pos);

			uint16* tempData = new uint16[width*height];

			archive.Read( (void *)tempData, width * height * sizeof(uint16) );

			mySprite->GetBitmap(i)->CreatePixelData(tempData);	
			
			delete [] tempData;
		}
#endif

	}
	else if(version >= 3)
	{
#ifndef C2D_DIRECT_DISPLAY_LIB
		if(!EntityImage::Read( archive ))
			return false;

		ClonedGallery* tempGallery;

		archive >> tempGallery;

		if(tempGallery)
		{
			mySprite->CopyGalleryData(*tempGallery);
		}

		// delete the old cloned gallery
		delete tempGallery;
#endif
	}
	else
	{
		_ASSERT(false);
		return false;
	}

#ifndef C2D_DIRECT_DISPLAY_LIB
	mySprite->SetPlane(myPlane);
	SetCurrentBitmap();
#endif		

	if(myVisibleFlag)
		Link(true);
	else
		Unlink();

	return true;
}


int32 ClonedEntityImage::GetAbsoluteBaseImage()
{
	ASSERT(myAbsoluteBaseImage == 0);
		
	#ifdef C2D_DIRECT_DISPLAY_LIB
		return myAbsoluteBaseImage;
	#else
		return mySprite->ClonedAbsoluteBase();
	#endif
}

ClonedEntityImage::~ClonedEntityImage()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	
	DeleteAnimStrip();
	
	delete myAniStrip;
	myAniStrip = NULL;
	#endif


}

#ifdef C2D_DIRECT_DISPLAY_LIB
// when the direct display engine loses it's surfaces
// we need to refresh each and every animstrip 
void ClonedEntityImage::RefreshAllAnimStrips()
{
	for(C2DANISTRIPVECTOR_ITERATOR it = ourClonedAnimationStrips.begin();it != 
		ourClonedAnimationStrips.end(); it++)
	{
			
			(*it)->Create(theMainView.GetSpritePtr(),
									NULL,
									-1,-1,-1);

			(*it)->SetColorKey(0);

			(*it)->Restore();
	}
}

void ClonedEntityImage::DeleteAnimStrip()
{
//	C2DANISTRIPMAP_ITERATOR it = ourClonedAnimationStrips.find(myFilePath.GetFullPath().c_str());
	for(C2DANISTRIPVECTOR_ITERATOR it = ourClonedAnimationStrips.begin();it != 
		ourClonedAnimationStrips.end(); it++)
	{
		if((*it) == myAniStrip)
		{
			ourClonedAnimationStrips.erase(it);
			break;
		}
	}


}
#endif

