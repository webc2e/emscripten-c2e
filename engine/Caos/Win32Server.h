// Wrapper for Win32 version of server side interface

#ifndef _WIN32
#error Win32 only - SockServer used for other platforms
#endif

#ifndef WIN_32_SERVER_H
#define WIN_32_SERVER_H

#include "../ServerThread.h"
#include "../../common/ServerSide.h"
#include "RequestManager.h"

#define	WM_INCOMINGREQUEST (WM_USER+1)

namespace Win32Server
{
	static ServerThread ourServerThread;
	static ServerSide ourServerSide;
	static RequestManager ourRequestManager;

	bool StartInterface(HWND window);
	void StopInterface();
	void RequestArrived();
}

#endif

