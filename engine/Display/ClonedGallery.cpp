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
//			THIS CLASS STILL EXISTS FOR BACKWARD COMPATIBILITY
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ClonedGallery.h"
#include "SharedGallery.h"
#include "System.h"
#include <stdio.h>
#include "ErrorMessageHandler.h"
#include "DisplayEngine.h"
#include	"Bitmap.h"
#include	"../File.h"

CREATURES_IMPLEMENT_SERIAL( ClonedGallery )

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				baseImage - the image in the sprite file
//							to start loading from
//				numImages - the number of images in the file that belong
//							to this entity (starting from the base image)
//				
//
// Description: Cloned Galleries keep their bitmap data locally
//						
// ----------------------------------------------------------------------

ClonedGallery::ClonedGallery(std::string const &name,
							 uint32 baseImage,
							 uint32 numImages)
:Gallery(name,baseImage,numImages)
{
}


ClonedGallery::~ClonedGallery()
{			
}

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool ClonedGallery::Write(CreaturesArchive &archive) const
{
	Gallery::Write( archive );

	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool ClonedGallery::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();

	if(version >= 13 )
	{
		if(!Gallery::Read( archive ))
			return false;
	}
	else if(version >= 3)
	{
		if(!Gallery::Read( archive ))
			return false;


		archive >> myBaseImage;
	}

	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}



