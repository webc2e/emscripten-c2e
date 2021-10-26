// -------------------------------------------------------------------------
// Filename:    OpSpec.cpp
// Class:       OpSpec
// Purpose:     Data structure for describing a CAOS op and argumentlist.
// Description:
//
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "OpSpec.h"
#include "../CreaturesArchive.h"
#include "CAOSConstants.h"
#include <string>
#include "../../common/C2eDebug.h"

// Only things needed or useful to CAOS tool are serialised out
// Try not to change this, as it means you need a new CAOS tool
bool OpSpec::Write(CreaturesArchive &archive) const
{
	archive << myOpcode << myName << myParameters << mySpecialCode;
	archive << mySubCommands;
	archive << myHelpParameters << myHelpCategory << myHelpGeneral;
	archive << myCommandTable;

	return true;
}

bool OpSpec::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		archive >> myOpcode >> myName >> myParameters >> mySpecialCode;
		archive >> mySubCommands;
		archive >> myHelpParameters >> myHelpCategory >> myHelpGeneral;
		archive >> myCommandTable;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

std::string OpSpec::GetPrettyName() const
{
	if (IsSubCommand())
		return mySuperName + " " + myName;
	else
		return myName;
}

// static
std::string OpSpec::GetTableType(int table)
{
	std::string type_string;
	switch(table)
	{
		case idCommandTable: type_string = "command";  break;
		case idIntegerRVTable: type_string = "integer"; break;
		case idVariableTable: type_string = "variable"; break;
		case idFloatRVTable: type_string = "float"; break;
		case idStringRVTable: type_string = "string"; break;
		case idAgentRVTable: type_string = "agent"; break;
		default : type_string = "unknown";
	}
	return type_string;
}


std::string OpSpec::GetPrettyType() const
{
	if (IsSubCommand())
	{
		ASSERT(mySuperCommandTable >= 0);
		return GetTableType(mySuperCommandTable);
	}
	else
		return GetTableType(myCommandTable);

}

bool OpSpec::IsSubCommand() const
{
	return myCommandTable >= FIRST_SUB_COMMAND_TABLE;
}

// static
bool OpSpec::CompareAlphabetic(const OpSpec& a, const OpSpec& b)
{
	if (a.GetPrettyName() < b.GetPrettyName())
		return true;

	if (a.GetPrettyName() == b.GetPrettyName())
	{
		return CompareCommandsFirst(a, b);
	}
	
	return false;
}

// static
bool OpSpec::CompareTypeFirst(const OpSpec& a, const OpSpec& b)
{
	// by category
	if ((int)a.myHelpCategory < (int)b.myHelpCategory)
		return true;
	
	if ((int)a.myHelpCategory == (int)b.myHelpCategory)
	{
		return CompareAlphabetic(a, b);
	}

	return false;
}

// static
bool OpSpec::CompareCommandsFirst(const OpSpec& a, const OpSpec& b)
{
	int a_cmd = a.myCommandTable;
	int b_cmd = b.myCommandTable;
	if (a.IsSubCommand()) // subcommands count as commands for sorting
		a_cmd = idCommandTable;
	if (b.IsSubCommand())
		b_cmd = idCommandTable;
	if (a_cmd < b_cmd)
		return true;

	return false;
}

void OpSpec::SetSuperCommand(const OpSpec& superSpec)
{
	mySuperName = superSpec.myName;
	mySuperCommandTable = superSpec.myCommandTable;
}

