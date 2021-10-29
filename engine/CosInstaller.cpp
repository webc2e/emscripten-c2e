// --------------------------------------------------------------------------
// Filename:	CosInstaller.cpp
// Class:		CosInstaller
// Purpose:		This class is a quick way of reading in cos files and
// processing
//				the commands therein.
//
// History:
// -------
// 05Feb98	Alima		Created.
//
//
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "CosInstaller.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <strstream>

#include "App.h"
#include "World.h"
#include "FilePath.h"

#include "Caos/CAOSMachine.h"
#include "Caos/Orderiser.h"
#include "Display/ErrorMessageHandler.h"
#include "../common/FileFuncs.h"
#include "DirectoryManager.h"

// this is the old constructor kept so that
// we can still load any files that are in the
// bootstrap directory and not in subfolders
CosInstaller::CosInstaller() {
  // there is no local world version of the bootstrap folder
  // so no extra searches needed here.
  // get list of files in bootstrap directory
  // if there is a switcher folder (which is always the very first in the list)
  // then load it if not then load whatever is in the base.  This is so as not
  // to disturb older layouts

  std::string path(theDirectoryManager.GetDirectory(BOOTSTRAP_DIR));
  std::string switcher(path);
  //	if(theApp.GetScreenSaverConfig())
  //		switcher += "Configuration" + std::string(PathSeparator());
  //	else
  switcher += "000 Switcher" + std::string(PathSeparator());

  if (FileExists(switcher.c_str()))
    LoadCosFiles(switcher, false);
  else
    LoadCosFiles(path, false);
}

// Read commands from any stream
CosInstaller::CosInstaller(std::istream &in, std::ostream &out) {
  ReadScriptStream(in, false, false, false, &out);
}

// This is the new world switcher style constructor where you tell it
// which bootstrap folders to load.
// the cosinstaller weeds through the list of bootstarp folders
// and installs all new cos files and updates the scripts from
// any other folders as required.
// not that the very first bootstrap folder is the world switcher
CosInstaller::CosInstaller(std::vector<std::string> &bootstrapFoldersToLoad) {
  if (bootstrapFoldersToLoad.empty())
    return;

  std::vector<std::string> allBootStraps;

  for (int i = 0; i < theDirectoryManager.GetAuxDirCount(BOOTSTRAP_DIR); ++i) {
    GetDirsInDirectory(theDirectoryManager.GetAuxDir(BOOTSTRAP_DIR, i),
                       allBootStraps);
  }

  // put them in alphabetical
  std::sort(allBootStraps.begin(), allBootStraps.end());
  // remove duplicates
  allBootStraps.erase(std::unique(allBootStraps.begin(), allBootStraps.end()),
                      allBootStraps.end());

  // delete the bootstrap folder reserved for the world switcher
  allBootStraps.erase(allBootStraps.begin());

  // if the size of the unloaded bootstraps is equal to the
  // number of bootstrap folders found then just load them all in alphabetical
  // order.
  std::vector<std::string>::const_iterator it;
  std::vector<std::string>::iterator itLoadAll;

  int size = bootstrapFoldersToLoad.size();

  SetUpProgressBar(bootstrapFoldersToLoad);

  // here we sort out exactly which folders we need to look at
  // first get the oldest folder that we need to begin our journey with
  // this will be the top of the list of the unloaded list.

  // Get rid of all folders above that.
  itLoadAll = std::find(allBootStraps.begin(), allBootStraps.end(),
                        *(bootstrapFoldersToLoad.begin()));

  // erase up to but not including the first new folder
  allBootStraps.erase(allBootStraps.begin(), itLoadAll);

  // now go through the full list and do this:
  // if the folder appears on the Toload list then we need to load all
  // installion scripts as well as event scripts
  // if the folder does not appear on the ToLoad list then we *only* load the
  // event scripts for that folder
  for (it = allBootStraps.begin(); it != allBootStraps.end(); it++) {
    // the folder is in the to load list so load everything
    itLoadAll = std::find(bootstrapFoldersToLoad.begin(),
                          bootstrapFoldersToLoad.end(), (*it));
    FilePath path(*itLoadAll, BOOTSTRAP_DIR);
    if (itLoadAll != bootstrapFoldersToLoad.end()) {
      LoadCosFiles(path.GetFullPath() + PathSeparator(), true);
    } else // the folder is old so just update the event scripts
    {
      bool dontInjectInstallScript = true;
      LoadCosFiles(path.GetFullPath() + PathSeparator(), true,
                   dontInjectInstallScript);
    }
  }

  allBootStraps.clear();
  theApp.EndProgressBar();
}

