// ModuleInterface.h

// Interface that the engine uses to communicate with 
// external modules

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

#include <string> // should be forward declaration, but don't know how to with templates
class CAOSDescription;
class CreaturesArchive;
#include "../engine/Agents/AgentHandle.h"

class ModuleInterface
{
public:
	static int InterfaceVersion() { return 6; };

	// Basic information
	virtual void ModuleName(std::string& name) = 0;
	virtual int ModuleVersion() = 0;

	// Called at startup to allow the module to add CAOS commands
	virtual void AddCAOSCommands(CAOSDescription& caosDescription) = 0;
	
	// Called during shutdown of application (the module can delete itself here)
	virtual void Shutdown() = 0;

	// Called every tick, with the number of ticks since the engine started
	virtual void Update(int) {}

	// Called before destroying and after making a world
	virtual void WorldEnding() {}
	virtual void WorldLoaded() {}
	
	// Persistence
	virtual bool Persistent() { return false; };
	virtual bool Write(CreaturesArchive &ar) { return true; };
	virtual bool Read(CreaturesArchive &ar) { return true; };

	// Called when an agent is twinned, so any data you have referring
	// to it can also be duplicated if necessary
	virtual void Twinned(AgentHandle original, AgentHandle clone) { return; };

	virtual bool IsNetworkInterface() { return false; }
};


#endif

