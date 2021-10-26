#include "CameraSprite.h"
#include "../Display/DisplayEngine.h"
#include <fstream>


CameraSprite::CameraSprite():
mySurface(0),
myOriginalViewToPhysicalHeightRatio(0.0f),
myOriginalViewToPhysicalWidthRatio(0.0f)
{
	myCurrentBound.top = 0;
	myCurrentBound.left = 0;
	myCurrentBound.bottom = 4;
	myCurrentBound.right = 4;

	// default order of backgrounds
	myBackgrounds.push_back(&myBackground0);
	myBackgrounds.push_back(&myBackground1);
	SetInterestLevel(false);

	myEntityHandler.SetOwner(this);
	myAmIACameraFlag = true;

}

CameraSprite::~CameraSprite()
{
	if(mySurface)
		DisplayEngine::theRenderer().ReleaseSurface(mySurface);
}

bool CameraSprite::Create(std::string defaultBackground,
			int32 topLeftXofBackground, // top left world co-ordinates 
			int32 topLeftYofBackground,
			Position worldPosition)
{

	myEntityHandler.SetWorldPosition(worldPosition);
	return myBackgrounds[0]->Create(defaultBackground,
							Position(topLeftXofBackground,
							topLeftYofBackground),
							worldPosition);

}

bool CameraSprite::Create(std::string gallery_name,
							RECT& bounds,
							Position worldPosition)
{

	myEntityHandler.SetWorldPosition(worldPosition);
	return myBackgrounds[0]->Create(gallery_name,
							bounds,
							worldPosition);
}

bool CameraSprite::Create(uint8 whichBackground,
						  Position worldPosition,
				std::string & gallery_name,
				Position topLeft,
				uint32 width,
				uint32 height)
{
	_ASSERT(whichBackground <= 1);
	
	myCurrentBound.top = 0;
	myCurrentBound.left = 0;
	myCurrentBound.bottom = height;
	myCurrentBound.right = width;

	myEntityHandler.SetWorldPosition(worldPosition);
 return myBackgrounds[whichBackground]->Create( worldPosition,
				gallery_name,
				 topLeft,
				 width,
				 height);
}

void CameraSprite::SwapBackgrounds()
{
		Background* background0 = myBackgrounds[0];
		Background* background1 = myBackgrounds[1];
	
	if(myBackgrounds[0] == &myBackground0)
	{
		myBackgrounds[0] = background1;
		myBackgrounds[1] = background0;
	}
	else
	{
		myBackgrounds[0] = background0;
		myBackgrounds[1] = background1;
	}
}

void CameraSprite::Update(Position worldPosition,
						  bool completeRedraw,
						  bool justBackBuffer)
{
	myEntityHandler.Update(worldPosition);

	Background *background = myBackgrounds[0];
	background->SetDisplayPosition(worldPosition);

	// the individual cameras should only draw to the back buffer
	DisplayEngine::theRenderer().Update(background,&myEntityHandler,
	   completeRedraw,justBackBuffer,mySurface);
}

void CameraSprite::SetViewArea(RECT& area)
{
	myViewArea.top = area.top;
	myViewArea.bottom =area.bottom;
	myViewArea.left = area.left;
	myViewArea.right =area.right;
}


void CameraSprite::GetViewArea(RECT& rect)
 {
    rect.top = myViewArea.top;
    rect.bottom = myViewArea.bottom;
    rect.left = myViewArea.left;
    rect.right = myViewArea.right;
 }


bool CameraSprite::SaveAsSpriteFile( std::string const &filename ) const
{
#ifndef C2E_SDL

	DDSURFACEDESC2	surfaceDescription;
	surfaceDescription.dwSize = sizeof( DDSURFACEDESC2 );

	if( mySurface->Lock( NULL, &surfaceDescription, DDLOCK_WAIT, NULL ) == DD_OK )
	{
		uint16* sourcePtr = (uint16*)surfaceDescription.lpSurface;
		int32 sourceStep = surfaceDescription.lPitch >> 1;
		uint16 width = surfaceDescription.dwWidth;
		uint16 height = surfaceDescription.dwHeight;
		int32 byteWidth = width << 1;

		std::ofstream stream( filename.c_str(), std::ios::binary | std::ios::out );

		uint32 pixelFormat, pixelFlag = 0;
		DisplayEngine::theRenderer().GetPixelFormat( pixelFormat );
		if( pixelFormat == RGB_565 ) pixelFlag = 1;
		uint16 count = 1;
		uint32 offset = 4 + 4 + 2 + 2 + 2;
		stream.write( (const char *) &pixelFlag, sizeof( uint32 ) );
		stream.write( (const char *) &count, sizeof( uint16 ) );
		stream.write( (const char *) &offset, sizeof( uint32 ) );
		stream.write( (const char *) &width, sizeof( uint16 ) );
		stream.write( (const char *) &height, sizeof( uint16 ) );
		while( height-- )
		{
			stream.write( (const char *)sourcePtr, byteWidth );
			sourcePtr += sourceStep;
		}
		mySurface->Unlock( NULL );
		return true;
	}
	else
		return false;

#else
	// non-directx
	SDL_LockSurface(mySurface);	
	uint16 *sourcePtr = (uint16*)(mySurface->pixels);
	int32 sourceStep = mySurface->pitch >> 1;
	uint16 width = mySurface->w;
	uint16 height = mySurface->h;
	int32 byteWidth = width << 1;
	std::ofstream stream( filename.c_str(), std::ios::binary | std::ios::out );

	uint32 pixelFormat, pixelFlag = 0;
	DisplayEngine::theRenderer().GetPixelFormat( pixelFormat );
	if( pixelFormat == RGB_565 ) pixelFlag = 1;
	uint16 count = 1;
	uint32 offset = 4 + 4 + 2 + 2 + 2;
	stream.write( (const char *) &pixelFlag, sizeof( uint32 ) );
	stream.write( (const char *) &count, sizeof( uint16 ) );
	stream.write( (const char *) &offset, sizeof( uint32 ) );
	stream.write( (const char *) &width, sizeof( uint16 ) );
	stream.write( (const char *) &height, sizeof( uint16 ) );
	while( height-- )
	{
		stream.write( (const char *)sourcePtr, byteWidth );
		sourcePtr += sourceStep;
	}
	SDL_UnlockSurface(mySurface);

	return true;
#endif
}


