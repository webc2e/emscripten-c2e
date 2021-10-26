cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(pray)

add_library(
  pray STATIC 
	PrayBuilder.cpp
	PrayChunk.cpp
	PrayHandlers.cpp
	PrayManager.cpp
	StringIntGroup.cpp
)
