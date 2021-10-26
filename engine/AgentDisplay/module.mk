# -*- Makefile -*- module for AgentDisplay
# $Id: module.mk,v 1.2 2000/03/14 17:58:06 firving Exp $

_SOURCES :=  engine/AgentDisplay/EntityImage.cpp \
	engine/AgentDisplay/EntityImageClone.cpp \
	engine/AgentDisplay/EntityImageWithEmbeddedCamera.cpp

SOURCES += $(_SOURCES)

LIBRARIES += libAgentDisplay.so

libAgentDisplay.so: $(patsubst %.cpp, %.o, $(_SOURCES))
libAgentDisplay.do: $(patsubst %.cpp, %.od, $(_SOURCES))
libAgentDisplay.ho: $(patsubst %.cpp, %.oh, $(_SOURCES))
