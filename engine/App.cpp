// app.cpp
// 22/02/2001 Dave Carter: added include for creature/vocab.h

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "CosInstaller.h"

#include "ModuleImporter.h"

#include "App.h"

#include "Agents/AgentConstants.h"
#include "Maths.h"
#include "Caos/CAOSMachine.h"
#include "Caos/Orderiser.h"
#include "Display/DisplayErrorConstants.h"

#include "Display/DisplayEngine.h"

#include "AgentDisplay/EntityImage.h"
#include "Display/ErrorMessageHandler.h"
#include "Camera/MainCamera.h"
#include "General.h"		// for logging macros
#include "World.h"
#ifdef _WIN32
#include "RegistryHandler.h"
#endif
#include "C2eServices.h"
#include "Sound/Soundlib.h"
#include "Display/SharedGallery.h"

#include "Sound/MusicTimer.h"
#include "Sound/MusicManager.h"
#include "Sound/MusicGlobals.h"

#include "AgentManager.h"

#include "Creature/SensoryFaculty.h"
#include "Creature/Vocab.h"

#include "../common/PRAYFiles/PrayManager.h"
#include "Creature/Brain/BrainScriptFunctions.h"
#include "Caos/AutoDocumentationTable.h"
#include "TimeFuncs.h"
#include "../common/FileFuncs.h"
#include "../common/StringFuncs.h"
#include "DirectoryManager.h"

#include "build.h"

#include "../common/C2eDebug.h"

#ifndef _WIN32
// VK_ scancodes
#include "unix/KeyScan.h"
// for stat()
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include "Display/Win32ScreenKiller.h"
#endif

#ifdef C2E_SDL
#include "Display/SDL/SDL_Main.h"	// for SignalTerminateApp()
#endif

#include <fstream>
#include <locale.h>

////////////////////// GLOBALS ////////////////////////

App App::ourApp;
App& App::GetTheApp()
{
	return ourApp;
}

const int App::ourTickLengthsAgo = 10;
int App::myWorldTickInterval = 50;

SoundManager *theSoundManager = NULL; // pointer to global Sound Manager
MusicManager *theMusicManager = NULL; // pointer to global Music Manager
/////////////////////////////////////////////////////////////////////////////
// App

App::App() {
  std::cout << "The app has been created" << std::endl;

#ifdef NETTY
  Netty = new TheNetty();
  Netty->TitleOut("DS");
#endif

  myWorldTickInterval = 50;

  myWorld = NULL;
#ifndef C2E_SDL
  myCursor = NULL;
#endif
  mySystemTick = 0;
  myPlaneForConnectiveAgentLines = 9998;
  myShouldHighlightAgentsKnownToCreatureFlag = false;
  myShouldSkeletonsAnimateDoubleSpeedFlag = false;
  myCreaturePermissionToHighlight = 0;
  myPlayAllSoundsAtMaximumLevel = false;

  mySaveNextTick = false;
  myLoadThisWorldNextTick = "";
  myQuitNextTick = false;
  myProgressBar = NULL;

  myAutoKillAgentsOnError = false;

#ifndef C2E_SDL
  myMutex = 0;
#endif

  myRecentTickLengths.resize(ourTickLengthsAgo);
  std::fill(myRecentTickLengths.begin(), myRecentTickLengths.end(),
            (uint32)(GetWorldTickInterval() * 0.9));
  myLastTickGap = -1;
  myRecentTickPos = 0;
  myPasswordForNextWorldLoaded = "";
  myIHaveAPassword = false;

  myRenderDisplay = true;
  myRenderDisplayNextTick = false;
  myFastestTicks = false;
  myToggleFullScreenNextTick = false;
  myResizedFlag = false;
  myMovedFlag = false;
  myDisplaySettingsErrorNextTick = false;
  myDelayedResizeFlag = false;
  myDelayedMovingFlag = false;
  IsScreenSaver = false;
  IsScreenSaverPreview = false;
  IsScreenSaverConfig = false;
#ifdef _WIN32
  myPreviewWindowHandle = NULL;
#endif

  myScrollingSpeedRangeUp.push_back(1);
  myScrollingSpeedRangeUp.push_back(2);
  myScrollingSpeedRangeUp.push_back(4);
  myScrollingSpeedRangeUp.push_back(8);
  myScrollingSpeedRangeUp.push_back(16);
  myScrollingSpeedRangeUp.push_back(32);
  myScrollingSpeedRangeUp.push_back(64);
  myScrollingSpeedRangeDown.push_back(0);
  myScrollingSpeedRangeDown.push_back(1);
  myScrollingSpeedRangeDown.push_back(2);
  myScrollingSpeedRangeDown.push_back(4);
  myScrollingSpeedRangeDown.push_back(8);
  myScrollingSpeedRangeDown.push_back(16);
  myScrollingSpeedRangeDown.push_back(32);
  myScrollingMask = 1 + 2 + 4 + 8;
  myMaximumDistanceBeforePortLineWarns = 600.0f;
  myMaximumDistanceBeforePortLineSnaps = 800.0f;
}

App::~App() { ; }

/////////////////////////////////////////////////////////////////////////////
// App initialization

