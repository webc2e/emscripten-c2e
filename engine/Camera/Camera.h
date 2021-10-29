// --------------------------------------------------------------------------
// Filename:	Camera.h
// Class:		Camera
// Purpose:		This class provides a view of the world to clients.
//				The camera has a position in world coordinates.
//				The interface will give te client the usual
//camera 				operations e.g. panning, tracking.
//
//				Each camera must have access to a display engine.
//This is 				generally passed by the parent window.
//
//
//
// Description: All entities are stored in myEntities.  At the start all objects
//				are on the free list.  From there they can be put on
//the 				update list.
//
//

//
// History:
// -------
// 11Nov98	Alima			Created.
// --------------------------------------------------------------------------
#ifndef DX_CAMERA_H
#define DX_CAMERA_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../Display/Position.h"
#include "../PersistentObject.h"
#include "../../common/BasicException.h"
#include "../Agents/AgentHandle.h"
#include "../FilePath.h"

////////////////////////////////////////////////////////////////////////////
// typedefs
////////////////////////////////////////////////////////////////////////////
typedef std::vector<std::string>::iterator BACKGROUND_NAME_ITERATOR;

enum TransitionModes {
  NOFLIP = 0,
  FLIP,
  BURST,
  SHRINK,
  NUM_TRANSITION_MODES,
};

class Background;
class CameraSprite;
class Line;
class EntityImage;

class C2DScreen;
class C2DSpriteList;
class C2DSprite;
class C2DAniStrip;
class C2DLayer;
class MapImage;

class DrawableObject;

class Camera : public PersistentObject //, public DrawableObject
{
  CREATURES_DECLARE_SERIAL(Camera)
public:
  Camera();

  Camera(int32 viewx,  // world x co-ordinate of view
         int32 viewy,  // world y co-ordinate of view
         int32 width,  // width of camera
         int32 height, // height of camera
         std::string &defaultBackground,
         uint32 topLeftXofBackground, // top left world co-ordinates
         uint32 topLeftYofBackground  // of background as a meta room
  );

  Camera(int32 viewx,  // world x co-ordinate of view
         int32 viewy,  // world y co-ordinate of view
         int32 width,  // width of camera
         int32 height, // height of camera
         std::string &defaultBackground,
         RECT &bounds // of background as a meta room
  );

  void InitialiseCameraVariables();

  FilePath MakeBackgroundPath(std::string const &backgroundName);

  // ----------------------------------------------------------------------
  // Method:      Create
  // Arguments:  viewx - world x co-ordinate of view
  //				viewy - world y co-ordinate of view
  //				width -  width of camera
  //				height -  height of camera
  //				backgroundName - background file to use
  //				topLeftXofBackground - top left world
  //co-ordinates 				topLeftYofBackground) - of background as a meta room
  // Returns:     true if the camera was created OK false otherwise
  // Description: Used in two stage creation of camera it's possible to
  //				run the world with the camera disabled
  //
  // ----------------------------------------------------------------------
  bool Create(int32 viewx,  // world x co-ordinate of view
              int32 viewy,  // world y co-ordinate of view
              int32 width,  // width of camera
              int32 height, // height of camera
              std::string &defaultBackground1,
              int32 topLeftXofBackground, // top left world co-ordinates
              int32 topLeftYofBackground  // of background as a meta room
  );

  bool Create(int32 viewx,  // world x co-ordinate of view
              int32 viewy,  // world y co-ordinate of view
              int32 width,  // width of camera
              int32 height, // height of camera
              std::string &defaultBackground1,
              RECT &bounds // of background as a meta room
  );

  virtual void ChangeMetaRoom(std::string &backgroundName, RECT &rect,
                              int32 newviewx, int32 newviewy, int32 flip,
                              bool bCentre);

  void SetBackground(std::string &backgroundName, int32 transitionMethod);

  void AddDirtyRect(RECT rect);

  void DoChangeMetaRoom(std::string &backgroundName, RECT &bounds,
                        int32 newviewx, int32 newviewy, bool useTopLeftOnly,
                        bool bCentre);

  virtual ~Camera();

  void SetPosition(Position &position);

  virtual Position GetWorldPosition(void);

#ifdef C2D_DIRECT_DISPLAY_LIB
  C2DScreen *GetSpritePtr() { return myScreen; }
#else
  CameraSprite *GetSpritePtr() { return myScreen; }
#endif

  // ----------------------------------------------------------------------
  // Method:      Update
  // Arguments:   completeRedraw - whether to do a complete redraw or just
  //              dirty rectangles.
  // Returns:     None
  // Description: Tells the display engine to update camera
  //
  // ----------------------------------------------------------------------
  void Update(bool completeRedraw, bool justBackBuffer = false);

