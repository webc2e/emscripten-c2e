// --------------------------------------------------------------------------
// Filename:	Camera.cpp
// Class:		Camera
// Purpose:		This class provides a view of the world to clients.
//				The camera has a position in world coordinates.
//				The interface will give the client the usual
// camera 				operations e.g. panning, tracking.
//
//				Each camera must keep track of the of its world
// coordinates 				and the agents.
//
//
//
//				The camera keeps two backgrounds one incoming
// and one
// outgoing 				so that if a meta room change is
// requested the the incoming 				 background is loaded in
// and the outgoing background is kept in case there is an error in loading and
// we need to go back to it. Also the outgoing background serves as a cache so
// that if the last seen meta room in case the user goes back to it immediately.
// And finally it allows you to use fading effects between the two backgrounds
// when going from meta room to meta room.
//
//
// Description: All entities are stored in myEntities.
//
//
//
// History:
// -------
// 11Nov98	Alima			Created.
// 28Feb98					Updated to take a vector
// collection of background
//							names instead of just
// one. Should
// I
// rely on the 							 map always
// being there and just take a reference?
//
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "Camera.h"
#include "../../common/FileFuncs.h"

#ifdef C2D_DIRECT_DISPLAY_LIB
#include "../Caos/CAOSVar.h"
#include <C2DLayer.h>
#include <C2DScreen.h>
#include <C2DSprite.h>
#include <C2DSurface.h>

#else
	#include "../Display/DisplayEngine.h"
	#include "../Camera/CameraSprite.h"
	#include "../Display/Sprite.h"
	#include "../Display/Line.h"
#endif // C2E_OLD_DISPLAY

#include "../AgentDisplay/EntityImage.h"
#include "../Display/ErrorMessageHandler.h"
#include "../InputManager.h"
#include "../App.h"
#include <algorithm>
#include "../World.h"
#include "../Map/Map.h"
#include "../Agents/PointerAgent.h"
#include "../Creature/Creature.h"
#include "../Agents/Agent.h"
#include "../mfchack.h"
#include "../DirectoryManager.h"
#include "../Display/DisplayErrorConstants.h"
#include "../Display/Window.h"
#include "../../common/StringFuncs.h"
#include	"MapImage.h"

#ifndef _WIN32
#include "../unix/KeyScan.h"
#endif

#ifdef C2E_SDL
#include <SDL/SDL.h>
#endif

// serialize out as many members as you need to recreate yourself
CREATURES_IMPLEMENT_SERIAL(Camera)

Camera::Camera(int32 viewx,  // world x co-ordinate of view
               int32 viewy,  // world y co-ordinate of view
               int32 width,  // width of view
               int32 height, // height of view
               std::string &defaultBackground,
               uint32 topLeftXofBackground, // top left world co-ordinates
               uint32 topLeftYofBackground  // of background as a meta room
               )
    : myWorldPosition(viewx, viewy), myScreen(0), myMapImage(0),
      myMapImageDisplayed(0) {
  InitialiseCameraVariables();

  if (!Create(viewx,                // world x co-ordinate of view
              viewy,                // world y co-ordinate of view
              width,                // width of camera
              height,               // height of camera
              defaultBackground,    // background file to use
              topLeftXofBackground, // top left world co-ordinates
              topLeftYofBackground)) {
    // "The display engine is shutting down because the following background
    // file was not created %1."
    std::string string = ErrorMessageHandler::Format(
        theDisplayErrorTag, (int)sidNoBackgroundCreated,
        std::string("Camera::Camera"), defaultBackground.c_str());

    throw(CameraException(string, __LINE__));
  }

  myDisabledFlag = false;
}

Camera::Camera(int32 viewx,  // world x co-ordinate of view
               int32 viewy,  // world y co-ordinate of view
               int32 width,  // width of camera
               int32 height, // height of camera
               std::string &defaultBackground,
               RECT &bounds // of background as a meta room
               )
    : myWorldPosition(viewx, viewy), myScreen(0), myMapImage(0),
      myMapImageDisplayed(0) {
  InitialiseCameraVariables();

  if (!Create(viewx,             // world x co-ordinate of view
              viewy,             // world y co-ordinate of view
              width,             // width of camera
              height,            // height of camera
              defaultBackground, // background file to use
              bounds             // top left world co-ordinates
              )) {

    // "The display engine is shutting down because the following background
    // file was not created %1."
    std::string string = ErrorMessageHandler::Format(
        theDisplayErrorTag, (int)sidNoBackgroundCreated,
        std::string("Camera::Camera"), defaultBackground.c_str());

    throw(CameraException(string, __LINE__));
  }
  myDisabledFlag = false;
}

Camera::Camera()
    : myWorldPosition(0, 0), myScreen(0), myMapImage(0),
      myMapImageDisplayed(0) {
  InitialiseCameraVariables();
}

void Camera::InitialiseCameraVariables() {
#ifdef C2D_DIRECT_DISPLAY_LIB

  myScreen = new C2DScreen();
  myDisplayList = new C2DSpriteList;
  myPicture = NULL;
  myDoWackyScrollFlag = false;
  myFullScreenFlag = true;

#else
  myScreen = new CameraSprite();
#endif

  myObjectToKeepUpWith = NULL;
  myTrackObject = NULLHANDLE;
  myTrackPercentX = 75;
  myTrackPercentY = 75;
  myTrackStyle = 0;
  myTrackInRectangleLastTick = false;
  myTrackPositionLastTick = Position(0, 0);
  myTrackMetaRoomLastTick = -1;
  myTrackTransition = 0;
  myMiddleDrag = false;
  myViewWidth = -1;
  myViewHeight = -1;
  myAcceleration = 4;
  myMinimumSpeed = 4;
  myChangingRoomsFlag = 0;
  myKeepUpSpeedX = 0;
  myKeepUpSpeedY = 0;
  myKeepUpCountX = 0;
  myKeepUpCountY = 0;
  myKeepUpPanX = 0;
  myKeepUpPanY = 0;
  myCompleteRedraw = true;
  myLoadingFlag = false;
  myDisabledFlag = true;
}

Camera::~Camera() {
  delete myScreen;

#ifdef C2D_DIRECT_DISPLAY_LIB
  delete myDisplayList;
  myDisplayList = NULL;
  delete myPicture;
  myPicture = NULL;
#endif

  // all cameras should be deleted by now
  delete myMapImage;
  myMapImage = NULL;

  // everyone should have unregistered themselves from floating by now?
  ASSERT(myFloatingThings.empty());
}

// ----------------------------------------------------------------------
// Method:      Create
// Arguments:  viewx - world x co-ordinate of view
//				viewy - world y co-ordinate of view
//				width -  width of camera
//				height -  height of camera
//				backgroundName - background file to use
//				topLeftXofBackground - top left world
// co-ordinates 				topLeftYofBackground) - of
// background as a meta room Returns:     true if the camera was created OK
// false otherwise Description: Used in two stage creation of camera it's
// possible to
//				run the world with the camera disabled
//
// ----------------------------------------------------------------------
bool Camera::Create(int32 viewx,  // world x co-ordinate of view
                    int32 viewy,  // world y co-ordinate of view
                    int32 width,  // width of camera
                    int32 height, // height of camera
                    std::string &defaultBackground,
                    int32 topLeftXofBackground, // top left world co-ordinates
                    int32 topLeftYofBackground  // of background as a meta room
) {
  myBackground1FilePath;
  if (!defaultBackground.empty()) {
    myBackground1FilePath = MakeBackgroundPath(defaultBackground);

    defaultBackground = myBackground1FilePath.GetFullPath();

    if (!FileExists(defaultBackground.c_str()))
      return false;
  } else {
    // for remote cameras we do not need a background so don't
    // return anything here
  }

#ifdef C2D_DIRECT_DISPLAY_LIB
  SwitchFullScreenMode(myFullScreenFlag);
#else
  if (myScreen)
    myScreen->Create(defaultBackground, topLeftXofBackground,
                     topLeftYofBackground, myWorldPosition);
#endif

  // set default values should be atltered by later
  // commands such as change meta room
  myViewHeight = height;
  myViewWidth = width;

  // find out if the background is smaller than the display width do something
  SetViewAndDisplayArea();
  return true;
}