bool App::Init() {
  theFlightRecorder.Log(FLIGHT_STARTUP, "In App init");
  std::string path = theDirectoryManager.GetDirectory(CREATURE_DATABASE_DIR);

  SharedGallery::theSharedGallery().SetCreatureGalleryFolder(path);
  SharedGallery::theSharedGallery().CleanCreatureGalleryFolder();

#ifdef _WIN32
  // Prevent multiple copies of game from running.
  while (!CheckForMutex()) {
    // We don't want the error message when we're a screen saver
    if (!IsScreenSaver) {
      std::string message =
          GetGameName() + " " + theCatalogue.Get("app_error", 0);
      if (::MessageBox(theMainWindow, message.c_str(), GetGameName().c_str(),
                       MB_SYSTEMMODAL | MB_RETRYCANCEL) == IDCANCEL)
        return false;
    } else {
      HANDLE hProcess = GetCurrentProcess();
      TerminateProcess(hProcess, -1);
    }
  }

  // Copy protection (primitive)
  // Each product needing this has to be specifically hard-coded.
  // If you use it, you need to add catalogue entries to your
  // game's catalogue files appropriately:
  //
  // TAG "folks_you_gotta_put_a_cd_in"
  // "You must have your Sea-Monkeys Disc in your CD-ROM drive to play.\nPlease
  // insert your Sea-Monkeys CD-ROM and try again."
  /*	  SPARKY
          if (myGameName == "Sea-Monkeys")
          {
                  // It would be simply irritating to check for the screen saver
                  if (!(IsAppAScreenSaver() || GetScreenSaverConfig() ||
     GetIsScreenSaverPreview()))
                  {
                          theFlightRecorder.Log(FLIGHT_STARTUP, "CD check");
                          while (!CheckForCD())
                          {
                                  std::string message =
     theCatalogue.Get("folks_you_gotta_put_a_cd_in", 0); if
     (::MessageBox(theMainWindow, message.c_str(), GetGameName().c_str(),
     MB_SYSTEMMODAL | MB_RETRYCANCEL) == IDCANCEL) return false;
                          }
                  }
          }
    */
  // Check disk space
  theFlightRecorder.Log(FLIGHT_STARTUP, "Disk space check");
  int expectedSystem = 75 * 1024 * 1024;
  int expectedGeneral = 32 * 1024 * 1024;
  UserSettings().Get("DiskSpaceCheckSystem", expectedSystem);
  UserSettings().Get("DiskSpaceCheck", expectedGeneral);
  if (!CheckAllFreeDiskSpace(expectedSystem, expectedGeneral)) {
    std::ostringstream out;
    out << theCatalogue.Get("app_insufficient_disk_space", 0) << GetGameName()
        << theCatalogue.Get("app_insufficient_disk_space", 1)
        << expectedSystem / 1024 / 1024
        << theCatalogue.Get("app_insufficient_disk_space", 2)
        << expectedGeneral / 1024 / 1024
        << theCatalogue.Get("app_insufficient_disk_space", 3);

    if (::MessageBox(theMainWindow, out.str().c_str(), GetGameName().c_str(),
                     MB_YESNO) == IDNO)
      return false;
  }
#else
  // There are fairly deliberately no non-Win32 mutex, diskspace, CD protection
  // checks. If they're needed on a particular platform, they should be added
  // here.
#endif

  // Load modules
  theFlightRecorder.Log(FLIGHT_STARTUP, "Loading modules");
  std::string errorFile;
  if (!ModuleImporter::LoadModules(errorFile)) {
    ErrorMessageHandler::Show("module_error", 0, "App::Init",
                              errorFile.c_str());
    return false;
  }

  // Load syntax
  theFlightRecorder.Log(FLIGHT_STARTUP, "Loading syntax tables");
  theCAOSDescription.LoadDefaultTables();
  // From modules
  ModuleImporter::ResetIterator();
  while (ModuleInterface *moduleInterface = ModuleImporter::NextModule()) {
    moduleInterface->AddCAOSCommands(theCAOSDescription);
  }

  // Serialise out caos.syntax for CAOS tool to load in
  theFlightRecorder.Log(FLIGHT_STARTUP, "Making syntax file for CAOS tool");
  std::string syntaxfile;

  syntaxfile = theDirectoryManager.GetDirectory(MAIN_DIR, true) + "caos.syntax";
  if (!theCAOSDescription.SaveSyntax(syntaxfile)) {
    ErrorMessageHandler::Show("app_error", 1, "App::Init", syntaxfile.c_str());
  }

  CAOSMachine::InitialiseHandlerTables();

  std::string langid = GetLangCatalogue();

  // set flight recorder mask
  theFlightRecorder.Log(FLIGHT_STARTUP, "Flight recorder self reference ;-)");
  int mask = FLIGHT_RUNTIME +
             FLIGHT_NETBABEL; // default to runtime errors and netbabel
  UserSettings().Get("FlightRecorderMask", mask);
  theFlightRecorder.SetCategories(mask);

  // Set default Eame variables
  GetEameVar("engine_nudge_border_t").SetInteger(2);
  GetEameVar("engine_nudge_border_b").SetInteger(2);
  GetEameVar("engine_nudge_border_l").SetInteger(2);
  GetEameVar("engine_nudge_border_r").SetInteger(2);

  // Pray
  theFlightRecorder.Log(FLIGHT_STARTUP, "Setting up PRAY system");
  thePrayManager.SetLanguage(langid);
  AddBasicPrayDirectories();

  // Seed random number generators
  theFlightRecorder.Log(FLIGHT_STARTUP, "Seeding random number generators");
  srand(GetTimeStamp() + GetRealWorldTime() + 1);
  RandQD1::seed(GetTimeStamp() + GetRealWorldTime());

  // Call other initialisation functions
  theFlightRecorder.Log(FLIGHT_STARTUP, "Calling generic init functions");
  CallInitialisationFunctions();

  // set up the main view before we do anything in the world
  theFlightRecorder.Log(FLIGHT_STARTUP, "Setting up view");
  SetUpMainView();
  theMainView.Enable();
  CreateProgressBar();

  // Load in the world
  theFlightRecorder.Log(FLIGHT_STARTUP, "Loading Startup world");
  DoLoadWorld(std::string("Startup"));

  // Read values from standard game variables
  RefreshFromGameVariables();

  // this now has to be done after we know whether we use midi or not
  theFlightRecorder.Log(FLIGHT_STARTUP, "Setting up sound");
  SetUpSound();

  // Load catalogue files in
  theFlightRecorder.Log(FLIGHT_STARTUP, "Reinitialising catalogue files");
  InitLocalisation();

  return true;
}

void App::UpdateApp() {
  uint32 newStartStamp = GetTimeStamp();
  myLastTickGap = newStartStamp - myStartStamp;
  myStartStamp = newStartStamp;

  theMainView.MakeTheEntityHandlerResetBoundsProperly();

  // Perform world functions which are triggered by CAOS commands,
  // but have to be run in the main game loop rather than within
  // a virtual machine.

  if (myDisplaySettingsErrorNextTick) {
#ifndef C2E_SDL
    std::string s =
        theCatalogue.Get(theDisplayErrorTag, (int)sidDodgyPixelFormat2);
    theMainView.PrepareForMessageBox();
    ::MessageBox(NULL, s.c_str(), theApp.GetGameName().c_str(),
                 MB_OK | MB_ICONSTOP);
    theMainView.EndMessageBox();
#else
    // TODO: implement non-directx version
#endif
    myDisplaySettingsErrorNextTick = false;
  }

  if (myMovedFlag) {
    internalWindowHasMoved();
    myMovedFlag = false;
  }
  if (myResizedFlag) {
    internalWindowHasResized();
    myResizedFlag = false;
  }
  if (myDelayedMovingFlag) {
    myDelayedMovingFlag = false;
    myMovedFlag = true;
    internalWindowHasMoved();
  }
  if (myDelayedResizeFlag) {
    myDelayedResizeFlag = false;
    myResizedFlag = true;
    internalWindowHasResized();
  }

  if (mySaveNextTick) {
    myWorld->Save();
    mySaveNextTick = false;
  }
  if (myQuitNextTick) {
    theFlightRecorder.Log(FLIGHT_SHUTDOWN, "Signalling termination...\n");
    SignalTerminateApplication();
    myQuitNextTick = false;
  }

  if (!myLoadThisWorldNextTick.empty()) {
    ASSERT(myWorld);
    DoLoadWorld(myLoadThisWorldNextTick);
    myLoadThisWorldNextTick = "";
  }

  // Look for global input
  HandleInput();

  // Increase system tick
  mySystemTick++;

#ifdef _WIN32
  // Check for screensaver startup being over
  if (IsAppAScreenSaver() && mySystemTick == 10) {
    Win32ScreenKiller::Stop();
  }
#endif

  // Update all modules
  ModuleImporter::ResetIterator();
  while (ModuleInterface *moduleInterface = ModuleImporter::NextModule()) {
    moduleInterface->Update(mySystemTick);
  }

  myWorld->TaskSwitcher();

  if (myRenderDisplay || myRenderDisplayNextTick) {
    theMainView.Render();
    myRenderDisplayNextTick = false;
  }

  // Find time tick took
  uint32 endStamp = GetTimeStamp();
  uint32 tickLength = endStamp - myStartStamp;
  ++myRecentTickPos;
  if (myRecentTickPos >= ourTickLengthsAgo)
    myRecentTickPos = 0;
  myRecentTickLengths[myRecentTickPos] = tickLength;
}

/////////////////////////////////////////////////////////////////////////////

void App::ShutDown() {
  theFlightRecorder.Log(FLIGHT_SHUTDOWN, "App Shutting down...");

  // tell the camera to stop running
  // (before world object disappears!)
  theMainView.ShutDown();

  if (theMusicManager) {
    delete theMusicManager;
    theMusicManager = NULL;
  }

  if (theMusicSoundManager) {
    delete theMusicSoundManager;
    theMusicSoundManager = NULL;
  }

  if (theSoundManager) {
    delete theSoundManager;
    theSoundManager = NULL;
  }

  if (myProgressBar) {
    delete myProgressBar;
  }

  if (myWorld) {
    delete myWorld;
    myWorld = NULL;
  }

  SharedGallery::theSharedGallery().DestroyGalleries();

#ifndef C2E_SDL
  // Provides mutual exclusion with itself / any emergency kits etc.
  if (myMutex) {
    ReleaseMutex(myMutex);
    CloseHandle(myMutex);
  }
#else
  // TODO: non-win32 version
#endif

  // Shutdown all modules
  ModuleImporter::ResetIterator();
  while (ModuleInterface *moduleInterface = ModuleImporter::NextModule()) {
    moduleInterface->Shutdown();
  }

  theFlightRecorder.Log(FLIGHT_SHUTDOWN, "TheApp has shutdown...\n");
}

