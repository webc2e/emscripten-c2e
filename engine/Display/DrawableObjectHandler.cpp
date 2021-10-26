// --------------------------------------------------------------------------
// Filename:	DrawableObjectHandler.cpp
// Class:		DrawableObjectHandler
// Purpose:		This class manages all drawable objects. 
//				
//				
//
// Description: This handler receives objects and sorts them into plane order  
//				ready for drawing.  The entitie are put in myRenderObjects
//
//
//				There are two lists of rectangles:
//				myOldRects holds the previous frame's dirty rectangles. 
//				myNewRects holds the current frame's dirty rectangles.
//				Before doing a dirty rect draw the new list is merged into 
//				the old list.
//				When the entities have been drawn
//				myOldRects are discarded and MyNewRects are moved into myOldRects.			
//				
//
//				Note: Eventually there will be methods to add and remove
//					  entities from the handler rather than refreshing
//					  the whole list each update.
// History:
// ------- 
// 11Nov98	Alima			Created
//							Hooks are in to draw cameras
// 16Dec98  Alima			Added dirty Rect scheme
// 20Aug99	Daniel			Made it much cleaner -
//							myRenderMappings is very basically is a list of all sprites
//							shared by all instances of drawable object handlers.
//							This is so that each drawable object handler can have its
//							own interest level concerning camera shyness
//							Made the sorting of agents into planes more efficient.
//
//							
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"DrawableObjectHandler.h"
#include	"System.h"
#include	"DisplayEngine.h"
#include	"Sprite.h"
#include	"Line.h"
#include	"../Camera/CameraSprite.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////
// constants
////////////////////////////////////////////////////////////////////////////
// This is the threshold number of entities above which we will do a complete
// screen redraw
//const int MAX_DIRTY_RECTS = 4000;

DrawableObjectListMap DrawableObjectHandler::myRenderObjects;
DrawableObjectRenderMappings DrawableObjectHandler::myRenderMappings;

bool DrawableObjectHandler::ourAlreadyDoneSetCurrentBoundsThisTick = false;

// ----------------------------------------------------------------------
// Method:      Add 
// Arguments:   array- list of entities to draw	
//				max_entity_count - the total in the array		
//
// Returns:     true  - no fail condition!
//
// Description: Currently this adds a new list of entities.  It maintains
//				a dirty rect list for all objects by getting the bounds for
//				each entity.  The rectangle is only added to the NewRects
//				list if it is in view.  Likewise the entity is only
//				added to myRenderObjects if it is in view.  the entities
//				are added in plane order
//
//				Note: eventually there will be methods to add and remove
//					  entities from the handler rather than refreshing
//					  the whole list each update
//						
// ----------------------------------------------------------------------
bool DrawableObjectHandler::Add(DrawableObject* const sprite)
{
	if(myShutDownFlag)
		return false;

	if (myRenderMappings.find(sprite) != myRenderMappings.end())
		return false;

    RECT displayRect;
    POINT topLeft;
    POINT bottomRight;

    myOwnerCamera->GetViewArea(displayRect);

	// Take the bounding rectangle for each entity
	// for the dirty rect scheme
	RECT rect;
    sprite->GetBound(rect);

    //weed out rects that are just not on screen
    topLeft.x = rect.left;
    topLeft.y = rect.top;
    bottomRight.x = rect.right;
    bottomRight.y = rect.bottom;

	if(IsRectOnScreen(rect,displayRect))
	{
	  //clip rectangles that are not on screen here
    if(rect.top < displayRect.top) rect.top = displayRect.top;
    if(rect.left < displayRect.left) rect.left = displayRect.left;
    if(rect.right > displayRect.right) rect.right = displayRect.right;
    if(rect.bottom > displayRect.bottom) rect.bottom = displayRect.bottom;

    if(PointInRectangle(&displayRect,topLeft) || PointInRectangle(&displayRect,bottomRight))
        {
		myNewRects.push_back(rect);
		}// end if PointInRectangle....
	}
		
		return InsertObject(sprite,&rect);


	return false;
}


bool DrawableObjectHandler::InsertObject( DrawableObject* const  obj, RECT* rect /* = NULL*/)
{
	// insert depending on what it is
	uint32 thePlane = obj->GetPlane();

	myRenderObjects[thePlane].push_back(obj);
	myRenderMappings.insert(std::make_pair(obj,--(myRenderObjects[thePlane].end())));

	// don't deal with lines
	return true;
}


// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   None 			
// Returns:     None
//			
// ----------------------------------------------------------------------
DrawableObjectHandler::~DrawableObjectHandler()
{
}

// ----------------------------------------------------------------------
// Method:      Draw 
// Arguments:   completeRedraw - whether to draw the whole screen or
//								 just dirty rects.
// Returns:     None
//
// Description: myRenderObjects has been presorted into plane order and
//				non visible entities have been weeded out so just draw
//				the list.
//			
// ----------------------------------------------------------------------
void DrawableObjectHandler::Draw(bool completeRedraw)
{

	DrawableObjectListMap::iterator it;
	DrawableObjectList::iterator oit;
	
	RECT rect;

	RECT displayRect;


	myOwnerCamera->GetViewArea(displayRect);

	Box displayBox(displayRect);
	Box entityBox;
	

	DrawableObject* obj = NULL;

	for(it = myRenderObjects.begin(); it != myRenderObjects.end(); it++)
	for(oit = (*it).second.begin(); oit != (*it).second.end(); oit++)
		{
		obj = (*oit);

		// if remote cameras cannot be allowed to try to draw themselves
		// or any other camera
		if (obj->AreYouALine() && myOwnerCamera->AreYouARemoteCamera())
			continue;
		
		if (obj->AreYouACamera() && obj == myOwnerCamera)
			continue;

		if(obj->AreYouACamera() && myOwnerCamera->AreYouARemoteCamera())
			continue;

        obj->GetBound(rect);
		entityBox = rect;
        //weed out rects that are just not on screen

		if (!(amIInterestedInCameraShyObjects && obj->AreYouCameraShy()))
			if (entityBox.IntersectRect(entityBox,displayBox))
			if (!(amIInterestedInCameraShyObjects && obj->AreYouACamera()))
            {
				//gtb!temp!
				obj->SetScreenPosition(Position(
					rect.left - myWorldPosition.GetX()
					,
					rect.top - myWorldPosition.GetY()
				));

				// virtual function that eventually calls the display engine
				obj->Draw();
			}

		}

 	myOldDirtyTiles.clear();
	myOldDirtyTiles = myNewDirtyTiles;
	myNewDirtyTiles.clear();
}