bool Camera::Create(int32 viewx,  // world x co-ordinate of view
                    int32 viewy,  // world y co-ordinate of view
                    int32 width,  // width of camera
                    int32 height, // height of camera
                    std::string &defaultBackground,
                    RECT &bounds // of background as a meta room
) {
  myBackground1FilePath = MakeBackgroundPath(defaultBackground);

  myViewHeight = height;
  myViewWidth = width;

#ifdef C2D_DIRECT_DISPLAY_LIB
  //	_ASSERT(false);
  return true;
#else
  myScreen->Create(myBackground1FilePath.GetFullPath(), bounds,
                   myWorldPosition);
#endif

  // find out if the background is smaller than the display width do something
  SetViewAndDisplayArea();
  return true;
}

FilePath Camera::MakeBackgroundPath(std::string const &backgroundName) {

#ifdef C2D_DIRECT_DISPLAY_LIB
  return FilePath(backgroundName + ".bmp", BACKGROUNDS_DIR);
#else
  std::string lowerBackground = backgroundName;
  LowerCase(lowerBackground);
  return FilePath(lowerBackground + ".blk", BACKGROUNDS_DIR);
#endif
}

void Camera::DoChangeMetaRoom(std::string &backgroundName, RECT &bounds,
                              int32 newviewx, int32 newviewy,
                              bool useTopLeftOnly, bool bCentre) {

  myChangingRoomsFlag = true;

  myBackground2FilePath = myBackground1FilePath;
  myBackground1FilePath = MakeBackgroundPath(backgroundName);

  // switch the backgrounds so that the current one is now the
  // outgoing background

#ifdef C2D_DIRECT_DISPLAY_LIB
  if (!FileExists(myBackground1FilePath.GetFullPath().c_str())) {
    std::string string = ErrorMessageHandler::Format(
        theDisplayErrorTag, (int)sidNoBackgroundCreated,
        std::string("Camera::Camera"),
        myBackground1FilePath.GetFullPath().c_str());

    throw(CameraException(string, __LINE__));
  }
  myScreen->LoadBitmap(myBackground1FilePath.GetFullPath().c_str());

  delete myPicture;

  myPicture =
      new C2DLayer(myScreen, myBackground1FilePath.GetFullPath().c_str());
  myScreen->Flip();

  myMetaRoomBounds.top = bounds.top;
  myMetaRoomBounds.left = bounds.left;
  // for some reason the map takes one pixel off I'm too scared to change it
  //  in the map so I'll readdress the balance here
  myMetaRoomBounds.right = bounds.right + 1;
  myMetaRoomBounds.bottom = bounds.bottom + 1;
#else
  myScreen->SwapBackgrounds();

  bool ok = false;
  if (useTopLeftOnly) {

    ok = myScreen->Create(myBackground1FilePath.GetFullPath(), bounds.top,
                          bounds.left, myWorldPosition);
  } else {
    ok = myScreen->Create(myBackground1FilePath.GetFullPath(), bounds,
                          myWorldPosition);
  }

  if (!ok) {
    myScreen->SwapBackgrounds();
    return;
  }
#endif

  // if no view set set it to the top left
  if (newviewx == 0 && newviewy == 0) {
    myWorldPosition.SetX(bounds.top);
    myWorldPosition.SetY(bounds.left);
  } else {
    if (bCentre)
      CentriseCoordinates(newviewx, newviewy);
    myWorldPosition.SetX(newviewx);
    myWorldPosition.SetY(newviewy);
  }

  NormaliseWorldPosition();

#ifdef C2D_DIRECT_DISPLAY_LIB
//	_ASSERT(false);
#else
  myScreen->SetDisplayPosition(myWorldPosition);
#endif
}

void Camera::Flip(int32 flip) {
  // if this is not the main camera then don't do any screen transitions
  switch (flip) {
  case NOFLIP: {
    myChangingRoomsFlag = false;
    Update(true, myLoadingFlag);
    break;
  }
  case FLIP: {

    Update(true, true); // full screen, just the back buffer

#ifdef C2D_DIRECT_DISPLAY_LIB

//	_ASSERT(false);
#else
    DisplayEngine::theRenderer().FlipScreenHorizontally();
#endif
    myChangingRoomsFlag = false;
    break;
  }
  case BURST: {
    Update(true, true); // full screen, just the back buffer
#ifdef C2D_DIRECT_DISPLAY_LIB

//	_ASSERT(false);
#else
    DisplayEngine::theRenderer().Burst();

#endif
    myChangingRoomsFlag = false;
    break;
  }
  default: {
    myChangingRoomsFlag = false;
    Update(true, myLoadingFlag);
    break;
  }
  }
}

void Camera::SetBackground(std::string &backgroundName,
                           int32 transitionMethod) {

  RECT bounds;

#ifndef C2D_DIRECT_DISPLAY_LIB
  bounds.top = myScreen->GetTop(0);
  bounds.left = myScreen->GetLeft(0);
  bounds.right = bounds.left + myScreen->GetPixelWidth(0);
  bounds.bottom = bounds.top + myScreen->GetPixelHeight(0);

#endif

  DoChangeMetaRoom(backgroundName, bounds, myWorldPosition.GetX(),
                   myWorldPosition.GetY(),
                   false, // use top left only
                   false);

#ifndef C2D_DIRECT_DISPLAY_LIB
  Flip(transitionMethod);
#endif
}

// ----------------------------------------------------------------------
// Method:      ChangeMetaRoom
// Arguments:   backgroundname - new backgrounds name
//
// Returns:     None
// Description: Switch the backgrounds so that the current one is now the
//				outgoing background.  Create the incoming
// background
//
// ----------------------------------------------------------------------
void Camera::ChangeMetaRoom(std::string &backgroundName, RECT &bounds,
                            int32 newviewx, int32 newviewy, int32 flip,
                            bool bCentre) {

  DoChangeMetaRoom(backgroundName, bounds, newviewx, newviewy, false, bCentre);
  myChangingRoomsFlag = false;

  Update(true, true);
}

// ----------------------------------------------------------------------
// Method:      Add
// Arguments:   newEntity - pointer to new entity to add to the
//				list
// Returns:     None
// Description: This adds the given entityImage to the current update
//				list.
//
// ----------------------------------------------------------------------
void Camera::Add(EntityImage *const newEntity) {
  if (newEntity) {
#ifdef C2D_DIRECT_DISPLAY_LIB

    int x = newEntity->GetWorldX();
    int y = newEntity->GetWorldY();
    WorldToScreen(x, y);

    if (newEntity->GetSpritePtr())
      myDisplayList->AddSprite(newEntity->GetSpritePtr());

#else
    // need to move this to sprite
    myScreen->Add(newEntity->GetSpritePtr());
#endif
  }
}

#ifdef C2D_DIRECT_DISPLAY_LIB
C2DSprite *Camera::AddAnimStrip(C2DAniStrip *strip, int plane) {
  return myDisplayList->AddSprite(strip, plane);
}
#endif

