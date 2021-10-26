// --------------------------------------------------------------------------
// Filename:	MainCamera.cpp
// Class:		MainCamera
// Purpose:		This class provides a view of the world to clients.
//				The MainCamera has a position in world coordinates.
//				The interface will give te client the usual camera
//				operations e.g. panning, tracking. 
//
//				The MainCamera has a window inside it which takes care of
//				the windows messages.  It needs to have the name of
//				the background it displays and a list of entities.
//				It may also have a list of cameras each of which
//				draw to the display engine owned by the main window.
//				
//				
//
// Description: All entities are stored in myEntities.  At the start all objects  
//				are on the free list.  From there they can be put on the 
//				update list.
//
//				There is only ever one mainview which gets created the
//				first time someone tries to access it.
//				The main view can be disabled which will suspend the
//				display engine.
//			
//				
//
// History:
// -------  
// 11Nov98	Alima			Created.
//
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MainCamera.h"
#include "../Agents/PointerAgent.h"
#include "RemoteCamera.h"
#include "../Display/DisplayErrorConstants.h"
#include "../AgentManager.h"

#ifdef C2D_DIRECT_DISPLAY_LIB
#include "../AgentDisplay/EntityImage.h"
#include "../AgentDisplay/EntityImageClone.h"
#else
	#include "CameraSprite.h"
	#include "../Display/DisplayEngine.h"
#endif // C2D_DIRECT_DISPLAY_LIB

#include "../Display/ErrorMessageHandler.h"


#ifdef _WIN32
	#include "../RegistryHandler.h"
#else
	#include <signal.h>
	#include <errno.h>
#endif

#ifndef C2E_SDL
	#include "../Display/DirectX/Window.h"
#endif

#include "../World.h"
#include "../FilePath.h"
#include "../DirectoryManager.h"
#include "../Display/Line.h"
#include "../App.h"
#include	"MapImage.h"

#include "../Display/Window.h"

// serialize out as many members as you need to recreate yourself
CREATURES_IMPLEMENT_SERIAL( MainCamera )

#ifdef C2D_DIRECT_DISPLAY_LIB
bool MainCamera::myC2DEngineRunningFlag = false;
#endif

MainCamera::MainCamera()
:myFullScreenFlag(0),
myShutDownFlag(1)
{

#ifdef C2D_DIRECT_DISPLAY_LIB

//	_ASSERT(false);
#else
	DisplayEngine::theRenderer().
		SetFlags(DISPLAY_BACKGROUND|DISPLAY_SPRITES);
#endif
	

}

MainCamera::~MainCamera()
{

}



bool MainCamera::StartUp(int32 viewx, // world x co-ordinate of view
						 int32 viewy, // world y co-ordinate of view
						 bool fullScreen,
						 std::string& defaultBackground,
						 uint32 topLeftXofBackground, // top left world co-ordinates 
						 uint32 topLeftYofBackground, // of background
						   uint32 storedDisplayType,
						std::vector<std::string>& foldersToConvert)
{	
	myWorldPosition.SetX(viewx);
	myWorldPosition.SetY(viewy);	

	// check that the position is within the bounds of the
	myFullScreenFlag = fullScreen;

#ifdef C2D_DIRECT_DISPLAY_LIB
	SetFullScreenMode(myFullScreenFlag);
#endif	

	StartDisplayEngine( storedDisplayType,
						foldersToConvert);	

	// fully create the camera
	if(!Create(viewx, // world x co-ordinate of view
		viewy, // world y co-ordinate of view
		-1, // width and height of camera not valid for main window
		-1,
		defaultBackground,
		topLeftXofBackground, // top left world co-ordinates 
		topLeftYofBackground))//scale
	{
		return false;
	}

	SetViewAndDisplayArea();

#ifndef C2E_SDL
	// resize the window to its default size here
	RECT rect;

	GetWindowRect(GetDesktopWindow(),&rect);

	int left,top,right,bottom;	
	left = 0;
	top = 0;
	right = SYSTEM_WINDOW_WIDTH;
	bottom = SYSTEM_WINDOW_HEIGHT;

	// Get default window pos from registry
	theApp.UserSettings().Get(std::string("WindowLeft"), left);
	theApp.UserSettings().Get(std::string("WindowTop"), top);
	theApp.UserSettings().Get(std::string("WindowRight"), right);
	theApp.UserSettings().Get(std::string("WindowBottom"),bottom);

	rect.left=left;
	rect.top=top;
	rect.right=right;
	rect.bottom=bottom;

	if((rect.right - rect.left) <= 0 || (rect.bottom - rect.top) <= 0
		|| rect.top > GetSystemMetrics(SM_CYSCREEN) ||
		rect.left > GetSystemMetrics(SM_CXSCREEN))
	{
		rect.left = 0;
		rect.top = 0;
		rect.right = SYSTEM_WINDOW_WIDTH;
		rect.bottom = SYSTEM_WINDOW_HEIGHT;
	}

#endif

// TODO: some window sizing code here maybe?

	myShutDownFlag= false;

#ifndef C2E_SDL
	// don't show the window as the reactive resize call will sort the
	// update out
	ResizeWindow(rect,SWP_HIDEWINDOW);
#endif
	
	return true;
	
}