// ----------------------------------------------------------------------
// Method:      SetUpMainView
// Arguments:   None
//
// Returns:     None
// Description: initializes the mainview with the background.
// ----------------------------------------------------------------------
void App::SetUpMainView() {
  // no need to send the full path to the background
  // anymore

  // get the camera up and running
  bool fullscreen = true;
  int inthack = 1;
  UserSettings().Get("FullScreen", inthack);
  fullscreen = (inthack != 0);

  if (IsScreenSaver || IsScreenSaverPreview)
    fullscreen = true;

  // This needs to be moved to whichever object
  // wants control of the cameras and resetting
  // meta rooms
  // but for now...

  // NOTE: This appears also in World::Init()!
  // - BenC 03Feb00

  std::string defaultBackground("GreatOutdoors");
  UserSettings().Get("Default Background", defaultBackground);

  // Has display type been previously determined?
  int registry_display_type = RGB_UNKNOWN;
  MachineSettings().Get("Display Type", registry_display_type);

  std::vector<std::string> pathsToImageFoldersToConvert;

  {
    for (int i = 0; i < theDirectoryManager.GetAuxDirCount(IMAGES_DIR); ++i) {
      std::string path = theDirectoryManager.GetAuxDir(IMAGES_DIR, i);
      pathsToImageFoldersToConvert.push_back(path);
    }
  }

  {
    for (int i = 0; i < theDirectoryManager.GetAuxDirCount(BACKGROUNDS_DIR);
         ++i) {
      std::string path = theDirectoryManager.GetAuxDir(BACKGROUNDS_DIR, i);
      pathsToImageFoldersToConvert.push_back(path);
    }
  }

  {
    for (int i = 0; i < theDirectoryManager.GetAuxDirCount(OVERLAYS_DIR); ++i) {
      std::string path = theDirectoryManager.GetAuxDir(OVERLAYS_DIR, i);
      pathsToImageFoldersToConvert.push_back(path);
    }
  }

  if (!theMainView.StartUp(0, // world coordinates of view
                           0,
                           fullscreen,        // fullscreen or windowed?
                           defaultBackground, // default background file
                           0,                 // world coordinate of
                           0,                 //  top left corner of background
                           registry_display_type,
                           pathsToImageFoldersToConvert)) {
    theMainView.Disable();
  } else {
    theMainView.MakeMapImage();
    DisableMapImage();
  }
}

// ----------------------------------------------------------------------
// Method:      EnableMainView
// Arguments:   None
//
// Returns:     None
//
// Description: Restarts the display engine
//
// ----------------------------------------------------------------------
void App::EnableMainView() { theMainView.Enable(); }

// ----------------------------------------------------------------------
// Method:      DisableMainView
// Arguments:   None
//
// Returns:     None
//
// Description:Stops the display engine from trying to display
//
// ----------------------------------------------------------------------
void App::DisableMainView() { theMainView.Disable(); }
void App::EnableMapImage() { theMainView.EnableMapImage(); }

void App::DisableMapImage() { theMainView.DisableMapImage(); }

void App::WindowHasMoved() { myDelayedMovingFlag = true; }

void App::WindowHasResized() { myDelayedResizeFlag = true; }

void App::internalWindowHasResized() {
#ifdef C2E_SDL
  // just refresh the screen directly - don't bother mucking around
  // with window redraw messages...
  theMainView.Update(true, false);
#else
  theMainView.ResizeWindow();
#endif
  theAgentManager.ExecuteScriptOnAllAgents(
      SCRIPTWINDOWRESIZED, COASVARAGENTNULL, INTEGERZERO, INTEGERZERO);
}

void App::internalWindowHasMoved() { theMainView.MoveWindow(); }

void App::SetUpSound(void) {
  // Option to turn of fall sound
  int disable = 0;
  UserSettings().Get("DisableSound", disable);
  if (disable == 1) {
    theSoundManager = NULL;
    theMusicSoundManager = NULL;
    return;
  }

  // Now want two sound managers, allocated 1 meg each,
  // one for game sound, one for game music:
  try {
    // TODO: Fix Heisenbug.  If you remove the next line,
    // then with no sound under Linux/GCC you get a segfault.
    std::cout << "Initializing Sound" << std::endl;
    theSoundManager = new SoundManager;
  } catch (BasicException &e) {
    theSoundManager = NULL;
    theMusicSoundManager = NULL;

    ErrorMessageHandler::Show(e, std::string("App::SetUpSound"));
    return;
  }

  if (theSoundManager) {
    theSoundManager->InitializeCache(2 * 1024);
  }

  // don't go to the expense of creating the music manager
  // if it is not needed...

  if (IOnlyPlayMidiMusic == false) {
    try {
      theMusicSoundManager = new SoundManager;
    } catch (SoundManager::SoundException &e) {
      theMusicSoundManager = NULL;

      ErrorMessageHandler::Show(e, std::string("App::SetUpSound"));
      return;
    }

    if (theMusicSoundManager) {
      theMusicSoundManager->InitializeCache(2 * 1024);
    } else {
      // no music message?
      return;
    }

    // Create the global music manager
    theMusicManager = new MusicManager;

    if (!theMusicManager) {
      // message...
      return;
    }

    // Now try to load the encrypted script into the music manager
    if (!theMusicManager->LoadScrambled()) {
      // Unsuccessful
      // **** Should report the error in some way here ?

      // Dispose of the music manager
      delete theMusicManager;
      theMusicManager = NULL;
    }

    // !!! no thread is used for now if C3 decided that one is needed later
    // then then reinstate UpdateThread
    /* Audio thread handle elsewhere

    // Begin a thread to update the sound / music manager
    // Create a running thread

    // PC 18/6/98 - changed priority to normal

    audioThread = AfxBeginThread( AudioThreadUpdate,
                                                              NULL,
                                                              THREAD_PRIORITY_NORMAL,
                                                              0,
                                                              0,
                                                              NULL); */

    if (theMusicManager) {
      theMusicManager->Play();
    }
  }
}

// ----------------------------------------------------------------------
// Method:      ChangeResolution
// Arguments:   None
//
// Returns:     None
//
// Description: Finds out whether the requested display mode is viable and
//				switches to it.  For smoother results change the
// size
// of 				window too. Note that the display modes are
// enumrated at 				start up of the display engine
//
// ----------------------------------------------------------------------
void App::ChangeResolution() {
#ifdef C2E_SDL

  DisplayEngine::theRenderer().NextScreenMode(); // in sdl version only
  internalWindowHasResized();

#else // original directx version

  // just a test cycle through resolutions
  // each application would have a way
  // of allowing the user to select a resolution
  static int32 res[3][2] = {640, 480, 800, 600, 1024, 768};
  static int x = 0;

  theMainView.ChangeDisplayMode(res[x][0], res[x][1]);
  internalWindowHasResized();
  x++;

  if (x > 2)
    x = 0;
#endif
}

// ----------------------------------------------------------------------
// Method:      ToggleFullScreenMode
// Arguments:   None
//
// Returns:     true if changed OK
//				false otherwise
//
// Description: On each request it flips the engine between fullscreen
//				and windowed mode.
//
// ----------------------------------------------------------------------
void App::ToggleFullScreenMode() {
  if (theMainView.ToggleFullScreenMode()) {
    theAgentManager.ExecuteScriptOnAllAgents(
        SCRIPTWINDOWRESIZED, COASVARAGENTNULL, INTEGERZERO, INTEGERZERO);
  } else {
    DisplaySettingsErrorNextTick();
  }
}

// For language tags, we use values in language.cfg if available,
// otherwise we use whatever is in User.cfg

std::string App::GetLangCLib() {
  Configurator languageConfig;
  languageConfig.BindToFile("language.cfg");

  std::string langCLibrary = "english"; // default to English, ugh!
  if (languageConfig.Exists("LanguageCLibrary"))
    languageConfig.Get("LanguageCLibrary", langCLibrary);
  else
    UserSettings().Get("LanguageCLibrary", langCLibrary);

  return langCLibrary;
}

std::string App::GetLangCatalogue() {
  Configurator languageConfig;
  languageConfig.BindToFile("language.cfg");

  std::string langid = "en"; // default to English, ugh!
  if (languageConfig.Exists("Language"))
    languageConfig.Get("Language", langid);
  else
    UserSettings().Get("Language", langid);

  return langid;
}

