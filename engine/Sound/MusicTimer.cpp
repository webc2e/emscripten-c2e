// --------------------------------------------------------------------------
// Filename:	Music Timer.cpp
//
// Purpose:		GetTime() function
//
// Description:	Only included to avoid header clashes between dsound.h
//				and mmsystem.h
//
// History:
// 29Apr98	Peter	Created
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "MusicTimer.h"
#include "../../common/C2eTypes.h"

#ifdef C2E_SDL
#include <SDL/SDL.h>
#else
#ifdef _WIN32
#include <mmsystem.h>
#endif
#endif

// ----------------------------------------------------------------------
// Method:		MusicGetTime
// Arguments:	None
// Returns:		See below
// Description:	The timeGetTime function retrieves the system time, in
//				milliseconds. The system time is the time
//elapsed since 				Windows was started.
// ----------------------------------------------------------------------
int MusicGetTime() {
#ifdef C2E_SDL
  return (int)SDL_GetTicks();
#else
#ifdef _WIN32
  return timeGetTime();
#else
#error Need timer function
#endif
#endif
}