// ----------------------------------------------------------------------
// Method:      Add
// Arguments:   newEntity - pointer to new entity to add to the
//				list
// Returns:     None
// Description: This adds the given entityImage to the current update
//				list.
//
// ----------------------------------------------------------------------
void Camera::Add(DrawableObject *const newEntity) {
  if (newEntity) {
#ifdef C2D_DIRECT_DISPLAY_LIB
    _ASSERT(false);
#else
    myScreen->Add(newEntity);
#endif
  }
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
bool Camera::Remove(EntityImage *const chop, bool stopTracking /*=true*/) {
  // make sure that this isn't the
  // trackobject if so stop tracking
  if ((myTrackObject.IsValid()) &&
      myTrackObject.GetAgentReference().GetEntityImage() == chop &&
      stopTracking) {
    myTrackObject = NULLHANDLE;
  }

#ifdef C2D_DIRECT_DISPLAY_LIB
  myDisplayList->RemoveSprite(chop->GetSpritePtr());
  //		chop->ClearSpritePtr();
  return true;
#else
  return myScreen->Remove(chop->GetSpritePtr());
#endif
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
bool Camera::Remove(DrawableObject *const chop) {
  // make sure that this isn't the
  // trackobject if so stop tracking

#ifdef C2D_DIRECT_DISPLAY_LIB
  _ASSERT(false);
  return false;
#else
  return myScreen->Remove(chop);
#endif
}

// ----------------------------------------------------------------------
// Method:      DoUpdateAfterAdjustments
// Arguments:   None
// Returns:     None
// Description: Does common camera update needed when the camera has been
//				adjusted.
//
// ----------------------------------------------------------------------
void Camera::DoUpdateAfterAdjustments() {
  // Remember we need to do a complete redraw this time
  myCompleteRedraw = true;
}

// ----------------------------------------------------------------------
// Method:      MoveTo
// Arguments:   x - xpositon to move to
//				y - yposition to move to
// Returns:     None
// Description: Resets the camera postion to the given co-ordinates
//
//
// ----------------------------------------------------------------------
void Camera::MoveTo(int32 x, int32 y, int pan) {
  if (pan) {
    PanTo(x, y);
  } else {

    Position pos = myWorldPosition;
    myWorldPosition.SetX(x);
    myWorldPosition.SetY(y);

    NormaliseWorldPosition();

    if (myWorldPosition != pos)
      DoUpdateAfterAdjustments();
  }
}

// ----------------------------------------------------------------------
// Method:      MoveBy
// Arguments:   x - xpositon to move by
//				y - yposition to move by
// Returns:     None
// Description: Adjusts the camera postion to the given co-ordinates
//
//
// ----------------------------------------------------------------------
void Camera::MoveBy(int32 x, int32 y) {
  //	OutputFormattedDebugString("Camera::MoveBy(int32 %d,int32 %d)\n\n", x,
  // y);
  Position testPos = myWorldPosition;

  myWorldPosition.AdjustX(x);
  myWorldPosition.AdjustY(y);
  NormaliseWorldPosition();

#ifdef C2D_DIRECT_DISPLAY_LIB
  if (testPos.GetX() != myWorldPosition.GetX()) {
    if (x < 0)
      myPicture->ScrollLeft(abs(x));
    else if (x > 0)
      myPicture->ScrollRight(x);

    // Commented this out because they were causing a scrolling bug
    //		if(!myDoWackyScrollFlag)
    //			myDisplayList->Scroll(x,0);
  }

  if (testPos.GetY() != myWorldPosition.GetY()) {
    if (y < 0)
      myPicture->ScrollUp(abs(y));
    else if (y > 0)
      myPicture->ScrollDown(y);

    // Commented this out because they were causing a scrolling bug
    //		if(!myDoWackyScrollFlag)
    //			myDisplayList->Scroll(0,y);
  }
#endif

  if (testPos != myWorldPosition)
    DoUpdateAfterAdjustments();
}

// ----------------------------------------------------------------------
// Method:      PanTo
// Arguments:   x - xpositon to pan to
//				y - yposition to pan to
// Returns:     None
// Description: Sets up a pan cycle where the camera will pan each update
//				until it has reached the gievn x,y position
//
//
// ----------------------------------------------------------------------
void Camera::PanTo(int32 x, int32 y) {
  // How far is this from where the camera is now?

  int32 xd, yd;

  // must send even numbers or else
  // pan won't work properly

  NormaliseWorldCoordinates(x, y);

  xd = x - myWorldPosition.GetX();
  yd = y - myWorldPosition.GetY();

  yd -= yd % 4;
  xd -= xd % 4;

  myPanVector.SetX(xd);
  myPanVector.SetY(yd);

  myPanVelocity.Reset();
  myDistancePanned.Reset();
}

bool Camera::Pan() {
  // only pan if we have somewhere we are heading to
  int32 acceleration = 8;
  int32 minimumSpeed = 4;

  if (myPanVector.GetX() == 0 && myPanVector.GetY() == 0) {
    myPanVelocity.Reset();
    myDistancePanned.Reset();
    return false;
  }

  // taken from creatures2 probably could be tidied up
  // will look later
  if (myPanVector.GetX() < 0) // Moving left
  {
    // Stop if we're very near or are going to overshoot
    if (myPanVector.GetX() >= -acceleration ||
        myPanVector.GetX() >= myPanVelocity.GetX()) {
      // Finish last step
      myPanVelocity.SetX(myPanVector.GetX());
    } else {
      // Accelerate if we are up to half way there
      if (myDistancePanned.GetX() > myPanVector.GetX()) {
        myPanVelocity.AdjustX(-acceleration);
      } else {
        // Slow down until we reach minimum speed
        if (myPanVelocity.GetX() + acceleration <= -minimumSpeed) {
          myPanVelocity.AdjustX(acceleration);
        } else {
          myPanVelocity.SetX(-minimumSpeed);
        }
      }
    }
  } else {
    if (myPanVector.GetX() > 0) // Moving right
    {
      // Stop if we're very near or are going to overshoot
      if (myPanVector.GetX() <= acceleration ||
          myPanVector.GetX() <= myPanVelocity.GetX()) {
        // Finish last step
        myPanVelocity.SetX(myPanVector.GetX());
      } else {
        // Accelerate if we are up to half way there
        if (myDistancePanned.GetX() < myPanVector.GetX()) {
          myPanVelocity.AdjustX(acceleration);
        } else {
          // Slow down until we reach minimum speed
          if (myPanVelocity.GetX() - acceleration >= minimumSpeed) {
            myPanVelocity.AdjustX(-acceleration);
          } else {
            myPanVelocity.SetX(minimumSpeed);
          }
        }
      }
    } else {
      // Stop Panning horizontally
      myPanVelocity.SetX(0);
    }
  }

  if (myPanVector.GetY() < 0) // Moving up
  {
    // Stop if we're very near or are going to overshoot
    if (myPanVector.GetY() >= -acceleration ||
        myPanVector.GetY() >= myPanVelocity.GetY()) {
      // Finish last step
      myPanVelocity.SetY(myPanVector.GetY());
    } else {
      // Accelerate if we are up to half way there
      if (myDistancePanned.GetY() > myPanVector.GetY()) {
        myPanVelocity.AdjustY(-acceleration);
      } else {
        // Slow down until we reach minimum speed
        if (myPanVelocity.GetY() + acceleration <= -minimumSpeed) {
          myPanVelocity.AdjustY(acceleration);
        } else {
          myPanVelocity.SetY(-minimumSpeed);
        }
      }
    }
  } else {
    if (myPanVector.GetY() > 0) // Moving down
    {
      // Stop if we're very near or are going to overshoot
      if (myPanVector.GetY() <= acceleration ||
          myPanVector.GetY() <= myPanVelocity.GetY()) {
        // Finish last step
        myPanVelocity.SetY(myPanVector.GetY());
      } else {
        // Accelerate if we are up to half way there
        if (myDistancePanned.GetY() < myPanVector.GetY()) {
          myPanVelocity.AdjustY(acceleration);
        } else {
          // Slow down until we reach minimum speed
          if (myPanVelocity.GetY() - acceleration >= minimumSpeed) {
            myPanVelocity.AdjustY(-acceleration);
          } else {
            myPanVelocity.SetY(minimumSpeed);
          }
        }
      }
    } else {
      // Stop Panning vertically
      myPanVelocity.SetY(0);
    }
  }

  MoveBy(myPanVelocity.GetX(), myPanVelocity.GetY());
  myDistancePanned.AdjustX(myPanVelocity.GetX());
  myDistancePanned.AdjustY(myPanVelocity.GetY());
  myPanVector.AdjustX(-myPanVelocity.GetX());
  myPanVector.AdjustY(-myPanVelocity.GetY());

  return true;
}

void Camera::Update(bool completeRedraw, bool justBackBuffer) {
  if ((myChangingRoomsFlag && !justBackBuffer) || myDisabledFlag)
    return;

  NormaliseWorldPosition();

  // find out if the background is smaller than the display width do something
  SetViewAndDisplayArea();

  int n = myFloatingThings.size();
  for (int i = 0; i < n; ++i) {
    if (myFloatingThings[i].IsValid())
      myFloatingThings[i].GetAgentReference().CameraPositionNotify();
  }

#ifdef C2D_DIRECT_DISPLAY_LIB
  static bool drawPic = true;
  if (drawPic)
    myPicture->Draw(myScreen->GetBack());
  myDisplayList->Draw(myScreen->GetBack(), 0, 0, C2DBLT_TRANS);

  if (myMapImage && myMapImageDisplayed) {
    myMapImage->DrawLineToBackBuffer();
  }

  myScreen->Flip();
#else
  myScreen->Update(myWorldPosition, completeRedraw, justBackBuffer);
#endif
}

void Camera::NormaliseWorldPosition() {
  int32 x = myWorldPosition.GetX();
  int32 y = myWorldPosition.GetY();

  NormaliseWorldCoordinates(x, y);
  myWorldPosition.SetX(x);
  myWorldPosition.SetY(y);

#ifndef C2D_DIRECT_DISPLAY_LIB
  myScreen->SetWorldPosition(myWorldPosition);
#endif

  SetViewAndDisplayArea();
}

void Camera::CalculateDisplayDimensions(int32 &displayWidth,
                                        int32 &displayHeight) {
  // trim the display dimensions to match the available amount of background
  // if it is bigger.

#ifdef C2D_DIRECT_DISPLAY_LIB
  displayWidth = myMetaRoomBounds.right;
  displayHeight = myMetaRoomBounds.bottom;
#else
  myScreen->GetConsideredDisplayArea(0, displayWidth, displayHeight);

  if (myViewWidth != -1 && myViewHeight != -1) {
    if (myViewWidth < displayWidth || displayWidth == 0) {
      displayWidth = myViewWidth;
    } else {
      myViewWidth = displayWidth;
    }

    if (myViewHeight < displayHeight || displayHeight == 0) {
      displayHeight = myViewHeight;
    } else {
      myViewHeight = displayHeight;
    }
  }
#endif
}

void Camera::NormaliseWorldCoordinates(int32 &x, int32 &y) {
#ifdef C2D_DIRECT_DISPLAY_LIB

  // all we need check here is whether the x and y coordinate is
  // within the meta room bound as the bitmap itself will wrap

  if (x + myScreen->GetWidth() > myMetaRoomBounds.right) {

    x = myMetaRoomBounds.right - myScreen->GetWidth();
  } else {
    if (x < myMetaRoomBounds.left) {
      x = myMetaRoomBounds.left;
    }
  }

  if (y < myMetaRoomBounds.top) {
    y = myMetaRoomBounds.top;
  } else {
    if (y + myScreen->GetHeight() > myMetaRoomBounds.bottom) {
      y = myMetaRoomBounds.bottom - myScreen->GetHeight();
    }
  }

#else

  // here we have no wrapping so we need to ensure that we do not
  // run out of backround pixels to draw.
  int32 displayWidth;
  int32 displayHeight;
  int32 top = myScreen->GetTop(0);
  int32 left = myScreen->GetLeft(0);
  int32 pixelWidth = myScreen->GetPixelWidth(0);
  int32 pixelHeight = myScreen->GetPixelHeight(0);

  CalculateDisplayDimensions(displayWidth, displayHeight);

  // make sure that we never are asked to do draw more background
  // than is possible.  We'll have no world wrapping here

  // absolute x plus the display width is too large for
  // my bitmap
  if (x - left + displayWidth > pixelWidth) {
    x = pixelWidth - displayWidth + left;
  }

  if (y - top + displayHeight > pixelHeight) {
    y = pixelHeight;
    y = pixelHeight - displayHeight + top;
  }

  // don't go off the left edge or the top
  if (x - left < 0) {
    x = left;
  }

  if (y - top < 0) {
    y = top;
  }
#endif
}

void Camera::Track(AgentHandle &agent, int xpercent, int ypercent, int style,
                   int transition) {
  // do any unfinished business first
  StopTracking();

  myTrackObject = agent;
  myTrackPercentX = xpercent;
  myTrackPercentY = ypercent;
  myTrackStyle = style;
  myTrackInRectangleLastTick = false;
  myTrackPositionLastTick = myWorldPosition;
  myTrackTransition = transition;

  // if this is a creature then tell it to
  // remember that it is being tracked when it grows up for example
  if (myTrackObject.IsCreature()) {
    myTrackObject.GetCreatureReference().RememberThatYouAreBeingTracked(true);
  }

  SetTrackMetaRoom();
}

// this is for when a creature has been tracked but has grown up
// in between.  The growing up process destroys all current entity
// images and in doing they get unlinked from the camera.
// So the camera needs to know that it should start tracking again
void Camera::YourTrackObjectHasGrownUp(AgentHandle &agent) {
  // presumably then we can use the previous tracking settings
  myTrackObject = agent;
}

void Camera::SetTrackMetaRoom() {
  ASSERT(myTrackObject.IsValid());

  float objx = myTrackObject.GetAgentReference().GetCentre().x;
  float objy = myTrackObject.GetAgentReference().GetCentre().y;

  theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(objx, objy),
                                                   myTrackMetaRoomLastTick);
}

// ----------------------------------------------------------------------
// Method:      ChangeDisplayMode
// Arguments:   width - width to change to
//				height - height to change to
//
// Returns:     true if you choose a legitimate display mode and we
//				switched to it,  false otherwise.
//
// Description: Finds out whether the requested display mode is viable and
//				switches to it.  For smoother results change the
// size
// of 				window too. Note that the display modes are
// enumrated at 				start up of the engine
//
// ----------------------------------------------------------------------

// TODO: is this fn ever used?
bool Camera::ChangeDisplayMode(uint32 width, uint32 height) {
  bool ok = false;
#ifdef C2D_DIRECT_DISPLAY_LIB

  _ASSERT(false);
#else
#ifdef C2E_SDL
  _ASSERT(false);
#else
  ok = DisplayEngine::theRenderer().ChangeDisplayMode(width, height);
#endif
#endif

  Update(true); // complete redraw
  return ok;
}

void Camera::FadeScreen() {
#ifdef C2D_DIRECT_DISPLAY_LIB

  _ASSERT(false);
#else
  DisplayEngine::theRenderer().FadeScreen();
#endif
}

void Camera::Refresh() {
  TrackObject();
  Update(true, myLoadingFlag);
}

AgentHandle Camera::GetTrackAgent() { return myTrackObject; }

bool Camera::TrackObject() {
  if (myTrackObject.IsValid()) {
    // if flexible style, don't track unless we were in the
    // rectangle last tick.
    if ((myTrackStyle == 1) && !myTrackInRectangleLastTick)
      return false;

    // again, if flexible style, don't track is someone else moved
    // the camera (for example, gone to favourite place)
    if ((myTrackStyle == 1) && (myTrackPositionLastTick != myWorldPosition))
      return false;

    int offbyx, offbyy;
    if (!InTrackingRectangle(offbyx, offbyy)) {
      float objx = myTrackObject.GetAgentReference().GetCentre().x;
      float objy = myTrackObject.GetAgentReference().GetCentre().y;

      int metaRoomID;
      bool ok = theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(
          Vector2D(objx, objy), metaRoomID);
      if (!ok)
        return false;

      if (theApp.GetWorld().GetMap().GetCurrentMetaRoom() != metaRoomID) {
        // if we're not hard tracking, and it's the camera that's
        // changed meta room, not us, then don't change back
        if ((myTrackStyle != 2) && (myTrackMetaRoomLastTick == metaRoomID))
          return false;

        // let the pointer decide whether it should carry objects from
        // meta room to meta room
        thePointer.GetPointerAgentReference()
            .DecideIfCarriedAgentShouldBeDropped();
        theApp.GetWorld().SetMetaRoom(metaRoomID, myTrackTransition, objx, objy,
                                      true);
      } else {
        MoveBy(-offbyx, -offbyy);
      }

      return true;
    }
  }
  return false;
}

bool Camera::InTrackingRectangle() {
  int offbyx, offbyy;
  return InTrackingRectangle(offbyx, offbyy);
}

bool Camera::InTrackingRectangle(int &offbyx, int &offbyy) {
  ASSERT(myTrackObject.IsValid());

  int x = (int)myTrackObject.GetAgentReference().GetCentre().x;
  int y = (int)myTrackObject.GetAgentReference().GetCentre().y;

  // get view rectangle
  RECT rect = myViewArea;
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  RECT rectView = rect;

  // make into tracking rectangle
  {
    int allow = (width * myTrackPercentX) / 100;
    int disallow = width - allow;
    rect.left += disallow / 2;
    rect.right -= disallow / 2;
    if (rect.right < rect.left)
      rect.right = rect.left + 1;
  }

  {
    int allow = (height * myTrackPercentY) / 100;
    int disallow = height - allow;
    rect.top += disallow / 2;
    rect.bottom -= disallow / 2;
    if (rect.bottom < rect.top)
      rect.bottom = rect.top + 1;
  }

  // stretch rectangle if at edge of meta room (in
  // case our object goes through a door there, in which
  // case we want it are track them - e.g. out of learning
  // room door in Creatures 3)
  {
    int metaRoomID = GetMetaRoom();
    int x, y, w, h;
    if (!theApp.GetWorld().GetMap().GetMetaRoomLocation(metaRoomID, x, y, w, h))
      return false;
    if (rectView.top <= y)
      rect.top = rectView.top;
    if (rectView.bottom >= y + h - 1)
      rect.bottom = rectView.bottom;
    if (rectView.left <= x)
      rect.left = rectView.left;
    if (rectView.right >= x + w - 1)
      rect.right = rectView.right;
  }

  // set off by values
  {
    offbyx = 0;
    offbyy = 0;

    if (x < rect.left)
      offbyx = rect.left - x;
    else if (x > rect.right)
      offbyx = rect.right - x;

    if (y < rect.top)
      offbyy = rect.top - y;
    else if (y > rect.bottom)
      offbyy = rect.bottom - y;
  }

  return (offbyx == 0) && (offbyy == 0);
}

// explicit call from outside to stop tracking
void Camera::StopTracking() {
  // if this is a creature then tell it to
  // forget that it is being tracked
  if (myTrackObject.IsCreature()) {
    myTrackObject.GetCreatureReference().RememberThatYouAreBeingTracked(false);
  }

  myTrackObject = NULLHANDLE;
}

void Camera::CentriseCoordinates(int32 &x, int32 &y) {

  x = x - (myDisplayRect.right / 2);
  y = y - (myDisplayRect.bottom / 2);
}

void Camera::CentreOn(int32 x, int32 y, int pan) {
  if (pan == 2)
    pan = IsPointOnScreen(x, y);

  CentriseCoordinates(x, y);

  if (pan) {
    PanTo(x, y);
  } else {
    MoveTo(x, y);
  }
}

// entity to keep on screen
// distance to stay away from it
void Camera::KeepUpWithMouse(EntityImage *me) { myObjectToKeepUpWith = me; }

bool Camera::KeepUpThenWithMouse() {
  if (myObjectToKeepUpWith != NULL) {
    int32 x = myObjectToKeepUpWith->GetPosition().GetX();
    int32 y = myObjectToKeepUpWith->GetPosition().GetY();

    int32 panx = 0, pany = 0;

    bool isFullScreen = false;

#ifdef C2D_DIRECT_DISPLAY_LIB
    isFullScreen = myScreen->GetFullScreen() ? true : false;
#else
    isFullScreen = DisplayEngine::theRenderer().IsFullScreen();
#ifdef C2E_SDL
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
      isFullScreen = true;
#endif
#endif

    if (isFullScreen && ((theApp.myScrollingMask & 1) == 1)) {
      int32 mouseActiveBorderSizeTop =
          theApp.GetEameVar("engine_nudge_border_t").GetInteger();
      int32 mouseActiveBorderSizeBottom =
          theApp.GetEameVar("engine_nudge_border_b").GetInteger();
      int32 mouseActiveBorderSizeLeft =
          theApp.GetEameVar("engine_nudge_border_l").GetInteger();
      int32 mouseActiveBorderSizeRight =
          theApp.GetEameVar("engine_nudge_border_r").GetInteger();

      // mouse
      if (x <= myWorldPosition.GetX() + mouseActiveBorderSizeLeft)
        panx = -1;
      else if (x >= myWorldPosition.GetX() +
                        (myDisplayRect.right - myDisplayRect.left) -
                        mouseActiveBorderSizeRight)
        panx = 1;
      if (y <= myWorldPosition.GetY() + mouseActiveBorderSizeTop)
        pany = -1;
      else if (y >= myWorldPosition.GetY() +
                        (myDisplayRect.bottom - myDisplayRect.top) -
                        mouseActiveBorderSizeBottom)
        pany = 1;
    }

    if (!theApp.GetInputManager().IsKeyDown(VK_SHIFT) &&
        !theApp.GetInputManager().IsKeyDown(VK_CONTROL) &&
        !theApp.GetInputManager().IsKeyDown(VK_MENU)) {
      if ((theApp.myScrollingMask & 2) == 2) {
        if (!theApp.GetInputManager().GetTranslatedCharTarget() ||
            theApp.GetInputManager().GetTranslatedCharTarget()->AllowScroll()) {
          // keyboard
          if (theApp.GetInputManager().IsKeyDown(VK_LEFT))
            panx = -1;
          else if (theApp.GetInputManager().IsKeyDown(VK_RIGHT))
            panx = 1;

          if (theApp.GetInputManager().IsKeyDown(VK_UP))
            pany = -1;
          else if (theApp.GetInputManager().IsKeyDown(VK_DOWN))
            pany = 1;
        }
      }
    }

    // process them to decide how much to move camera by:
    if (panx != 0) {
      myKeepUpCountX++;
      if (myKeepUpCountX > theApp.myScrollingSpeedRangeUp.size())
        myKeepUpCountX = theApp.myScrollingSpeedRangeUp.size();
      myKeepUpSpeedX = theApp.myScrollingSpeedRangeUp[myKeepUpCountX - 1];
      myKeepUpPanX = panx;
    } else if (myKeepUpCountX != 0) {
      if (myKeepUpCountX > theApp.myScrollingSpeedRangeDown.size())
        myKeepUpCountX = theApp.myScrollingSpeedRangeDown.size();

      ASSERT(myKeepUpPanX != 0);
      myKeepUpSpeedX = theApp.myScrollingSpeedRangeDown[myKeepUpCountX - 1];
      panx = myKeepUpPanX;

      myKeepUpCountX--;
    } else {
      // ASSERT(myKeepUpSpeedX == 0);
      // ASSERT(myKeepUpCountX == 0);
      // OutputDebugString("ResetX\n");
    }

    if (pany != 0) {
      myKeepUpCountY++;
      if (myKeepUpCountY > theApp.myScrollingSpeedRangeUp.size())
        myKeepUpCountY = theApp.myScrollingSpeedRangeUp.size();
      myKeepUpSpeedY = theApp.myScrollingSpeedRangeUp[myKeepUpCountY - 1];
      myKeepUpPanY = pany;
    } else if (myKeepUpCountY != 0) {
      if (myKeepUpCountY > theApp.myScrollingSpeedRangeDown.size())
        myKeepUpCountY = theApp.myScrollingSpeedRangeDown.size();

      ASSERT(myKeepUpPanY != 0);
      myKeepUpSpeedY = theApp.myScrollingSpeedRangeDown[myKeepUpCountY - 1];
      pany = myKeepUpPanY;

      myKeepUpCountY--;
    } else {
      // ASSERT(myKeepUpSpeedY == 0);
      // ASSERT(myKeepUpCountY == 0);
      // OutputDebugString("ResetY\n");
    }

    // std::ostringstream zub;
    // zub << "s:" <<  << " " << '\0';
    // OutputFormattedDebugString("s: %d %d", myKeepUpSpeedX, myKeepUpSpeedY);

    panx *= myKeepUpSpeedX;
    pany *= myKeepUpSpeedY;

    if (panx != 0 || pany != 0) {
      // OutputFormattedDebugString("MoveBy(%d, %d)\n", panx, pany);
      MoveBy(panx, pany);
      return true;
    }
  }
  return false;
}

bool Camera::MiddleMouseDragMouseWheel() {
  InputManager &im = theApp.GetInputManager();
  int panx = 0;
  int pany = 0;

  // Scan the pending inputevents:
  for (int i = 0; i < im.GetEventCount(); ++i) {
    const InputEvent *ev = &im.GetEvent(i);

    // Middle button down
    if (ev->EventCode == InputEvent::eventMouseDown &&
        ev->MouseButtonData.button == InputEvent::mbMiddle &&
        ((theApp.myScrollingMask & 4) == 4)) {
      myMiddleDrag = true;
      myMiddleDragMX = ev->MouseButtonData.mx;
      myMiddleDragMY = ev->MouseButtonData.my;
      GlobalSetCapture();
    } else if (ev->EventCode == InputEvent::eventMouseUp &&
               ev->MouseButtonData.button == InputEvent::mbMiddle) {
      myMiddleDrag = false;
      GlobalReleaseCapture();
    } else if (myMiddleDrag && ev->EventCode == InputEvent::eventMouseMove) {
      panx += (myMiddleDragMX - ev->MouseButtonData.mx);
      pany += (myMiddleDragMY - ev->MouseButtonData.my);
      myMiddleDragMX = ev->MouseButtonData.mx;
      myMiddleDragMY = ev->MouseButtonData.my;
    }

    if ((ev->EventCode == InputEvent::eventMouseWheel) &&
        ((theApp.myScrollingMask & 8) == 8)) {
      pany -= ev->MouseWheelData.delta;
    }
  }
  if (panx != 0 || pany != 0) {
    MoveBy(panx, pany);
    return true;
  }

  return myMiddleDrag;
}

// ----------------------------------------------------------------------
// Method:      AddDirtyRect
// Arguments:   rect - new dirty rectangle for a fast drawn object
//
// Returns:     None
//
// Description:
//
// ----------------------------------------------------------------------
void Camera::AddDirtyRect(RECT rect) {
  //	myEntityHandler.AddDirtyRect(rect);
  //	_ASSERT(false);
  // move to sprite
}

bool Camera::IsPointOnScreen(int32 x, int32 y) {
  POINT point;
  point.x = x;
  point.y = y;

  return PointInRectangle(&myViewArea, point) ? true : false;
}

void Camera::AddFloatingThing(AgentHandle &a) {
  ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) ==
         myFloatingThings.end());
  myFloatingThings.push_back(a);
  ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) !=
         myFloatingThings.end());
}

