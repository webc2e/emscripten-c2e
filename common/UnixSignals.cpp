#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "UnixSignals.h"
#include <sstream>
// #include <execinfo.h>

#include "../engine/Display/ErrorMessageHandler.h"

// NB: This isn't thread safe yet, but lc2e only
// uses it in one thread so we're OK.

// This would need to be in thread local storage?
bool ourSignalCatcherInside = false;

void UnixSignalCatcher(int signal) {
  // Prevent recursive entry
  if (ourSignalCatcherInside) {
    std::cerr << "Recursive signal " << std::endl;
    exit(1);
  }
  ourSignalCatcherInside = true;

  std::ostringstream out;
  out << "UXS0001: Caught Unix signal ";
  switch (signal) {
  case SIGHUP:
    out << "SIGHUP";
    break;
  case SIGINT:
    out << "SIGINT";
    break;
  case SIGQUIT:
    out << "SIGQUIT";
    break;
  case SIGILL:
    out << "SIGILL";
    break;
  case SIGTRAP:
    out << "SIGTRAP";
    break;
  case SIGABRT:
    out << "SIGABRT/SIGIOT";
    break;
  case SIGBUS:
    out << "SIGBUS";
    break;
  case SIGFPE:
    out << "SIGFPE";
    break;
  case SIGKILL:
    out << "SIGKILL";
    break;
  case SIGUSR1:
    out << "SIGUSR1";
    break;
  case SIGSEGV:
    out << "SIGSEGV";
    break;
  case SIGUSR2:
    out << "SIGUSR2";
    break;
  case SIGPIPE:
    out << "SIGPIPE";
    break;
  case SIGALRM:
    out << "SIGALM";
    break;
  case SIGTERM:
    out << "SIGTERM";
    break;
  // case SIGSTKFLT:
  //   out << "SIGSTKFLT";
  // break;
  case SIGCHLD:
    out << "SIGCHLD";
    break;
  case SIGCONT:
    out << "SIGCONT";
    break;
  case SIGSTOP:
    out << "SIGSTOP";
    break;
  case SIGTSTP:
    out << "SIGTSTP";
    break;
  case SIGTTIN:
    out << "SIGTTIN";
    break;
  case SIGTTOU:
    out << "SIGTTOU";
    break;
  case SIGURG:
    out << "SIGURG";
    break;
  case SIGXCPU:
    out << "SIGXCPU";
    break;
  case SIGXFSZ:
    out << "SIGXFSZ";
    break;
  case SIGVTALRM:
    out << "SIGVTALRM";
    break;
  case SIGPROF:
    out << "SIGPROF";
    break;
  case SIGWINCH:
    out << "SIGWINCH";
    break;
  case SIGIO:
    out << "SIGIO/SIGPOLL";
    break;
  // case SIGPWR:
  //   out << "SIGPWR";
  //   break;
  // case SIGUNUSED:
  //   out << "SIGUNUSED";
  //   break;
  default:
    out << "Unknown";
    break;
  }
  out << " " << signal << std::endl;

  // Stack trace
  out << std::endl;
  void *buf[32];
  // int count = backtrace(buf,32);
  // char **s = backtrace_symbols(buf,count);
  // for(int i=0; i < count; i++)
  // 	out << s[i] << std::endl;
  // std::cerr << out.str() << std::endl;

  // Display in GUI dialog
  // ErrorDialog::DisableContinueButtonNextDialog();
  ErrorMessageHandler::NonLocalisable(out.str(), "UnixSignals::UnixSignals");

  // Allow entry again
  ourSignalCatcherInside = false;
}

CatchUnixSignals::CatchUnixSignals() {
  struct sigaction sa_new;
  struct sigaction sa_old;
  sa_new.sa_handler = UnixSignalCatcher;
  // Trap all signals except a specific set
  sigemptyset(&sa_new.sa_mask);
  for (int i = 0; i < 35; ++i) {
    // These signals aren't trapped:
    // SIGWINCH - console window that games was launched from was resized
    // SIGCHLD - called process has finished (e.g. when launching a URL)
    if ((i != SIGINT) && (i != SIGTERM) && (i != SIGQUIT) && (i != SIGTRAP) &&
        (i != SIGTSTP) && (i != SIGCONT) && (i != SIGHUP) && (i != SIGWINCH) &&
        (i != SIGCHLD) && (i != SIGPIPE)) {
      sigaction(i, &sa_new, &sa_old);
    }
  }

  // These signals are ignored:
  // SIGPIPE - not connected to internet when do a ::send
  sa_new.sa_handler = SIG_IGN;
  sigaction(SIGHUP, &sa_new, &sa_old);
  sigaction(SIGPIPE, &sa_new, &sa_old);
}

CatchUnixSignals::~CatchUnixSignals() {}