CosInstaller::CosInstaller(int32 subbootCheckNumber, int32 folderNumbers) {
  // check through the list of subboot folders
  int32 folderCheckNumber = 1;

  // std::string path(theDirectoryManager.GetDirectory(BOOTSTRAP_DIR));
  std::string subfolderpath;

  char temp[_MAX_PATH];

  sprintf(temp, "Subboot %d", subbootCheckNumber);
  subfolderpath =
      theDirectoryManager.GetDirectory(BOOTSTRAP_DIR) + temp + PathSeparator();

  // find the folder and load all the required bootstrap folders
  while (folderCheckNumber <= folderNumbers) {
    if (folderCheckNumber & folderNumbers) {
      sprintf(temp, "%d", folderCheckNumber);

      LoadCosFiles(subfolderpath + temp + PathSeparator());
    }

    folderCheckNumber <<= 1;
  }
}

void CosInstaller::LoadCosFiles(const std::string &bootstrap_dir,
                                bool showProgress /* = false*/,
                                bool dontInjectInstallScript /*=false*/) {

  std::vector<std::string> files;
  GetFilesInDirectory(bootstrap_dir.c_str(), files);

  // sort into alphabetical order, so we can
  // easily force map.cos to be at the start
  // by calling it !map.cos
  std::sort(files.begin(), files.end());

  // read them all in

  int n = files.size();

  for (int i = 0; i < n; ++i) {
    theApp.UpdateProgressBar();

    // only read in files ending in .cos
    int len = files[i].size();
    if (len >= 4) {
      if (files[i].substr(len - 4, 4) == ".cos")
        ReadScriptFile(std::string(bootstrap_dir) + files[i],
                       dontInjectInstallScript);
    }
  }
}

void CosInstaller::SetUpProgressBar(
    std::vector<std::string> &bootstrapFoldersToLoad) {
  theApp.StartProgressBar(2);

  int n = 0;
  std::vector<std::string> files;

  std::vector<std::string>::const_iterator it;
  for (it = bootstrapFoldersToLoad.begin(); it != bootstrapFoldersToLoad.end();
       it++) {
    FilePath path((*it).c_str(), BOOTSTRAP_DIR);
    GetFilesInDirectory(path.GetFullPath() + PathSeparator(), files);

    // read them all in

    n += files.size();
    files.clear();
  }

  theApp.SpecifyProgressIntervals(n);
}

// read  just one cos file right away
CosInstaller::CosInstaller(std::string &script, bool dontInjectInstallScript,
                           bool injectUninstallScript,
                           bool dontInjectEventScripts, std::ostream *out) {
  if (script != "")
    ReadScriptFile(script, dontInjectInstallScript, injectUninstallScript,
                   dontInjectEventScripts, out);
}

CosInstaller::~CosInstaller() {
  myTextBuffer = "";
  myExecuteScripts.clear();
}