void Camera::RemoveFloatingThing(AgentHandle &a) {
  ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) !=
         myFloatingThings.end());
  myFloatingThings.erase(
      std::find(myFloatingThings.begin(), myFloatingThings.end(), a));
  ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) ==
         myFloatingThings.end());
}

Position Camera::GetWorldPosition(void) { return myWorldPosition; }

bool Camera::Visible(RECT &rect) {

  // move to sprite
#ifdef C2D_DIRECT_DISPLAY_LIB
  return ((rect.right >= myViewArea.left) && (rect.left <= myViewArea.right) &&
          (rect.top <= myViewArea.bottom) && (rect.bottom >= myViewArea.top));
#else

  return myScreen->IsRectOnScreen(rect);
#endif
}

bool Camera::Write(CreaturesArchive &ar) const {
  ar << myAmIInterestedInCameraShyObjects;

  ar << myPanVelocity;
  ar << myPanVector;
  ar << myDistancePanned;
  ar << myAcceleration;
  ar << myMinimumSpeed;

  ar << myViewWidth;
  ar << myViewHeight;
  ar << myViewArea;

  ar << myWorldPosition;

  ar << myTrackObject;
  ar << myTrackPercentX << myTrackPercentY;
  ar << myTrackStyle;
  ar << myTrackInRectangleLastTick;
  ar << myTrackPositionLastTick;
  ar << myTrackMetaRoomLastTick;
  ar << myTrackTransition;
  ar << myMiddleDrag;
  ar << myMiddleDragMX << myMiddleDragMY;

  // if you want you can alway make sure that the
  // pointer stays a certain
  ar << myObjectToKeepUpWith;
  ar << myKeepUpSpeedX << myKeepUpSpeedY;
  ar << myKeepUpCountX << myKeepUpCountY;
  ar << myKeepUpPanX << myKeepUpPanY;

  ar << myChangingRoomsFlag;

  ar << myDisabledFlag;

#ifdef C2D_DIRECT_DISPLAY_LIB

  ar << myBackground1FilePath;
#else

  //***********************
  // write first background
  ar << myScreen->GetDisplayPosition(0);

  ar << myScreen->GetBackgroundTopLeft(0);

  ar << myScreen->GetWidth(0);
  ar << myScreen->GetHeight(0);

  ar << myScreen->GetPixelWidth(0);
  ar << myScreen->GetPixelHeight(0);

  ar << myBackground1FilePath;

  //***********************
  // write second background
  ar << myScreen->GetDisplayPosition(1);

  ar << myScreen->GetBackgroundTopLeft(1);

  ar << myScreen->GetWidth(1);
  ar << myScreen->GetHeight(1);

  ar << myScreen->GetPixelWidth(1);
  ar << myScreen->GetPixelHeight(1);

  ar << myBackground2FilePath;

#endif
  return true;
}

