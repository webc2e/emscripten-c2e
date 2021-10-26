/**
 * $Id: Circle.h,v 1.3 2001/04/24 15:01:01 firving Exp $
 *
 * A basic circle class with support for intersection, viewing arc and
 * line of sight occlusion tests.
 *
 * Author: gbuttimore
 */

#ifndef Circle_H
#define Circle_H

#include "../common/Vector2D.h"


class CreaturesArchive;
class Angle;


class Circle
{
protected:
	Vector2D myCentrePoint;

	float myRadius;
	float myRadiusSquared;

public:
	// Constructors:
	Circle(){myRadius=myRadiusSquared=0.0f;}
	Circle(const Vector2D& c, float r)
		: myCentrePoint(c), myRadius(r)
	{
		myRadiusSquared = r*r;
	}
	Circle(float x, float y, float r)
		: myCentrePoint(x,y), myRadius(r)
	{
		myRadiusSquared = r*r;
	}

	// Get/Set methods:
	inline float GetRadius() const
	{
		return myRadius;
	}
	inline void SetRadius(float r)
	{
		myRadius = r;
		myRadiusSquared = r*r;
	}

	inline const Vector2D& GetCentre() const
	{
		return myCentrePoint;
	}
	inline void SetCentre(const Vector2D& v)
	{
		myCentrePoint = v;
	}

	// Intersection tests:
	inline bool IntersectsWithCircle(const Circle& c) const
	{
		float radii = myRadius+c.myRadius;
		return myCentrePoint.SquareDistanceTo(c.myCentrePoint) 
			< radii*radii;
	}
	inline bool ContainsPoint(const Vector2D& p) const
	{
		return myCentrePoint.SquareDistanceTo(p) < myRadiusSquared;
	}
	inline bool IntersectsWithLine(const Vector2D& l1, const Vector2D& l2) const
	{
		return GetSquarePerpendicularDistanceOfLineFromPoint(l1, l2, myCentrePoint)
			< myRadiusSquared;
	}

	// Occlusion/line of sight tests:
	inline bool IsOccludedByCircle(const Vector2D& viewer, const Circle& c) const
	{
		Vector2D fromViewerToCircle(myCentrePoint-viewer);
		Vector2D perpVector(GetPerpVector(fromViewerToCircle));

		// gtb!temp!:
//		agentTemp->DrawLine(viewer, fromViewerToCircle+perpVector);
//		agentTemp->DrawLine(viewer, fromViewerToCircle-perpVector);

		// only occludes if the two lines of sight are both blocked:
		return c.IntersectsWithLine(viewer, myCentrePoint-perpVector)
			&& c.IntersectsWithLine(viewer, myCentrePoint+perpVector);
	}
	bool IntersectsWithViewingArc(
		const Vector2D& viewer, 
		const Angle& viewerFacingAngle, 
		float viewArcSize) const;

	//temp!
//	static Agent* agentTemp;

	// serialisation
	bool Write( CreaturesArchive &ar ) const;
	bool Read( CreaturesArchive &ar );

protected:
	static float GetSquarePerpendicularDistanceOfLineFromPoint(
		const Vector2D& l1,
		const Vector2D& l2,
		const Vector2D& p)
	{
		float top = (l2.y-l1.y)*(p.x-l1.x) - (l2.x-l1.x)*(p.y-l1.y);
		float bottom = l1.SquareDistanceTo(l2);
		return top*top/bottom;
	}
	Vector2D GetPerpVector(const Vector2D& fromViewerToCircle) const
	{
		Vector2D perpVector(fromViewerToCircle.y, -fromViewerToCircle.x);
		perpVector = perpVector.Normalise();
		perpVector *= myRadius;
		return perpVector;
	}
};

#endif//Circle_H

