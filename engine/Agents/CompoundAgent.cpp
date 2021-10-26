// CompoundAgent.cpp
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompoundAgent.h"
#include "../World.h"
#include "UIPart.h"


#include "CompoundPart.h"


CREATURES_IMPLEMENT_SERIAL( CompoundAgent )

            ///////////////////////////////////////////////////
            // CompoundAgent class: objects consisting of   //
            // more than one, non-articulated entity.        //
            ///////////////////////////////////////////////////

void CompoundAgent::ChangeCameraShyStatus(bool shy)
{
	for(int i=0;i<myParts.size(); i++)
		if (myParts[i])
			myParts[i]->ChangeCameraShyStatus(shy);
}

bool CompoundAgent::AddPart( int id, CompoundPart* part )
{
	_ASSERT(!myGarbaged);

	ASSERT( part != NULL );

	// expand, if necessary
	while( myParts.size() <= id )
		myParts.push_back( (CompoundPart*)NULL );

	if (myParts[id])
	{
		// Hmm, we already have that part here :(
		return false;
	}

	myParts[id] = part;

	part->SetParent( this );
	// sort out position and plane relative to part0
	if( id != 0 )
	{
		part->SussPosition( myPositionVector );
		part->SussPlane( GetPlane(0) );
		part->ChangeCameraShyStatus( (GetAttributes() & attrCameraShy) == attrCameraShy );
	}
	else
	{
		part->GetEntity()->SetPlane( GetPlane(0) );
		myNormalBasePlaneForPartZero = GetPlane(0);
		part->ChangeCameraShyStatus( (GetAttributes() & attrCameraShy) == attrCameraShy );
	}
	return true;
}

void CompoundAgent::DestroyPart( int id )
{
	if (id >= 1 && id < myParts.size() && myParts[id])
	{
		delete myParts[id];
		myParts[id] = NULL;
	}

	// Shrink the parts collection down to size :)

	int top,toppity;
	for (top = 0; top < myParts.size(); top++)
	{
		if (myParts[top] != NULL)
			toppity = top;
	}

	if (toppity != myParts.size()-1)
		myParts.resize(toppity+1);
}

void CompoundAgent::Trash()
{
	_ASSERT(!myGarbaged);

	// Delete entities to ensure galleries are
	// destroyed
	int n = myParts.size();
	for (int i = 1; i < n; ++i)
	{
		if (myParts[i])
		{
			delete myParts[i];
			myParts[i] = NULL;
		}
	}

	base::Trash();

	// Now rid ourselves of our "actual entity" (ie part zero)
	// Sometimes it isn't there (file not found, for example)
	if (myParts.size() > 0)
	{
		delete myParts[0];
		myParts[0] = NULL;
	}
	
	myParts.clear();
}


CompoundPart* CompoundAgent::GetPart( int partid )
{
	_ASSERT(!myGarbaged);

	if (partid < 0 || partid >= myParts.size() )
		return NULL;

	return myParts[ partid ];
}

int CompoundAgent::GetIndex( CompoundPart* part )
{
	for(int i=0; i < myParts.size(); i++)
		if (myParts[i] == part)
			return i;
	return -1;
}


// Basic initialisation used by constructors
void CompoundAgent::Init()
{
	myNormalBasePlaneForPartZero = 0;
}


// Serialisation & subclass constr
CompoundAgent::CompoundAgent()
{
	if (!myFailedConstructionException.empty())
		return;
	Init();
	if (!myFailedConstructionException.empty())
		return;
	myAgentType = AgentHandle::agentNormal | AgentHandle::agentCompound;
}


CompoundAgent::CompoundAgent(	int family, int genus, int species, uint32 id,
							 FilePath const& gallery,
							 int numimages,
							   int baseimage,
							   int plane)
{
	try
	{
		Classifier c(family, genus, species);
		myClassifier = c;
		myID = id;
		Init();										// set defaults
		myAgentType = AgentHandle::agentNormal | AgentHandle::agentCompound;
		// create a default part 0
		Vector2D zero(0.0f,0.0f);
 		CompoundPart* part0 = new CompoundPart( gallery, baseimage, zero, plane,numimages );
		AddPart( 0, part0 );
		SetNormalPlane(plane,0);
		InitialisePickupPointsAndHandles();
			
		SetWidth(part0->GetEntity()->GetWidth());
		SetHeight(part0->GetEntity()->GetHeight());

	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0010: Unknown exception caught in compound agent constructor";
	}
}

