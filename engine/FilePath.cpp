#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "FilePath.h"
#include "App.h"
#include "CreaturesArchive.h"
#include "Display/ErrorMessageHandler.h"
#include "../common/StringFuncs.h"
#include <algorithm>

#include "../common/FileFuncs.h"
#include "../common/StringFuncs.h"
#include "DirectoryManager.h"
// ****************************************************************************
FilePath::FilePath( std::string name, 
				   int directory , 
				   bool checkLocallyFirst/*= true*/,
				   bool forceLocal /*= false*/)
	: myName( name ),
	  myDirectory( directory ),
	  myUseLocalWorldDirectoryVersionOfTheFile(0)
{


#ifdef NETTY
	bool check=false;
	if (myName.find("f00e")!=-1)
	{
		theApp.Netty->TextOut("FilePath::GetFullPath",myName.c_str());
		check=true;
	}
#endif


	// For all requests we must search for the files in
	// both in the local world  directory version of the folder and the 
	// main directory versions that was installed (these files are global to all
	// worlds)
	// If the file is definitely found in the local directory then it will flagged
	// for use instead of any main directory versions.
	if(checkLocallyFirst)
	{
		std::string path;
		if(forceLocal || GetWorldDirectoryVersionOfTheFile(path))
		{
			myUseLocalWorldDirectoryVersionOfTheFile = true;
		}
	}
}
// ****************************************************************************
void FilePath::LowerCase()
{
	::LowerCase(myName);
}
// ****************************************************************************
std::string FilePath::GetFullPath() const
{
	if( myDirectory < 0 )
		return myName;

#ifdef NETTY
	bool check=false;
	if (myName.find("f00e")!=-1)
	{
		theApp.Netty->TextOut("FilePath::GetFullPath",myName.c_str());
		check=true;
	}
#endif


	std::string localpath;
	if(myUseLocalWorldDirectoryVersionOfTheFile )
	{
		GetWorldDirectoryVersionOfTheFile(localpath);
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("localpath",localpath.c_str());
		}
#endif

		return localpath; 
	}
	else
	{
		// Try in principal directory first
		// (under Linux the principal directory is not 0, but the
		// home directory)

		// SPARKY check the CU directories first
		std::string attempt = theDirectoryManager.GetDirectory( myDirectory,true) + myName;
		if (FileExists(attempt.c_str()))
		{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("CU",attempt.c_str());
		}
#endif

			return attempt;
		}

		if (myDirectory == IMAGES_DIR)
		{
			int x = attempt.find_last_of(".");
			if( x != -1 )
				attempt = attempt.substr( 0, x + 1 );
			attempt += "c16";
			if (FileExists(attempt.c_str()))
			{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("CU",attempt.c_str());
		}
#endif

				return attempt;
			}
		}

		// SPARKY then the main directories
		attempt = theDirectoryManager.GetDirectory( myDirectory) + myName;
		if (FileExists(attempt.c_str()))
		{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("MD",attempt.c_str());
		}
#endif

			return attempt;
		}

		if (myDirectory == IMAGES_DIR)
		{
			int x = attempt.find_last_of(".");
			if( x != -1 )
				attempt = attempt.substr( 0, x + 1 );
			attempt += "c16";
			if (FileExists(attempt.c_str()))
			{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("MD",attempt.c_str());
		}
#endif

				return attempt;
			}
		}


		// SPARKY and finally
		// Search in auxiliary directories for the file
		int n = theDirectoryManager.GetAuxDirCount( myDirectory );
		for (int i = 0; i < n; ++i)
		{
			std::string attempt = theDirectoryManager.GetAuxDir( myDirectory, i) + myName;
			if (FileExists(attempt.c_str()))
			{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("AD",attempt.c_str());
		}
#endif

				return attempt;
			}

			// Look for c16 as well, if in images directory
			if (myDirectory == IMAGES_DIR)
			{
				int x = attempt.find_last_of(".");
				if( x != -1 )
					attempt = attempt.substr( 0, x + 1 );
				attempt += "c16";
				if (FileExists(attempt.c_str()))
				{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("AD",attempt.c_str());
		}
#endif

					return attempt;
				}
			}

		}
		// SPARKY And if the file isn't there then we return the CU directory name

#ifdef NETTY
std::string x=theDirectoryManager.GetDirectory( myDirectory,true ) + myName;
		if(check)
		{
			theApp.Netty->TextOut("default",x.c_str());
		}
		return x;
#else
		return theDirectoryManager.GetDirectory( myDirectory,true ) + myName;
