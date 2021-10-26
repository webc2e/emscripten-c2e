// --------------------------------------------------------------------------
// Filename:	MainCamera.h
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
//				There is only ever one main camera it belongs to no-one but
//				can be enable or disabled by anyone
//			
//				
//
// History:
// -------  
// 11Nov98	Alima			Created.
// --------------------------------------------------------------------------
#ifndef		DX_MAINCAMERA_H
#define		DX_MAINCAMERA_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../Display/Position.h"
#include "Camera.h"

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif
#include <vector>
#include <string>

// make life easier when accessing the main view
#define theMainView MainCamera::theMainCamera()

class RemoteCamera;
class DrawableObject;

class MainCamera: public Camera 
{
	CREATURES_DECLARE_SERIAL( MainCamera )

public:
	bool IsFullScreen();

	// constructors are private to ensure that only one
	// Main camera exists

	~MainCamera();



// ----------------------------------------------------------------------
// Method:      StartUp
// Arguments:  viewx - world x co-ordinate of view
//				viewy - world y co-ordinate of view
//				width -  width of camera
//				height -  height of camera
//				backgroundName - background file to use
//				topLeftXofBackground - top left world co-ordinates 
//				topLeftYofBackground) - of background as a meta room
// Returns:     true if the camera was created OK false otherwise
// Description: Used in two stage creation of camera it's possible to
//				run the world with the camera disabled
//			
// ----------------------------------------------------------------------

	bool StartUp(int32 viewx, // world x co-ordinate of view
					int32 viewy, // world y co-ordinate of view
					bool fullScreen,
					std::string& defaultBackground,
					uint32 topLeftXofBackground, // top left world co-ordinates 
					uint32 topLeftYofBackground, // of background
					  uint32 storedDisplayType,
						std::vector<std::string>& foldersToConvert);

	bool Enable();
	bool Disable();

	// new serialization stuff
	virtual bool Write( CreaturesArchive &ar ) const;
	virtual bool Read( CreaturesArchive &ar );


// ----------------------------------------------------------------------
// Method:      CreateCamera 
// Arguments:   x,y world position and size of the  
//				
//
// Returns:     Pointer to the new camera
// Description: This creates a new camera
//				
// ----------------------------------------------------------------------
	RemoteCamera* CreateCamera(RECT& view,
								 RECT& bound,
							  std::string& background,
							  uint32 topLeftXofBackground,
							  	uint32 topLeftYofBackground,
							  int32 plane);

	void RemoveCamera(Camera* camera);

	virtual void ChangeMetaRoom(std::string& backgroundName,	
		RECT& rect,
			int32 newviewx,
			int32 newviewy,
			int32 flip,
			bool bCentre,
			int32 viewWidth = -1,
			int32 viewHeight=-1);

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
bool ToggleFullScreenMode();


// ----------------------------------------------------------------------
// Method:      Render 
// Arguments:   None
//
// Returns:     None
// Description: Updates the screen
//				
// ----------------------------------------------------------------------
	void Render();

// ----------------------------------------------------------------------
// Method:      ScreenToWorld 
// Arguments:   x - screen x position
//				y - screen y position
//
// Returns:     None
// Description: Converts screen coordinates to world coordinates
//				
// ----------------------------------------------------------------------

	void StartDisplayEngine(  uint32 storedDisplayType,
						std::vector<std::string>& foldersToConvert);

	void ShutDown();

	void MakeMapImage();
	void EnableMapImage();
	void DisableMapImage();
	void ToggleMapImage();
	bool IsMapDisplayed();
#ifdef C2E_SDL
	void ResizeWindow( int w, int h );
#else
	void ResizeWindow();
	void ResizeWindow(RECT& rect,UINT flags = SWP_SHOWWINDOW);
#endif

	void MoveWindow();
	void MoveWindow(int32 x, int32 y);
	int32 GetViewWidth();
	int32 GetViewHeight();

	bool GetPixelFormat(uint32& format);

	void PrepareForMessageBox();
	void EndMessageBox();

	void UpdateCameraPlane(Camera* camera);
	void AddCamera(Camera* camera);
	virtual void Add(EntityImage* const newEntity);
	void Add(DrawableObject* const newEntity);

	Line* NewLine(int x1,
					int y1,
					int x2,
					int y2 ,	 
					uint8 lineColourRed,
					uint8 lineColourGreen,
					uint8 lineColourBlue,
					uint8 stippleon,
					uint8 stippleoff,
					uint32 stipplestartAt,
					int plane);



	bool Visible( RECT& rect, int32 scope);


	// ----------------------------------------------------------------------
	// Method:      Remove
	// Arguments:   newEntity - pointer to new entity to chop from the
	//				list
	// Returns:     true if the entity was found and removed false otherwise
	// Description: This removes the given entityImage from the current update
	//				list.
	//			
	// ----------------------------------------------------------------------
	virtual bool Remove(EntityImage* const chop,bool stopTracking = true);
	virtual bool Remove(DrawableObject* const chop);
	virtual void UpdatePlane(EntityImage* entityImage);
	virtual void SetViewAndDisplayArea();
	void UpdatePlane(DrawableObject* drawableObject);

	void MakeTheEntityHandlerResetBoundsProperly();

	// ----------------------------------------------------------------------
	// Method:      theMainCamera 
	// Arguments:   None
	//
	// Returns:     reference to the only main camera object
	// Description: The onlyway to access the main camera
	//				
	// ----------------------------------------------------------------------
	static MainCamera& theMainCamera();

private:
	// Private constructors to ensure only one Main Camera exists
	MainCamera();

	// Copy constructor and assignment operator
	// Declared but not implemented
	MainCamera (const MainCamera&);
	MainCamera& operator= (const MainCamera&);

	bool myFullScreenFlag;

	std::vector<Camera*> myCameras;

	bool myShutDownFlag;

	#ifdef C2D_DIRECT_DISPLAY_LIB
		static bool myC2DEngineRunningFlag;
	#endif
};

#endif		// DX_MAINCAMERA_H

