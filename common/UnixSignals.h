// Classes to give better error reporting for 
// access violations and other errors.

#ifndef UNIX_SIGNALS_H
#define UNIX_SIGNALS_H

// TODO: Change this to #ifndef UNIX (whatever the define is!)
#ifdef _WIN32
#error Unix signals is Unix only
#endif

#include <signal.h>
#include "BasicException.h"

class CatchUnixSignals
{
public:
	CatchUnixSignals();
	~CatchUnixSignals();
};

void Fooble();

#endif // UNIX_SIGNALS_H

