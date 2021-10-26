# -*- Makefile -*- module for Camera
# $Id: module.mk,v 1.3 2000/04/04 18:01:52 firving Exp $

_SOURCES := engine/Camera/MapImage.cpp \
	engine/Camera/Camera.cpp \
	engine/Camera/RemoteCamera.cpp \
	engine/Camera/MainCamera.cpp \
	engine/Camera/CameraSprite.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libCamera.so

libCamera.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libCamera.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libCamera.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
