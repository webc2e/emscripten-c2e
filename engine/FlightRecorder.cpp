// -------------------------------------------------------------------------
// Filename:    FlightRecorder.cpp
// Class:       FlightRecorder
// Purpose:     Basic logging system
// Description:
// Each log entry should be a single line of text.
//
// Usage:
//
//
// History:
// Jan99	  BenC	Initial version
// -------------------------------------------------------------------------

// Categories so far:
// 1 - error message logging
// 16 - shutdown sequence logging

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "C2eServices.h"	// to get around circular dependency problem
#include "FlightRecorder.h"
#include "Display/ErrorMessageHandler.h"
#include "../common/FileFuncs.h"

FlightRecorder::FlightRecorder()
{
	// We need some way of letting people enable startup logging (the config file
	// is no good for this, as it hasn't been read yet!).  We look for this file...
	if (FileExists("logtastic"))
	{
		// enable startup logging (and shutdown for good measure)
		myEnabledCategories = FLIGHT_RUNTIME + FLIGHT_STARTUP + FLIGHT_SHUTDOWN;
	}
	else
	{
		// default to basic errors
		myEnabledCategories = FLIGHT_RUNTIME;
	}

	myOutFile = NULL;
	myOutFilename[0] = '\0';
	strcpy( myOutFilename, "creatures_engine_logfile.txt" );
}


FlightRecorder::~FlightRecorder()
{
	if( myOutFile )
	{
		Log( FLIGHT_INTERNAL, "");
		std::string ended = std::string("LOG ENDED ") + ErrorMessageHandler::ErrorMessageFooter();
		Log( FLIGHT_INTERNAL, ended.c_str());
		fclose( myOutFile );
	}
} 

void FlightRecorder::SetOutFile( const char* filename )
{
	// close existing file if any
	if( myOutFile )
	{
		fclose( myOutFile );
		myOutFile = NULL;
	}

	strcpy( myOutFilename, filename );
}


void FlightRecorder::Log(FlightCategory category, const char* fmt, ... )
{
	char buf[4096];
	va_list args;
	int len;

	if (category != 0)
	{
		if( !( ((uint32)category) & myEnabledCategories ) )
			return;
	}

	// open file if needed...
	bool madeFile = false;
	if( !myOutFile )
	{
		if( myOutFilename[0] )
			myOutFile = fopen( myOutFilename, "a+tc" );
		madeFile = true;
	}

	if( !myOutFile )
	{
		std::cerr << "Failed to make flight recorder file " << myOutFilename;
		return;
	}

	if (madeFile)
	{
		Log( FLIGHT_INTERNAL, "----------------------------------------------------" );
		std::string started = std::string("LOG STARTED ") + ErrorMessageHandler::ErrorMessageFooter();
		Log( FLIGHT_INTERNAL, started.c_str());
		Log( FLIGHT_INTERNAL, "");
	}
	va_start(args, fmt);
	len = vsprintf( buf, fmt, args);
	va_end(args);

	// append a linefeed
	buf[len] = '\n';
	buf[++len] = '\0';
	ASSERT(myOutFile);
	fwrite( buf, 1, len, myOutFile );
	fflush( myOutFile );
}

void FlightRecorder::SetCategories( uint32 enablemask )
{
	myEnabledCategories = enablemask;
	// And even when the config file is read, if logtastic is there, log startup and shutdown
	if (FileExists("logtastic"))
	{
		myEnabledCategories |= (FLIGHT_STARTUP + FLIGHT_SHUTDOWN);
	}
}

