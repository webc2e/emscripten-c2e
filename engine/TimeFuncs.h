#ifndef TIMEFUNCS_H
#define TIMEFUNCS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"

#ifndef WINELIB
#ifndef _WIN32
// a win32 structure
typedef struct _SYSTEMTIME
{
	uint16 wYear;
	uint16 wMonth;
    uint16 wDayOfWeek;
	uint16 wDay;
	uint16 wHour;
	uint16 wMinute;
    uint16 wSecond;
	uint16 wMilliseconds;
} SYSTEMTIME;
#endif // ! _WIN32
#endif // ! WINELIB

bool IsValidTime(SYSTEMTIME& time);
bool IsValidDate(SYSTEMTIME& time);
bool IsValidGameTime(SYSTEMTIME& time);

// moved here from Display/Window.h
#ifndef WINELIB
#ifndef _WIN32
	void GetLocalTime( SYSTEMTIME* t );
#endif
#endif
int GetTimeStamp(); // accurate to milliseconds
int64 GetHighPerformanceTimeStamp(); // as accurate as possible!
int64 GetHighPerformanceTimeStampFrequency(); // in counts per second

// seconds since midnight (00:00:00), January 1, 1970 UTC
uint32 GetRealWorldTime();


#endif // TIMEFUNCS_H

