// Brain.h: interface for the Brain class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Brain_H
#define Brain_H

#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER


#include "../Faculty.h"
#include "../Genome.h"

class Lobe;
class Tract;
class Instinct;
class BrainComponent;

#include <vector>
typedef std::vector<class Instinct*> Instincts;
typedef std::vector<Tract*> Tracts;
typedef std::vector<Lobe*> Lobes;
typedef std::vector<BrainComponent*> BrainComponents;

class Brain : public Faculty {
	typedef Faculty base;
	friend class BrainAccess;								// for the Vat Kit
	CREATURES_DECLARE_SERIAL(Brain)
public:
	void AddInstinct(int verb, int noun, float qualifier, int drive);

	
	struct KnowledgeAction
	{
		int attentionId;
		int decisionId;
		float strength;
	};
	// Knowledge:
	KnowledgeAction GetKnowledge(int drive);


	// Constructor/destructor:
	Brain();
	virtual ~Brain();


	// Faculty functions for creating and updating the brain:
	virtual void ReadFromGenome(Genome& genome) throw(GenomeInitFailedException);
	virtual void Update();


	// Biochemistry:
	void RegisterBiochemistry(float* chemicals);
	virtual float* GetLocusAddress(int type, int organ, int tissue, int locus);


	// Functions used in instinct processing:
	void ClearActivity();
	void UpdateComponents();


	// Used by friend or foe mechanism:
	void SetNeuronState(const char* lobeTokenString, const int neuron, const int var, const float value );


	// Main I/O functions:
	void SetInput(const char* lobeTokenString, int whichNeuron, float toWhat);
	int GetWinningId(const char* lobeTokenString);


	// Vat Kit init functions:
	bool InitLobeFromDescription(std::istream &in) throw(GenomeInitFailedException);
	bool InitTractFromDescription(std::istream &in) throw(GenomeInitFailedException);


	// Set funcs used by Vat Kit to set variables remotely:
	bool SetDendriteWeight( const int tract, const int dendrite, const int var, const float value );
	bool SetNeuronState( const int lobe, const int neuron, const int var, const float value );
	float GetNeuronState( const char *lobeTokenString, const int neuron, const int state);
	bool SetLobeSVFloat(const int lobe, const int entryNo, const float value);
	bool SetTractSVFloat(const int tract, const int entryNo, const float value);

	int GetLobeSize(const char* lobeTokenString);
	char* GetLobeNameFromTissueId(int tissueId);


	// Serialisation:
	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);


	// CAOS commands for dumping out details to the Vat Kit:
	void DumpSpec(std::ostream& out);
	bool DumpLobe(int l, std::ostream& out);
	bool DumpTract(int t, std::ostream& out);
	bool DumpNeuron(int l, int n, std::ostream& out);
	bool DumpDendrite(int t, int d, std::ostream& out);

	// CAOS commands for dumping out details to a Palm export:
	bool DumpAllDendrites(int t, std::ostream& out);
	bool UnDumpAllDendrites(int t, std::istream& in);
	bool UnDumpDendrite(int t, int d, std::istream& in);

	// Instincts:
	void SetWhetherToProcessInstincts(bool b);
	bool GetWhetherToProcessInstincts();
	int GetNoOfInstinctsLeftToProcess();


protected:
	Lobe* GetLobeFromTokenString(const char* lobeTokenString);
	Lobe* GetLobeFromTissueId(int tissueId);

	bool myInstinctsAreBeingProcessed;
	Instincts myInstincts;
	float *myPointerToChemicals;
	int instinctChemicalNumber;
	int preInstinctChemicalNumber;

	BrainComponents myBrainComponents;
	Lobes myLobes;
	Tracts myTracts;
	
	std::vector<KnowledgeAction> myAssistanceKnowledge;
	int myLastKnowledgeUpdated;

	static Lobe ourDummyLobe;
};
#endif//Brain_H

