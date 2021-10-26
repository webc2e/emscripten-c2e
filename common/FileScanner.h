// --------------------------------------------------------------------------------------
// File:		FileScanner.h
// Class:		DirectoryScanner
// Purpose:		Non recursive file finder for the genetics permutations
// generator NextGEN Description:
//  The DirectoryScanner class is used to search for genetics files in
//  directories. It is designed to have a Java style enumeration interface.
//  calling getNextFile() until the result is NULL.
//
// History:
//  April99		DanS	Initial Version
//	16Jun99		DanS	Restyled to bring into line with coding styles.
//
// --------------------------------------------------------------------------------------

#ifndef FILESCANNER_H
#define FILESCANNER_H

#ifdef _WIN32

#include "windows.h"

// --------------------------------------------------------------------------------------
// Filename separator descriptions.
//
// Purpose: If program is ever ported to platforms other than windows, then this
// pathsep
//          system will allow easy porting to filesystems other than those using
//          '\' as
//			the separator
// --------------------------------------------------------------------------------------
#if defined(__RISCOS__)
#define PATHSEP '.'
#define PATHSEPS "."
#define PATHSEPSD ".."
#elif defined(__OS2__) || defined(__MSDOS__) || defined(__WINNT__) ||          \
    defined(WIN32)
#define PATHSEP '\\'
#define PATHSEPS "\\"
#define PATHSEPSD "\\\\"
#else // Assume a sane filing system
#define PATHSEP '/'
#define PATHSEPS "/"
#define PATHSEPSD "//"
#endif

#include <string>

class DirectoryScanner {
private:
  // Attribute: myDat - Windows Find Data - this makes the class non-portable.
  WIN32_FIND_DATA myDat;

  // Attribute: myHandle - Windows handle on the find process
  HANDLE myHandle;

  // Attribute: myFlag - A flag used to indicate the end of the scan has been
  // reached
  int myFlag;

  // Attribute: myPath - The path under scrutiny.
  std::string myPath;

  // Attribute: myRet - The container for the string being returned.
  std::string myRet;

public:
  // ----------------------------------------------------------------------------------
  // Constructor
  // Arguments:		const char *filespec - The wildcarded filespec to scan.
  // Returns:			(None)
  // Description:		Prepares the class for enumerating the files
  // ----------------------------------------------------------------------------------
  DirectoryScanner(const char *filespec);

  // ----------------------------------------------------------------------------------
  // Destructor
  // Arguments, Returns: (None)
  // Description:			Cleans the class up, releases Sys resources &
  // memory
  // ----------------------------------------------------------------------------------
  ~DirectoryScanner();

  // ----------------------------------------------------------------------------------
  // Method:		getNextFile
  // Arguments:	(None)
  // Returns:		const char* - the name of the next file in the
  // enumeration Description:	This is where the file is selected from the list
  // of remaining files.
  // ----------------------------------------------------------------------------------
  const char *getNextFile();
};

#endif // FILESCANNER_H

#endif
