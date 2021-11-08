// -------------------------------------------------------------------------
// Filename:    SDL_Main.cpp
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "SDL_Main.h"
#include "../../InputManager.h"
#include "../ErrorMessageHandler.h"
#include "../DisplayEngine.h"
#include "../../Camera/MainCamera.h"
#include "../../World.h"
#include "../../App.h"
#include <exception>
#include "../../unix/KeyScan.h"
#ifdef _WIN32
	#include "../../../common/CStyleException.h"
	#include "../../RegistryHandler.h"
	#include "../../Caos/Win32Server.h"

	#include <SDL/SDL_syswm.h>
#else
	#include "../../../common/UnixSignals.h"
	#include "../../Caos/SockServer.h"
	#include "scrap.h"
#endif

#include <time.h>

#ifdef C2E_USING_GTK
	#include <gtk/gtk.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const unsigned int theClientServerBufferSize = 1024 * 1024;
#ifdef _WIN32
HWND theMainWindow = 0;
#endif

#define WHEEL_DELTA 120 /* Value for rolling one detent */

static bool ourRunning;
// static bool ourTerminateApplication;
static bool ourQuit;

static bool InitInstance();
static bool DoStartup();
static void DoShutdown();
static void HandleEvent(const SDL_Event &event);

static bool ourCurrentlyTicking = true;
static bool ourSingleTick = false;

#ifndef _WIN32
static SockServer *ourSockServer = NULL;
#endif

SDL_Event event;
int ticksSinceStart;

extern "C" int main(int argc, char *argv[]) {
  EM_ASM({ Module.logReadFiles = 'true'; });
  ticksSinceStart = 0;

  try {
    ourRunning = true;

    // Forward command line to app, converting
    // it to Windows command line format
    std::string commandLine;
    for (int i = 1; i < argc; ++i) {
      commandLine += argv[i];
      if (i != argc - 1)
        commandLine += " ";
    }
    if (!theApp.ProcessCommandLine(commandLine))
      return 0;

    if (!InitInstance()) {
      std::cout << "Unable to load..." << std::endl;
      return 0;
    }

    // Option to turn off all sound
    int disableSound = 0;
    theApp.UserSettings().Get("DisableSound", disableSound);

    // Initialize the SDL library (starts the event loop)
    // - we specify video and audio at once for DirectX
    if (SDL_Init((disableSound == 1) ? SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE
                                     : SDL_INIT_VIDEO | SDL_INIT_AUDIO |
                                           SDL_INIT_NOPARACHUTE) < 0) {
      std::cerr << "Failed to initialise SDL " << std::endl;
      std::cerr << SDL_GetError() << std::endl;
      return 1;
    }
    SDL_ShowCursor(0);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // Icon for Window managers that cope with it
    // std::string iconFilename;
    // theApp.UserSettings().Get("Icon", iconFilename);
    // SDL_Surface *icon = SDL_LoadBMP(iconFilename.c_str());
    // if (icon) {
    //   SDL_WM_SetIcon(icon, NULL);
    //   SDL_FreeSurface(icon);
    // }

    // #ifndef _WIN32
    // 	// Set up the clipboard
    // 	if ( init_scrap() < 0 ) {
    // 		fprintf(stderr, "Couldn't init clipboard: %s\n",
    // SDL_GetError()); 		return 1;
    // 	}
    // #endif

    ResetWindowTitle();

    if (!DoStartup()) {
      std::cout << "startup failed" << std::endl;
      SDL_Quit();
      return 0;
    }

#ifdef _WIN32
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif
    ourQuit = false;

    {
      // Under Win32 we use the old event/mutex/shared memory based
      // inter process communication protocol, for compatibility with
      // existing tools and the message loop.  For other platforms we
      // use TCP/IP sockets.
#ifndef _WIN32
      // int sockServerPort = 20001;
      // theApp.UserSettings().Get("Port", sockServerPort);
      // int sockServerSecurity = 1;
      // theApp.UserSettings().Get("PortSecurity", sockServerSecurity);
      // SockServer sockServer(sockServerPort, sockServerSecurity != 0);
      // ourSockServer = &sockServer;
#endif

#ifndef _WIN32
      // Catch signals from here, as this is where we jump back to
      // when we continue from them!
      CatchUnixSignals usignals;
#endif

      int tick = (int)SDL_GetTicks();
      //       while (!ourQuit) {

      // #ifndef _WIN32
      //         sockServer.ProcessRequests();
      // #endif
      //       }
      // EM_ASM({console.log('only once')});
      emscripten_set_main_loop(mainloop, 0, 1);
      // }
    }

    // DoShutdown();
  }
  // We catch all exceptions for robustness.
  catch (std::exception &e) {
    ErrorMessageHandler::Show(e, std::string("main"));
  } catch (...) {
    // We don't want to localise this message, to avoid getting into
    // infinite exception traps when the catalogues are bad.
    ErrorMessageHandler::NonLocalisable(
        "NLE0004: Unknown exception caught in initialisation or main loop",
        std::string("main"));
  }

  // SDL_Quit();
  return TRUE;
}

