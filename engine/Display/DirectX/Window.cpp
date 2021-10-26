// -------------------------------------------------------------------------
// Filename:    Window.cpp
// Class:       None
// Purpose:     Main window and message pump stuff
// Description:
//				Window.cpp and Window.h provide the basic framework
//				services used by the game:
//				- The Main Window
//				- Timing
//				- Mouse and keyboard input
//
//				The single App object is probably about the only class
//				which should interact with Window.h and Window.cpp...
//
// Usage:
//
//
// History:
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#define _WIN32_DCOM

#include "window.h"
#include "../../InputManager.h"
#include "../../RegistryHandler.h"
#include "../ErrorMessageHandler.h"
#include "../../Caos/Win32Server.h"

#ifdef C2D_DIRECT_DISPLAY_LIB
#else
	#include "../DisplayEngine.h"
#endif //C2D_DIRECT_DISPLAY_LIB

#include "../../Camera/MainCamera.h"

#include "../../World.h"
#include "../../../common/CStyleException.h"
#include "../../App.h"
#include "../../DirectoryManager.h"

#include "../../Sound/Soundlib.h"
#include "../../Sound/MusicManager.h"

//#include "../../CosInstaller.h"

#include <Zmouse.h>
#include <time.h>
#include <new.h>
#include <locale>

#ifdef _WIN32
  #include <windowsx.h> // for GET_X_LPARAM / GET_Y_LPARAM
  #include <objbase.h>
#endif

#include "../../../common/C2eDebug.h"

// Time for a nasty hack!
// Get the WM_MOUSEWHEEL message without enabling all the NT-specific
// features in the Windows header files.
// See winuser.h and article Q169088 in the MS knowledge base.
#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL                   0x020A
	#define WHEEL_DELTA                     120     /* Value for rolling one detent */
	#define WHEEL_PAGESCROLL                (UINT_MAX) /* Scroll one page */
#endif // !WM_MOUSEWHEEL

#define		SYSTEM_WINDOW_WIDTH	806
#define		SYSTEM_WINDOW_HEIGHT 625

// locals
static bool		ourTickPending;
//bool DisplayURL=false;

static UINT ourMouseWheelMessage;

// This is set while in a tick or request manager handler.
// If a message box pops up to display an error, then 
// the message queue is effectively reentrant, so we 
// have to ignore some messages.
static bool		ourDuringProcessing;

static UINT		ourTimerID = 0;
static bool ourRunning;

static bool ourTerminateApplication;

static bool ourMissedAnIncomingMessage;

// New code for additional Thread

static bool myTickerPaused;
static bool myTickerPresent;
static HANDLE myTickerThread;

// extern
HWND theMainWindow = 0;

static bool InitInstance(HINSTANCE instance);
static bool DoStartup( HWND window );
static void DoShutdown(HWND window);
static int32 WINAPI MainWindowProc(HWND window,uint32 message,WPARAM wparam, LPARAM lparam);

static bool ourQuit;

static bool ourWindowHasMoved = false;
static bool ourWindowHasResized = false;

#define		WM_TICK (WM_USER+0)
// WM_INCOMINGREQUEST is WM_USER+1
#define		WM_DRAWFRONTBUFFER (WM_USER+2)

float ExceptionTestDivide(int a, int b)
{
	return a / b;
}

void ExceptionMetaDivide()
{
	ExceptionTestDivide(7, 0);
}

