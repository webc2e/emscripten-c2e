// --------------------------------------------------------------------------
// Filename:	EntityImage.cpp
// Class:		EntityImage
// Purpose:		This class is the interface to the rest of C2e.  This class 
//				is closely based on the C2 entity and will evolve as required.
//				
//				
//
// Description: 
///* Notes:
//
//	An entity is a 'thing' that exists in the game world (unlike a sprite
//	for example, which only exists in the computer). Entities are contained
//	within the various types of Object (creatures, machines, etc.); some
//	objects may contain several Entities.
//
//	Entities are the things that get redrawn when a rectangle is invalidated.
//	A library of pointers to all extant entities is stored in EntityLib[]
//	for this purpose.
//
//	Entities are ALWAYS encapsulated in Objects of one kind or another. Thus,
//	although each entity contains its own X&Y members, these get set as
//	appropriate by the Object which owns them, and don't need to be
//	initialised when creating the entity itself. Thus, entities can be
//	created dynamically by the objects that contain them.
//
//
//	Entities:
//
//
//	Entity				Simplest form of entity, instantiated for use
//	|					within SimpleObjects and CompoundObjects.
//	|					Also serves as base class for articulated entities,
//	| 					for encapsulation within Creatures.
//	| 					Whereas a Sprite uniquely represents a given bitmap,
//	| 					several Entities can refer to the same Sprite
//	| 					- an Entity represents the simplest individual
//	| 					object in the world; several Entities may LOOK
//	| 					the same, but ARE different.
//	|
//	|
//	BodyPart
//	| |
//	| Body				Parts of articulated objects. A Limb is connected to
//	| 					one or no daughter Limbs (eg. a thigh Limb connects
//	| 					to a shin Limb); A Body is similar, but connects
//	Limb                to several Limb chains. The BodyPart class contains
//						methods & data that are common to both types
//						Body & Limb classes contain pointers to BodyData
//						or LimbData classes respectively.
//
//						The BodyPart Entities contain pointers to LimbData
//						or BodyData structs, which contain the XY
//						information that is common to all BodyParts with
//						the same appearance.
//
//
//
//
//				
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added comments. 
//							Changed Class and file names from Object to EntityImage.
//							Introducing Methods from the C2 Entity class.
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#ifdef C2D_DIRECT_DISPLAY_LIB
	#include <C2D.h>
	#include <C2DScreen.h>
	#include <C2DSprite.h>
	#include "../File.h"
	#include "../..\common\FileFuncs.h"
#else
	#include	"../Display/Line.h"
	#include	"../Display/Sprite.h"
#endif //C2D_DIRECT_DISPLAY_LIB

#include	"EntityImage.h"
#include	"../AgentManager.h"
#include	"../Camera/MainCamera.h"
#include	"../Display/ErrorMessageHandler.h"
#include "../Display/DisplayErrorConstants.h"
#include "../Display/DisplayEngine.h"

#include "../App.h"

#ifdef C2D_DIRECT_DISPLAY_LIB
std::map<std::string,C2DAniStrip*> EntityImage::ourAnimationStrips;
#endif

//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( EntityImage)

EntityImage::EntityImage():
	myCurrentOverlayIndex(-1),
	myReadArchivedGalleryFlag(1),
	myCurrentBaseImageIndex(0),
myAbsoluteBaseImage(0),
myCurrentImageIndex (0),
mySavedImageIndexState(0),
myFrameRate(1), 
myFrameCount(0),
myCurrentWidth(0),
myCurrentHeight(0),
myVisibleFlag(true),
mySprite(0),
mySpriteIsCameraShy(false),
myDrawAlphaFlag(false),
myAlphaIntensity(0),
myScaledFlag(false),
myBlockWidth(0),
myBlockHeight(0)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
				myAniStrip = NULL;
	#else
				mySprite = new Sprite();
	#endif
}


EntityImage::EntityImage(FilePath galleryName,
						int32 numimages,
						int32 plane,
						int32 worldx, 
						int32 worldy, 
						uint32 baseimage,	
						uint32 currentimage)
:myNumberOfImages(numimages),
myCurrentOverlayIndex(-1),
myReadArchivedGalleryFlag(1),
myFrameRate(1), myFrameCount(0),
myCurrentWidth(0),
myCurrentHeight(0),
myVisibleFlag(true),
mySpriteIsCameraShy(false),
mySprite(0),
myFilePath(galleryName),
myDrawAlphaFlag(false),
myAlphaIntensity(0),
myScaledFlag(false),
myBlockWidth(0),
myBlockHeight(0)
{
	galleryName.LowerCase();
	myFilePath.LowerCase();
	myPlane = plane;

	// keep these values for when the gallery gets reset
	myDefaultGalleryName = galleryName;
	myDefaultBaseImage = baseimage;

#ifdef C2D_DIRECT_DISPLAY_LIB
	myDefaultBaseImage =0;
	myAniStrip = NULL;
	if(myNumberOfImages != 0 && baseimage > myNumberOfImages)
	{
		char buf[_MAX_PATH] = "\0";

		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidInvalidBasePose,
									std::string("EntityImage::EntityImage"),
									galleryName.GetFullPath().c_str());

		throw EntityImageException(string,__LINE__);
	}