void mainloop() {
  try {
    ticksSinceStart += 1;
    int tick = (int)SDL_GetTicks();
    if (ourQuit) {
      std::cout << "quitting!" << std::endl;
      emscripten_cancel_main_loop();
    }

    int tickDelta = App::GetWorldTickInterval() - ((int)SDL_GetTicks() - tick);
    // if ((tickDelta > 0) && (!theApp.GetFastestTicks()))
    // SDL_Delay(tickDelta);
    tick = (int)SDL_GetTicks();

    while (SDL_PollEvent(&event) == 1) {
      HandleEvent(event);
    }

    // if (theApp.myToggleFullScreenNextTick) {
    //   theApp.ToggleFullScreenMode();
    //   theApp.myToggleFullScreenNextTick = false;
    // }

    if (ourCurrentlyTicking || ourSingleTick) {
      ourSingleTick = false;
      // Tell the app to update;
      theApp.UpdateApp();
      // App has finished with this set of events now.
      theApp.GetInputManager().SysFlushEventBuffer();
    }
  } catch (std::exception &e) {
    
    std::cout << "An error happened" << std::endl;
    std::cout << "ticksSinceStart: " << ticksSinceStart << std::endl;
  } catch (...) {
    std::cout << "An error happened" << std::endl;
    std::cout << "ticksSinceStart: " << ticksSinceStart << std::endl;
  }
}

