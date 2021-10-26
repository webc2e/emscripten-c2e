#include "Win32ScreenKiller.h"

#include <winbase.h>
#include "../../common/C2eDebug.h"

HANDLE Win32ScreenKiller::ourThread;
POINT Win32ScreenKiller::ourPoint;
HANDLE Win32ScreenKiller::ourMainThread;
// We rely on this being atomic - should really use a mutex, but this
// code is so nasty and cowboyish anyway...
bool Win32ScreenKiller::ourEndNow; 

void Win32ScreenKiller::Start(HANDLE mainThread)
{
	ourMainThread = mainThread;
	ourEndNow = false;

	GetCursorPos(&ourPoint);

	DWORD dummy;
	ourThread = CreateThread( NULL,
			0,
			ThreadMain,
			(LPVOID)0,
			0,
			&dummy );
}

DWORD WINAPI Win32ScreenKiller::ThreadMain(void*)
{
	while(!ourEndNow)
	{
		// Mouse
		POINT pt;
		GetCursorPos(&pt);
		if (pt.x == 400 && pt.y == 300)
		{
			// When the screen mode changes, it moves the mouse to the
			// centre of the screen.  This is a nasty, cowboyish way of
			// avoiding that.
			ourPoint.x = pt.x;
			ourPoint.y = pt.y;
		}
		bool mouseMoved = (pt.x  != ourPoint.x) || (pt.y != ourPoint.y);

		// Keyboard - can't get this to work
		bool keyboardDown = false;
		/* for (int i = 0; i < 256; ++i)
		{
			SHORT value = GetAsyncKeyState(i);
			if ((value & 0xf000) == 0xf000)
			{
				keyboardDown = true;
				break;
			}
		} */

		if (mouseMoved || keyboardDown)
		{
			if (SuspendThread(ourMainThread) == -1)
				ASSERT(false);
			ChangeDisplaySettings(NULL, 0);
			HANDLE hProcess = GetCurrentProcess();
			TerminateProcess(hProcess, -1);
		}
		Sleep(10);
	}

	return 0;
}

void Win32ScreenKiller::Stop()
{
	ourEndNow = true;
}

