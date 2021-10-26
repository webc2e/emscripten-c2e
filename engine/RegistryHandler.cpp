// --------------------------------------------------------------------------
// Filename:	Registry.cpp
// Class:		Registry
// Purpose:		This allows you to open any registry keys you specify. 
//				
//				
//				
//
// Description: There should only ever be one Registry that is shared
//				by everyone.  To this end this class has
//				private constructors and a static member function which
//				gives access to the one and only Registry to all clients.
//
//				
//				Once you call get new key the registry will assume that
//				everytime you call it you will want to use that key
// History:
// -------  
// 17Feb99	Alima			Created. 
//			Alima			default Values  can now be set when we open a
//							 registry key
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../xp.h"

#include "RegistryHandler.h"
#include "../common/C2eDebug.h"
#include "../common/Configurator.h"
#include "../common/FileFuncs.h"
#include "App.h"

Registry::Registry()
:myKey(0)
{
	ChooseDefaultKey("C2e");
}

Registry::~Registry()
{
	if(myKey) 
	{
		RegCloseKey(myKey);
		myKey = NULL;
	}
}

HKEY Registry::CreateKey(HKEY hive /*= HKEY_CURRENT_USER*/)
{
	static char KeyClass[] = "";
	unsigned long opened;
	int err;

	err=RegCreateKeyEx(hive,
				 		myDefaultKeyPath.c_str(),
				 		0,KeyClass,REG_OPTION_NON_VOLATILE,
				 		KEY_ALL_ACCESS,NULL,
				 		&myKey,&opened);
	ASSERT(err==ERROR_SUCCESS);   
	return myKey;
}

HKEY Registry::GetKey(HKEY hive /*= HKEY_CURRENT_USER*/)
{
	int err = RegOpenKeyEx(hive,
				 		myDefaultKeyPath.c_str(),
				 		0,
				 		KEY_READ,
				 		&myKey);

	return (err==ERROR_SUCCESS) ? myKey : 0;
}

bool Registry::CreateValue(const std::string& value,
						   const std::string& data,
							HKEY hive)/* = HKEY_CURRENT_USER */
{
	int err2;
	DWORD dataSize = 200;

	CreateKey(hive);

	if(myKey)
	{
		err2 = RegSetValueEx(myKey,			// handle of key to set
							value.c_str(),	// address of name of value to query
							NULL,			// reserved
							REG_SZ,			// value type string
							(BYTE*)data.c_str(),		// address of data buffer
							data.size() +1 ); // must add null terminator		
		RegCloseKey(myKey);
		myKey = NULL;
	}
	else
		return false;

	if(err2 != ERROR_SUCCESS)
		return false;
	else
		return true;
}

bool Registry::CreateValue(const std::string& value,
								   const int& data,
								   HKEY hive)/* = HKEY_CURRENT_USER */
{
	long err2;
	DWORD dataSize = sizeof(unsigned int);
	CreateKey(hive);

	if(myKey)
	{
		err2 =	RegSetValueEx(myKey,			// handle of key to set
							value.c_str(),	// address of name of value to query
							NULL,			// reserved
							REG_DWORD,			// value type
							(BYTE*)&data,		// address of data buffer
							dataSize );	
	
		RegCloseKey(myKey);
		myKey = NULL;
	}
	else
		return false;

	if(err2 != ERROR_SUCCESS)
		return false;
	else
		return true;
}

bool Registry::CreateValue(const std::string& value,
							const bool& data,
							HKEY hive)
{
	int boolean = data ? 1 : 0;
	return CreateValue(value, boolean, hive);
}

