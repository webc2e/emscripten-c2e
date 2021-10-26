#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "RemoteCamera.h"

#ifdef C2D_DIRECT_DISPLAY_LIB

#else
	#include "CameraSprite.h"
	#include "../Display/DisplayEngine.h"
#endif // C2D_DIRECT_DISPLAY_LIB

#include "../Display/ErrorMessageHandler.h"
#include "../Agents/Agent.h"
#include "../App.h"
#include "../World.h"
#include "MainCamera.h"
#include "../Display/DisplayErrorConstants.h"

// serialize out as many members as you need to recreate yourself
CREATURES_IMPLEMENT_SERIAL( RemoteCamera )

RemoteCamera::RemoteCamera()
:myCameraWidth(0),
myCameraHeight(0)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
	myScreen->SetInterestLevel(true);
	myScreen->SetRemoteCameraFlag(true);
	#endif

}

RemoteCamera::~RemoteCamera()
{

}

RemoteCamera::RemoteCamera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground,
			uint32 topLeftXofBackground, // top left world co-ordinates 
			uint32 topLeftYofBackground, // of background as a meta room
			int32 plane,
			RECT& bound)
:Camera(viewx, // world x co-ordinate of view
					viewy, // world y co-ordinate of view
					width, // width of camera view
					height, // height of camera view
					defaultBackground,
					topLeftXofBackground, // top left world co-ordinates 
					topLeftYofBackground)
{

	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
		myScreen->SetRemoteCameraFlag(true);
		myScreen->SetInterestLevel(true);

		myScreen->SetPlane(plane);

		myScreen->SetCurrentBound(&bound);
	#endif

	myCameraHeight= bound.bottom - bound.top;
	myCameraWidth = bound.right - bound.left;

	myOriginalViewToPhysicalWidthRatio = float((float(bound.right - bound.left) )/float(width));
	myOriginalViewToPhysicalHeightRatio = float((float(bound.bottom - bound.top) )/float(height));


	bool ok = false;

	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
		ok = myScreen->SetUpDrawingSurface(myOriginalViewToPhysicalHeightRatio,
			 myOriginalViewToPhysicalWidthRatio);
	#endif

	if(!ok)
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)sidNoSecondaryCameraCreated,
										std::string("RemoteCamera::RemoteCamera"));


		throw(CameraException(string,__LINE__));
	}

	// set up a default meta room to look at
	int x, y, w, h;
	RECT metaRoomMBR;
	int metaRoomID=0;
	theApp.GetWorld().GetMap().GetMetaRoomIDForPoint( Vector2D( (float)viewx, (float)viewy ), metaRoomID );
	if( theApp.GetWorld().GetMap().GetMetaRoomLocation
		(metaRoomID, x, y, w, h) )
	{
		std::string background;
		theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID, background);
		
		if(!background.empty())
		{
			metaRoomMBR.left = x;
			metaRoomMBR.right = x+w-1;
			metaRoomMBR.top = y;
			metaRoomMBR.bottom = y+h-1;
	
			ChangeMetaRoom(background, metaRoomMBR, -1, -1, 0, 0);
		}
		Position pos(viewx, viewy);
		SetPosition(pos);
	}

}



void RemoteCamera::PhysicallyMoveToWorldPosition(int32 x, int32 y)
{
	RECT rect;
	rect.top = y;
	rect.bottom = rect.top + myCameraHeight;
	rect.left = x;
	rect.right = rect.left + myCameraWidth;

	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
		myScreen->SetCurrentBound(&rect);
	#endif
}

void RemoteCamera::ZoomBy(int32 pixels,int32 x, int32 y )
{

	if(x != -1 && y != -1)
		CentreOn(x,y);
	else
		GetViewCentre(x,y);

	// take the set amount of pixels of the veiw width and height
	// do it at both ends ie take from the view area and the view width/height


	// now make sure that whatever adjustments that were made are
	// symmetrical
	int testWidth = myViewWidth+pixels;
	int testHeight = myViewHeight + pixels;

#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
#else
	if( (testWidth > 0 && testWidth < DisplayEngine::theRenderer().GetSurfaceWidth()) &&
		( testHeight > 0 && testHeight < DisplayEngine::theRenderer().GetSurfaceHeight()))
	{
		myViewWidth+= pixels;
		myViewHeight+=pixels;
	}
#endif

	myOriginalViewToPhysicalWidthRatio = float((float(myCameraWidth) )/float(myViewWidth));
	myOriginalViewToPhysicalHeightRatio = float((float(myCameraHeight) )/float(myViewHeight));

	// find out if the background is smaller than the display width do something
	SetViewAndDisplayArea();

	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
		myScreen->SetUpDrawingSurface(myOriginalViewToPhysicalHeightRatio,
		 myOriginalViewToPhysicalWidthRatio);
	#endif

	CentreOn(x,y);
}