int SDLKeyToWinKey(SDLKey sym) {
  switch (sym) {
  case SDLK_BACKSPACE:
    return VK_BACK;
  case SDLK_TAB:
    return VK_TAB;
  case SDLK_CLEAR:
    return VK_CLEAR;
  case SDLK_RETURN:
    return VK_RETURN;
  case SDLK_PAUSE:
    return VK_PAUSE;
  case SDLK_ESCAPE:
    return VK_ESCAPE;
  case SDLK_SPACE:
    return VK_SPACE;
  case SDLK_EXCLAIM:
    return '!';
  case SDLK_QUOTEDBL:
    return '"';
  case SDLK_HASH:
    return '#';
  case SDLK_DOLLAR:
    return '$';
  case SDLK_AMPERSAND:
    return '&';
  case SDLK_QUOTE:
    return '\'';
  case SDLK_LEFTPAREN:
    return '(';
  case SDLK_RIGHTPAREN:
    return ')';
  case SDLK_ASTERISK:
    return '*';
  case SDLK_PLUS:
    return '+';
  case SDLK_COMMA:
    return ',';
  case SDLK_MINUS:
    return 189; // Undocumented, not even in headers, VK_ for minus
  case SDLK_PERIOD:
    return '.';
  case SDLK_SLASH:
    return '/';
  case SDLK_0:
    return '0';
  case SDLK_1:
    return '1';
  case SDLK_2:
    return '2';
  case SDLK_3:
    return '3';
  case SDLK_4:
    return '4';
  case SDLK_5:
    return '5';
  case SDLK_6:
    return '6';
  case SDLK_7:
    return '7';
  case SDLK_8:
    return '8';
  case SDLK_9:
    return '9';
  case SDLK_COLON:
    return ':';
  case SDLK_SEMICOLON:
    return ';';
  case SDLK_LESS:
    return '<';
  case SDLK_EQUALS:
    return 187; // Undocumented, not even in headers, VK_ for plus
  case SDLK_GREATER:
    return '>';
  case SDLK_QUESTION:
    return '?';
  case SDLK_AT:
    return '@';
  case SDLK_LEFTBRACKET:
    return '[';
  case SDLK_BACKSLASH:
    return '\\';
  case SDLK_RIGHTBRACKET:
    return ']';
  case SDLK_CARET:
    return '^';
  case SDLK_UNDERSCORE:
    return '_';
  case SDLK_BACKQUOTE:
    return '`';
  case SDLK_a:
    return 'A';
  case SDLK_b:
    return 'B';
  case SDLK_c:
    return 'C';
  case SDLK_d:
    return 'D';
  case SDLK_e:
    return 'E';
  case SDLK_f:
    return 'F';
  case SDLK_g:
    return 'G';
  case SDLK_h:
    return 'H';
  case SDLK_i:
    return 'I';
  case SDLK_j:
    return 'J';
  case SDLK_k:
    return 'K';
  case SDLK_l:
    return 'L';
  case SDLK_m:
    return 'M';
  case SDLK_n:
    return 'N';
  case SDLK_o:
    return 'O';
  case SDLK_p:
    return 'P';
  case SDLK_q:
    return 'Q';
  case SDLK_r:
    return 'R';
  case SDLK_s:
    return 'S';
  case SDLK_t:
    return 'T';
  case SDLK_u:
    return 'U';
  case SDLK_v:
    return 'V';
  case SDLK_w:
    return 'W';
  case SDLK_x:
    return 'X';
  case SDLK_y:
    return 'Y';
  case SDLK_z:
    return 'Z';
  case SDLK_DELETE:
    return VK_DELETE;
  case SDLK_KP0:
    return VK_NUMPAD0;
  case SDLK_KP1:
    return VK_NUMPAD1;
  case SDLK_KP2:
    return VK_NUMPAD2;
  case SDLK_KP3:
    return VK_NUMPAD3;
  case SDLK_KP4:
    return VK_NUMPAD4;
  case SDLK_KP5:
    return VK_NUMPAD5;
  case SDLK_KP6:
    return VK_NUMPAD6;
  case SDLK_KP7:
    return VK_NUMPAD7;
  case SDLK_KP8:
    return VK_NUMPAD8;
  case SDLK_KP9:
    return VK_NUMPAD9;
  case SDLK_KP_PERIOD:
    return VK_DECIMAL;
  case SDLK_KP_DIVIDE:
    return VK_DIVIDE;
  case SDLK_KP_MULTIPLY:
    return VK_MULTIPLY;
  case SDLK_KP_MINUS:
    return VK_SUBTRACT;
  case SDLK_KP_PLUS:
    return VK_ADD;
  case SDLK_KP_ENTER:
    return VK_RETURN; // virtual key codes don't seem to distinguish numpad
                      // return from main one
  case SDLK_KP_EQUALS:
    return '='; // Never heard of a keypad equals!
  case SDLK_UP:
    return VK_UP;
  case SDLK_DOWN:
    return VK_DOWN;
  case SDLK_RIGHT:
    return VK_RIGHT;
  case SDLK_LEFT:
    return VK_LEFT;
  case SDLK_INSERT:
    return VK_INSERT;
  case SDLK_HOME:
    return VK_HOME;
  case SDLK_END:
    return VK_END;
  case SDLK_PAGEUP:
    return VK_PRIOR;
  case SDLK_PAGEDOWN:
    return VK_NEXT;
  case SDLK_F1:
    return VK_F1;
  case SDLK_F2:
    return VK_F2;
  case SDLK_F3:
    return VK_F3;
  case SDLK_F4:
    return VK_F4;
  case SDLK_F5:
    return VK_F5;
  case SDLK_F6:
    return VK_F6;
  case SDLK_F7:
    return VK_F7;
  case SDLK_F8:
    return VK_F8;
  case SDLK_F9:
    return VK_F9;
  case SDLK_F10:
    return VK_F10;
  case SDLK_F11:
    return VK_F11;
  case SDLK_F12:
    return VK_F12;
  case SDLK_F13:
    return VK_F13;
  case SDLK_F14:
    return VK_F14;
  case SDLK_F15:
    return VK_F15;
  case SDLK_NUMLOCK:
    return VK_NUMLOCK;
  case SDLK_CAPSLOCK:
    return 0; // not in Windows
  case SDLK_SCROLLOCK:
    return VK_SCROLL;
  case SDLK_RSHIFT:
    return VK_RSHIFT;
  case SDLK_LSHIFT:
    return VK_LSHIFT;
  case SDLK_RCTRL:
    return VK_RCONTROL;
  case SDLK_LCTRL:
    return VK_LCONTROL;
  case SDLK_RALT:
    return VK_RMENU;
  case SDLK_LALT:
    return VK_LMENU;
  case SDLK_RMETA:
    return 0; // not in Windows
  case SDLK_LMETA:
    return 0; // not in Windows
  // case SDLK_LSUPER:
  //   return VK_LMENU; // Should be left Windows key
  // case SDLK_RSUPER:
  //   return VK_RMENU; // Should be right Windows key
  case SDLK_MODE:
    return 0; // not in Windows
  case SDLK_HELP:
    return VK_HELP;
  case SDLK_PRINT:
    return VK_PRINT;
  case SDLK_SYSREQ:
    return 0; // what is this?
  case SDLK_BREAK:
    return VK_CANCEL;
  case SDLK_MENU:
    return VK_MENU;
  case SDLK_POWER:
    return 0; // not in windows
  default:
    return (int)sym;
  }
}

