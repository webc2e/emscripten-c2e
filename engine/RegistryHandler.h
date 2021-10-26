// Windows registry class.  This is a legacy from before
// the Configurator was used.  It should not be used
// for any new code.

#ifndef REGISTRY_HANDLER_H
#define REGISTRY_HANDLER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Configurator;

class Registry
{
public:
	virtual ~Registry();
	Registry();

	bool CreateValue(const std::string& value, const int& data, HKEY hive = HKEY_CURRENT_USER);
	bool CreateValue(const std::string& value, const std::string& data, HKEY hive = HKEY_CURRENT_USER);
	bool CreateValue(const std::string& value, const bool& data, HKEY hive = HKEY_CURRENT_USER);

	bool GetValue(const std::string& valueName, int& data, HKEY hive = HKEY_CURRENT_USER);
	bool GetValue(const std::string& valueName, std::string& data, HKEY hive = HKEY_CURRENT_USER, bool create = true);
	bool GetValue(const std::string& valueName, bool& data, HKEY hive = HKEY_CURRENT_USER);
	
	void ChooseDefaultKey(const std::string& gameName);
	std::string DefaultKey();

	static void MigrateFromRegistry();
	static void TransferToRegistryForTools();
	static void MigrateAuxiliaryDirectoriesFromRegistry();

private:		
	static void MigrateString(const std::string& key, Registry& registry, HKEY hive, Configurator& dest);
	static void MigrateInteger(const std::string& key, Registry& registry, HKEY hive, Configurator& dest);
	static void MigrateBool(const std::string& key, Registry& registry, HKEY hive, Configurator& dest);

	static void MigrateAuxiliaryString(const std::string& key, Registry& registry, HKEY hive, Configurator& dest, const std::string& auxiliaryPrefix);

	static void UnmigrateDirectory(const std::string& key, Registry& registry, HKEY hive, Configurator& conf);

	HKEY CreateKey(HKEY hive = HKEY_CURRENT_USER);
	HKEY GetKey(HKEY hive = HKEY_CURRENT_USER);

	HKEY myKey;
	std::string myDefaultKeyPath;

};

#endif // REGISTRY_HANDLER_H