bool App::InitLocalisation() {
  std::string langCLibrary = GetLangCLib();
  std::string langid = GetLangCatalogue();

  // set our locale
  // just time and date, as other stuff messes up parsers
  setlocale(LC_TIME, langCLibrary.c_str());

  // read in set of catalogue files for language
  theCatalogue.Clear();
  try {
    int count = theDirectoryManager.GetAuxDirCount(CATALOGUE_DIR);
    for (int i = 0; i < count; ++i) {
      std::string catpath = theDirectoryManager.GetAuxDir(CATALOGUE_DIR, i);

      Catalogue localCatalogue;
      localCatalogue.AddDir(catpath, langid);
      // Merge them in to check for bad clashes.  Two tags
      // can go in twice, as long as the initial parts
      // contain the same data
      theCatalogue.Merge(localCatalogue);
    }
  } catch (Catalogue::Err &e) {
    ErrorMessageHandler::Show(e, "App::InitLocalisation");
    return false;
  }

  if (!SensoryFaculty::SetupStaticVariablesFromCatalogue())
    return false;

  if (!Vocab::SetupCatalogueVector())
    return false;

  InitBrainMappingsFromCatalogues();
  // Add the Catalogue Entries for the pray manager....
  std::string prayCatalogueName = "Pray System File Extensions";
  if (theCatalogue.TagPresent(prayCatalogueName)) {
    thePrayManager.ClearChunkFileExtensionList();
    thePrayManager.GarbageCollect(true);
    for (int i = 0; i < theCatalogue.GetArrayCountForTag(prayCatalogueName);
         i++) {

      std::string anExtension = theCatalogue.Get(prayCatalogueName, i);
      thePrayManager.AddChunkFileExtension(anExtension);
    }
    thePrayManager.RescanFolders();
  }

  InitLocalCatalogueFilesFromTheWorldsDirectory();

  return true;
}

bool App::InitLocalCatalogueFilesFromTheWorldsDirectory() {
  std::string langid = GetLangCatalogue();
  try {
    std::string catpath;

    // also get the local catalogue files
    if (theDirectoryManager.GetWorldDirectoryVersion(CATALOGUE_DIR, catpath)) {
      theCatalogue.AddDir(catpath, langid);
    }
  } catch (Catalogue::Err &e) {
    ErrorMessageHandler::Show(e, "App::InitLocalisation");
    return false;
  }

  return true;
}

bool App::CreateNewWorld(std::string &worldName) {
  bool ret;
  std::cout << "Creating new world: " << worldName << std::endl;
  FilePath path(worldName, WORLDS_DIR);
  //	MessageBox(NULL,path.GetFullPath().c_str(),"",MB_OK);
  ret = CreateDirectory(path.GetFullPath().c_str(), NULL) ? true : false;

  return ret;
}

#ifndef C2E_SDL
// shouldn't this be in window.cpp?
void App::BeginWaitCursor() {
  // set the wait cursor

  myCursor = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));

  ShowCursor(true);
}

void App::EndWaitCursor() {
  // set the cursor back to whatever it was
  SetCursor(myCursor);
}
#endif

void App::HandleInput() {
  // scan the pending input events
  for (int i = 0; i < myInputManager.GetEventCount(); ++i) {
    const InputEvent *ev = &myInputManager.GetEvent(i);

    if (ev->EventCode == InputEvent::eventTranslatedChar) {
      int key = ev->KeyData.keycode;

      if (myInputManager.GetTranslatedCharTarget()) {
        myInputManager.GetTranslatedCharTarget()->SendChar(key);
      }
    }

    if (ev->EventCode == InputEvent::eventKeyDown) {
      int key = ev->KeyData.keycode;

      if (myInputManager.GetTranslatedCharTarget()) {
        myInputManager.GetTranslatedCharTarget()->RawKey(key);
      }

      // deal with debug key presses
      if (DebugKeyNow()) {
        // VK_PAUSE is handled in MainWindowProc, so it works
        // even when the game is debug paused (to unpause it)
        // Similarly VK_SPACE
        if (key == VK_INSERT)
          theMainView.ToggleMapImage();
        else if (key == VK_PRIOR)
          GetWorld().GetPreviousMetaRoom();
        else if (key == VK_NEXT)
          GetWorld().GetNextMetaRoom();
#ifndef C2E_SDL
        // sdl version does screenmode changes from main msg loop
        else if (key == VK_HOME)
          ChangeResolution();
#endif
        else if (key == VK_DELETE)
          myShouldHighlightAgentsKnownToCreatureFlag =
              !myShouldHighlightAgentsKnownToCreatureFlag;
        else if (key == VK_END)
          myShouldSkeletonsAnimateDoubleSpeedFlag =
              !myShouldSkeletonsAnimateDoubleSpeedFlag;
      } else if (DebugKeyNowNoShift()) {
        if (key == VK_NUMPAD0 || key == VK_NUMPAD1 || key == VK_NUMPAD2 ||
            key == VK_NUMPAD3 || key == VK_NUMPAD4 || key == VK_NUMPAD5 ||
            key == VK_NUMPAD6)
          SetWhichCreaturePermissionToHighlight(key == VK_NUMPAD0   ? 1
                                                : key == VK_NUMPAD1 ? 2
                                                : key == VK_NUMPAD2 ? 4
                                                : key == VK_NUMPAD3 ? 8
                                                : key == VK_NUMPAD4 ? 16
                                                : key == VK_NUMPAD5 ? 32
                                                                    : 0);
      }
    }
  }
}

// called also when the game is reloaded
void App::DoRefreshFromGameVariables() {
  // initialse some variables from the world

#ifdef C2D_DIRECT_DISPLAY_LIB
  CAOSVar scroll = theApp.GetWorld().GetGameVar("engine_DoWackyScroll");

  if (scroll.GetType() == CAOSVar::typeInteger) {
    _ASSERT(scroll.GetType() == CAOSVar::typeInteger);
    theMainView.DoWackyScroll(scroll.GetInteger() == 0 ? false : true);
  }
#endif

  CAOSVar &planeForLines = GetWorld().GetGameVar("engine_plane_for_lines");
  if (planeForLines.GetType() == CAOSVar::typeInteger)
    myPlaneForConnectiveAgentLines =
        planeForLines.GetInteger() == 0 ? 9998 : planeForLines.GetInteger();

  CAOSVar &soundLevels =
      GetWorld().GetGameVar("engine_playAllSoundsAtMaximumLevel");
  ASSERT(soundLevels.GetType() == CAOSVar::typeInteger);

  myPlayAllSoundsAtMaximumLevel = soundLevels.GetInteger() == 0 ? false : true;

  CAOSVar &skeletonUpdate =
      GetWorld().GetGameVar("engine_SkeletonUpdateDoubleSpeed");
  ASSERT(skeletonUpdate.GetType() == CAOSVar::typeInteger);
  // default is false
  myShouldSkeletonsAnimateDoubleSpeedFlag =
      skeletonUpdate.GetInteger() == 0 ? false : true;

  CAOSVar &midi = GetWorld().GetGameVar("engine_usemidimusicsystem");
  ASSERT(midi.GetType() == CAOSVar::typeInteger);
  IOnlyPlayMidiMusic = midi.GetInteger() == 0 ? false : true;

  CAOSVar &HH = GetWorld().GetGameVar("engine_creature_pickup_status");
  if (HH.GetType() == CAOSVar::typeInteger) // Has to be an integer
  {
    myCreaturePickupStatus = HH.GetInteger();
  }

  CAOSVar &CDBW =
      GetWorld().GetGameVar("engine_distance_before_port_line_warns");
  if (CDBW.GetType() == CAOSVar::typeFloat) {
    myMaximumDistanceBeforePortLineWarns = CDBW.GetFloat();
  }
  CAOSVar &CDBS =
      GetWorld().GetGameVar("engine_distance_before_port_line_snaps");
  if (CDBS.GetType() == CAOSVar::typeFloat) {
    myMaximumDistanceBeforePortLineSnaps = CDBS.GetFloat();
  }
}

float App::GetTickRateFactor() {
  ASSERT(myRecentTickLengths.size() == ourTickLengthsAgo);

  uint32 totalTime = 0;
  for (int i = 0; i < ourTickLengthsAgo; ++i)
    totalTime += myRecentTickLengths[i];

  float averageTickLength = (float)totalTime / (float)ourTickLengthsAgo;
  float factor = averageTickLength / (float)GetWorldTickInterval();

  return factor;
}

