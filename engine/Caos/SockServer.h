#ifndef SOCK_SERVER_H
#define SOCK_SERVER_H

#include "../../common/SimpleMutex.h"
#include <SDL/SDL.h>
#include <vector>
#include <SDL/SDL_thread.h>

class SockServer
{
public:
	SockServer(int port, bool secure);
	~SockServer();

	void ProcessRequests();

private:
	static int Main(void *data);
	int Main();
	void ShutdownSocket(int fd);

	int myPort;
	bool mySecure;
	SDL_Thread* myThread;

	std::vector<int> myPendingSockets;
	SimpleMutex myPendingSocketsMutex;

	bool myTerminateFlag;
	SimpleMutex myTerminateFlagMutex;
};

#endif

