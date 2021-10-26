// ----------------------------------------------------------------------------
// Filename:	BrainAccess.h
// Class:		BrainAccess
// Purpose:		Functions to access the brain
// Description:	The class is a friend of brain, lbe, tract and svrule and can
//				therefore access its members. The class has as many protected
//				functions that can be used to access the brain by any class
//				that inherits it/
// Usage:		Inheritable only
// -----------------------------------------------------------------------------


#ifndef BrainAccess_H
#define BrainAccess_H

#include <vector>
#include "Brain.h"


#ifdef VAT_KIT
#include "Lobe.h"
#include "Tract.h"
#include "BrainComponent.h"
#endif//VAT_KIT


class BrainAccess
{

public:

	// -------------------------------------------------------------------------
	// Method:		DefaultDumpVersion
	// Arguments:	none
	// Returns:		new value;		
	// -------------------------------------------------------------------------
	static inline float DefaultDumpVersion(){return 1.1f;};


	// -------------------------------------------------------------------------
	// Method:		BrainDumpVersion
	// Arguments:	none
	// Returns:		version no		
	// -------------------------------------------------------------------------
	static inline float BrainDumpVersion(){
		return ourBrainDumpVersion;
	};

protected:

	// -------------------------------------------------------------------------
	// Method:		ResetDumpVersion
	// Arguments:	none
	// Returns:		none;		
	// -------------------------------------------------------------------------
	inline void ResetDumpVersion(){ourBrainDumpVersion = DefaultDumpVersion();};

	// -------------------------------------------------------------------------
	// Method:		BrainDumpVersion
	// Arguments:	new version no
	// Returns:		none		
	// -------------------------------------------------------------------------
	inline void BrainDumpVersion(float v){
		ourBrainDumpVersion = v;
	};


#ifdef VAT_KIT
	// -------------------------------------------------------------------------
	// Method:		GetLobes
	// Arguments:	brain 
	// Returns:		the brains myLobes vector
	// -------------------------------------------------------------------------
	inline Lobes &GetLobes(Brain &brain){return brain.myLobes;};

	// -------------------------------------------------------------------------
	// Method:		GetLobe
	// Arguments:	brain, lobe number
	// Returns:		the requested lobe
	// -------------------------------------------------------------------------
	inline Lobe &GetLobe(Brain &brain, int l){return *brain.myLobes[l];};

	// -------------------------------------------------------------------------
	// Method:		GetNoLobes
	// Arguments:	brain 
	// Returns:		the size of the brains myLobes vector
	// -------------------------------------------------------------------------
	inline int GetNoLobes(Brain &brain){return brain.myLobes.size();};

	// -------------------------------------------------------------------------
	// Method:		GetNeurons
	// Arguments:	lobe 
	// Returns:		the lobes myNeurons vector
	// -------------------------------------------------------------------------
	inline std::vector<Neuron *> &GetNeurons(Lobe &lobe){return lobe.myNeurons;};

	// -------------------------------------------------------------------------
	// Method:		GetNeuron
	// Arguments:	lobe, neuron number
	// Returns:		the requested neuron
	// -------------------------------------------------------------------------
	inline Neuron &GetNeuron(Lobe &lobe, int n){return *lobe.myNeurons[n];};

	// -------------------------------------------------------------------------
	// Method:		GetLobeSize
	// Arguments:	lobe 
	// Returns:		the size of the myNeurons vector
	// -------------------------------------------------------------------------
	inline int GetLobeSize(Lobe &lobe){return lobe.myNeurons.size();};

	// -------------------------------------------------------------------------
	// Method:		GetTracts
	// Arguments:	brain 
	// Returns:		the brains myTracts vector
	// -------------------------------------------------------------------------
	inline Tracts &GetTracts(Brain &brain){return brain.myTracts;};

	// -------------------------------------------------------------------------
	// Method:		GetTract
	// Arguments:	brain, tract number
	// Returns:		the requested tract
	// -------------------------------------------------------------------------
	inline Tract &GetTract(Brain &brain, int t){return *brain.myTracts[t];};

	// -------------------------------------------------------------------------
	// Method:		GetDstLobe
	// Arguments:	tract 
	// Returns:		the tracts destination lobe
	// -------------------------------------------------------------------------
	inline Lobe &GetDstLobe(Tract &tract){return *tract.myDst.lobe;};

	// -------------------------------------------------------------------------
	// Method:		GetSrcLobe
	// Arguments:	tract 
	// Returns:		the tracts source lobe
	// -------------------------------------------------------------------------
	inline Lobe &GetSrcLobe(Tract &tract){return *tract.mySrc.lobe;};

