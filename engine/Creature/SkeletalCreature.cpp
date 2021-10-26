#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SkeletalCreature.h"


#include "../App.h"
#include "../World.h"
#include <fstream>
#include <string>
#include "../Display/ErrorMessageHandler.h"
#include "Biochemistry/BiochemistryConstants.h"
#include "../Agents/Vehicle.h"
#include "../Display/TintManager.h"
#include "../Camera/MainCamera.h" // when we update plot order we need to tell the display system
#include "CreatureHead.h"
#include "LifeFaculty.h"
#include "ReproductiveFaculty.h"
#include "SensoryFaculty.h"
#include "../Agents/PointerAgent.h"
//#ifdef C2D_DIRECT_DISPLAY_LIB

//#else
#include "../Display/CreatureGallery.h"

//#endif






CREATURES_IMPLEMENT_SERIAL( SkeletalCreature )



// HACK gtb!temp!
bool SkeletalCreature::SetBaseImage( int image, int partid )
{
	return true;
}



/*********************************************************************
* Public: CanSee.
* Return true if this agent can see another
*********************************************************************/
bool SkeletalCreature::CanSee(const AgentHandle& other)
{
	// create other2 because there's no const IsValid function
	// i.e. one which tests the AgentHandle validity but does
	// not NULL the pointer if need be:
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	_ASSERT(other2.IsValid());

	if (!base::CanSee(other))
		return false;
	Agent& agentref = other2.GetAgentReference();

	// Can't see invisible things
	if (agentref.TestAttributes(attrInvisible))
		return false;

	Vector2D pressed(agentref.WhereShouldCreaturesPressMe());
	// too high?
	if (pressed.y < myPositionVector.y-myCurrentHeight)
		return false;
	// too low?
	if (pressed.y > myPositionVector.y+2.0f*myCurrentHeight)
		return false;

	// if you're not in the same root vehicle you can't see the agent:
	if (GetRootVehicle()!=agentref.GetRootVehicle())
		return false;

	return true;
}


bool SkeletalCreature::ValidatePart(int partid)
{
	_ASSERT(!myGarbaged); 
	return true;
}



bool SkeletalCreature::GetCreatureRoomID(int &roomID)
{
	Map& map = theApp.GetWorld().GetMap();
	return map.GetRoomIDForPoint(myDownFootPosition, roomID);
}




// to facilitate creature drowning:
const int WATER_ROOM_TYPE_1 = 8;
const int WATER_ROOM_TYPE_2 = 9;



// retn message# (ACTIVATE1 etc) appropriate for a click at a given
// position, else return -1
// Currently set up so that a click over ANY SkeletalCreature's head will
// cause ACTIVATE1 = pleasure (pat), and over his body will
// cause DEACTIVATE = pain (slap)
int SkeletalCreature::ClickAction(int x, int y)
{
	_ASSERT(!myGarbaged);

	// if the default action is holding hands then
	// stop right here
	if ((myDefaultClickAction == START_HOLD_HANDS) || (myDefaultClickAction == STOP_HOLD_HANDS))
		return myDefaultClickAction;

    Vector2D pt(x, y);
	Box r;
	RECT rr;

	GetAgentExtent(r);

    // Teaching pointer.  Is click over head?
    myLimbs[BODY_LIMB_HEAD]->GetBound(rr);
	if( y < (r=rr).CenterPoint().y )
        return ACTIVATE1;

    // Is click over legs?
    myLimbs[BODY_LIMB_LEFT_LEG]->GetBound(rr);
	if( (r = rr).PointInBox( pt ) )
        return DEACTIVATE;

    myLimbs[BODY_LIMB_RIGHT_LEG]->GetBound(rr);
	if( (r = rr).PointInBox( pt ) )
        return DEACTIVATE;

    return INVALID;
}


// where SkeletalCreatures should reach out to when trying to Activate you.
Vector2D SkeletalCreature::WhereShouldCreaturesPressMe() 
{
	_ASSERT(!myGarbaged);
	return myExtremes[BODY_LIMB_HEAD];
}






///////////////////////////////////////////////////////////////////////////////
//  Initialisation:



// Serialisation constructor
SkeletalCreature::SkeletalCreature()
	: Creature(),
	myTintin(NULL),
	myCompositeBodyPartGallery(NULL),
	myCompositeBodyPartGalleryForNextAgeStage(NULL)
{
	SetupDefaults();								// set defaults
}

// Destructor.
SkeletalCreature::~SkeletalCreature()
{
}

// 'Birth' constructor. 
// Called during "new: crea <moniker> <sex>" macro to create a new SkeletalCreature.
SkeletalCreature::SkeletalCreature(	int family, uint32 id,
				    AgentHandle gene, int gene_index,	// your genome
					int   sex,		// your sex 0=randomly choose 1=male 2=female
					int   variant)  // 0 means choose randomly from 1 to NUM_BEHAVIOUR_VARIANTS)
	: Creature(family, id, gene, gene_index, sex, variant), // construct creature
	myTintin(NULL),
	myCompositeBodyPartGallery(NULL),
	myCompositeBodyPartGalleryForNextAgeStage(NULL)
{
	try
	{
		SetupDefaults();							// set defaults

		Init();
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
		return;
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0014: Unknown exception caught in SkeletalCreature constructor";
		return;
	}
}