// Destructor
CompoundAgent::~CompoundAgent()
{
#ifdef _DEBUG
	// Check all parts were removed by Trash above
	int n = myParts.size();
	for (int i = 0; i < n; ++i)
	{
		ASSERT(!myParts[i]);
	}
#endif
}

// adjust world position of all constituent entities to abs xy & redraw
// No validity checks made
void CompoundAgent::MoveTo(float x,float y)
{
    _ASSERT(!myGarbaged);

	base::MoveTo(x, y);
	Vector2D position(x, y);

	PartIterator it;
	for( it = myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
			(*it)->SussPosition( position );
    }
}


void CompoundAgent::OwnUpdate()
{
	_ASSERT(!myGarbaged);

	// Call update routine for each part
	PartIterator it;
	for( it=myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
			(*it)->Tick();
	}

	SetWidth((*myParts.begin())->GetEntity()->GetWidth());
	SetHeight((*myParts.begin())->GetEntity()->GetHeight());
}

void CompoundAgent::Update()
{
	_ASSERT(!myGarbaged);

	base::Update();
	OwnUpdate();
}

void CompoundAgent::HandleUI( Message *msg )
{
	_ASSERT(!myGarbaged);

	base::HandleUI(msg);

	PartIterator it;
	for( it=myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
		{
			if( (*it)->GetType() & CompoundPart::partUI ) 
				((UIPart*)(*it))->HandleUI( msg );
		}
	}
}




bool CompoundAgent::HitTest(const Vector2D& point,bool alwaysTestPixelPerfectly /* = false*/)
{
	_ASSERT(!myGarbaged);

	int px = Map::FastFloatToInteger(point.x);
	int py = Map::FastFloatToInteger(point.y); 

	PartIterator it;

	for( it=myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
		{
			if((*it)->GetPixelPerfectTesting() || alwaysTestPixelPerfectly)
			{
		
				if(!(*it)->GetEntity()->IsPixelTransparent(px, py))
					return true;
			}
			else
			{
				// we are not bothering to check for transparent pixels
				float top,left,bottom,right;
				top = myPositionVector.y + (*it)->GetRelY();
				left = myPositionVector.x + (*it)->GetRelX();
				bottom = top + (*it)->GetEntity()->GetHeight();
				right = left + (*it)->GetEntity()->GetWidth();
				if ((point.x >= left) &&
					(point.x <= right) &&
					(point.y >= top) &&
					(point.y <= bottom))
					return true;
			}
		}
	}
	
	return false;
}


void CompoundAgent::Hide()
{
	_ASSERT(!myGarbaged);

	PartIterator it;

	for( it=myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
			(*it)->Hide();
	}
}

void  CompoundAgent::Show()
{
	_ASSERT(!myGarbaged);

	PartIterator it;

	for( it=myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
			(*it)->Show();
	}
}

// if any part is on screen return true
bool CompoundAgent::Visibility(int scope)
{
	_ASSERT(!myGarbaged);

	PartIterator it;

	bool onScreen = false;
	for( it=myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it )
		{
			onScreen = (*it)->Visibility(scope);

			if(onScreen == true)
				return true;
		}
	}
	return false;
}


EntityImage* CompoundAgent::GetPrimaryEntity()
{
	_ASSERT(!myGarbaged);

	ASSERT( myParts.size() >= 1 );	// must have part 0
	return myParts[0]->GetEntity();
}

// return *principal* plot plane
int CompoundAgent::GetPlane(int part)
{
	_ASSERT(!myGarbaged);

	ASSERT( myParts.size() >= 1 );	// must have part 0

	if (!ValidatePart(part))
		return false;


	if (part == 0)
		return myParts[0]->GetEntity()->GetPlane();
	else
		return myParts[part]->GetEntity()->GetPlane() - myParts[0]->GetEntity()->GetPlane();
}

