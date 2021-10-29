// -------------------------------------------------------------------------
// Filename:    C2eServices.h
// Class:       None (but see also C2eDebug.cpp)
// Purpose:     Provide ASSERT and debug string output. 
// Description:
//
// Usage:
//
//
// History:
// 13Aug99  BenC  Pulled out of C2eServices
// -------------------------------------------------------------------------


#ifndef C2EDEBUG_H
#define C2EDEBUG_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


void OutputFormattedDebugString(const char* fmt, ... );

#ifdef _WIN32
	#include<windows.h>		// for OutputDebugString
	#include <crtdbg.h>		// win32 debug stuff
	#ifndef ASSERT
		#define ASSERT _ASSERT
	#endif
	#define DEBUGMESSAGE(d) MessageBox(NULL, d, "Debug Message", MB_ICONINFORMATION);
#else
	#include <cassert>
	#define ASSERT(X) assert((X))
	#define _ASSERT(X) assert((X))
	void OutputDebugString( const char* lpOutputString );
	#ifndef NDEBUG
	#define _DEBUG
	#endif
#endif

#endif // C2EDEBUG_H

