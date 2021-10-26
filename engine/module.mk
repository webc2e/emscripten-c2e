# -*- Makefile -*- module for Engine
# $Id: module.mk,v 1.11 2001/01/31 18:05:09 firving Exp $

_SOURCES := engine/AgentManager.cpp \
	engine/App.cpp \
	engine/C2eServices.cpp \
	engine/Classifier.cpp \
	engine/CosInstaller.cpp \
	engine/CreaturesArchive.cpp \
	engine/CustomHeap.cpp \
	engine/Entity.cpp \
	engine/FilePath.cpp \
	engine/FlightRecorder.cpp \
	engine/General.cpp \
	engine/InputManager.cpp \
	engine/Maths.cpp \
	engine/Message.cpp \
	engine/PersistentObject.cpp \
	engine/Scramble.cpp \
	engine/Stimulus.cpp \
	engine/World.cpp \
	engine/md5.cpp \
	engine/mfchack.cpp \
	engine/TimeFuncs.cpp \
	engine/ModuleImporter.cpp \
	engine/DirectoryManager.cpp \
	engine/Presence.cpp \
	engine/Angle.cpp \
	engine/Circle.cpp \
	engine/UniqueIdentifier.cpp \
	engine/QuickFsp.cpp
	
SOURCES += $(_SOURCES)

include engine/Creature/module.mk
include engine/Agents/module.mk
include engine/Caos/module.mk
include engine/Display/module.mk
include engine/Map/module.mk
include engine/Sound/module.mk
include engine/unix/module.mk
include engine/AgentDisplay/module.mk
include engine/Camera/module.mk