bool App::CreateProgressBar() {

#ifdef C2D_DIRECT_DISPLAY_LIB

#else
  if (myProgressBar)
    return true;

  if (theCatalogue.TagPresent("progress_indicators")) {
    try {
      // sprite file
      std::string spriteFilename = theCatalogue.Get("progress_indicators", 1);
      FilePath galleryName(spriteFilename, IMAGES_DIR);
      myProgressBar = new EntityImage(galleryName, 0, 1000, 0, 0, 0, 0);
      myProgressBar->Unlink();

      return true;
    } catch (BasicException &e) {
      ErrorMessageHandler::Show(e, "App::CreateProgressBar");
      return false;
    }
  }

#endif
  return false;
}

void App::StartProgressBar(int catOffset) {
#ifndef C2D_DIRECT_DISPLAY_LIB

  // should this stuff really be in App?

  if (myProgressBar && !myWorld->IsStartUpWorld() &&
      !DisplayEngine::theRenderer().ProgressBarAlreadyStarted()) {
    theMainView.SetLoading(false);
    // background file, if available
    std::string progressBackgroundName =
        theCatalogue.Get("progress_indicators", 0);
    if (!progressBackgroundName.empty()) {
      RECT progressBackground;
      progressBackground.left = 0;
      progressBackground.right = 800;
      progressBackground.top = 0;
      progressBackground.bottom = 600;
      theMainView.ChangeMetaRoom(progressBackgroundName, progressBackground, 0,
                                 0, 0, 0);
    }
    theMainView.SetLoading(true);

    // popup dialog
    int32 popposx;
    int32 popposy;
    {
      // set pose
      myProgressBar->SetPose(
          atoi(theCatalogue.Get("progress_indicators", catOffset + 3)));
      Bitmap *bitmap = myProgressBar->GetCurrentBitmap();
      // centre on screen

      popposx = atoi(theCatalogue.Get("progress_indicators", catOffset + 1));
      popposy = atoi(theCatalogue.Get("progress_indicators", catOffset + 2));

      if (popposx == -1)
        popposx = (DisplayEngine::theRenderer().GetSurfaceWidth() -
                   bitmap->GetWidth()) /
                  2;
      if (popposy == -1)
        popposy = (DisplayEngine::theRenderer().GetSurfaceHeight() -
                   bitmap->GetHeight()) /
                  2;

      DisplayEngine::theRenderer().ClientToScreen(popposx, popposy);
      bitmap->SetPosition(Position(popposx, popposy));
      // render it
      bool ok = DisplayEngine::theRenderer().RenderBitmapToFrontBuffer(bitmap);
    }

    // progress bar pose
    myProgressBar->SetPose(
        atoi(theCatalogue.Get("progress_indicators", catOffset)));
    Bitmap *bitmap = myProgressBar->GetCurrentBitmap();
    // offset of progress indicator relative to popup dialog
    int32 barposx =
        popposx + atoi(theCatalogue.Get("progress_indicators", catOffset + 4));
    int32 barposy =
        popposy + atoi(theCatalogue.Get("progress_indicators", catOffset + 5));
    bitmap->SetPosition(Position(barposx, barposy));
    // tell display engine about it
    bool ok = DisplayEngine::theRenderer().CreateProgressBar(bitmap);
  }
#endif // C2D_DIRECT_DISPLAY_LIB
}

void App::SpecifyProgressIntervals(int updateIntervals) {
#ifndef C2D_DIRECT_DISPLAY_LIB
  if (myProgressBar && !myWorld->IsStartUpWorld())
    DisplayEngine::theRenderer().StartProgressBar(updateIntervals);
#endif // C2D_DIRECT_DISPLAY_LIB
}

void App::UpdateProgressBar(int amount /* = 1 */) {
#ifndef C2D_DIRECT_DISPLAY_LIB
  if (theMainView.GetLoading() && myProgressBar && !myWorld->IsStartUpWorld()) {
    DisplayEngine::theRenderer().UpdateProgressBar(amount);
  }
#endif // C2D_DIRECT_DISPLAY_LIB
}

void App::EndProgressBar() {
#ifndef C2D_DIRECT_DISPLAY_LIB
  theMainView.SetLoading(false);
  DisplayEngine::theRenderer().EndProgressBar();
#endif // C2D_DIRECT_DISPLAY_LIB
}

void App::SetPassword(std::string &password) {
  myPasswordForNextWorldLoaded = password;
  myIHaveAPassword = true;
}

std::string App::GetPassword() { return myPasswordForNextWorldLoaded; }

bool App::DoINeedToGetPassword() {
  bool temp = myIHaveAPassword;
  myIHaveAPassword = false;
  return temp;
}

void App::RefreshFromGameVariables() {
  DoRefreshFromGameVariables();
  if (myWorld)
    myWorld->DoRefreshFromGameVariables();
}

void App::DoLoadWorld(const std::string &worldName) {
#ifdef _WIN32
  // Sigh... Hardcodingtastic.  Unfortunately, we can't
  // parameterise this, as then people would hack it
  // too easily.
/*	// SPARKY
        if (myGameName == "Docking Station")
        {
                // If we are loading a Creatures 3 world
                if (worldName != "Startup" &&
   GetEameVar("engine_no_auxiliary_bootstrap_1").GetInteger() == 0)
                {
                        while (!CheckForCD())
                        {
                                std::string message =
   theCatalogue.Get("ds_c3_app_cd_needed", 0); if (::MessageBox(theMainWindow,
   message.c_str(), GetGameName().c_str(), MB_SYSTEMMODAL | MB_RETRYCANCEL) ==
   IDCANCEL)
                                {
                                        HANDLE hProcess = GetCurrentProcess();
                                        TerminateProcess(hProcess, -1);
                                }
                        }
                }
        }*/
#endif

  // try loading main world
  if (myWorld)
    delete myWorld;
  myWorld = new World;
  myWorld->Init();
  if (!myWorld->Load(worldName, false)) {
    // failed to load main world, try backup
    if (myWorld)
      delete myWorld;
    myWorld = new World;
    myWorld->Init();
    myWorld->Load(worldName, true);
  }
}

bool App::ProcessCommandLine(std::string commandLine) {
  theFlightRecorder.Log(FLIGHT_STARTUP, "Processing command line\n");

#ifdef _WIN32
  // Screensaver.  Note that it can be launched directory (with engine.exe
  // renamed to GameName.scr) or indirectly (via ScreenStub.exe) with the
  // screensaver command line switches.

  // Process screensaver command line
  if (commandLine.compare(0, 2, "/s") == 0 ||
      commandLine.compare(0, 2, "-s") == 0 ||
      commandLine.compare(0, 1, "s") == 0) {
    std::string path, appName, extension;
    GetAppDetails(&path, &appName, &extension);
    if (!stricmp(extension.c_str(), "scr")) // SPARKY was strcmpi
      SetGameName(appName);
    else if (GetGameName().empty())
      // Note: find_last_of(" ") isn't quite correct, as it won't cope with
      // game names with spaces in.  This is a nasty hack at the last minute
      // for Sea Monkeys, so you can come and hit me if you like. - Francis
      SetGameName(commandLine.substr(commandLine.find_last_of(" ") + 1,
                                     commandLine.size()));
    IsScreenSaver = true;
    GetEameVar("engine_screensaver").SetInteger(1);

    HANDLE pseudoThis = GetCurrentThread();
    HANDLE realThis;
    if (!DuplicateHandle(GetCurrentProcess(), pseudoThis, GetCurrentProcess(),
                         &realThis, 0, FALSE, 0))
      ASSERT(false);
    Win32ScreenKiller::Start(realThis);

    return true;
  }
  // Process screensaver preview command line
  if (commandLine.compare(0, 2, "/p") == 0 ||
      commandLine.compare(0, 2, "-p") == 0 ||
      commandLine.compare(0, 1, "p") == 0) {
    std::string path, appName, extension;
    GetAppDetails(&path, &appName, &extension);
    if (!stricmp(extension.c_str(), "scr")) // SPARKY was strcmpi
      SetGameName(appName);
    else if (GetGameName().empty())
      SetGameName(commandLine.substr(commandLine.find_last_of(" ") + 1,
                                     commandLine.size()));
    IsScreenSaverPreview = true;
    GetEameVar("engine_scrpreview").SetInteger(1);

    /*
    char hWndStr[5];
    strcpy(hWndStr, commandLine.substr(commandLine.find('p') + 2, 4).c_str());
    myPreviewWindowHandle = (HWND)strtoul(hWndStr, NULL, 10);
    */

    return false;
  }
  // Process screensaver configure command line
  if (commandLine.compare(0, 2, "/c") == 0 ||
      commandLine.compare(0, 2, "-c") == 0 ||
      commandLine.compare(0, 1, "c") == 0) {
    std::string path, appName, extension;
    GetAppDetails(&path, &appName, &extension);
    if (!stricmp(extension.c_str(), "scr")) // SPARKY was strcmpi
      SetGameName(appName);
    else if (GetGameName().empty())
      SetGameName(commandLine.substr(commandLine.find_last_of(" ") + 1,
                                     commandLine.size()));
    IsScreenSaverConfig = true;
    GetEameVar("engine_scrconfig").SetInteger(1);
    return true;
  }
#endif

  // Check for both with and without a space (now you
  // can launch without specifying the game name at the
  // command line)
  if (commandLine.substr(0, 11) == "--autokill ") {
    myAutoKillAgentsOnError = true;
    commandLine = commandLine.substr(11);
  }
  if (commandLine.substr(0, 10) == "--autokill") {
    myAutoKillAgentsOnError = true;
    commandLine = commandLine.substr(10);
  }

  // Get the game name if any from the command line
  SetGameName(commandLine);

  return true;
}

