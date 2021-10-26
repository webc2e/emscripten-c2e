#include "Presence.h"

#include "AgentManager.h"
#include "Agents/Agent.h"
#include "Maths.h"

const float EPSILON = 0.001f;

// local function helper:
// y becomes the direction of the basis vector specified:
Vector2D GetVectorRelativeToBasis(const Vector2D& vectorToTransform, const Vector2D& basisVectorExpressedInThisSpace)
{
	return
		basisVectorExpressedInThisSpace*vectorToTransform.y +
		basisVectorExpressedInThisSpace.GetPerpendicular()*vectorToTransform.x;
}





float TendTo(float from, float atRate, float to)
{
	return from*(1.0f-atRate)+ to*atRate;
}

void Presence::ImpactWith(Presence& p, const Vector2D& yourOldCentre)
{
	// Reset my position to be exactly touching (or just beyond) the colliding object:
	float desiredDistance = myCircle.GetRadius() + p.myCircle.GetRadius();
	Vector2D goingTowards = myCircle.GetCentre();
	float a = yourOldCentre.SquareDistanceTo(goingTowards);

	//gtb!temp!
	Vector2D newCentre = goingTowards;
	if (a!=0.0f)
	{
		float c = yourOldCentre.SquareDistanceTo(p.myCircle.GetCentre()) - desiredDistance*desiredDistance;
		float b = 2.0f*(goingTowards-yourOldCentre).DotProduct(yourOldCentre-p.myCircle.GetCentre());

		float squareRoot = sqrtf(b*b - 4.0f*a*c);
		float t1 = (-b + squareRoot) / (2.0f*a);
		float t2 = (-b - squareRoot) / (2.0f*a);
		float t = OUR_MIN(t1,t2);// (pick the point closer to your old centre)
		newCentre = goingTowards*t + yourOldCentre*(1.0f-t);
	}

	// Get basis for collision, y is along the line of the collision,
	// x is at 90 degrees to that:
	Angle toCollisionBasisFromWorldCoords;
	toCollisionBasisFromWorldCoords.SetVector(p.myCircle.GetCentre() - newCentre);

	// Work out collision velocities:
	Vector2D velocityRelativeToCollisionMe = GetVectorRelativeToBasis(
		myAgentsVelocityVector,
		toCollisionBasisFromWorldCoords.GetNegation().GetVector());
	Vector2D velocityRelativeToCollisionYou = GetVectorRelativeToBasis(
		p.myAgentsVelocityVector,
		toCollisionBasisFromWorldCoords.GetNegation().GetVector());

	if (a!=0.0f)
	{
		// don't stop them if they're moving away from each other?
		if (velocityRelativeToCollisionMe.y<=0.0f && velocityRelativeToCollisionYou.y>=0.0f)
			return;

		// okay to update position now then:
		myCircle.SetCentre(newCentre);
		UpdateAgentPosition();
	}


	// Set new collision velocities:
	float oldMe = velocityRelativeToCollisionMe.y;
	float oldYou = velocityRelativeToCollisionYou.y;
	float fixedDistance = fabsf(p.myAgentsAerodynamicFactor - myAgentsAerodynamicFactor);
	velocityRelativeToCollisionMe.y = TendTo(oldYou,fixedDistance,-oldMe)*myAgentsElasticity;
	velocityRelativeToCollisionYou.y = TendTo(oldMe,fixedDistance,-oldYou)*p.myAgentsElasticity;

	velocityRelativeToCollisionMe.x *= myAgentsFriction;
	velocityRelativeToCollisionYou.x *= p.myAgentsFriction;


	// Update the agent's velocity based on our calculations:
	myAgentsVelocityVector = GetVectorRelativeToBasis(
		velocityRelativeToCollisionMe,
		toCollisionBasisFromWorldCoords.GetVector());
	p.myAgentsVelocityVector = GetVectorRelativeToBasis(
		velocityRelativeToCollisionYou,
		toCollisionBasisFromWorldCoords.GetVector());
}



void Presence::UpdateFromAgent()
{
	// position:
	float r = OUR_MIN(myAgentsWidth,myAgentsHeight)/2.0f;
	myCircle.SetRadius(r);
	myCircle.SetCentre(Vector2D(
		myAgentsPositionVector.x + myAgentsWidth/2.0f,
		myAgentsPositionVector.y + myAgentsHeight - r
	));

	// velocity:
	myAngledVelocity = GetVectorRelativeToBasis(
		myAgentsVelocityVector,
	  	// the new basis vector (the antifacing) expressed in the current space (facing space):
		myFacingAngle.GetNegation().GetVector()
	);
}
void Presence::UpdateAgent()
{
	// velocity:
	myAgentsVelocityVector = GetVectorRelativeToBasis(
		myAngledVelocity,
		// the new basis vector (the facing) expressed in the current space (normal coordinate space):
		myFacingAngle.GetVector()
	);
}


void Presence::UpdateAgentPosition()
{
	// position:
	myAgentsPositionVector = Vector2D(
		myCircle.GetCentre().x - myAgentsWidth/2.0f,
		myCircle.GetCentre().y - myAgentsHeight + myCircle.GetRadius()
	);
}


