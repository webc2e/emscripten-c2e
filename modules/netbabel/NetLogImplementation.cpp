#ifdef _MSC_VER
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include "NetLogImplementation.h"
#include "../../engine/C2eServices.h"

void NetLogImplementation::Log(const char* text)
{
	theFlightRecorder.Log(FLIGHT_NETBABEL, "netbabel: %s", text);
}

