#ifndef WIN_32_SCREEN_KILLER_H
#define WIN_32_SCREEN_KILLER_H

// Kills the entire application if someone moves the
// mouse or presses a key.  Used for screensaver.

#ifndef _WIN32
	#error This really is Windows only
#endif

#include <windows.h>

class Win32ScreenKiller
{
public:
	static void Start(HANDLE mainThread);
	static void Stop();

private:
	static DWORD WINAPI ThreadMain(void *);

	static POINT ourPoint;
	static HANDLE ourThread;
	static HANDLE ourMainThread;
	static bool ourEndNow;
};

#endif