bool Camera::Read(CreaturesArchive &ar) {

  int32 version = ar.GetFileVersion();

  if (version >= 13) {

    ar >> myAmIInterestedInCameraShyObjects;

#ifndef C2D_DIRECT_DISPLAY_LIB
    myScreen->SetInterestLevel(myAmIInterestedInCameraShyObjects);
#endif

    ar >> myPanVelocity;
    ar >> myPanVector;
    ar >> myDistancePanned;
    ar >> myAcceleration;
    ar >> myMinimumSpeed;

    ar >> myViewWidth;
    ar >> myViewHeight;
    ar >> myViewArea;

    ar >> myWorldPosition;

    ar >> myTrackObject;

    ar >> myTrackPercentX >> myTrackPercentY;
    ar >> myTrackStyle;
    ar >> myTrackInRectangleLastTick;
    ar >> myTrackPositionLastTick;
    ar >> myTrackMetaRoomLastTick;
    ar >> myTrackTransition;
    ar >> myMiddleDrag;
    ar >> myMiddleDragMX >> myMiddleDragMY;

    // if you want you can alway make sure that the
    // pointer stays a certain
    ar >> myObjectToKeepUpWith;
    ar >> myKeepUpSpeedX >> myKeepUpSpeedY;
    ar >> myKeepUpCountX >> myKeepUpCountY;
    ar >> myKeepUpPanX >> myKeepUpPanY;

    ar >> myChangingRoomsFlag;

    ar >> myDisabledFlag;

#ifdef C2D_DIRECT_DISPLAY_LIB
    ar >> myBackground1FilePath;

    Create(0, // world x co-ordinate of view
           0, // world y co-ordinate of view
           0, // width of camera
           0, // height of camera
           myBackground1FilePath.GetFileName(),
           0, // top left world co-ordinates
           0  // of background as a meta room
    );
#else
    // read in background data for first background
    //********

    Position pos;
    Position topLeft;
    ar >> pos;

    ar >> topLeft;

    uint32 width;
    uint32 height;
    ar >> width;
    ar >> height;

    ar >> myPixelWidth;
    ar >> myPixelHeight;

    ar >> myBackground1FilePath;

    std::string backgroundName = myBackground1FilePath.GetFileName();

    if (!backgroundName.empty()) {
      backgroundName = myBackground1FilePath.GetFullPath();
    }

    myScreen->Create(0, pos, backgroundName, topLeft, myPixelWidth,
                     myPixelHeight);

    //**********

    ar >> pos;

    ar >> topLeft;

    ar >> width;
    ar >> height;

    ar >> myPixelWidth;
    ar >> myPixelHeight;

    ar >> myBackground2FilePath;

    backgroundName = myBackground2FilePath.GetFileName();

    if (!backgroundName.empty()) {
      backgroundName = myBackground2FilePath.GetFullPath();
    }

    myScreen->Create(1, pos, backgroundName, topLeft, myPixelWidth,
                     myPixelHeight);

    //*************

    if (version < 31) {
      bool swap;
      ar >> swap;
      // no need to swap backgrounds anymore as backgrounds are
      // not serialized out anymore
    }

    // DoUpdateAfterAdjustments();
#endif

  } else if (version >= 3) {
#ifndef C2D_DIRECT_DISPLAY_LIB

    // do a dummy read of the drawable object
    RECT bound;
    Position pos;
    bool tempBool;
    ar >> pos;
    ar >> bound;
    ar >> tempBool;
    ar >> tempBool;
    ar >> tempBool;
    // do a dummy read of the entityhandler

    // world position
    ar >> pos;

    // shutdown useless
    ar >> tempBool;
    // camerashy
    ar >> myAmIInterestedInCameraShyObjects;
    myScreen->SetInterestLevel(myAmIInterestedInCameraShyObjects);

    ar >> myPanVelocity;
    ar >> myPanVector;
    ar >> myDistancePanned;
    ar >> myAcceleration;
    ar >> myMinimumSpeed;

    ar >> myViewWidth;
    ar >> myViewHeight;
    ar >> myViewArea;

    ar >> myWorldPosition;

    ar >> myTrackObject;

    ar >> myTrackPercentX >> myTrackPercentY;
    ar >> myTrackStyle;
    ar >> myTrackInRectangleLastTick;
    ar >> myTrackPositionLastTick;
    ar >> myTrackMetaRoomLastTick;
    ar >> myTrackTransition;
    ar >> myMiddleDrag;
    ar >> myMiddleDragMX >> myMiddleDragMY;

    // if you want you can alway make sure that the
    // pointer stays a certain
    ar >> myObjectToKeepUpWith;
    ar >> myKeepUpSpeedX >> myKeepUpSpeedY;
    ar >> myKeepUpCountX >> myKeepUpCountY;
    ar >> myKeepUpPanX >> myKeepUpPanY;

    ar >> myChangingRoomsFlag;

    ar >> myDisabledFlag;

    // read in background data for first background
    //********

    Position topLeft;
    ar >> pos;

    ar >> topLeft;

    uint32 width;
    uint32 height;
    ar >> width;
    ar >> height;

    ar >> myPixelWidth;
    ar >> myPixelHeight;

    ar >> myBackground1FilePath;

    std::string backgroundName = myBackground1FilePath.GetFileName();

    if (!backgroundName.empty()) {
      backgroundName = myBackground1FilePath.GetFullPath();
    }

    myScreen->Create(0, pos, backgroundName, topLeft, myPixelWidth,
                     myPixelHeight);

    //**********

    ar >> pos;

    ar >> topLeft;

    ar >> width;
    ar >> height;

    ar >> myPixelWidth;
    ar >> myPixelHeight;

    ar >> myBackground2FilePath;

    backgroundName = myBackground2FilePath.GetFileName();

    if (!backgroundName.empty()) {
      backgroundName = myBackground2FilePath.GetFullPath();
    }

    myScreen->Create(1, pos, backgroundName, topLeft, myPixelWidth,
                     myPixelHeight);

    //*************

    bool swap;
    ar >> swap;
    if (swap) {

      myScreen->SwapBackgrounds();
    }

    DoUpdateAfterAdjustments();
#endif

  } else {
    _ASSERT(false);
    return false;
  }

  return true;
}

