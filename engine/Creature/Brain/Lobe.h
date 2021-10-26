// Lobe.h: interface for the Lobe class.
#ifndef Lobe_H
#define Lobe_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BrainComponent.h"
#include "Neuron.h"
#include "../../Token.h"
#include "../Genome.h"

#include <vector>
typedef std::vector<Neuron*> Neurons;


class Lobe : public BrainComponent { 
	typedef BrainComponent base;
	friend class BrainAccess;
	CREATURES_DECLARE_SERIAL(Lobe)
public:
	Lobe(Genome& genome) throw(GenomeInitFailedException);
	Lobe(std::istream &in) throw(GenomeInitFailedException);
	Lobe();
	Lobe(const Lobe& l);
	virtual ~Lobe();

	virtual void Initialise();

	virtual void DoUpdate();
	void ClearActivity();
	void DoUpdateFromDesc(std::istream &inn);
	void DoUpdateNeuronFromDesc(int i, std::istream &in);
	virtual void TraceDebugInformation();

	inline operator char*() {			return myName;}
	inline TOKEN GetToken() {			return myToken;}
	inline int GetNoOfNeurons() {		return myNeurons.size();}
	inline Neuron* GetNeuron(int i) {	return myNeurons[i];}	// for Tract init

	// new by gtb 6/1/00:
	inline int GetWidth() {				return myWidth;}
	inline int GetHeight() {			return myHeight;}
	

	float GetNeuronState(int whichNeuron, int whichState);
	float* GetNeuronStatePointer(int whichNeuron, int whichState);
	int GetWhichNeuronWon();
	void SetNeuronInput(int whichNeuron, float toWhat);


	inline int GetTissueId() {			return myTissueId;}
	inline SVRuleVariables& GetSpareNeuronVariables() const {
		return *mySpareNeuronVariables;
	}
	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	// sets used by appletts
	bool SetNeuronState( const int neuron, const int var, const float value );
	bool SetSVFloat(int entryNo, float value);

	void DumpLobe(std::ostream& out);
	bool DumpNeuron(int n, std::ostream& out);
	int DumpSize();

protected:
	int myWinningNeuronId;
	SVRuleVariables* mySpareNeuronVariables;

	TOKEN myToken;
	char myName[5];
	int myTissueId;
	Neurons myNeurons;

	int myX, myY, myWidth, myHeight;
	int myColour[3];

	float* myNeuronInput;
};
#endif//Lobe_H