#endif

	myCurrentImageIndex = baseimage;
	myCurrentBaseImageIndex = baseimage;

	// allow for entity images with blank galleries now
	if(!galleryName.empty())
	{
#ifdef C2D_DIRECT_DISPLAY_LIB
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
		/*	else
			{
			// do not give an error message here because .hed
			// files are useful but not necessary
			
				std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidNoGalleryCreated,
									std::string("EntityImage::EntityImage"),
									headerFile.c_str());

				throw EntityImageException(string,__LINE__);

			}*/
		}	

#else
		// check that you now have a gallery
		if(!SetGallery(galleryName,  baseimage))
		{
			char buf[_MAX_PATH] = "\0";

			std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)sidNoGalleryCreated,
										std::string("EntityImage::EntityImage"),
										galleryName.GetFullPath().c_str());


			throw EntityImageException(string,__LINE__);
		}
#endif

		Init(plane,//plane
			 worldx, // world x
			 worldy, // world y
			 baseimage,	// base image
			 currentimage,
			 numimages);
	}
}

#ifdef C2D_DIRECT_DISPLAY_LIB

void EntityImage::CreateAnimStrip(int blockWidth,int blockHeight)
{
	myBlockWidth=blockWidth;
	myBlockHeight=blockHeight;

	if(!SetGallery(myFilePath,  myCurrentBaseImageIndex,myNumberOfImages,false))
	{
		char buf[_MAX_PATH] = "\0";

		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidNoGalleryCreated,
									std::string("EntityImage::EntityImage"),
									myDefaultGalleryName.GetFullPath().c_str());


		throw EntityImageException(string,__LINE__);
	}
}

void EntityImage::RemoveAllAnimStrips()
{
	for(C2DANISTRIPMAP_ITERATOR it = ourAnimationStrips.begin();it != 
		ourAnimationStrips.end(); it++)
	{
			delete (*it).second;
			(*it).second = NULL;
			ourAnimationStrips.erase(it);
	}

}

void EntityImage::RefreshAllAnimStrips()
{
	for(C2DANISTRIPMAP_ITERATOR it = ourAnimationStrips.begin();it != 
		ourAnimationStrips.end(); it++)
	{
			
			(*it).second->Create(theMainView.GetSpritePtr(),
									NULL,
									-1,-1,-1);

			(*it).second->SetColorKey(0);

			(*it).second->Restore();
	}
}

#endif


void EntityImage::YouAreCameraShy(bool really)
{
	mySpriteIsCameraShy = really;

#ifdef C2D_DIRECT_DISPLAY_LIB
	//_ASSERT(false);
#else
	mySprite->YouAreCameraShy(really);
#endif
}

bool EntityImage::AreYouCameraShy()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		//_ASSERT(false);
		return false;
	#else
		return mySprite->AreYouCameraShy();
	#endif
}

void EntityImage::Init(int32 plane,//plane
		 int32 worldx, // world x
		 int32 worldy, // world y
		 uint32 baseimage,					// base image
		 uint32 currentimage,
		 uint32 numimages)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB

	#else
		if(!mySprite)
			mySprite = new Sprite();
	#endif

	if(numimages)
		myNumberOfImages = numimages;
	else 
	{
		#ifndef C2D_DIRECT_DISPLAY_LIB
			myNumberOfImages = mySprite->GetBitmapCount() - baseimage;
		#endif
	
	}

	myAnimLength = 0;

	myPlane = plane;

	#ifdef C2D_DIRECT_DISPLAY_LIB
			myCurrentBaseImageIndex = 0;
			myAbsoluteBaseImage = 0;
			myCurrentImageIndex = 0;
			mySavedImageIndexState = 0;
	#else
			mySprite->SetPlane(plane);


	#endif
			myCurrentBaseImageIndex = baseimage;
			myAbsoluteBaseImage = baseimage;
			myCurrentImageIndex = baseimage;
			mySavedImageIndexState = currentimage;

	myWorldPosition.SetX(worldx);
	myWorldPosition.SetY(worldy);

	SetCurrentBitmap();

#ifndef C2D_DIRECT_DISPLAY_LIB
	Link();
#endif
}

