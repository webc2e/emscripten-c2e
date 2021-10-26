cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(biochemistry)
  
add_library(
	biochemistry
  Biochemistry.cpp
	BiochemistryConstants.cpp
	Chemical.cpp
	Emitter.cpp
	NeuroEmitter.cpp
	Organ.cpp
	Receptor.cpp
)