bool Registry::GetValue(const std::string& valueName,
						std::string& data,
						HKEY hive /* = HKEY_CURRENT_USER */,
						bool create /* = true */)
{
	char buf[_MAX_PATH];
	DWORD dataSize = _MAX_PATH;
	DWORD type = REG_SZ;

	int err;
	myKey = GetKey(hive);

	if(myKey)
	{
		err = RegQueryValueEx(myKey,	// handle of key to query
								valueName.c_str(),	// address of name of value to query
								NULL,			// reserved
								&type,		// address of buffer for value type
								(BYTE*)buf,			// address of data buffer
								&dataSize );
		RegCloseKey(myKey);
		myKey = NULL;
	}
	else
		return false;

	if(err != ERROR_SUCCESS)
	{
		if (create)
			CreateValue(valueName, data, hive);
		return false;
	}
	else
	{
		data = buf;
		return true;
	}
}

bool Registry::GetValue(const std::string& valueName,
						int& data,	
						HKEY hive) /* = HKEY_CURRENT_USER */
{
	unsigned int defaultData = data;

	DWORD type;//= REG_DWORD;

	DWORD dataSize = sizeof(unsigned int);
	int err;
	myKey = GetKey(hive);

	if(myKey)
	{
		err = RegQueryValueEx(myKey,	// handle of key to query
								valueName.c_str(),	// address of name of value to query
								NULL,			// reserved
								&type,		// address of buffer for value type
								(BYTE*)&data,			// address of data buffer
								&dataSize );
		RegCloseKey(myKey);
		myKey = NULL;
	}
	else 
		return false;

	if(err != ERROR_SUCCESS)
	{
		CreateValue(valueName, data, hive);

		return false;
	}
	else
		return true;
}

bool Registry::GetValue(const std::string& valueName,
						bool& data,	
						HKEY hive)
{
	int boolean = data ? 1 : 0;
	if (GetValue(valueName, boolean, hive))
	{
		data = (boolean == 0) ? false : true;
		return true;
	}
	else
		return false;
}

std::string Registry::DefaultKey()
{
	return myDefaultKeyPath;
}

void Registry::ChooseDefaultKey(const std::string& gameName)
{
	myDefaultKeyPath = std::string("Software\\") + COMPANY +"\\" + gameName;
}

// --------------------------------------------------------------------------

void Registry::MigrateString(const std::string& key, Registry& registry, HKEY hive, Configurator& dest)
{
	std::string value;
	if (registry.GetValue(key, value, hive))
		dest.Set(key, value);
}

void Registry::MigrateInteger(const std::string& key, Registry& registry, HKEY hive, Configurator& dest)
{
	int value;
	if (registry.GetValue(key, value, hive))
		dest.Set(key, value);
}

void Registry::MigrateBool(const std::string& key, Registry& registry, HKEY hive, Configurator& dest)
{
	bool value;
	if (registry.GetValue(key, value, hive))
		dest.Set(key, value ? 1 : 0);
}

void Registry::UnmigrateDirectory(const std::string& key, Registry& registry, HKEY hive, Configurator& conf)
{
	std::string value;
	value = conf.Get(key);
	if (!value.empty())
	{
		char buf[_MAX_PATH + 1];
		char *filePart;
		GetFullPathName(value.c_str(), _MAX_PATH, buf, &filePart);
		std::string fullPath(buf);
		AddTrailingPathSeparator(fullPath);
		registry.CreateValue(key, fullPath, hive);
	}
}

