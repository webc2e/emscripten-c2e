#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#ifdef _WIN32

#include "CStyleException.h"
#include "C2eDebug.h"
#include "MapScanner.h"
#include <eh.h>
#include <sstream>


// Thread local storage, used to store the stack
// pointer to terminate our backtrace at
__declspec(thread) unsigned int toplevelEbp = 0;

CStyleException::CStyleException(unsigned int exceptionCode,
                                 EXCEPTION_RECORD exceptionRecord,
                                 unsigned int *IPBacktraceStart,
                                 int IPBacktraceCount) {
  myExceptionCode = exceptionCode;
  myExceptionRecord = exceptionRecord;
  if (IPBacktraceCount > MAX_IP_BACKTRACE)
    IPBacktraceCount = MAX_IP_BACKTRACE;
  memcpy(myIPBacktrace, IPBacktraceStart,
         IPBacktraceCount * sizeof(unsigned int));
  myIPBacktraceCount = IPBacktraceCount;
}

// We store the exception code and generate the text when we
// need it, to avoid possible overhead when exceptions
// are deliberately caught.
const char *CStyleException::what() const {
  std::ostringstream out;
  out << "CSE0001: Caught C-style structured exception at ";
  ASSERT(reinterpret_cast<void *>(myIPBacktrace[0]) ==
         myExceptionRecord.ExceptionAddress);
  out << "0x" << (void *)myExceptionRecord.ExceptionAddress << "\n\n";

  switch (myExceptionCode) {
  case EXCEPTION_ACCESS_VIOLATION: {
    out << "EXCEPTION_ACCESS_VIOLATION\nThe thread attempted to "
           "read from or write to a virtual address for which it "
           "does not have the appropriate access.";
    out << std::endl << "Attempt was to ";
    if (myExceptionRecord.ExceptionInformation[0] == 0)
      out << "read from";
    else if (myExceptionRecord.ExceptionInformation[0] == 1)
      out << "write to";
    else
      out << "unknown";
    out << " address 0x" << (void *)myExceptionRecord.ExceptionInformation[1];
    break;
  }
  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    out << "EXCEPTION_ARRAY_BOUNDS_EXCEEDED\nThe thread attempted to "
           "access an array element that is out of bounds, and the "
           "underlying hardware supports bounds checking.";
    break;
  case EXCEPTION_BREAKPOINT:
    out << "EXCEPTION_BREAKPOINT\nA breakpoint was encountered.";
    break;
  case EXCEPTION_DATATYPE_MISALIGNMENT:
    out << "EXCEPTION_DATATYPE_MISALIGNMENT\nThe thread attempted to "
           "read or write data that is misaligned on hardware that does "
           "not provide alignment.\nFor example, 16-bit values must be "
           "aligned on 2-byte boundaries, 32-bit values on 4-byte "
           "boundaries, and so on.";
    break;
  case EXCEPTION_FLT_DENORMAL_OPERAND:
    out << "EXCEPTION_FLT_DENORMAL_OPERAND\nOne of the operands in a "
           "floating-point operation is denormal.\nA denormal value is "
           "one that is too small to represent as a standard "
           "floating-point value.";
    break;
  case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    out << "EXCEPTION_FLT_DIVIDE_BY_ZERO\nThe thread attempted to "
           "divide a floating-point value by a floating-point "
           "divisor of zero.";
    break;
  case EXCEPTION_FLT_INEXACT_RESULT:
    out << "EXCEPTION_FLT_INEXACT_RESULT\nThe result of a "
           "floating-point operation cannot be represented exactly "
           "as a decimal fraction.";
    break;
  case EXCEPTION_FLT_INVALID_OPERATION:
    // This exception represents any floating-point exception
    // not included in this list.
    out << "EXCEPTION_FLT_INVALID_OPERATION\nUnusual floating-point "
           "exception.";
    break;
  case EXCEPTION_FLT_OVERFLOW:
    out << "EXCEPTION_FLT_OVERFLOW\nThe exponent of a "
           "floating-point operation is greater than the magnitude "
           "allowed by the corresponding type.";
    break;
  case EXCEPTION_FLT_STACK_CHECK:
    out << "EXCEPTION_FLT_STACK_CHECK\nThe stack overflowed or "
           "underflowed as the result of a floating-point operation.";
    break;
  case EXCEPTION_FLT_UNDERFLOW:
    out << "EXCEPTION_FLT_UNDERFLOW\nThe exponent of a "
           "floating-point operation is less than the magnitude "
           "allowed by the corresponding type.";
    break;
  case EXCEPTION_ILLEGAL_INSTRUCTION:
    out << "EXCEPTION_ILLEGAL_INSTRUCTION\nThe thread tried to "
           "execute an invalid instruction.";
    break;
  case EXCEPTION_IN_PAGE_ERROR:
    out << "EXCEPTION_IN_PAGE_ERROR\nThe thread tried to access "
           "a page that was not present, and the system was unable "
           "to load the page. For example, this exception might occur "
           "if a network connection is lost while running a program "
           "over the network.";
    break;
  case EXCEPTION_INT_DIVIDE_BY_ZERO:
    out << "EXCEPTION_INT_DIVIDE_BY_ZERO\nThe thread attempted to "
           "divide an integer value by an integer divisor of zero.";
    break;
  case EXCEPTION_INT_OVERFLOW:
    out << "EXCEPTION_INT_OVERFLOW\nThe result of an integer "
           "operation caused a carry out of the most significant "
           "bit of the result.";
    break;
  case EXCEPTION_INVALID_DISPOSITION:
    out << "EXCEPTION_INVALID_DISPOSITION\nAn exception handler "
           "returned an invalid disposition to the exception "
           "dispatcher. Programmers using a high-level language "
           "such as C should never encounter this exception.";
    break;
  case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    out << "EXCEPTION_NONCONTINUABLE_EXCEPTION\nThe thread "
           "attempted to continue execution after a noncontinuable "
           "exception occurred.";
    break;
  case EXCEPTION_PRIV_INSTRUCTION:
    out << "EXCEPTION_PRIV_INSTRUCTION\nThe thread attempted to "
           "execute an instruction whose operation is not allowed "
           "in the current machine mode.";
    break;
  case EXCEPTION_SINGLE_STEP:
    out << "EXCEPTION_SINGLE_STEP\nA trace trap or other "
           "single-instruction mechanism signaled that one instruction "
           "has been executed.";
    break;
  case EXCEPTION_STACK_OVERFLOW:
    out << "EXCEPTION_STACK_OVERFLOW\nThe thread used up its stack.";
    break;
  case DBG_CONTROL_C:
    out << "DBG_CONTROL_C\nShould only be raised for the benefit of a "
           "debugger.";
    break;
  default:
    out << myExceptionCode << "\nUnknown type.";
    break;
  }

  std::vector<std::string> moduleFilenames;

  // Find filenames of engine and DLL map files
  char buff[1024];
  {
    HMODULE myHandle = GetModuleHandle(NULL);
    GetModuleFileName(myHandle, buff, 1024);
    std::string myExecutable = buff;
    myExecutable.replace(myExecutable.length() - 3, 3, "map");
    moduleFilenames.push_back(myExecutable);
  }
  for (int i = 0; i < ourModules.size(); ++i) {
    GetModuleFileName(ourModules[i], buff, 1024);
    std::string myExecutable = buff;
    myExecutable.replace(myExecutable.length() - 3, 3, "map");
    moduleFilenames.push_back(myExecutable);
  }

  out << std::endl << std::endl;

  std::vector<MapScanner::FunctionData> functionDatum;
  MapScanner mappings(moduleFilenames, myIPBacktrace, myIPBacktraceCount,
                      functionDatum);
  if (mappings.nofile) {
    out << "Error opening map file";
  } else {
    for (int i = 0; i < myIPBacktraceCount; ++i) {
      if (functionDatum[i].found) {
        out << "Contained in: \"" << functionDatum[i].functionName << "\" in "
            << functionDatum[i].objectFile << std::endl;
        // Print the mangled name as well
        // out << " (mangled " << functionDatum[i].mangledName << ")" <<
        // std::endl;
      } else {
        out << "Function not found in map file at ";
        out << "0x" << reinterpret_cast<void *>(myIPBacktrace[i]);
        out << std::endl;
      }
    }
  }
  myMessage = out.str();
  return myMessage.c_str();
}

