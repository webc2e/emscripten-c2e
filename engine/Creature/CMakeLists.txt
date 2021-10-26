cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(creature)

add_subdirectory(Brain)
add_subdirectory(Biochemistry)
add_subdirectory(History)
  
add_library(
  creature STATIC
  BodyPartOverlay.cpp
	Creature.cpp
	CreatureFacultyInterface.cpp
	AgentFacultyInterface.cpp
	CreatureAgent.cpp
	CreatureHead.cpp
	ExpressiveFaculty.cpp
	Faculty.cpp
	Genome.cpp
	GenomeStore.cpp
	LifeFaculty.cpp
	LinguisticFaculty.cpp
	MotorFaculty.cpp
	MusicFaculty.cpp
	ReproductiveFaculty.cpp
	SensoryFaculty.cpp
	SkeletalCreature.cpp
	Vocab.cpp
	voice.cpp
)
