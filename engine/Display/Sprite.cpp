// --------------------------------------------------------------------------
// Filename:	Sprite.h
// Class:		Sprite
// Purpose:		Sprites are bitmap holders they know how to draw
// themselves
//				taking account of transparent
//
//
//
//
// Description: Bitmaps can be either compressed or normal.  If they are
//				compressed then they have pointers to the first
// pixel 				in every line.
//
//				Bitmaps either create their own data or point to
//				their data in a memory mapped file.
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added LoadFromS16
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include	"Sprite.h"
#include	"DisplayEngine.h"
#include	"SharedGallery.h"
#include	"CompressedBitmap.h"
#include	"TintManager.h"

Sprite::FontCache Sprite::myFontGalleries;

Sprite::Sprite() : myGallery(0), myOverlayGallery(0) {
  // we can hide some layers if we want
  // but first of all set them to true
  myAmIASpriteFlag = true;

  myIAmClonedFlag = false;

  myDrawMirroredFlag = false;

  myFileFormat = Gallery::IAmUncompressed;

  myDrawAlphaFlag = false;
  myAlphaIntensity = 0;

  int i;
  for (i = 0; i < NUM_BITMAPS; i++) {
    myBitmapsToDraw[i] = true;
    myBitmapsToDrawMirrored[i] = false;
    myCompressedBitmaps[i] = NULL;
  }

  for (i = 0; i < NUM_BITMAPS; i++) {
    myBitmaps[i] = NULL;
  }
}

Sprite::~Sprite() {
  int i;
  for (i = 0; i < NUM_BITMAPS; i++) {
    myBitmaps[i] = NULL;
    myCompressedBitmaps[i] = NULL;
    myBitmapsToDrawMirrored[i] = false;
  }

  if (myGallery) {
    SharedGallery::RemoveGallery(myGallery);
    myGallery = NULL;
  }

  if (myOverlayGallery) {
    SharedGallery::RemoveGallery(myOverlayGallery);
    myOverlayGallery = NULL;
  }
}

void Sprite::Draw() {
  if (myDrawMirroredFlag) {
    DrawMirrored();
  } else if (myDrawAlphaFlag && myAlphaIntensity != 0) {
    DrawAlpha();
  } else {
    int i;
    if (myFileFormat == Gallery::IAmUncompressed) {
      for (i = 0; i < NUM_BITMAPS; i++) {

        if (myBitmaps[i] && myBitmapsToDraw[i] == true) {
          if (myBitmapsToDrawMirrored[i] == true) {
            DisplayEngine::theRenderer().DrawMirroredSprite(GetScreenPosition(),
                                                            *myBitmaps[i]);
          } else {
            DisplayEngine::theRenderer().DrawSprite(GetScreenPosition(),
                                                    *myBitmaps[i]);
          }
        }
      }
    } else {
      for (i = 0; i < NUM_BITMAPS; i++) {
        if (myCompressedBitmaps[i] && myBitmapsToDraw[i] == true) {

          if (myBitmapsToDrawMirrored[i] == true) {
            DisplayEngine::theRenderer().DrawMirroredCompressedSprite(
                GetScreenPosition(), myCompressedBitmaps[i]);
          } else {
            DisplayEngine::theRenderer().DrawCompressedSprite(
                GetScreenPosition(), myCompressedBitmaps[i]);
          }
        }
      }
    }
  }
}

void Sprite::DrawMirrored() {
  int i;

  if (myFileFormat == Gallery::IAmUncompressed) {

    for (i = 0; i < NUM_BITMAPS; i++) {
      //
      if (myBitmaps[i] && myBitmapsToDraw[i] == true) {
        DisplayEngine::theRenderer().DrawMirroredSprite(GetScreenPosition(),
                                                        *myBitmaps[i]);
      }
    }
  } else {
    for (i = 0; i < NUM_BITMAPS; i++) {
      if (myCompressedBitmaps[i] && myBitmapsToDraw[i] == true) {
        DisplayEngine::theRenderer().DrawMirroredCompressedSprite(
            GetScreenPosition(), myCompressedBitmaps[i]);
      }
    }
  }
}

