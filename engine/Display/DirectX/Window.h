// -------------------------------------------------------------------------
// Filename:    Window.h
// Class:       None
// Purpose:     Main window and message pump stuff
// Description:
//				Window.cpp and Window.h provide the basic framework
//				services used by the game:
//				- The Main Window
//				- Timing
//				- Mouse and keyboard input
//
//				The single App object is probably about the only class
//				which should interact with Window.h and Window.cpp...
//
// Usage:
//
//
// History:
// -------------------------------------------------------------------------
#ifndef OURWINDOW_H
#define OURWINDOW_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <windows.h>
#include "../../../common/C2eTypes.h"
#include "../../Agents/AgentHandle.h"
#include "../../TimeFuncs.h"

extern HWND theMainWindow;
void ResetWindowTitle();

void SendTickMessage();
bool SetGameTicking(bool bActive);
bool GetGameTicking();

void SignalTerminateApplication();

// provides a means to single step through _every_ line of code executed
// by an agent, even if several are run in one tick
void SetSingleStepAgent(AgentHandle agent);
AgentHandle GetSingleStepAgent();
void WaitForSingleStepCommand();

extern AgentHandle waitingForAgent;
inline bool CheckSingleStepAgent(AgentHandle agent)
{
	return agent == waitingForAgent && agent.IsValid();
}

bool GlobalIsKeyDown(int keycode);

void GlobalSetCapture();
void GlobalReleaseCapture();

void GlobalWarpMouse(int newX, int newY);

#endif // OURWINDOW_H

