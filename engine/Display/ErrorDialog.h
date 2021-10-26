#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H


#ifdef C2E_USING_GTK
	#include "GTK/GTK_ErrorDialog.h"
#else
	#ifdef _WIN32
		#include "DirectX/ErrorDialog.h"
	#else
		#include "Console/ConsoleErrorDialog.h"
	#endif
#endif


#endif // ERRORDIALOG_H

