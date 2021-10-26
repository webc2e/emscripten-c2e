// BenC 30Dec99

#ifndef FILEFUNCS_H
#define FILEFUNCS_H

#include <string>

// returns true if file or directory exists
bool FileExists( const char* filename );

#if defined(_WIN32) || defined(WINELIB)
	#include <windows.h>
#else
	// win32 replacements
	bool DeleteFile( const char* filename );
	bool MoveFile( const char* src, const char* dest );
	bool CopyFile( const char* src, const char* dest, bool overwrite );
	bool CreateDirectory(const char* name, void* ignored);
#endif

int FileSize( const char* filename );

std::string FileExtension(const std::string& filename);

// Return the path separator \ or /
// (code won't work with Mac or Acorn path seperators yet)
char* PathSeparator();
void AddTrailingPathSeparator(std::string& path);

#include <sstream>
// Munch white space, and return the first line after that
// Strip \r at the end
void GetLineCompatibleMunch(std::istream& in, char* buf, int len);
// and std::getline
void GetLineCompatible(std::istream& in, std::string& str);

void MakeFilenameSafe(std::string& filename);

#endif // FILEFUNCS_H

