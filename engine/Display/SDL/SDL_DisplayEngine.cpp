// --------------------------------------------------------------------------
// Filename:	SDL/SDL_DisplayEngine.cpp
//
// A non-directx version of the DisplayEngine
//
//
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../../../common/C2eTypes.h"

// Nasm declarations (the variables are
// defined in Bitmap.cpp, the functions
// in Plot.asm).  For now we use these
// global variables even in the non-asm
// version - hopefully won't slow it down!
extern int32 bitmapHeight;
extern int32 bitmapWidth;
extern uint16 *data_ptr;
extern uint16 *screen_ptr;
extern uint32 data_step;
extern uint32 screen_step;
extern int dwordWidth;
extern uint16 *compressedData_ptr;

#ifdef C2E_NASM_RENDERING
extern "C" {
void NasmDrawSprite();
void NasmDrawCompressedSprite();
}
#endif

#include "../../../common/FileFuncs.h"
#include "../../AgentDisplay/EntityImage.h"
#include "../../App.h"
#include "../../C2eServices.h"
#include "../../Camera/MapImage.h"
#include "../../File.h"
#include "../../ProgressDialog.h"
#include "../Background.h"
#include "../ClonedGallery.h"
#include "../CompressedBitmap.h"
#include "../DisplayEngine.h"
#include "../DisplayErrorConstants.h"
#include "../DrawableObjectHandler.h"
#include "../EasterEgg.h"
#include "../ErrorMessageHandler.h"
#include "../SharedGallery.h"
#include "../System.h"
#include "SDLStretch/SDLStretch.h"
#include <string>

#include <algorithm>

DisplayEngine DisplayEngine::myRenderer;
RECT DisplayEngine::ourSurfaceArea;

DisplayEngine &DisplayEngine::theRenderer() { return myRenderer; }

DisplayEngine::DisplayEngine() {
  myCurrentOffScreenBufferPtr = 0;
  myBackBuffer = NULL;
  myPixelFormat = RGB_UNKNOWN;
  myWaitingForMessageBoxFlag = 0;
  myProgressBitmap = NULL;
  myProgressSurface = NULL;
  myProgressBarHasBeenStarted = 0;
  myFlags = 0;
  myWindowedModeW = 800;
  myWindowedModeH = 600;
  myCurrentFullScreenMode = 0;
  myIsTheWorldRoundFlag = false;
  myDesiredWorldRoundness = false;
}