// Set principal plot plane and jiggle the parts around to fit
void CompoundAgent::SetPlane(int plane, int part)
{
	_ASSERT(!myGarbaged);

	ASSERT( myParts.size() >= 1 );	// must have part 0
	
	if ((part >= myParts.size()) || (myParts[part] == NULL))
		return;
	if (part == 0)
	{
		myParts[0]->GetEntity()->SetPlane(plane);

		PartIterator it;

		for( it = myParts.begin()+1; it != myParts.end(); ++it )
		{
			if( *it)
			{
				(*it)->SussPlane( plane );
			}
		}
	}
	else
	{
		(myParts[part])->GetEntity()->SetPlane(plane);
		(myParts[part])->SussPlane( myParts[0]->GetEntity()->GetPlane() );
	}
	

}

void CompoundAgent::ChangePhysicalPlane(int plane)
{
	_ASSERT(!myGarbaged);
	_ASSERT( myParts.size() >= 1 );

	myParts[0]->GetEntity()->SetPlane(plane);
	PartIterator it = myParts.begin();
	for( ++it; it != myParts.end(); ++it )
	{
		if( *it)
		{
			(*it)->SussPlane( plane );
		}
	}
}

void CompoundAgent::SetNormalPlane(int plane, int part)
{
	_ASSERT(!myGarbaged);
	if (!ValidatePart(part))
		return;
	SetPlane(plane, part);
	if (part == 0)
		myNormalBasePlaneForPartZero = plane;
}

// Scan list of active hotspots, then retn message# (ACTIVATE1 etc) 
// appropriate for a click at the given position, else return -1
int CompoundAgent::ClickAction(int x,int y)    // xy in world coords
{
	_ASSERT(!myGarbaged);
	DecideClickActionByState();
	if (myDefaultClickAction != -1)
	{
		// Think about this... If we are over an important UIPart, then we are also -1
		for(int i=0; i < myParts.size(); ++i)
		{
			if (!ValidatePart(i))
				continue;
			CompoundPart* part = myParts[i];
			if (part->GetType() & (CompoundPart::partEdit | CompoundPart::partButton))
			{
				// Hmm, are we inside it?
				float clickX = x;
				float clickY = y;
				float partX = myPositionVector.x + part->GetRelX();
				float partY = myPositionVector.y + part->GetRelY();
				// If up or left of part, don't get size from entity (quicker)
				if ((clickX < partX) || (clickY < partY))
					continue;
				// Right then, is it in the entity?
				partX += part->GetEntity()->GetWidth();
				partY += part->GetEntity()->GetHeight();
				// if down / right of part, it's not in the entity
				if ((clickX > partX) || (clickY > partY))
					continue;
				return -1; // This is inside the entity of a UI button or text edit field.
			}
		}
	}
	return myDefaultClickAction;
}



///////////////////// macro helper functions ///////////////////

// Helper function for ANIM macro - read a square bracketed string from ip[], 
// and use as animation string for entity
//char* CompoundAgent::SetAnim(char* ip,int part)
bool CompoundAgent::SetAnim(const uint8* anim, int length, int partid )
{
	_ASSERT(!myGarbaged);

	if (!ValidatePart(partid))
		return false;

	return (myParts[partid]->SetAnim( anim, length ));
}

bool CompoundAgent::SetFrameRate(const uint8 rate, int partid)
{
	_ASSERT(!myGarbaged);
	if (!ValidatePart(partid))
		return false;
	myParts[partid]->SetFrameRate(rate);
	return true;
}
// virtual helper function for OVER macro.
// return true if current PART's animation is over.
bool CompoundAgent::AnimOver( int partid )
{
	_ASSERT(!myGarbaged);
		
	if (!ValidatePart(partid))
		return true;

	return myParts[partid]->GetEntity()->AnimOver();
}