Presence::Presence(Vector2D& velocityVector, Vector2D& positionVector, float& width, float& height,
float& aeroFactor, float& elasticity, float& friction) :
	myAgentsVelocityVector(velocityVector),
	myAgentsPositionVector(positionVector),
	myAgentsWidth(width),
	myAgentsHeight(height),
	myAgentsAerodynamicFactor(aeroFactor),
	myAgentsElasticity(elasticity),
	myAgentsFriction(friction)
{
	// kludge numbers for my SM image file, gtb!temp!
	myNoOfSpritesForEachRotation = 8;
	myNoOfRotations = 12;

	myCircle = Circle();
	myFacingAngle = 0.0f;
	myViewArcSize = 0.125f;// this angle either side of the facing angle
	myAngledVelocity = Vector2D(0.0f,0.0f);

	myAngularDamping = 0.0f;
	myDampingVector.x = 0.0f;
	myDampingVector.y = 0.0f;
}

Presence::~Presence()
{
}


void Presence::SetSpriteFormat(int noOfSpritesInOneRotation, int noOfRotations)
{
	myNoOfSpritesForEachRotation = noOfSpritesInOneRotation;
	myNoOfRotations = noOfRotations;
}

const Vector2D& Presence::GetPosition() const
{
	return myCircle.GetCentre();
}



const Angle& Presence::GetViewArcSize() const
{
	return myViewArcSize;
}
void Presence::SetViewArcSize(const Angle& a)
{
	myViewArcSize = a;
}



int Presence::GetPoseOffset() const
{
	Angle a(myFacingAngle);
	int rotationNo = (int)((a.GetAsFraction()*(float)myNoOfRotations + 0.5f));
	if (rotationNo==myNoOfRotations)
		rotationNo = 0;
	return myNoOfSpritesForEachRotation * rotationNo;
}


const Circle& Presence::GetCircle() const
{
	return myCircle;
}



void Presence::SetFacingAngle(const Angle& a)
{
	myFacingAngle = a;
	UpdateFromAgent();
}
const Angle& Presence::GetFacingAngle() const
{
	return myFacingAngle;
}








void Presence::Update()
{
	// later on, if you're carrying something you should maybe
	// not be allowed to turn if that moves your carried object
	// into the way of something else.  alternatively you could
	// automagically drop the carried object i guess.
	myFacingAngle.RotateBy(myAngularVelocity);

	myAngularVelocity.ScaleBy(1.0f - myAngularDamping);
	myAngledVelocity.x *= 1.0f - myDampingVector.x;
	myAngledVelocity.y *= 1.0f - myDampingVector.y;

	UpdateAgent();
}






float Presence::GetAngularVelocity() const
{
	return myAngularVelocity.GetAsFraction();
}
void Presence::SetAngularVelocity(const Angle& a)
{
	myAngularVelocity = a;
}
float Presence::GetForwardsVelocity() const
{
	return myAngledVelocity.y;
}
void Presence::SetForwardsVelocity(float speedScalar)
{
	myAngledVelocity.y = speedScalar;
}
float Presence::GetSidewaysVelocity() const
{
	return myAngledVelocity.x;
}
void Presence::SetSidewaysVelocity(float speedScalar)
{
	myAngledVelocity.x = speedScalar;
}



float Presence::GetAngularDamping() const
{
	return myAngularDamping;
}
void Presence::SetAngularDamping(float d)
{
	myAngularDamping = d;
}
float Presence::GetForwardsDamping() const
{
	return myDampingVector.y;
}
void Presence::SetForwardsDamping(float d)
{
	myDampingVector.y = d;
}
float Presence::GetSidewaysDamping() const
{
	return myDampingVector.x;
}
void Presence::SetSidewaysDamping(float d)
{
	myDampingVector.x = d;
}




bool Presence::IsImpactingWith(const Presence& p)
{
	// can't collide with self:
	if (&p==this)
		return false;

	// two unmovable things can't collide:
	if (myAgentsAerodynamicFactor==0.0f)
		return false;

	// no collision?
	if (!p.GetCircle().IntersectsWithCircle(myCircle))
		return false;

	return true;
}



void Presence::SetNoOfSpritesInOneRotation(int n)
{
	myNoOfSpritesForEachRotation = n;
}
int Presence::GetNoOfSpritesInOneRotation() const
{
	return myNoOfSpritesForEachRotation;
}


bool Presence::Write(CreaturesArchive &ar) const
{
	myAngularVelocity.Write(ar);
	ar << myAngledVelocity;

	myFacingAngle.Write(ar);
	myCircle.Write(ar);

	myViewArcSize.Write(ar);

	ar << myNoOfRotations;
	ar << myNoOfSpritesForEachRotation;

	ar << myAngularDamping;
	ar << myDampingVector;

	return true;
}

bool Presence::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();

	if(version >= 22)
	{

		myAngularVelocity.Read(ar);
		ar >> myAngledVelocity;


		myFacingAngle.Read(ar);
		myCircle.Read(ar);

		myViewArcSize.Read(ar);

		ar >> myNoOfRotations;
		ar >> myNoOfSpritesForEachRotation;

		ar >> myAngularDamping;
		ar >> myDampingVector;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success!
	return true;
}





// Occlusion/line of sight tests:
bool Presence::IsOccludedByPresence(const Vector2D& viewer, const Presence& p) const
{
	return myCircle.IsOccludedByCircle(viewer, p.GetCircle());
}
bool Presence::IntersectsWithViewingArc(
	const Vector2D& viewer, 
	const Angle& viewerFacingAngle, 
	float viewArcSize) const
{
	return myCircle.IntersectsWithViewingArc(viewer, viewerFacingAngle, viewArcSize);
}

