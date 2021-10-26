// --------------------------------------------------------------------------------------
// Filename:	PrayException.h
// Class:		PrayException
// Purpose:		To provide a standard Exception which is thrown by Pray Code.
// Description:
//  The PrayException is thrown by any of the code in the PrayManager or PrayChunk(Ptr)
//  which needs to express a problem. It carries a message in its msg field, and should
//  not be allowed to go outside of code which directly interacts with the Pray Systems.
//
// History:
//  22Jun99	DanS	Initial Version
//
// --------------------------------------------------------------------------------------

#ifndef PRAYEXCEPTION_H
#define PRAYEXCEPTION_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>

class PrayException
{
public:
	// ----------------------------------------------------------------------------------
	// Enumeration
	// Description: This enumeration is a list of the known codes for the exception.
	// ----------------------------------------------------------------------------------
	enum {
		eidChunkNotFound,
		eidFilePulledOut,
		eidFileTooShortInSeek,
		eidFileTooShortForHeader,
		eidFileTooShortForUData,
		eidFileTooShortForCData,
		eidDataMismatch,
		eidChunkExists,
		eidFileTooShort,
		eidNotPrayFile,
		eidCompressError,
		eidFileNotOpen,
		eidSizeMismatch,
	};

	// ----------------------------------------------------------------------------------
	// Method:		GetMessage
	// Arguments:	(None)
	// Returns:		std::string - the message
	// Description:	This retrieves the message stored in the exception.
	// ----------------------------------------------------------------------------------
	std::string GetMessage() { return myMessage; }

	// ----------------------------------------------------------------------------------
	// Method:		GetCode
	// Arguments:	(None)
	// Returns:		int - the message location code
	// Description:	This retrieves the message code stored in the exception.
	// ----------------------------------------------------------------------------------
	int GetCode() { return myCode; }

	// ----------------------------------------------------------------------------------
	// Method:		GetCode
	// Arguments:	(None)
	// Returns:		std::string - the filename
	// Description:	This retreives the name of the file in which the
	// error happend, or an empty string if not available.
	// ----------------------------------------------------------------------------------
	std::string GetPrayFile() { return myPrayFile; }

	// ----------------------------------------------------------------------------------
	// Constructor
	// Arguments:	msg - std::string - The message to store
	//				code - int        - The code to indicate where the error occured.
	// Returns:		(None)
	// Description:	Creates the Exception. The code should indicate where the error was.
	// ----------------------------------------------------------------------------------
	PrayException(std::string msg, int code, std::string prayFile)
		: myMessage(msg), myCode(code), myPrayFile(prayFile) { }

private:
	std::string myMessage;
	int myCode;
	std::string myPrayFile; // file that the error happened in, if available
};

#endif //PRAYEXCEPTION_H

