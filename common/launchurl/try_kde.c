#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int inpath( const char *prog );
extern int urllaunch_opt_debug;

static int is_kde_running( void );



int try_kde( const char* url )
{
	char cmdbuf[ PATH_MAX*4 ];

	if( urllaunch_opt_debug )
		printf("enter try_kde()\n");

	
	if( is_kde_running() )
	{
		if( urllaunch_opt_debug )
			printf("detected kde, trying kfmclient\n");
		if( inpath( "kfmclient" ) )
		{
			sprintf(cmdbuf, "kfmclient openURL %s", url );
			if( system( cmdbuf ) == 0 )
				return 1;
			else
			{
				if( urllaunch_opt_debug )
					printf( "kfmclient failed\n");
			}
		}
		else
		{
			if( urllaunch_opt_debug )
				printf("kfmclient not found\n");
		}
	}
	return 0;
}


/*
 * Try to detect KDE using the 'dcop' command.
 * I think this should work on KDE2, but I don't think it'll work on KDE1.
 *
 */

static int is_kde_running( void )
{
		
	if( !inpath( "dcop" ) )
		return 0;

	if( system( "dcop >/dev/null 2>&1" ) == 0 )
		return 1;

	return 0;
}



