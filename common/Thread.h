// $Id: Thread.h,v 1.1 2000/08/25 16:09:09 firving Exp $
//
// Class to encapsulate threading, for portability, and
// for ease of use with C++ (rather than static or C
// functions).

#ifndef C2E_THREAD
#define C2E_THREAD

#include <process.h>

// Instantiate Thread with RECEIVER as the class 
// which contains the thread main function that
// you want to call.
template<class RECEIVER>
class Thread
{
public:
	// This is a pointer to a member function
	typedef void (RECEIVER::* MemberFunction) ();

	// Start a new thread.  The thread main function is in the
	// specificed receiver object, choosen with the member function
	// pointer.  Returns false on failure.
	bool BeginThread(RECEIVER* receiver, MemberFunction mainFunction);

	// Immediately kill the thread in a brutal fashion
	void UnsafeTerminate();

private:
	static unsigned int __stdcall StaticMain(void *data);

	unsigned int myThreadID;
	RECEIVER* myReceiver;
	MemberFunction myMainFunction;

};

#ifdef WIN32

// static 
template<class RECEIVER>
unsigned int __stdcall Thread<RECEIVER>::StaticMain(void *data)
{
	Thread<RECEIVER>* thread = (Thread<RECEIVER>*) data;
	((thread->myReceiver)->*(thread->myMainFunction))();
	return 0;
}

template<class RECEIVER>
bool Thread<RECEIVER>::BeginThread(RECEIVER* receiver, MemberFunction mainFunction)
{
	myReceiver = receiver;
	myMainFunction = mainFunction;

	unsigned long result = _beginthreadex(
		0, // security - must be 0 on win98
		0, // stack size, or 0
		&StaticMain, // start of thread execution
		(void*) this, // argument list
		0, // can be CREATE_SUSPEND or 0.
		&myThreadID); // Thread id stored in here

	return result != -1;
}

template<class RECEIVER>
void Thread<RECEIVER>::UnsafeTerminate()
{
	_endthreadex(0);
}

#endif

// TODO: put other platform implementations below

#endif // C2E_THREAD

