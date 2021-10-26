// -------------------------------------------------------------------------
// Filename:	InputManager.cpp
// Class:		InputManager
// Purpose:		Provide an independant mechanism for handling input devices.
// Description:
//
// Coming soon to a header file near you!
//
//
// Usage:
//
//
//
// History:
// 26Jan99		Creation	BenC
// -------------------------------------------------------------------------


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "InputManager.h"
#include "C2eServices.h"
#include "App.h"
#include "CreaturesArchive.h"
#include "Display/Window.h"

#ifndef _WIN32
	#include "Display/SDL/scrap.h"
#endif

#ifdef C2D_DIRECT_DISPLAY_LIB
	#include "Camera/MainCamera.h"
#endif

// how many ticks we store mouse position for
const int InputManager::ourRecentPositions = 3;
// how many ticks ago velocity is measured from
const int InputManager::ourVelocityAgo = 3; 

InputManager::InputManager()
{
	myEventPendingMask = 0;
	myMouseX = 0;
	myMouseY = 0;

	ASSERT(ourVelocityAgo <= ourRecentPositions);
	myRecentPos = 0;
	myRecentMouseY.resize(ourRecentPositions);
	myRecentMouseX.resize(ourRecentPositions);
	for (int i = 0; i < ourRecentPositions; ++i)
	{
		myRecentMouseX[i] = 0;
		myRecentMouseY[i] = 0;
	}

	myTranslatedCharTarget = 0;
}


int InputManager::GetEventCount()
{
	return myEventBuffer.size();
}

int InputManager::GetPendingMask()
{
	return myEventPendingMask;
}


const InputEvent& InputManager::GetEvent( int i )
{
	return myEventBuffer[i];
}


// system-framework access:
void InputManager::SysFlushEventBuffer()
{
	// store mouse move information for velocity
	myRecentPos++;
	if (myRecentPos >= ourRecentPositions)
		myRecentPos = 0;
	myRecentMouseX[myRecentPos] = myMouseX;
	myRecentMouseY[myRecentPos] = myMouseY;

	myEventBuffer.clear();
	myEventPendingMask = 0;
}


void InputManager::SysAddKeyDownEvent( int keycode )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventKeyDown;
	ev.KeyData.keycode = keycode;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventKeyDown;
}

void InputManager::SysAddKeyUpEvent( int keycode )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventKeyUp;
	ev.KeyData.keycode = keycode;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventKeyUp;
}

void InputManager::SysAddTranslatedCharEvent( int keycode )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventTranslatedChar;
	ev.KeyData.keycode = keycode;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventTranslatedChar;
}

void InputManager::SysAddMouseDownEvent( int mx, int my, int button )
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	theMainView.ScaleMouseCoordinatesAccordingToWindowSize(mx, my);
#endif

	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseDown;
	ev.MouseButtonData.button = button;
	ev.MouseButtonData.mx = mx;
	ev.MouseButtonData.my = my;
	myEventBuffer.push_back( ev );

	myMouseX = mx;
	myMouseY = my;
	myEventPendingMask |= InputEvent::eventMouseDown;
}

void InputManager::SysAddMouseUpEvent( int mx, int my, int button )
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	theMainView.ScaleMouseCoordinatesAccordingToWindowSize(mx, my);
#endif

	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseUp;
	ev.MouseButtonData.button = button;
	ev.MouseButtonData.mx = mx;
	ev.MouseButtonData.my = my;
	myEventBuffer.push_back( ev );

	myMouseX = mx;
	myMouseY = my;
	myEventPendingMask |= InputEvent::eventMouseUp;
}

void InputManager::SysAddMouseMoveEvent( int mx, int my )
{
#ifdef C2D_DIRECT_DISPLAY_LIB
	theMainView.ScaleMouseCoordinatesAccordingToWindowSize(mx, my);
#endif

	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseMove;
	ev.MouseMoveData.mx = mx;
	ev.MouseMoveData.my = my;
	myEventBuffer.push_back( ev );

	myMouseX = mx;
	myMouseY = my;
	myEventPendingMask |= InputEvent::eventMouseMove;
}


void InputManager::SysAddMouseWheelEvent( int mx, int my, int delta )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseWheel;
	ev.MouseWheelData.delta = delta;
	ev.MouseWheelData.mx = mx;
	ev.MouseWheelData.my = my;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventMouseWheel;
}

bool InputManager::IsKeyDown( int keycode )
{
	return GlobalIsKeyDown(keycode);
}

void InputManager::SetMousePosition(int newX, int newY)
{
	GlobalWarpMouse(newX, newY);
	myMouseX = newX;
	myMouseY = newY;
}