EntityImage::~EntityImage()
{

	Unlink();

	ResetLines();

#ifdef C2D_DIRECT_DISPLAY_LIB

	if(myAniStrip)
		DeleteAnimStrip();
	myAniStrip = NULL;

#endif
	delete mySprite;
	mySprite = NULL;
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void EntityImage::DeleteAnimStrip()
{
	C2DANISTRIPMAP_ITERATOR it = ourAnimationStrips.find(myFilePath.GetFullPath().c_str());

	if(it != ourAnimationStrips.end())
	{
		(*it).second->DecrementReferenceCount();

		if( !(*it).second->IsUsed())
		{
			delete (*it).second;
			(*it).second = NULL;
			ourAnimationStrips.erase(it);
		}
	}
}
#endif
// when reloading the skeleton we don't want to register
// the entity image with the display until the very last minute
void EntityImage::Link(bool forceLink /*=false*/)
{
	if(myReadArchivedGalleryFlag || forceLink)
	{
		theMainView.Add(this);
		myVisibleFlag = true;
	}
}

void EntityImage::Unlink()
{
	theMainView.Remove(this);
	myVisibleFlag = false;
}

// ----------------------------------------------------------------------
// Method:      SetGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseImage - the image in the sprite file that this
//							 particular entity will start from 
//				numImages - the number of images for this sprite
//							
//				
// Returns:     None
//
// Description: Create a gallery for this entity  and set up a
//				sprite to hold your current image data	in this base class
//				the number of images is largely ignored, override if you 
//				want to utilise it
//						
// ----------------------------------------------------------------------
bool EntityImage::SetGallery(FilePath const& galleryName, 
							 uint32 baseimage,
							 uint32 numImages /*= 0*/,
							 bool reset/*=true*/)
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	if(!FileExists(galleryName.GetFullPath().c_str()))
	{
		return false;
	}

	if(galleryName == myFilePath && myAniStrip != NULL)
		return true;

	C2DSprite* oldSprite = mySprite; 
	// if you already have a gallery prepare to change it
	if( reset && myAniStrip != NULL)
	{
		DeleteAnimStrip();
		Unlink();
		myAniStrip = NULL;
		myVisibleFlag = true;
	}

	C2DANISTRIPMAP_ITERATOR it = ourAnimationStrips.
	find(galleryName.GetFullPath().c_str());

	if(it == ourAnimationStrips.end())
	{
		myAniStrip = new C2DAniStrip(theMainView.GetSpritePtr(),
								galleryName.GetFullPath().c_str(),
								myBlockWidth,myBlockHeight,myNumberOfImages);

		if(!myAniStrip->IsValid())
			return false;
		if (!myAniStrip->m_lpDDS)
			return false;

		myAniStrip->SetColorKey(0);

		mySprite = theMainView.AddAnimStrip(myAniStrip,myPlane);

		ourAnimationStrips[galleryName.GetFullPath().c_str()] = myAniStrip;
	}
	else
	{
		myAniStrip = (*it).second;
		myAniStrip->IncrementReferenceCount();

		mySprite = theMainView.AddAnimStrip(myAniStrip,myPlane);
	}

	if(oldSprite)
	{
			mySprite->CopyDrawingAttributes(*oldSprite);
				delete oldSprite;
	}

	if(!myVisibleFlag && mySprite)
	{
		Unlink();
	}
			
#else
	if(!mySprite)
		mySprite = new Sprite();
#endif

	if(galleryName.GetFullPath().empty())
		return false;

	myAbsoluteBaseImage = baseimage;
	myFilePath = galleryName;
	myFilePath.LowerCase();

	bool carryOn = true;
		
	#ifndef C2D_DIRECT_DISPLAY_LIB
		carryOn = mySprite->SetGallery(galleryName.GetFullPath());
	#endif
		
	if(carryOn)
	{
		if(reset)
			myCurrentImageIndex = baseimage;

		SetCurrentBitmap();
		YouAreCameraShy(mySpriteIsCameraShy);	

		return true;
	}

	return false;
}





// ----------------------------------------------------------------------
// Method:      SetOverlayGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//		
//				
// Returns:     None
//
// Description: Create an overlay gallery for this entity.  Certain
//				types of entity (body parts) will want to replace their
//				normal image by one from an overlay gallery.  The overlay
//				gallery is optional and must be created explicitly if
//				it is needed.
//						
// ----------------------------------------------------------------------
void EntityImage::SetOverlayGallery(FilePath const& galleryName)
{
	if(galleryName.GetFullPath().empty())
		return;

	myOverlayFilePath = galleryName;

	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	#else
		mySprite->SetOverlayGallery(galleryName.GetFullPath());
	#endif


	//char db[256];
//	sprintf(db, "SetOverlayGallery %s\n", galleryName.c_str());
//	OutputDebugString(db);
}


bool EntityImage::SetOverlayIndex(int32 image, int32 layer)
{
	bool ok = false;
	myCurrentOverlayIndex = image;

		if(myCurrentOverlayIndex != -1)
		{
			#ifdef C2D_DIRECT_DISPLAY_LIB
				SetCurrentWidth(myBlockWidth);
				SetCurrentHeight(myBlockHeight);
			#else
				mySprite->ShowBitmap(layer);
				ok = mySprite->SetBitmap(image,layer,Sprite::OVERLAY_GALLERY);
				SetCurrentWidth(mySprite->GetBitmap()->GetWidth());
				SetCurrentHeight(mySprite->GetBitmap()->GetHeight());
				RECT bound;
				GetBound(bound);
				mySprite->SetCurrentBound(&bound);
				mySprite->SetPlane(myPlane);
			#endif	
		}
		else
		{
			#ifndef C2D_DIRECT_DISPLAY_LIB
				mySprite->ShowBitmap(0);
			#endif
			
		}

	return ok;
}

#ifndef C2D_DIRECT_DISPLAY_LIB
// ----------------------------------------------------------------------
// Method:      GetBitmap 
// Arguments:   index - index into sprite gallery
//				
// Returns:     pointer to the selected bitmap
//
// Description: returns a bitmap specified by index	
//						
// ----------------------------------------------------------------------
Bitmap* EntityImage::GetBitmap(int32 index)
{
	return mySprite->GetBitmap(index);
}
	
// ----------------------------------------------------------------------
// Method:      GetCurrentBitmap 
// Arguments:   None
//				
// Returns:     pointer to the currently showing bitmap
//
// Description: returns a bitmap sopecified by the current Image Index	
//						
// ----------------------------------------------------------------------
Bitmap* EntityImage::GetCurrentBitmap()
{
return mySprite->GetBitmap(myCurrentImageIndex);
}
	