  void NormaliseWorldPosition();
  void NormaliseWorldCoordinates(int32 &x, int32 &y);

  // attempts to draw this camera are foiled
  virtual void Draw() { ; }
  virtual void DrawMirrored() { ; }
  virtual void SetScreenPosition(Position pos) { ; }

  void CentreMe(Position &pos);
  void CentreMe(RECT &rect);

  void GetDisplayArea(RECT &rect);

  // ----------------------------------------------------------------------
  // Method:      Visible
  // Arguments:   test - rectangle inside which to test whether I am visible.
  // Returns:     true if i am visible within that rectangle false otherwise
  // Description: Tells caller whether the entity exists inside the bounds
  //              of the given rectangle
  //
  //              dummy function returns true until I have sussed
  //              cameras out - needs to be here because it is a property
  //				of a drawable object of course
  //
  // ----------------------------------------------------------------------
  virtual bool Visible(RECT &rect);

  // ----------------------------------------------------------------------
  // Method:      SetEntityImageList
  // Arguments:   array - list of entityImages to deal with next
  //				count - the number of images we are dealing with
  // Returns:     None
  // Description: This allows the caller to tell us which entities
  //				we are working with next.  The array will change
  //				to an STL mechanism when i know a bit more about
  //				the room model
  //
  // ----------------------------------------------------------------------
  void MoveTo(int32 x, int32 y, int pan = 0);

  bool Pan();
  void PanTo(int32 x, int32 y);

  // you cannot zoom the main camera!
  virtual void ZoomBy(int32 pixels, int32 x, int32 y) { ; }

  void MoveBy(int32 x, int32 y);

  void DoUpdateAfterAdjustments();

  virtual void CalculateDisplayDimensions(int32 &displayWidth,
                                          int32 &displayHeight);

  //	virtual void SetCurrentBound(RECT* rect = NULL){;}

  // ----------------------------------------------------------------------
  // Method:      Remove
  // Arguments:   newEntity - pointer to new entity to chop from the
  //				list
  // Returns:     true if the entity was found and removed false otherwise
  // Description: This removes the given entityImage from the current update
  //				list.
  //
  // ----------------------------------------------------------------------
  virtual bool Remove(EntityImage *const chop, bool stopTracking = true);
  virtual bool Remove(DrawableObject *const chop);

  // ----------------------------------------------------------------------
  // Method:      Add
  // Arguments:   newEntity - pointer to new entity to add to the
  //				list
  // Returns:     None
  // Description: This adds the given entityImage to the current update
  //				list.
  //
  // ----------------------------------------------------------------------
  virtual void Add(EntityImage *const newEntity);

#ifdef C2D_DIRECT_DISPLAY_LIB
  C2DSprite *AddAnimStrip(C2DAniStrip *strip, int plane);
  void SwitchFullScreenMode(bool fullscreen);
  void ScaleMouseCoordinatesAccordingToWindowSize(int &x, int &y);
#endif

  virtual void Add(DrawableObject *const newEntity);

  virtual void Track(AgentHandle &agent, int xpercent, int ypercent, int style,
                     int transition);

  void SetTrackMetaRoom();

  // ----------------------------------------------------------------------
  // Method:      UpdatePlane
  // Arguments:   entityImage - the image to be updated
  //
  //
  // Returns:     None
  //
  // Description: an image has changed its plane order
  //				remove it and add it so that it gets put in the
  //correct 				z order
  //
  //
  // ----------------------------------------------------------------------
  virtual void UpdatePlane(EntityImage *entityImage);

  // ----------------------------------------------------------------------
  // Method:      ChangeDisplayMode
  // Arguments:   width - width to change to
  //				height - height to change to
  //
  // Returns:     true if you choose a legitimate display mode and we
  //				switched to it,  false otherwise.
  //
  // Description: Finds out whether the requested display mode is viable and
  //				switches to it.  For smoother results change the size
  //of 				window too. Note that the display modes are enumrated at 				start up of the
  //engine
  //
  // ----------------------------------------------------------------------
  bool ChangeDisplayMode(uint32 width, uint32 height);

  Position GetBackgroundTopLeft();

  void FadeScreen();

  bool IsPointOnScreen(int32 x, int32 y);

  virtual void Refresh();

  virtual bool TrackObject();
  void StopTracking();
  bool InTrackingRectangle();
  bool InTrackingRectangle(int &offbyx, int &offbyy);
  AgentHandle GetTrackAgent();

  void YourTrackObjectHasGrownUp(AgentHandle &agent);

  void DoTrackingChecks();

  void CentriseCoordinates(int32 &x, int32 &y);
  void CentreOn(int32 x, int32 y, int pan = 0);

