// --------------------------------------------------------------------------
// Filename:	SharedGallery.cpp
// Class:		SharedGallery
// Purpose:		This class handles requests to create galleries.
//				
//
// Description: This holds a map of gallery names to gallery.
//				When a request for a new gallery comes in it creates
//				a memorymapped file.  Then the new gallery is created 
//				whose bitmaps point to the relevent parts of the 
//				memory mapped file.
//
//				If an existing gallery is requested then the pointer for that
//				gallery is passed back.
//
//
//				Note: at the moment there is no limit on how many files
//				you can have open.  There is no reference checking to tell
//				when to close galleries down			
//				
//
// History:
// 11Dec98	Alima		created
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Gallery.h"
#include "SharedGallery.h"
#include "CreatureGallery.h"
#include "DisplayEngine.h"		// for displaying errors
#include "../General.h"
#include "ErrorMessageHandler.h"
#include <algorithm>
#include "../../common/StringFuncs.h"
#include <fstream>
#include "DisplayErrorConstants.h"

#include "../../common/FileFuncs.h"
////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////
const std::string S16 = ".s16";
const std::string C16 = ".c16";

const int32 MINIMUM_BITMAP_WIDTH = 3;
const int32 MINIMUM_BITMAP_HEIGHT = 3;

////////////////////////////////////////////////////////////////////////////
// My static variables
////////////////////////////////////////////////////////////////////////////
SharedGallery SharedGallery::mySharedGallery;
std::map<std::string, Gallery*> SharedGallery::myGalleries;
std::map<Gallery*,std::string > SharedGallery::myClonedGalleries;
std::map<Gallery*,std::string > SharedGallery::myCreatureGalleries;


SharedGallery::SharedGallery()
{
}

// ----------------------------------------------------------------------
// Method:      theSharedGallery 
// Arguments:   None 		
//
// Returns:     reference to the shared gallery
//
// Description: This is the only way to use the shared gallery
//						
// ----------------------------------------------------------------------
SharedGallery& SharedGallery::theSharedGallery()
{
	return mySharedGallery;
}



SharedGallery::~SharedGallery()
{
}

// ----------------------------------------------------------------------
// Method:      CreateGallery
// Arguments:   galleryName - the name of the gallery file
//
// Returns:     pointer to the gallery created
// Description: checks whether the gallery already exists and if not
//				creates one and adds it to the list of galleries.  three
//				different types of gallery can be created depending on
//				the file extension.
//				background, compressed, and normal gallery.
//				
//				Now if you get an S16 extension check if there is a 
//				C16 version to use which will be preferred
// ----------------------------------------------------------------------
Gallery* SharedGallery::CreateGallery(std::string const &galleryIn)
{
	// see if we already have this gallery open
	std::string galleryName = galleryIn;
	std::string testString = galleryName;

	int x = testString.find_last_of(".");

	if(x == -1)
	{
		if(testString.empty())
		{
			ErrorMessageHandler::Throw(theDisplayErrorTag,
								(int)sidEmptyGalleryString,
								std::string("SharedGallery::DoCreateGallery"));
		}
		else
		{
			ErrorMessageHandler::Throw(theDisplayErrorTag,
									(int)sidNoGalleryFound,
									std::string("SharedGallery::DoCreateGallery"),
									testString.c_str());
		}

	
		return NULL;
	}

	std::string ext = testString.substr(x, 4);

	// check for a preferred C16 version
	if(ext[1] == 's')	
	{
		// not compressed sprite gallery

		std::string tempGalleryName = testString;
		tempGalleryName[x+1] = 'c';

		if(FileExists(tempGalleryName.c_str()))
		{
			testString = tempGalleryName;
			// get the revised extension

			ext = testString.substr(x, 4);
	//		galleryName.SetExtension( "c16" );
			galleryName = galleryName.substr( 0, x + 1 );
			galleryName += "c16";

		}
	}

	testString = galleryName;

	GALLERYMAP_ITERATOR it = myGalleries.find(testString);
	
	Gallery* gallery = NULL;

	if(it == myGalleries.end())
	{
		// nobody should be asking for a creature gallery that doesn't exist
		gallery = LookForCreatureGallery(testString);
		
		if(gallery)
			return gallery;
		

		gallery = DoCreateGallery(testString, 
											0,
											0,
											false);

		if(gallery && gallery->IsValid())
		{
			myGalleries[testString] = gallery;
			return gallery;
		}
		else
		{
			delete gallery;
			return NULL;
		}
	}
	else
	{
		// update this gallery's reference count
		((*it).second)->IncrementReferenceCount();
		return (*it).second;
	}
	return NULL;
}


