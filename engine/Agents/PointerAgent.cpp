// PointerAgent.cpp
//

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "PointerAgent.h"
#include "../Agents/Port.h"
#include "../Agents/InputPort.h"
#include "../Agents/OutputPort.h"
#include "../World.h"
#include "../App.h"
#include "../Creature/SkeletalCreature.h"
#include "../AgentManager.h"
#include "../InputManager.h"
#include "../Camera/MainCamera.h"	
#include "../Creature/SensoryFaculty.h"
#include "../Creature/LinguisticFaculty.h"
#include "../AgentDisplay/EntityImageClone.h"

// Needed for highlite

#include "../Display/Position.h"
#include "../mfchack.h"

CREATURES_IMPLEMENT_SERIAL( PointerAgent )

int32 PointerAgent::GetPlaneForCarriedAgent()
{
	_ASSERT(!myGarbaged);

	// default is one less than your plane
	int32 plane = -1;
	if(myEntityImage)
	{
		plane = myEntityImage->GetPlane() - 100;
	}
	return plane;
}


void PointerAgent::Tint(const uint16* tintTable, int part, bool oneImage)
{
	_ASSERT(!myGarbaged);

	// we could delete everytime or do a check to 
	// see whether we have a cloned entity already and reload?
	if(myEntityImage)
	{

		bool wasVisible = myEntityImage->IsVisible();

		// create a temporary cloned entity image
		theMainView.RemoveFloatingThing(mySelf);	
		
		int storedPose = myEntityImage->GetPose();

		if(CreateClonedEntityImage(oneImage))
		{
			
			myEntityImage->SetPose(oneImage ? 0 : storedPose);



			// if no tinttable given then we want to cancel a previous tint
			if(tintTable)
				myEntityImage->RecolourGallery(tintTable);

			theMainView.AddFloatingThing(mySelf);
			theMainView.KeepUpWithMouse(GetEntityImage());

			if(wasVisible)
				myEntityImage->Link(true);
			else
				myEntityImage->Unlink();

			DoSetCameraShyStatus();
		
		}
	}
}

