# -*- Makefile -*- module for Common
# $Id: module.mk,v 1.9 2000/11/23 13:31:02 firving Exp $

_SOURCES := \
	common/BasicException.cpp \
	common/C2eDebug.cpp \
	common/Catalogue.cpp \
	common/FileLocaliser.cpp \
	common/Position.cpp \
	common/SimpleLexer.cpp \
	common/Vector2D.cpp \
	common/Configurator.cpp \
	common/StringFuncs.cpp \
	common/FileFuncs.cpp \
	common/SimpleMutex.cpp \
	common/UnixSignals.cpp \
	common/ProfanityFilter.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libCommon.so

libCommon.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libCommon.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libCommon.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))

include common/PRAYFiles/module.mk