#endif

	}
}
// ****************************************************************************
std::string FilePath::GetFileName() const
{
	return myName;
}
// ****************************************************************************
CreaturesArchive &operator<<( CreaturesArchive &archive, FilePath const &filePath )
{
	archive << filePath.myDirectory << filePath.myName 
		<< filePath.myUseLocalWorldDirectoryVersionOfTheFile;
	return archive;
}
// ****************************************************************************
CreaturesArchive &operator>>( CreaturesArchive &archive, FilePath &filePath )
{

	archive >> filePath.myDirectory >> filePath.myName 
		>> filePath.myUseLocalWorldDirectoryVersionOfTheFile;


	// just do a quickie check to check that the file is still
	//  (or is now suddenly) available locally
	std::string path;
	if(filePath.GetWorldDirectoryVersionOfTheFile(path))
	{
		filePath.myUseLocalWorldDirectoryVersionOfTheFile = true;
	}
	else
	{
		filePath.myUseLocalWorldDirectoryVersionOfTheFile = false;
	}
	return archive;
}
// ****************************************************************************
bool operator==( FilePath const &l, FilePath const &r )
{
	return l.myDirectory == r.myDirectory  &&
		(CaseInsensitiveCompare( l.myName.c_str(), r.myName.c_str() ) == 0 )
		&&l.myUseLocalWorldDirectoryVersionOfTheFile ==
		r.myUseLocalWorldDirectoryVersionOfTheFile; 
}
// ****************************************************************************
bool operator!=( FilePath const &l, FilePath const &r )
{
	return !(l == r);
}
// ****************************************************************************
bool operator<( FilePath const &l, FilePath const &r )
{
	if( l.myDirectory < r.myDirectory ) return true;
	return l.myName < r.myName;
}
// ****************************************************************************
void FilePath::SetExtension( std::string extension )
{
	int x = myName.find_last_of(".");
	if( x == -1 )
		myName += "." + extension;
	else
		myName = myName.substr( 0, x + 1 ) + extension;
}
// ****************************************************************************
void FilePath::GetExtension( std::string& extension )
{
	int x = myName.find_last_of(".");

	if( x == -1 )
	{
		extension.erase();
	}
	else
	{
		extension =  myName.substr( x, myName.length() );
	}
}
// ****************************************************************************
void FilePath::SetFilePath( std::string name, int directory )
{
	myName = name;
	myDirectory = directory;
}
// ****************************************************************************
bool FilePath::GetWorldDirectoryVersionOfTheFile(std::string& path,
												bool forcecreate) const
{
#ifdef NETTY
	bool check=false;
	if (myName.find("rubber")!=-1)
	{
		theApp.Netty->TextOut("FilePath::GWDVOTF",myName.c_str());
		theApp.Netty->TextOut("FilePath::GWDVOTF",path.c_str());
		check=true;
	}
#endif

	// ****************************
	// SPARKY

	{	// This checks the CU directories
		path=theDirectoryManager.GetDirectory(myDirectory,true);
		std::string tempPath = path;
		path+= myName;

#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut(path.c_str());
		}
#endif


		if( FileExists( path.c_str() ) )
		{
			return true;
		}

		if(myDirectory == IMAGES_DIR)
		{
			std::string tempName;

			// get the name without the extension in case we have to alter it
			int x = myName.find_last_of(".");
				if( x != -1 )
			{
				tempName = myName.substr( 0, x + 1 );
			}

			tempName+= "c16";
			tempPath+= tempName;
			if(FileExists(tempPath.c_str()))
			{

				path = tempPath;
				return true;
			}
			
		}
	}


	// ****************************



	if(theDirectoryManager.GetWorldDirectoryVersion(myDirectory,
													path,
													forcecreate))
	{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut(path.c_str());
		}
#endif
		std::string tempPath = path;
		path+= myName;

		if( FileExists( path.c_str() ) )
		{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("exists",path.c_str());
		}
#endif

			return true;
		}
	
		// now check for C16 version as the call would have
		// sent S16 by default
		if(myDirectory == IMAGES_DIR)
		{
			std::string tempName;

			// get the name without the extension in case we have to alter it
			int x = myName.find_last_of(".");
				if( x != -1 )
			{
				tempName = myName.substr( 0, x + 1 );
			}

			tempName+= "c16";
			tempPath+= tempName;
			if(FileExists(tempPath.c_str()))
			{

				path = tempPath;
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut(path.c_str());
		}
#endif
				return true;
			}
			
		}
	}
	else
	{
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("Why did this happen?");
		}
#endif

	}
#ifdef NETTY
		if(check)
		{
			theApp.Netty->TextOut("Not there");
		}
#endif

	return false;
}
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************