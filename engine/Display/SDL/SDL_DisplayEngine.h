// --------------------------------------------------------------------------
// Filename:	SDL_DisplayEngine.h
// --------------------------------------------------------------------------
#ifndef SDL_DISPLAYENGINE_H
#define SDL_DISPLAYENGINE_H

#ifndef C2E_SDL
#error not C2E_SDL not defined
#endif

#include <SDL/SDL.h>

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#ifndef _WIN32
#include <string.h>
inline void ZeroMemory(void *mem, size_t count) { memset(mem, 0, count); }
#endif

#include "../Bitmap.h"
#include "../Gallery.h"
#include "../Position.h"

#define DISPLAY_MONITOR (1 << 0)
#define DISPLAY_BACKGROUND (1 << 1)
#define DISPLAY_SPRITES (1 << 2)

// const std::string theDisplayErrorTag("display_engine");

///////////////////////////////////////////////////////////////////////////
// Code to switch between pixel formats taken from C2 Renderer
///////////////////////////////////////////////////////////////////////////

enum PIXEL_FORMAT {
  RGB_UNKNOWN = 0,
  RGB_555,
  RGB_565,
};

struct RGB {
  uint8 red;
  uint8 green;
  uint8 blue;
};

// C16 file flags.
const DWORD C16_FLAG_565 = 0x00000001;
const DWORD C16_16BITFLAG = 0x00000002;

#define RGB_TO_565(r, g, b, result)                                            \
  result = (((r)&0xf8) << 8) | (((g)&0xfc) << 3) | (((b)&0xf8) >> 3);

#define RGB_TO_555(r, g, b, result)                                            \
  result = (((r)&0xf8) << 7) | (((g)&0xf8) << 2) | (((b)&0xf8) >> 3);

#define P565_TO_RGB(pixel, r, g, b)                                            \
  (r) = ((pixel) >> 8);                                                        \
  (g) = ((pixel) >> 3);                                                        \
  (b) = ((pixel) << 3);

#define P555_TO_RGB(pixel, r, g, b)                                            \
  (r) = ((pixel) >> 7);                                                        \
  (g) = ((pixel) >> 2);                                                        \
  (b) = ((pixel) << 3);

class Background;
class Bitmap;
class CompressedBitmap;
class NormalGallery;
class MapImage;
class DrawableObjectHandler;
class EntityImage;

class DisplayEngine //: public Engine
{
public:
  DisplayEngine(int do_not_call_me, int as_i_never_return) {
    while (true)
      ;
  }

  enum SIDText {
    sidNoBackgroundCreated = 0,
    sidNoGalleryCreated,
    sidNoGalleryFound,
    sidGeneralDirectDrawError,
    sidGeneralBackgroundDrawingError,
    sidGeneralSpriteDrawingError,
    sidNoImagesDirectory,
    sidEmptyGalleryString,
    sidConvertingGraphics,
    sidDirectDrawNotCreated,
    sidGalleryNotCreated,
    sidGalleryNotFound,
    sidGalleryUnknownType,
    sidMapImageNotCreated,
    sidMissingGallery,
    sidGalleryNotSpecified,
    sidCreatureGalleryCorrupt,
    sidNoSecondaryCameraCreated,
    sidBitmapSizeTooSmall,
    sidInvalidBasePose,
    sidDodgyPixelFormat
  };

  // constructors are private to ensure only one object
  // ever exists

  // destructor
  virtual ~DisplayEngine(void);

  static DisplayEngine &theRenderer();

  void CreateTest();

  void Stop(void);

  //	bool Check16BitFormat(uint32 storedDisplayType,
  //						std::vector<std::string>&
  // foldersToConvert);

  //	void DoConversion(std::string& imagepath);

  void DrawSpriteToBitmap(Bitmap *destination, Position pos,
                          Bitmap *const source, uint16 textColourRef = 0,
                          uint16 backgroundColourRef = 0);

  void DrawLine(int32 x1, int32 y1, int32 x2, int32 y2, uint8 lineColourRed = 0,
                uint8 lineColourGreen = 0, uint8 lineColourBlue = 0,
                uint8 stippleon = 0, uint8 stippleoff = 0,
                uint32 stipplestartAt = 0);

  void DrawLineToBitmap(Bitmap *bitmap, int32 x1, int32 y1, int32 x2, int32 y2,
                        uint8 lineColourRed = 0, uint8 lineColourGreen = 0,
                        uint8 lineColourBlue = 0);

  void GetNextWord(std::string &sentence, std::string &word);

  ////////////////////////////////////////////////////////////////////////////
  // Get and Set Methods...
  ////////////////////////////////////////////////////////////////////////////

  inline uint32 &Flags(void);

  inline void SetFlags(uint32 flags);

