// NOTE: This file will be probably merged into Map.cpp after CA is up and
// running

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Map.h"
#include "../Maths.h"
#include "../Agents/Agent.h"
#include "RoomCA.h"
#include <fstream>
#include <algorithm>
#include "../AgentManager.h"

const float CA_MULTIPLIER = 10.0f;
const int MAX_CA_DIST = 30;


CREATURES_IMPLEMENT_SERIAL( Link );







void Map::RecalculateAllNavigationalCAs()
{
	// loop through all rooms blanking their nav CAs:
	for (int k=0; k<=myMaxRoomID; ++k) 
	{
		if (myRoomCollection[k]==NULL)
			continue;

		Room& r = *(myRoomCollection[k]);

		// blank current values for all navigable CAs in this room:
		for (int i=0; i<CA_PROPERTY_COUNT; ++i) 
		{
			if( IsCANavigable( i ) )
				r.caValues[i] = 0.0f;
		}
	}

	for (AgentListIterator a=theAgentManager.GetAgentIteratorStart(); !theAgentManager.IsEnd(a); a++)
	{
		AgentHandle& agent = (*a);

		if (agent.IsValid())
		{
			agent.GetAgentReference().ForceCaAddition();
		}
	}
}







// ---------------------------------------------------------------------
// Method:		SetRoomType
// Arguments:	roomID - ID of room
//				type - new type for room
// Returns:		true for success, false otherwise
// Description:	Sets the room type of the specified room. Effects CA
//				rates.
// ---------------------------------------------------------------------
bool Map::SetRoomType( int roomID, int type )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	room->type = type;
	return true;
}

// ---------------------------------------------------------------------
// Method:		GetRoomType
// Arguments:	roomID - ID of room
//				type - type of room
// Returns:		true for success, false otherwise
// Description:	Gets the room type of the specified room.
// ---------------------------------------------------------------------
bool Map::GetRoomType( int roomID, int &type )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	type = room->type;
	return true;
}

// ---------------------------------------------------------------------
// Method:		SetRoomProperty
// Arguments:	roomID - ID of room
//				caIndex - index of CA property to change
//				value - new value for the CA property
// Returns:		true for success, flase otherwise
// ---------------------------------------------------------------------
bool Map::SetRoomProperty( int roomID, int caIndex, float value )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	if( caIndex < 0 || caIndex >= CA_PROPERTY_COUNT )
		return false;

	room->caValues[ caIndex ] = value;
	return true;
}

// ---------------------------------------------------------------------
// Method:		GetRoomProperty
// Arguments:	roomID - ID of room
//				caIndex - index of CA property
//				value - value for the CA property
// Returns:		true for success, flase otherwise
// ---------------------------------------------------------------------
bool Map::GetRoomProperty( int roomID, int caIndex, float &value )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	if( caIndex < 0 || caIndex >= CA_PROPERTY_COUNT )
		return false;

	value = room->caValues[ caIndex ];
	//If it is a navigable CA map -infinity -> infinity to 0->1
	if( IsCANavigable( caIndex ) )
	{
		if( value < 0 ) value = 0;
		else value = 1 - 1 / ( value + 1 );
	}
	return true;
}

bool Map::GetRoomPropertyMinusMyContribution( AgentHandle const &handle, float &value )
{
	AgentHandle two(handle);
	if( !two.IsValid() )
		return false;
	Agent &agent = handle.GetAgentReference();
	int roomID, caIndex;
	Room *room;
	if( !agent.GetRoomID( roomID ) )
		return false;
	if (!GetValidRoomPointer(roomID, room))
		return false;
	caIndex = agent.GetCAIndex();
	if( caIndex < 0 || caIndex >= CA_PROPERTY_COUNT )
		return false;
	if( !IsCANavigable( caIndex ) )
		return false;

	value = room->caValues[ caIndex ];
	value -= CA_MULTIPLIER * agent.GetCAIncrease();

	if( value < 0 )
		value = 0;
	else
		value = 1 - 1 / ( value + 1 );

	return true;
}

