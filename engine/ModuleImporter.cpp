#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ModuleImporter.h"
#include "../common/C2eDebug.h"
#include "General.h"
#ifdef _WIN32
	#include <windows.h>
	#include "../common/CStyleException.h"
#else
	#include <dlfcn.h>
#endif
#include <sstream>


#ifdef _WIN32
	#ifdef _DEBUG
		#define DLL_FILESPEC "engineDebug-*.dll"
	#else
		#define DLL_FILESPEC "engine-*.dll"
	#endif
#else
	#ifdef _DEBUG
		#define DLL_FILESPEC "lc2e-*.do"
	#else
		#define DLL_FILESPEC "lc2e-*.so"
	#endif
#endif

#ifdef MODULE_EMBEDDED
	extern "C"
	{
		ModuleInterface* CreaturesEngineModuleInterface(void);
		int CreaturesEngineModuleInterfaceVersion(void);
	}
#endif

ModuleImporter ModuleImporter::theModuleImporter;

// static 
bool ModuleImporter::LoadModules(std::string& errorFile)
{
	return theModuleImporter.DoLoadModules(errorFile);
}

ModuleImporter::ModuleImporter()
{
	myIterator = 0;
	myNetworkInterface = NULL;
}

bool ModuleImporter::DoLoadModules(std::string& errorFile)
{
	#ifdef MODULE_EMBEDDED
		return LoadModule("lc2e-netbabel");
	#else
		std::vector<std::string> moduleFilenames;
		if (!GetFilesInDirectory("./", moduleFilenames, DLL_FILESPEC))
			return true;

		for (int i = 0; i < moduleFilenames.size(); ++i)
		{
			if (!LoadModule(moduleFilenames[i]))
			{
				errorFile = moduleFilenames[i];
				return false;
			}
		}

		return true;
	#endif
}

#ifdef _WIN32
	#define EXPORT_SPEC __declspec(dllexport)
#else
	#define EXPORT_SPEC
#endif

typedef EXPORT_SPEC ModuleInterface* (* ReturnInterfaceFunction)(void);
typedef EXPORT_SPEC int (* ReturnInterfaceFunctionVersion)(void);

bool ModuleImporter::LoadModule(const std::string& module)
{
	ModuleInterface* iFace = OSExtractInterface(module);
	if (!iFace)
		return false;
	myModules.push_back(iFace);

	if (iFace->IsNetworkInterface())
	{
		if (myNetworkInterface)
			return false;

		myNetworkInterface = static_cast<NetworkInterface*>(iFace);
	}

	return true;
}

// OS specific code to extract the interface
ModuleInterface* ModuleImporter::OSExtractInterface(const std::string& module)
{
#ifdef _WIN32
	HINSTANCE hinstDLL = ::LoadLibrary(module.c_str());
	if (!hinstDLL)
		return NULL;
	// Note that the library is never unloaded.  Windows unloads it
	// when the process shuts down - to unload it properly during shutdown
	// you have to be careful of the destruction order of static members.

	// We would ideally do FreeLibrary before the "return NULL" in 
	// all the error cases here.  However that causes lockups in the
	// debugger under NT, and perhaps other problems so it isn't worth it.

	ReturnInterfaceFunctionVersion pVersionFunction = (ReturnInterfaceFunctionVersion) ::GetProcAddress(hinstDLL, "CreaturesEngineModuleInterfaceVersion");
	if (!pVersionFunction )
		return NULL;
	int foreignVersion = pVersionFunction();
	if (foreignVersion != ModuleInterface::InterfaceVersion())
		return NULL;
	
	ReturnInterfaceFunction pReturnInterfaceFunction = (ReturnInterfaceFunction) ::GetProcAddress(hinstDLL, "CreaturesEngineModuleInterface");
	if (!pReturnInterfaceFunction)
		return NULL;
	CStyleException::AddModule(hinstDLL);

	return (pReturnInterfaceFunction)();
#elif MODULE_EMBEDDED
	int foreignVersion = CreaturesEngineModuleInterfaceVersion();
	if (foreignVersion != ModuleInterface::InterfaceVersion())
	{
		std::cerr << "Foreign interface version " << foreignVersion << 
					" differs from ours " << ModuleInterface::InterfaceVersion() << 
					" in built in module " << module << std::endl;
		return NULL;
	}

	return CreaturesEngineModuleInterface();
#else
	void* hinstSO = dlopen(module.c_str(), RTLD_NOW);
	if (!hinstSO)
	{
		std::cerr << "Failed to dlopen " << module << std::endl;
		std::cerr << dlerror() << std::endl;
		return NULL;
	}

	ReturnInterfaceFunctionVersion pVersionFunction = (ReturnInterfaceFunctionVersion) dlsym(hinstSO, "CreaturesEngineModuleInterfaceVersion");
	if (!pVersionFunction )
	{
		std::cerr << "Failed to dlsym function CreaturesEngineModuleInterfaceVersion in " << module;
		return NULL;
	}
	int foreignVersion = pVersionFunction();
	if (foreignVersion != ModuleInterface::InterfaceVersion())
	{
		std::cerr << "Foreign interface version " << foreignVersion << 
					" differs from ours " << ModuleInterface::InterfaceVersion() << 
					" in " << module << std::endl;
		std::cerr << dlerror() << std::endl;
		return NULL;
	}
	
	ReturnInterfaceFunction pReturnInterfaceFunction = (ReturnInterfaceFunction) dlsym(hinstSO, "CreaturesEngineModuleInterface");
	if (!pReturnInterfaceFunction)
	{
		std::cerr << "Failed to dlsym function CreaturesEngineModuleInterface in " << module << std::endl;
		std::cerr << dlerror() << std::endl;
		return NULL;
	}

	return (pReturnInterfaceFunction)();
#endif
}

void ModuleImporter::ResetIterator()
{
	theModuleImporter.myIterator = 0;
}

ModuleInterface* ModuleImporter::NextModule()
{
	ASSERT(theModuleImporter.myIterator >= 0 && theModuleImporter.myIterator <= theModuleImporter.myModules.size());
	if (theModuleImporter.myIterator == theModuleImporter.myModules.size())
		return NULL;

	ModuleInterface* ret = theModuleImporter.myModules[theModuleImporter.myIterator];
	theModuleImporter.myIterator++;

	return ret;
}

NetworkInterface* ModuleImporter::GetNetworkInterface()
{
	return theModuleImporter.myNetworkInterface;
}

std::string ModuleImporter::GetModulesDescriptiveList()
{
	std::string modules;

	bool firstModule = true;
	ResetIterator();
	while (ModuleInterface *moduleInterface = NextModule())
	{
		if (!firstModule)
			modules += ", ";
		firstModule = false;
		std::string name;
		moduleInterface->ModuleName(name);
		modules += name;
		std::ostringstream out;
		out << moduleInterface->ModuleVersion();
		modules += " ";
		modules += out.str();
	}

	return modules;
}

