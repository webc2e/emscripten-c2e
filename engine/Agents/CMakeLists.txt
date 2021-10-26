cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(agents)
  
add_library(
  agents STATIC 
  Agent.cpp
	AgentConstants.cpp
	AgentHandle.cpp
	CameraPart.cpp
	CompoundAgent.cpp
	CompoundPart.cpp
	InputPort.cpp
	MessageQueue.cpp
	OutputPort.cpp
	PointerAgent.cpp
	Port.cpp
	PortBundle.cpp
	QuoteFactory.cpp
	SimpleAgent.cpp
	UIPart.cpp
	Vehicle.cpp
)
