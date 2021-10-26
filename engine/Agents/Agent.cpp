// Agent.cpp
//
//////////////////////////////////////////////
// Agent class: base class for all
// agent classes.
//////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Agent.h"
#include "InputPort.h"
#include "OutputPort.h"
#include "Port.h"
#include "../Sound/Soundlib.h"
#include "../Caos/CAOSMachine.h"
#include "../World.h"
#include "../App.h"
#include "../Camera/MainCamera.h"	// for theMainView
#include "../Message.h"
#include "PointerAgent.h"
#include "Agent.h"
#include <string>
#include "../AgentManager.h"
#include "../Map/Map.h"
#include "../Maths.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Caos/DebugInfo.h"
#include <sstream>
#include "../Caos/RuntimeErrorDialog.h"
#include "Vehicle.h"
#include <algorithm>
#include "../Caos/DisplayHandlers.h"
#include "../C2eServices.h"

#include "../Display/Window.h" // SetGameTicking
#include "../AgentDisplay/EntityImage.h"



#include "../Presence.h"

Presence& Agent::GetPresence()
{
	return *myPresence;
}




CREATURES_IMPLEMENT_SERIAL( Agent )






void Agent::SetShowCore(bool b)
{
	myShowCoreFlag = b;
}
bool Agent::GetShowCore()
{
	return myShowCoreFlag;
}



bool Agent::TestAlteredBoundingBox(const Box& b)
{
	Box oldOne = myAlteredBoundingBox;

	SetAlteredBoundingBox(b);
	bool result = TestCurrentLocation(true);
	SetAlteredBoundingBox(oldOne);

	return result;
}
// added by gtb for jon:
const Box& Agent::GetAlteredBoundingBox()
{
	return myAlteredBoundingBox;
}
void Agent::SetAlteredBoundingBox(const Box& b)
{
	myAlteredBoundingBox = b;

	myCoreWidth = myAlteredBoundingBox.right - myAlteredBoundingBox.left;
	myCoreHeight= myAlteredBoundingBox.bottom - myAlteredBoundingBox.top;

	myCorePositionVector.x = myPositionVector.x+myAlteredBoundingBox.left;
	myCorePositionVector.y = myPositionVector.y+myAlteredBoundingBox.top;
}
float Agent::GetCoreWidth()
{
	return myCoreWidth;
}
float Agent::GetCoreHeight()
{
	return myCoreHeight;
}

const Vector2D& Agent::GetCorePosition()
{
	return myCorePositionVector;
}

void Agent::SetWidth(float f)
{
	myCurrentWidth = f;

	// if bounding box not set:
	if (myAlteredBoundingBox.right==0.0f && myAlteredBoundingBox.left==0.0f &&
		myAlteredBoundingBox.bottom==0.0f && myAlteredBoundingBox.top==0.0f)
	{
		myCoreWidth = myCurrentWidth;
	}
}
void Agent::SetHeight(float f)
{
	myCurrentHeight = f;

	// if bounding box not set:
	if (myAlteredBoundingBox.right==0.0f && myAlteredBoundingBox.left==0.0f &&
		myAlteredBoundingBox.bottom==0.0f && myAlteredBoundingBox.top==0.0f)
	{
		myCoreHeight = myCurrentHeight;
	}
}

Vector2D Agent::GetCorePosOf(const Vector2D& v)
{
	return Vector2D(v.x+myAlteredBoundingBox.left, v.y+myAlteredBoundingBox.top);
}
Vector2D Agent::GetPosFromCore(const Vector2D& v)
{
	return Vector2D(v.x-myAlteredBoundingBox.left, v.y-myAlteredBoundingBox.top);
}



bool Agent::VmIsRunning()
{
	return GetVirtualMachine().IsRunning();
}


void Agent::CallScript(int eventNo, CAOSVar const& p1, CAOSVar const& p2)
{
	CAOSMachine* oldVM = myVirtualMachines.back();

	bool lastVMsInstFlag = myVirtualMachines.back()->GetInstFlag();
	// if we're pushing the script from a CALL
	if (mySpareVirtualMachines.empty())
	{
		if (myVirtualMachines.size()>=MAX_VMS_PER_AGENT)
			//gtb! already the max no of VMs running on the agent
			//so we bail out 
			throw BasicException("CallScript recursion too deep");

		mySpareVirtualMachines.push_back(new CAOSMachine());
	}

	myVirtualMachines.push_back(mySpareVirtualMachines.back());
	mySpareVirtualMachines.pop_back();

	// make sure the new VM is in the same INST state as the last one:
	myVirtualMachines.back()->SetInstFlag(lastVMsInstFlag);

	ExecuteScriptForEvent(eventNo, mySelf, p1, p2);

	// if the CALLed script hasn't finished yet
	// make sure we don't execute any more on this VM:
	if (myVirtualMachines.back()!=oldVM)
		oldVM->ClearQuanta();

}


void Agent::PopCurrentVmIfFinished()
{
//	bool lastVMsInstFlag = myVirtualMachines.back()->GetInstFlag();

	if (!GetVirtualMachine().IsRunning() && myVirtualMachines.size()>1)
	{
		mySpareVirtualMachines.push_back(myVirtualMachines.back());
		myVirtualMachines.pop_back();
	}

	// make sure the new VM is in the same INST state as the last one:
//	myVirtualMachines.back()->SetInstFlag(lastVMsInstFlag);
}


void Agent::StopAllScripts()
{
	// Reduce no of VMs to one:
	while (myVirtualMachines.size()>1)
	{
		myVirtualMachines.back()->StopScriptExecuting();
		PopCurrentVmIfFinished();
	}
	// Stop that one's scripts:
	myVirtualMachines.back()->StopScriptExecuting();
}






// This function returns the base vehicle of an object, e.g. the root vehicle of a creature
// on a hoverboard (an open air cabin) is NULLHANDLE, i.e. the world, whereas the root vehicle
// of a creature in a lift is just the lift.
AgentHandle Agent::GetRootVehicle()
{
	AgentHandle v = mySelf;
	do {
		// Get the vehicle/carrier of the current object:
		v = v.GetAgentReference().GetCarrier();

		// Stop if the object has no carrier, i.e. it's just in the main world:
		if (v.IsInvalid())
			return NULLHANDLE;

		// Also stop if we've reached a non-OpenAir cabin:
		if (v.IsVehicle() && !v.GetAgentReference().TestAttributes(Agent::attrOpenAirCabin))
			return v;

		// Otherwise keep going until we get to the base vehicle:
	} while (true);
}





#ifdef AGENT_PROFILER
	int Agent::ourAgentProfilerTicks = 0;
	double Agent::ourAgentProfilerPaceTotal = 0.0;
#endif

int theAgentCount = 0;

int GetAgentCount()
{
	return theAgentCount;
}

//------------------------------------------------------------------
//
// Agent stuff proper
//


// this function should only be called after the Map has been reset completely:
void Agent::ForceCaAddition()
{
	// reset processing state:
	if( myCAIsNavigable )
	{
		myCAProcessingState = stateSettingsChange;
		myPrevCAIndex = -1;
		myPrevCAIncrease = 0.0f;
		myPrevCAIsNavigable = false;
		myPrevRoomWhenEmitting = -1;

		// update values into the map:
		HandleCA();
	}
}




// Serialisation & subclass constructor
Agent::Agent():
	myAgentType(AgentHandle::agentNormal),
	myReferenceCount(0),
	myEntityImage(NULL),
	myVelocityXVariable(myVelocityVector.x, myStoppedFlag),
	myVelocityYVariable(myVelocityVector.y, myStoppedFlag)
{
	++theAgentCount;
	try
	{
		Init();
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0013: Unknown exception caught in agent constructor";
	}
	if (!myFailedConstructionException.empty())
	{
		myAgentType = 0;
		return;
	}

}


Agent::~Agent()
{
	--theAgentCount;
	_ASSERT(myGarbaged);
	_ASSERT(myReferenceCount == 0);
}



// Virtual
uint32 Agent::GetMagicNumber()
{
	return 3141592654U;
}

void Agent::SpeakSentence(std::string& thisSentence)
{
	_ASSERT(!myGarbaged);
	if (GetVoice().GetCurrentVoice() == "")
		return;
	if (!GetVoice().BeginSentence( thisSentence ))
		return;
	int32 delay = 0;
	uint32 sound;
	while (GetVoice().GetNextSyllable(sound,delay)) {
			SoundEffect(sound,delay);
	}

}


bool Agent::IsConnectionValid
	(Port* in, Port* out, bool& warning)
{
	static const float safeDist = theApp.GetMaximumDistanceBeforePortLineWarns();
	static const float maxDist = theApp.GetMaximumDistanceBeforePortLineSnaps();
	// Get two positions
	Vector2D inPosition = in->GetWorldPosition();
	Vector2D outPosition = out->GetWorldPosition();

	// Get distance between
	float dist = inPosition.SquareDistanceTo(outPosition);
	if (dist > safeDist*safeDist)
		warning = true;
	else
		warning = false;

	if (dist > maxDist*maxDist)
	{
		return false;
	}

	Agent& inAgent = in->GetOwner().GetAgentReference();
	Agent& outAgent = out->GetOwner().GetAgentReference();

	int inID;
	if (!theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(inAgent.GetCentre(), inID))
		return false;
	int outID;
	if (!theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(outAgent.GetCentre(), outID))
		return false;

	if (inID != outID)
		return false;

	AgentHandle inCarrier = inAgent.GetCarrier();
	if ((inCarrier.IsVehicle()) && 
		(inCarrier.GetAgentReference().GetMovementStatus() == FLOATING))
		return false;
	AgentHandle outCarrier = outAgent.GetCarrier();
	if ((outCarrier.IsVehicle()) && 
		(outCarrier.GetAgentReference().GetMovementStatus() == FLOATING))
		return false;
	return true;
}


void Agent::InitialisePickupPointsAndHandles()
{
	_ASSERT(!myGarbaged);
	int i;
	if (GetPrimaryEntity())
	{
		// fill in invalid positions for all PUPT / PUHLs
		for(i = 0; i < GetPrimaryEntity()->GetAbsolutePoseMax(); i++)
		{
			myPointsWhereIamPickedUp.push_back(INVALID_POSITION);	// Indexed by pose.
			myPointsIUseToPickUpAgents.push_back(INVALID_POSITION);
		}
	}
}


// Basic initialisation used by constructors
void Agent::Init()
{
	int i;


	myShowCoreFlag = false;

	myPresence = new Presence(
		myVelocityVector, myPositionVector, myCurrentWidth, myCurrentHeight, myAeroDynamicFactor, myCollisionFactor, myFrictionFactor
	);

	myVirtualMachines.push_back(new CAOSMachine());

#ifdef AGENT_PROFILER
	myAgentProfilerCumulativeTime = 0;
	myAgentProfilerCumulativeTicks = 0;
#endif

	myResetLines = false;
	myUpdateTick = 0;

	// Used for bounding box checks
	myWidthTemp = -1.0f;
	myHeightTemp = -1.0f;

	myHighlightedFlag = false;
	myHighlightColour.r = 0;
	myHighlightColour.g = 0;
	myHighlightColour.b = 0;
	myMarchingAntCount = 0;

	myDrawMirroredFlag = false;
	myDrawFlippedVerticallyFlag = false;
	myDrawAlphaFlag = false;

	myOverrideCategory = -1;

    myMovementStatus = AUTONOMOUS;	// default to fixed locn (not floating etc)

    myAttributes = 0;				// default not carryable, etc.
	myCreaturePermissions = 0;		// default to creature can do nothing.

	myClickActionState = -1;		// default to no cycling of click state

	myCurrentWidth = myCoreWidth  = 0.0f;
	myCurrentHeight= myCoreHeight = 0.0f;
	myCorePositionVector = ZERO_VECTOR;
	// gtb:
	myAlteredBoundingBox.top = myAlteredBoundingBox.bottom = 0.0f;
	myAlteredBoundingBox.left= myAlteredBoundingBox.right  = 0.0f;


	for (i = 0; i < NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS; i++)
	{
		// initialise the actions to activate1
		myDefaultClickActionCycles[i] =ACTIVATE1;
	}

	myCarrierAgent = NULLHANDLE;				// no carrier
	myCarriedAgent = NULLHANDLE;			// not carrying anything
	myInputMask = 0;				// not interested in any input events

	myTimerRate = 0;					// default = issue no TIMER events
	myTimer = 0;
	myRunning = true;           // Agent's update should be called

	mySoundHandle=-1;					// No controlled sound is active
	mySoundName=0;
	mySoundLooping=FALSE;


	myPermiability = 50;
	myLastWallHit = -1;
	myGeneralRange = 500.0f;		// how far away agents can hear and see things
								// by default.  Can change with RNGE command.
	myGravitationalAcceleration = 0.3f;
	myStoppedFlag = false;
	
	myPositionVector = Vector2D(-9876.0f, -9876.0f); // off the display
	myFloatingPositionVector = Vector2D(-9876.0f, -9876.0f); // off the display
	myInvalidPosition = true;

	myVelocityVector = ZERO_VECTOR;
	myElasticity = 100;
	myCollisionFactor = 1.0f;
	myAeroDynamicPercentage = 0;
	myAeroDynamicFactor = 1.0f;
	myFrictionFactor = 0.0f;
	myFrictionPercentage = 100;

			// set up the PortBundle

	myDefaultClickAction = ACTIVATE1;

	myPrevCAIndex = myCAIndex = -1;
	myPrevCAIncrease = myCAIncrease = 0.0;
	myPrevCAIsNavigable = myCAIsNavigable = false;
	myPrevRoomWhenEmitting = -1;
	myCAProcessingState = stateNotProcessing;

	for (i=0; i<GLOBAL_VARIABLE_COUNT; ++i) {
		myGlobalVariables[i].SetInteger(0);
	}

	// by default do pixel perfect testing for Hit Tests
	myPixelPerfectHitTestFlag = true;
	myFailedConstructionException = "";

	myGarbaged = false;
	myImpendingDoom = false;
	mySelf = AgentHandle(this);
	myPorts.Init( mySelf );
}


