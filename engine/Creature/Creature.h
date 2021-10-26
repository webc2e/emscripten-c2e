
#ifndef Creature_H
#define Creature_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "../Agents/CompoundAgent.h"
#include "CreatureFacultyInterface.h"
#include "Biochemistry/BiochemistryConstants.h"
#include "CreatureConstants.h"



class Faculty;

class LinguisticFaculty;
class MotorFaculty;
class SensoryFaculty;
class ReproductiveFaculty;
class ExpressiveFaculty;
class LifeFaculty;
class MusicFaculty;

class Brain;
class Biochemistry;


class Creature : public CompoundAgent, public CreatureFacultyInterface
{
//	CREATURES_DECLARE_SERIAL( Creature )
	typedef Agent superbase;
	typedef CompoundAgent base;

public:
	// casting:

protected:
	std::string myMotherMoniker;		// to identify my mother
	std::string myFatherMoniker;		// to identify my father

	bool myBeingTrackedFlag;

	AgentHandle	myIt;					// object of this creature's attention, NULL if none
	bool myIntrospectiveFlag;			// for extraspective actions like GO EAST and GO WEST
	Vector2D myItPosition;


	void TrackIt();					// update ItX,ItY if IT has moved
public:

////////////////////////////////////////////////////////////////////////////
// IT and MONIKER stuff:
	inline void SetItPosition(const Vector2D& position)
	{
		myItPosition = position;
	}
	inline Vector2D GetItPosition()					// return curr IT's location
	{
		return myItPosition;
	}
	virtual void SetItAgent(AgentHandle& a) 
	{	// define object of your attention
		// stop script if extraspective and old it is being invalidated:
		myIt = a;
		TrackIt();                      // find out where it is
	}
	virtual AgentHandle GetItAgent() const
	{ 		// return curr IT object
		return myIt;
	}
	virtual void SetIntrospective(bool b) 
	{
		myIntrospectiveFlag= b;
	}

	virtual bool IsIntrospective() 
	{
		return myIntrospectiveFlag;
	}
	virtual std::string GetMoniker()
	{
		return GetGenomeStore().MonikerAsString(0);
	}

	// 32 character truncation of moniker (because the genome 
	// can only store 32 characters, this is all that GetMotherMoniker
	// and GetFatherMoniker return, so we compare to this short form)
	virtual std::string GetShortMoniker()
	{
		std::string mon = GetGenomeStore().MonikerAsString(0);
		if (mon.size() > 32)
			return mon.substr(0, 32);
		else
			return mon;
	}

	// 32 char only
	virtual std::string GetMotherMoniker() const 
	{
		return myMotherMoniker;
	}

	// 32 char only
	virtual std::string GetFatherMoniker() const 
	{
		return myFatherMoniker;
	}

	void RememberThatYouAreBeingTracked(bool amibeingtracked);





	
//////////////////////////////////////////////////////////////////////////////
// SKELETAL CREATURE dummy functions for faculty compatibility:
	virtual void ResetAnimationString() {}
	virtual bool PrepareForAgeing(int age);
	virtual void SetEyeState(int i) {}
	virtual void SetFacialExpression(int headset, int earset= -1) {}
	virtual int GetFacialExpression() {return 0;}
	virtual Vector2D GetNosePosition() {return GetCentre();}

///////////////////////////////////////////////////////////////////////////////
// more dummy SKELETAL CREATURE functions, this time for macros:
	virtual bool GetCreatureRoomID(int &roomID)
	{
		return GetRoomID(roomID);
	}
	virtual void Walk() {}
	virtual bool CanReachIt() {
		if (myIntrospectiveFlag || myIt.IsInvalid())
			return false;
		Box b;
		myIt.GetAgentReference().GetAgentExtent(b);
		return DoBoundsIntersect(b);
	}
	virtual bool CannotBeMuchCloser() {
		if (myIntrospectiveFlag || myIt.IsInvalid())
			return true;
		return CanReachIt();
	}
	virtual int ReachOut(BOOL Point=FALSE) {return CanReachIt() ? 1 : -1;}