// This automatically generates .cfg files from the
// registry entries that previous versions of the
// engine used.
void Registry::MigrateFromRegistry()
{
	if (theApp.GetGameName().empty())
	{
		ASSERT(false);
		return;
	}

	Registry registry;
	registry.ChooseDefaultKey(theApp.GetGameName());
	Configurator& machine = theApp.MachineSettings();
	Configurator& user = theApp.UserSettings();

	// Machine settings
	machine.Set("Game Name", theApp.GetGameName());
	MigrateInteger("Display Type", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Main Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Sounds Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Images Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Genetics Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Body Data Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Overlay Data Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Backgrounds Directory", registry, HKEY_LOCAL_MACHINE, machine);
    MigrateString("Catalogue Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Bootstrap Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Worlds Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Exported Creatures Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Resource Files Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Journal Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Creature Database Directory", registry, HKEY_LOCAL_MACHINE, machine);
	MigrateString("Users Directory", registry, HKEY_LOCAL_MACHINE, machine);

	// User settings
	MigrateString("Default Background", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("DiskSpaceCheck", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("DiskSpaceCheckSystem", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("FlightRecorderMask", registry, HKEY_CURRENT_USER, user);
	MigrateBool("FullScreen", registry, HKEY_CURRENT_USER, user);
	MigrateString("Language", registry, HKEY_CURRENT_USER, user);
	MigrateString("LanguageCLibrary", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("WindowBottom", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("WindowLeft", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("WindowRight", registry, HKEY_CURRENT_USER, user);
	MigrateInteger("WindowTop", registry, HKEY_CURRENT_USER, user);

	// Write to disk
	machine.Flush();
	user.Flush();
}

void Registry::MigrateAuxiliaryString(const std::string& key, Registry& registry, HKEY hive, Configurator& dest, const std::string& auxiliaryPrefix)
{
	std::string value;
	if (registry.GetValue(key, value, hive, false))
		dest.Set(auxiliaryPrefix + key, value);
}

// An auxiliary directory is one where the game can load extra
// graphics, sound and so on.  For example Docking Station uses
// Creatures 3 as an auxiliary game.  The auxiliary directories
// are never written to.
void Registry::MigrateAuxiliaryDirectoriesFromRegistry()
{
	// Loop through all specified auxiliary game names
	Configurator& machine = theApp.MachineSettings();
	int i = 0;
	while(true)
	{
		++i;
		std::ostringstream out;
		out << "Win32 Auxiliary Game Name " << i;
		std::string auxiliaryGameName = machine.Get(out.str());
		if (auxiliaryGameName == "")
			break;

		// Make prefix
		std::ostringstream moreOut;
		moreOut << "Auxiliary " << i << " ";
		std::string auxiliaryPrefix = moreOut.str();

		// For each one, extract its registry settings for directories,
		// and add them as auxiliary directories to the config files.

		// The commented out ones are deliberately not migrated, as it
		// isn't useful to do so (for Docking Station, anyway...).  If 
		// you need them, you'll need to change other code as well as
		// they wouldn't currently work with auxiliary directories.
		Registry registry;
		registry.ChooseDefaultKey(auxiliaryGameName);
		// MigrateAuxiliaryString("Main Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Sounds Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Images Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Genetics Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Body Data Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Overlay Data Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Backgrounds Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Catalogue Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Bootstrap Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		// MigrateAuxiliaryString("Worlds Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Exported Creatures Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		MigrateAuxiliaryString("Resource Files Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		// MigrateAuxiliaryString("Journal Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		// MigrateAuxiliaryString("Creature Database Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
		// MigrateAuxiliaryString("Users Directory", registry, HKEY_LOCAL_MACHINE, machine, auxiliaryPrefix);
	}
}

// This writes the game name and all the directories to the registry.
// Tools can look there to find out which game to default to, and
// where catalogue files and other resources are.
void Registry::TransferToRegistryForTools()
{
	if (theApp.GetGameName().empty())
	{
		ASSERT(false);
		return;
	}

	// Store our game as the default one for tools to use
	Registry engineRegistry;
	engineRegistry.ChooseDefaultKey("Creatures Engine");
	engineRegistry.CreateValue("Default Game", theApp.GetGameName());

	// Copy directories over, so tools can find them
	Registry registry;
	registry.ChooseDefaultKey(theApp.GetGameName());
	Configurator& machine = theApp.MachineSettings();

	UnmigrateDirectory("Main Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Sounds Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Images Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Genetics Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Body Data Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Overlay Data Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Backgrounds Directory", registry, HKEY_LOCAL_MACHINE, machine);
    UnmigrateDirectory("Catalogue Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Bootstrap Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Worlds Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Exported Creatures Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Resource Files Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Journal Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Creature Database Directory", registry, HKEY_LOCAL_MACHINE, machine);
	UnmigrateDirectory("Users Directory", registry, HKEY_LOCAL_MACHINE, machine);
}

