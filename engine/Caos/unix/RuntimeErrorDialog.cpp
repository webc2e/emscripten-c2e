
#ifdef _WIN32
#error Use the Win32 version instead
#endif

// There's now a GTK error dialog, as well as this
// basic replacement which just uses the console

#include "../RuntimeErrorDialog.h"
#include "../../Camera/MainCamera.h"
#include <sstream>


RuntimeErrorDialog::RuntimeErrorDialog()
{
	myCameraChecked = false;
	myCameraPossible = true;
}

int RuntimeErrorDialog::DisplayDialog()
{
	std::cout << theCatalogue.Get("system_title", 0) << std::endl;
	std::cout << myText << std::endl;
	std::cout << "Action: Ignore" << std::endl; 
	return RED_IGNORE;
}

void RuntimeErrorDialog::SetText(const std::string& text)
{
#ifdef _WIN32
	// DOS has soooooo much to answer for...
	std::string filter;
	for (int i = 0; i < text.size(); ++i)
	{
		char c = text[i];
		if (c == '\n')
			filter += "\r\n";
		else
			filter += c;
	}

	myText = filter;
#else
	myText = text;
#endif
}

void RuntimeErrorDialog::SetCameraPossible(bool camera)
{
	myCameraPossible = camera;
}


