# -*- Makefile -*- module for Agents
# $Id: module.mk,v 1.3 2000/03/14 18:13:55 firving Exp $

_SOURCES := engine/Agents/Agent.cpp \
	engine/Agents/AgentConstants.cpp \
	engine/Agents/AgentHandle.cpp \
	engine/Agents/CameraPart.cpp \
	engine/Agents/CompoundAgent.cpp \
	engine/Agents/CompoundPart.cpp \
	engine/Agents/InputPort.cpp \
	engine/Agents/MessageQueue.cpp \
	engine/Agents/OutputPort.cpp \
	engine/Agents/PointerAgent.cpp \
	engine/Agents/Port.cpp \
	engine/Agents/PortBundle.cpp \
	engine/Agents/QuoteFactory.cpp \
	engine/Agents/SimpleAgent.cpp \
	engine/Agents/UIPart.cpp \
	engine/Agents/Vehicle.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libAgents.so

libAgents.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libAgents.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libAgents.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