  int32 GetSurfaceWidth(void) { return mySurfaceArea.right; }
  int32 GetSurfaceHeight(void) { return mySurfaceArea.bottom; }

  void GetSurfaceArea(RECT &rect) {
    rect.top = mySurfaceArea.top;
    rect.bottom = mySurfaceArea.bottom;
    rect.left = mySurfaceArea.left;
    rect.right = mySurfaceArea.right;
  }

  void ClientToScreen(int32 &x, int32 &y);
  void ScreenToClient(int32 &x, int32 &y);

  inline bool IsFullScreen();

  bool ProgressBarAlreadyStarted() { return myProgressBarHasBeenStarted; }

  ////////////////////////////////////////////////////////////////////////////
  // Rendering Methods
  ////////////////////////////////////////////////////////////////////////////

  // dummy function needed because is a pure
  // virtual in the base class.  Must decide
  // whether to remove it from engine.
  virtual void Update() {}

  void DrawToFrontBuffer();

  inline void DrawBitmap(Position &position, Bitmap &bitmap);
  inline void DrawWholeBitmapRegardless(Position &position, Bitmap &bitmap);

  void DrawSprite(Position &position, Bitmap &bitmap);
  void DrawCompressedSprite(Position &position, CompressedBitmap *bitmap);
  void DrawMirroredCompressedSprite(Position &position,
                                    CompressedBitmap *bitmap);
  void DrawAlphaCompressedSprite(Position &position, CompressedBitmap *bitmap,
                                 int intensity);
  void DrawAlphaSprite(Position &position, Bitmap &bitmap, int intensity);

  void DrawMirroredSprite(Position &position, Bitmap &bitmap);

  uint16 *GetBackBufferPtr() {
    if (myCurrentOffScreenBufferPtr)
      return myCurrentOffScreenBufferPtr;
    else
      return OpenBackBuffer();
  }

  // Switch to the next fullscreen mode
  // (this function replaces directx version ChangeDisplayMode() )
  void NextScreenMode();

  bool ToggleFullScreenMode();

  // pass new width and height to display
  void ResizeWindow(int w, int h);

  void MoveWindow();
  void MoveWindow(int32 x, int32 y);

  // store the file that is currently being converted
  // so that if there are any interruptions we can deal
  // with them.
  static bool StoreFileBeingConverted(std::string &fileName);

  static bool FileBeingConverted(std::string &fileName);

  bool SafeImageConvert(std::string &name, std::string &, PIXEL_FORMAT To,
                        std::string &pPrevFileName);

  void FadeScreen();

  bool Start(uint32 storedDisplayType,
             std::vector<std::string> &foldersToConvert,
             std::string const &transitionGallery, bool fullScreen = false);

  void Update(Background *background, DrawableObjectHandler *entityHandler,
              bool completeRedraw, bool justBackBuffer,
              SDL_Surface *surfaceToDrawOn = NULL);

  //	void ResizeWindow(RECT& rect,UINT flags  = SWP_SHOWWINDOW);
  //	static HRESULT CALLBACK EnumModesCallback(  LPDDSURFACEDESC2
  // lpDDSurfaceDesc,
  // LPVOID lpContext);

  SDL_Rect ConvertRect(RECT &rect);
  bool BlitToFrontBuffer(RECT &destination, SDL_Surface *image, RECT &source,
                         bool transparencyAware);

  bool BlitToBackBuffer(RECT &destination, SDL_Surface *image, RECT &source,
                        bool transparencyAware);

  SDL_Surface *CreateSurface(int32 width, int32 height,
                             bool tryVideoFirst = false);

  void ReleaseSurface(SDL_Surface *&tempSurface);

  bool FlipScreenHorizontally();
  void FlipScreenVertically();
  bool SlideScreen();
  void Shrink(int32 x, int32 y);
  void Burst();

  inline void ChangeSuspend(bool start);

  void PrepareForMessageBox();
  void EndMessageBox();

  static bool ClipLine(RECT *mb, int &x0, int &y0, int &x1, int &y1,
                       unsigned char colour);

  static int CalcOutCode(RECT *mb, int x, int y);

  uint16 ConvertRGB(int r, int g, int b);

  bool CreateProgressBar(Bitmap *bitmap);
  void StartProgressBar(int updateIntervals);
  void UpdateProgressBar(int amount);
  void EndProgressBar();

  bool RenderBitmapToFrontBuffer(Bitmap *bitmap);

  // TODO: merge these two fns. sigh.
  PIXEL_FORMAT GetMyPixelFormat() { return myPixelFormat; }
  bool GetPixelFormat(uint32 &format) {
    format = (uint32)myPixelFormat;
    return true;
  }