bool PointerAgent::CreateClonedEntityImage(bool oneImage /*=  false*/)
{
	
	// the parameters are slightly different for the direct display engine.
	// as it needs the block width and height for the animation frames
	ClonedEntityImage* clone = NULL;

#ifdef C2D_DIRECT_DISPLAY_LIB

		clone =	new  ClonedEntityImage(myEntityImage->GetFilePath(),
				myEntityImage->GetPlane(),
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				myEntityImage->GetBlockWidth(),
				myEntityImage->GetGalleryBitmapCount(),
				myEntityImage->GetBlockHeight() );

		if(clone)
		{
		// must delete the entityimage first in this case
		delete myEntityImage;
		myEntityImage = NULL; // set to null so no access violation later in this method
		clone->CreateAnimStrip(myEntityImage->GetBlockWidth(), myEntityImage->GetBlockHeight());
		}
#else

		clone =	new  ClonedEntityImage(myEntityImage->GetFilePath(),
				myEntityImage->GetPlane(),
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				myEntityImage->GetAbsoluteBaseImage(),
				oneImage ? 1 : myEntityImage->GetGalleryBitmapCount(),
				myEntityImage->GetAbsoluteBaseImage() );

#endif

		if(clone)
		{
			delete myEntityImage;
			myEntityImage = clone;
			myEntityImageIsCloned = true;

			return true;
		}

		return false;
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void PointerAgent::DrawOutlineAroundImage(int red, int green, int blue)
{
	
		if(CreateClonedEntityImage())
		{
			myEntityImage->DrawOutlineAroundImage(red,green,blue);
		}
}
#endif // C2D_DIRECT_DISPLAY_LIB


void PointerAgent::UnClone(int part /*= 0*/)
{
	if(myEntityImageIsCloned)
	{
		// create a temporary cloned entity image
		theMainView.RemoveFloatingThing(mySelf);	

		// create a normal entity image to be swapped in
		EntityImage* ent =	new  EntityImage(myEntityImage->GetFilePath(),
						myEntityImage->GetGalleryBitmapCount(),
						myEntityImage->GetPlane(),
						Map::FastFloatToInteger(myPositionVector.x),
						Map::FastFloatToInteger(myPositionVector.y),
						myEntityImage->GetAbsoluteBaseImage(),
						0 );

#ifdef C2D_DIRECT_DISPLAY_LIB
		ent->CreateAnimStrip(myEntityImage->GetBlockWidth(), 
							myEntityImage->GetBlockHeight());
#endif //  C2D_DIRECT_DISPLAY_LIB

			ent->SetPose(myEntityImage->GetPose());

			delete myEntityImage;
			myEntityImage = ent;

			theMainView.AddFloatingThing(mySelf);
			theMainView.KeepUpWithMouse(GetEntityImage());

			myEntityImage->Link(true);

			DoSetCameraShyStatus();

	}

}

PointerAgent::PointerAgent(	int family, int genus, int species, uint32 id,
						   FilePath const& gallery,
						   int numimages,
						int baseimage)		
    : base(family, genus, species, id, gallery, numimages, baseimage,
					EntityImage::PLANE_NEAREST_THE_FRONT,                     // plane
				   true),
				   myCanCarryAgentsFromMetaRoomToMetaRoom(false),
				   myWiredToMouse(true)
{
	if (!myFailedConstructionException.empty())
		return;

	try
	{
		//  0 - normal
		//  1 - right button does what left click would do
		//	2 - left button does what right click would do
		myMouseButtonAction = DoMouseClicksAsNormal; // default is normal 

		myIsHoldingHandsWithSkeletalCreature = false;
		myHandHeldSkeletalCreature = NULLHANDLE; // you are not holding hands
		myMarchingAntCount = 8;

		myMovementStatus = MOUSEDRIVEN;           // tell update fn to move with mouse
		myInputPort = NULL;
		myOutputPort = NULL;
		myPort = NULL;
		myConnecting = false;
		myConnectingAgent = NULLHANDLE;
		myScanInputEvents = true;
		myLastPointerActionDispatched = 0;
		myAgentType = AgentHandle::agentNormal | AgentHandle::agentSimple | AgentHandle::agentPointer;

		theMainView.AddFloatingThing(mySelf);

		myName = SensoryFaculty::GetCategoryName(SensoryFaculty::GetCategoryIdOfAgent(GetAsAgentHandle()));

	}	
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0009: Unknown exception caught in pointer constructor";
	}
}


// serialisation constructor
PointerAgent::PointerAgent()
{
	myMouseButtonAction = DoMouseClicksAsNormal; // default is normal 

	myIsHoldingHandsWithSkeletalCreature = false;
	myHandHeldSkeletalCreature = NULLHANDLE; // you are not holding hands

	myMovementStatus = MOUSEDRIVEN;           // tell update fn to move with mouse
	myInputPort = NULL;
	myOutputPort = NULL;
	myPort = NULL;
	myConnecting = false;
	myConnectingAgent = NULLHANDLE;
	myScanInputEvents = true;
	myCanCarryAgentsFromMetaRoomToMetaRoom = false;
	myMarchingAntCount = 8;
	myLastPointerActionDispatched = 0;
	theMainView.AddFloatingThing(mySelf);
}

// Destructor
PointerAgent::~PointerAgent()
{
}



void PointerAgent::Trash()
{
	_ASSERT(!myGarbaged);

	theMainView.RemoveFloatingThing(mySelf);
	myHandHeldSkeletalCreature = NULLHANDLE;

	// This must be the last line in the function
	base::Trash();
}

void PointerAgent::HandlePickup(Message* Msg)
{
	// you can't pickup the pointer
	_ASSERT(!myGarbaged);
}

void PointerAgent::HandleDrop(Message* Msg)
{
	// you can't drop the pointer!
	_ASSERT(!myGarbaged);
}




