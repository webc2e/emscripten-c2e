# -*- Makefile -*- module for Brain
# $Id: module.mk,v 1.2 2000/03/14 18:13:55 firving Exp $

_SOURCES := engine/Creature/Brain/Brain.cpp \
	engine/Creature/Brain/BrainComponent.cpp \
	engine/Creature/Brain/BrainScriptFunctions.cpp \
	engine/Creature/Brain/Dendrite.cpp \
	engine/Creature/Brain/Instinct.cpp \
	engine/Creature/Brain/Lobe.cpp \
	engine/Creature/Brain/Neuron.cpp \
	engine/Creature/Brain/SVRule.cpp \
	engine/Creature/Brain/Tract.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libBrain.so

libBrain.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libBrain.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libBrain.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