  void KeepUpWithMouse(EntityImage *me);
  bool KeepUpThenWithMouse();
  bool MiddleMouseDragMouseWheel();

  void AddFloatingThing(AgentHandle &a);
  void RemoveFloatingThing(AgentHandle &a);

  void ScreenToWorld(int &x, int &y);
  void ScreenToWorld(Vector2D &vec);
  bool WorldToScreen(int &x, int &y);
  bool WorldToScreen(Vector2D &vec);

  void ConvertToWorldCoordinates(RECT &rect);
  void ConvertToDisplayCoordinates(RECT &rect);
  void GetViewCentre(int32 &centrex, int32 &centrey);
  void GetViewArea(RECT &rect);
  virtual void SetViewAndDisplayArea();

  int32 GetViewWidth(void) { return myViewArea.right - myViewArea.left; }
  int32 GetViewHeight(void) { return myViewArea.bottom - myViewArea.top; }
  int32 LeftOfView() { return myViewArea.left; }
  int32 TopOfView() { return myViewArea.top; }

  bool IsSameBackGround(const std::string &background);

  std::string GetBackgroundName() const;

  void SetLoading(bool flag);
  bool GetLoading();

  void Flip(int32 flip);

  int GetTrackTransition() { return myTrackTransition; }
  void SetTrackTransition(int trans) { myTrackTransition = trans; }

  virtual bool IsRemote() { return false; }

  virtual void Render() { ; }

#ifdef C2D_DIRECT_DISPLAY_LIB
  void DrawLineToScreen(int x1, int y1, int x2, int y2, uint8 lineColourRed,
                        uint8 lineColourGreen, uint8 lineColourBlue);

  void DoWackyScroll(bool scroll) { myDoWackyScrollFlag = scroll; }
  void SetFullScreenMode(bool flag) { myFullScreenFlag = flag; }
#endif

  // new serialization stuff
  virtual bool Write(CreaturesArchive &ar) const;
  virtual bool Read(CreaturesArchive &ar);
  bool SaveAsSpriteFile(std::string const &filename) const;

  //////////////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////////////
  class CameraException : public BasicException {
  public:
    CameraException(std::string what, uint32 line)
        : BasicException(what.c_str()), lineNumber(line) {
      ;
    }

    uint32 LineNumber() { return lineNumber; }

  private:
    uint32 lineNumber;
  };

protected:
  // Copy constructor and assignment operator
  // Declared but not implemented
  Camera(const Camera &);
  Camera &operator=(const Camera &);

  // camera direction
  Position myPanVelocity;
  Position myPanVector;
  Position myDistancePanned;
  int32 myAcceleration;
  int32 myMinimumSpeed;

  // the actual size of the area we are displaying
  // based on the size of the current background
  RECT myDisplayRect;

  // the size of my display in pixels
  // starting from the top left world coordinate
  // position
  RECT myViewArea;
  // easier to store these explicitly
  int32 myViewWidth;
  int32 myViewHeight;

  Position myWorldPosition; // this is the view
  AgentHandle myTrackObject;
  int32 myTrackPercentX, myTrackPercentY;
  int32 myTrackStyle;
  bool myTrackInRectangleLastTick;
  Position myTrackPositionLastTick;
  int myTrackMetaRoomLastTick;
  int myTrackTransition;
  bool myMiddleDrag;
  int myMiddleDragMX, myMiddleDragMY;

  // if you want you can alway make sure that the
  // pointer stays a certain
  EntityImage *myObjectToKeepUpWith;
  int32 myKeepUpSpeedX, myKeepUpSpeedY;
  int32 myKeepUpCountX, myKeepUpCountY;
  int32 myKeepUpPanX, myKeepUpPanY;

  bool myChangingRoomsFlag;

  bool myDisabledFlag;

  uint32 myPixelWidth;
  uint32 myPixelHeight;

  // things that need to move with the camera
  std::vector<AgentHandle> myFloatingThings;

  // whether to do complete redraw next frame
  bool myCompleteRedraw;

#ifdef C2D_DIRECT_DISPLAY_LIB
  C2DScreen *myScreen;
  C2DLayer *myPicture;
  C2DSpriteList *myDisplayList;
  bool myDoWackyScrollFlag;
  RECT myMetaRoomBounds; // the absolute limit of the metaroom
  bool myFullScreenFlag;

#else
  CameraSprite *myScreen;
#endif

  bool myLoadingFlag;

  bool myAmIInterestedInCameraShyObjects;

  FilePath myBackground1FilePath;
  FilePath myBackground2FilePath;

  MapImage *myMapImage;
  bool myMapImageDisplayed;

private:
  virtual int GetMetaRoom();
};

#endif // DX_CAMERA_H
