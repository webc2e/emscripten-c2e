#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "HistoryStore.h"
#include "../../CreaturesArchive.h"
#include "../../Caos/HistoryHandlers.h"

#ifndef ISOLATED_HISTORY
#include "../../App.h"
#include "../../World.h"
#include "../Creature.h"
#include "../LifeFaculty.h"
#include "../../AgentManager.h"
#include "../../Maths.h"
#endif

CreatureHistory& HistoryStore::GetCreatureHistory(const std::string& moniker)
{
	if (moniker.empty())
		return myNullHistory;

	CreatureHistory& history = myCreatureHistories[moniker];
	if (history.myMoniker.empty())
		history.myMoniker = moniker;
	return history;
}

bool HistoryStore::IsThereCreatureHistory(const std::string& moniker) const
{
	if (moniker.empty())
		return false;

	bool historyPresent = (myCreatureHistories.find(moniker) != myCreatureHistories.end());
	return historyPresent;
}

CreaturesArchive &operator<<( CreaturesArchive &archive, HistoryStore const &historyStore )
{
	archive << historyStore.myCreatureHistories;
	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, HistoryStore &historyStore )
{
	archive >> historyStore.myCreatureHistories;
	return archive;
}


std::string HistoryStore::Next(const std::string& current)
{
	if (myCreatureHistories.empty())
		return "";

	if (current.empty() || !IsThereCreatureHistory(current))
		return myCreatureHistories.begin()->first;

	std::map<std::string, CreatureHistory>::iterator next = myCreatureHistories.find(current);
	ASSERT(next != myCreatureHistories.end());
	++next;
	
	if (next == myCreatureHistories.end())
		return "";

	return next->first;
}

std::string HistoryStore::Previous(const std::string& current)
{
	if (myCreatureHistories.empty())
		return "";

	if (current.empty() || !IsThereCreatureHistory(current))
		return (--myCreatureHistories.end())->first;

	std::map<std::string, CreatureHistory>::iterator prev = myCreatureHistories.find(current);
	ASSERT(prev != myCreatureHistories.end());
	
	if (prev == myCreatureHistories.begin())
		return "";

	--prev;
	
	return prev->first;
}

void HistoryStore::WipeCreatureHistory(const std::string& moniker)
{
	std::map<std::string, CreatureHistory>::iterator it = myCreatureHistories.find(moniker);
	if (it != myCreatureHistories.end())
		myCreatureHistories.erase(it);
}

