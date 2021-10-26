// --------------------------------------------------------------------------
// Filename:	Creatures Archive.cpp
// Class:		CreaturesArchive
// Purpose:		Serialises persistent objects to and from a file
//
// Description:
//
// Objects of class PersistentObject can be written to a CreaturesArchive
// in such a way that they can safely be read in again with their pointers
// still making sense.  This class will also take circular references into
// account.
//
// And class which should be serialised should be derived from 
// PersistentObject, with the Load and Save macros overridden.  All derived
// classes should be added to the list of tests in Read(PersistentObject *)
//
// History:
// 22Apr98	Peter Chilvers	Created
// 09Jun98	Peter Chilvers	Altered Read / Write (PersistentObject *) to
//							use run time information
//							Fixed bug in Read(CString &string)
// 10Apr98	Peter Chilvers	Extended list of types supported by Read/Write
// 14Aug98	Peter Chilvers	Converted Write to take const argument
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreaturesArchive.h"
#include "PersistentObject.h"
#include <string>

const int BUFFER_SIZE = 4096;

// Compression level 0 - 9
// Can be set at runtime by the static member SetZLibCompressionLevel
int C2E_MODULE_API CreaturesArchive::ourZLibCompressionLevel = 6;

// ----------------------------------------------------------------------
// Method:		CreaturesArchive
// Arguments:	file	 - File being written to / read from
//				rwStatus - Reading / Writing
// Returns:		Nothing
// Description:	Constructs an archive to either read from a file or
//				write to a file (which should already be open)
// ----------------------------------------------------------------------
CreaturesArchive::CreaturesArchive( std::istream &stream, Mode mode, bool bNoVersion /* = false */ )
	: myInStream(&stream), myMode( mode ),
	myAgentArchiveStyle(PERSISTENT_OBJECTS),
	myAgentArchivedCount(0), myFirstAgent(NULL),
	myUpdateProgressBarFunction(NULL)
{
	ASSERT(myInStream->good());

	myCloningASkeletalCreature = false;

	myStreamBuffer.zalloc = (alloc_func)NULL;
	myStreamBuffer.zfree = (free_func)NULL;
	myStreamBuffer.opaque = (voidpf)NULL;

	myCompressedDataBuffer = new unsigned char[BUFFER_SIZE];
	myUncompressedDataBuffer = new unsigned char[BUFFER_SIZE];

	std::string baseHint = "Creatures Evolution Engine - Archived information file. zLib 1.13 compressed.";
	baseHint += (char)26; // MS-DOS EOF
	baseHint += (char)4;  // Linux EOF (Hopefully)
	std::string readHint = baseHint;

	ASSERT(mode == Load);

	myStreamBuffer.next_in = myCompressedDataBuffer;
	myStreamBuffer.next_out = myUncompressedDataBuffer;
	myStreamBuffer.avail_in = 0;
	myStreamBuffer.avail_out = BUFFER_SIZE;
	myLastUncompressedDataRead = myUncompressedDataBuffer;
	int err = inflateInit(&myStreamBuffer);
	if (err != Z_OK)
	{
		throw Exception( "CRA0001: zlib failed to initialise" );
	}
	myInStream->read(&(readHint.at(0)),readHint.length());
	if (myUpdateProgressBarFunction)
		myUpdateProgressBarFunction(myInStream->gcount());
	if (readHint != baseHint)
	{
		throw Exception( "CRA0002: Not a creatures archive" );
	}

	Read( myVersion );
	if (!bNoVersion)
	{
		if( myVersion > GetCurrentVersion() )
		{
#ifndef NON_LOCALISED_ERRORS
			std::string str = ErrorMessageHandler::Format("archive_error", 6, "CreaturesArchive::CreaturesArchive");
#else
			std::string str = "Incompatible version";
#endif
			throw Exception( str.c_str() );
		}
	}
}

