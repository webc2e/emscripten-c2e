# -*- Makefile -*- module for Creature
# $Id: module.mk,v 1.4 2000/03/14 17:58:06 firving Exp $

_SOURCES := engine/Creature/BodyPartOverlay.cpp \
	engine/Creature/Creature.cpp \
	engine/Creature/CreatureFacultyInterface.cpp \
	engine/Creature/AgentFacultyInterface.cpp \
	engine/Creature/CreatureAgent.cpp \
	engine/Creature/CreatureHead.cpp \
	engine/Creature/ExpressiveFaculty.cpp \
	engine/Creature/Faculty.cpp \
	engine/Creature/Genome.cpp \
	engine/Creature/GenomeStore.cpp \
	engine/Creature/LifeFaculty.cpp \
	engine/Creature/LinguisticFaculty.cpp \
	engine/Creature/MotorFaculty.cpp \
	engine/Creature/MusicFaculty.cpp \
	engine/Creature/ReproductiveFaculty.cpp \
	engine/Creature/SensoryFaculty.cpp \
	engine/Creature/SkeletalCreature.cpp \
	engine/Creature/Vocab.cpp \
	engine/Creature/voice.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libCreature.so

libCreature.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libCreature.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libCreature.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))

include engine/Creature/Brain/module.mk
include engine/Creature/Biochemistry/module.mk
include engine/Creature/History/module.mk
