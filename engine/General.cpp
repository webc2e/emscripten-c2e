#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "General.h"
#include "App.h"
#include "Creature/SkeletonConstants.h"	// for MALE/FEMALE
#include "Display/ErrorMessageHandler.h"
#include "AgentManager.h"
#include "Maths.h"
#include "../common/Catalogue.h"
#include "World.h"
#include "../common/FileFuncs.h"
#include "DirectoryManager.h"
#include "ModuleImporter.h"
#include "QuickFsp.h"

#ifdef _WIN32
// for UUID stuff
#include <rpc.h>
#include <rpcdce.h>
#else
// uname
#include <sys/utsname.h>
#endif

#include <stdarg.h>
#include "../common/FileFuncs.h"
#ifdef _WIN32
#include <shellapi.h>
#else	// posix
#include "sys/stat.h"
#include "unistd.h"
#include "sys/types.h"
#include "dirent.h"
#include "fnmatch.h"
#endif

#include <sstream>
#include <map>

// Return an ascii filespec in the form xxxx.yyy, given fsp as a 4-char token (eg 'NORN')
// and a 3 character string for the suffix.
// Eg. BuildFsp(Tok('g','r','e','n'),"spr") returns "gren.spr"
// SubDir is a constant referring to an optional path to the directory, eg. 
// BuildFsp(0x30303030,"tst",BODY_DATA_DIR) might return "C:\SFC\Body Data\0000.tst"

// changed behaviour to accomodate world directories
// first check whether the file is in the local directory
// *if* it is *there* then choose it over the main directory
// otherwise *always* return the main directory version
// ****************************************************************************
std::string BuildFspNoCache(DWORD fsp, char const* ext, int SubDir /* =-1 */, bool isOverlay/* = false*/)
{



	// treat DWORD token as 4 chars
	char* AscFsp = (char*)&fsp;

	// copy chars into filespec from lo to hi
	std::string mainNamePart;
  	for (int i=0; i<4; i++)
    	mainNamePart += tolower(*AscFsp++);

	// extension
	mainNamePart += ".";
	mainNamePart += ext;

	std::string SFPath;
	if (SubDir != -1) 							// if a subdirectory is required
	{

#ifdef NETTY
	theApp.Netty->TextOut("BuildFSPNC");
#endif


		// First check whether the file is in the world directory...
		std::string path;
		if (theDirectoryManager.GetWorldDirectoryVersion(SubDir,path))
		{
			SFPath = path;

			if(isOverlay)
			{
				SFPath = SFPath + "over_";
			}

			SFPath = SFPath + mainNamePart;

			if(FileExists(SFPath.c_str()))
				return SFPath;
		}


		// ... otherwise look in auxiliary directories
		std::string stubName = mainNamePart;
		if (isOverlay)
		{
			stubName = "over_" + stubName;
		}
		FilePath filePath(stubName, SubDir);
		if (FileExists(filePath.GetFullPath().c_str()))
		{
			SFPath = filePath.GetFullPath().c_str();
			return SFPath;
		}

		// SPARKY Otherwise look in CU
		{
			SFPath=theDirectoryManager.GetDirectory(SubDir,true);
			SFPath+=stubName;
			if (FileExists(SFPath.c_str()))
			{
#ifdef NETTY
				theApp.Netty->TextOut("Fuck",SFPath.c_str());
#endif
				return SFPath;
			}

		}




		// ... finally use the directory itself
		SFPath = theDirectoryManager.GetDirectory(SubDir);
	}
	else
	{
		SFPath = "";						// else path starts with nothing
	}

	if (isOverlay)
		SFPath += "over_";
	SFPath += mainNamePart;					// add filename to optional path
#ifdef NETTY
	theApp.Netty->TextOut("BuildFSPNC",SFPath.c_str());
#endif

	return SFPath;
}

#include <map>
// ****************************************************************************
struct BuildFspParams
{
	DWORD fsp;
	std::string ext;
	int SubDir;
	bool isOverlay;