bool CompoundAgent::ValidatePart(int partid)
{
	_ASSERT(!myGarbaged);

	if ( partid < 0 || partid >= myParts.size() || myParts[ partid ] == NULL)
		return false;
	return true;
}

// virtual helper function for POSE macro.
// Stop any anim and set a new pose.
bool CompoundAgent::ShowPose( int pose, int partid )
{
	_ASSERT(!myGarbaged);

	if (!ValidatePart(partid))
		return false;

	if (!myParts[partid]->GetEntity()->ValidatePose( pose ))
		return false;

	myParts[partid]->GetEntity()->SetPose( pose );


	if(partid == 0)
	{
		SetWidth(myParts[partid]->GetEntity()->GetWidth());
		SetHeight(myParts[partid]->GetEntity()->GetHeight());
	}

	return true;
}

// virtual helper function for POSE
int CompoundAgent::GetPose( int partid )
{
	_ASSERT(!myGarbaged);

	if (!ValidatePart(partid))
		return -1;

	return myParts[partid]->GetEntity()->GetPose();
}


// Helper fn for BASE macro - set base sprite for given part
bool CompoundAgent::SetBaseImage( int image, int partid )
{
	_ASSERT(!myGarbaged);

	if (!ValidatePart(partid))
		return false;

	return myParts[partid]->GetEntity()->SetBaseAndCurrentIndex( image );
}

int CompoundAgent::GetBaseImage(int part)
{
	_ASSERT(!myGarbaged);

	if (!ValidatePart(part))
		return -1;

	return myParts[part]->GetEntity()->GetBaseAndCurrentIndex();
}

int CompoundAgent::GetAbsoluteBase(int part)
{
	_ASSERT(!myGarbaged);

	if (!ValidatePart(part))
		return -1;

	return myParts[part]->GetEntity()->GetAbsoluteBaseImage();
}

// virtual
void CompoundAgent::DrawLine( int32 x1,
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

	// Determine the most forward of the parts and use that...
	int bestplane = 0;
	int bestpart = 0;
	/*
	for(int i=1; i < myParts.size(); i++)
	{
		if (myParts[i] == NULL)
			continue;
		if (bestplane <= myParts[i]->GetEntity()->GetPlane())
		{
			bestplane = myParts[i]->GetEntity()->GetPlane();
			bestpart = i;
		}
	}
	*/
	myParts[bestpart]->DrawLine(x1,y1,x2,y2,
		lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stippleStart);
	myResetLines = true;
}


// virtual
void CompoundAgent::ResetLines()
{
	_ASSERT(!myGarbaged);
	myParts[0]->ResetLines();
	/*
	for(int i=0; i < myParts.size(); i++)
		if (myParts[i] != NULL)
			myParts[i]->ResetLines();
	*/
}

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool CompoundAgent::Write(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);

    // call base class function first
    base::Write( archive );

	archive << myParts;
	archive << myNormalBasePlaneForPartZero;

	return true;
}


// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool CompoundAgent::Read(CreaturesArchive &archive)
{
	_ASSERT(!myGarbaged);

	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{
		// call base class function first
		if(!base::Read( archive ))
			return false;

		archive >> myParts;
		archive >> myNormalBasePlaneForPartZero;

		_ASSERT(myParts.size()> 0);
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// Reset camera shyness
	DoSetCameraShyStatus();
#ifndef C2D_DIRECT_DISPLAY_LIB
	DrawMirrored(IsMirrored());
#endif
	return true;
}


bool CompoundAgent::SetPixelPerfectTesting(bool flag,int partNumber)
{
	_ASSERT(!myGarbaged);

	if(partNumber < myParts.size())
	{
		CompoundPart* cp  = myParts[partNumber];
		if(cp)
		{
			cp->SetPixelPerfectTesting(flag);
			return true;
		}
	}
	return false;
}

void CompoundAgent::SetGallery(FilePath const& galleryName, int baseimage, int part)
{
	_ASSERT(!myGarbaged);

	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp  = myParts[part];
		if(cp)
		{
			EntityImage* entityImage = cp->GetEntity();
			if(entityImage->GetGalleryName() != galleryName.GetFullPath())
			{
				entityImage->Unlink();
				entityImage->SetGallery(galleryName,baseimage);
				entityImage->Link();
			}
		}
		DoSetCameraShyStatus();
		UpdateFromPresence();
	}
}