DisplayEngine::DisplayEngine(uint32 flags) {
  myCurrentOffScreenBufferPtr = 0;
  myBackBuffer = NULL;
  myPixelFormat = RGB_UNKNOWN;
  myWaitingForMessageBoxFlag = 0;
  myProgressBitmap = NULL;
  myProgressSurface = NULL;
  myProgressBarHasBeenStarted = 0;
  myFlags = flags;
  myWindowedModeW = 800;
  myWindowedModeH = 600;
  myCurrentFullScreenMode = 0;
  myIsTheWorldRoundFlag = false;
  myDesiredWorldRoundness = false;
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
DisplayEngine::~DisplayEngine(void) {}

// ----------------------------------------------------------------------
// Method:      ClearBuffers
// Arguments:   None
//
// Returns:     None
//
// Description: fills all front and back buffers with black
//
// ----------------------------------------------------------------------
void DisplayEngine::ClearBuffers(void) {
  // TODO: clear myBackBuffer here.
}

// ----------------------------------------------------------------------
// Method:      Start
// Arguments:   window - handle of a window to associate the engine with
//				fullScreen - true if this engine should run full
// screen 								false if
// it should run in a window.
//
// Returns:     true if the engine has started up OK false otherwise
//
// Description: Creates the direct draw object and surfaces either for
//				fullscreen mode or for windowed mode - the set
// up is 				slightly different
//
// ----------------------------------------------------------------------
bool DisplayEngine::Start(uint32 storedDisplayType,
                          std::vector<std::string> &foldersToConvert,
                          std::string const &transitionGallery,
                          bool fullScreen /*= false*/) {
  // TODO: transitionGallery?

  // sort out myPixelFormat and myFullScreenModes vector
  SussScreenModes();

  // look for 800x600
  int i;
  myCurrentFullScreenMode = 0;

  for (i = 0; i < myFullScreenModes.size(); ++i) {
    if (myFullScreenModes[i].w == 800 && myFullScreenModes[i].h == 600) {
      myCurrentFullScreenMode = i;
    }
  }

  if (fullScreen) {
    if (!SetMode(myCurrentFullScreenMode))
      return false;
  } else {
    if (!SetModeWindowed(800, 600))
      return false;
  }

  myEngineRunningFlag = true;

  return true;
}

void DisplayEngine::SussScreenModes() {
  const SDL_VideoInfo *vidinfo = SDL_GetVideoInfo();

  printf("\nVideo information:\n");
  printf("  %d bits per pixel, ", vidinfo->vfmt->BitsPerPixel);
  printf("%d bytes per pixel\n", vidinfo->vfmt->BytesPerPixel);
  printf("  rmask: 0x%x ", vidinfo->vfmt->Rmask);
  printf("gmask: 0x%x ", vidinfo->vfmt->Gmask);
  printf("bmask: 0x%x\n", vidinfo->vfmt->Bmask);

  if (vidinfo->vfmt->BitsPerPixel == 16 && vidinfo->vfmt->BytesPerPixel == 2 &&
      vidinfo->vfmt->Rmask == 0x7c00 && vidinfo->vfmt->Gmask == 0x03e0 &&
      vidinfo->vfmt->Bmask == 0x001f) {
    myPixelFormat = RGB_555;
    printf("  Display 555 format, also known as 15 bit\n");
  } else if (vidinfo->vfmt->BitsPerPixel == 16 &&
             vidinfo->vfmt->BytesPerPixel == 2 &&
             vidinfo->vfmt->Rmask == 0xf800 && vidinfo->vfmt->Gmask == 0x07e0 &&
             vidinfo->vfmt->Bmask == 0x001f) {
    myPixelFormat = RGB_565;
    printf("  Display 565 format\n");
  } else {
    // for all other pixelformats, we just use 565.
    // SDL will create a shadow surface and convert on the fly.
    myPixelFormat = RGB_565;
    printf("  Display is %d bit\n", vidinfo->vfmt->BitsPerPixel);
    printf("  WARNING - there will be a performance hit.\n");
    printf("            For best results, set your display depth\n");
    printf("            to 16 (or 15) bits in X configuration.\n");
  }

  // See what fullscreen resolutions we can run in
  printf("  Available fullscreen modes: ");
  SDL_Rect **r;
  r = SDL_ListModes(NULL, SDL_FULLSCREEN);
  if (r && r != (SDL_Rect **)-1) {
    while (*r) {
      printf("%dx%d ", (*r)->w, (*r)->h);
      myFullScreenModes.push_back(**r);
      ++r;
    }
  }
  printf("\n");
}

bool DisplayEngine::SetModeWindowed(int w, int h) {
  //	printf("SetModeWindowed %d,%d\n",w,h );
  ASSERT(myPixelFormat == RGB_555 || myPixelFormat == RGB_565);

  int bitdepth = (myPixelFormat == RGB_555) ? 15 : 16;

#ifdef _WIN32
  //	myBackBuffer = SDL_SetVideoMode( w, h, 16, 0);
  myBackBuffer = SDL_SetVideoMode(w, h, bitdepth, SDL_RESIZABLE);
#else
  myBackBuffer = SDL_SetVideoMode(w, h, bitdepth, SDL_RESIZABLE);
#endif
  if (!myBackBuffer) {
    // TODO: use logfile?
    printf("SDL_SetVideoMode() failed: %s\n", SDL_GetError());
    return false;
  }
  ourSurfaceArea.left = 0;
  ourSurfaceArea.top = 0;
  ourSurfaceArea.right = w;
  ourSurfaceArea.bottom = h;
  myFullScreenFlag = false;

  // store w & h so we can restore size after a switch to fullscreen mode
  myWindowedModeW = w;
  myWindowedModeH = h;

  ClearBuffers();
  return true;
}

bool DisplayEngine::SetMode(int modenum) {
  //	printf("SetMode %d\n",modenum);

  ASSERT(myPixelFormat == RGB_555 || myPixelFormat == RGB_565);

  if (myFullScreenModes.empty())
    return SetModeWindowed(myWindowedModeW, myWindowedModeH);

  ASSERT(modenum >= 0 && modenum < myFullScreenModes.size());

  int w = myFullScreenModes[modenum].w;
  int h = myFullScreenModes[modenum].h;
  int bitdepth = (myPixelFormat == RGB_555) ? 15 : 16;
  bitdepth = 16;

  myBackBuffer = SDL_SetVideoMode(w, h, bitdepth, SDL_FULLSCREEN);
  if (!myBackBuffer) {
    // TODO: could return some meaningful SDL_GetError() text here...
    printf("SDL_SetVideoMode() failed: %s\n", SDL_GetError());
    return false;
  }

  myCurrentFullScreenMode = modenum;
  ourSurfaceArea.left = 0;
  ourSurfaceArea.top = 0;
  ourSurfaceArea.right = w;
  ourSurfaceArea.bottom = h;
  myFullScreenFlag = true;

  ClearBuffers();
  return true;
}

void DisplayEngine::MoveWindow() {}

void DisplayEngine::MoveWindow(int32 x, int32 y) {}

void DisplayEngine::ResizeWindow(int w, int h) {
  // this'll turn off fullscreen mode if it is on...
  SetModeWindowed(w, h);
}

void DisplayEngine::CreateUserInterfaceGalleries() {
#ifdef WORK_IN_PROGRESS
  FilePath path("trans.s16", IMAGES_DIR);

  if (!FileExists(path.GetFullPath().c_str()))
    return;

  // if this doesn't get created it's no big deal.
  myTransitionGallery = new Gallery(path);

  if (!myTransitionGallery)
    return;

  Bitmap *bitmap = myTransitionGallery->GetBitmap(0);

  if (!bitmap)
    return;

  mySpriteSurface =
      CreateSurface(bitmap->GetWidth(), bitmap->GetHeight(), true);

  if (!bitmap || !mySpriteSurface)
    return;

  DDSURFACEDESC2 surfaceDescription;
  ZeroMemory(&surfaceDescription, sizeof(DDSURFACEDESC2));

  surfaceDescription.dwSize = sizeof DDSURFACEDESC2;

  HRESULT res =
      mySpriteSurface->Lock(NULL, &surfaceDescription, DDLOCK_WAIT, NULL);

  if (res == DD_OK) {
    DDCOLORKEY color_key = {0, 0};

    mySpriteSurface->SetColorKey(DDCKEY_SRCBLT, &color_key);

    const uint16 *sourcePtr = bitmap->GetData();
    uint16 *destPtr = (uint16 *)surfaceDescription.lpSurface;

    int32 bitmapWidth = bitmap->GetWidth();
    int32 bitmapHeight = bitmap->GetHeight();

    // the surface is created to be the same
    // size as the entity bounds
    int32 destStep = (surfaceDescription.lPitch >> 1);
    int32 sourceStep = 0;
    destStep = destStep - bitmapWidth;
    for (; bitmapHeight--;) {
      for (int32 width = bitmapWidth; width--;)
        *destPtr++ = *sourcePtr++;

      destPtr += destStep;
    }
    mySpriteSurface->Unlock(NULL);
  }
#endif
}

// TODO: fix ickiness - pixelconversion is pretty convoluted at the moment
// pPrevFileName is not even used.
bool DisplayEngine::SafeImageConvert(std::string &name,
                                     std::string &tmpFileName, PIXEL_FORMAT To,
                                     std::string &pPrevFileName) {
  bool bConvert = false;
  PIXEL_FORMAT From;
  try {
    File f(name, GENERIC_READ);

    if (f.Valid()) {
      uint32 dwFlags;
      if (f.Read(&dwFlags, sizeof(uint32))) {
        if ((dwFlags & C16_FLAG_565) && (To == RGB_555)) {
          From = RGB_565;
          bConvert = true;
        }

        if (!(dwFlags & C16_FLAG_565) && (To == RGB_565)) {
          From = RGB_555;
          bConvert = true;
        }
      }
      f.Close();
    }
  } catch (File::FileException &) {
    std::cerr << "DisplayEngine::SafeImageConvert FileException " << name
              << std::endl;
    return false;
  }

  // ****************Changes
  if (bConvert) {
    /* std::cout << "DisplayEngine::SafeImageConvert() '" << name << "' to ";
    if( To == RGB_555 )
            std::cout << "555... " << std::endl;
    else if( To == RGB_565 )
            std::cout << "565... " << std::endl;
    else
            std::cout << "???... " << std::endl; */

    if (!CopyFile(name.c_str(), tmpFileName.c_str(), true)) {
      std::cerr << "Copy temp '" << name << "' to '" << tmpFileName
                << "' failed" << std::endl;
      perror("Reason for failure:");
      return false;
    };

    SharedGallery::theSharedGallery().ConvertGallery(tmpFileName, To);

    if (!CopyFile(tmpFileName.c_str(), name.c_str(), true)) {
      std::cerr << "Copy back '" << tmpFileName << "' to '" << name
                << "' failed" << std::endl;
      perror("Reason for failure:");
      return false;
    }

    // std::cout << "Conversion done" << std::endl;
  }

  return true;
}

#ifdef LEFT_IN_FOR_REFERENCE

bool DisplayEngine::Check16BitFormat(
    uint32 storedDisplayType, std::vector<std::string> &foldersToConvert) {
  uint32 format = 0;
  if (!GetPixelFormat(format)) {
    theApp.myQuitNextTick = true;
    return false;
  }

  myPixelFormat = PIXEL_FORMAT(format);

  if (storedDisplayType == RGB_UNKNOWN) {
    // Assume just installed.
    // This is OK now as the S16 files say what they contain.
    storedDisplayType = RGB_565;
  }

  // if the current pixel format is different to the video
  // card's format then we need to change
  if (storedDisplayType != myPixelFormat) {
    // tell 'em what we are going to do...

    for (int i = 0; i < foldersToConvert.size(); ++i) {
      DoConversion(foldersToConvert[i]);
    }
  }

  // Store new display type in registry.
  int data = myPixelFormat;
  std::string valueName("Display Type");
  theApp.MachineSettings().Set(valueName, data);
  // make sure config file is updated in case we crash :-)
  theApp.MachineSettings().Flush();

  return true;
}

void DisplayEngine::DoConversion(std::string &imagePath) {
  std::string msg =
      theCatalogue.Get(theDisplayErrorTag, (int)sidConvertingGraphics);
  msg += imagePath;

  ProgressDialog progress(myWindow);
  progress.SetText(msg);

  bool status_ok = true;

  if (status_ok) {
    HFILE file_handle;
    int path_position;
    struct _finddata_t file_data;

    std::string prevFileName;
    std::string tmpFilePath;

    path_position = imagePath.size() + 1;
    tmpFilePath = imagePath;
    tmpFilePath += "*.*";

    // count the images
    // Count images.
    int32 imageCount = 0;
    file_handle = _findfirst(tmpFilePath.c_str(), &file_data);
    while (_findnext(file_handle, &file_data) == 0)
      imageCount++;
    _findclose(file_handle);

    progress.SetCounterRange(imageCount);

    file_handle = _findfirst(tmpFilePath.c_str(), &file_data);

    // no files in this directory!!!
    if (file_handle == -1) {
      status_ok = FALSE;
    } else {

      std::string filename;
      // the first file will always be the root directory
      while (_findnext(file_handle, &file_data) == 0) {
        progress.AdvanceProgressBar();
        filename = file_data.name;
        if (filename == "." || filename == ".." || filename == "tmp.blk" ||
            filename == "tmp.s16" || filename == "tmp.c16")
          continue;

        int x = filename.find_last_of(".");

        if (x == -1) {
          continue;
        }

        //************ Changes
        std::string ext = filename.substr(x, x + 3);
        tmpFilePath = imagePath;

        bool convertFile = false;
        if (ext == ".S16" || ext == ".s16") {
          convertFile = true;
          tmpFilePath += "tmp.s16";
        }

        if (ext == ".C16" || ext == ".c16") {
          convertFile = true;
          tmpFilePath += "tmp.c16";
        }

        if (ext == ".BLK" || ext == ".blk" || ext == ".Blk") {
          convertFile = true;
          tmpFilePath += "tmp.blk";
        }

        if (convertFile) {
          filename = imagePath + filename; // file_data.name;

          //************ End Changes
          SafeImageConvert(filename, tmpFilePath, myPixelFormat, prevFileName);
        }
      }
      _findclose(file_handle);
    }

  } else {
    ErrorMessageHandler::Show(theDisplayErrorTag,
                              (int)DisplayEngine::sidNoImagesDirectory,
                              std::string("DisplayEngine::DoConversion"));
  }
}

// taken from SFC tested but not commented
bool DisplayEngine::SafeImageConvert(std::string &name,
                                     std::string &tmpFileName, PIXEL_FORMAT To,
                                     std::string &pPrevFileName) {
  bool bConvert = FALSE;
  PIXEL_FORMAT From;
  try {
    // If file is not already converted, back it up.
    File f(name);

    if (f.Valid()) {
      uint32 dwFlags;
      if (f.Read(&dwFlags, sizeof(uint32))) {
        if ((dwFlags & C16_FLAG_565) && (To == RGB_555)) {
          From = RGB_565;
          bConvert = true;
        }

        if (!(dwFlags & C16_FLAG_565) && (To == RGB_565)) {
          From = RGB_555;
          bConvert = true;
        }
      }
      f.Close();
    }

    // ****************Changes
    if (bConvert) {
      CopyFile(name.c_str(), tmpFileName.c_str(), false);
      StoreFileBeingConverted(name);

      SharedGallery::theSharedGallery().ConvertGallery(
          FilePath(tmpFileName, -1), To);

      CopyFile(tmpFileName.c_str(), name.c_str(), false);

      StoreFileBeingConverted(std::string(""));
    }

  } catch (File::FileException &) {
    return false;
  }

  return true;
}

#endif // LEFT_IN_FOR_REFERENCE

// ----------------------------------------------------------------------
// Method:      Stop
// Arguments:   None
// Returns:     None
//
// Description: Releases all the surfaces
//
// ----------------------------------------------------------------------
void DisplayEngine::Stop(void) {
#ifdef WORK_IN_PROGRESS
  if (mySpriteSurface)
    mySpriteSurface->Release();
#endif

  if (myTransitionGallery)
    delete myTransitionGallery;
}

////////////////////////////////////////////////////////////////////////////
// Rendering Methods
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      Update
// Arguments:   background - a background to draw
//				entityHandler - list of drawable objects
//				monitor - not used yet
//				completeRedraw - wether we are drawing the whole
// scene or just dirty rects
//
// Returns:     None
//
// Description: Draw the background, then the sprites, then monitor
//				to the backbuffer.
//
// ----------------------------------------------------------------------
void DisplayEngine::Update(Background *background,
                           DrawableObjectHandler *entityHandler,
                           bool completeRedraw, bool justBackBuffer,
                           SDL_Surface *surfaceToDrawOn /*= NULL*/) {
  // nasty hack to stop screen redraws while world is loading...
  // (ugh.)
  if (myProgressBarHasBeenStarted)
    return;

  if (myEngineRunningFlag == false) {
    return;
  }

  int oldPitch = myPitch;
  // only open the back buffer once
  // this update method will be called repeatedly by all
  // and sundry
  if (surfaceToDrawOn == NULL) {
    if (!OpenBackBuffer()) {
      CloseBackBuffer();
      return;
    }
    mySurfaceArea = ourSurfaceArea;
  } else {
    // or draw on your own private surface - this is used for
    // remote cameras when other methods ask for the backbuffer they'll
    // get this one by default
    CloseBackBuffer();

    if (SDL_LockSurface(surfaceToDrawOn) == -1) {
      ASSERT(false);
      return;
    }

    myCurrentOffScreenBufferPtr = (uint16 *)surfaceToDrawOn->pixels;
    myPitch = surfaceToDrawOn->pitch >> 1;
    myPitchForBackgroundTiles = (myPitch - 128) * 2;
    mySurfaceArea.left = 0;
    mySurfaceArea.top = 0;
    mySurfaceArea.right = surfaceToDrawOn->w;
    mySurfaceArea.bottom = surfaceToDrawOn->h;
  }

  // draw the background parts
  background->Draw(completeRedraw, entityHandler->GetUpdateList(),
                   entityHandler->GetDirtyTileList());
  // draw the sprites
  entityHandler->Draw(completeRedraw);
  DrawGuttering(background->GetPixelWidth(), background->GetPixelHeight());

  // tidy up the open backbuffers
  // making sure we close the right version
  if (surfaceToDrawOn == NULL) {
    CloseBackBuffer();
  } else {
    SDL_UnlockSurface(surfaceToDrawOn);
    mySurfaceArea = ourSurfaceArea;
    myCurrentOffScreenBufferPtr = NULL;
    myPitch = oldPitch;
    myPitchForBackgroundTiles = (myPitch - 128) * 2;
  }

  if (!justBackBuffer) {
    DrawToFrontBuffer();
  }
}

uint16 *DisplayEngine::OpenBackBuffer(void) {
  // it could be that we are getting called
  // by update when we are in the midst of changing our
  // resolution
  if (myBackBuffer) {
    if (SDL_LockSurface(myBackBuffer) == -1) {
      ASSERT(false);
      return NULL;
    }

    myCurrentOffScreenBufferPtr = (uint16 *)myBackBuffer->pixels;
    myPitch = myBackBuffer->pitch >> 1;
    myPitchForBackgroundTiles = (myPitch - 128) * 2;
    return myCurrentOffScreenBufferPtr;
  } else {
    return NULL;
  }
}

// ----------------------------------------------------------------------
// Method:      DrawToFrontBuffer
// Arguments:   None
// Returns:     None
//
// Description: Calls SDL_UpdateRect() to display the virtual
// surface (myBackBuffer)
//
// ----------------------------------------------------------------------
void DisplayEngine::DrawToFrontBuffer() {
  if (!myEngineRunningFlag)
    return;

  POINT point = {0, 0};

  // TODO: maybe use an extra surface and hardware flipping in
  // fullscreen mode?

  // The pretty flashing rectangle in the corner
  // which shows you that SDL is working when
  // things look bad
  // SDL_Rect r;
  // r.x = r.y = 0;
  // r.w = r.h = 10;
  // SDL_FillRect(myBackBuffer, &r, rand());

  // BEGIN ROUND WORLD EASTER EGG
  if (myIsTheWorldRoundFlag || myDesiredWorldRoundness) {
    if (OpenBackBuffer()) {
      EasterEgg::CunningBlit(GetBackBufferPtr(), GetBackBufferPtr(),
                             mySurfaceArea.right, mySurfaceArea.bottom, myPitch,
                             myPitch, myIsTheWorldRoundFlag,
                             myDesiredWorldRoundness);

      CloseBackBuffer();
    }
  }
  // END ROUND WORLD EASTER EGG

  // TODO: can we get at the dirty-rectangle list to only update
  // areas which have changed?
  SDL_UpdateRect(myBackBuffer, 0, 0, 0, 0);
}

void DisplayEngine::FadeScreen() {}

void DisplayEngine::FlipScreenVertically() {}

void DisplayEngine::Shrink(int32 x, int32 y) {}

bool DisplayEngine::FlipScreenHorizontally() { return false; }

bool DisplayEngine::SlideScreen() { return false; }

void DisplayEngine::Burst() {}

// ----------------------------------------------------------------------
// Method:      DrawSprite
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to
// draw
// Returns:     None
//
// Description: Draw the sprite to the back buffer
//
//
// ----------------------------------------------------------------------
void DisplayEngine::DrawSprite(Position &position, Bitmap &bitmap) {
  // work out how much to increase the data and sreen pointers
  // on when drawing

  if (!GetDrawingParameters(position, &bitmap, data_step, data_ptr, screen_step,
                            screen_ptr, bitmapWidth, bitmapHeight))
    return;

    // draw taking account of transparent pixels

#ifdef C2E_NO_INLINE_ASM
  uint16 pixel = 0;
  for (; bitmapHeight--;) {
    for (int32 k = bitmapWidth; k--;) {
      pixel = *data_ptr++;
      if (pixel)
        *screen_ptr = pixel;
      screen_ptr++;
    }
    data_ptr += data_step;
    screen_ptr += screen_step;
  }
#else

  if (bitmapWidth < 1 || bitmapHeight < 1)
    return;

#ifdef C2E_NASM_RENDERING
  { NasmDrawSprite(); }
#else
  _asm
  {
		mov esi,dword ptr [data_ptr]
		mov edi,dword ptr [screen_ptr]
		mov ebx,dword ptr [data_step]
		mov edx,dword ptr [screen_step]
		
		push dword ptr [bitmapHeight]		;Stack up bitmapHeight for later 

	topOfOuterLoop:
		mov ecx,dword ptr [bitmapWidth]		;Get the number of pixels we are playing with
	topOfInnerLoop:
		;Load pixel into AX
		lodsw
		test ax,ax
		je dontstore
		mov word ptr [edi],ax				;Store screen pixel
	dontstore:
		add edi,2							;Increment screen pointer one word

		;Inner loop epilogue
		dec ecx
		jne topOfInnerLoop					;If not done with line, jump to top of inner loop

		; Deal with the widths :)


		;Outer loop epilogue
		pop ecx								;Destack bitmapHeight

		lea esi,[esi+ebx*2]					;data_ptr += data_step
		
		dec ecx
		
		lea edi,[edi+edx*2]					;screen_ptr += screen_step
		
		push ecx							;Restack bitmapHeight
		
		jne topOfOuterLoop

		;asm epilogue

		pop ecx								;clean stack pre popa
      }
#endif // C2E_NASM_RENDERING
#endif
}

