cmake_minimum_required(VERSION 3.19)

project(webc2e)

add_definitions(
	-g
	# -DNOT_STRETCHING_AT_ALL
  -DC2E_NO_INLINE_ASM
  -DC2E_SDL
	-DLINUX_PORT
	-s USE_PTHREADS
	-s USE_SDL=1
	-s USE_SDL_MIXER=1
)

add_subdirectory(common)
add_subdirectory(engine)

set(CMAKE_EXECUTABLE_SUFFIX ".html")

set(
	SOURCES

	common/BasicException.cpp
	common/C2eDebug.cpp
	common/Catalogue.cpp
	common/FileLocaliser.cpp
	common/Position.cpp
	common/SimpleLexer.cpp
	common/Vector2D.cpp
	common/Configurator.cpp
	common/StringFuncs.cpp
	common/FileFuncs.cpp
	common/SimpleMutex.cpp
	common/UnixSignals.cpp
	common/ProfanityFilter.cpp

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

	engine/Creature/BodyPartOverlay.cpp
	engine/Creature/Creature.cpp
	engine/Creature/CreatureFacultyInterface.cpp
	engine/Creature/AgentFacultyInterface.cpp
	engine/Creature/CreatureAgent.cpp
	engine/Creature/CreatureHead.cpp
	engine/Creature/ExpressiveFaculty.cpp
	engine/Creature/Faculty.cpp
	engine/Creature/Genome.cpp
	engine/Creature/GenomeStore.cpp
	engine/Creature/LifeFaculty.cpp
	engine/Creature/LinguisticFaculty.cpp
	engine/Creature/MotorFaculty.cpp
	engine/Creature/MusicFaculty.cpp
	engine/Creature/ReproductiveFaculty.cpp
	engine/Creature/SensoryFaculty.cpp
	engine/Creature/SkeletalCreature.cpp
	engine/Creature/Vocab.cpp
	engine/Creature/voice.cpp

	engine/Creature/Brain/Brain.cpp
	engine/Creature/Brain/BrainComponent.cpp
	engine/Creature/Brain/BrainScriptFunctions.cpp
	engine/Creature/Brain/Dendrite.cpp
	engine/Creature/Brain/Instinct.cpp
	engine/Creature/Brain/Lobe.cpp
	engine/Creature/Brain/Neuron.cpp
	engine/Creature/Brain/SVRule.cpp
	engine/Creature/Brain/Tract.cpp

	engine/Creature/Biochemistry/Biochemistry.cpp
	engine/Creature/Biochemistry/BiochemistryConstants.cpp
	engine/Creature/Biochemistry/Chemical.cpp
	engine/Creature/Biochemistry/Emitter.cpp
	engine/Creature/Biochemistry/NeuroEmitter.cpp
	engine/Creature/Biochemistry/Organ.cpp
	engine/Creature/Biochemistry/Receptor.cpp

	engine/Creature/History/CreatureHistory.cpp
	engine/Creature/History/HistoryStore.cpp
	engine/Creature/History/LifeEvent.cpp

	engine/Agents/Agent.cpp
	engine/Agents/AgentConstants.cpp
	engine/Agents/AgentHandle.cpp
	engine/Agents/CameraPart.cpp
	engine/Agents/CompoundAgent.cpp
	engine/Agents/CompoundPart.cpp
	engine/Agents/InputPort.cpp
	engine/Agents/MessageQueue.cpp
	engine/Agents/OutputPort.cpp
	engine/Agents/PointerAgent.cpp
	engine/Agents/Port.cpp
	engine/Agents/PortBundle.cpp
	engine/Agents/QuoteFactory.cpp
	engine/Agents/SimpleAgent.cpp
	engine/Agents/UIPart.cpp
	engine/Agents/Vehicle.cpp

	engine/Caos/AgentHandlers.cpp
	engine/Caos/AutoDocumentationTable.cpp
	engine/Caos/CAOSDescription.cpp
	engine/Caos/CAOSException.cpp
	engine/Caos/CAOSMachine.cpp
	engine/Caos/CAOSTables.cpp
	engine/Caos/CAOSVar.cpp
	engine/Caos/CompoundHandlers.cpp
	engine/Caos/CreatureHandlers.cpp
	engine/Caos/DebugHandlers.cpp
	engine/Caos/DebugInfo.cpp
	engine/Caos/DisplayHandlers.cpp
	engine/Caos/GeneralHandlers.cpp
	engine/Caos/HistoryHandlers.cpp
	engine/Caos/Lexer.cpp
	engine/Caos/MacroScript.cpp
	engine/Caos/MapHandlers.cpp
	engine/Caos/OpSpec.cpp
	engine/Caos/Orderiser.cpp
	engine/Caos/PortHandlers.cpp
	engine/Caos/Scriptorium.cpp
	engine/Caos/SoundHandlers.cpp
	engine/Caos/TableSpec.cpp
	engine/Caos/VelocityVariable.cpp
	engine/Caos/SockServer.cpp
	engine/Caos/unix/RuntimeErrorDialog.cpp

	engine/Display/Background.cpp
	engine/Display/Bitmap.cpp
	engine/Display/ClonedGallery.cpp
	engine/Display/CompressedBitmap.cpp
	engine/Display/CreatureGallery.cpp
	engine/Display/DrawableObject.cpp
	engine/Display/DrawableObjectHandler.cpp
	engine/Display/ErrorMessageHandler.cpp
	engine/Display/Gallery.cpp
	engine/Display/Line.cpp
	engine/Display/SharedGallery.cpp
	engine/Display/Sprite.cpp
	engine/Display/System.cpp
	engine/Display/TintManager.cpp
	engine/Display/EasterEgg.cpp
	engine/Display/DisplayEnginePlotFunctions.cpp
	engine/Display/Console/ConsoleErrorDialog.cpp

	engine/Map/Map.cpp
	engine/Map/CARates.cpp
	engine/Map/MapCA.cpp
	engine/Map/RoomCA.cpp

	engine/Sound/SDL/SDL_Soundlib.cpp
	engine/Sound/stub/stub_MidiModule.cpp
	engine/Sound/stub/stub_CDRomDevice.cpp
	engine/Sound/MusicAction.cpp
	engine/Sound/MusicAleotoricLayer.cpp
	engine/Sound/MusicEffect.cpp
	engine/Sound/MusicExpression.cpp
	engine/Sound/MusicGlobals.cpp
	engine/Sound/MusicLayer.cpp
	engine/Sound/MusicLoopLayer.cpp
	engine/Sound/MusicManager.cpp
	engine/Sound/MusicScript.cpp
	engine/Sound/MusicTimer.cpp
	engine/Sound/MusicTrack.cpp
	engine/Sound/MusicUpdatable.cpp
	engine/Sound/MusicVariable.cpp
	engine/Sound/MusicVariableContainer.cpp

	engine/unix/File.cpp

	engine/AgentDisplay/EntityImage.cpp
	engine/AgentDisplay/EntityImageClone.cpp
	engine/AgentDisplay/EntityImageWithEmbeddedCamera.cpp

	engine/Camera/MapImage.cpp
	engine/Camera/Camera.cpp
	engine/Camera/RemoteCamera.cpp
	engine/Camera/MainCamera.cpp
	engine/Camera/CameraSprite.cpp
)

add_executable(webc2e
  ${SOURCES}
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -s INITIAL_MEMORY=536870912 -s CASE_INSENSITIVE_FS=1 -s USE_ZLIB -s USE_PTHREADS -s USE_SDL=1 -s USE_SDL_MIXER=1 -s ASSERTIONS=1 -s LLD_REPORT_UNDEFINED -s NO_DISABLE_EXCEPTION_CATCHING --preload-file '../../C2E/GAMES/Creatures\ 3@/' --preload-file '../../C2E/GAMES/Creatures\ 3@/'")

target_link_libraries(webc2e
	unix
	unix_display
	sdl
	sdl_stretch
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
	sound
)
