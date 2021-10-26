// Loads in all module DLLs, and provides a collection of 
// interfaces to them which the rest of the app can 
// refer to.

#ifndef MODULE_IMPORTER_H
#define MODULE_IMPORTER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../modules/ModuleInterface.h"
#include "../modules/NetworkInterface.h"
#include <vector>
#include <string>

class ModuleImporter
{
public:
	static bool LoadModules(std::string& errorFile);

	static void ResetIterator();
	static ModuleInterface* NextModule();

	static NetworkInterface* GetNetworkInterface();

	static std::string GetModulesDescriptiveList();

private:
	static ModuleImporter theModuleImporter;

	ModuleImporter();

	bool DoLoadModules(std::string& errorFile);
	bool LoadModule(const std::string& module);
	ModuleInterface* OSExtractInterface(const std::string& module);

	std::vector<ModuleInterface*> myModules;
	int myIterator;

	NetworkInterface* myNetworkInterface;
};

#endif