	bool operator<(const BuildFspParams& other) const
	{
		if (fsp < other.fsp)
			return true;
		if (fsp > other.fsp)
			return false;
		if (ext < other.ext)
			return true;
		if (ext > other.ext)
			return false;
		if (SubDir < other.SubDir)
			return true;
		if (SubDir > other.SubDir)
			return false;
		if (isOverlay < other.isOverlay)
			return true;
		if (isOverlay > other.isOverlay)
			return false;
		return false;
	}
};
// ****************************************************************************
std::map<BuildFspParams, std::string> ourBuildFspCache;
// Speed up this search...
// (it hits the disk too much)
std::string BuildFsp(DWORD fsp, char const* ext, int SubDir /* =-1 */, bool isOverlay/* = false*/)
{
	BuildFspParams params;
	params.fsp = fsp;
	params.ext = ext;
	params.SubDir = SubDir;
	params.isOverlay = isOverlay;

	std::map<BuildFspParams, std::string>::iterator it = ourBuildFspCache.find(params);
	if (it != ourBuildFspCache.end())
	{
#ifdef NETTY
	theApp.Netty->TextOut("BuildFSP",it->second.c_str());

	bool check=false;
	if (it->second.find("rubber")!=-1)
	{
		theApp.Netty->TextOut("Hello",it->second.c_str());
		check=true;
	}
#endif

		return it->second;
	}

	std::string result = BuildFspNoCache(fsp, ext, SubDir, isOverlay);
	ourBuildFspCache[params] = result;

#ifdef NETTY
	theApp.Netty->TextOut("BuildFSP",result.c_str());
#endif


	return result;
}
// ****************************************************************************
void ClearFspCache()
{
	ourBuildFspCache.clear();
	QuickFsp::ClearCache();
}

// Find the most suitable available attachment or sprite file, given the specification
// of the creature & part number. Return the MONIKER of this file (not the path).
// If the explicit file is not found, try Breed 0. If still no good, try the previous age.

// LEGACY FUNCTION: See QuickFsp.h/cpp for replacement.  This is still
// here so we can check the new function behaves the same (except for being quicker!)
// using ASSERTs
DWORD SlowValidFsp(int Part,			// part number
			   int Genus,			// Genus (NORN, GRENDEL, ETTIN, SIDE)
			   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
			   int Age,				// IDEAL age & variant to look for
			   int Breed,
			   char* Ext,			// file extension ("spr" or "att")
			   int Dir,				// subdirectory type, eg. BODY_DATA_DIR
			   bool isOverlay /*= false*/)
{

	// Range validation
	if (Part < 0)
		Part = 0;
	Part = Part % 26;
	if (Genus < 0)
		Genus = 0;
	Genus = Genus % 4;
	if (Sex < 1)
		Sex = 1;
	if (Sex > 2)
		Sex = 2;
	if (Age < 0)
		Age = 0;
	Age = Age % 10;
	if (Breed < 0)
		Breed = 0;
	Breed = Breed % 26;

	int s,v,g,a;
	DWORD Fsp;
	Sex-=MALE;										// cvt sex into range 0 to 1
	std::string string;

	for	(s=0; s<2; s++) {							// do ideal sex first, else opposite sex

		if	(s==0)
			g = Genus + Sex * 4;					// 0123 = male norn/gren/ett/side 4567=female
		else
			g = Genus + (1-Sex) * 4;				// 2nd time through, look for opposite sex

		for	(v=Breed; v>=0; v--) {				// variant is next most important item
	
			for	(a=Age; a>=0; a--) {				// if can't find this age, use previous age

				Fsp = (Part+'a') | 
							((g+'0')<<8) | 
							((a+'0')<<16) | 
							((v+'a')<<24);
				string = BuildFsp(Fsp,Ext,Dir,isOverlay);
				if (FileExists(string.c_str()))
				{
#ifdef NETTY
	theApp.Netty->TextOut("slowvalidFSP",string.c_str());
#endif
					return Fsp;
				}

			}										// else try previous age
		}											// and previous variant
	}

	for	(s=0; s<2; s++) {							// do ideal sex first, else opposite sex

		if	(s==0)
			g = Genus + Sex * 4;					// 0123 = male norn/gren/ett/side 4567=female
		else
			g = Genus + (1-Sex) * 4;				// 2nd time through, look for opposite sex

		// Try higher variants, as well as lower
		for	(v=Breed+1; v<26; v++) {				// variant is next most important item
	
			for	(a=Age; a>=0; a--) {				// if can't find this age, use previous age

				Fsp = (Part+'a') | 
							((g+'0')<<8) | 
							((a+'0')<<16) | 
							((v+'a')<<24);
				string = BuildFsp(Fsp,Ext,Dir,isOverlay);


#ifdef NETTY
	theApp.Netty->TextOut("slowvalidFSP",string.c_str());
#endif



				if (FileExists(string.c_str())) {
					return Fsp;
				}

			}										// else try previous age
		}											// and previous variant
	}												// and opposite sex

	// Try next genus down (grendel/norn/ettin)
	if (Genus != 0)
	{
		DWORD nfsp = SlowValidFsp(Part,Genus-1,Sex+MALE,Age,Breed,Ext,Dir,isOverlay);
		return nfsp;
	}
	return 0;
}
// ****************************************************************************
// Overall Fsp function - calls QuickFsp, and in debug mode checks result agains SlowValidFsp
DWORD ValidFsp(int Part,			// part number
			   int Genus,			// Genus (NORN, GRENDEL, ETTIN, SIDE)
			   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
			   int Age,				// IDEAL age & variant to look for
			   int Breed,
			   char* Ext,			// file extension ("spr" or "att")
			   int Dir,				// subdirectory type, eg. BODY_DATA_DIR
			   bool isOverlay /*= false*/)
{
	DWORD quick = QuickFsp::ValidFsp(Part, Genus, Sex, Age, Breed, Ext, Dir, isOverlay);

	// Check result against original slow function
	// (the new optimised algorithm won't in _all_ cases give the same
	// answers, because it is more sensibly structured.  In particular,
	// it doesn't have the confusing property of putting breed before sex
	// in some cases and sex before breed in others)
/* #ifdef _DEBUG
	DWORD slow = SlowValidFsp(Part, Genus, Sex, Age, Breed, Ext, Dir, isOverlay);
	ASSERT(slow == quick);
#endif */

	return quick;
}

