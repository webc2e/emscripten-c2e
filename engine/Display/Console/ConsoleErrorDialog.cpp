#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "ConsoleErrorDialog.h"
#include "../../C2eServices.h"
#include "../DisplayEngine.h"
#include "../../FlightRecorder.h"

#include <sstream>

bool ErrorDialog::ourDisableContinueButton = false;

ErrorDialog::ErrorDialog() : myReturnCode(ED_QUIT) {}

int ErrorDialog::DisplayDialog() {
  theFlightRecorder.Log(FLIGHT_RUNTIME, "Error in: %s", mySource.c_str());
  theFlightRecorder.Log(FLIGHT_RUNTIME, myText.c_str());

  // not implemented yet (no input implemented)
  if (ourDisableContinueButton) {
    ourDisableContinueButton = false;
  }

  // print message
  std::cout << "Error in: " << mySource << std::endl;
  std::cout << myText << std::endl;
  std::cout << "Continuing" << std::endl;
  myReturnCode = ED_CONTINUE;

  // log error code
  switch (myReturnCode) {
  case ED_CONTINUE:
    theFlightRecorder.Log(FLIGHT_RUNTIME, "Action: Continue");
    break;
  case ED_QUIT:
    theFlightRecorder.Log(FLIGHT_RUNTIME, "Action: Quit");
    break;
  case ED_BRUTAL_ABORT:
    theFlightRecorder.Log(FLIGHT_RUNTIME, "Action: Brutal Abort");
    break;
  }

  return myReturnCode;
}

void ErrorDialog::SetText(const std::string &source, const std::string &text) {
  mySource = source;
  myText = text;
}

void ErrorDialog::DisableContinueButtonNextDialog() {
  ourDisableContinueButton = true;
}
