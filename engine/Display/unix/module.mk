# -*- Makefile -*- module for UnixDisplay
# $Id: module.mk,v 1.2 2000/03/14 17:58:07 firving Exp $

_SOURCES := engine/Display/unix/MemoryMappedFile.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libUnixDisplay.so

libUnixDisplay.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libUnixDisplay.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libUnixDisplay.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
