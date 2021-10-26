#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <limits.h>

extern int runslurp( const char *cmd, char* outbuf, int outbufsize,
	int* status );
extern int inpath( const char *prog );
extern int urllaunch_opt_debug;


int is_gnome_running( void );
int do_dirty_work( const char *url );


int try_gnome( const char *url )
{
	if( urllaunch_opt_debug )
		printf("enter try_gnome()\n");

	if( !is_gnome_running() )
		return 0;

	if( urllaunch_opt_debug )
		printf("looks like gnome is running\n");

	if( !do_dirty_work( url ) )
		return 0;
	
	return 1;
}


/*
 * is_gnome_running()
 *
 * Hacky test to try to determine if user is _currently_ running a gnome desktop.
 * Searches "gnome-session" in the process list for the current user.
 *
 * Improvements welcome!
 *
 */

int is_gnome_running( void )
{
	char buf[ 8192 ];
	char cmd[ PATH_MAX ];
	const char *user;

	user = getenv( "USER" );
	if( !user )
		return 0;

	sprintf( cmd, "ps -u %s", user );

	if( !runslurp( cmd, buf, sizeof(buf), 0 ) )
		return 0;

//	printf("------------\n%s--------------\n", buf );

	if( strstr( buf, "gnome-session" ) )
		return 1;

	return 0;
}



int do_dirty_work( const char *url )
{
	void *gnomesupportlib;
	void *gnomelib;
	void (*gnome_url_show)( const char *url );
	void (*gnomelib_init)(const char *app_id, const char *app_version);

	const char *err;
	
	gnomesupportlib = dlopen( "libgnomesupport.so", RTLD_NOW|RTLD_GLOBAL );
	if( !gnomesupportlib )
	{
		fprintf(stderr,"couldn't dlopen libgnomesupport.so: %s\n",dlerror() );
		return 0;
	}
	gnomelib = dlopen( "libgnome.so", RTLD_LAZY );

	if( !gnomelib )
	{
		fprintf(stderr,"couldn't dlopen libgnome.so: %s\n",dlerror() );
		return 0;
	}

	gnomelib_init = dlsym( gnomelib, "gnomelib_init" );
	err = dlerror();
	if( err )
	{
		fprintf(stderr,"couldn't find gnomelib_init(): %s\n", err );
		return 0;
	}

	gnome_url_show = dlsym( gnomelib, "gnome_url_show" );
	err = dlerror();
	if( err )
	{
		fprintf(stderr,"couldn't find gnome_url_show(): %s\n", err );
		return 0;
	}

	gnomelib_init( "launchurl", "0.1" );
	gnome_url_show( url );

	dlclose( gnomelib );
	dlclose( gnomesupportlib );

	return 1;
}



