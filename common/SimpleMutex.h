// -------------------------------------------------------------------------
// Filename: SimpleMutex.h
// Class:    SimpleMutex
// Purpose:  For arbitration of resources accessed by multiple threads
//
// Description:
//
//  This class is designed to provide a hassle-free mutex object.
//  It doesn't support frills like named mutexes or timeouts.
//
// Usage:
//  Call Lock() to obtain the mutex. Lock() will block until the mutex is
//  available.
//  Release() reliquishes the lock.
//  If any errors occur, a SimpleMutex::Err (derived from Exception)
//  will be thrown.
//
//
// History:
// 30Jul99   BenC   Initial version
// -------------------------------------------------------------------------

#ifndef SIMPLEMUTEX_H
#define SIMPLEMUTEX_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <SDL/SDL.h>
#endif

#include "BasicException.h"
#include "C2eDebug.h"

class SimpleMutex {
public:
  // ----------------------------------------------------------------------
  // Method:		Construction/Destruction
  // Arguments:	none
  // Returns:		none
  // Description: throws SimpleMutex::Err if anything goes wrong.
  // ----------------------------------------------------------------------
  SimpleMutex();
  ~SimpleMutex();

  // ----------------------------------------------------------------------
  // Method:		Lock
  // Arguments:	none
  // Returns:		none
  // Description:
  //  Blocks the calling thread indefinitely until the mutex is available.
  //  throws SimpleMutex::Err if anything goes wrong.
  // ----------------------------------------------------------------------
  void Lock();

  // ----------------------------------------------------------------------
  // Method:		Release
  // Arguments:	none
  // Returns:		none
  // Description:
  //  Unlocks the mutex. Must be paired with Lock().
  // ----------------------------------------------------------------------
  void Release();

private:
#ifdef _WIN32
  HANDLE myMutex;
#else
  SDL_mutex *myMutex;
#endif
};

// inlines:

#ifdef _WIN32

inline SimpleMutex::SimpleMutex() {
  myMutex = CreateMutex(NULL, false, NULL);
  if (myMutex == (HANDLE)ERROR_INVALID_HANDLE) {
    throw BasicException("Couldn't create Win32 mutex");
  }
}

inline SimpleMutex::~SimpleMutex() {
  if (CloseHandle(myMutex) == 0) {
    throw BasicException("Couldn't close Win32 mutex");
  }
}

inline void SimpleMutex::Lock() {
  if (WaitForSingleObject(myMutex, INFINITE) != WAIT_OBJECT_0) {
    throw BasicException("Couldn't lock Win32 mutex");
  }
}

inline void SimpleMutex::Release() {
  if (ReleaseMutex(myMutex) == 0) {
    throw BasicException("Couldn't release Win32 mutex");
  }
}

#else // SDL

inline SimpleMutex::SimpleMutex() {
  myMutex = SDL_CreateMutex();
  if (!myMutex)
    throw BasicException("Failed to create SDL mutex");
}

inline SimpleMutex::~SimpleMutex() { SDL_DestroyMutex(myMutex); }

inline void SimpleMutex::Lock() {
  if (SDL_mutexP(myMutex) == -1)
    throw BasicException("Failed to lock SDL mutex");
}

inline void SimpleMutex::Release() {
  if (SDL_mutexV(myMutex) == -1)
    throw BasicException("Failed to release SDL mutex");
}

#endif

class SimpleMutexLock {
public:
  SimpleMutexLock(SimpleMutex &simpleMutex) : mySimpleMutex(simpleMutex) {
    mySimpleMutex.Lock();
  }

  ~SimpleMutexLock() { mySimpleMutex.Release(); }

private:
  SimpleMutex &mySimpleMutex;
};

#endif // SIMPLEMUTEX_H