bool CosInstaller::AddScript(const std::string &text, Classifier classifier,
                             std::ostream *finalOut) {
  Orderiser o;
  MacroScript *m;

  // std::cout << classifier.Family() << classifier.Genus() << classifier.Species()
  //           << std::endl;
  // std::cout << text.c_str() << std::endl;
  // get the script compiled
  m = o.OrderFromCAOS(text.c_str());

  if (m) {
    m->SetClassifier(classifier);
    if (theApp.GetWorld().GetScriptorium().InstallScript(m)) {
      // ok
    }
#ifdef _DEBUG
    else {
      std::ostringstream out;
      if (!myCurrentFileForErrorMessages.empty())
        out << myCurrentFileForErrorMessages << std::endl;
      out << theCatalogue.Get("script_error", 1);
      classifier.StreamClassifier(out);
      classifier.StreamAgentNameIfAvailable(out);
      out << theCatalogue.Get("script_error", 2);
      ShowError("script_error", 4, out.str(), finalOut);
    }
#endif
    // Don't delete this, as it is referenced from the scriptorium
    // delete m;
  } else {
    std::ostringstream out;
    if (!myCurrentFileForErrorMessages.empty())
      out << myCurrentFileForErrorMessages << std::endl;
    classifier.StreamClassifier(out);
    classifier.StreamAgentNameIfAvailable(out);
    out << std::endl;
    out << o.GetLastError() << std::endl;
    CAOSMachine::FormatErrorPos(out, o.GetLastErrorPos(), text.c_str());

    ShowError("script_error", 4, out.str(), finalOut);
    return false;
  }

  return true;
}

// when loading in a series of new products we may only want to update the
// scriptorium
bool CosInstaller::ReadScriptFile(std::string const &filename,
                                  bool dontInjectInstallScript,
                                  bool injectUninstallScript,
                                  bool dontInjectEventScripts,
                                  std::ostream *out) {
  std::ifstream in(filename.c_str());
  myCurrentFileForErrorMessages = filename;
  bool OK = ReadScriptStream(in, dontInjectInstallScript, injectUninstallScript,
                             dontInjectEventScripts, out);
  myCurrentFileForErrorMessages = "";
  return OK;
}

// when loading in a series of new products we may only want to update the
// scriptorium
bool CosInstaller::ReadScriptStream(std::istream &in,
                                    bool dontInjectInstallScript,
                                    bool injectUninstallScript,
                                    bool dontInjectEventScripts,
                                    std::ostream *out) {
  char buf[1024];
  buf[0] = 0;
  std::string removeScript;

  GetLineCompatibleMunch(in, buf, 1024);
  do {
    // check what kind of statement we are getting and
    // store it in our array
    if (!strncmp(buf, "scrp", 4)) {
      // create the classifier
      std::istrstream get_class(buf);
      char buf_class[1024];
      get_class >> buf_class; // strip scrp

      int32 family = 0;
      int32 genus = 0;
      int32 species = 0;
      int32 event = 0;

      get_class >> family;
      get_class >> genus;
      get_class >> species;
      get_class >> event;

      // get the text
      while (in.good() && strncmp(buf, "endm", 4)) {
        GetLineCompatibleMunch(in, buf, 1024);

        // don't add endm or comments
        if (strncmp(buf, "endm", 4) && strncmp(buf, "*", 1)) {
          myTextBuffer += buf;
          myTextBuffer += " ";
        }
      }

      myEventScripts.push_back(myTextBuffer);
      myEventScriptsClassifiers.push_back(
          Classifier(family, genus, species, event));
      myTextBuffer = "";
    } else if (!strncmp(buf, "rscr", 4)) {
      // finish on a remove script
      if (injectUninstallScript) {
        // get the text
        while (in.good() && strncmp(buf, "endm", 4)) {
          GetLineCompatibleMunch(in, buf, 1024);

          // don't add endm or comments
          if (strncmp(buf, "endm", 4) && strncmp(buf, "*", 1)) {
            myTextBuffer += buf;
            myTextBuffer += " ";
          }
        }

        removeScript = myTextBuffer;
        myTextBuffer = "";
      }
      break;
    } else if (!strncmp(buf, "iscr", 4)) {
    } else if (!strncmp(buf, "*", 1)) {
    } else {
      if (strncmp(buf, "endm", 4) && strncmp(buf, "*", 1)) {
        myTextBuffer += buf;
        myTextBuffer += " ";
      }

      while (in.good() && strncmp(buf, "endm", 4) && strncmp(buf, "scrp", 4) &&
             strncmp(buf, "rscr", 4)) {
        GetLineCompatibleMunch(in, buf, 1024);
        // don't add endm, scrp or comments
        if (strncmp(buf, "endm", 4) && strncmp(buf, "scrp", 4) &&
            strncmp(buf, "*", 1) && strncmp(buf, "rscr", 4)) {
          myTextBuffer += buf;
          myTextBuffer += " ";
        }
      }

      if (!dontInjectInstallScript) {
        // then add to my Exection scripts
        myExecuteScripts.push_back(myTextBuffer);
      }
      myTextBuffer = "";
    }

    if (strncmp(buf, "scrp", 4) && strncmp(buf, "rscr", 4)) {
      GetLineCompatibleMunch(in, buf, 1024);
    }
  } while (in.good());

  // Execute things in the right order...

  // Remove script
  if (!removeScript.empty()) {
    ExecuteScript(removeScript, out);
    removeScript = "";
  }

  // Event scripts
  if (!dontInjectEventScripts) {
    for (int i = 0; i < myEventScripts.size(); ++i) {
      if (!AddScript(myEventScripts[i], myEventScriptsClassifiers[i], out))
        return false;
    }
  }
  myEventScripts.clear();
  myEventScriptsClassifiers.clear();

  // Install scripts
  if (!ExecuteScripts(out))
    return false;

  return true;
}

