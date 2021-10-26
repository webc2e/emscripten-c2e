// Smart agent pointers (handles) for efficient garbage collection
// Authors: Robert Dickson, Dan Silverstone


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "AgentHandle.h"
#include "Agent.h"

#include "../Creature/AgentFacultyInterface.h"
#include "../Creature/CreatureFacultyInterface.h"

C2E_MODULE_API AgentHandle NULLHANDLE;

// Default constructor
AgentHandle::AgentHandle() 
{
	myAgentPointer = NULL;
}


// Constructor
AgentHandle::AgentHandle(Agent* agent) 
{
	myAgentPointer = agent;
	if (agent) 
	{	
		/* Test code
		unsigned magic;
		try 
		{
			magic = agent->GetMagicNumber();
		}
		catch (...)
		{
			throw AgentHandleException("AHE0022: Attempt to construct a handle with an invalid pointer");
		}

		if (magic != 3141592654)
			throw AgentHandleException("AHE0023: Attempt to construct a handle with an invalid pointer");

		if (agent->IsGarbage())
			throw AgentHandleException("AHE0024: Attempt to construct a handle on a garbaged agent");
		*/
		agent->IncrementReferenceCount();
	}
}


AgentHandle::AgentHandle(Agent& agent) 
{
	myAgentPointer = &agent;
	/* Test code
	unsigned magic;
	try 
	{
		magic = myAgentPointer->GetMagicNumber();
	}
	catch (...)
	{
		throw AgentHandleException("AHE0025: Attempt to construct a handle with an invalid reference");
	}

	if (magic != 3141592654)
		throw AgentHandleException("AHE0026: Attempt to construct a handle with an invalid reference");
	*/

	// Treat handles on garbaged agents as null handles
	if (agent.IsGarbage()) 
	{
		myAgentPointer = NULL;
		return;
	} 
	else 
	{
		agent.IncrementReferenceCount();
	}
}

// Copy constructor
AgentHandle::AgentHandle(const AgentHandle& handle)
{
	myAgentPointer = handle.myAgentPointer;
	if (myAgentPointer) 
	{
		if (myAgentPointer->IsGarbage())
		{
			// Treat handles on garbaged agents as null handles
			myAgentPointer = NULL;
			return;
		}
		myAgentPointer->IncrementReferenceCount();
	}
}
// Destructor
AgentHandle::~AgentHandle() 
{
	if (myAgentPointer) 
	{
		int count = myAgentPointer->DecrementReferenceCount();
		if (count == 0) 
		{
			delete myAgentPointer;
		}
	}
}


CreaturesArchive& operator<<( CreaturesArchive &ar, AgentHandle const &agentHandle )
{
	// If the agent pointer is null OR the pointed to agent is garbage serialize a null agent
	
	if( const_cast< AgentHandle &>(agentHandle).IsValid() )
		ar << agentHandle.myAgentPointer;
	else
		ar << (Agent*)(NULL);
	return ar;
}

CreaturesArchive& operator>>( CreaturesArchive &ar, AgentHandle &agentHandle )
{
	Agent *agent;
	ar >> agent;
	if (agent)
		_ASSERT(!(agent->IsGarbage()));
	agentHandle = AgentHandle( agent );
	return ar;
}
//
// Overloaded operators
// 

// Assignment
AgentHandle& AgentHandle::operator=(const AgentHandle& handle)
{
	// Tell the source agent that someone new is now looking at it
	if (handle.myAgentPointer && !handle.myAgentPointer->IsGarbage()) 
	{
		handle.myAgentPointer->IncrementReferenceCount();
	}

	if (myAgentPointer) 
	{
		// Break my current connection
		int count = myAgentPointer->DecrementReferenceCount();
		// If I was last connection, clean up
		if (count == 0)
		{
			delete myAgentPointer;
			myAgentPointer = NULL;
		}
	}

	// Do the actual assignment of the raw pointers
	if (handle.myAgentPointer && !handle.myAgentPointer->IsGarbage())
		myAgentPointer = handle.myAgentPointer;
	else
		myAgentPointer = NULL;
	return *this;
}

// Equality
bool AgentHandle::operator==(const AgentHandle& handle) const 
{
	return (myAgentPointer == handle.myAgentPointer);
}
// Inequality
bool AgentHandle::operator!=(const AgentHandle& handle) const 
{
	return (myAgentPointer != handle.myAgentPointer);
}

// Arbitary comparison - for containers
bool AgentHandle::operator<(const AgentHandle& handle) const
{
	return (myAgentPointer < handle.myAgentPointer);
}


Agent& AgentHandle::GetAgentReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0001: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0002: Attempt to access garbaged handle");
	if (!(myAgentPointer->GetAgentType() & agentNormal))
		throw AgentHandleException("AHE0003: Attempt to access incorrect type (raw agent expected)");
	return *((Agent *)(myAgentPointer));
}

