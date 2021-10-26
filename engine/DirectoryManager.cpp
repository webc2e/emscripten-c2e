#include "DirectoryManager.h"
#ifdef _WIN32
#include "RegistryHandler.h"
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "../common/FileFuncs.h"
#include "App.h"
#include "Display/ErrorMessageHandler.h"
#include "World.h"

#include "emscripten.h"

DirectoryManager DirectoryManager::ourDirectoryManager;
// ****************************************************************************
bool DirectoryManager::ReadFromConfigurationFiles() {
  myMainAuxiliary = 0;
  theApp.MachineSettings().Get("Main Auxiliary", myMainAuxiliary);

  EM_ASM({ console.log('I received: ' + $0); }, 100);
  EM_ASM(
      {
        FS.mkdir("/mydocs");
        console.log($0);
      },
      theApp.MyDocuments);

  // SPARKY
  // Added for Windows XP!
  // Why? Because simple file sharing means that non-admin users can't create
  // worlds Best I get the My Documents location!
  // NOTE: need something to show the difference between DS and C3!
  strcat((char *)&theApp.MyDocuments, "/Creatures/");
  CreateDirectory((char *)&theApp.MyDocuments, 0);
  strcat((char *)&theApp.MyDocuments,
         (theApp.MachineSettings().Get("Game Name")).c_str());
  strcat((char *)&theApp.MyDocuments, "/");
  CreateDirectory((char *)&theApp.MyDocuments, 0);

  // MessageBox(NULL,(theApp.MachineSettings().Get("Game
  // Name")).c_str(),"Hello",MB_OK);

  const int special[] = {
      3, //"Main Directory",
      3, //"Sounds Directory",
      3, //"Images Directory",
      3, //"Genetics Directory",
      3, //"Body Data Directory",
      3, //"Overlay Data Directory",
      3, //"Backgrounds Directory",
      3, //"Catalogue Directory",
      0, //"Bootstrap Directory",
      1, //"Worlds Directory",
      1, //"Exported Creatures Directory",
      2, //"Resource Files Directory",			### My Agents
      1, //"Journal Directory",
      1, //"Creature Database Directory",
      1, //"Users Directory",
  };

  const char *keys[] = {
      "Main Directory",
      "Sounds Directory",
      "Images Directory",
      "Genetics Directory",
      "Body Data Directory",
      "Overlay Data Directory",
      "Backgrounds Directory",
      "Catalogue Directory",
      "Bootstrap Directory",
      "Worlds Directory",
      "Exported Creatures Directory",
      "Resource Files Directory",
      "Journal Directory",
      "Creature Database Directory",
      "Users Directory",
  };

  const char *defaults[] = {
      "",
      "Sounds",
      "Images",
      "Genetics",
      "Body Data",
      "Overlay Data",
      "Backgrounds",
      "Catalogue",
      "Bootstrap",
      "My Worlds",
      "My Creatures",
      "My Agents",
      "Journal",
      "Creature Galleries",
      "Users",
  };

  myDirectories.clear();
  myDirectories.resize(NUM_DIRS);
  myDirectoriesCU.clear();
  myDirectoriesCU.resize(NUM_DIRS);

  for (int dir = 0; dir < NUM_DIRS; dir++) {
    int aux = -1;
    while (true) {
      ++aux;
      std::ostringstream out;
      out << "Auxiliary " << aux << " ";
      std::string auxiliaryPrefix = out.str();
      if (aux == 0)
        auxiliaryPrefix = "";

      std::string buffer;
      std::string bufferCU;
      std::string key((char *)(keys[dir]));
      key = auxiliaryPrefix + key;
      std::string defaultdir((char *)(defaults[dir]));

      if (special[dir] == 1) { // We create the path based on CSIDL_PERSONAL
        if (aux > 0)
          break;

        buffer = (char *)&theApp.MyDocuments + defaultdir;
        bufferCU = (char *)&theApp.MyDocuments + defaultdir;
        // AddTrailingPathSeparator(buffer);
        CreateDirectory(buffer.c_str(), NULL);
      } else {

        theApp.MachineSettings().Get(key, buffer);
        if (buffer.empty()) {
          // if we were looking for an auxiliary directory, then stop
          if (aux > 0)
            break;

          // if there's no default directory (i.e., for the Main directory) then
          // complain
          if (defaultdir.empty()) {
            ErrorMessageHandler::NonLocalisable(
                "NLE0001: '%s' setting not found.",
                std::string("DirectoryManager::ReadFromConfigurationFiles"),
                key.c_str());
            return false;
          }

          // set the default
          // This depends on whether the directory type is a fixed or user
          // writeable
          ASSERT(dir > MAIN_DIR);
          ASSERT(!myDirectories[MAIN_DIR][0].empty());

          buffer = myDirectories[MAIN_DIR][0] + defaultdir;
          AddTrailingPathSeparator(buffer);
          CreateDirectory(buffer.c_str(), NULL);
        }
      }

      // if there's no trailing backslash add one:
      AddTrailingPathSeparator(buffer);

      if (!FileExists(buffer.c_str())) {
        // if we were looking for an auxiliary directory, then forget it
        // (the product referred to may not be installed properly)
        if (aux > 0)
          break;

        ErrorMessageHandler::NonLocalisable(
            "NLE0002: The following directory doesn't exist:\n%s"
            "\nMake sure the configuration file entry is correct!",
            std::string("DirectoryManager::ReadFromConfigurationFiles"),
            buffer.c_str());
        return false;
      }

      if (dir != MAIN_DIR) {
        // If we've got a very short relative path, then add ./ to
        // the start.  This is because the GetWorldDirectoryVersion
        // function expects the end part to be separated by two slashes,
        // for conversion to a world directory version.
        size_t end = buffer.find_last_of("//");
        size_t start = buffer.find_last_of("//", end - 1);
        if (end == -1 || start == -1)
          buffer = "." + std::string(PathSeparator()) + buffer;
      }

      // For type 3 directories we need to create the directory
      // *and* create a different entry in the CU list
      if (special[dir] == 3) {
        bufferCU = (char *)&theApp.MyDocuments + defaultdir + "/";
        CreateDirectory(bufferCU.c_str(), NULL);
      } else {
        bufferCU = buffer; // buffer;
      }

      myDirectories[dir].resize(aux + 1);
      myDirectories[dir][aux] = buffer;
      myDirectoriesCU[dir].resize(aux + 1);
      myDirectoriesCU[dir][aux] = bufferCU;
    }
  }

  return true;
}
// ****************************************************************************
// get the world directory path of the given folder
// e.g. ../Worlds/Myworld/Images
bool DirectoryManager::GetWorldDirectoryVersion(
    int dir, std::string &path, bool createFilePlease /*=false*/,
    std::string worldName) {

  path = GetDirectory(dir);
  // get the words between the last two backslashes
  size_t end = path.find_last_of("//");
  size_t start = path.find_last_of("//", end - 1);
  if (end == -1 || start == -1) {
    // no main directory version!
    return false;
  }

  // hurrah now we have the name of the directory!
  std::string name = path.substr(start + 1, end - start - 1);

  return GetWorldDirectoryVersion(name, path, createFilePlease, worldName);
}
// ****************************************************************************
// for attic files, we separate out this part of the function
bool DirectoryManager::GetWorldDirectoryVersion(const std::string &name,
                                                std::string &path,
                                                bool createFilePlease,
                                                std::string worldName) {
  bool dirmissing;

  // now get the worlds directory
  path = GetDirectory(WORLDS_DIR);

  if (worldName.empty())
    worldName = theApp.GetWorldName();

  if (worldName.empty()) {
    path += name + PathSeparator();
    return false;
  } else {
    path += worldName + PathSeparator() + name + PathSeparator();
  }

  dirmissing = !FileExists(path.c_str());
  if (dirmissing) {
    if (createFilePlease) {
      return CreateDirectory(path.c_str(), NULL) ? true : false;
    } else // just report that there was no directory at all
    {
      return false;
    }
  }

  return true;
}
// ****************************************************************************
DirectoryManager &DirectoryManager::GetDirectoryManager() {
  return ourDirectoryManager;
}
// ****************************************************************************
int DirectoryManager::GetAuxDirCount(int dir) {
  return myDirectories[dir].size();
}
// ****************************************************************************
std::string DirectoryManager::GetAuxDir(int dir, int index, bool useCU) {
  if (dir >= myDirectories.size()) {
    std::cerr << "Tried to read directory " << dir << " when we only have "
              << myDirectories.size() << std::endl;
    return "";
  }
  if (index >= myDirectories[dir].size()) {
    std::cerr << "Tried to read auxiliary " << index << " when we only have "
              << myDirectories[dir].size() << " for directory " << dir
              << std::endl;
    return "";
  }

  if (useCU) {
    return myDirectoriesCU[dir][index];
  } else {
    return myDirectories[dir][index];
  }
}
// ****************************************************************************
std::string DirectoryManager::GetDirectory(int dir, bool useCU) {
  return GetAuxDir(dir, myMainAuxiliary, useCU);
}
// ****************************************************************************
std::string DirectoryManager::GetDirectory(int dir) {
  return GetAuxDir(dir, myMainAuxiliary, false);
}
// ****************************************************************************
