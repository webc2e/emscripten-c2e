// World.cpp
//

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "App.h"
#include "Message.h"
#include "World.h"
#include "CreaturesArchive.h"
#include "Agents/Agent.h"
#include "Agents/PointerAgent.h"
#include "Creature/Creature.h"
#include "Camera/MainCamera.h"
#include "AgentManager.h"
#include "Map/Map.h"
#ifdef _WIN32
#include "RegistryHandler.h"
#endif
#include "CosInstaller.h"
#include "Display/SharedGallery.h"
#include "Display/ErrorMessageHandler.h"

#include "Sound/MusicManager.h"

#include "../common/FileFuncs.h"
#include "ModuleImporter.h"
#include "DirectoryManager.h"

#include "UniqueIdentifier.h"

#include <fstream>
#include <algorithm>

AgentHandle thePointer;

TintManager &World::GetTintManager(int index) {
  if (index > myTints.size() - 1)
    myTints.resize(index + 1);
  return myTints[index];
}

void World::SetTintManager(int index, uint8 red, uint8 green, uint8 blue,
                           uint8 rot, uint8 swap) {
  int currentSize = myTints.size() - 1;
  if (index > currentSize)
    myTints.resize(index + 1);
  myTints[index].BuildTintTable(red, green, blue, rot, swap);
}

World::World() {
  initialisedSanityCheck = false;
  myCurrentlyLoadingWorld = false;
}

void World::Init() {
  ASSERT(initialisedSanityCheck == false);
  myCurrentlyLoadingWorld = false;

  myEventTimer = 0;
  initialisedSanityCheck = true;
  loadOnlyOnceSanityCheck = false;
  myCurrentMetaRoom = 0;
  myName = "";

  myNeedToBackUp = false;

  mySelectedCreature = NULLHANDLE;

  // creatures adventures settings
  // macro commands should reset these
  myDayLengthInMinutes = 20;

  // Get day and season lengths
  mySeasonLengthInDays = 3;

  mySeasonCount = 4;

  myYearLengthInDays = mySeasonCount * mySeasonLengthInDays;

  myWorldTick = 0;
  myPausedWorldTick = false;

  theMainView.GetPixelFormat(myLastPixelFormat);

  GetGameVar("engine_debug_keys").SetInteger(1);
  GetGameVar("engine_full_screen_toggle").SetInteger(1);
  GetGameVar("engine_dumb_creatures").SetInteger(0);
  GetGameVar("engine_zlib_compression").SetInteger(6);

  // twin and triplet probabilities
  {
    GetGameVar("engine_multiple_birth_first_chance").SetFloat(0.0f);
    GetGameVar("engine_multiple_birth_subsequent_chance").SetFloat(0.0f);
    GetGameVar("engine_multiple_birth_maximum").SetInteger(1);
    GetGameVar("engine_multiple_birth_identical_chance").SetFloat(0.5f);
  }

  GetGameVar("engine_synchronous_learning").SetInteger(0);

  myUniqueIdentifier = theCatalogue.Get("World Friendly Name", 0) +
                       std::string("-") + GenerateUniqueIdentifier("", "");

  // NOTE: This also appears in App:SetupMainView()!
  //  - BenC 03Feb00
  std::string value("Default Background");
  std::string data("GreatOutdoors");
  theApp.UserSettings().Get(value, data);

  GetGameVar("engine_distance_before_port_line_warns").SetFloat(600.0f);
  GetGameVar("engine_distance_before_port_line_snaps").SetFloat(800.0f);

  int mid, rid;
  bool ok;

  ok = myMap.SetMapDimensions(8000, 8000);
  ok = myMap.AddMetaRoom(0, 0, 800, 600, data, mid);

  ok = myMap.AddRoom(0, 0, 200, 0, 0, 100, 300, rid);
  ok = myMap.AddRoom(0, 200, 400, 0, 0, 300, 200, rid);
  ok = myMap.AddRoom(0, 0, 200, 100, 300, 500, 500, rid);
  ok = myMap.AddRoom(0, 200, 400, 300, 200, 500, 500, rid);

  myMap.SetDoorPermiability(1, 3, 0);

  ok = myMap.AddBackground(0, data);
  ok = myMap.SetCurrentBackground(0, data);
  ok = myMap.SetCurrentMetaRoom(0);
  ok = myMap.SetGridSize(200);

  thePointer = theAgentManager.CreatePointer();

  myCurrentMetaRoom = myMap.GetCurrentMetaRoom();
}

