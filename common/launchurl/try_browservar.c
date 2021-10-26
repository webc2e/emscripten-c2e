#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int inpath( const char *prog );

extern int urllaunch_opt_debug;

/* Check for browsers in the BROWSER environment variable.
 *
 * returns 1 if a browser was launched, 0 otherwise.
 * 
 *
 * See http://www.tuxedo.org/~esr/BROWSER/
 *
 * TODO:
 * Tidy it up.
 * Maybe implement the modifications detailed at
 * http://www.dwheeler.com/browse/secure_browser.html
 *
 * */
int try_browservar( const char *url )
{
	
	const char *browser = getenv( "BROWSER" );
	const char *next;
	char fmtbuf[ PATH_MAX ];
	char cmdbuf[ PATH_MAX ];
	int status;

	if( urllaunch_opt_debug )
		printf("enter try_browservar()\n");
	
	if( !browser )
	{
		if( urllaunch_opt_debug )
			printf("BROWSER var not set\n");
		return 0; 
	}
	while( *browser )
	{
		next = strchr( browser, ':' );
		if( next )
		{
			strncpy( fmtbuf, browser, next-browser );
			fmtbuf[next-browser] = '\0';
			browser = next+1;
		}
		else
		{
			strcpy( fmtbuf, browser );
			browser += strlen(browser);
		}

		/* does the string include a "%s" for inserting the url? */
		if( strstr( fmtbuf, "%s" ) )
		{
			/* TODO: should do our _own_ formatting.
				ie bad things will happen if other formatting commands are
				in fmtbuf... */
			sprintf( cmdbuf, fmtbuf, url );
		}
		else
		{
			/* nope - append url */
			sprintf( cmdbuf, "%s %s", fmtbuf, url );
		}

		if( urllaunch_opt_debug )
			printf("trying '%s'\n",cmdbuf);

		status = system( cmdbuf );
		if( status == 0 )
		{
			if(urllaunch_opt_debug)
				printf("->it worked!\n");
			return 1;
		}
		else
		{
		}
	}

	
	if( urllaunch_opt_debug )
		printf( "no joy using BROWSER :-(" );
	
	return 0;
}