SDLKey WinKeyToSDLKey(int keycode) {
  // These three have two corresponding SDL
  // codes (left and right), so you'll need
  // to treat them specially
  ASSERT(keycode != VK_SHIFT);
  ASSERT(keycode != VK_CONTROL);
  ASSERT(keycode != VK_MENU);

  switch (keycode) {
  case VK_BACK:
    return SDLK_BACKSPACE;
  case VK_TAB:
    return SDLK_TAB;
  case VK_CLEAR:
    return SDLK_CLEAR;
  case VK_RETURN:
    return SDLK_RETURN;
  case VK_PAUSE:
    return SDLK_PAUSE;
  case VK_ESCAPE:
    return SDLK_ESCAPE;
  case VK_SPACE:
    return SDLK_SPACE;
  case '+':
    return SDLK_PLUS;
  case ',':
    return SDLK_COMMA;
  case '/':
    return SDLK_SLASH;
  case '0':
    return SDLK_0;
  case '1':
    return SDLK_1;
  case '2':
    return SDLK_2;
  case '3':
    return SDLK_3;
  case '4':
    return SDLK_4;
  case '5':
    return SDLK_5;
  case '6':
    return SDLK_6;
  case '7':
    return SDLK_7;
  case '8':
    return SDLK_8;
  case '9':
    return SDLK_9;
  case ':':
    return SDLK_COLON;
  case ';':
    return SDLK_SEMICOLON;
  case '<':
    return SDLK_LESS;
  case '=':
    return SDLK_EQUALS;
  case '>':
    return SDLK_GREATER;
  case '?':
    return SDLK_QUESTION;
  case '@':
    return SDLK_AT;
  case '[':
    return SDLK_LEFTBRACKET;
  case '\\':
    return SDLK_BACKSLASH;
  case ']':
    return SDLK_RIGHTBRACKET;
  case '^':
    return SDLK_CARET;
  case '_':
    return SDLK_UNDERSCORE;
  case 'A':
    return SDLK_a;
  case 'B':
    return SDLK_b;
  case 'C':
    return SDLK_c;
  case 'D':
    return SDLK_d;
  case 'E':
    return SDLK_e;
  case 'F':
    return SDLK_f;
  case 'G':
    return SDLK_g;
  case 'H':
    return SDLK_h;
  case 'I':
    return SDLK_i;
  case 'J':
    return SDLK_j;
  case 'K':
    return SDLK_k;
  case 'L':
    return SDLK_l;
  case 'M':
    return SDLK_m;
  case 'N':
    return SDLK_n;
  case 'O':
    return SDLK_o;
  case 'P':
    return SDLK_p;
  case 'Q':
    return SDLK_q;
  case 'R':
    return SDLK_r;
  case 'S':
    return SDLK_s;
  case 'T':
    return SDLK_t;
  case 'U':
    return SDLK_u;
  case 'V':
    return SDLK_v;
  case 'W':
    return SDLK_w;
  case 'X':
    return SDLK_x;
  case 'Y':
    return SDLK_y;
  case 'Z':
    return SDLK_z;
  case VK_DELETE:
    return SDLK_DELETE;
  case VK_NUMPAD0:
    return SDLK_KP0;
  case VK_NUMPAD1:
    return SDLK_KP1;
  case VK_NUMPAD2:
    return SDLK_KP2;
  case VK_NUMPAD3:
    return SDLK_KP3;
  case VK_NUMPAD4:
    return SDLK_KP4;
  case VK_NUMPAD5:
    return SDLK_KP5;
  case VK_NUMPAD6:
    return SDLK_KP6;
  case VK_NUMPAD7:
    return SDLK_KP7;
  case VK_NUMPAD8:
    return SDLK_KP8;
  case VK_NUMPAD9:
    return SDLK_KP9;
  case VK_DECIMAL:
    return SDLK_KP_PERIOD;
  case VK_DIVIDE:
    return SDLK_KP_DIVIDE;
  case VK_MULTIPLY:
    return SDLK_KP_MULTIPLY;
  case VK_SUBTRACT:
    return SDLK_KP_MINUS;
  case VK_ADD:
    return SDLK_KP_PLUS;
  case VK_UP:
    return SDLK_UP;
  case VK_DOWN:
    return SDLK_DOWN;
  case VK_RIGHT:
    return SDLK_RIGHT;
  case VK_LEFT:
    return SDLK_LEFT;
  case VK_INSERT:
    return SDLK_INSERT;
  case VK_HOME:
    return SDLK_HOME;
  case VK_END:
    return SDLK_END;
  case VK_PRIOR:
    return SDLK_PAGEUP;
  case VK_NEXT:
    return SDLK_PAGEDOWN;
  case VK_F1:
    return SDLK_F1;
  case VK_F2:
    return SDLK_F2;
  case VK_F3:
    return SDLK_F3;
  case VK_F4:
    return SDLK_F4;
  case VK_F5:
    return SDLK_F5;
  case VK_F6:
    return SDLK_F6;
  case VK_F7:
    return SDLK_F7;
  case VK_F8:
    return SDLK_F8;
  case VK_F9:
    return SDLK_F9;
  case VK_F10:
    return SDLK_F10;
  case VK_F11:
    return SDLK_F11;
  case VK_F12:
    return SDLK_F12;
  case VK_F13:
    return SDLK_F13;
  case VK_F14:
    return SDLK_F14;
  case VK_F15:
    return SDLK_F15;
  case VK_NUMLOCK:
    return SDLK_NUMLOCK;
  case VK_SCROLL:
    return SDLK_SCROLLOCK;
  case VK_RSHIFT:
    return SDLK_RSHIFT;
  case VK_LSHIFT:
    return SDLK_LSHIFT;
  case VK_RCONTROL:
    return SDLK_RCTRL;
  case VK_LCONTROL:
    return SDLK_LCTRL;
  case VK_RMENU:
    return SDLK_RALT;
  case VK_LMENU:
    return SDLK_LALT;
  case VK_PRINT:
    return SDLK_PRINT;
  case VK_CANCEL:
    return SDLK_BREAK;
  case VK_MENU:
    return SDLK_MENU;

  default:
    return (SDLKey)keycode;
  }
}