#ifdef _WIN32
// Get the application path name and extension from the command line
bool App::GetAppDetails(std::string *path, std::string *fileName,
                        std::string *extension) {
  std::string commandLine, appName;

  commandLine = GetCommandLine();

  // Get the full path and filename from the command line
  int pathEnd = commandLine.find(".");
  appName = commandLine.substr(0, pathEnd + 4);
  if (appName[0] == '\"')
    appName = appName.substr(1, appName.size() - 1);

  *path = appName.substr(0, appName.rfind('\\'));

  // Make into a long filename (we don't use GetLongPathName
  // as it doesn't work on Windows 95 or Windows NT)
  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile(appName.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    ::FindClose(hFind);
    appName = fd.cFileName;
  } else {
    ASSERT(false);
    return false;
  }

  // Remove the path from the filename and extension
  *extension = appName.substr(appName.rfind('.') + 1, appName.size());
  *fileName = appName.substr(appName.rfind('\\') + 1,
                             appName.rfind('.') - (appName.rfind('\\') + 1));

  return true;
}
#endif

// Now for the engine game variable documentation tables.....

TableSpec ourGameVariables[] = {
    TableSpec("Game Variables"),
    TableSpec("Variable (for @#GAME@)", "Expected Type", "Default",
              "Description"),

    TableSpec("@engine_debug_keys@", "Integer", "1",
              "This determines if the debug keys are operative. If the value "
              "is <b>1</b> then the keys described in the @#Debug Keys@ table "
              "are functional.  For non-numeric keypad keys, you must hold "
              "Shift down with the key."),
    TableSpec("@engine_full_screen_toggle@", "Integer", "1",
              "Alt+Enter usually toggles full screen mode.  Set to 0 to "
              "disable this."),
    TableSpec("@engine_SkeletonUpdateDoubleSpeed@", "Integer", "0",
              "If non-zero, skeleton updates happen each tick. If Zero, they "
              "happen every other tick (default)"),
    TableSpec("@engine_creature_pickup_status@", "Integer", "0",
              "If zero, right click on a creature is pickup.  If one, right "
              "click is holding hands.  If two, hold shift to pickup, don't "
              "hold shift to hold hands.  If three, as two but creature must "
              "be a selectable one as according to the Grettin game variable."),
    TableSpec("@engine_dumb_creatures@", "Integer", "0",
              "If non-zero, creatures do not burble."),
    TableSpec("@engine_pointerCanCarryObjectsBetweenMetaRooms@", "Integer", "0",
              "If zero, the pointer drops objects when a metaroom change "
              "occurs. If non-zero, then it continues to carry them allowing "
              "objects to be moved between metarooms by the pointer."),
    TableSpec(
        "@engine_password@", "String", "&lt;NONE&gt;",
        "If set, this allows the determining of the password for the world. "
        "This does not set anything other than the pswd returned - enabling "
        "world-switchers to correctly deal with the passwords."),
    TableSpec("@engine_creature_template_size_in_mb@", "Integer", "1",
              "This is the amount of data space allocated to the creature "
              "gallery sprite files. If a single life stage image set exceeds "
              "this value - problems will ensue"),
    TableSpec(
        "@engine_near_death_track_name@", "String", "&lt;NONE&gt;",
        "This sets the track (and optionally MNG file) for the track which the "
        "engine will play when a creature gets near death within the game."),
    TableSpec("@engine_plane_for_lines@", "Integer", "9998",
              "This sets the plane for lines drawn on entity images. This "
              "includes Connective agent lines and the debug cabins"),
    TableSpec("@engine_synchronous_learning@", "Integer", "0",
              "By default learning is asynchronous, so the creature learns "
              "from all @#STIM@s.  Set to 1 to make the creatures learn only "
              "from @#STIM@s caused by the action they are thinking about "
              "carried out by the agent their attention is on."),
    TableSpec("@engine_zlib_compression@", "Integer", "6",
              "Saved worlds and other archives are compressed using zlib.  "
              "This sets the compression level. The value ranges between: 0 - "
              "No compression, 1 - Best speed, 9 - Best compression."),
    TableSpec("@engine_other_world@", "String", "&lt;NONE&gt;",
              "Used by @#FILE OOPE@ to allow saving to other world's journal "
              "directories."),
    TableSpec("@engine_netbabel_save_passwords@", "Integer", "0",
              "Set to 1 to tell the netbabel module to save passwords in "
              "user.cfg, 0 for it to forget them."),

    TableSpec(), // Birth deserves a seperate table
    TableSpec("@engine_multiple_birth_first_chance@", "Float", "0.0",
              "This sets the chance (from 0 to 1) that a mate event will "
              "result in multiple pregnancies"),
    TableSpec("@engine_multiple_birth_subsequent_chance@", "Float", "0.0",
              "This sets the subsequent chance (from 0 to 1) that more than "
              "two children are conceived"),
    TableSpec("@engine_multiple_birth_maximum@", "Integer", "1",
              "This sets the maximum number of births allowed by the engine"),
    TableSpec(
        "@engine_multiple_birth_identical_chance@", "Float", "0.5",
        "This sets the chance that the children will be genetically identical"),

    TableSpec(), // Time specs
    TableSpec("@engine_LengthOfDayInMinutes@", "Integer", "0",
              "This sets the number of minutes in a day. (Game time)"),
    TableSpec("@engine_LengthOfSeasonInDays@", "Integer", "0",
              "This sets the number of days in a season. (Game time)"),
    TableSpec("@engine_NumberOfSeasons@", "Integer", "0",
              "This sets the number of seasons in a year. (Game Time)"),

    TableSpec(), // Volume and sound Specs
    TableSpec(
        "@engine_mute@", "Integer", "0",
        "This sets whether (non zero) or not (zero) to mute the MIDI player"),
    TableSpec("@engine_playAllSoundsAtMaximumLevel@", "Integer", "0",
              "If non-zero, all sounds played by agents are set to full volume "
              "and center pan."),
    TableSpec("@engine_usemidimusicsystem@", "Integer", "0",
              "If non-zero, this tells the engine to use MIDI music instead of "
              "the MUNGED data usually used."),

    TableSpec(), // Now the cav specifics
    TableSpec("@cav_birthdate@", "String", "&lt;NONE&gt;",
              "If set, the engine uses this to inform the birthday agent when "
              "it is the player's birthday."),
    TableSpec(
        "@cav_quittime@", "String", "&lt;NONE&gt;",
        "If set, the engine uses this to inform the countdown clock agent to "
        "show its countdown, and also to quit the game at the specified time."),
    TableSpec("@cav_gamelengthIsPerDay@", "String", "&lt;NONE&gt;",
              "If set, the engine uses this to inform the countdown clock "
              "agent when the maximum play time for the day is up."),
    TableSpec("@cav_useparentmenu@", "Integer", "0",
              "If non-zero, this causes the engine to deal with extra "
              "information such as the countdown clock and birthday agents. "
              "These are activated by registry settings."),
    TableSpec("@cav_CountdownClockAgent@", "Agent", "NULL",
              "If not null, countdown messages are sent to this agent."),
    TableSpec("@cav_BirthdayBannerAgent@", "Agent", "NULL",
              "If not null, birthday messages are sent to this agent."),

    TableSpec(),
    TableSpec(
        "@engine_distance_before_port_line_warns@", "Float", "600.0",
        "At this distance, port lines will pulse red not blue to indicate that "
        "they are nearning their maximum length before they snap"),
    TableSpec("@engine_distance_before_port_line_snaps@", "Float", "800.0",
              "At this distance, port lines will simply snap."),
};

