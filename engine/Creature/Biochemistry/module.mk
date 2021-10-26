# -*- Makefile -*- module for Biochemistry
# $Id: module.mk,v 1.2 2000/03/14 18:13:55 firving Exp $

_SOURCES := engine/Creature/Biochemistry/Biochemistry.cpp \
	engine/Creature/Biochemistry/BiochemistryConstants.cpp \
	engine/Creature/Biochemistry/Chemical.cpp \
	engine/Creature/Biochemistry/Emitter.cpp \
	engine/Creature/Biochemistry/NeuroEmitter.cpp \
	engine/Creature/Biochemistry/Organ.cpp \
	engine/Creature/Biochemistry/Receptor.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libBiochemistry.so

libBiochemistry.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libBiochemistry.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libBiochemistry.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
