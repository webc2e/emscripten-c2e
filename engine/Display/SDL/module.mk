# -*- Makefile -*- module for SDL
# $Id: module.mk,v 1.10 2001/05/02 17:52:16 firving Exp $

_SOURCES := engine/Display/SDL/SDL_DisplayEngine.cpp \
			engine/Display/SDL/scrap.cpp

# now using gtk version
#	engine/Display/SDL/SDL_ErrorDialog.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libSDLc2e.so

libSDLc2e.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libSDLc2e.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libSDLc2e.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))


# should be moved up a few directories I think (BenC)...
engine/Display/SDL/SDL_Main.od: engine/Display/SDL/SDL_Main.cpp
engine/Display/SDL/SDL_Main.oh: engine/Display/SDL/SDL_Main.cpp
engine/Display/SDL/SDL_Main.o: engine/Display/SDL/SDL_Main.cpp

include engine/Display/SDL/SDLStretch/module.mk
