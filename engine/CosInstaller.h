// --------------------------------------------------------------------------
// Filename:	CosInstaller.h
// Class:		CosInstaller
// Purpose:		To install scripts into the scriptorium
//				To create new objects from a cos file
//			
//				
//
// Description: The cosinstaller can still load cos scripts for the bootstrap
//				directory but Note that the new world switcher style requires
//				one folder per product or update to appear in the bootstrap 
//				directory.  The products must be named alphabetically 
//				corresponding to their release dates.  Eg 0100 C3, 0200 C3 Life Kit
//			
//
// History:
// -------  
// 16Mar98	Alima		Created
// --------------------------------------------------------------------------
#ifndef COS_INSTALLER_H
#define COS_INSTALLER_H
#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

// --------------------------------------------------------------------------
// Filename:	CosInstaller.h
// Class:		CosInstaller
// Purpose:		This class loads installation and event scripts.
//			
//				
//
//			
//
// History:
// -------  Alima		created
// 26Jun99	Alima		Altered to work with the world switcher directory structure.
//						  The cos installer now needs to load all scripts
//						for each new product detected.
// --------------------------------------------------------------------------

#include <string>
#include <vector>
#include <iostream>
#include "Classifier.h"

class CosInstaller
{
public:

	CosInstaller();
	CosInstaller(std::string& script, bool dontInjectInstallScript = false, bool injectUninstallScript = false, bool dontInjectEventScripts = false, std::ostream* out = NULL);
	CosInstaller(std::vector<std::string>& bootStrapFoldersToLoad);
	CosInstaller(std::istream& in, std::ostream& out);
	CosInstaller(int32 subbootNumbers,
							int32 folderNumbers);
	~CosInstaller();

	void LoadCosFiles(const std::string& bootstrap_dir,bool showProgress= false,
		bool dontInjectInstallScript = false );


	void SetUpProgressBar(std::vector<std::string>& bootstrapFoldersToLoad);

	bool AddScript(const std::string& text, Classifier classifier, std::ostream* finalOut);
	bool ReadScriptFile(std::string const& filename, bool dontInjectInstallScript = false, bool injectUninstallScript = false, bool dontInjectEventScripts = false, std::ostream* out = NULL);
	bool ReadScriptStream(std::istream& in, bool dontInjectInstallScript, bool injectUninstallScript, bool dontInjectEventScripts, std::ostream* out);
	bool ExecuteScripts(std::ostream* out);
	bool ExecuteScript(const std::string& text, std::ostream* finalOut);

protected:
	void ShowError(const std::string& tag, int offset, const std::string& report, std::ostream* out);

private:
	std::string myTextBuffer;
	std::vector<std::string> myExecuteScripts;
	std::vector<std::string> myEventScripts;
	std::vector<Classifier> myEventScriptsClassifiers;


	std::string myCurrentFileForErrorMessages;
};
#endif //COS_INSTALLER_H

