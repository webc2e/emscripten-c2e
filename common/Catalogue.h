// -------------------------------------------------------------------------
// Filename:    Catalogue.h
// Class:       Catalogue
// Purpose:
// Provide a flexible, simple and reliable stringtable.
//
// Description:
// The catalogue holds a list of localised strings, each of which can be
// called up using a unique (integer) id.
// The catalogue is loaded from catalogue files on disk. These files
// use a simple text format. The contents of the catalogue are compiled
// from any number of these files. Strings don't care which file they
// came from, or even which language - they are simply indexed by their
// id.
//
// If any of the catalogue functions fail, a Catalogue::Err exception
// is throw, containing an error message. Note that this message
// isn't localised (strangely enough).
//
// Usage:
//
//
// History:
// 19Feb99	Initial version		BenC
// -------------------------------------------------------------------------


#ifndef CATALOGUE_H
#define CATALOGUE_H



#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include <map>
#include <iostream>

#include "BasicException.h"

class Catalogue
{
public:
	Catalogue();

	// ------------------------------------------------------------------------
	// Method:		Get
	// Arguments:	basetag - name tag of id base
	//				offsetid - offset from basetag
	// Returns:		string
	// Description:
	// Looks up a string in the catalogue by tag.
	// A Catalogue::Err will be thrown if the string (or the tag) can't be
	// found.
	// ------------------------------------------------------------------------
	const char* Get( const std::string& basetag, int offsetid=0 ) const;

	// ------------------------------------------------------------------------
	// Arguments:	nametag - name of tag to look for
	// Returns:		true if the tag exists, false otherwise
	// ------------------------------------------------------------------------
	bool TagPresent( const std::string& nametag ) const;


	// ------------------------------------------------------------------------
	// Method:		GetArrayCountForTag
	// Arguments:	nametag - name of tag to look for
	// Returns:		no of items in the array or -1 if undefined
	// Description:
	// Finds the array count associated with a tag.
	// Tags are defined using the TAG keyword in catalogue files.
	// A Catalogue::Err will be thrown if the tag can't be found.
	// ------------------------------------------------------------------------
	int GetArrayCountForTag( const std::string& nametag ) const;

	
	// ------------------------------------------------------------------------
	// Method:		Clear
	// Arguments:	None
	// Returns:		None
	// Description:
	// Clears the catalogue - all strings and tags will be discarded.
	// ------------------------------------------------------------------------
	void Clear();


	// ------------------------------------------------------------------------
	// Method:		AddFile
	// Arguments:	file - base name of file (will be localised)
	//				langid - preferred language
	// Returns:		None
	// Description:
	// Add a catalogue file to the catalogue. The filename given will be
	// localised to the best matching file that can be found for the
	// preferred language.
	// A Catalogue::Err will be thrown if any error occurs (eg strings
	// with conflicting IDs)
	// ------------------------------------------------------------------------
	void AddFile( const std::string& file, const std::string& langid );


	// ------------------------------------------------------------------------
	// Method:		AddDir
	// Arguments:	dir - path of directory to scan
	//				langid - preferred language
	// Returns:		None
	// Description:
	// Scans the specified directory and adds to the catalogue the best
	// matching files for the preferred language. 
	// A Catalogue::Err will be thrown if any error occurs (eg strings
	// with conflicting IDs)
	// ------------------------------------------------------------------------
	void AddDir( const std::string& dir, const std::string& langid );

	
	// Makes sure that if the same tag is in both the catalogues, then they have
	// exactly the same entries
	void CheckForClashes(const Catalogue& from) const;

	// Merges two catalogues together.  Checkes there are no clashes first.
	void Merge(const Catalogue& from);

	// ------------------------------------------------------------------------
	// Class: Catalogue::Err
	// Description:
	// The exception class used by Catalogue. Derived from BasicException (so
	// the error message can be retreived using the what() member function).
	// ------------------------------------------------------------------------
	class Err : public BasicException
	{
	public:
		Err( const char* fmt, ... );
	};

	// diagnostics:
	void DumpStrings( std::ostream& out );
	void DumpTags( std::ostream& out );

private:
	const char* Get( int id ) const;
	int FindTag( const std::string& nametag ) const;
	bool GetOverride( const std::string& nametag ) const;

	int myNextFreeId;

	struct TagStruct {
		int id;
		int noOfItems;		// -1 means unspecified
		bool override;
	};

	std::map< int, std::string > myStrings;
	std::map< std::string, struct TagStruct > myTags;

	void AddLocalisedFile( const std::string& file );
};

inline const char* Catalogue::Get( int id ) const
{
	std::map< int, std::string >::const_iterator it;

	it = myStrings.find( id );
	if( it == myStrings.end() )
		throw Err( "CLE0017: Couldn't find string <%d>", id );
	return (*it).second.c_str();
}

inline const char* Catalogue::Get( const std::string& basetag, int offsetid ) const
{
	int baseIndex = FindTag( basetag );
	int noItems = myTags.find(basetag)->second.noOfItems;
	if (noItems == -1)
	{
		throw Err( "CLE0013: Internal catalogue error, unexpected -1");
	}

	if (offsetid < 0 || offsetid >= noItems)
	{
		throw Err( "CLE0012: Index %d out of range 0 to %d for tag %s",
			offsetid, noItems - 1, basetag.c_str());
	}

	return Get(baseIndex + offsetid );
}

inline int Catalogue::GetArrayCountForTag( const std::string& nametag ) const
{
	std::map< std::string, struct TagStruct >::const_iterator it;

	it = myTags.find( nametag );
	if( it == myTags.end() )
		throw Err( "CLE0015: Couldn't find Tag \"%s\"", nametag.c_str() );

	return (*it).second.noOfItems;
}


inline int Catalogue::FindTag( const std::string& nametag ) const
{
	std::map< std::string, struct TagStruct >::const_iterator it;

	it = myTags.find( nametag );
	if( it == myTags.end() )
		throw Err( "CLE0016: Couldn't find Tag \"%s\"", nametag.c_str() );

	return (*it).second.id;
}

inline bool Catalogue::TagPresent( const std::string& nametag ) const
{
	std::map< std::string, struct TagStruct >::const_iterator it;

	it = myTags.find( nametag );

	return it != myTags.end();
}

inline bool Catalogue::GetOverride( const std::string& nametag ) const
{
	std::map< std::string, struct TagStruct >::const_iterator it;

	it = myTags.find( nametag );
	if (it == myTags.end())
		return false;
	return it->second.override;	
}

#endif // CATALOGUE_H


