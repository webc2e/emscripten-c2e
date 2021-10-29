// --------------------------------------------------------------------------
// Filename:	App.h
// Class:		App
// Purpose:		This contains the application class
//
//
// Description:
//
//
//
// History:
// -------  Steve Grand		created
// 04Dec98	Alima			Commented out old display engine stuff
// and put
//							in new display engine
// hooks.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SMALL_FURRY_CREATURES_H
#define SMALL_FURRY_CREATURES_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../common/C2eTypes.h"
#ifndef C2E_SDL
	#include "Display/Window.h"
#endif
#include "TimeFuncs.h"
#include "InputManager.h"

#include "../common/Configurator.h"
#include "../modules/ModuleAPI.h"

//#include "c:\projects\nettymessage\thenetty.h"

class World;
class Agent;
class PrayManager;

#include "Caos/CAOSVar.h"

#define theApp App::GetTheApp()
extern class SoundManager *theSoundManager; // pointer to sound manager object
extern class MusicManager
    *theMusicManager; // pointer to global music manager object

class EntityImage;

class C2E_MODULE_API App {
public:
  App();
  ~App();
  static App &GetTheApp();

  // Sets up the registry-replacing config files, which are accessed by
  // UserSettings() and MachineSettings()
  // _Should_ really be in Init() or constructor, but we need this
  // separate function because App::Init() is called _after_ the window
  // is opened but the startup code accesses other App members before
  // the window is opened.
  //
  // Sigh.
  //
  // (Startup code is in Display/Window.cpp or Display/SDL/SDL_Main.cpp)
  bool InitConfigFiles();

  char MyDocuments[MAX_PATH + 1] = "/mydocs";

  std::string GetWarpOutgoingPath();
  std::string GetWarpIncomingPath();

  void DisplaySettingsErrorNextTick() { myDisplaySettingsErrorNextTick = true; }

  inline int GetLinePlane() { return myPlaneForConnectiveAgentLines; }

  inline int GetCreaturePickupStatus() { return myCreaturePickupStatus; }

  inline bool ShouldSkeletonsAnimateDoubleSpeed() {
    return myShouldSkeletonsAnimateDoubleSpeedFlag;
  }
  inline void ShouldSkeletonsAnimateDoubleSpeed(bool b) {
    myShouldSkeletonsAnimateDoubleSpeedFlag = b;
  }

  inline bool ShouldHighlightAgentsKnownToCreature() {
    return myShouldHighlightAgentsKnownToCreatureFlag;
  }
  inline void SetWhetherWeShouldHighlightAgentsKnownToCreature(bool b) {
    myShouldHighlightAgentsKnownToCreatureFlag = b;
  }
  inline void SetWhichCreaturePermissionToHighlight(int i) {
    myCreaturePermissionToHighlight = i;
  }
  inline int GetWhichCreaturePermissionToHighlight() {
    return myCreaturePermissionToHighlight;
  }

  inline float GetMaximumDistanceBeforePortLineWarns() {
    return myMaximumDistanceBeforePortLineWarns;
  }
  inline float GetMaximumDistanceBeforePortLineSnaps() {
    return myMaximumDistanceBeforePortLineSnaps;
  }

  // return a reference to the world
  World &GetWorld() { return *myWorld; }

  std::string GetWorldName();

  InputManager &GetInputManager() { return myInputManager; }

  std::string GetGameName() { return myGameName; }
  void SetGameName(const std::string &gameName) { myGameName = gameName; }

  static int GetWorldTickInterval();
  static void SetWorldTickInterval(int worldTickInterval);

  bool DoYouOnlyPlayMidiMusic() { return IOnlyPlayMidiMusic; }

  bool PlayAllSoundsAtMaximumLevel() { return myPlayAllSoundsAtMaximumLevel; }

  void HandleInput();

  bool Init();

  void UpdateApp();
  void ShutDown();

  void ChangeResolution();
  void ToggleFullScreenMode();
  void SetUpMainView(); // here until I know what the room system will
                        // be about
  void DisableMapImage();
  void EnableMapImage();

  void WindowHasResized();
  void WindowHasMoved();

  void DisableMainView();
  void EnableMainView();

  void ToggleMidi() { IOnlyPlayMidiMusic = !IOnlyPlayMidiMusic; }

  bool InitLocalisation();
  bool CreateNewWorld(std::string &worldName);

  void BeginWaitCursor();
  void EndWaitCursor();

  inline uint32 GetSystemTick() const { return mySystemTick; }
  float GetTickRateFactor();
  bool GetFastestTicks() const { return myFastestTicks; }
  int GetLastTickGap() const { return myLastTickGap; }

  bool CreateProgressBar();
  void StartProgressBar(int catOffset);
  void SpecifyProgressIntervals(int updateIntervals);

  void UpdateProgressBar(int amount = 1);
  void EndProgressBar();

