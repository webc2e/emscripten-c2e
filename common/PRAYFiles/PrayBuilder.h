// This is the class to manage building a pray file - basically it constructs on a few filenames
// and then it's done - it's plainly an encapsulation. You can then interrogate the object
// to determine the return success etc.

// This class will be in the __new__ defined Coding Style when I can get around to it.
// (Dan)


#ifndef PRAYBUILDER_H
#define PRAYBUILDER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "../../modules/ModuleAPI.h"

const bool CompressionFlag = true;

class SimpleLexer;

class C2E_MODULE_API PrayBuilder
{
public:
	PrayBuilder(std::string sourceFile, std::string destFile);
	bool Successful();
	std::string Output();

private:
	void decodeType(int thisType,std::string& into);
	bool expect(int thisType, std::string& thisToken, int expectedType, int linenumber);
	void munchComment(int& currentTokenType, std::string& currentToken, SimpleLexer& inputLexer);

#ifdef _MSC_VER
#pragma warning (disable : 4251)
#endif
	std::ostringstream myOutputStream;
	bool mySuccessfulBuild;
	std::string myOutput;
#ifdef _MSC_VER
#pragma warning (default : 4251)
#endif
};

#endif // PRAYBUILDER_H