// mbLeft=1, mbRight=2, mbMiddle=4
int SDLButtonToAppButton(int but) {
  if (but == 1)
    return 1;
  else if (but == 3)
    return 2;
  else if (but == 2)
    return 4;
  return -1;
}

void HandleEvent(const SDL_Event &event) {
  switch (event.type) {
  case SDL_MOUSEMOTION:
    theApp.GetInputManager().SysAddMouseMoveEvent(event.motion.x,
                                                  event.motion.y);
    break;
  case SDL_MOUSEBUTTONDOWN: {
    int button = SDLButtonToAppButton(event.button.button);
    if (button != -1) {
      theApp.GetInputManager().SysAddMouseDownEvent(event.button.x,
                                                    event.button.y, button);
    }
    // In X and SDL mouse wheel events come via the 4th and
    // 5th mouse buttons being pressed down
    else if (event.button.button == 4) {
      theApp.GetInputManager().SysAddMouseWheelEvent(
          event.button.x, event.button.y, WHEEL_DELTA);
    } else if (event.button.button == 5) {
      theApp.GetInputManager().SysAddMouseWheelEvent(
          event.button.x, event.button.y, -WHEEL_DELTA);
    }
    break;
  }
  case SDL_MOUSEBUTTONUP: {
    int button = SDLButtonToAppButton(event.button.button);
    if (button != -1) {
      theApp.GetInputManager().SysAddMouseUpEvent(
          event.button.x, event.button.y,
          SDLButtonToAppButton(event.button.button));
    }
    break;
  }
  case SDL_KEYDOWN: {
    int winKey = SDLKeyToWinKey(event.key.keysym.sym);

    // Under SDL use Alt+Break rather than Control+Break
    // as X doesn't seem to cope with that
    if (winKey == VK_PAUSE && GlobalIsKeyDown(VK_MENU)) {
#ifdef _WIN32
      if (::MessageBox(theMainWindow, theCatalogue.Get("control_break", 0),
                       "MainWindowProc", MB_SYSTEMMODAL | MB_OKCANCEL) == IDOK)
#else
      // No message box here, tough
#endif
        SignalTerminateApplication();
    } else if ((winKey == VK_PAUSE) && (theApp.DebugKeyNow())) {
      // VK_PAUSE is handled here (rather than the more
      // platform independent App::HandleInput()) so it works
      // even when the game is debug paused (to unpause it)
      SetGameTicking(!GetGameTicking());
    } else if ((winKey == VK_SPACE) && !GetGameTicking()) {
      // Similarly, VK_SPACE needs to work even when game paused
      SendTickMessage();
    } else if (winKey == 'G' && GlobalIsKeyDown(VK_CONTROL)) {
      if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
        SDL_WM_GrabInput(SDL_GRAB_ON);
      else
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    } else if (winKey == VK_TAB && GlobalIsKeyDown(VK_MENU)) {
      // Ignore Alt+Tab completely (for Windows-like
      // window managers)
      break;
    }

    theApp.GetInputManager().SysAddKeyDownEvent(winKey);
    unsigned char charKey = (unsigned char)event.key.keysym.unicode;
    // Pressing Delete seems to send 127, when we don't really
    // want a char for that?  Not easy to make this stuff
    // compatible with the Windows version...
    if (charKey != 127) {
      // Nasty kludge to prevent sending of a char for Control+1.
      // For some reason X treats that as a 1, which it shouldn't.
      // (in Creatures 3, Control+1 opens the creature selection
      // panel, but shouldn't also type a 1 in the speech bubble)
      // The same thing happens for other control keys, so we just
      // don't send chars if Control is held down (I don't like this
      // though!)
      bool controlDown = GlobalIsKeyDown(VK_CONTROL);
      if (!controlDown)
        theApp.GetInputManager().SysAddTranslatedCharEvent(charKey);
    }

    break;
  }
  case SDL_KEYUP:
    //		printf("keyup '%s'\n", SDL_GetKeyName( event.key.keysym.sym ) );
    theApp.GetInputManager().SysAddKeyUpEvent(
        SDLKeyToWinKey(event.key.keysym.sym));

    // did try this upon keydown, but SDL went a bit weird and
    // thought that keys were being held down.
    if ((event.key.keysym.mod & KMOD_ALT) && // SHIFT also?
        event.key.keysym.sym == SDLK_RETURN) {
      theApp.ToggleFullScreenMode();
      theApp.WindowHasResized();
    }
    // directx version processes this as one of the
    // debug keys
    if ((event.key.keysym.mod & KMOD_SHIFT) &&
        event.key.keysym.sym == SDLK_HOME) {
      theApp.ChangeResolution();
      theApp.WindowHasResized();
    }
    break;
  case SDL_VIDEORESIZE:
    // Workaround to a bug in KDE's window manager (it happens
    // in kwm 0.14 patchlevel 3, and no doubt other versions).
    // Spurious resize events with size 32 by 32 arrive.
    if (event.resize.w != 32 || event.resize.h != 32) {
      theMainView.ResizeWindow(event.resize.w, event.resize.h);
      theApp.WindowHasResized();
    }
    break;
  case SDL_ACTIVEEVENT:
    break;
  case SDL_QUIT:
    // Pass quit messages on to CAOS, via a fake press of Escape
    theApp.GetInputManager().SysAddKeyDownEvent(VK_ESCAPE);
    theApp.GetInputManager().SysAddTranslatedCharEvent(VK_ESCAPE);
    theApp.GetInputManager().SysAddKeyUpEvent(VK_ESCAPE);
    // ourQuit = true;
    break;
#ifdef _WIN32
  case SDL_SYSWMEVENT:
    SDL_SysWMEvent *wmevent = (SDL_SysWMEvent *)&event;
    if (wmevent->msg->msg == WM_INCOMINGREQUEST) {
      // Respond to a request coming in from the external interface
      Win32Server::RequestArrived();
    }
    break;
#endif
  }
}

