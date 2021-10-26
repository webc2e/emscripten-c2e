// --------------------------------------------------------------------------
// Filename:	ErrorMessageHandler.h
// Class:		ErrorMessageHandler
// Purpose:		To display error messages 
//				
//				
//				
//
// Description: There should only ever be one errormessage handler that is shared
//				by all 
//
//				Doesn't do too much yet but think of the possibilites
//						
//
// History:
// ------- 
// 03Feb99	Alima		created.
//						Added load string
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ErrorMessageHandler.h"
#include "../C2eServices.h"
#ifndef LINUX_PORT
#include "DisplayEngine.h"
#endif
#include "../build.h"
#include "../General.h"

#ifdef _WIN32
#include "Window.h"
#include <lmcons.h> // for UNLEN 
#endif

#ifdef C2E_SDL
#include "SDL/SDL_Main.h"
#ifdef _WIN32

#endif
#endif

#include <time.h> // for time()
#include "ErrorDialog.h"


#ifndef _WIN32
	#include <stdlib.h>	// for getenv()
	#include <signal.h>
	#include <errno.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#ifdef _WIN32
ErrorMessageHandler::ErrorMessageHandler()
 : myWindow(0)
{
}
#else
ErrorMessageHandler::ErrorMessageHandler() {}
#endif

#ifdef LINUX_PORT
#include <stdarg.h>
#endif


// Private function to actually display message
void ErrorMessageHandler::ShowErrorMessage( const std::string& message,
			const std::string& source)
{
#ifndef LINUX_PORT
	std::string spacedMessage = message;
	if (message[message.size() - 1] != '\n')
		spacedMessage += "\n";

	std::string final_message = spacedMessage + "\n" + ErrorMessageFooter();

	ErrorDialog dlg;
	dlg.SetText(source, final_message);
	int ret;
#ifdef _WIN32
	ret = dlg.DisplayDialog(theMainWindow);
#else
	ret = dlg.DisplayDialog();
#endif

	if (ret == ErrorDialog::ED_QUIT)
	{
		theFlightRecorder.Log(FLIGHT_SHUTDOWN, "Quit button for the ShowErrorMessage() clicked... Signalling Termination...\n");
		SignalTerminateApplication();
	}
	else if (ret == ErrorDialog::ED_BRUTAL_ABORT)
	{
		theFlightRecorder.Log(FLIGHT_SHUTDOWN, "Erkity, BRUTAL ABORT!!!!!!!");
#ifdef _WIN32
		HANDLE hProcess = GetCurrentProcess();
		TerminateProcess(hProcess, -1);	// Francis you dozy fuckwit, at least I've finally found who's fault the
						// disappearing server is!!!!
#else
		if (kill(0, SIGKILL) == -1) 
			std::cerr << "Failed to kill self, error " << errno << std::endl;
		else
			std::cerr << "Killed self" << std::endl;
#endif
	}
#else
	fprintf(stderr,"Something so very bad has happened");
#endif
}

ErrorMessageHandler& ErrorMessageHandler::theHandler()
{
	static ErrorMessageHandler ourHandler;
	return ourHandler;
}

#ifdef _WIN32
void ErrorMessageHandler::SetWindow(HWND window)
{
	theHandler().myWindow = window;
}
#endif


// Variable arguments doesn't work with const & strings, for some reason,
// so we just let them get copied.  Similarly, it needs to be static.
// Don't pass std::strings as the variable parameters to this -
// do a .c_str() to make them char* first.  vsprintf only understands char*.
void ErrorMessageHandler::Show(std::string baseTag, int offsetID, std::string source, ...)
{
#ifndef LINUX_PORT
	// Read from catalogue
	std::string unformatted = theCatalogue.Get(baseTag, offsetID);
	
	// Get variable argument list
	va_list args;
	va_start(args, source);
	char szBuffer[4096];
	int nBuf = vsprintf(szBuffer, unformatted.c_str(), args);
	szBuffer[nBuf] = '\0';
	ASSERT(nBuf >= 0 && nBuf < sizeof(szBuffer) / sizeof(szBuffer[0]));
	va_end(args);

	std::string message(szBuffer);

	theHandler().ShowErrorMessage(message, source);
#endif
}