bool MainCamera::Enable()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	if(myShutDownFlag)
		return false;
	myC2DEngineRunningFlag = true;
#else
	if(myShutDownFlag)
		return false;

	DisplayEngine::theRenderer().ChangeSuspend(true);
#endif
	
	myDisabledFlag = false;
	myChangingRoomsFlag = false;

	Update(true,myLoadingFlag);
	return true;
}

bool MainCamera::Disable()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	if(myShutDownFlag)
		return false;
	myC2DEngineRunningFlag = false;
#else
	if(myShutDownFlag)
		return false;
	myDisabledFlag = true;
	DisplayEngine::theRenderer().ChangeSuspend(false);
#endif

	return false;
}



#ifdef C2E_SDL

void MainCamera::ResizeWindow( int w, int h )
{
	if(myShutDownFlag)
		return;
	DisplayEngine::theRenderer().ResizeWindow(w,h);
	Update(true,myLoadingFlag);
}

#else
// directx version(s)

void MainCamera::ResizeWindow()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	
	if(myShutDownFlag)
		return;

	// make the display rectangle equal to the resized wind
	RECT testRect;
	::GetClientRect(theMainWindow,&testRect);
		
	if(testRect.right - testRect.left <= 0 || testRect.bottom - testRect.top <=0)
	{
		return;
	}
	
	::GetClientRect(theMainWindow,&myDisplayRect);	

#else
	if(myShutDownFlag)
		return;
	
	DisplayEngine::theRenderer().ResizeWindow();
#endif
	Update(true,myLoadingFlag);
}
#endif

void MainCamera::MoveWindow()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	
	if(myShutDownFlag)
		return;
//	_ASSERT(false);
#else
	if(myShutDownFlag)
		return;
	
	DisplayEngine::theRenderer().MoveWindow();
#endif
	Update(true,myLoadingFlag);
}

void MainCamera::MoveWindow(int32 x, int32 y)
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	
	if(myShutDownFlag)
		return;
	//_ASSERT(false);
#else
	if(myShutDownFlag)
		return;
	
	DisplayEngine::theRenderer().MoveWindow(x,y);
#endif

	Update(true,myLoadingFlag);
}


#ifndef C2E_SDL
void MainCamera::ResizeWindow(RECT& rect, UINT flags /* = SWP_SHOWWINDOW*/)
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	
	if(myShutDownFlag)
		return;

//	myDisplayRect.top = rect.top;
//	myDisplayRect.left = rect.left;
//	myDisplayRect.bottom = rect.bottom;
//	myDisplayRect.right = rect.right;

	SetWindowLong(theMainWindow,GWL_STYLE,WS_CAPTION|WS_VISIBLE|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
	
		
	// reset the window size
	SetWindowPos(  theMainWindow,             // handle to window
				   HWND_TOP,  // placement-order handle
				   rect.left,                 // horizontal position
				   rect.top,                 // vertical position 
				   rect.right - rect.left,  // width
				   rect.bottom - rect.top,  // height
				   SWP_FRAMECHANGED |SWP_SHOWWINDOW   // window-positioning flags
				 );

#else
	if(myShutDownFlag)
		return;
	
	DisplayEngine::theRenderer().ResizeWindow(rect);
#endif

	Update(true,myLoadingFlag);
}
#endif

