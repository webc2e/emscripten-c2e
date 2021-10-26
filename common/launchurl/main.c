/* 
 * Main program for launchurl - you don't want to include this if you are using
 * it statically as a lib.
 * 
 */

#include <stdio.h>

#include "launchurl.h"

extern int urllaunch_opt_debug;
int urllaunch_opt_showversion=0;
int urllaunch_opt_showhelp=0;

#define VERSIONSTRING "0.2"

int main( int argc, char* argv[] )
{
	int i;
	int status = 0;

	if( argc<2 )
		urllaunch_opt_showhelp = 1;


	/* set options */
	for( i=1; i<argc; ++i )
	{
		if( argv[i][0] == '-' )
		{
			if( strcmp( argv[i], "-v" )==0 ||
				strcmp( argv[i], "-version") ==0 )
			{
				urllaunch_opt_showversion = 1;
			}
			else if( strcmp( argv[i], "-d" )==0i ||
				strcmp( argv[i], "-debug" )==0 )
			{
				urllaunch_opt_debug = 1;
			}

			else if( strcmp( argv[i], "-?" )==0 ||
				strcmp( argv[i], "-h" )==0 ||
				strcmp( argv[i], "--help" )==0 )
			{
				urllaunch_opt_showhelp = 1;
			}
			else
			{
				fprintf( stderr, "error - unknown option: '%s'\n",argv[i] );
				fprintf( stderr, "type launchurl -? for options\n" );
				return 1;
			}
		}	
	}


	if( urllaunch_opt_showhelp )
	{
		printf("launchurl - open url with users preferred browser\n" );
		printf("usage: launchurl [-d|-debug] [-v|-version] url\n" );
	}

	if( urllaunch_opt_showversion )
	{
		printf( "launchurl version "VERSIONSTRING"\n" );
	}
	

	/* look for url params */
	for( i=1; i<argc; ++i )
	{
		if( argv[i][0] != '-' )
		{
			if( !launchurl( argv[i] ) )
			{
				fprintf( stderr, "Couldn't launch a browser for '%s'\n",
					argv[i] );
				status = 1;
			}
		}
	}

	if( urllaunch_opt_debug )
	{
		printf("launchurl exiting.\n");
	}
	
	
	return status;
}

