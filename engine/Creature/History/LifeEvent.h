#ifndef LIFE_EVENT_H
#define LIFE_EVENT_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>
#include "../../../common/C2eTypes.h"
#include "../../../modules/ModuleAPI.h"

class C2E_MODULE_API LifeEvent
{
public:
	// please add new values to the end of 
	// this enum, as the values are used in 
	// CAOS scripts
	enum EventType
	{
		typeConceived, // naturally
		typeSpliced, // artificially
		typeEngineered, // engineered

		typeBorn,
		typeNewLifeStage,
		typeExported,
		typeImported,
		typeDied,

		typeBecamePregnant,
		typeImpregnated,
		typeChildBorn,

		typeLaid, // fired from CAOS, not engine
		typeLaidEgg,// fired from CAOS, not engine
		typePhotographed,// fired from CAOS, not engine

		typeCloned, // another conception
		typeClonedSource,

 		typeWarpedOut, // exported directly to another machine over internet
		typeWarpedIn, // imported directly from another machine over internet
	};

	EventType myEventType;
	
	uint32 myWorldTick;
	uint32 myAgeInTicks;
	uint32 myRealWorldTime; // seconds since midnight (00:00:00), January 1, 1970 UTC
	int myLifeStage;
	
	std::string myRelatedMoniker1;
	std::string myRelatedMoniker2;

	std::string myUserText;
	std::string myPhoto;

	std::string myWorldName;
	std::string myWorldUniqueIdentifier;

	std::string myNetworkUser; // Babel user id

	// This is meta-data.  Whether the rest of the data in this 
	// life event has been uploaded to the tracking server yet
	// (as in Docking Station)
	bool myNetworkNeedUploading;
	// So we can track when we need to tell the server about 
	// new user text
	std::string myUploadedUserText;

	friend CreaturesArchive &operator<<( CreaturesArchive &archive, LifeEvent const &lifeEvent );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, LifeEvent &lifeEvent );

	LifeEvent();
	bool operator==(LifeEvent& other);

	bool CreaturePresentForEvent() const;
	static bool CreaturePresentForEvent(EventType event);
};


#endif // LIFE_EVENT_H