CreaturesArchive::CreaturesArchive( std::ostream &stream, Mode mode, bool bNoVersion /* = false */ )
	: myOutStream(&stream), myMode( mode ),
	myAgentArchiveStyle(PERSISTENT_OBJECTS),
	myAgentArchivedCount(0), myFirstAgent(NULL)
{
	ASSERT(myOutStream->good());

	myCloningASkeletalCreature = false;

	myStreamBuffer.zalloc = (alloc_func)NULL;
	myStreamBuffer.zfree = (free_func)NULL;
	myStreamBuffer.opaque = (voidpf)NULL;

	myCompressedDataBuffer = new unsigned char[BUFFER_SIZE];
	myUncompressedDataBuffer = new unsigned char[BUFFER_SIZE];

	std::string baseHint = "Creatures Evolution Engine - Archived information file. zLib 1.13 compressed.";
	baseHint += (char)26; // MS-DOS EOF
	baseHint += (char)4;  // Linux EOF (Hopefully)
	std::string readHint = baseHint;

	ASSERT(mode == Save);

	myStreamBuffer.next_in = myUncompressedDataBuffer;
	myStreamBuffer.next_out = myCompressedDataBuffer;
	myStreamBuffer.avail_in = 0;
	myStreamBuffer.avail_out = BUFFER_SIZE;
	int err = deflateInit(&myStreamBuffer, ourZLibCompressionLevel);
	if (err != Z_OK)
	{
		throw Exception( "CRA0003: zlib failed to initialise" );
	}
	myOutStream->write(readHint.c_str(),readHint.length());

	myVersion = GetCurrentVersion();
	Write( myVersion );
}

// ----------------------------------------------------------------------
// Method:		~CreaturesArchive
// Arguments:	None
// Returns:		Nothing
// Description:	Destructor.
// ----------------------------------------------------------------------
CreaturesArchive::~CreaturesArchive()
{
	if (myMode == Load)
		inflateEnd(&myStreamBuffer);
	else
	{
		int zret = Z_OK;
		while (myStreamBuffer.avail_in > 0 || zret == Z_OK)
		{
			zret = deflate(&myStreamBuffer,Z_FINISH);
			if (myStreamBuffer.avail_out < BUFFER_SIZE)
			{
				myOutStream->write((char*)myCompressedDataBuffer,BUFFER_SIZE - myStreamBuffer.avail_out);
				myStreamBuffer.next_out = myCompressedDataBuffer;
				myStreamBuffer.avail_out = BUFFER_SIZE;
			}
		}
		deflateEnd(&myStreamBuffer);
	}
	delete [] myCompressedDataBuffer;
	delete [] myUncompressedDataBuffer;
}



// ----------------------------------------------------------------------
// Method:		Skip
// Arguments:	count - number of bytes to skip over
// Returns:		Nothing
// Description:	Does a forward seek in the stream
// ----------------------------------------------------------------------
void CreaturesArchive::Skip( int count )
{
	_ASSERT(false); // We shouldn't be skipping around.
	// myStream->seekg( count, std::istream::cur );
}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	an object of one of several provided types
// Returns:		Nothing
// Description:	Adds the object to the archive, taking previous
//				references to persistent objects into account
// ----------------------------------------------------------------------
void CreaturesArchive::Write( int value)
{
	ASSERT( IsSaving() );
	WriteBinary( value );
}


void CreaturesArchive::Write(uint8 value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}



// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	an object of one of several provided types
// Returns:		Nothing
// Description:	Adds the object to the archive, taking previous
//				references to persistent objects into account
// ----------------------------------------------------------------------
void CreaturesArchive::Write(double value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}

void CreaturesArchive::Write(uint16 value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}


void CreaturesArchive::Write(uint32 value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}

void CreaturesArchive::Write(float value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}

void CreaturesArchive::Write(int32 value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}

void CreaturesArchive::Write(int16 value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}

void CreaturesArchive::Write(int8 value)
{
	ASSERT( IsSaving());
	WriteBinary( value );
}

void CreaturesArchive::Write(LPCTSTR string)
{
	ASSERT( IsSaving());
	int32 len = strlen( string );
	WriteBinary( len );
	Write( string, len );
}

void CreaturesArchive::Write(const std::string& string)
{
	ASSERT( IsSaving() );
	int32 len = string.size();
	WriteBinary( len );
	Write( string.c_str(), len );
}

void CreaturesArchive::Write(const SYSTEMTIME& time)
{
	ASSERT( IsSaving() );
	WriteBinary( time.wDay );
	WriteBinary( time.wDayOfWeek );
	WriteBinary( time.wHour );
	WriteBinary( time.wMilliseconds );
	WriteBinary( time.wMinute );
	WriteBinary( time.wMonth );
	WriteBinary( time.wSecond );
	WriteBinary( time.wYear );
}

/*
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;
*/


void CreaturesArchive::Write(const Vector2D &v)
{
	WriteBinary(v.x);
	WriteBinary(v.y);
}


void CreaturesArchive::Write(const Box &rect)
{
	WriteBinary(rect.left);
	WriteBinary(rect.top);
	WriteBinary(rect.right);
	WriteBinary(rect.bottom);
}

