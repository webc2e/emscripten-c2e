// -------------------------------------------------------------------------
// Filename:    AgentManager.h
// Class:       AgentManager
// Purpose:     Looks after the Agent set
// Description: Initially just moving static information from the Agent Class
//
//				I have made the constructor private so that there can be
//				only one agent manager
// Usage:		
//
// History:
// 20Jan99 Alima	Initial version
//					Moved the main view from the world class to here.
//					agents add themselves to the display engine when they
//					construct and remove themselves during destruction.
//					This is all done through the Agent manager.
// 27Jan99	Alima	Added update plane so that objects can tell the display
//					engine that they have switched planes
//					
// -------------------------------------------------------------------------
#ifndef AGENT_MANAGER_H
#define AGENT_MANAGER_H

#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include "../common/C2eTypes.h"

#include <map>
#include <list>
#include <vector>
#include "mfchack.h"
#include <algorithm>

#include "Caos/CAOSVar.h"
#include "Classifier.h"
#include "../modules/ModuleAPI.h"
#include "FilePath.h"

////////////////////////////////////////////////////////////////////////////
// Foward Declarations all types of agent we can create
// for now these are the un-recfactored classes
////////////////////////////////////////////////////////////////////////////
class Agent;
class Creature;
class SimpleAgent;
class PointerAgent;
class SkeletalCreature;
class MainCamera;
class EntityImage;
class CompoundAgent;
class Classifier;
class Vehicle;
class CreaturesArchive;
class FilePath;
class AgentManager;

typedef std::list<AgentHandle> AgentList;
typedef std::list<AgentHandle>::iterator AgentListIterator;
typedef std::map<uint32, AgentListIterator> AgentMap;
typedef std::map<uint32, AgentListIterator>::iterator AgentMapIterator;
typedef std::list<AgentMapIterator>	AgentMapIteratorList;
typedef AgentMapIteratorList::iterator	AgentMapIteratorListIterator;
typedef std::vector<AgentHandle> CreatureCollection;
typedef CreatureCollection::iterator CreatureCollectionIterator;

typedef std::multimap<Classifier,AgentHandle> TaxonomicalAgentMap;
typedef TaxonomicalAgentMap::iterator TaxonomicalAgentIterator;

typedef std::map<uint32, TaxonomicalAgentIterator> TaxonomicalHelperMap;
typedef TaxonomicalHelperMap::iterator TaxonomicalHelperIterator;

// so that calls to the agent manager are tidy
#define theAgentManager AgentManager::GetAgentManager()

class C2E_MODULE_API AgentManager
{
public:
// constructor made private to ensure only one agent manager exists
AgentManager(int do_not_call_me, int ever_as_i_throw)
{
while (true);
}
////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
	~AgentManager();


////////////////////////////////////////////////////////////////////////////
// Agent Creation methods
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      CreateCreatureAgent
// Arguments:   None 		
//
// Returns:     pointer to the newly created agent
//
// Description: The only way to create a creature.  Note that this no longer
//				immediately adds this to the agent update list
//						
// ----------------------------------------------------------------------
AgentHandle CreateCreatureAgent(
					int family,
				    AgentHandle gene, int gene_index,
					int   sex,
					int   variant,
					FilePath const& gallery,
					int numimages,
					int baseimage,
					int plane);

// ----------------------------------------------------------------------
// Method:      CreateSkeletalCreature
// Arguments:   None 		
//
// Returns:     pointer to the newly created agent
//
// Description: The only way to create a creature.  Note that this no longer
//				immediately adds this to the agent update list
//						
// ----------------------------------------------------------------------
	AgentHandle CreateSkeletalCreature(int family,
		AgentHandle& gene, int gene_index, int8 sex, int8 variant=0);


// ----------------------------------------------------------------------
// Method:      AddCreatureToUpdateList
// Arguments:   handle of the newly created creature 		
//
// Returns:     pointer to the newly created agent
//
// Description: Creature is ready to go on the update list
//						
// ----------------------------------------------------------------------	
	void AddCreatureToUpdateList(AgentHandle const& creature);

// ----------------------------------------------------------------------
// Method:      CreateSimpleAgent
// Arguments:   filename - name of gallery
//				firstimage - base pointer for the objects images in gallery
//				imagecount - number of images in gallery
//				plane - z - order
//				attr - agent attributes
//						
//
// Returns:     pointer to the newly created agent
//
// Description: The only way to create a SimpleAgent. 
//				 Add it to the agent library mapped by a unique id
//						
// ----------------------------------------------------------------------
	AgentHandle CreateSimpleAgent(int family, int genus, int species,
		FilePath const& gallery,
		int32 imagecount,
		int32 firstimage,
		int32 plane);



// ----------------------------------------------------------------------
// Method:      CreateCompoundAgent
// Arguments:   gallery - name of gallery
//				baseimage - base pointer for the objects images in gallery
//				numimages - number of images in gallery
//				uniqueid - to map the agent
//						
//
// Returns:     pointer to the newly created agent
//
// Description: The only way to create a CompoundAgent. 
//				 Add it to the agent library mapped by a unique id
//						
// ----------------------------------------------------------------------
AgentHandle CreateCompoundAgent
	(int family, int genus, int species,
	FilePath const& gallery,
	int32 numimages,
	int32 baseimage,
	int32 plane);


AgentHandle CreateVehicle
	(int family, int genus, int species,
	FilePath const& gallery,
	int32 numimages,
	int32 baseimage,
	int32 plane);

AgentHandle CreatePointer();


////////////////////////////////////////////////////////////////////////////
// Agent Update methods
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      theAgentManager
// Arguments:   None 		
//
// Returns:     reference to the display engine
//
// Description: This is the only way to use the agent manager
//						
// ----------------------------------------------------------------------
	static AgentManager& GetAgentManager();

	


