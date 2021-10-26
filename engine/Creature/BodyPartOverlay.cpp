// --------------------------------------------------------------------------
// Filename:	BodyPartOverlay.cpp
// Class:		BodyPartOverlay
// Purpose:		This class allows overlays such as facial expressions
//				and footgear (ok shoes) to be placed on body parts.
//			
//				
//
// Description: There may be a set of overlays that we wish to use to replace
//				body parts.  We can have up to 10 overlays each with their
//				own set of attachments.  Where they share attachments the
//				the same coordinates will appear.
//
//				This will have to have links into
//			
//			
//
// History:
// ------- 
// 6Mar98	Alima		Created
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BodyPartOverlay.h"
#include "../Entity.h"
#include "../General.h"
#include "../DirectoryManager.h"

CREATURES_IMPLEMENT_SERIAL( BodyPartOverlay )


BodyPartOverlay::BodyPartOverlay()
:myBodyPart(0)
{

	Init();
}


BodyPartOverlay::BodyPartOverlay(BodyPart* bodyPart,
								int32 part,// part number
								int32 genus,// Genus (NORN, GRENDEL, ETTIN SIDE)
								int32 sex,// IDEAL Sex to look for (MALE/FEMALE)
								int32 age,// IDEAL age & variant to look for
								int32 variant)
								:myBodyPart(bodyPart)
								
{

	Init();
		
	ReloadOverlay(part,// part number
				genus,// Genus (NORN, GRENDEL, ETTIN SIDE)
				sex,// IDEAL Sex to look for (MALE/FEMALE)
				age,// IDEAL age & variant to look for
				variant);


}

void BodyPartOverlay::Init()
{
	// set all clothing sets to none
	for(int i=0; i< NUM_LAYERS;i++)
	{
		myClothingSetForEachLayer[i] = -1;
		myStoredClothingSetForEachLayer[i] = -1;
	}


}

void BodyPartOverlay::ReloadOverlay(	int32 part,// part number
								int32 genus,// Genus (NORN, GRENDEL, ETTIN SIDE)
								int32 sex,// IDEAL Sex to look for (MALE/FEMALE)
								int32 age,// IDEAL age & variant to look for
								int32 variant)
{
	// clothes have a prefix
	std::string galleryname;

	galleryname = BuildFsp(ValidFsp(part,
									genus,
									sex, // gender indifferent default to male
									age,
									variant, // variant is indifferent default to 'a'
									"c16",
									OVERLAYS_DIR, true),
									"c16",
									-1,
									true); // overlay

	myBodyPart->SetOverlayGallery(FilePath( galleryname, OVERLAYS_DIR ));
}

BodyPartOverlay::~BodyPartOverlay()
{

}


void BodyPartOverlay::SetBodyPartFileName(int part,
								   int genus,
								   int sex,
								   int age,
								   int variant)
{
	uint32 fsp = ValidFsp(part,
						genus,
						sex,
						age,
						variant,
						"c16",
						OVERLAYS_DIR);
	if(fsp)
	{
		// TODO:  Alima this will need to be a filepath
		// but for now this should not be used
		myBodyPartFileName = BuildFsp(fsp, "");

		int x = myBodyPartFileName.find_last_of(".");
		std::string ext = myBodyPartFileName.substr(0, x);

		myBodyPartFileName = ext;
	}
}


bool BodyPartOverlay::SetOverlay(int32 set,int32 direction,int32 layer)
{


	//no overlay is being used
	if(set ==-1)
	{
		myBodyPart->SetOverlayIndex(set,layer);

		if(layer == -1)
		{
			for(int32 i =0; i< NUM_LAYERS; i++)
			{
				myClothingSetForEachLayer[i] = set;
			}

			// clear all overkay bitmaps
			myBodyPart->SetOverlayIndex(layer,layer);
		}
		else
		{
			myClothingSetForEachLayer[layer] = set;
		}
		return false;
	}
	
	myClothingSetForEachLayer[layer] = set;

	int32 overlayindex =  set * MAXVIEWS;

	// now add on the approriate amount of sprites for the direction
	overlayindex+=direction;


	return myBodyPart->SetOverlayIndex(overlayindex,layer);

}


int BodyPartOverlay::GetOverlay(int32 layer /*=-1*/)
{
	// return the outermost layer of clothing if no layer
	// is specified

	if(layer == -1)
	{
		for(int32 i = NUM_LAYERS -1; i--;)
		{
			if(myClothingSetForEachLayer[i] != -1)
				return myClothingSetForEachLayer[i];
		}
	}
	else
	{
		if(layer < NUM_LAYERS)
		return myClothingSetForEachLayer[layer];
	}
	return -1;
}


bool BodyPartOverlay::UpdateOverlay(int32 direction)
{
	int32 overlayindex =0;
	int32 set =-1;
	int clothes = 0;
	for(int32 i =0; i< NUM_LAYERS; i++)
	{
		set = myClothingSetForEachLayer[i];
		if(set !=-1)
		{
			overlayindex =  set * MAXVIEWS;
			overlayindex+=direction;
			 if(myBodyPart->SetOverlayIndex(overlayindex,i))
				 clothes++;
		}
	}
	return clothes ? true : false;
}

void BodyPartOverlay::Hide()
{
myBodyPart->SetOverlayIndex(-1,-1);
}


	// new serialization stuff
bool BodyPartOverlay::Write( CreaturesArchive &ar ) const
{
	ar << myBodyPart;

	ar << myBodyPartFileName;

	for(int32 i =0; i< NUM_LAYERS; i++)
	{
	ar <<  myClothingSetForEachLayer[i];
	ar <<  myStoredClothingSetForEachLayer[i];
	}
	return true;
}

bool BodyPartOverlay::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		ar >> myBodyPart;

		ASSERT(myBodyPart);

		ar >> myBodyPartFileName;

		for(int32 i =0; i< NUM_LAYERS; i++)
		{
			ar >>  myClothingSetForEachLayer[i];
			if(version >= 25)
				ar >>  myStoredClothingSetForEachLayer[i];
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}


void BodyPartOverlay::StoreCurrentState()
{
	for(int i=0; i< NUM_LAYERS;i++)
	{
			myStoredClothingSetForEachLayer[i] = 
				myClothingSetForEachLayer[i];
	}
}

void BodyPartOverlay::ReturnToStoredState(int direction)
{
	for(int i=0; i< NUM_LAYERS;i++)
	{
			myClothingSetForEachLayer[i] = 
				myStoredClothingSetForEachLayer[i];
	}

	// for each layer set the overlay
	int set =-1;
	int overlayFileIndex = 0;

	for(int32 layer = 0; layer < NUM_LAYERS; ++layer)
	{
		set = myClothingSetForEachLayer[layer];
			
		int32 overlayindex = -1;
		
		if(set != -1)
		{
			overlayindex = set * MAXVIEWS;
			// now add on the appropriate amount of sprites for the direction
			overlayindex+=direction;
		}

		 myBodyPart->SetOverlayIndex(overlayindex,layer);
	}
}