void PointerAgent::StartHoldingHandsWithSkeletalCreature(AgentHandle& skeletalCreature)
{
	_ASSERT(skeletalCreature.IsValid());

	if (myIsHoldingHandsWithSkeletalCreature)
	{
		if (myHandHeldSkeletalCreature.IsValid())
			myHandHeldSkeletalCreature.GetSkeletalCreatureReference().StopHoldingHandsWithThePointer
				(mySelf,INTEGERZERO, INTEGERZERO);
	}
	myIsHoldingHandsWithSkeletalCreature = true;
	myHandHeldSkeletalCreature = skeletalCreature;
	ShowPose(1);
}


void PointerAgent::StopHoldingHandsWithSkeletalCreature()
{
	if (!myIsHoldingHandsWithSkeletalCreature)
		return;

	myIsHoldingHandsWithSkeletalCreature = false;
	myHandHeldSkeletalCreature = NULLHANDLE;
	ShowPose(0);
	int x = myEntityImage->GetWorldX();
	int y = myEntityImage->GetWorldY();
	theMainView.WorldToScreen(x, y);
	theApp.GetInputManager().SetMousePosition(x, y);
}


void PointerAgent::HoldHandsWithSkeletalCreature()
{
	SkeletalCreature& skeletalCreature = myHandHeldSkeletalCreature.GetSkeletalCreatureReference();

	int hand;
	int direction = skeletalCreature.GetDirection();

	switch (direction)
	{
	case NORTH: 
	case EAST:
		hand = BODY_LIMB_RIGHT_ARM;
		break;
	default:
		hand = BODY_LIMB_LEFT_ARM;
		break;
	}

	Vector2D handPosition;
	skeletalCreature.GetExtremePoint(hand, handPosition);

	Vector2D offset(-5.0f, -10.0f);
	Vector2D entityPosition(handPosition + offset);
	base::MoveTo(entityPosition.x, entityPosition.y);
}