static bool InitInstance() {
  // UGH. This config-setup stuff should be handled by App::Init(),
  // but that fn isn't called until after the window is opened.
  if (!theApp.InitConfigFiles()) {
    ErrorMessageHandler::Show("NLE0015: Parsing error reading in configuration "
                              "file, machine.cfg or user.cfg",
                              0, "InitInstance");
    return false;
  }

  // get the file paths from the registry
  if (!theDirectoryManager.ReadFromConfigurationFiles()) {
    std::cout << "Unable to read from config files" << std::endl;
    return false;
  }

  // set up the catalogue (localised stringtable) and any
  // other localisable stuff.
  if (!theApp.InitLocalisation())
    return false;

  return true;
}

static bool DoStartup() {
  //	ourTerminateApplication = false;

#ifdef _WIN32
  // Retrieve window handle for Windows
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  SDL_GetWMInfo(&info);
  theMainWindow = info.window;
#endif

  // set up the game
  if (!theApp.Init())
    return false;

#ifdef _WIN32
  // Start Win32 external interface, passing it the window handle
  // (don't worry too much if it fails!)
  Win32Server::StartInterface(theMainWindow);
#endif

  return true;
}

static void DoShutdown() {
  ourRunning = false;

#ifdef _WIN32
  // Close down external interface
  Win32Server::StopInterface();
#endif

  theApp.ShutDown();
}

