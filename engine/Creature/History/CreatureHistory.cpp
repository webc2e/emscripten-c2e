#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreatureHistory.h"
#include "../../CreaturesArchive.h"

#ifndef ISOLATED_HISTORY
#include "../../World.h"
#include "../../App.h"
#include "../Creature.h"
#include "../LifeFaculty.h"
#include "../../AgentManager.h"
#endif

CreatureHistory::CreatureHistory()
	: myGender(-1), myGenus(-1), myVariant(-1),
	  myCrossoverMutationCount(-1), myCrossoverCrossCount(-1),
	  myNetworkNeedUploading(true), myWarpHoleVeteran(false)
{
}

#ifndef ISOLATED_HISTORY
void CreatureHistory::AddEvent(const LifeEvent::EventType& eventType,
			 const std::string& relatedMoniker1,
			 const std::string& relatedMoniker2,
			 bool executeScript /* = true */)
{
	// don't store life events for the null moniker
	if (myMoniker.empty())
		return;

	AgentHandle creature = theAgentManager.FindCreatureAgentForMoniker(myMoniker);
	if (eventType == LifeEvent::typeBorn) //Doesn't check for typeImported anymore in case it is a related (but dead) creature.
		ASSERT(creature.IsValid());

	LifeEvent event;
	event.myEventType = eventType;

	event.myWorldTick = theApp.GetWorld().GetWorldTick();
	if (creature.IsCreature())
	{
		event.myAgeInTicks = creature.GetCreatureReference().Life()->GetTickAge();
		event.myLifeStage = creature.GetCreatureReference().Life()->GetAge();
	}
	else
	{
		event.myAgeInTicks = (uint32)-1;
		event.myLifeStage = -1;
	}
	event.myRealWorldTime = GetRealWorldTime();

	event.myRelatedMoniker1 = relatedMoniker1;
	event.myRelatedMoniker2 = relatedMoniker2;
	
	event.myWorldName = theApp.GetWorld().GetWorldName();
	event.myWorldUniqueIdentifier = theApp.GetWorld().GetUniqueIdentifier();

	// We don't fill in the network user by default - it happens
	// when the Norn is first uploaded in netbabel, at which
	// point we are confident who the user is (where as here
	// we may not have logged in yet).
	event.myNetworkUser = "";

	// add new event to container
	myLifeEvents.push_back(event);
	int newEventNo = myLifeEvents.size() - 1;
#ifdef _DEBUG
	int spareCapacity = myLifeEvents.capacity() - myLifeEvents.size();
#endif

	// send event
	if( executeScript )
	{
		CAOSVar p1, p2;
		p1.SetString(myMoniker);
		p2.SetInteger(newEventNo);
		theAgentManager.ExecuteScriptOnAllAgentsDeferred(SCRIPT_LIFE_EVENT, COASVARAGENTNULL, p1, p2);
	}
}
#endif

CreaturesArchive &operator<<( CreaturesArchive &archive, CreatureHistory const &creatureHistory )
{
	archive << creatureHistory.myMoniker;
	archive << creatureHistory.myName;
	archive << creatureHistory.myGender;
	archive << creatureHistory.myGenus;
	archive << creatureHistory.myVariant;
	archive << creatureHistory.myLifeEvents;
	archive << creatureHistory.myCrossoverMutationCount;
	archive << creatureHistory.myCrossoverCrossCount;
	archive << creatureHistory.myNetworkNeedUploading;
	archive << creatureHistory.myWarpHoleVeteran;
	archive << creatureHistory.myUploadedName;

	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, CreatureHistory &creatureHistory )
{
	archive >> creatureHistory.myMoniker;
	archive >> creatureHistory.myName;
	archive >> creatureHistory.myGender;
	archive >> creatureHistory.myGenus;
	archive >> creatureHistory.myVariant;
	archive >> creatureHistory.myLifeEvents;
	archive >> creatureHistory.myCrossoverMutationCount;
	archive >> creatureHistory.myCrossoverCrossCount;
	if (archive.GetFileVersion() >= 23)
	{
		archive >> creatureHistory.myNetworkNeedUploading;
		archive >> creatureHistory.myWarpHoleVeteran;
	}
	else
	{
		creatureHistory.myNetworkNeedUploading = true;
		creatureHistory.myWarpHoleVeteran = false;
	}

	if (archive.GetFileVersion() >= 28)
	{
		archive >> creatureHistory.myUploadedName;
	}

	return archive;
}

LifeEvent* CreatureHistory::GetLifeEvent(int i)
{
	if (i < 0 || i >= myLifeEvents.size())
		return NULL;

	return &myLifeEvents[i];
}

const LifeEvent* CreatureHistory::GetLifeEvent(int i) const
{
	if (i < 0 || i >= myLifeEvents.size())
		return NULL;

	return &myLifeEvents[i];
}

int CreatureHistory::FindLifeEvent(const LifeEvent::EventType& eventType, int fromIndex)
{
	if (fromIndex < -1)
		fromIndex = -1;
	for (int i = fromIndex + 1; i < myLifeEvents.size(); ++i)
	{
		if (myLifeEvents[i].myEventType == eventType)
			return i;
	}
	return -1;
}

int CreatureHistory::FindLifeEventReverse(const LifeEvent::EventType& eventType, int fromIndex)
{
	if (fromIndex == -1 || fromIndex > myLifeEvents.size())
		fromIndex = myLifeEvents.size();
	for (int i = fromIndex - 1; i >= 0; --i)
	{
		if (myLifeEvents[i].myEventType == eventType)
			return i;
	}
	return -1;
}

// Number of life events for the creature which 
// actually happened to him (and hence we can
// guarantee the indices for, even as he goes between
// worlds)
CreatureHistory::SolidIndex CreatureHistory::SolidCount() const
{
	SolidIndex count;
	for (int i = 0; i < myLifeEvents.size(); ++i)
		if (myLifeEvents[i].CreaturePresentForEvent())
			count.value++;
	return count;
}

LifeEvent* CreatureHistory::GetSolidLifeEvent(CreatureHistory::SolidIndex solidIndex)
{
	for (int i = 0; i < myLifeEvents.size(); ++i)
	{
		if (myLifeEvents[i].CreaturePresentForEvent())
		{
			solidIndex.value--;
			if (solidIndex.value == -1)
				return &myLifeEvents[i];
		}
	}
	ASSERT(false);
	return NULL;
}