void CompoundAgent::GetGallery(std::string& str, int part)
{
	if(part < myParts.size() && part >= 0)
	{
		EntityImage* entityImage = myParts[part]->GetEntity();

		if(entityImage)
		{
			str = entityImage->GetGalleryName();
		}
	}
}

void CompoundAgent::Tint(const uint16* tintTable, int part, bool oneImage)
{
	_ASSERT(!myGarbaged);

	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp = myParts[part];
		if (cp)
		{
			cp->Tint(tintTable, oneImage);
			DoSetCameraShyStatus();
		}
	}
}

void CompoundAgent::UnClone(int part/*=0*/)
{
	_ASSERT(!myGarbaged);

	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp = myParts[part];
		if (cp)
		{
			cp->UnClone();
			DoSetCameraShyStatus();
		}
	}

}

void CompoundAgent::DrawMirrored(bool mirror)
{
	_ASSERT(!myGarbaged);

	PartIterator it = myParts.begin();

	for( ; it != myParts.end(); ++it )
	{
		if( *it)
		{
			(*it)->DrawMirrored( mirror );
		}
	}
	myDrawMirroredFlag = mirror;
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void CompoundAgent::DrawFlippedVertically(bool flip)
{
	_ASSERT(!myGarbaged);

	PartIterator it = myParts.begin();

	for( it; it != myParts.end(); ++it )
	{
		if( *it)
		{
			(*it)->DrawMirrored(false);
			(*it)->DrawFlippedVertically( flip );
		}
	}
}


void CompoundAgent::CreateAnimStrip(int blockwidth, int blockheight, int part)
{
	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp = myParts[part];
		if (cp)
		{
			cp->CreateAnimStrip(blockwidth,  blockheight);

			if( part != 0 )
			{
				cp->SussPosition( myPositionVector );
				cp->SussPlane( GetPlane(0) );
				cp->ChangeCameraShyStatus( (GetAttributes() & attrCameraShy) == attrCameraShy );
			}
			else
			{
				cp->GetEntity()->SetPlane( GetPlane(0) );
				myNormalBasePlaneForPartZero = GetPlane(0);
				cp->ChangeCameraShyStatus( (GetAttributes() & attrCameraShy) == attrCameraShy );
				SetWidth(cp->GetEntity()->GetWidth());
				SetHeight(cp->GetEntity()->GetHeight());
			}
		}
	}

}

void CompoundAgent::SetSpriteRotationAngle(uint16 angle)
{

	_ASSERT(!myGarbaged);

	PartIterator it = myParts.begin();

	for( it; it != myParts.end(); ++it )
	{
		if( *it)
		{
		// must override any other effects
		(*it)->DrawMirrored(false);
		(*it)->DrawFlippedVertically(false);
		(*it)->SetSpriteRotationAngle(angle);
		}
	}
}

uint16 CompoundAgent::GetSpriteRotationAngle()
{
	_ASSERT(!myGarbaged);

	//all parts should have the same rotation angle
	PartIterator it = myParts.begin();
	if( *it)
	{
		return (*it)->GetSpriteRotationAngle();
	}

	return 0;
}


#endif

void CompoundAgent::Scale(float scaleby, bool on)
{	
	PartIterator it = myParts.begin();
	
	EntityImage* ent = NULL;

	for( it = myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it)
		{
			ent = (*it)->GetEntity();
			ent->Scale( scaleby , on);
		}
	}
}