void SignalTerminateApplication() { ourQuit = true; }

static bool waitingForSingleStep = false;
AgentHandle waitingForAgent;

void SetSingleStepAgent(AgentHandle agent) {
  waitingForSingleStep = false;
  waitingForAgent = agent;
}

AgentHandle GetSingleStepAgent() { return waitingForAgent; }

void WaitForSingleStepCommand() {
  waitingForSingleStep = true;
  SDL_Event event;
  do {
    SDL_Delay(1);
    while (SDL_PollEvent(&event) == 1) {
      HandleEvent(event);
      switch (event.type) {
      case SDL_KEYDOWN: {
        int winKey = SDLKeyToWinKey(event.key.keysym.sym);

        if (winKey == VK_PAUSE) {
          SetSingleStepAgent(NULLHANDLE);
        }
        break;
      }
#ifdef _WIN32
      case SDL_SYSWMEVENT: {
        SDL_SysWMEvent *wmevent = (SDL_SysWMEvent *)&event;
        if (wmevent->msg->msg == WM_INCOMINGREQUEST) {
          // Respond to a request coming in from the external interface
          Win32Server::RequestArrived();
        }
        break;
      }
#endif
      }
    }

#ifndef _WIN32
    ourSockServer->ProcessRequests();
#endif
  } while (waitingForSingleStep);
}