void MainCamera::StartDisplayEngine(  uint32 storedDisplayType,
						std::vector<std::string>& foldersToConvert)
{
	FilePath path( "trans.s16", IMAGES_DIR);

#ifdef C2D_DIRECT_DISPLAY_LIB
	myC2DEngineRunningFlag = true;
#else

		if(!DisplayEngine::theRenderer().Start( storedDisplayType,
					foldersToConvert, path.GetFullPath(), myFullScreenFlag))
		{
	#ifndef _WIN32
			printf( "DisplayEngine::Start() failed\n");
			kill(0, SIGKILL); 
	#else
			// display engine will have displayed a message
			PostMessage(theMainWindow,WM_CLOSE,0,0);
	#endif
		}

	#ifndef C2E_SDL
		// Store the new pixel format in case it has changed
		// Store new display type in registry.
		std::string valueName("Display Type");
		int intDisplayType = storedDisplayType;
		theApp.MachineSettings().Set(valueName, intDisplayType);
		theApp.MachineSettings().Flush();
	#endif
#endif // C2D_DIRECT_DISPLAY_LIB

}

void MainCamera::Render()
{
	// ensure that no rogue rendering gets done
	if(myShutDownFlag)
		return;

	if (myTrackObject.IsInvalid())
		StopTracking();

	// if we are not panning...
	if(!Pan())
	{
		// drag with middle mouse button
		if (!MiddleMouseDragMouseWheel())
		{
			// make sure pointer pos is up to date for this
			thePointer.GetPointerAgentReference().CameraPositionNotify(); 
			// if the pointer isn't push-scrolling the edge of the screen...
			if (!KeepUpThenWithMouse())
			{
				// if we are tracking an object then pan to set it in the centre
				if (!TrackObject())
				{
				}
			}
		}
	}

	DoTrackingChecks();

	std::vector<Camera*>::iterator it;

	if(myMapImage)
		myMapImage->Update();

	// tell your cameras to do some tracking
	for(it = myCameras.begin(); it != myCameras.end(); it++)
	{
		(*it)->Render();
		
	}
	
	bool renderAll;
#ifdef C2D_DIRECT_DISPLAY_LIB	
	if(myShutDownFlag)
		return;
	renderAll = false;
#else
	renderAll = DisplayEngine::theRenderer().ShouldIRenderTheEntireMainCameraOrNot();
#endif

	Update(myCompleteRedraw || renderAll, myLoadingFlag);
	// complete redraw the first frame, and not after that
	myCompleteRedraw = false;
	
	if (myTrackObject.IsValid())
		SetTrackMetaRoom();
}

void MainCamera::ShutDown()
{
	myShutDownFlag = true;
	
#ifndef C2D_DIRECT_DISPLAY_LIB
	myScreen->NotifyCameraIsShuttingDown();
#endif //C2D_DIRECT_DISPLAY_LIB

	#ifndef C2E_SDL
	if (!(theApp.IsAppAScreenSaver() || theApp.GetIsScreenSaverPreview()))
	{
		RECT rect;

#ifdef C2D_DIRECT_DISPLAY_LIB
		::GetWindowRect(theMainWindow,&rect);
#else
		DisplayEngine::theRenderer().GetWindowRect(rect);
#endif //C2D_DIRECT_DISPLAY_LIB

		theApp.UserSettings().Set(std::string("WindowLeft"), rect.left);
		theApp.UserSettings().Set(std::string("WindowRight"), rect.right);
		theApp.UserSettings().Set(std::string("WindowTop"), rect.top);
		theApp.UserSettings().Set(std::string("WindowBottom"), rect.bottom);

		// Don't save maximized
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(theMainWindow, &wp);
		bool maximized = (wp.showCmd == SW_SHOWMAXIMIZED);
		if (maximized)
		{
			theApp.UserSettings().Set(std::string("WindowLeft"), 0);
			theApp.UserSettings().Set(std::string("WindowRight"), SYSTEM_WINDOW_WIDTH);
			theApp.UserSettings().Set(std::string("WindowTop"), 0);
			theApp.UserSettings().Set(std::string("WindowBottom"), SYSTEM_WINDOW_HEIGHT);
		}
	}
	#endif

	// Save fullscreenness
	if (!(theApp.IsAppAScreenSaver() || theApp.GetIsScreenSaverPreview()))
		 theApp.UserSettings().Set(std::string("FullScreen"), myFullScreenFlag);

#ifndef C2D_DIRECT_DISPLAY_LIB
	DisplayEngine::theRenderer().Stop();
#else
	myC2DEngineRunningFlag = false;
#endif //C2D_DIRECT_DISPLAY_LIB

}