void HandleMessages(UINT msgLow, UINT msgHigh, bool bOnce = false)
{
	MSG	message;
	BOOL ret = 0;
	while (!ourQuit && ret != -1 && PeekMessage(&message, NULL, msgLow, msgHigh, PM_NOREMOVE))
	{
		// Strangely GetMessage returns BOOL, even 
		// though it can return three different values
		// (0, -1 and other)
		ret = GetMessage(&message,NULL,msgLow, msgHigh);
		if (ret == 0)
			ourQuit = true;
		else if (ret != -1)
		{
			// We handle tick messages directly.
			// This is so we can break into infinite loops
			// using the debugger - if we dispatch ticks
			// via DispatchMessage, then we can't do so because
			// Windows and/or the debugger are pants.
			if (message.message == WM_TICK)
			{
				if (ourRunning)
				{
					// Tell timer queue the next tick (ensures
					// if we take longer than we should to process,
					// we at least start the next tick straight away)
					ourTickPending = false;

					if (!ourDuringProcessing)
					{
						// Mark that we're processing the tick
						ourDuringProcessing = true;

						// Tell the app to update;
						theApp.UpdateApp();
						// App has finished with this set of events now.
						theApp.GetInputManager().SysFlushEventBuffer();

						ourDuringProcessing = false;
					}
				}
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}

		if (bOnce)
			return;
	}
}

std::string memoryOutMessage = std::string("Out of memory launching game");

// Note, this failed handler only works when CustomHeap is disabled
int NewFailedHandler( size_t size )
{
	// catch everything and ignore, as we are low on memory
	try
	{
		theFlightRecorder.Log(FLIGHT_RUNTIME, "Memory allocation failed\n");
		
#ifdef C2D_DIRECT_DISPLAY_LIB
		theMainView.PrepareForMessageBox();
#else
		DisplayEngine::theRenderer().PrepareForMessageBox();
#endif

	}
	catch (...)
	{
	}	
	FatalAppExit(0, memoryOutMessage.c_str());
	return 1;
}

// ****************************************************************************
// Entry point
int WINAPI WinMain(HINSTANCE instance,HINSTANCE previous_instance,LPSTR command_line,int show_command)
{
	theFlightRecorder.Log(FLIGHT_STARTUP, "Start of WinMain\n");

	// Catch C style exceptions as C++ style ones
	// for more informative messages
	MakeCStyleExceptionsCppStyle cstyle;

	// We catch all exceptions for robustness and better error reporting
	try 
	{
		// Use these to test the exception handlers:
		// ExceptionMetaDivide();
		// ExceptionTestDivide(7, 0);
		// int y = 342234243;
		// int* z = reinterpret_cast<int *>(y);
		// int p = *z;

		// give error when memory runs out
		_set_new_handler(NewFailedHandler);

		ourRunning = true;

		// Control how memory allocation is debugged
		#ifdef	_DEBUG
			int debug_flag=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		//	debug_flag|=_CRTDBG_DELAY_FREE_MEM_DF;
			debug_flag|=_CRTDBG_LEAK_CHECK_DF;
		//	debug_flag|=_CRTDBG_CHECK_ALWAYS_DF; // added by GRT
			_CrtSetDbgFlag(debug_flag);
		#endif




		// Init com (for midi music)
/*
		if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		{
			ErrorMessageHandler::NonLocalisable("NLE0020: CoInitializeEx failed",
				std::string("WinMain"));
			return 0;
		}   
*/
/*
		// ***************************
		if (DisplayURL)
		{
			DisplayURL=false;
			WORD Status = (DWORD)ShellExecute(GetDesktopWindow(),NULL, "http://www.theregister.co.uk", NULL, "", SW_SHOWNORMAL);
			//WORD Status = (DWORD)ShellExecute(GetDesktopWindow(),NULL, "http://www.theregister.co.uk", NULL, "", SW_SHOWDEFAULT);
			char xxx[10];
			sprintf((char*)&xxx,"%d",Status);
			::MessageBox(theMainWindow, (char*)&xxx, "MainWindowProc",MB_OK);
			//WORD Status = (DWORD)ShellExecute(theMainWindow,NULL, "http://www.theregister.co.uk", NULL, "", SW_SHOWNORMAL);

		}
		return 0;*/
		// ***************************

		// Request millisecond resolution
		timeBeginPeriod(1);



		// Forward command line to app
		if (!theApp.ProcessCommandLine(std::string(command_line)))
		{
			return 0;
		}

		if( !InitInstance(instance) )
		{
			return 0;
		}

		theFlightRecorder.Log(FLIGHT_STARTUP, "Entering main message loop!");
		ourQuit = false;
		while (!ourQuit)
		{
			WaitMessage();

			// loop, getting messages until none are left

			MSG message;
			while (!ourQuit && PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE))
			{


				if (ourMissedAnIncomingMessage)
				{
					PostMessage(theMainWindow, WM_INCOMINGREQUEST, 0, 0);
					ourMissedAnIncomingMessage = false;
				}

				HandleMessages(0, WM_USER - 1);
				ASSERT(WM_USER < 0x7FFF);
				ASSERT(WM_TICK == WM_USER);
				HandleMessages(WM_TICK, WM_TICK, true);

				ASSERT(WM_DRAWFRONTBUFFER == WM_USER + 2);
				HandleMessages(WM_DRAWFRONTBUFFER, WM_DRAWFRONTBUFFER);

				ASSERT(WM_INCOMINGREQUEST == WM_TICK + 1);
				HandleMessages(WM_INCOMINGREQUEST, WM_INCOMINGREQUEST);


				HandleMessages(WM_DRAWFRONTBUFFER + 1, 0x7FFF);
				ASSERT(WM_APP == 0x8000); // make sure we cover all messages
				HandleMessages(WM_APP, UINT_MAX);

				// We do this here because error dialogs cause message procedure
				// reentrance.  When the Quit button is pressed in the error 
				// dialog, we want to wait until the message processing gets out to 
				// this level before doing the quit.
				if (ourTerminateApplication)
				{
					theFlightRecorder.Log(FLIGHT_SHUTDOWN, "OurTerminate is happening...\n");
					::DestroyWindow(theMainWindow);
				}
			}
		}
	}
	// We catch all exceptions in release mode for robustness.
	// In debug mode it is more useful to see what line of code
	// the error was caused by.
	catch (std::exception& e)
	{
		ErrorMessageHandler::Show(e, std::string("WinMain"));
	}
	catch (...)
	{
		// We don't want to localise this message, to avoid getting into
		// infinite exception traps when the catalogues are bad.
		ErrorMessageHandler::NonLocalisable("NLE0004: Unknown exception caught in initialisation or main loop",
			std::string("WinMain"));
	}
	theFlightRecorder.Log(FLIGHT_SHUTDOWN,"Message Loop has terminated.");
	timeEndPeriod(1);
    CoUninitialize();

	return TRUE;
}
// ****************************************************************************
void ResetWindowTitle()
{
	if (theMainWindow != 0)
	{
		std::string title = theApp.GenerateWindowTitle();
		SetWindowText(theMainWindow, title.c_str());
	}
}

