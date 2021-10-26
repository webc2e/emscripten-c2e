// -------------------------------------------------------------------------
// Filename:    
// Class: 
// Purpose:     
// Description:
//
// Usage:
//
// History:
// -------------------------------------------------------------------------


#include "TimeFuncs.h"
#include <time.h>


#ifdef _WIN32
#include <mmsystem.h>	// for timeGetTime
#else
#include <sys/time.h>
#endif

#ifdef C2E_SDL
#include <SDL/SDL.h>
#endif


uint32 GetRealWorldTime()
{
	return (uint32)(time(NULL));
}



#ifdef C2E_SDL
int GetTimeStamp()
	{ return (int)SDL_GetTicks(); }
#else
	#ifdef _WIN32
		int GetTimeStamp()
			{ return timeGetTime(); }
	#endif
#endif



int64 GetHighPerformanceTimeStamp()
{
#ifdef _WIN32
	LARGE_INTEGER stamp;
	if (QueryPerformanceCounter(&stamp))
		return stamp.QuadPart;
	else
		return 0;
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	int64 j = (int64)1000000 * (int64)tv.tv_sec + (int64)tv.tv_usec;

	return j;
#endif
}

int64 GetHighPerformanceTimeStampFrequency()
{
#ifdef _WIN32
	LARGE_INTEGER stamp;
	if (QueryPerformanceFrequency(&stamp))
		return stamp.QuadPart;
	else
		return 0;
#else
	return (int64)1000000;
#endif
}

#ifndef WINELIB
#ifndef _WIN32

// replacement for win32 call
void GetLocalTime( SYSTEMTIME* t )
{
	time_t bar;
	time( &bar );
	struct tm* foo = localtime( &bar );
	t->wYear = foo->tm_year + 1900;	// TODO: +1900? MS docs are ambiguous.
	t->wMonth = foo->tm_mon+1;
	t->wDayOfWeek = foo->tm_wday;
	t->wDay = foo->tm_mday;
	t->wHour = foo->tm_hour;
	t->wMinute = foo->tm_min;
	t->wSecond = foo->tm_sec;
	t->wMilliseconds = 0;		// don't need sub-second accuracy anyway.
}

#endif
#endif





//Check for invalid time components
bool IsValidTime(SYSTEMTIME& time)
{
	// TODO: Is this correct? (off-by-one error?)
	if(time.wHour < 0 || time.wHour > 23)
		return false;

	if( time.wMinute > 59)
		return false;

	if( time.wSecond > 59)
		return false;

	if( time.wMilliseconds > 999)
		return false;

	return true;
}

//Check for invalid time components
bool IsValidDate(SYSTEMTIME& time)
{
	if(time.wDay == 0 || time.wMonth == 0)
		return false;

	return true;
}

// game time must be at least one second!!
bool IsValidGameTime(SYSTEMTIME& time)
{
	if(time.wHour==0 && time.wMinute == 0 && time.wSecond ==0)
	return false;

	return true;
}