World::~World() {
  // tell all modules
  ModuleImporter::ResetIterator();
  while (ModuleInterface *moduleInterface = ModuleImporter::NextModule()) {
    moduleInterface->WorldEnding();
  }

  theAgentManager.KillAllAgents();
}

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool World::Write(CreaturesArchive &ar) const {
  theApp.StartProgressBar(8);

  // after the version number (when it goes in) always

  // write the password - this must be first so the world switcher commands can
  // find it!
  ar << myPassword;

  // was music event - only here for backwards compatibility
  int32 temp = 0;
  ar << temp;

  // moved this so that it can be reffered to while creating agents
  ar << myTints;
  if (!myScriptorium.Write(ar))
    return false;
  if (!myMap.Write(ar))
    return false;
  ar << mySeasonCount;
  ar << mySeasonLengthInDays;
  ar << myDayLengthInMinutes;
  ar << myYearLengthInDays;
  ar << myWorldTick;
  ar << myPausedWorldTick;

  ar << mySelectedCreature;

  ar << myGameVars;

  ar << myLoadedBootstrapFolders;

  ar << myMessageQueue;

  ar << myLastPixelFormat;

  ar << myFilesForAtticDelayed;
  ar << myFilesForAtticNextTime;
  ar << myFilesJustCreated;
  ar << myFilesInThePorch;

  ar << myUniqueIdentifier;

  ar << myHistoryStore;

  theAgentManager.Write(ar);
  theMainView.Write(ar);

  // Modules - write out their names and data
  ModuleImporter::ResetIterator();
  while (ModuleInterface *moduleInterface = ModuleImporter::NextModule()) {
    if (moduleInterface->Persistent()) {
      std::string moduleName;
      moduleInterface->ModuleName(moduleName);
      ar << moduleName;
      moduleInterface->Write(ar);
    }
  }
  // mark end of modules
  ar << std::string("");

  theApp.EndProgressBar();

  return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool World::Read(CreaturesArchive &ar, int fileSize) {
  myCurrentlyLoadingWorld = true;

  theAgentManager.KillAllAgents();
  GetMap().Reset();

  int32 version = ar.GetFileVersion();

  if (version >= 3) {
    theApp.StartProgressBar(2);
    theApp.SpecifyProgressIntervals(fileSize);

    // always read the the version (when it goes in) and then the password
    ar >> myPassword;

    // was music event - only here for backwards compatibility
    int32 temp;
    ar >> temp;

    // moved this to the top so that agents can refer to it during creation.
    if (version >= 24) {
      ar >> myTints;
    }

    myScriptorium.Clear();
    if (!myScriptorium.Read(ar)) {
      myCurrentlyLoadingWorld = false;
      return false;
    }
    if (!myMap.Read(ar)) {
      myCurrentlyLoadingWorld = false;
      return false;
    }

    ar >> mySeasonCount;
    ar >> mySeasonLengthInDays;
    ar >> myDayLengthInMinutes;
    ar >> myYearLengthInDays;
    ar >> myWorldTick;
    ar >> myPausedWorldTick;

    ar >> mySelectedCreature;

    // old unwanted creatures adventures crap
    if (version <= 29) {
      AgentHandle stupidBirthdayAgent;
      AgentHandle dumbCountDownClock;

      ar >> stupidBirthdayAgent;
      ar >> dumbCountDownClock;
    }

    ar >> myGameVars;

    ar >> myLoadedBootstrapFolders;

    // more old unwanted....
    if (version <= 29) {
      SYSTEMTIME wasGameEndTimeAndPlayLengh;

      ar >> wasGameEndTimeAndPlayLengh;
      ar >> wasGameEndTimeAndPlayLengh;
    }

    ar >> myMessageQueue;

    ar >> myLastPixelFormat;

    ar >> myFilesForAtticDelayed;
    ar >> myFilesForAtticNextTime;
    ar >> myFilesJustCreated;
    ar >> myFilesInThePorch;

    ar >> myUniqueIdentifier;

    ar >> myHistoryStore;

    if (!theAgentManager.Read(ar)) {
      theApp.EndProgressBar();
      myCurrentlyLoadingWorld = false;
      return false;
    }

    if (!theMainView.Read(ar)) {
      theApp.EndProgressBar();
      myCurrentlyLoadingWorld = false;
      return false;
    }

    // moved this to the top so that agents can refer to it during creation.
    // in later versions.
    if (version < 24) {
      ar >> myTints;
    }

    if (ar.GetFileVersion() >= 17) {
      // see what modules saved to the archive file
      std::string moduleName;
      ar >> moduleName;
      while (!moduleName.empty()) {
        ModuleImporter::ResetIterator();
        ModuleInterface *moduleInterface = NULL;
        while (moduleInterface = ModuleImporter::NextModule()) {
          std::string searchModuleName;
          moduleInterface->ModuleName(searchModuleName);
          if (searchModuleName == moduleName)
            break;
        }
        if (moduleInterface) {
          // we've found the module
          moduleInterface->Read(ar);
        } else {
          throw BasicException(ErrorMessageHandler::Format("module_error", 1,
                                                           "World::Read",
                                                           moduleName.c_str())
                                   .c_str());
        }

        ar >> moduleName;
      }
    }

    theApp.EndProgressBar();
  } else {
    _ASSERT(false);
    return false;
  }

  // success!
  // only for newly created worlds never the world switcher
  if (myName != "Startup") {
    CheckForNewNewlyInstalledProductsOrAddOns();
  }

  int meta = myMap.GetCurrentMetaRoom();
  std::string back;
  myMap.GetCurrentBackground(meta, back);
  Position pos = theMainView.GetWorldPosition();

  SetMetaRoom(meta, 0, pos.GetX(), pos.GetY(), false);

  myCurrentlyLoadingWorld = false;
  return true;
}

///////////////////////// Task-switcher /////////////////////////
// Most of these fns are not class members - it's simpler!

// Timer tick has occurred. Do a new batch of tasks
void World::TaskSwitcher() {
  ASSERT(initialisedSanityCheck);

  // First, do all things that MUST happen every tick.
  theAgentManager.UpdateAllAgents();

  // Handle messages
  TaskHandleMessages();

  // to do look at this for timing music issues
  // use system tick, as music carries on even when game paused
  if (theApp.GetSystemTick() % 20 == 19)
    TaskMusic();

  // Update ticks for this world
  if (!myPausedWorldTick) {
    myWorldTick++;
    if ((myWorldTick % 2) == 0)
      myMap.UpdateCurrentCAProperty();

    // Update any active sounds
    if (theSoundManager)
      theSoundManager->Update();
  }

  // if in the likely event someone plays the game for enough hours
  // then reset the game tick.  This only happens after 6 years of
  // solid playing with one world, so we won't worry about it too
  // much.
  if (myWorldTick == 0xFFFFFFFF)
    myWorldTick = 0;

  // if there are missing .mng files then the
  // music engine will not be there but it shouldn't
  // stop the other stuff carrying on...
  if (theMusicManager && theMusicSoundManager) {
    theMusicSoundManager->Update();
    theMusicManager->Update();
  }

  // Upload history to module interface (for tracking
  // of Creature, essentially for Docking Station, or
  // perhaps you could log it to a local file or database
  // for another reason)
  NetworkInterface *networkInterface = ModuleImporter::GetNetworkInterface();
  if (networkInterface)
    networkInterface->HistoryFeed(GetHistoryStore());
}

// Handle any pending messages & stimuli
void World::TaskHandleMessages() {
  Message Msg;
  AgentHandle agent;

  while (ReadMessage(Msg)) // get ptr to msg
  {
    if ((agent = Msg.GetTo()).IsValid())

    {
      if (agent.GetAgentReference().IsRunning() &&
          !agent.GetAgentReference().AreYouDoomed()) {
        Agent &agentref = agent.GetAgentReference();
#ifdef AGENT_PROFILER
        int64 stamp1 = GetHighPerformanceTimeStamp();
#endif
        agent.GetAgentReference().HandleMessage(&Msg); // handle it
#ifdef AGENT_PROFILER
        if (!agentref.IsGarbage()) {
          int64 stamp2 = GetHighPerformanceTimeStamp();
          agentref.myAgentProfilerCumulativeTime -= stamp1;
          agentref.myAgentProfilerCumulativeTime += stamp2;
        }
#endif
      }
    }
  }
}

void World::TaskMusic(bool forceUpdate) {
  // This is called once a second

  std::string Track(ME_NO_TRACK);

  if (theApp.DoYouOnlyPlayMidiMusic()) {
    int roomID;
    int32 x, y;
    theMainView.GetViewCentre(x, y);
    bool found = myMap.GetRoomIDForPoint(Vector2D((int)x, (int)y), roomID);

    // do we have a roomID please?
    // if so get the track
    if (found) {
      myMap.GetRoomTrack(roomID, Track);
    }

    // still not finished check whether to default to
    // the meta room track
    if (Track.empty()) {
      found = myMap.GetMetaRoomIDForPoint(Vector2D((int)x, (int)y), roomID);

      if (found) {
        myMap.GetMetaRoomTrack(roomID, Track);
      }
    }

    if (Track != ME_NO_TRACK && !Track.empty()) {
      if (theSoundManager)
        theSoundManager->PlayMidiFile(Track);
    }
    return;
  }

  // Update music system only if it exists & it is not under agent control.
  if (!theMusicManager)
    return;

  uint32 iSelectableCount = 0;
  uint32 iLowestHealth = 255;
  float fMood = 0;
  float fThreat = 0;

  theAgentManager.CalculateMoodAndThreat(iSelectableCount, iLowestHealth, fMood,
                                         fThreat);

  std::string selectedTrack;

  if (iLowestHealth < HEALTH_NEARDEATH && myEventTimer == 0) {
    CAOSVar &nearDeathTrackName = GetGameVar("engine_near_death_track_name");
    if (nearDeathTrackName.GetType() == CAOSVar::typeString) {
      nearDeathTrackName.GetString(selectedTrack);
      myEventTimer = 15;
    }
  }

  // Events are all triggered in CAOS, so we don't need to worry directly about
  // them...
  if (myEventTimer == 0) {
    // Right then, there is not an event in progress....
    // Determine the Location of the currently selected Creature....
    AgentHandle currentCreature(GetSelectedCreature());
    if (currentCreature.IsCreature()) {
      Vector2D cPos = currentCreature.GetAgentReference().GetPosition();
      int x = Map::FastFloatToInteger(cPos.x);
      int y = Map::FastFloatToInteger(cPos.y);
      if (theMainView.IsPointOnScreen(x, y)) {
        int roomID;
        if (GetMap().GetRoomIDForPoint(cPos, roomID)) {
          GetMap().GetRoomTrack(roomID, selectedTrack);
        }
      }
    }

    // Okay then, if we still don't have a track, then let's play at center
    // room...
    if (selectedTrack.empty()) {
      int32 x, y;
      theMainView.GetViewCentre(x, y);
      int roomID;
      if (GetMap().GetRoomIDForPoint(Vector2D((int)x, (int)y), roomID)) {
        GetMap().GetRoomTrack(roomID, selectedTrack);
      }
    }

    // Hmm, if we still don't have a track, let's fall back on the metaroom
    // itself
    if (selectedTrack.empty()) {
      GetMap().GetMetaRoomTrack(GetMap().GetCurrentMetaRoom(), selectedTrack);
    }

    // Right then, we have a track in theory....
  } else
    myEventTimer--;

  if (!selectedTrack.empty()) {
    // We have a track, so let's see if we need to start it....
    std::string prevTrack;
    if (theMusicManager->GetCurrentTrackName()) {
      prevTrack = theMusicManager->GetCurrentTrackName();
    }
    if (selectedTrack != prevTrack) {
      // Hrm matron! We have to set a new track going as it were :)
      theMusicManager->BeginTrack(selectedTrack.c_str());
      myEventTimer = 2; // This prevents track switches for 2 seconds
    }
  }

  theMusicManager->UpdateSettings(fMood, fThreat);
}

void World::TriggerTrack(std::string &trackName, int secondsToEnforce) {
  if (!theMusicManager)
    return;

  std::string prevTrack;
  if (theMusicManager->GetCurrentTrackName()) {
    prevTrack = theMusicManager->GetCurrentTrackName();
  }
  if (trackName != prevTrack) {
    myEventTimer = secondsToEnforce;
    theMusicManager->BeginTrack(trackName.c_str());
  }
}

int World::MuteSoundManagers(int andMask, int eorMask) {
  // std::cout << "MuteSoundManagers before " << theSoundManager->IsMixerFaded()
  // << " m "
  //<< theMusicSoundManager->IsMixerFaded() << std::endl;
  int managers = 0;
  if (!theSoundManager || theSoundManager->IsMixerFaded())
    managers += 1;
  if (!theMusicSoundManager || theMusicSoundManager->IsMixerFaded())
    managers += 2;
  managers = (managers & andMask) ^ eorMask;

  if (theSoundManager) {
    if (managers & 1)
      theSoundManager->FadeOut();
    else
      theSoundManager->FadeIn();
  }

  if (theMusicSoundManager) {
    if (managers & 2)
      theMusicSoundManager->FadeOut();
    else
      theMusicSoundManager->FadeIn();
  }

  // std::cout << "MuteSoundManagers after " << theSoundManager->IsMixerFaded()
  // << " m "
  //<< theMusicSoundManager->IsMixerFaded() << std::endl;
  return managers;
}

void World::GetNextMetaRoom() {
  bool ok = false;
  myCurrentMetaRoom = GetMap().GetCurrentMetaRoom();
  int start = myCurrentMetaRoom;

  do {
    myCurrentMetaRoom++;

    if (myCurrentMetaRoom > GetMap().GetMetaRoomMaxID())
      myCurrentMetaRoom = 0;

    ok = SetMetaRoom(myCurrentMetaRoom, 0, -1, -1);
  } while (!ok && myCurrentMetaRoom != start);
}

void World::GetPreviousMetaRoom() {
  bool ok = false;
  myCurrentMetaRoom = GetMap().GetCurrentMetaRoom();
  int start = myCurrentMetaRoom;

  do {
    myCurrentMetaRoom--;

    if (myCurrentMetaRoom < 0)
      myCurrentMetaRoom = GetMap().GetMetaRoomMaxID();

    ok = SetMetaRoom(myCurrentMetaRoom, 0, -1, -1);
  } while (!ok && myCurrentMetaRoom != start);
}

bool World::SetMetaRoom(uint32 metaRoomID, uint32 transitionType, int32 xCamera,
                        int32 yCamera, bool bCentre) {
  // store old camera position
  uint32 metaPreviousRoomID = GetMap().GetCurrentMetaRoom();
  bool bResult = GetMap().SetMetaRoomDefaultCameraLocation(
      metaPreviousRoomID, theMainView.GetWorldPosition());

  // change meta room
  GetMap().SetCurrentMetaRoom(metaRoomID);

  // now tell the main view to change meta room too
  std::string background;

  int x, y, w, h;
  RECT metaRoomMBR;
  if (!GetMap().GetMetaRoomLocation(metaRoomID, x, y, w, h))
    return false;
  GetMap().GetCurrentBackground(metaRoomID, background);

  if (!background.empty()) {
    metaRoomMBR.left = x;
    metaRoomMBR.right = x + w - 1;
    metaRoomMBR.top = y;
    metaRoomMBR.bottom = y + h - 1;

    if (xCamera < 0 || yCamera < 0) {
      if (!GetMap().GetMetaRoomDefaultCameraLocation(metaRoomID, xCamera,
                                                     yCamera))
        return false;
    }

    theMainView.ChangeMetaRoom(background, metaRoomMBR, xCamera, yCamera,
                               transitionType, bCentre);
  }
  TaskMusic(true);
  return true;
}

// now work out the time of day type stuff on the fly from the game
// tick when asked.
int World::GetTimeOfDay(bool useCurrentTime, uint32 worldTick) {
  if (useCurrentTime)
    worldTick = myWorldTick;

  int x = NUMBER_OF_TIMES_OF_DAY;

  // get the length of one day in milliseconds as specified by the
  // game variables
  uint32 oneDay =
      (1000 / theApp.GetWorldTickInterval()) * 60 * myDayLengthInMinutes;

  // get the interval in the day - the millisecond interval that falls between
  // one of the 5 times of day from dawn to night
  uint32 interval = worldTick % oneDay;

  TimeOfDay calcTimeOfDay = (TimeOfDay)0;

  // now find out exactly which of those times of day that millisecond interval
  // lies between by calculating each of the interval bounds staring from
  // night time and going backwards to dawn.
  while (x) {
    // is the interval between eg night and evening?
    if ((interval < x * oneDay / NUMBER_OF_TIMES_OF_DAY) &&
        (interval >= (x - 1) * oneDay / NUMBER_OF_TIMES_OF_DAY)) {
      // if so set the time of day to the evening time
      calcTimeOfDay = TimeOfDay(x - 1);
      break;
    }
    // if not set the parameters to look between evening and afternoon
    x--;
  }

  // if it between

  return int(calcTimeOfDay);
}

uint32 World::GetYearsElapsed(bool useCurrentTime, uint32 worldTick) {
  if (useCurrentTime)
    worldTick = myWorldTick;

  // if in the event that a user plays the game so long that the game tick
  // resets itself keep a log of how many times they have done this in game
  // years note that this will be slightly wrong if they change the time of day
  // variables of the world
  return (worldTick / ((1000 / theApp.GetWorldTickInterval()) *
                       myYearLengthInDays * myDayLengthInMinutes * 60));
}

int World::GetSeason(bool useCurrentTime, uint32 worldTick) {
  if (useCurrentTime)
    worldTick = myWorldTick;

  int calcSeason = 0;
  int x = mySeasonCount;

  // get the length of one year in milliseconds as specified by the game
  // variables
  uint32 oneYear = (1000 / theApp.GetWorldTickInterval()) * 60 *
                   myDayLengthInMinutes * myYearLengthInDays;

  // get the millisecond interval in the year to find out which season
  // it falls in
  uint32 interval = worldTick % oneYear;

  // for each season interval (backwards) - summer to spring, autumn to summer,
  // winter to autumn check whether the millisecond interval lies between anyone
  // of these this actually starts from winter and goes backwards
  while (x) {
    // does the interval lie between autumn and summer?
    if ((interval < x * oneYear / mySeasonCount) &&
        (interval >= (x - 1) * oneYear / mySeasonCount)) {
      // if so it is still summer
      calcSeason = x - 1;
      break;
    }
    // else check if the interval is between winter and autumn
    x--;
  }

  // if all else fails it is spring
  return int(calcSeason);
}

int World::GetDayInSeason(bool useCurrentTime, uint32 worldTick) {
  if (useCurrentTime)
    worldTick = myWorldTick;

  // get the length of one season in milliseconds as specified by the game
  // variables
  uint32 oneSeason = (1000 / theApp.GetWorldTickInterval()) * 60 *
                     myDayLengthInMinutes * mySeasonLengthInDays;

  // get interval within season that we are
  uint32 interval = worldTick % oneSeason;

  int calcDay = interval / (oneSeason / mySeasonLengthInDays);
  ASSERT(calcDay >= 0 && calcDay < mySeasonLengthInDays);

  return calcDay;
}

CAOSVar &World::GetGameVar(const std::string &name) {
  if (name.empty())
    throw BasicException("Empty string GAME variables not allowed");
  return (myGameVars[name]);
}

std::string World::GetNextGameVar(const std::string &name) {
  if (name.empty() && myGameVars.size() > 0)
    return myGameVars.begin()->first;

  std::map<std::string, CAOSVar>::iterator it;
  it = myGameVars.find(name);
  if (it != myGameVars.end())
    it++;

  if (it == myGameVars.end())
    return "";
  else
    return it->first;
}

void World::DeleteGameVar(const std::string &name) {
  std::map<std::string, CAOSVar>::iterator it;
  it = myGameVars.find(name);
  if (it != myGameVars.end())
    myGameVars.erase(it);
}

AgentHandle World::GetSelectedCreature() { return mySelectedCreature; }

void World::SetSelectedCreature(AgentHandle &creature) {
  mySelectedCreature = creature;

  CAOSVar p1;
  p1.SetAgent(mySelectedCreature);
  theAgentManager.ExecuteScriptOnAllAgentsDeferred(
      SCRIPTSELECTEDCREATURECHANGED, COASVARAGENTNULL, p1, INTEGERZERO);
}

// ---------------------------------------------------------------------
// Method:		Load
// Arguments:	Name of world file to load
// Returns:		true if successful
// Description:	Loads the specified world. Also sets the name of the world
//				to be used by save
// ---------------------------------------------------------------------
bool World::Load(std::string const &worldName, bool loadBackup) {
  // We can only load once - world must be reconstructed to clear
  // variables for later loads
  if (loadOnlyOnceSanityCheck) {
    ASSERT(false);
    ErrorMessageHandler::Show("archive_error", 11, "World::Load");
    return false;
  }
  loadOnlyOnceSanityCheck = true;

  theMainView.SetLoading(true);

  // Prepare the view
  theMainView.Refresh();

  // Clear the catalogue and reload
  theApp.InitLocalisation();

  myNeedToBackUp = false;
  myName = worldName;

  bool worldHasLoaded = false;

  FilePath path(worldName, WORLDS_DIR);
  std::string fullPath = path.GetFullPath() + PathSeparator();

  CreateDirectory(fullPath.c_str(), NULL);
  fullPath += DefaultWorldName();
  // if no main world and everything in it, try the backup straight away
  // (this means we load the backup, rather than bootstrapping)
  if (!loadBackup && !FileExists(fullPath.c_str())) {
    std::string oldName = path.GetFullPath() + PathSeparator() + OldWorldName();
    if (FileExists(oldName.c_str()))
      fullPath = oldName;
    else
      loadBackup = true;
  }

  if (loadBackup)
    fullPath += ".bak";

  // if the world and everthying in it does not exist then
  // load all the bootstraps
  if (FileExists(fullPath.c_str()) == false) {
    // if this is the start up world then load the start up world
    if (worldName == "Startup") {
      CosInstaller scripts;

      worldHasLoaded = true;
      MoveFilesToBasement();
      ProcessFilesInPorch();
    } else {
      // register the default switcher world as loaded as we do not want
      // to include it as a real bootstrap world
      myLoadedBootstrapFolders.empty();
      myLoadedBootstrapFolders.push_back("000 Switcher");

      CheckForNewNewlyInstalledProductsOrAddOns();

      worldHasLoaded = true;
      MoveFilesToBasement();
      ProcessFilesInPorch();
    }

    // show the pointer
    thePointer.GetAgentReference().Show();
  }

  if (!worldHasLoaded) {
    try {
      std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
      CreaturesArchive archive(file, CreaturesArchive::Load);
      archive.SetUpdateProgressBarFunction(UpdateProgressBar);
      int fileSize = FileSize(fullPath.c_str());
      Read(archive, fileSize);

      // We've loaded in a good archive, so signal to back it up on the next
      // save
      if (!loadBackup)
        myNeedToBackUp = true;
      worldHasLoaded = true;

      // Delete any files we don't need any more
      MoveFilesToBasement();
      MoveFilesToAttic();
      ProcessFilesInPorch();
    } catch (BasicException &e) {
      ErrorMessageHandler::Show(e, "World::Load");
    } catch (...) {
      ErrorMessageHandler::Show("archive_error", 4, "World::Load");
    }
  }

  // now initialize your game variables
  theApp.RefreshFromGameVariables();

  theMainView.SetLoading(false);
  theApp.WindowHasResized();

  // tell agents we've loaded
  theAgentManager.ExecuteScriptOnAllAgents(
      SCRIPT_WORLD_LOADED, COASVARAGENTNULL, INTEGERZERO, INTEGERZERO);

  // tell all modules
  ModuleImporter::ResetIterator();
  while (ModuleInterface *moduleInterface = ModuleImporter::NextModule()) {
    moduleInterface->WorldLoaded();
  }

  theMainView.MakeTheEntityHandlerResetBoundsProperly();
  theMainView.Render();

  return worldHasLoaded;
}

void World::DoRefreshFromGameVariables() {
  //********************    GENERAL ENGINE VARIABLES  ************************//
  // time stuff - if they are not set then don't update your variables
  bool invalidData = false;
  // length of day (mins)
  CAOSVar &var = GetGameVar("engine_LengthOfDayInMinutes");

  ASSERT(var.GetType() == CAOSVar::typeInteger);

  int32 temp = var.GetInteger();
  if (temp != 0)
    myDayLengthInMinutes = temp;
  else
    invalidData = true;

  // length of seasons (days)
  CAOSVar &seasonLength = GetGameVar("engine_LengthOfSeasonInDays");

  ASSERT(seasonLength.GetType() == CAOSVar::typeInteger);

  temp = seasonLength.GetInteger();
  if (temp != 0)
    mySeasonLengthInDays = temp;
  else
    invalidData = true;

  // number of seasons
  CAOSVar &numseasons = GetGameVar("engine_NumberOfSeasons");

  ASSERT(numseasons.GetType() == CAOSVar::typeInteger);

  temp = numseasons.GetInteger();
  if (temp != 0)
    mySeasonCount = temp;
  else
    invalidData = true;

  if (!invalidData)
    myYearLengthInDays = mySeasonCount * mySeasonLengthInDays;

  CAOSVar &mute = GetGameVar("engine_mute");
  if (theSoundManager) {
    if (mute.GetInteger() == 0)
      theSoundManager->MuteMidiPlayer(false);
    else if (mute.GetInteger() == 1)
      theSoundManager->MuteMidiPlayer(true);
  }

  CAOSVar &dropAll =
      GetGameVar("engine_pointerCanCarryObjectsBetweenMetaRooms");
  ASSERT(dropAll.GetType() == CAOSVar::typeInteger);

  dropAll.GetInteger() == 0
      ? thePointer.GetPointerAgentReference().CarryAgentsFromMetaRoomToMetaRoom(
            false)
      : thePointer.GetPointerAgentReference().CarryAgentsFromMetaRoomToMetaRoom(
            true);

  // initialise from game variable first if this is empty
  // try the
  CAOSVar &pass = GetGameVar("engine_password");
  if (pass.GetType() == CAOSVar::typeString) {
    pass.GetString(myPassword);
  }

  // get your password from the world switcher if it has been changed
  // this overrides the game variable
  std::string replacementPassword = theApp.GetPassword();

  if (theApp.DoINeedToGetPassword())
    myPassword = replacementPassword;

  // zlib compression level
  int zlibCompressionLevel = GetGameVar("engine_zlib_compression").GetInteger();
  CreaturesArchive::SetZLibCompressionLevel(zlibCompressionLevel);
}

void World::CheckForNewNewlyInstalledProductsOrAddOns() {
  std::vector<std::string> unLoadedBootstrapFolders;
  FilePath path(myName, WORLDS_DIR);

  // Check each auxiliary bootstrap directory to see if we want to load it
  for (int i = 0; i < theDirectoryManager.GetAuxDirCount(BOOTSTRAP_DIR); ++i) {
    // Check out the Eame variable to say if we should boot this directory
    std::ostringstream out;
    out << "engine_no_auxiliary_bootstrap_" << i;
    CAOSVar &var = theApp.GetEameVar(out.str());
    int val = (var.GetType() == CAOSVar::typeInteger && var.GetInteger() == 0)
                  ? 0
                  : 1;

    // Look for a file overriding the eame variable
    std::string auxBootstrapFile =
        path.GetFullPath() + std::string(PathSeparator()) + out.str();
    if (FileExists(auxBootstrapFile.c_str())) {
      std::ifstream in(auxBootstrapFile.c_str());
      in >> val;
    } else {
      // Make that file for new worlds
      std::ofstream out(auxBootstrapFile.c_str());
      out << val;
    }

    if (val == 0)
      GetMissingDirsInDirectory(theDirectoryManager.GetAuxDir(BOOTSTRAP_DIR, i),
                                myLoadedBootstrapFolders,
                                unLoadedBootstrapFolders);
  }

  // sort into alphabetical order,
  std::sort(unLoadedBootstrapFolders.begin(), unLoadedBootstrapFolders.end());
  // remove duplicates
  unLoadedBootstrapFolders.erase(std::unique(unLoadedBootstrapFolders.begin(),
                                             unLoadedBootstrapFolders.end()),
                                 unLoadedBootstrapFolders.end());

  // now get the cos installer to load these new bootstrap folders in
  CosInstaller cos(unLoadedBootstrapFolders);

  // now register those missing bootstrap folders as loaded
  std::vector<std::string>::iterator it;
  for (it = unLoadedBootstrapFolders.begin();
       it != unLoadedBootstrapFolders.end(); it++) {
    myLoadedBootstrapFolders.push_back((
        *it)); //(unLoadedBootstrapFolders.begin(),unLoadedBootstrapFolders.end());
  }

  // sort into alphabetical order,
  std::sort(myLoadedBootstrapFolders.begin(), myLoadedBootstrapFolders.end());

  // if there is a file called "magic.cos" in this world's directory then
  // load it and then delete it.
  // Now look at all bootstraps in a folder called magic
  // read them in and then delete them
  std::string fullPath =
      path.GetFullPath() + std::string(PathSeparator()) + "magic.cos";

  if (FileExists(fullPath.c_str())) {
    CosInstaller cos(fullPath);
    DeleteFile(fullPath.c_str());
  }
}

bool World::Save() {

  if (myName == "") {
    ErrorMessageHandler::Show("archive_error", 7, "World::Save");
    return false;
  }

  FilePath path(myName, WORLDS_DIR);
  std::string fullPath =
      path.GetFullPath() + PathSeparator() + DefaultWorldName();
  std::string backPath =
      path.GetFullPath() + PathSeparator() + DefaultWorldName() + ".bak";
  std::string tempPath =
      path.GetFullPath() + PathSeparator() + DefaultWorldName() + ".tmp";

  // myNeedToBackUp is true if we have successfully loaded an archive, so we
  // know we have a safe version to backup.  This way, the .bak file is
  // guaranteed to always be a file which can be loaded in.
  if (myNeedToBackUp) {
    DeleteFile(backPath.c_str());
    MoveFile(fullPath.c_str(), backPath.c_str());
    myNeedToBackUp = false;
  }

  try {
    std::ofstream file(tempPath.c_str(), std::ios::out | std::ios::binary);
    CreaturesArchive archive(file, CreaturesArchive::Save);
    Write(archive);
  } catch (BasicException &e) {
    ErrorMessageHandler::Show(e, "World::Save");
    return false;
  } catch (...) {
    ErrorMessageHandler::Show("archive_error", 3, "World::Save");
    return false;
  }

  DeleteFile(fullPath.c_str());
  MoveFile(tempPath.c_str(), fullPath.c_str());
  return true;
}

bool World::CopyWorldDirectory(std::string const &source,
                               std::string const &destination) {
  return CopyDirectory(FilePath(source, WORLDS_DIR).GetFullPath(),
                       FilePath(destination, WORLDS_DIR).GetFullPath());
}

bool World::DeleteWorldDirectory(std::string const &source) {
  FilePath path(source, WORLDS_DIR);
  return DeleteDirectory(path.GetFullPath());
}

int World::WorldCount() {
  FilePath path("", WORLDS_DIR);

  myWorldNames.clear();

  GetDirsInDirectory(path.GetFullPath(), myWorldNames);

  // remove Startup directory from list
  std::vector<std::string>::iterator it =
      std::remove(myWorldNames.begin(), myWorldNames.end(), "Startup");
  it = std::remove(myWorldNames.begin(), it, "startup");
  if (it != myWorldNames.end())
    myWorldNames.erase(it);

  return myWorldNames.size();
}

bool World::WorldName(int index, std::string &name) {
  // fill in the worldnames
  WorldCount();
  if (index < 0 || index >= myWorldNames.size())
    return false;
  name = myWorldNames[index];
  return true;
}

void World::WriteMessage(AgentHandle const &from, AgentHandle const &to,
                         int msg, CAOSVar const &p1, CAOSVar const &p2,
                         unsigned delay) {
  myMessageQueue.WriteMessage(from, to, msg, p1, p2, delay);
}

void World::RemoveMessagesAbout(AgentHandle &o) {
  myMessageQueue.RemoveMessagesAbout(o);
}

bool World::ReadMessage(Message &message) {
  return myMessageQueue.ReadMessage(message);
}

bool World::GetPassword(int index, std::string &password) {
  // instead of creating a temp string var borrow the reference
  if (!WorldName(index, password)) {
    return false;
  }

  // if this is you then give them your own password if
  // it isn't empty
  if (password == myName) {
    if (!myPassword.empty()) {
      password = myPassword;
      return true;
    }
  }

  // look at the first few  details of the serialized file
  FilePath path(password, WORLDS_DIR);
  std::string fullPath = path.GetFullPath() + PathSeparator();
  CreateDirectory(fullPath.c_str(), NULL);
  fullPath += DefaultWorldName();

  // look for old serialisation name
  if (!FileExists(fullPath.c_str()))
    fullPath = path.GetFullPath() + PathSeparator() + OldWorldName();

  // if the world and everthying in it does not exist then
  // load all the bootstraps
  if (FileExists(fullPath.c_str())) {
    std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
    CreaturesArchive archive(file, CreaturesArchive::Load);
    try {
      // should output a version here
      archive >> password;
      return true;
    } catch (BasicException &e) {
      ErrorMessageHandler::Show(e, "World::GetPassword");
      return false;
    } catch (...) {
      ErrorMessageHandler::Show("archive_error", 4, "World::GetPassword");
      return false;
    }
  } else {
    // if there is a file called "magic.cos" in this world's directory then
    // load it and then delete it.
    path.SetFilePath(password, WORLDS_DIR);

    fullPath = path.GetFullPath() + std::string(PathSeparator()) + "magic.cos";
    if (FileExists(fullPath.c_str())) {
      CosInstaller cos(fullPath);
      CAOSVar &pass = GetGameVar("engine_password");
      if (pass.GetType() == CAOSVar::typeString) {
        pass.GetString(password);
        return true;
      }
    }
  }
  return false;
}

void World::SetPassword(std::string &password) { myPassword = password; }

bool World::IsStartUpWorld() { return myName == std::string("Startup"); }

void World::MarkFileForAttic(const FilePath &fileToDeleteLater) {
  // Only attic things which are in the world directory
  std::string path;
  if (fileToDeleteLater.GetWorldDirectoryVersionOfTheFile(path))
    myFilesForAtticDelayed.push_back(fileToDeleteLater);
}

void World::MarkFileCreated(const FilePath &fileJustCreated) {
  // Return if it isn't in the world's directory
  std::string path;
  if (!fileJustCreated.GetWorldDirectoryVersionOfTheFile(path))
    return;

  bool wasOnDeleteList = false;

  // Make sure it is not on the delayed delete list
  if (DeleteFromFilePathList(myFilesForAtticDelayed, fileJustCreated))
    wasOnDeleteList = true;
  // ... or the main delete list
  if (DeleteFromFilePathList(myFilesForAtticNextTime, fileJustCreated))
    wasOnDeleteList = true;

  // if it wasn't on any delete list
  if (!wasOnDeleteList) {
    myFilesJustCreated.push_back(fileJustCreated);

    std::string stairFileTemp = GetBasementPath() + BasementFileName() + ".tmp";
    {
      std::ofstream stairStream(stairFileTemp.c_str(),
                                std::ios::out | std::ios::binary);
      CreaturesArchive archive(stairStream, CreaturesArchive::Save);
      archive << myFilesJustCreated;
    }

    std::string stairFile = GetBasementPath() + BasementFileName();
    DeleteFile(stairFile.c_str());
    MoveFile(stairFileTemp.c_str(), stairFile.c_str());
  }
}

bool World::DeleteFromFilePathList(std::vector<FilePath> &list,
                                   const FilePath &item) {
  std::vector<FilePath>::iterator it =
      std::remove(list.begin(), list.end(), item);
  bool deleted = (it != list.end());
  list.erase(it, list.end());
  return deleted;
}

std::string World::GetBasementPath() {
  std::string basementPath;
  theDirectoryManager.GetWorldDirectoryVersion("Basement", basementPath, true);
  return basementPath;
}

void World::MoveFilesToAttic() {
  // Create attic directory if necessary
  std::string atticPath;
  theDirectoryManager.GetWorldDirectoryVersion("Attic", atticPath, true);

  // Remove files from next time list
  int n = myFilesForAtticNextTime.size();
  for (int i = 0; i < n; ++i) {
    std::string fullFilename = myFilesForAtticNextTime[i].GetFullPath();
    std::string atticFilename =
        atticPath + myFilesForAtticNextTime[i].GetFileName();

    // Move file to the attic directory
    DeleteFile(atticFilename.c_str());
    MoveFile(fullFilename.c_str(), atticFilename.c_str());
    // If the move fails, don't worry about it
    // (the worst that can happen is we have unused files about the place)
  }

  // And copy from delayed list to next time list
  // (the delay is so we only actually delete files after
  // successfully loading twice, so it is guranteed that
  // no .bak exists which refers to them)
  myFilesForAtticNextTime = myFilesForAtticDelayed;
  myFilesForAtticDelayed.clear();
}

void World::MoveFilesToBasement() {
  std::string stairFile = GetBasementPath() + BasementFileName();
  std::vector<FilePath> tryingToClimbTheStairs;
  {
    std::ifstream stairStream(stairFile.c_str(),
                              std::ios::in | std::ios::binary);
    if (stairStream.good()) {
      CreaturesArchive archive(stairStream, CreaturesArchive::Load);
      archive >> tryingToClimbTheStairs;
    }
  }

  int n = tryingToClimbTheStairs.size();
  for (int i = 0; i < n; ++i) {
    std::vector<FilePath>::iterator it =
        std::find(myFilesJustCreated.begin(), myFilesJustCreated.end(),
                  tryingToClimbTheStairs[i]);
    if (it == myFilesJustCreated.end()) {
      // This file was created recently, but the
      // archive referring to it wasn't saved.  This
      // means we don't need it any more, so we
      // move it to the basement
      std::string fullFilename = tryingToClimbTheStairs[i].GetFullPath();
      std::string basementFilename =
          GetBasementPath() + tryingToClimbTheStairs[i].GetFileName();

      DeleteFile(basementFilename.c_str());
      MoveFile(fullFilename.c_str(), basementFilename.c_str());
      // If the move fails, don't worry about it
      // (worse that can happen is we have unused files about)
    }
  }

  DeleteFile(stairFile.c_str());
  myFilesJustCreated.clear();
}

std::string World::GetPorchPath() {
  std::string porchPath;
  theDirectoryManager.GetWorldDirectoryVersion("Porch", porchPath, true);
  return porchPath;
}

void World::MoveFileToPorch(std::string creatureFile) {
  std::string justFileName =
      creatureFile.substr(creatureFile.find_last_of("/:\\") + 1);
  std::string porchFilename = GetPorchPath() + justFileName;

  DeleteFile(porchFilename.c_str());
  MoveFile(creatureFile.c_str(), porchFilename.c_str());

  myFilesInThePorch.push_back(justFileName);
}

bool World::WarpExtension(const std::string &file) {
  std::string warpExtension = ".warp";
  if (file.size() >= warpExtension.size()) {
    std::string extension =
        file.substr(file.size() - warpExtension.size(), warpExtension.size());
    if (extension == warpExtension)
      return true;
  }
  return false;
}

void World::ProcessFilesInPorch() {
  std::vector<std::string> filesInPorch;
  GetFilesInDirectory(GetPorchPath(), filesInPorch, "*.creature");
  GetFilesInDirectory(GetPorchPath(), filesInPorch, "*.warp");

  int n = filesInPorch.size();
  for (int i = 0; i < n; ++i) {
    std::vector<std::string>::iterator it = std::find(
        myFilesInThePorch.begin(), myFilesInThePorch.end(), filesInPorch[i]);
    std::string porchFilename = GetPorchPath() + filesInPorch[i];
    if (it == myFilesInThePorch.end()) {
      // This file is in the physical porch, but not in our record of what
      // should be in the porch.  This means we crashed after importing, so
      // we move the file back to the exported creature directory.

      std::string exportedFilename =
          theDirectoryManager.GetDirectory(CREATURES_DIR);
      if (WarpExtension(filesInPorch[i])) {
        // if the file name ends in the warp file extension,
        // then we move it back to a different place
        exportedFilename = theApp.GetWarpIncomingPath();
      }
      exportedFilename += filesInPorch[i];

      CopyFile(porchFilename.c_str(), exportedFilename.c_str(), TRUE);
    }
    // We don't need the file any more - either we've copied it to the exported
    // creature directory, or we've reloaded the world which contains the
    // imported creature
    DeleteFile(porchFilename.c_str());
  }

  myFilesInThePorch.clear();
}

void World::LoadSpecifiedBootstraps(int32 subbootNumbers, int32 folderNumbers,
                                    bool refreshWorld) {
  if (refreshWorld) {
    theAgentManager.KillAllAgents();
    GetMap().Reset();
    thePointer = theAgentManager.CreatePointer();
  }

  CosInstaller cos(subbootNumbers, folderNumbers);
}

// static
void World::UpdateProgressBar(int amount) { theApp.UpdateProgressBar(amount); }

const char *World::DefaultWorldName() {
  if (theApp.GetGameName() == std::string("Sea-Monkeys"))
    return "TheSeaAndAllThatLurksInItsDepths";
  else
    return "SpaceAndAllThatIsOutThere";
}