static bool InitInstance(HINSTANCE instance)
{
	// UGH. This config-setup stuff should be handled by App::Init(),
	// but that fn isn't called until after the window is opened.
	theFlightRecorder.Log(FLIGHT_STARTUP, "Init config files\n");

//	if(!theApp.IsAppAScreenSaver() && !theApp.GetScreenSaverConfig() && !theApp.GetIsScreenSaverPreview())
	{
		if ( !theApp.InitConfigFiles() )
		{
			ErrorMessageHandler::NonLocalisable("NLE0015: Parsing error reading in configuration file, machine.cfg or user.cfg", "InitInstance");
			return false;
		}
	}

	// get the file paths from the registry
	theFlightRecorder.Log(FLIGHT_STARTUP, "Setting up directory manager");
	if ( !theDirectoryManager.ReadFromConfigurationFiles() )
		return false;

	// set up the catalogue (localised stringtable) and any
	// other localisable stuff.
	theFlightRecorder.Log(FLIGHT_STARTUP, "Loading in catalogues");
	if( !theApp.InitLocalisation() )
		return false;

	// get out of memory string
	memoryOutMessage = theCatalogue.Get("memory_out", 0);

	// mouse wheel support
	theFlightRecorder.Log(FLIGHT_STARTUP, "Mouse wheel support");
	ourMouseWheelMessage = RegisterWindowMessage(MSH_MOUSEWHEEL);

	// Load icon
	theFlightRecorder.Log(FLIGHT_STARTUP, "Loading icon");
	std::string strIconName = theApp.GetGameName() + ".ico";
	HANDLE icon = LoadImage(NULL, strIconName.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	std::string winClassName = "CLSystemClass";

	// Create window class
	WNDCLASS	engine_class=
	{
		CS_HREDRAW|CS_VREDRAW,
		(WNDPROC)MainWindowProc,
		0,0,
		instance,
		(HICON)icon,
		LoadCursor(NULL,NULL),
		(HBRUSH)GetStockObject(BLACK_BRUSH),
		"",
		winClassName.c_str()
	};
	RECT rect;

	theFlightRecorder.Log(FLIGHT_STARTUP, "Registering window class");
	if(!RegisterClass(&engine_class))
	{
		ErrorMessageHandler::Show("window_error", 0, "InitInstance");
		return false;
	}

	// just set the default size here and wait for the app to
	// figure out what the custom window was
	rect.left = 0;
	rect.top = 0;
	rect.right = SYSTEM_WINDOW_WIDTH;
	rect.bottom = SYSTEM_WINDOW_HEIGHT;

	std::string title = theApp.GenerateWindowTitle();

	HWND parentWin = NULL;//theApp.GetPreviewWindowHandle();
	DWORD winStyle = WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
	DWORD extendedWinStyle = WS_EX_LTRREADING|WS_EX_APPWINDOW;
	
	if(theApp.GetIsScreenSaverPreview())
	{
//		if(parentWin != NULL)
//		{
//			GetClientRect(parentWin, &rect);
//			winStyle = WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CHILD;
//			winClassName = NULL;
//			extendedWinStyle = NULL;
//		}
//		else
			return false;
	}
	else
		parentWin = NULL;

	theFlightRecorder.Log(FLIGHT_STARTUP, "Creating window");
	theMainWindow = CreateWindowEx(extendedWinStyle,
						winClassName.c_str(),
						title.c_str(),
						winStyle,
						rect.left,rect.top,
						rect.right - rect.left,rect.bottom - rect.top,
						parentWin,
						NULL,
						instance,
						NULL);

	if( !theMainWindow )
		return false;
	else
		return true;
	theFlightRecorder.Log(FLIGHT_STARTUP, "InitInstance done");
}

DWORD WINAPI Ticker( LPVOID ob )
{
	// Whilst we are running, let's gogogo
	myTickerPaused = false;
	myTickerPresent = true;

	while (myTickerPresent)
	{
		if (theApp.GetFastestTicks())
			Sleep(1);
		else
			Sleep(theApp.GetWorldTickInterval());

		if (myTickerPaused)
			PostMessage( theMainWindow, WM_DRAWFRONTBUFFER,0,0);
		else
			SendTickMessage();
	}

	myTickerPaused = false;
	myTickerPresent = false;
	return 0;
}


void SendTickMessage()
{
	if (theMainWindow == NULL)
		return;

	// use theTickPending flag to avoid build-up of messages.
	if( !ourTickPending )
	{
		ourTickPending = true;
		PostMessage( theMainWindow, WM_TICK,0,0);
	}
}

// The main message handler...


static int32 WINAPI MainWindowProc(HWND window,uint32 message,WPARAM wparam, LPARAM lparam)
{
	MakeCStyleExceptionsCppStyle cstyle;

	// We catch all exceptions in release mode for robustness.
	// In debug mode it is more useful to see what line of code
	// the error was caused by.
	try 
	{
		int x,y;

		// if not running, then we're shutting down
		// so don't do any special message handling
		if (!ourRunning) 
		{
			return DefWindowProc(window,message,wparam,lparam);
		}
		// old style mouse wheel message
		if (ourMouseWheelMessage != 0 && message == ourMouseWheelMessage)
		{
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);

			// ugly signbit hacking!
			theApp.GetInputManager().SysAddMouseWheelEvent( x,y, wparam);

			return 0L;
		}

		if (theApp.myToggleFullScreenNextTick)
		{
			theApp.ToggleFullScreenMode();
			theApp.myToggleFullScreenNextTick = false;
		}

		if (ourWindowHasMoved)
		{
			theApp.WindowHasMoved();
			ourWindowHasMoved = false;
		}
		if (ourWindowHasResized)
		{
			theApp.WindowHasResized();
			ourWindowHasResized = false;
		}

/*
		if (DisplayURL)
		{
			DisplayURL=false;
			WORD Status = (DWORD)ShellExecute(GetDesktopWindow(),NULL, "http://www.theregister.co.uk", NULL, "", SW_SHOWNORMAL);
			//WORD Status = (DWORD)ShellExecute(GetDesktopWindow(),NULL, "http://www.theregister.co.uk", NULL, "", SW_SHOWDEFAULT);
			char xxx[10];
			sprintf((char*)&xxx,"%d",Status);
			::MessageBox(theMainWindow, (char*)&xxx, "MainWindowProc",MB_OK);
			//WORD Status = (DWORD)ShellExecute(theMainWindow,NULL, "http://www.theregister.co.uk", NULL, "", SW_SHOWNORMAL);

		}
  */
#define WM_LAUNCH_URL (WM_USER+3)
		switch (message)
		{
			/*
		case WM_LAUNCH_URL:
			{
				DisplayURL=true;
			}
			break;
			  */
		case WM_TICK:
			// We tick directly in the main message loop now
			// (see HandleMessages)
			// This code here still gets called when error dialogs 
			// are up, and MainWindowProc is called by the dialog 
			// display code.  So, we have to clear pending tick flag,
			// and we can be sure that we must be during processing
			ourTickPending = false;
		//	ASSERT(ourDuringProcessing);
			break;
		case WM_DRAWFRONTBUFFER:
			// Tell timer queue the next tick (ensures
			// if we take longer than we should to process,
			// we at least start the next tick straight away)
			ourTickPending = false;

			if (ourDuringProcessing)
				break;

			// Mark that we're processing the tick
			ourDuringProcessing = true;

			#ifndef C2D_DIRECT_DISPLAY_LIB
				// Tell the app to update;
				DisplayEngine::theRenderer().DrawToFrontBuffer();
			#endif

			ourDuringProcessing = false;

			break;

		case WM_MOUSEMOVE:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseMoveEvent(x, y);
			break;
		case WM_INCOMINGREQUEST:
			if (ourDuringProcessing)
			{
				// can't loose incoming requests, or waiting 
				// process will never get a response
				ourMissedAnIncomingMessage = true;
				break;
			}

			// Mark that we're processing something
			ourDuringProcessing = true;

			// Respond to a request coming in from the external interface
			Win32Server::RequestArrived();

			ourDuringProcessing = false;

			break;
		case WM_KEYDOWN:
			if (wparam == VK_CANCEL)
			{
				if (ourDuringProcessing)
					break;
				ourDuringProcessing = true;

#ifdef C2D_DIRECT_DISPLAY_LIB
				theMainView.PrepareForMessageBox();
#else
				DisplayEngine::theRenderer().PrepareForMessageBox();
#endif
				if (::MessageBox(theMainWindow, theCatalogue.Get("control_break", 0), "MainWindowProc", MB_SYSTEMMODAL | MB_OKCANCEL) == IDOK)
					SignalTerminateApplication();

#ifdef C2D_DIRECT_DISPLAY_LIB
				theMainView.EndMessageBox();
#else
				DisplayEngine::theRenderer().EndMessageBox();
#endif

				ourDuringProcessing = false;
			}
			else if ((wparam == VK_PAUSE) && (theApp.DebugKeyNow()))
			{
				// VK_PAUSE is handled here (rather than the more
				// platform independent App::HandleInput()) so it works
				// even when the game is debug paused (to unpause it)
				SetGameTicking(!GetGameTicking());
			}
			else if ((wparam == VK_SPACE) && !GetGameTicking() && (theApp.DebugKeyNow()))
			{
				// Similarly, VK_SPACE needs to work even when game paused
				SendTickMessage();
			}
			else
			{
				theApp.GetInputManager().SysAddKeyDownEvent( wparam );
			}
			break;
		case WM_KEYUP:
			theApp.GetInputManager().SysAddKeyUpEvent( wparam );
			break;
		case WM_LBUTTONDOWN:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseDownEvent( x,y,InputEvent::mbLeft );
			break;
		case WM_LBUTTONUP:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseUpEvent( x,y,InputEvent::mbLeft );
			break;
		case WM_RBUTTONDOWN:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseDownEvent( x,y,InputEvent::mbRight );
			break;
		case WM_RBUTTONUP:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseUpEvent( x,y,InputEvent::mbRight );
			break;
		case WM_MBUTTONDOWN:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseDownEvent( x,y,InputEvent::mbMiddle );
			break;
		case WM_MBUTTONUP:
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);
			theApp.GetInputManager().SysAddMouseUpEvent( x,y,InputEvent::mbMiddle );
			break;
		case WM_MOUSEWHEEL:
			// Modern style mouse wheel message
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);

			// ugly signbit hacking!
			theApp.GetInputManager().SysAddMouseWheelEvent( x,y,
				( HIWORD( wparam ) & 0x8000 ) ?	// -ve or +ve?
				(int)HIWORD( wparam ) - 0x10000 : HIWORD( wparam ) );

			break;
		case WM_CHAR:
			theApp.GetInputManager().SysAddTranslatedCharEvent(wparam);
			break;
		case WM_CREATE:
			// the time starts itself before the CreateWindow
			// function can return so update the timer here
			theMainWindow = window;

			if( !DoStartup(window) )
			{
				DestroyWindow( window );	// bail out
			}
			break;
		case WM_CLOSE:
			::PostMessage(theMainWindow, WM_KEYDOWN, VK_ESCAPE, 0);
			::PostMessage(theMainWindow, WM_KEYUP, VK_ESCAPE, 0);
			break;
		case WM_DESTROY:
			theFlightRecorder.Log(FLIGHT_SHUTDOWN, "Doshutdown...\n");
			DoShutdown(window);
			theFlightRecorder.Log(FLIGHT_SHUTDOWN, "PostQuitMessage...\n");
			PostQuitMessage(TRUE);
			break;
		case WM_KILLFOCUS:
			SetFocus((HWND)wparam);
			return DefWindowProc(window,message,wparam,lparam);
		case WM_SYSCOMMAND:
			{
				int uCmdType = wparam & 0xFFF0;
				if (uCmdType == SC_SCREENSAVE || uCmdType == SC_MONITORPOWER)
					break;

				if (uCmdType == SC_MOVE || uCmdType == SC_SIZE
					|| uCmdType == SC_MAXIMIZE)
				{
					if (theApp.IsFullScreen())
						break;
				}

				LRESULT result = DefWindowProc(window,message,wparam,lparam);
				return result;
			}
		case WM_ENTERMENULOOP:
		case WM_ENTERSIZEMOVE:
			{
				if (ourDuringProcessing)
					break;
				ourDuringProcessing = true;
				LRESULT result = DefWindowProc(window,message,wparam,lparam);
				return result;
			}
		case WM_EXITMENULOOP:
		case WM_EXITSIZEMOVE:
			{
				ourDuringProcessing = false;
				LRESULT result = DefWindowProc(window,message,wparam,lparam);
				return result;
			}
		case WM_SYSKEYUP:
			{
				if (ourDuringProcessing)
					break;

				// deal with debug key presses
				CAOSVar& debugKeys = theApp.GetWorld().GetGameVar("engine_full_screen_toggle");
				if (debugKeys.GetInteger() == 1)
				{
					// this tells you that return was released while
					// holding the alt key down
					if(wparam == VK_RETURN)
					{
						theApp.ToggleFullScreenMode();
					}
				}
			}
			break;
		case WM_SIZE:
			{
				int32 rval = DefWindowProc(window,message,wparam,lparam);
				ourWindowHasResized = true;
				return rval;
				break;
			}
		case WM_SIZING:
			{
				RECT* lpr = (RECT*)lparam;
				if (lpr->right - lpr->left < 128)
					lpr->right = lpr->left + 128;
				if (lpr->bottom - lpr->top < 64)
					lpr->bottom = lpr->top + 64;
#ifdef C2D_DIRECT_DISPLAY_LIB
				int width = lpr->right - lpr->left;
				int height = lpr->bottom - lpr->top;
				int height_conv = height * 800 / 600;
				int width_conv = width * 600 / 800;
				switch (wparam)
				{
					case WMSZ_BOTTOM:
						lpr->right = lpr->left + height_conv;
						break;
					case WMSZ_LEFT:
						lpr->top = lpr->bottom - width_conv;
						break;
					case WMSZ_RIGHT:
						lpr->bottom = lpr->top + width_conv;
						break;
					case WMSZ_TOP:
						lpr->left = lpr->right - height_conv;
						break;
					case WMSZ_TOPLEFT:
						lpr->left = lpr->right - max(height_conv, width);
						lpr->top = lpr->bottom - max(height, width_conv);
						break;
					case WMSZ_TOPRIGHT:
						lpr->right = lpr->left + max(height_conv, width);
						lpr->top = lpr->bottom - max(height, width_conv);
						break;
					case WMSZ_BOTTOMLEFT:
						lpr->left = lpr->right - max(height_conv, width);
						lpr->bottom = lpr->top + max(height, width_conv);
						break;
					case WMSZ_BOTTOMRIGHT:
						lpr->right = lpr->left + max(height_conv, width);
						lpr->bottom = lpr->top + max(height, width_conv);
						break;
				}
#endif
				break;
			}
		case WM_ERASEBKGND:
			{
				// Here we should actually determine the non-display region and do summat wittit...
				RECT windowRect;
				GetWindowRect(window,&windowRect);
				int displayWidth = 800;
				int	displayHeight = 600;
#ifdef C2D_DIRECT_DISPLAY_LIB


#else
				displayWidth = DisplayEngine::theRenderer().GetSurfaceWidth();
				displayHeight = DisplayEngine::theRenderer().GetSurfaceHeight();
#endif
				if ((displayWidth >= (windowRect.right-windowRect.left)) &&
					(displayHeight >= (windowRect.bottom-windowRect.top)))
					break;
				// Hmm, we need to draw some of it :(
				HDC dc = (HDC)wparam;
				// Now create a nice black brush for ourselves...
				LOGBRUSH lBrush;
				lBrush.lbColor = RGB(0,0,0);
				lBrush.lbHatch = 0;
				lBrush.lbStyle = BS_SOLID;
				HBRUSH blackBrush = CreateBrushIndirect(&lBrush);
				// Now Select the brush into the DC
				HBRUSH oldBrush = (HBRUSH)SelectObject(dc,blackBrush);

				// Now draw two rectangles, the first to cover the right hand side...
				if (displayWidth < (windowRect.right-windowRect.left))
				{
					// Draw the left hand side (max height - displayHeight)
					RECT rightRect;
					rightRect.top = windowRect.top;
					rightRect.bottom = displayHeight;
					rightRect.left = displayWidth;
					rightRect.right = windowRect.right;
					FillRect(dc,&rightRect,blackBrush);
				}
				if (displayHeight < (windowRect.bottom-windowRect.top))
				{
					RECT rightRect;
					rightRect.top = displayHeight;
					rightRect.bottom = windowRect.bottom;
					rightRect.left = windowRect.left;
					rightRect.right = windowRect.right;
					FillRect(dc,&rightRect,blackBrush);
				}
				// Now re-select the old brush...
				SelectObject(dc,oldBrush);
				DeleteObject(blackBrush);
				// Return done and diddled
				return 1; // Processed :)
				break;
			}
		case WM_MOVE:
			{
				int32 rval = DefWindowProc(window,message,wparam,lparam);
				ourWindowHasMoved = true;
				ourWindowHasResized = true;
				return rval;
				break;
			}
		case WM_SETCURSOR:
			{
				// Blank cursor in client area
				int nHittest = LOWORD(lparam);
				if (nHittest == HTCLIENT)
				{
					SetCursor(NULL);
					return TRUE;
				}
				return DefWindowProc(window,message,wparam,lparam);
			}
		case WM_QUERYENDSESSION:
			{
				if (theMainWindow)
					SetForegroundWindow(theMainWindow);
				::PostMessage(theMainWindow, WM_KEYDOWN, VK_ESCAPE, 0);
				::PostMessage(theMainWindow, WM_KEYUP, VK_ESCAPE, 0);
				return FALSE;
			}
		case WM_NCHITTEST:
			{
			if(theApp.IsFullScreen())
				return HTCLIENT;
			//break;
			}
		default:
			{
			return DefWindowProc(window,message,wparam,lparam);
			}
		}
	}
	// We catch all exceptions in release mode for robustness.
	// In debug mode it is more useful to see what line of code
	// the error was caused by.
	catch (std::exception& e)
	{
		ErrorMessageHandler::Show(e, std::string("MainWindowProc"));
		ourDuringProcessing = false;
	}
	catch (...)
	{
		// We don't want to localise this message, to avoid getting into
		// infinite exception traps when the catalogues are bad.
		ErrorMessageHandler::NonLocalisable("NLE0003: Unknown exception caught in message processing",
			std::string("MainWindowProc"));
		ourDuringProcessing = false;
	}

	return 0L;
}

