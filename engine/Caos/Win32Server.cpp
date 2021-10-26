#include "Win32Server.h"
#include "../App.h"
#include "../Display/ErrorMessageHandler.h"

extern ServerThread Win32Server::ourServerThread;
extern ServerSide Win32Server::ourServerSide;
extern RequestManager Win32Server::ourRequestManager;

const unsigned int theClientServerBufferSize = 1024*1024;

bool Win32Server::StartInterface(HWND window)
{
	if( !ourServerSide.Create( theApp.GetGameName().c_str(), theClientServerBufferSize ) )
	{
		ErrorMessageHandler::Show("window_error", 2, "Window::DoStartUp");
		return false;
	}
	if( !ourServerThread.Run( window, ourServerSide.GetRequestEvent(),
		WM_INCOMINGREQUEST ) )
	{
		// need to do cleanup here
		ErrorMessageHandler::Show("window_error", 3, "Window::DoStartUp");
		return false;
	}

	return true;
}

void Win32Server::StopInterface()
{
	ourServerThread.Die();
	ourServerSide.Close();
}


void Win32Server::RequestArrived()
{
	ourRequestManager.HandleIncoming( ourServerSide );
}

