#ifndef		BACKGROUND_H
#define		BACKGROUND_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	<stdlib.h>
#include	"Position.h"
#include	<string>
#include    <vector>
#include	<list>

#define		BACKGROUND_WIDTH			DEFAULT_ENVIRONMENT_WIDTH
#define		BACKGROUND_HEIGHT			DEFAULT_ENVIRONMENT_HEIGHT

#define		BACKGROUND_BITMAP_WIDTH		DEFAULT_ENVIRONMENT_RESOLUTION
#define		BACKGROUND_BITMAP_HEIGHT	DEFAULT_ENVIRONMENT_RESOLUTION

typedef std::pair<int,int> IntegerPair;
typedef std::list<IntegerPair> IntegerPairList;
typedef IntegerPairList::iterator IntegerPairListIterator;


class Gallery;

class Background 
{

public:

	Background();

	~Background();

	void Draw(bool completeRedraw,
		std::vector<RECT>& dirtyRects,
		IntegerPairList& dirtyTiles);

	bool InitialiseGallery(std::string const& gallery_name);

	Position GetDisplayPosition(void) const{return myWorldPosition;}

	int32 GetPixelWidth(){return myPixelWidth;}
	int32 GetPixelHeight(){return myPixelHeight;}

	inline void SetDisplayPosition(Position& position);

	void GetConsideredDisplayArea(int32& displayWidth,
								int32& displayHeight);


	bool Create(Position pos,
		std::string const& gallery_name,
				Position topLeft,
				uint32 width,
				uint32 height);

	bool Create(std::string const& gallery_name,
					Position topLeft,
					Position displayPosition);


	bool Create(std::string const& gallery_name,
						RECT& bounds,
							Position displayposition);


	inline int32 GetTop();
	inline int32 GetLeft();

	inline Position GetTopLeft() const;

	std::string GetBackgroundName() const;

	std::string GetBackgroundPath() const;

	uint32 GetWidth()const {return myWidth;}
	uint32 GetHeight()const {return myHeight;}



private:

	// Copy constructor and assignment operator
	// Declared but not implemented
	Background (const Background&);
	Background& operator= (const Background&);

	Position	myWorldPosition;
	Position	myTopLeftWorldCoordinate;

	int32		myWidth;
	int32		myHeight;

	int32		myPixelWidth;
	int32		myPixelHeight;
	
	Gallery*	myGallery;

};

int32 Background::GetTop()
{
	return myTopLeftWorldCoordinate.GetY();
}

int32 Background::GetLeft()
{
	return myTopLeftWorldCoordinate.GetX();
}

Position Background::GetTopLeft() const
{
	return myTopLeftWorldCoordinate;
}




// ----------------------------------------------------------------------
// Method:      SetDisplayPosition 
// Arguments:   position - world coordinates of the view
//			
// Returns:     None
//
// Description: Update the current world coordinates of the view so that
//				we know which portions of the background we need to draw.
//				Get the real world coordinates and then convert them to 
//				absolute coordinates from 0,0 by subtracting them from the
//				top left corner.
//				
//			
// ----------------------------------------------------------------------
void Background::SetDisplayPosition(Position& position)
{
	myWorldPosition=position;
	myWorldPosition-= myTopLeftWorldCoordinate;
}


#endif		// BACKGROUND_H

