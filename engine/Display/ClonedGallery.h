// --------------------------------------------------------------------------
// Filename:	ClonedGallery.cpp
// Class:		ClonedGallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				Cloned galleries need to hold all their bitmap data locally
//				so they do not use memory mapped files.  Also we must be
//				provided with the bae image and the number of images that belong
//				to this particular gallery since there may be many sets of
//				related sprites in a sprite file.
//			
//				
//
// Description: A cloned gallery cannot be compressed because its bitmaps
//				are used as a canvas to display other sprites.
//
// History:
// -------  
// 11Nov98	Alima			Created
//
//
//			Note that this file is only here for backward compatiblity now
// --------------------------------------------------------------------------
#ifndef		CLONED_GALLERY_H
#define		CLONED_GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	<string>
#include	"Gallery.h"

class Bitmap;
class File;

// this will need to be a persistent
class ClonedGallery: public Gallery
{
	CREATURES_DECLARE_SERIAL( ClonedGallery )

public:

////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				background - create a background file
//
// Description: This memory maps the supplied file
//						
// ----------------------------------------------------------------------
	ClonedGallery(){;}

	ClonedGallery(std::string const &name,
					uint32 baseimage,
					uint32 numImages);


// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   none
//
// Description: Deletes the created bitmaps and unmaps and closes any
//				memory mapped files
//						
// ----------------------------------------------------------------------
	virtual ~ClonedGallery();


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);

private:

	// Copy constructor and assignment operator
	// Declared but not implemented
	ClonedGallery (const ClonedGallery&);
	ClonedGallery& operator= (const ClonedGallery&);

};

#endif		// CLONED_GALLERY_H

