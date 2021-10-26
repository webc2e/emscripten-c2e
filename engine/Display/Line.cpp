#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Line.h"
#include "DisplayEngine.h"

Line::Line(int32 x1, int32 y1, int32 x2, int32 y2,
		   int32 worldx1, int32 worldy1, int32 worldx2, int32 worldy2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/,
							 uint8 stippleOnValue /*=0*/,
							 uint8 stippleOffValue /*=0*/,
							 uint32 stipplestartAt/*=0*/,
							 uint32 _plane/*=9998*/):

	IamValid(1)
{	
	myAmIALineFlag = true;
	myPlane = _plane;
	myCurrentBound.left = worldx1;
	myCurrentBound.right = worldx2;
	myCurrentBound.top = worldy1;
	myCurrentBound.bottom = worldy2;

	myPlane = _plane;
	myWorldx1=worldx1;
	myWorldx2=worldx2;
	myWorldy1=worldy1;
	myWorldy2=worldy2;

	myScreenX1 = x1;
	myScreenY1 = y1;
	myScreenX2 = x2;
	myScreenY2 = y2;

	myStippleOffValue = stippleOffValue;
	myStippleOnValue = stippleOnValue;

	myStippleStartAt = stipplestartAt;

	myLineColourRed=lineColourRed;
	myLineColourGreen=lineColourGreen;
	myLineColourBlue=lineColourBlue;

	IamValid = true;
}

void Line::Draw()
{
	if(IamValid)
	{
		DisplayEngine::theRenderer().
		DrawLine(myScreenX1,myScreenY1,myScreenX2,myScreenY2,
		myLineColourRed,myLineColourGreen,myLineColourBlue,
		myStippleOnValue,
		myStippleOffValue,
		myStippleStartAt);
	}
}


void Line::DrawMirrored()
{
}

void Line::SetCurrentBound(RECT* rect/*= NULL*/)
{
	if(myScreenX2 > myScreenX1)
	{
		myCurrentBound.left =myWorldx1;
		myCurrentBound.right=myWorldx2+1;

	}
	else
	{
		myCurrentBound.left =myWorldx2;
		myCurrentBound.right=myWorldx1+1;
	}

	if(myScreenY2 > myScreenY1)
	{
		myCurrentBound.top = myWorldy1;
		myCurrentBound.bottom = myWorldy2+1;
	}
	else
	{
		myCurrentBound.top = myWorldy2;
		myCurrentBound.bottom = myWorldy1+1;
	}
}

// ----------------------------------------------------------------------
// Method:      GetWorldPosition
// Arguments:   None
// Returns:     None
// Description: This function is needed to draw the object
//				but each drawable object can have a different way of
//				storing it.
//				
//			
// ----------------------------------------------------------------------
Position Line::GetWorldPosition()
{
	return Position(myWorldx1,myWorldy1);
}

void Line::SetScreenPosition(Position pos)
{
	myScreenPosition = pos;

	if(myScreenX2 > myScreenX1)
	{
		int xdelta = myScreenX2 - myScreenX1;

		myScreenX1 = pos.GetX();

		myScreenX2 = myScreenX1 + xdelta;
	}
	else
	{
		int xdelta = myScreenX1 - myScreenX2;

		myScreenX2 = pos.GetX();

		myScreenX1 = myScreenX2 + xdelta;

	}

	if(myScreenY2 > myScreenY1)
	{
		int ydelta = myScreenY2 - myScreenY1;

		myScreenY1 = pos.GetY();

		myScreenY2 = myScreenY1 + ydelta;
	}
	else
	{
		int ydelta = myScreenY1 - myScreenY2;

		myScreenY2 = pos.GetY();

		myScreenY1 = myScreenY2 + ydelta;
	}
}

bool Line::Visible(RECT& rect)
{
	return true;
}