	// ----------------------------------------------------------------------
	// Agent searching routines
	// ----------------------------------------------------------------------

	void FindByFGS( AgentList& agents, const Classifier& c );

	void FindByArea( AgentList& input, AgentList& agents, const Box& r );
	void FindByArea( AgentList& agents, const Box& r );
	void FindByAreaAndFGS( AgentList& agents, const Classifier& c,
		const Box& r );

	void FindBySightAndFGS( AgentHandle const& viewer,
		AgentList& agents,
		const Classifier& c );


// ----------------------------------------------------------------------
// Method:      WhoAmITouching
// Arguments:   me - creature doing the looking		
//
// Returns:     the first creature you can hear
//
// Description: cycle through creatures and find the first one that
//				 the given creature is touching
//						
// ----------------------------------------------------------------------
AgentHandle WhoAmITouching(AgentHandle& me);

// ----------------------------------------------------------------------
// Method:      WhatAmITouching
// Arguments:   me - creature doing the looking		
//
// Returns:     the first creature you can hear
//
// Description: cycle through agents and find the best one that
//				 the given creature is touching
//						
// ----------------------------------------------------------------------
AgentHandle WhatAmITouching(Vector2D& ptThis, AgentHandle& me,uint32 AttrFields,
									 uint32 AttrState);

AgentHandle WhatAmIOver(const AgentHandle& me, const Vector2D& xy);
AgentHandle WhatCreatureAmIOver(const AgentHandle& me, const Vector2D& xy);


AgentHandle WhatVehicleCouldIDropInto(const AgentHandle& me, const AgentHandle& ignore);

// ----------------------------------------------------------------------
// Method:      HowManyCreaturesCanIHear
// Arguments:   me - agent doing the hearing
//				score - total for how much noise neighbouring creatures
//						are making		
//
// Returns:     the first creature you can hear
//
// Description: cycle through creatures and accumulate a score for each
//				one I can hear
//						
// ----------------------------------------------------------------------
void HowManyCreaturesCanIHear(AgentHandle& me, int32& score);

// ----------------------------------------------------------------------
// Method:      GenericMatchForWildCard
// Arguments:   classifier details		
//
// Returns:     the first creature you find with matching classifier
//				criteria
//
// Description: cycle through agents and find the first one that
//				 matches the given criteria
//						
// ----------------------------------------------------------------------
//AgentHandle GenericMatchForWildCard(const Classifier& rOther, 
//								bool bFamily /*= TRUE*/,
//								bool bGenus/* = TRUE*/, 
//								bool bSpecies /*= TRUE*/, 
//								bool bEvent /*= TRUE*/,
//								AgentMapIterator& startFrom/*NULL*/);

// ----------------------------------------------------------------------
// Method:      CalculateMoodAndThreat
// Arguments:   iSelectableCount - how many creatures were selectable
//				iLowestHealth - the lowest health value encountered
//				fMood		- sum of creature's mood
//				fThreat   - sum of creature's threat		
//
// Returns:     the first creature you can hear
//
// Description: get a sum of selectable creature's mood and threat.
//						
// ----------------------------------------------------------------------
void CalculateMoodAndThreat(uint32& iSelectableCount,
							uint32& iLowestHealth,
							float& fMood,
							float& fThreat);

