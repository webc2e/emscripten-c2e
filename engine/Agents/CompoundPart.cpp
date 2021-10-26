// -------------------------------------------------------------------------
// Filename:    CompoundPart.cpp
// Class:       CompoundPart
// Purpose:     Base class for compound parts, which form CompoundAgents
// Description:
//
// Usage:
//
//
// History:
// 11Feb99	BenC	Initial version
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompoundPart.h"
#include "CompoundAgent.h"
#include "../AgentDisplay/EntityImageClone.h"
#include "../Display/TintManager.h"
#include "../App.h"
#include "../World.h"


CREATURES_IMPLEMENT_SERIAL( CompoundPart )

CompoundPart::CompoundPart()
:myRelativePosition(0.0f,0.0f),
myRelPlane(0),
myEntity(NULL),
myType(partPlain),
myPixelPerfectHitTestFlag(true)
{
}


CompoundPart::CompoundPart( FilePath const& gallery, int baseimage, Vector2D& relPos, int relplane, int numImages  )
:myType(partPlain),
myPixelPerfectHitTestFlag(true)
{
	myRelativePosition = relPos;
	myRelPlane = relplane;
#ifdef C2D_DIRECT_DISPLAY_LIB
	myWorldIndex=-1;
	myRedTint=-1;
	myGreenTint=-1;
	myBlueTint=-1;
	myRotation=-1;
	mySwap=-1;
#endif
	myEntity = new EntityImage( gallery, numImages, myRelPlane, -7654, -7654, baseimage, baseimage );
	myEntityImageIsCloned = false;
}


CompoundPart::~CompoundPart()
{
	if (myEntity)
	{
		delete myEntity;
		myEntity = NULL;
	}
}

