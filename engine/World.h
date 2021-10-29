#ifndef WORLD_H
#define WORLD_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif


#include "Caos/Scriptorium.h"
#include "Map/Map.h"
#include "Agents/MessageQueue.h"
#include "Creature/History/HistoryStore.h"


#include "Display/TintManager.h"

class Creature;
class FilePath;

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

extern AgentHandle thePointer; // default cursor

class World {
public:
  void Init();

  Map &GetMap() { return myMap; };

  void TaskSwitcher(); // Timer tick has occurred. Do a new batch of tasks
  void TaskMusic(bool forceUpdate = false);
  void TaskHandleMessages();

  Scriptorium &GetScriptorium() { return myScriptorium; }

  void ToggleFullScreenMode();
  void GetNextMetaRoom();
  void GetPreviousMetaRoom();

  bool SetMetaRoom(uint32 metaRoomID, uint32 transitionType = 0,
                   int32 xCamera = -1, int32 yCamera = -1,
                   bool bCentre = false);
  inline uint32 GetWorldTick() const { return myWorldTick; }
  inline void SetWorldTick(uint32 newTick) { myWorldTick = newTick; }

  void ToggleSeasons();
  void ToggleTimeOfDay();

  // Trigger to enable music events from CAOS.
  void TriggerTrack(std::string &trackName, int secondsToEnforce);

  // And mute controls...
  int MuteSoundManagers(int andMask, int eorMask);

  // ------------------------------------------------------------------------
  // Function:	GetGameVar()
  // Description:	Fetches (or creates) a caos game variable. These
  // globally
  //				available, string-indexed variables are stored in
  //the 				World object. If GetGameVar is called for a non-existant 				var, a new var
  //is created (contents undefined).
  // Arguments:	name - name of variable.
  // Returns:		a reference to a CAOSVar (which can hold int, float,
  //				string or agent).
  // ------------------------------------------------------------------------
  CAOSVar &GetGameVar(const std::string &name);

  // ------------------------------------------------------------------------
  // Function:	DeleteGameVar()
  // Description:	Deletes a game variable. This function may be called
  // even
  //				for non-existant variables.
  // Arguments:	name - name of variable.
  // Returns:
  // ------------------------------------------------------------------------
  void DeleteGameVar(const std::string &name);
  std::string GetNextGameVar(const std::string &name);

  AgentHandle GetSelectedCreature();
  void SetSelectedCreature(AgentHandle &h);

  bool GetPassword(int index, std::string &worldName);
  void SetPassword(std::string &password);

  static const char *OldWorldName() { return "TheWorldAndEverythingInIt"; }
  static const char *DefaultWorldName();
  static std::string BasementFileName() {
    return std::string("ClimbingOutOfTheBasement");
  }

  void CheckForNewNewlyInstalledProductsOrAddOns();

  virtual ~World();

  enum { HEALTH_NEARDEATH = 10 };

  World();

  enum TimeOfDay {
    DAWN = 0,
    MORNING,
    AFTERNOON,
    EVENING,
    NIGHT,

    NUMBER_OF_TIMES_OF_DAY
  };

  int GetTimeOfDay(bool useCurrentTime = true, uint32 worldTick = 0);
  int GetSeason(bool useCurrentTime = true, uint32 worldTick = 0);
  uint32 GetYearsElapsed(bool useCurrentTime = true, uint32 worldTick = 0);
  int GetDayInSeason(bool useCurrentTime = true, uint32 worldTick = 0);

  // ---------------------------------------------------------------------
  // Method:		WriteMessage
  // Arguments:	from - agent sending the message
  //				to - agent recieving the message
  //				msg - message type
  //				p1, p2 - parameters
  //				delay
  // Returns:
  // Description:	Enters a new message into the queue, either for
  //				immediate dispatch( on the next tick) or delayed for
  //a 				specified number of ticks
  // ---------------------------------------------------------------------
  void WriteMessage(AgentHandle const &from, AgentHandle const &to, int msg,
                    CAOSVar const &p1, CAOSVar const &p2, unsigned delay);

  void RemoveMessagesAbout(AgentHandle &o);

  bool ReadMessage(Message &message);

  TintManager &GetTintManager(int index);
  void SetTintManager(int index, uint8 red, uint8 green, uint8 blue, uint8 rot,
                      uint8 swap);