void DrawableObjectHandler::SetInterestLevel(bool interested)
{
	amIInterestedInCameraShyObjects = interested;
}

// ----------------------------------------------------------------------
// Method:      Update 
// Arguments:   pos - new view position in world coords			
//
// Returns:     None
//
// Description: This updates the items in the renderList and the 
//				dirty rectangles and resets the view position
//						
// ----------------------------------------------------------------------
void DrawableObjectHandler::Update(Position pos)
{
	if(myShutDownFlag)
		return ;
	RECT rect;
	DrawableObjectListMap::iterator it;
	DrawableObjectList::iterator oit;
	RECT displayRect;

	myOwnerCamera->GetViewArea(displayRect);
	myNewRects.clear();

	std::map<IntegerPair,bool> ourBackgroundTilesMap;

	int top,bottom,left,right = 0;
	for(it = myRenderObjects.begin(); it != myRenderObjects.end(); it++)
	for(oit = (*it).second.begin(); oit != (*it).second.end(); oit++)
	{

		DrawableObject* sprite = (*oit);
		if (!ourAlreadyDoneSetCurrentBoundsThisTick || sprite->AreYouACamera())
			sprite->SetCurrentBound();
		sprite->GetBound(rect);

		if(IsRectOnScreen(rect,displayRect))
		{
			//clip rectangles that are not totally on screen here
			if(rect.top < displayRect.top) rect.top = displayRect.top;
			if(rect.left < displayRect.left) rect.left = displayRect.left;
			if(rect.right > displayRect.right) rect.right = displayRect.right;
			if(rect.bottom > displayRect.bottom) rect.bottom = displayRect.bottom;

			Position backgroundTopLeft = myOwnerCamera->GetBackgroundTopLeft(0);

			top = (rect.top - backgroundTopLeft.GetY())>>7;
			bottom = (rect.bottom - backgroundTopLeft.GetY())>>7;
			left = (rect.left-backgroundTopLeft.GetX())>>7;
			right = (rect.right-backgroundTopLeft.GetX())>>7;

			for(int i = left; i <= right; i++ )
			{
				for (int j = top; j <= bottom; j++)
				{
					IntegerPair tilepos(i,j);
					ourBackgroundTilesMap[tilepos] = true;
				}
			}
		}
	}
	// now add the fast rectangles

	RECT_ITERATOR fastRect;

	// add to the old rects since they are the current set needing updating
	for(fastRect = myFastRects.begin(); fastRect != myFastRects.end(); fastRect++)
	{
		myOldRects.push_back(*fastRect);
	}
	myFastRects.clear();
	myWorldPosition = pos;
	ourAlreadyDoneSetCurrentBoundsThisTick = true;
	std::map<IntegerPair,bool>::iterator mapIterator(ourBackgroundTilesMap.begin());
	for(;mapIterator != ourBackgroundTilesMap.end();mapIterator++)
		myNewDirtyTiles.push_back((*mapIterator).first);
}

void DrawableObjectHandler::ShutDown()
{
	myShutDownFlag = true;
}

// ----------------------------------------------------------------------
// Method:      AddDirtyRect 
// Arguments:   rect - new dirty rectangle for a fast drawn object			
//
// Returns:     None
//
// Description: 
//						
// ----------------------------------------------------------------------
void DrawableObjectHandler::AddDirtyRect(RECT rect)
{
	if(!myShutDownFlag)
	myFastRects.push_back(rect);
}




// ----------------------------------------------------------------------
// Method:      Remove
// Arguments:   newEntity - pointer to new entity to chop from the
//				list
// Returns:     true if the entity was found and removed false otherwise
// Description: This removes the given entityImage from the current update
//				list.
//			
// ----------------------------------------------------------------------
bool DrawableObjectHandler::Remove(DrawableObject* const chop)
{
	if(myShutDownFlag)
		return false;

	// check through the renderList for the offending entityImage and remove
	// it
	DrawableObjectList::iterator oit;

	std::map<DrawableObject*,std::list<DrawableObject*>::iterator>::iterator mit;

	mit = myRenderMappings.find(chop);
	if (mit == myRenderMappings.end())
		return false;
	oit = (*mit).second;

	myRenderObjects[(*oit)->GetPlane()].erase(oit);
	myRenderMappings.erase(mit);

	return true;
}



void DrawableObjectHandler::SetWorldPosition(Position pos)
 {
	myWorldPosition = pos;
 }

bool DrawableObjectHandler::Write(CreaturesArchive& ar) const
{
	ar << myWorldPosition;
	ar << myShutDownFlag;
	ar << amIInterestedInCameraShyObjects;
	return true;
}

bool DrawableObjectHandler::Read(CreaturesArchive& ar)
{
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		ar >> myWorldPosition;
		ar >> myShutDownFlag;
		ar >> amIInterestedInCameraShyObjects;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