#endif
			


void EntityImage::DestroyGallery()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	// don't think I have to do anything
#else
	mySprite->DestroyPrimaryGallery();
#endif
}




bool EntityImage::SetCurrentBitmap()
{
	// bounds checking here?  the gallery checks for
	// overall out of bounds we have no means of checking
	// whether the agent engineer has exceeded individual
	// sprite bounds
	bool ok = false;


#ifdef C2D_DIRECT_DISPLAY_LIB

	SetCurrentWidth(myBlockWidth);
	SetCurrentHeight(myBlockHeight);
	if(mySprite)
		mySprite->m_Frame = myCurrentImageIndex;
	ok = true;
#else

	if(mySprite)
	{
		ok = mySprite->SetBitmap(myCurrentImageIndex,
									0,
									Sprite::PRIMARY_GALLERY);

		RECT bound;
		GetBound(bound);
		mySprite->SetCurrentBound(&bound);
		SetCurrentWidth(mySprite->GetBitmap()->GetWidth());
		SetCurrentHeight(mySprite->GetBitmap()->GetHeight());
		mySprite->SetPlane(myPlane);
	}
#endif // C2D_DIRECT_DISPLAY_LIB
	return ok;

}
	
void EntityImage::SetPose(int32 pose)
{    
	// kill any running anim
	myAnimLength = 0;

	// set image index into gallery
    myCurrentImageIndex = myCurrentBaseImageIndex + pose;	

	SetCurrentBitmap();

}

// ----------------------------------------------------------------------
// Method:      SetCurrentIndex 
// Arguments:   image - the absolute index of the image in the gallery
//				file
//				
// Returns:     None
//
// Description: updates the current image index and bitmap	
//						
// ----------------------------------------------------------------------
void EntityImage::SetCurrentIndex(uint32 image) 
{
	myCurrentImageIndex = image;
	SetCurrentBitmap();
}

void EntityImage::HideCurrentBitmap()
{
#ifdef C2D_DIRECT_DISPLAY_LIB

	//_ASSERT(false);
#else
	mySprite->HideBitmap(0);
#endif
}

void EntityImage::ShowCurrentBitmap()
{
#ifdef C2D_DIRECT_DISPLAY_LIB

	//_ASSERT(false);
#else
	mySprite->ShowBitmap(0);
#endif
}

// ----------------------------------------------------------------------
// Method:      SetCurrentIndexFromBase 
// Arguments:   image - the index of the image in the gallery
//				file as an offset from the base.
//				
// Returns:     None
//
// Description: updates the current image index and bitmap	
//						
// ----------------------------------------------------------------------
void EntityImage::SetCurrentIndexFromBase(uint32 image)
{
	myCurrentImageIndex = myCurrentBaseImageIndex + image;
	SetCurrentBitmap();
}



// ----------------------------------------------------------------------
// Method:      GetLargestBound
// Arguments:   rect - a rect structure to receive the bounds 
//
// Returns:     Cycle through bitmaps in your gallery and get the largest
//				bound
//			
// ----------------------------------------------------------------------
void EntityImage::GetLargestBound(RECT& rect) 
{
	#ifdef C2D_DIRECT_DISPLAY_LIB

	//_ASSERT(false);
	#else
	for (uint32 i = myAbsoluteBaseImage; i< myAbsoluteBaseImage + myNumberOfImages; i++)
	{
		GetBitmap(i)->UpdateBoundIfLarger(rect);
	}
	#endif
}


// ----------------------------------------------------------------------
// Method:      Visible 
// Arguments:   test - rectangle to test my position against
//
// Returns:     true if this entity is visible within the bounds of the
//				rectangle supplied. false otherwise
//			
// ----------------------------------------------------------------------
bool EntityImage::Visible(const RECT& test)
{
		// compute dl & dr as the left and right bounds of
	// the object, relative to the View rectangle

	int dl = GetWorldX() - test.left;


    int dr = dl+GetWidth();

	return (((dl > (test.right - test.left) )
		||(test.bottom < GetWorldY())
		||(dr<0)
		||(test.top > (GetWorldY() + GetHeight())))? 0:1) ;
		
}

// ----------------------------------------------------------------------
// Method:      IsPixelTransparent 
// Arguments:   imageIndex - the sprite image to test
//								if this is -1 then use the current image
//				x - x coordinate of the pixel to test
//				y - y coordinate of the pixel to test
//
// Returns:     true if the pixel at the given coordinates is transparent
//				false otherwise
//			
// ----------------------------------------------------------------------
bool EntityImage::IsPixelTransparent(uint32 x,
									 uint32 y,
									 int32 imageIndex/* =-1*/)
{
	if(imageIndex == -1)
		imageIndex = myCurrentImageIndex;

	// make the world coordinates local to the bitmap
	x  -= myWorldPosition.GetX();
	y  -= myWorldPosition.GetY();

	#ifdef C2D_DIRECT_DISPLAY_LIB
		if(mySprite)
			return mySprite->IsPixelTransparent(x, y);
		else 
			return true;
	#else
		return mySprite->IsPixelTransparent(x, y, imageIndex);
	#endif
}

