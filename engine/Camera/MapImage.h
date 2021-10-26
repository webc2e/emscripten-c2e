// --------------------------------------------------------------------------
// Filename:	MapImage.h
// Class:		MapImage
// Purpose:		Draws the Map?
//				
//				
//				
//
// Description: 
//								
//
// History:
// -------  
// Feb98	Alima			Created
//			
// --------------------------------------------------------------------------

#ifndef MAP_IMAGE_H
#define MAP_IMAGE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include	"../../common/BasicException.h"
#include <vector>

class Line;

class MapImage 
{
	public:
	MapImage();
	MapImage(int32 plane,
			bool fullScreen);

	virtual ~MapImage();

	virtual bool Update();

#ifdef C2D_DIRECT_DISPLAY_LIB
	void DrawLineToBackBuffer();
#endif

	void Enable(){myIAmActiveFlag = true;}
	void Disable(){myIAmActiveFlag = false;}


//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
	class MapImageException: public BasicException
	{
	public:
		MapImageException(std::string what, uint16 line):
		  BasicException(what.c_str()),
		lineNumber(line){;}


		uint16 LineNumber(){return lineNumber;}
	private:
		uint16 lineNumber;

	};

	private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	MapImage (const MapImage&);
	MapImage& operator= (const MapImage&);

	void ResetLines();

	std::vector<Line*> myLines;

	bool myIAmActiveFlag;
};

#endif//MAPIMAGE_H