TableSpec ourEameVariables[] = {
    TableSpec("Eame Variables"),
    TableSpec("Variable (for @#EAME@)", "Expected Type", "Default",
              "Description"),

    TableSpec(
        "@engine_no_auxiliary_bootstrap_nnn@", "Integer", "0",
        "Set to non-zero to prevent @#LOAD@ using that auxiliary Bootstrap "
        "directory for new worlds.  nnn can be 0 for the main Bootstrap "
        "directory, or the number of an auxiliary one.  The setting is stored "
        "for each world in files in the world's directory."),
    TableSpec("@engine_clone_upon_import@", "Integer", "0",
              "Set to 1 to force @#PRAY IMPO@ to clone all creatures."),
    TableSpec("@engine_screensaver@", "Integer", "0 or 1",
              "Returns 1 if the engine is in screen saver mode, 0 otherwise. "
              "This should treated as a read only game variable"),
    TableSpec("@engine_scrpreview@", "Integer", "0 or 1",
              "Returns 1 if the engine is in screen saver preview mode, 0 "
              "otherwise. This should treated as a read only game variable"),
    TableSpec("@engine_scrconfig@", "Integer", "0 or 1",
              "Returns 1 if the engine is in screen saver configuration mode, "
              "0 otherwise. This should treated as a read only game variable"),
    TableSpec("@engine_decouple_mouse@", "Integer", "0",
              "Set to 1 to disconnect the physical mouse from the @#PNTR@ "
              "agent.  You can then @#MVTO@ the pointer agent without the user "
              "getting in the way."),
    TableSpec(
        "@engine_nudge_border_t@", "Integer", "2",
        "Alters the top border for screen-edge nudge scrolling in full screen "
        "mode.  There are also engine_nudge_border_b, engine_nudge_border_l, "
        "engine_nudge_border_r for the bottom, left and right borders.  You "
        "can use negative values, and should do particularly if you have a "
        "pointer hotspot not at the top left corner of the pointer sprite.  "
        "See @#SCOL@ for related features."),
};

TableSpec ourDebugKeys[] = {
    TableSpec("Debug Keys"),
    TableSpec("Key", "Action"),

    TableSpec("Pause", "Performs a DEBUG pause on the game (no more ticking "
                       "messages are sent)"),
    TableSpec("Space", "Even when debug paused, this causes a tick to execute "
                       "- allowing single stepping of the world"),

    TableSpec(),
    TableSpec("Insert", "Turns the mapline display on or off"),
    TableSpec("Page Up", "Moves to the previous Metaroom"),
    TableSpec("Page Down", "Moves to the next Metaroom"),
    TableSpec("Home",
              "Cycles through the resolutions available to the display engine"),
    TableSpec("Delete", "Toggles the highlight box around agents which a "
                        "creature is paying attention to"),
    TableSpec(
        "End",
        "Toggles the skeleton update speed between normal and double speed"),

    TableSpec(),

    TableSpec("Numpad 0",
              "Sets creature permission highlights to CanActivate1"),
    TableSpec("Numpad 1",
              "Sets creature permission highlights to CanActivate2"),
    TableSpec("Numpad 2",
              "Sets creature permission highlights to CanDeactivate"),
    TableSpec("Numpad 3", "Sets creature permission highlights to CanHit"),
    TableSpec("Numpad 4", "Sets creature permission highlights to CanEat"),
    TableSpec("Numpad 5", "Sets creature permission highlights to CanPickUp"),
    TableSpec("Numpad 6", "Turns creature permission highlights off"),

};
int dummyAppOwnedTables =
    AutoDocumentationTable::RegisterTable(ourGameVariables,
                                          sizeof(ourGameVariables)) +
    AutoDocumentationTable::RegisterTable(ourDebugKeys, sizeof(ourDebugKeys)) +
    AutoDocumentationTable::RegisterTable(ourEameVariables,
                                          sizeof(ourEameVariables));

int App::EorWolfValues(int andMask, int eorMask) {
  int wolf = 0;

  if (myRenderDisplay)
    wolf += 1;
  if (myFastestTicks)
    wolf += 2;
  if (myRenderDisplayNextTick)
    wolf += 4;
  if (myAutoKillAgentsOnError)
    wolf += 8;

  wolf = (wolf & andMask) ^ eorMask;

  myRenderDisplay = (wolf & 1) == 1;
  myFastestTicks = (wolf & 2) == 2;

  myRenderDisplayNextTick = (wolf & 4) == 4;
  myAutoKillAgentsOnError = (wolf & 8) == 8;

  // In case autokill has changed
  ResetWindowTitle();
  return wolf;
}

bool App::DebugKeyNowNoShift() {
  return (GetWorld().GetGameVar("engine_debug_keys").GetInteger() == 1);
}

bool App::DebugKeyNow() {
  return DebugKeyNowNoShift() && GetInputManager().IsKeyDown(VK_SHIFT);
}

#ifdef _WIN32

bool App::CheckForCD() {
  // Look for msnope32.dll in the windows system directory
  char systemDirectory[MAX_PATH + 1];
  GetSystemDirectory(systemDirectory, MAX_PATH);
  std::string overrideFilename =
      std::string(systemDirectory) + "\\msnope32.dll";
  if (FileExists(overrideFilename.c_str())) {
    // if we have it, it's a developer overriding the protection
    return true;
  }

  // Otherwise look for a CD..
  // Get all drive names
  char allDrives[4096];
  GetLogicalDriveStrings(4095, allDrives);

  char *pDrive = allDrives;
  while (*pDrive) {
    // Check next drive for being a CD
    std::string driveName = pDrive;
    if (GetDriveType(driveName.c_str()) == DRIVE_CDROM) {
      // Look for Creatures 3 icon file on the CD
      std::string fileToFind;
      if (myGameName == "Sea-Monkeys")
        fileToFind = driveName + "Intro\\SMIntro.avi";

      // This is the Creatures 3 check, for historyical reference:
      // fileToFind = driveName + "Install\\Install\\Creatures 3.ico";

      if (FileExists(fileToFind.c_str())) {
        return true;
      }
    }

    // add on length, and the null
    pDrive += driveName.size();
    ++pDrive;
  }

  // Nothing found, reject
  return false;
}

bool App::CheckForMutex() {
  std::string mutex_name = GetGameName() + "_unique";
  myMutex = CreateMutex(NULL, TRUE, mutex_name.c_str());
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    ReleaseMutex(myMutex);
    CloseHandle(myMutex);
    return false;
  }
  return true;
}

bool App::CheckAllFreeDiskSpace(int expectedSystem, int expectedGeneral) {
  // Check system directory
  char systemDirectory[MAX_PATH + 1];
  GetSystemDirectory(systemDirectory, MAX_PATH);
  if (!CheckFreeDiskSpace(systemDirectory, expectedSystem))
    return false;

  // Check paths in main game directories
  // (auxiliary directories aren't ever written to)
  for (int i = 0; i < NUM_DIRS; ++i) {
    if (!CheckFreeDiskSpace(theDirectoryManager.GetDirectory(i),
                            expectedGeneral))
      return false;
  }

  return true;
}