// ----------------------------------------------------------------------
// Method:      MakeMapImage 
// Arguments:   entity - an entityImage to make into a fast object
//			
// Returns:     true if the fast object was created OK
//				false otherwise
//
// Description: Creates a fast object out of the entity image.  fast objects
//				have their own direct draw surfaces in video memory and 
//				can draw themselves directly to the front or back buffers
//			
// ----------------------------------------------------------------------
void MainCamera::MakeMapImage()
{

#ifdef C2D_DIRECT_DISPLAY_LIB
	if(myMapImage||myShutDownFlag)
		return;
#else
	if(myMapImage||myShutDownFlag)
		return;
#endif

	try
	{
		myMapImage = new MapImage(9998, myFullScreenFlag);
	}
	catch(MapImage::MapImageException&)
	{
		ErrorMessageHandler::Show(theDisplayErrorTag, 
			(int)sidMapImageNotCreated,
			std::string("MainCamera::MakeMapImage"));
		delete myMapImage;
	}
}

void MainCamera::DisableMapImage()
{

	if(myMapImage)
	{
		myMapImage->Disable();
	}

	Update(true,myLoadingFlag);
}

void MainCamera::EnableMapImage()
{


	if(myMapImage)
	{
		myMapImage->Enable();
	}
}

void MainCamera::ToggleMapImage()
{
	myMapImageDisplayed = !myMapImageDisplayed;
	if(myMapImageDisplayed)
	{
		if(myMapImage)
		myMapImage->Enable();
	}
	else
	{
		if(myMapImage)
		myMapImage->Disable();
		Update(true,myLoadingFlag);
	}
}

bool MainCamera::IsMapDisplayed()
{
	return myMapImageDisplayed;
}

MainCamera& MainCamera::theMainCamera()
{
	static MainCamera ourMainView;
	return ourMainView;
}


RemoteCamera* MainCamera::CreateCamera(RECT& view,
								 RECT& bound,
								 std::string& background,
								 uint32 topLeftXofBackground,
								 uint32 topLeftYofBackground,
								 int32 plane)
{
	RemoteCamera* camera = new RemoteCamera(view.left, // world x co-ordinate of view
								view.top, // world y co-ordinate of view
								view.right-view.left, // width of camera
								view.bottom-view.top, // height of camera
								background,
								topLeftXofBackground, // top left world co-ordinates 
								topLeftYofBackground,
								plane,
								bound);
	
	myCameras.push_back(camera);

	#ifdef C2D_DIRECT_DISPLAY_LIB
//		_ASSERT(false);
	#else
		myScreen->Add(camera->GetSpritePtr());
	#endif

	return camera;
}


void MainCamera::RemoveCamera(Camera* camera)
{
	std::vector<Camera*>::iterator it;

	for(it = myCameras.begin(); it != myCameras.end(); it++)
	{
		if((*it)  == camera)
			break;
	}

	if(it != myCameras.end())
		myCameras.erase(it);

	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	#else
	myScreen->Remove(camera->GetSpritePtr());
	#endif
}

void MainCamera::UpdateCameraPlane(Camera* camera)
{
	// just remove the camera and then let it get entered in it's correct
	// plane
	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	#else
		myScreen->Remove(camera->GetSpritePtr());
		myScreen->Add(camera->GetSpritePtr());
	#endif
}


void MainCamera::AddCamera(Camera* camera)
{
	
	//when we add a new camera we must share our current list of
	// entities
	// but do not draw your self or it could go on forever!!!!
	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	#else
		myScreen->Add(camera->GetSpritePtr());
	#endif

}

void MainCamera::Add(EntityImage* const newEntity)
{
	// add it to all your cameras and your own render list
	Camera::Add(newEntity);
	
}

