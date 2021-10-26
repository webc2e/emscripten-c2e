
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SimpleAgent.h"
#include "../World.h"
#include "../App.h"
#include "../AgentManager.h"

#include "../AgentDisplay/EntityImageClone.h"



CREATURES_IMPLEMENT_SERIAL( SimpleAgent)



// Basic initialisation used by constructors
void SimpleAgent::Init()
{
    myNormalPlane=0;
#ifdef C2D_DIRECT_DISPLAY_LIB
	myWorldIndex=-1;
	myRedTint=-1;
	myGreenTint=-1;
	myBlueTint=-1;
	myRotation=-1;
	mySwap=-1;
#endif
}


// Virtual
uint32 SimpleAgent::GetMagicNumber()
{
	return 3141592654U;
}


// Serialisation & subclass constr
SimpleAgent::SimpleAgent()
{
	Init();
	myAgentType = AgentHandle::agentNormal | AgentHandle::agentSimple;
}


// Construct a SimpleAgent (or base class part, for subclasses) by supplying
// all necessary information
// Note: last few params have defaults for dumb objects

// changing the constructor to take the gallery name
SimpleAgent::SimpleAgent(	int family, int genus, int species,uint32 id,
							FilePath const &gallery,		// Images us
							int numimages,
							int baseimage,
                            int plane,              // normal plot plane
							bool cloneMyImages)
{
	try
	{
		Classifier c(family, genus, species);
		myClassifier = c;
		myID = id;
		myAgentType = AgentHandle::agentNormal | AgentHandle::agentSimple;
		myEntityImageIsCloned = cloneMyImages;
		Init();						// defaults

		// create a unique gallery for this agent as
		// we will need to draw to it


		// TODO: Rethink this coordinate (myPositionVector) passing in now we
		// have myInvalidPosition in Agent.h?

		if(myEntityImageIsCloned)
		{
		myEntityImage =	new ClonedEntityImage(gallery,
				plane,
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				baseimage,
				numimages,
				baseimage );

		}
		else
		{
		myEntityImage =	new  EntityImage(gallery,
					numimages,
					plane,
					Map::FastFloatToInteger(myPositionVector.x),
					Map::FastFloatToInteger(myPositionVector.y),
					baseimage,
					0 );
		}

		myNormalPlane=plane;                              // remember changeable stuff
		InitialisePickupPointsAndHandles();
		SetWidth(myEntityImage->GetWidth());
		SetHeight(myEntityImage->GetHeight());
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0008: Unknown exception caught in simple agent constructor";
	}
}

// Destructor
SimpleAgent::~SimpleAgent()
{
}

// virtual
void SimpleAgent::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stippleStart /* =0*/) 
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
		myEntityImage->DrawLine(x1,y1,x2,y2,
		lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stippleStart);
}


// virtual
void SimpleAgent::ResetLines()
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
		myEntityImage->ResetLines();
}

// Update function for all SimpleAgents. Called every clock tick.
void SimpleAgent::Update()
{
	_ASSERT(!myGarbaged);

	base::Update();

	_ASSERT(!myGarbaged);


	myEntityImage->Animate();

	SetWidth(myEntityImage->GetWidth());
	SetHeight(myEntityImage->GetHeight());
}

// retn message# (ACTIVATE1 etc) appropriate for a click at a given
// position, else return -1
int SimpleAgent::ClickAction(int x,int y)
{
	_ASSERT(!myGarbaged);

	// some agents may want to set up a cycle of left clicks
	DecideClickActionByState();
	return myDefaultClickAction;
}


// return *principal* plot plane
int SimpleAgent::GetPlane(int part)
{
	_ASSERT(!myGarbaged);

	return(myEntityImage->GetPlane());
}





///////////////////// macro helper functions ///////////////////


// Helper function for ANIM macro - read a square bracketed string from ip[], 
// and use as animation string for entity
bool SimpleAgent::SetAnim(const uint8* anim, int length,int part)
{
	_ASSERT(!myGarbaged);

	if (!myEntityImage->ValidateAnim( anim, length ))
		return false;
	myEntityImage->SetAnim( anim, length );
	return true;
}

// virtual helper function for OVER macro.
// return TRUE if current animation is over.
bool SimpleAgent::AnimOver(int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->AnimOver();
}

