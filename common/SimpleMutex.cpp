// -------------------------------------------------------------------------
// Filename: SimpleMutex.cpp
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
//  availible.
//  Release() reliquishes the lock.
//  If any errors occur, a SimpleMutex::Err (derived from Exception)
//  will be thrown.
//
//
// History:
// 30Jul99   BenC   Initial version
// -------------------------------------------------------------------------

// all methods are inline
// #include "SimpleMutex.h"
