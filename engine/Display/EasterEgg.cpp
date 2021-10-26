// $Id: EasterEgg.cpp,v 1.4 2000/06/23 17:09:21 firving Exp $

#include "EasterEgg.h"
#include "../Maths.h"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <string.h>
#endif

void EasterEgg::CunningBlit
	(uint16* from, 
	 uint16* to, 
	 int width, 
	 int height, 
	 int fromPitch, 
	 int toPitch,
	 bool& amIRound,
	 bool doIWantToBe)
{
	static int count = 0;

	int rad = OUR_MIN(width, height) * OUR_MIN(width, height) / 4;

	uint16* buf = new uint16[width];
	for(int scanline = 0; scanline < height; scanline++)
	{
		int yrel = - (height / 2) + scanline;
		// SPARKY CHANGES
		int xend = (int)sqrt((double)(rad - yrel * yrel));

		bool neg = false;
		if (xend < 0)
		{
			neg = true;
			xend = - xend;
		}
		if (count < 100)
		{
			xend = xend + ((width / 2 - xend) * (100 - count) / 100);
		}
		if (neg)
			xend = -xend;

		if (xend < 1)
		{
#ifdef _WIN32
			ZeroMemory(to, width * 2);
#else
			bzero(to, width * 2);
#endif
			to += width;	
		}
		else
		{
			memcpy(buf, from, width * 2);

			for(int xpos = 0; xpos < width; xpos++)
			{
				int xrel = - (width / 2) + xpos;

				uint16 px;
				if (xrel < - xend || xrel > xend)
					px = 0;
				else
				{
					int offset = ( (xrel + xend) * width / 2 / xend );
					if (offset < 0)
						offset = 0;
					else if (offset >= width)
						offset = width - 1;
					px = *(buf +  offset);
				}
				*to = px;

				to++;
			}
		}

		from += fromPitch;
		to += toPitch - width;
	}
	delete[] buf;

	if (doIWantToBe)
	{
		if (count < 100)
			count += 5;
	}
	else
		count -= 5;
	if (doIWantToBe && count == 100)
		amIRound = true;
	else if (!doIWantToBe && count == 0)
		amIRound = false;
}