// virtual helper function for POSE macro.
// Stop any anim and set a new pose.
bool SimpleAgent::ShowPose(int pose,int part)
{
	_ASSERT(!myGarbaged);

	if (!myEntityImage->ValidatePose( pose ))
		return false;
	myEntityImage->SetPose(pose);

	SetWidth(myEntityImage->GetWidth());
	SetHeight(myEntityImage->GetHeight());

	return true;
}

// virtual helper function for POSE
int SimpleAgent::GetPose(int part)
{
	_ASSERT(!myGarbaged);

	return myEntityImage->GetPose();
}


// Helper fn for BASE macro - set base sprite for obj
bool SimpleAgent::SetBaseImage(int image,int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->SetBaseAndCurrentIndex(image);
}

int SimpleAgent::GetBaseImage(int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->GetBaseAndCurrentIndex();
}

int SimpleAgent::GetAbsoluteBase(int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->GetAbsoluteBaseImage();
}


// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool SimpleAgent::Write(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);
    base::Write( archive );
	archive << myNormalPlane;

#ifdef C2D_DIRECT_DISPLAY_LIB
	archive << myWorldIndex;
	archive << myRedTint;
	archive << myGreenTint;
	archive << myBlueTint;
	archive << myRotation;
	archive << mySwap;
#endif
	
	archive <<	myEntityImageIsCloned;

	return true;
}