void EntityImage::CentreMe()
{
	int32 x;
	int32 y;

	int32 centrex;
	int32 centrey;

	theMainView.GetViewCentre(centrex,centrey);

	x = centrex - GetWidth()/2;
	y = centrex - GetHeight()/2;

	myWorldPosition.SetX(x);
	myWorldPosition.SetY(y);
}


bool EntityImage::Write(CreaturesArchive &archive) const
{
	archive << myPlane << myWorldPosition;
	archive << mySpriteIsCameraShy;

	archive << myFilePath;
	archive << myAbsoluteBaseImage;

	archive << myCurrentImageIndex;
	archive << myCurrentBaseImageIndex;
	archive << myNumberOfImages;
	archive << myDefaultBaseImage;
	archive << mySavedImageIndexState;

	archive << myReadArchivedGalleryFlag;

	archive << myBlockWidth;
	archive << myBlockHeight;

#ifdef C2D_DIRECT_DISPLAY_LIB
	if (myBlockWidth > 0 && myBlockHeight > 0)
	{
		if (!mySprite)
		{
			std::string message = "No sprite but we have a block width/height.\nFilename: " + myFilePath.GetFileName();
			throw EntityImageException(message, __LINE__);
		}
	}
	else
	{
		if (mySprite)
		{
			std::string message = "No block width/height but we have a sprite.\nFilename: " + myFilePath.GetFileName();
			throw EntityImageException(message, __LINE__);
		}
	}
#endif


	#ifdef C2D_DIRECT_DISPLAY_LIB
		archive << FilePath();
	#else
		// a gallery of possible overlays
		if(mySprite->HasOverlayGallery())
			archive << myOverlayFilePath;
		else
			archive << FilePath();
	#endif

	archive	<< myCurrentOverlayIndex;

	for(int32 i= 0; i < MaxAnimLength;i++)
	{
		archive << myAnim[i];
	}

	archive << myAnimLength;		// 0 if no anim playing
	archive << myAnimNext;			// next frame to show
	archive << myFrameRate;
	archive << myFrameCount;
	archive << myCurrentWidth << myCurrentHeight;
	archive << myVisibleFlag;

	archive << myDrawAlphaFlag;
	archive << myAlphaIntensity;

#ifdef C2D_DIRECT_DISPLAY_LIB
	if(mySprite)
	{
		archive << true;
		archive <<  mySprite->m_BltType;
		archive <<  mySprite->m_Frame;
		archive << myScaledFlag;
		archive << mySprite->m_ScaleFactor;
		archive <<  mySprite->m_AlphaValue;
		archive <<  mySprite->m_ShadowValue;
		archive <<  mySprite->m_ShadowOffsetX;
		archive << mySprite->m_ShadowOffsetY;
		archive << mySprite->m_StretchedWidth;
		archive << mySprite->m_StretchedHeight;
	}
	else
	{
		archive << false;
	}
#endif

	return true;
}

bool EntityImage::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{
		archive >> myPlane;

		archive >> myWorldPosition;
		archive >> mySpriteIsCameraShy;

		archive >> myFilePath;

		if(myFilePath.empty())
		{
			std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)sidGalleryNotSpecified,
									std::string("EntityImage::Read"));

			throw EntityImageException(string, __LINE__);
		}

		archive >> myAbsoluteBaseImage;
		archive >> myCurrentImageIndex;
		archive >> myCurrentBaseImageIndex;
		archive >> myNumberOfImages;
		archive >> myDefaultBaseImage;
		archive >> mySavedImageIndexState;
		archive >> myReadArchivedGalleryFlag;

#ifdef C2D_DIRECT_DISPLAY_LIB
		// for now in the direct display engine cloned entities don't store their data
		myReadArchivedGalleryFlag = true;
#endif

		if(version >= 22)
		{
			archive >> myBlockWidth;
			archive >> myBlockHeight;
		}

		if(myReadArchivedGalleryFlag)
		{
#ifdef C2D_DIRECT_DISPLAY_LIB
			// An agent can be saved before BLCK has called on it,
			// then the width and height are zero.
			if (myBlockWidth > 0 && myBlockHeight > 0)
#endif
			{
				SetGallery(myFilePath,myAbsoluteBaseImage, 0, false);
				// success
				SetCurrentBitmap();

#ifdef C2D_DIRECT_DISPLAY_LIB
				if (!mySprite)
				{
					std::string message = "Failed to make a sprite during loading\nFilename: " + myFilePath.GetFileName();
					throw EntityImageException(message, __LINE__);
				}
#endif
			}
		}

		archive >> myOverlayFilePath;

		if(myReadArchivedGalleryFlag)
		{
			// a gallery of possible overlays
			if(!myOverlayFilePath.empty())
			{
				SetOverlayGallery(myOverlayFilePath);
			}
		}

		archive >> myCurrentOverlayIndex;

		for(int32 i= 0; i < MaxAnimLength;i++)
		{
			archive >> myAnim[i];
		}

		archive >> myAnimLength;		// 0 if no anim playing
		archive >> myAnimNext;			// next frame to show
		archive >> myFrameRate;
		archive >> myFrameCount;
		archive >> myCurrentWidth >> myCurrentHeight;
		archive >> myVisibleFlag;

		if(version >= 25)
		{
			archive >> myDrawAlphaFlag;
			archive >> myAlphaIntensity;
			if (version < 34)
			{
				// Older versions had a bug where they saved random
				// data for these values - we override that!
				myDrawAlphaFlag = false;
				myAlphaIntensity = 0;
			}

#ifndef C2D_DIRECT_DISPLAY_LIB
			if (mySprite)
				mySprite->DrawAlphaBlended(myDrawAlphaFlag, myAlphaIntensity);
#endif
		}

