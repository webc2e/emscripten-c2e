#ifndef BrainConstants_H
#define BrainConstants_H

const int NUM_SVRULE_VARIABLES=8;
typedef float SVRuleVariables[NUM_SVRULE_VARIABLES];

const int MAX_LOBES=255;
const int MAX_TRACTS=255;
const int MAX_DENDRITES_PER_TRACT=255*255;
const int MAX_NEURONS_PER_LOBE=255*255;

const int MAX_INSTINCTS = 255;		// max # pending instincts allowed


enum NeuronVariableNames {
	STATE_VAR,
	INPUT_VAR,
	OUTPUT_VAR,
	THIRD_VAR,
	FOURTH_VAR,
	FIFTH_VAR,
	SIXTH_VAR,
	NGF_VAR				// Neural Growth Factor - controls dendrite migration
};

enum DendriteVariableNames {
	WEIGHT_SHORTTERM_VAR,
	WEIGHT_LONGTERM_VAR,
	SECOND_DENDRITE_VAR,
	THIRD_DENDRITE_VAR,
	FOURTH_DENDRITE_VAR,
	FIFTH_DENDRITE_VAR,
	SIXTH_DENDRITE_VAR,
	STRENGTH_VAR		// Determines how 'permanent' the dendrite is (i.e. unwilling to migrate)
};


#endif//BrainConstants_H