bool SimpleAgent::Read(CreaturesArchive &archive)
{
	_ASSERT(!myGarbaged);


	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{

		// call base class function first
		if(!base::Read( archive ))
			return false;

		archive >> myNormalPlane;

#ifdef C2D_DIRECT_DISPLAY_LIB
	archive >> myWorldIndex;
	archive >> myRedTint;
	archive >> myGreenTint;
	archive >> myBlueTint;
	archive >> myRotation;
	archive >> mySwap;

	if(myWorldIndex != -1)
	{
		TintManager& tint = theApp.GetWorld().GetTintManager(myWorldIndex);	
		Tint( tint.GetMyTintTable());
	}
	else if(myRedTint != 1 && myGreenTint != -1 
		&& myBlueTint !=-1 && myRotation !=-1 
		&& mySwap !=-1)
	{
		TintManager tintin;
		tintin.BuildTintTable(myRedTint,myGreenTint,myBlueTint,myRotation,mySwap);
		Tint( tintin.GetMyTintTable());
	}

#endif

		if(version > 31)
			archive >>	myEntityImageIsCloned;

	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}



void SimpleAgent::MoveTo(float x,float y)
{
	_ASSERT(!myGarbaged);
	base::MoveTo(x, y);
	if (myEntityImage)
		myEntityImage->SetPosition
			(Map::FastFloatToInteger(x), 
			 Map::FastFloatToInteger(y));
}

void SimpleAgent::SetNormalPlane(int p, int part)
{
	_ASSERT(!myGarbaged);
	myNormalPlane = p;
	ChangePhysicalPlane(p);
}

int SimpleAgent::GetNormalPlane(int part)
{
	_ASSERT(!myGarbaged);
	return myNormalPlane;
}


void SimpleAgent::Trash()
{
	_ASSERT(!myGarbaged);

	// This must be last line in the function
	base::Trash();
}


void SimpleAgent::Tint(const uint16* tintTable, int part, bool oneImage)
{
	_ASSERT(!myGarbaged);

	// we could delete everytime or do a check to 
	// see whether we have a cloned entity already and reload?
	if(myEntityImage)
	{
		bool wasVisible = myEntityImage->IsVisible();

		int storedPose =  myEntityImage->GetPose();

		if(CreateClonedEntityImage(oneImage))
		{

		myEntityImage->SetPose(oneImage ? 0 : storedPose);


		//  wants to cancel a previous tint if no tintable is given
		if(tintTable)
		{
			myEntityImage->RecolourGallery(tintTable);
		}

		if(wasVisible)
			myEntityImage->Link(true);	
		else
			myEntityImage->Unlink();


		DoSetCameraShyStatus();

		}
	}
}

bool SimpleAgent::CreateClonedEntityImage(bool oneImage/*= false*/)
{
		ClonedEntityImage* clone = NULL;

#ifdef C2D_DIRECT_DISPLAY_LIB
		// create a temporary cloned entity image
		int savedBlockWidth = myEntityImage->GetBlockWidth();
		int savedBlockHeight = myEntityImage->GetBlockHeight();

		clone =	new  ClonedEntityImage(myEntityImage->GetFilePath(),
				myEntityImage->GetPlane(),
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				savedBlockWidth,
				myEntityImage->GetGalleryBitmapCount(),
				savedBlockHeight );

		if(clone)
		{
			delete myEntityImage;
			myEntityImage = NULL; // note we delete again later in the common branch null is safe to delete
			clone->CreateAnimStrip(savedBlockWidth,savedBlockHeight);
		}

#else

		clone =	new  ClonedEntityImage(myEntityImage->GetFilePath(),
				myEntityImage->GetPlane(),
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				myEntityImage->GetAbsoluteBaseImage(),
				oneImage ? 1 : myEntityImage->GetGalleryBitmapCount(),
				myEntityImage->GetAbsoluteBaseImage() );

#endif

		if(clone != NULL)
		{
			delete myEntityImage;
			myEntityImage = clone;

			myEntityImageIsCloned = true;
			return true;
		}

		return false;
}

// restore the normal entity image from a memory sucking clone
void SimpleAgent::UnClone(int part/* = 0*/)
{

	if(myEntityImageIsCloned)
	{
#ifdef C2D_DIRECT_DISPLAY_LIB
		
		int savedBlockWidth = myEntityImage->GetBlockWidth();
		int savedBlockHeight = myEntityImage->GetBlockHeight();

#endif

		// create a normal entity image to be swapped in
		EntityImage* ent =	new  EntityImage(myEntityImage->GetFilePath(),
					myEntityImage->GetGalleryBitmapCount(),
					myEntityImage->GetPlane(),
					Map::FastFloatToInteger(myPositionVector.x),
					Map::FastFloatToInteger(myPositionVector.y),
					myEntityImage->GetAbsoluteBaseImage(),
					0 );

		delete myEntityImage;
		myEntityImage = NULL;

#ifdef C2D_DIRECT_DISPLAY_LIB
	
		ent->CreateAnimStrip(savedBlockWidth,savedBlockHeight);
#endif // C2D_DIRECT_DISPLAY_LIB

	
		myEntityImage = ent;


		myEntityImageIsCloned = false;

		DoSetCameraShyStatus();

	}
}

bool SimpleAgent::SetFrameRate(const uint8 rate, int partid) 
{
	_ASSERT(!myGarbaged);
	myEntityImage->SetFrameRate(rate); 
	return true; 
}

void SimpleAgent::ChangePhysicalPlane(int p)
{
	_ASSERT(!myGarbaged);
	myEntityImage->SetPlane(p);
	if (myCarriedAgent.IsValid())
		myCarriedAgent.GetAgentReference().
						ChangePhysicalPlane(GetPlaneForCarriedAgent());

}

#ifdef C2D_DIRECT_DISPLAY_LIB
void SimpleAgent::RememberTintInformation(int worldIndex, 
										  int part /*= 0*/,
								int red/* =-1*/,
								int green /*=-1*/, 
								int blue /*=-1*/, 
								int rot/*=-1*/, 
								int swp/*=-1*/
							)
{
	myWorldIndex = worldIndex;
	myRedTint = red;
	myGreenTint = green;
	myBlueTint = blue;
	myRotation = rot;
	mySwap =swp;
}

void SimpleAgent::RestoreTint(int part /*=0*/)
{
	if(myWorldIndex != -1)
	{
		TintManager& tint = theApp.GetWorld().GetTintManager(myWorldIndex);	
		Tint( tint.GetMyTintTable());
	}
	else if(myRedTint != 1 && myGreenTint != -1 
		&& myBlueTint !=-1 && myRotation !=-1 
		&& mySwap !=-1)
	{
		TintManager tintin;
		tintin.BuildTintTable(myRedTint,myGreenTint,myBlueTint,myRotation,mySwap);
		Tint( tintin.GetMyTintTable());
	}
}

void SimpleAgent::DrawOutlineAroundImage(int red, int green, int blue)
{

		if(CreateClonedEntityImage())
		{

			myEntityImage->DrawOutlineAroundImage(red,green,blue);
		}

}

#endif

