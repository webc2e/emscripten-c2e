/*
 * try_misc.c
 *
 * This is our last-ditch method - run through a hardcoded list of browsers
 * until we find one in the path.
 * 
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int inpath( const char *prog );
extern int urllaunch_opt_debug;


struct Entry
{
	const char* prog;		/* binary to check for in path */
	const char* cmdline;	/* command to run (with %s for url) */
};


/*
 * TODO: add
 * 
 * Addtions:
 *  galleon
 *  gnome-moz-remote? (I couldn't get it working well - Ben)
 *  
 */

static struct Entry X11Browsers[] =
{
	{ "kfmclient", "kfmclient openURL %s" },
	{ "opera", "opera -newbrowser %s &" },
	{ "mozilla", "mozilla %s &" },
//	{ "netscape", "" },
//	{ "konqueror", "" },
	{ "links", "xterm -T \"Links\" -e links %s &" },
	{ "lynx", "xterm -T \"Lynx\" -e lynx %s &" },
	{ "w3m", "xterm -T \"w3m\" -e w3m %s &" },
};

static struct Entry TextModeBrowsers[] =
{
	{ "w3m", "w3m %s &" },
	{ "lynx", "lynx %s &" },
	{ "links", "links %s &" },
};




int try_misc( const char* url )
{
	char cmdbuf[ PATH_MAX*4 ];
	int i;
	struct Entry *entry;
	int numentries;

	if( urllaunch_opt_debug )
		printf("enter try_misc()\n");

	/* use presence of DISPLAY var to decide if we're under X11 */
	if( getenv( "DISPLAY" ) )
	{
		numentries = sizeof( X11Browsers ) / sizeof( struct Entry );
		entry = X11Browsers;

		if( urllaunch_opt_debug )
			printf("found DISPLAY, so it looks like we're run under X\n");
	}
	else
	{
		numentries = sizeof( TextModeBrowsers ) / sizeof( struct Entry );
		entry = TextModeBrowsers;
		if( urllaunch_opt_debug )
			printf("DISPLAY not set, looks like we're running in textmode\n");
	}

	/* check for each browser in turn... */
	i=0;
	while( i<numentries )
	{
		if( inpath( entry->prog ) )
		{
			sprintf(cmdbuf, entry->cmdline, url );
			if( urllaunch_opt_debug )
				printf("found %s (trying '%s')\n",entry->prog, cmdbuf );
			if( system( cmdbuf ) == 0 )
			{
				if( urllaunch_opt_debug )
					printf("it worked!\n");
				return 1;
			}
		}
		else
		{
			if( urllaunch_opt_debug )
				printf("can't find %s\n", entry->prog );
		}
		++i;
		++entry;
	}
	
	/* nothing found... */
	if( urllaunch_opt_debug )
		printf("no luck with misc browser list...\n");
	return 0;
}



