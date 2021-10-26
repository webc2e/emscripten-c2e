#ifndef CAOS_TABLES_H
#define CAOS_TABLES_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

enum {
	idCommandTable = 0,
	idIntegerRVTable,
	idVariableTable,
	idFloatRVTable,
	idStringRVTable,
	idAgentRVTable,
	FIRST_SUB_COMMAND_TABLE
	};

// Add strings correspondingly to CAOSTables.cpp
enum 
{
	categoryNoNeedToDocument = 0, // must be zero (OpSpec constructor defaults to 0)
	categoryAgents,
	categoryBrain,
	categoryCamera,
	categoryCDPlayer,
	categoryCompound,
	categoryCreatures,
	categoryDebug,
	categoryFiles,
	categoryFlow,
	categoryGenetics,
	categoryHistory,
	categoryInput,
	categoryMap,
	categoryMotion,
	categoryNet,
	categoryPorts,
	categoryResources,
	categoryScripts,
	categorySounds,
	categoryTime, 
	categoryVariables,
	categoryVehicles,
	categoryWorld,

	categoryMax
};

#endif

