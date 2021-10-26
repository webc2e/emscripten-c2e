#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "DrawableObject.h"
#include "../PersistentObject.h"

DrawableObject::DrawableObject()
		:myScreenPosition(0,0), 
		myCameraShyFlag(false),
		myPlane(0),
		myAmIASpriteFlag(false),
		myAmIALineFlag(false),
		myAmIACameraFlag(false),
		myAmRemoteCameraFlag(false)
{
	;
}