  // ----------------------------------------------------------------------
  // Method:		Write
  // Arguments:	archive - archive being written to
  // Returns:		true if successful
  // Description:	Overridable function - writes details to archive,
  //				taking serialisation into account
  // ----------------------------------------------------------------------
  virtual bool Write(CreaturesArchive &archive) const;

  // ----------------------------------------------------------------------
  // Method:		Read
  // Arguments:	archive - archive being read from
  // Returns:		true if successful
  // Description:	Overridable function - reads detail of class from
  // archive
  // ----------------------------------------------------------------------
  virtual bool Read(CreaturesArchive &archive, int fileSize);

  // ---------------------------------------------------------------------
  // Method:		Load
  // Arguments:	Name of world file to load
  // Returns:		true if successful
  // Description:	Loads the specified world. Also sets the name of the
  // world
  //				to be used by save.  Should only be called
  //immediately 				after constructing.  (Unfortunately we can't make it a 				parameter
  //to the constructor, because it could call 				CAOS commands from the boostrap,
  //and some of those require 				myWorld to be set in App).
  // ---------------------------------------------------------------------
  bool Load(std::string const &worldName, bool loadBackup);

  void LoadSpecifiedBootstraps(int32 subboot_numbers, int32 folder_numbers,
                               bool refreshWorld);

  // ---------------------------------------------------------------------
  // Method:		Save
  // Arguments:	None
  // Returns:		true if successful
  // Description:	Saves the current world.
  // ---------------------------------------------------------------------
  bool Save();

  // ---------------------------------------------------------------------
  // Method:		Copy
  // Arguments:	source - name of world to copy
  //				destination - name of world copy to create
  // Returns:		true if successful
  // Description:	Copies the source directory and any sub-directories to
  //				the destination.
  // ---------------------------------------------------------------------
  static bool CopyWorldDirectory(std::string const &source,
                                 std::string const &destination);
  static bool DeleteWorldDirectory(std::string const &source);

  int WorldCount();

  bool WorldName(int index, std::string &name);

  std::string GetWorldName() const { return myName; }
  std::string GetUniqueIdentifier() const { return myUniqueIdentifier; }

  bool IsStartUpWorld();

  bool GetPausedWorldTick() const { return myPausedWorldTick; }
  void SetPausedWorldTick(bool paused) { myPausedWorldTick = paused; }

  void MarkFileForAttic(const FilePath &fileToDeleteLater);
  void MarkFileCreated(const FilePath &fileJustCreated);
  void MoveFileToPorch(std::string creatureFile);

  HistoryStore &GetHistoryStore() { return myHistoryStore; }

  bool GetWhetherLoading() { return myCurrentlyLoadingWorld; }

  void DoRefreshFromGameVariables();

  static bool WarpExtension(const std::string &file);

  static void UpdateProgressBar(int amount);

protected:
  void MoveFilesToAttic();
  void MoveFilesToBasement();
  void ProcessFilesInPorch();
  std::string GetBasementPath();
  std::string GetPorchPath();
  static bool DeleteFromFilePathList(std::vector<FilePath> &list,
                                     const FilePath &item);

private:
  // Music timer to prevent replacement of CAOS invoked music
  int myEventTimer;
  bool loadOnlyOnceSanityCheck;
  bool initialisedSanityCheck;

  Scriptorium myScriptorium;
  Map myMap;
  int32 myCurrentMetaRoom;

  // time of day stuff
  uint32 mySeasonCount;
  uint32 mySeasonLengthInDays;
  uint32 myDayLengthInMinutes;
  uint32 myYearLengthInDays;
  uint32 myWorldTick;

  bool myPausedWorldTick;

  AgentHandle mySelectedCreature;

  std::map<std::string, CAOSVar> myGameVars;
  std::vector<TintManager> myTints;
  MessageQueue myMessageQueue;

  std::string myName;
  std::string myPassword;
  std::string myUniqueIdentifier;

  uint32 myLastPixelFormat;
  bool myNeedToBackUp;
  std::vector<std::string> myWorldNames;
  std::vector<std::string> myLoadedBootstrapFolders;

  std::vector<FilePath> myFilesForAtticDelayed;
  std::vector<FilePath> myFilesForAtticNextTime;

  std::vector<FilePath> myFilesJustCreated;
  std::vector<std::string> myFilesInThePorch;

  HistoryStore myHistoryStore;

  bool myCurrentlyLoadingWorld;
};

/////////////////////////////////////////////////////////////////////////////

#endif // WORLD_H