	// -------------------------------------------------------------------------
	// Method:		IsMigrate
	// Arguments:	tract 
	// Returns:		whether the tract has random connections and migrates
	// -------------------------------------------------------------------------
	inline bool IsMigrate(Tract &tract)
				{return tract.myDendritesAreRandomlyConnectedAndMigrate;};

	// -------------------------------------------------------------------------
	// Method:		GetNoTracts
	// Arguments:	brain  
	// Returns:		the brains myTracts size
	// -------------------------------------------------------------------------
	inline int GetNoTracts(Brain &brain){return brain.myTracts.size();};

	// -------------------------------------------------------------------------
	// Method:		GetDendrites
	// Arguments:	tract 
	// Returns:		the lobes myDendrites vector
	// -------------------------------------------------------------------------
	inline std::vector<Dendrite *> &GetDendrites(Tract &tract){return tract.myDendrites;};

	// -------------------------------------------------------------------------
	// Method:		GetDendrite
	// Arguments:	tract, dendrite number
	// Returns:		the requested dendrite
	// -------------------------------------------------------------------------
	inline Dendrite &GetDendrite(Tract &tract, int d){return *tract.myDendrites[d];};

	// -------------------------------------------------------------------------
	// Method:		GetSrcNeuron
	// Arguments:	tract, dendrite number
	// Returns:		pointer to the source neuron of the dendrite in the tract
	// -------------------------------------------------------------------------
	inline Neuron *GetSrcNeuron(Tract &tract, int d)
								{return tract.myDendrites[d]->srcNeuron;};

	// -------------------------------------------------------------------------
	// Method:		GetDstNeuron
	// Arguments:	tract, dendrite number
	// Returns:		pointer to the destination neuron of the dendrite in the tract
	// -------------------------------------------------------------------------
	inline Neuron *GetDstNeuron(Tract &tract, int d)
								{return tract.myDendrites[d]->dstNeuron;};

	// -------------------------------------------------------------------------
	// Method:		GetTractSize
	// Arguments:	tracts 
	// Returns:		the size of the myDendrites vector
	// -------------------------------------------------------------------------
	inline int GetTractSize(Tract &tracts){return tracts.myDendrites.size();};

	// -------------------------------------------------------------------------
	// Method:		GetLobeX
	// Arguments:	lobe 
	// Returns:		the lobes myX
	// -------------------------------------------------------------------------
	inline int GetLobeX(Lobe &lobe){return lobe.myX;};

	// -------------------------------------------------------------------------
	// Method:		GetLobeY
	// Arguments:	lobe 
	// Returns:		the lobes myY
	// -------------------------------------------------------------------------
	inline int GetLobeY(Lobe &lobe){return lobe.myY;};

	// -------------------------------------------------------------------------
	// Method:		GetLobeWidth
	// Arguments:	lobe 
	// Returns:		the lobes myWidth
	// -------------------------------------------------------------------------
	inline int GetLobeWidth(Lobe &lobe){return lobe.myWidth;};

	// -------------------------------------------------------------------------
	// Method:		GetLobeHeight
	// Arguments:	lobe 
	// Returns:		the lobes myHeight
	// -------------------------------------------------------------------------
	inline int GetLobeHeight(Lobe &lobe){return lobe.myHeight;};

	// -------------------------------------------------------------------------
	// Method:		GetLobeRed
	// Arguments:	lobe 
	// Returns:		the lobes myColour[0]
	// -------------------------------------------------------------------------
	inline int GetLobeRed(Lobe &lobe){return lobe.myColour[0];};

	// -------------------------------------------------------------------------
	// Method:		GetLobeGreen
	// Arguments:	lobe 
	// Returns:		the lobes myColour[1]
	// -------------------------------------------------------------------------
	inline int GetLobeGreen(Lobe &lobe){return lobe.myColour[1];};

	// -------------------------------------------------------------------------
	// Method:		GetLobeBlue
	// Arguments:	lobe 
	// Returns:		the lobes myColour[0]
	// -------------------------------------------------------------------------
	inline int GetLobeBlue(Lobe &lobe){return lobe.myColour[2];};


	// -------------------------------------------------------------------------
	// Method:		SetLobeX
	// Arguments:	lobe, new value
	// Returns:		none
	// Description:	Sets the lobe myX to new value
	// -------------------------------------------------------------------------
	inline void SetLobeX(Lobe &lobe, int value){lobe.myX = value;};

	// -------------------------------------------------------------------------
	// Method:		SetLobeY
	// Arguments:	lobe, new value
	// Returns:		none
	// Description:	Sets the lobe myY to new value
	// -------------------------------------------------------------------------
	inline void SetLobeY(Lobe &lobe, int value){lobe.myY = value;};

	// -------------------------------------------------------------------------
	// Method:		SetLobeWidth
	// Arguments:	lobe, new value
	// Description:	sets the lobes myWidth to the new value
	// -------------------------------------------------------------------------
	inline void SetLobeWidth(Lobe &lobe, int value){lobe.myWidth = value;};