// Express any genes relevant to current age:
bool SkeletalCreature::ReadFromGenome()
{
	_ASSERT(!myGarbaged);

	base::ReadFromGenome();
	try
	{
		Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

		if(!ExpressParticularGenes(g, Life()->GetAge()))
			return false;
	}
	catch(Genome::GenomeException& e)
	{
		ErrorMessageHandler::Show(e, std::string("SkeletalCreature::ReadFromGenome"));
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// LOCI FACULTY:


bool SkeletalCreature::FormBodyParts()
{
	Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());
	if(!ExpressParticularGenes(g, Life()->GetAge()))
		return false;
	return true;
}







#define LOC_MUSCLES_INCREASE (1.0f/256.0f)


// constants related to 'reach out and touch' grid:
const int REACHGRIDXPOSES = 3;	// # horizontal distances can reach to
const int REACHGRIDYPOSES = 4;	// # vertical distances can reach to
const int NUMVARIANTS = 26;


typedef std::pair<int,int> IntegerPair;
typedef std::list<IntegerPair> IntegerPairList;
typedef IntegerPairList::iterator IntegerPairListIterator;

inline IntegerPair PairUp(int a, int b)
{
	return std::make_pair(a,b);
}




void SkeletalCreature::HoldHandsWithThePointer()
{
	// Sanity Check...
	if (thePointer.GetPointerAgentReference().GetHandHeldSkeletalCreature() != mySelf)
	{
		StopHoldingHandsWithThePointer(mySelf,INTEGERZERO,INTEGERZERO);
		return;
	}
	// If falling, don't need to make any effort to move towards pointer
	if (!myStoppedFlag)
		return;

	// Get the location of the physical mouse
	int x = theApp.GetInputManager().GetMouseX();
	int dummy=0;
	// Convert to world coords
	theMainView.ScreenToWorld(x, dummy);

	static const float shift = 5.0f;

	float px = (float)x;
	float delta;

	if ((px >= myDownFootPosition.x-30.0f) && 
		(px <= myDownFootPosition.x+30.0f))
	{
		myStandStill = true;
		return;
	}

	myStandStill = false;
	if (px < myDownFootPosition.x)
	{
		SetDirection(WEST);
		delta = myDownFootPosition.x - px;
		if (delta > shift)
			delta = shift;
		delta = -delta;
	}
	else
	{
		SetDirection(EAST);
		delta = px - myDownFootPosition.x; 
		if (delta > shift)
			delta = shift;
	}

	Vector2D downFootNew;
	bool collision, footChange, fall;
	int wall;
	theApp.GetWorld().GetMap().ShiftCreatureAlongFloor
		(myDownFootPosition, myUpFootPosition, myMinX, myMaxX, myMinY,
			myPermiability, delta, downFootNew,
			collision, wall, footChange, fall);

	myDownslopeLocus = 0.0f;
	myUpslopeLocus = 0.0f;

	if (footChange)
	{
		if (myCurrentDownFoot == LEFT)
			myCurrentDownFoot = RIGHT;
		else
			myCurrentDownFoot = LEFT;
	}
	myDownFootPosition = downFootNew;
	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();

	if (fall) 
	{
		myStoppedFlag = false;
		myVelocityVector = ZERO_VECTOR;
	}

	if (collision) 
	{
		myLastWallHit = wall;
		ExecuteScriptForEvent(SCRIPTBUMP, mySelf, INTEGERZERO, 
			INTEGERZERO);
	}
}


void SkeletalCreature::StartHoldingHandsWithThePointer
	(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2)
{
	if (myIsHoldingHandsWithThePointer)
		return;

	if (!Life()->GetWhetherAlert() || Life()->GetWhetherZombie() || (myMovementStatus == INVEHICLE))
		return;

	myDoubleSpeedFlag = true;
	myIsHoldingHandsWithThePointer = true;
	myLifeFaculty->SetWhetherZombie(true);	
	myDefaultClickAction = STOP_HOLD_HANDS;
	SetAnimationString(myGaitTable[0]);
	thePointer.GetPointerAgentReference().StartHoldingHandsWithSkeletalCreature(mySelf);
	ExecuteScriptForEvent(SCRIPTSTARTHOLDHANDS, from, p1, p2);
}


void SkeletalCreature::HandleStartHoldHands(Message* Msg)
{
	if (Msg->GetFrom().GetType() == CAOSVar::typeAgent)
		StartHoldingHandsWithThePointer(Msg->GetFrom().GetAgent(), Msg->GetP1(), Msg->GetP2());
}


void SkeletalCreature::StopHoldingHandsWithThePointer
	(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2)
{
	// If I'm not currently holding hands, then there's nothing to do
	if (!myIsHoldingHandsWithThePointer)
		return;
	myIsHoldingHandsWithThePointer = false;
	myDoubleSpeedFlag = false;
	myStandStill = false;
	myLifeFaculty->SetWhetherZombie(false);
	// Make the pointer stop holding hands with me
	_ASSERT(mySelf == thePointer.GetPointerAgentReference().GetHandHeldSkeletalCreature());
	thePointer.GetPointerAgentReference().StopHoldingHandsWithSkeletalCreature();
	ExecuteScriptForEvent(SCRIPTSTOPHOLDHANDS, from, p1, p2);
}


void SkeletalCreature::HandleStopHoldHands(Message* Msg)
{
	if (Msg->GetFrom().GetType() == CAOSVar::typeAgent)
		StopHoldingHandsWithThePointer(Msg->GetFrom().GetAgent(), Msg->GetP1(), Msg->GetP2());
}









///////////////////////////////////////////////////////////////////////////////
// LOCI FACULTY:

// respond to an enquiry by a biochemical Receptor/Emitter object for its locus address
float* SkeletalCreature::GetLocusAddress(int type,		// ID relates to RECEPTOR or EMITTER locus?
							 int organ,
							 int tissue,		// tissue containing locus
							 int locus) {		// locus on object
	float* baseLocus = base::GetLocusAddress(type, organ, tissue, locus);
	if (baseLocus!=&myInvalidLocus)
		return baseLocus;

	if (organ==ORGAN_CREATURE)
	{
		if (type==RECEPTOR)
		{
			// Receptor loci (locus is modulated according to chemical concentration)
			switch (tissue)
			{
				case TISSUE_SENSORIMOTOR:
					if (locus>=LOC_GAIT0 && locus<=LOC_GAIT15)
						return &myGaitLoci[locus-LOC_GAIT0];
					break;
			}
		}
		else
		{
 			// Emitter loci (locus is a source of chemical emission)
			switch (tissue)
			{							// according to tissue...
				case TISSUE_SOMATIC:
					if (locus==LOC_MUSCLES)
						return &myMusclesLocus;
					break;

				case TISSUE_SENSORIMOTOR:
					if(locus>=LOC_E_GAIT0 && locus<=LOC_E_GAIT15)
						return &myGaitLoci[locus-LOC_E_GAIT0];

					switch (locus)
					{
						case LOC_UPSLOPE:
							return &myUpslopeLocus;
						case LOC_DOWNSLOPE:
							return &myDownslopeLocus;
					}
					break;
			}
		}
	}

    // Unrecognised ID.
	return &myInvalidLocus;
}











// virtual
bool SkeletalCreature::MoveToSafePlaceGivenCurrentLocation()
{
	if (!myBody)
		// If body is not present, then we've been trashed
		return true;

	Vector2D downFootPosition;
	bool ok = theApp.GetWorld().GetMap().FindSafeCreatureLocation
		(myExtremes[BODY_LIMB_LEFT_LEG], 
		 myExtremes[BODY_LIMB_RIGHT_LEG], 
		 myMinY, myCurrentDownFoot, myPermiability, downFootPosition);
	if (ok) 
	{
		myDownFootPosition = downFootPosition;
		UpdatePositionsWithRespectToDownFoot();
		CommitPositions();
		// Creatures can never be directly placed on the floor - they
		// must always fall to the floor.
		myStoppedFlag = false;
	}
	return ok;
}


// virtual
bool SkeletalCreature::MoveToSafePlace(const Vector2D& positionPreferred)
{
	Vector2D downFootPosition;
	Vector2D delta = positionPreferred - myDownFootPosition;

	bool ok = theApp.GetWorld().GetMap().FindSafeCreatureLocation
		(myExtremes[BODY_LIMB_RIGHT_LEG] + delta, 
		myExtremes[BODY_LIMB_RIGHT_LEG] + delta, 
		myMinY + delta.y, myCurrentDownFoot, 
		myPermiability, downFootPosition);

	if (ok) 
	{
		myDownFootPosition = downFootPosition;
		UpdatePositionsWithRespectToDownFoot();
		CommitPositions();
		// Creatures can never be directly placed on the floor - they
		// must always fall to the floor.
		myStoppedFlag = false;
	}
	return ok;
}

void SkeletalCreature::MoveTo(float x,float y)
{
    

	myUpFootPosition = Vector2D(-9876.0f, -9876.0f);
	myPreviousMinY = -9876.0f;
	myCurrentDownFoot = LEFT;
	myDownFootPosition = ZERO_VECTOR;
	UpdatePositionsWithRespectToDownFoot();
	Vector2D r(myExtremes[BODY_LIMB_RIGHT_LEG]);
	if (r.y > 0.0f) {
		// The right foot is the down foot
		myCurrentDownFoot = RIGHT;
		myDownFootPosition = r;
	}
	myDownFootPosition.x += x - myPositionVector.x; 
	myDownFootPosition.y += y - myPositionVector.y; 
	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();
	// Creatures can never be directly placed on the floor - they
	// must always fall to the floor.
	myStoppedFlag = false;
}


Vector2D SkeletalCreature::GetMouthMapPosition()
{
	CreatureHead* head = (CreatureHead*)myLimbs[BODY_LIMB_HEAD];
	return head->GetMouthMapPosition();
}

void SkeletalCreature::MoveBy(float dx,float dy)
{
	myDownFootPosition.x += dx;
	myDownFootPosition.y += dy;
	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();
	// Creatures can never be directly placed on the floor - they
	// must always fall to the floor.
	myStoppedFlag = false;
}


void SkeletalCreature::MoveFootTo(float x,float y)
{
	myUpFootPosition = Vector2D(-9876.0f, -9876.0f);
	myPreviousMinY = -9876.0f;
	// Assume left foot is down foot
	myCurrentDownFoot = LEFT;
	myDownFootPosition = ZERO_VECTOR;
	UpdatePositionsWithRespectToDownFoot();
	Vector2D r(myExtremes[BODY_LIMB_RIGHT_LEG]);
	if (r.y > 0.0f)
	{
		// The right foot is the down foot
		myCurrentDownFoot = RIGHT;
	}
	myDownFootPosition.x = x;
	myDownFootPosition.y = y;

	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();
	// Creatures can never be directly placed on the floor - they
	// must always fall to the floor.
	myStoppedFlag = false;
}



bool SkeletalCreature::TestMoveFootTo(float x, float y, bool forcePositionCheck)
{
	

	Vector2D newpos(x, y);
	Vector2D delta = newpos - myDownFootPosition;	
	return TestMoveBy(delta.x, delta.y, forcePositionCheck);
}

bool SkeletalCreature::TestMoveTo(float x, float y, bool forcePositionCheck)
{
	

	Vector2D newpos(x, y);
	Vector2D delta = newpos - myPositionVector;	
	return TestMoveBy(delta.x, delta.y, forcePositionCheck);
}


// virtual
bool SkeletalCreature::TestCurrentLocation(bool forcePositionCheck)
{
	
	return TestMoveBy(0.0f, 0.0f, forcePositionCheck);
}


// virtual
bool SkeletalCreature::TestRoomSystemLocation()
{
	
	if (!TestAttributes(attrSufferCollisions))
		return true;
	return theApp.GetWorld().GetMap().IsCreatureLocationValidInRoomSystem
		(myExtremes[BODY_LIMB_LEFT_LEG], 
		myExtremes[BODY_LIMB_RIGHT_LEG], 
		myMinY, myPermiability);
}


// virtual
bool SkeletalCreature::TestMoveBy(float x, float y, bool forcePositionCheck)
{
	
	if (!forcePositionCheck && !TestAttributes(attrSufferCollisions))
		return true;

	Vector2D delta(x, y);
	Vector2D footLeft = myExtremes[BODY_LIMB_LEFT_LEG] + delta;
	Vector2D footRight = myExtremes[BODY_LIMB_RIGHT_LEG] + delta;
	float yMin = myMinY + delta.y;

	if ((myMovementStatus == FLOATING) ||
		(myMovementStatus == CARRIED))
		// Can move everywhere
		return true;
	else if (myMovementStatus == INVEHICLE)
	{
		Box cabin;
		myCarrierAgent.GetVehicleReference().GetCabinExtent(cabin);
		return theApp.GetWorld().GetMap().IsCreatureLocationValidInRectangle
			(footLeft, footRight, yMin, cabin.left, cabin.right, cabin.top, cabin.bottom);
	}
	else
	{
		return theApp.GetWorld().GetMap().IsCreatureLocationValidInRoomSystem
			(footLeft, footRight, yMin, myPermiability);
	}
}


// virtual
float SkeletalCreature::GetDistanceToObstacle(int direction)
{
	

	Vector2D footLeft = myExtremes[BODY_LIMB_LEFT_LEG];
	Vector2D footRight = myExtremes[BODY_LIMB_RIGHT_LEG];
	bool collision;
	float distanceCollision;

	if (myMovementStatus == AUTONOMOUS)
	{
		theApp.GetWorld().GetMap().TestForCreatureCollisionInRoomSystem
			(footLeft, footRight, myMinY, direction, myGeneralRange,
				myPermiability, collision, distanceCollision);
	}
	else if (myMovementStatus == INVEHICLE)
	{
		Box cabin;
		myCarrierAgent.GetVehicleReference().GetCabinExtent(cabin);
		theApp.GetWorld().GetMap().TestForCreatureCollisionInRectangle
			(footLeft, footRight, myMinY, direction, myGeneralRange,
				cabin.left, cabin.right, cabin.top, cabin.bottom, 
				collision, distanceCollision);
	}
	else
		return FLT_MAX;

	if (collision)
		return distanceCollision;
	else
		return FLT_MAX;
}







void SkeletalCreature::HandleMovementWhenAutonomous()
{
	if (myStoppedFlag)
		return;

	int permiability;
	bool collision;
	int wall;
	bool stopped;
	Vector2D velocityCollision;
	int foot;

	
	if (TestAttributes(attrSufferCollisions))
		permiability = myPermiability;
	else
		permiability = 0;

	/* test code */
	Vector2D left = myExtremes[BODY_LIMB_LEFT_LEG];
	Vector2D right = myExtremes[BODY_LIMB_RIGHT_LEG];
	float minytemp = myMinY;
	Vector2D down = myDownFootPosition;
	int foottemp = myCurrentDownFoot;
	Vector2D vel = myVelocityVector;
	/* rewind to here */
	 myExtremes[BODY_LIMB_LEFT_LEG] = left;
	 myExtremes[BODY_LIMB_RIGHT_LEG] = right;
	 myMinY = minytemp;
	 myDownFootPosition = down;
	 myCurrentDownFoot = foottemp;
	 myVelocityVector = vel;



	bool applyPhysics = 
		(myAttributes & attrSufferPhysics) == attrSufferPhysics;
	theApp.GetWorld().GetMap().MoveCreatureInsideRoomSystem
	   (myExtremes[BODY_LIMB_LEFT_LEG],
		myExtremes[BODY_LIMB_RIGHT_LEG],
		myMinY,
		applyPhysics,
		permiability,
		myCollisionFactor,
		myAeroDynamicFactor,
		myGravitationalAcceleration,
		myDownFootPosition,
		myVelocityVector,
		collision,
		wall,
		stopped,
		foot,
		velocityCollision);

	if (stopped) 
	{
		myCurrentDownFoot = foot;
		myStoppedFlag = true;
	}

	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();

	EmergencySafePositionRoom();

	if (collision) {
		myLastWallHit = wall;
		CAOSVar vcx, vcy;
		vcx.SetFloat(velocityCollision.x);
		vcy.SetFloat(velocityCollision.y);
		ExecuteScriptForEvent(SCRIPTCOLLISION, mySelf, vcx, vcy);
	}
}


// virtual
void SkeletalCreature::HandleMovementWhenInVehicle()
{
	if (myStoppedFlag)
		return;

	bool collision;
	int wall;
	bool stopped;
	Vector2D velocityCollision;
	int foot;


	ASSERT(myCarrierAgent.IsVehicle());	
	Vehicle& v = myCarrierAgent.GetVehicleReference();
	Box r;
	v.GetCabinExtent(r);

	bool applyPhysics = 
		(myAttributes & attrSufferPhysics) == attrSufferPhysics;
	theApp.GetWorld().GetMap().MoveCreatureInsideRectangle
	   (myExtremes[BODY_LIMB_LEFT_LEG],
		myExtremes[BODY_LIMB_RIGHT_LEG],
		myMinY,
		applyPhysics,
		myCollisionFactor,
		myAeroDynamicFactor,
		myGravitationalAcceleration,
		r.left, r.right, r.top, r.bottom,
		myDownFootPosition,
		myVelocityVector,
		collision,
		wall,
		stopped,
		foot,
		velocityCollision);

	if (stopped) 
	{
		myCurrentDownFoot = foot;
		myStoppedFlag = true;
	}

	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();

	if (collision) 
	{
		myLastWallHit = wall;
		CAOSVar vcx, vcy;
		vcx.SetFloat(velocityCollision.x);
		vcy.SetFloat(velocityCollision.y);
		ExecuteScriptForEvent(SCRIPTCOLLISION, mySelf, vcx, vcy);
	}
}









// General Initialisation
void SkeletalCreature::SetupDefaults()
{
	myBeingTrackedFlag = false;

	myReach = Vector2D((float)atof(theCatalogue.Get("Creature Reach", 0)), (float)atof(theCatalogue.Get("Creature Reach", 1)));

	myStandStill = false;
	myIsHoldingHandsWithThePointer = false;
	myBodyPartToBeCreated = 0;
	myBodyPartToBeCreatedForNextAgeStage =0;
	myDoubleSpeedFlag = false;
	myIntrospectiveFlag = false;
	myBodyPartsAreFullyFormedFlag = false;
	myBodyPartsAgeStage =0;
	myJustAgedFlag = false;
	myAgeingDone = false;
	myAgeAlreadyLoaded = 0;

	// tell the camera you have grown up if
	// you are being tracked
	myBeingTrackedFlag = false;
	myNumSpritesInFile = 0;
	myCompositeBodyPartGallery = NULL;
	myCompositeBodyPartGalleryForNextAgeStage = NULL;
	myLastAge = 0;
	myPreloadLastAge= 0;
	myTintin = new TintManager();

	_ASSERT(myTintin);


	{
		for (int i=0; i<MAX_GAITS; i++)
			myGaitLoci[i] = 0.0f;
	}

    static char StartPose[] = "112212212011200";    // initial pose$
    int i;

    myBody = NULL;
    for (i=0; i<6; i++)		
    {
        myLimbs[i] = NULL;                  
		myExtremes[i] = Vector2D(-9876.0f, -9876.0f);
	}

    myCurrentDirection=WEST;
    myDownFootPosition = Vector2D(-9876.0f, -9876.0f);
	myUpFootPosition = Vector2D(-9876.0f, -9876.0f);
	myPreviousUpFootPosition = Vector2D(-9876.0f, -9876.0f);
	myPreviousMinY = -9876.0f;
    myCurrentDownFoot=RIGHT;
	myNormalPlane=100;
                              
    strcpy(myCurrentPoseString,StartPose);                 // init myCurrentPoseString sensibly
    strcpy(myTargetPoseString,StartPose);               // init myTargetPoseString sensibly
	for	(i=0; i<MAX_POSES; i++)
		strcpy(myPoseStringTable[i],StartPose);				// set pose table to safe defaults
    myFacialExpression = EXPR_NORMAL;                   // normal facial expression
	myEarSet = EARS_NORMAL;
    myEyeState = 1;                                   // eyes open
 	*myAnimationString = 0; 
	myAnimationPointer = myAnimationString;					// No animation active

	// set default gaits
    for (i=0; i<MAX_GAITS; i++)
		strcpy(myGaitTable[i],"013014015016R");

	myMusclesLocus = 0.0f;							// reset loci
    myUpslopeLocus = 0.0f;
    myDownslopeLocus = 0.0f;

	myBodyIsSuitedUp = false;

	for (int v=0; v<NUMPARTS; v++)
	{
		myVariants[v] = 0;
	}

	myEntitiesAreCameraShy = false;

	myAgentType |= AgentHandle::agentSkeletalCreature;
	// don't want to be identified as a compound agent for macro commands etc:
	myAgentType &= ~AgentHandle::agentCompound;
}

// in- myCurrentDirection
//     myCurrentDownFoot
//     myDownFootPosition
// out-myExtremes
//     myUpFootPosition
//	   myPreviousUpFootPosition
//	   myPreviousMinY
//     myPositionVector
//     myCurrentWidth
//     myCurrentHeight
//     changes positions of myBody and myLimbs
void SkeletalCreature::UpdatePositionsWithRespectToDownFoot()
{
	
	_ASSERT(myBody);

    int i,OffsetX,OffsetY;
    Limb* currentLimb;
    LimbData* currentLimbData;
    int Lpv;
	int xd, yd;
	int LegOther;
	int Leg;
	Vector2D p;

    // Now calculate the world XYs for each body part, from xy of 
	// current down foot.
    // While this is happening, examine the new posn & the size of each
    // limb's sprite, and build a new Bounding rect

    // first calc world coords of topleft of body entity by measuring
    // back from the currently 'down' foot...
	if (myCurrentDownFoot==LEFT) {
		Leg = BODY_LIMB_LEFT_LEG;
		LegOther = BODY_LIMB_RIGHT_LEG;
	}
	else {
		Leg = BODY_LIMB_RIGHT_LEG;   // index of 'down' leg
		LegOther = BODY_LIMB_LEFT_LEG;
	}

    OffsetX = 0;                                    // to calc dist frm
    OffsetY = 0;                                    // body to foot tip
    for (currentLimb = myLimbs[Leg];                       // descend limb chain
         currentLimb!=NULL;                                  // adding up offsets
         currentLimb = currentLimb->GetNextLimb()) {                       // till reach tip

        currentLimbData = currentLimb->GetLimbData();                            // ptr to data fr limb type
        Lpv = currentLimb->GetView();                             // temp store fr View		
		OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);
        OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);
    }

	myBody->SetPosition
		(Map::FastFloatToInteger(myDownFootPosition.x - OffsetX - myBody->GetBodyData()->JoinX[Leg][myBody->GetView()]),// calc abs xy of
         Map::FastFloatToInteger(myDownFootPosition.y - OffsetY - myBody->GetBodyData()->JoinY[Leg][myBody->GetView()]));// topleft of body

	myExtremes[Leg] = myDownFootPosition;
	p = myDownFootPosition;


	xd = OffsetX;
	yd = OffsetY;
	p.x -= xd;
	p.y -= yd;
	Vector2D downjoin = p;

	int ax = myBody->GetBodyData()->JoinX[Leg][myBody->GetView()];
	int ay = myBody->GetBodyData()->JoinY[Leg][myBody->GetView()];
	int bx = myBody->GetBodyData()->JoinX[LegOther][myBody->GetView()];
	int by = myBody->GetBodyData()->JoinY[LegOther][myBody->GetView()];
	p.x -= ax - bx;
	p.y -= ay - by;
	Vector2D upjoin = p;

	OffsetX = 0;
	OffsetY = 0;
	for (currentLimb = myLimbs[LegOther];                       // descend limb chain
         currentLimb!=NULL;                                  // adding up offsets
         currentLimb = currentLimb->GetNextLimb()) {                       // till reach tip

        currentLimbData = currentLimb->GetLimbData();                            // ptr to data fr limb type
        Lpv = currentLimb->GetView();                             // temp store fr View
		OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);
        OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);
    }
	p.x += OffsetX;
	p.y += OffsetY;
	myUpFootPosition = p;
	myExtremes[LegOther] = myUpFootPosition;


	// left arm 
	p = downjoin;
	bx = myBody->GetBodyData()->JoinX[BODY_LIMB_LEFT_ARM][myBody->GetView()];
	by = myBody->GetBodyData()->JoinY[BODY_LIMB_LEFT_ARM][myBody->GetView()];
	p.x -= ax - bx;
	p.y -= ay - by;
	OffsetX = 0;
	OffsetY = 0;
	for (currentLimb = myLimbs[BODY_LIMB_LEFT_ARM];                       // descend limb chain
         currentLimb!=NULL;                                  // adding up offsets
         currentLimb = currentLimb->GetNextLimb()) {                       // till reach tip
        currentLimbData = currentLimb->GetLimbData();                            // ptr to data fr limb type
        Lpv = currentLimb->GetView();                             // temp store fr View
		OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);
        OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);
    }
	p.x += OffsetX;
	p.y += OffsetY;
	myExtremes[BODY_LIMB_LEFT_ARM] = p;

	// neck
	p = downjoin;
	bx = myBody->GetBodyData()->JoinX[BODY_LIMB_HEAD][myBody->GetView()];
	by = myBody->GetBodyData()->JoinY[BODY_LIMB_HEAD][myBody->GetView()];
	p.x -= ax - bx;
	p.y -= ay - by;
	Vector2D neckjoin = p;

	// right arm
	p = downjoin;
	bx = myBody->GetBodyData()->JoinX[BODY_LIMB_RIGHT_ARM][myBody->GetView()];
	by = myBody->GetBodyData()->JoinY[BODY_LIMB_RIGHT_ARM][myBody->GetView()];
	p.x -= ax - bx;
	p.y -= ay - by;
	OffsetX = 0;
	OffsetY = 0;
	for (currentLimb = myLimbs[BODY_LIMB_RIGHT_ARM];                       // descend limb chain
         currentLimb!=NULL;                                  // adding up offsets
         currentLimb = currentLimb->GetNextLimb()) {                       // till reach tip
        currentLimbData = currentLimb->GetLimbData();                            // ptr to data fr limb type
        Lpv = currentLimb->GetView();                             // temp store fr View
		OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);
        OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);
    }
	p.x += OffsetX;
	p.y += OffsetY;
	myExtremes[BODY_LIMB_RIGHT_ARM] = p;


	// tail
	p = downjoin;
	bx = myBody->GetBodyData()->JoinX[BODY_LIMB_TAIL][myBody->GetView()];
	by = myBody->GetBodyData()->JoinY[BODY_LIMB_TAIL][myBody->GetView()];
	p.x -= ax - bx;
	p.y -= ay - by;
	OffsetX = 0;
	OffsetY = 0;
	for (currentLimb = myLimbs[BODY_LIMB_TAIL];                       // descend limb chain
         currentLimb!=NULL;                                  // adding up offsets
         currentLimb = currentLimb->GetNextLimb()) {                       // till reach tip
        currentLimbData = currentLimb->GetLimbData();                            // ptr to data fr limb type
        Lpv = currentLimb->GetView();                             // temp store fr View
		OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);
        OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);
    }
	p.x += OffsetX;
	p.y += OffsetY;
	myExtremes[BODY_LIMB_TAIL] = p;
	

    // Body's entity WorldX/Y now contain abs xy of topleft of body.
    // Now propagate this through to all dependent limbs...
    for (i=0; i<MAX_BODY_LIMBS; i++) 
    {    // for each limb chain
        OffsetX = myBody->GetWorldX() + myBody->GetBodyData()->JoinX[i][myBody->GetView()]; // start point of 1st
        OffsetY = myBody->GetWorldY() + myBody->GetBodyData()->JoinY[i][myBody->GetView()]; // limb in chain

        for (currentLimb = myLimbs[i];                     // descend limb chain
             currentLimb!=NULL;
             currentLimb = currentLimb->GetNextLimb()) 
        {

            currentLimbData = currentLimb->GetLimbData();                        // ptr to data fr limb type
            Lpv = currentLimb->GetView();                         // temp store fr View

            // Move limb to correct position.
            currentLimb->SetPosition
				(OffsetX - currentLimbData->StartX[Lpv], 
				 OffsetY - currentLimbData->StartY[Lpv]);

            // get start point of next limb
            OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);          
            OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);  
        }
    }

	// Update the head sprite
	CreatureHead* head = (CreatureHead*)myLimbs[BODY_LIMB_HEAD];
	if(head)
		head->UpdatePosn(myCurrentDirection,myFacialExpression,myEyeState,myEarSet);

	RECT box;
	head->GetBound(box);
	float width = box.right - box.left;
	float height = box.bottom - box.top;
    currentLimbData = myLimbs[BODY_LIMB_HEAD]->GetLimbData();   
    Lpv = myLimbs[BODY_LIMB_HEAD]->GetView();
	OffsetX = currentLimbData->StartX[Lpv]; 
	OffsetY = currentLimbData->StartY[Lpv];
	Vector2D necktopleft(neckjoin);
	necktopleft.x -= OffsetX;
	necktopleft.y -= OffsetY;

	Box headfloat;
	headfloat.left = necktopleft.x + width/6.0f;
	headfloat.top = necktopleft.y + height/6.0f;
	headfloat.right = necktopleft.x + width - width/6.0f;

	myExtremes[BODY_LIMB_HEAD].x = necktopleft.x + (width/2.0f);
	myExtremes[BODY_LIMB_HEAD].y = headfloat.top;
	float xmin = FLT_MAX;
	float xmax = -FLT_MAX;
	float ymin = FLT_MAX;
	float ymax = -FLT_MAX;
	for (i=0; i<6; i++)
	{
		if (myExtremes[i].x < xmin) xmin = myExtremes[i].x;
		if (myExtremes[i].x > xmax) xmax = myExtremes[i].x;
		if (myExtremes[i].y < ymin) ymin = myExtremes[i].y;
		if (myExtremes[i].y > ymax) ymax = myExtremes[i].y;
	}
	if (headfloat.left < xmin)
		xmin = headfloat.left;
	if (headfloat.right > xmax)
		xmax = headfloat.right;

	myPositionVector.x = xmin;
	myPositionVector.y = ymin;
	// gtb, n.b.: doesn't use setwidth or setheight
	myCurrentWidth = xmax - xmin;
	myCurrentHeight = ymax - ymin;
	myMinX = xmin;
	myMaxX = xmax;
	myMinY = ymin;
	myMaxY = ymax;
}


