/**
 * $Id: Presence.h,v 1.3 2001/04/24 15:01:04 firving Exp $
 *
 * Implements an additional physical presence for agents which allows them to 
 * have rotation, angular velocity and velocity in the direction of their facing.
 * It also provides for line of sight tests within an arc in the faced direction
 * and occlusion of objects behind other objects in this line of sight test.
 *
 * Author: gbuttimore
 */

#ifndef Presence_H
#define Presence_H

#include "Angle.h"
#include "Circle.h"


class Presence
{
public:
	Presence(Vector2D& velocityVector, Vector2D& positionVector, float& width, float& height,
		float& aeroFactor, float& elasticity, float& friction);
	~Presence();

	// pose stuff:
	void SetSpriteFormat(int noOfSpritesInOneRotation, int noOfRotations);
	int GetPoseOffset() const;
	int GetNoOfSpritesInOneRotation() const;
	void SetNoOfSpritesInOneRotation(int n);

	const Angle& GetViewArcSize() const;
	void SetViewArcSize(const Angle& a);

	const Vector2D& GetPosition() const;
	const Circle& GetCircle() const;

	const Angle& GetFacingAngle() const;
	void SetFacingAngle(const Angle& a);

	// velocity stuff:
	float GetAngularVelocity() const;
	float GetAngularDamping() const;
	float GetForwardsVelocity() const;
	float GetForwardsDamping() const;
	float GetSidewaysVelocity() const;
	float GetSidewaysDamping() const;

	void SetAngularVelocity(const Angle& a);
	void SetAngularDamping(float d);
	void SetForwardsVelocity(float speedScalar);
	void SetForwardsDamping(float d);
	void SetSidewaysVelocity(float speedScalar);
	void SetSidewaysDamping(float d);

	// keeping information in sync between here and Agent:
	void UpdateFromAgent();
	void Update();

	// serialisation
	bool Write( CreaturesArchive &ar ) const;
	bool Read( CreaturesArchive &ar );

	// Occlusion/line of sight tests:
	bool IsImpactingWith(const Presence& p);
	void ImpactWith(Presence& p, const Vector2D& yourOldCentre);
	bool IsOccludedByPresence(const Vector2D& viewer, const Presence& p) const;
	bool IntersectsWithViewingArc(
		const Vector2D& viewer, 
		const Angle& viewerFacingAngle, 
		float viewArcSize) const;

protected:
	void UpdateAgentPosition();
	void UpdateAgent();

	Angle myAngularVelocity;
	Vector2D myAngledVelocity;

	Angle myFacingAngle;
	Circle myCircle;

	Angle myViewArcSize;

	int myNoOfRotations;
	int myNoOfSpritesForEachRotation;

	// Agent var references:
	Vector2D& myAgentsVelocityVector;
	Vector2D& myAgentsPositionVector;
	float& myAgentsWidth;
	float& myAgentsHeight;
	float& myAgentsAerodynamicFactor;
	float& myAgentsElasticity;
	float& myAgentsFriction;

	float myAngularDamping;
	Vector2D myDampingVector;
};

#endif//Presence_H