void Sprite::HideBitmap(int32 layer) {
  myBitmapsToDraw[layer] = false;

  // show all layers below this one
  for (int i = layer + 1; i < NUM_BITMAPS; i++) {
    myBitmapsToDraw[i] = true;
  }
}

void Sprite::ShowBitmap(int32 layer) {
  myBitmapsToDraw[layer] = true;

  // blank out all layers below this one
  if (layer == 0) {
    for (int i = layer + 1; i < NUM_BITMAPS; i++) {
      myBitmapsToDraw[i] = false;
    }
  }
}

uint16 *Sprite::GetImageData() const {
  if (myFileFormat == Gallery::IAmUncompressed)
    return myBitmaps[0]->GetData();
  else
    return myCompressedBitmaps[0]->GetData();
}

int32 Sprite::GetWidth() {
  if (myFileFormat == Gallery::IAmUncompressed) {
    _ASSERT((myBitmaps[0]));
    return (myBitmaps[0])->GetWidth();
  } else {
    _ASSERT((myCompressedBitmaps[0]));
    return (myCompressedBitmaps[0])->GetWidth();
  }
}

int32 Sprite::GetHeight() {
  if (myFileFormat == Gallery::IAmUncompressed) {
    _ASSERT((myBitmaps[0]));
    return (myBitmaps[0])->GetHeight();
  } else {
    _ASSERT((myCompressedBitmaps[0]));
    return (myCompressedBitmaps[0])->GetHeight();
  }
}

Bitmap *Sprite::GetBitmap(void) {
  if (myFileFormat == Gallery::IAmUncompressed) {
    return (myBitmaps[0]);
  } else {
    return (myCompressedBitmaps[0]);
  }
}

bool Sprite::SetBitmap(uint32 index, int32 layer, WhichGallery choice) {

  Gallery *galleryChoice = NULL;

  switch (choice) {
  case PRIMARY_GALLERY: {
    galleryChoice = myGallery;
    break;
  }
  case OVERLAY_GALLERY: {
    galleryChoice = myOverlayGallery;
    break;
  }
  default: {
    galleryChoice = myGallery;
    break;
  }
  }

  if (layer >= NUM_BITMAPS) {
    return false;
  }

  if (myFileFormat == Gallery::IAmUncompressed) {
    if (!(myBitmaps[layer] = galleryChoice->GetBitmap(index)))
      return false;

  } else {
    if (!(myCompressedBitmaps[layer] =
              (CompressedBitmap *)galleryChoice->GetBitmap(index)))
      return false;
  }
  return true;
}

Bitmap *Sprite::GetBitmap(uint32 index) const {
  return myGallery->GetBitmap(index);
}

void Sprite::SetScreenPosition(Position pos) { myScreenPosition = pos; }

void Sprite::SetClippedDimensions(int32 w, int32 h) {
  int i;
  for (i = 0; i < NUM_BITMAPS; i++) {
    if (myBitmaps[i])
      myBitmaps[i]->SetClippedDimensions(w, h);

    if (myCompressedBitmaps[i])
      myCompressedBitmaps[i]->SetClippedDimensions(w, h);
  }
}

bool Sprite::Visible(RECT &test) {
  /// implement this if still used??!!!
  _ASSERT(false);
  return false;
}

// this is so that the dirty rects can look after themselves
void Sprite::SetCurrentBound(RECT *rect /*= NULL*/) {
  if (rect) {
    myCurrentBound.bottom = rect->bottom + 1;
    myCurrentBound.right = rect->right + 1;
    myCurrentBound.top = rect->top;
    myCurrentBound.left = rect->left;
  }
}