void PointerAgent::Update()
{
	_ASSERT(!myGarbaged);

	// vel added by gtb for presence stuff:
	myInvalidPosition = false;
	if (myWiredToMouse)
	{
		myVelocityVector.x = theApp.GetInputManager().GetMouseVX();
		myVelocityVector.y = theApp.GetInputManager().GetMouseVY();
	}
	else
	{
		myVelocityVector.x = 0;
		myVelocityVector.y = 0;
	}
	if (myCarriedAgent!=NULLHANDLE)
	{
		myCarriedAgent.GetAgentReference().SetVelocity(myVelocityVector);
	}

	myWiredToMouse = (theApp.GetEameVar("engine_decouple_mouse").GetInteger() == 0);

	CAOSVar p1, p2;

	// Call base behaviour
	base::Update();

	_ASSERT(!myGarbaged);

	// Get the location of the physical mouse
	int x = theApp.GetInputManager().GetMouseX();
	int y = theApp.GetInputManager().GetMouseY();
	// Convert to world coords
	theMainView.ScreenToWorld(x, y);

	if (myIsHoldingHandsWithSkeletalCreature)
	{
		if (myHandHeldSkeletalCreature.IsInvalid())
		{
			// My little friend has died, stop holding hands
			StopHoldingHandsWithSkeletalCreature();
			// Simply move the pointer entity to where the physical mouse is
			if (myWiredToMouse)
				base::MoveTo(x,y);
		}
		else
		{
			HoldHandsWithSkeletalCreature();
		}
	}
	else
	{
		// Simply move the pointer entity to where the physical mouse is
		if (myWiredToMouse)
			base::MoveTo(x, y);
	}

	if (myConnecting)
	{
		if (myConnectingAgent.IsInvalid())
		{
			myConnecting = false;
			myPort = NULL;
			myInputPort = NULL;
			myOutputPort = NULL;
			myResetLines = true;
			ExecuteScriptForEvent(SCRIPTPOINTERPORTCANCEL, mySelf, 
					INTEGERZERO, INTEGERZERO);
			return;
		}
	}

	if (myConnecting) 
	{
		Vector2D portPosition(myPort->GetWorldPosition());
		Vector2D mousePosition(GetHotSpot());
		int r, g, b;
		float dist = portPosition.SquareDistanceTo(mousePosition);
		float snapDist = theApp.GetMaximumDistanceBeforePortLineSnaps()+5.0f;
		float warnDist = theApp.GetMaximumDistanceBeforePortLineWarns();
		if (dist > snapDist*snapDist)
		{
			// Return to non-connecting mode
			myConnectingAgent = NULLHANDLE;
			myConnecting = false;
			myPort = NULL;
			myInputPort = NULL;
			myOutputPort = NULL;
			myResetLines = true;
			ExecuteScriptForEvent(SCRIPTPOINTERPORTCANCEL, mySelf, 
					INTEGERZERO, INTEGERZERO);
			return;
		}

		if (dist > warnDist*warnDist)
		{
			r = 255;
			g = 0;
			b = 0;
		}
		else
		{
			r = 255;
			g = 255;
			b = 255;
		}

		DrawLine(Map::FastFloatToInteger(mousePosition.x), 
			Map::FastFloatToInteger(mousePosition.y), 
			Map::FastFloatToInteger(portPosition.x), 
			Map::FastFloatToInteger(portPosition.y), 
			r,g,b, 4, 4,myMarchingAntCount);
		
		if (myPort == myInputPort)
			myMarchingAntCount = (myMarchingAntCount - 1) % 8;
		else
		{
			myMarchingAntCount = (myMarchingAntCount + 1) % 8;
		}
		myResetLines = true;
	}

	AgentHandle a;
	(myCarriedAgent.IsValid())? a = myCarriedAgent: a = FindSkeletalCreature();

	Classifier c = GetClassifier();
	c.SetEvent(SCRIPTPOINTERACTIONDISPATCH);
	if( a.IsSkeletalCreature() )
	{
		int msgid = a.GetAgentReference().ClickAction
			(Map::FastFloatToInteger(myPositionVector.x),
			Map::FastFloatToInteger(myPositionVector.y));

		if( msgid != -1 )
		{
			if (msgid == ACTIVATE1)
			{
				p1.SetInteger(2);
				if (myLastPointerActionDispatched != 2)
					ExecuteScriptForClassifier(c, a, p1, p2);	
				myLastPointerActionDispatched = 2;
			}
			else if (msgid == DEACTIVATE)
			{
				p1.SetInteger(1);
				if (myLastPointerActionDispatched != 1)
					ExecuteScriptForClassifier(c, a, p1, p2);	
				myLastPointerActionDispatched = 1;
			}
			else if (myLastPointerActionDispatched)
			{
				p1.SetInteger(0);
				ExecuteScriptForClassifier(c,a,p1,p2);
				myLastPointerActionDispatched = 0;
			}
		}
		else if (myLastPointerActionDispatched)
		{
			p1.SetInteger(0);
			ExecuteScriptForClassifier(c,a,p1,p2);
			myLastPointerActionDispatched = 0;
		}
	}
	else
	{
		if (myLastPointerActionDispatched)
		{
			p1.SetInteger(0);
			ExecuteScriptForClassifier(c,a,p1,p2);
			myLastPointerActionDispatched = 0;
		}
	}

	if (myScanInputEvents)
		ScanInputEvents();

	// Dan's demo of what background tiles are affected by the pointer agent...
	// Uncomment this to see what a waste of time it would be to re-implement fast entities

	/*
	Position backgroundTopLeft = theMainView.GetBackgroundTopLeft();
	RECT rect;
	myEntityImage->GetBound(rect);
	int top,left,bottom,right;

	top = (rect.top - backgroundTopLeft.GetY())>>7;
	bottom = (rect.bottom - backgroundTopLeft.GetY())>>7;
	left = (rect.left-backgroundTopLeft.GetX())>>7;
	right = (rect.right-backgroundTopLeft.GetX())>>7;

	// Right then, top is the topleft tile.
	// OutputFormattedDebugString("The pointer is over tiles: (%d,%d) to (%d,%d)\n",left,top,right,bottom);

	top = top * 128;
	bottom = (bottom*128) + 128;
	left = left * 128;
	right = (right * 128) + 128;

	// Now outline the tiles in question :)
	myResetLines = true;
	DrawLine(left,top,right,top,255,0,255);
	DrawLine(left,top,left,bottom,255,0,255);
	DrawLine(right,top,right,bottom,255,0,255);
	DrawLine(right,bottom,left,bottom,255,0,255);
	*/

}

