// Faculty.cpp: implementation of the Faculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Faculty.h"
#include "AgentFacultyInterface.h"
#include "CreatureFacultyInterface.h"
#include "../Agents/AgentHandle.h"


CREATURES_IMPLEMENT_SERIAL(Faculty)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Faculty::Faculty()
{
	myActiveFlag = true;
}

Faculty::~Faculty()
{

}




void Faculty::Init() {}
void Faculty::Update() {}
void Faculty::ReadFromGenome(Genome& g) {}




void Faculty::SetCallback(AgentFacultyInterface* a, CreatureFacultyInterface* f)
{
	myCreatureAsAgent = a;
	myCreature = f;
}



float* Faculty::GetLocusAddress(int type, int organ, int tissue, int locus)
{
	return NULL;
}




bool Faculty::Write(CreaturesArchive &archive) const {

	archive << myActiveFlag;

	return true;
}

bool Faculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		// before version 15 this was saved out, now it's reset
		// automatically:
		if (version < 15)
		{
			AgentHandle c;
			archive >> c;
			myCreature = &(c.GetCreatureFacultyInterfaceReference());
		}
		if (version >= 39)
		{
			archive >> myActiveFlag;
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}




void Faculty::SetActive(bool b)
{
	myActiveFlag = b;
}

bool Faculty::IsActive()
{
	return myActiveFlag;
}



