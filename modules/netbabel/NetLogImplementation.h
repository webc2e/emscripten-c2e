// Trivial class to transfer BabelCloak logs into
// the flight recorder.

#ifndef NET_LOGIN_IMPLEMENTATION_H
#define NET_LOGIN_IMPLEMENTATION_H

#include <NetLogInterface.h>

class NetLogImplementation : public NetLogInterface
{
public:
	virtual void Log(const char* text);
};

#endif