	virtual void MoveFootTo(float x,float y) {
		MoveTo(x-myCurrentWidth/2.0f,y-myCurrentHeight);
	}
	virtual bool TestMoveFootTo(float x, float y, bool forcePositionCheck) {
		return TestMoveTo(x-myCurrentWidth/2.0f,y-myCurrentHeight,forcePositionCheck);
	}
	virtual Vector2D GetMouthMapPosition() {return GetCentre();}
	virtual void SetDirection(int d) {}
	virtual int GetDirection() const {return 0;}
	virtual void ChangeHairStateInSomeWay(int action) {}
	virtual int GetOverlayIndex(int region) {return 0;}
	virtual int GetClothingSet(int bodypart, int layer) {return 0;}
	virtual bool WearOutfit(int set,int layer) {return false;}
	virtual bool WearItem(int bodypart, int set, int layer) {return false;}
	virtual bool HasTargetPoseStringBeenReached() {return true;}
	virtual void SetGait(int gait) {}
	virtual void StopHoldingHandsWithThePointer(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2) {}
	virtual bool AreYouHoldingHandsWithThePointer() {return false;}
	virtual Vector2D GetDownFootPosition() {
		return Vector2D(myPositionVector.x+myCurrentWidth/2.0f, myPositionVector.y+myCurrentHeight);
	}
	virtual Vector2D GetUpFootPosition() {return GetCentre();}

	// For the PrayHandler:
	virtual void GetBodyPartFileName(int index, std::string& path) = 0;

	// new one:
	virtual float GetSensoryDistanceFromAgent(AgentHandle a) = 0;

//////////////////////////
// BASIC FUNCTIONS:
protected:
	void Construct(int family, uint32 id, AgentHandle gene, int gene_index, int sex, int variant);
	Creature();
	Creature(int family, uint32 id, AgentHandle gene, int gene_index, int sex, int variant);
	Creature(int family, uint32 id,	AgentHandle gene, int gene_index, int sex, int variant,
			FilePath const& gallery, int numimages, int baseimage, int plane);
public:
	virtual ~Creature();				// destruct an obj & remove it from objlib
	virtual void Init();				// common initialisation

	virtual float* GetLocusAddress(int type, int tissue, int organ, int locus);
	virtual bool ReadFromGenome();


////////////////////////////////////////////////////////////////////////////////
// AGENT:
	virtual void HandlePickup(Message* Msg);

	virtual void SpeakSentence(std::string& thisSentence);
	void OwnUpdate();
	virtual void Update();
   	void OwnTrash();
   	virtual void Trash();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	bool OwnWrite(CreaturesArchive &archive) const;
	bool OwnRead(CreaturesArchive &archive);


///////////////////////////////////////////////////////////////////////////////
// CREATURE:
	virtual bool Approach(bool doApproachNotRetreat=true) = 0;

	virtual float GetDriveLevel(int i);
	int GetHighestDrive();
	void MakeYourselfTired();

	virtual ExpressiveFaculty* Expressive() 
	{
		_ASSERT(!myGarbaged);
		return myExpressiveFaculty;
	}

	virtual LinguisticFaculty* Linguistic() 
	{
		_ASSERT(!myGarbaged);
		return myLinguisticFaculty;
	}

	virtual ReproductiveFaculty* Reproductive() 
	{
		_ASSERT(!myGarbaged);
		return myReproductiveFaculty;
	}

	virtual SensoryFaculty* Sensory() 
	{
		_ASSERT(!myGarbaged);
		return mySensoryFaculty;
	}

	virtual MotorFaculty* Motor() 
	{
		_ASSERT(!myGarbaged);
		return myMotorFaculty;
	}

	virtual LifeFaculty* Life() 
	{
		_ASSERT(!myGarbaged);
		return myLifeFaculty;
	}

	virtual MusicFaculty* Music() 
	{
		_ASSERT(!myGarbaged);
		return myMusicFaculty;
	}

	virtual Brain* GetBrain() 
	{
		_ASSERT(!myGarbaged);
		return myBrain;
	}

	virtual Biochemistry* GetBiochemistry() 
	{
		_ASSERT(!myGarbaged);
		return myBiochemistry;
	}

	enum {noOfFaculties=9};


	virtual float* GetInvalidLocusAddress() 
	{
		_ASSERT(!myGarbaged);
		return &myInvalidLocus;
	}

	Faculty* GetFaculty(int id)
	{
		return id>=0 && id<noOfFaculties ? myFaculties[id] : NULL;
	}

protected:
	ExpressiveFaculty* myExpressiveFaculty;
	LinguisticFaculty* myLinguisticFaculty;
	ReproductiveFaculty* myReproductiveFaculty;
	SensoryFaculty* mySensoryFaculty;
	MotorFaculty* myMotorFaculty;
	LifeFaculty* myLifeFaculty;
	MusicFaculty* myMusicFaculty;
	Brain* myBrain;
	Biochemistry* myBiochemistry;

	Faculty* myFaculties[noOfFaculties];

	float myDriveLoci[NUMDRIVES];
	float myConstantLocus;
	float myFloatingLoci[NUM_FLOATING_LOCI];
	float myAirQualityLocus;
	float myCrowdedLocus;
	float myInvalidLocus;

	// these two don't need to be serialised:
	int myUpdateTickOffset;
	static int ourNextUpdateTickOffsetToUse;

private:
	void SetupDefaults();
};
#endif //Creature_H

