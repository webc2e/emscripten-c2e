

// header file for users of GENERAL.CPP
// Windows version

// C2E - WOULD BE NICE TO KILL THIS FILE OFF ALLTOGETHER - Ben
// Too right! Robert
// So why didn't you? Ben ;-)

#ifndef general_h
#define general_h


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include <string>
#include <vector>

class Agent;	// so I can refer to it

enum DIRECTION
{
	DR_LEFT = 0,
	DR_RIGHT,
	DR_UP,
	DR_DOWN,

	DR_COUNT,
	DR_INVALID = DR_COUNT
};

// Return an ascii filespec in the form xxxx.yyy, given fsp as a 4-char token (eg 'NORN')
// and a 3 character string for the suffix.
// Eg. BuildFsp(Tok('g','r','e','n'),"spr") returns "gren.spr"
// SubDir is a constant referring to an optional path to the directory, eg. 
// BuildFsp(0x30303030,"tst",BODY_DATA_DIR) might return "C:\SFC\Body Data\0000.tst"
std::string BuildFsp(DWORD fsp,
			   char const* ext,
			   int SubDir=-1,
			   bool isOverlay = false);	
std::string BuildFspNoCache(DWORD fsp,
			   char const* ext,
			   int SubDir=-1,
			   bool isOverlay = false);	
void ClearFspCache();

// Find the most suitable available attachment or sprite file, given the specification
// of the creature & part number. Return the MONIKER of this file (not the path).
// If the explicit file is not found, try Variant 0. If still no good, try the previous age.
DWORD ValidFsp(int Part,			// part number
			   int Genus,			// Genus (NORN, GRENDEL, ETTIN, SIDE)
			   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
			   int Age,				// IDEAL age & variant to look for
			   int Breed,
			   char* Ext,			// file extension ("spr" or "att")
			   int Dir,				// subdirectory type, eg. BODY_DATA_DIR
			   bool isOverlay = false);


template <typename T>
void DeleteObjects( T start, T end )
{
	while( start != end )
		delete *start++;
}

bool GetFilesInDirectory(const std::string& path, std::vector<std::string>& files,
						 const std::string& wildcard = "*");
bool GetDirsInDirectory(const std::string& path, std::vector<std::string>& dirs);
bool GetMissingDirsInDirectory(const std::string& path,
							   const std::vector<std::string>& dirsToTestAgainst,
							   std::vector<std::string>& dirs);
bool CopyDirectory( std::string const &source, std::string const &destination );
bool DeleteDirectory( std::string directory );

std::string WildSearch(int f, int g, int s, const std::string& tag_stub);

std::string DisplayAndModuleString();
std::string OperatingSystemString();

#endif // general_h