static bool DoStartup( HWND window )
{
	theFlightRecorder.Log(FLIGHT_STARTUP, "In DoStartup");

	ourTerminateApplication = false;
	ourMissedAnIncomingMessage = false;

	ErrorMessageHandler::SetWindow(window);

	// set up the game
	theFlightRecorder.Log(FLIGHT_STARTUP, "Calling App init");
	if( !theApp.Init() )
		return false;

	// start up the external interface so other programs can talk to us
	theFlightRecorder.Log(FLIGHT_STARTUP, "Starting external interface");
	if (!Win32Server::StartInterface(window))
		return false;

	// kickstart the multimedia timer (Or Nice new Thread :)
	ourTickPending = false;
	ourDuringProcessing = false;
	_ASSERT(ourTimerID == 0);
	theFlightRecorder.Log(FLIGHT_STARTUP, "Kick starting timer");
	if (!SetGameTicking(true))
		return false;

	theFlightRecorder.Log(FLIGHT_STARTUP, "Finished DoStartup");
	return true;
}



static void DoShutdown(HWND window)
{
	ourRunning = false;

	if (!SetGameTicking(false))
		ErrorMessageHandler::Show("window_error", 4, "Window::DoShutDown");
	myTickerPresent = false;

	Win32Server::StopInterface();

	theApp.ShutDown();
}

