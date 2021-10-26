cmake_minimum_required(VERSION 3.19)

project(webc2e)

add_definitions(
	-DNOT_STRETCHING_AT_ALL
  -DC2E_NO_INLINE_ASM
  -DC2E_SDL
	-s USE_PTHREADS
	-s USE_SDL=2
)

add_subdirectory(common)
add_subdirectory(engine)

set(CMAKE_EXECUTABLE_SUFFIX ".html")

add_executable(webc2e 
  engine/Display/SDL/SDL_Main.cpp
  engine/AgentManager.cpp
	engine/App.cpp
	engine/C2eServices.cpp
	engine/Classifier.cpp
	engine/CosInstaller.cpp
	engine/CreaturesArchive.cpp
	engine/CustomHeap.cpp
	engine/Entity.cpp
	engine/FilePath.cpp
	engine/FlightRecorder.cpp
	engine/General.cpp
	engine/InputManager.cpp
	engine/Maths.cpp
	engine/Message.cpp
	engine/PersistentObject.cpp
	engine/Scramble.cpp
	engine/Stimulus.cpp
	engine/World.cpp
	engine/md5.cpp
	engine/mfchack.cpp
	engine/TimeFuncs.cpp
	engine/ModuleImporter.cpp
	engine/DirectoryManager.cpp
	engine/Presence.cpp
	engine/Angle.cpp
	engine/Circle.cpp
	engine/UniqueIdentifier.cpp
	engine/QuickFsp.cpp
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_ZLIB -s USE_PTHREADS -s USE_SDL=2 -s LLD_REPORT_UNDEFINED -s NO_DISABLE_EXCEPTION_CATCHING --preload-file '../../C2E/GAMES/Creatures\ 3@/'")

target_link_libraries(webc2e
	unix
	common
  pray
  biochemistry
  brain
  history
  creature
	agents
	caos
	map
	display
	camera
	agent_display
	unix_display
	sdl
	sound
)