  void SetPassword(std::string &password);
  std::string GetPassword();
  bool DoINeedToGetPassword();

  void RefreshFromGameVariables();

  bool ProcessCommandLine(std::string commandLine);
  bool AutoKillAgentsOnError() const { return myAutoKillAgentsOnError; }
  int EorWolfValues(int andMask, int eorMask);

  bool DebugKeyNow();
  bool DebugKeyNowNoShift();

  std::string GenerateWindowTitle();

  // These provide callback to external modules, that need
  // initialisation functions calling
  typedef void (*InitialisationFunction)(App &);
  static int AddInitialisationFunction(InitialisationFunction function);
  static std::vector<InitialisationFunction> &GetInitialisationFunctions();
  void CallInitialisationFunctions();

#ifdef _WIN32
  bool GetAppDetails(std::string *path, std::string *fileName,
                     std::string *extension);
#endif
  std::string GetNetworkUserId();
  std::string GetNetworkNickname();
  void NotifyNewNickname(const std::string &nickName);

  // Like game variables, only global to the engine (engine game variables)
  CAOSVar &GetEameVar(const std::string &name);
  std::string GetNextEameVar(const std::string &name);
  void DeleteEameVar(const std::string &name);

  CAOSVar &GetGameVar(const std::string &name);

  std::string GetLangCLib();
  std::string GetLangCatalogue();

private:
  void DoRefreshFromGameVariables();

  void DoLoadWorld(const std::string &worldName);

  void internalWindowHasResized();
  void internalWindowHasMoved();

  static App ourApp;

#ifdef _WIN32
  bool CheckForCD();
  bool CheckForMutex();
  bool CheckAllFreeDiskSpace(int expectedSystem, int expectedGeneral);
  bool CheckFreeDiskSpace(std::string path, int expected);
#endif

  void AddBasicPrayDirectories();

public:
  bool IsFullScreen();
#ifdef _WIN32
  HWND GetPreviewWindowHandle();
#endif
  bool GetIsScreenSaverPreview();
  bool GetScreenSaverConfig();
  bool IsAppAScreenSaver();
  bool mySaveNextTick;
  std::string myLoadThisWorldNextTick;
  bool myQuitNextTick;
  bool myToggleFullScreenNextTick;

  int myScrollingMask;
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif
  std::vector<byte> myScrollingSpeedRangeUp;
  std::vector<byte> myScrollingSpeedRangeDown;
#ifdef _MSC_VER
#pragma warning(default : 4251)
#endif

  static const int ourTickLengthsAgo;

  // access settings specific to current player
  Configurator &UserSettings() { return myUserSettings; }

  // access settings global to whole installation (eg directory paths)
  Configurator &MachineSettings() { return myMachineSettings; }

  std::string GetDefaultMNG();

private:
#ifdef _WIN32
  HWND myPreviewWindowHandle;
#endif
  bool IsScreenSaverPreview;
  bool IsScreenSaverConfig;
  bool IsScreenSaver;
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif
  // registry replacement
  Configurator myUserSettings;
  Configurator myMachineSettings;

  bool myResizedFlag;
  bool myMovedFlag;
  bool myDisplaySettingsErrorNextTick;
  bool myDelayedResizeFlag;
  bool myDelayedMovingFlag;
  bool InitLocalCatalogueFilesFromTheWorldsDirectory();

  bool myShouldSkeletonsAnimateDoubleSpeedFlag;
  bool myShouldHighlightAgentsKnownToCreatureFlag;
  int myCreaturePermissionToHighlight;
  int myPlaneForConnectiveAgentLines;

  int myCreaturePickupStatus;

  void SetUpSound(void);
  bool IOnlyPlayMidiMusic;
#ifdef _WIN32
#ifndef C2E_SDL
  HCURSOR myCursor;
#endif
  HANDLE myMutex;
#endif

  World *myWorld;

  // The system framework will keep the InputManager fed (see window.cpp)
  InputManager myInputManager;

  std::string myGameName;

  uint32 mySystemTick;
  EntityImage *myProgressBar;
  bool myPlayAllSoundsAtMaximumLevel;
  bool myAutoKillAgentsOnError;

  std::vector<uint32> myRecentTickLengths;
  int myRecentTickPos;
  std::string myPasswordForNextWorldLoaded;
  bool myIHaveAPassword;

  bool myRenderDisplay;
  bool myRenderDisplayNextTick;
  bool myFastestTicks;

  float myMaximumDistanceBeforePortLineWarns;
  float myMaximumDistanceBeforePortLineSnaps;

  uint32 myStartStamp;
  int myLastTickGap;

  static int myWorldTickInterval;

  std::map<std::string, CAOSVar> myEameVars;

#ifdef _MSC_VER
#pragma warning(default : 4251)
#endif

#ifdef NETTY
public:
  TheNetty *Netty;
#endif
};

/////////////////////////////////////////////////////////////////////////////

#endif
