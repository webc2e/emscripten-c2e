// --------------------------------------------------------------------------------------
// File:		FileScanner.cpp
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
#ifdef _WIN32
#include "FileScanner.h"
#include <string>

DirectoryScanner::DirectoryScanner(const char *filespec) {
  myHandle = FindFirstFile(filespec, &(myDat));
  // failure to prepare CDirectory Scanner....
  if (myHandle == INVALID_HANDLE_VALUE)
    myFlag = 0;
  else
    myFlag = -1;
  std::string apath;
  apath.assign(filespec);
  myPath.assign(apath, 0, apath.find_last_of(PATHSEP) + 1);
}

DirectoryScanner::~DirectoryScanner() { FindClose(myHandle); }

const char *DirectoryScanner::getNextFile() {
  if (myFlag == 0)
    return NULL;
  myRet.assign(myPath);
  myRet.append(myDat.cFileName);
  myFlag = FindNextFile(myHandle, &myDat);
  return myRet.c_str();
}

#endif