	// -------------------------------------------------------------------------
	// Method:		SetLobeHeight
	// Arguments:	lobe, new value
	// Description:	sets the lobes myHeight to the new value
	// -------------------------------------------------------------------------
	inline void SetLobeHeight(Lobe &lobe, int value){lobe.myHeight = value;};

	// -------------------------------------------------------------------------
	// Method:		LobeIdInList
	// Arguments:	lobe
	// Returns:		none
	// Description:	finds the index of the lobe in the lobes vector
	// -------------------------------------------------------------------------
	inline int LobeIdInList(Lobe &lobe) {return *lobe.GetPointerToIdInList();};

	// -------------------------------------------------------------------------
	// Method:		NeuronIdInList
	// Arguments:	neuron
	// Returns:		none
	// Description:	finds the index of the neuron in the vector
	// -------------------------------------------------------------------------
	inline int NeuronIdInList(Neuron &neuron){return neuron.idInList;};

	// -------------------------------------------------------------------------
	// Method:		TractIdInList
	// Arguments:	tract
	// Returns:		none
	// Description:	finds the index of the tract in the tracts vector
	// -------------------------------------------------------------------------
	inline int TractIdInList(Tract &tract){return *tract.GetPointerToIdInList();};

	// -------------------------------------------------------------------------
	// Method:		TractIdInList
	// Arguments:	dendrite
	// Returns:		none
	// Description:	finds the index of the dendrite in the vector
	// -------------------------------------------------------------------------
	inline int DendriteIdInList(Dendrite &dendrite){return dendrite.idInList;};

	// -------------------------------------------------------------------------
	// Method:		SetVar
	// Arguments:	neuron to set, var in neuron to set, value to set to
	// Returns:		none
	// Description:	sets the var in neuron to value
	// -------------------------------------------------------------------------
	inline void SetVar(Neuron &neuron, const int var, const float value);

	// -------------------------------------------------------------------------
	// Method:		SetVar
	// Arguments:	dendrite to set, var in dendrite to set, value to set to
	// Returns:		none
	// Description:	sets the var in dendrite to value
	// -------------------------------------------------------------------------
	inline void SetVar(Dendrite &dendrite, const int var, const float value);

	// -------------------------------------------------------------------------
	// Method:		GetVar
	// Arguments:	neuron to get from, var in neuron to get
	// Returns:		none
	// Description:	gets the var in neuron
	// -------------------------------------------------------------------------
	inline const float GetVar(const Neuron &neuron, const int var);

	// -------------------------------------------------------------------------
	// Method:		GetVar
	// Arguments:	dendrite to get from, var in dendrite to get
	// Returns:		none
	// Description:	gets the var in dednrite
	// -------------------------------------------------------------------------
	inline const float GetVar(const Dendrite &dendrite, const int var);	

	// -------------------------------------------------------------------------
	// Method:		GetLobeName
	// Arguments:	lobe
	// Returns:		lobe name character string
	// Description:	
	// -------------------------------------------------------------------------
	inline const char *GetLobeName(Lobe &lobe) const 
		{const char *lobeName = (char*)lobe; return lobeName;};

	// -------------------------------------------------------------------------
	// Method:		GetTractName
	// Arguments:	pointer to a tract
	// Returns:		tract name character string
	// Description:	
	// -------------------------------------------------------------------------
	inline const char *GetTractName(const Tract &tract) const {return tract.myName.c_str();};	
	
	// -------------------------------------------------------------------------
	// Method:		GetDendriteDesc
	// Arguments:	dendrite, tract, src neuron number
	//				dst neuron number
	// Returns:		dendrite number
	// Description:	fills in src and dst neuron numbers if src & dst args
	//				are not null
	// -------------------------------------------------------------------------
	inline const int GetDendriteDesc(Dendrite &dendrite, Tract &tract, int *src, int *dst)  
	{
		if(src)
			*src = NeuronIdInList(*dendrite.srcNeuron);
		if(dst)
			*dst = NeuronIdInList(*dendrite.dstNeuron);
		return DendriteIdInList(dendrite);
	};

	// -------------------------------------------------------------------------
	// Method:		GetLobeSize
	// Arguments:	lobe
	// Returns:		number of neurons in lobe
	// Description:	Used by lobeinputdlg 
	// -------------------------------------------------------------------------
	inline int GetLobeSize(const Lobe &lobe){return lobe.myNeurons.size();};

	// -------------------------------------------------------------------------
	// Method:		SetLobeInputs
	// Arguments:	lobe, pointer to array of floats
	// Returns:		none
	// Description:	fills lobe inputs with floats
	// -------------------------------------------------------------------------
	inline void SetLobeInputs(Lobe &lobe, const float * const values);