#ifdef C2D_DIRECT_DISPLAY_LIB
		if(version >= 22)
		{
			bool spritesaved = false;

			archive >> spritesaved;

			if(spritesaved)
			{
				if (!mySprite)
				{
					std::string message = "Inconsistent state while loading sprite\nFilename: " + myFilePath.GetFileName();
					throw EntityImageException(message, __LINE__);
				}
				archive >>  mySprite->m_BltType;
				archive >>  mySprite->m_Frame;
				archive >> myScaledFlag;
				archive >> mySprite->m_ScaleFactor;
				archive >>  mySprite->m_AlphaValue;
				archive >>  mySprite->m_ShadowValue;
				archive >>  mySprite->m_ShadowOffsetX;
				archive >> mySprite->m_ShadowOffsetY;
				archive >> mySprite->m_StretchedWidth;
				archive >> mySprite->m_StretchedHeight;
			}
		}
#endif

	}
	else
	{
		//_ASSERT(false);
		return false;
	}


#ifdef C2D_DIRECT_DISPLAY_LIB
if(mySprite)
	mySprite->m_Plane = myPlane;
#else
	mySprite->SetPlane(myPlane);
#endif

	if(myVisibleFlag)
		Link();
	else
		Unlink();

	return true;
}


void EntityImage::SetFrameRate(const uint8 rate)
{
	if (rate < 1)
		return;
	myFrameRate = rate;
	myFrameCount = 0;
}


void EntityImage::SetAnim( const uint8* newanim, int length )
{
	ASSERT( length <= MaxAnimLength );

	// allow SetAnim(NULL)
	if( newanim == NULL || length == 0 || length > MaxAnimLength )
	{
		myAnimLength = 0;
		return;
	}

	// check for malicious agent engineers
	if( newanim[0] == AnimWrapValue )
	{
		myAnimLength = 0;
		return;
	}

	//memcpy( myAnim, newanim, length );
	for(int i=0; i < length; i++)
		myAnim[i] = newanim[i];
	myAnimLength = length;
	myAnimNext = 0;
}

bool EntityImage::ValidateAnim( const uint8* newanim, int length )
{
	ASSERT( length <= MaxAnimLength );

	// allow SetAnim(NULL)
	if( newanim == NULL || length == 0 || length > MaxAnimLength )
		return true;

	// check for malicious agent engineers
	if( newanim[0] == AnimWrapValue )
		return true;

	// loop through animation string, looking for errors
	for (int i = 0; i < length; ++i)
	{
		int pose = *(newanim + i);
		// stop at anim wrap value
		if (pose == AnimWrapValue) 
		{
			// check wrap point is OK if there is one
			if (i < length - 1)
			{
				int wrap = *(newanim + i + 1);
				if (wrap < 0 || wrap >= i)
					return false;
			}
			break;
		}
		// check pose OK
		if (!ValidatePose(pose))
			return false;
	}

	return true;
}

bool EntityImage::SetBaseAndCurrentIndex(uint32 baseimage)
{
	uint32 numBitmaps= 0;

	#ifdef C2D_DIRECT_DISPLAY_LIB
		numBitmaps = myNumberOfImages;
	#else
		numBitmaps = mySprite->GetBitmapCount();
	#endif

	// check new POSE is in range
	if(baseimage + myAbsoluteBaseImage < numBitmaps && 
	   baseimage < myNumberOfImages)
	{
		// change the value
		uint32 old_value = myCurrentBaseImageIndex;
		myCurrentBaseImageIndex = myAbsoluteBaseImage + baseimage;

		// check any animation string in progress is valid
		// Darn, we have to check if our animation can cope :(
		uint8 testAnim[MaxAnimLength];
		for(int i=0; i < MaxAnimLength; i++)
			testAnim[i] = (uint8)(myAnim[i]);
		if (!ValidateAnim(testAnim, myAnimLength))
		{
			// if invalid anim, go back to old base
			myCurrentBaseImageIndex = old_value;
			return false;
		}
		return true;
	}
	return false;
}


bool EntityImage::AnimOver()
{
	return (myAnimLength == 0);
}

void EntityImage::Animate()
{
	int numberOfBitmapsInGallery = 0;
	#ifdef C2D_DIRECT_DISPLAY_LIB
	numberOfBitmapsInGallery = 	myNumberOfImages;
	#else
		RECT bound;
		GetBound(bound);
		mySprite->SetCurrentBound(&bound);
		numberOfBitmapsInGallery = mySprite->GetBitmapCount();
	#endif

	// anim running?
	if( !myAnimLength )
		return;

	//Return if not time for an animation tick :)
	if (++myFrameCount % myFrameRate)
		return;

	uint32 frame = myAnim[ myAnimNext ];
	if( frame == AnimWrapValue )
	{
		// if there is a number after AnimWrapValue, loop back to that frame
		if( myAnimNext < myAnimLength - 1 )
			myAnimNext = myAnim[ myAnimNext + 1];
		else
			myAnimNext = 0;
		frame = myAnim[ myAnimNext ];
	}

	// add some runtime bounds checking here?
	myCurrentImageIndex = myCurrentBaseImageIndex + frame;

	if(myCurrentImageIndex >= myAbsoluteBaseImage + myNumberOfImages)
	{
		myCurrentImageIndex = myAbsoluteBaseImage;
	}


	ASSERT( myCurrentImageIndex < numberOfBitmapsInGallery  );
	SetCurrentBitmap();

	// frame advance, anim over?
	if( ++myAnimNext >= myAnimLength )
		myAnimLength = 0;	// kill kill kill!
}

