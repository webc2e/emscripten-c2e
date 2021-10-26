// Unix

// --------------------------------------------------------------------------
// Filename:	MemoryMappedFile.cpp
// Class:		MemoryMappedFile
// Purpose:		Allows client to map to a file on disk or map to part
//				of an already existing memory mapped file.
//
// Description:
//				note myFileData can be moved to the relevant part of the 
//				memory mapped view e.g. in a composite file like creature
//				gallery I like to skip the offset and moniker key.
//				However myConstantPtrToViewOfFile will _always_ point to
//				the start of the file mapping.  Always use this pointer to
//				unmap the file.
//
// History:
// --------------------------------------------------------------------------


#include "MemoryMappedFile.h"
//#include "ErrorMessageHandler.h"


#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <map>

std::map<std::string, MemoryMappedFile::FileMapping> ourMemoryMappedFileCache;
std::map<std::string, int> ourMemoryMappedFileCount;

MemoryMappedFile::MemoryMappedFile() :
	myFile(-1),myLength(0),myBasePtr(NULL),myPosition(0),
	myMappedToExistingFileOKFlag(false), myActualMapPtr(NULL),
	myActualLength(0)
{
}

MemoryMappedFile::MemoryMappedFile( const std::string& filename,
	uint32 desiredAccessFlags, uint32 sharemodeFlags ) :
	myFile(-1),myLength(0),myBasePtr(NULL),myPosition(0),
	myMappedToExistingFileOKFlag(false), myActualMapPtr(NULL),
	myActualLength(0)
{
	Open( filename, desiredAccessFlags, sharemodeFlags );
}



void MemoryMappedFile::Open( const std::string& filename,
	uint32 desiredAccessFlags,	// =GENERIC_READ|GENERIC_WRITE
	uint32 sharemodeFlags,		// =FILE_SHARE_READ|FILE_SHARE_WRITE
	uint32 fileSize )			// =0
{
	// In case of reopening (when ageing creatures),
	// close first
	Close();

	// open the file
	int oflags = 0;

	if( ( desiredAccessFlags & GENERIC_READ) &&
		( desiredAccessFlags & GENERIC_WRITE ) )
	{
		oflags |= O_RDWR;
	}
	else if( desiredAccessFlags & GENERIC_READ )
        	oflags |= O_RDONLY;
    	else if( desiredAccessFlags & GENERIC_WRITE )
        	oflags |= O_RDWR;

	// We store file handles in a cache to reduce the number of open file
	// handles for Norns, which the rest of the code tends to open multiple
	// times.  This is limited to 1024 in Linux (kernel 2.4 era) per process.
	myFilename = filename;
	if (ourMemoryMappedFileCache.find(filename) != ourMemoryMappedFileCache.end())
	{
		myFile = ourMemoryMappedFileCache[filename];
		++ourMemoryMappedFileCount[filename];
		// std::cout << "*** Reused " << filename << " count " << ourMemoryMappedFileCount[filename] << std::endl;
	}
	else
	{
		myFile = open( filename.c_str(), oflags, S_IREAD|S_IWRITE );
		if( myFile < 0 )
		{
			perror("MemoryMappedFileException open perror");
			throw MemoryMappedFileException(
				std::string("MemoryMappedFile::Open() - open failed for ") + filename, __LINE__ );
		}
		ourMemoryMappedFileCache[filename] = myFile;
		ourMemoryMappedFileCount[filename] = 1;
		// std::cout << "*** New " << filename << " count " << ourMemoryMappedFileCount[filename] << std::endl;
	}

	if( fileSize == 0 )
	{
		// get size of entire file
		struct stat inf;
		if( fstat( myFile, &inf ) != 0 )
		{
			throw MemoryMappedFileException(
				"MemoryMappedFile::Open() - fstat failed", __LINE__ );
		}
		myLength = (uint32)inf.st_size;
	}
	else
	{
		myLength = fileSize;
		// under Unix, a file must be large
		// enough _before_ you map it
		ftruncate(myFile, myLength);
	}

	// map it into memory

	int flags = MAP_SHARED;	// or MAP_PRIVATE?
	int prot = 0; 
	if( desiredAccessFlags & GENERIC_WRITE )
		prot |= PROT_WRITE;
	if( desiredAccessFlags & GENERIC_READ )
		prot |= PROT_READ;

	myBasePtr = (byte*)mmap( 0, myLength, prot, flags, myFile, 0 );
	myActualMapPtr = myBasePtr;
	myActualLength = myLength;

	if( myBasePtr == MAP_FAILED )
	{
		throw MemoryMappedFileException(
			"MemoryMappedFile::Open() - mmap failed", __LINE__ );
	}
}





MemoryMappedFile::~MemoryMappedFile()
{
	Close();
}



void MemoryMappedFile::Close()
{
	if( myActualMapPtr && myActualLength > 0 )
		munmap( myActualMapPtr, myActualLength );

	if( myFile != -1)
	{
		// Close file if nobody is using the cached file handle any more
		ourMemoryMappedFileCount[myFilename]--;
		// std::cout << "*** Uncached " << myFilename << " count " << ourMemoryMappedFileCount[myFilename] << std::endl;
		if (ourMemoryMappedFileCount[myFilename] == 0)
		{
			// std::cout << "*** Closed " << myFilename << " count " << std::endl;
			close( myFile );
			ourMemoryMappedFileCount.erase(ourMemoryMappedFileCount.find(myFilename));
			ourMemoryMappedFileCache.erase(ourMemoryMappedFileCache.find(myFilename));
		}
		myFile = -1;
	}

	myActualMapPtr = NULL;
	myActualLength = 0;
	myBasePtr = NULL;
	myLength = 0;
	myPosition = 0;
//gtb fix to match windows version, this line commented out:
//myMappedToExistingFileOKFlag = false;
}



void MemoryMappedFile::Seek(int32 moveBy, File::FilePosition from)
{
	uint32 newpos;
	switch(from)
	{
	case(File::Start): 
		{
			newpos = moveBy;
			break;
		}

	case(File::Current):
		{
			newpos = myPosition + moveBy;
			break;
		}
	case(File::End):
		{
			newpos = myLength - moveBy;
			break;
		}
	}

	// allowed to point to last+1
	if( newpos > myLength || newpos < 0)
	{
		throw MemoryMappedFileException(
			"MemoryMappedFile::Seek() - out of range", __LINE__ );
	}
		
	myPosition = newpos;
}

MemoryMappedFile::MemoryMappedFile(FileMapping memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap)
				 :myFile(-1/*was 0, fix by gtb and fai*/), // you don't have your own memory file
				myLength(numBytesToMap),
				myPosition(0),
				myActualMapPtr(NULL),
				myActualLength(0)
{
	int flags = MAP_SHARED;	// or MAP_PRIVATE?
	int prot = PROT_READ; 

	myBasePtr = (byte*)mmap( 0, myLength, prot, flags, memoryFile, 0 );
	myActualMapPtr = myBasePtr;
	myActualLength = myLength;

	if( myBasePtr == MAP_FAILED )
	{
		std::cout << "Error number " << errno << std::endl;
		throw MemoryMappedFileException(
			"MemoryMappedFile::Open() - mmap of others file failed", __LINE__ );
	}

	ASSERT(highOffset == 0);
	myBasePtr += lowOffset;
	myLength -= lowOffset;

	myMappedToExistingFileOKFlag = true;
}


