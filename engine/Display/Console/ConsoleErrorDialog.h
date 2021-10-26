// --------------------------------------------------------------------------
// Filename:	GTK_ErrorDialog.h
// Class:		ErrorDialog
// Purpose:		This provides a general error dialog
//
// History:
// --------------------------------------------------------------------------

#ifndef GTK_ERROR_DIALOG_H
#define GTK_ERROR_DIALOG_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>

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
	int DisplayDialog();

// attributes:
public:
	std::string myText;
	std::string mySource;
	
	static void DisableContinueButtonNextDialog();

private:
	int myReturnCode;
	static bool ourDisableContinueButton;
};


#endif // GTK_ERROR_DIALOG_H

