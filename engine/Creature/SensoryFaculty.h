// SensoryFaculty.h: interface for the SensoryFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SensoryFaculty_H
#define SensoryFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif



#include "Biochemistry/Biochemistry.h"
#include "../Display/ErrorMessageHandler.h"

#include "Definitions.h"
#include "../Stimulus.h"
#include "MotorFaculty.h"
#include "CreatureFacultyInterface.h"
#include "Faculty.h"

#include "../Agents/AgentHandle.h"

#include <vector>
#include <string>


class SensoryFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(SensoryFaculty)
public:
	typedef Faculty base;

	// 39 ("geat") is the error code, for
	// backwards compatibility.
	static const int ourCatagoryIdError;

	SensoryFaculty();
	virtual ~SensoryFaculty();
	virtual void PostInit();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);


	inline float GetNounStimThisTick() {return 0.0f;}
	inline float GetVerbStimThisTick() {return 0.0f;}

	int myNumDrives;
	int *myDriveChemicals;

	void ReadFromGenome(Genome& g);
	void Update();

	int SetupDriveChemicals();
	void AdjustChemicalLevel(int i, float f);
	void AdjustChemicalLevelWithTraining(int i, float f, int fromScriptEventNo, AgentHandle fromAgent);
	inline int GetNoOfKnownAgents() {
		return myKnownAgents.size();
	}
	inline AgentHandle GetKnownAgent(int i) {
		return myKnownAgents[i];
	}
	void SetKnownAgent(int i, AgentHandle& a){
		myKnownAgents[i] = a;
	};
	void SetVisualInput(int i);

	void Stimulate(AgentHandle& from, int stim, int fromScriptEventNo, float strengthMultiplier = 1.0f, bool forceNoLearning = false);
	void Stimulate(Stimulus s);

	static int GetNumCategories();
	static bool SetupStaticVariablesFromCatalogue();
	static int GetCategoryIdOfAgent(AgentHandle a);
	// Don't call this next one if you can avoid it - each agent
	// can have its own category now
	static int GetCategoryIdOfClassifier(const Classifier* a);

	static std::string GetCategoryName(int i);
	int GetChemicalNumberOfDrive(int driveId)
	{
		if(driveId < 0 && driveId >= myNumDrives) return -1;
		return myDriveChemicals[driveId] < 0 ||
			myDriveChemicals[driveId] > 255 ? -1 : 
		myDriveChemicals[driveId];
	};

	int GetDriveNumberOfChemical(int chem)
	{
		for(int i = 0; i != myNumDrives; i++)
		{
			if( myDriveChemicals[i] == chem )
				return i;
		}
		return -1;
	}


	virtual void Trash()
	{
		myKnownAgents.clear();
		myFriendsAndFoeHandles.clear();
		base::Trash();
		for( int i = 0; i < NUMSTIMULI; ++i )
		{
			myStimulusLib[i].fromAgent = NULLHANDLE;
			myStimulusLib[i].toCreature = NULLHANDLE;
		}
	}

	AgentHandle GetNearestCreatureOrPointer();

	int PayAttentionToCreature(int creatureNo);
	int PayAttentionToCreature(AgentHandle& lookAtCreature);
	int PayAttentionToAgent(AgentHandle& agent);

	void SetCreatureActingUponMe(AgentHandle& creatureOrPointer);
	int GetOpinionOfCreature(AgentHandle& creatureOrPointer, float &opinion, float &moodOpinion);

	void ResolveFriendAndFoe();
	static void RemoveFromAllFriendAndFoe(AgentHandle creatureOrPointer);

	void SetCategoryRepresentativeAlgorithm(int id, int setTo);
	int GetCategoryRepresentativeAlgorithm(int id);


protected:
	void SetSeenFriendOrFoe(AgentHandle& creautreOrPointer);
	void ClearSeenFriendsOrFoes();
	bool AssignFriendOrFoeByMoniker(std::string moniker, AgentHandle creatureOrPointer);

	void RemoveFriendAndFoe(AgentHandle& creatureOrPointer);
	void CleanUpInvalidFriendAgentHandles();
	int AddFriendOrFoe(AgentHandle& creatureOrPointer);

	float DistanceToNearestCreature(int sex, int genus);

protected:
	std::vector<AgentHandle> myKnownAgents;
	Stimulus myStimulusLib[NUMSTIMULI];

	static int ourNumCategories;
	static std::vector<Classifier> ourCategoryClassifiers;
	static std::vector<std::string> ourCategoryNames;

	std::vector<AgentHandle> myFriendsAndFoeHandles;
	std::vector<std::string> myFriendsAndFoeMonikers;
	std::vector<int> myFriendsAndFoeLastEncounters;
	typedef std::vector<AgentHandle>::iterator FriendOrFoeIterator;
	bool myAddedAFriendOnThisUpdate;

	// new for Sea Monkeys:
	void SetupInitialCategoryRepresentativeAlgorithms();
	std::vector<int> myCategoryRepresentativeAlgorithms;
};

#endif//SensoryFaculty_H