void ResetWindowTitle() {
  std::string title = theApp.GenerateWindowTitle();
  // The second parameter (icon) here is the
  // caption when the window is iconised
  SDL_WM_SetCaption(title.c_str(), title.c_str());
}

// these  let you stop the game running (except
// request manager), get thet state of the game running,
// and notify that the game should tick once
bool SetGameTicking(bool bActive) {
  ourCurrentlyTicking = bActive;
  return true;
}

bool GetGameTicking() { return ourCurrentlyTicking; }

void SendTickMessage() { ourSingleTick = true; }

bool GlobalIsKeyDown(int keycode) {
  int numkeys;
  Uint8 *state = SDL_GetKeyboardState(&numkeys);

  // Cope with keys which have two SDL meanings
  if (keycode == VK_SHIFT)
    return (state[SDLK_LSHIFT] == SDL_PRESSED) ||
           (state[SDLK_RSHIFT] == SDL_PRESSED);
  if (keycode == VK_CONTROL)
    return (state[SDLK_LCTRL] == SDL_PRESSED) ||
           (state[SDLK_RCTRL] == SDL_PRESSED);
  if (keycode == VK_MENU)
    return (state[SDLK_LALT] == SDL_PRESSED) ||
           (state[SDLK_RALT] == SDL_PRESSED);

  SDLKey sym = WinKeyToSDLKey(keycode);
  if (sym < 0 || sym > numkeys)
    return false;
  else
    return state[sym] == SDL_PRESSED;
}

// Don't think we have these two in SDL
// (they let you drag the mouse out of the window,
// but still receive the mouse move events - for example
// when middle mouse dragging in windowed mode)

// The SDL_WM_GrabInput stuff is different - it doesn't
// send events from _outside_ the window, so doesn't help.
// See the Ctrl+G handler above.
void GlobalSetCapture() {}
void GlobalReleaseCapture() {}

void GlobalWarpMouse(int newX, int newY) {
  if (SDL_GetAppState() & SDL_APPMOUSEFOCUS &&
      SDL_GetAppState() & SDL_APPINPUTFOCUS) {
    SDL_WarpMouse(newX, newY);
  }
}