// ----------------------------------------------------------------------
// Method:      DrawMirroredSprite
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to
// draw
// Returns:     None
//
// Description: Miror the sprite to the back buffer.
//
// ----------------------------------------------------------------------
void DisplayEngine::DrawMirroredSprite(Position &position, Bitmap &bitmap) {
  int32 bitmapWidth;
  int32 bitmapHeight;

  // work out how much to increase the data and sreen pointers
  // on when drawing
  uint32 data_step;

  uint16 *data_ptr;

  uint32 screen_step;
  uint16 *screen_ptr;

  if (!GetDrawingParameters(position, &bitmap, data_step, data_ptr, screen_step,
                            screen_ptr, bitmapWidth, bitmapHeight))
    return;

  // Draw each data line backwards

  // create a temporary pointer to the start of the first line of data

  uint16 *dataLineStart = data_ptr;

  // move the dataptr to the end of the line
  data_ptr += bitmapWidth;

  // draw taking account of transparent pixels
  uint16 pixel = 0;
  for (; bitmapHeight--;) {
    for (int32 k = bitmapWidth; k--;) {
      // move along the line
      dataLineStart++;
      // draw backwards
      pixel = *data_ptr--;
      if (pixel)
        *screen_ptr = pixel;
      screen_ptr++;
    }
    // get to the start of the next line
    dataLineStart += data_step;
    // move the dataa pointer to the end of the line
    data_ptr = dataLineStart + bitmapWidth;
    screen_ptr += screen_step;
  }
}

// ----------------------------------------------------------------------
// Method:      GetCompressedDrawingParameters
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to
// draw 				data_step - place to put the amount of
// bitmap data we need 							 to skip
// when drawing 				data_ptr - workout where to
// start drawing from in the bitmap
// data 				screen_step
//- place to put the amount of screen pixels
// we need 							 to skip when
// drawing screen_ptr - workout where on screen to start drawing bitmapWidth -
// bitmapHeigth - workout exactly how much of the bitmap we need
// to draw due to lipping right - whether we'll need to clip right
// wether - whether we'll need to clip bottom Returns:     None
//
// Description: Get all the information we need for drawing
//
//
// ----------------------------------------------------------------------
bool DisplayEngine::GetCompressedDrawingParameters(
    Position &position, CompressedBitmap *bitmap, uint32 &data_step,
    uint8 *&compressedData_ptr, uint32 &screen_step, uint16 *&screen_ptr,
    int32 &bitmapWidth, int32 &bitmapHeight, bool &rightClip, bool &bottomClip,
    bool &topClip, bool &leftClip) {

  int32 x = position.GetX();
  int32 y = position.GetY();

  bitmapWidth = bitmap->GetWidth();
  bitmapHeight = bitmap->GetHeight();

  // work out how much to increase the data and sreen pointers
  // on when drawing
  data_step = bitmap->GetWidth();

  compressedData_ptr = (uint8 *)bitmap->GetData();
  ASSERT(compressedData_ptr);

  //	screen_step=ourSurfaceArea.right;

  screen_ptr = GetBackBufferPtr();
  screen_step = myPitch;

  ASSERT(screen_ptr);

  rightClip = false;
  bottomClip = false;
  topClip = false;

  // determine whether we have to clip the
  // sprite
  if (x < 0) {
    bitmapWidth += x;
    if (bitmapWidth < 0)
      return false;
    x = 0;
    leftClip = true;
  }

  if (y < 0) {
    bitmapHeight += y;
    if (bitmapHeight < 0)
      return false;
    topClip = true;
    compressedData_ptr = bitmap->GetScanLine(0 - y);
    y = 0;
  }

  //	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
  int32 t = (x + bitmapWidth) - myPitch;

  // if the bitmap needs clipping to the right
  if (t >= 0) {
    bitmapWidth -= t;
    if (bitmapWidth <= 0)
      return false;
    rightClip = true;
  }

  t = (y + bitmapHeight) - mySurfaceArea.bottom + 1;

  // if the bitmap needs clipping at the bottom
  if (t >= 0) {
    bitmapHeight -= t;
    if (bitmapHeight <= 0)
      return false;
    bottomClip = true;
  }

  data_step -= bitmapWidth;
  screen_step -= bitmapWidth;

  screen_ptr += (y * myPitch) + x;

  return true;
}

