# -*- Makefile -*- module for Sound
# $Id: module.mk,v 1.4 2000/07/05 14:24:36 firving Exp $

_SOURCES := engine/Sound/SDL/SDL_Soundlib.cpp \
	engine/Sound/stub/stub_MidiModule.cpp \
	engine/Sound/stub/stub_CDRomDevice.cpp \
	engine/Sound/MusicAction.cpp \
	engine/Sound/MusicAleotoricLayer.cpp \
	engine/Sound/MusicEffect.cpp \
	engine/Sound/MusicExpression.cpp \
	engine/Sound/MusicGlobals.cpp \
	engine/Sound/MusicLayer.cpp \
	engine/Sound/MusicLoopLayer.cpp \
	engine/Sound/MusicManager.cpp \
	engine/Sound/MusicScript.cpp \
	engine/Sound/MusicTimer.cpp \
	engine/Sound/MusicTrack.cpp \
	engine/Sound/MusicUpdatable.cpp \
	engine/Sound/MusicVariable.cpp \
	engine/Sound/MusicVariableContainer.cpp

#	engine/Sound/stub/stub_Soundlib.cpp	\

SOURCES += $(_SOURCES)

LIBRARIES += libSound.so

libSound.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libSound.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libSound.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))