void PointerAgent::ScanInputEvents()
{
	_ASSERT(!myGarbaged);
	CAOSVar p1, p2;
	

	InputManager& im = theApp.GetInputManager();
	int i;
	const InputEvent* ev;
	int msgid;	
	InputPort *ip;
	OutputPort *op;
	Port* port;
	bool inputnotoutput;
	Vector2D hotspot;
	hotspot = GetHotSpot();

	// Scan the pending inputevents:
	for( i=0; i<im.GetEventCount(); ++i )
	{
		ev = &im.GetEvent(i);
		if( ev->EventCode == InputEvent::eventMouseDown &&
			ev->MouseButtonData.button == InputEvent::mbRight &&
			!TreatBothButtonsAsLeftClick()||
			( ev->EventCode == InputEvent::eventMouseDown &&
			ev->MouseButtonData.button == InputEvent::mbLeft && TreatBothButtonsAsRightClick() )
			)
		{	 
			// Start RMB
			if (myIsHoldingHandsWithSkeletalCreature)
			{
				if (myHandHeldSkeletalCreature.IsValid())
				{
					// Write the normal message to the clicked-on agent
					p1.SetFloat(myPositionVector.x - myHandHeldSkeletalCreature.GetAgentReference().GetPosition().x);
					p2.SetFloat(myPositionVector.y - myHandHeldSkeletalCreature.GetAgentReference().GetPosition().y);
					theApp.GetWorld().WriteMessage(mySelf,myHandHeldSkeletalCreature,STOP_HOLD_HANDS,p1,p2,0);
					return;
				}
			}

			if (myConnecting) 
			{
				// Cancel the connect operation
				myInputPort = NULL;
				myOutputPort = NULL;
				myPort = NULL;
				myConnecting = false;
				myConnectingAgent = NULLHANDLE;
				myResetLines = true;
				ExecuteScriptForEvent(SCRIPTPOINTERPORTCANCEL, mySelf, 
					INTEGERZERO, INTEGERZERO);
				return;
			}

			if (myCarriedAgent.IsValid())
			{
				theApp.GetWorld().WriteMessage(mySelf,myCarriedAgent,DROP,
					INTEGERZERO, INTEGERZERO, 0);
				return;
			}

			AgentHandle a = Find();
			if (a.IsValid()) {
				port = a.GetAgentReference().IsPointOverPort(hotspot, inputnotoutput);
				if (port) {
					if (inputnotoutput) {
						ip = (InputPort*)port;
						// input port being fed already?
						op = ip->GetConnectedPort(); 
						if (op != NULL) {
							// disconnect
							ip->DisconnectFromOutputPort();
							ip->GetOwner().GetAgentReference().ResetLines();
							ExecuteScriptForEvent(SCRIPTPOINTERPORTDISCONNECT, mySelf,
								INTEGERZERO, INTEGERZERO);
							return;
						}
					}
				}
			}

			// Are we allowed to pick this agent up?
			// (note: a mouseable agent may be behind some scenery which would have been
			//  found by Find() above)
			a = IsTouching(attrMouseable, attrMouseable);
			if (a.IsValid())
			{
				theApp.GetWorld().WriteMessage(mySelf,a,PICKUP,
					INTEGERZERO, INTEGERZERO, 0);
			}	
			// End RMB
		}


		if( ev->EventCode == InputEvent::eventMouseDown &&
			ev->MouseButtonData.button == InputEvent::mbLeft && !TreatBothButtonsAsRightClick()||
			( ev->EventCode == InputEvent::eventMouseDown &&
			ev->MouseButtonData.button == InputEvent::mbRight && TreatBothButtonsAsLeftClick() )
			)
		{
			// Start LMB
			if (myIsHoldingHandsWithSkeletalCreature)
				// Block LMB if currently holding hands 
				return;

			AgentHandle a;

			if (myCarriedAgent.IsValid())
			{
#ifdef C2D_DIRECT_DISPLAY_LIB
				a = myCarriedAgent;
#else
				// Block LMB if currently carrying
				return;
#endif
			}
	
			bool agentFound = false;
			if(a == NULLHANDLE)
				a = Find();

			if (a.IsValid()) 
			{
				// Does this agent have any ports?
				port = a.GetAgentReference().IsPointOverPort(hotspot, inputnotoutput);
				if (port) 
				{
					// Clicked on an I/O port
					if (inputnotoutput) 
					{
						// Clicked on an input port
						if (myInputPort != NULL) 
						{
							// Already got an input port, error
							ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
								INTEGERZERO, INTEGERZERO);	
							return;
						}
						if (myOutputPort) 
						{
							if (port->GetOwner() == myOutputPort->GetOwner()) 
							{
								// The two ports belong to same agent, error
								ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
									INTEGERZERO, INTEGERZERO);
								return;
							}
						}

						ip = (InputPort*)port;
						op = ip->GetConnectedPort(); 
						if (op != NULL) {
							// Input port is already being fed
							if (myConnecting) 
							{
								// Can't connect
								ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
									INTEGERZERO, INTEGERZERO);
								return;
							}
							// Edit the link
							ip->DisconnectFromOutputPort();
							ip->GetOwner().GetAgentReference().ResetLines();
							myInputPort = NULL;
							myOutputPort = op;
							myPort = (Port*)op;
							myConnecting = true;
							myConnectingAgent = a;
							ExecuteScriptForEvent(SCRIPTPOINTERPORTDISCONNECT, mySelf,
								INTEGERZERO, INTEGERZERO);
							ExecuteScriptForEvent(SCRIPTPOINTERPORTSELECT, mySelf,
								INTEGERZERO, INTEGERZERO);
							return;
						}

						if (myOutputPort)
						{
							// Already got an output port, and we've clicked 
							// on an input port. Check the connection.
							bool dummy;
							bool valid = ip->GetOwner().GetAgentReference().IsConnectionValid
								(ip, myOutputPort, dummy);
							if (valid)
							{
								ip->ConnectToOutputPort(myOutputPort);
								myInputPort = NULL;
								myOutputPort = NULL;
								myPort = NULL;
								myConnecting = false;
								myConnectingAgent = NULLHANDLE;
								myResetLines = true;
								ExecuteScriptForEvent(SCRIPTPOINTERPORTCONNECT, mySelf,
									INTEGERZERO, INTEGERZERO); 
								return;
							}
							else
							{
								ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
									INTEGERZERO, INTEGERZERO);
								return;
							}
						}
						else 
						{
							myInputPort = ip;
							myPort = port;
							myConnecting = true;
							myConnectingAgent = a;
							ExecuteScriptForEvent(SCRIPTPOINTERPORTSELECT, mySelf,
									INTEGERZERO, INTEGERZERO); 
							return;
						}
					}
					else
					{
						// Clicked on an output port
						if (myOutputPort != NULL) 
						{
							// Already got an output port, error
							ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
								INTEGERZERO, INTEGERZERO);
							return;
						}
						if (myInputPort) 
						{
							if (port->GetOwner() == myInputPort->GetOwner()) 
							{
								// The two ports belong to same agent, error
								ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf, INTEGERZERO, INTEGERZERO);
								return;
							}
							// Already got an input port, and we've clicked 
							// on an output port. Check the connection.
							bool dummy;
							bool valid = myInputPort->GetOwner().GetAgentReference().IsConnectionValid
								(myInputPort, port, dummy);
							if (valid)
							{
								myInputPort->ConnectToOutputPort((OutputPort*)port);
								myInputPort = NULL;
								myOutputPort = NULL;
								myPort = NULL;
								myConnecting = false;
								myConnectingAgent = NULLHANDLE;
								myResetLines = true;
								ExecuteScriptForEvent(SCRIPTPOINTERPORTCONNECT, mySelf,
									INTEGERZERO, INTEGERZERO); 
								return;
							}
							else
							{
								ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
									INTEGERZERO, INTEGERZERO);
								return;
							}
						}
						else 
						{
							myOutputPort = (OutputPort*)port;
							myPort = port;
							myConnecting = true;
							myConnectingAgent = a;	
							ExecuteScriptForEvent(SCRIPTPOINTERPORTSELECT, mySelf,
									INTEGERZERO, INTEGERZERO); 
							return;
						}
					}
				}
				else 
				{
					// Didn't click on a port
					if (myConnecting) 
					{
						// Clicked on a non-port section of an agent
						ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
							INTEGERZERO, INTEGERZERO);
						return;
					}

					agentFound = true;
					theApp.GetWorld().WriteMessage( mySelf, a, SCRIPTUIMOUSEDOWN,
						INTEGERZERO, INTEGERZERO, 0);
				}
			}
		
			if (myConnecting) 
			{
				// Clicked on empty space while connecting
				ExecuteScriptForEvent(SCRIPTPOINTERPORTERROR, mySelf,
					INTEGERZERO, INTEGERZERO);
				return;
			}	