void SkeletalCreature::UpdateFeet()
{
	if (myInvalidPosition)
		return;

	bool fall;
	bool collision;
	bool footChange;
	int wall;
	CAOSVar vcx, vcy;

	if (myMovementStatus == INVEHICLE) 
	{
		Box r;
		myCarrierAgent.GetVehicleReference().GetCabinExtent(r);
		theApp.GetWorld().GetMap().TestNewUpFootInRectangle
			(myDownFootPosition,
			myUpFootPosition, myMinX, myMaxX, 
			r.left, r.right, r.top, r.bottom,
			myDownFootPosition, collision, wall, footChange);

		if (footChange) 
		{
			// Swap down foot
			if (myCurrentDownFoot == LEFT)
				myCurrentDownFoot = RIGHT;
			else
				myCurrentDownFoot = LEFT;	
		}

		if (collision || footChange) 
			UpdatePositionsWithRespectToDownFoot();
		CommitPositions();

		if (collision) 
		{
			myLastWallHit = wall;
			ExecuteScriptForEvent(SCRIPTBUMP, mySelf, vcx, vcy);
		}

		return;
	}

	bool right = (myDownFootPosition.x < myUpFootPosition.x);

	Vector2D lefttemp = myExtremes[BODY_LIMB_LEFT_LEG];
	Vector2D righttemp = myExtremes[BODY_LIMB_RIGHT_LEG];
	float minytemp = myMinY;
	Vector2D downtemp = myDownFootPosition;
	int foottemp = myCurrentDownFoot;
 	bool directionRight = (myCurrentDirection == EAST);

	float gradientDownhill, gradientUphill;
	theApp.GetWorld().GetMap().TestNewUpFootInRoomSystem
			(myDownFootPosition,
			myPreviousUpFootPosition, myUpFootPosition,
			myMinX, myMaxX,
			myPreviousMinY, myMinY, 
			myPermiability, 
			directionRight,
			myDownFootPosition, 
			gradientDownhill, gradientUphill,
			collision, wall,
			footChange, 
			fall);

	myDownslopeLocus = gradientDownhill;
	myUpslopeLocus = gradientUphill;

	if (footChange) 
	{
		// Swap down foot
		if (myCurrentDownFoot == LEFT)
			myCurrentDownFoot = RIGHT;
		else
			myCurrentDownFoot = LEFT;	
	}	

	if (collision || footChange || fall) 
		UpdatePositionsWithRespectToDownFoot();
	CommitPositions();

	EmergencySafePositionRoom();

	if (fall) 
	{	
		// Stop walking, start falling
		myStoppedFlag = false;
		myVelocityVector = ZERO_VECTOR;
	}

	if (collision) 
	{
		myLastWallHit = wall;	
		ExecuteScriptForEvent(SCRIPTBUMP, mySelf, vcx, vcy);
	}
}



void SkeletalCreature::GetExtremesForPose
	(char* pose, Vector2D extremes[MAX_BODY_LIMBS])
{
	int i;
	char oldpose[MAX_POSE_STRING_LENGTH+1];
	strcpy(oldpose, myCurrentPoseString);
	bool oldinvalidpos = myInvalidPosition;
	int oldCurrentDownFoot = myCurrentDownFoot;
	Vector2D oldDownFootPosition = myDownFootPosition;

	myCurrentDownFoot = LEFT;
	myDownFootPosition = ZERO_VECTOR;
	// This stops UpdateFeet() putting its grim oar in
	myInvalidPosition = true;
	SetCurrentPoseString(pose);
	for (i=0; i<MAX_BODY_LIMBS; ++i)
		extremes[i] = myExtremes[i];
	myCurrentDownFoot = oldCurrentDownFoot;
	myDownFootPosition = oldDownFootPosition;
	SetCurrentPoseString(oldpose);
	myInvalidPosition = oldinvalidpos;
}


void SkeletalCreature::GeneratePlotOrder()
{
	static int PlaneOffset[4][NUMPARTS] = {
		// North - away from the camera
        { 1,				// Head	 
		  0,				// Body
		 -1,-2,-3,			// Left Leg
		 -1,-2,-3,			// Right Leg
		 -1,-2,				// Left Arm
		 -1,-2,				// Right Arm 
		  1,2,				// The Tail
		  1,1,2 },	        // Head Limbs. 

		// South - towards the camera
        { 3,				// Head
		  0,				// Body
		 -1,0,1,			// Left Leg // shins and feet need to be closer to the camera
		 -1,0,1,			// Right Leg
		 -1,1,				// Left Arm // lower arms should be in front of body
		 -1,1,				// Right Arm
		 -1,-2,				// The Tail
		 -5,-5,-4 },        // Head Limbs.

		// East - right
		{ 2,				// Head
		  0,				// Body
		 -1,-2,-1,			// Left Leg
		  1, 2, 3,			// Right Leg
		 -1,-2,				// Left Arm
		  2, 3,				// Right Arm
		  -1, 0,				// Tail
		  -3, 3, 2 },		// Head Limbs

		// West - left
		{ 2,				// Head
		  0,				// Body
		  1, 2, 3,			// Left Leg
		 -1,-2,-1,			// Right Leg
		  2, 3,				// Left Arm
		 -1,-2,				// Right Arm
		  -1, 0,				// Tail
		  3,-3, 2 }			// Head Limbs
        };
	// The Body Entity is in Plot Position 0 relative to the above tables

	EntityImage* allMyEntities[NUMPARTS];
	allMyEntities[PART_HEAD] = myLimbs[0];
	allMyEntities[PART_BODY] = myBody;
	allMyEntities[PART_LTHIGH] = myLimbs[1];
	allMyEntities[PART_LSHIN] = myLimbs[1]->GetNextLimb();
	allMyEntities[PART_LFOOT] = myLimbs[1]->GetNextLimb()->GetNextLimb();
	allMyEntities[PART_RTHIGH] = myLimbs[2];
	allMyEntities[PART_RSHIN] = myLimbs[2]->GetNextLimb();
	allMyEntities[PART_RFOOT] = myLimbs[2]->GetNextLimb()->GetNextLimb();
	allMyEntities[PART_LHUMERUS] = myLimbs[3];
	allMyEntities[PART_LRADIUS] = myLimbs[3]->GetNextLimb();
	allMyEntities[PART_RHUMERUS] = myLimbs[4];
	allMyEntities[PART_RRADIUS] = myLimbs[4]->GetNextLimb();
	allMyEntities[PART_TAILROOT] = myLimbs[5];
	allMyEntities[PART_TAILTIP] = myLimbs[5]->GetNextLimb();
	allMyEntities[PART_LEFT_EAR] = ((CreatureHead*)myLimbs[0])->GetLimb(0);
	allMyEntities[PART_RIGHT_EAR] = ((CreatureHead*)myLimbs[0])->GetLimb(1);
	allMyEntities[PART_HAIR] = ((CreatureHead*)myLimbs[0])->GetLimb(2);

	

	for(int bodyDirection=0; bodyDirection<4; bodyDirection++)
		for(int headDirection=0; headDirection<4; headDirection++)
		{
			IntegerPairList myOrder;
			myOrder.push_back(PairUp(PlaneOffset[bodyDirection][PART_HEAD],PART_HEAD));
			int currentBodyPart;

			for(currentBodyPart = PART_BODY;
				currentBodyPart < PART_LEFT_EAR;
				currentBodyPart++)
				{
					int currentPlane = PlaneOffset[bodyDirection][currentBodyPart];
					IntegerPairListIterator it;
					for(it = myOrder.begin();
						it != myOrder.end();
						it++)
						{
							int thisPlane = (*it).first;
							if (thisPlane > currentPlane)
							{
								myOrder.insert(it,PairUp(currentPlane,currentBodyPart));
								break;
							}
						}
					if (it == myOrder.end())
						myOrder.push_back(PairUp(currentPlane,currentBodyPart));
				}
			for(currentBodyPart = PART_LEFT_EAR;
				currentBodyPart < NUMPARTS;
				currentBodyPart++)
				{
					int currentPlane = PlaneOffset[headDirection][currentBodyPart];
					IntegerPairListIterator it;
					for(it = myOrder.begin();
						it != myOrder.end();
						it++)
						{
							int thisPlane = (*it).first;
							if (thisPlane > currentPlane)
							{
								myOrder.insert(it,PairUp(currentPlane,currentBodyPart));
								break;
							}
						}
					if (it == myOrder.end())
						myOrder.push_back(PairUp(currentPlane,currentBodyPart));
				}
			// In theory (Fingers Crossed and all that Jazz)...
			// The myOrder should be a nice integer pair list in order of plotting :)
			IntegerPairListIterator magicIterator = myOrder.begin();

			for(currentBodyPart = PART_HEAD;
				currentBodyPart < NUMPARTS;
				currentBodyPart++)
				{
					myPlotOrders[bodyDirection][headDirection][currentBodyPart] = 
						allMyEntities[(*magicIterator).second];
					magicIterator++;
				}
		}
}



// Set the display plane for each of the skeleton's entities
// so that all limbs are plotted in the correct order
// this is only necessary when the direction has changed
void SkeletalCreature::UpdatePlotOrder()
{
	
	
#ifdef _DEBUG
	for(int j = 0; j < 4; j++)
		for(int k = 0; k < 4; k++)
			for(int l = 0; l < NUMPARTS; l++)
				_ASSERT( myPlotOrders[j][k][l] );
#endif

	// Carried agents are _always_ in the Right hand :(
	// Thus we need to suss the direction of the creature out...
	// If facing right (east) the creature needs to plot the carried agent _just_ before the right humerus
	// Otherwise _just_ after the right radius will do nicely :)

	int headDirection = ((CreatureHead*)myLimbs[0])->GetCurrentDirection();
	int i;
	ASSERT(myCurrentDirection < 4); // myCurrentDirection is unsigned anyway
	ASSERT(headDirection >= 0 && headDirection < 4);

	for(i=PART_HEAD; i < NUMPARTS; i++)
	{
		if (myCarriedAgent.IsValid())
			if (myCurrentDirection == EAST)
				if (myPlotOrders[myCurrentDirection][headDirection][i] == myLimbs[4]) // right humerus
					myCarriedAgent.GetAgentReference().ChangePhysicalPlane( myBody->GetPlane() );

		myPlotOrders[myCurrentDirection][headDirection][i]->SetPlane(myBody->GetPlane());
		myPlotOrders[myCurrentDirection][headDirection][i]->YouAreCameraShy(myEntitiesAreCameraShy);
		theMainView.UpdatePlane(myPlotOrders[myCurrentDirection][headDirection][i]);
		
		if (myCarriedAgent.IsValid())
			if (myCurrentDirection != EAST)
				if (myPlotOrders[myCurrentDirection][headDirection][i] == myLimbs[4]->GetNextLimb()) // right radius
					myCarriedAgent.GetAgentReference().ChangePhysicalPlane( myBody->GetPlane() );
	}
}



// Master update function:
//  1/ Updates all limb entities' world xys, based on xy of down foot
//  2/ Update bounding rectangle
//  3/ Updates feet, swapping to new foot if necessary
//  4/ Detects collisions with walls, and slides obj to avoid exceeding limit
void SkeletalCreature::UpdateSkeleton()
{
	UpdatePositionsWithRespectToDownFoot();                       // update xys & bounding box
    UpdateFeet();                       // test against floor & swap feet
	UpdatePlotOrder();
}



// Sets body/limb angles, views and images
// Sets myCurrentDirection
// Sets myCurrentPoseString
void SkeletalCreature::SetCurrentPoseString(char* pose)
{
	

	// The number of limbs in each limb chain
	static int chainLengths[MAX_BODY_LIMBS] = {           
        1,          // head                        
        3,          // l leg
        3,          // r leg
        2,          // l arms
        2,          // r arm
        2			// tail
	};         
	// Index into pose string for first limb in each chain
    static int Index[MAX_BODY_LIMBS] = {
        _HEAD,                                      
        _LTHIGH,                                    
        _RTHIGH,
        _LHUMERUS,
        _RHUMERUS,
        _TAILROOT
	};
    int i,j;
    Limb* currentLimb;
    int p;

    if (pose[_DIRN] != 'X') {   
		// Set new direction if not 'no change'
		myCurrentDirection = pose[_DIRN]-'0';                    
    }

    if (pose[_BODY] != 'X') {
		// Set body angle if not 'no change'
		myBody->SetAngle(pose[_BODY]-'0');
	}
	myBody->SetViewAndImage(myCurrentDirection);

    for (i=0; i<MAX_BODY_LIMBS; i++) {  
        currentLimb = myLimbs[i];                
        for (j=0; (j<chainLengths[i])&&(currentLimb);j++) {                            
            p = pose[Index[i]+j];                  
            if (p!='X')            
				// Set the limb's angle if not 'no change'            
				currentLimb->SetAngle(p-'0');            
			currentLimb->SetViewAndImage(myCurrentDirection);
            currentLimb = currentLimb->GetNextLimb();
        }
    }
		
	// Update the creature's expression
	CreatureHead* head = (CreatureHead*)myLimbs[BODY_LIMB_HEAD];
	head->SetCurrentPoseString(myCurrentDirection,myFacialExpression,myEyeState,myEarSet);

    UpdateSkeleton();
	

    for (i=0; i<MAX_POSE_STRING_LENGTH; i++)                    // keep a copy of curr pose
        if  (pose[i]!='X')                          
            myCurrentPoseString[i] = pose[i];

}


// Return the correct vertical head angle for looking towards curr target
// ('3'=up '2'=straight '1'=down '0'=straight down)

const int TILTGRIDX = 32;   // space between each dist element in table
const int TILTGRIDY = 8;    // height between each y element
const int TILTX = 4;        // # x (dist) elements in table
const int TILTY = 10;       // # y (height) elements in table