void CreaturesArchive::Write(const RECT& rect)
{
	WriteBinary(rect.left);
	WriteBinary(rect.top);
	WriteBinary(rect.right);
	WriteBinary(rect.bottom);
}

void CreaturesArchive::Write( const void *data, size_t count )
{
	// Do some compression, and pump the data out...
	int written = 0;
	while (written < count)
	{
		// transfer as much information as we can into the input buffer...
		if (myStreamBuffer.avail_in < BUFFER_SIZE)
		{
			int toAdd = count - written;
			if (toAdd > (BUFFER_SIZE - myStreamBuffer.avail_in))
				toAdd = (BUFFER_SIZE - myStreamBuffer.avail_in);
			memcpy(myStreamBuffer.next_in + myStreamBuffer.avail_in, (char*)data + written,toAdd);
			written += toAdd;
			myStreamBuffer.avail_in += toAdd;
		}
		if (myStreamBuffer.avail_in < BUFFER_SIZE)
			return; // We have not filled the buffer, so there wasn't enough data to think about

		// Right then, we have a full input buffer, let's compress it & dump the output
		// to disk...
		while (myStreamBuffer.avail_in > 0)
		{
			deflate(&myStreamBuffer,0);
			if (myStreamBuffer.avail_out == 0)
			{
				// we have some data to dump to disk...
				myOutStream->write( (char*)myCompressedDataBuffer,BUFFER_SIZE );
				myStreamBuffer.next_out = myCompressedDataBuffer;
				myStreamBuffer.avail_out = BUFFER_SIZE;
			}
			// Repeat until we have no more input...
		}

		// Right then, there is no more input in the input buffer, so return to the written < count
		myStreamBuffer.avail_in = 0;
		myStreamBuffer.next_in = myUncompressedDataBuffer;
	}
}