void CameraSprite::Draw()
{
	if(!mySurface)
		return;

	Update(myEntityHandler.GetWorldPosition(),true,true);

	RECT tempView;
	tempView.bottom = myViewArea.bottom;
	tempView.top = myViewArea.top;
	tempView.left = myViewArea.left;
	tempView.right = myViewArea.right;

	bool transparencyAware = false;

	RECT displayRect;

	DisplayEngine::theRenderer().GetSurfaceArea(displayRect);

	// clip the screen bound against the displayArea
	int temp =0;

	// clip the camera's physical bound
	if(myScreenBound.top < displayRect.top)
	{
		myScreenBound.top = displayRect.top;
		temp = int((myScreenBound.bottom - myScreenBound.top)/myOriginalViewToPhysicalHeightRatio);
		tempView.top = myViewArea.bottom - temp;
	}

	if(myScreenBound.left < displayRect.left)
	{
		myScreenBound.left = displayRect.left;
		temp = int((myScreenBound.right - myScreenBound.left)/myOriginalViewToPhysicalWidthRatio);
		tempView.left = myViewArea.right - temp;
	}

	if(myScreenBound.right > displayRect.right)
	{
		myScreenBound.right = displayRect.right;
		temp = int((myScreenBound.right - myScreenBound.left)/myOriginalViewToPhysicalWidthRatio);
		tempView.right = myViewArea.left + temp;

	}

	if(myScreenBound.bottom > displayRect.bottom)
	{
		myScreenBound.bottom = displayRect.bottom;		
		temp =  int((myScreenBound.bottom - myScreenBound.top)/myOriginalViewToPhysicalHeightRatio);
		tempView.bottom = myViewArea.top + temp;
	}

	RECT drawFrom;
	drawFrom.top = tempView.top - myViewArea.top;
	drawFrom.left = tempView.left - myViewArea.left;
	drawFrom.right = tempView.right - myViewArea.left;
	drawFrom.bottom = tempView.bottom - myViewArea.top;

	DisplayEngine::theRenderer().BlitToBackBuffer(myScreenBound, 
								  mySurface,
								  drawFrom,
								  transparencyAware);
}

void CameraSprite::SetCurrentBound(RECT* rect /*= NULL*/)
{
	if(rect)
	{
		myCurrentBound.bottom = rect->bottom;
		myCurrentBound.top = rect->top;
		myCurrentBound.left = rect->left;
		myCurrentBound.right = rect->right;

	}

}

void CameraSprite::SetScreenBound(RECT* rect /*= NULL*/)
{
	if(rect)
	{
		myScreenBound.bottom = rect->bottom;
		myScreenBound.top = rect->top;
		myScreenBound.left = rect->left;
		myScreenBound.right = rect->right;

	}

}

void CameraSprite::SetScreenPosition(Position pos)
{
	myScreenBound.top = pos.GetY();
	myScreenBound.left = pos.GetX();
	myScreenBound.bottom = myScreenBound.top + myCurrentBound.bottom - myCurrentBound.top;
	myScreenBound.right = myScreenBound.left + myCurrentBound.right - myCurrentBound.left;
}

bool CameraSprite::SetUpDrawingSurface(float originalwidthratio, float originalheightratio)
{
	myOriginalViewToPhysicalHeightRatio = originalwidthratio;
	myOriginalViewToPhysicalWidthRatio = originalheightratio;
		
	// make it the size of our surface area
	int width = myViewArea.right - myViewArea.left;
	int height = myViewArea.bottom - myViewArea.top;

	RECT drawFrom;
	drawFrom.top =0;
	drawFrom.left =0;
	drawFrom.right = width;
	drawFrom.bottom =height;

	if(mySurface)
	{
		DisplayEngine::theRenderer().ReleaseSurface(mySurface);
	}

	// get the display engine to create a surface the size of my view rect
	// for me
	mySurface = DisplayEngine::theRenderer().
					CreateSurface(drawFrom.right,
					drawFrom.bottom);
	ASSERT(mySurface);
	
	return (mySurface == NULL ? false : true);
}

bool CameraSprite::Visible( RECT& test)
{
	return myEntityHandler.IsRectOnScreen(myCurrentBound,test);
}

