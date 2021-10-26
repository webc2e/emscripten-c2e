# -*- Makefile -*- module for PRAY
# $Id: module.mk,v 1.2 2000/03/14 17:58:06 firving Exp $

_SOURCES := \
	common/PRAYFiles/PrayBuilder.cpp \
	common/PRAYFiles/PrayChunk.cpp \
	common/PRAYFiles/PrayHandlers.cpp \
	common/PRAYFiles/PrayManager.cpp \
	common/PRAYFiles/StringIntGroup.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libPRAY.so

libPRAY.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libPRAY.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libPRAY.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