uint16* EntityImage::GetImageData() const
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		//_ASSERT(false);
		return NULL;
	#else
		return mySprite->GetImageData();
	#endif
}

void EntityImage::SetPlane(int plane)
{
	myPlane = plane; 

	#ifdef C2D_DIRECT_DISPLAY_LIB
		if(mySprite)
			mySprite->m_Plane =plane;
	#else
		mySprite->SetPlane(plane);
	#endif

	if(myVisibleFlag)
		theMainView.UpdatePlane(this);

	// Now for each Line, update its plane...
#ifdef C2D_DIRECT_DISPLAY_LIB
	//_ASSERT(false);
#else
	if (myLines.empty())
		return;

	if(myVisibleFlag)
	{
		std::vector<Line*>::iterator it;

		for(it = myLines.begin(); it != myLines.end(); it++)
		{
			Line* l = (*it);
			l->SetPlane(myPlane);
			theMainView.UpdatePlane(l);
		}
	}
#endif
	
}

void EntityImage::ResetLines()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	if(mySprite)
		mySprite->ResetLines();

#else
	if (myLines.empty())
		return;

	std::vector<Line*>::iterator it;

	for(it = myLines.begin(); it != myLines.end(); it++)
	{
		theMainView.Remove(*it);
		delete (*it);
	}
	myLines.clear();
#endif
}

void EntityImage::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stipplestartAt/* = 0*/) 
{
	
#ifdef C2D_DIRECT_DISPLAY_LIB
	//_ASSERT(false);
	//gtb!
	if (mySprite)
	{
		int wx1 = x1;
		int wy1 = y1;
		int wx2 = x2;
		int wy2 = y2;

		theMainView.WorldToScreen(wx1,wy1);
		theMainView.WorldToScreen(wx2,wy2);

		mySprite->AddLine(wx1,wx2,wy1,wy2,lineColourRed,lineColourGreen,lineColourBlue);
	}
#else

	Line* line = theMainView.NewLine(x1,y1,x2,y2,lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stipplestartAt,theApp.GetLinePlane());

	if (line)
		myLines.push_back(line);
#endif





}


bool EntityImage::ResetMainGallery(FilePath &name)
{
	// if there is no name then we are swapping in the
	// normal default gallery
	if(name.empty())
	{
		name = myDefaultGalleryName;
		myCurrentImageIndex = mySavedImageIndexState;
	}
	else
	{
		// we are replacing the normal gallery
		// save the last index
		mySavedImageIndexState = myCurrentImageIndex;
	}
	Unlink();
	myCurrentImageIndex = myDefaultBaseImage;
	myCurrentBaseImageIndex = myDefaultBaseImage;
	bool ok= 	SetGallery(name,myDefaultBaseImage);
	Link();
	return ok;
}

bool EntityImage::RemoveGallery()
{
	Unlink();

	#ifdef C2D_DIRECT_DISPLAY_LIB
		//_ASSERT(false);
		return false;
	#else
		mySprite->DestroyPrimaryGallery();
	#endif
	return true;

}

void EntityImage::ReloadGallery(FilePath const& galleryName, 
							 uint32 baseimage,
							 uint32 numImages /*= 0*/)
{
	myCurrentBaseImageIndex = baseimage;
	SetGallery(galleryName,baseimage,numImages);
}


void EntityImage::SetPosition(int32 x, int32 y)
{
	myWorldPosition.SetX(x);
	myWorldPosition.SetY(y);

	#ifdef C2D_DIRECT_DISPLAY_LIB

	int wx = x;
	int wy = y;

	theMainView.WorldToScreen(wx,wy);


	if(mySprite)
	{
		mySprite->m_PosX = wx;
		mySprite->m_PosY = wy;
	}

	#else
		RECT bound;
		GetBound(bound);
		mySprite->SetCurrentBound(&bound);
	#endif
}

bool EntityImage::Visible(int32 scope)
{
	RECT r;
	GetBound(r);
	return theMainView.Visible(r,scope);
}

void EntityImage::PrepareForReloadingGallery()
{
	Unlink();
								
}

bool EntityImage::IsMirrored()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		//_ASSERT(false);
		return false;
	#else
		return mySprite->IsMirrored();
	#endif
}

void EntityImage::DrawMirrored(bool mirror)
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	if(mySprite)
	{
		if(mirror)
		{
			mySprite->m_BltType = C2DBLT_TRANSHFLIP;
		}
		else
		{
			// we might be sacled in which case we leave it as it is
			if(!myScaledFlag)
				mySprite->m_BltType = C2DBLT_TRANS;
		}
	}

#else
	mySprite->DrawMirrored(mirror);
