# -*- Makefile -*- module for Unix
# $Id: module.mk,v 1.2 2000/03/14 17:58:07 firving Exp $

_SOURCES := engine/unix/File.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libUnix.so

libUnix.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libUnix.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libUnix.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
