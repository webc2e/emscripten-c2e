// -------------------------------------------------------------------------
// Filename:    DirectoryManager.h
// Class:       DirectoryManager
// Purpose:
// To separate out the directory commands from the app.
//
// Description:
//	Retrieves directory paths from the registry
//
//
//
// History:
// 10Mar00	Initial version		Alima
// -------------------------------------------------------------------------

#ifndef DIRECTORY_MANAGER_H
#define DIRECTORY_MANAGER_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include <string>
#include <vector>

enum
{
    MAIN_DIR =	0,
    SOUNDS_DIR,
    IMAGES_DIR,
    GENETICS_DIR,
    BODY_DATA_DIR,
	OVERLAYS_DIR,
    BACKGROUNDS_DIR,
    CATALOGUE_DIR,
	BOOTSTRAP_DIR,
	WORLDS_DIR,
	CREATURES_DIR,
	PRAYFILE_DIR,
	JOURNAL_DIR,
	CREATURE_DATABASE_DIR,
	USERS_DIR,
	NUM_DIRS
};

#include "../modules/ModuleAPI.h"

#define theDirectoryManager DirectoryManager::GetDirectoryManager()

class C2E_MODULE_API DirectoryManager
{
public:
	static DirectoryManager& GetDirectoryManager();

	bool ReadFromConfigurationFiles();

	std::string GetDirectory( int dir);
	std::string GetDirectory( int dir,bool useCU);
	int GetAuxDirCount(int dir);
	std::string GetAuxDir( int dir, int index,bool useCU=false);

	// get the world directory path of the given folder
	// e.g. ../Worlds/Myworld/Images
	bool GetWorldDirectoryVersion(int dir, std::string& path, bool createFilePlease =false, std::string worldName = "");

	// or a named one that isn't in the main world (attic etc.)
	bool GetWorldDirectoryVersion(const std::string& name, std::string& path, bool createFilePlease, std::string worldName = "");

private:
#ifdef _MSC_VER
#pragma warning (disable : 4251)
#endif
		static DirectoryManager ourDirectoryManager;
		std::vector< std::vector<std::string> > myDirectories;
		std::vector< std::vector<std::string> > myDirectoriesCU;
		int myMainAuxiliary;
#ifdef _MSC_VER
#pragma warning (default : 4251)
#endif

};

#endif //DIRECTORY_MANAGER_H

