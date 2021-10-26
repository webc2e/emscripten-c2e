// --------------------------------------------------------------------------
// Filename:	RuntimeErrorDialog.h
// Class:		RuntimeErrorDialog
// Purpose:		This provides a general progress dialog
//
// Description: Shows the progression of a certain process.
//
// --------------------------------------------------------------------------

#ifndef _RUNTIME_ERROR_DIALOG_H
#define _RUNTIME_ERROR_DIALOG_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include <string>

#ifdef _WIN32
LRESULT CALLBACK RuntimeErrorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef C2E_USING_GTK
#include <gtk/gtk.h>
#endif


class RuntimeErrorDialog
{
public:

	enum
	{
		RED_RUNNING,
		RED_IGNORE,
		RED_FREEZE,
		RED_KILL,
		RED_PAUSEGAME,
		RED_STOPSCRIPT
	};

	RuntimeErrorDialog();

	void SetText(const std::string& text);
	void SetCameraPossible(bool camera);
	int DisplayDialog();
#ifdef _WIN32
	LRESULT ProcessMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

// attributes:
public:
	std::string myText;
	bool myCameraChecked;
	bool myCameraPossible;

#ifdef C2E_USING_GTK
	static int callback_delete_event( GtkWidget* w, GdkEvent* e,
	    gpointer dat );

	static void callback_destroy( GtkWidget* w, gpointer dat );
	static void callback_IgnoreClicked( GtkWidget* w, gpointer dat );
	static void callback_FreezeClicked( GtkWidget* w, gpointer dat );
	static void callback_KillClicked( GtkWidget* w, gpointer dat );
	static void callback_PauseGameClicked( GtkWidget* w, gpointer dat );
	static void callback_StopScriptClicked( GtkWidget* w, gpointer dat );
	static void callback_MoveCameraToggled( GtkWidget* w, gpointer dat );

	int myReturnCode;
#endif

};


#endif	// _RUNTIME_ERROR_DIALOG_H

