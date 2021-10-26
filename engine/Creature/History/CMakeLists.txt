cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(history)
  
add_library(
  history STATIC 
  CreatureHistory.cpp
	HistoryStore.cpp
	LifeEvent.cpp
)