// ----------------------------------------------------------------------
// Method:      SetGallery
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseImage - the image in the sprite file that
// this 							 particular entity will start from
// numImages - the number of images for this sprite
//
//
// Returns:     None
//
// Description: Create a gallery for this entity  and set up a
//				sprite to hold your current image data	in this
// base class 				the number of images is largely ignored,
// override if you 				want to utilise it
//
// ----------------------------------------------------------------------
bool Sprite::SetGallery(std::string const &galleryName) {

  if (galleryName.empty())
    return false;

  // if you already have a gallery get rid of it
  if (myGallery && myGallery->GetName() != galleryName) {

    SharedGallery::theSharedGallery().RemoveGallery(myGallery);
    myGallery = 0;
  }

  if (!myGallery) {
    // entity images can use one of several galleries
    // let the gallery decide which type of sprite to create
    myGallery = SharedGallery::theSharedGallery().CreateGallery(galleryName);

    // now we have a gallery we can create our sprite
    if (myGallery) {
      myFileFormat = myGallery->GetFileFormat();

      return true;
    }
  }

  return false;
}

bool Sprite::SetGallery(std::string const &galleryName, uint32 baseImage,
                        uint32 numImages /*= 0*/) {
  myGallery = SharedGallery::theSharedGallery().CreateClonedGallery(
      galleryName, baseImage, numImages);
  myIAmClonedFlag = true;

  // now we have a gallery we can create our sprite
  if (myGallery) {
    myFileFormat = myGallery->GetFileFormat();

    return true;
  }

  return (myGallery != NULL);
}

uint32 Sprite::GetBitmapCount() const { return myGallery->GetCount(); }

// ----------------------------------------------------------------------
// Method:      SetOverlayGallery
// Arguments:   galleryName - file name containg the sprites for this entity
//
//
// Returns:     None
//
// Description: Create an overlay gallery for this entity.  Certain
//				types of entity (body parts) will want to
//replace
// their 				normal image by one from an overlay gallery.  The
// overlay 				gallery is optional and must be created
// explicitly if 				it is needed.
//
// ----------------------------------------------------------------------
void Sprite::SetOverlayGallery(std::string const &galleryName) {
  if (galleryName.empty())
    return;

  // If you have a gallery, and it is not the one you want...
  if (myOverlayGallery && myOverlayGallery->GetName() != galleryName) {
    SharedGallery::theSharedGallery().RemoveGallery(myOverlayGallery);
    myOverlayGallery = NULL;
  }

  // If no gallery yet...
  if (!myOverlayGallery) {
    // entity images can use one of several galleries
    // let the gallery decide which type of sprite to create
    myOverlayGallery =
        SharedGallery::theSharedGallery().CreateGallery(galleryName);
  }

  // char db[256];
  //	sprintf(db, "SetOverlayGallery %s\n", galleryName.c_str());
  //	OutputDebugString(db);
}

void Sprite::DestroyPrimaryGallery() {
  _ASSERT(myGallery);
  SharedGallery::theSharedGallery().RemoveGallery(myGallery);
}

bool Sprite::IsPixelTransparent(uint32 x, uint32 y, int32 imageIndex) {
  return myGallery->IsPixelTransparent(x, y, imageIndex);
}

std::string Sprite::GetGalleryName() const {
  if (myGallery)
    return myGallery->GetName();

  return "";
}

std::string Sprite::GetOverlayGalleryName() const {
  if (myOverlayGallery)
    return myOverlayGallery->GetName();

  return "";
}

void Sprite::RecolourPrimaryGallery(const uint16 *tintTable) {
  myGallery->Recolour(tintTable);
}

void Sprite::CopyGalleryData(ClonedGallery &gallery) {
  if (myGallery)
    DestroyPrimaryGallery();

  myGallery = new Gallery(gallery);

  if (myGallery) {
    myFileFormat = myGallery->GetFileFormat();

    SharedGallery::theSharedGallery().AddClonedGallery(myGallery);
  }
}

uint32 Sprite::ClonedAbsoluteBase() const {
  return myGallery->ClonedAbsoluteBase();
}