// Don't pass std::strings as the variable parameters to this -
// do a .c_str() to make them char* first.  vsprintf only understands char*.
std::string ErrorMessageHandler::Format(std::string baseTag, int offsetID, std::string source, ...)
{
#ifndef LINUX_PORT
	// Read from catalogue
	std::string unformatted = theCatalogue.Get(baseTag, offsetID);

	// Get variable argument list
	va_list args;
	va_start(args, source);
	char szBuffer[4096];
	int nBuf = vsprintf(szBuffer, unformatted.c_str(), args);
	szBuffer[nBuf] = '\0';
	ASSERT(nBuf >= 0 && nBuf < sizeof(szBuffer) / sizeof(szBuffer[0]));
	va_end(args);

	return source + std::string("\n\n") + std::string(szBuffer);
#endif
}

// Don't pass std::strings as the variable parameters to this -
// do a .c_str() to make them char* first.  vsprintf only understands char*.
void ErrorMessageHandler::Throw(std::string baseTag, int offsetID, std::string source, ...)
{
#ifndef LINUX_PORT
	// Read from catalogue
	std::string unformatted = theCatalogue.Get(baseTag, offsetID);

	// Get variable argument list
	va_list args;
	va_start(args, source);
	char szBuffer[4096];
	int nBuf = vsprintf(szBuffer, unformatted.c_str(), args);
	szBuffer[nBuf] = '\0';
	ASSERT(nBuf >= 0 && nBuf < sizeof(szBuffer) / sizeof(szBuffer[0]));
	va_end(args);

	std::string full = source + std::string("\n\n") + std::string(szBuffer);
	throw BasicException(full.c_str());
#endif
}

// This is for strings which are needed before the catalogues are initialised.
// These messages should have NLExxxx at the start - a NonLocalisable Error
// number - so they can be easily distinguished.

// Don't pass std::strings as the variable parameters to this -
// do a .c_str() to make them char* first.  vsprintf only understands char*.
void ErrorMessageHandler::NonLocalisable(std::string unformatted, std::string source, ...)
{
#ifndef LINUX_PORT
	// Get variable argument list
	va_list args;
	va_start(args, source);
	char szBuffer[4096];
	int nBuf = vsprintf(szBuffer, unformatted.c_str(), args);
	szBuffer[nBuf] = '\0';
	ASSERT(nBuf >= 0 && nBuf < sizeof(szBuffer) / sizeof(szBuffer[0]));
	va_end(args);

	std::string message(szBuffer);

	theHandler().ShowErrorMessage(message, source);
#endif
}

// Display message from exception
void ErrorMessageHandler::Show(std::exception& e, std::string source)
{
	theHandler().ShowErrorMessage(e.what(), source);
}

// Add useful information to all error messages
std::string ErrorMessageHandler::ErrorMessageFooter()
{

	
	std::string header;
#ifndef LINUX_PORT
	// OS
	header += OperatingSystemString() + "\n";

	// modules
	header += DisplayAndModuleString() + "\n";

	// date / time
	time_t aclock;
	time(&aclock);
	struct tm* newtime;
	newtime = localtime(&aclock);
	header += asctime(newtime);
	header.erase(header.size() - 1);

#ifdef _WIN32
	// user name
	char name[UNLEN + 1];
	DWORD size = UNLEN;
	if (GetUserName(name, &size))
	{
		header += " - ";
		header += name;
	}
#else
	// POSIX
	header += " - ";
	const char* name = getenv( "LOGNAME" );
	if( name )
		header += name;
	else
		header += "Mr X";
#endif

	// version
	header += " - ";
	header += GetEngineVersion();

	// build number if available
	if (theCatalogue.TagPresent("build_number"))
		header += std::string(" b") + theCatalogue.Get("build_number", 0);

	// port number if available
	if (theCatalogue.TagPresent("port_number"))
		header += std::string(" p") + theCatalogue.Get("port_number", 0);
#endif
	return header;
} 