Gallery* SharedGallery::LookForCreatureGallery(std::string const& galleryName)
{

	CREATUREGALLERYMAP_ITERATOR it;

	for(it = myCreatureGalleries.begin(); it!= myCreatureGalleries.end(); it++)
	{
		if((*it).second == galleryName)
		{
			break;
		}
	}

	if(it == myCreatureGalleries.end())
	{
		const char *c = galleryName.c_str();
		return NULL;
	}
	else
	{
		// update this gallery's reference count
		((*it).first)->IncrementReferenceCount();
		return (*it).first;
	}


return NULL;
}

// ----------------------------------------------------------------------
// Method:      CreateClonedGallery
// Arguments:   galleryName - the name of the gallery file
//
// Returns:     pointer to the gallery created
// Description: Creates a whole new cloned gallery
//				keep track of how many we have created????
//				
//			
// ----------------------------------------------------------------------
Gallery* SharedGallery::CreateClonedGallery(std::string const &galleryIn,
											uint32 baseImage,
											uint32 numImages)
{
	std::string testString = galleryIn;

	Gallery* gallery = DoCreateGallery(testString, 
											baseImage,
											numImages,
											true);//clone

	if(gallery->IsValid())
	{
		// the key this time is the gallery pointer
		myClonedGalleries[gallery] = galleryIn;
		return gallery;
	}
	else
	{
		delete gallery;
		return NULL;
	}
		

	return NULL;
}

Gallery* SharedGallery::DoCreateGallery(std::string const& galleryIn,
											uint32 baseImage, 
											uint32 numImages,
											bool clone)
{
	std::string galleryName = galleryIn;
	int x = galleryName.find_last_of(".");

	if(x == -1)
	{
		if(galleryName.empty())
		{
			ErrorMessageHandler::Throw(theDisplayErrorTag,
								(int)sidEmptyGalleryString,
								std::string("SharedGallery::DoCreateGallery"));
		}
		else
		{
			ErrorMessageHandler::Throw(theDisplayErrorTag,
									(int)sidNoGalleryFound,
									std::string("SharedGallery::DoCreateGallery"),
									galleryName.c_str());
		}

	
		return NULL;
	}

	std::string ext = galleryName.substr(x, 4);

	// check for a preferred C16 version
	if(ext[1] == 'S' || ext[1] == 's')	
	{
		// not compressed sprite gallery
		std::string tempGalleryName = galleryName;
		tempGalleryName[x+1] = 'c';
		if(FileExists(tempGalleryName.c_str()))
		{
			galleryName = tempGalleryName;
			// get the revised extension
			ext = galleryName.substr(x, 4);
		}
 	}

	if (!FileExists(galleryName.c_str()))
	{
		ErrorMessageHandler::Throw(theDisplayErrorTag,
								(int)sidNoGalleryFound,
								std::string("SharedGallery::DoCreateGallery"),
								galleryName.c_str());
		return NULL;
	}

	// assume it is an s16 format file
	std::string tmpFilePath = galleryName + ".tmp.s16";
	std::string prevFileName;

	std::transform( ext.begin(), ext.end(), ext.begin(), tolower );

		
	if(ext == ".blk")
	{	// not compressed background tile gallery
		tmpFilePath = galleryName + ".tmp.blk";
	}
	else if(ext == ".c16")
	{	//compressed sprite gallery
		tmpFilePath = galleryName + ".tmp.c16";
	}

	// convert the images to 555 or 565 format if necessary
	DisplayEngine::theRenderer().SafeImageConvert(galleryName,
					tmpFilePath,
					DisplayEngine::theRenderer().GetMyPixelFormat(), 
					prevFileName);
	::DeleteFile(tmpFilePath.c_str());

	Gallery* gallery =  new Gallery(galleryName,
								baseImage,
								numImages,
								clone);

	// TODO: this is just a warning for physics stuff
	// do you still use this???
/*	if(!gallery->ValidateBitmapSizes(MINIMUM_BITMAP_WIDTH, MINIMUM_BITMAP_HEIGHT))
	{
		ErrorMessageHandler::Throw(theDisplayErrorTag, 
			(int)sidBitmapSizeTooSmall,
		std::string("SharedGallery::CreateGallery"), 
		galleryName.GetFullPath().c_str(),MINIMUM_BITMAP_WIDTH,MINIMUM_BITMAP_HEIGHT);

	
	}*/

	return gallery;
}

void SharedGallery::AddClonedGallery( Gallery *gallery )
{
	myClonedGalleries[gallery] = "Anon";
}