bool GetFilesInDirectory(const std::string& path, std::vector<std::string>& files, const std::string& wildcard /* = "*" */)
{
#ifdef _WIN32
	std::string str = path + wildcard;
	WIN32_FIND_DATA finddata;
	HANDLE h = FindFirstFile( str.c_str(), &finddata );
	if( h == INVALID_HANDLE_VALUE )
		return false;
	do
	{
		if( !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			files.push_back( finddata.cFileName  );
	}
	while( FindNextFile( h, &finddata ) );
	DWORD err = GetLastError();
	FindClose( h );

	if( err != ERROR_NO_MORE_FILES )
		return false;
	else
		return true;
#else
	// posix version
	DIR* dir;
	struct dirent* dir_entry;
	dir = opendir( path.c_str() );
	if( !dir )
		return false;
	while( dir_entry = readdir(dir) ) 
	{
		struct stat s;
		int fnresult = fnmatch( wildcard.c_str(), dir_entry->d_name, 0 );
		if( fnresult == FNM_NOMATCH )
			continue;
		if( fnresult != 0 )	// error?
		{
			closedir(dir);
			return false;
		}

		std::string fullfilename( path + dir_entry->d_name );
		if( stat( fullfilename.c_str(), &s ) != 0 )
		{
			closedir(dir);
			return false;
		}

		if( !S_ISDIR( s.st_mode ) )
			files.push_back( dir_entry->d_name );
	}
	closedir( dir );
	return true;
#endif
}



bool GetDirsInDirectory(const std::string& path, std::vector<std::string>& dirs)
{
#ifdef _WIN32
	std::string str = path + "*";
	WIN32_FIND_DATA finddata;
	HANDLE h = FindFirstFile( str.c_str(), &finddata );
	if( h == INVALID_HANDLE_VALUE )
		return false;
	do
	{
		if( (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			(*finddata.cFileName != '.'))
			dirs.push_back( finddata.cFileName  );
	}
	while( FindNextFile( h, &finddata ) );
	DWORD err = GetLastError();
	FindClose( h );

	if( err != ERROR_NO_MORE_FILES )
		return false;
	else
		return true;
#else
	// posix version
	DIR* dir;
	struct dirent* dir_entry;
	dir = opendir( path.c_str() );
	if( !dir )
		return false;
	while( dir_entry = readdir(dir) ) 
	{
		struct stat s;
		std::string fullfilename( path + dir_entry->d_name );
		if( stat( fullfilename.c_str(), &s ) != 0 )
		{
			closedir(dir);
			return false;
		}

		if( S_ISDIR( s.st_mode ) &&
			strcmp( ".", dir_entry->d_name ) != 0 &&
			strcmp( "..", dir_entry->d_name ) != 0 )
		{
			dirs.push_back( dir_entry->d_name );
		}
	}
	closedir(dir);

	return true;
#endif
}

// only load the directories that do not appear in the specified vector to test against
bool GetMissingDirsInDirectory(const std::string& path,
							   const std::vector<std::string>& dirsToTestAgainst,
							   std::vector<std::string>& dirs)
{

	// first get them all
	if(GetDirsInDirectory(path,dirs))
	{
		std::vector<std::string>::const_iterator it;
		std::vector<std::string>::iterator itToGo;

		// then weed out the ones we don't want
		for( it = dirsToTestAgainst.begin(); it != dirsToTestAgainst.end(); it++ )
		{
			itToGo = std::remove( dirs.begin(), dirs.end(), (*it) );
			if(itToGo != dirs.end() )
			{
				dirs.erase(itToGo);
			}
		}
		return true;
	}

	return false;
}


bool CopyDirectory( std::string const &source, std::string const &destination )
{
	std::vector<std::string> files, dirs;
	std::vector<std::string>::const_iterator i;

#ifdef _WIN32
	const char* sep = "\\";
#else
	const char* sep = "/";
#endif

	CreateDirectory( destination.c_str(), NULL );

	GetFilesInDirectory( source + sep, files );

	for( i = files.begin(); i != files.end(); ++i )
		CopyFile( (source + sep + *i).c_str(),
			(destination + sep + *i).c_str(), FALSE );

	GetFilesInDirectory( source + sep, dirs );
	for( i = dirs.begin(); i != dirs.end(); ++i )
		CopyDirectory( (source + sep + *i).c_str(),
			(destination + sep + *i).c_str() );

	return true;
}



#ifdef _WIN32
bool DeleteDirectory( std::string directory )
{
	SHFILEOPSTRUCT fileOp;

	//this struct requires a *double* zero terminated string!
	char *buffer = new char[ directory.size() + 2 ];
	directory.copy( buffer, directory.size() );
	buffer[ directory.size() ] = 0;
	buffer[ directory.size() + 1 ] = 0;

	fileOp.hwnd = 0;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = buffer;
	fileOp.pTo = 0;
	fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;

	if( SHFileOperation( &fileOp ) != 0 )
	{
		ErrorMessageHandler::Show("archive_error", 8, "World::Delete");
		return false;
	}

	delete [] buffer;
	return true;
}
#else

// Recursive delete (posix version)
bool DeleteDirectory( std::string directory )
{

	if( directory[ directory.size()-1 ] != '/' )
		directory += '/';

	DIR* dir;
	struct dirent* dir_entry;
	dir = opendir( directory.c_str() );
	if( !dir )
		return false;

	while( dir_entry = readdir(dir) )
	{
		// this bit is quite important :-)
		if( strcmp( dir_entry->d_name, "." )==0 ||
			strcmp( dir_entry->d_name, ".." )==0 )
		{
			continue;
		}


		struct stat s;
		std::string fullfilename( directory + dir_entry->d_name );

		if( stat( fullfilename.c_str(), &s ) != 0 )
		{
			closedir(dir);
			return false;
		}

		if( S_ISDIR( s.st_mode ) )
		{
			if( !DeleteDirectory( fullfilename.c_str() ) )
			{
				closedir(dir);
				return false;
			}
		}
		else
		{
			if( unlink( fullfilename.c_str() ) != 0 )
			{
				closedir(dir);
				return false;
			}
		}
	}

	closedir(dir);

	// zap the now-empty dir
	if( rmdir( directory.c_str() ) != 0 )
	{
		return false;
	}
	
	return true;
}

#endif //_WIN32

std::string WildSearch(int f, int g, int s, const std::string& tag_stub)
{
	std::string final_tag;

	std::ostringstream out1;
	out1 << tag_stub << " " << f << " " << g << " " << s;
	if (theCatalogue.TagPresent(out1.str()))
		final_tag = out1.str();
	else
	{
		std::ostringstream out2;
		out2 << tag_stub << " " << f << " " << g << " 0";
		if (theCatalogue.TagPresent(out2.str()))
			final_tag = out2.str();
		else
		{
			std::ostringstream out3;
			out3 << tag_stub << " " << f << " 0 0";
			if (theCatalogue.TagPresent(out3.str()))
				final_tag = out3.str();
		}
	}

	return final_tag;
}

std::string DisplayAndModuleString()
{
	std::string str;
#ifdef C2D_DIRECT_DISPLAY_LIB
	str = "DirectDisplay DirectX";
#else
	#ifdef C2E_SDL
		str = "OriginalDisplay SDL";
	#else
		str = "OriginalDisplay DirectX";
	#endif
#endif

	str += " (";
	std::string modules = ModuleImporter::GetModulesDescriptiveList();
	if (modules.empty())
		modules = "no modules";
	str += modules;
	str += ")";
	return str;
}

std::string OperatingSystemString()
{
	std::string str;
#ifdef _WIN32
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	GetVersionEx(&versionInfo);
	std::ostringstream out;

	out << "Windows ";
	if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32s)
		out << "3.1";
	else if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		out << "95/98";
	else if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		out << "NT/2000";
	else
		out << "Unknown";

	out << " ";
	out << versionInfo.dwMajorVersion << "." << versionInfo.dwMinorVersion;

	if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		out << " build " << LOWORD(versionInfo.dwBuildNumber);
	else
		out << " build " << versionInfo.dwBuildNumber;

  	out << " (" << versionInfo.szCSDVersion << ")";
	
	str = out.str();
#else
	struct utsname uinfo;
	uname(&uinfo);
	str = std::string(uinfo.sysname) + std::string(" ") + std::string(uinfo.nodename) + std::string(" ") +
		  std::string(uinfo.release) + std::string(" ") + std::string(uinfo.version) + std::string(" ") +
		  std::string(uinfo.machine) + std::string(" ");
#endif
	return str;
}