// virtual
bool Agent::MoveToSafePlaceGivenCurrentLocation()
{
	Vector2D positionSafe;
	bool ok = theApp.GetWorld().GetMap().FindSafeAgentLocation
			(myCorePositionVector, myCoreWidth, myCoreHeight, 
			 myPermiability, positionSafe);
	if (ok)
		MoveCoreTo(positionSafe);
	return ok;
}

// virtual
bool Agent::MoveToSafePlace(const Vector2D& positionPreferred)
{
	Vector2D positionSafe;
	bool ok = theApp.GetWorld().GetMap().FindSafeAgentLocation
			(GetCorePosOf(positionPreferred), myCoreWidth, myCoreHeight, 
			 myPermiability, positionSafe);
	if (ok)
		MoveCoreTo(positionSafe);
	return ok;
}




Vector2D Agent::WhereShouldCreaturesPressMe() {
	_ASSERT(!myGarbaged);
	Vector2D pressPoint(myPositionVector.x + (GetWidth()/2.0f), myPositionVector.y + GetHeight());
	return pressPoint;
}



Port* Agent::IsPointOverPort(Vector2D& p, bool& inputnotoutput)
{
	_ASSERT(!myGarbaged);

	int in = myPorts.GetInputPortCount();
	int out = myPorts.GetOutputPortCount();
	if ((in == 0) && (out == 0))
		// Most agents don't have ports, bomb out straight away
		return NULL;

	int i;
	InputPort *ip;
	OutputPort *op;
	Vector2D pos;

	// Search for the point lying over an input port
	for (i=0;i<in;++i) {
		ip = myPorts.GetInputPort(i);
		if (ip == NULL) 
			continue;
		pos = myPositionVector + ip->GetRelativePosition();
		if ((fabsf(pos.x-p.x) > 10.0f) || (fabsf(pos.y-p.y) > 10.0f))
			continue;
		inputnotoutput = true;
		return ip;
	}

	// Search for the point lying over an output port
	for (i=0;i<out;++i) {
		op = myPorts.GetOutputPort(i);
		if (op == NULL) 
			continue;
		pos = myPositionVector + op->GetRelativePosition();
		if ((fabsf(pos.x-p.x) > 10.0f) || (fabsf(pos.y-p.y) > 10.0f))
			continue;
		inputnotoutput = false;
		return op;
	}
	return NULL;
}



// virtual
float Agent::GetDistanceToObstacle(int direction)
{
	_ASSERT(!myGarbaged);

	bool collision;
	float distanceCollision;

	if (myMovementStatus == AUTONOMOUS)
	{
		theApp.GetWorld().GetMap().TestForAgentCollisionInRoomSystem
			(myCorePositionVector, myCoreWidth, myCoreHeight, direction, 
			 myGeneralRange, myPermiability, collision, 
			 distanceCollision);
	}
	else if (myMovementStatus == INVEHICLE)
	{
		Box cabin;
		myCarrierAgent.GetVehicleReference().GetCabinExtent(cabin);
		theApp.GetWorld().GetMap().TestForAgentCollisionInRectangle
			(myCorePositionVector, myCoreWidth, myCoreHeight, direction, 
			 myGeneralRange, cabin.left, cabin.right, cabin.top, 
			 cabin.bottom, collision, distanceCollision);
	}
	else
		return FLT_MAX;

	if (collision)
		return distanceCollision;
	else
		return FLT_MAX;
}



int Agent::GetLastWallHit() 
{
	_ASSERT(!myGarbaged);
	return myLastWallHit;
}

// static
bool Agent::IsFromCreature(Message* Msg)
{
	bool isCreature = false;
	if (Msg->GetFrom().GetType() == CAOSVar::typeAgent)
		isCreature = Msg->GetFrom().GetAgent().IsCreature();

	return isCreature;
}

// Handle a message from another agent
void Agent::HandleMessage(Message* Msg)
{
	_ASSERT(!myGarbaged);

	bool isCreature = IsFromCreature(Msg);

    switch (Msg->GetMsg()) {
	// activation events - call virtual function
    case ACTIVATE1:
		
		if (isCreature && !TestCreaturePermissions(permCanActivate1))
			break;
        HandleActivate1(Msg);
        break;

    case ACTIVATE2:
		// creaturecast
		if (isCreature && !TestCreaturePermissions(permCanActivate2))
			break;
        HandleActivate2(Msg);
        break;

    case DEACTIVATE:
		if (isCreature && !TestCreaturePermissions(permCanDeactivate))
			break;
        HandleDeactivate(Msg);
        break;

    case HIT:
		if (isCreature && !TestCreaturePermissions(permCanHit))
			break;
        HandleHit(Msg);
        break;

    case PICKUP:
		if (isCreature && !TestCreaturePermissions(permCanPickUp))
			break;
        HandlePickup(Msg);
        break;

    case DROP:
        HandleDrop(Msg);
        break;

	// system events - execute any script for this obj/event

    case EAT:
		if (isCreature && !TestCreaturePermissions(permCanEat))
			break;
        HandleEat(Msg);
        break;

	case START_HOLD_HANDS:
        HandleStartHoldHands(Msg);
		break;

	case STOP_HOLD_HANDS:
		HandleStopHoldHands(Msg);
		break;

// Please remember! (tie a knot in your pyjamas),
// update the documentation table in Message.cpp if
// you add a new message, or change the meaning
// of any of these

	default:
		// Message is not one of the fixed messages defined in Message.h.
		// This is used to implement generic inter-agent communication
		// for injected COBs.
		if( Msg->GetMsg() >= SCRIPTUIMIN && Msg->GetMsg() <= SCRIPTUIMAX )
			HandleUI( Msg );
		else
			HandleOther(Msg);
        break;
    }
}


