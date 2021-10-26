#ifndef AGENT_HANDLE_H
#define AGENT_HANDLE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/BasicException.h"
#include "../../modules/ModuleAPI.h"

// Forward references
class Agent;
	class AgentFacultyInterface; // implemented by agent
class SimpleAgent;
class PointerAgent;
class CompoundAgent;
class Vehicle;
class Creature;
	class CreatureFacultyInterface; // implemented by creature
class SkeletalCreature;
class CreaturesArchive;

class C2E_MODULE_API AgentHandle {
public:	

	enum {
		agentNormal = 1,
		agentSimple = 2,
		agentPointer = 4,
		agentCompound = 8,
		agentVehicle = 16,
		agentCreature = 32,
		agentSkeletalCreature = 64,
	};
	// Default constructor
	AgentHandle();
	// Constructor
	AgentHandle(Agent* agent);

	// Constructor added by Gavin
	AgentHandle(Agent& agent);

	// Copy constructor
	AgentHandle(const AgentHandle& handle);
	
	// Destructor
	~AgentHandle();


	//
	// Overloaded operators
	// 

	// Assignment
	AgentHandle& operator=(const AgentHandle& handle);

	// Equality
	bool operator==(const AgentHandle& handle) const;

	// Inequality
	bool operator!=(const AgentHandle& handle) const; 

	// Arbitary comparison - for containers
	bool operator<(const AgentHandle& handle) const; 

private:
	// These functions are declared but not defined so that the
	// compiler can catch comparisons and assignments to a raw
	// agent pointer
	bool operator!=(const Agent* agent) const;
	bool operator==(const Agent* agent) const;
	AgentHandle& operator=(const Agent* agent);
public:

	//
	// Check functions
	//

	bool IsValid();
	bool IsInvalid();
	bool IsSimpleAgent();
	bool IsPointerAgent();
	bool IsCompoundAgent();
	bool IsVehicle();
	bool IsCreature();
	bool IsSkeletalCreature();
	

	// 
	// Accessor functions
	//
	 
	Agent& GetAgentReference() const;
	AgentFacultyInterface& GetAgentFacultyInterfaceReference() const;
	SimpleAgent& GetSimpleAgentReference() const;
	PointerAgent& GetPointerAgentReference() const;
	CompoundAgent& GetCompoundAgentReference() const;
	Vehicle& GetVehicleReference() const;
	Creature& GetCreatureReference() const;
	CreatureFacultyInterface& GetCreatureFacultyInterfaceReference() const;
	SkeletalCreature& GetSkeletalCreatureReference() const;
	
private:
	Agent* myAgentPointer;
	friend C2E_MODULE_API CreaturesArchive& operator<<( CreaturesArchive &ar, AgentHandle const &agentHandle );
	friend C2E_MODULE_API CreaturesArchive& operator>>( CreaturesArchive &ar, AgentHandle &agentHandle );
};

C2E_MODULE_API CreaturesArchive& operator<<( CreaturesArchive &ar, AgentHandle const &agentHandle );
C2E_MODULE_API CreaturesArchive& operator>>( CreaturesArchive &ar, AgentHandle &agentHandle );


class AgentHandleException : public BasicException
{
public:
	AgentHandleException(char* message) : BasicException(message) { }
};

// Global
extern C2E_MODULE_API AgentHandle NULLHANDLE;



#endif // AGENT_HANDLE_H

