#ifndef SkeletalCreature_H
#define SkeletalCreature_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Creature.h"
#include "Definitions.h"
#include "Genome.h"
#include "../Entity.h"
#include "../AgentManager.h"
#include "../General.h"
#include "../mfchack.h"

class Message;

const int MAX_GAITS = 16;		// number of possible walking gaits (including default gait)
const int MAX_SKELETON_ANIMATION_STRING_LENGTH = ((30 * 3) + 2);  // 30 Poses + 'R' + '\0';
const int MAX_POSE_STRING_LENGTH = 15;					// # bytes in a Pose string
const int POSE_TOUCH=0;
const int MAX_POSES=256;

class TintManager;
class CreatureGallery;




class SkeletalCreature : public Creature
{
	CREATURES_DECLARE_SERIAL( SkeletalCreature )
	typedef Creature base;


public:
	// to override CompoundAgent back to Agent:
	bool SetBaseImage( int image, int partid );
	virtual bool ValidatePart(int partid);


public:
	// MACRO functions:
	virtual bool GetCreatureRoomID(int &roomID);
	virtual Vector2D GetMouthMapPosition();
	virtual void Walk();
	virtual bool CanReachIt();
	virtual int ReachOut(BOOL Point=FALSE);
	virtual bool CannotBeMuchCloser();
	virtual void MoveFootTo(float x,float y);
	virtual bool TestMoveFootTo(float x, float y, bool forcePositionCheck);
	virtual void SetDirection(int d) {myCurrentDirection=d;}
	virtual int GetDirection() const {return myCurrentDirection;}
	virtual void ChangeHairStateInSomeWay(int action);
	virtual int GetOverlayIndex(int region);
	virtual int GetClothingSet(int bodypart, int layer);
	virtual bool WearOutfit(int set,int layer);
	virtual bool WearItem(int bodypart, int set, int layer);
	virtual bool HasTargetPoseStringBeenReached();
	virtual void SetGait(int gait);
	virtual void StopHoldingHandsWithThePointer(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2);
	virtual bool AreYouHoldingHandsWithThePointer() {return myIsHoldingHandsWithThePointer;}
	virtual Vector2D GetDownFootPosition();
	virtual Vector2D GetUpFootPosition();


public:
	// Creature functions (needed by faculties):
	virtual bool PrepareForAgeing(int age);
	virtual void SetEyeState(int i) {myEyeState = i;}
	virtual void ResetAnimationString();
	virtual inline Vector2D GetNosePosition() {return GetDownFootPosition();}
	void RemakeSkeletonAfterSerialisation();
	void RemakeSkeletonAfterImporting();
	virtual void GetBodyPartFileName(int index, std::string& path);
	virtual void SetFacialExpression(int headset, int earset= -1){myFacialExpression = headset;}
	virtual int GetFacialExpression() {return myFacialExpression;}
	virtual float GetSensoryDistanceFromAgent(AgentHandle a);


public:
	SkeletalCreature();							// Serialisation constr
	virtual ~SkeletalCreature();				// destruct an obj & remove it from objlib
	SkeletalCreature(int family, uint32 id,
				AgentHandle gene, int gene_index,			// your genome
				int   sex,				// your sex 0=randomly choose 1=male 2=female
				int variant);		// only the agent manager can create SkeletalCreatures (except serialization)

	virtual float* GetLocusAddress(int type, int tissue, int organ, int locus);
	virtual bool ReadFromGenome();

	bool FormBodyParts();
	virtual bool Approach(bool doApproachNotRetreat=true);











////////////////////////////////////////////////////////////////////////////////
// AGENT:
public:
	virtual void Update();
   	virtual void Trash();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	virtual int ClickAction(int x, int y);
 	virtual Vector2D WhereShouldCreaturesPressMe();

	virtual bool CanSee(const AgentHandle& other);

	void GetTintValues(int16& redTint, int16& greenTint, int16& blueTint, int16& rotation, int16& swap);

protected:
	// New style plot order array.
	EntityImage* myPlotOrders[4][4][NUMPARTS];

	bool myDoubleSpeedFlag;
	bool myJustAgedFlag;
	CreatureGallery*	myCompositeBodyPartGallery;
	CreatureGallery*	myCompositeBodyPartGalleryForNextAgeStage;
	int32				myNumSpritesInFile;
	int32				myNumberOfSpritesInFileforAgeing;