#ifdef C2D_DIRECT_DISPLAY_LIB
			
	if (myCarriedAgent.IsValid())
	{
		a = myCarriedAgent;
	}
	else
	{
		a = IsTouching(0, 0);
	}
#else
			a = IsTouching(0, 0);
#endif

		
			if (a.IsValid() &&
				((a.GetAgentReference().GetAttributes() & attrActivateable) != attrActivateable) )
			{
				a = NULLHANDLE;
			}
		
			if( a.IsValid() )
			{
				agentFound = true;

				msgid = a.GetAgentReference().ClickAction
					(Map::FastFloatToInteger(myPositionVector.x),
					Map::FastFloatToInteger(myPositionVector.y));

				if( msgid != -1 )
				{
					Classifier c = a.GetAgentReference().GetClassifier();
					// Write pointer messages to the clicked-on agent
					if (msgid == ACTIVATE1) {
						c.SetEvent(SCRIPTPOINTERACT1);
						ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);	
					}
					else if (msgid == ACTIVATE2) {
						c.SetEvent(SCRIPTPOINTERACT2);
						ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);		
					}
					else if (msgid == DEACTIVATE) {
						c.SetEvent(SCRIPTPOINTERDEAC);
						ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);	
					}
					else if (msgid == DROP) {
						c.SetEvent(SCRIPTPOINTERDROP);
						ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);	
					}
					else if (msgid == PICKUP) {
						c.SetEvent(SCRIPTPOINTERPICKUP);
   						ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);	
					}
					
					// Write the normal message to the clicked-on agent
					p1.SetFloat(myPositionVector.x - a.GetAgentReference().GetPosition().x);
					p2.SetFloat(myPositionVector.y - a.GetAgentReference().GetPosition().y);
					// If this is a skeletalCreature, then don't send the message. (pointer scripts send them)
					if (!a.IsSkeletalCreature())
						theApp.GetWorld().WriteMessage(mySelf,a,msgid,p1,p2,0);
					else if (msgid != ACTIVATE1 && msgid != DEACTIVATE)
						theApp.GetWorld().WriteMessage(mySelf,a,msgid,p1,p2,0);
				}
			}
			
			if (agentFound == false)
			{
				// Clicked on the background
				ExecuteScriptForEvent(SCRIPTPOINTERCLICKEDBACKGROUND, mySelf,
					INTEGERZERO, INTEGERZERO); 
			}
			// End LMB
		}
	}
}


