/*
 * launchurl - tries to detect the users 'preferred' web browser and use
 *             it to launch a url.
 *
 * 19Sept2001
 * Hacked up by Ben Campbell
 * (ben.campbell@ntlworld.com or ben.campbell@justaddmonsters.com)
 *
 * For now, lets say this code is under a BSD style licence.
 * Feel free to hack it and fix it and statically link with it and
 * use it in closed source software and whatever you like, but please
 * send back any improvements!
 *
 * TODO:
 * - Lots of dodgy fixed-size buffer and string issues (security risk?)
 * - Add more browsers to lists in try_misc.c
 * - Better checks for kde and gnome
 * - Hand-roll a safer/better system() call
 * - Turn it into a library?
 * - Unite all the free OS hackers behind the common cause and standardise
 *   some of this kind of stuff instead of implementing and reimplementing
 *   zillions of little bespoke hacks. Haha.
 *
 * Ideally, this utility should be made obsolete.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
	int launchurl( const char* url );
#ifdef __cplusplus
}
#endif


