// Faculty.h: interface for the Faculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Faculty_H
#define Faculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Biochemistry/ChemicallyActive.h"
//#include "Biochemistry/BiochemistryConstants.h"
#include "../PersistentObject.h"

// I'm afraid we need this one Gav - gcc gives
// scary warnings otherwise
#include "../Agents/AgentHandle.h"

class Genome;
class AgentFacultyInterface;
class CreatureFacultyInterface;

//class Faculty : public ChemicallyActive, public PersistentObject {
class Faculty : public PersistentObject, public ChemicallyActive  {
	CREATURES_DECLARE_SERIAL(Faculty)

	// bit of an abuse of notion here:
	typedef Faculty base;
public:
	Faculty();
	virtual ~Faculty();

	void SetActive(bool b);
	bool IsActive();

	virtual void SetCallback(AgentFacultyInterface* a, CreatureFacultyInterface* f);
	virtual void Init();
	virtual void Update();
	virtual void ReadFromGenome(Genome& g);
	virtual float* GetLocusAddress(int type, int organ, int tissue, int locus);

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	virtual void Trash() {}
protected:
	bool myActiveFlag;
//	AgentHandle myCreature;
	CreatureFacultyInterface* myCreature;
	AgentFacultyInterface* myCreatureAsAgent;
};
 
#endif//Faculty_H