// gtb!temp!
//  Functions which convert between world and screen coordinates:
template <class IntOrFloat>
void GenericWorldToScreen(IntOrFloat &x, IntOrFloat &y, IntOrFloat viewAreaLeft,
                          IntOrFloat viewAreaTop) {
  x -= viewAreaLeft;
  y -= viewAreaTop;
}

template <class IntOrFloat>
void GenericScreenToWorld(IntOrFloat &x, IntOrFloat &y, IntOrFloat viewAreaLeft,
                          IntOrFloat viewAreaTop) {
  x += viewAreaLeft;
  y += viewAreaTop;
}

// ----------------------------------------------------------------------
// Method:      ScreenToWorld
// Arguments:   x - screen x position
//				y - screen y position
//
// Returns:     None
// Description: Converts screen coordinates to world coordinates
//
// ----------------------------------------------------------------------
void Camera::ScreenToWorld(int &x, int &y) {
  GenericScreenToWorld<int>(x, y, myViewArea.left, myViewArea.top);
}

void Camera::ScreenToWorld(Vector2D &vec) {
  GenericScreenToWorld<float>(vec.x, vec.y, myViewArea.left, myViewArea.top);
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void Camera::ScaleMouseCoordinatesAccordingToWindowSize(int &x, int &y) {

  // the direct display engine scales the whole viewport scene into the window
  // so the mouse coordinates need to be fudged so that you can
  // reach past the edge of the client window.
  // make the display rectangle equal to the resized wind
  RECT testRect;
  ::GetClientRect(theMainWindow, &testRect);

  if (testRect.right - testRect.left <= 0 ||
      testRect.bottom - testRect.top <= 0) {
    return;
  }

  float width = float(testRect.right - testRect.left);
  float height = float(testRect.bottom - testRect.top);

  if (x > 0) {
    float newx = float(x / width) * float(myScreen->GetWidth());
    x = FastFloatToInteger(newx);

    //	x = (x/testRect.right - testRect.left) * myScreen->GetWidth();
  }

  if (y > 0) {
    // y = (y/testRect.bottom - testRect.top) * myScreen->GetHeight();
    float newy = float(y / height) * float(myScreen->GetHeight());
    y = FastFloatToInteger(newy);
  }
}
#endif

// ----------------------------------------------------------------------
// Method:      WorldtoScreen
// Arguments:   x - screen x position
//				y - screen y position
//
// Returns:     None
// Description: Converts world coordinates to screen coordinates
//
// ----------------------------------------------------------------------
bool Camera::WorldToScreen(int &x, int &y) {
  GenericWorldToScreen<int>(x, y, myViewArea.left, myViewArea.top);
  //	x -= myViewArea.left;
  //	y -= myViewArea.top;

  POINT point;
  point.x = x;
  point.y = y;

  return PointInRectangle(&myDisplayRect, point) ? true : false;
}

bool Camera::WorldToScreen(Vector2D &vec) {
  GenericWorldToScreen<float>(vec.x, vec.y, myViewArea.left, myViewArea.top);
  //	vec.x -= (float)myViewArea.left;
  //	vec.y -= (float)myViewArea.top;

  POINT point;
  point.x = Map::FastFloatToInteger(vec.x);
  point.y = Map::FastFloatToInteger(vec.y);

  return PointInRectangle(&myDisplayRect, point) ? true : false;
}

// ----------------------------------------------------------------------
// Method:      ConvertToWorldCoordinates
// Arguments:   rect - rectangle to convert from world co-ordinates to
//						the screen co-ordinates
//
// Returns:     None
//
// Description: Converts teh bounds of the given rectangle into
//				world coordinates
//
// ----------------------------------------------------------------------
void Camera::ConvertToWorldCoordinates(RECT &rect) {
  int32 width = rect.right - rect.left;
  int32 height = rect.bottom - rect.top;

  rect.top = rect.top + myViewArea.top;
  rect.bottom = rect.top + height;
  rect.left = rect.left + myViewArea.left;
  rect.right = rect.left + width;
}

// ----------------------------------------------------------------------
// Method:      ConvertToDisplayCoordinates
// Arguments:   rect - rectangle to convert from world co-ordinates to
//						the screen co-ordinates
//
// Returns:     None
//
// Description: Converts the bounds of the given rectangle into
//				screen coordinates
//
// ----------------------------------------------------------------------
void Camera::ConvertToDisplayCoordinates(RECT &rect) {
  int32 width = rect.right - rect.left;
  int32 height = rect.bottom - rect.top;

  rect.top = rect.top - myViewArea.top;
  rect.bottom = rect.top + height;
  rect.left = rect.left - myViewArea.left;
  rect.right = rect.left + width;
}

void Camera::GetViewCentre(int32 &centrex, int32 &centrey) {
  centrex = myViewArea.left + (myDisplayRect.right / 2);

  centrey = myViewArea.top + (myDisplayRect.bottom / 2);
}

// ----------------------------------------------------------------------
// Method:      SetViewAndDisplayArea
// Arguments:   pos - camera x,y position
//
//
// Returns:     None
// Description: Updates the view area to where the camera is positioned
//				and also the display area
//
// ----------------------------------------------------------------------
void Camera::SetViewAndDisplayArea() {
  int32 displayWidth;
  int32 displayHeight;

#ifdef C2D_DIRECT_DISPLAY_LIB

  // make the display rectangle equal to the resized wind
  RECT testRect;
  ::GetClientRect(theMainWindow, &testRect);

  if (testRect.right - testRect.left <= 0 ||
      testRect.bottom - testRect.top <= 0) {
    return;
  }

  displayWidth = testRect.right - testRect.left;
  displayHeight = testRect.bottom - testRect.top;

  myViewArea.top = myWorldPosition.GetY();
  myViewArea.bottom = myWorldPosition.GetY() + myScreen->GetHeight();
  myViewArea.left = myWorldPosition.GetX();
  myViewArea.right = myWorldPosition.GetX() + myScreen->GetWidth();

  if (displayWidth > (myMetaRoomBounds.right - myMetaRoomBounds.left)) {
    displayWidth = myMetaRoomBounds.right - myMetaRoomBounds.left;
  }

  if (displayHeight > (myMetaRoomBounds.bottom - myMetaRoomBounds.top)) {
    displayHeight = myMetaRoomBounds.bottom - myMetaRoomBounds.top;
  }

#else
  CalculateDisplayDimensions(displayWidth, displayHeight);

  myViewArea.top = myWorldPosition.GetY();
  myViewArea.bottom = myWorldPosition.GetY() + displayHeight;

  myViewArea.left = myWorldPosition.GetX();
  myViewArea.right = myWorldPosition.GetX() + displayWidth;

#endif

  // only draw in this area - this takes the size of the
  // background into consideration
  myDisplayRect.top = 0;
  myDisplayRect.bottom = displayHeight;
  myDisplayRect.left = 0;
  myDisplayRect.right = displayWidth;

#ifndef C2D_DIRECT_DISPLAY_LIB
  myScreen->SetViewArea(myViewArea);
#endif
}

void Camera::GetViewArea(RECT &rect) {
  rect.top = myViewArea.top;
  rect.bottom = myViewArea.bottom;
  rect.left = myViewArea.left;
  rect.right = myViewArea.right;
}

bool Camera::IsSameBackGround(const std::string &background) {
  bool same = false;
#ifdef C2D_DIRECT_DISPLAY_LIB
//	_ASSERT(false);
#else

  if (background == myScreen->GetBackgroundName(0))
    same = true;
#endif

  return same;
}

std::string Camera::GetBackgroundName() const {
#ifdef C2D_DIRECT_DISPLAY_LIB
  //	_ASSERT(false);
  return std::string();
#else
  return myScreen->GetBackgroundName(0);
#endif
}

// ----------------------------------------------------------------------
// Method:      UpdatePlane
// Arguments:   entityImage - the image to be updated
//
//
// Returns:     None
//
// Description: an image has changed its plane order
//				remove it and add it so that it gets put in the
// correct 				z order
//
//
// ----------------------------------------------------------------------
void Camera::UpdatePlane(EntityImage *entityImage) {
  Remove(entityImage, false); // don't stop tracking if you were being tracked
#ifdef C2D_DIRECT_DISPLAY_LIB
//	_ASSERT(false);
#else
  myScreen->Add(entityImage->GetSpritePtr()); // as I am adding you in right now
#endif
}

void Camera::GetDisplayArea(RECT &rect) {
  rect.top = myDisplayRect.top;
  rect.bottom = myDisplayRect.bottom;
  rect.left = myDisplayRect.left;
  rect.right = myDisplayRect.right;
}

void Camera::SetLoading(bool flag) { myLoadingFlag = flag; }

bool Camera::GetLoading() { return myLoadingFlag; }

void Camera::DoTrackingChecks() {
  if (myTrackObject.IsValid()) {
    int offbyx, offbyy;
    if (!InTrackingRectangle(offbyx, offbyy)) {
      // stop tracking if brittle and out of rectangle
      if (myTrackStyle == 0)
        StopTracking();
      // force stay in rectangle if hard tracking
      else if (myTrackStyle == 2)
        MoveBy(-offbyx, -offbyy);

      myTrackInRectangleLastTick = false;
    } else
      myTrackInRectangleLastTick = true;

    myTrackPositionLastTick = myWorldPosition;
  }
}

void Camera::SetPosition(Position &position) { myWorldPosition = position; }

bool Camera::SaveAsSpriteFile(std::string const &filename) const {
#ifdef C2D_DIRECT_DISPLAY_LIB
  //	_ASSERT(false);
  return false;
#else
  return myScreen->SaveAsSpriteFile(filename);
#endif
}

Position Camera::GetBackgroundTopLeft() {
#ifdef C2D_DIRECT_DISPLAY_LIB
  //	_ASSERT(false);
  return Position();
#else
  return myScreen->GetBackgroundTopLeft(0);
#endif
}

int Camera::GetMetaRoom() {
  return theApp.GetWorld().GetMap().GetCurrentMetaRoom();
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void Camera::DrawLineToScreen(int x1, int y1, int x2, int y2,
                              uint8 lineColourRed, uint8 lineColourGreen,
                              uint8 lineColourBlue) {

  if (myScreen) {
    uint32 lineColour = 0;
    lineColour = (((lineColourRed)&0xf8) << 8) |
                 (((lineColourGreen)&0xfc) << 3) |
                 (((lineColourBlue)&0xf8) >> 3);
    HRESULT ret = myScreen->GetBack()->SafeLock();
    if (ret != DD_OK)
      return;
    myScreen->GetBack()->Line(x1, y1, x2, y2, lineColour);
    myScreen->GetBack()->UnLock();
  }
}

void Camera::SwitchFullScreenMode(bool fullscreen) {
  if (fullscreen) {
    if (!myScreen->CreateFullScreen(theMainWindow, 800, 600, 16))
      return;
  } else {
    myScreen->GetDD()->RestoreDisplayMode();
    SetWindowLong(theMainWindow, GWL_STYLE,
                  WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_THICKFRAME |
                      WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

    RECT rect;

    GetWindowRect(GetDesktopWindow(), &rect);

    // Get default window pos from registry
    int left, top, right, bottom;

    left = 0;
    top = 0;
    right = 800;
    bottom = 600;

    theApp.UserSettings().Get(std::string("WindowLeft"), left);
    theApp.UserSettings().Get(std::string("WindowTop"), top);
    theApp.UserSettings().Get(std::string("WindowRight"), right);
    theApp.UserSettings().Get(std::string("WindowBottom"), bottom);

    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;

    if ((rect.right - myDisplayRect.left) <= 0) {
      rect.left = 0;
      rect.right = 800;
    }

    if ((rect.bottom - rect.top) <= 0) {
      rect.top = 0;
      rect.bottom = 600;
    }

    // reset the window size
    SetWindowPos(theMainWindow,                    // handle to window
                 HWND_TOP,                         // placement-order handle
                 rect.left,                        // horizontal position
                 rect.top,                         // vertical position
                 rect.right - rect.left,           // width
                 rect.bottom - rect.top,           // height
                 SWP_FRAMECHANGED | SWP_SHOWWINDOW // window-positioning flags
    );

    myScreen->CreateWindowed(theMainWindow, 800, 600);
    SetViewAndDisplayArea();
  }

  myScreen->LoadBitmap(myBackground1FilePath.GetFullPath().c_str());
  delete myPicture;
  myPicture =
      new C2DLayer(myScreen, myBackground1FilePath.GetFullPath().c_str());
}

#endif
