cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(caos)

add_library(
  caos STATIC 
  AgentHandlers.cpp
	AutoDocumentationTable.cpp
	CAOSDescription.cpp
	CAOSException.cpp
	CAOSMachine.cpp
	CAOSTables.cpp
	CAOSVar.cpp
	CompoundHandlers.cpp
	CreatureHandlers.cpp
	DebugHandlers.cpp
	DebugInfo.cpp
	DisplayHandlers.cpp
	GeneralHandlers.cpp
	HistoryHandlers.cpp
	Lexer.cpp
	MacroScript.cpp
	MapHandlers.cpp
	OpSpec.cpp
	Orderiser.cpp
	PortHandlers.cpp
	Scriptorium.cpp
	SoundHandlers.cpp
	TableSpec.cpp
	VelocityVariable.cpp
	SockServer.cpp
	unix/RuntimeErrorDialog.cpp
)
