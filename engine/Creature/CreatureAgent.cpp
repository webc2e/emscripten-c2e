#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreatureAgent.h"
#include "../Agents/CompoundPart.h"
#include "SensoryFaculty.h"

#include "../App.h"
#include "../World.h"
#include "../Map/Map.h"
#include "MotorFaculty.h"
#include "../AgentManager.h"



CREATURES_IMPLEMENT_SERIAL( CreatureAgent )




bool CreatureAgent::CanSee(const AgentHandle& other)
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

	// if you're not in the same root vehicle you can't see the agent:
	if (GetRootVehicle()!=agentref.GetRootVehicle())
		return false;

	return true;
}




bool CreatureAgent::Approach(bool doApproachNotRetreat)
{
	int scriptToRun = -1;
	CAOSVar itX,itY;

	if (myIt.IsValid())
	{
		if (CannotBeMuchCloser())
			// reached it so stop approaching:
			return false;

		scriptToRun = SCRIPT_CRAG_NAV_VALID_IT;
	}
	else
	{
		Map& map = theApp.GetWorld().GetMap();
		int attnId = Motor()->GetCurrentAttentionId();
		int caIndex = theAgentManager.GetSmellIdFromCategoryId(attnId);

		int roomId, roomIdHighest;
		if (GetRoomID(roomId))
		{
			int creatureTargCategory = SensoryFaculty::GetCategoryIdOfAgent(mySelf);
			int creatureRealRoomId = -1;
			// set it so you can't smell yourself if you're navigating to other creatures of your category:
			if (attnId==creatureTargCategory)
			{	
				if (GetRoomID(creatureRealRoomId))
					map.AlterCAEmission(creatureRealRoomId, caIndex, -GetCAIncrease());
				else
					creatureRealRoomId = -1;
			}

/*			CaRoomCategoryFound roomCat = doApproachNotRetreat ?
				map.GetRoomIDWithHighestCA(roomId, caIndex, ALL_DOORS_AND_LINKS, roomIdHighest) :
				map.GetRoomIDWithLowestCA(roomId, caIndex, ALL_DOORS_AND_LINKS, roomIdHighest);
*/

			CaRoomCategoryFound roomCat =
				// this chooses the function:
				(map.*(doApproachNotRetreat ? &Map::GetRoomIDWithHighestCA : &Map::GetRoomIDWithLowestCA))
				// these are the arguments, regardless:
				(roomId, caIndex, ALL_DOORS_AND_LINKS, roomIdHighest);

			// set it so you can smell yourself again:
			if (attnId==creatureTargCategory && creatureRealRoomId>=0)
				map.AlterCAEmission(creatureRealRoomId, caIndex, +GetCAIncrease());


			if (roomCat)
			{
				map.GetRoomCentre(roomIdHighest, myItPosition);
			}
			scriptToRun = SCRIPT_CRAG_NAV + roomCat;
		}
		else
		{
			scriptToRun = SCRIPT_CRAG_NAV_OUTSIDE_ROOM_SYSTEM;
		}
	}

	itX.SetFloat(myItPosition.x);
	itY.SetFloat(myItPosition.y);

	CallScript(scriptToRun, itX, itY);

	// keep trying to get to it:
	return true;
}


void CreatureAgent::GetBodyPartFileName(int index, std::string& path)
{
	path = myParts.front()->GetEntity()->GetGalleryName();
	
	// get the name without the extension:
	int x = path.find_last_of(".");
	if (x==-1)
		x = path.size()-1;

	// get the name without the path:
	int y = path.find_last_of("/\\:");
	y = y==-1 ? 0 : y+1;

	path = path.substr(y, x - y);
}



bool CreatureAgent::SetAnim(const uint8* anim, int length, int partid )
{
	if (!ValidatePart(partid))
		return true;

	if (!myParts[partid]->GetEntity()->ValidateAnim( anim, length ))
		return true;

	return (myParts[partid]->SetAnim( anim, length ));
}


bool CreatureAgent::ShowPose( int pose, int partid )
{
	if (!ValidatePart(partid))
		return true;

	if (!myParts[partid]->GetEntity()->ValidatePose( pose ))
		return true;

	myParts[partid]->GetEntity()->SetPose( pose );


	if(partid == 0)
	{
		SetWidth(myParts[partid]->GetEntity()->GetWidth());
		SetHeight(myParts[partid]->GetEntity()->GetHeight());
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//  Initialisation:


// Serialisation constructor
CreatureAgent::CreatureAgent() : base()
{
}

// Destructor.
CreatureAgent::~CreatureAgent()
{
}

// 'Birth' constructor. 
CreatureAgent::CreatureAgent(int family, uint32 id, AgentHandle gene, int gene_index, int sex, int variant,
	FilePath const& gallery, int numimages, int baseimage, int plane)
	: base(family, id, gene, gene_index, sex, variant,
	gallery, numimages, baseimage, plane)
{
	try
	{
		// Do general Creature init:
		Init();
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
		return;
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0014: Unknown exception caught in CreatureAgent constructor";
		return;
	}
}



void CreatureAgent::Trash()
{
	base::Trash();
}


void CreatureAgent::Update()
{
	base::Update();
}



bool CreatureAgent::Write(CreaturesArchive &archive) const
{
	base::Write(archive);
	return true;
}

bool CreatureAgent::Read(CreaturesArchive &archive) 
{
	if(!base::Read(archive))
		return false;
	// Debugging:
	//CAOSVar age;
	//age.SetString("age");
	//OutputFormattedDebugString("Monkey age: %d\n", myNameVars[age].GetInteger());
	return true;
}


float CreatureAgent::GetSensoryDistanceFromAgent(AgentHandle a)
{
	// gtb: change this to euclidean distance?

	// manhattan dist in X and Y:
	return fabsf(
		GetCentre().x -
		a.GetAgentReference().WhereShouldCreaturesPressMe().x
	) + 
	fabsf(
		GetCentre().y -
		a.GetAgentReference().WhereShouldCreaturesPressMe().y
	);
}

