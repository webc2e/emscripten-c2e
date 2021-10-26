#include "Circle.h"


#include "CreaturesArchive.h"

//Agent* Circle::agentTemp = NULL;

#include "Angle.h"


bool Circle::IntersectsWithViewingArc(const Vector2D& viewer, const Angle& viewerFacingAngle, float viewArcSize) const
{
	Vector2D fromViewerToCircle(myCentrePoint-viewer);
	Vector2D perpVector(GetPerpVector(fromViewerToCircle));

	float leftRayArcDist = Angle(fromViewerToCircle-perpVector).GetArcDistanceTo(viewerFacingAngle).GetAsFraction();
	float rightRayArcDist= Angle(fromViewerToCircle+perpVector).GetArcDistanceTo(viewerFacingAngle).GetAsFraction();

	// gtb!temp!:
/*	Vector2D dir = fromViewerToCircle-perpVector;
	dir = dir.Normalise();
	dir *= (1.0f-leftRayArcDist)*250.0f;
	agentTemp->DrawLine(viewer, dir);
	dir = fromViewerToCircle+perpVector;
	dir = dir.Normalise();
	dir *= (1.0f-rightRayArcDist)*250.0f;
	agentTemp->DrawLine(viewer, dir);
*/
//	agentTemp->DrawLine(viewer, fromViewerToCircle+perpVector);
//	agentTemp->DrawLine(viewer, fromViewerToCircle-perpVector);

	// can see it if either the left ray or right ray is inside the viewing arc:
	return leftRayArcDist<=viewArcSize || rightRayArcDist<=viewArcSize;
}




bool Circle::Write(CreaturesArchive &ar) const
{
	ar << myCentrePoint;
	ar << myRadius;
	return true;
}

bool Circle::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();

	if(version >= 22)
	{
		ar >> myCentrePoint;
		ar >> myRadius;
		myRadiusSquared = myRadius;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success!
	return true;
}