bool App::CheckFreeDiskSpace(std::string path, int expected) {
  // Expand relative paths
  char buf[_MAX_PATH + 1];
  char *filePart;
  GetFullPathName(path.c_str(), _MAX_PATH, buf, &filePart);
  path = buf;

  if (path.size() < 3)
    return true;

  path = path.substr(0, 3);

  DWORD sectorsPerCluster = 0;
  DWORD bytesPerSector = 0;
  DWORD numberOfFreeClusters = 0;
  DWORD totalNumberOfClusters = 0;
  BOOL ok = GetDiskFreeSpace(path.c_str(), &sectorsPerCluster, &bytesPerSector,
                             &numberOfFreeClusters, &totalNumberOfClusters);
  if (ok) {
    unsigned int size =
        numberOfFreeClusters * sectorsPerCluster * bytesPerSector;
    if (size < expected)
      return false;
  }

  return true;
}

#endif // WIN32

std::string App::GenerateWindowTitle() {

  std::string title;
  // title="SPARKY";

  // Game name
  if (!GetGameName().empty())
    title += GetGameName() + std::string(" - ");

  // Engine version
  title += theCatalogue.Get("system_title", 1) + std::string(" ") +
           GetEngineVersion();

  // Build number if available
  if (theCatalogue.TagPresent("build_number"))
    title += std::string(" B") + theCatalogue.Get("build_number", 0);

  // Port number if available
  if (theCatalogue.TagPresent("port_number"))
    title += std::string(" P") + theCatalogue.Get("port_number", 0);

  // Reminder that autokill is turned on
  if (AutoKillAgentsOnError())
    title += std::string(" - ") + theCatalogue.Get("system_title", 2);

  return title;
}

std::string App::GetWarpOutgoingPath() {
  std::string nickname = GetNetworkNickname();
  LowerCase(nickname);
  if (GetNetworkNickname().empty())
    nickname = "None";

  std::string path =
      theDirectoryManager.GetDirectory(USERS_DIR) + nickname + PathSeparator();
  if (!FileExists(path.c_str()))
    CreateDirectory(path.c_str(), NULL);
  path += std::string("Warp Out") + PathSeparator();
  if (!FileExists(path.c_str()))
    CreateDirectory(path.c_str(), NULL);
  return path;
}

std::string App::GetWarpIncomingPath() {
  std::string nickname = GetNetworkNickname();
  LowerCase(nickname);
  if (GetNetworkNickname().empty())
    nickname = "None";

  std::string path =
      theDirectoryManager.GetDirectory(USERS_DIR) + nickname + PathSeparator();
  if (!FileExists(path.c_str()))
    CreateDirectory(path.c_str(), NULL);
  path += std::string("Warp In") + PathSeparator();
  if (!FileExists(path.c_str()))
    CreateDirectory(path.c_str(), NULL);
  return path;
}

void App::NotifyNewNickname(const std::string &nickName) {
  AddBasicPrayDirectories();

  NetworkInterface *networkInterface = ModuleImporter::GetNetworkInterface();
  if (networkInterface) {
    thePrayManager.AddDir(GetWarpIncomingPath());
    networkInterface->SetDirectories(GetWarpOutgoingPath(),
                                     GetWarpIncomingPath());
  }

  thePrayManager.RescanFolders();
}

void App::AddBasicPrayDirectories() {
  thePrayManager.ClearDirList();
  {
    for (int i = 0; i < theDirectoryManager.GetAuxDirCount(PRAYFILE_DIR); ++i) {
      thePrayManager.AddDir(theDirectoryManager.GetAuxDir(PRAYFILE_DIR, i));
    }
  }
  {
    for (int i = 0; i < theDirectoryManager.GetAuxDirCount(CREATURES_DIR);
         ++i) {
      thePrayManager.AddDir(theDirectoryManager.GetAuxDir(CREATURES_DIR, i));
    }
  }
}

// These provide callback to external modules, that need
// initialisation functions calling
int App::AddInitialisationFunction(InitialisationFunction function) {
  GetInitialisationFunctions().push_back(function);
  return 0;
}

std::vector<App::InitialisationFunction> &App::GetInitialisationFunctions() {
  static std::vector<InitialisationFunction> ourmyInitialisationFunctions;
  return ourmyInitialisationFunctions;
}

void App::CallInitialisationFunctions() {
  std::vector<InitialisationFunction> &InitialisationFunctions =
      GetInitialisationFunctions();
  for (int i = 0; i < InitialisationFunctions.size(); ++i) {
    InitialisationFunctions[i](*this);
  }
}

std::string App::GetNetworkUserId() {
  NetworkInterface *networkInterface = ModuleImporter::GetNetworkInterface();
  if (networkInterface)
    return networkInterface->GetUserId();
  else
    return "";
}

std::string App::GetNetworkNickname() {
  NetworkInterface *networkInterface = ModuleImporter::GetNetworkInterface();
  if (networkInterface)
    return networkInterface->GetNickname();
  else
    return "";
}

std::string App::GetWorldName() {
  std::string worldName("");

  if (myWorld)
    return myWorld->GetWorldName();

  return worldName;
}

bool App::InitConfigFiles() {
#ifdef _WIN32
  if (!GetGameName().empty()) {
    Registry registry;
    registry.ChooseDefaultKey(GetGameName());
    std::string value;
    if (registry.GetValue("Main Directory", value, HKEY_LOCAL_MACHINE))
      SetCurrentDirectory(value.c_str());
  }
#endif

  std::string machineConfig = "machine.cfg";
  std::string userConfig = "user.cfg";

  if (!myMachineSettings.BindToFile(machineConfig))
    return false;
  if (!myUserSettings.BindToFile(userConfig))
    return false;

#ifdef _WIN32
  // If either configuration file missing, then get settings from
  // registry instead...
  if (!FileExists(machineConfig.c_str()) || !FileExists(userConfig.c_str())) {
    Registry::MigrateFromRegistry();
  }

  // Get auxiliary directories from registry, if asked to do so
  Registry::MigrateAuxiliaryDirectoriesFromRegistry();
#endif

  // Read game name from configurator file
  SetGameName(myMachineSettings.Get("Game Name"));

#ifdef _WIN32
  // Put settings in registry, for tools to look at
  Registry::TransferToRegistryForTools();
#endif

  return true;
}

// static
int App::GetWorldTickInterval() { return myWorldTickInterval; }

// static
void App::SetWorldTickInterval(int worldTickInterval) {
  myWorldTickInterval = worldTickInterval;
}

bool App::IsAppAScreenSaver() { return IsScreenSaver; }

bool App::GetScreenSaverConfig() { return IsScreenSaverConfig; }

bool App::GetIsScreenSaverPreview() { return IsScreenSaverPreview; }

#ifdef _WIN32
HWND App::GetPreviewWindowHandle() { return myPreviewWindowHandle; }
#endif

CAOSVar &App::GetEameVar(const std::string &name) {
  if (name.empty())
    throw BasicException("Empty string EAME variables not allowed");
  return (myEameVars[name]);
}

std::string App::GetNextEameVar(const std::string &name) {
  if (name.empty() && myEameVars.size() > 0)
    return myEameVars.begin()->first;

  std::map<std::string, CAOSVar>::iterator it;
  it = myEameVars.find(name);
  if (it != myEameVars.end())
    it++;

  if (it == myEameVars.end())
    return "";
  else
    return it->first;
}

void App::DeleteEameVar(const std::string &name) {
  std::map<std::string, CAOSVar>::iterator it;
  it = myEameVars.find(name);
  if (it != myEameVars.end())
    myEameVars.erase(it);
}

CAOSVar &App::GetGameVar(const std::string &name) {
  return GetWorld().GetGameVar(name);
}

std::string App::GetDefaultMNG() {
  std::string defaultMNG = "music.mng";
  UserSettings().Get("Default Munge", defaultMNG);
  return defaultMNG;
}

bool App::IsFullScreen() { return theMainView.IsFullScreen(); }
