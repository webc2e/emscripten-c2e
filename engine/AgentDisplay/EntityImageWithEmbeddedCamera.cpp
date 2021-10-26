#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "EntityImageWithEmbeddedCamera.h"
#include "../Camera/MainCamera.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Display/DisplayErrorConstants.h"

#ifdef C2D_DIRECT_DISPLAY_LIB
#include <C2D.h>
#include <C2DScreen.h>
#include <C2DSprite.h>
#else
	#include "../Display/DisplayEngine.h"
#endif // C2D_DIRECT_DISPLAY_LIB

#include "../Camera/RemoteCamera.h"

//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( EntityImageWithEmbeddedCamera)

EntityImageWithEmbeddedCamera::EntityImageWithEmbeddedCamera():
myCamera(0)
{}


EntityImageWithEmbeddedCamera::	EntityImageWithEmbeddedCamera(FilePath const& galleryName,
								int32 numimages,
								int32 plane,//plane of entityimage camera will be on pixel behind
								int32 worldx, 
								int32 worldy, 
								uint32 baseimage,					
								uint32 currentimage,
								uint32 viewWidth,
								uint32 viewHeight,
								uint32 cameraWidth,
								uint32 cameraHeight):
EntityImage(galleryName,
			numimages,
			plane,
			worldx, 
			worldy, 
			baseimage,					
			currentimage )	
{
	// we need both the view and the world position of the rectangle
	RECT view;
	view.top = worldy;
	view.bottom = view.top + viewHeight;
	view.left = worldx;
	view.right = view.left + viewWidth;

	RECT bound;
	bound.top = worldy;
	bound.bottom = bound.top + cameraHeight;
	bound.left = worldx;
	bound.right = bound.left + cameraWidth;
	std::string emptystring;
	try
	{
// create a camera for yourself
	myCamera = theMainView.CreateCamera(view,
										bound,
								 emptystring,// would be background file
								 0,// top left coordinate of background
								 0,// these values are default and should be reset
								 plane - 1); // by the meta command later
	}
	catch(Camera::CameraException& e)
	{
		ErrorMessageHandler::Show(e, std::string("EntityImageWithEmbeddedCamera::EntityImageWithEmbeddedCamera"));
		
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
								(int)sidNoSecondaryCameraCreated,
								std::string("EntityImageWithEmbeddedCamera::EntityImageWithEmbeddedCamera"));

	
		throw EntityImageException(string,__LINE__);
	}

}

EntityImageWithEmbeddedCamera::~EntityImageWithEmbeddedCamera()
{
	theMainView.RemoveCamera(myCamera);
	delete myCamera;
	myCamera = NULL;
}


void EntityImageWithEmbeddedCamera::AdjustXY(int32 x, int32 y)
{
	EntityImage::AdjustXY(x,y);

}

void EntityImageWithEmbeddedCamera::SetPlane(int plane)
{
	EntityImage::SetPlane(plane);

	myCamera->SetPlane(plane);

	theMainView.UpdateCameraPlane(myCamera);
}


void EntityImageWithEmbeddedCamera::SetPosition(int32 x, int32 y)
{
	EntityImage::SetPosition(x,y);
	myCamera->PhysicallyMoveToWorldPosition(x,y);
}

bool EntityImageWithEmbeddedCamera::Write(CreaturesArchive& archive) const
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
		return NULL;
	#else
	EntityImage::Write(archive);

	archive << myCamera;
	#endif

	return true;
}

bool EntityImageWithEmbeddedCamera::Read(CreaturesArchive& archive) 
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
		return NULL;
	#else
		
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!EntityImage::Read(archive))
			return false;

		archive >> myCamera;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success
	theMainView.AddCamera(myCamera);
#endif

	return true;
}

