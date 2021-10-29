// --------------------------------------------------------------------------
// Filename:	Background.cpp
// Class:		Background
// Purpose:		This class handles the drawing of the background tiles.
//
//
//
//
// Description: The background has a pointer to its gallery.  All background
//				tiles must be the same size 128 by 128.  This
//class uses the 				DisplayEngine and SharedGallery
// classes. 				The background effectively only has one
// bitmap in the gallery 				which always points to the relevant
// tile.
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added shared gallery.
// 20Jan99  Alima			Now we can read the Background file from
// the CD
//							if the path does not exist on a
//cd
// drive then 							the original path remains
// assuming that the 							background file
// has been installed to that path.
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include	"Background.h"
#include	"Gallery.h"
#include	"Bitmap.h"
#include	"DisplayEngine.h"
#include	"SharedGallery.h"
#include	"Position.h"
#include	"ErrorMessageHandler.h"

Background::Background()
    : myWidth(0), myHeight(0), myGallery(NULL), myPixelWidth(0),
      myPixelHeight(0) {}

// ----------------------------------------------------------------------
// Method:      Create
// Arguments:   gallery_name - file containing the gallery
//                                  just dirty rects
//              topLeft - world co-ordinates of the top left
//							corner of this
// background
//
// Returns:     true if the background creates OK false otherwise
//
// Description: Main part of the two stage background creation
//
//
// ----------------------------------------------------------------------
bool Background::Create(std::string const &gallery_name, Position topLeft,
                        Position worldPosition) {

  myWorldPosition = worldPosition;

  myTopLeftWorldCoordinate = topLeft;

  if (InitialiseGallery(gallery_name)) {

    if (myGallery && myGallery->IsValid()) {

      myWidth = myGallery->GetTileWidth();
      myHeight = myGallery->GetTileHeight();

      myPixelWidth = myWidth * myGallery->GetBitmapWidth(0);
      myPixelHeight = myHeight * myGallery->GetBitmapHeight(0);
      return true;
    }
  }
  return false;
}

bool Background::Create(Position pos, std::string const &gallery_name,
                        Position topLeft, uint32 pixelwidth,
                        uint32 pixelheight) {
  myTopLeftWorldCoordinate = topLeft;
  myWorldPosition = pos;

  if (InitialiseGallery(gallery_name)) {

    if (myGallery && myGallery->IsValid()) {

      myWidth = myGallery->GetTileWidth();
      myHeight = myGallery->GetTileHeight();

      myPixelWidth = pixelwidth;
      myPixelHeight = pixelheight;
      return true;
    }
  }

  return false;
}

bool Background::InitialiseGallery(std::string const &gallery_name) {
  if (!gallery_name.empty()) {
    myWidth = BACKGROUND_WIDTH;
    myHeight = BACKGROUND_HEIGHT;

    // if you already have a gallery get rid of it
    if (myGallery) {
      SharedGallery::theSharedGallery().RemoveGallery(myGallery);
      myGallery = NULL;
    }

    myGallery = SharedGallery::theSharedGallery().CreateGallery(gallery_name);

    if (myGallery)
      return true;
  }

  return false;
}

// ----------------------------------------------------------------------
// Method:      Create
// Arguments:   gallery_name - file containing the gallery
//                                  just dirty rects
//              topLeft - world co-ordinates of the top left
//							corner of this
// background
//
// Returns:     true if the background creates OK false otherwise
//
// Description: Main part of the two stage background creation
//
//
// ----------------------------------------------------------------------
bool Background::Create(std::string const &gallery_name, RECT &bounds,
                        Position worldposition) {

  myWorldPosition = worldposition;

  myTopLeftWorldCoordinate.SetX(bounds.left);
  myTopLeftWorldCoordinate.SetY(bounds.top);

  if (InitialiseGallery(gallery_name)) {

    // if you already have a gallery get rid of it
    if (myGallery) {
      SharedGallery::theSharedGallery().RemoveGallery(myGallery);
      myGallery = NULL;
    }

    myGallery = SharedGallery::theSharedGallery().CreateGallery(gallery_name);

    if (myGallery && myGallery->IsValid()) {

      myWidth = myGallery->GetTileWidth();
      myHeight = myGallery->GetTileHeight();

      // don't let them mess you arround by giving you the wrong bounds
      // yeah!
      int32 preferredWidth = bounds.right - bounds.left;
      int32 preferredHeight = bounds.bottom - bounds.top;

      if (preferredWidth > myWidth * myGallery->GetBitmapWidth(0)) {
        preferredWidth = myWidth * myGallery->GetBitmapWidth(0);
      }

      if (preferredHeight > myHeight * myGallery->GetBitmapHeight(0)) {
        preferredHeight = myHeight * myGallery->GetBitmapHeight(0);
      }

      myPixelWidth = preferredWidth;
      myPixelHeight = preferredHeight;
      return true;
    }
  }
  return false;
}

Background::~Background() {}

