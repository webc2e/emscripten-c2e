cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(agent_display)

add_library(
  agent_display STATIC 
  EntityImage.cpp
	EntityImageClone.cpp
	EntityImageWithEmbeddedCamera.cpp
)