// ---------------------------------------------------------------------
// Method:		GetRoomPropertyChange
// Arguments:	roomID - ID of room
//				caIndex - index of CA property
//				value - difference between current value and last value
// Returns:		true for success, false otherwise
// Description:	This function returns the change in CA property for a
//				room.
// ---------------------------------------------------------------------
bool Map::GetRoomPropertyChange( int roomID, int caIndex, float &value )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	if( caIndex < 0 || caIndex >= CA_PROPERTY_COUNT )
		return false;

	value = room->caValues[ caIndex ] - room->caOldValues[ caIndex ];
	return true;
}


bool Map::SetCARates( int roomType, int caIndex, CARates const &rates )
{
	if( roomType >= 0 && roomType < ROOM_TYPE_COUNT &&
		caIndex >= 0 && caIndex < CA_PROPERTY_COUNT )
	{
		myCARates[roomType][caIndex] = rates;
		return true;
	}
	return false;
}

bool Map::GetCARates( int roomType, int caIndex, CARates &rates )
{
	if( roomType >= 0 && roomType < ROOM_TYPE_COUNT &&
		caIndex >= 0 && caIndex < CA_PROPERTY_COUNT )
	{
		rates = myCARates[roomType][caIndex];
		return true;
	}
	return false;
}

CARates const &Map::GetCARates( int roomType, int caIndex )
{
	if( roomType >= 0 && roomType < ROOM_TYPE_COUNT &&
		caIndex >= 0 && caIndex < CA_PROPERTY_COUNT )
	{
		return myCARates[roomType][caIndex];
	}
	return myCARates[0][0];
}

bool Map::IncreaseCAProperty( int roomID, int caIndex, float value )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	if( caIndex < 0 || caIndex >= CA_PROPERTY_COUNT )
		return false;

	room->caValues[ caIndex ] = BoundIntoZeroOne( room->caValues[ caIndex ] + value );

	return true;
}

bool Map::IncreaseCAInput( int roomID, float value )
{
	Room *room;

	// Check that the room exists
	if (!GetValidRoomPointer(roomID, room))
		return false;

	room->caInput += value;

	return true;
}




// ---------------------------------------------------------------------------
// Function:	GetLinkBetweenRooms
// Description:	Gets a pointer to the link that lies between two rooms
// Arguments:	id1 - ID of first room (in)
//				id2 - ID of second room (in)
//				link - link pointer between the rooms (out)
// Returns:		true if good input, false otherwise	
// ---------------------------------------------------------------------------
bool Map::GetLinkBetweenRooms
	(const int roomID1, const int roomID2, Link * & link)
{

	if (roomID1==roomID2)
		return false;

	// Check that the room exists
	Room* room1;
	if (!GetValidRoomPointer(roomID1, room1))
		return false;
	Room* room2;
	if (!GetValidRoomPointer(roomID2, room2))
		return false;

	int count = room1->linkCollection.size();
	LinkIterator linkIterator = room1->linkCollection.begin();
	Link *l;
	while (count-- )
	{
		l = (*linkIterator++);
		if ((l->parent1==roomID1 && l->parent2==roomID2) ||
		  (l->parent2==roomID1 && l->parent1==roomID2)) {
			link = l;
			return true;
		}
	}

	return false;
}



// ---------------------------------------------------------------------------
// Function:	GetLinkPermiability
// Description:	Gets the permiability of a link between two rooms
// Arguments:	roomID1 - ID of first room (in)
//				roomID2 - ID of second room (in)
//				permiability - permiability of the link (out)
// Returns:		true if good input, false otherwise
// --------------------------------------------------------------------------
bool Map::GetLinkPermiability(const int roomID1, const int roomID2,
	int & permiability)
{
	Link *link;
	bool ok;
	
	// Check that the room exists
	Room* room1;
	if (!GetValidRoomPointer(roomID1, room1))
		return false;
	Room* room2;
	if (!GetValidRoomPointer(roomID2, room2))
		return false;

	ok = GetLinkBetweenRooms(roomID1, roomID2, link);

	permiability = ok ? (int)(link->permiability) : 0;
	return true;
}



