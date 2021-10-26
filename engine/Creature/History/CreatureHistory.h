#ifndef CREATURE_HISTORY_H
#define CREATURE_HISTORY_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "LifeEvent.h"
#include "../../Agents/AgentHandle.h"
#include "../../../modules/ModuleAPI.h"

#include <vector>

class C2E_MODULE_API CreatureHistory
{
public:
	CreatureHistory();

	void AddEvent(const LifeEvent::EventType& eventType,
			 const std::string& relatedMoniker1,
			 const std::string& relatedMoniker2,
			 bool executeScript = true);

	int Count() const { return myLifeEvents.size(); };
	LifeEvent* GetLifeEvent(int i);
	const LifeEvent* GetLifeEvent(int i) const;

	// These solid indices are guaranteed between worlds
	struct SolidIndex
	{
		SolidIndex() { value = 0;};
		bool operator<(const SolidIndex& other) { return value < other.value; };
		int value;
	};
	SolidIndex SolidCount() const;
	LifeEvent* GetSolidLifeEvent(SolidIndex solidIndex);

	int FindLifeEvent(const LifeEvent::EventType& eventType, int fromIndex);
	int FindLifeEventReverse(const LifeEvent::EventType& eventType, int fromIndex);

	friend CreaturesArchive &operator<<( CreaturesArchive &archive, CreatureHistory const &creatureHistory );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, CreatureHistory &creatureHistory );

	std::string myMoniker;
	std::string myName;
	int myGender;
	int myGenus;
	int myVariant;
	int myCrossoverMutationCount;
	int myCrossoverCrossCount;

	// More meta-data.  The last name uploaded to the tracking server.
	std::string myUploadedName;

	// This is meta-data.  It says whether the rest of the data in this 
	// life event has been uploaded to the tracking server yet
	// (as in Docking Station);
	bool myNetworkNeedUploading;
	bool myWarpHoleVeteran;

#ifdef _MSC_VER
#pragma warning (disable : 4251)
#endif
	std::vector<LifeEvent> myLifeEvents;
#ifdef _MSC_VER
#pragma warning (default : 4251)
#endif
};


#endif // CREATURE_HISTORY_H