	static void KillAgent(const AgentHandle& agent);
	void UpdateAllAgents();
	void KillAllAgents();
	void ExecuteScriptOnAllAgents(int event, const CAOSVar& from, const CAOSVar& p1, const CAOSVar& p2);
	void ExecuteScriptOnAllAgentsDeferred(int event, const CAOSVar& from, const CAOSVar& p1, const CAOSVar& p2);
	void ExecuteScriptOnAgent(AgentHandle agent, int event, const CAOSVar& from, const CAOSVar& p1, const CAOSVar& p2);
	void ExecuteDeferredScripts();

// ----------------------------------------------------------------------
// Method:      UpdateSmellClassifierList
// Arguments:	caIndex classifier
//
// Returns:     None
//
// Description: Updates a list for what smells are linked to which classifiers.
// ----------------------------------------------------------------------
	bool UpdateSmellClassifierList(int caIndex, Classifier* classifier);

// ----------------------------------------------------------------------
// Method:      GetCategoryIdFromSmellId
// Arguments:	caIndex
//
// Returns:     an id
//
// Description: Gets the classifier of agents which emit the smell ID given.
// ----------------------------------------------------------------------
	int GetCategoryIdFromSmellId(int smellId);
	
// ----------------------------------------------------------------------
// Method:      GetClassifierFromSmellId
// Arguments:	caIndex
//
// Returns:     an id
//
// Description: Gets the classifier of agents which emit the smell ID given.
// ----------------------------------------------------------------------

	int GetSmellIdFromCategoryId(int categoryId);



// ----------------------------------------------------------------------
// Method:      UniqueCreaturePlane 
// Arguments:   me - don't compare me to myself	
//
// Returns:     a unique plane
//
// Description: cycle through agents and find an available plane
//				Scan all existing creatures and pick a plot plane 
//				that doesn't clash with any of them.
//				This plot plane will be yours for the rest of your life 
//				OR UNTIL YOU ARE IMPORTED ELSEWHERE!				
// ----------------------------------------------------------------------
	int32 UniqueCreaturePlane(AgentHandle& me);


// ----------------------------------------------------------------------
// Method:      GetAgentFromID 
// Arguments:   id - id of agent to find			
//
// Returns:     the agent with the requested id or none
//
// Description: go figure
//						
// ----------------------------------------------------------------------
	AgentHandle GetAgentFromID(uint32 id);

	AgentListIterator GetAgentIteratorStart();

	AgentHandle FindCreatureAgentForMoniker(std::string moniker);
	AgentHandle FindAgentForMoniker(std::string moniker);

// ----------------------------------------------------------------------
// Method:      GetAgentIDMap 
// Arguments:   None		
//
// Returns:     constant reference to the agentid map
//
// Description: Sometimes it's just easier to allow access to the map
//						
// ----------------------------------------------------------------------
	static AgentMap& GetAgentIDMap();
	static CreatureCollection& GetCreatureCollection();

	AgentHandle FindNextAgent(AgentHandle& was, const Classifier& c);
	AgentHandle FindPreviousAgent(AgentHandle& was, const Classifier& c);

	AgentHandle GetCreatureByIndex(uint32 index);
	int GetCreatureCount();

	void RegisterClone(AgentHandle& clone);

	bool IsEnd(AgentListIterator& it);

#ifdef C2D_DIRECT_DISPLAY_LIB
void RestoreAllTints();
#endif

	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;

	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);

// This *should* be private, but for an annoying 'feature' of friend declarations
	struct DeferredScript
	{
		DeferredScript() {}
		DeferredScript(int theEvent, const CAOSVar &theFrom,
			const CAOSVar &theP1, const CAOSVar &theP2)
			:event( theEvent), from( theFrom ), p1( theP1 ), p2( theP2 )
		{}
		int event;
		CAOSVar from;
		CAOSVar p1;
		CAOSVar p2;
	};

	static FilePath MakeGalleryPath( std::string const& name);

private:

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
	AgentManager();



// ----------------------------------------------------------------------
// Method:      CreateUniqueAgentID
// Arguments:   None 		
//
// Returns:     the unique id created
//
// Description: Creates a unique id for the caller
//						
// ----------------------------------------------------------------------
	static uint32 CreateUniqueAgentID();


////////////////////////////////////////////////////////////////////////////
// Copy Constructor and assigment operator declared but not implemented
////////////////////////////////////////////////////////////////////////////
	AgentManager(const AgentManager&);
	AgentManager& operator=(const AgentManager&);

////////////////////////////////////////////////////////////////////////////
// Members
////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (disable : 4251)
#endif
	static AgentManager ourAgentManager;
	static AgentMap ourAgentMap;
	static AgentList ourAgentList;
	static TaxonomicalAgentMap ourFGSMap;
	static TaxonomicalHelperMap ourFGSHelperMap;

	static AgentMapIteratorList ourKillList;
	static std::vector<int> ourCategoryIdsForSmellIds;
	static CreatureCollection ourCreatureCollection;
	static uint32 ourBaseUniqueID;

	static int ourDummyInitVar;
	static int InitCategorySmellMap();
	
	typedef std::list< DeferredScript > DeferredScriptList;
	DeferredScriptList myDeferredScripts;
#ifdef _MSC_VER
#pragma warning (default : 4251)
#endif
};

CreaturesArchive& operator<<( CreaturesArchive &ar, AgentManager::DeferredScript const &script );
CreaturesArchive& operator>>( CreaturesArchive &ar, AgentManager::DeferredScript &script );

#endif //AGENT_MANAGER