// Do as little as possible in this function, as we could
// be in a tricky situation.  When the exception is caught,
// we do the heavier processing (in the .what() function).
void CStyleTranslationFunction(unsigned int u, EXCEPTION_POINTERS *pExp) {
  // Prevent this function being called recursively
  _set_se_translator(NULL);

  // Trace up the stack, so we can list
  // all the functions.
  unsigned int ipBacktrace[MAX_IP_BACKTRACE];
  int ipBacktraceCount = 0;

  ipBacktrace[ipBacktraceCount] = pExp->ContextRecord->Eip;
  ipBacktraceCount++;
  unsigned int ebp = pExp->ContextRecord->Ebp;

  // If we can safely backtrace, then do so
  if (toplevelEbp != 0) {
    try {
      while ((ebp != toplevelEbp) && (ipBacktraceCount < MAX_IP_BACKTRACE)) {
        unsigned int *ebpPointer = reinterpret_cast<unsigned int *>(ebp);
        unsigned int nextEbp = *ebpPointer;
        unsigned int nextIp = *(ebpPointer + 1);

        ASSERT(ipBacktraceCount < MAX_IP_BACKTRACE);
        ipBacktrace[ipBacktraceCount] = nextIp;
        ipBacktraceCount++;

        ebp = nextEbp;
      }
    } catch (...) {
      // Probably ran off the end of memory, dereferencing
      // ebpPointer.  This will happen if MakeCStyleExceptionsCppStyle 't
      // hasn't been called in enough places - e.g. MainWindowProc.
      ASSERT(false);
    }
  }

  // Set ourselves back as the translation function
  _set_se_translator(CStyleTranslationFunction);

  throw CStyleException(u, *(pExp->ExceptionRecord), ipBacktrace,
                        ipBacktraceCount);
}

MakeCStyleExceptionsCppStyle::MakeCStyleExceptionsCppStyle() {
  // Store what the Ebp was before
  myPreviousEbp = toplevelEbp;

  // Find the stack base pointer (ebp) of the
  // function which called this constructor. This
  // new "top level" ebp will mark where we stop
  // our backtrace
  unsigned int currentEbp;
  _asm mov currentEbp, ebp;
  unsigned int *ebpPointer = reinterpret_cast<unsigned int *>(currentEbp);
  unsigned int nextEbp = *ebpPointer;
  toplevelEbp = nextEbp;
  ASSERT(toplevelEbp != 0);

  // Set the conversion function
  _set_se_translator(CStyleTranslationFunction);
}

MakeCStyleExceptionsCppStyle::~MakeCStyleExceptionsCppStyle() {
  // Clear this to revert to tracing up to where we did before
  toplevelEbp = myPreviousEbp;
}

std::vector<HINSTANCE> CStyleException::ourModules;

void CStyleException::AddModule(HINSTANCE module) {
  ourModules.push_back(module);
}

#endif