// Table of head angles, each row refers to a height of target above
// eye level (row 2 = eye level or above); each column refers to a distance
// from eye (col 0 = eye, 1 = 32 pels away...)
char Look[TILTY][TILTX] = {

    //<32 <64 <96 <128
    { '3','3','3','3' },    // >32   pels above eye
    { '3','3','3','2' },    // 24-32 pels above eye
    { '3','3','2','2' },    // 16-24 pels above eye
    { '3','2','2','2' },    // 8-16  pels above eye
    { '2','2','2','2' },    // 0-8   pels above eye
    { '1','2','2','2' },    // 0-8   pels below eye
    { '0','1','2','2' },    // 8-16  pels below eye
    { '0','1','1','2' },    // 16-24 pels below eye
    { '0','0','1','1' },    // 24-32 pels below eye
    { '0','0','1','1' }     // >32   pels below eye

};

inline char SkeletalCreature::HeadAngle() 
{
	

    int x,y;
    if  (myIt.IsInvalid())							// if no IT object defined,
        return('1');                        // always look straight ahead

    x = Map::FastFloatToInteger(fabsf( myItPosition.x - myDownFootPosition.x));                     // dist to target

    if  (x<16)                              // if IT is very close, look to
        return((myIt.GetAgentReference().GetPlane()<GetPlane())  // north or south
                ? '5' : '4');

    x /=TILTGRIDX;                          // x posn in table

    if  (x>=TILTX)                      // if far away, never look up or down
        return('1');

	y = Map::FastFloatToInteger((myItPosition.y-myBody->GetWorldY())/
            TILTGRIDY+(TILTY/2));       // y posn in table (eye level=3)
    if  (y<0) y=0;
    if  (y>=TILTY) y=TILTY-1;

    return(Look[y][x]);                 // else ret correct angle fr height/dist
}



// Move one more step towards myTargetPoseString (if necessary) & redraw whole obj
// If a head angle of '?' is defined, Skeleton will automatically look
// up or down as required to face IT.
// If a Direction of '?' is defined, Skeleton will incrmentally turn
// head then body towards IT. A dirn of '!' means turn to face AWAY from IT.
void SkeletalCreature::MoveTowardsTargetPoseString()
{
	

    char    NextPose[] = "XXXXXXXXXXXXXXX";
    char    Pose[MAX_POSE_STRING_LENGTH+1];
    int     Status;
    int     NextDirn;
    int     ToIt=0;




    // Table of next body direction & head angle to use when turning
    // incrementally towards a new direction. Allows head to swivel
    // ahead of body in initial part of the turn.
    // Each row corresponds to a current situation composed of body & head
    // positions. Each column represents a target direction.
    // Each entry is an int: high byte = next direction; lo byte = next head
	// angle (both as ascii chars (1st char in 'XX' is hi 2nd is lo)
    static int Next[6][4] = {
// target: N    S    E    W
		{ '52','42','12','42' },    // Body=E Head=Straight
		{ '50','41','12','41' },    // Body=E Head=Askance

		{ '53','43','43','13' },    // Body=W Head=Straight
		{ '50','41','41','13' },    // Body=W Head=Askance

		{ '50','41','12','13' },    // Body=N/S Head=Straight
		{ '50','41','12','13' },    // Body=N/S Head=Askance
	};

	// Make a copy of target pose, so I can alter it without changing original

	if(myIsHoldingHandsWithThePointer && myCurrentDirection == EAST  )
	{
		myTargetPoseString[_RHUMERUS] = '1';
		myTargetPoseString[_RRADIUS] = '2';
	}
	else if(myIsHoldingHandsWithThePointer && myCurrentDirection == WEST )
	{
		myTargetPoseString[_LHUMERUS] = '1';
		myTargetPoseString[_LRADIUS] = '2';
	}

	ASSERT( strlen( myTargetPoseString ) <= MAX_POSE_STRING_LENGTH );	//_BC_
	strcpy(Pose,myTargetPoseString);




	// If a direction is specified, turn incrementally towards that direction
	if  (Pose[_DIRN]=='?')                      // cvt 'towards it' to dirn
    {
		if  (myIntrospectiveFlag)
        {
            Pose[_DIRN]='X';
        }
        else 
        {
            Pose[_DIRN] = (myItPosition.x - myDownFootPosition.x < 0.0f) ? '3' : '2';
            ToIt++;                             // remember intention fr later
        }
    }
    else if (Pose[_DIRN]=='!')                  // cvt 'away frm it' to dirn
    {
        if  (myIntrospectiveFlag)
            Pose[_DIRN]='X';
        else
            Pose[_DIRN] = ( myItPosition.x - myDownFootPosition.x  < 0.0f) ? '2' : '3';
    }

    if  ((Pose[_DIRN]!='X')&&                   // if already facing that way
         (Pose[_DIRN]!=myCurrentPoseString[_DIRN])) {      // nowt to do, otherwise...

        switch (myCurrentDirection) {                      // get hi bits of curr status
        case EAST:                              // ie. E=0 W=2 N/S=4
            Status=0;
            break;
        case WEST:
            Status=2;
            break;
        default:
            Status=4;
            break;
        }

        if (myCurrentPoseString[_HEAD]>='4')              // get lo bit of status
		{
            Status++;                           // ie. straight=0 askance=1
			if (myCurrentPoseString[_HEAD]=='4' && myCurrentDirection==NORTH)
			{
				Pose[_DIRN]='0'+EAST;			// if facing north, but looking
			}									// to camera, face east.
		}

        NextDirn = Next[Status]
                       [Pose[_DIRN]-'0'];       // get new dirn & head angle


        Pose[_HEAD] = NextDirn>>8;              // store new direction
        Pose[_DIRN] = NextDirn&0x00FF;          // and head angle in pose

        if  ((ToIt)&&(Pose[_DIRN]=='1')) {      // if table said south
            if  ((myIt.IsValid()) && myIt.GetAgentReference().GetPlane()<GetPlane())     // & trying to face an IT
                Pose[_DIRN] = '0';              // that's N of you, turn via N
        }
        if  ((ToIt)&&(Pose[_HEAD]=='4')) {      // likewise for head
            if  ((myIt.IsValid()) && myIt.GetAgentReference().GetPlane()<GetPlane())
                Pose[_HEAD] = '5';
        }

    }

    // If a head angle of '?' has been defined, choose one that looks
    // vertically directly at IT (unless no it, when look straight ahead)
    // (choose TARGET here, next block will change to it incrementally).
    // (If currently turning, any '?' will have been replaced with abs angle)
    if  (Pose[_HEAD]=='?')
        Pose[_HEAD] = HeadAngle();

    // run through pose[] string, comparing it with myCurrentPoseString[], and build
    // a 'next step' string to use for calling SetPose().
	// For every limb that moves, add one to the myMusclesLocus emitter locus. The
	// biochemistry can use this value to determine how much energy is being expended
    NextPose[_DIRN] = Pose[_DIRN];                  // dirn is already incrmntl

    for (int i=1; i<MAX_POSE_STRING_LENGTH; i++) {              // fr each entry in pose
                                                    // (excl. dirn)
        if  (Pose[i]!='X') {                        // if relevant...

            if  (Pose[i]>myCurrentPoseString[i]) {             // next pose is
                NextPose[i] = myCurrentPoseString[i]+1;        // incrementally different
				myMusclesLocus+=LOC_MUSCLES_INCREASE;	// update emitter locus
			}
            else if (Pose[i]<myCurrentPoseString[i]) {
                NextPose[i] = myCurrentPoseString[i]-1;
				myMusclesLocus+=LOC_MUSCLES_INCREASE;	// update emitter locus
			}

        }
    }

    if  (Pose[_HEAD]>='4')                          // DON'T do incrementally
        NextPose[_HEAD]=Pose[_HEAD];                // when turning to or from
    else if ((myCurrentPoseString[_HEAD]>='4')                 // south or
             &&(Pose[_HEAD]<'4'))                   // north (cos would pass
        NextPose[_HEAD]='1';                        // through south first)




    SetCurrentPoseString(NextPose);                              // make the changes
}



// If the given pose is not the current myTargetPoseString, then make it so.
// Return TRUE if the pose you wanted has already been reached.
// This function may be called repeatedly for macro commands like POSE and REAC, where
// the command must start the Skeleton shifting towards a given pose, and repeat the
// action until that pose has been reached.
bool SkeletalCreature::SetTargetPoseString(char* pose)
{
	

	// compare given pose to current myTargetPoseString. If they are the same, then either
	// the target has already been reached, or the shift is in progress. 
	// If they are different, then the new target should be submitted
	if	(strcmp(myTargetPoseString,pose)) {					// if this is a new target pose...
		strcpy(myTargetPoseString,pose);					// define new target pose
		return false;								// report not had a chance to reach it yet
	}
	// If given pose is already the target, then return TRUE
	return HasTargetPoseStringBeenReached();							
}

// Test to see if the Skeleton has reached the current target pose yet.
// Return TRUE if it has. Used by Animate() and SetPose() to test when
// pose change has been accomplished
bool SkeletalCreature::HasTargetPoseStringBeenReached()
{
	

	int i;

    for (i=0; i<MAX_POSE_STRING_LENGTH; i++) {                  // retn TRUE if all done
        if  (((myTargetPoseString[i]>='0')&&(myTargetPoseString[i]<='9'))&&
             (myTargetPoseString[i]!=myCurrentPoseString[i]))
             return(false);
    }
    return(true);
}





void SkeletalCreature::Update()
{
	_ASSERT(!myGarbaged);

	Agent::Update();
	// skip CompoundAgent
	base::OwnUpdate();
	_ASSERT(!myGarbaged);

	// Do we need more of these all over the place?
	if (myInvalidPosition)
		return;

	bool doWeWantDoubleSpeedSkeletons = 
		theApp.ShouldSkeletonsAnimateDoubleSpeed() || myDoubleSpeedFlag;

	// every other tick don't update skeleton if we want normal speed skeletons:
	if (!((myUpdateTick%2)!=0 && !doWeWantDoubleSpeedSkeletons))
	{
		if (!myStoppedFlag || myMovementStatus==CARRIED || myMovementStatus==FLOATING)
		{
			// Update the creature's expression
			CreatureHead* head = (CreatureHead*)myLimbs[BODY_LIMB_HEAD];
			head->SetCurrentPoseString(myCurrentDirection,myFacialExpression,myEyeState,myEarSet);
		} else {
			// Do walking stuff...
			myMusclesLocus = 0.0f;					// reset 'muscle energy expended' accumulator

			// move another step towards current target pose. If already there, then this will
			// merely update gaze and blinking
			MoveTowardsTargetPoseString();					

			// update any animation if the target has been reached
			if ((*myAnimationPointer)&&(HasTargetPoseStringBeenReached())) {						// if reached target and an anim is active

				if (!myStandStill)
					myAnimationPointer+=3;											// move on to next target pose

				ASSERT(myAnimationPointer - myAnimationString <= MAX_SKELETON_ANIMATION_STRING_LENGTH);

				if	(*myAnimationPointer=='R')									// on REPEAT code, start again
					myAnimationPointer = myAnimationString;

				if	(*myAnimationPointer)  									// if not reached end
				{
					// get 3-digit pose#
					ASSERT(isdigit(myAnimationPointer[0]));
					ASSERT(isdigit(myAnimationPointer[1]));
					ASSERT(isdigit(myAnimationPointer[2]));
					int poseID = ((myAnimationPointer[0]-'0')*100) + ((myAnimationPointer[1]-'0')*10) + (myAnimationPointer[2]-'0'); 	

					// check value of pose
					ASSERT((poseID >= 0) && (poseID <= 255));

					strcpy(myTargetPoseString,myPoseStringTable[poseID]);
				}
			}

			// updates were here:
			if (myStoppedFlag)
			{
				// Only age when on the floor
				if(ContinueLoadingDataForNextAgeStage())
				{
					if(Life()->GetAge() <= NUMAGES)
					{
						Genome g(GetGenomeStore(), 0, Life()->GetSex(),	myAgeAlreadyLoaded, Life()->GetVariant());
						// added by gtb:
						_ASSERT(!myGarbaged);
						PreloadBodyPartsForNextAgeStage(g, Life()->GetSex());
					}
				}
			}
		}
	}

	if (myIsHoldingHandsWithThePointer) 
	{			
		HoldHandsWithThePointer();
	}

	// not our tick?
	if ((myUpdateTick%4)!=myUpdateTickOffset)
		return;

	if (!Life()->GetWhetherDead()) {
		// show pregnancy stage on skeleton:
		SetPregnancyStage(Reproductive()->GetProgesteroneLevel());

		// drowning locus:
		myAirQualityLocus = 1.0f;
		int roomId;
		if (theApp.GetWorld().GetMap().GetRoomIDForPoint(myLimbs[BODY_LIMB_HEAD]->CentrePoint(), roomId))
		{
			int roomType;
			if (theApp.GetWorld().GetMap().GetRoomType(roomId, roomType))
			{
				if (roomType==WATER_ROOM_TYPE_1 || roomType==WATER_ROOM_TYPE_2)
					myAirQualityLocus = 0.0f;
			}
		}
	}
}






	/*
	// Draw the bounding box of the creature
	DrawLine(myMinX, myMinY, myMaxX, myMinY,
		255, 255, 255);
	DrawLine(myMaxX, myMinY, myMaxX, myMaxY,
		255, 255, 255);
	DrawLine(myMaxX, myMaxY, myMinX, myMaxY,
		255, 255, 255);
	DrawLine(myMinX, myMaxY, myMinX, myMinY,
		255, 255, 255);

	DrawLine(myExtremes[BODY_LIMB_LEFT_LEG].x, 
		myExtremes[BODY_LIMB_LEFT_LEG].y, myExtremes[BODY_LIMB_LEFT_LEG].x,
		myMinY, 255, 255, 0);

	DrawLine(myExtremes[BODY_LIMB_RIGHT_LEG].x, 
		myExtremes[BODY_LIMB_RIGHT_LEG].y, myExtremes[BODY_LIMB_RIGHT_LEG].x,
		myMinY, 255, 255, 0);

	DrawLine(myExtremes[BODY_LIMB_LEFT_LEG].x, 
		myMinY, myExtremes[BODY_LIMB_RIGHT_LEG].x,
		myMinY, 255, 255, 0);
	
	
	DrawLine(myDownFootPosition.x - 5, myDownFootPosition.y - 5,
			myDownFootPosition.x + 5, myDownFootPosition.y + 5,
		 255, 255, 255);
	DrawLine(myDownFootPosition.x - 5, myDownFootPosition.y + 5,
			myDownFootPosition.x + 5, myDownFootPosition.y - 5,
		 255, 255, 255);
	myResetLines = true;
	*/
	





// start a new animation, given a c++ string ("001002003R") (3-digit pose#s!)
void SkeletalCreature::SetAnimationString(char* animation)
{
	

#ifdef _DEBUG
    char* c = animation;
    while (*c)
    {
        ASSERT(isdigit(*c) || *c == 'R');
        c++;
    }
	ASSERT(strlen(animation) <= MAX_SKELETON_ANIMATION_STRING_LENGTH);
#endif

	if (!strcmp(myAnimationString, animation))			// already doing this animation?
		return;

	strcpy(myAnimationString,animation);								// copy anim$ to member
	myAnimationPointer = myAnimationString;									// point to start

    // Get the pose ID 
	int poseID = ((myAnimationPointer[0]-'0')*100) + ((myAnimationPointer[1]-'0')*10) + (myAnimationPointer[2]-'0'); 	

	strcpy(myTargetPoseString,myPoseStringTable[poseID]);					// define a new target pose
}


// Helper function for ANIM macro
bool SkeletalCreature::SetAnim(const uint8* anim, int length,int part)	// helper fn for ANIM macro
{
	// Can't set an animation if holding hands, do nothing
	if (myIsHoldingHandsWithThePointer)
		return true;

	if (length == 0)
	{
		SetAnimationString("013014015016R");
		return true;
	}

	static char buf[MAX_SKELETON_ANIMATION_STRING_LENGTH];
	uint8 pose;

	for (int i=0; i<length; i++) {
		char *c = &buf[i*3];
		pose = (uint8)anim[i];
		if (pose == 255) {
			// 255 ie repeat must be last in animation
			if (i != length-1)
				return false;
			strcpy(c, "R");
		}
		else 
			sprintf(c, "%03d", pose);
	}
	SetAnimationString(buf);
	return true;
}


// helper function for OVER macro.
// return TRUE if current animation is over.
bool SkeletalCreature::AnimOver(int part)
{
	

    return((!*myAnimationPointer)?1:0);
}





void SkeletalCreature::SetGait(int gait)
{
	

	SetAnimationString(myGaitTable[gait]);
}


// helper function for POSE macro. Return TRUE when desired pose has been reached
bool SkeletalCreature::ShowPose(int pose,int part)
{
	

	// Can't set a pose if holding hands, do nothing
	if (myIsHoldingHandsWithThePointer)
		return true;

	if (pose >= 0 && pose < MAX_POSES)
	{
		ResetAnimationString();									// cancel any animation
		SetTargetPoseString(myPoseStringTable[pose]);			// start new pose shift if necc
		return true;
	}
	else
		return false;
}

