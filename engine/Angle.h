/**
 * $Id: Angle.h,v 1.4 2001/08/23 16:23:54 firving Exp $
 *
 * Implements a simple angle class which provides basic things like
 * scaling and rotation of angles in either degrees, radians or fraction
 * of circle.  It also provides conversion to and from the equivalent
 * normalise vector to the angle.
 *
 * Author: gbuttimore
 */

#ifndef Angle_H
#define Angle_H

#include "../common/Vector2D.h"
#include "Maths.h"

class CreaturesArchive;

class Angle
{
	#define PI_VALUE (3.14159265435f)

	#define HALF_CIRCLE (PI_VALUE)
	#define RIGHT_ANGLE (HALF_CIRCLE/2.0f)
	#define FULL_CIRCLE (2.0f*HALF_CIRCLE)

protected:
	// Angle is clock starting at NORTH:
	float myAngleInRadians;
	Vector2D myNormedVector;

public:
	// Constructing:
	Angle()
	{
		SetInRadians(0.0f);
	}
	Angle(const Angle& a)
	{
		 myAngleInRadians = a.myAngleInRadians;
		 myNormedVector = a.myNormedVector;
	}
	Angle(float a)
	{
		SetAsFraction(a);
	}
	Angle(const Vector2D& v)
	{
		SetVector(v);
	}	

	void ScaleBy(float f)
	{
		SetInRadians(
			myAngleInRadians<=HALF_CIRCLE ?
				myAngleInRadians * f :
				- (FULL_CIRCLE-myAngleInRadians) * f 
		);
	}

	// Rotations:
	void RotateBy(const Angle& a)
	{
		RotateByRadians(a.myAngleInRadians);
	}
	void RotateByFraction(float a)
	{
		RotateByRadians(a*FULL_CIRCLE);
	}
	void RotateByDegrees(float a)
	{
		RotateByRadians(a*FULL_CIRCLE/360.0f);
	}
	void RotateByRadians(float a)
	{
		SetInRadians(myAngleInRadians+a);
	}

	// Rotations:
	Angle GetRotationBy(const Angle& a) const
	{
		return GetRotationByRadians(a.myAngleInRadians);
	}
	Angle GetRotationByFraction(float a) const
	{
		return GetRotationByRadians(a*FULL_CIRCLE);
	}
	Angle GetRotationByDegrees(float a) const
	{
		return GetRotationByRadians(a*FULL_CIRCLE/360.0f);
	}
	Angle GetRotationByRadians(float a) const
	{
		Angle angleToReturn;
		angleToReturn.SetInRadians(myAngleInRadians + a);
		return angleToReturn;
	}

	// Get special (returns angle < PI always)
	Angle GetArcDistanceTo(const Angle& a) const
	{
		Angle angleToReturn;
		angleToReturn.SetInRadians(a.myAngleInRadians - myAngleInRadians);

		// Convert angles of more than 180 degrees to less than
		// (as the max arc dist is 180 degrees):
		if (angleToReturn.myAngleInRadians>=HALF_CIRCLE)
			angleToReturn.SetInRadians(-angleToReturn.myAngleInRadians);

		return angleToReturn;
	}

	Angle GetNegation() const
	{
		return Angle(-GetAsFraction());
	}

	// Get functions:
	Vector2D GetVector() const
	{
		return myNormedVector;
	}
	float GetInRadians() const
	{
		return myAngleInRadians;
	}
	float GetInDegrees() const
	{
		return myAngleInRadians*360.0f/FULL_CIRCLE;
	}
	float GetAsFraction() const
	{
		return myAngleInRadians/FULL_CIRCLE;
	}

	// Set functions:
	void SetVector(const Vector2D& v);

	void SetInRadians(float a)
	{
		myAngleInRadians = BoundRadians(a);
		myNormedVector = ConvertAngleToVectorInRadians(a);
	}
	void SetInDegrees(float a)
	{
		SetInRadians(a*FULL_CIRCLE/360.0f);
	}
	void SetAsFraction(float a)
	{
		SetInRadians(a*FULL_CIRCLE);
	}


	// serialisation
	bool Write( CreaturesArchive &ar ) const;
	bool Read( CreaturesArchive &ar );


protected:
	static float BoundRadians(float a)
	{
		// get it in range [0,2PI)
		while (a>=FULL_CIRCLE)
			a-=FULL_CIRCLE;
		while (a<0.0f)
			a+=FULL_CIRCLE;
		return a;
	}
	static float ConvertVectorToAngleInRadians(const Vector2D& v)
	{
		// Ensure strictly in range that is valid for acosf
		float xcomp = v.x;
		if (xcomp < -1.0f)
			xcomp = -1.0f;
		if (xcomp > 1.0f)
			xcomp = 1.0f;

		// Work out anticlock angle from East:
		float a = acosf(xcomp);
		// Be thoroughly sure we don't get invalid floats
		if (isnan(a))
			a = HALF_CIRCLE;
		if (v.y>0.0f)
			a = -a;

		// Change to clockwise angle from East:
		a = -a;

		// Convert to rotating starting from North rather than East:
		a += RIGHT_ANGLE;

		return a;
	}
	static Vector2D ConvertAngleToVectorInRadians(float a)
	{
		// Convert to rotating starting from East rather than North:
		a -= RIGHT_ANGLE;

		// Convert to anticlock rather than clockwise:
		a = - a;

		// Reverse y coord because in C2e we measure Y downwards
		// rather than upwards:
		return Vector2D(cosf(a), - sinf(a));
	}
};


#endif//Angle_H