// ----------------------------------------------------------------------
// Method:      GetConsideredDisplayArea
// Arguments:   displayWidth - buffer to recieve the display width
//
//              displayHeight - buffer to recieve the display width
//
// Returns:     None
//
// Description: Works out how far the backgorund can extend for the
//				current screen resolution.  Windowed mode can
//only use
//
//
//
// ----------------------------------------------------------------------
void Background::GetConsideredDisplayArea(int32 &displayWidth,
                                          int32 &displayHeight) {
  displayWidth = DisplayEngine::theRenderer().GetSurfaceWidth();
  displayHeight = DisplayEngine::theRenderer().GetSurfaceHeight();

  if (myPixelWidth != 0) {
    if (displayWidth > myPixelWidth) {
      displayWidth = myPixelWidth;
    }
  }

  if (myPixelHeight != 0) {

    if (displayHeight > myPixelHeight) {
      displayHeight = myPixelHeight;
    }
  }
}

std::string Background::GetBackgroundName() const {
  return myGallery->GetName();
}

// large inline code
// ----------------------------------------------------------------------
// Method:      Draw
// Arguments:   completeRedraw - whether to draw the whole screen or
//                                  just dirty rects
//              dirtyRects - a list of dirty rects to compare against
//
// Returns:     true if the bitmap loads OK false otherwise
//
// Description: Works out which tile needs to be drawn.  If this is not
//				a complete redraw then a tile which coincides
//with each 				rectangle in the list must be found.
//
//
// ----------------------------------------------------------------------
void Background::Draw(bool completeRedraw, std::vector<RECT> &dirtyRects,
                      IntegerPairList &dirtyTiles) {

  if (!(myGallery && myGallery->IsValid()))
    return;

  myGallery->StartTileCount();
  ldiv_t x_div = ldiv((long)myWorldPosition.GetX(),
                      DEFAULT_ENVIRONMENT_RESOLUTION); // bitmap width
  ldiv_t y_div = ldiv((long)myWorldPosition.GetY(),
                      DEFAULT_ENVIRONMENT_RESOLUTION); // bitmap height
  // check whether we need to draw only part of the bitmap
  if (x_div.rem < 0) {
    x_div.quot--;
    x_div.rem += DEFAULT_ENVIRONMENT_RESOLUTION;
  }
  if (y_div.rem < 0) {
    y_div.quot--;
    y_div.rem += DEFAULT_ENVIRONMENT_RESOLUTION;
  }

  // this changes depending on the size of the background and the current
  // resolution
  int32 displayWidth = 0;
  int32 displayHeight = 0;
  GetConsideredDisplayArea(displayWidth, displayHeight);

  // draw the whole background
  if (completeRedraw || dirtyRects.size() > 50) {
    //	the background gallery effectively only has one bitmap which
    // points to the relevant tile

    // I have replaced these with the environment resolution
    // - we know that
    // the width and height are always 128
    //  int32	bitmapWidth =	myGallery->GetBitmapWidth(0);
    //	int32	bitmapHeight =	myGallery->GetBitmapHeight(0);

    // find out which tile we need by dividing the world position
    // by the size of the tiles

    int32 y = y_div.quot;
    int32 x = x_div.quot;

    int32 tile = 0;
    int32 nextSet = y + x * myHeight;

    // make the position a negative number so that by adding
    // 128 to it each tile we draw we get just enough tiles
    // for the size of the display area

    int32 xpos = -x_div.rem;
    int32 ypos = -y_div.rem;

    Bitmap *bitmap = NULL;
    while (ypos < displayHeight) {

      x = x_div.quot;

      y++;
      tile = nextSet;

      while (xpos < displayWidth) {

        // draw a line in the xplane at given position
        bitmap = myGallery->GetTile(tile);
        if (bitmap) {
          bitmap->SetPosition(Position(xpos, ypos));
          if ((xpos < 0 || ypos < 0) ||
              (xpos + DEFAULT_ENVIRONMENT_RESOLUTION > displayWidth ||
               ypos + DEFAULT_ENVIRONMENT_RESOLUTION > displayHeight))
            bitmap->Draw();
          else
            bitmap->DrawWholeBitmapRegardless();

          tile += myHeight;
        }

        xpos += DEFAULT_ENVIRONMENT_RESOLUTION;
        x++;
      }
      // move down a line on the screen

      xpos = -x_div.rem;

      ypos += DEFAULT_ENVIRONMENT_RESOLUTION;
      nextSet++;
    }
  } else {

    // find out which background tiles intersect
    //	with the rect and draw
    // the visible parts of them

    IntegerPairListIterator it;
    Bitmap *bitmap = NULL;

    int32 xpos = 0;
    int32 ypos = 0;
    int32 y = 0;
    int32 x = 0;

    // screen position

    for (it = dirtyTiles.begin(); it != dirtyTiles.end(); it++) {
      x = (*it).first;
      y = (*it).second;
      // get the position of each tile
      xpos = (((*it).first - x_div.quot) << 7) - x_div.rem;
      ypos = (((*it).second - y_div.quot) << 7) - y_div.rem;

      // get the right tile

      bitmap = myGallery->GetTile(((x % myWidth) * myHeight) + (*it).second);
      if (bitmap) {
        bitmap->SetPosition(Position(xpos, ypos));

        if ((xpos < 0 || ypos < 0) ||
            (xpos + DEFAULT_ENVIRONMENT_RESOLUTION > displayWidth ||
             ypos + DEFAULT_ENVIRONMENT_RESOLUTION > displayHeight))
          bitmap->Draw();
        else
          bitmap->DrawWholeBitmapRegardless();
      }

    } // end for rect = dirtyRects

  } // end else not complete draw
}

std::string Background::GetBackgroundPath() const {
  std::string path;
  if (myGallery) {
    path = myGallery->GetName();
  }

  return path;
}