// Helper function: TOUC macro
// Reach out a bit more towards target xy, (legs at equal heights, head
// facing target, limbs & body as appropriate)
// Return >0 when tip of appendage is as near as it's going to get,
// 0 if still moving, or -1 if failed (no IT obj defined, or IT is below floor level)
// Incremental function - call every clock-tick
// 27/4/96: Parameter added to enable creature to "point out" an object, instead of
// "reach out and touch it", using the same pose tables. The pose is the same except the
// creature faces the camera not the object. This can be used so a creature can ask the
// user what an object is called, for example.
int SkeletalCreature::ReachOut(BOOL Point /*=FALSE*/)		// TRUE if should point to obj, not touch it
{
	

	if (!CanReachIt())
		return -1;									// failed to reach out and touch IT

	if (myIt.GetAgentReference().GetCarrier() == mySelf)					// If I'm already carrying it don't start a 
		return 1;									// silly carrot & stick oscillation!

    // choose pose appropriate to relative position of target
    int GridY = Map::FastFloatToInteger
		((myDownFootPosition.y-myItPosition.y)
            /(GetReachY()/REACHGRIDYPOSES));         // y index into table
    int GridX = Map::FastFloatToInteger
		((myItPosition.x-myDownFootPosition.x)
            /(GetReachX()/REACHGRIDXPOSES));         // +/- x index into table

    // ensure within limits of available poses, in case not quite in reach
    if (GridX<0)                                    // if targ is west of you
        GridX = -GridX;                             // cvt index to +ve
    if  (GridX>=REACHGRIDXPOSES)                    // GridX can't be -ve,
        GridX = REACHGRIDXPOSES-1;                  // cos already ABSed
    if  (GridY<0)
        GridY = 0;
    if  (GridY>=REACHGRIDYPOSES)
        GridY = REACHGRIDYPOSES-1;

    // then move a bit more towards that dirn & pose. Return 1 if pose has been reached
	ResetAnimationString();										// stop any existing animation

	char* pose = (char*)&(myPoseStringTable[POSE_TOUCH+GridX*REACHGRIDYPOSES+GridY][0]); // pose

	if	(Point) {									// if I'm to point it out not touch it
		char pointpose[MAX_POSE_STRING_LENGTH+1];				// then make a copy of the touch pose
		strcpy(pointpose,pose);						// and change its head angle to
		pointpose[_HEAD] = '4';						// face camera instead of object
	    return(SetTargetPoseString(pointpose))?1:0;		// and move towards that pose
	}

    return(SetTargetPoseString(pose))?1:0;				// else, just use pose straight from table
}














Vector2D SkeletalCreature::GetMapCoordOfWhereIPickUpAgentsFrom(int pose, bool bRelative) 
{
	

	if (bRelative)
		return Vector2D(myExtremes[BODY_LIMB_RIGHT_ARM].x - myPositionVector.x,
		myExtremes[BODY_LIMB_RIGHT_ARM].y - myPositionVector.y);
	else
		return Vector2D(myExtremes[BODY_LIMB_RIGHT_ARM].x,
		myExtremes[BODY_LIMB_RIGHT_ARM].y);
}

Vector2D SkeletalCreature::GetMyOffsetOfWhereIAmPickedUpFrom(int pose /*= -1*/)
{
	

	return Vector2D(myExtremes[BODY_LIMB_HEAD].x - myPositionVector.x, 0.0f);
}




///////////////////////// construct from a genome ///////////////////////////


/****
25/10/95 APPEARANCE GENES
Each APPEARANCE gene determines the visual variant of a single body part (or pair of parts in the
case of legs etc.).
To ensure that these genes do not get inherited from a single parent, it is important to spread
them evenly throughout the genome. Doing so will vastly increase the likelihood that crossing-
over will extract some of mum's appearance genes, and some of dad's.
Incidentally, where possible I should place APPEARANCE genes near to any INSTINCT or other
trait genes that might reasonably be expected to be linked - eg. if facial APPEARANCE and
mood-related genes are close to each other, then I can associate grumpy face types with grumpy
personality, with a reasonable likelihood of them remaining linked.


ATTACHMENT AND SPRITE LIBRARY FILENAMES
Files are in the format: PGAV.ATT or PGAV.SPR, 
where	P = part# ("A" to "Z"), 
		G = genus+sex*4, i.e. 0=male norn, 1=male grendel, 4=female norn...
		A = age ("0" to "9"), 
		V = variant ("0" to "9").
Eg. A012.ATT is the attachment table for variant 2 of the head ("A") of a male norn ("0") aged '1'.
If the explicit file cannot be found, then the system defaults by searching for:-
First: a younger age
Second: an earlier variant of the same or younger age
Third: the opposite sex version of any of the above
If no file is found after this, an error results.


*****/


// Helper for skeleton constructor. Read an attachment table file and return a pointer
// to a copyable BodyData struct containing the attachment data that needs to be
// copied to a Body constructor. The file is an exact binary list of items in the order
// that would be used for initialisation of a BodyData struct.
BodyData* GetBodyData( int Part,			// part number
					   int Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
					   int Age,				// IDEAL age & variant to look for
					   int Variant)

{
	static BodyData Data;
	DWORD Fsp;
	int i,j,jx,jy;

	Fsp = ValidFsp(Part,Genus,Sex,Age,Variant,			// find most appropriate file
					BODY_DATA_EXTENSION,BODY_DATA_DIR);

    if (Fsp)
    {
	    std::ifstream in(BuildFsp(Fsp,BODY_DATA_EXTENSION,BODY_DATA_DIR).c_str());
	    ASSERT(in.good());
	    for	(j=0; j<MAXVIEWS; j++) 
        {
		    for	(i=0; i<MAX_BODY_LIMBS; i++) 
            {
			    in >> jx >> jy;
			    Data.JoinX[i][j] = jx;
			    Data.JoinY[i][j] = jy;
		    }  
	    }		
    }
    else
    {
		// get the default nortn set for that age
		Fsp = ValidFsp(Part,0,Sex,Age,0,			// find most appropriate file
					BODY_DATA_EXTENSION,BODY_DATA_DIR);

		 if (Fsp)
			{
				std::ifstream in(BuildFsp(Fsp,BODY_DATA_EXTENSION,BODY_DATA_DIR).c_str());
				ASSERT(in.good());
				for	(j=0; j<MAXVIEWS; j++) 
				{
					for	(i=0; i<MAX_BODY_LIMBS; i++) 
					{
						in >> jx >> jy;
						Data.JoinX[i][j] = jx;
						Data.JoinY[i][j] = jy;
					}  
				}		
			}
		  else
		  {
			// if that doesn't work then oh dear!
				for	(j=0; j<MAXVIEWS; j++) 
				{
					for	(i=0; i<MAX_BODY_LIMBS; i++) 
						Data.JoinX[i][j] = Data.JoinY[i][j] = 0;
				}	
		  }
    }
	return &Data;										// data remains valid till next call here
}

// Ditto for LimbData
LimbData* GetLimbData( int Part,			// part number
					   int Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
					   int Age,				// IDEAL age & variant to look for
					   int Variant)

{
	static LimbData Data;
	DWORD Fsp;
	int sx,sy,ex,ey,i;

	Fsp = ValidFsp(Part,Genus,Sex,Age,Variant,			// find most appropriate file
					BODY_DATA_EXTENSION,BODY_DATA_DIR);

	if (Fsp)
	{
		// Read ASCII file as integer, then cvt to byte
		std::string fullFilePath = BuildFsp(Fsp,BODY_DATA_EXTENSION,BODY_DATA_DIR);
		std::ifstream in(fullFilePath.c_str());
		ASSERT(in.good());
		for	(i=0; i<MAXVIEWS; i++) {
			in >> sx >> sy >> ex >> ey;
			Data.StartX[i]=sx;
			Data.StartY[i]=sy; 
			Data.EndX[i]=ex;
			Data.EndY[i]=ey;
		}
	}

	return &Data;										// data remains valid till next call here
}


void SkeletalCreature::ChangePhysicalPlane(int plane)
{
	
	if (myBody == NULL)
		return;
	myBody->SetPlane(plane);
	UpdatePlotOrder();
}

// Construct body parts using a genome:
// - create a unique sprite file (using your moniker as its filename) 
//	 containing all the correct sprites for your body's appearance, species & sex
// - create all the Body/Limb entities (including optional tail) 
// - set each Body and Limb entity's attachment tables to suit the sprites
// Call this function during construction and whenever Age changes
bool SkeletalCreature::CreateSkeleton
	(Genome& genome, int Age)
{
	// Now that I know this, I can set up creature's Classifier in the following format:-
	// - Family  = CREATURE, 
	// - Genus   = 1 to 4 (norn, grendel, ettin, side), 
	// - Species = used to represent sex - 1=male or 2=female (allows sex-specific scripts)
	uint8 Sex = genome.GetSex();

	

	if(!myCompositeBodyPartGallery)
	{
		CAOSVar& sprite = theApp.GetWorld().GetGameVar("engine_creature_template_size_in_mb");
		_ASSERT(sprite.GetType() == CAOSVar::typeInteger);
		char buf[5];
		sprintf(buf,"%d",Age);
		std::string galleryName = genome.GetMoniker() + buf;

		std::string path = theDirectoryManager.GetDirectory(CREATURE_DATABASE_DIR);
		// for now until I redo the Creature Gallery lets only resize if the
		// creature lib is empty
		myCompositeBodyPartGallery = new CreatureGallery(path,
														sprite.GetInteger(),
														galleryName);
		_ASSERT(myCompositeBodyPartGallery);
		ReadAppearanceGenes(genome, *myTintin);
	}



	if(myJustAgedFlag == true)
		return true;

	// don't destroy all your parts if you already have them
	// just reload them
	if(HasBody())
	{
		return ReloadSkeleton(genome,Age);
	}

	// Destroy any existing body parts, prior to defining a new set (will also
	// destroy image gallery and remove old sprites from cache)
	DestroySkeleton();

	// use a tint manager for the duration of creating this norn

	if(myLastAge != Age)
	{
		ReadAppearanceGenes(genome, *myTintin);
	}


	myLastAge = Age;

	ASSERT(GetMoniker() == genome.GetMoniker());

	// create one body part at a time
	return CreateBodyPart(Sex, Age, *myTintin);
}


bool SkeletalCreature::HasBody()
{
	

	// is it OK to assume that if it has a body then it will
	// have all the limbs that it needs too??
	if(myBody)
		return true;
		 
	return false; 
}

bool SkeletalCreature::ReloadSkeleton(Genome& genome,
							  int Age)
{
	

	byte Sex=genome.GetSex();
	int32 HasHair =0;
	int32 HasTail =0;

	// work out how many actual parts are needed for this creature, ie. don't count
	// optional tail if it has not been genetically specified
	int ValidParts = NUMPARTS;



	// Create necessary Body and Limb entities using the attachment-point tables 
	// specified for that part, genus, sex, age and variant
	// (or defaults thereof if no specific file exists)

    // ...start with the body...
	myBody->PrepareForReloadingGallery();
	
	((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->PrepareForReloadingGallery();


	for(int32 i = BODY_LIMB_LEFT_LEG; i< MAX_BODY_LIMBS;i++)
	{
		myLimbs[i]->PrepareForReloadingGallery();
	}


	if(myLastAge != Age)
	{
		// use a tint manager for the duration of creating this norn
		ReadAppearanceGenes(genome, *myTintin);
	}
	// we can't continue if a gallery wasn't created
	// get a gallery for each age stage
	char buf[5];
	sprintf(buf,"%d",Age);
	std::string galleryName = GetMoniker() + buf;

	FilePath name(galleryName + ".c16",IMAGES_DIR );
#ifdef NETTY
	theApp.Netty->TextOut("1",galleryName.c_str());
#endif
	bool ok = false;

	if(myCompositeBodyPartGallery->IsComplete())
	{
		ok = true;
	}
	else
	{

		for(int part = 0; part < ValidParts; part++)
		{
			BuildGalleries(Sex, Age,myBodyPartToBeCreated,*myTintin);
		}

	
	 if(myCompositeBodyPartGallery->CreateGallery(name.GetFullPath(),
										   GetUniqueID(),
										   ValidParts,
										   Sex,
										   Age,
										   myBodySprites,
										   myCompositeBodyPartGallery,
										   myNumSpritesInFile,
										   true))
									ok = true;


	}


	if(ok)
	{			            

	FilePath name(galleryName + ".c16",IMAGES_DIR );
#ifdef NETTY
	theApp.Netty->TextOut("2",galleryName.c_str());
#endif

	myBody->Reload( GetBodyData(PART_BODY, myGenera[PART_BODY], Sex, Age, myVariants[PART_BODY]),
					name,
					myBodySprites[PART_BODY],
					Age);




	LimbData* (*GetLimbDataPtr)( int,			// part number
					   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int,				// IDEAL Sex to look for (MALE/FEMALE)
					   int,				// IDEAL age & variant to look for
					   int);

	GetLimbDataPtr = &GetLimbData;



	((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->Reload(Age,name,GetLimbDataPtr,myBodySprites);
	///////////////////////////////////////////////////////////////////////////////
	// right leg
	///////////////////////////////////////////////////////////////////////////////

	int32 partNumber = PART_LTHIGH;
	for(int32 i = BODY_LIMB_LEFT_LEG; i < MAX_BODY_LIMBS; i++)
	{

		// i is the part number and will be updated by the limbchain
	myLimbs[i]->Reload(partNumber,
			GetLimbDataPtr,
			name,
			myBodySprites,
			0,
			Age);

	}

	GeneratePlotOrder();
	// Only set my current pose string if the world is here now
	// Otherwise problems ensue with carried agents etc.
	if (!theApp.GetWorld().GetWhetherLoading())
		SetCurrentPoseString(myCurrentPoseString);						// reset planes, pose$s etc.
   // UpdatePlotOrder();
	// if you were being tracked then tell the main camera that you have
	// grown up
	if(myBeingTrackedFlag)
		theMainView.YourTrackObjectHasGrownUp(mySelf);

	myNumSpritesInFile =0;

	return true;
	} //end check gallery was created

	return false;
}


void SkeletalCreature::Trash() 
{
	myIt = NULLHANDLE;
	DestroySkeleton();

	myBody = NULL;

	if(myCompositeBodyPartGalleryForNextAgeStage != myCompositeBodyPartGallery )
	{
		delete myCompositeBodyPartGalleryForNextAgeStage;
		myCompositeBodyPartGalleryForNextAgeStage = NULL;
	}



	delete myCompositeBodyPartGallery;
	myCompositeBodyPartGallery = NULL;

	delete myTintin;

	// skip CompoundAgent
	base::OwnTrash();

	// This must be the last line in the function
	Agent::Trash();

}



// Kill body & limb entities (during destruction or prior to redefining appearance).
void SkeletalCreature::DestroySkeleton()
{
	

    int i;
    Limb *l,*p;

    for (i=0; i<MAX_BODY_LIMBS; i++) {            // delete all limb chains...
        while ((l=myLimbs[i])!=NULL) {    // while some chain left
            p = NULL;                          // (used for NULLing ptrs)
            while (l->GetNextLimb()) {           // find its last element
                p = l;                      // & next to last
                l = l->GetNextLimb();
            }
            if (p)                          // annul pointer to this element
                p->SetNextLimb(NULL);            // (within limb itself...
            else
                myLimbs[i] = NULL;             // ...or within Body)
            delete l;                       // & remove last element
        }                                   // find new last element & repeat
    }                                       // repeat with next chain

	if( myBody != NULL )	// sanity check.
	{
		delete myBody;                           // delete body
		myBody = NULL;
	}
}

// new style funbctions for wearing overlays now
// we can wear different items on each individual body part

int SkeletalCreature::GetOverlayIndex(int bodypart)
{
	

	int index = -1;

switch(bodypart)
	{
	case PART_HEAD:
		{
			_ASSERT(myLimbs[BODY_LIMB_HEAD]);
			index = myLimbs[BODY_LIMB_HEAD]->GetCurrentIndex();
			break;
		}
	case PART_BODY:
		{
			_ASSERT(myBody);
			index = myBody->GetCurrentIndex();
			break;
		}
	case PART_LTHIGH:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			index = myLimbs[BODY_LIMB_LEFT_LEG]->GetCurrentIndex();
			break;
		}
	case PART_LSHIN:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			index = myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_LFOOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			index = myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_RTHIGH:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			index = myLimbs[BODY_LIMB_RIGHT_LEG]->GetCurrentIndex();
			break;

		}
	case PART_RSHIN:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			index = myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_RFOOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			index = myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_LHUMERUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
			index = myLimbs[BODY_LIMB_LEFT_ARM]->GetCurrentIndex();
			break;
		}
	case PART_LRADIUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
			index = myLimbs[BODY_LIMB_LEFT_ARM]->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_RHUMERUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
			index = myLimbs[BODY_LIMB_RIGHT_ARM]->GetCurrentIndex();
			break;
		}
	case PART_RRADIUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
			index = myLimbs[BODY_LIMB_RIGHT_ARM]->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_TAILROOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_TAIL]);
			index = myLimbs[BODY_LIMB_TAIL]->GetCurrentIndex();
			break;
		}
	case PART_TAILTIP:
		{
			_ASSERT(myLimbs[BODY_LIMB_TAIL]);
			index = myLimbs[BODY_LIMB_TAIL]->GetNextLimb()->GetCurrentIndex();
			break;
		}
	case PART_LEFT_EAR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetOverlayIndex(HEAD_LIMB_LEFT_EAR);
		}
	case PART_RIGHT_EAR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetOverlayIndex(HEAD_LIMB_RIGHT_EAR);
		}
	case PART_HAIR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetOverlayIndex(HEAD_LIMB_HAIR);
		}
	default:
		return index;
	}
	return index;
}