float InputManager::GetMouseVX()
{
	int velo_pos = (myRecentPos + 1 - ourVelocityAgo + ourRecentPositions) % ourRecentPositions;
	return (float)(myMouseX - myRecentMouseX[velo_pos]) / (float)ourVelocityAgo;
}

float InputManager::GetMouseVY()
{
	int velo_pos = (myRecentPos + 1 - ourVelocityAgo + ourRecentPositions) % ourRecentPositions;
	return (float)(myMouseY - myRecentMouseY[velo_pos]) / (float)ourVelocityAgo;
}

TranslatedCharTarget *InputManager::GetTranslatedCharTarget() const
{
	return myTranslatedCharTarget;
}

void InputManager::SetTranslatedCharTarget( TranslatedCharTarget *target, bool tellLoser )
{
	TranslatedCharTarget *oldTarget = myTranslatedCharTarget;
	myTranslatedCharTarget = target;
	if( tellLoser && oldTarget )
		oldTarget->LoseFocus();
	if (myTranslatedCharTarget)
		myTranslatedCharTarget->GainFocus();
}

TranslatedCharTarget::~TranslatedCharTarget()
{
	InputManager& inputManager = theApp.GetInputManager();
	if( inputManager.GetTranslatedCharTarget() == this )
		inputManager.SetTranslatedCharTarget( NULL, false );
}

void TranslatedCharTarget::SaveFocusState( CreaturesArchive &archive ) const
{
	bool target = (theApp.GetInputManager().GetTranslatedCharTarget() == this);
	archive << target;
}

void TranslatedCharTarget::RestoreFocusState( CreaturesArchive &archive )
{
	bool flag;
	archive >> flag;
	if (flag)
		theApp.GetInputManager().SetTranslatedCharTarget( this, true );
}

#ifndef _WIN32
	static char *scrapSDL = NULL;
#endif

std::string InputManager::FetchTextFromClipboard()
{
#ifdef _WIN32
    if (!IsClipboardFormatAvailable(CF_TEXT)) 
        return ""; 
    if (!OpenClipboard(NULL)) 
        return ""; 

	std::string ret;
    HGLOBAL hglb = GetClipboardData(CF_TEXT); 
    if (hglb != NULL) 
    { 
        char* pstr = (char*)GlobalLock(hglb); 
        if (pstr != NULL) 
        { 
            ret = pstr;
            GlobalUnlock(hglb); 
        } 
    } 
    CloseClipboard(); 

	// Convert CR/LF to LF
	std::string convText;
	for (int i = 0; i < ret.size(); ++i)
	{
		char c = ret[i];
		if (c != 13)
			convText += c;
	}

    return convText; 
#else // C2E_SDL
	return ""; // this doesn't work yet

	int scraplen;

	get_scrap(T('T','E','X','T'), &scraplen, &scrapSDL);
	if ( scraplen == 0 ) 
		return "";

	// Convert the scrap from Mac text to UNIX text
	char *cp;
	int i;
	for (cp = scrapSDL, i = 0; i < scraplen; ++cp, ++i)
	{
		if (*cp == '\r')
			*cp = '\n';
	}
	return scrapSDL;
#endif
}

void InputManager::SendTextToClipboard(const std::string& text)
{
#ifdef _WIN32
	// Convert LF to CR/LF
	std::string convText;
	for (int i = 0; i < text.size(); ++i)
	{
		char c = text[i];
		if (c == 10)
			convText += (char)13;
		convText += c;
	}

    int cch = convText.size();
 
    // Open the clipboard, and empty it. 
    if (!OpenClipboard(NULL)) 
        return; 
    EmptyClipboard(); 

    // Allocate a global memory object for the text. 
    HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 
        (cch + 1) * sizeof(char)); 
    if (hglbCopy == NULL) 
    { 
        CloseClipboard(); 
        return;
    } 

    // Lock the handle and copy the text to the buffer. 
    char* pCopy = (char*)GlobalLock(hglbCopy); 
    memcpy(pCopy, (char*)convText.c_str(), cch); 
    pCopy[cch] = (char)0;    // null character 
    GlobalUnlock(hglbCopy); 

    // Place the handle on the clipboard. 
    SetClipboardData(CF_TEXT, hglbCopy); 

	// Close the clipboard
	CloseClipboard(); 
#else
	return; // this doesn't work yet
	
	const char *textC = text.c_str();
	{ /* Convert the scrap from UNIX text to Mac text */
		char *cp;
		int   i;
		scrapSDL = (char*)realloc(scrapSDL, strlen(textC)+1);
		strcpy(scrapSDL, textC);
		for ( cp=scrapSDL, i=0; i<strlen(scrapSDL); ++cp, ++i ) {
			if ( *cp == '\n' )
				*cp = '\r';
		}
		textC = scrapSDL;
	}
	put_scrap(T('T','E','X','T'), strlen(textC), (char*)textC);
#endif
}

