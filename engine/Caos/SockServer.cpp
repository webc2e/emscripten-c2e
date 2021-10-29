// Sockets based external interface, used for communication
// by external apps on non-Win32 versions.

#include <SDL/SDL.h>
#include "SockServer.h"
#include "../../common/BasicException.h"
#include "../../common/FileFuncs.h"
#include "../CosInstaller.h"
#include "../DirectoryManager.h"

#include <iostream>
#include <string.h>
#include <sys/types.h>
#ifdef _WIN32
	#include <winsock2.h>
	#define ENOTSOCK WSAENOTSOCK
	#define ENOTCONN WSAENOTCONN
	#define EADDRINUSE WSAEADDRINUSE

	#define close closesocket
	#define bzero ZeroMemory
	#define SETSOCKOPTCAST const char *
	#define socklen_t int
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	// #include <tcpd.h>
	#include <unistd.h>
	#define SETSOCKOPTCAST void *
#endif
#include <string>
#include <errno.h>
#include <sstream>
#include <strstream>
#include <fstream>

SockServer::SockServer(int port, bool secure) {
  myPort = port;
  mySecure = secure;
  // if (myPort >= 0) {
  //   myThread = SDL_CreateThread(&SockServer::Main, this);
  //   if (myThread == NULL)
  //     throw BasicException("Failed to make SockServer thread");
  // }
}

SockServer::~SockServer() {
  // TODO: Debrutalise!
  if (myPort >= 0)
    SDL_KillThread(myThread);
}

void SockServer::ProcessRequests() {
  myPendingSocketsMutex.Lock();

  for (int i = 0; i < myPendingSockets.size(); ++i) {
    int insockfd = myPendingSockets[i];

    // receive stuff
    bool done = false;
    std::string gotMessage;
    do {
      const int maxInput = 1024 * 64;
      char input[maxInput + 1];
      int retRecv = recv(insockfd, input, maxInput, 0);
      if (retRecv == -1) {
        std::cerr << "Failed to CAOS receive anything" << std::endl;
        return;
      }

      input[retRecv] = 0;
      gotMessage += std::string(input);
      if (gotMessage.find("\nrscr") != std::string::npos)
        done = true;

      if (retRecv == 0)
        done = true;
    } while (!done);

    // install
    std::istrstream in(&gotMessage[0], gotMessage.size());
    std::stringstream out;
    CosInstaller cosInstaller(in, out);

    // send stuff
    std::string message = out.str();
    int retSend = send(insockfd, message.c_str(), message.size(), 0);
    if (retSend == -1) {
      std::cerr << "Failed to CAOS send at all" << std::endl;
      return;
    }
    if (retSend != message.size()) {
      std::cerr << "Failed to CAOS send everything" << std::endl;
    }

    // shutdown socket
    ShutdownSocket(insockfd);
  }

  myPendingSockets.clear();
  myPendingSocketsMutex.Release();
}

void SockServer::ShutdownSocket(int fd) {
  int retShutdown = shutdown(fd, 2);
  if (retShutdown == -1) {
    std::cerr << "Shutting down temp CAOS socket failed : ";
    if (errno == EBADF)
      std::cerr << "EBADF";
    else if (errno == ENOTSOCK)
      std::cerr << "ENOTSOCK";
    else if (errno == ENOTCONN)
      std::cerr << "ENOTCONN";
    else
      std::cerr << "unknown";
    std::cerr << std::endl;
    return;
  }

  int retClose = close(fd);
  if (retClose == -1) {
    std::cerr << "Closing temp CAOS socket failed error " << errno;
    return;
  }
}

int SockServer::Main(void *data) { return ((SockServer *)data)->Main(); }

int SockServer::Main() {
  ASSERT(myPort >= 0);
  int portnum = myPort;

  // make a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cerr << "Failed to make CAOS socket" << std::endl;
    return 1;
  }

  // attach to a port and address
  int retBind;
  do {
    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(portnum);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(localAddr.sin_zero), 8);

    // Reuse ports which are in TIME_WAIT, so the game
    // binds to the correct port when you restart, without
    // having to timeout
    int i = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (SETSOCKOPTCAST)&i,
                   sizeof(i)) < 0) {
      std::cerr << "Failed to COAS setsockopt SO_REUSEADDR" << std::endl;
      return 1;
    }

    retBind =
        bind(sockfd, (struct sockaddr *)&localAddr, sizeof(struct sockaddr));
    if (retBind == -1) {
      std::cerr << "Failed to bind CAOS to socket on port " << portnum;
      std::cerr << " with err " << errno << std::endl;
      if (errno != EADDRINUSE)
        return 1;
      ++portnum;
    }
  } while (retBind == -1);
  myPort = portnum;
  std::cout << std::endl
            << "CAOS injection bound to port " << portnum << std::endl;

  // write port to file, so external programs can use it
  {
    // To ~/.creaturesengine
    std::string creaturesEngineHome =
        std::string(getenv("HOME")) + "/.creaturesengine";
    CreateDirectory(creaturesEngineHome.c_str(), NULL);
    std::string portFilename = creaturesEngineHome + "/port";
    std::ofstream portFile(portFilename.c_str());
    portFile << portnum;
  }
  {
    // To the game home directory (e.g. ~/.creatures3)
    std::string portFilename =
        theDirectoryManager.GetDirectory(MAIN_DIR) + "port";
    std::ofstream portFile(portFilename.c_str());
    portFile << portnum;
  }

  // say we're a server
  int retListen = listen(sockfd, 64);
  if (retListen == -1) {
    std::cerr << "Failed to say we're CAOS listening" << std::endl;
    perror("Listening error");
    return 1;
  }

  while (true) {
    // accept things
    struct sockaddr_in incomingSockAddr;
    socklen_t incomingSockAddrLen = sizeof(incomingSockAddr);
    int insockfd = accept(sockfd, (struct sockaddr *)&incomingSockAddr,
                          &incomingSockAddrLen);
    if (insockfd == -1) {
      std::cerr << "Failed to accept CAOS connection error " << errno
                << std::endl;
      perror("Socket error");
    } else if (mySecure &&
               incomingSockAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
      std::cerr << "CAOS connection attempt from ";
      std::cerr << inet_ntoa(incomingSockAddr.sin_addr);
      std::cerr << " ignored, only localhost allowed" << std::endl;
      std::cerr << "(add PortSecurity 0 to user.cfg to allow other connections)"
                << std::endl;
      ShutdownSocket(insockfd);
    } else {
      myPendingSocketsMutex.Lock();
      myPendingSockets.push_back(insockfd);
      myPendingSocketsMutex.Release();
    }
  }

  return 0;
}
