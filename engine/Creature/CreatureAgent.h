#ifndef CreatureAgent_H
#define CreatureAgent_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Creature.h"



class CreatureAgent : public Creature
{
	CREATURES_DECLARE_SERIAL( CreatureAgent )
	typedef Creature base;

public:
	virtual bool CanSee(const AgentHandle& other);
	virtual bool Approach(bool doApproachNotRetreat=true);

	// for GALL command:
	virtual void GetBodyPartFileName(int index, std::string& path);

	virtual bool ShowPose(int pose, int partid);
	virtual bool SetAnim(const uint8* anim, int length,int partid);

	CreatureAgent();							// Serialisation constr
	virtual ~CreatureAgent();				// destruct an obj & remove it from objlib
	CreatureAgent(int family, uint32 id,
				AgentHandle gene, int gene_index,			// your genome
				int   sex,				// your sex 0=randomly choose 1=male 2=female
				int variant,		// only the agent manager can create CreatureAgents (except serialization)
				FilePath const& gallery,
				int numimages,
				int baseimage,
				int plane);

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	virtual void Trash();
	virtual void Update();

	virtual float GetSensoryDistanceFromAgent(AgentHandle a);
};

#endif //CreatureAgent_H

