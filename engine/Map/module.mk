# -*- Makefile -*- module for Map
# $Id: module.mk,v 1.2 2000/03/14 17:58:07 firving Exp $

_SOURCES := engine/Map/Map.cpp \
	engine/Map/CARates.cpp \
	engine/Map/MapCA.cpp \
	engine/Map/RoomCA.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libMap.so

libMap.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libMap.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libMap.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
