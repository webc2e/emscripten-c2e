# -*- Makefile -*- module for Display
# $Id: module.mk,v 1.11 2001/01/31 18:05:10 firving Exp $

_SOURCES := engine/Display/Background.cpp \
	engine/Display/Bitmap.cpp \
	engine/Display/ClonedGallery.cpp \
	engine/Display/CompressedBitmap.cpp \
	engine/Display/CreatureGallery.cpp \
	engine/Display/DrawableObject.cpp \
	engine/Display/DrawableObjectHandler.cpp \
	engine/Display/ErrorMessageHandler.cpp \
	engine/Display/Gallery.cpp \
	engine/Display/Line.cpp \
	engine/Display/SharedGallery.cpp \
	engine/Display/Sprite.cpp \
	engine/Display/System.cpp \
	engine/Display/TintManager.cpp \
	engine/Display/EasterEgg.cpp \
	engine/Display/DisplayEnginePlotFunctions.cpp

ifeq ($(ERROR_OUTPUT),console)
	_SOURCES += engine/Display/Console/ConsoleErrorDialog.cpp
else
	_SOURCES += engine/Display/GTK/GTK_ErrorDialog.cpp
endif

SOURCES += $(_SOURCES)

LIBRARIES += libDisplay.so

libDisplay.so: $(patsubst %.cpp, %.o, $(_SOURCES)) engine/Display/Plot.o
libDisplay.do: $(patsubst %.cpp, %.od, $(_SOURCES)) engine/Display/Plot.o
libDisplay.ho: $(patsubst %.cpp, %.oh, $(_SOURCES)) engine/Display/Plot.o

include engine/Display/SDL/module.mk
include engine/Display/unix/module.mk