// ----------------------------------------------------------------------
// Method:      GetDrawingParameters
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to
// draw 				data_step - place to put the amount of
// bitmap data we need 							 to skip
// when drawing 				data_ptr - workout where to
// start drawing from in the bitmap
// data 				screen_step
//- place to put the amount of screen pixels
// we need 							 to skip when
// drawing screen_ptr - workout where on screen to start drawing bitmapWidth -
// bitmapHeigth - workout exactly how much of the bitmap we need
// to draw due to lipping Returns:     true if the bitmap is on screen and can
// be drawn
//              false otherwise
//
// Description: Get all the information we need for drawing
//
//
// ----------------------------------------------------------------------
bool DisplayEngine::GetDrawingParameters(Position &position, Bitmap *bitmap,
                                         uint32 &data_step, uint16 *&data_ptr,
                                         uint32 &screen_step,
                                         uint16 *&screen_ptr,
                                         int32 &bitmapWidth,
                                         int32 &bitmapHeight) {
  int32 x = position.GetX();
  int32 y = position.GetY();

  bitmapWidth = bitmap->GetWidth();
  bitmapHeight = bitmap->GetHeight();

  // work out how much to increase the data and screen pointers
  // on when drawing
  data_step = bitmap->GetWidth();

  data_ptr = bitmap->GetData();
  ASSERT(data_ptr);

  screen_ptr = GetBackBufferPtr();
  ASSERT(screen_ptr);
  if (!screen_ptr)
    return false;

  screen_step = myPitch;

  // determine whether we have to clip the
  // sprite
  if (x < 0) {
    bitmapWidth += x;
    if (bitmapWidth < 0)
      return false;

    // only modify the dataptr (not the compressed one)
    data_ptr -= x;
    x = 0;
  }
  if (y < 0) {
    bitmapHeight += y;
    if (bitmapHeight < 0)
      return false;

    // only modify the dataptr (not the compressed one)
    data_ptr -= (y * bitmap->GetWidth());
    y = 0;
  }

  //	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;

  int32 t = (x + bitmapWidth) - myPitch;

  // if the bitmap needs clipping to the right
  if (t >= 0) {
    bitmapWidth -= t;
    if (bitmapWidth <= 0)
      return false;
  }

  t = (y + bitmapHeight) - mySurfaceArea.bottom;

  // if the bitmap needs clipping at the bottom
  if (t >= 0) {
    bitmapHeight -= t;
    if (bitmapHeight <= 0)
      return false;
  }

  data_step -= bitmapWidth;
  screen_step -= bitmapWidth;

  screen_ptr += (y * myPitch) + x;

  // if the background is smaller than the surface then try to centre it

  /*	if(myFullScreenFlag)
          {
                  if(ourDisplayArea.bottom < ourSurfaceArea.bottom)
                  {
                          screen_ptr += myPitch *
                                  ((ourSurfaceArea.bottom-ourDisplayArea.bottom)/2);
                  }
          }*/

  return true;
}

// ----------------------------------------------------------------------
// Method:      GetCompressedDrawingParameters
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to
// draw 				data_step - place to put the amount of
// bitmap data we need 							 to skip
// when drawing 				data_ptr - workout where to
// start drawing from in the bitmap
// data 				screen_step
//- place to put the amount of screen pixels
// we need 							 to skip when
// drawing screen_ptr - workout where on screen to start drawing bitmapWidth -
// bitmapHeigth - workout exactly how much of the bitmap we need
// to draw due to lipping right - whether we'll need to clip right
// wether - whether we'll need to clip bottom Returns:     None
//
// Description: Get all the information we need for drawing
//
//
// ----------------------------------------------------------------------
bool DisplayEngine::GetCompressedDrawingParameters16Bit(
    Position &position, Bitmap *bitmap, uint32 &data_step,
    uint16 *&compressedData_ptr, uint32 &screen_step, uint16 *&screen_ptr,
    int32 &bitmapWidth, int32 &bitmapHeight, bool &rightClip, bool &bottomClip,
    bool &topClip) {
  int32 x = position.GetX();
  int32 y = position.GetY();

  bitmapWidth = bitmap->GetWidth();
  bitmapHeight = bitmap->GetHeight();

  // work out how much to increase the data and sreen pointers
  // on when drawing
  data_step = bitmap->GetWidth();

  compressedData_ptr = bitmap->GetData();
  ASSERT(compressedData_ptr);

  screen_ptr = GetBackBufferPtr();
  ASSERT(screen_ptr);

  screen_step = myPitch;

  rightClip = false;
  bottomClip = false;
  topClip = false;

  // determine whether we have to clip the
  // sprite
  if (x < 0) {
    bitmapWidth += x;
    if (bitmapWidth < 0)
      return false;
    x = 0;
  }
  if (y < 0) {
    topClip = true;
    bitmapHeight += y;
    if (bitmapHeight < 0)
      return false;
    y = 0;
  }
  //	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
  int32 t = (x + bitmapWidth) - myPitch;

  // if the bitmap needs clipping to the right
  if (t >= 0) {
    bitmapWidth -= t;
    if (bitmapWidth <= 0)
      return false;
    rightClip = true;
  }

  t = (y + bitmapHeight) - mySurfaceArea.bottom + 1;

  // if the bitmap needs clipping at the bottom
  if (t >= 0) {
    bitmapHeight -= t;
    if (bitmapHeight <= 0)
      return false;
    bottomClip = true;
  }

  data_step -= bitmapWidth;
  screen_step -= bitmapWidth;

  screen_ptr += (y * myPitch) + x;
  return true;
}

// ----------------------------------------------------------------------
// Method:      DrawSpriteToBitmap
// Arguments:   destination - bitmap to draw to
//				pos - point on bitmap to start drawing
//				source - sprite data to draw
//
// Returns:     none
//
// Description: Draws the given sprite to the given bitmap
//
// ----------------------------------------------------------------------
void DisplayEngine::DrawSpriteToBitmap(Bitmap *destination, Position position,
                                       Bitmap *const source,
                                       uint16 textColour /*=0*/,
                                       uint16 backgroundColour /* = 0*/) {

  //	if the sprite is too big then flag it
  if (destination->GetWidth() < source->GetWidth())
    return; // change to return bool then!!!

  uint16 *destPtr = destination->GetData();
  uint32 sourceHeight = source->GetHeight();
  uint32 sourceWidth = source->GetWidth();
  uint16 *sourcePtr = source->GetData();

  ASSERT(destPtr);
  ASSERT(sourcePtr);

  // step is the difference to jump between lines
  uint32 dest_step = destination->GetWidth() - source->GetWidth();

  int32 x = position.GetX();
  int32 y = position.GetY();

  // make sure that we are not trying to overwrite our
  // sprite
  ASSERT(y <= (destination->GetHeight() - sourceHeight) && y >= 0);

  // find out where we should start drawing
  destPtr += (y * destination->GetWidth()) + x;

  // if we don't have to fart around with changing
  // text and background colour
  if (textColour == 0 && backgroundColour == 0) {
    while (sourceHeight--) {
      for (uint32 width = 0; width < sourceWidth; width++) {
        *destPtr++ = *sourcePtr++;
      }
      destPtr += dest_step;
    }
  } else {
    while (sourceHeight--) {
      for (uint32 width = 0; width < sourceWidth; width++) {
        if (*sourcePtr++ == 0)
          *destPtr++ = textColour;
        else
          *destPtr++ = backgroundColour;
      }
      destPtr += dest_step;
    }
  }
}

// ----------------------------------------------------------------------
// Method:      DrawLine
// Arguments:   bitmap - bitmap to write to
//				x and y coordinates of start point and end point
// of
// the 				line 				r,g,b colour
// values for the line
//
// Returns:     none
//
// Description: Draws the line to the given bitmap.
//
//
// ----------------------------------------------------------------------
void DisplayEngine::DrawLineToBitmap(Bitmap *bitmap, int32 x1, int32 y1,
                                     int32 x2, int32 y2,
                                     uint8 lineColourRed /*= 0*/,
                                     uint8 lineColourGreen /*= 0*/,
                                     uint8 lineColourBlue /*= 0*/) {
  // make sure that the line is within the bitmap's bounds
  //
  if (x1 < 0)
    x1 = 0;
  if (x2 < 0)
    x2 = 0;
  if (y2 < 0)
    y2 = 0;

  if (y1 < 0)
    y1 = 0;
  if (x1 >= bitmap->GetWidth())
    x1 = bitmap->GetWidth() - 1;
  if (x2 >= bitmap->GetWidth())
    x2 = bitmap->GetWidth() - 1;
  if (y1 >= bitmap->GetHeight())
    y1 = bitmap->GetHeight() - 1;
  if (y2 >= bitmap->GetHeight())
    y2 = bitmap->GetHeight() - 1;

  uint16 lineColour = 0;
  if (lineColourRed == 0 && lineColourGreen == 0 && lineColourBlue == 0) {
    if (myPixelFormat == RGB_565) {
      RGB_TO_565(192, 255, 255, lineColour);
    } else {
      RGB_TO_565(192, 255, 255, lineColour);
    }

  } else {
    if (myPixelFormat == RGB_565) {
      RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
    } else {
      RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
    }
  }

  uint16 *pixels = bitmap->GetData();
  uint16 *currentPixel = 0;

  int32 xi, yi, dx, dy, i, j;
  if (x1 > x2)
    xi = -1;
  else
    xi = 1;
  if (y1 > y2)
    yi = -1;
  else
    yi = 1;

  dx = abs(x2 - x1);
  dy = abs(y2 - y1);

  if (dx == 0 && dy == 0)
    return;

  currentPixel = pixels + (y1 * bitmap->GetWidth()) + x1;
  *currentPixel = lineColour;
  if (dy > dx) {
    j = dy;
    i = dy >> 1;
    do {
      y1 += yi;
      if ((i += dx) > dy) {
        i -= dy;
        x1 += xi;
      }
      currentPixel = pixels + (y1 * bitmap->GetWidth()) + x1;
      *currentPixel = lineColour;
    } while (--j);
  } else {
    j = dx;
    i = dx >> 1;
    do {
      x1 += xi;
      if ((i += dy) > dx) {
        i -= dx;
        y1 += yi;
      }
      currentPixel = pixels + (y1 * bitmap->GetWidth()) + x1;
      *currentPixel = lineColour;
    } while (--j);
  }
}

