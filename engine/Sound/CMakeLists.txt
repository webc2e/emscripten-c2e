cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(sound)

add_library(
  sound STATIC 
  SDL/SDL_Soundlib.cpp
	stub/stub_MidiModule.cpp
	stub/stub_CDRomDevice.cpp
	MusicAction.cpp
	MusicAleotoricLayer.cpp
	MusicEffect.cpp
	MusicExpression.cpp
	MusicGlobals.cpp
	MusicLayer.cpp
	MusicLoopLayer.cpp
	MusicManager.cpp
	MusicScript.cpp
	MusicTimer.cpp
	MusicTrack.cpp
	MusicUpdatable.cpp
	MusicVariable.cpp
	MusicVariableContainer.cpp
)