int SkeletalCreature::GetClothingSet(int bodypart, int layer)
{
	

	int index = -1;

	switch(bodypart)
	{
	case PART_HEAD:
		{
			_ASSERT(myLimbs[BODY_LIMB_HEAD]);
			index = myLimbs[BODY_LIMB_HEAD]->GetOverlay(layer);
			break;
		}
	case PART_BODY:
		{
			_ASSERT(myBody);
			index = myBody->GetOverlay(layer);
			break;
		}
	case PART_LTHIGH:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			index = myLimbs[BODY_LIMB_LEFT_LEG]->GetOverlay(layer);
			break;
		}
	case PART_LSHIN:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			index = myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_LFOOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			index = myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_RTHIGH:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			index = myLimbs[BODY_LIMB_RIGHT_LEG]->GetOverlay(layer);
			break;

		}
	case PART_RSHIN:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			index = myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_RFOOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			index = myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_LHUMERUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
			index = myLimbs[BODY_LIMB_LEFT_ARM]->GetOverlay(layer);
			break;
		}
	case PART_LRADIUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
			index = myLimbs[BODY_LIMB_LEFT_ARM]->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_RHUMERUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
			index = myLimbs[BODY_LIMB_RIGHT_ARM]->GetOverlay(layer);
			break;
		}
	case PART_RRADIUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
			index = myLimbs[BODY_LIMB_RIGHT_ARM]->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_TAILROOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_TAIL]);
			index = myLimbs[BODY_LIMB_TAIL]->GetOverlay(layer);
			break;
		}
	case PART_TAILTIP:
		{
			_ASSERT(myLimbs[BODY_LIMB_TAIL]);
			index = myLimbs[BODY_LIMB_TAIL]->GetNextLimb()->GetOverlay(layer);
			break;
		}
	case PART_LEFT_EAR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetClothingSet(HEAD_LIMB_LEFT_EAR,layer);
		}
	case PART_RIGHT_EAR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetClothingSet(HEAD_LIMB_RIGHT_EAR,layer);;
		}
	case PART_HAIR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetClothingSet(HEAD_LIMB_HAIR,layer);;
		}
	default:
		return index;
	}
	return index;
}



bool SkeletalCreature::WearOutfit(int set, int layer)
{
	

		_ASSERT(myLimbs[BODY_LIMB_HEAD]);
		myLimbs[BODY_LIMB_HEAD]->SetClothing(set,layer);

		_ASSERT(myBody);
		myBody->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
		myLimbs[BODY_LIMB_LEFT_LEG]->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
		myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
		myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->GetNextLimb()->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
		myLimbs[BODY_LIMB_RIGHT_LEG]->SetClothing(set,layer);


		_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
		myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
		myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->GetNextLimb()->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
		myLimbs[BODY_LIMB_LEFT_ARM]->SetClothing(set,layer);

		_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
		myLimbs[BODY_LIMB_LEFT_ARM]->GetNextLimb()->SetClothing(set,layer);

	
		_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
		myLimbs[BODY_LIMB_RIGHT_ARM]->SetClothing(set,layer);
		
		_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
		myLimbs[BODY_LIMB_RIGHT_ARM]->GetNextLimb()->SetClothing(set,layer);
	
		_ASSERT(myLimbs[BODY_LIMB_TAIL]);
		myLimbs[BODY_LIMB_TAIL]->SetClothing(set,layer);


		_ASSERT(myLimbs[BODY_LIMB_TAIL]);
		myLimbs[BODY_LIMB_TAIL]->GetNextLimb()->SetClothing(set,layer);
		
		
		((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->WearItem(HEAD_LIMB_LEFT_EAR,set,layer);
		
		((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->WearItem(HEAD_LIMB_RIGHT_EAR,set,layer);
	
		((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->WearItem(HEAD_LIMB_HAIR,set,layer);
		return true;
}

bool SkeletalCreature::WearItem(int bodypart, int set , int layer)
{
	

	switch(bodypart)
	{
	case PART_HEAD:
		{
			_ASSERT(myLimbs[BODY_LIMB_HEAD]);
			myLimbs[BODY_LIMB_HEAD]->SetClothing(set,layer);
			break;
		}
	case PART_BODY:
		{
			_ASSERT(myBody);
			myBody->SetClothing(set,layer);
			break;
		}
	case PART_LTHIGH:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			myLimbs[BODY_LIMB_LEFT_LEG]->SetClothing(set,layer);
			break;
		}
	case PART_LSHIN:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_LFOOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_LEG]);
			myLimbs[BODY_LIMB_LEFT_LEG]->GetNextLimb()->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_RTHIGH:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			myLimbs[BODY_LIMB_RIGHT_LEG]->SetClothing(set,layer);
			break;

		}
	case PART_RSHIN:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_RFOOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_LEG]);
			myLimbs[BODY_LIMB_RIGHT_LEG]->GetNextLimb()->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_LHUMERUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
			myLimbs[BODY_LIMB_LEFT_ARM]->SetClothing(set,layer);
			break;
		}
	case PART_LRADIUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_LEFT_ARM]);
			myLimbs[BODY_LIMB_LEFT_ARM]->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_RHUMERUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
			myLimbs[BODY_LIMB_RIGHT_ARM]->SetClothing(set,layer);
			break;
		}
	case PART_RRADIUS:
		{
			_ASSERT(myLimbs[BODY_LIMB_RIGHT_ARM]);
			myLimbs[BODY_LIMB_RIGHT_ARM]->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_TAILROOT:
		{
			_ASSERT(myLimbs[BODY_LIMB_TAIL]);
			myLimbs[BODY_LIMB_TAIL]->SetClothing(set,layer);
			break;
		}
	case PART_TAILTIP:
		{
			_ASSERT(myLimbs[BODY_LIMB_TAIL]);
			myLimbs[BODY_LIMB_TAIL]->GetNextLimb()->SetClothing(set,layer);
			break;
		}
	case PART_LEFT_EAR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->WearItem(HEAD_LIMB_LEFT_EAR,set,layer);
		}
	case PART_RIGHT_EAR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->WearItem(HEAD_LIMB_RIGHT_EAR,set,layer);
		}
	case PART_HAIR:
		{
			return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->WearItem(HEAD_LIMB_HAIR,set,layer);
		}
	default:
		return false;
	}

	return true;
}

bool SkeletalCreature::HitTest(const Vector2D& point, bool alwaysTestPixelPerfectly /* = false*/)
{
	Box rect;
	GetAgentExtent(rect);
	return rect.PointInBox(point) ? true : false;
}