// jumps to the next fullscreen mode
void DisplayEngine::NextScreenMode() {
  if (myFullScreenFlag) {
    if (myCurrentFullScreenMode + 1 < myFullScreenModes.size())
      SetMode(myCurrentFullScreenMode + 1);
    else
      SetMode(0); // wrap (will drop back to windowed mode
                  // if necessary
  } else
    SetMode(myCurrentFullScreenMode); // go into fullscreen mode
}

// Returns:     true if changed OK
//				false otherwise
bool DisplayEngine::ToggleFullScreenMode() {
  if (myFullScreenFlag)
    return SetModeWindowed(myWindowedModeW, myWindowedModeH);
  else
    return SetMode(myCurrentFullScreenMode);
}

void DisplayEngine::PrepareForMessageBox() {
  if (!myEngineRunningFlag)
    return;

  // change to windowed mode if necessary
  if (myFullScreenFlag) {
    SetModeWindowed(ourSurfaceArea.right, ourSurfaceArea.bottom);

    // Win32 version redrew display here - I don't
    // see why we need to particularly.  Just wait a
    // tick.
  }
}

void DisplayEngine::EndMessageBox() {
  if (!myEngineRunningFlag)
    return;
  // change back to fullscreen mode if necessary
}

// ----------------------------------------------------------------------
// Method:      ClientToScreen
// Arguments:   x,y - co-ordinates to convert
//
// Returns:     None
//
// Description: Converts the window co-ordinates to screen co-ordinates
//
// ----------------------------------------------------------------------
void DisplayEngine::ClientToScreen(int32 &x, int32 &y) {
  //	POINT point={x,y};
  //	::ClientToScreen(myWindow,&point);
  //	x = point.x;
  //	y = point.y;
}

void DisplayEngine::ScreenToClient(int32 &x, int32 &y) {
  //	POINT point={x,y};
  //	::ScreenToClient(myWindow,&point);
  //	x = point.x;
  //	y = point.y;
}

SDL_Rect DisplayEngine::ConvertRect(RECT &rect) {
  SDL_Rect sdlRect;
  sdlRect.x = rect.left;
  sdlRect.y = rect.top;
  sdlRect.w = rect.right - rect.left;
  sdlRect.h = rect.bottom - rect.top;
  return sdlRect;
}

// This is used by App::StartProgressBar() to draw the progressbar background
bool DisplayEngine::RenderBitmapToFrontBuffer(Bitmap *bitmap) {
  ASSERT(bitmap != 0);

  // a temp SDL surface
  // TODO: allow for other pixelformats?

  int bitdepth = 16;

  SDL_Surface *surface;
  if (myPixelFormat == RGB_555) {
    surface = SDL_CreateRGBSurface(SDL_HWSURFACE, bitmap->GetWidth(),
                                   bitmap->GetHeight(), bitdepth, 0x7C00, 0x03E0,
                                   0x001F, 0);
  } else // 565
  {
    surface = SDL_CreateRGBSurface(SDL_HWSURFACE, bitmap->GetWidth(),
                                   bitmap->GetHeight(), bitdepth, 0xF800, 0x07E0,
                                   0x001F, 0);
  }

  if (!surface)
    return false;

  SDL_SetColorKey(surface, SDL_SRCCOLORKEY, 0);

  const uint16 *sourcePtr = bitmap->GetData();
  uint16 *destPtr = (uint16 *)surface->pixels;

  int32 bitmapWidth = bitmap->GetWidth();
  int32 bitmapHeight = bitmap->GetHeight();

  // the surface is created to be the same
  // size as the entity bounds
  int32 destStep = (surface->pitch >> 1);
  int32 sourceStep = 0;
  destStep = destStep - bitmapWidth;
  for (; bitmapHeight--;) {
    for (int32 width = bitmapWidth; width--;)
      *destPtr++ = *sourcePtr++;

    destPtr += destStep;
  }

  // render it
  RECT clip;

  clip.left = 0;
  clip.top = 0;
  clip.right = bitmap->GetWidth();
  clip.bottom = bitmap->GetHeight();

  RECT source;
  source.top = bitmap->GetPosition().GetY();
  source.left = bitmap->GetPosition().GetX();
  source.right = source.left + clip.right;
  source.bottom = source.top + clip.bottom;

  BlitToFrontBuffer(source, surface, clip, true);

  SDL_FreeSurface(surface);
  return true;
}

// This routine is kind of misnamed nowdays...
bool DisplayEngine::BlitToFrontBuffer(RECT &destination, SDL_Surface *image,
                                      RECT &source, bool transparencyAware) {
  SDL_Rect sourceSDLRect = ConvertRect(source);
  SDL_Rect destSDLRect = ConvertRect(destination);
  int res = SDL_BlitSurface(image, &sourceSDLRect, myBackBuffer, &destSDLRect);

  ASSERT(res == 0);

  SDL_UpdateRect(myBackBuffer, destSDLRect.x, destSDLRect.y, destSDLRect.w,
                 destSDLRect.h);

  return res == 0;
}

bool DisplayEngine::BlitToBackBuffer(RECT &destination, SDL_Surface *image,
                                     RECT &source, bool transparencyAware) {
  SDL_Rect sourceSDLRect = ConvertRect(source);
  SDL_Rect destSDLRect = ConvertRect(destination);
#ifdef NOT_STRETCHING_AT_ALL
  // A non stretched version for if we don't have SDL_StretchSurface.
  // Since we do now, this may not be any use :-)
  sourceSDLRect.w = destSDLRect.w;
  sourceSDLRect.h = destSDLRect.h;

  int res = SDL_BlitSurface(image, &sourceSDLRect, myBackBuffer, &destSDLRect);
  ASSERT(res == 0);
  return res == 0;
#else
  if (source.bottom <= source.top || source.top >= source.bottom ||
      source.right <= source.left || source.left >= source.right ||
      destination.bottom <= destination.top ||
      destination.top >= destination.bottom ||
      destination.right <= destination.left ||
      destination.left >= destination.right) {
    // std::cout << "Camera clipped..." << std::endl;
    return false;
  } else {
    ASSERT(sourceSDLRect.x + sourceSDLRect.w <= image->w);
    ASSERT(sourceSDLRect.x >= 0);
    ASSERT(sourceSDLRect.y + sourceSDLRect.h <= image->h);
    ASSERT(sourceSDLRect.y >= 0);

    SDL_StretchSurface(image, &sourceSDLRect, myBackBuffer, &destSDLRect);
    return true;
  }
#endif
}

// for Rob
// it is possible to draw a stippled or dotted line by specifying how many
// coloured pixels to draw followed by how many blank pixels.To make a dotted
// line you specify how many pixels to draw (stippleon) and how many should be
// blank (stipple off) so stippleon =4, stippleoff = 3 should give you:
// ---- xxx----xxx ----   (where - is coloured pixel an x is transparent pixel).

// I call one set of stippleons plus one set off stippleoffs a stipplesection.
//   So this is one stipple section:
//  ----xxx

// You can make marching ants by specifying (using the new parameter) where in
// the section you would like to start drawing.
// Therefore the value is from 0 to stippleon + stippleoff.   So if you start
// drawing at 2 you'll get
// --xxx----xxx----xxx

// if you start drawing at 5 you'll get
// xx----xxx----