AgentHandle PointerAgent::Find()
{
	_ASSERT(!myGarbaged);
	return theAgentManager.WhatAmIOver(mySelf, GetHotSpot());
}

AgentHandle PointerAgent::FindSkeletalCreature()
{
	_ASSERT(!myGarbaged);
	AgentHandle c = theAgentManager.WhatCreatureAmIOver(mySelf,GetHotSpot());
	return c.IsSkeletalCreature() ? c : NULLHANDLE;
}




// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool PointerAgent::Write(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);

	base::Write( archive );
	archive << myHotSpot <<
		myConnecting <<
		myInputPort << myOutputPort << myPort << myConnectingAgent <<
		myIsHoldingHandsWithSkeletalCreature << 
		myHandHeldSkeletalCreature <<
		myScanInputEvents << myMouseButtonAction << myLastPointerActionDispatched
		<< myName;

	return true;
}


// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool PointerAgent::Read(CreaturesArchive &archive)
{
	_ASSERT(!myGarbaged);

	int32 version = archive.GetFileVersion();
	
	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive >> myHotSpot >>
			myConnecting >>
			myInputPort >> myOutputPort >> myPort >> myConnectingAgent >>
			myIsHoldingHandsWithSkeletalCreature >> 
			myHandHeldSkeletalCreature >>
			myScanInputEvents >> myMouseButtonAction >> myLastPointerActionDispatched
			>> myName;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	// success!
	theMainView.KeepUpWithMouse(GetEntityImage());
	
	return true;
}