void CreaturesArchive::Write(const PersistentObject *object)
{

	ASSERT( IsSaving() );

	// Is this a null pointer ?
	if (object == NULL)
	{
		// Yes - just write an empty value
		Write ( NULL_ARCHIVE_OBJECT );
		return;
	}

	if ((myAgentArchiveStyle != PERSISTENT_OBJECTS))
	{
		if( object->IsAgent() )
		{
			myAgentArchivedCount++;

			if (myAgentArchivedCount > 1)
			{
				if (myFirstAgent == object)
				{
					Write ( FIRST_AGENT );
					return;
				}
				if (myAgentArchiveStyle == ONE_AGENT_ONLY_NULL_OTHERS)
				{
					Write ( NULL_ARCHIVE_OBJECT );
					return;
				}
				else if (myAgentArchiveStyle == ONE_AGENT_ONLY_COPY_POINTER)
				{
					Write ( RAW_POINTER );
					WriteBinary ( object );
					return;
				}
			}
			else
			{
				myFirstAgent = object;
			}
		}
	}
	
	// Has this object already been serialised?
	std::map< PersistentObject const*, int32 >::const_iterator
		itor = myArchiveMap.find( object );

	if( itor == myArchiveMap.end() )
	{
		// We need to add this object to the archive, and
		// write its data to the file
		int32 id = myArchiveMap.size();
		myArchiveMap[object] = id;

		// Write the new id to the file
		Write ( id );

		std::map< std::string, int32 >::const_iterator
			classItor = myClassMap.find( object->GetClassNameX() );
		if( classItor == myClassMap.end() )
		{
			int32 classID = myClassMap.size();
			myClassMap[ object->GetClassNameX() ] = classID;
			Write ( classID );
			Write ( object->GetClassNameX() );
		}
		else
		{
			Write ( classItor->second );
		}
		// Write the class name of the object as a string

		// Now write the rest of the object's data to the archive
		Write("OBST");
		object -> Write ( *this );
		Write("OBEN");
	}
	else
	{
		// The object has already been serialised, so we just write
		// out its ID.  When the object is read back in, the archiver
		// will recognise that it has already been loaded at an earlier
		// point, and will return a pointer to the earlier object
		Write ( itor->second );

	}
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	an reference to an object of one of several provided types
// Returns:		Nothing
// Description:	Adds the object to the archive, taking previous
//				references to persistent objects into account
// ----------------------------------------------------------------------
void CreaturesArchive::Read(int &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}


void CreaturesArchive::Read(uint8 &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(double &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(uint16 &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(uint32 &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(float &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(int8 &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(int32 &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read(int16 &value)
{
	ASSERT( IsLoading() );

	ReadBinary(value);
}

void CreaturesArchive::Read( std::string& value)
{
	ASSERT( IsLoading() );
	int32 len;
	ReadBinary(len);
	if (len < 0)
		throw Exception( "CRA0009: negative string size" );
	// Sanity check
	if (len < 100000)
	{
		value.resize(len);
		Read( &value[0], len );
	}
	else
	{
		// For long strings, we read them slower
		// This is to avoid trouble with allocating
		// all the memory in the computer, and the archive
		// containing it all as it is broken
		value = "";
		for (int i = 0; i < len; ++i)
		{
			char c;
			ReadBinary(c);
			value += c;
		}
	}
}

void CreaturesArchive::Read( SYSTEMTIME& time)
{
	ASSERT( IsLoading() );
	ReadBinary( time.wDay );
	ReadBinary( time.wDayOfWeek );
	ReadBinary( time.wHour );
	ReadBinary( time.wMilliseconds );
	ReadBinary( time.wMinute );
	ReadBinary( time.wMonth );
	ReadBinary( time.wSecond );
	ReadBinary( time.wYear );
}

void CreaturesArchive::Read(Vector2D &v)
{
	Read(v.x);
	Read(v.y);
}


void CreaturesArchive::Read(Box &rect)
{
	Read(rect.left);
	Read(rect.top);
	Read(rect.right);
	Read(rect.bottom);
}

void CreaturesArchive::Read(RECT& rect)
{
	Read(rect.left);
	Read(rect.top);
	Read(rect.right);
	Read(rect.bottom);
}



void CreaturesArchive::Read( void *buffer, size_t count )
{
	// Let's do it...
	int read = 0;
	while (read < count)
	{
		// If there is some data in the buffer left, use it...
		if (myLastUncompressedDataRead < myStreamBuffer.next_out)
		{
			int toRead = count - read;
			if (toRead > (myStreamBuffer.next_out - myLastUncompressedDataRead))
				toRead = (myStreamBuffer.next_out - myLastUncompressedDataRead);
			memcpy((char*)buffer + read, myLastUncompressedDataRead,toRead);
			myLastUncompressedDataRead += toRead;
			read += toRead;
		}
		if (read == count)
			return; // No need to process more...

		// Okay then, I've used up all the output...
		myLastUncompressedDataRead = myUncompressedDataBuffer;
		myStreamBuffer.next_out = myUncompressedDataBuffer;
		myStreamBuffer.avail_out = BUFFER_SIZE;
		// Next we have to deal such that, until we have a fully used output buffer,
		// or we hit the end of our input stream, deal with it...
		if (myInStream->good())
		{
			while (myInStream->good() && myStreamBuffer.avail_out > 0)
			{
				// Right then, if we have run out of input, get another chunk....
				if (myStreamBuffer.avail_in == 0)
				{
					myStreamBuffer.next_in = myCompressedDataBuffer;
					myInStream->read((char*)myCompressedDataBuffer,BUFFER_SIZE);
					myStreamBuffer.avail_in = myInStream->gcount();
					if (myUpdateProgressBarFunction)
						myUpdateProgressBarFunction(myInStream->gcount());
				}
				int result = inflate(&myStreamBuffer,0);
				if (result != Z_OK && result != Z_STREAM_END)
					throw Exception( "CRA0004: zlib inflate error reading compressed stream" );
			}
		}
		else
		{
			// Erm, we have no more in the stream, but there may be data in the buffer left
			int result = inflate(&myStreamBuffer,0);
			if (result != Z_OK && result != Z_STREAM_END)
				throw Exception( "CRA0005: zlib inflate error reading compressed stream" );
		}
		
		// No more was read in, when we needed it... ;-(
		if (myStreamBuffer.next_out == myUncompressedDataBuffer)
		{
			throw Exception( "CRA0006: decompression stream ended before expected" );
		}
	}
}


void CreaturesArchive::Read(PersistentObject *&object)
{
	// First, read in the id of the persistent object
	int id;
	Read(id);

	// Check for a null pointer first
	if (id == NULL_ARCHIVE_OBJECT)
	{
		// No more work to do...
		object = NULL;
		return;
	}

	// Serialised through memory, and pointing to same object
	if (id == RAW_POINTER)
	{
		ReadBinary(object);
		return;
	}

	if (id == FIRST_AGENT)
	{
		object = const_cast<PersistentObject *>(myFirstAgent);
		return;
	}

	// Is this object already in the archive?
	if(id >= 0 && id < myArchiveVector.size() )
	{
		object = myArchiveVector[id];
		return;
	}

	// Next read in the name of the class that we need to create
	std::string className;
	int32 classID;
	Read( classID );
	if( classID < myClassVector.size() )
	{
		className = myClassVector[ classID ];
	}
	else
	{
		Read( className );
		myClassVector.push_back( className );
	}


	// Hack by gtb:
	// If we're reading in a C3 archive change the Creature objects
	// to be SkeletalCreatures:
	if (GetFileVersion()<14 && className=="Creature")
	{
		className = "SkeletalCreature";
	}

	// Create an object of the class name's type
	object = PersistentObject::New( className.c_str() );

	// Was an object successfully created ?
	if (object)
	{
		if ((myAgentArchiveStyle != PERSISTENT_OBJECTS))
		{
			if( object->IsAgent() )
			{
				myAgentArchivedCount++;

				if (myAgentArchivedCount == 1)
					myFirstAgent = object;
			}
		}

		// Yes - Add it to the archive.  It should automatically
		// gain the same ID as it had when serialised out
		myArchiveVector.push_back( object );

		// ... and then load in its data (this will call the derived class's
		// Load function, not the base class's)
		std::string magic;
		Read( magic );
		if ( magic != "OBST" )
			throw Exception( "CRA0007: object start marker mismatch" );
		object -> Read ( *this );
		Read( magic );
		if ( magic != "OBEN" )
			throw Exception( "CRA0008: object end marker mismatch" );
	}

}

void CreaturesArchive::WriteFloatRefTarget(const float &v)
{
	Write( v );
	if( myFloatMap.find( &v ) != myFloatMap.end() )
	{
#ifndef NON_LOCALISED_ERRORS
		std::string str = ErrorMessageHandler::Format("archive_error", 0, "CreaturesArchive::WriteFloatRef");
#else
		std::string str = "WriteFloatRefTarget failed";
#endif
		throw Exception( str.c_str() );
	}
	int siz = myFloatMap.size();
	myFloatMap[ &v ] = siz;
}

void CreaturesArchive::WriteFloatRef(const float *v)
{
	if( !v )
	{
		Write( int32( -1 ) );
		return;
	}
	std::map< ArchiveFloat const*, int32 >::const_iterator it =
		myFloatMap.find( v );
	if( it == myFloatMap.end() )
	{
#ifndef NON_LOCALISED_ERRORS
		std::string str = ErrorMessageHandler::Format("archive_error", 5, "CreaturesArchive::WriteFloatRef");
#else
		std::string str = "WriteFloatRef failed";
#endif
		throw Exception( str.c_str() );
	}
	Write( it->second );
}

void CreaturesArchive::ReadFloatRefTarget(float &v)
{
	Read( v );
	myFloatVector.push_back( &v );
}

void CreaturesArchive::ReadFloatRef(float *&v)
{
	int32 id;
	Read( id );
	if( id == -1 )
	{
		v = 0;
		return;
	}

	int siz = myFloatVector.size();
	if( id < 0 || id >= siz )
	{
#ifndef NON_LOCALISED_ERRORS
		std::string str = ErrorMessageHandler::Format("archive_error", 1, "CreaturesArchive::ReadFloatRef");
#else
		std::string str = "ReadFloatRef failed";
#endif
		throw Exception( str.c_str() );
	}
	v = myFloatVector[id];
}

// ---------------------------------------------------------------------
// Method:		GetCurrentVersion
// Arguments:	None
// Returns:		Version number of archive being read
// Description:	This can be used by objects being read, to read in old
//				object schemas.
// ---------------------------------------------------------------------
int32 CreaturesArchive::GetFileVersion()
{
	return myVersion;
}

void CreaturesArchive::SetAgentArchiveStyle(AgentArchiveStyle style)
{
	myAgentArchiveStyle = style;
}

// ---------------------------------------------------------------------
// Method:		GetCurrentVersion
// Arguments:	None
// Returns:		Version number of the archive that this engine writes out.
// Description:	This number should be incremented when ANY class changes
//				the format it is archived in.
// ---------------------------------------------------------------------
int32 CreaturesArchive::GetCurrentVersion()
{
	return 39;
}

void CreaturesArchive::SetZLibCompressionLevel(int level)
{
	ourZLibCompressionLevel = level;
}

void CreaturesArchive::ForceOpenRange(int& item, int lower, int upper)
{
	if (item < lower)
		item = lower;
	if (item >= upper)
		item = upper - 1;
}

void CreaturesArchive::ForceOpenRangeException(const int& item, int lower, int upper)
{
	if (item < lower || item >=upper)
		throw Exception( "CRA0010: range check failed" );
}

void CreaturesArchive::SetUpdateProgressBarFunction(UpdateProgressBarFunction function)
{
	myUpdateProgressBarFunction = function;
}