// ---------------------------------------------------------------------------
// Function:	SetLinkPermiability
// Description:	Sets the permiability of a link between two rooms
// Arguments:	roomID1 - ID of first room (in)
//				roomID2 - ID of second room (in)
//				permiability - new permiability of the door (in)
// Returns:		true if good input, false otherwise
// --------------------------------------------------------------------------
bool Map::SetLinkPermiability(const int roomID1, const int roomID2, 
	const int permiability)
{
	if ((permiability < IMPERMIABLE) || (permiability > PERMIABLE))
		return false;

	if (roomID1==roomID2)
		return false;

	// Check that the room exists
	Room* room1;
	if (!GetValidRoomPointer(roomID1, room1))
		return false;
	Room* room2;
	if (!GetValidRoomPointer(roomID2, room2))
		return false;
	float length = ((room1->endFloor.y - room1->endCeiling.y) + (room1->startFloor.y - room1->startCeiling.y) +
		(room2->endFloor.y - room2->endCeiling.y) + (room2->startFloor.y - room2->startCeiling.y)) / 4.0f;


	Link *link;
	if (GetLinkBetweenRooms(roomID1, roomID2, link)) {
		link->permiability = permiability;

		if (permiability==0) {
			room1->perimeterLength -= length;
			room2->perimeterLength -= length;
			myLinkCollection.remove(link);
			room1->linkCollection.remove(link);
			room2->linkCollection.remove(link);
			return true;
		}
	}
	else
	{
		if (permiability==0)
			return true;

		Link *link = new Link();
		link->parent1 = roomID1;
		link->parent2 = roomID2;
		link->permiability = permiability;
		link->length = length;
		myLinkCollection.push_back(link);
		room1->linkCollection.push_back(link);
		room2->linkCollection.push_back(link);
		room1->perimeterLength += length;
		room2->perimeterLength += length;
	}
	CalculateRoomPermiabilityAndDoorage(room1);
	CalculateRoomPermiabilityAndDoorage(room2);

	// gtb
	// when links change we need to blank and
	// recalculate all the nav CAs just in case:
	// but removed because too slow
//	RecalculateAllNavigationalCAs();

	return true;
}

/*
void LogCAValue( int id, float value )
{
	static std::ofstream stream( "ca.log" );
	if( id == -1 ) stream << '\n';
	else
		stream << '\t' << id << '\t' << value;
}
*/

void Map::UpdateCurrentCAProperty()
{
	if( !IsCANavigable( myNextCAProperty ) )
	{
		int idCount = myRoomCount;
		int linkCount = myLinkCollection.size();
		IntegerIterator integerIterator = myRoomIDCollection.begin();
		LinkIterator linkIterator = myLinkCollection.begin();
		register Door* door;
		register Room* room;

		//
		// Inputs should have been set for this myNextCAProperty in the Agent update loop
		//

		while (idCount--)
		{
			room = myRoomCollection[*(integerIterator++)];
			room->caOlderValues[myNextCAProperty] = room->caOldValues[myNextCAProperty];
			room->caOldValues[myNextCAProperty] = room->caValues[myNextCAProperty];
			// Uses current CA values to calculate loss-or-gained values which go into the temp value.
			// Then the current CA values are set to zero (ready to be accumulated later on):
			UpdateRoomCA( myCARates[room->type][myNextCAProperty], room->caInput,
				room->caValues[myNextCAProperty], room->caTempValue );
			room->caInput = 0;
		}
		// Restore for later
		integerIterator = myRoomIDCollection.begin();
		idCount = myRoomCount;

		while (linkCount--)
			UpdateDoorCABetweenRooms(**(linkIterator++));

		int doorCount;
		DoorIterator doorIterator;
		doorCount = myDoorCollection.size();
		doorIterator = myDoorCollection.begin();

		while (doorCount--)
		{
			door = *(doorIterator++);
			if (door->parentCount == 2)
				UpdateDoorCABetweenRooms(*door);
		}

		while (idCount--)
		{
			room = myRoomCollection[*(integerIterator++)];
			room->caValues[myNextCAProperty] +=
					room->caTempValue * (1.0f - room->caTotalDoorage );
			if( myNextCAProperty == 7 )
			{
				float projectedValue;
				GetRoomProperty( room->roomID, myNextCAProperty, projectedValue );
			}
		}

	}

	myNextCAProperty = (myNextCAProperty + 1) % CA_PROPERTY_COUNT;
}