SimpleAgent& AgentHandle::GetSimpleAgentReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0004: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0005: Attempt to access garbaged handle");
	if (!(myAgentPointer->GetAgentType() & agentSimple))
		throw AgentHandleException("AHE0006: Attempt to access incorrect type (simple agent expected)");
	return *((SimpleAgent *)(myAgentPointer));
}

PointerAgent& AgentHandle::GetPointerAgentReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0007: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0008: Attempt to access garbaged handle");
	if ( !(myAgentPointer->GetAgentType() & agentPointer) )
		throw AgentHandleException("AHE0009: Attempt to access incorrect type (pointer agent expected)");
	return *((PointerAgent *)(myAgentPointer));
}

CompoundAgent& AgentHandle::GetCompoundAgentReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0010: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0011: Attempt to access garbaged handle");
	if (!(myAgentPointer->GetAgentType() & agentCompound))
		throw AgentHandleException("AHE0012: Attempt to access incorrect type (compound agent expected)");
	return *((CompoundAgent *)(myAgentPointer));
}


Vehicle& AgentHandle::GetVehicleReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0013: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0014: Attempt to access garbaged handle");
	if (!(myAgentPointer->GetAgentType() & agentVehicle))
		throw AgentHandleException("AHE0015: Attempt to access incorrect type (vehicle expected)");
	return *((Vehicle *)(myAgentPointer));
}


Creature& AgentHandle::GetCreatureReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0019: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0020: Attempt to access garbaged handle");
	if ( !(myAgentPointer->GetAgentType() & agentCreature) )
		throw AgentHandleException("AHE0021: Attempt to access incorrect type (creature expected)");

	return *((Creature *)(myAgentPointer));
}

SkeletalCreature& AgentHandle::GetSkeletalCreatureReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0019: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0020: Attempt to access garbaged handle");
	if ( !(myAgentPointer->GetAgentType() & agentSkeletalCreature) )
		throw AgentHandleException("AHE0021: Attempt to access incorrect type (creature expected)");

	return *((SkeletalCreature *)(myAgentPointer));
}






// dynamic cast ones:

AgentFacultyInterface& AgentHandle::GetAgentFacultyInterfaceReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0001: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0002: Attempt to access garbaged handle");
	if (!(myAgentPointer->GetAgentType() & agentNormal))
		throw AgentHandleException("AHE0003: Attempt to access incorrect type (raw agent expected)");
	return *(dynamic_cast<AgentFacultyInterface*>(myAgentPointer));
}

CreatureFacultyInterface& AgentHandle::GetCreatureFacultyInterfaceReference() const 
{
	if (myAgentPointer == NULL)
		throw AgentHandleException("AHE0019: Attempt to access NULL handle");
	if (myAgentPointer->IsGarbage())
		throw AgentHandleException("AHE0020: Attempt to access garbaged handle");
	if ( !(myAgentPointer->GetAgentType() & agentCreature) )
		throw AgentHandleException("AHE0021: Attempt to access incorrect type (creature expected)");

	return *(dynamic_cast<CreatureFacultyInterface*>(myAgentPointer));
}




bool AgentHandle::IsValid()
{
	if (myAgentPointer == NULL)
		return false;
	if (myAgentPointer->IsGarbage())
	{
		int count = myAgentPointer->DecrementReferenceCount();
		// If I was last connection, clean up
		if (count == 0) 
		{
			delete myAgentPointer;
		}
		myAgentPointer = NULL;
		return false;
	}
	return true;
}


bool AgentHandle::IsInvalid()
{
	if (myAgentPointer == NULL)
		return true;
	if (myAgentPointer->IsGarbage())
	{
		int count = myAgentPointer->DecrementReferenceCount();
		// If I was last connection, clean up
		if (count == 0) 
		{
			delete myAgentPointer;
		}
		myAgentPointer = NULL;
		return true;
	}
	return false;
}


bool AgentHandle::IsSimpleAgent()
{
	if (IsInvalid())
		return false;
	return ( (myAgentPointer->GetAgentType() & agentSimple) == agentSimple );
}

bool AgentHandle::IsPointerAgent()
{
	if (IsInvalid())
		return false;
	return ( (myAgentPointer->GetAgentType() & agentPointer) == agentPointer );
}

bool AgentHandle::IsCompoundAgent()
{
	if (IsInvalid())
		return false;
	return ( (myAgentPointer->GetAgentType() & agentCompound) == agentCompound );
}

bool AgentHandle::IsVehicle() 
{
	if (IsInvalid())
		return false;
	return ( (myAgentPointer->GetAgentType() & agentVehicle) == agentVehicle );
}

bool AgentHandle::IsCreature()
{
	if (IsInvalid())
		return false;
	return ( (myAgentPointer->GetAgentType() & agentCreature) == agentCreature );
}

bool AgentHandle::IsSkeletalCreature()
{
	if (IsInvalid())
		return false;
	return ( (myAgentPointer->GetAgentType() & agentSkeletalCreature) == agentSkeletalCreature );
}