	int32				myVariants[NUMPARTS];	// variant # to use for each part
    int32				myGenera[NUMPARTS];
	// which sprite# in final sprite file refers to the first pose of each body part
	// Filled in while copying sprites from library files. Used to construct Body & Limbs
	int32				myBodySprites[NUMPARTS];
	Body*				myBody;						// ptr to Body Ent
	Limb*				myLimbs[MAX_BODY_LIMBS];		// limbs (or NULL if none)
	uint32				myBodyPartToBeCreated;
	uint32				myBodyPartToBeCreatedForNextAgeStage;
	bool				myBodyPartsAreFullyFormedFlag;
	int					myBodyPartsAgeStage;
	int					myLastAge;
	int					myPreloadLastAge;
	bool				myAgeingDone;
	int					myAgeAlreadyLoaded;
	TintManager*		myTintin;

	bool myIsHoldingHandsWithThePointer;


	bool myEntitiesAreCameraShy;

	uint8	myCurrentDirection;					// which dirn facing (ESWN)
	byte	myCurrentDownFoot;					// which foot is 'down' 0/1 LEFT/RIGHT
	Vector2D	myDownFootPosition;
	Vector2D	myUpFootPosition;
	int32   myNormalPlane;				// plot plane when not in a vehicle
	Vector2D  myPreviousUpFootPosition;
	float myPreviousMinY;
	float myMinX;
	float myMaxX;
	float myMinY;
	float myMaxY;
	Vector2D myExtremes[MAX_BODY_LIMBS];

	char	myCurrentPoseString[MAX_POSE_STRING_LENGTH+1];		// Pose$ representing current pose, used by UpdatePose()
	char	myTargetPoseString[MAX_POSE_STRING_LENGTH+1];	// Pose that UpdatePose() is currently striving towards

	char	myAnimationString[MAX_SKELETON_ANIMATION_STRING_LENGTH];		// current animation string
	char*	myAnimationPointer;	
	
	bool myStandStill;



	byte	myFacialExpression;			// 012: normal/happy/sad face
	uint8	myEarSet;
	int		myEyeState;					// 01: eyes closed/open
	bool	myBodyIsSuitedUp;


   	char	myPoseStringTable[MAX_POSES][MAX_POSE_STRING_LENGTH+1];// array of poses fr actions & reaching out
	char	myGaitTable[MAX_GAITS]			// array of walking animations ("010203R"), one for
					   [MAX_SKELETON_ANIMATION_STRING_LENGTH];	// each gait receptor (room fr 8 poses + 'R',\0)



	// Chemo-receptor and emitter loci that are controlled/monitored by skeleton
	// rather than creature
	float myMusclesLocus;				// emitter: amount of energy expended by movement this tick
    float myUpslopeLocus;				// emitter: how steep is the slope I'm facing
    float myDownslopeLocus;			// emitter: how steep is the slope I'm facing
	float myGaitLoci[MAX_GAITS];		// trigger different walking gaits ([0]=default)


	Vector2D myReach;				// Reach (inited from catalogue)

	int	myHowToTouchIt;				// Msg you want to send to IT
									// (used to determine hotspot for Touch)

protected:
	float GetReachX();
	float GetReachY();

	//
	// Movement
	//
	virtual void HandleMovementWhenAutonomous();
	//virtual void HandleMovementWhenFloating();
	virtual void HandleMovementWhenInVehicle();

	void EmergencySafePositionRoom();

	void GeneratePlotOrder();

	void UpdatePositionsWithRespectToDownFoot();
	void CommitPositions();

	void UpdatePlotOrder();			// Set the display plane for each
									// of this body's limbs,
									// for so that all limbs are plotted
									// in the correct order
									// after a change of direction

	void UpdateFeet();				// swap 'down' foot if necessary

	char HeadAngle();				// used by SetPose() to look up/down
	bool HasBody();
		




	bool CreateBodyPart(int sex, int age,TintManager& tinty);
	bool PreloadBodyPartsForNextAgeStage(Genome& genome, int sex);

	bool ContinueLoadingDataForNextAgeStage();

	bool BuildGalleries(int sex, int age,uint32& currentPart, TintManager& tinty);



public:
	// Holding hands
	virtual void HandleStartHoldHands(Message* Msg);
	virtual void HandleStopHoldHands(Message* Msg);
	void StartHoldingHandsWithThePointer
		(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2);
	void HoldHandsWithThePointer();