int Map::GetCAIndex(void)
{
	return myNextCAProperty;
}

bool Map::WhichDirectionToFollowCA(int currentRoomID, int caIndex, int &mapDirection, bool approachNotRetreat)
{
	Room* currentRoom;
	if (!GetValidRoomPointer(currentRoomID, currentRoom))
		return false;


	// See which is the highest CA neighbour:
	IntegerCollection neighbours;

	// Add LINK collection:
	int count = currentRoom->linkCollection.size();
	LinkIterator linkIterator = currentRoom->linkCollection.begin();
	while( count-- )
	{
		Link& l = *(*linkIterator++);

		if(l.parent1==currentRoom->leftFloorRoomID 
			|| l.parent2==currentRoom->rightFloorRoomID)
		{
			int perm = 0;
			GetDoorPermiability(l.parent1 , l.parent2, perm);
			if(perm == 0)
			{
				// only add if has 0 permiability else will be added 
				// as left or right neighbour
				neighbours.push_back(
					l.parent2==currentRoomID ? l.parent1 :l.parent2);
			}
		}
		else
		{
			// meta link or up/down link
			neighbours.push_back(
				l.parent2==currentRoomID ? l.parent1 :l.parent2);
		}
	}


/*	// gtb! TODO:  Change this to use the room->linkCollection instead of the global map one:
	LinkIterator i;
	for (i = myLinkCollection.begin(); i != myLinkCollection.end(); ++i)
	{
		Link& l = **i;

		if (l.parent1==currentRoomID || l.parent2==currentRoomID) 		
		{	
			if(l.parent1==currentRoom->leftFloorRoomID 
				|| l.parent2==currentRoom->rightFloorRoomID)
			{
				int perm = 0;
				GetDoorPermiability(l.parent1 , l.parent2, perm);
				if(perm == 0)
				{
					// only add if has 0 permiability else will be added 
					// as left or right neighbour
					neighbours.push_back(
						l.parent2==currentRoomID ? l.parent1 :l.parent2);
				}
			}
			else
			{
				// meta link or up/down link
				neighbours.push_back(
					l.parent2==currentRoomID ? l.parent1 :l.parent2);
			}
		}
	}*/

	// Add left and right neighbours if permiable 
	// (else not traverable or door shut and will already be included in links list)
	int leftPerm = 0;
	int rightPerm = 0;
	GetDoorPermiability(currentRoom->roomID , currentRoom->leftFloorRoomID, leftPerm);
	GetDoorPermiability(currentRoom->roomID , currentRoom->rightFloorRoomID, rightPerm);

	if (currentRoom->leftFloorRoomID != -1 && leftPerm != 0) 
		neighbours.push_back(currentRoom->leftFloorRoomID);
	if (currentRoom->rightFloorRoomID != -1 && rightPerm != 0)
		neighbours.push_back(currentRoom->rightFloorRoomID);



	// Work out highest CA room:
	int roomIDHighest = currentRoomID;
	float bestValue;
	GetRoomProperty( currentRoomID, caIndex, bestValue );
	IntegerIterator itor;

	for( itor = neighbours.begin(); itor != neighbours.end(); ++itor )
	{
		float value;
		GetRoomProperty( *itor, caIndex, value );
		if( approachNotRetreat ? value>bestValue : value<bestValue )
		{
			bestValue = value;
			roomIDHighest = *itor;
		}
	}


	// Work out direction from highest room ID:
	if (roomIDHighest==currentRoomID) 
	{
		mapDirection = GO_NOWHERE;
		return true;
	}
	if (roomIDHighest==currentRoom->leftFloorRoomID) 
	{
		if(leftPerm != 0)
			mapDirection = GO_WALK_LEFT;
		else	// must have link with shut door 
			mapDirection = GO_IN;
		return true;
	}
	if (roomIDHighest==currentRoom->rightFloorRoomID) 
	{
		if(rightPerm != 0)
			mapDirection = GO_WALK_RIGHT;
		else	// must have link with shut door 
			mapDirection = GO_IN;
		return true;
	}

	Room* highestRoom;
	if (!GetValidRoomPointer(roomIDHighest, highestRoom))
		return false;

	// changing meta rooms
	if (currentRoom->metaRoomID != highestRoom->metaRoomID) {
		mapDirection = GO_OUT;
		return true;
	}


	// CAdv specific (i think)
	float maxNavigationDistanceSquared = 500.0f * 500.0f;
	if (currentRoom->centre.SquareDistanceTo(highestRoom->centre) > maxNavigationDistanceSquared) {
		mapDirection = GO_IN;
		return true;
	}





	// handle vehicle movement lifts cable cars etc
	Vector2D deltaToHighestRoom = highestRoom->centre - currentRoom->centre;
	// gtb (01/11/00), bias coordinates so that up/down is heavily favoured over left/right
	// (i.e. coordinates look more up/down than they actually are)
	deltaToHighestRoom.y *= 2.0f;

	float sideOfForwardSlashLine = deltaToHighestRoom.x + deltaToHighestRoom.y;	// up-left is neg, right-down is pos
	float sideOfBackSlashLine = deltaToHighestRoom.x - deltaToHighestRoom.y;	// down-left is neg, up-right is pos

	mapDirection = 
		sideOfForwardSlashLine>0.0f ?
			sideOfBackSlashLine>0.0f ? GO_RIGHT : GO_DOWN :
			sideOfBackSlashLine>0.0f ? GO_UP : GO_LEFT;

//	OutputFormattedDebugString("CA direction: %d\n", mapDirection);

	return true;
}