void DisplayEngine::DrawLine(
    int32 x1, int32 y1, int32 x2, int32 y2, uint8 lineColourRed /*= 0*/,
    uint8 lineColourGreen /*= 0*/, uint8 lineColourBlue /*= 0*/,
    uint8 stippleon /* =0*/,   // how many coloured pixels
    uint8 stippleoff /* = 0*/, // how many transparent
    uint32 stipplestartAt /*= 0*/) {
  if (!ClipLine(&mySurfaceArea, (int &)x1, (int &)y1, (int &)x2, (int &)y2, 0))
    return;

  // Bomb out if undrawable
  if ((x1 == x2) && (y1 == y2))
    return;

  uint32 stippleoncount = 0;
  uint32 stippleoffcount = 0;

  int stipplevalue = 0;

  if ((stippleon + stippleoff) != 0)
    stipplevalue = stipplestartAt % (stippleon + stippleoff);

  if (stipplevalue > stippleon - 1) {
    stipplevalue -= stippleon;
    stippleoffcount = stipplevalue;
    stippleoncount = stippleon;
  } else {
    stippleoncount = stipplevalue;
  }

  uint16 lineColour = 0;
  if (lineColourRed == 0 && lineColourGreen == 0 && lineColourBlue == 0) {
    if (myPixelFormat == RGB_565) {
      RGB_TO_565(192, 255, 255, lineColour);
    } else {
      RGB_TO_565(192, 255, 255, lineColour);
    }

  } else {
    if (myPixelFormat == RGB_565) {
      RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
    } else {
      RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
    }
  }

  uint16 *screen_ptr = GetBackBufferPtr();
  ASSERT(screen_ptr);

  uint32 screen_step = myPitch;

  uint16 *currentPixel = 0;

  int32 xi, yi, dx, dy, i, j;
  if (x1 > x2)
    xi = -1;
  else
    xi = 1;
  if (y1 > y2)
    yi = -1;
  else
    yi = 1;

  dx = abs(x2 - x1);
  dy = abs(y2 - y1);

  currentPixel = screen_ptr + (y1 * myPitch) + x1;
  if (stippleon == 0 && stippleoff == 0) {
    *currentPixel = lineColour;
  } else if (stippleoncount < stippleon) {
    stippleoncount++;
    stippleoffcount = 0;
    *currentPixel = lineColour;
  } else {
    if (stippleoffcount < stippleoff) {
      *currentPixel = 0;
      stippleoffcount++;
    } else {
      stippleoffcount = 0;
      stippleoncount = 0;
    }
  }

  if (dy > dx) {
    j = dy;
    i = dy >> 1;
    do {
      y1 += yi;
      if ((i += dx) > dy) {
        i -= dy;
        x1 += xi;
      }
      currentPixel = screen_ptr + (y1 * myPitch) + x1;

      if (stippleon == 0 && stippleoff == 0) {
        *currentPixel = lineColour;
      } else if (stippleoncount < stippleon) {
        stippleoncount++;
        stippleoffcount = 0;
        *currentPixel = lineColour;
      } else {
        if (stippleoffcount < stippleoff) {
          *currentPixel = 0;
          stippleoffcount++;
        } else {
          stippleoffcount = 0;
          stippleoncount = 0;
        }
      }
    } while (--j);
  } else {
    j = dx;
    i = dx >> 1;
    do {
      x1 += xi;
      if ((i += dy) > dx) {
        i -= dx;
        y1 += yi;
      }
      currentPixel = screen_ptr + (y1 * myPitch) + x1;

      if (stippleon == 0 && stippleoff == 0) {
        *currentPixel = lineColour;
      }
      if (stippleoncount < stippleon) {
        stippleoncount++;
        stippleoffcount = 0;
        *currentPixel = lineColour;
      } else {
        if (stippleoffcount < stippleoff) {
          *currentPixel = 0;
          stippleoffcount++;
        } else {
          stippleoffcount = 0;
          stippleoncount = 0;
        }
      }

    } while (--j);
  }
}

void DisplayEngine::DrawCompressedSprite(Position &position,
                                         CompressedBitmap *bitmap) {
  //	OutputDebugString("get drawing parameters\n");
  // assume that we will do left or top clip
  bool rightClip = false;
  bool bottomClip = false;
  bool topClip = false;
  bool leftClip = false;
  int32 bytes = 0;

  int32 x = position.GetX();
  int32 y = position.GetY();

  int32 bitmapWidth = bitmap->GetWidth();
  bitmapHeight = bitmap->GetHeight();

  // work out how much to increase the data and sreen pointers
  // on when drawing
  uint32 data_step = bitmapWidth;
  // when both left and right clipped
  // top and bottom clipped
  uint32 rightClipped = 0;
  uint32 leftClipped = bitmapWidth;
  uint32 topClipped = bitmapHeight;

  compressedData_ptr = bitmap->GetData();
  ASSERT(compressedData_ptr);

  screen_step = myPitch;
  //	OutputDebugString("getting back buffer\n");
  screen_ptr = GetBackBufferPtr();
  ASSERT(screen_ptr);

  //	OutputDebugString("got back buffer\n");

  // determine whether we have to clip the
  // sprite
  if (x < 0) {
    bitmapWidth += x;
    if (bitmapWidth < 0)
      return;
    x = 0;
    leftClip = true;
    // this is the position in the bitmap to
    // start drawing at
    leftClipped -= bitmapWidth;
  } else {
    leftClipped = 0;
  }

  if (y < 0) {
    bitmapHeight += y;
    if (bitmapHeight < 0)
      return;
    topClip = true;
    //!!!!!!!!
    compressedData_ptr = (uint16 *)bitmap->GetScanLine(0 - y);
    y = 0;
    topClipped -= bitmapHeight;

  } else {
    topClipped = 0;
  }

  int32 t = (x + bitmapWidth) - myPitch;

  // if the bitmap needs clipping to the right
  if (t >= 0) {
    bitmapWidth -= t;
    if (bitmapWidth <= 0)
      return;
    rightClip = true;
    rightClipped = t;
  }

  t = (y + bitmapHeight) - mySurfaceArea.bottom;

  // if the bitmap needs clipping at the bottom
  if (t >= 0) {
    bitmapHeight -= t;
    if (bitmapHeight <= 0)
      return;
    bottomClip = true;
  }

  data_step -= bitmapWidth;
  screen_step -= bitmapWidth;

  screen_ptr += (y * myPitch) + x;

  uint16 tag = 0;
  int32 thisStep = 0;
  uint16 count = 0;
  uint32 pixelsDrawnThisLine = 0;

  //	OutputDebugString("start actual drawing\n");
  // if no clipping is required
  if (data_step == 0) {
    if (bitmapHeight == 0 || bitmapWidth == 0)
      return;

    ushort *puVar1;
    ushort uVar2;
    int iVar3;
    uint uVar4;
    int iVar5;
    ushort *puVar6;
    ushort *puVar7;
    
    iVar3 = screen_step;
    iVar5 = bitmapHeight;
    puVar6 = compressedData_ptr;
    puVar7 = screen_ptr;
    do {
      while( true ) {
        puVar1 = puVar6 + 1;
        uVar2 = *puVar6;
        puVar6 = puVar1;
        if (uVar2 == 0) break;
        if ((uVar2 & 1) == 0) {
          puVar7 = (ushort *)((int)puVar7 + (uint)uVar2);
        }
        else {
          uVar4 = (uint)(uVar2 >> 1);
          while (uVar4 != 0) {
            uVar4 = uVar4 - 1;
            *puVar7 = *puVar6;
            puVar6 = puVar6 + 1;
            puVar7 = puVar7 + 1;
          }
        }
      }
      puVar7 = puVar7 + iVar3;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);

    return;
    //		OutputDebugString("end no clip \n");*/
  }    // end if datastep ==0
  else // some clipping is required
  {
    if ((rightClip && !leftClip) || (bottomClip && !leftClip)) {
      //	OutputDebugString("start r b not left\n");

      uint32 i = 0;
      int32 drawHeight = bitmapHeight;
      if (topClip) {
        i = topClipped;
        drawHeight = bitmapHeight + topClipped;
      }
      // for each line
      for (i; i < drawHeight; i++) {
        thisStep = bitmapWidth;
        // find out what type of pixel we have
        tag = *compressedData_ptr++;
        while (tag) {
          // find the number of pixels to plot
          count = tag >> 1;

          // we have a run of colours
          if (tag & 0x01) {
            // if we are not at our stopping point yet
            if (count <= thisStep) {
              thisStep -= count;
              // we need to keep drawing unless
              // this is the end of the
              // line in which case there will be a tag
              // anyway
              bytes = count << 1;
              memcpy(screen_ptr, compressedData_ptr, bytes);
              compressedData_ptr += count;
              screen_ptr += count;
            } else // pixel count has over run
            {
              //	uint32 over = count - thisStep;
              // then draw the remaining amount of pixels
              // and set the step to zero
              // draw all the colours
              // draw what you will
              bytes = thisStep << 1;
              memcpy(screen_ptr, compressedData_ptr, bytes);
              compressedData_ptr += thisStep;
              screen_ptr += thisStep;

              // don't worry about moving past the overrun
              // in the data file
              // since thisStep = 0 will take care of it
              thisStep = 0;
            }
          } // end if colour
          else {
            // calculate which pixel in this line
            // to start from
            //	thisStep += count;
            // if we are not at our starting point yet
            if (count <= thisStep) {
              // skip past the transparent pixels on the screen
              // part of the bitmap
              thisStep -= count;
              screen_ptr += count;
              tag = *compressedData_ptr++;
              continue;
            } else // pixel count has over run
            {
              screen_ptr += thisStep;
              // don't worry about moving past the overrun
              // in the data
              // since thisStep = 0 will take care of it
              thisStep = 0;
            }
          } // end else black

          if (thisStep == 0) {
            // no more to draw
            // move the data ptr on to the end of this line
            //
            if (i + 1 < drawHeight) {
              //!!!!!!!!!
              compressedData_ptr = (uint16 *)bitmap->GetScanLine(i + 1);

              if (!compressedData_ptr)
                return;

              compressedData_ptr--;
            } else {
              // force the tag to zero so that everything stops
              tag = 0;
              continue;
            }

          } // end this step over

          tag = *compressedData_ptr++;
        } // end while tag
        screen_ptr += screen_step;
      } // bitmap height
      //	OutputDebugString("end r b not left\n");
    } // end if right/bottom
    else {
      //	OutputDebugString("start left\n");
      // left clip
      uint32 nextline = topClipped;
      uint32 leftSkip = 0;
      for (; bitmapHeight--;) {
        nextline++;
        thisStep = 0;
        pixelsDrawnThisLine = 0;
        leftSkip = 0;

        // find out what type of pixel we have
        tag = *compressedData_ptr++;
        while (tag) {
          // find the number of colours to plot
          count = tag >> 1;
          // calculate which pixel in this line
          // to start from
          thisStep += count;

          if (tag & 0x01) // colour pixel
          {
            // if we are not at our starting point yet
            if (thisStep <= leftClipped) {
              // move the data on to the next tag
              compressedData_ptr += count;
              leftSkip += count;
            } else {

              // find out how many pixels we are over and
              // draw them to the screen
              uint32 over = thisStep - leftClipped;

              // draw the amount we are over

              thisStep -= over;
              if (leftSkip < leftClipped) {
                compressedData_ptr += leftClipped - leftSkip;
                leftSkip = leftClipped;
              }

              // if we are still supposed to be drawing pixels
              if (over + pixelsDrawnThisLine <= bitmapWidth) {
                bytes = over << 1;
                memcpy(screen_ptr, compressedData_ptr, bytes);
                compressedData_ptr += over;
                screen_ptr += over;
                pixelsDrawnThisLine += over;
              } else {
                // too many pixels in this run
                // any portion to draw?
                uint32 therest = bitmapWidth - pixelsDrawnThisLine;
                if (therest) {
                  bytes = therest << 1;
                  memcpy(screen_ptr, compressedData_ptr, bytes);
                  compressedData_ptr += therest;
                  screen_ptr += therest;
                  pixelsDrawnThisLine += therest;

                  // now skip to the end of the bitmap data
                  compressedData_ptr = (uint16 *)bitmap->GetScanLine(nextline);
                  if (!compressedData_ptr)
                    return;

                  compressedData_ptr--;
                }
              }
            }
          }      // end if colour
          else { // transparent
            // calculate which pixel in this line
            // to start from
            //	thisStep += count;
            // if we are not at our starting point yet
            if (thisStep <= leftClipped) {
              // do nothing for we don't need to draw this
              // part of the bitmap
              leftSkip += count;
              //	continue;
            } else // thisStep is larger than data_step
            {
              // find out how many pixels we are over and
              // draw them to the screen
              uint32 over = thisStep - leftClipped;

              // draw the amount we are over

              thisStep -= over;

              // find out how many pixels we are over and
              // draw them to the screen
              // if we are still supposed to be drawing pixels
              if (leftSkip < leftClipped) {
                leftSkip = leftClipped;
              }

              if (over + pixelsDrawnThisLine <= bitmapWidth) {
                // move the screen ptr on by the required amount of steps
                // because these are transparent
                screen_ptr += over;
                // pretend that we didn't go over so that
                //  we can draw to the end of the line

                pixelsDrawnThisLine += over;
              } else {
                // too many pixels in this run
                // any portion to draw?
                uint32 therest = bitmapWidth - pixelsDrawnThisLine;
                if (therest) {
                  screen_ptr += therest;

                  pixelsDrawnThisLine += therest;
                  // finally skip past the right clipped area
                  compressedData_ptr = (uint16 *)bitmap->GetScanLine(nextline);
                  if (!compressedData_ptr)
                    return;

                  compressedData_ptr--;
                }
              }
            }

          } // end colour check
          tag = *compressedData_ptr++;
        } // end while tag
        screen_ptr += screen_step;
      } // bitmap height
        //	OutputDebugString("end left\n");
    }   // end left clip
  }     // end else clipping required
}

