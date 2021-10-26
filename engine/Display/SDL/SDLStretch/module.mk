# -*- Makefile -*- module for SDLStretch
# $Id: module.mk,v 1.2 2001/05/08 15:49:27 firving Exp $

SDLSTRETCH_SOURCES := engine/Display/SDL/SDLStretch/SDLStretch.cpp

SOURCES += $(SDLSTRETCH_SOURCES)

LIBRARIES += libSDLStretch.so

libSDLStretch.so: $(patsubst %.cpp, %.o, $(SDLSTRETCH_SOURCES))
libSDLStretch.do: $(patsubst %.cpp, %.od, $(SDLSTRETCH_SOURCES))
libSDLStretch.ho: $(patsubst %.cpp, %.oh, $(SDLSTRETCH_SOURCES))