// ----------------------------------------------------------------------
// Method:      DrawOnMe
// Arguments:   whereToDraw - whjere to start plotting on the bitmap
//				bitmapToDraw - what we should be drawing
// Returns:     None
// Description: Allows a sprite to be drawn ontop of this sprite
//
// ----------------------------------------------------------------------
void Sprite::DrawOnMe(Position &whereToDraw, Bitmap *const bitmapToDraw) {
  if (myIAmClonedFlag) {
    DisplayEngine::theRenderer().DrawSpriteToBitmap(myBitmaps[0], whereToDraw,
                                                    bitmapToDraw);
  }
}

void Sprite::DrawString(int32 x, int32 y, const std::string &text,
                        int defaultFontIndex,
                        std::vector<Sprite::CharStyle> charStyles,
                        int stylesOffset) {

  if (myIAmClonedFlag) {
    DisplayEngine &engine = DisplayEngine::theRenderer();
    Gallery *defaultTextGallery = myFontGalleries[defaultFontIndex].myGallery;
    if (!defaultTextGallery)
      return;

    Bitmap *destination = myBitmaps[0];

    // if you have been passed specific colours
    // to draw in then do

    uint32 length = text.size();

    uint32 destinationWidth = destination->GetWidth();
    uint32 destinationHeight = destination->GetHeight();

    // all letters are of the same width OK?
    uint32 sourceHeight = defaultTextGallery->GetBitmapWidth(0);

    int32 nDrawn = 0;

    // only draw whole letters
    if (y + sourceHeight >= destinationHeight)
      return;

    uint32 sourceWidth;

    int fontIndex = defaultFontIndex;
    int styleScanner = 0;

    // Note side-effect: sourceWidth =...
    while (nDrawn < length) {
      int c = text[nDrawn];
      if (c < 0)
        c += 256;
      c -= ' ';
      if (c >= 0) {
        sourceWidth = defaultTextGallery->GetBitmapWidth(c);
        if (x + sourceWidth > destinationWidth)
          break;

        // Look for change of colour
        while (styleScanner < charStyles.size()) {
          int start = charStyles[styleScanner].myCharStart;
          if (nDrawn >= start - stylesOffset) {
            fontIndex = charStyles[styleScanner].myFontIndex;
            ++styleScanner;
          } else
            break;
        }

        Gallery *textGallery = myFontGalleries[fontIndex].myGallery;
        if (!textGallery)
          return;
        Bitmap *bitmap = textGallery->GetBitmap(c);
        if (bitmap) {
          // for each letter in the text draw a letter to the bitmap
          DrawSpriteToBitmap(destination, Position(x, y), bitmap);
        } else {
          //				OutputDebugString("no bitmaps found in
          // Chars.s16");
          return;
        }
        x += sourceWidth;
      }
      ++nDrawn;
    }
  }
}

int Sprite::SelectFont(const std::string &fontName, int r, int g, int b,
                       int rot, int swap) {
  FontCache::iterator it;
  int index = 0;
  for (it = myFontGalleries.begin(); it != myFontGalleries.end(); ++it) {
    if (it->myName == fontName && it->r == r && it->g == g && it->b == b &&
        it->rot == rot && it->swap == swap)
      return index;
    ++index;
  }

  // Make a cloned gallery
  Gallery *gallery =
      SharedGallery::theSharedGallery().CreateClonedGallery(fontName, 0, 0);
  ASSERT(gallery);

  // Tint it
  TintManager manage;
  manage.BuildTintTable(r, g, b, rot, swap);
  gallery->Recolour(manage.GetMyTintTable());

  // Throw exception if text gallery not found
  myFontGalleries.push_back(CachedFont(fontName, gallery, r, g, b, rot, swap));
  return myFontGalleries.size() - 1;
}