// Default message handlers for agents (virtual, so may be
// overloaded by subclasses). As base class is abstract, these handlers
// do nothing. Subclasses must define their own handlers wherever an action
// is required for this message
// Message is passed as a param, so can determine sender & supp. data
// Default message handlers for Agents (virtual, so may be
// overloaded by subclasses)
void Agent::HandleActivate1(Message* Msg)
{
	_ASSERT(!myGarbaged);

	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
   	int code =
		ExecuteScriptForEvent(SCRIPTACTIVATE1, from, p1, p2);
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(IsFromCreature(Msg)))
	{
		char s[1000];
		sprintf(s, "No activate 1 script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}


void Agent::HandleActivate2(Message* Msg)
{
	_ASSERT(!myGarbaged);
	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
  	int code =
		ExecuteScriptForEvent(SCRIPTACTIVATE2, from, p1,p2);
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(IsFromCreature(Msg)))
	{
		char s[1000];
		sprintf(s, "No activate 2 script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}


void Agent::HandleDeactivate(Message* Msg)
{
	_ASSERT(!myGarbaged);

	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
   	int code =
		ExecuteScriptForEvent(SCRIPTDEACTIVATE, from, p1, p2 );
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(IsFromCreature(Msg)))
	{
		char s[1000];
		sprintf(s, "No deactivate script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}


void Agent::HandleHit(Message* Msg)
{
	_ASSERT(!myGarbaged);

	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	int code =
		ExecuteScriptForEvent(SCRIPTHIT, from, p1, p2);
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(IsFromCreature(Msg)))
	{
		char s[1000];
		sprintf(s, "No hit script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}

void Agent::HandleStartHoldHands(Message* Msg)
{
	_ASSERT(!myGarbaged);
	// handled by the creature not here	
}

void Agent::HandleStopHoldHands(Message* Msg)
{
	_ASSERT(!myGarbaged);
	// handled by the creature not here
}

bool RecursiveCarrierTester(AgentHandle& what, AgentHandle& potentialCarrier)
{
	if (what == potentialCarrier)
		return false;
	// Jump up through the potentialCarrier's carrier chain until we hit nullhandle or "what"
	while (potentialCarrier.GetAgentReference().GetCarrier().IsValid() && potentialCarrier.GetAgentReference().GetCarrier() != what)
		potentialCarrier = potentialCarrier.GetAgentReference().GetCarrier();
	if (potentialCarrier.GetAgentReference().GetCarrier().IsInvalid())
		return true;
	return false;
}


bool Agent::TryToBePickedUp
	(AgentHandle newCarrierAgent, CAOSVar& p1, CAOSVar& p2)
{
	if (newCarrierAgent.IsInvalid())
		// Fail
		return false;

	bool pickupOK =
		// pickup request from the pointer...
		newCarrierAgent.IsPointerAgent() ? (TestAttributes(attrMouseable) ? true : false) :
		// pickup request from a Creature...
		newCarrierAgent.IsCreature() ? (TestCreaturePermissions(permCanPickUp) ? true : false) :
		// pickup request from Vehicle... (they can pick up anything)
		newCarrierAgent.IsVehicle() ? RecursiveCarrierTester(mySelf,newCarrierAgent) :
		// pickup requst from anything else...
		(TestAttributes(attrCarryable) ? RecursiveCarrierTester(mySelf,newCarrierAgent) : false);
	if (!pickupOK)
		// Fail
		return false;

	if (myCarrierAgent == newCarrierAgent)
		// We are already being carried by the requesting agent, fail
		return false;

	// Ask the new carrier if it can carry me
	if (!newCarrierAgent.GetAgentReference().CanStartCarrying(mySelf))
		return false;

	// If I'm currently being carried by another agent, then make it
	// drop me
	AgentHandle oldCarrier(myCarrierAgent);
	if (oldCarrier.IsValid())
	{
		oldCarrier.GetAgentReference().StopCarrying(mySelf);
		if (oldCarrier.IsVehicle()) 
			ExecuteScriptForEvent(SCRIPTDROPBYVEHICLE, oldCarrier, INTEGERZERO, INTEGERZERO);
		else 
			ExecuteScriptForEvent(SCRIPTDROP, oldCarrier, INTEGERZERO, INTEGERZERO);
	}

	// Make the new carrier start carrying me
	newCarrierAgent.GetAgentReference().StartCarrying(mySelf);

	MarkValidPositionNow();
	
	if (newCarrierAgent.IsVehicle()) 
		ExecuteScriptForEvent(SCRIPTPICKUPBYVEHICLE, newCarrierAgent, p1, p2);
	else 
		ExecuteScriptForEvent(SCRIPTPICKUP, newCarrierAgent, p1, p2);

	return true;
}


bool Agent::TryToBeDropped
	(AgentHandle fromAgent, CAOSVar& p1, CAOSVar& p2, bool forceDrop)
{
	if (myCarrierAgent.IsInvalid())
		return false;

	AgentHandle oldCarrier = myCarrierAgent;

	if (forceDrop)
	{
		// Make the current carrier drop me
		oldCarrier.GetAgentReference().StopCarrying(mySelf);
		// Move myself into a safe place on the map, relative to where I am now
		MoveToSafePlaceGivenCurrentLocation();
		if (oldCarrier.IsVehicle())
			ExecuteScriptForEvent( SCRIPTDROPBYVEHICLE, fromAgent, p1, p2 );
		else
			ExecuteScriptForEvent( SCRIPTDROP, fromAgent, p1, p2 );
	}
	else 
	{
		bool findSafePlace;
		AgentHandle v;
		if (!oldCarrier.GetAgentReference().CanStopCarrying(mySelf, v,
			findSafePlace))
		{
			// The pointer is unable to drop me where I currently am
			GoBackToLastClickActionState();
			// Cannot drop at all
			return false;
		}

		// Make the current carrier drop me
		oldCarrier.GetAgentReference().StopCarrying(mySelf);
		if (v.IsValid())
		{
			v.GetAgentReference().StartCarrying(mySelf);
		}
		else if (findSafePlace)
		{
			// Move myself into a safe place on the map, relative to where I am now
			MoveToSafePlaceGivenCurrentLocation();
		}
		if (oldCarrier.IsVehicle())
			ExecuteScriptForEvent( SCRIPTDROPBYVEHICLE, fromAgent, p1, p2 );
		else
			ExecuteScriptForEvent( SCRIPTDROP, fromAgent, p1, p2 );
		if (v.IsValid())
			ExecuteScriptForEvent( SCRIPTPICKUPBYVEHICLE, fromAgent, INTEGERZERO, INTEGERZERO);
	}
	return true;
}


void Agent::HandlePickup(Message* Msg)
{
	_ASSERT(!myGarbaged);
	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	if (from.GetType() == CAOSVar::typeAgent)
		(void)TryToBePickedUp( from.GetAgent(), p1, p2 );
}

void Agent::HandleDrop(Message* Msg) 
{
	_ASSERT(!myGarbaged);

	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	if (from.GetType() == CAOSVar::typeAgent)
		(void)TryToBeDropped(from.GetAgent(), p1, p2, false);
}








void Agent::HandleEat(Message* Msg)
{
	_ASSERT(!myGarbaged);
  
	CAOSVar from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	int code =
		ExecuteScriptForEvent(SCRIPTEAT, from, p1, p2 );
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(IsFromCreature(Msg)))
	{
		char s[1000];
		sprintf(s, "No eat script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}

void Agent::HandleUI(Message* Msg)
{
	_ASSERT(!myGarbaged);
	HandleOther( Msg );
}

/*********************************************************************
* Public: HandleOther.
*********************************************************************/
void Agent::HandleOther(Message* Msg)
{
	_ASSERT(!myGarbaged);

	// Execute macro event script Msg.
	ExecuteScriptForEvent(Msg->GetMsg(), Msg->GetFrom(), Msg->GetP1(), Msg->GetP2());
}

// If this agent is touching (within the bounds of) any other agent
// whose Attributes (CARRYABLE, etc) agree with that given, return the
// address of the topmost such agent. Else, return NULL

// The AttrFields & AttrState params are used to define the kind of agent
// I'm interested in. For example:
//      - if AttrFields==CARRYABLE and AttrState==CARRYABLE
//        then the fn will look for a carryable agent.
//      - if AttrFields==CARRYABLE|LIVING and AttrState==CARRYABLE
//        then fn will look for a CARRYABLE, non-LIVING thing.
//      - if AttrFields and AttrState ==0, then fn will look for ANY agent

AgentHandle Agent::IsTouching( DWORD AttrFields, // which Attr bit(s) I care about
                            DWORD AttrState)  // what state I want them to be
{
	_ASSERT(!myGarbaged);
    Vector2D ptThis;

	// Get my boundary...
	if (mySelf == thePointer) 
	{
        // If I am the pointer, use my hotspot.
		ptThis = thePointer.GetPointerAgentReference().GetHotSpot();
	}
	else
	{
        // Use centre base of agent.
        Box rectThis;
        GetAgentExtent(rectThis);
		ptThis.x = rectThis.left + (rectThis.Width() / 2.0f);
        ptThis.y = rectThis.bottom;
	}
	return theAgentManager.WhatAmITouching(ptThis,mySelf,AttrFields,AttrState);                              
}


// Calculate volume (-5000 -> 0) and panning values (-10000 -> 10000)
// Given the agents current location
void Agent::SoundLevels(long &volume, long &pan)
{
	_ASSERT(!myGarbaged);

	if(theApp.PlayAllSoundsAtMaximumLevel())
	{
// TODO: get rid of directsound constants!
#ifndef C2E_SDL
		volume = DSBVOLUME_MAX;
		pan = DSBPAN_CENTER;
#else
		// ARE THESE CORRECT?
		volume = 0;
		pan = 0;
#endif
		return;
	}

	int x=FastFloatToInteger(myPositionVector.x);
	int y=FastFloatToInteger(myPositionVector.y);

	Box screen;
	RECT temp;
	theMainView.GetViewArea(temp);
	screen = temp;

	int cx=(temp.left+temp.right)/2;
	int cy=(temp.top+temp.bottom)/2;

	x-=cx;
	y-=cy;

	if(screen.Width() == 0.0f)
	{
		volume = SoundMinVolume;
		return;
	}

	pan=FastFloatToInteger
		((x*5000.0f) / FastFloatToInteger(screen.Width()));
	if (pan<-10000)
	{
		pan=-10000;
	}
	else
	{
		if (pan>10000)
		{
			pan=10000;
		}
	}

	if (x<0)
	{
		x=-x;
	}

	if (y<0)
	{
		y=-y;
	}

	if (x>=FastFloatToInteger(screen.Width()/2.0f))
	{
		x-=FastFloatToInteger(screen.Width()/2.0f);
	}
	else
	{
		x=0;
	}
	
	if (y>=FastFloatToInteger(screen.Height()/2.0f))
	{
		y-=FastFloatToInteger(screen.Height()/2.0f);
	}
	else
	{
		y=0;
	}

	volume = ( SoundMinVolume * x) / ( FastFloatToInteger(screen.Width()) )
		   + ( SoundMinVolume * y) / ( FastFloatToInteger(screen.Height()) );
	
	if (volume < SoundMinVolume)
	{
		volume = SoundMinVolume;
	}

}


bool Agent::Visibility(int scope)
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
	{
		return myEntityImage->Visible(scope);
	}

	return false;
}

void Agent::Hide()
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
		myEntityImage->Unlink();

}

void Agent::Show()
{
	_ASSERT(!myGarbaged);

	// must force the entity to show because some entity Images
	// ingore the link command otherwise see cloned entity image and Guy
	if(myEntityImage)
		myEntityImage->Link(true);
}

Vector2D Agent::GetMapCoordOfWhereIPickUpAgentsFrom(int pose /*= -1*/, bool bRelative /* = false */)
{
	_ASSERT(!myGarbaged);

	// default is my position or global point
	Vector2D pos;
	if (bRelative)
	{
		pos = ZERO_VECTOR;
	}
	else
	{
		pos = myPositionVector;
	}

	if (GetPrimaryEntity())
	{
		int imageIndex = pose;
		if (imageIndex == -1)
			imageIndex = GetPrimaryEntity()->GetAbsolutePose();
	
		ASSERT(myPointsIUseToPickUpAgents.size() == GetPrimaryEntity()->GetAbsolutePoseMax());
		if (imageIndex >= 0 && imageIndex < myPointsIUseToPickUpAgents.size())
		{
			// the first point is 
			Vector2D currentpos = myPointsIUseToPickUpAgents[imageIndex];
			if (currentpos != INVALID_POSITION)
			{
				pos.x += currentpos.x;
				pos.y += currentpos.y;
				return pos;
			}
		}
	}
	return pos;
}

Vector2D Agent::GetMyOffsetOfWhereIAmPickedUpFrom(int pose /*= -1*/)
{
	_ASSERT(!myGarbaged);

	// default pickup point is near the top in the middle:
	Vector2D pos(GetWidth()/2.0f,GetHeight()/4.0f);

	if(GetPrimaryEntity())
	{
		int imageIndex = pose;
		if(imageIndex == -1)
			imageIndex = GetPrimaryEntity()->GetAbsolutePose();
		
	//	ASSERT(myPointsWhereIamPickedUp.size() == GetPrimaryEntity()->GetAbsolutePoseMax());
		if (imageIndex >= 0 && imageIndex < myPointsWhereIamPickedUp.size())
		{
			Vector2D currentpos = myPointsWhereIamPickedUp[imageIndex];
			if (currentpos != INVALID_POSITION)
				return currentpos;
		}
	}

	return pos;
}

EntityImage* Agent::GetPrimaryEntity()
{
	_ASSERT(!myGarbaged);

	return myEntityImage;
}


int32 Agent::GetPlaneForCarriedAgent()
{
	_ASSERT(!myGarbaged);

	// default is one less than your plane
	int32 plane = -1;
	if(myEntityImage)
	{
		plane = myEntityImage->GetPlane() -1;
	}
	return plane;
}

bool Agent::GetRoomID(int &roomID) //this would be much quicker if the agent kept track of 
				//current room
{
	_ASSERT(!myGarbaged);

	if (myMovementStatus == INVEHICLE)
	{
		int cabinRoom = myCarrierAgent.GetVehicleReference().GetCabinRoom();
		if (cabinRoom > -1)
		{
			roomID = cabinRoom;
			return true;
		}
	}

	return theApp.GetWorld().GetMap().GetRoomIDForPoint(GetCentre(), 
		roomID );
}


void Agent::HandleConnections()
{
	_ASSERT(!myGarbaged);

	int count = myPorts.GetInputPortCount();
	if (count == 0)
		return;

	InputPort *inport;
	OutputPort *outport;
	int i;
	Vector2D inpos;
	Vector2D outpos;
	bool valid, warning, broke=false;

	for (i=0; i<count; ++i) {
		inport = myPorts.GetInputPort(i);
		if (inport == NULL)
			continue;
		inport->Relax(20);
		outport = inport->GetConnectedPort();
		if (outport == NULL)
			continue;

		valid = IsConnectionValid(inport, outport, warning);
		if (!valid)
		{
			// Break the connection
			inport->DisconnectFromOutputPort();
			broke = true;
			theApp.GetWorld().WriteMessage( mySelf, 
				outport->GetOwner(), SCRIPTCONNECTIONBREAK, 
				INTEGERZERO, INTEGERZERO, 0 );
			continue;
		}

		inpos = myPositionVector + inport->GetRelativePosition();
		outpos = outport->GetOwner().GetAgentReference().GetPosition() +
			outport->GetRelativePosition();

		int r, g, b;
		if (warning)
		{
			r = 155 + inport->GetExcitementLevel()/2;
			g = 0;
			b = 0;
		}
		else 
		{
			r = 55 + inport->GetExcitementLevel();
			g = 155 + (inport->GetExcitementLevel()/2);
			b = 155 + (inport->GetExcitementLevel()/2);
		}
		DrawLine(FastFloatToInteger(outpos.x), 
			FastFloatToInteger(outpos.y), 
			FastFloatToInteger(inpos.x), 
			FastFloatToInteger(inpos.y), 
			r, g, b, 4, 4, myMarchingAntCount);
		myMarchingAntCount = (myMarchingAntCount - 1) % 8;
		myResetLines = true;
	}

	if (broke)
	{
		ExecuteScriptForEvent(SCRIPTCONNECTIONBREAK, mySelf,
								INTEGERZERO, INTEGERZERO);
	}
}


bool Agent::SetEmission(int caIndex, float value)
{
	_ASSERT(!myGarbaged);
	if ((caIndex < -1) || (caIndex > CA_PROPERTY_COUNT))
		return false;
	if ((myCAIndex == caIndex) && (myCAIncrease == value))
		return true;
	myCAIndex = caIndex;
	myCAIncrease = value;
	myCAProcessingState = stateSettingsChange;
	myCAIsNavigable = theApp.GetWorld().GetMap().IsCANavigable(caIndex);
	return true;
}


void Agent::HandleCA()
{
	_ASSERT(!myGarbaged);

	Map &map = theApp.GetWorld().GetMap();

	switch(myCAProcessingState)
	{
	case stateNotProcessing:
		return;

	case stateStableSettings:
		// We have stable settings. This means that we have not done a change.
		// This means that we only need check if the room we are (or are not) in has changed...
		if (myCAIsNavigable)
		{
			// We are emitting a navigable CA :)
			int roomID;
			GetRoomID(roomID);	// We can ignore the bool return code as roomID is -1 for no room.
			if (roomID != myPrevRoomWhenEmitting)
			{
				// Right then, we have moved room, so let's process it....
				if (myPrevRoomWhenEmitting != -1)
				{
					map.AlterCAEmission(myPrevRoomWhenEmitting,myPrevCAIndex,-myPrevCAIncrease);
				}
				// Now let's load our new values into the new room....
				if (roomID != -1)
				{
					map.AlterCAEmission(roomID,myCAIndex,myCAIncrease);
				}
				// As we are on stablesettings mode, simply propogate the new roomID...
				myPrevRoomWhenEmitting = roomID;
			}
		}
		else
		{
			// Not a navigable CA, so update if it's our turn to do so.
			if (map.GetCAIndex() == myCAIndex)
			{
				int roomID;
				if (GetRoomID(roomID))
					map.IncreaseCAInput(roomID, myCAIncrease);
			}
		}
		break;
	case stateSettingsChange:
		// We have a settings change which means one of four state changes.
		// Either:
		//         Non -> Nav
		//         Nav -> Non
		//         Nav -> Nav
		//         Non -> Non

		// State changes happen in two parts. First of all, we delete our old navigational effect.
		if (myPrevCAIsNavigable)
		{
			if (myPrevRoomWhenEmitting != -1)
				map.AlterCAEmission(myPrevRoomWhenEmitting,myPrevCAIndex,-myPrevCAIncrease);
		}

		// Now we propogate current CA settings into prev settings...
		myPrevCAIsNavigable = myCAIsNavigable;
		myPrevCAIncrease = myCAIncrease;
		myPrevCAIndex = myCAIndex;
		// myPrevRoomWhenEmitting = <WAIT>

		// Are we disabling the CA Emissions (I.E. is our CA index now -1)
		if (myPrevCAIndex == -1)
		{
			// We are disabling CA Emissions for this Agent.
			myCAProcessingState = stateNotProcessing;
			return;
		}

		// Right then, we need to decide what's happening with the CA....
		if (myCAIsNavigable)
		{
			// Okay then, we need to process an AlterCAEmission....
			Map &map = theApp.GetWorld().GetMap();
			int roomID;
			GetRoomID(roomID);
			myPrevRoomWhenEmitting = roomID;
			if (roomID != -1)
			{
				map.AlterCAEmission(roomID,myCAIndex,myCAIncrease);
			}
		}
		// Having processed the state change, we enter stable mode.
		myCAProcessingState = stateStableSettings;
	}
}


// Called by every agent every tick
// virtual
void Agent::Update()
{
	_ASSERT(!myGarbaged);


#ifdef DRAW_CIRCLES
  // Draw circle around agent:
	if (myUpdateTick%1==0)
	{
		ResetLines();
		Angle a;
		float r = myPresence->GetCircle().GetRadius();
		Vector2D c = myPresence->GetCircle().GetCentre();
		Vector2D u,v;
		u = c + a.GetVector()*r;
		for (int i=0; i<8; i++)
		{
			a.RotateBy(Angle(0.125f));
			v = c + a.GetVector()*r;
			DrawLine(u.x,u.y, v.x,v.y, 255,255,255);
			u = v;
		}
	}
	// end gtb!temp!
#endif//DRAW_CIRCLES

	if (myResetLines)
	{
		ResetLines();
		myResetLines = false;
	}


	myUpdateTick++;

	if (myUpdateTick % 5 == 0)
		HandleSound();

	// maybe extend this to do nothing if not in a valid position?
	if (!myInvalidPosition)
	{
		Vector2D oldPos = myPresence->GetCircle().GetCentre(); //myPositionVector

		// update presence and get it to update our velocity:
		UpdateFromPresence();

		// update our physics:
		HandleMovement();

		//added by gtb
		if (TestAttributes(attrRotatable) || TestAttributes(attrPresence))
		{
			// Update presence location and velocity from our physics:
			myPresence->UpdateFromAgent();

			if (TestAttributes(attrPresence))
			{
				for (AgentListIterator it=theAgentManager.GetAgentIteratorStart(); !theAgentManager.IsEnd(it); ++it)
				{
					if ((*it).IsValid())
					{
						if ((*it).GetAgentReference().TestAttributes(attrPresence))
						{
							Presence& p = (*it).GetAgentReference().GetPresence();

							if (myPresence->IsImpactingWith(p))
							{
								CAOSVar impactAgentVar;
								impactAgentVar.SetAgent(*it);

								// Acts on Agent variables:
								myPresence->ImpactWith(p, oldPos);
								// gtb!temp! to get entity to move:
								MoveTo(myPositionVector.x, myPositionVector.y);
	//							SetRunning(false);

	/*							HandleMovement();
								myPresence->UpdateFromAgent();
								myPresence->Update();
								UpdateFromPresence();
	*/
#ifdef DEBUG
								// breakpointable code for debugging:
								if (myPresence->IsImpactingWith(p))
								{
									int x = 0;
									x = x + 3;
								}
								else
								{
									int y = 1;
									y = y + 3;
								}
#endif
								// call impact script:
								CallScript(SCRIPTIMPACT, impactAgentVar, INTEGERZERO);
							}
					
						}
					}
				}
			}
		}
	}

	HandleCA();

	// Check for bounding box changes that take us out of the map
	float width = GetWidth();
	float height = GetHeight();
	bool ok = true;
	if (!myInvalidPosition && TestAttributes(attrSufferCollisions) &&
		(myMovementStatus == AUTONOMOUS) &&
		(myWidthTemp > 0.0f) && (myHeightTemp > 0.0f) && !mySelf.IsCreature()) 
	{
		if ((width != myWidthTemp) || (height != myHeightTemp)) {
			ok = TestCurrentLocation(false);
		}
	}
	if (!ok)
	{
		std::string message = ErrorMessageHandler::Format("invalid_map_error", 0, std::string("Agent::Update"), 
			FastFloatToInteger(myWidthTemp), 
			FastFloatToInteger(myHeightTemp),
			FastFloatToInteger(width), 
			FastFloatToInteger(height));
		CAOSMachineRunError temporaryRunError(message.c_str());
		HandleRunError(temporaryRunError);
	}
	myWidthTemp = width;
	myHeightTemp = height;

	// Deal with input events if there are any
	if( theApp.GetInputManager().GetPendingMask() & myInputMask )
	{
		HandleInputEvents();
		_ASSERT(!myGarbaged);
	}

	// Deal with any input-to-output connections heading out of this agent
	HandleConnections();


	// Highlight for creature permissions:
	int creaturePerm = theApp.GetWhichCreaturePermissionToHighlight();
	if (creaturePerm && TestCreaturePermissions(creaturePerm)) {
		Box r;
		GetAgentExtent(r);
		// draw X:
		DrawLine(FastFloatToInteger(r.left), FastFloatToInteger(r.top), FastFloatToInteger(r.right), FastFloatToInteger(r.bottom), 30,255,255, 0,0);		//yellow:
		DrawLine(FastFloatToInteger(r.right), FastFloatToInteger(r.top), FastFloatToInteger(r.left), FastFloatToInteger(r.bottom), 30,255,255, 0,0);
		myResetLines = true;
	}
	// Draw bounding box if this agent is highlighted:
	if (myHighlightedFlag) {
		Box r;
		GetAgentExtent(r);
		// draw box:
		DrawLine(FastFloatToInteger(r.left), FastFloatToInteger(r.top), FastFloatToInteger(r.right), FastFloatToInteger(r.top), myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		DrawLine(FastFloatToInteger(r.left), FastFloatToInteger(r.top), FastFloatToInteger(r.left), FastFloatToInteger(r.bottom), myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		DrawLine(FastFloatToInteger(r.left), FastFloatToInteger(r.bottom), FastFloatToInteger(r.right), FastFloatToInteger(r.bottom), myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		DrawLine(FastFloatToInteger(r.right), FastFloatToInteger(r.top), FastFloatToInteger(r.right), FastFloatToInteger(r.bottom), myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		myResetLines = true;
	}
	if (myShowCoreFlag)
	{
		// draw diamond:
		const Vector2D& pos = myCorePositionVector;
		float w = myCoreWidth;
		float h = myCoreHeight;
		float w2 = myCoreWidth/2.0f;
		float h2 = myCoreHeight/2.0f;
		int r = 255;
		int g = 30;
		int b = 255;
		DrawLine(FastFloatToInteger(pos.x), FastFloatToInteger(pos.y+h2), FastFloatToInteger(pos.x+w2), FastFloatToInteger(pos.y), r,g,b, 0,0);		//    line top    '/'
		DrawLine(FastFloatToInteger(pos.x), FastFloatToInteger(pos.y+h2), FastFloatToInteger(pos.x+w2), FastFloatToInteger(pos.y+h), r,g,b, 0,0);	//    line bottom '\'
		DrawLine(FastFloatToInteger(pos.x+w), FastFloatToInteger(pos.y+h2), FastFloatToInteger(pos.x+w2), FastFloatToInteger(pos.y), r,g,b, 0,0);	//    line top    '\'
		DrawLine(FastFloatToInteger(pos.x+w), FastFloatToInteger(pos.y+h2), FastFloatToInteger(pos.x+w2), FastFloatToInteger(pos.y+h), r,g,b, 0,0);	//    line bottom '/'
		myResetLines = true;
	}

	_ASSERT(!myGarbaged);

	try
	{

		// update timer and issue a TIMER script event if timed out
		if( myTimerRate>0 )
		{
			if( ++myTimer >= myTimerRate )
			{

				ExecuteScriptForEvent(SCRIPTTIMER, mySelf, INTEGERZERO,
					INTEGERZERO);
				_ASSERT(!myGarbaged);
				myTimer = 0;
			}
		}

		if (IsRunning())
		{
			_ASSERT(!myGarbaged);
			GetVirtualMachine().UpdateVM( 5 );	//  instructions per tick

			PopCurrentVmIfFinished();
		}
	}
	catch( CAOSMachine::InvalidAgentHandle& te)
	{
		// Right then, are we running an exception script already?!?!?!
		Classifier d;
		GetVirtualMachine().GetScript()->GetClassifier(d);
		if (d.myEvent == SCRIPTAGENTEXCEPTION)
		{
			// Okay then, we were running an exception script already, so do it via HRE
			HandleRunError(te);
		}
		else
		{
			// We were not running an exception script, do we have one?
			Classifier c(myClassifier);
			c.SetEvent(SCRIPTAGENTEXCEPTION);
			CAOSVar p1,p2;
			p1.SetInteger(d.myEvent);
			p2.SetString(te.what());
		
			if (ExecuteScriptForClassifier(c,mySelf,p1,p2) != EXECUTE_SCRIPT_STARTED)				
				// We could not start an exception script
				HandleRunError(te);
			
		}
	}
	catch( CAOSMachineRunError& e )
	{
		HandleRunError(e);
	}
	catch(BasicException& e)
	{
		myRunning = false;
		ErrorMessageHandler::Show(e, "Agent::Update");
	}
}



void Agent::MoveTo(float x, float y)
{
	_ASSERT(!myGarbaged);
	myPositionVector.x = x;
	myPositionVector.y = y;

	myCorePositionVector = GetCorePosOf(myPositionVector);
}

void Agent::MoveCoreTo(const Vector2D& v)
{
	_ASSERT(!myGarbaged);
	MoveTo(GetPosFromCore(v).x, GetPosFromCore(v).y);
}


void Agent::MoveBy(float xd, float yd)
{
	_ASSERT(!myGarbaged);

	if (GetMovementStatus() == Agent::FLOATING)
	{
		Vector2D floatPos = myFloatingPositionVector;
		FloatRelativeToWorld(floatPos);
		FloatTo(Vector2D(floatPos.x+xd, floatPos.y+yd));
	}
	else
		MoveTo(myPositionVector.x+xd, myPositionVector.y+yd);
}


// virtual
bool Agent::TestCurrentLocation(bool forcePositionCheck)
{
	_ASSERT(!myGarbaged);
	return TestMoveTo(myPositionVector.x, myPositionVector.y, forcePositionCheck);
}


//virtual
bool Agent::TestRoomSystemLocation()
{
	if (!TestAttributes(attrSufferCollisions))
		return true;
	return theApp.GetWorld().GetMap().IsAgentLocationValidInRoomSystem
		(myCorePositionVector, myCoreWidth, myCoreHeight, 
			myPermiability);
}


// virtual
bool Agent::TestMoveTo(float x, float y, bool forcePositionCheck)
{
	_ASSERT(!myGarbaged);

	if (!forcePositionCheck && !TestAttributes(attrSufferCollisions))
		return true;
	if ((myMovementStatus == FLOATING) ||
		(myMovementStatus == CARRIED))
		// Can move everywhere
		return true;
	else
	{
		Vector2D corePos = GetCorePosOf(Vector2D(x,y));
		if (myMovementStatus == INVEHICLE)
		{
			Box cabin;
			myCarrierAgent.GetVehicleReference().GetCabinExtent(cabin);
			return theApp.GetWorld().GetMap().IsAgentLocationValidInRectangle
				(corePos, myCoreWidth, myCoreHeight,
				cabin.left, cabin.right, cabin.top, cabin.bottom);
		}
		else
		{
			return theApp.GetWorld().GetMap().IsAgentLocationValidInRoomSystem
				(corePos, myCoreWidth, myCoreHeight, myPermiability);
		}
	}
}


// virtual
bool Agent::TestMoveBy(float x, float y, bool forcePositionCheck)
{
	_ASSERT(!myGarbaged);
	return TestMoveTo(myPositionVector.x+x,myPositionVector.y+y,forcePositionCheck);
}





//
//
// Movement Routines
//
//

void Agent::HandleMovement()
{
	_ASSERT(!myGarbaged);

	switch( myMovementStatus )
	{
	case AUTONOMOUS:
		HandleMovementWhenAutonomous();
		break;
	case FLOATING:
		HandleMovementWhenFloating();	
		break;
	case INVEHICLE:
		HandleMovementWhenInVehicle();
		break;
	case CARRIED:
		HandleMovementWhenCarried();
		break;
	}
}


void Agent::HandleMovementWhenAutonomous()
{
	if (myStoppedFlag)
	{
#ifdef C2D_DIRECT_DISPLAY_LIB
		// the new display only deals in screen positions I'm afraid so
		// I have to renew it each time
		MoveTo(myPositionVector.x, myPositionVector.y);
#endif
		return;

	}

	bool collision;
	bool stopped;
	int wall;
	Vector2D velocityCollision;
	int permiability;

	// Calculate permiability
	if (TestAttributes(attrSufferCollisions))
		permiability = myPermiability;
	else
		permiability = 0;

	Vector2D myNewCorePositionVector(myCorePositionVector);

	bool applyPhysics = 
		(myAttributes & attrSufferPhysics) == attrSufferPhysics;


	theApp.GetWorld().GetMap().MoveAgentInsideRoomSystem
		(myCoreWidth, myCoreHeight, applyPhysics,
		permiability,
		myCollisionFactor, myAeroDynamicFactor, 
		myFrictionFactor,
		myGravitationalAcceleration,
		myNewCorePositionVector, myVelocityVector, collision, wall,
		stopped, velocityCollision);

	MoveCoreTo(myNewCorePositionVector);

	if (stopped)
		myStoppedFlag = true;

	if (collision) {
		CAOSVar vcx, vcy;
		myLastWallHit = wall;
		vcx.SetFloat(velocityCollision.x);
		vcy.SetFloat(velocityCollision.y);
	
		ExecuteScriptForEvent(SCRIPTCOLLISION, mySelf, vcx, vcy); 
	}
}


void Agent::HandleMovementWhenFloating()
{
	bool collision;
	bool stopped;
	int wall;
	int permiability = 0;
	Vector2D velocityCollision;

	theApp.GetWorld().GetMap().MoveAgentInsideRoomSystem
		(myCurrentWidth, myCurrentHeight, false,
		permiability, 
		myCollisionFactor, myAeroDynamicFactor, 
		myFrictionFactor,
		myGravitationalAcceleration,
		myFloatingPositionVector, myVelocityVector,
		collision, wall,
		stopped, velocityCollision);
	
	// We move here, as well as in the camera.  This is to be
	// sure our position is reasonably up to date if another 
	// script looks at it in the meanwhile
	CameraPositionNotify();
}


void Agent::HandleMovementWhenInVehicle()
{
	_ASSERT(!myGarbaged);

	if (myStoppedFlag)
	{
#ifdef C2D_DIRECT_DISPLAY_LIB
		// the new display only deals in screen positions I'm afraid so
		// I have to renew it each time
		MoveTo(myPositionVector.x, myPositionVector.y);
#endif
		return;
	}

	float w,h;
	Vector2D p, vel;
	bool collision;
	int wall;
	bool stopped;
	Vector2D velocityCollision;
	CAOSVar vcx, vcy;
	Box r;

	p = myCorePositionVector;
	vel = myVelocityVector;
	w = GetCoreWidth();
	h = GetCoreHeight();

	// vehicle cast
	ASSERT(GetCarrier().IsVehicle());
	
	Vehicle& v = GetCarrier().GetVehicleReference();

	v.GetCabinExtent(r);

	bool applyPhysics = (myAttributes & attrSufferPhysics) == attrSufferPhysics;
	theApp.GetWorld().GetMap().MoveAgentInsideRectangle
		(w, h, applyPhysics,
		 myCollisionFactor, myAeroDynamicFactor, 
			myFrictionFactor,
			myGravitationalAcceleration,
		 r.left,
		 r.right,
		 r.top,
		 r.bottom,
		 p, vel, 
		 collision, 
		 wall, 
		 stopped,
		 velocityCollision);

	myVelocityVector = vel;
	MoveCoreTo(p);

	if (stopped)
		myStoppedFlag = true;

	if (collision) {
		myLastWallHit = wall;
		vcx.SetFloat(velocityCollision.x);
		vcy.SetFloat(velocityCollision.y);
		
		ExecuteScriptForEvent(SCRIPTCOLLISION, mySelf, vcx, vcy); 
	}
}


void Agent::HandleMovementWhenCarried()
{
	_ASSERT(!myGarbaged);
	_ASSERT( myCarrierAgent.IsValid() );
	_ASSERT( myCarrierAgent.GetAgentReference().myCarriedAgent == mySelf );


	Vector2D carrier = myCarrierAgent.GetAgentReference().GetMapCoordOfWhereIPickUpAgentsFrom();
	Vector2D offset = GetMyOffsetOfWhereIAmPickedUpFrom();
	Vector2D p = carrier - offset;

	MoveTo(p.x, p.y);
}








// scan through the pending input events and run scripts for any which
// the agent is listening for...
void Agent::HandleInputEvents()
{
	_ASSERT(!myGarbaged);

	InputManager& im = theApp.GetInputManager();
	int i;
	int scriptnum;
	const InputEvent* ev;
	int param1,param2;
	CAOSVar p1, p2;

	for( i=0; i<im.GetEventCount(); ++i )
	{
		ev = &im.GetEvent(i);

		if( ev->EventCode & myInputMask)
		{
			param1 = 0;
			param2 = 0;
			switch( ev->EventCode )
			{
			case InputEvent::eventKeyUp:
				scriptnum = SCRIPTRAWKEYUP;
				param1 = ev->KeyData.keycode;
				break;
			case InputEvent::eventKeyDown:
				scriptnum = SCRIPTRAWKEYDOWN;
				param1 = ev->KeyData.keycode;
				break;
			case InputEvent::eventTranslatedChar:
				scriptnum = SCRIPTRAWTRANSLATEDCHAR;
				param1 = ev->KeyData.keycode;
				break;
			case InputEvent::eventMouseMove:
				scriptnum = SCRIPTRAWMOUSEMOVE;
				param1 = ev->MouseMoveData.mx;
				param2 = ev->MouseMoveData.my;
				// translate into world coords
				theMainView.ScreenToWorld( param1, param2);
				break;
			case InputEvent::eventMouseDown:
				scriptnum = SCRIPTRAWMOUSEDOWN;
				param1 = ev->MouseButtonData.button;
				break;
			case InputEvent::eventMouseUp:
				scriptnum = SCRIPTRAWMOUSEUP;
				param1 = ev->MouseButtonData.button;
				break;
			case InputEvent::eventMouseWheel:
				scriptnum = SCRIPTRAWMOUSEWHEEL;
				param1 = ev->MouseWheelData.delta;
				break;
			default:
				ASSERT( false );	// unknown event type!
				break;
			}

			// no "from" agent for inputevents
			p1.SetInteger(param1);
			p2.SetInteger(param2);
			ExecuteScriptForEvent(scriptnum, NULLHANDLE, p1, p2);
			_ASSERT(!myGarbaged);
		}
	}
}





// helper for Update()
// This updates any active controlled sound
// (Altering their panning/volume as position changes
// and stopping/restarting them as they move in and
// out of range

void Agent::HandleSound()
{
	_ASSERT(!myGarbaged);

	if (!theSoundManager)
		return;
	
	// Is a sound active
	if (mySoundName)
	{

		// Is it currently playing
		if (mySoundHandle>=0)
		{
			
			// Sound is playing - adjust levels or finish it
			if (theSoundManager->FinishedControlledSound(mySoundHandle))
			{
				theSoundManager->StopControlledSound(mySoundHandle);
				mySoundHandle=-1;
				mySoundName=0;
			}
			else
			{
				if (Visibility(0))
				{
					long volume, pan;
					SoundLevels(volume,pan);
					theSoundManager->UpdateControlledSound(mySoundHandle,volume,pan);
				}
				else
				{
					// agent off screen - suspend/destroy it
					theSoundManager->StopControlledSound(mySoundHandle);
					mySoundHandle=-1;
					if (!mySoundLooping)
					{
						// Don't restart a one off sound
						mySoundName=0;
					}
				}
			}
		}
		else
		{
			// Sound is active, but not currently playing
			if (Visibility(0))
			{
				// restart it if it is in range
				SOUNDHANDLE h;
				SOUNDERROR err;
				long volume, pan;
				SoundLevels(volume,pan);
				err=theSoundManager->StartControlledSound(mySoundName,h,volume,pan,mySoundLooping);
				if (err || h==-1)
				{
					// "SOUND ERROR: %d PLAYING %s %d\n",err,tokn_str,mySoundHandle
				}
				else
				{
					mySoundHandle=h;
					// "Sound %s %d re-entered range\n",tokn_str,mySoundHandle
				}
			}
		}
	}
}



// retn message# (ACTIVATE1 etc) appropriate for a click at a given
// position, else return -1
// Since this is the default version of this virtual function, it should
// just return ACTIVATE1, which is the default for simple agents
// (compound agents will return different msgs according to which button
// was under xy)

int Agent::ClickAction(int x,int y)
{
	_ASSERT(!myGarbaged);

	return -1;
}


int Agent::ExecuteScriptForClassifier
	(const Classifier& c, const AgentHandle& from,
	 const CAOSVar& p1, const CAOSVar& p2)
{
	CAOSVar fromVar;
	fromVar.SetAgent(from);
	return ExecuteScriptForClassifier(c, fromVar, p1, p2);
}

int Agent::ExecuteScriptForClassifier
	(const Classifier& c, const CAOSVar& from,
	 const CAOSVar& p1, const CAOSVar& p2)
{
	_ASSERT(!myGarbaged);

	MacroScript* m;
	try
	{
		if( GetVirtualMachine().IsRunning() && (c.Event() != SCRIPTAGENTEXCEPTION))
		{
			// script-interruption rules. UGH.

			// - timer can't interrupt anything
			// - can't interrupt a LOCKed script
			if( c.Event() == SCRIPTTIMER ||
				GetVirtualMachine().IsLocked() )
			{
				return EXECUTE_SCRIPT_NOT_INTERRUPTIBLE;
//std::cout << "ExecuteScript not interrupt ";
//c.StreamClassifier(std::cout, true);
//std::cout << std::endl;

			}
		}

		m = theApp.GetWorld().GetScriptorium().FindScript(c);
		if( m == NULL )
			return EXECUTE_SCRIPT_NOT_FOUND;

		GetVirtualMachine().StartScriptExecuting( m, mySelf ,from, p1, p2 );

		// Execute at least one instruction immediately.
		// This ensures that scripts starting with INST get executed
		// atomically, otherwise a little bit of the script gets done
		// immediately.
		GetVirtualMachine().UpdateVM( 1 );
		PopCurrentVmIfFinished();

//std::cout << "ExecuteScript OK ";
//c.StreamClassifier(std::cout, true);
//std::cout << std::endl;

		return EXECUTE_SCRIPT_STARTED;
	}
	catch( CAOSMachine::InvalidAgentHandle& te)
	{
		// Right then, are we running an exception script already?!?!?!
		Classifier d;
		GetVirtualMachine().GetScript()->GetClassifier(d);
		if (d.myEvent == SCRIPTAGENTEXCEPTION)
		{
			// Okay then, we were running an exception script already, so do it via HRE
			HandleRunError(te);
		}
		else
		{
			// We were not running an exception script, do we have one?
			Classifier c(myClassifier);
			c.SetEvent(SCRIPTAGENTEXCEPTION);
			CAOSVar p1,p2;
			p1.SetInteger(d.myEvent);
			p2.SetString(te.what());
			
			if (ExecuteScriptForClassifier(c,mySelf,p1,p2) != EXECUTE_SCRIPT_STARTED)				
				// We could not start an exception script
				HandleRunError(te);
			
		}
	}
	catch( CAOSMachineRunError& e )
	{
		HandleRunError(e);
	}
	return EXECUTE_SCRIPT_STARTED;
}


int Agent::ExecuteScriptForEvent
	(int event, const CAOSVar& from,
	 const CAOSVar& p1, const CAOSVar& p2)
{
	_ASSERT(!myGarbaged);
	Classifier c(myClassifier);
	c.SetEvent(event);
	return ExecuteScriptForClassifier(c, from, p1, p2);
}


int Agent::ExecuteScriptForEvent
	(int event, const AgentHandle& from,
	 const CAOSVar& p1, const CAOSVar& p2)
{
	CAOSVar fromVar;
	fromVar.SetAgent(from);
	return ExecuteScriptForEvent(event, fromVar, p1, p2);
}









///////////////////// macro helper functions ///////////////////

// Default version of virtual helper function for POSE macro.
// Overloaded forms set a new pose & return TRUE if change is complete. I return TRUE always.
bool Agent::ShowPose(int pose,int part)
{
	_ASSERT(!myGarbaged);
	return true;
}

// Default version of virtual helper function for POSE
// Overloaded forms get current pose for given part#
int Agent::GetPose(int part)
{
	_ASSERT(!myGarbaged);
	return 0;
}




// default helper fn for BASE macro - set base sprite for obj or given part
bool Agent::SetBaseImage(int image,int part)
{
	_ASSERT(!myGarbaged);
	return true;
}













void Agent::Trash()
{
	_ASSERT(!myGarbaged);

	delete myPresence;

	// Dump my previous CA if I need to.
	if (myCAProcessingState != stateNotProcessing)
	{
		if (myPrevCAIsNavigable)
		{
			theApp.GetWorld().GetMap().AlterCAEmission(myPrevRoomWhenEmitting,myPrevCAIndex,-myPrevCAIncrease);
		}
	}

	// Clear any messages that will be for us
	theApp.GetWorld().RemoveMessagesAbout(mySelf);

	BreakLinksToOtherAgents();

	// Delete genome files
	GetGenomeStore().ClearAllSlots();

	// And we're not registered as floating any more
	if ( myMovementStatus == FLOATING )
		theMainView.RemoveFloatingThing(mySelf);

	// Clear all global variables thus relinquishing any pointers
	for (int i=0; i<GLOBAL_VARIABLE_COUNT; ++i) 
		myGlobalVariables[i].SetInteger(0);
	myNameVars.clear();

	myRunning = false;

	// kill any running scripts:
	while (!myVirtualMachines.empty())
	{
		CAOSMachine* c;
		c = myVirtualMachines.back();
		myVirtualMachines.pop_back();
		delete c;
	}
	// kill any spare VMs:
	while (!mySpareVirtualMachines.empty())
	{
		CAOSMachine* c;
		c = mySpareVirtualMachines.back();
		mySpareVirtualMachines.pop_back();
		delete c;
	}


	myPorts.Trash();

	if (myEntityImage)
		delete myEntityImage;
	myEntityImage = NULL;

	// Clears the type to prevent RTTI from the AgentHandle's point 
	// of view.
	myAgentType = 0; 

	if (mySoundHandle>=0)
	{
		StopControlledSound(mySoundHandle);
	}

	// These must be the last two lines in the function
	mySelf = NULLHANDLE;
	myGarbaged = true;
}




// Play this sound effect if this agent is audible to the user
// The sound starts playing at the appropriate volume/panning
// but the agent has no further control over the sound
// (i.e. when it stops/altering its panning)
// the sound is played after 'delay' ticks
void Agent::SoundEffect(DWORD fsp, int delay)
{
	_ASSERT(!myGarbaged);

    // Just in case sound effect called before sound manager initialised!
    if (!theSoundManager)
        return;

	long volume,pan;

	if (Visibility(0))
	{
		SoundLevels(volume,pan);
		SOUNDERROR err;
		err = theSoundManager->PlaySoundEffect(fsp,delay,volume,pan);
	}

}

// Play this sound  if this agent is audible to the user
// The sound starts playing at the appropriate volume/panning
// and is continually updated by 'Agent::Update()'
// This alters its panning and volume as the agent moves
// and also checks if it goes in or out of range.
// Looped sounds are restarted when they come back in range
// while non-looped sounds cease to exist when out of range
void Agent::ControlledSound(DWORD fsp, BOOL loop)
{
	_ASSERT(!myGarbaged);

	if (!theSoundManager)
		return;

	long volume,pan;
				// determine volume according to location
	if (Visibility(0))
	{
		SoundLevels(volume,pan);
		SOUNDERROR err;
	
		SOUNDHANDLE h;
		err=theSoundManager->StartControlledSound(fsp,h,volume,pan,loop);
		if (err)
		{
			// SOUND ERROR: %d PLAYING %s\n",err,tokn_str
		}
		else
		{
			if (mySoundHandle>=0)
			{
				// Sound %s replacing %s\n",tokn_str,old_str

				// Fade out any sound currently playing
				theSoundManager->StopControlledSound(mySoundHandle,TRUE);
			}

			mySoundHandle=h;
			mySoundName=fsp;
			mySoundLooping=loop;
		}

	}
	else
	{
		if (loop)
		{
			if (mySoundHandle>=0)
			{
				// Sound %s (off-screen) replacing %s\n",tokn_str,old_str
				// Fade out any sound currently playing
				theSoundManager->StopControlledSound(mySoundHandle,TRUE);
			}

			mySoundHandle=-1;		// off range, so not playing
			mySoundName=fsp;
			mySoundLooping=loop;
			
		}
	}

}

// If a controlled sound is currently active,
// it is stopped (either abruptly or faded)
// Hence Stop...Controlled...Sound...
void Agent::StopControlledSound(BOOL fade)
{
	_ASSERT(!myGarbaged);


	if (mySoundHandle>=0)
	{
        if (theSoundManager)
		    theSoundManager->StopControlledSound(mySoundHandle,fade);
	}

	mySoundHandle=-1;
	mySoundName=0;
	mySoundLooping=FALSE;

}


void Agent::PauseControlledSound()				// Stop active sound when game is paused
{
	_ASSERT(!myGarbaged);

	if (mySoundHandle>=0)
	{
		if (theSoundManager)
			theSoundManager->StopControlledSound(mySoundHandle);
		mySoundHandle=-1;

		// If the sound is looping, we want it to start up again when unpaused
		if (!mySoundLooping)
		{
			mySoundName=0;
		}
	}

}



/*********************************************************************
* Public: CanHear.
* Return true if this agent can hear another
*********************************************************************/
bool Agent::CanHear(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	_ASSERT( other2.IsValid() );

	// Can't hear self
	if (mySelf == other)
		return false;

	Vector2D v1(GetCentre());
	Vector2D v2(other.GetAgentReference().GetCentre());

	int v1id,v2id;
	if (theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(v1,v1id) && 
		theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(v2,v2id))
	{
		if (v1id == v2id)
		{
			float audioRange = GetAudibleRange();
			return v1.SquareDistanceTo(v2) <= (audioRange * audioRange);
		}
	}
	return false;
}


/*********************************************************************
* Public: CanSee.
* Return true if this agent can see another
*********************************************************************/
bool Agent::CanSee(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	ASSERT( other2.IsValid() );

	// Can't see self
	if (mySelf == other)
		return false;

	Vector2D v1 = GetCentre();
	Vector2D v2 = other.GetAgentReference().GetCentre();

	// Check if in visual range
	float visualRange = GetVisualRange();
	float distance2 = v1.SquareDistanceTo(v2);
	if (distance2 > (visualRange * visualRange))
		return false;

	// Check if walls in the way
	int perm = GetPermiability();
	bool canSee;
	bool result = theApp.GetWorld().GetMap().CanPointSeePoint(v1, v2, perm, canSee);
	if (!result) {
		return false;
	}

	return canSee;
}





/*********************************************************************
* Public: CanTouch.
* Return true if this agent is touching other
*********************************************************************/
bool Agent::CanTouch(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	ASSERT( other2.IsValid() );

	if (mySelf == other)
		return false;

	Box r1;
	GetAgentExtent(r1);

	Box r2;
	other.GetAgentReference().GetAgentExtent(r2);

	if( r1.left > r2.right || r2.left > r1.right )
		return false;
	if( r1.top > r2.bottom || r2.top > r1.bottom )
		return false;

	return true;
}








// virtual
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Agent::Write(CreaturesArchive &ar) const
{
	_ASSERT(!myGarbaged);

	int i;

	ar << (int)0;		// reserved for version hacking
	ar << myID;			// Unique ID.
	ar << myUpdateTick;
	myClassifier.Write(ar);
	ar << myAttributes;
	ar << myCreaturePermissions;
	ar << myMovementStatus;
	ar << myGarbaged;
	ar << myRunning;
	ar << myInputMask;
	ar << myTimer;
	ar << myTimerRate;
	ar << myDefaultClickAction;		// used by ClickAction()

	// display stuff
	ar << myEntityImage;


	ar << myCarriedAgent;
	ar << myCarrierAgent;

	ar << myGeneralRange;
	ar << myInvalidPosition;
	ar << myHighlightedFlag;
	ar << myHighlightColour.r << myHighlightColour.g << myHighlightColour.b;
	
	ar << myPointsWhereIamPickedUp;
	ar << myPointsIUseToPickUpAgents;


	// physics stuff
	ar << myPositionVector;
	ar << myFloatingPositionVector;
	ar << myVelocityVector;
	ar << myElasticity;
	ar << myFrictionPercentage;
	ar << myFrictionFactor;
	ar << myCollisionFactor;
	ar << myAeroDynamicFactor;	// % velocity lost due to media resistance.
	ar << myAeroDynamicPercentage;
	ar << myPermiability;		// Below which the agent cannot pass through.
	ar << myGravitationalAcceleration;	// Rate at which agent accelerates in pixels per tick.
	ar << myStoppedFlag;
	ar << myLastWallHit;
	ar << myFloatingWith;
	ar << myFloatees;

	ar << myCAIndex;		//Index of the CA property which this agent will effect
	ar << myCAIncrease;		//Amount by which the CA property will be increased
	ar << myCAIsNavigable;
	ar << myPrevCAIndex;
	ar << myPrevCAIncrease;
	ar << myPrevCAIsNavigable;
	ar << myPrevRoomWhenEmitting;
	// ar << myCAProcessingState;
	int tempState;
	if (myCAProcessingState == stateNotProcessing)
		tempState = 0;
	else if (myCAProcessingState == stateStableSettings)
		tempState = 1;
	else
		tempState = 2;
	ar << tempState;

	ar << myPixelPerfectHitTestFlag;
	ar << myClickActionState;
	for( i = 0; i < 3; ++i )
		ar << myDefaultClickActionCycles[i];

	ar << myVirtualMachines;
	ar << mySpareVirtualMachines;
	if( !myPorts.Write(ar) )
		return false;


	// agent variables... OV00-OV99
	// ugly code, but don't want overhead of 100 persistentobjects...
	ar << (int)GLOBAL_VARIABLE_COUNT;			// to allow future expansion
	for( i=0; i<GLOBAL_VARIABLE_COUNT; ++i )
		myGlobalVariables[i].Write( ar );
	ar << myNameVars;

	ar << myGenomeStore;

	ar << myAgentType;
	myVoice.Write(ar);
	ar << myImpendingDoom;
			
	ar << myCurrentWidth;
	ar << myCurrentHeight;
	ar << myResetLines;

	ar << myDrawMirroredFlag;
	ar << myDrawFlippedVerticallyFlag;


	// obsolete stuff, or stuff I'm not sure about (benc)
//	SOUNDHANDLE	mySoundHandle;	// Controlled Sound (-1=Not playing)
//	DWORD mySoundName;			// Name of active sound (0=none)
//	BOOL mySoundLooping;			// Controlled sound is looping

	ar << mySoundName;
	ar << mySoundLooping;

	// Serialise these if we want bounding box checks on
	ar << myWidthTemp << myHeightTemp;

	myPresence->Write(ar);

	ar << myDrawAlphaFlag;
	ar << myOverrideCategory;

	ar << myCoreWidth;
	ar << myCoreHeight;
	ar << myCorePositionVector;
	ar << myAlteredBoundingBox;
	ar << myShowCoreFlag;

	// bytecount - num of bytes to skip over (to allow extensions)
	ar << (int)0;		// leeway to allow extension hacking

	return true;
}



// virtual
bool Agent::Read( CreaturesArchive &ar )
{
	_ASSERT(!myGarbaged);

	int32 version = ar.GetFileVersion();
	int i;
	int varcount;
	int tmp_int;

	if(version >= 3)
	{
		
		ar >> tmp_int;
		if( tmp_int != 0 )
		{
			return false;
		}

		ar >> myID;			// Unique ID.

		ar >> myUpdateTick;
		
		if(!myClassifier.Read(ar))
			return false;

		ar >> myAttributes;
		ar >> myCreaturePermissions;
		ar >> myMovementStatus;
		ar >> myGarbaged;
		ar >> myRunning;
		ar >> myInputMask;
		ar >> myTimer;
		ar >> myTimerRate;
		ar >> myDefaultClickAction;		// used by ClickAction()

		// display stuff
		ar >> myEntityImage;

		ar >> myCarriedAgent;
		ar >> myCarrierAgent;

		ar >> myGeneralRange;
		ar >> myInvalidPosition;
		ar >> myHighlightedFlag;
		ar >> myHighlightColour.r >> myHighlightColour.g >> myHighlightColour.b;

		ar >> myPointsWhereIamPickedUp;
		ar >> myPointsIUseToPickUpAgents;

		// physics stuff
		ar >> myPositionVector;
		ar >> myFloatingPositionVector;
		ar >> myVelocityVector;
		ar >> myElasticity;
		ar >> myFrictionPercentage;
		ar >> myFrictionFactor;
		ar >> myCollisionFactor;
		ar >> myAeroDynamicFactor;	// % velocity lost due to media resistance.
		ar >> myAeroDynamicPercentage;
		ar >> myPermiability;		// Below which the agent cannot pass through.
		ar >> myGravitationalAcceleration;	// Rate at which agent accelerates in pixels per tick.
		ar >> myStoppedFlag;
		ar >> myLastWallHit;
		ar >> myFloatingWith;
		ar >> myFloatees;

		ar >> myCAIndex;
		ar >> myCAIncrease;
		ar >> myCAIsNavigable;
		ar >> myPrevCAIndex;
		ar >> myPrevCAIncrease;
		ar >> myPrevCAIsNavigable;
		ar >> myPrevRoomWhenEmitting;
		// ar >> myCAProcessingState;
		int tempState;
		ar >> tempState;
		if (tempState == 0)
			myCAProcessingState = stateNotProcessing;
		else if (tempState == 1)
			myCAProcessingState = stateStableSettings;
		else
			myCAProcessingState = stateSettingsChange;

		if( myCAIsNavigable )
		{
			myCAProcessingState = stateSettingsChange;
			myPrevCAIndex = -1;
			myPrevCAIncrease = 0.0f;
			myPrevCAIsNavigable = false;
			myPrevRoomWhenEmitting = -1;
		}

		ar >> myPixelPerfectHitTestFlag;
		ar >> myClickActionState;
		for( i = 0; i < 3; ++i )
			ar >> myDefaultClickActionCycles[i];

		if (version<15)
		{
			if( !GetVirtualMachine().Read(ar) )
				return false;
		}
		else 
		{
			ar >> myVirtualMachines;
			ar >> mySpareVirtualMachines;
		}


		if( !myPorts.Read(ar) )
			return false;

		ar >> varcount;
		CreaturesArchive::ForceOpenRange(varcount, 0, GLOBAL_VARIABLE_COUNT + 1);

		for( i=0; i<varcount; ++i )
			myGlobalVariables[i].Read( ar );
		
		if (version >= 21)
			ar >> myNameVars;

		ar >> myGenomeStore;

		ar >> myAgentType;

		if(!myVoice.Read(ar))
			return false;

		ar >> myImpendingDoom;
		ar >> myCurrentWidth;
		ar >> myCurrentHeight;
		ar >> myResetLines;

		ar >> myDrawMirroredFlag;

		if(version >= 26)
		{
			ar >> myDrawFlippedVerticallyFlag;
		}

		// obsolete stuff, or stuff I'm not sure about (benc)
	//	SOUNDHANDLE	mySoundHandle;	// Controlled Sound (-1=Not playing)
	//	DWORD mySoundName;			// Name of active sound (0=none)
	//	BOOL mySoundLooping;			// Controlled sound is looping

		ar >> mySoundName;
		ar >> mySoundLooping;

		ar >> myWidthTemp >> myHeightTemp;

		if (version >= 22)
		{
			myPresence->Read(ar);
		}

		if (version >= 25)
		{
			ar >> myDrawAlphaFlag;
		}

		if (version >= 27)
			ar >> myOverrideCategory;
		else
			myOverrideCategory = -1;

		if (version >=36)
		{
			ar >> myCoreWidth;
			ar >> myCoreHeight;
			ar >> myCorePositionVector;
			ar >> myAlteredBoundingBox;

			if (version >= 37)
			{
				ar >> myShowCoreFlag;
			}
			else
			{
				myShowCoreFlag = false;
			}
		}
		else
		{
			myCorePositionVector = myPositionVector;
			myCoreWidth = myCurrentWidth;
			myCoreHeight= myCurrentHeight;
			// myAlteredBoundingBox is suitably null already
		}

		// bytecount - num of bytes to skip over (to allow extensions)
		ar >> tmp_int;
		//	ar.Skip( tmp_int );

	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success!

	if (myMovementStatus == FLOATING)
	{
		theMainView.AddFloatingThing(mySelf);
	//		CameraPositionNotify();
	}

	// Reset camera shyness
	DoSetCameraShyStatus();

#ifndef C2D_DIRECT_DISPLAY_LIB
	DrawMirrored(IsMirrored());
#endif

	return true;
}





bool Agent::CanStartCarrying(AgentHandle& a)
{
	// We can only carry one agent at a time
	if (myCarriedAgent.IsInvalid())
		return true;
	else
		return false;
}


bool Agent::CanStopCarrying(AgentHandle& a, AgentHandle& v, bool& findSafePlace)
{
	findSafePlace = false;
	v = NULLHANDLE;
	Agent& agentref = a.GetAgentReference();
	if (agentref.TestAttributes(attrFloatable))
		return true;
	v = theAgentManager.WhatVehicleCouldIDropInto(a, mySelf);
	bool validVehicle = v.IsValid();

	if ((myAgentType & AgentHandle::agentPointer) != AgentHandle::agentPointer)
	{
		// Not being dropped by the pointer
		if (validVehicle && (v.GetAgentReference().GetMovementStatus() != Agent::FLOATING))
			return true;
		else
			v = NULLHANDLE;
		if (agentref.TestRoomSystemLocation())
			return true;
		findSafePlace = true;
		return true;
	}
	else
	{
		// Being dropped by the pointer
		if (validVehicle)
		{
			if ((a.IsCreature()) && 
				(v.GetAgentReference().GetMovementStatus() == Agent::FLOATING))
			{
				// Can't drop creatures into the inventory
				v = NULLHANDLE;
				return false;
			}
			return true;
		}
		if (agentref.TestRoomSystemLocation())
			return true;
		return false;
	}
}


void Agent::StartCarrying(AgentHandle& a)
{
	_ASSERT(!myGarbaged);

	Agent& agentref = a.GetAgentReference();

	// Create parent-child relationships
	agentref.SetCarrier(mySelf);
	myCarriedAgent = a;

	// Agent is now being carried
	agentref.SetMovementStatus(CARRIED);

	// Clear the velocity. This isn't strictly necessary
	// because agents can't move on their own when in the 
	// carried state, but it seems correct to clear it.
	agentref.SetVelocity(ZERO_VECTOR);

	// Move the picked-up agent to the appropriate carry point
	Vector2D p = GetMapCoordOfWhereIPickUpAgentsFrom() -
		agentref.GetMyOffsetOfWhereIAmPickedUpFrom();
	agentref.MoveTo(p.x, p.y);
	
	// Place the carried agent into the appropriate carry plane
	int32 plane = GetPlaneForCarriedAgent();
	if (plane != -1)
		agentref.ChangePhysicalPlane(plane);

	// Make the picked-up agent have the same camera-shy status as the
	// carrier 
	agentref.ChangeCameraShyStatus((myAttributes & attrCameraShy) == attrCameraShy);
	
	// Inform the new carrier that it has gained a carried agent
	ExecuteScriptForEvent(SCRIPGOTCARRIEDAGENT, a, INTEGERZERO, INTEGERZERO);

	if (myAgentType & AgentHandle::agentPointer)
	{
		// I'm the pointer and I'm picking up an agent, so run the agent's
		// script on the pointer's VM
		Classifier c = agentref.GetClassifier();
		c.SetEvent(SCRIPTPOINTERPICKUP);
		ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);
	}
}


void Agent::StopCarrying(AgentHandle& a)
{
	_ASSERT(!myGarbaged);

	Agent& agentref = a.GetAgentReference();
	
	// Break the parent-child relationship
	agentref.SetCarrier(NULLHANDLE);
	myCarriedAgent = NULLHANDLE;

	// Restore the agent to its default movement status
	if (agentref.TestAttributes(attrFloatable))
		agentref.SetMovementStatus(FLOATING);
	else
		agentref.SetMovementStatus(AUTONOMOUS);

	// Restore the agent to its default plane
	agentref.ChangePhysicalPlane(agentref.GetNormalPlane(0));

	// Restore the agent to its default camera-shy status
	agentref.ChangeCameraShyStatus(agentref.TestAttributes(attrCameraShy) == attrCameraShy);

	// Inform the old carrier that it has lost its carried agent
	ExecuteScriptForEvent(SCRIPTLOSTCARRIEDAGENT, a, INTEGERZERO, INTEGERZERO);

	if (myAgentType & AgentHandle::agentPointer)
	{
		// I'm the pointer and I'm dropping an agent, so run the agent's
		// script on the pointer's VM
		Classifier c = agentref.GetClassifier();
		c.SetEvent(SCRIPTPOINTERDROP);
		ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);


		if (((agentref.TestAttributes(attrSufferCollisions) & 
			Agent::attrSufferCollisions)
			== Agent::attrSufferCollisions) &&
			(agentref.GetMovementStatus() == AUTONOMOUS))
		{
			// Give mouse velocity to dropped agent, facilitates throwing 
			Vector2D v(theApp.GetInputManager().GetMouseVX(), 
			   theApp.GetInputManager().GetMouseVY());
			agentref.SetVelocity(v);
		}
		else 
		{
			// Stop moving
			agentref.SetVelocity(ZERO_VECTOR);
		}
	}
	else
		// Dropped agents stop moving by default
		agentref.SetVelocity(ZERO_VECTOR);
}



bool Agent::AddAPointWhereYouCanPickMeUp(int pose, int x, int y)
{
	_ASSERT(!myGarbaged);

	if (pose >= 0 && pose < myPointsWhereIamPickedUp.size())
	{
		myPointsWhereIamPickedUp[pose].x = x;	// Indexed by absolute pose
		myPointsWhereIamPickedUp[pose].y = y;	// Indexed by absolute pose
		return true;
	}
	else if (pose == -1)
	{
		for (int i = 0; i < myPointsWhereIamPickedUp.size(); ++i)
		{
			myPointsWhereIamPickedUp[i].x = x;	// Indexed by absolute pose
			myPointsWhereIamPickedUp[i].y = y;	// Indexed by absolute pose
		}
		return true;
	}
	else
		return false;
}

bool Agent::AddAPointWhereIPickAgentsUp(int pose, int x, int y, int part /*=0*/)
{
	_ASSERT(!myGarbaged);

	if (pose >= 0 && pose < myPointsIUseToPickUpAgents.size())
	{
		myPointsIUseToPickUpAgents[pose].x = x;	// Indexed by absolute pose
		myPointsIUseToPickUpAgents[pose].y = y;	// Indexed by absolute pose
		return true;
	}
	else if (pose == -1)
	{
		for (int i = 0; i < myPointsIUseToPickUpAgents.size(); ++i)
		{
			myPointsIUseToPickUpAgents[i].x = x;	// Indexed by absolute pose
			myPointsIUseToPickUpAgents[i].y = y;	// Indexed by absolute pose
		}
		return true;
	}
	else
		return false;
}

bool Agent::DoBoundsIntersect(const Box& rect)
{
	_ASSERT(!myGarbaged);

	Box intersectRect;
	Box bound;
	Box actrect(rect);

	GetAgentExtent(bound);

	return intersectRect.IntersectRect(actrect, bound) ? true : false;
}

bool Agent::HitTest(const Vector2D& point,
					bool alwaysTestPixelPerfectly /* = false*/)
{
	_ASSERT(!myGarbaged);

	Box rect;
	GetAgentExtent(rect);           // otherwise get its boundary
	int px = FastFloatToInteger(point.x);
	int py = FastFloatToInteger(point.y);

				
	if (rect.PointInBox(point))
	{
		if(myPixelPerfectHitTestFlag || alwaysTestPixelPerfectly)
		{
			if(myEntityImage)
			{
				return !(myEntityImage->IsPixelTransparent(px,py));
			}
		}
		else
		{
			return true;
		}
	}
	return false;
}


void Agent::SetGallery(FilePath const& galleryName, int baseimage, int part)
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
	{
		if(myEntityImage->GetGalleryName() != galleryName.GetFullPath())
		{
			myEntityImage->Unlink();
			// we are changing gallerries so we must tell the entity
			myEntityImage->SetGallery(galleryName,baseimage);
			myEntityImage->Link();
			DoSetCameraShyStatus();
			UpdateFromPresence();
		}
	}

}


void Agent::DrawLine(
				Vector2D& startPoint,
				Vector2D& directionAndLength,
				uint8 lineColourRed /*= 255*/,
				uint8 lineColourGreen /*= 255*/,
				uint8 lineColourBlue /*= 255*/,
				uint8 stippleon /* =0*/,
				uint8 stippleoff/* = 0*/,
				uint32 stippleStart/* = 0*/) 
{
	Vector2D endPoint(startPoint+directionAndLength);
	DrawLine(FastFloatToInteger(startPoint.x), FastFloatToInteger(startPoint.y), FastFloatToInteger(endPoint.x), FastFloatToInteger(endPoint.y),
		lineColourRed, lineColourGreen, lineColourBlue,
		stippleon, stippleoff, stippleStart);
}

void Agent::GetGallery(std::string& string, int part)
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
	{
		string = myEntityImage->GetGalleryName();
	}
}


// virtual
void Agent::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stippleStart) 
{
	_ASSERT(!myGarbaged);
}


// virtual
void Agent::ResetLines()
{
	_ASSERT(!myGarbaged);
}

bool Agent::SetAttributes(uint32 attributes)
{ 
	_ASSERT(!myGarbaged);

	myAttributes = attributes;

	if (TestAttributes(attrSufferCollisions) && !myInvalidPosition) 
	{
		bool ok = TestCurrentLocation(true);
		if (!ok) {
			return false;
		}
	}
	if (TestAttributes(attrFloatable)) {
		if (GetMovementStatus() == AUTONOMOUS)
			SetMovementStatus(FLOATING);
	}
	else {
		if (GetMovementStatus() == FLOATING)
			SetMovementStatus(AUTONOMOUS);
	}

	if (TestAttributes(attrSufferPhysics))
		myStoppedFlag = false;

	DoSetCameraShyStatus();
	return true;
}

void Agent::DoSetCameraShyStatus()
{
	ChangeCameraShyStatus((myAttributes & attrCameraShy) == attrCameraShy);
}

void Agent::ChangeCameraShyStatus(bool shy)
{
	if (myEntityImage)
		myEntityImage->YouAreCameraShy(shy);
}

void Agent::SetMovementStatus(int type)
{ 
	_ASSERT(!myGarbaged);

	if (myMovementStatus != type)
	{
		if (myMovementStatus == FLOATING)
			theMainView.RemoveFloatingThing(mySelf);

		myMovementStatus = type;

		if (myMovementStatus == FLOATING)
		{
			myFloatingPositionVector = myPositionVector;
			WorldToFloatRelative(myFloatingPositionVector);
			theMainView.AddFloatingThing(mySelf);
		}
	}
}

void Agent::CameraPositionNotify()
{
	_ASSERT(!myGarbaged);

	if (myMovementStatus == FLOATING)
	{
		// We move here, as well as in the camera.  This is to be
		// sure our position is reasonably up to date if another 
		// script looks at it in the meanwhile
		Vector2D myNewPos = myFloatingPositionVector;
		FloatRelativeToWorld( myNewPos );
		MoveTo(myNewPos.x, myNewPos.y);
	}
	else
	{
		ASSERT(false);
	}
}

void Agent::FloatTo(const Vector2D& floatpos)
{
	_ASSERT(!myGarbaged);
	ASSERT(GetMovementStatus() == FLOATING);
	myFloatingPositionVector = floatpos;
	WorldToFloatRelative(myFloatingPositionVector);
	CameraPositionNotify();
}


void Agent::MarkValidPositionNow()
{
	_ASSERT(!myGarbaged);
	myInvalidPosition = false;
}


void Agent::HandleRunError(CAOSMachineRunError& e)
{
	_ASSERT(!myGarbaged);

	std::ostringstream out;

	// agent classifier
	out << theCatalogue.Get("runtime_error", 4);
	myClassifier.StreamClassifier(out, false);
	myClassifier.StreamAgentNameIfAvailable(out);
	out << " ";

	// script classifier
	if (GetVirtualMachine().GetScript())
	{
		out << theCatalogue.Get("runtime_error", 7) << " ";
		GetVirtualMachine().StreamClassifier(out);
		out << " ";
	}

	// unique id
	out << theCatalogue.Get("runtime_error", 5) << " " << GetUniqueID() << " ";

	// message
	out << std::endl << e.what();
	
	// script position
	if (GetVirtualMachine().GetScript())
	{
		out << std::endl;
		GetVirtualMachine().StreamIPLocationInSource(out);
	}

	// time/date/user/version
	out << std::endl << std::endl << ErrorMessageHandler::ErrorMessageFooter();
	out << '\0';

	int ret;
	bool camera;

	theFlightRecorder.Log(FLIGHT_RUNTIME, theCatalogue.Get("runtime_error_log", 0));
	theFlightRecorder.Log(FLIGHT_RUNTIME,"%s",out.str().c_str());

	if (theApp.AutoKillAgentsOnError())
	{
		ret = RuntimeErrorDialog::RED_KILL;
		camera = false;
	}
	else
	{
		RuntimeErrorDialog dlg;
		dlg.SetText(std::string(out.str()));
		dlg.SetCameraPossible(GetVirtualMachine().GetScript() != NULL);
		ret = dlg.DisplayDialog();
		camera = dlg.myCameraChecked;
	}

	if (camera && GetVirtualMachine().GetScript())
	{
		Vector2D centre = GetCentre();
		DisplayHandlers::CameraHelper(GetVirtualMachine(), FastFloatToInteger(centre.x), 
			FastFloatToInteger(centre.y), 0, true, NULL);
	}

	if (ret == RuntimeErrorDialog::RED_FREEZE)
	{
		theFlightRecorder.Log(FLIGHT_RUNTIME,theCatalogue.Get("runtime_error_log", 1));
		myRunning = false;
	}
	else if (ret == RuntimeErrorDialog::RED_KILL)
	{
		if (theApp.AutoKillAgentsOnError())
			theFlightRecorder.Log(FLIGHT_RUNTIME,theCatalogue.Get("runtime_error_log", 5));
		else
			theFlightRecorder.Log(FLIGHT_RUNTIME,theCatalogue.Get("runtime_error_log", 2));

		if( mySelf != thePointer)
		{
			myImpendingDoom = true;
			StopAllScripts();
			return;
		}
	}
	else if (ret == RuntimeErrorDialog::RED_PAUSEGAME)
	{
		theFlightRecorder.Log(FLIGHT_RUNTIME,theCatalogue.Get("runtime_error_log", 3));
		SetGameTicking(false);
	}
	else if (ret == RuntimeErrorDialog::RED_STOPSCRIPT)
	{
		theFlightRecorder.Log(FLIGHT_RUNTIME,theCatalogue.Get("runtime_error_log", 6));
		StopAllScripts();
	}
	else if (ret == RuntimeErrorDialog::RED_IGNORE)
	{
		theFlightRecorder.Log(FLIGHT_RUNTIME,theCatalogue.Get("runtime_error_log", 4));
	}
	else
		ASSERT(false);
}

//These two functions are mutually exclusive an agent either cliks
// or clacs
// clic allows the agent engineer to set up a short cycle of activations
// for the left mouse button

	// set the default clickaction
void Agent::SetDefaultClickAction( int msgid )
{
	_ASSERT(!myGarbaged);

	myDefaultClickAction = msgid; 
	
	// minus one here means don't do any cycling
	myClickActionState =-1;
}


void Agent::SetDefaultClickActionCycle(int i0, int i1, int i2)
{
	_ASSERT(!myGarbaged);

	myClickActionState = 0;
	myDefaultClickActionCycles[0] = i0;
	myDefaultClickActionCycles[1] = i1;
	myDefaultClickActionCycles[2] = i2;
}


void Agent::DecideClickActionByState()
{
	_ASSERT(!myGarbaged);

	// if this value is minus one then ignore this
	// command
	if(myClickActionState !=-1)
	{
		// please make sure that this has been initialized correctly
		// otherwise 
		// we'll get a never ending loop
		myDefaultClickAction = -1;
		// skip over any unused slots
		while (myDefaultClickAction == -1)
		{

		myDefaultClickAction = myDefaultClickActionCycles[myClickActionState];
			myClickActionState++;

			if(myClickActionState >= NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS)
			{
				myClickActionState =0;
			}

		}
	}
	
}

// undo the last action this is useful if the drop command
// was sent but was disallowed
void Agent::GoBackToLastClickActionState()
{
	// if this value is minus one then ignore this
	// command
	if(myClickActionState !=-1)
	{
		if(myClickActionState > 0)
		{
			
			myClickActionState--;
		}
		else
		{
			myClickActionState = NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS - 1;
		}
	}

}

void Agent::BreakLinksToOtherAgents()
{
	_ASSERT(!myGarbaged);

	// Disconnect us from anything else
	myPorts.KillAllConnections();

	// Make sure we're not being carried
	if ( myCarrierAgent.IsValid() )
	{
		TryToBeDropped(mySelf, INTEGERZERO, INTEGERZERO, true);
	}

	// Make sure we're not carrying anything
	if ( myCarriedAgent.IsValid() )
	{
		myCarriedAgent.GetAgentReference().TryToBeDropped
			(mySelf, INTEGERZERO, INTEGERZERO, true);
	}

	// Make sure we're not floating with anyone, and noone is floating with us
	DetachFloatingWith();
	while(!myFloatees.empty())
	{
		if (myFloatees[0].IsValid())
			myFloatees[0].GetAgentReference().DetachFloatingWith();
		else
			myFloatees.erase(myFloatees.begin());
	}
}

void Agent::DetachFloatingWith()
{
	if (myFloatingWith.IsValid())
	{
		Agent& with = myFloatingWith.GetAgentReference();
		with.myFloatees.erase( std::remove(with.myFloatees.begin(), with.myFloatees.end(), mySelf),
			with.myFloatees.end());
		myFloatingWith = NULLHANDLE;
	}
}

void Agent::AttachFloatingWith(AgentHandle& with)
{
	DetachFloatingWith();
	if (with.IsValid())
	{
		myFloatingWith = with;
		with.GetAgentReference().myFloatees.push_back(mySelf);
	}
}


void Agent::WorldToFloatRelative(Vector2D& vec)
{
	if (myFloatingWith.IsValid())
	{
		Agent& agent = myFloatingWith.GetAgentReference();
		vec.x = vec.x - agent.myPositionVector.x;
		vec.y = vec.y - agent.myPositionVector.y;
	}
	else
		theMainView.WorldToScreen(vec);
}

void Agent::FloatRelativeToWorld(Vector2D& vec)
{
	if (myFloatingWith.IsValid())
	{
		Agent& agent = myFloatingWith.GetAgentReference();
		vec += agent.myPositionVector;
	}
	else
		theMainView.ScreenToWorld(vec);
}

bool Agent::IsThereAScript(int event)
{
	Classifier c(myClassifier);
	c.SetEvent(event);
	return theApp.GetWorld().GetScriptorium().FindScript(c) ? true : false;
}

bool Agent::SetCreaturePermissions(uint32 i)
{
	_ASSERT(!myGarbaged);

	// check all the scripts are there
	
	if ((i & permCanActivate1) && !IsThereAScript(SCRIPTACTIVATE1))
		return false;

	if ((i & permCanActivate2) && !IsThereAScript(SCRIPTACTIVATE2))
		return false;

	if ((i & permCanDeactivate) && !IsThereAScript(SCRIPTDEACTIVATE	))
		return false;

// Don't check pickup - there are often scripts without it, when 
// the creature can pick the agent up
//	if ((i & permCanPickUp) && !IsThereAScript(SCRIPTPICKUP))
//		return false;

	if ((i & permCanHit) && !IsThereAScript(SCRIPTHIT))
		return false;

	if ((i & permCanEat) && !IsThereAScript(SCRIPTEAT))
		return false;

	myCreaturePermissions = i;
	return true;
}

// turn mirroring for the current sprite on or off
void Agent::DrawMirrored(bool mirror)
{
	if(myEntityImage)
	{
		myEntityImage->DrawMirrored(mirror);
		myDrawMirroredFlag = mirror;
	}

}


Vector2D Agent::GetCentre()
{
	_ASSERT(!myGarbaged);
	return Vector2D
		(myPositionVector.x+myCurrentWidth/2.0f, 
		myPositionVector.y+myCurrentHeight/2.0f);
};

void Agent::UpdateFromPresence()
{
	//gtb! fai!
	if (TestAttributes(attrRotatable) || TestAttributes(attrPresence))
	{
		myPresence->Update();
		SetBaseImage(myPresence->GetPoseOffset());
	}
}



#ifdef CODE_BEING_WORKED_ON_BY_GTB_BUT_NOT_READY_YET
void FindByRealisticSightAndFGS(
	AgentHandle const& viewer,
	AgentList& agents,
	const Classifier& c );

//gtb!temp!
void FindByRealisticSightAndFGS(
	AgentHandle const& viewer,
	AgentList& agents,
	const Classifier& c )
{
	// get agents in our area (not including viewer thankfully):
	theAgentManager.FindBySightAndFGS(viewer, agents, c);
	AgentListIterator it, nextit;


	// Set up some viewer stats (where we are, what angle we're looking:
	Agent& v = viewer.GetAgentReference();
	Vector2D viewerEye = v.GetCentre();
	Angle viewerFacingAngle = v.GetPresence().GetFacingAngle();
	float viewArcSize = v.GetPresence().GetViewArcSize().GetAsFraction();// note this is half the actual viewing size!


	//temp!
/*	Circle::agentTemp = &v;
	v.ResetLines();
	Vector2D dir;
	dir = viewerFacingAngle.GetRotationBy(Angle(viewArcSize)).GetVector()*50.0f;
	v.DrawLine(viewerEye, dir,80);
	dir = viewerFacingAngle.GetRotationBy(Angle(1.0f-viewArcSize)).GetVector()*50.0f;
	v.DrawLine(viewerEye, dir,80);
	dir = viewerFacingAngle.GetVector()*25.0f;
	v.DrawLine(viewerEye, dir,0,0,255);*/

	// prune only those in the viewing arc
	for (nextit,it=agents.begin(); it!=agents.end(); it=nextit)
	{
		nextit = it;// so that we can safely erase 'it'
		++nextit;

		const Presence& p = (*it).GetAgentReference().GetPresence();

		// if outside the viewing arc completely:
		if (!p.IntersectsWithViewingArc(viewerEye, viewerFacingAngle, viewArcSize))
		{
			// erase it from the list:
			agents.erase(it);
		}
	}


	// prune those occluded:
	for (nextit, it=agents.begin(); it!=agents.end(); it=nextit)
	{
		nextit=it;
		++nextit;

		Agent& itAgent = (*it).GetAgentReference();

		// Check whether this itAgent is occluded by any of the others:
		for (AgentListIterator othernext, other=agents.begin(); other!=agents.end(); other=othernext)
		{
			othernext = other;// so that we can safely erase 'it'
			++othernext;

			if (*other==*it)
				continue;

			Agent& otherAgent = (*other).GetAgentReference();
			if (itAgent.GetPresence().
				IsOccludedByPresence(viewerEye, otherAgent.GetPresence()))
			{
				// erase current it and advance to the next:
				agents.erase(it);
				break;
			}
		}
	}
}
#endif//CODE_BEING_WORKED_ON_BY_GTB_BUT_NOT_READY_YET



#ifdef C2D_DIRECT_DISPLAY_LIB
void Agent::CreateAnimStrip(int blockwidth, int blockheight, int part)
{

	if(myEntityImage)
	{
		myEntityImage->CreateAnimStrip(blockwidth,blockheight);
		SetWidth(myEntityImage->GetWidth());
		SetHeight(myEntityImage->GetHeight());

	}
}

void Agent::DrawFlippedVertically(bool flip)
{
	if(myEntityImage)
	{
		// must override any other effects
		DrawMirrored(false);
		myEntityImage->DrawFlippedVertically(flip);
		myDrawFlippedVerticallyFlag = flip;
	}

}

uint16 Agent::GetSpriteRotationAngle()
{
	if(myEntityImage)
	{
		return myEntityImage->GetSpriteRotationAngle();
	}

	return 0;
}

void Agent::SetSpriteRotationAngle(uint16 angle)
{
	if(myEntityImage)
	{
		// must override any other effects
		DrawMirrored(false);
		DrawFlippedVertically(false);
		myEntityImage->SetSpriteRotationAngle(angle);
	}
}

#endif

void Agent::Scale(float scaleby, bool onOrOff)
{
	if(myEntityImage)
		myEntityImage->Scale(scaleby, onOrOff);
}

void Agent::DrawAlphaBlended(int alphamask, bool onOrOff, int part )
{
	if(myEntityImage)
		myEntityImage->DrawAlphaBlended(alphamask,onOrOff);
}


void Agent::DrawShadowed(int shadowvalue, int x, int y, bool onOrOff)
{
	if(myEntityImage)
		myEntityImage->DrawShadowed(shadowvalue, x, y, onOrOff);
}

void Agent::DrawToNewWidthAndHeight(int stretchedWidth, int stretchedHeight, bool onOrOff)
{
	if(myEntityImage)
		myEntityImage->DrawToNewWidthAndHeight(stretchedWidth, stretchedHeight,onOrOff);
}

CAOSVar& Agent::GetNameVar( const CAOSVar& name )
{
	return( myNameVars[ name ] );
}

void Agent::GetNextNameVar( CAOSVar& name )
{
	// Special case of empty string, find the first value
	std::string moose;
	if (name.GetType() == CAOSVar::typeString)
		name.GetString(moose);
	if ((
			((name.GetType() == CAOSVar::typeString) && (moose == std::string("")))
		)
		&& myNameVars.size() > 0)
	{
		name = myNameVars.begin()->first;
		return;
	}

	// Search for entry after the one passed in
	std::map< CAOSVar, CAOSVar >::iterator it;
	it = myNameVars.find(name);
	if (it != myNameVars.end())
		it++;

	if (it == myNameVars.end())	
		name.SetString(std::string(""));
	else 
		name = it->first;
}

void Agent::DeleteNameVar( const CAOSVar& name )
{
	std::map< CAOSVar, CAOSVar >::iterator it;
	it = myNameVars.find( name );
	if( it != myNameVars.end() )
		myNameVars.erase( it );
}


void Agent::SetOverrideCategory(int category)
{
	myOverrideCategory = category;
}

int Agent::GetOverrideCategory()
{
	return myOverrideCategory;
}

