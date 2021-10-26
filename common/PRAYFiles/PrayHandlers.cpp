// --------------------------------------------------------------------------------------
// Filename:	PrayHandlers.cpp
// Class:		PrayHandlers
// Purpose:		Centralised CAOS routines for PRAY commands
// Description:
//  The PrayHandlers class provides the CAOS machine with the PRAY commands, and
//  rValues
//
// History:
//  24Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "PrayHandlers.h"

#include "PrayBuilder.h"
#include "PrayChunk.h"
#include "PrayException.h"
#include "PrayManager.h"
#include "StringIntGroup.h"

#include "../../engine/AgentManager.h"
#include "../../engine/App.h"
#include "../../engine/Caos/CAOSMachine.h"
#include "../../engine/Caos/CAOSVar.h"
#include "../../engine/CosInstaller.h"
#include "../../engine/Creature/Creature.h"
#include "../../engine/Creature/ReproductiveFaculty.h"
#include "../../engine/Creature/SkeletalCreature.h"
#include "../../engine/DirectoryManager.h"
#include "../../engine/File.h"
#include "../../engine/FilePath.h"
#include "../../engine/General.h"
#include "../../engine/ModuleImporter.h"
#include "../../engine/World.h"

#include "../FileFuncs.h"
#include "../StringFuncs.h"

#ifdef C2D_DIRECT_DISPLAY_LIB

#else
#include "../../engine/Display/DisplayEngine.h"
#endif

#include "../../engine/Caos/GeneralHandlers.h"
#include "../BasicException.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "../FileFuncs.h"

#include <iostream>
#include <string>
#include <strstream>
#include <vector>

#include <sstream>

// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_TEST(CAOSMachine &vm) {
  std::string name;
  vm.FetchStringRV(name);
  try {
    return thePrayManager.CheckChunk(name);
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  } catch (...) {
    vm.ThrowRunError(0);
  }
  return -1;
}
// ****************************************************************************
void PrayHandlers::Command_PRAY(CAOSMachine &vm) {
  static CommandHandler subcmds[] = {SubCommand_PRAY_REFR,
                                     SubCommand_PRAY_GARB};
  int thiscmd;
  thiscmd = vm.FetchOp();
  (subcmds[thiscmd])(vm);
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY(CAOSMachine &vm) {
  static IntegerRVHandler subhandlers[] = {
      IntegerRV_PRAY_COUN, IntegerRV_PRAY_AGTI, IntegerRV_PRAY_DEPS,
      IntegerRV_PRAY_FILE, IntegerRV_PRAY_TEST, IntegerRV_PRAY_INJT,
      IntegerRV_PRAY_EXPO, IntegerRV_PRAY_IMPO, IntegerRV_PRAY_MAKE,
      IntegerRV_PRAY_KILL,
  };
  int thisrv;
  thisrv = vm.FetchOp();
  return (subhandlers[thisrv])(vm);
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_MAKE(CAOSMachine &vm) {
  int WhichJournal = vm.FetchIntegerRV();
  std::string journalName, prayName;
  vm.FetchStringRV(journalName);
  int WhichPray = vm.FetchIntegerRV();
  vm.FetchStringRV(prayName);
  CAOSVar &report = vm.FetchVariable();

  // Safety first
  MakeFilenameSafe(journalName);
  MakeFilenameSafe(prayName);

  std::string aDir, journalDir;
  if (WhichJournal == 0)
    theDirectoryManager.GetWorldDirectoryVersion(JOURNAL_DIR, journalDir,
                                                 false);
  else
    journalDir = theDirectoryManager.GetDirectory(JOURNAL_DIR);

  if (journalDir.empty())
    vm.ThrowRunError(CAOSMachine::sidPrayBuilderError,
                     "Journal Folder not found");

  journalName = journalDir + journalName;
  if (WhichPray == 0)
    aDir = theDirectoryManager.GetDirectory(PRAYFILE_DIR);
  else
    aDir = theDirectoryManager.GetDirectory(CREATURES_DIR);

  if (aDir.empty())
    vm.ThrowRunError(CAOSMachine::sidPrayBuilderError,
                     "Specified Pray Folder not found");
  prayName = aDir + prayName;

  bool ok = MakeHelper(journalDir, journalName, prayName, report);

  return ok ? 0 : 1;
}
// ****************************************************************************
bool PrayHandlers::MakeHelper(std::string aDir, std::string journalName,
                              std::string prayName, CAOSVar &report) {
  // add the current directory onto the potentially relative paths
  // that the DirectoryManager returns,
#ifdef _WIN32
  char buf[_MAX_PATH + 1];
  char *filePart;
  GetFullPathName(journalName.c_str(), _MAX_PATH, buf, &filePart);
  journalName = buf;
  GetFullPathName(prayName.c_str(), _MAX_PATH, buf, &filePart);
  prayName = buf;
#else
  char buf[1024];
  getcwd(buf, 1024); // SPARKY was getcwd
  if (journalName[0] != '/')
    journalName = buf + std::string("/") + journalName;
  if (prayName[0] != '/')
    prayName = buf + std::string("/") + prayName;
#endif

  // preserve current directory, and change to one where build happens
  char folderBuffer[1024];
  getcwd(folderBuffer, 1024);
  chdir(aDir.c_str()); // SPARKY was chdir

  // perform the build
  PrayBuilder pb(journalName, prayName);
  report.SetString(pb.Output());

  // restore
  chdir(folderBuffer);

  return pb.Successful();
}
// ****************************************************************************
void PrayHandlers::StringRV_PRAY(CAOSMachine &vm, std::string &str) {
  static StringRVHandler substrings[] = {
      StringRV_PRAY_PREV, StringRV_PRAY_NEXT, StringRV_PRAY_AGTS,
      StringRV_PRAY_BACK, StringRV_PRAY_FORE,
  };
  int thisst;
  thisst = vm.FetchOp();
  (substrings[thisst])(vm, str);
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_INJT(CAOSMachine &vm) {
  std::string name;
  vm.FetchStringRV(name);
  int doInjt = vm.FetchIntegerRV();
  CAOSVar &report = vm.FetchVariable();

  if (DependencyHelper(vm, name, doInjt)) {
    report.SetString("Failed Dependency Scan");
    return -3; // Failed Dependency scan :(
  }

  try {
    // 1. Get Chunk from Manager...
    PrayChunkPtr chunk(thePrayManager.GetChunk(name));
    // 2. With the chunk, now construct an input stream around it
    std::istrstream ist((char *)chunk->GetData(), chunk->GetSize());
    // 3. Now we have the stream, construct the StringIntGroup around it...
    StringIntGroup vals(ist);
    // 4. Now we have the group object, let's bingle on through the systems
    std::string scr;
    int count;
    if (!vals.FindInt("Agent Type", count))
      return -1;
    if (!vals.FindInt("Script Count", count))
      return -2;
    int loop;
    for (loop = 0; loop < count; loop++) {
      std::ostringstream st;
      st << "Script " << (loop + 1);
      if (!vals.FindString(st.str(), scr)) {
        // Script "n" has failed... let's report it...
        report.SetString(st.str());
        return -1; // -1 == failed to find script in report var
      }
    }
    // Right then, we know that the scripts work, so let's play "Installey"
    if (!doInjt)
      return 0;
    for (loop = 0; loop < count; loop++) {
      std::ostringstream st;
      st << "Script " << (loop + 1);
      try {
        std::string ignorableCAOS = "";
        CosInstaller c(ignorableCAOS);
        std::string scrpt;
        vals.FindString(st.str(), scrpt);
        std::istringstream scrp(scrpt);
        if (!c.ReadScriptStream(scrp, false, false, false, &st)) {
          report.SetString(st.str());
          return -2; // -2 == failed to install script in report var
        }

      } catch (...) {
        report.SetString(st.str());
        return -2; // -2 == failed to install script in report var.
      }
    }
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  } catch (...) {
    vm.ThrowRunError(0);
  }
  // Woohoo - success!!!
  return 0; // 0 == success :)
}
// ****************************************************************************
void PrayHandlers::SubCommand_PRAY_REFR(CAOSMachine &vm) {
  try {
    thePrayManager.RescanFolders();
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  } catch (...) {
    vm.ThrowRunError(0);
  }
}
// ****************************************************************************
void PrayHandlers::SubCommand_PRAY_GARB(CAOSMachine &vm) {
  bool force = (vm.FetchIntegerRV() != 0);
  thePrayManager.GarbageCollect(force);
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_FILE(CAOSMachine &vm) {
  // The purpose of this command is to install a file.
  std::string filename;
  vm.FetchStringRV(filename);
  int where = vm.FetchIntegerRV();
  bool doIt = vm.FetchIntegerRV() != 0;
  // Simply this...
  return InstallFile(filename, ConvertCategoryToAppId(where), doIt) ? 0 : 1;
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_COUN(CAOSMachine &vm) {
  std::string type;
  vm.FetchStringRV(type);
  std::vector<std::string> temp;
  thePrayManager.GetChunks(type, temp);
  return temp.size();
}
// ****************************************************************************
int PrayHandlers::ConvertCategoryToAppId(int cat) {

  switch (cat) {
  case 0:
    return -1; // Denied :) return MAIN_DIR;
  case 1:
    return SOUNDS_DIR;
  case 2:
    return IMAGES_DIR;
  case 3:
    return GENETICS_DIR;
  case 4:
    return BODY_DATA_DIR;
  case 5:
    return OVERLAYS_DIR;
  case 6:
    return BACKGROUNDS_DIR;
  case 7:
    return CATALOGUE_DIR;
  case 8:
    return -1; // Denied :) return BOOTSTRAP_DIR;
  case 9:
    return -1; // Denied :) return WORLDS_DIR;
  case 10:
    return CREATURES_DIR;
  case 11:
    return PRAYFILE_DIR;
  default:
    return -1;
  }
  return -1;
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_AGTI(CAOSMachine &vm) {
  std::string name;
  vm.FetchStringRV(name);
  std::string tag;
  vm.FetchStringRV(tag);
  int def = vm.FetchIntegerRV();
  try {
    // 1. Get Chunk from Manager...
    PrayChunkPtr chunk(thePrayManager.GetChunk(name));
    // 2. With the chunk, now construct an input stream around it
    std::istrstream ist((char *)chunk->GetData(), chunk->GetSize());
    // 3. Now we have the stream, construct the StringIntGroup around it...
    StringIntGroup vals(ist);
    // 4. Now we have the group object, simply fetch the int...
    vals.FindInt(tag, def);
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  } catch (...) {
    vm.ThrowRunError(0);
  }

  return def;
}

// ****************************************************************************
bool PrayHandlers::InstallFile(std::string filename, int category,
                               bool doInstall) {

#ifdef NETTY
  theApp.Netty->TextOut("##### PRAY", filename.c_str());
#endif

  //	MessageBox(NULL,filename.c_str(),"Here it is",MB_OK);

  if (category < 0)
    return false;

  // Stop lots of nasty pray tricks to write out files
  MakeFilenameSafe(filename);

  // Determine source of the file...
  int src = 0;

  // Could we install the file if need be ?

  if (thePrayManager.CheckChunk(filename)) {
    src = 1;
  }

  FilePath filepath(filename, category);
  filepath.LowerCase();
  std::string mainified = filepath.GetFullPath();
  if (FileExists(mainified.c_str()))
    src += 4;

  // Now then, we know where the best/known place is for the file...

  if (!doInstall)
    return src != 0;

  // Hmm, we are installing the file, let's see first of all if we have it.

  if (src > 1)
    return true; // No need to install the file, it's there already

  // Okay, we need to install the file... we install it by writing it to
  // "Worldified"

  if (src == 0) {
    // Erkle, the file is not installed, and not available, let's fail :(
    return false;
  }

  PrayChunkPtr theFile(thePrayManager.GetChunk(filename));

  FILE *f;

  // char test[1024];
  // strcpy((char*)&test,mainified.c_str());
  //  SPARKY
  //	MessageBox(NULL,mainified.c_str(),"Here it is",MB_OK);
  // theApp.MyDocuments
  if ((f = fopen(mainified.c_str(), "wb")) == NULL) {
    // Erk - we have failed to install the file - exit silently :(
    return false; // I.E. Fail on resource
  }

  // Okay the file was opened.....

  if (fwrite(theFile->GetData(), 1, theFile->GetSize(), f) !=
      theFile->GetSize()) {
    // Aah - serious problem here - failed to write the file :(
    fclose(f);
    DeleteFile(mainified.c_str());
    return false;
  }

  // Okay, we wrote the file out, close it
  fclose(f);

  // Files are being installed, and may be creature body parts, so
  // we need to clear our cache which is used for those.
  ClearFspCache();

  // If we are installing a catalogue, then reset the catalogue entries
  if (category == CATALOGUE_DIR) {
    theApp.InitLocalisation();
  }

  // try silently automatically loading hed files if they are present
  if (category == IMAGES_DIR) {
    // if this is a bmp file then swap the extension for a .hed
    std::string headerFile = filename;
    LowerCase(headerFile);

    int x = headerFile.find_last_of(".");

    if (x != -1) {
      std::string ext = headerFile.substr(x + 1, 3);

      std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

      if (ext == "bmp") {
        headerFile = headerFile.substr(0, x + 1);
        headerFile += "hed";

        // first check that the files doesn't already exist on disk
        std::string mainified = theDirectoryManager.GetDirectory(category);
        mainified.append(headerFile);

        if (!FileExists(mainified.c_str())) {
          if (thePrayManager.CheckChunk(headerFile)) {
            PrayChunkPtr theFile(thePrayManager.GetChunk(headerFile));

            FILE *f;
            if ((f = fopen(mainified.c_str(), "wb")) != NULL) {
              if (fwrite(theFile->GetData(), 1, theFile->GetSize(), f) !=
                  theFile->GetSize()) {
                // Aah - serious problem here - failed to write the file :(
                fclose(f);
                DeleteFile(mainified.c_str());
              }
            }

            fclose(f);
          }
        }
      } // ext == bmp
    }
  }

  if ((category == IMAGES_DIR) || (category == BACKGROUNDS_DIR) ||
      (category == OVERLAYS_DIR)) {
    // Convert it...
    std::string theImageFileName = mainified;
    std::string tempFileName = mainified;
    std::string ext = theImageFileName.substr(theImageFileName.length() - 3);
    if (ext == "s16" || ext == "S16")
      tempFileName += ".tmp.s16";
    else if (ext == "c16" || ext == "C16")
      tempFileName += ".tmp.c16";
    else if (ext == "blk" || ext == "BLK" || ext == "Blk" || ext == "bLk" ||
             ext == "blK" || ext == "BLk" || ext == "BlK" || ext == "bLK" ||
             ext == "BLK")
      tempFileName += ".tmp.blk";
    else
      tempFileName = "";
    if (tempFileName.empty() == false) {
#ifdef C2D_DIRECT_DISPLAY_LIB
      _ASSERT(false);
#else
      DisplayEngine::theRenderer().SafeImageConvert(
          theImageFileName, tempFileName,
          DisplayEngine::theRenderer().GetMyPixelFormat(), tempFileName);
      ::DeleteFile(tempFileName.c_str());
#endif
    }
  }
  // And say "Wahey :)"
  return true;
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_DEPS(CAOSMachine &vm) {
  // We would do the deps here :)
  std::string name;
  vm.FetchStringRV(name);
  int doinst = vm.FetchIntegerRV();
  return DependencyHelper(vm, name, doinst);
}
// ****************************************************************************
int PrayHandlers::DependencyHelper(CAOSMachine &vm, std::string &name,
                                   int doinst) {
  try {
    // 1. Get Chunk from Manager...
    PrayChunkPtr chunk(thePrayManager.GetChunk(name));
    // 2. With the chunk, now construct an input stream around it
    std::istrstream ist((char *)chunk->GetData(), chunk->GetSize());
    // 3. Now we have the stream, construct the StringIntGroup around it...
    StringIntGroup vals(ist);

    // Our group object is now ready for use.. First of all, check for the C3
    // Agent Details...

    std::string dep;
    int count, cat;
    if (!vals.FindInt("Agent Type", count))
      return -1;
    if (!vals.FindInt("Dependency Count", count))
      return -2;
    int loop;
    for (loop = 0; loop < count; loop++) {
      std::ostringstream st, cst;
      st << "Dependency " << (loop + 1);
      cst << "Dependency Category " << (loop + 1);
      if (!vals.FindString(st.str(), dep))
        return -(loop + 3);
      if (!vals.FindInt(cst.str(), cat))
        return -((loop + 3) + count + count);
      cat = ConvertCategoryToAppId(cat);
      if (cat == -1)
        return ((loop + 1) + count + count);
    }

    // Okay then - the agent is correctly formed enough for us to do a
    // dependency evaluate on...
    for (loop = 0; loop < count; loop++) {
      std::ostringstream st, cst;
      st << "Dependency " << (loop + 1);
      cst << "Dependency Category " << (loop + 1);
      vals.FindString(st.str(), dep);
      vals.FindInt(cst.str(), cat);
      // We transform the cat (file category) into an App directory id...
      cat = ConvertCategoryToAppId(cat);
      if (cat == -1)
        return ((loop + 1) + count + count);
      // Okay then, we have a valid file in a valid place...
      // Try to install (or check to install) the file
      if (!InstallFile(dep, cat, doinst != 0))
        // Okay then, we can't find the file anywhere sensible, so fail on this
        // dep...
        return loop + 1;
    }
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  } catch (...) {
    vm.ThrowRunError(0);
  }

  return 0;
}
// ****************************************************************************
inline std::vector<std::string>::iterator
helper_find(std::vector<std::string> &vec, std::string &what) {
  std::vector<std::string>::iterator it = vec.begin();
  while (it != vec.end()) {
    if ((*it) == what)
      return it;
    it++;
  }
  return it;
}
// ****************************************************************************
void PrayHandlers::StringRV_PRAY_PREV(CAOSMachine &vm, std::string &str) {
  std::string type, name;
  vm.FetchStringRV(type);
  vm.FetchStringRV(name);
  std::vector<std::string> chunks;
  thePrayManager.GetChunks(type, chunks);
  if (chunks.size() == 0) {
    str = "";
    return;
  }
  std::vector<std::string>::iterator it = helper_find(chunks, name);
  if (it == chunks.begin())
    it = chunks.end();
  str = *(--it);
}
// ****************************************************************************
void PrayHandlers::StringRV_PRAY_NEXT(CAOSMachine &vm, std::string &str) {
  std::string type, name;
  vm.FetchStringRV(type);
  vm.FetchStringRV(name);
  std::vector<std::string> chunks;
  thePrayManager.GetChunks(type, chunks);
  if (chunks.size() == 0) {
    str = "";
    return;
  }
  std::vector<std::string>::iterator it = helper_find(chunks, name);
  if (it != chunks.end())
    it++;
  if (it == chunks.end())
    it = chunks.begin();
  str = *it;
}
// ****************************************************************************
void PrayHandlers::StringRV_PRAY_AGTS(CAOSMachine &vm, std::string &str) {
  std::string name;
  vm.FetchStringRV(name);
  std::string tag;
  vm.FetchStringRV(tag);
  vm.FetchStringRV(str);
  try {
    // 1. Get Chunk from Manager...
    PrayChunkPtr chunk(thePrayManager.GetChunk(name));
    // 2. With the chunk, now construct an input stream around it
    std::istrstream ist((char *)chunk->GetData(), chunk->GetSize());
    // 3. Now we have the stream, construct the StringIntGroup around it...
    StringIntGroup vals(ist);
    // 4. Now we have the group object, simply fetch the string...
    vals.FindString(tag, str);
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  } catch (...) {
    vm.ThrowRunError(0);
  }
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_IMPO(CAOSMachine &vm) {
  std::string moniker_chunk;
  vm.FetchStringRV(moniker_chunk);

  bool doStuff = (vm.FetchIntegerRV() == 1);
  bool porching = (vm.FetchIntegerRV() != 1);

  AgentHandle handle;
  int ret = ImportHelper(moniker_chunk, handle, doStuff, porching);
  vm.SetTarg(handle);

  // Stop scripts, for security.  The running script gets
  // exported and sent over the network, and would carry
  // on, allowing people to run arbitary code by sending a creature.
  // No more Midas :-(
  if (handle.IsValid())
    handle.GetAgentReference().StopAllScripts();

  // A file has gone, so we make sure nothing tries to read from it
  thePrayManager.RescanFolders();

  return ret;
}
// ****************************************************************************
int PrayHandlers::ImportHelper(std::string moniker_chunk,
                               AgentHandle &newHandle, bool doStuff,
                               bool porching) {
  // These variables are used in the exception handler, so
  // have to be declared outside the try
  AgentHandle handle;
  std::string creatureFile;

  try {
    // Test if we are a warping import, or an ordinary import
    creatureFile =
        thePrayManager.GetChunkParentFile(moniker_chunk + ".creature");
    bool warp = World::WarpExtension(creatureFile);

    if (thePrayManager.CheckChunk(moniker_chunk + ".creature") == 0)
      return 2; // That Moniker is not in the PRAY mappings

    std::string chunkType = thePrayManager.GetChunkType(moniker_chunk);
    if (chunkType.empty())
      return 2; // That Moniker is not in the PRAY mappings
    std::string safetyNameAddition = WorkOutSafetyNameAddition(chunkType);

    // First read in list of stored genetic files...
    std::vector<std::string> geneNames;
    const PrayChunkPtr &geneListChunk =
        thePrayManager.GetChunk(moniker_chunk + ".glist.creature");
    std::istrstream geneStream((char *)geneListChunk->GetData(),
                               geneListChunk->GetSize());
    CreaturesArchive geneListArchive(geneStream, CreaturesArchive::Load);
    geneListArchive >> geneNames;

    // Reconcile history for each genome, and check that gene chunks are there
    int monIndex;
    bool successReconcile = true;
    std::vector<CreatureHistory> histories(geneNames.size());
    for (monIndex = 0; monIndex < geneNames.size(); ++monIndex) {
      geneListArchive >> histories[monIndex];
      successReconcile =
          successReconcile &&
          theApp.GetWorld().GetHistoryStore().ReconcileImportedHistory(
              geneNames[monIndex], histories[monIndex], false);

      // Test if this moniker_chunk is in the PRAY system.
      if (thePrayManager.CheckChunk(geneNames[monIndex] + safetyNameAddition +
                                    ".genetics") == 0)
        throw BasicException("One or more genetics files required are missing "
                             "from the PRAY maps.");
    }

    // If the forced clone variable is set, and we're not a warp creature,
    // then force the cloning.
    if (!warp &&
        theApp.GetEameVar("engine_clone_upon_import").GetInteger() == 1)
      successReconcile = false;

    if (!doStuff)
      return successReconcile ? 0 : 1;

    // Now restore those genetic files...
    {
      std::vector<std::string>::const_iterator monIter;
      for (monIter = geneNames.begin(), monIndex = 0;
           monIter != geneNames.end(); ++monIter, ++monIndex) {
        {
          const PrayChunkPtr &gC = thePrayManager.GetChunk(
              *monIter + safetyNameAddition + ".genetics");
          std::string geneticsName = GenomeStore::Filename(*monIter);
          File f;
          f.Create(geneticsName);
          f.Write(gC->GetData(), gC->GetSize());
          f.Close();

          FilePath filepath(*monIter + ".gen", GENETICS_DIR);
          theApp.GetWorld().MarkFileCreated(filepath);
        }

        if (successReconcile) // We already know this will work.
          theApp.GetWorld().GetHistoryStore().ReconcileImportedHistory(
              geneNames[monIndex], histories[monIndex], true);
      }
    }

    // Get that Chunk :)
    // Load in the actual binary serialised creature
    const PrayChunkPtr &creatureChunk =
        thePrayManager.GetChunk(moniker_chunk + ".creature");
    std::istrstream stream((char *)creatureChunk->GetData(),
                           creatureChunk->GetSize());
    CreaturesArchive archive(stream, CreaturesArchive::Load);
    archive.SetAgentArchiveStyle(CreaturesArchive::ONE_AGENT_ONLY_NULL_OTHERS);
    archive.SetCloningASkeletalCreature(true);
    archive >> handle;

    // See if we need to clone everything in the genome store
    if (!successReconcile) {
      // we do, so clone it
      GenomeStore &theStore = handle.GetCreatureReference().GetGenomeStore();
      theStore.ClonedEntirely();

      HistoryStore &historyStore = theApp.GetWorld().GetHistoryStore();
      // copy over gender and variant from our imported file.
      // this could be different from the history in the world, if
      // we are importing a hatched version of a Norn that is still
      // in its egg in the world, for example
      for (int i = 0; i < theStore.GetSlotCount(); i++) {
        if (!theStore.MonikerAsString(i).empty()) {
          // delete the temporary gene file if not referenced
          if (theAgentManager.FindAgentForMoniker(geneNames[i]).IsInvalid()) {
            FilePath filepath(geneNames[i] + ".gen", GENETICS_DIR);
            theApp.GetWorld().MarkFileForAttic(filepath);
          }

          // copy over gender/variant
          CreatureHistory &historyAfter =
              historyStore.GetCreatureHistory(theStore.MonikerAsString(i));
          historyAfter.myGender = histories[i].myGender;
          historyAfter.myVariant = histories[i].myVariant;
          // could copy name over here as well
        }
      }
    }

    // Load in the photos
    // (but only if we didn't clone everything.  If we cloned everything,
    // then the photos aren't relevant any more)
    std::string moniker = geneNames[0];
    if (successReconcile) {
      ASSERT(geneNames.size() > 0);
      ASSERT((moniker_chunk == moniker + "." + chunkType) ||
             ((chunkType == "EXPC" || chunkType == "SFAM") &&
              moniker_chunk == moniker));
      CreatureHistory &history =
          theApp.GetWorld().GetHistoryStore().GetCreatureHistory(moniker);
      int count = history.Count();
      for (int historyIndex = 0; historyIndex < count; ++historyIndex) {
        std::string photo = history.GetLifeEvent(historyIndex)->myPhoto;
        if (!photo.empty()) {
          const PrayChunkPtr &gC =
              thePrayManager.GetChunk(photo + safetyNameAddition + ".photo");
          File f;
          FilePath filepath(photo + ".s16", IMAGES_DIR, true, true);

          // force creation of path
          std::string dummy;
          filepath.GetWorldDirectoryVersionOfTheFile(dummy, true);

          f.Create(filepath.GetFullPath());
          f.Write(gC->GetData(), gC->GetSize());
          f.Close();

          // convert to correct 555/565 format
          std::string theImageFileName = filepath.GetFullPath();
          std::string tempFileName = theImageFileName;
          std::string ext =
              theImageFileName.substr(theImageFileName.length() - 3);
          if (ext == "s16" || ext == "S16")
            tempFileName += ".tmp.s16";
          else if (ext == "c16" || ext == "C16")
            tempFileName += ".tmp.c16";
          else if (ext == "blk" || ext == "BLK" || ext == "Blk" ||
                   ext == "bLk" || ext == "blK" || ext == "BLk" ||
                   ext == "BlK" || ext == "bLK" || ext == "BLK")
            tempFileName += ".tmp.blk";
          else
            tempFileName = "";
          if (tempFileName.empty() == false) {
#ifdef C2D_DIRECT_DISPLAY_LIB
            _ASSERT(false);
#else
            DisplayEngine::theRenderer().SafeImageConvert(
                theImageFileName, tempFileName,
                DisplayEngine::theRenderer().GetMyPixelFormat(), tempFileName);
            ::DeleteFile(tempFileName.c_str());
#endif
          }

          theApp.GetWorld().MarkFileCreated(filepath);
        }
      }
    }

    // Add us to the agent manager
    theAgentManager.RegisterClone(handle);
    newHandle = handle;

    if (handle.IsSkeletalCreature())
      handle.GetSkeletalCreatureReference().RemakeSkeletonAfterImporting();

    // Import events have happened
    if (successReconcile) {
      for (std::vector<std::string>::const_iterator monIter = geneNames.begin();
           monIter != geneNames.end(); ++monIter) {
        theApp.GetWorld()
            .GetHistoryStore()
            .GetCreatureHistory(*monIter)
            .AddEvent(
                (warp ? LifeEvent::typeWarpedIn : LifeEvent::typeImported), "",
                "");
      }
    }

    // Move the file to the porch (unless no porching, e.g. for starter family)
    // We don't porch warp creatures, as their filenames aren't unique for the
    // same creature, so they don't get unporched properly.
    if (porching && !warp)
      theApp.GetWorld().MoveFileToPorch(creatureFile);
    else if (warp)
      DeleteFile(creatureFile.c_str());

    return successReconcile ? 0 : 1;
  } catch (BasicException &be) {
    thePrayManager.IntegrityViolation(creatureFile, be.what());
    if (handle.IsValid())
      theAgentManager.KillAgent(handle);
    newHandle = NULLHANDLE;
    return 4;
  } catch (PrayException &pe) {
    thePrayManager.IntegrityViolation(creatureFile, pe.GetMessage());
    if (handle.IsValid())
      theAgentManager.KillAgent(handle);
    newHandle = NULLHANDLE;
    return 4;
  }
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_EXPO(CAOSMachine &vm) {
  std::string chunkType;
  vm.FetchStringRV(chunkType);

  // Check we're looking at a valid creature
  Creature &creature = vm.GetCreatureTarg();

  // Get creature's name
  std::string moniker = creature.GetMoniker();
  CreatureHistory &cHist =
      theApp.GetWorld().GetHistoryStore().GetCreatureHistory(moniker);
  std::string name = cHist.myName;

  // Generate filename from name and moniker
  std::string filename = name;
  if (!name.empty())
    filename += "_";
  std::string extraExtensionPart =
      theCatalogue.TagPresent("export_extra_extension")
          ? theCatalogue.Get("export_extra_extension", 0)
          : "";
  filename += moniker + extraExtensionPart + ".creature";
  for (int tempLoop = 0; tempLoop < filename.length(); tempLoop++) {
    if (filename.at(tempLoop) == '-')
      filename.at(tempLoop) = '_';
  }
  MakeFilenameSafe(filename);

  // It's a bit bad that this code has to worry about this, but
  // you have to kill agents differently if they are the virtual
  // machine currently running
  bool selfExporting = (vm.GetTarg() == vm.GetOwner());

  int ret = -1;
  try {
    ret =
        ExportHelper(creature, chunkType,
                     theDirectoryManager.GetDirectory(CREATURES_DIR) + filename,
                     selfExporting, false);
  } catch (PrayException &pe) {
    vm.ThrowRunError(BasicException(pe.GetMessage().c_str()));
  }

  // There's a new file, so we make sure it's accessible
  thePrayManager.RescanFolders();

  return ret;
}
// ****************************************************************************
std::string
PrayHandlers::WorkOutSafetyNameAddition(const std::string &chunkType) {
  // Work out what string to add onto each chunk name, to
  // ensure the creatures of a different type are distinct - i.e.
  // creatures in the warp directory aren't confused with those in
  // My Creatures
  if (chunkType == "EXPC" || chunkType == "SFAM") {
    // For backwards compatibility, we add no extension
    // for these two types. They aren't ever confused,
    // because the CAOS forces all imported starter families
    // to be cloned.
    // In future, not using SFAM is sensible.
    return "";
  } else {
    return "." + chunkType;
  }
}
// ****************************************************************************
// throws PrayException, static
int PrayHandlers::ExportHelper(Creature &creature, std::string chunkType,
                               std::string filename, bool selfExporting,
                               bool warping) {
  std::string moniker = creature.GetMoniker();

  CreatureHistory &cHist =
      theApp.GetWorld().GetHistoryStore().GetCreatureHistory(moniker);
  if (warping)
    cHist.myWarpHoleVeteran = true;

  // Make the resource type a valid one
  if (chunkType.empty())
    chunkType = "EXPC";
  chunkType = chunkType + chunkType + chunkType + chunkType;
  chunkType = chunkType.substr(0, 4);

  std::string safetyNameAddition = WorkOutSafetyNameAddition(chunkType);

  // Store the (moniker) names of the genetic files to be stored as a seperate
  // chunk
  GenomeStore &theStore = creature.GetGenomeStore();
  std::vector<std::string> geneNames;
  for (int i = 0; i < theStore.GetSlotCount(); i++) {
    std::string interestingMoniker = theStore.MonikerAsString(i);
    if (!interestingMoniker.empty()) {
      if (thePrayManager.CheckChunk(interestingMoniker + safetyNameAddition +
                                    ".genetics") != 0)
        return 1; // already on disk in some form

      geneNames.push_back(interestingMoniker);
    }
  }
  std::ostringstream geneStream;
  {
    CreaturesArchive geneArchive(geneStream, CreaturesArchive::Save);
    // put the gene file names in the archive
    geneArchive << geneNames;

    // and their history data as well
    for (std::vector<std::string>::const_iterator monIter = geneNames.begin();
         monIter != geneNames.end(); ++monIter) {
      // before we export their history, add the appropriate life event
      CreatureHistory &history =
          theApp.GetWorld().GetHistoryStore().GetCreatureHistory(*monIter);
      if (warping)
        history.AddEvent(LifeEvent::typeWarpedOut, "", "", false);
      else
        history.AddEvent(LifeEvent::typeExported, "", "", false);

      // Ensures the correct user ID is filled in the life events, if
      // we can't send the history because we are offline.  Particularly
      // crucial is that the export life event gets the right network user.
      NetworkInterface *networkInterface =
          ModuleImporter::GetNetworkInterface();
      if (networkInterface) {
        // Before warping, ensure network user ID is correctly
        // filled in, so tracking data is accurate.
        if (warping)
          if (!networkInterface->HistoryFillInUserID(
                  theApp.GetWorld().GetHistoryStore(), *monIter))
            return 2; // don't know user ID for NET: EXPO
      }

      // Write to archive
      geneArchive << history;
    }
  }
  thePrayManager.AddChunkToFile(
      moniker + safetyNameAddition + ".glist.creature", "GLST", filename,
      geneStream.str().size(), (uint8 *)geneStream.str().data(), true);

  // Make Sure Creature is not connected to others (carried etc.)
  creature.BreakLinksToOtherAgents();

  // Now store the creature in an archive, and write chunk out...
  std::ostringstream stream;
  {
    CreaturesArchive archive(stream, CreaturesArchive::Save);
    archive.SetAgentArchiveStyle(CreaturesArchive::ONE_AGENT_ONLY_NULL_OTHERS);
    archive << AgentHandle(creature);
  }
  thePrayManager.AddChunkToFile(moniker + safetyNameAddition + ".creature",
                                "CREA", filename, stream.str().size(),
                                (uint8 *)stream.str().data(), true);

  // Store the genetics files in chunks and write them out
  for (std::vector<std::string>::const_iterator monIter = geneNames.begin();
       monIter != geneNames.end(); ++monIter) {

    std::string fileName = GenomeStore::Filename(*monIter);
    File f;
    f.Open(fileName);
    uint8 *geneticData = new uint8[f.GetSize()];
    f.Read(geneticData, f.GetSize());
    try {
      thePrayManager.AddChunkToFile(*monIter + safetyNameAddition + ".genetics",
                                    "GENE", filename, f.GetSize(), geneticData,
                                    true);
    } catch (PrayException &pe) {
      delete[] geneticData;
      f.Close();
      throw(pe);
    }
    delete[] geneticData;
    f.Close();
  }

  // Write out the photos to their own chunks
  CreatureHistory &history =
      theApp.GetWorld().GetHistoryStore().GetCreatureHistory(moniker);
  int count = history.Count();
  for (int historyIndex = 0; historyIndex < count; ++historyIndex) {
    std::string photo = history.GetLifeEvent(historyIndex)->myPhoto;
    if (!photo.empty()) {
      File f;
      f.Open(FilePath(photo + ".s16", IMAGES_DIR).GetFullPath());
      uint8 *data = new uint8[f.GetSize()];
      f.Read(data, f.GetSize());
      try {
        thePrayManager.AddChunkToFile(photo + safetyNameAddition + ".photo",
                                      "PHOT", filename, f.GetSize(), data,
                                      true);
      } catch (PrayException &pe) {
        delete[] data;
        f.Close();
        throw(pe);
      }
      delete[] data;
      f.Close();
    }
  }

  // Write meta-deta as the main chunk
  StringIntGroup information;
  std::string str;
  creature.GetBodyPartFileName(0, str);
  information.Clear();

  ASSERT(cHist.Count() > 0);

  // NOTE: These meta-data are documented in PRAY EXPO's help
  // - please document any new ones there!

  // From members of LifeEvent.h
  information.AddInt("Exported At World Time",
                     cHist.GetLifeEvent(cHist.Count() - 1)->myWorldTick);
  information.AddInt("Creature Age In Ticks",
                     cHist.GetLifeEvent(cHist.Count() - 1)->myAgeInTicks);
  information.AddInt("Exported At Real Time",
                     cHist.GetLifeEvent(cHist.Count() - 1)->myRealWorldTime);
  information.AddInt("Creature Life Stage",
                     cHist.GetLifeEvent(cHist.Count() - 1)->myLifeStage);

  information.AddString("Exported From World Name",
                        cHist.GetLifeEvent(cHist.Count() - 1)->myWorldName);
  information.AddString(
      "Exported From World UID",
      cHist.GetLifeEvent(cHist.Count() - 1)->myWorldUniqueIdentifier);

  information.AddString("Native Network User",
                        cHist.GetLifeEvent(0)->myNetworkUser);
  information.AddString("Last Network User",
                        cHist.GetLifeEvent(cHist.Count() - 1)->myNetworkUser);

  // From members of CreatureHistory.h
  information.AddString("Creature Name", cHist.myName);
  information.AddInt("Gender", cHist.myGender);
  information.AddInt("Genus", cHist.myGenus + 1);
  information.AddInt("Variant", cHist.myVariant);

  // Other
  information.AddString("Head Gallery", str);
  information.AddInt("Pregnancy Status",
                     creature.Reproductive()->IsPregnant() ? 1 : 0);

  // Custom meta-data - all NAME variables on the creature
  // that begin with "Pray Extra " are added as meta data
  {
    // Enumerate through all name variables
    CAOSVar name;
    std::string str;
    name.SetString("");
    do {
      creature.GetNextNameVar(name);
      if (name.GetType() == CAOSVar::typeString)
        name.GetString(str);
      else
        str = "z"; // not an empty string

      // Look for string variables with the appropriate
      // starting tag
      if (str.size() > 11) {
        if (str.substr(0, 11) == "Pray Extra ") {
          // Extract the content and add it in
          CAOSVar &content = creature.GetNameVar(name);
          if (content.GetType() == CAOSVar::typeString) {
            std::string contentString;
            content.GetString(contentString);
            information.AddString(str, contentString);
          } else if (content.GetType() == CAOSVar::typeInteger)
            information.AddInt(str, content.GetInteger());
        }
      }
    } while (!str.empty());
  }

  std::ostringstream sigstream;
  information.SaveToStream(sigstream);
  thePrayManager.AddChunkToFile(moniker + safetyNameAddition, chunkType,
                                filename, sigstream.str().size(),
                                (uint8 *)sigstream.str().data(), true);

  // Kill off the creature in the game
  // (Don't tell anyone in the community, but whenever a Norn
  // is exported it is actually cloned and the original brutally
  // murdered.)
  if (selfExporting)
    creature.YouAreNowDoomed();
  else
    theAgentManager.KillAgent(AgentHandle(creature));

  // Hey, something's happened to you all!
  {
    for (std::vector<std::string>::const_iterator monIter = geneNames.begin();
         monIter != geneNames.end(); ++monIter) {
      CreatureHistory &history =
          theApp.GetWorld().GetHistoryStore().GetCreatureHistory(*monIter);
      CAOSVar p1, p2;
      p1.SetString(*monIter);
      p2.SetInteger(history.Count() - 1);
      theAgentManager.ExecuteScriptOnAllAgentsDeferred(
          SCRIPT_LIFE_EVENT, COASVARAGENTNULL, p1, p2);
    }
  }

  return 0;
}
// ****************************************************************************
int PrayHandlers::IntegerRV_PRAY_KILL(CAOSMachine &vm) {
  std::string chunk;
  vm.FetchStringRV(chunk);

  if (thePrayManager.CheckChunk(chunk) == 0)
    return 0;

  // Delete the file
  std::string creatureFile = thePrayManager.GetChunkParentFile(chunk).c_str();
  DeleteFile(creatureFile.c_str());

  // Erase our record of it
  thePrayManager.RescanFolders();

  return 1;
}
// ****************************************************************************
// These different from PREV and NEXT in that they don't loop, but return
// an empty string at the end/beginning
void PrayHandlers::StringRV_PRAY_BACK(CAOSMachine &vm, std::string &str) {
  std::string type, name;
  vm.FetchStringRV(type);
  vm.FetchStringRV(name);
  std::vector<std::string> chunks;
  thePrayManager.GetChunks(type, chunks);
  if (chunks.size() == 0) {
    str = "";
    return;
  }
  std::vector<std::string>::iterator it = helper_find(chunks, name);
  if (it == chunks.begin())
    str = "";
  else
    str = *(--it);
}
// ****************************************************************************
void PrayHandlers::StringRV_PRAY_FORE(CAOSMachine &vm, std::string &str) {
  std::string type, name;
  vm.FetchStringRV(type);
  vm.FetchStringRV(name);
  std::vector<std::string> chunks;
  thePrayManager.GetChunks(type, chunks);
  if (chunks.size() == 0) {
    str = "";
    return;
  }
  std::vector<std::string>::iterator it = helper_find(chunks, name);
  if (it == chunks.end())
    it = chunks.begin();
  else
    it++;
  if (it == chunks.end())
    str = "";
  else
    str = *it;
}
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