void Sprite::MeasureString(const std::string &text, int fontIndex, int32 &width,
                           int32 &height) {
  if (myIAmClonedFlag) {
    height = 0;
    width = 0;

    Gallery *textGallery = myFontGalleries[fontIndex].myGallery;

    if (!textGallery)
      return;

    uint32 length = text.size();

    for (uint32 i = 0; i < length; ++i) {
      int c = text[i];
      if (c < 0)
        c += 256;
      c -= ' ';
      if (c >= 0) {
        width += textGallery->GetBitmapWidth(c);
        int h = textGallery->GetBitmapHeight(c);
        if (h > height)
          height = h;
      }
    }
  }
}

void Sprite::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
                      uint8 lineColourRed /*= 0*/,
                      uint8 lineColourGreen /*= 0*/,
                      uint8 lineColourBlue /*= 0*/) {
  if (myIAmClonedFlag) {
    // ask the display engine to do this for you
    DisplayEngine::theRenderer().DrawLineToBitmap(
        myBitmaps[0], x1, y1, x2, y2, lineColourRed, lineColourGreen,
        lineColourBlue);
  }
}

void Sprite::DrawSpriteToBitmap(Bitmap *destination, Position position,
                                Bitmap *const source) {
  _ASSERT(myIAmClonedFlag == true);

  if (myIAmClonedFlag) {
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

    while (sourceHeight--) {
      for (uint32 width = 0; width < sourceWidth; width++) {
        *destPtr++ = *sourcePtr++;
      }
      destPtr += dest_step;
    }
  }
}

void Sprite::Clear() {
  if (myIAmClonedFlag) {
    for (int i = 0; i < NUM_BITMAPS; i++) {
      if (myBitmaps[i])
        myBitmaps[i]->ResetCanvas();
    }
  }
}

void Sprite::CreateGalleryForCloning(std::string path, uint32 baseImage,
                                     uint32 pixelFormat, uint32 numBitmaps) {

  myGallery = new Gallery();

  if (myGallery) {
    myGallery->SetUpGalleryShellForCloning(path, baseImage, pixelFormat,
                                           numBitmaps);

    myFileFormat = Gallery::IAmUncompressed;

    SharedGallery::theSharedGallery().AddClonedGallery(myGallery);

    myIAmClonedFlag = true;
  }
}

void Sprite::DrawMirrored(uint32 layer, bool yesOrNo) {
  _ASSERT(layer < NUM_BITMAPS);

  if (layer < NUM_BITMAPS && layer >= 0) {
    myBitmapsToDrawMirrored[layer] = yesOrNo;
  }
}

void Sprite::DrawMirrored(bool mirror) {
  myDrawMirroredFlag = mirror;
  for (int i = 0; i < NUM_BITMAPS; i++) {
    myBitmapsToDrawMirrored[i] = mirror;
  }
}

void Sprite::DrawAlphaBlended(bool drawAlpha, int intensity) {
  myDrawAlphaFlag = drawAlpha;
  myAlphaIntensity = intensity;
}

void Sprite::DrawAlpha() {
  for (int i = 0; i < NUM_BITMAPS; ++i) {
    if (myFileFormat == Gallery::IAmCompressed) {
      if (myCompressedBitmaps[i] && myBitmapsToDraw[i] == true) {
        DisplayEngine::theRenderer().DrawAlphaCompressedSprite(
            GetScreenPosition(), myCompressedBitmaps[i], myAlphaIntensity);
      }
    } else if (myFileFormat == Gallery::IAmUncompressed) {
      if (myBitmaps[i] && myBitmapsToDraw[i] == true) {
        DisplayEngine::theRenderer().DrawAlphaSprite(
            GetScreenPosition(), *myBitmaps[i], myAlphaIntensity);
      }
    }
  }
}

CreaturesArchive &operator<<(CreaturesArchive &ar,
                             Sprite::CharStyle const &style) {
  ar << style.myCharStart << style.myFontIndex;
  return ar;
}

CreaturesArchive &operator>>(CreaturesArchive &ar, Sprite::CharStyle &style) {
  ar >> style.myCharStart >> style.myFontIndex;
  return ar;
}
