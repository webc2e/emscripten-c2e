#include "Angle.h"

#include "CreaturesArchive.h"
#include <math.h>
#include "Maths.h"

bool Angle::Write(CreaturesArchive &ar) const
{
	ar << myAngleInRadians;
	ar << myNormedVector;
	return true;
}

bool Angle::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();

	if(version >= 22)
	{
		ar >> myAngleInRadians;
		ar >> myNormedVector;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success!
	return true;
}

void Angle::SetVector(const Vector2D& v)
{
	myNormedVector = v.Normalise();
	if (isnan(myNormedVector.x) || isnan(myNormedVector.x))
	{
		myNormedVector.x = 0.0f;
		myNormedVector.y =-1.0f;
	}
	myAngleInRadians = BoundRadians(ConvertVectorToAngleInRadians(myNormedVector));
}

