# -*- Makefile -*- module for History
# $Id: module.mk,v 1.2 2000/03/14 18:13:56 firving Exp $

_SOURCES := engine/Creature/History/CreatureHistory.cpp \
	engine/Creature/History/HistoryStore.cpp \
	engine/Creature/History/LifeEvent.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libHistory.so

libHistory.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libHistory.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libHistory.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
