#ifdef _WIN32
// Classes to give better error reporting for
// access violations and other errors.

// This only works under Windows, and the code is also
// very 32 bit, Intel and  possibly Microsoft Visual C++
// specific.  Not portable, certainly.

// Back tracing doesn't work with Frame-Pointer Optimisation
// in Visual C++.  It can crash your application with
// Abnormal Program Termination errors. Specify the /Oy- switch
// to turn off that optimisation, and you'll get a more detailed
// function breakdown.

//
// KNOWN VC6.0 PROBLEM:
//
// In release mode, SE exceptions won't be caught/translated unless
// you're also throwing standard C++ exceptions.
//
// This is because VC6.0 compiles for synchronous exception handling
// by default. Switching the project settings to async exception
// handling fixes the problem, as does throwing your exception class
// somewhere in the code flow.
//
// Most of the time this won't be a problem. I only discovered it when
// compiling a testbed app which did no "throw"s of its own.
// C3 never seemed to have any problems of this sort.
//
// More info:
// Microsoft Systems Journal, October '99, Bugslayer column:
//
//  http://msdn.microsoft.com/library/periodic/period99/bugslayer1099.htm
//

#ifndef C_STYLE_EXCEPTION_H
#define C_STYLE_EXCEPTION_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "BasicException.h"
#include <Windows.h>
#include <vector>

// Create an instance of MakeCStyleExceptionsCppStyle to enable
// conversion of all C style structured exceptions to C++ ones.
// Backtraces are enabled for the duration of its scope, after
// scope is left only the deepest function name is shown.

// A good place to instantiate this is in WinMain and in
// the ThreadMain of all threads.  In Creatures 3, the two
// non-main threads were just simple workers passing data
// on, and didn't have any user interface or error reporting.
// Because of this, the comment "KNOWN VC6.0 PROBLEM" above
// applied, and it wasn't worth calling this.

// Also, you need to have a nested instance of this class
// in any Windows callbacks, such as WndProc.  This is because
// the stack backtrace stops being valid at that point.
// A nested instance just changes where the backtrace goes to,
// - it doesn't otherwise affect things.

// If you have other DLLs with map files, call
// CStyleException::AddModule for each one.

class MakeCStyleExceptionsCppStyle {
public:
  MakeCStyleExceptionsCppStyle();
  ~MakeCStyleExceptionsCppStyle();

private:
  unsigned int myPreviousEbp;
};

// Not an MFCly named class, but a genuine
// need for a "C" at the start of a class name :-)
const int MAX_IP_BACKTRACE = 64;
class CStyleException : public BasicException {
public:
  CStyleException(unsigned int exceptionCode, EXCEPTION_RECORD exceptionRecord,
                  unsigned int *IPBacktraceStart, int IPBacktraceCount);
  virtual const char *what() const;
  static void AddModule(HINSTANCE module);

private:
  unsigned int myExceptionCode;
  EXCEPTION_RECORD myExceptionRecord;
  unsigned int myIPBacktrace[MAX_IP_BACKTRACE];
  unsigned int myIPBacktraceCount;

  static std::vector<HINSTANCE> ourModules;
};

#endif // C_STYLE_EXCEPTION_H

#endif