// ----------------------------------------------------------------------
// Method:      DrawMirroredCompressedSprite
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to
// draw
// Returns:     None
//
// Description: Mirror the sprite to the back buffer taking account of
//				Transparency encoding
//
//
// ----------------------------------------------------------------------
void DisplayEngine::DrawMirroredCompressedSprite(Position &position,
                                                 CompressedBitmap *bitmap) {

  bool rightClip = false;
  bool bottomClip = false;
  bool topClip = false;
  bool leftClip = false;
  int32 bytes = 0;

  int32 x = position.GetX();
  int32 y = position.GetY();

  int32 bitmapWidth = bitmap->GetWidth();
  int32 bitmapHeight = bitmap->GetHeight();

  // work out how much to increase the data and sreen pointers
  // on when drawing
  uint32 data_step = bitmapWidth;
  // when both left and right clipped
  // top and bottom clipped
  uint32 rightClipped = 0;
  uint32 leftClipped = bitmapWidth;
  uint32 topClipped = bitmapHeight;

  uint16 *compressedData_ptr = bitmap->GetData();
  ASSERT(compressedData_ptr);

  uint32 screen_step = myPitch;

  uint16 *screen_ptr = GetBackBufferPtr();
  ASSERT(screen_ptr);

  // determine whether we have to clip the
  // sprite
  // As we are mirrored we must swap the clippin over so that
  // the new left side of the bitmap gets clipped by whatever the right side
  // would have been.  this is hard to explain but if you draw it out you'll
  // understand
  if (x < 0) {
    bitmapWidth += x;
    if (bitmapWidth < 0)
      return;
    x = 0;

    leftClip = true;
    //	leftClipped  -=bitmapWidth;
    rightClipped -= bitmapWidth;

  } else {
    //	leftClipped = 0;
    rightClipped = 0;
  }

  if (y < 0) {
    bitmapHeight += y;
    if (bitmapHeight < 0)
      return;
    topClip = true;
    //!!!!!!!!
    compressedData_ptr = (uint16 *)bitmap->GetScanLine(0 - y);
    y = 0;
    topClipped -= bitmapHeight;

  } else {
    topClipped = 0;
  }

  int32 t = (x + bitmapWidth) - myPitch;

  // if the bitmap needs clipping to the right
  if (t >= 0) {
    bitmapWidth -= t;
    if (bitmapWidth <= 0)
      return;
    rightClip = true;
    // this is the position in the bitmap to
    // start drawing at
    //	rightClipped=t;
    leftClipped = t;

  } else {
    //	rightClipped = 0;
    leftClipped = 0;
  }

  t = (y + bitmapHeight) - mySurfaceArea.bottom;

  // if the bitmap needs clipping at the bottom
  if (t >= 0) {
    bitmapHeight -= t;
    if (bitmapHeight <= 0)
      return;
    bottomClip = true;
  }

  bitmapWidth--;

  if (bitmapWidth <= 0)
    return;

  data_step -= bitmapWidth;
  screen_step -= bitmapWidth;

  screen_ptr += (y * myPitch) + x;

  uint16 tag = 0;
  int32 thisStep = 0;
  uint16 count = 0;
  uint32 pixelsDrawnThisLine = 0;

  // keep a pointer to the start of the screen line
  uint16 *screenLineStart = screen_ptr;

  // move the screen pointer to the end of the line so that we can
  // draw backwards
  screen_ptr += bitmapWidth;

  // if no clipping is required
  if (data_step == 0) {
    // draw taking account of transparent pixels

    for (; bitmapHeight--;) {
      tag = *compressedData_ptr++;
      while (tag) {
        // find the number of colours to plot
        count = tag >> 1;

        // check whether the run is transparent or colour
        if (tag & 0x01) {
          for (; count--;) {
            // move along the screen line
            screenLineStart++;
            *screen_ptr-- = *compressedData_ptr;
            compressedData_ptr++; //=sizeof(uint16);
          }
        } else {
          // move along the screen line
          screenLineStart += count;
          screen_ptr -= count;
        }
        tag = *compressedData_ptr++;
      } // end  while tag
      // move to the next screen line
      screenLineStart += screen_step;
      // make sure that the screen pointer draws the line backwards
      screen_ptr = screenLineStart + bitmapWidth;
    }  // end for bitmap heigth--
  }    // end if datastep ==0
  else // some clipping is required
  {
    if ((rightClip && !leftClip) || (bottomClip && !leftClip)) {
      uint32 nextline = topClipped;
      uint32 leftSkip = 0;

      for (; bitmapHeight--;) {
        nextline++;

        thisStep = 0;
        pixelsDrawnThisLine = 0;
        leftSkip = 0;
        // find out what type of pixel we have
        tag = *compressedData_ptr++;
        while (tag) {
          // find the number of colours to plot
          count = tag >> 1;

          // calculate which pixel in this line
          // to start from
          thisStep += count;

          // if we have a colour to draw
          if (tag & 0x01) {
            // if we are not at our starting point yet
            if (thisStep <= leftClipped) {
              // move the data on to the next tag
              compressedData_ptr += count;
              leftSkip += count;
            } else {
              // find out how many pixels we are over and
              // draw them to the screen
              uint32 over = thisStep - leftClipped;

              // pretend that we didn't go over so that
              //  we can draw to the end of the line
              thisStep -= over;

              if (leftSkip < leftClipped) {
                compressedData_ptr += leftClipped - leftSkip;
                leftSkip = leftClipped;
              }

              // if we are still supposed to be drawing pixels
              if (over + pixelsDrawnThisLine <= bitmapWidth) {
                // draw the amount we are over
                for (uint32 i = over; i--;) {
                  // move along the screen line
                  screenLineStart++;
                  *screen_ptr-- = *compressedData_ptr++;
                }

                pixelsDrawnThisLine += over;

              } else {
                // too many pixels in this run
                // any portion to draw?
                uint32 therest = bitmapWidth - pixelsDrawnThisLine;
                if (therest) {
                  // draw the amount we are over
                  for (uint32 i = therest; i--;) {
                    // move along the screen line
                    screenLineStart++;
                    *screen_ptr-- = *compressedData_ptr;
                    compressedData_ptr++;
                  }
                  pixelsDrawnThisLine += therest;

                  // now skip to the end of the bitmap data
                  compressedData_ptr = (uint16 *)bitmap->GetScanLine(nextline);
                  if (!compressedData_ptr)
                    return;

                  compressedData_ptr--;
                }
              }
            }
          } // end if colour
          else {
            // we are drawing a transparent pixel
            // calculate which pixel in this line
            // to start from

            // if we are not at our starting point yet
            if (thisStep <= leftClipped) {
              // do nothing for we don't need to draw this
              // part of the bitmap
              leftSkip += count;
            } else // thisStep is larger than data_step
            {
              // find out how many pixels we are over and
              // draw them to the screen
              uint32 over = thisStep - leftClipped;
              // pretend that we didn't go over so that
              //  we can draw to the end of the line
              thisStep -= over;

              // find out how many pixels we are over and
              // draw them to the screen
              // if we are still supposed to be drawing pixels
              if (leftSkip < leftClipped) {
                leftSkip = leftClipped;
              }

              if (over + pixelsDrawnThisLine <= bitmapWidth) {
                // move the screen ptr on by the required amount of steps
                // because these are transparent
                // move along the screen line
                screenLineStart += over;
                screen_ptr -= over;
                pixelsDrawnThisLine += over;
              } else {
                // too many pixels in this run
                // any portion to draw?
                uint32 therest = bitmapWidth - pixelsDrawnThisLine;
                if (therest) {
                  screen_ptr -= therest;
                  // move along the screen line
                  screenLineStart += therest;

                  pixelsDrawnThisLine += therest;
                  // finally skip past the right clipped area
                  compressedData_ptr = (uint16 *)bitmap->GetScanLine(nextline);
                  if (!compressedData_ptr)
                    return;

                  compressedData_ptr--;
                }
              }
            }

          } // end colour check
          tag = *compressedData_ptr++;
        } // end while tag
          // move along the screen line
        screenLineStart += screen_step;
        screen_ptr = screenLineStart + bitmapWidth;
      } // bitmap height
    }   // end if right/bottom
    else {

      uint32 i = 0;
      int32 drawHeight = bitmapHeight;
      if (topClip) {
        i = topClipped;
        drawHeight = bitmapHeight + topClipped;
      }

      // for each line
      for (i; i < drawHeight; i++) {

        thisStep = bitmapWidth;
        // find out what type of pixel we have
        tag = *compressedData_ptr++;
        while (tag) {
          // find the number of pixels to plot
          count = tag >> 1;

          // we have a run of colours
          if (tag & 0x01) {
            // if we are not at our stopping point yet
            if (count <= thisStep) {
              thisStep -= count;
              // we need to keep drawing unless
              // this is the end of the
              // line in which case there will be a tag
              // anyway
              for (uint32 i = count; i--;) {
                // move along the screen line
                screenLineStart++;
                *screen_ptr-- = *compressedData_ptr++;
              }
            } else // pixel count has over run
            {
              // then draw the remaining amount of pixels
              // and set the step to zero
              // draw all the colours
              // draw what you will
              for (uint32 i = thisStep; i--;) {
                // move along the screen line
                screenLineStart++;
                *screen_ptr-- = *compressedData_ptr++;
              }

              // don't worry about moving past the overrun
              // in the data file
              // since thisStep = 0 will take care of it
              thisStep = 0;
            }
          }    // end if colour
          else // transparent
          {
            // calculate which pixel in this line
            // to start from

            // if we are not at our stopping point yet
            if (count <= thisStep) {
              // skip past the transparent pixels on the screen
              // part of the bitmap
              thisStep -= count;
              // move along the screen line
              screenLineStart += count;
              screen_ptr -= count;
              tag = *compressedData_ptr++;
              continue;
            } else // pixel count has over run
            {
              // move along the screen line
              screenLineStart += thisStep;
              screen_ptr -= thisStep;
              // don't worry about moving past the overrun
              // in the data
              // since thisStep = 0 will take care of it
              thisStep = 0;
            }
          } // end else black

          if (thisStep == 0) {
            // no more to draw
            // move the data ptr on to the end of this line
            //
            // no more to draw
            // move the data ptr on to the end of this line
            //
            if (i + 1 < drawHeight) {
              //!!!!!!!!!
              compressedData_ptr = (uint16 *)bitmap->GetScanLine(i + 1);

              if (!compressedData_ptr)
                return;

              compressedData_ptr--;
            } else {
              // force the tag to zero so that everything stops
              tag = 0;
              continue;
            }

          } // end this step over

          tag = *compressedData_ptr++;
        } // end while tag
        // move along the screen line
        screenLineStart += screen_step;
        screen_ptr = screenLineStart + bitmapWidth;
      } // bitmap height

    } // end left clip
  }   // end else clipping required
}