////////////////////////////////////




template<class T>
CaRoomCategoryFound GetRoomIDWithCA(T& t, Map& map, int roomID, int caIndex, CaRoomSearchCriteria includeUpDownOrLinks, int &roomIdBest)
{
	Room* room;

	if (!map.GetValidRoomPointer(roomID, room))
		return NONE_FOUND;

	// Get the immediate neighbours:
	IntegerCollection neighbours;
	if( includeUpDownOrLinks )
	{
		map.GetNeighbours( roomID, DIRECTION_ALL, neighbours );
	}
	else
	{
		if (room->leftFloorRoomID != -1) 
			neighbours.push_back(room->leftFloorRoomID);
		if (room->rightFloorRoomID != -1)
			neighbours.push_back(room->rightFloorRoomID);
	}
	int noOfImmediateNeighbours = neighbours.size();

	// Add links if asked to:
	if( includeUpDownOrLinks==ALL_DOORS_AND_LINKS )
	{
		// Add links:
		int count = room->linkCollection.size();
		LinkIterator linkIterator = room->linkCollection.begin();
		Link *link;
		while( count-- )
		{
			link = (*linkIterator++);

			int otherRoomID;
			if( link->parent1 == roomID )
				otherRoomID = link->parent2;
			else
				otherRoomID = link->parent1;

			// add room ID (who cares if we add it twice, we'll still find it as the highest one):
			neighbours.push_back(otherRoomID);
		}
	}

	// Init search with current room:
	roomIdBest = roomID;
	float bestSoFar;
	int bestIdSoFar = -1;
	map.GetRoomProperty( roomID, caIndex, bestSoFar );

	// Find best room:
	IntegerIterator itor;
	int i;
	for (itor=neighbours.begin(), i=0; itor!=neighbours.end(); ++itor, ++i)
	{
		float value;
		map.GetRoomProperty( *itor, caIndex, value );
		if( T::BetterThan(value, bestSoFar) )
		{
			bestIdSoFar = i;
			bestSoFar = value;
			roomIdBest = *itor;
		}
	}

	// Return what kind of room we found:
	if (roomIdBest==roomID)
	{
		ASSERT(bestIdSoFar==-1);
		return CURRENT_ROOM_FOUND;
	}

	if (bestIdSoFar<noOfImmediateNeighbours)
	{
		return NEIGHBOUR_FOUND;
	}
	else
	{
		return LINK_FOUND;
	}
}