void MainCamera::Add(DrawableObject* const newEntity)
{
	// add it to all your cameras and your own render list
	Camera::Add(newEntity);
}

Line* MainCamera::NewLine(int x1,
					int y1,
					int x2,
					int y2 ,	 
					uint8 lineColourRed,
					uint8 lineColourGreen,
					uint8 lineColourBlue,
					uint8 stippleon,
					uint8 stippleoff,
					uint32 stipplestartAt,
					int plane)
{
	#ifndef C2D_DIRECT_DISPLAY_LIB
	RECT rect;
	GetViewArea(rect);

	bool ok;

	// NOTE: I think that this view area is one frame behind!
	// This means that the clipping is inaccurate and that the
	// DisplayEngine::DrawLine() call can actually draw off the
	// display and into random memory!
	// This was causing crashes in the Linux (SDL) version. 
	// As a workaround, I got the SDL version of DrawLine to 
	// call ClipLine just before drawing. This means that clipping
	// is done twice which is a bit silly, but it doesn't crash any more.
	// Maybe directX version needs looking at too?
	// BenC - 06aug2000
	ok = DisplayEngine::ClipLine(&rect, x1, y1, x2, y2, 1);
	if (ok)
	{
		if ((x1 != x2) || (y1 != y2))
		{
			int tempx1 = x1;
			int tempx2 = x2;
			int tempy1 = y1;
			int tempy2 = y2;
		
			theMainView.WorldToScreen(tempx1,tempy1);
			theMainView.WorldToScreen(tempx2,tempy2);

			Line* line = new Line(tempx1,tempy1,tempx2,tempy2,
									x1, y1, x2, y2,
									lineColourRed,lineColourGreen,lineColourBlue,
									stippleon,stippleoff,stipplestartAt,plane);
			Camera::Add(line);
			return line;
		}
	}
#endif
	return NULL;
}




void MainCamera::PrepareForMessageBox()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	// be in windowed mode so that we can see the error message
	if (!myC2DEngineRunningFlag)
		return;
	if(myFullScreenFlag)
		SwitchFullScreenMode(false);
#else
	DisplayEngine::theRenderer().PrepareForMessageBox();
#endif
}
void MainCamera::EndMessageBox()
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	// go back to full screen mode
	if (!myC2DEngineRunningFlag)
		return;
	if(myFullScreenFlag)
		SwitchFullScreenMode(true);
#else
	DisplayEngine::theRenderer().EndMessageBox();
#endif
	
	Update(true,myLoadingFlag);
}

// ----------------------------------------------------------------------
// Method:      ChangeMetaRoom
// Arguments:   backgroundname - new backgrounds name
//			
// Returns:     None
// Description: Switch the backgrounds so that the current one is now the
//				outgoing background.  Create the incoming background
//			
// ----------------------------------------------------------------------
void MainCamera::ChangeMetaRoom(std::string& backgroundName	,
		RECT& bounds,
			int32 newviewx,
			int32 newviewy,
			int32 flip,
			bool bCentre,
			int32 viewWidth, /* = -1*/
			int32 viewHeight/*=-1*/)
{
#ifndef C2D_DIRECT_DISPLAY_LIB
	RECT oldDisplayRect = myDisplayRect;

	bool smallBefore = (myScreen->GetPixelWidth(0) <= oldDisplayRect.right)
		|| (myScreen->GetPixelHeight(0) <= oldDisplayRect.bottom);
#endif

	Camera::DoChangeMetaRoom(backgroundName,bounds,newviewx,newviewy,false,bCentre);
#ifndef C2D_DIRECT_DISPLAY_LIB
	// See if the display area has changed
	if ((oldDisplayRect.left != myDisplayRect.left)
		|| (oldDisplayRect.top != myDisplayRect.top)
		|| (oldDisplayRect.right != myDisplayRect.right)
		|| (oldDisplayRect.bottom != myDisplayRect.bottom))
	{
		// This tests to see if we've moved into a meta room which is smaller
		// than the display area.  If we have, we give a resize event as agents
		// floating on the right/bottom of the screen need to move out of the
		// black gutter.
		bool smallAfter = (myScreen->GetPixelWidth(0) <= myDisplayRect.right)
			|| (myScreen->GetPixelHeight(0) <= myDisplayRect.bottom);

		// If we're changed to a small metaroom, always refresh.  If we've
		// changed to a large metaroom only refresh the first time (i.e.
		// when we've changed _from_ a small metaroom)
		if (smallAfter || smallBefore)
			theApp.WindowHasResized();
	}
#endif
	// Effects
	Flip(flip);
}

