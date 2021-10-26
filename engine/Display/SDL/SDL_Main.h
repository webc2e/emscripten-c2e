// main for sdl version (derived from window.h/window.cpp)

#ifndef SDL_MAIN_H
#define SDL_MAIN_H

#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif

#include "../../../common/C2eTypes.h"
#include "../../Agents/AgentHandle.h"

#ifdef _WIN32
extern HWND theMainWindow;
#endif

void mainloop();

void ResetWindowTitle();

// tell the game to shut down
void SignalTerminateApplication();

bool SetGameTicking(bool bActive);
bool GetGameTicking();
void SendTickMessage();

extern AgentHandle waitingForAgent;
inline bool CheckSingleStepAgent(AgentHandle agent) {
  return agent == waitingForAgent && agent.IsValid();
}
void SetSingleStepAgent(AgentHandle agent);
AgentHandle GetSingleStepAgent();
void WaitForSingleStepCommand();

bool GlobalIsKeyDown(int keycode);

void GlobalSetCapture();
void GlobalReleaseCapture();

void GlobalWarpMouse(int newX, int newY);

#endif // OURWINDOW_H