void CompoundAgent::DrawAlphaBlended(int alphamask, bool onOrOff, int part )
{
	PartIterator it = myParts.begin();

#ifdef C2D_DIRECT_DISPLAY_LIB
	EntityImage* ent = NULL;

	if(part == -1)
	{
		for( it = myParts.begin(); it != myParts.end(); ++it )
		{
			if( *it)
			{
				ent = (*it)->GetEntity();
				ent->DrawAlphaBlended(alphamask,onOrOff);
			}
		}
	}
	else
	{
		if(part < myParts.size() && part >= 0)
		{
			CompoundPart* cp  = myParts[part];
			if(cp)
			{
				cp->DrawAlphaBlended(onOrOff, alphamask);
			}
		}
	}
#else
	if(part == -1)
	{
		for( it = myParts.begin(); it != myParts.end(); ++it )
		{
			if( *it)
			{
				(*it)->DrawAlphaBlended(onOrOff,alphamask);
			}
		}
	}
	else
	{
		if(part < myParts.size() && part >= 0)
		{
			CompoundPart* cp  = myParts[part];
			if(cp)
			{
				cp->DrawAlphaBlended(onOrOff, alphamask);
			}
		}
	}
#endif
}


void CompoundAgent::DrawShadowed(int shadowvalue, int x, int y, bool onOrOff)
{
	PartIterator it = myParts.begin();
	
	EntityImage* ent = NULL;

	for( it = myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it)
		{
			ent = (*it)->GetEntity();
			ent->DrawShadowed(shadowvalue, x, y, onOrOff);
		}
	}

}

void CompoundAgent::DrawToNewWidthAndHeight(int stretchedWidth, int stretchedHeight, bool onOrOff)
{
		
	PartIterator it = myParts.begin();
	
	EntityImage* ent = NULL;

	for( it = myParts.begin(); it != myParts.end(); ++it )
	{
		if( *it)
		{
			ent = (*it)->GetEntity();
			ent->DrawToNewWidthAndHeight(stretchedWidth, stretchedHeight, onOrOff);
		}
	}
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void CompoundAgent::RememberTintInformation(int worldIndex, 
											int part /*= 0*/,
								int red/* =-1*/,
								int green /*=-1*/, 
								int blue /*=-1*/, 
								int rot/*=-1*/, 
								int swp/*=-1*/
								)
{
	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp = myParts[part];
			cp->RememberTintInformation(worldIndex,red,
											green,blue,rot,swp);
		
	}
}

void CompoundAgent::RestoreTint(int part/* = 0 */)
{
	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp = myParts[part];
			cp->RestoreTint();	
	}
}

#endif

int CompoundAgent::NextPart(int previous)
{
	if (previous < 0 || previous >= myParts.size())
		return 0;

	for (int i = previous + 1; i < myParts.size(); ++i)
	{
		if (myParts[i] != NULL)
			return i;
	}

	return -1;
}


void CompoundAgent::MovePart(int part, Vector2D position)
{
	_ASSERT(!myGarbaged);
	if(part < myParts.size() && part >= 0)
	{
		CompoundPart* cp = myParts[part];
		if (cp)
		{
			cp->SetRelX(position.x);
			cp->SetRelY(position.y);
			cp->SussPosition( myPositionVector );
		}
	}
}

// Returns the part under the given coordinate.
// Just uses the rectangles and ignores transparency for now
// (it probably should do different things based on the type of the part,
// but that is excessively complex for the original application of it)
int CompoundAgent::GetPartOver( const Vector2D& absolutePosition )
{
	for (int i=myParts.size(); i >=0 ; --i)
	{
		if (!ValidatePart(i))
			continue;

		CompoundPart* part = myParts[i];
		
		// Hmm, are we inside it?
		float clickX = absolutePosition.x;
		float clickY = absolutePosition.y;
		float partX = myPositionVector.x + part->GetRelX();
		float partY = myPositionVector.y + part->GetRelY();
		// If up or left of part, don't get size from entity (quicker)
		if ((clickX < partX) || (clickY < partY))
			continue;
		// Right then, is it in the entity?
		partX += part->GetEntity()->GetWidth();
		partY += part->GetEntity()->GetHeight();
		// if down / right of part, it's not in the entity
		if ((clickX > partX) || (clickY > partY))
			continue;

		// This is inside the entity of a UI button or text edit field.
		return i; 
	}

	return -1;
}