bool CosInstaller::ExecuteScript(const std::string &text,
                                 std::ostream *finalOut) {
  Orderiser o;
  MacroScript *m;
  CAOSMachine vm;
  std::ostream *out = NULL;

  m = o.OrderFromCAOS(text.c_str());

  if (m) {
    try {
      CAOSVar from;
      from.SetAgent(NULLHANDLE);
      vm.StartScriptExecuting(m, NULLHANDLE, from, INTEGERZERO, INTEGERZERO);
      vm.SetOutputStream(finalOut);
      vm.UpdateVM(-1);
    } catch (CAOSMachineRunError &e) {
      std::ostringstream out;
      if (!myCurrentFileForErrorMessages.empty())
        out << myCurrentFileForErrorMessages << std::endl;
      out << e.what();
      vm.StreamIPLocationInSource(out);
      out << std::endl;

      ShowError("script_error", 4, out.str(), finalOut);

      // clean up after the error
      vm.StopScriptExecuting();

      return false;
    } catch (std::exception &e) {
      std::ostringstream out;
      if (!myCurrentFileForErrorMessages.empty())
        out << myCurrentFileForErrorMessages << std::endl;
      out << e.what();
      vm.StreamIPLocationInSource(out);
      out << std::endl;

      ShowError("script_error", 4, out.str(), finalOut);

      // clean up after the error
      vm.StopScriptExecuting();

      return false;
    }

    // finished with this script now.
    delete m;
  } else {
    std::ostringstream out;
    if (!myCurrentFileForErrorMessages.empty())
      out << myCurrentFileForErrorMessages << std::endl;
    out << o.GetLastError() << std::endl;
    CAOSMachine::FormatErrorPos(out, o.GetLastErrorPos(), text.c_str());

    ShowError("script_error", 4, out.str(), finalOut);

    return false;
  }

  return true;
}

bool CosInstaller::ExecuteScripts(std::ostream *finalOut) {
  std::vector<std::string>::iterator it;
  for (it = myExecuteScripts.begin(); it != myExecuteScripts.end(); it++) {
    if (!ExecuteScript(*it, finalOut))
      return false;
  }

  // now clear the scripts
  myExecuteScripts.clear();

  return true;
}

void CosInstaller::ShowError(const std::string &tag, int offset,
                             const std::string &report, std::ostream *out) {
  if (out)
    *out << report;
  else
    ErrorMessageHandler::Show(tag.c_str(), offset, "CosInstaller::ShowError",
                              report.c_str());
}