namespace MapCA
{
	class Lowest{	public:	static bool BetterThan(float a, float b){return a<b;}	} theLowestOne;
	class Highest{	public:	static bool BetterThan(float a, float b){return a>b;}	} theHighestOne;
}

CaRoomCategoryFound Map::GetRoomIDWithHighestCA( int roomID, int caIndex, CaRoomSearchCriteria includeUpDownOrLinks, int &roomIDHighest )
{
	return GetRoomIDWithCA(MapCA::theHighestOne, *this, roomID, caIndex, includeUpDownOrLinks, roomIDHighest );
}

CaRoomCategoryFound Map::GetRoomIDWithLowestCA( int roomID, int caIndex, CaRoomSearchCriteria includeUpDownOrLinks, int &roomIDLowest )
{
	return GetRoomIDWithCA(MapCA::theLowestOne, *this, roomID, caIndex, includeUpDownOrLinks, roomIDLowest );
}





// Added by gtb for Sea Monkey's navigation CAs which need to
// go in all directions, not just along floors:
std::set<int> const &Map::GetSuperNavigableCAIndices()
{
	static bool cached = false;
	static std::set<int> indices;
	if( !cached )
	{
		if (theCatalogue.TagPresent("Super-Navigable CA Indices"))
		{
			int n = theCatalogue.GetArrayCountForTag( "Super-Navigable CA Indices" );
			for( int i = 0; i < n; ++i )
				indices.insert( atoi( theCatalogue.Get( "Super-Navigable CA Indices", i ) ) );
			cached = true;
		}
	}
	return indices;
}

bool Map::IsCASuperNavigable( int index )
{
	std::set<int> const &set = GetSuperNavigableCAIndices();
	return set.find( index ) != set.end();
}






std::set<int> const &Map::GetNavigableCAIndices()
{
	static bool cached = false;
	static std::set<int> indices;
	if( !cached )
	{
		int n = theCatalogue.GetArrayCountForTag( "Navigable CA Indices" );
		int i;
		for( i = 0; i < n; ++i )
			indices.insert( atoi( theCatalogue.Get( "Navigable CA Indices", i ) ) );

		// n.b. super-navigable CAs are navigable too:
		if (theCatalogue.TagPresent("Super-Navigable CA Indices"))
		{
			n = theCatalogue.GetArrayCountForTag( "Super-Navigable CA Indices" );
			for( i = 0; i < n; ++i )
				indices.insert( atoi( theCatalogue.Get( "Super-Navigable CA Indices", i ) ) );
		}

		cached = true;
	}
	return indices;
}

bool Map::IsCANavigable( int index )
{
	std::set<int> const &set = GetNavigableCAIndices();
	return set.find( index ) != set.end();
}




int caTock = 0;