// ----------------------------------------------------------------------
// Method:      ToggleFullScreenMode 
// Arguments:   None
//			
// Returns:     true if changed OK
//				false otherwise
//
// Description: On each request it flips the engine between fullscreen
//				and windowed mode.
//			
// ----------------------------------------------------------------------
bool MainCamera::ToggleFullScreenMode()
{
	bool ok = true;
#ifdef C2D_DIRECT_DISPLAY_LIB
	if (!myC2DEngineRunningFlag)
		return false;
	myFullScreenFlag = !myFullScreenFlag;
	SwitchFullScreenMode(myFullScreenFlag);
	EntityImage::RefreshAllAnimStrips();
	ClonedEntityImage::RefreshAllAnimStrips();
	theAgentManager.RestoreAllTints();
#else
	ok = DisplayEngine::theRenderer().ToggleFullScreenMode();
	myFullScreenFlag = DisplayEngine::theRenderer().IsFullScreen();
#endif
	return ok;
}

bool MainCamera::Visible( RECT& rect, int32 scope)
{
	bool onCamera = Camera::Visible(rect);

	if(onCamera)
	{
		return true;
	}

	return false;
}

void MainCamera::UpdatePlane(EntityImage* entityImage)
{
	Remove(entityImage,false);//don't stop tracking if you were being tracked
	Add(entityImage);		// as I am adding you in right now
}

void MainCamera::UpdatePlane(DrawableObject* drawableObject)
{
	Remove(drawableObject);
	Add(drawableObject);
}

// ----------------------------------------------------------------------
// Method:      Remove
// Arguments:   newEntity - pointer to new entity to chop from the
//				list
// Returns:     true if the entity was found and removed false otherwise
// Description: This removes the given entityImage from the current update
//				list.
//			
// ----------------------------------------------------------------------
bool MainCamera::Remove(EntityImage* const chop,bool stopTracking/*=true*/)
{
	return	Camera::Remove(chop,stopTracking);
}


// ----------------------------------------------------------------------
// Method:      Remove
// Arguments:   newEntity - pointer to new entity to chop from the
//				list
// Returns:     true if the entity was found and removed false otherwise
// Description: This removes the given entityImage from the current update
//				list.
//			
// ----------------------------------------------------------------------
bool MainCamera::Remove(DrawableObject* const chop)
{
	return Camera::Remove(chop);
}

bool MainCamera::Write(CreaturesArchive &ar) const
{
	if(Camera::Write(ar))
	{

		int size = myCameras.size();
		ar << size;

		std::vector<Camera*>::const_iterator it;

		const Camera* camera = NULL;

		for(it = myCameras.begin(); it != myCameras.end(); it++)
		{
			camera = (*it);
			ar << camera;
		}
		return true;
	}

	return false;

}

bool MainCamera::Read(CreaturesArchive &ar)
{
			
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

	if(!Camera::Read(ar))
		return false;
	

		Camera* camera =NULL;
		int size=0;
		ar >> size;


		for(int i= 0; i < size; i++)
		{
			ar >> camera;
			myCameras.push_back(camera);
		}
	}
	else
	{
		_ASSERT(false);
		return false;

	}

	return true;
	
}


bool MainCamera::GetPixelFormat(uint32& format)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
	//	_ASSERT(false);
	format = 2;
		return true;
	#else
		return DisplayEngine::theRenderer().GetPixelFormat(format);
	#endif
}

void MainCamera::MakeTheEntityHandlerResetBoundsProperly()
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
//		_ASSERT(false);
	#else
		myScreen->MakeSetCurrentBoundsGetCalled();
	#endif
}

void MainCamera::SetViewAndDisplayArea()
{
	Camera::SetViewAndDisplayArea();
}

bool MainCamera::IsFullScreen()
{
	return myFullScreenFlag;
}