#endif
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void EntityImage::DrawFlippedVertically(bool flip)
{
	if(mySprite)
	{
		if(flip)
		{
			mySprite->m_BltType = C2DBLT_TRANSVFLIP;
		}
		else
		{
			// we might be scaled in which case we leave it as it is
			if(!myScaledFlag)
				mySprite->m_BltType = C2DBLT_TRANS;
		}
	}
}

void EntityImage::SetSpriteRotationAngle(uint16 angle)
{
	if(angle > 0)
	{
			mySprite->m_BltType = C2DBLT_TRANSROTATED;
			
	}
	else
	{
		// we might be scaled in which case we leave it as it is
		if(!myScaledFlag)
			mySprite->m_BltType = C2DBLT_TRANS;
	}

	mySprite->m_Angle = angle;
}

uint16 EntityImage::GetSpriteRotationAngle()
{
	return mySprite->m_Angle;
}

void EntityImage::DrawOutlineAroundImage(int red, int green, int blue)
{
	// the direct display engine doesn't care about pixel format
		
	uint16 lineColour;
	RGB_TO_565(red, green, blue, lineColour);
	myAniStrip->DrawOutlineAroundImage(lineColour);
}

#endif

int32 EntityImage::GetAbsoluteBaseImage()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		//_ASSERT(false);
		return -1;
	#else
		return myAbsoluteBaseImage;
	#endif
}

bool EntityImage::ValidatePose(int pose)
{
	int numBitmaps = 0;

#ifdef C2D_DIRECT_DISPLAY_LIB
	numBitmaps = myNumberOfImages;
#else
numBitmaps =  mySprite->GetBitmapCount();
#endif

	if (pose < 0 || myCurrentBaseImageIndex < 0)
		return false;

	if ((myCurrentBaseImageIndex + pose) >= myAbsoluteBaseImage + myNumberOfImages)
		return false;

	if ((myCurrentBaseImageIndex + pose) >= numBitmaps)
		return false;
	
	return true;
}


std::string EntityImage::GetGalleryName()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	std::string filename = myFilePath.GetFileName();
	int x = filename.find_last_of(".");

	if(x != -1)
	{
		filename = filename.substr(0,x);
	}

	return filename;

#else
	return mySprite->GetGalleryName();
#endif
}

uint32 EntityImage::GetGalleryBitmapCount()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		return myNumberOfImages;
	#else
		return mySprite->GetBitmapCount();
	#endif
}

void EntityImage::RecolourGallery(const uint16* tintTable)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	
	mySprite->m_AniStrip->TintSurface(tintTable,65535);

	#else
		mySprite->RecolourPrimaryGallery(tintTable);
	#endif
}

void EntityImage::Scale(float scaleBy, bool on)
{
	if(!mySprite)
		return;

#ifdef C2D_DIRECT_DISPLAY_LIB
	if(on && scaleBy > 0.0f)
	{
		mySprite->m_ScaleFactor = scaleBy;
		mySprite->m_BltType = C2DBLT_TRANSSCALED;
		myScaledFlag  = true;
	}
	else
	{
		mySprite->m_BltType = C2DBLT_TRANS;
		myScaledFlag = false;
	}
#else

#endif
}

void EntityImage::DrawAlphaBlended(int alphavalue, bool on)
{
	if(!mySprite)
		return;

#ifdef C2D_DIRECT_DISPLAY_LIB
	if(on)
	{
		mySprite->m_BltType = C2DBLT_TRANSALPHA;
		mySprite->m_AlphaValue = alphavalue;
	}
	else
	{
		mySprite->m_BltType = C2DBLT_TRANS;
	}
#else
	myDrawAlphaFlag = on;
	myAlphaIntensity = alphavalue;
	mySprite->DrawAlphaBlended(on, alphavalue);
#endif
}

void EntityImage::DrawShadowed(int shadowvalue, int x, int y, bool on)
{
	if(!mySprite)
		return;

#ifdef C2D_DIRECT_DISPLAY_LIB
	if(on)
	{
		mySprite->m_BltType = C2DBLT_TRANSSHADOW;
		mySprite->m_ShadowValue = shadowvalue;
		mySprite->m_ShadowOffsetX = x;
		mySprite->m_ShadowOffsetY = y;
	}
	else
	{
		mySprite->m_BltType = C2DBLT_TRANS;
	}
#endif
}

void EntityImage::DrawToNewWidthAndHeight(int newwidth, int newHeight, bool on)
{
	if(!mySprite)
		return;

#ifdef C2D_DIRECT_DISPLAY_LIB
	if(on)
	{
		if(newwidth == 0)
		{
			newwidth = myCurrentWidth;
		}

		if(newHeight ==  0)
		{
			newHeight = myCurrentHeight;
		}

		mySprite->m_BltType = C2DBLT_TRANSSTRETCHED;
		mySprite->m_StretchedWidth = newwidth;
		mySprite->m_StretchedHeight = newHeight;
	}
	else
	{
		mySprite->m_BltType = C2DBLT_TRANS;
	}
#endif
}

#ifndef C2D_DIRECT_DISPLAY_LIB
void EntityImage::UpdateOverlayMirroring(uint32 layer, bool yesOrNo)
{
	if(!mySprite)
		return;

	mySprite->DrawMirrored(layer,yesOrNo);
}



#endif

