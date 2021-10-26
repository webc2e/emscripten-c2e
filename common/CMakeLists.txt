cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(common)

add_subdirectory(PRAYFiles)
  
add_library(
  common STATIC 
  BasicException.cpp
  C2eDebug.cpp
  Catalogue.cpp
	FileLocaliser.cpp
	Position.cpp
	SimpleLexer.cpp
	Vector2D.cpp
	Configurator.cpp
	StringFuncs.cpp
	FileFuncs.cpp
	SimpleMutex.cpp
	UnixSignals.cpp
	ProfanityFilter.cpp
)