void PointerAgent::CameraPositionNotify()
{
	_ASSERT(!myGarbaged);

	int x, y;
	x = theApp.GetInputManager().GetMouseX();
	y = theApp.GetInputManager().GetMouseY();
	theMainView.ScreenToWorld(x, y);

	// when holding hands don't double update
	if(myIsHoldingHandsWithSkeletalCreature && (myHandHeldSkeletalCreature.IsValid()))
	{
	}
	else
	{
		if (myWiredToMouse)
			base::MoveTo( x,y);
	}

	


	// update position of agent we are carrying after camera has moved
	if (myCarriedAgent.IsValid())
	{
		myCarriedAgent.GetAgentReference().HandleMovementWhenCarried();
	}
}







// 0 - normal
// 1 - right button does what left button does
// 2 - left button does what right button does
void PointerAgent::DefineMouseClicks(int definition)
{
	_ASSERT(!myGarbaged);

	myMouseButtonAction = definition;
}

void PointerAgent::SetScanInputEvents(bool bValue)
{
	_ASSERT(!myGarbaged);

	myScanInputEvents = bValue;
}

bool PointerAgent::GetScanInputEvents()
{
	_ASSERT(!myGarbaged);
	return myScanInputEvents;
}


void PointerAgent::DecideIfCarriedAgentShouldBeDropped()
{
	_ASSERT(!myGarbaged);

	if(!myCanCarryAgentsFromMetaRoomToMetaRoom)
	{
		if (myCarriedAgent.IsValid())
			myCarriedAgent.GetAgentReference().TryToBeDropped
				(mySelf, INTEGERZERO, INTEGERZERO, true);
	}
}




void PointerAgent::SetName(std::string name)
{
	CreatureCollection &creatureCollection = theAgentManager.GetCreatureCollection();
	int noCreatures = creatureCollection.size();
	int pointerCat = SensoryFaculty::GetCategoryIdOfAgent(GetAsAgentHandle());

	// tell all creatures the new pointer name
	for(int c = 0; c != noCreatures; c++)
	{
		LinguisticFaculty &l = *creatureCollection[c].GetCreatureReference().Linguistic();
		if(l.GetPlatonicWord(LinguisticFaculty::NOUN, pointerCat) == myName)
		{
			// if have learnt name already alter it else may not have learnt to speak
			l.SetWord(LinguisticFaculty::NOUN, pointerCat, name, true);
		}
	}

	myName = name;
}

void PointerAgent::MoveTo(float x, float y)
{
	base::MoveTo(x, y);

	if (!myWiredToMouse)
		return;

	int logicalX = (int)x;
	int logicalY = (int)y;
	theMainView.WorldToScreen(logicalX, logicalY);

	int physicalX = theApp.GetInputManager().GetMouseX();
	int physicalY = theApp.GetInputManager().GetMouseY();
	if (logicalX != physicalX || logicalY != physicalY)
		theApp.GetInputManager().SetMousePosition(logicalX, logicalY);
	
}