bool SetGameTicking(bool bActive)
{
	if (!myTickerPresent)
	{
		DWORD dummy;
		myTickerThread = CreateThread( NULL,
			0,
			Ticker,
			(LPVOID)0,
			0,
			&dummy );
		myTickerPresent = myTickerThread != 0;
	}
	if (!myTickerPresent)
	{
		ErrorMessageHandler::Show("window_error", 6, "SetCunningTickerThread");
		return false;
	}
	myTickerPaused = !bActive;
	return true;
}

bool GetGameTicking()
{
	return (!myTickerPaused) && myTickerPresent;
}

void SignalTerminateApplication()
{
	theFlightRecorder.Log(FLIGHT_SHUTDOWN, "Quit has been signalled...\n");
	ourTerminateApplication = true;
}

static bool waitingForSingleStep = false;
AgentHandle waitingForAgent;

void SetSingleStepAgent(AgentHandle agent)
{
	waitingForSingleStep = false;
	waitingForAgent = agent;
}

AgentHandle GetSingleStepAgent()
{
	return waitingForAgent;
}

void WaitForSingleStepCommand()
{
	waitingForSingleStep = true;
	do
	{
		Sleep(0);
		MSG message;
		if (PeekMessage(&message, NULL, WM_INCOMINGREQUEST, WM_INCOMINGREQUEST, PM_NOREMOVE))
		{
			PeekMessage(&message, NULL, WM_INCOMINGREQUEST, WM_INCOMINGREQUEST, PM_REMOVE);
			Win32Server::RequestArrived();
		}
		else if (PeekMessage(&message, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
		{
			if (message.wParam == VK_PAUSE)
				SetSingleStepAgent(NULLHANDLE);
		}
	}
	while (waitingForSingleStep);
}


bool GlobalIsKeyDown(int keycode)
{
	// keys are only down if we have the focus
	// no window has the mouse captured (which happens
	// if we are resizing/moving with the keyboard)
	if (::GetForegroundWindow() == theMainWindow &&
		::GetCapture() == NULL)
		return ( GetAsyncKeyState( keycode ) < 0 );
	else
		return false;
}

void GlobalSetCapture()
{
	::SetCapture(theMainWindow);
}

void GlobalReleaseCapture()
{
	::ReleaseCapture();
}

void GlobalWarpMouse(int newX, int newY)
{
	if (::GetForegroundWindow() == theMainWindow)
	{
		POINT pt;
		pt.x = newX;
		pt.y = newY;
		ClientToScreen(theMainWindow, &pt);
		SetCursorPos(pt.x, pt.y);
	}
}