/////////////////////////////////////////////////////////////////////////////////////
//	Display Utitilities
/////////////////////////////////////////////////////////////////////////////////////
// stolen magic code
bool DisplayEngine::ClipLine(RECT *mb, int &x0, int &y0, int &x1, int &y1,
                             unsigned char colour) {
  //	if(x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0)
  //		return false;

  int x = 0, y = 0, outcode0, outcode1, outcodeout;
  bool accept, done;

  //	return;

  accept = FALSE;
  done = FALSE;

  /* Calc outcodes */
  outcode0 = CalcOutCode(mb, x0, y0);
  outcode1 = CalcOutCode(mb, x1, y1);

  do {
    if (!outcode0 & !outcode1) {
      accept = TRUE; /* trivial accept */
      done = TRUE;
    } else if ((outcode0 & outcode1) != 0) {
      done = TRUE; /* trivial reject */
    } else {
      if (outcode0)
        outcodeout = outcode0;
      else
        outcodeout = outcode1;

      /* bit masks: 1=BOTTOM,2=TOP,4=RIGHT,8=LEFT */
      if (outcodeout & 1) {
        x = x0 + ((x1 - x0) * ((mb->bottom - 1) - y0)) / (y1 - y0);
        y = mb->bottom - 1;
      } else if (outcodeout & 2) {
        x = x0 + ((x1 - x0) * (mb->top - y0)) / (y1 - y0);
        y = mb->top;
      } else if (outcodeout & 4) {
        y = y0 + ((y1 - y0) * ((mb->right - 1) - x0)) / (x1 - x0);
        x = mb->right - 1;
      } else if (outcodeout & 8) {
        y = y0 + ((y1 - y0) * (mb->left - x0)) / (x1 - x0);
        x = mb->left;
      }

      if (outcodeout == outcode0) {
        x0 = x;
        y0 = y;
        outcode0 = CalcOutCode(mb, x0, y0);
      } else {
        x1 = x;
        y1 = y;
        outcode1 = CalcOutCode(mb, x1, y1);
      }
    }
  } while (!done);

  return accept;
}

int DisplayEngine::CalcOutCode(RECT *mb, int x, int y) {
  int outcode = 0;

  /* bit masks: 1=BOTTOM,2=TOP,4=RIGHT,8=LEFT */

  if (y >= mb->bottom)
    outcode |= 1;
  else if (y < mb->top)
    outcode |= 2;

  if (x >= mb->right)
    outcode |= 4;
  else if (x < mb->left)
    outcode |= 8;

  return (outcode);
}

SDL_Surface *DisplayEngine::CreateSurface(int32 width, int32 height,
                                          bool tryVideoFirst /*=false*/) {
  SDL_Surface *image;

  int bitdepth = 16;

  if (myPixelFormat == RGB_555) {
    image = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY, width, height, bitdepth, 0x7C00,
                                 0x03E0, 0x001F, 0);
  } else // 565
  {
    image = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY, width, height, bitdepth, 0xF800,
                                 0x07E0, 0x001F, 0);
  }

  return image;
}

void DisplayEngine::ReleaseSurface(SDL_Surface *&tempSurface) {
  SDL_FreeSurface(tempSurface);
  tempSurface = NULL;
}

bool DisplayEngine::CreateProgressBar(Bitmap *bitmap) {
  if (!bitmap)
    return false;

  myProgressBitmap = bitmap;

  //	myProgressSurface = SDL_CreateRGBSurface( SDL_SWSURFACE,
  //			myProgressBitmap->GetWidth(),
  // myProgressBitmap->GetHeight(), 16, 			0xF800,
  // 0x07E0, 			0x001F,0 );

  myProgressSurface = CreateSurface(myProgressBitmap->GetWidth(),
                                    myProgressBitmap->GetHeight());

  if (!myProgressSurface)
    return false;

  SDL_SetColorKey(myProgressSurface, SDL_SRCCOLORKEY, 0);

  const uint16 *sourcePtr = myProgressBitmap->GetData();
  uint16 *destPtr = (uint16 *)myProgressSurface->pixels;

  int32 bitmapWidth = bitmap->GetWidth();
  int32 bitmapHeight = bitmap->GetHeight();

  // the surface is created to be the same
  // size as the entity bounds
  int32 destStep = (myProgressSurface->pitch >> 1);
  int32 sourceStep = 0;
  destStep = destStep - bitmapWidth;
  for (; bitmapHeight--;) {
    for (int32 width = bitmapWidth; width--;)
      *destPtr++ = *sourcePtr++;

    destPtr += destStep;
  }

  return true;
}

void DisplayEngine::StartProgressBar(int updateIntervals) {
  if (myProgressBitmap && myEngineRunningFlag) {
    myProgressMax = updateIntervals;
    myProgressCount = 0;

    myProgressBarHasBeenStarted = true;
    myPreviousProgressRight = -1;
  }
}

void DisplayEngine::UpdateProgressBar(int amount) {
  if (!myProgressBitmap || !myProgressSurface)
    return;

  myProgressCount += amount;
  if (myProgressCount > myProgressMax)
    myProgressMax = myProgressMax;

  RECT clip;

  clip.left = 0;
  clip.top = 0;
  clip.right = int((double)myProgressBitmap->GetWidth() *
                   (double)myProgressCount / (double)myProgressMax);
  if (clip.right < 0)
    clip.right = 0;
  if (clip.right > myProgressBitmap->GetWidth())
    clip.right = myProgressBitmap->GetWidth();
  clip.bottom = myProgressBitmap->GetHeight();

  if (clip.right == myPreviousProgressRight)
    return;
  myPreviousProgressRight = clip.right;

  RECT source;
  source.top = myProgressBitmap->GetPosition().GetY();
  source.left = myProgressBitmap->GetPosition().GetX();
  source.right = source.left + clip.right;
  source.bottom = source.top + clip.bottom;

  BlitToFrontBuffer(source, myProgressSurface, clip, true);
}

void DisplayEngine::EndProgressBar() {
  myProgressBitmap = NULL;
  myProgressBarHasBeenStarted = false;
}

uint16 DisplayEngine::ConvertRGB(int r, int g, int b) {
  uint16 ret;
  if (myPixelFormat == RGB_565)
    RGB_TO_565(r, g, b, ret)
  else
    RGB_TO_555(r, g, b, ret)
  return ret;
}

void DisplayEngine::DrawGuttering(int width, int height) {
  SDL_Rect r;
  if (width < mySurfaceArea.right) {
    r.x = width;
    r.y = 0;
    r.w = mySurfaceArea.right - width;
    r.h = mySurfaceArea.bottom;
    // SDL_FillRect(myBackBuffer, &r, 0);
  }
  if (height < mySurfaceArea.bottom) {
    r.x = 0;
    r.y = height;
    r.w = mySurfaceArea.right;
    r.h = mySurfaceArea.bottom - height;
    // SDL_FillRect(myBackBuffer, &r, 0);
  }
}