void SkeletalCreature::GetBodyPartFileName(int bodypart, std::string& path)
{

	if(bodypart < MAX_BODY_LIMBS)
	{
		Limb* limb = myLimbs[bodypart];
		limb->GetBodyPartFileName(path);

	}
	else
	{
		// this is probably part of the head
		((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->GetHeadPartFileName(bodypart,path);
	}
}

void SkeletalCreature::ChangeCameraShyStatus(bool shy)
{
	myEntitiesAreCameraShy = (myAttributes & attrCameraShy) == attrCameraShy;
}

// virtual
void SkeletalCreature::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stipplestartAt/*=0*/) 
{
	

	if(myBody)
		myBody->DrawLine(x1,y1,x2,y2,
		lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stipplestartAt);
}


// virtual
void SkeletalCreature::ResetLines()
{
	if(myBody)
		myBody->ResetLines();
}




bool SkeletalCreature::Approach(bool doApproachNotRetreat)
{
	Walk();

	// Map:
	Map& map = theApp.GetWorld().GetMap();
	int creatureFootRoomId;
	if (!GetCreatureRoomID(creatureFootRoomId))
	{
		ResetAnimationString();
		return false;
	}
	int attnId = Motor()->GetCurrentAttentionId();
	int creatureTargCategory = SensoryFaculty::GetCategoryIdOfAgent(mySelf);
	int caIndex = theAgentManager.GetSmellIdFromCategoryId(attnId);
	int creatureRealRoomId = -1;
	// set it so you can't smell yourself if you're navigating to other creatures of your category:
	if (attnId==creatureTargCategory)
	{	
		if (GetRoomID(creatureRealRoomId))
			map.AlterCAEmission(creatureRealRoomId, caIndex, -GetCAIncrease());
		else
			creatureRealRoomId = -1;
	}
	int mapDirection = GO_NOWHERE;
	map.WhichDirectionToFollowCA(creatureFootRoomId, caIndex, mapDirection, doApproachNotRetreat);// true means approach
	// set it so you can smell yourself again:
	if (attnId==creatureTargCategory && creatureRealRoomId>=0)
		map.AlterCAEmission(creatureRealRoomId, caIndex, +GetCAIncrease());


	// Send peak of smell signal:
	if (mapDirection==GO_NOWHERE)
	{ 
		Sensory()->Stimulate(mySelf, STIM_REACHED_PEAK_OF_SMELL0 + caIndex, -1);
	}

	// Regular walking:
	if (GetItAgent().IsValid()) {
		SetIntrospective(false);
		if (!CannotBeMuchCloser())		// keep trying to get to it:
			return true;

		ResetAnimationString();
		return false;
	}
	else
	{
		// CA stuff:
		SetIntrospective(true);
		if (mapDirection==GO_WALK_LEFT || mapDirection==GO_WALK_RIGHT) 
		{
			// Set to walk left or right:
			Vector2D pos = GetDownFootPosition();
			pos.x += (mapDirection==GO_WALK_LEFT) ? -500.0f : +500.0f;
			SetItPosition(pos);
			SetIntrospective(false);
		} 
		else 
		{
			//Otherwise if we're there already keep walking anyway (introspectively)
			//in the hope of getting to the linked door.
			if (GetDirection()==NORTH || GetDirection()==SOUTH)
			{
				// (but if we're walking into the screen try to walk west or east)
				SetDirection(RndFloat()>0.5f ? WEST : EAST);
			}

			// Stimulate the creature that we want to go a particular direction:
			Sensory()->Stimulate(mySelf, STIM_GO_NOWHERE + mapDirection, -1);
		}
		return true;
	}
}



// Helper function for APPR & WALK macros. Set up the best animation for walking, by looking at
// the myGaitLoci[] receptors. If none active, use normal gait
void SkeletalCreature::Walk()
{
	// find strongest receptor signal for which I have a gait defined
	float Strength=0.0f;
	int Choice=0;							// default to gait[0]
	for (int i=0; i<MAX_GAITS; i++) {
		if (myGaitTable[i][0]) {
			if (myGaitLoci[i] > Strength) {
				Strength = myGaitLoci[i];
				Choice = i;
			}
		}
	}
	SetAnimationString(myGaitTable[Choice]);		// use either strongest receptor's gait, or default
}
 
// Express any genes relevant to current myAge
bool SkeletalCreature::ExpressParticularGenes(Genome& g2, int age) 
{
	try
	{
		// Others can be expressed at any time...
		if(!CreateSkeleton(g2,age)) {				// express new body appearance now I'm older
			return false;
		} 

		int i,j;
		int c;
		// Read the POSE genes to determine the creature's poses
		// Late-switching genes overwrite equivalent predecessors
		g2.Reset();
		while ((g2.GetGeneType(CREATUREGENE,G_POSE,NUMCREATURESUBTYPES))!=false) {
			j = g2.GetCodon(0,MAX_POSES-1);		// pose# to define
			for	(i=0; i<MAX_POSE_STRING_LENGTH; i++) {
				// copy the pose string from gene
				c = g2.GetCodon(' ','Z');		

				// ensure characters are valid pose elements
				if (i == 0)
				{
					if (!strchr("?!X0123",c))
						c = 'X';
				}
				else if (i == 1)
				{
					if (!strchr("?X012345",c))
						c = 'X';
				}
				else
				{
					if (!strchr("X0123",c))
						c = 'X';
				}

				myPoseStringTable[j][i] = c;
			}
			myPoseStringTable[j][MAX_POSE_STRING_LENGTH] = '\0';		// & terminate string
		}

		// Read the GAIT genes to determine the creature's walking animations
		// Late-switching genes overwrite equivalent predecessors
		g2.Reset();
		while ((g2.GetGeneType(CREATUREGENE,G_GAIT,NUMCREATURESUBTYPES))!=false) {
			int base=0;
			j = g2.GetCodon(0,MAX_GAITS-1);		// gait# to define
			for	(i=0; i<8; i++) {
				c = g2.GetCodon(0,MAX_POSES-1);	// get up to eight pose numbers (0=end)
				if	(c) {						// store each pose# as 3 ascii digits
					base = 3 * i;
					myGaitTable[j][base] = (c/100)+'0'; 		// ("001" to "999")
					myGaitTable[j][base+1] = ((c/10)%10)+'0';
					myGaitTable[j][base+2] = (c%10)+'0';
					ASSERT(isdigit(myGaitTable[j][base]));
					ASSERT(isdigit(myGaitTable[j][base+1]));
					ASSERT(isdigit(myGaitTable[j][base+2]));
				}
				else 							// but if pose# is zero, end of anim
					break;
			}
			base = 3 * i;
			myGaitTable[j][base] = 'R';				// then store 'R' cos all gaits repeat
			myGaitTable[j][base+1] = '\0';			// and then terminate string
			// note: pose# 0 can signify 'end' cos pose0 is a reach-out pose not a walking one
		}

	}
	catch(Genome::GenomeException& e)
	{
		ErrorMessageHandler::Show(e, std::string("SkeletalCreature::ExpressParticularGenes"));
		return false;
	}
	return true;
}


bool SkeletalCreature::GetExtremePoint(int bodypart,Vector2D& point)
{
	

	if(bodypart < MAX_BODY_LIMBS)
	{
		point =  myExtremes[bodypart];
		return true;
	}

	return false;
}

// blame Alima for these
void SkeletalCreature::SetPregnancyStage(float progesteroneLevel)
{
	

	myBody->SetPregnancyStage(progesteroneLevel);
}

// x means tidy the hair x stages
// -x means untidy the hair x stages
void SkeletalCreature::ChangeHairStateInSomeWay(int action)
{
	

	if(action < 0)
	{
		for(int i =0; i<abs(action); i++)
		{
			((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->RuffleHair();
		}
	}
	else if(action > 0)
	{
		for(int i =0; i<action; i++)
		{
			((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->TidyHair();
		}
	}
}




bool SkeletalCreature::CannotBeMuchCloser()
{
	if (myIntrospectiveFlag)
		return true;

	return 
		// can reach equally either side but we stop slightly closer than we can reach
		// just to be sure.
		fabsf(myDownFootPosition.x - myItPosition.x ) <= 
		(GetReachX()*0.7f);
}



float SkeletalCreature::GetReachX()
{
	return myCurrentWidth*0.8;
}

float SkeletalCreature::GetReachY()
{
	return myCurrentHeight*1.25f;
}


// return true if creature within reach of IT
bool SkeletalCreature::CanReachIt()
{
    if (myIntrospectiveFlag)
		return false;

    return
		// can reach equally either side:
		fabsf(myDownFootPosition.x - myItPosition.x ) <= GetReachX() &&
		// and reach a little below itself:
		myItPosition.y-myDownFootPosition.y <= (GetReachY()) &&
		// and pretty high too:
		myDownFootPosition.y-myItPosition.y <= GetReachY();
}	



// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool SkeletalCreature::Write(CreaturesArchive &archive) const
{	
	// skip compound agent serialisation:
	Agent::Write(archive);

	int i;
	archive << myDoubleSpeedFlag;
	archive << myIsHoldingHandsWithThePointer;


	archive <<	myMotherMoniker;
	archive <<	myFatherMoniker;

	for(i = 0; i< NUMPARTS; i++)
	{
		archive << myVariants[i];	// variant # to use for each part
		archive << myGenera[i];
		archive <<	myBodySprites[i];
	}

	// uncommented out by alima - now the skeleton can reload itself
	archive <<	myBody;						// ptr to Body Ent
	for( i = 0; i < MAX_BODY_LIMBS; ++i )
		archive << myLimbs[i];

	archive <<	myCurrentDirection;					// which dirn facing (ESWN)
	archive <<	myCurrentDownFoot;					// which foot is 'down' 0/1 LEFT/RIGHT
	archive <<	myDownFootPosition;
	archive <<	myUpFootPosition;
	archive <<   myNormalPlane;				// plot plane when not in a vehicle
	archive <<  myPreviousUpFootPosition;

	archive.Write( myCurrentPoseString, MAX_POSE_STRING_LENGTH+1 );
	archive.Write( myTargetPoseString, MAX_POSE_STRING_LENGTH+1 );	// Pose that UpdatePose() is currently striving towards

	archive.Write( myAnimationString, MAX_SKELETON_ANIMATION_STRING_LENGTH );		// current animation string
	archive << int( myAnimationPointer - myAnimationString );				


	archive << myMinX;
	archive << myMinY;
	archive << myMaxX;
	archive << myMaxY;


	for( i = 0; i < MAX_BODY_LIMBS; ++i )
	{
		archive << myExtremes[i];
	}

	archive <<	myFacialExpression;			// 012: normal/happy/sad face
	archive <<	myEarSet;
	archive <<		myEyeState;					// 01: eyes closed/open
	archive <<	myBodyIsSuitedUp;
	archive << myBeingTrackedFlag;


	for( i = 0; i < MAX_POSES; ++i )
		archive.Write( myPoseStringTable[i], MAX_POSE_STRING_LENGTH+1 );

	for( i = 0; i < MAX_GAITS; ++i )
		archive.Write( myGaitTable[i], MAX_SKELETON_ANIMATION_STRING_LENGTH );


	// Chemo-receptor and emitter loci that are controlled/monitored by skeleton
	// rather than creature
	archive.WriteFloatRefTarget( myMusclesLocus );
    archive.WriteFloatRefTarget( myUpslopeLocus );
    archive.WriteFloatRefTarget( myDownslopeLocus );
	for( i = 0; i < MAX_GAITS; ++i )
		archive.WriteFloatRefTarget( myGaitLoci[i] );		// trigger different walking gaits ([0]=default)


	// to do with IT (attended-to object)...
	archive <<	myIt;					// object of this creature's attention
									// NULL if none
	archive << myIntrospectiveFlag;			// for extraspective actions like GO EAST and GO WEST
	archive << myItPosition;				// Last known xy of target to touch
									// when trying to Activate It etc.

	archive << myHowToTouchIt;		// deprecated but kept for serialisation compatibility
	archive << myEntitiesAreCameraShy;

	archive << myStandStill;
	archive << myPreviousUpFootPosition;
	archive << myPreviousMinY;

	// write out but not with agent:
	base::OwnWrite(archive);
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool SkeletalCreature::Read(CreaturesArchive &archive)
{
	myReach = Vector2D((float)atof(theCatalogue.Get("Creature Reach", 0)), (float)atof(theCatalogue.Get("Creature Reach", 1)));

	
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		// skip compound agent serialisation:
		if(!Agent::Read(archive))
			return false;

		int i;
		archive >> myDoubleSpeedFlag;
		archive >> myIsHoldingHandsWithThePointer;

		archive >>	myMotherMoniker;
		archive >>	myFatherMoniker;

		for(i = 0; i< NUMPARTS; i++)
		{
			archive >> myVariants[i];	// variant # to use for each part
			archive >> myGenera[i];
			archive >> myBodySprites[i];
		}

		archive >>	myBody;						// ptr to Body Ent

		// for some reason this doesn't read the Creature head
		for( i = 0; i < MAX_BODY_LIMBS; ++i )
			archive >> myLimbs[i];

		archive >>	myCurrentDirection;					// which dirn facing (ESWN)
		archive >>	myCurrentDownFoot;					// which foot is 'down' 0/1 LEFT/RIGHT
		archive >>	myDownFootPosition;
		archive >>	myUpFootPosition;
		archive >>  myNormalPlane;				// plot plane when not in a vehicle

		//Reset normal plane
		myNormalPlane = theAgentManager.UniqueCreaturePlane( mySelf );



		archive >>  myPreviousUpFootPosition;

		archive.Read( myCurrentPoseString, MAX_POSE_STRING_LENGTH+1 );
		archive.Read( myTargetPoseString, MAX_POSE_STRING_LENGTH+1 );	// Pose that UpdatePose() is currently striving towards

		archive.Read( myAnimationString, MAX_SKELETON_ANIMATION_STRING_LENGTH );		// current animation string

		int temp;
		archive >> temp;
		myAnimationPointer = myAnimationString + temp;

		archive >> myMinX;
		archive >> myMinY;
		archive >> myMaxX;
		archive >> myMaxY;


		for( i = 0; i < MAX_BODY_LIMBS; ++i )
		{
			archive >> myExtremes[i];
		}

		archive >>	myFacialExpression;			// 012: normal/happy/sad face
		archive >>	myEarSet;
		archive >>		myEyeState;					// 01: eyes closed/open
		archive >>	myBodyIsSuitedUp;
		archive >> myBeingTrackedFlag;


		for( i = 0; i < MAX_POSES; ++i )
			archive.Read( myPoseStringTable[i], MAX_POSE_STRING_LENGTH+1 );

		for( i = 0; i < MAX_GAITS; ++i )
			archive.Read( myGaitTable[i], MAX_SKELETON_ANIMATION_STRING_LENGTH );



		// Chemo-receptor and emitter loci that are controlled/monitored by skeleton
		// rather than creature
		archive.ReadFloatRefTarget( myMusclesLocus );
		archive.ReadFloatRefTarget( myUpslopeLocus );
		archive.ReadFloatRefTarget( myDownslopeLocus );
		for( i = 0; i < MAX_GAITS; ++i )
			archive.ReadFloatRefTarget( myGaitLoci[i] );		// trigger different walking gaits ([0]=default)


		// to do with IT (attended-to object)...
		
		archive >>	myIt;					// object of this creature's attention
										// NULL if none
		archive >> myIntrospectiveFlag;			// for extraspective actions like GO EAST and GO WEST
		archive >> myItPosition;				// Last known xy of target to touch
										// when trying to Activate It etc.
		archive >> myHowToTouchIt;		// deprecated but kept for serialisation compatibility
		archive >> myEntitiesAreCameraShy;

		archive >> myStandStill;
		archive >> myPreviousUpFootPosition;
		archive >> myPreviousMinY;

		// don't serialise Agent again:
		if(!base::OwnRead(archive))
			return false;

		// If cloning a SkeletalCreature, we have to remake our skeleton later,
		// when the moniker is known
		if (!archive.GetCloningASkeletalCreature())
			RemakeSkeletonAfterSerialisation();
		}
	else
	{
		_ASSERT(false);
		return false;
	}

	// Imported creatures don't need position checking as later they will be
	// set in an invalid position anyway... Cloned creatures don't need
	// position checking either as they'll be in an OK place anyway.
	if (!archive.GetCloningASkeletalCreature())
		if (myMovementStatus != INVEHICLE) 
			EmergencySafePositionRoom();

	return true;
}

void SkeletalCreature::RemakeSkeletonAfterImporting()
{
	// Creatures are imported to a possibly invalid position -
	// we mark it as such, until CAOS moves the creature to a valid
	// position
	myInvalidPosition = true;

	RemakeSkeletonAfterSerialisation();
}

void SkeletalCreature::RemakeSkeletonAfterSerialisation()
{
	Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

	CreateSkeleton(g, Life()->GetAge());
}

bool SkeletalCreature::Visibility(int scope)
{
	
	// return true if any part of the creature is on screen

	if(myBody->Visible(scope))
		return true;
	Limb* currentLimb = NULL;

	for (int i=0; i<MAX_BODY_LIMBS; i++) 
	{                    // for each limb chain

        for (currentLimb = myLimbs[i];                     // descend limb chain
             currentLimb!=NULL;
             currentLimb = currentLimb->GetNextLimb()) 
			 {
				 if(currentLimb->Visible(scope))
					 return true;
			 }
	}

	return ((CreatureHead*)(myLimbs[BODY_LIMB_HEAD]))->Visibility(scope);
}



void SkeletalCreature::ReadAppearanceGenes(Genome& genome, TintManager& tinty)
{
	int32 HasHair =0;
	int32 HasTail =0;
	uint8 Genus=0;
	int region, variant, vgenus = 0;


	// Which body parts have their variant data set by which Appearance gene (eg. REGION_LEGS
	// must determine the variant#s of both thighs, shins & feet.
	// Up to 6 parts may be set for each region - entries are part#s or -1 if unused.
	static int RegionParts[NUMREGIONS][6] = {
		{PART_HEAD,PART_LEFT_EAR,PART_RIGHT_EAR,-1,-1,-1},					// REGION_HEAD
		{PART_BODY,-1,-1,-1,-1,-1},					// REGION_BODY
		{PART_LFOOT,PART_LSHIN,PART_LTHIGH,
		 PART_RFOOT,PART_RSHIN,PART_RTHIGH},		// REGION_LEGS
		{PART_LRADIUS,PART_LHUMERUS,
		 PART_RRADIUS,PART_RHUMERUS,-1,-1},			// REGION_ARMS
		{PART_TAILTIP,PART_TAILROOT,-1,-1,-1,-1},	// REGION_TAIL
		{PART_HAIR,-1,-1,-1,-1,-1},					//REGION_HAIR
	};

	// Read the (only) GENUS (Header) gene and determine creature's genus
	// and his mum's & dad's names
	genome.Reset();
	while ((genome.GetGeneType(CREATUREGENE,G_GENUS,NUMCREATURESUBTYPES,SWITCH_AGE))!=FALSE) {
		Genus = genome.GetCodon(0,3);

		char charArray[64];

		int i;
		for ( i = 0; i < 64; ++i)
		{
			char c = genome.GetChar();
			charArray[i] = c;
		}

// (now in creature.cpp):
		// used for detecting family members with senses -
		// and no doubt generally useful!
//		myMotherMoniker = std::string(charArray, 32).c_str(); 
//		myFatherMoniker = std::string(charArray + 32, 32).c_str();
		// c_str() in these lines is to force stripping to trailing zero -
		// remember that std::strings have a length, and can actually contain
		// zero characters in the middle of them.

		 // Assume all body part variants will be the correct genus.
		for (i = 0; i < NUMPARTS; i++)
			myGenera[i] = Genus;

		// (re-)read the APPEARANCE genes to determine which visual variant of any body part to use
		// and build an array showing which to use for each individual part
		genome.Reset();
		while ((genome.GetGeneType(CREATUREGENE,G_APPEARANCE,NUMCREATURESUBTYPES,SWITCH_AGE))!=FALSE) {

			region = genome.GetCodon(0,NUMREGIONS-1);		// body region being defined
			variant = genome.GetCodon(0, NUMVARIANTS-1);	// visual variant to use
			vgenus = genome.GetCodon(0, NUMCREATUREGENI);  // genus of creature the variant applies to.

			for	(i=0; (i<6)&&(RegionParts[region][i]!=-1); i++) // for each valid entry in LUT
			{
				myVariants[RegionParts[region][i]] = variant;		// set that part to correct variant
				myGenera[RegionParts[region][i]] = vgenus;
			}

			if	(region==REGION_TAIL) HasTail++;
			if	(region==REGION_HAIR) HasHair++;			//
		};

		// if the genome has no tail or hair then 
		// add some in the same variant as the head
		variant = myVariants[RegionParts[REGION_HEAD][0]];
		vgenus = myGenera[RegionParts[REGION_HEAD][0]];

		if(!HasHair)
		{
			for	(i=0; (i<6)&&(RegionParts[REGION_HAIR][i]!=-1); i++) 
			{
			
				myVariants[RegionParts[REGION_HAIR][i]] = variant;
				myGenera[RegionParts[REGION_HAIR][i]] = vgenus;
			}
		}
		if(!HasTail)
		{
			for	(i=0; (i<6)&&(RegionParts[REGION_TAIL][i]!=-1); i++) 
			{
				myVariants[RegionParts[REGION_TAIL][i]] = variant;		
				myGenera[RegionParts[REGION_TAIL][i]] = vgenus;
			}
		}
	}


		// Set up colour table for re-colouring sprites in response to any pigment genes

	int pig_total[3],pig_count[3];
	int Pigment[3];
	pig_total[0]=pig_total[1]=pig_total[2]=0;				// default pigmentation is mid-strength
	pig_count[0]=pig_count[1]=pig_count[2]=0;

	// For the the extention allowing more colour variation
	unsigned short  ColourRotation = 128;
	unsigned short  ColourSwap = 128;
	//int Extns=0;
	


	// gtb, 18/10/00:  fixed it so that if no pigment genes are found for this
	// lifestage it uses the one from the previous age:
	int genomeSavedAge = genome.GetAge();

	bool pigmentGenesFound = false;
	do
	{
		genome.Reset();
		while ((genome.GetGeneType(CREATUREGENE,G_PIGMENT,NUMCREATURESUBTYPES,SWITCH_AGE))!=FALSE) 
		{
			pigmentGenesFound = true;

			int pig = genome.GetCodon(0,2);					// pigment being defined (r,g,b)
			pig_total[pig] += genome.GetCodon(0,255);				// intensity (0=faint 128=normal 255=strong)
			pig_count[pig]++;
		}

		// done baby:
		if (genome.GetAge()==AGE_BABY)
			break;

		// otherwise try previous age for pigments:
		genome.SetAge(genome.GetAge()-1);
	} while (!pigmentGenesFound);
	genome.SetAge(genomeSavedAge);


	bool pigmentBleedGenesFound = false;
	do
	{
		genome.Reset();
		while ((genome.GetGeneType(CREATUREGENE, G_PIGMENTBLEED, NUMCREATURESUBTYPES, SWITCH_AGE)) != FALSE) 
		{
			pigmentBleedGenesFound = true;

			// rotation of colour space (0<-128->255  r<->g<->b)
			ColourRotation = (ColourRotation + genome.GetCodon(0, 255)) / 2;		

			// swap r & b (0 = none, 255 = total)
			ColourSwap = (ColourSwap + genome.GetCodon(0,255)) / 2;
		}

		// done baby:
		if (genome.GetAge()==AGE_BABY)
			break;

		// otherwise try previous age for pigments:
		genome.SetAge(genome.GetAge()-1);
	} while (!pigmentBleedGenesFound);
	genome.SetAge(genomeSavedAge);


	
	// Determine the average amount of pigmentation for each colour
	int piggy;
	for ( piggy=0;piggy<3;piggy++)
	{
		if (pig_count[piggy]>0)
		{
			Pigment[piggy]=(byte) (pig_total[piggy]/pig_count[piggy]);
		}
		else
		{
			Pigment[piggy]=128;
		}
	}

	

	tinty.BuildTintTable(Pigment[0],Pigment[1],Pigment[2],ColourRotation,ColourSwap);
	//tinty.BuildTintTable(255,255,128,255,255);

}


bool SkeletalCreature::CreateBodyPart(int sex, int age,TintManager& tinty)
{
	if(myBodyPartToBeCreated < NUMPARTS)
	{
		return BuildGalleries(sex,age,myBodyPartToBeCreated, tinty);
	}
	else
	{
		Limb* Root = NULL;

		Limb* Mid = NULL;
		Limb* Tip = NULL;
		
		bool gallery = false;

		// if you haven't already preloaded your body parts...
		if(!myCompositeBodyPartGallery->IsComplete())
		{
			char buf[5];
			sprintf(buf,"%d",age);
			std::string galleryName = GetMoniker() + buf;
			
			FilePath name(galleryName + ".c16",IMAGES_DIR );
#ifdef NETTY
	theApp.Netty->TextOut("3",galleryName.c_str());
#endif


		 gallery = myCompositeBodyPartGallery->CreateGallery(name.GetFullPath(),
								   GetUniqueID(),
								   NUMPARTS,
								   sex,
								   age,
								   myBodySprites,
								   myCompositeBodyPartGallery,
								   myNumSpritesInFile,
								   false);

								
		}


		if(gallery)
		{
			// find out whether the norn should be mirrored
			CAOSVar& sprite = theApp.GetWorld().GetGameVar("engine_mirror_creature_body_parts");
			_ASSERT(sprite.GetType() == CAOSVar::typeInteger);
			bool mirror = false;

			sprite.GetInteger()  == 1 ? mirror = true : mirror = false;


			char buf[5];
			sprintf(buf,"%d",age);
			std::string galleryName = GetMoniker() + buf;

			// get a unique plot plane to put my body in
			myNormalPlane = theAgentManager.UniqueCreaturePlane(mySelf);			            

			FilePath name( galleryName + ".c16",IMAGES_DIR );
#ifdef NETTY
	theApp.Netty->TextOut("4",galleryName.c_str());
#endif

			myBody = new Body(
				// BodyData struct from ATT file
				GetBodyData(PART_BODY, myGenera[PART_BODY], sex, age, myVariants[PART_BODY]),
				name,
				myBodySprites[PART_BODY],
				myNumSpritesInFile, //base sprite
				myNormalPlane,
				PART_BODY,
				myGenera[PART_BODY],
				sex,
				age,
				myVariants[PART_BODY],
				PART_BODY,
				mirror);    

			_ASSERT(myBody);


			LimbData* (*GetLimbDataPtr)( int,			// part number
							   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
							   int,				// IDEAL Sex to look for (MALE/FEMALE)
							   int,				// IDEAL age & variant to look for
							   int);

			GetLimbDataPtr = &GetLimbData;

			CreatureHead* head;  /// only use the head pointer once
				// ... then create & attach limb chains...
			head = new CreatureHead(
				// LimbData struct from ATT file
				NULL,               				
				 name,
				myBodySprites,
				myNumSpritesInFile,
				PART_HEAD,
				myGenera[PART_HEAD],
				sex,
				age,
				myVariants[PART_HEAD],
				GetLimbDataPtr,
				myBodySprites[PART_HEAD + 1],
				PART_HEAD,
				mirror);


			AttachLimbChain(BODY_LIMB_HEAD, head);

			///////////////////////////////////////////////////////////////////////////////
			// right leg
			///////////////////////////////////////////////////////////////////////////////

			Tip = new Limb(
				NULL,
				GetLimbData(PART_RFOOT, myGenera[PART_RFOOT], sex,	age, myVariants[PART_RFOOT]),
				 name,
				myBodySprites[PART_RFOOT],
				myNumSpritesInFile,
				PART_RFOOT,
				myGenera[PART_RFOOT],
				sex,
				age,
				myVariants[PART_RFOOT],	
				PART_RFOOT,
				mirror);



			Mid = new Limb(
				Tip,
				GetLimbData(PART_RSHIN, myGenera[PART_RSHIN], sex, age, myVariants[PART_RSHIN]),
				 name,
				myBodySprites[PART_RSHIN],
				myNumSpritesInFile,
				PART_RSHIN,
				myGenera[PART_RSHIN],
				sex,
				age,
				myVariants[PART_RSHIN],
				PART_RSHIN,
				mirror);




			Root = new Limb(
				Mid,
				GetLimbData(PART_RTHIGH, myGenera[PART_RTHIGH], sex, age, myVariants[PART_RTHIGH]),
				  name,
				myBodySprites[PART_RTHIGH],
				myNumSpritesInFile,
				PART_RTHIGH,
				myGenera[PART_RTHIGH],
				sex,
				age,
				myVariants[PART_RTHIGH],
				PART_RTHIGH,
				mirror);

			AttachLimbChain(BODY_LIMB_RIGHT_LEG, Root);


			//////////////////////////////////////////////////////////////////////////////
			// left leg
			//////////////////////////////////////////////////////////////////////////////


			Tip = new Limb(
				NULL,								
				GetLimbData(PART_LFOOT, myGenera[PART_LFOOT], sex, age, myVariants[PART_LFOOT]),
				  name,
				myBodySprites[PART_LFOOT],
				myNumSpritesInFile,
				PART_LFOOT,
				myGenera[PART_LFOOT],
				sex,
				age,
				myVariants[PART_LFOOT],
				PART_LFOOT,
				mirror);




			Mid = new Limb(
				Tip,
				GetLimbData(PART_LSHIN, myGenera[PART_LSHIN], sex, age, myVariants[PART_LSHIN]),
				  name,
				myBodySprites[PART_LSHIN],
				myNumSpritesInFile,
				PART_LSHIN,
				myGenera[PART_LSHIN],
				sex,
				age,
				myVariants[PART_LSHIN],
				PART_LSHIN,
				mirror);

			


			Root = new Limb(Mid,
				GetLimbData(PART_LTHIGH, myGenera[PART_LTHIGH], sex, age, myVariants[PART_LTHIGH]),
				  name,
				myBodySprites[PART_LTHIGH],
				myNumSpritesInFile,
				PART_LTHIGH,		
				myGenera[PART_LTHIGH],
				sex,
				age,
				myVariants[PART_LTHIGH],
				PART_LTHIGH,
				mirror);


			AttachLimbChain(BODY_LIMB_LEFT_LEG, Root);

			//////////////////////////////////////////////////////////////////////////////
			// left arm
			//////////////////////////////////////////////////////////////////////////////
			Tip = new Limb(
				NULL,
				GetLimbData(PART_LRADIUS, myGenera[PART_LRADIUS], sex, age, myVariants[PART_LRADIUS]),
				  name,
				myBodySprites[PART_LRADIUS],
				myNumSpritesInFile,
				PART_LRADIUS,
				myGenera[PART_LRADIUS],
				sex,
				age,
				myVariants[PART_LRADIUS],
				PART_LRADIUS,
				mirror);


			Root = new Limb(
				Tip,
				GetLimbData(PART_LHUMERUS, myGenera[PART_LHUMERUS], sex, age, myVariants[PART_LHUMERUS]),
				  name,
				myBodySprites[PART_LHUMERUS],
				myNumSpritesInFile,
				PART_LHUMERUS,
				myGenera[PART_LHUMERUS],
				sex,
				age,
				myVariants[PART_LHUMERUS],
				PART_LHUMERUS,
				mirror);


			AttachLimbChain(BODY_LIMB_LEFT_ARM,Root);

			//////////////////////////////////////////////////////////////////////////////
			// right arm
			//////////////////////////////////////////////////////////////////////////////

			Tip  = new Limb(
				NULL,
				GetLimbData(PART_RRADIUS, myGenera[PART_RRADIUS], sex,	age, myVariants[PART_RRADIUS]),
				  name,
				myBodySprites[PART_RRADIUS],
				myNumSpritesInFile,
				PART_RRADIUS,
				myGenera[PART_RRADIUS],
				sex,
				age,
				myVariants[PART_RRADIUS],
				PART_RRADIUS,
				mirror);


			Root = new Limb(
				Tip,
				GetLimbData(PART_RHUMERUS, myGenera[PART_RHUMERUS], sex, age, myVariants[PART_RHUMERUS]),
				  name,
				myBodySprites[PART_RHUMERUS],
				myNumSpritesInFile,
				PART_RHUMERUS,
				myGenera[PART_RHUMERUS],
				sex,
				age,
				myVariants[PART_RHUMERUS],
				PART_RHUMERUS,
				mirror);


			AttachLimbChain(BODY_LIMB_RIGHT_ARM, Root);

				Tip = new Limb(
					NULL,
					GetLimbData(PART_TAILTIP, myGenera[PART_TAILTIP], sex,	age, myVariants[PART_TAILTIP]),
					  name,
					myBodySprites[PART_TAILTIP],
					myNumSpritesInFile,
					PART_TAILTIP,
					myGenera[PART_TAILTIP],
					sex,
					age,
					myVariants[PART_TAILTIP],
					PART_TAILTIP,
					mirror);
						

			
				Root = new Limb(
					Tip,
					GetLimbData(PART_TAILROOT, myGenera[PART_TAILROOT], sex, age, myVariants[PART_TAILROOT]),
					  name,
					myBodySprites[PART_TAILROOT],
					myNumSpritesInFile,
					PART_TAILROOT,
					myGenera[PART_TAILROOT],
					sex,
					age,
					myVariants[PART_TAILROOT],
					PART_TAILROOT,
					mirror);


				AttachLimbChain(BODY_LIMB_TAIL, Root);


			GeneratePlotOrder();
			SetCurrentPoseString(myCurrentPoseString);						// reset planes, pose$s etc.
			UpdatePlotOrder();
			// if you were being tracked then tell the main camera that you have
			// grown up
			if(myBeingTrackedFlag)
				theMainView.YourTrackObjectHasGrownUp(mySelf);


			myBodyPartsAreFullyFormedFlag = true;
			// we have all the parts we need for now
			// reset the counter and actually create the body
			myBodyPartToBeCreated =0;
			myNumSpritesInFile =0;

			return true;
		} //end check gallery was created
	}

	// there are three split stages before we get a fully formed creature
	if(!myCompositeBodyPartGallery->IsComplete())
		return true;
	else
		return false;
}


bool SkeletalCreature::BuildGalleries(int sex,
									  int age,
									  uint32& currentPart,
									  TintManager& tintTable)
{

		uint32 fsp =0;
	// bypass the shared gallery for this
	// because we will delete it ourselves in a moment

	fsp = ValidFsp(currentPart,
							myGenera[currentPart],
							sex,
							age,
							myVariants[currentPart],
							"c16",
							IMAGES_DIR);
	if(fsp)
	{

		FilePath galleryName(BuildFsp(fsp, "c16"), IMAGES_DIR);

#ifdef NETTY
	theApp.Netty->TextOut("BuildGalleries",(BuildFsp(fsp, "c16")).c_str());
#endif


		std::string fullPath = galleryName.GetFullPath();
		return myCompositeBodyPartGallery->BuildGalleries(fullPath,
														myNumSpritesInFile,
													currentPart,
													tintTable.GetMyTintTable());
			
	}
	else
	{
		// if there was no body part use a default male
		// version
		fsp = ValidFsp(currentPart,
							0,
							sex,
							age,
							0,
							"c16",
							IMAGES_DIR);

		// if you don't have basic norn body parts you are in biiiiig
		// trouble
		if(!fsp)
			return false;


#ifdef NETTY
	theApp.Netty->TextOut("BuildGalleries",(BuildFsp(fsp, "c16")).c_str());
#endif


		// try again
		FilePath galleryName(BuildFsp(fsp, "c16"), IMAGES_DIR);

		std::string fullPath = galleryName.GetFullPath();
		return myCompositeBodyPartGallery->BuildGalleries(fullPath,
														myNumSpritesInFile,
													currentPart,
													tintTable.GetMyTintTable());
	

	}


}


bool SkeletalCreature::PreloadBodyPartsForNextAgeStage(Genome& genome,int sex)
{
	// use a tint manager for the duration of creating this norn
	int age = myAgeAlreadyLoaded;
	if(!myCompositeBodyPartGalleryForNextAgeStage || myAgeingDone )
		return true;

	if(myPreloadLastAge != age)
	{
		ReadAppearanceGenes(genome, *myTintin);
	}
	myPreloadLastAge = age;

	if(myBodyPartToBeCreatedForNextAgeStage < NUMPARTS)
	{
		return BuildGalleries(sex,age, myBodyPartToBeCreatedForNextAgeStage,*myTintin);
	}
	else
	{
		// if you haven't already preloaded your body parts...
		if(!myCompositeBodyPartGalleryForNextAgeStage->IsComplete())
		{
			char buf[5];
			sprintf(buf,"%d",age);
			std::string galleryName = GetMoniker() + buf;
			
			FilePath name(galleryName + ".c16",IMAGES_DIR );


			myCompositeBodyPartGallery->CreateGallery(name.GetFullPath(),
								   GetUniqueID(),
								   NUMPARTS,
								   sex,
								   age,
								   myBodySprites,
								   myCompositeBodyPartGalleryForNextAgeStage,
								   myNumSpritesInFile,
								   false);
		}
			
		if(myCompositeBodyPartGalleryForNextAgeStage->IsComplete())
		{
			// we have successfully beaten the ageing process and preloaded
			// all our data in time

			CreatureGallery* temp = myCompositeBodyPartGallery;
			// we delete myCompositeBodyPartGallery as temp below
		
			myCompositeBodyPartGallery = myCompositeBodyPartGalleryForNextAgeStage;
			myBodyPartToBeCreated = NUMPARTS;
			myJustAgedFlag =false;

			ReloadSkeleton(genome,age);
			// we have all the parts we need for now
			// reset the counter and actually create the body
			myBodyPartToBeCreatedForNextAgeStage =0;

			delete temp;
			myCompositeBodyPartGalleryForNextAgeStage = NULL;
			myAgeingDone = true;
		}
	}
	return true;
}

// if the preloading was complete then delete the current creature gallery
// and use the current one
bool SkeletalCreature::PrepareForAgeing(int age)
{
	if(!myCompositeBodyPartGalleryForNextAgeStage)
	{
		CAOSVar& sprite = theApp.GetWorld().GetGameVar("engine_creature_template_size_in_mb");
		_ASSERT(sprite.GetType() == CAOSVar::typeInteger);

		char buf[5];
		sprintf(buf,"%d",age);
		std::string galleryName = GetMoniker() + buf;

		std::string path = theDirectoryManager.GetDirectory(CREATURE_DATABASE_DIR);

		// for now until I redo the Creature Gallery lets only resize if the
		// creature lib is empty
		myCompositeBodyPartGalleryForNextAgeStage = new CreatureGallery(path,
																		sprite.GetInteger(),
																		galleryName);
		_ASSERT(myCompositeBodyPartGalleryForNextAgeStage);
		myJustAgedFlag = true;
		myAgeingDone = false;
		myAgeAlreadyLoaded = age;
		return true;
	}

	return false;

	
}


bool SkeletalCreature::ContinueLoadingDataForNextAgeStage()
{
	return (myCompositeBodyPartGalleryForNextAgeStage  && myJustAgedFlag );
}


enum {
	POSE_TOUCH_NEAR_LOW = POSE_TOUCH,	// individual touch poses
	POSE_TOUCH_NEAR_LOWISH,
	POSE_TOUCH_NEAR_HIGHISH,
	POSE_TOUCH_NEAR_HIGH,
	POSE_TOUCH_MED_LOW,
	POSE_TOUCH_MED_LOWISH,
	POSE_TOUCH_MED_HIGHISH,
	POSE_TOUCH_MED_HIGH,
	POSE_TOUCH_FAR_LOW,
	POSE_TOUCH_FAR_LOWISH,
	POSE_TOUCH_FAR_HIGHISH,
	POSE_TOUCH_FAR_HIGH,

	// Pre-defined poses that might need to be referred to in code...
    POSE_STOP,              // stand & look at it <5>

    POSE_WALK1,
    POSE_WALK2,
    POSE_WALK3,
    POSE_WALK4,
    // All other poses are defined in Gene Editor
};

void SkeletalCreature::CommitPositions()
{
	myPreviousUpFootPosition = myUpFootPosition;
	myPreviousMinY = myMinY;
}



float SkeletalCreature::GetSensoryDistanceFromAgent(AgentHandle a)
{
	// X only:
	return fabsf(
		GetCentre().x -
		a.GetAgentReference().WhereShouldCreaturesPressMe().x
	);
}

void SkeletalCreature::GetTintValues(int16& redTint, int16& greenTint, int16& blueTint, int16& rotation, int16& swap)
{
	if (!myTintin)
		return;
	myTintin->GetTintValues(redTint, greenTint, blueTint, rotation, swap);
}

// A creature can get into a bad place (the Physics system
// does not check their heads, so maps can be made
// that can cause this - see Rob Dickson for details)
void SkeletalCreature::EmergencySafePositionRoom()
{
	// The creature may be _meant_ to be in an invalid
	// position, for example just after loading (if, say,
	// it had been serialised out from a meta room which
	// no longer exists)
	if (myInvalidPosition)
		return;

	// See if the creature is in an invalid position
	if (!theApp.GetWorld().GetMap().
		IsCreatureLocationValidInRoomSystem
		(myExtremes[BODY_LIMB_LEFT_LEG],
		myExtremes[BODY_LIMB_RIGHT_LEG],
		myMinY,
		myPermiability))
	{
		// If they are, then escape to safety!
		theFlightRecorder.Log(FLIGHT_RUNTIME, "Emergency escape for creature from down foot %f, %f.  The map is too tight for the skeletal creatures to properly fit in.", myDownFootPosition.x, myDownFootPosition.y);
		if (!MoveToSafePlaceGivenCurrentLocation())
			throw BasicException("Emergency safe place for Creature was not found");
	}
}