// ----------------------------------------------------------------------
// Method:      RemoveGallery
// Arguments:   None
//
// Returns:     true if something was deleted
// Description: Goes through and destroys all galleries				
//			
// ----------------------------------------------------------------------
bool SharedGallery::RemoveGallery(Gallery* gallery)
{
	_ASSERT(gallery);

	if( RemoveClonedGallery( gallery ) ) return true;

	// see if we already have this gallery open

	std::string testString = gallery->GetName();
	GALLERYMAP_ITERATOR it = myGalleries.find(testString);

	bool found = ( it != myGalleries.end() );
	if( found )
	{
		((*it).second)->DecrementReferenceCount();
		if(!(((*it).second)->IsUsed()))
		{
			/*
#ifdef _DEBUG
			OutputFormattedDebugString( "deleting gallery \"%s\": count now %d\n",
				it->second->GetName().GetFullPath().c_str(), myGalleries.size() - 1);
#endif*/
			delete (*it).second;
			(*it).second = NULL;
			myGalleries.erase(it);
		}
	}
	else
	{

	}
	return found;
}


bool SharedGallery::RemoveClonedGallery(Gallery*& gallery)
{
//	if( RemoveCreatureGallery( gallery ) ) return true;

	// see if we already have this gallery open
	CLONEDGALLERYMAP_ITERATOR it = myClonedGalleries.find(gallery);

	bool found = ( it != myClonedGalleries.end() );
	if( found )
	{
		/*
#ifdef _DEBUG
			OutputFormattedDebugString( "deleting cloned gallery \"%s\": count now %d\n",
				gallery->GetName().GetFullPath().c_str(), myClonedGalleries.size() - 1);
#endif*/
		delete (*it).first;
		myClonedGalleries.erase(it);
	}
	return found;
}


bool SharedGallery::RemoveCreatureGallery(Gallery*& gallery)
{
	// no refernce counting just delete away
	// see if we already have this gallery open
	CLONEDGALLERYMAP_ITERATOR it = myCreatureGalleries.find(gallery);

	bool found = ( it != myCreatureGalleries.end() );
	if( found )
	{		

			delete (*it).first;
		
			myCreatureGalleries.erase(it);

	}
	return false;
}

// ----------------------------------------------------------------------
// Method:      DestroyGalleries
// Arguments:   None
//
// Returns:     None
// Description: Goes through and destroys all galleries				
//			
// ----------------------------------------------------------------------
void SharedGallery::DestroyGalleries()
{
	if(myGalleries.size())
	{
	GALLERYMAP_ITERATOR it;
	for(it = myGalleries.begin(); it != myGalleries.end(); it++)
	{
		Gallery* gallery = (*it).second;
		delete (*it).second;
	}

	myGalleries.clear();
	}

	if(myClonedGalleries.size())
	{

	CLONEDGALLERYMAP_ITERATOR it2;
	for(it2 = myClonedGalleries.begin(); it2 != myClonedGalleries.end(); it2++)
	{
		delete (*it2).first;
	}

	myClonedGalleries.clear();
	}

	if(myCreatureGalleries.size())
	{
	CREATUREGALLERYMAP_ITERATOR it3;
	for(it3 = myCreatureGalleries.begin(); it3!= myCreatureGalleries.end(); it3++)
	{
		Gallery* gallery = (*it3).first;
		delete gallery;
	}

	myCreatureGalleries.clear();
	}

	CleanCreatureGalleryFolder();
}

// ----------------------------------------------------------------------
// Method:		CreateGallery
// Arguments:	uniqueid - Used as a key for the 
//						   composite creature gallery file.
//				numSpritesInFile - yes
// Returns:		a pointer to the created gallery
// Description:	Interface to the creature gallery getting the correct 
//				body parts as defined in the genome.  
//				
// ----------------------------------------------------------------------
Gallery* SharedGallery::CreateGallery(std::string moniker,
									  uint32 uniqueID,
									  uint32 ValidParts,
									  uint8 Sex,
									  uint8 Age,
									  int32 BodySprites[NUMPARTS],
									  CreatureGallery* bodyPartGallery,
									  Gallery creatureParts[NUMPARTS],
									  int32 numSpritesInFile,
									   bool onePassOnly /*= false*/)
{


	if(!bodyPartGallery)
		return NULL;

	Gallery* gallery = NULL;

	if(onePassOnly)
	{
		// gallery is null and creature gallery is not yet complete
		while (gallery == NULL && !bodyPartGallery->IsComplete())
		{
			// try having compressed Norns now
			// pass the request to the composite file handler
			gallery =  bodyPartGallery->
									AddCompressedCreature(moniker,
								   uniqueID,
								   ValidParts,
								   Sex,
								   Age,
								   BodySprites,
								   creatureParts,
								   numSpritesInFile);
		}
	}
	else
	{
	// try having compressed Norns now
		// pass the request to the composite file handler
	gallery =  bodyPartGallery->
									AddCompressedCreature(moniker,
								   uniqueID,
								   ValidParts,
								   Sex,
								   Age,
								   BodySprites,
								   creatureParts,
								   numSpritesInFile);
	}

	if(gallery)
	{
		std::string newName = gallery->GetName();
		gallery->SetName(newName);


		if(gallery->IsValid())
		{
			gallery->SetFileSpec(uniqueID );
			// this gallery is new nobody should be using it yet
			gallery->ResetReferenceCount();

			ReplaceCreatureGallery(gallery);
			// the key this time is the gallery pointer
			return gallery;
		}
		else
		{

			std::string name("A creature gallery");
			ErrorMessageHandler::Throw(theDisplayErrorTag,
				(int)sidGalleryNotCreated,
				std::string("SharedGallery::CreateGallery"),
				name.c_str());

			delete gallery;
			return NULL;
		}
	}
	return NULL;
}

