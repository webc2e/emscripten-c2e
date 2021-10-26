// --------------------------------------------------------------------------
// Filename:	ErrorDialog.h
// Class:		ErrorDialog
// Purpose:		This provides a general error dialog
//				
//				
//				
//
// Description: Shows the progression of a certain process.
//			
//			
//
// History:
// -------  
// 17Mar98	Alima		Created
//	
// --------------------------------------------------------------------------

#ifndef DX_ERROR_DIALOG_H
#define DX_ERROR_DIALOG_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../../common/C2eTypes.h"
#include <string>

LRESULT CALLBACK ErrorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class ErrorDialog
{
public:

	enum
	{
		ED_CONTINUE,
		ED_QUIT,
		ED_BRUTAL_ABORT,
	};

	ErrorDialog();

	void SetText(const std::string& source, const std::string& text);
	int DisplayDialog(HWND wnd);
	LRESULT ProcessMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// attributes:
public:
	std::string myText;
	std::string mySource;
};


#endif // DX_ERROR_DIALOG_H

