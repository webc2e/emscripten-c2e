#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int urllaunch_opt_debug=0;

extern int try_kde( const char *url );
extern int try_gnome( const char *url );
extern int try_browservar( const char *url );
extern int try_misc( const char *url );

#define MAX_URL_LENGTH 1024

/* convert dangerous characters in a URL to %XX or \A so that
 * commands called using the shell can't be subverted */
char* paranoid_escape(const char* url)
{
	static char escaped_url[ MAX_URL_LENGTH * 3 + 1 ];
	const char hex_digits[16] = "0123456789ABCDEF";
	int len;
	int i;
	char c;
	char *dest;

	len = strlen( url );
	if( len > MAX_URL_LENGTH )
		return NULL;

	dest = escaped_url;
	for( i = 0; i < len; ++i )
	{
		c = url[i];
		/* in fear, we only allow a specific fixed set of characters */
		/* TODO: follow http://www.dwheeler.com/browse/secure_browser.html ? */
		if( (c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z')
			|| (c >= '0' && c <= '9')
			|| c == '_' || c == '-' )
		{
			*dest++ = c;
		}
		/* shell escape another set - they are parsed by the browser, so
		 * need to be seen raw by it, rather than %XX escaped */
		else if (
			c == '\\' || c == ':' || c == '.' || c == '=' || c == '/'
			|| c == '%' || c == '?' || c == '&' || c == '=' || c == '#')
		{
			*dest++ = '\\';
			*dest++ = c;
		}
		/* and URL escape the rest */
		else
		{
			*dest++ = '%';
			*dest++ = hex_digits[c >> 4];	
			*dest++ = hex_digits[c & 0x0F];
		}
	}
	*dest = 0;

	return escaped_url;
}

/* main function, the one that launches the url */
int launchurl( const char* url )
{
	const char* safe_url = paranoid_escape(url);
	if( urllaunch_opt_debug )
		printf("escaped url %s\n", safe_url);
	if ( !safe_url )
	{
		fprintf(stderr, "url too long, max length %d\n", MAX_URL_LENGTH);
		return 0;	
	}

	if( try_kde(safe_url) )
		return 1;
	
	/* try_gnome uses a function call rather than shell launch, so doesn't need
	 * the safe URL */
	if( try_gnome(url) )
		return 1;

	if( try_browservar(safe_url) )
		return 1;

	if( try_misc(safe_url) )
		return 1;

	return 0;
}

/* returns 1 if 'prog' is in path and is executable, else 0 */
int inpath( const char *prog )
{
	char buf[ PATH_MAX ];
	char *bufp;
	char *next;
	int done;
	const char *path = getenv( "PATH" );

	if( !path )
		return 0;	/* not good. */

	done=0;
	while( !done )
	{
		bufp = buf;

		/* expand tilde into homedir */
		if( *path == '~' )
		{
			const char *home = getenv( "HOME" );
			if( !home )
				home = ".";
			strcpy( bufp, home );
			bufp += strlen(home);
			++path;
		}
	
		next = strchr( path, ':' );
		if( next )
		{
			strncpy( bufp, path, next-path );
			bufp[ next-path ] = '\0';
			path=next+1;
		}
		else
		{
			strcpy( bufp, path );
			done = 1;
		}
	
		strcat( buf, "/" );
		strcat( buf, prog );
		
		if( access( buf, X_OK ) == 0)
			return 1;	/* search no more */
	}

	/* if we got this far, we found nothing. */
	return 0;
}


int runslurp( const char *cmd, char* outbuf, int outbufsize, int* status )
{
	FILE *fp;
	int success=0;
	int tmp;
	int bytesread;

	memset( outbuf, 'z', outbufsize );

	
	fp = popen( cmd, "r" );
	if( !fp )
		return 0;

	bytesread = fread( outbuf, 1, outbufsize-1, fp );
	
	if( feof(fp) )
	{
		success = 1;
		outbuf[bytesread] = '\0';
	}

	tmp = pclose(fp);
	if( status )
		*status = tmp;

	return success;
}
