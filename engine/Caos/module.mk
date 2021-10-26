# -*- Makefile -*- module for Caos
# $Id: module.mk,v 1.6 2001/01/31 18:05:09 firving Exp $

_SOURCES := engine/Caos/AgentHandlers.cpp \
	engine/Caos/AutoDocumentationTable.cpp \
	engine/Caos/CAOSDescription.cpp \
	engine/Caos/CAOSException.cpp \
	engine/Caos/CAOSMachine.cpp \
	engine/Caos/CAOSTables.cpp \
	engine/Caos/CAOSVar.cpp \
	engine/Caos/CompoundHandlers.cpp \
	engine/Caos/CreatureHandlers.cpp \
	engine/Caos/DebugHandlers.cpp \
	engine/Caos/DebugInfo.cpp \
	engine/Caos/DisplayHandlers.cpp \
	engine/Caos/GeneralHandlers.cpp \
	engine/Caos/HistoryHandlers.cpp \
	engine/Caos/Lexer.cpp \
	engine/Caos/MacroScript.cpp \
	engine/Caos/MapHandlers.cpp \
	engine/Caos/OpSpec.cpp \
	engine/Caos/Orderiser.cpp \
	engine/Caos/PortHandlers.cpp \
	engine/Caos/Scriptorium.cpp \
	engine/Caos/SoundHandlers.cpp \
	engine/Caos/TableSpec.cpp \
	engine/Caos/VelocityVariable.cpp \
	engine/Caos/SockServer.cpp

ifeq ($(ERROR_OUTPUT),console)
	_SOURCES += engine/Caos/unix/RuntimeErrorDialog.cpp
else
	_SOURCES += engine/Caos/GTK/GTK_RuntimeErrorDialog.cpp
endif

SOURCES += $(_SOURCES)

LIBRARIES += libCaos.so

libCaos.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libCaos.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libCaos.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