void SharedGallery::ConvertGallery(std::string const& galleryPath, uint32 to)
{
	std::string galleryName = galleryPath;

	// see whether we are dealing with a compressed file by looking
	// at the file extension
	int x = galleryName.find_last_of(".");

	if(x == -1)
	{
		ErrorMessageHandler::Throw(theDisplayErrorTag,
			(int)sidGalleryNotFound,
			std::string("SharedGallery::ConvertGallery"),
			galleryName.c_str());
		return;
	}
	
	std::string ext = galleryName.substr(x, 4);

	// true at the end for write access
	Gallery* gallery = new Gallery(galleryPath, 0, 0, false, 10, 10, true);

	if(gallery)
	{
	gallery->ConvertTo(to);

	delete gallery;
	gallery = NULL;
	}

}


void SharedGallery::ReplaceCreatureGallery(Gallery* gallery)
{
	ASSERT(gallery);

	CREATUREGALLERYMAP_ITERATOR it;

	for(it = myCreatureGalleries.begin(); it!= myCreatureGalleries.end(); it++)
	{
		std::string name = (*it).second;
		if(name == gallery->GetName())
		{
			break;
		}
	}

	if(it != myCreatureGalleries.end())
	{
	// found the gallery to replace lets delete it
		delete (*it).first;
		myCreatureGalleries.erase(it);
	

	}

	// add the new gallery whatever happened
	// the key this time is the gallery pointer
	myCreatureGalleries[gallery] = gallery->GetName();

}

void SharedGallery::CleanCreatureGalleryFolder()
{
	// before we do anything else delete all creature galleries
	// that are not in use

	if (myCreatureGalleriesFolder.empty())
		return;

	std::vector<std::string> files;
	GetFilesInDirectory(myCreatureGalleriesFolder, files);

	std::string fileName;
	// no files in this directory!!!
	int i;
	for ( i = 0; i < files.size(); i++)
	{
		fileName = myCreatureGalleriesFolder + files[i];
		// delete only files ending in .creaturegallery
		int len = fileName.size();
		if (len >= 16)
		{
			if (fileName.substr(len - 16, 16) == ".creaturegallery")
				DeleteFile(fileName.c_str());
		}
	}
}

// see if we already have this gallery open
bool SharedGallery::QueryGalleryInUse(std::string const &galleryPath)
{
	// NOTE: The shared gallery now no lnoger "knows" about
	// world directories, it can only deal with path names so
	// you must query the world directory version for yourself now
	return QueryGalleryInUseHelper(galleryPath);
}


bool SharedGallery::QueryGalleryInUseHelper(std::string galleryName)
{
	// remove .s16 or .c16 if there already
	int x = galleryName.find_last_of(".");
	if (x != -1)
		galleryName = galleryName.substr(0, x);

	GALLERYMAP_ITERATOR it;

	it = myGalleries.find(galleryName + ".c16");
	if (it != myGalleries.end())
		return true;

	it = myGalleries.find(galleryName + ".s16");
	if (it != myGalleries.end())
		return true;

	return false;
}



void SharedGallery::DumpGalleries()
{
	static int name = 0;
	char file[100];
	sprintf(file, "%s%d", "c:\\GalleryDump", name++);

	std::ofstream out(file);

	std::map< std::string, Gallery*>::iterator iter1;
	std::map<Gallery*,std::string>::iterator iter2;
	std::map<Gallery*,std::string>::iterator iter3;

	out << "myGalleries\n\n";
	for( iter1=myGalleries.begin(); iter1!=myGalleries.end(); ++iter1 )
	{
		out << iter1->first << " " << iter1->second->GetName() << "\n";
	}

	out << "\n\nmyClonedGalleries\n\n";
	for( iter2=myClonedGalleries.begin(); iter2!=myClonedGalleries.end(); ++iter2 )
	{
		out << iter2->second << " " << iter2->first->GetName() << "\n";
	}

	out << "\n\nmyCreatureGalleries\n\n";
	for( iter3=myCreatureGalleries.begin(); iter3!=myCreatureGalleries.end(); ++iter3 )
	{
		out << iter3->second << " " << iter3->first->GetName() << "\n";
	}
}

std::string SharedGallery::GetCreatureGalleriesFolder()
{
	return myCreatureGalleriesFolder;
}

