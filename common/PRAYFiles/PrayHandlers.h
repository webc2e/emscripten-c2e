// --------------------------------------------------------------------------------------
// Filename:	PrayHandlers.h
// Class:		PrayHandlers
// Purpose:		Centralised CAOS routines for PRAY commands
// Description:
//  The PrayHandlers class provides the CAOS machine with the PRAY commands, and rValues
// --------------------------------------------------------------------------------------

#ifndef PRAYHANDLERS_H
#define PRAYHANDLERS_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

class CAOSMachine;
class BasicException;
class PrayException;
class Creature;
class AgentHandle;
class CAOSVar;

#include <string>
#include "../../modules/ModuleAPI.h"

class PrayHandlers
{
public:
	static void  Command_PRAY( CAOSMachine& vm );
	static int IntegerRV_PRAY( CAOSMachine& vm );
	static void StringRV_PRAY( CAOSMachine& vm, std::string& str );

	static C2E_MODULE_API int ExportHelper(Creature& creature, std::string chunkType, std::string filename, bool selfExporting, bool warping);
	static int ImportHelper(std::string moniker_chunk, AgentHandle& newHandle, bool doStuff, bool porching);
	static C2E_MODULE_API bool MakeHelper(std::string aDir, std::string journalName, std::string prayName, CAOSVar& report);

private:

	static void SubCommand_PRAY_REFR( CAOSMachine& vm );
	static void SubCommand_PRAY_GARB( CAOSMachine& vm );

	static int IntegerRV_PRAY_COUN( CAOSMachine& vm );
	static int IntegerRV_PRAY_AGTI( CAOSMachine& vm );
	static int IntegerRV_PRAY_DEPS( CAOSMachine& vm );
	static int IntegerRV_PRAY_FILE( CAOSMachine& vm );
	static int IntegerRV_PRAY_TEST( CAOSMachine& vm );
	static int IntegerRV_PRAY_INJT( CAOSMachine& vm );
	static int IntegerRV_PRAY_IMPO( CAOSMachine& vm );
	static int IntegerRV_PRAY_EXPO( CAOSMachine& vm );
	static int IntegerRV_PRAY_MAKE( CAOSMachine& vm );
	static int IntegerRV_PRAY_KILL( CAOSMachine& vm );

	static void StringRV_PRAY_PREV( CAOSMachine& vm, std::string& str );
	static void StringRV_PRAY_NEXT( CAOSMachine& vm, std::string& str );
	static void StringRV_PRAY_AGTS( CAOSMachine& vm, std::string& str );
	
	static void StringRV_PRAY_BACK( CAOSMachine& vm, std::string& str );
	static void StringRV_PRAY_FORE( CAOSMachine& vm, std::string& str );

	
	// rr functions
	static int ConvertCategoryToAppId(int cat);

	static int DependencyHelper(CAOSMachine& vm, std::string& name, int doinst);
	static bool InstallFile(std::string filename, int category, bool doInstall);

	static std::string WorkOutSafetyNameAddition(const std::string &chunkType);
};

#endif //PRAYHANDLERS_H

