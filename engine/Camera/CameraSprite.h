// --------------------------------------------------------------------------
// Filename:	DrawingSurface.h
// Class:		DrawingSurface
// Purpose:		This class provides a drawing surface that can be passed
// to the
//				display engine.
//
//
//
// Description:
//
//
//
//

//
// History:
// -------
// 2Mar00	Alima			Created.
// --------------------------------------------------------------------------
#ifndef CAMERA_SPRITE_H
#define CAMERA_SPRITE_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../Display/Background.h"
#include "../Display/DrawableObject.h"
#ifdef C2E_SDL
#include <SDL/SDL.h>
#else
#include <ddraw.h>
#endif
#include "../Display/DrawableObjectHandler.h"
#include "../FilePath.h"

class CameraSprite : public DrawableObject {
public:
  CameraSprite();
  virtual ~CameraSprite();

  bool Create(std::string defaultBackground,
              int32 topLeftXofBackground, // top left world co-ordinates
              int32 topLeftYofBackground, Position worldPosition);

  bool Create(std::string gallery_name, RECT &bounds, Position displayPosition);

  bool Create(uint8 i, Position pos, std::string &gallery_name,
              Position topLeft, uint32 width, uint32 height);

  /////////////////////////////////////////////////////////////
  // Background access
  /////////////////////////////////////////////////////////////
  Position GetDisplayPosition(uint8 i) const {
    return myBackgrounds[i]->GetDisplayPosition();
  }

  int32 GetPixelWidth(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetPixelWidth();
  }
  int32 GetPixelHeight(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetPixelHeight();
  }

  void SetDisplayPosition(Position &position) {
    myBackgrounds[0]->SetDisplayPosition(position);
  }

  void GetConsideredDisplayArea(uint8 i, int32 &displayWidth,
                                int32 &displayHeight) {
    _ASSERT(i <= 1);
    myBackgrounds[i]->GetConsideredDisplayArea(displayWidth, displayHeight);
  }

  int32 GetTop(uint8 i) {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetTop();
  }
  int32 GetLeft(uint8 i) {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetLeft();
  }

  Position GetBackgroundTopLeft(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetTopLeft();
  }

  std::string GetBackgroundName(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetBackgroundName();
  }

  std::string GetBackgroundPath(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetBackgroundPath();
  }

  uint32 GetWidth(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetWidth();
  }

  uint32 GetHeight(uint8 i) const {
    _ASSERT(i <= 1);
    return myBackgrounds[i]->GetHeight();
  }

  void SwapBackgrounds();

  void Update(Position displayPosition, bool completeRedraw,
              bool justBackBuffer);

  void SetInterestLevel(bool flag) { myEntityHandler.SetInterestLevel(flag); }

  bool IsRectOnScreen(RECT &rect) {
    return myEntityHandler.IsRectOnScreen(rect, myViewArea);
  }
  void SetViewArea(RECT &area);

  void GetViewArea(RECT &rect);

  Position GetWorldPosition(void) { return myEntityHandler.GetWorldPosition(); }

  void SetWorldPosition(Position pos) { myEntityHandler.SetWorldPosition(pos); }

  bool Visible(RECT &test);

  bool SaveAsSpriteFile(std::string const &filename) const;

  virtual void SetCurrentBound(RECT *rect = NULL);

  void SetScreenBound(RECT *rect = NULL);

  RECT GetCurrentBound() { return myCurrentBound; }

  RECT GetScreenBound() const { return myScreenBound; }

  // and a screen position
  virtual void SetScreenPosition(Position pos);

  virtual void Draw();
  virtual void DrawMirrored() { Draw(); }

  void SetViewToPhysicalWidthAndHeightRatio(
      float myOriginalViewToPhysicalHeightRatio,
      float myOriginalViewToPhysicalWidthRatio);

  bool SetUpDrawingSurface(float myOriginalViewToPhysicalHeightRatio,
                           float myOriginalViewToPhysicalWidthRatio);

  bool Remove(DrawableObject *const chop) {
    return myEntityHandler.Remove(chop);
  }
  bool Add(DrawableObject *const newThing) {
    return myEntityHandler.Add(newThing);
  }

  void MakeSetCurrentBoundsGetCalled() {
    myEntityHandler.MakeSetCurrentBoundsGetCalled();
  }

  void SetRemoteCameraFlag(bool flag) { myAmRemoteCameraFlag = flag; }

  void NotifyCameraIsShuttingDown() { myEntityHandler.ShutDown(); }

  void AssertSurface() { ASSERT(mySurface); }

private:
  DrawableObjectHandler myEntityHandler;
  // empty shells of 2 backgrounds
  // to take the incoming and outgoing backgrounds
  Background myBackground0;
  Background myBackground1;

  // so that i can index between the ingoing and outgoing
  // backgrounds and flip them about of necessary
  // the first is the incoming background
  // the second is the outgoing background
  std::vector<Background *> myBackgrounds;

#ifdef C2E_SDL
  SDL_Surface *mySurface;
#else
  IDirectDrawSurface4 *mySurface;
#endif

  RECT myViewArea;

  RECT myScreenBound;

  float myOriginalViewToPhysicalHeightRatio;
  float myOriginalViewToPhysicalWidthRatio;
};

#endif // CAMERA_SPRITE_H
