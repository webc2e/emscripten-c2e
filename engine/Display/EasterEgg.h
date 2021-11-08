// $Id: EasterEgg.h,v 1.2 2001/04/24 15:01:23 firving Exp $

#ifndef EASTER_EGG_H
#define EASTER_EGG_H

#include "../C2eServices.h"

class EasterEgg
{
public:
	static void CunningBlit
		(uint32* from, 
		 uint32* to, 
		 int width, 
		 int height, 
		 int fromPitch, 
		 int toPitch,
		 bool& amIRound,
		 bool doIWantToBe);
};

#endif

