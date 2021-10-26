#ifndef SIMPLEAGENT_H
#define SIMPLEAGENT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Agent.h"

class ClonedEntityImage;

class SimpleAgent : public Agent 
{
	typedef Agent base;

	CREATURES_DECLARE_SERIAL( SimpleAgent )

protected:

	int myNormalPlane;		// plot plane when not mouse-driven
	bool myEntityImageIsCloned;

#ifdef C2D_DIRECT_DISPLAY_LIB
	int	myWorldIndex;
	int myRedTint;
	int myGreenTint;
	int myBlueTint;
	int myRotation;
	int mySwap;
#endif

protected:

	virtual int ClickAction(int x, int y);

	// Public functions...

public:

	//--- constructors ---
	SimpleAgent();              	
	virtual ~SimpleAgent();			
	virtual void Trash();
	virtual uint32 GetMagicNumber();


	// Constructor can be called during subclass construction

	SimpleAgent(	int family, int genus, int species,uint32 id,
		FilePath const &gallery,	
				int numimages,
				int baseimage,
                int plane,          
				bool cloneMyImages);

	// virtual helper functions for macro commands
	virtual bool SetAnim(const uint8* anim, int length,int part);
	virtual bool SetFrameRate(const uint8 rate, int partid);

	virtual bool AnimOver(int part);				// helper fn for OVER macro
	virtual bool ShowPose(int pose,int part=0);	// helper fn for POSE macro
	virtual int GetPose(int part);				// helper function for POSE
	virtual bool SetBaseImage(int image,int part=0);	// default helper fn for BASE macro
	virtual int GetBaseImage(int part);
	virtual int GetAbsoluteBase(int part);


	virtual void ChangePhysicalPlane(int p);

	// displaying

	virtual void MoveTo(float x,float y);
	virtual int GetPlane(int part = 0);			// return *principal* plot plane
	virtual void SetNormalPlane(int p, int part = 0);
	virtual int GetNormalPlane(int part = 0);
	virtual void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon =0,
					uint8 stippleoff = 0,
					uint32 stippleStart =0);

	virtual void ResetLines();

	virtual void Update();


	virtual void Tint(const uint16* tintTable, int part = 0, bool oneImage=false);

	virtual void UnClone(int part = 0);

	virtual bool CreateClonedEntityImage(bool oneImage =  false);

#ifdef C2D_DIRECT_DISPLAY_LIB
	virtual void RememberTintInformation(int worldIndex, 
								int part = 0,
								int red =-1,
								int green =-1, 
								int blue =-1, 
								int rot=-1, 
								int swp=-1
							);

	virtual void RestoreTint(int part = 0 );

	virtual void DrawOutlineAroundImage(int red, int green, int blue);

#endif

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

protected:
	void Init();	// Basic initialisation used by constructors
};



#endif // SIMPLEAGENT_H

