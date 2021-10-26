cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(brain)
  
add_library(
  brain STATIC 
  Brain.cpp
	BrainComponent.cpp
	BrainScriptFunctions.cpp
	Dendrite.cpp
	Instinct.cpp
	Lobe.cpp
	Neuron.cpp
	SVRule.cpp
	Tract.cpp
)