#ifndef ISOLATED_HISTORY
bool HistoryStore::ReconcileImportedHistory(const std::string& moniker, CreatureHistory& importedHistory, bool performReconciliation)
{
#ifdef _DEBUG
	// Last event in imported file should always be an export event
	LifeEvent* lastImportedEvent = importedHistory.GetLifeEvent(importedHistory.Count() - 1);
	ASSERT((lastImportedEvent->myEventType == LifeEvent::typeExported) ||
		(lastImportedEvent->myEventType == LifeEvent::typeWarpedOut));
#endif

	int ooww = GetOutOfWorldState(moniker);
	ASSERT((ooww == 0) == (!IsThereCreatureHistory(moniker) || (GetCreatureHistory(moniker).Count() == 0)));

	// If there's no history already, then we're fine
	if (ooww == 0)
	{
		if (performReconciliation)
			GetCreatureHistory(moniker) = importedHistory;
		return true;
	}

	// Unless we're exported, definitely can't import
	if (ooww != 4)
		return false;

	// Do a few checks
	CreatureHistory& worldsHistory = GetCreatureHistory(moniker);
	// first event should always be the same
	ASSERT(*worldsHistory.GetLifeEvent(0) == *importedHistory.GetLifeEvent(0));

	// Match events up to when we were last exported from this world.
	// The imported creature must have the same set of events, or else
	// - we have been imported and exported in the mean time with a cloned
	//   copy of the creature file
	// - some funny monkeys with cloned worlds has happened
	// Either way, the resulting history would become inconsistent, and
	// forever young Norns would be allowed by continually exporting, and 
	// reimporting younger versions.
	int lastFromThisWorld;
	{
		int lastExportedFromThisWorld = worldsHistory.FindLifeEventReverse(LifeEvent::typeExported, -1);
		int lastWarpedOutFromThisWorld = worldsHistory.FindLifeEventReverse(LifeEvent::typeWarpedOut, -1);
		lastFromThisWorld = OUR_MAX(lastExportedFromThisWorld, lastWarpedOutFromThisWorld);
	}

	if (lastFromThisWorld == -1)
	{
		// but ooww says we've been exported, so we _must_ have
		// an export event - this ASSERT shows an internal logic error
		ASSERT(false);
		return false;
	}
#ifdef _DEBUG
	// just a quick check that we really are exported, as ooww says we are
	int lastToThisWorld;
	{
		int lastImportedToThisWorld = worldsHistory.FindLifeEventReverse(LifeEvent::typeImported, -1);
		int lastWarpedInToThisWorld = worldsHistory.FindLifeEventReverse(LifeEvent::typeWarpedIn, -1);
		lastToThisWorld = OUR_MAX(lastImportedToThisWorld, lastWarpedInToThisWorld);
	}
	ASSERT(lastToThisWorld < lastFromThisWorld);

	// world identifier of last world export event will match ours
	ASSERT(worldsHistory.GetLifeEvent(lastFromThisWorld)->myWorldUniqueIdentifier == theApp.GetWorld().GetUniqueIdentifier());
#endif

	// If there are more events up to export than there are in the imported history,
	// then we have a mismatch
	if (lastFromThisWorld >= importedHistory.Count())
		return false;

	// If _any_ life event is different up to our export, then fail
	for (int i = 0; i <= lastFromThisWorld; ++i)
	{
		if (!((*worldsHistory.GetLifeEvent(i)) == (*importedHistory.GetLifeEvent(i))))
			return false;
	}

	// Check globals other than name are the same
	if ((worldsHistory.myMoniker != importedHistory.myMoniker) ||
	    (worldsHistory.myGender != importedHistory.myGender) ||
	    (worldsHistory.myGenus != importedHistory.myGenus) ||
	    (worldsHistory.myVariant != importedHistory.myVariant))
	{
		// Can't see why these wouldn't match, without some of
		// the events being different as well.  This is bad, so ASSERT:
		ASSERT(false);
		return false;
	}

	// Horray, we match up happily
	if (performReconciliation)
	{
		CreatureHistory newHistory;

		// Add matching events before we went away
		// - we use the imported history, so photo files correspond
		// to those which have been imported
		newHistory.myLifeEvents.insert(newHistory.myLifeEvents.end(),
			importedHistory.myLifeEvents.begin(),
			importedHistory.myLifeEvents.begin() + lastFromThisWorld + 1);
		
		// Add events from import file while we were on holiday
		newHistory.myLifeEvents.insert(newHistory.myLifeEvents.end(),
			importedHistory.myLifeEvents.begin() + lastFromThisWorld + 1,
			importedHistory.myLifeEvents.end());

		// Events that happened back at home while we were away
		newHistory.myLifeEvents.insert(newHistory.myLifeEvents.end(),
			worldsHistory.myLifeEvents.begin() + lastFromThisWorld + 1,
			worldsHistory.myLifeEvents.end());

		// Transfer name and other globals
		newHistory.myMoniker = importedHistory.myMoniker;
		newHistory.myName = importedHistory.myName;
		newHistory.myGender = importedHistory.myGender;
		newHistory.myGenus = importedHistory.myGenus;
		newHistory.myVariant = importedHistory.myVariant;
		newHistory.myCrossoverMutationCount = importedHistory.myCrossoverMutationCount;
		newHistory.myCrossoverCrossCount = importedHistory.myCrossoverCrossCount;
		
		// only need to upload global history for tracking if 
		// neither existing history has been uploaded
		newHistory.myNetworkNeedUploading = 
			worldsHistory.myNetworkNeedUploading && importedHistory.myNetworkNeedUploading;
		// a veteran in either history is a veteran here
		newHistory.myWarpHoleVeteran = worldsHistory.myWarpHoleVeteran || newHistory.myWarpHoleVeteran;

		newHistory.myUploadedName = importedHistory.myUploadedName;

		// Wipe existing history, so photos are properly cleared out
		if (!HistoryHandlers::WipeHistoryHelper(*this, moniker))
			throw BasicException("Photo in use during history reconciliation");
		GetCreatureHistory(moniker) = newHistory;
	}

	return true;
}

#endif

// Returns for a moniker:
// 0 - never existed, or purged from history
// 1 - referenced genome
// 2 - creature agent made
// 3 - properly @#BORN@
// 4 - out of world, exported / warped out
// 5 - dead, agent still exists
// 6 - dead, no agent
// 7 - unreferenced genome
#ifndef ISOLATED_HISTORY
int HistoryStore::GetOutOfWorldState(const std::string& moniker)
{
	if (!IsThereCreatureHistory(moniker))
		return 0;

	CreatureHistory& history = GetCreatureHistory(moniker);
	bool foundExpInp = false;
	bool exported = false;
	bool died = false;
	for (int i = history.Count() - 1; i >= 0; --i)
	{
		LifeEvent* event = history.GetLifeEvent(i);
		ASSERT(event);
		if (!foundExpInp && ((event->myEventType == LifeEvent::typeExported) ||
							 (event->myEventType == LifeEvent::typeWarpedOut))  )
		{
			foundExpInp = true;
			exported = true;
		}
		else if (!foundExpInp && ((event->myEventType == LifeEvent::typeImported) ||
								(event->myEventType == LifeEvent::typeWarpedIn))  )
		{
			foundExpInp = true;
			exported = false;
		}
		else if (event->myEventType == LifeEvent::typeDied)
		{
			died = true;
		}
	}

	AgentHandle creatureAgent = theAgentManager.FindCreatureAgentForMoniker(moniker);
	if (creatureAgent.IsValid())
	{
		ASSERT(!exported);

		if (died)
			return 5;

		if (creatureAgent.GetCreatureReference().Life()->GetProperlyBorn())
			return 3;
		else
			return 2;
	}
	else
	{
		if (died)
		{
			ASSERT(!exported);
			return 6;
		}

		if (exported)
		{
			ASSERT(!died);
			return 4;
		}

		AgentHandle agent = theAgentManager.FindAgentForMoniker(moniker);
		if (agent.IsValid())
			return 1; // referenced genome
		else
		{
			if (history.Count() == 0)
				// this case happens if somebody used HIST NAME or something
				// on a moniker that wasn't otherwise in the world - so
				// it has a history, but no conception event
				return 0; 
			else
				return 7; // unreferenced genome
		}
	}

	ASSERT(false);
	return -1;
}
#endif