// virtual
void CompoundPart::Tick()
{
	myEntity->Animate();
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool CompoundPart::Write(CreaturesArchive& archive) const
{
	archive << myRelativePosition << myRelPlane;

	archive << myEntity;
	archive << myParent;
	archive << myPixelPerfectHitTestFlag;

	archive <<	myEntityImageIsCloned;

#ifdef C2D_DIRECT_DISPLAY_LIB
	archive << myWorldIndex;
	archive << myRedTint;
	archive << myGreenTint;
	archive << myBlueTint;
	archive << myRotation;
	archive << mySwap;
#endif

	return true;
}

bool CompoundPart::Read(CreaturesArchive& archive)
{
	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{
		archive >> myRelativePosition >> myRelPlane;

		archive >> myEntity;
		archive >> myParent;
		archive >> myPixelPerfectHitTestFlag;

		if(version > 31)
			archive >>	myEntityImageIsCloned;

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
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

bool CompoundPart::SetAnim(const uint8* anim, int length)
{
	if (!myEntity->ValidateAnim( anim, length ))
		return false;

	myEntity->SetAnim( anim, length );
	return true;
}

bool CompoundPart::SetFrameRate(const uint8 rate)
{
	myEntity->SetFrameRate(rate);
	return true;
}
// virtual
void CompoundPart::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stippleStart /*=0*/) 
{
	if(myEntity)
		myEntity->DrawLine(x1,y1,x2,y2,
		lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stippleStart);
}


// virtual
void CompoundPart::ResetLines()
{
	if(myEntity)
		myEntity->ResetLines();
}

void CompoundPart::Show()
{
	if(myEntity)
		myEntity->Link();
}

void CompoundPart::Hide()
{
	if(myEntity)
		myEntity->Unlink();
}


bool CompoundPart::Visibility(int scope)
{
	if(myEntity)
	{
		return myEntity->Visible(scope);
	}

	return false;
}

void CompoundPart::ChangeCameraShyStatus(bool shy)
{
	if(myEntity)
		myEntity->YouAreCameraShy(shy);
}

void CompoundPart::Tint(const uint16* tintTable, bool oneImage)
{
	EntityImage* entityImage = GetEntity();

	ClonedEntityImage*	clone = NULL;

#ifdef C2D_DIRECT_DISPLAY_LIB
	clone =	new  ClonedEntityImage(entityImage->GetFilePath(),
			entityImage->GetPlane(),
			entityImage->GetPosition().GetX(),
			entityImage->GetPosition().GetY(),
			entityImage->GetBlockWidth(),
			entityImage->GetGalleryBitmapCount(),
			entityImage->GetBlockHeight() );

	clone->CreateAnimStrip(entityImage->GetBlockWidth(), entityImage->GetBlockHeight());
#else
	clone =	new ClonedEntityImage(entityImage->GetFilePath(),
			entityImage->GetPlane(),
			entityImage->GetPosition().GetX(),
			entityImage->GetPosition().GetY(),
			entityImage->GetAbsoluteBaseImage(),
			oneImage ? 1 : entityImage->GetGalleryBitmapCount(),
			entityImage->GetAbsoluteBaseImage() );
#endif

	if(clone)
	{
		bool wasVisible = entityImage->IsVisible();

		clone->SetPose(oneImage ? 0 : entityImage->GetPose());

		delete entityImage;
		SetEntity(clone);

		// if no tinttable given then we want to cancel a previous tint

		if(tintTable)
			clone->RecolourGallery(tintTable);

		if(wasVisible)
			clone->Link(true);	
		else
			clone->Unlink();

		myEntityImageIsCloned = true;
	}
}

// restore normal entity image and remove memory sucking clone
void CompoundPart::UnClone()
{
	if(myEntityImageIsCloned)
	{

		EntityImage*	clone = GetEntity();


		EntityImage* entityImage =  new EntityImage( clone->GetFilePath(), 
										clone->GetGalleryBitmapCount(),
										clone->GetPlane(),
										clone->GetPosition().GetX(),
										clone->GetPosition().GetY(),
										clone->GetAbsoluteBaseImage(),
										clone->GetAbsoluteBaseImage() );


#ifdef C2D_DIRECT_DISPLAY_LIB
		entityImage->CreateAnimStrip(clone->GetBlockWidth(), clone->GetBlockHeight());
		entityImage->SetPose(clone->GetPose());
#endif// C2D_DIRECT_DISPLAY_LIB

		delete (clone);
		SetEntity(entityImage);

		myEntityImageIsCloned = false;
	}
}

void CompoundPart::DrawMirrored(bool mirror)
{
	if(myEntity)
		myEntity->DrawMirrored(mirror);
}


void CompoundPart::DrawAlphaBlended(bool drawAlpha,int intensity)
{
	if(myEntity)
	{
		myEntity->DrawAlphaBlended(intensity,drawAlpha);
	}
}

#ifdef C2D_DIRECT_DISPLAY_LIB
void CompoundPart::DrawFlippedVertically(bool flip)
{
	if(myEntity)
	{
		myEntity->DrawMirrored(false);
		myEntity->DrawFlippedVertically(flip);
	}
}

// I put this in before I realised that Keith hadn't implemented it yet
uint16 CompoundPart::GetSpriteRotationAngle()
{
	if(myEntity)
	{
		return myEntity->GetSpriteRotationAngle();
	}

	return 0;
}

void CompoundPart::SetSpriteRotationAngle(uint16 angle)
{
	if(myEntity)
	{
		// must override any other effects
		DrawMirrored(false);
		DrawFlippedVertically(false);
		myEntity->SetSpriteRotationAngle(angle);
	}
}

void CompoundPart::CreateAnimStrip(int blockwidth, int blockheight)
{
	if(myEntity)
	{
		myEntity->CreateAnimStrip(blockwidth,blockheight);
	}
}

void CompoundPart::RememberTintInformation(int worldIndex, 
								int red/* =-1*/,
								int green /*=-1*/, 
								int blue /*=-1*/, 
								int rot/*=-1*/, 
								int swp/*=-1*/)
{
	myWorldIndex = worldIndex;
	myRedTint = red;
	myGreenTint = green;
	myBlueTint = blue;
	myRotation = rot;
	mySwap =swp;
}

void CompoundPart::RestoreTint()
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
#endif