  void SetDesiredRoundness(bool howFlat) { myDesiredWorldRoundness = howFlat; }

  bool ShouldIRenderTheEntireMainCameraOrNot() {
    return myDesiredWorldRoundness || myIsTheWorldRoundFlag;
  }

  void DrawGuttering(int width, int height);

private:
  // ----------------------------------------------------------------------
  // Method:      Constructors
  // Description: These are private so that only I can create the
  //				single displayengine
  //
  // ----------------------------------------------------------------------
  DisplayEngine();

  DisplayEngine(uint32 flags);

  // Copy constructor and assignment operator
  // Declared but not implemented
  DisplayEngine(const DisplayEngine &);
  DisplayEngine &operator=(const DisplayEngine &);

  // set up screenmode (modenum is index into
  bool SetMode(int modenum);
  // enter or resize windowed mode
  bool SetModeWindowed(int w, int h);

  bool GetDrawingParameters(Position &position, Bitmap *bitmap,
                            uint32 &data_step, uint16 *&data_ptr,
                            uint32 &screen_step, uint16 *&screen_ptr,
                            int32 &bitmapWidth, int32 &bitmapHeight);

  bool GetCompressedDrawingParameters(
      Position &position, CompressedBitmap *bitmap, uint32 &data_step,
      uint8 *&compressedData_ptr, uint32 &screen_step, uint16 *&screen_ptr,
      int32 &bitmapWidth, int32 &bitmapHeight, bool &rightClip,
      bool &bottomClip, bool &topClip, bool &leftClip);

  bool GetCompressedDrawingParameters16Bit(
      Position &position, Bitmap *bitmap, uint32 &data_step,
      uint16 *&compressedData_ptr, uint32 &screen_step, uint16 *&screen_ptr,
      int32 &bitmapWidth, int32 &bitmapHeight, bool &rightClip,
      bool &bottomClip, bool &topClip);

  void CreateUserInterfaceGalleries();

  uint16 *OpenBackBuffer(void);
  inline void CloseBackBuffer(void);

  // fills front and back buffers with black
  void ClearBuffers(void);

  // merge these into one function?
  bool CreateFullscreenDisplaySurfaces(void);
  bool CreateWindowedDisplaySurfaces(void);

  bool DoChangeScreenMode(bool toWindowedMode);

  void StarBurst();

  // sets up myPixelFormat and myFullScreenModes
  void SussScreenModes();

  // the one and only display engine
  // accessible by through me only
  static DisplayEngine myRenderer;

  // these tell me which components I should
  // draw e.g background, sprites...
  uint32 myFlags;

  bool myWaitingForMessageBoxFlag;

  // the size of my direct draw surfaces
  static RECT ourSurfaceArea;

  // these 4 are set each Update()
  // pointer to the start of the back buffer surface
  uint16 *myCurrentOffScreenBufferPtr;
  int32 myPitch; // in 16bit pixels?
  int32 myPitchForBackgroundTiles;
  RECT mySurfaceArea; // tempory copy set during update (may not be main suface)

  PIXEL_FORMAT myPixelFormat;
  std::vector<SDL_Rect> myFullScreenModes;
  bool myFullScreenFlag;
  int myCurrentFullScreenMode;
  int myWindowedModeW;
  int myWindowedModeH;

  Gallery *myTransitionGallery;

  Bitmap *myProgressBitmap;
  int myProgressMax;
  int myProgressCount;
  bool myProgressBarHasBeenStarted;
  int myPreviousProgressRight;

  SDL_Surface *myProgressSurface;
  SDL_Surface *mySpriteSurface;

  // our virtual framebuffer
  SDL_Surface *myBackBuffer;

  // note whether I have been started up properly
  bool myEngineRunningFlag;

  bool myIsTheWorldRoundFlag;   // These are for the EasterEgg
  bool myDesiredWorldRoundness; // Ask Frankie for more info :)
};

// inline blit functions...
// for non-sdl version this is included from Bitmap.cpp
// for some reason...
#include "../DisplayEnginePlotFunctions.h"

inline bool DisplayEngine::IsFullScreen() {
  return (myFullScreenFlag == true) ? true : false;
}

inline uint32 &DisplayEngine::Flags(void) { return myFlags; }

inline void DisplayEngine::SetFlags(uint32 flags) { myFlags = flags; }

inline void DisplayEngine::ChangeSuspend(bool start) {
  //	OutputDebugString("ChangeSusp\n");
  myEngineRunningFlag = start;
}

inline void DisplayEngine::CloseBackBuffer(void) {
  if (myBackBuffer) {
    SDL_UnlockSurface(myBackBuffer);
    myCurrentOffScreenBufferPtr = NULL;
  }
}

#endif // SDL_DISPLAYENGINE_H