// n.b. this is only called for navigable CAs:
void Map::AlterCAEmission( int roomID, int caIndex, float difference )
{
	caTock++;

	Room *room;
	if ( !GetValidRoomPointer( roomID, room ) )
		return;
	CARates const &rates = GetCARates( room->type, caIndex );
	difference *= rates.GetGain();
	if (IsCASuperNavigable( caIndex ))
	{
		AlterSuperCAEmission( room, caIndex, difference * CA_MULTIPLIER, MAX_CA_DIST);
	}else
	{
		AlterCAEmission( room, caIndex, difference * CA_MULTIPLIER, MAX_CA_DIST, -1, NULL );
	}
}





#include <deque>

void Map::AlterSuperCAEmission(Room *firstRoom, int caIndex, float difference, int distance)
{
	std::deque<Room*> roomQueue, otherRoomQueue;

	roomQueue.push_back(firstRoom);
	// mark as visited:
	firstRoom->visitedOnTock = caTock;

	firstRoom->amountToAdd = difference;


	// PROCESS ROOMS ON QUEUE:

	while (distance--)
	{
		while (!roomQueue.empty())
		{
			Room* r = roomQueue.back();
			roomQueue.pop_back();

			// add CA difference:
			r->caValues[ caIndex ] += r->amountToAdd;

			// If we're not at max dist don't add any more rooms:
			if (!distance)
				continue;

			// Current room CA rates:
			CARates const &rates = GetCARates( r->type, caIndex );

			int count;
			// Recurse through links:
			count = r->linkCollection.size();
			LinkIterator linkIterator = r->linkCollection.begin();
			while( count-- )
			{
				Link* link = (*linkIterator++);

				int otherRoomID;
				if( link->parent1 == r->roomID )
					otherRoomID = link->parent2;
				else
					otherRoomID = link->parent1;
				Room *otherRoom;
				if (GetValidRoomPointer(otherRoomID, otherRoom) && otherRoom->visitedOnTock!=caTock)
				{
					CARates const &otherRates = GetCARates( otherRoom->type, caIndex );
					float reduce = rates.GetDiffusion() * otherRates.GetDiffusion();

					otherRoom->amountToAdd = r->amountToAdd*reduce*link->permiability*0.01f,
					// mark as visited:
					otherRoom->visitedOnTock = caTock;
					otherRoomQueue.push_back(otherRoom);
				}
			}

			// Recurse through doors:
			DoorIterator doorIterator=r->doorCollection.begin();
			count = r->doorCollection.size();
			while( count-- )
			{
				Door* door = (*doorIterator++);

				int otherRoomID;
				if( door->parent1 == r->roomID )
					otherRoomID = door->parent2;
				else
					otherRoomID = door->parent1;
				Room *otherRoom;
				if (GetValidRoomPointer(otherRoomID, otherRoom) && otherRoom->visitedOnTock!=caTock)
				{
					CARates const &otherRates = GetCARates( otherRoom->type, caIndex );
					float reduce = rates.GetDiffusion() * otherRates.GetDiffusion();

					otherRoom->amountToAdd = r->amountToAdd*reduce*door->permiability*0.01f,
					// mark as visited:
					otherRoom->visitedOnTock = caTock;
					otherRoomQueue.push_back(otherRoom);
				}
			}
		}

		otherRoomQueue.swap(roomQueue);
	}
}

