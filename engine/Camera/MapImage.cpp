// --------------------------------------------------------------------------
// Filename:	MapImage.h
// Class:		MapImage
// Purpose:		To allow clients to draw anything other than sprites to the
//				direct draw surfaces
//				
//				
//				
//
// Description: 
//				On Update() it draws to its own surface.
//				On DrawToBackBuffer it blits it to the back buffer.
//				Note that all such objects should be in plane order but let
//				the mouse be 9999.
//								
//
// History:
// -------  
// Mar/1/98	Alima			Created
//			
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../Display/DisplayEngine.h"


#include "MainCamera.h"
#include "../Display/ErrorMessageHandler.h"

#include "MapImage.h"
#include "../App.h"
#include "../World.h"
#include "../Map/Map.h"
#include "../Display/Line.h"


MapImage::MapImage(int32 plane,
				bool fullScreen)
				:myIAmActiveFlag(0)
{

}

MapImage::~MapImage()
{
	ResetLines();
}

void MapImage::ResetLines()
{
	std::vector<Line*>::iterator line;

	for(line = myLines.begin(); line != myLines.end(); line++)
	{
		theMainView.Remove(*line);
		delete (*line);
	}

	myLines.clear();
}

// ----------------------------------------------------------------------
// Method:      Update 
// Arguments:   None
//				
// Returns:     None
//
// Description: we have the DC from the surface and draw the current map
//				Draw the map to the front surface
//				 
//			
// ----------------------------------------------------------------------
bool MapImage::Update()
{

#ifndef C2D_DIRECT_DISPLAY_LIB

	std::vector<Line*>::iterator line;

	for(line = myLines.begin(); line != myLines.end(); line++)
	{
		theMainView.Remove(*line);
		delete (*line);
	}

	myLines.clear();

	ResetLines();


	if(!myIAmActiveFlag)
		return false;

	DoorCollection &d = theApp.GetWorld().GetMap().GetDoorCollection();
	DoorIterator it = d.begin();
	while (it != d.end()) {
		Door* door = *it;
	
		int x1 = Map::FastFloatToInteger(door->start.x);
	    int y1 = Map::FastFloatToInteger(door->start.y);
		int x2 = Map::FastFloatToInteger(door->end.x);
		int y2 = Map::FastFloatToInteger(door->end.y);
			

		unsigned char red, green, blue;

		if (door->parentCount == 1) {
			red = 0;
			green = 255;
			blue = 255;
		}
		else if (door->permiability == 0) {
			red = 255;
			green = 0;
			blue = 0;

		}
		else if (door->permiability == 100) {
			red = 0;
			green = 255;
			blue = 0;
		}
		else {
			red = 255;
			green = 255;
			blue = 0;
		}	

		Line* line = theMainView.NewLine(x1, y1, x2, y2, red, green, blue, 0, 0, 0, theApp.GetLinePlane());
		if (line)
			myLines.push_back(line);
	


		it++;
		
	}	
	


#endif

	return true;
}



#ifdef C2D_DIRECT_DISPLAY_LIB
// ----------------------------------------------------------------------
// Method:      DrawToBackBuffer 
// Arguments:   entityHandler - the keeper of all display entities
//				
// Returns:     None
//
// Description: Blits the fast object to the back buffer.
//				 
//			
// ----------------------------------------------------------------------
void MapImage::DrawLineToBackBuffer()
{
	RECT rect;
	unsigned char red, green, blue;
	Door *door;
	int l, r, t, b;
	theMainView.GetViewArea(rect);

	DoorCollection &d = theApp.GetWorld().GetMap().GetDoorCollection();
	l = rect.left;
	r = rect.right;
	t = rect.top;
	b = rect.bottom;
	DoorIterator it;

	int p1, p2, p3, p4;
	int x1, y1, x2, y2;
	bool ok;

	it = d.begin();
	while (it != d.end()) {
		door = *it;
	
		x1 = Map::FastFloatToInteger(door->start.x);
	    y1 = Map::FastFloatToInteger(door->start.y);
		x2 = Map::FastFloatToInteger(door->end.x);
		y2 = Map::FastFloatToInteger(door->end.y);
			
		ok = DisplayEngine::ClipLine(&rect, x1, y1, x2, y2, 1);
		if (ok) {
			if (door->parentCount == 1) {
				red = 0;
				green = 255;
				blue = 255;
			}
			else if (door->permiability == 0) {
				red = 255;
				green = 0;
				blue = 0;
			}
			else if (door->permiability == 100) {
				red = 0;
				green = 255;
				blue = 0;
			}
			else {
				red = 255;
				green = 255;
				blue = 0;
			}	
			p1 = x1-l;
			p2 = y1-t;
			p3 = x2-l;
			p4 = y2-t;

			if ((p1 != p3) || (p2 != p4))
				theMainView.DrawLineToScreen(p1, p2, p3, p4, red, green, blue);

		}
		it++;
	}	
}
#endif