	// -------------------------------------------------------------------------
	// Method:		GetUpdateSVRule
	// Arguments:	lobe
	// Returns:		svrule
	// Description:	pointer to the lobes svrule
	// -------------------------------------------------------------------------
	SVRule &GetUpdateSVRule(Lobe &lobe){return lobe.myUpdateRule;};
	
	// -------------------------------------------------------------------------
	// Method:		GetUpdateSVRule
	// Arguments:	tract
	// Returns:		svrule
	// Description:	returns a pointer to the tract svrule
	// -------------------------------------------------------------------------
	SVRule &GetUpdateSVRule(Tract &tract){return tract.myUpdateRule;};


	// -------------------------------------------------------------------------
	// Method:		GetInitSVRule
	// Arguments:	lobe
	// Returns:		svrule
	// Description:	pointer to the lobes svrule
	// -------------------------------------------------------------------------
	SVRule &GetInitSVRule(Lobe &lobe){return lobe.myInitRule;};
	
	// -------------------------------------------------------------------------
	// Method:		GetInitSVRule
	// Arguments:	tract
	// Returns:		svrule
	// Description:	returns a pointer to the tract svrule
	// -------------------------------------------------------------------------
	SVRule &GetInitSVRule(Tract &tract){return tract.myInitRule;};

	// -------------------------------------------------------------------------
	// Method:		GetSVRule
	// Arguments:	svrule, entry index
	// Returns:		svrule entry
	// Description:	returns a entry i of svrule
	// -------------------------------------------------------------------------
	const SVRuleEntry& GetSVRuleEntry(SVRule &sVRule, int i){return sVRule.myRule[i];};

	// -------------------------------------------------------------------------
	// Method:		SetSVRule
	// Arguments:	svrule, entry index, new value
	// Returns:		none
	// Description:	sets entry i of svrule to value
	// -------------------------------------------------------------------------
	void SetSVRuleEntry(SVRule &sVRule, int i, float value)
							{sVRule.myRule[i].floatValue = value;};


	// -------------------------------------------------------------------------
	// Method:		GetInitRunsAlways
	// Arguments:	lobe
	// Returns:		whether always run
	// Description:	returns a pointer to the tract svrule
	// -------------------------------------------------------------------------
	bool GetInitRunsAlways(Lobe &lobe){return lobe.myRunInitRuleAlwaysFlag;};

	// -------------------------------------------------------------------------
	// Method:		GetInitRunsAlways
	// Arguments:	tract
	// Returns:		whether always run
	// Description:	returns a pointer to the tract svrule
	// -------------------------------------------------------------------------
	bool GetInitRunsAlways(Tract &tract){return tract.myRunInitRuleAlwaysFlag;};
#endif

private:

	static float ourBrainDumpVersion;
};


#ifdef VAT_KIT
// -------------------------------------------------------------------------
// Method:		SetVar
// Arguments:	neuron to set, var in neuron to set, value to set to
// Returns:		none
// Description:	sets the var in neuron to value
// -------------------------------------------------------------------------
void BrainAccess::SetVar(Neuron &neuron, const int var, const float value)
{
	neuron.states[var] = value;
}


// -------------------------------------------------------------------------
// Method:		SetVar
// Arguments:	dendrite to set, var in dendrite to set, value to set to
// Returns:		none
// Description:	sets the var in dendrite to value
// -------------------------------------------------------------------------
void BrainAccess::SetVar(Dendrite &dendrite, const int var, const float value)
{
	dendrite.weights[var] = value;
}


// -------------------------------------------------------------------------
// Method:		GetVar
// Arguments:	neuron to get from, var in neuron to get
// Returns:		none
// Description:	gets the var in neuron
// -------------------------------------------------------------------------
const float BrainAccess::GetVar(const Neuron &neuron, const int var)
{
	return neuron.states[var];
}


// -------------------------------------------------------------------------
// Method:		GetVar
// Arguments:	dendrite to get from, var in dendrite to get
// Returns:		none
// Description:	gets the var in dednrite
// -------------------------------------------------------------------------
const float BrainAccess::GetVar(const Dendrite &dendrite, const int var)
{
	return dendrite.weights[var];
}

// -------------------------------------------------------------------------
// Method:		SetLobeInputs
// Arguments:	lobe, pointer to array of floats
// Returns:		none
// Description:	fills lobe inputs with floats
// -------------------------------------------------------------------------
void BrainAccess::SetLobeInputs(Lobe &lobe, const float * const values)
{
	for(int n = 0; n < lobe.myNeurons.size(); n++)
		lobe.SetNeuronInput(n, values[n]);
}
#endif//VAT_KIT

#endif