// n.b. this is only called for navigable CAs:
void Map::AlterCAEmission( Room *room, int caIndex, float difference,
						  int distance, int roomIDFrom, Link *linkFrom )
{
	Room *otherRoom;


	// Terminate the recursion
	room->caValues[ caIndex ] += difference;

	if( !distance-- )
		return;


	// Recurse through links:
	int count = room->linkCollection.size();
	LinkIterator linkIterator = room->linkCollection.begin();
	Link *link;
	CARates const &rates = GetCARates( room->type, caIndex );
	while( count-- )
	{
		link = (*linkIterator++);
		// Avoid going back along link we came from
		if( link == linkFrom )
			continue;
		int otherRoomID;
		if( link->parent1 == room->roomID )
			otherRoomID = link->parent2;
		else
			otherRoomID = link->parent1;
		Room *otherRoom;
		if( GetValidRoomPointer( otherRoomID, otherRoom ) )
		{
			CARates const &otherRates = GetCARates( otherRoom->type, caIndex );
			float reduce = rates.GetDiffusion() * otherRates.GetDiffusion();
			AlterCAEmission( otherRoom, caIndex,
				difference * reduce *
				link->permiability * 0.01f,
				distance, room->roomID, link );
		}
	}

	// Recurse through left navigation exits:
	int leftDistance = distance+1;
	float leftDifference = difference;
	Room *leftRoom = room;
	while( leftDistance-- && leftRoom->leftNavigableDoor != NULL )
	{
		if( leftRoom->leftNavigableDoor->permiability == 0 )
			break;
		if( GetValidRoomPointer( leftRoom->leftNavigableDoor->parent1, otherRoom ) )
		{
			CARates const &rates = GetCARates( leftRoom->type, caIndex );
			CARates const &otherRates = GetCARates( otherRoom->type, caIndex );

			leftDifference *= (rates.GetDiffusion() * otherRates.GetDiffusion() *
				leftRoom->leftNavigableDoor->permiability * 0.01f);

			leftRoom = otherRoom;

			// add influence to room left of current:
			leftRoom->caValues[ caIndex ] += leftDifference;

			int count = leftRoom->linkCollection.size();
			LinkIterator linkIterator = leftRoom->linkCollection.begin();
			Link *link;
			while( count-- && leftDistance )
			{
				link = (*linkIterator++);
				// Avoid going back along link we came from
				if( link == linkFrom )
					continue;
				int otherRoomID;
				if( link->parent1 == leftRoom->roomID )
					otherRoomID = link->parent2;
				else
					otherRoomID = link->parent1;
				Room *otherRoom;
				if( GetValidRoomPointer( otherRoomID, otherRoom ) )
				{
					CARates const &otherRates = GetCARates( otherRoom->type, caIndex );
					float reduce = rates.GetDiffusion() * otherRates.GetDiffusion();
					AlterCAEmission( otherRoom, caIndex,
						leftDifference * reduce *
						link->permiability * 0.01f,
						leftDistance, leftRoom->roomID, link );
				}
			}

		}
		else
			break;
	}

	// Recurse through right navigation exits:
	int rightDistance = distance + 1;
	float rightDifference = difference;
	Room *rightRoom = room;
	while( rightDistance-- && rightRoom->rightNavigableDoor != NULL )
	{
		if( rightRoom->rightNavigableDoor->permiability == 0 )
			break;
		if( GetValidRoomPointer( rightRoom->rightNavigableDoor->parent2, otherRoom ) )
		{
			CARates const &rates = GetCARates( rightRoom->type, caIndex );
			CARates const &otherRates = GetCARates( otherRoom->type, caIndex );

			rightDifference *= (rates.GetDiffusion() * otherRates.GetDiffusion() *
				rightRoom->rightNavigableDoor->permiability * 0.01f);

			rightRoom = otherRoom;

			rightRoom->caValues[ caIndex ] += rightDifference;


			int count = rightRoom->linkCollection.size();
			LinkIterator linkIterator = rightRoom->linkCollection.begin();
			Link *link;
			while( count-- && rightDistance )
			{
				link = (*linkIterator++);
				// Avoid going back along link we came from
				if( link == linkFrom )
					continue;
				int otherRoomID;
				if( link->parent1 == rightRoom->roomID )
					otherRoomID = link->parent2;
				else
					otherRoomID = link->parent1;
				Room *otherRoom;
				if( GetValidRoomPointer( otherRoomID, otherRoom ) )
				{
					CARates const &otherRates = GetCARates( otherRoom->type, caIndex );
					float reduce = rates.GetDiffusion() * otherRates.GetDiffusion();
					AlterCAEmission( otherRoom, caIndex,
						rightDifference * reduce *
						link->permiability * 0.01f,
						rightDistance, rightRoom->roomID, link );
				}
			}
			
		}
		else
			break;
	}
}