	// end new functions

	
	void GetExtremesForPose
	(char* pose, Vector2D extremes[MAX_BODY_LIMBS]);

	virtual void ChangePhysicalPlane(int plane);
	virtual void ChangeCameraShyStatus(bool shy);

	virtual int GetNormalPlane(int part = 0)
	{
		return myNormalPlane;
	}

	bool ExpressParticularGenes(Genome& g, int age);// Express any genes relevant to current age
	
	virtual void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon =0,
					uint8 stippleoff = 0,
					uint32 stipplestartAt =0);

	virtual void ResetLines();
	
	bool AreYourBodyPartsFullyFormed(){return myBodyPartsAreFullyFormedFlag;}


	inline int GetEyeState() const 
	{
		return myEyeState;
	}

	bool GetExtremePoint(int bodypart,Vector2D& point);

	//--- initialisers ---
	bool CreateSkeleton(Genome& genome,	int Age);
	bool ReloadSkeleton(Genome& genome,	int Age);
	void ReadAppearanceGenes(Genome& genome, TintManager& tinty);
	void DestroySkeleton();						// Kill body & limb entities
	void AttachLimbChain(int position,Limb* limbChain);	// attach a chain of Limbs to this joint (after constructn)

	// -- (virtual) helper functions for macro commands
	virtual bool SetAnim( const uint8* anim, int length,int part);	// helper fn for ANIM macro
	virtual bool AnimOver(int part);				// helper fn for OVER macro
	virtual bool ShowPose(int pose,int part=0);	// helper fn for POSE macro

	void SetCurrentPoseString(char* pose);			// go straight to this pose$
	void MoveTowardsTargetPoseString();					// go 1 more step towards TargetPose
	bool SetTargetPoseString(char* pose);		// define new TargetPose if necc. Retn TRUE if reached already

	void SetAnimationString(char* animation);			// start a new animation, given a c++ string ("0123R")

	void UpdateSkeleton(); // update all xys, sprite *s &
										// bounding box; avoid walls & swap
										// feet if requd. Generally init
										// body & limbs after a move/pose

	virtual void MoveBy(float xd,float yd);	
	virtual void MoveTo(float x, float y);		
	virtual bool TestMoveTo(float x, float y, bool forcePositionCheck);
	virtual bool TestMoveBy(float x, float y, bool forcePositionCheck);
	virtual bool TestCurrentLocation(bool forcePositionCheck);
	virtual bool TestRoomSystemLocation();
	virtual float GetDistanceToObstacle(int direction);
	virtual bool MoveToSafePlace(const Vector2D& positionPreferred);
	virtual bool MoveToSafePlaceGivenCurrentLocation();

	virtual int GetPlane(int part = 0);			// return *principal* plot plane

	virtual Vector2D GetMapCoordOfWhereIPickUpAgentsFrom(int pose = -1, bool bRelative = false);
	virtual Vector2D GetMyOffsetOfWhereIAmPickedUpFrom(int pose = -1);


	virtual bool HitTest(const Vector2D& point,bool alwaysTestPixelPerfectly  = false);
	virtual bool Visibility(int scope);	// retn 1 if any part of agent is on screen


	EntityImage* GetEntityImage() 
	{
		return myBody;
	}

	void SetPregnancyStage(float progesteroneLevel);


private:
	void SetupDefaults();
};










// stop any animation - MUST call this before trying to change pose via any other means
// than an animation!!!!
inline void SkeletalCreature::ResetAnimationString()
{
	
	*myAnimationString = 0; 
	myAnimationPointer = myAnimationString;
}


// attach a chain of limbs to a joint
inline void SkeletalCreature::AttachLimbChain(int position, Limb* limbChain)
{
	
	myLimbs[position] = limbChain;
}



// read world xy of down FOOT
inline Vector2D SkeletalCreature::GetDownFootPosition()
{
	
	return myDownFootPosition;
}


// read world xy of up FOOT
inline Vector2D SkeletalCreature::GetUpFootPosition()
{
	
	return myUpFootPosition;
}


// return *principal* plot plane
inline int SkeletalCreature::GetPlane(int part)
{
	
	return(myBody->GetPlane());
}





#endif //SkeletalCreature_H