void RemoteCamera::Render()
{
	if(!Pan())
	{
		// if we are tracking an object then pan to set it in the centre
		TrackObject();
		{
		}	
	}

	DoTrackingChecks();
}

void RemoteCamera::Refresh()
{
	Update(true,true);
}



bool RemoteCamera::Write(CreaturesArchive &ar) const
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
		Camera::Write(ar);

		ar << myCameraWidth;
		ar << myCameraHeight;
		ar << myScreen->GetScreenBound();
		ar << myScreen->GetPlane();
		ar << myOriginalViewToPhysicalWidthRatio;
		ar << myOriginalViewToPhysicalHeightRatio;
	#endif
	return true;
}

bool RemoteCamera::Read(CreaturesArchive &ar)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else		
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		if(!Camera::Read(ar))
			return false;

		ar >> myCameraWidth;
		ar >> myCameraHeight;
		RECT bound;
		ar >> bound;
		myScreen->SetCurrentBound(&bound);

		uint32 plane;
		ar >> plane;	
		myScreen->SetPlane(plane);

		ar >> myOriginalViewToPhysicalWidthRatio;
		ar >> myOriginalViewToPhysicalHeightRatio;

	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// find out if the background is smaller than the display width do something
	SetViewAndDisplayArea();

	// success
	if(!myScreen->SetUpDrawingSurface( myOriginalViewToPhysicalWidthRatio,
	 myOriginalViewToPhysicalHeightRatio))
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)sidNoSecondaryCameraCreated,
										std::string("RemoteCamera::RemoteCamera"));


		throw(CameraException(string,__LINE__));
	}
#endif
	return true;
}

void RemoteCamera::Track(AgentHandle& agent, int xpercent, int ypercent,
											 int style, int transition)
{
	myTrackStyle = style;
	Camera::Track(agent,xpercent,ypercent,myTrackStyle,0);
}

bool RemoteCamera::TrackObject()
{
	if (myTrackObject.IsValid())
	{
		// if flexible style, don't track unless we were in the
		// rectangle last tick.
		if ((myTrackStyle == 1) && !myTrackInRectangleLastTick)
			return false;

		int offbyx, offbyy;
		if (!InTrackingRectangle(offbyx, offbyy))
		{
			int32 objx = FastFloatToInteger(myTrackObject.GetAgentReference().GetCentre().x);
			int32 objy = FastFloatToInteger(myTrackObject.GetAgentReference().GetCentre().y);

			int metaRoomID;
			bool ok = theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
				(Vector2D((float)objx, (float)objy),metaRoomID);
			if (!ok)
				return false;

			
			int oldmetaRoomID;
			theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
				(Vector2D((float)myWorldPosition.GetX(), (float)myWorldPosition.GetY()),oldmetaRoomID);

			if(!ok)
				return false;

			if (oldmetaRoomID != metaRoomID)
			{
				// if we're not hard tracking, and it's the camera that's
				// changed meta room, not us, then don't change back
				if ((myTrackStyle != 2) && (myTrackMetaRoomLastTick == metaRoomID))
					return false;
				// get the new background
				std::string background;

				int x, y, w, h;
				RECT metaRoomMBR;

				if( !theApp.GetWorld().GetMap().GetMetaRoomLocation(metaRoomID, x, y, w, h) )	
				{
					return false;
				}


				metaRoomMBR.left = x;
				metaRoomMBR.right = x+w-1;
				metaRoomMBR.top = y;
				metaRoomMBR.bottom = y+h-1;


			
				theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID, background);
				ChangeMetaRoom(background,metaRoomMBR,objx,objy,false,0);
			}
			else
			{
				MoveBy(-offbyx, -offbyy);
			}

			return true;
		}
	}
	return false;
}

int RemoteCamera::GetMetaRoom()
{
	int metaRoomID;
	theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(Vector2D((float)myWorldPosition.GetX(), (float)myWorldPosition.GetY()), metaRoomID);
	return metaRoomID;
}

void RemoteCamera::SetPlane(uint32 plane)
{
	#ifdef C2D_DIRECT_DISPLAY_LIB
		_ASSERT(false);
	#else
		myScreen->SetPlane(plane);
	#endif
}

