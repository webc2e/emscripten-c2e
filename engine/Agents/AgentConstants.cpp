#include "AgentConstants.h"

TableSpec ourScriptNumbers[] =
{
	TableSpec("Script Numbers"),
	TableSpec("Number", "Name", "Description"),

	TableSpec("0", "@Deactivate@", "Called when the agent receives a deactivate message."),
	TableSpec("1", "@Activate 1@", "Called when the agent receives an activate 1 message."),
	TableSpec("2", "@Activate 2@", "Called when the agent receives an activate 2 message."),
	TableSpec("3", "@Hit@", "Called when the agent receives a hit message."),
	TableSpec("4", "Pickup", "Called when the agent has been picked up by something other than a vehicle."),
	TableSpec("5", "Drop", "Called when the agent has been dropped by something other than a vehicle."),
	TableSpec("6", "Collision", "Called when the agent collides with an obstacle.  @#_P1_@ and @#_P2_@ are the x and y components of the collision velocity."),
	TableSpec("7", "Bump", "Called when a creature walks into a wall."),
	TableSpec("8", "Impact", "Called when an agent's presence impacts with another agent's presence (this is assuming both agents have their presence switched on)."),
	TableSpec("9", "@Timer@", "Called at a regular time interval, as set by @#TICK@."),
	TableSpec("10", "Constructor", "Called on creation."),
	TableSpec("12", "@Eat@", "Called when the creature eats something."),
	TableSpec("13", "Start Hold Hands", "Called when a creature starts holding hands with the pointer."),
	TableSpec("14", "Stop Hold Hands", "Called when a creature stops holding hands with the pointer."),

	TableSpec(),
	TableSpec("16", "Quiescent on Agents", "Scripts 16 - 30 are executed on a creature when it decides to do something with its attention on an ordinary agent (rather than a creature).  The script should perform this action.  Quiescent means stand and watch it. The catalogue entry \"Action Script To Neuron Mappings\" maps the brain to these scripts, although which scripts require an it object are hard-wired."),
	TableSpec("17", "Activate 1 on Agents", "Activate 1 it."),
	TableSpec("18", "Activate 2 on Agents", "Activate 2 it."),
	TableSpec("19", "Deactivate on Agents", "Deactivate it."),
	TableSpec("20", "Approach on Agents", "Go up and look at it."),
	TableSpec("21", "Retreat on Agents", "Walk or run away from it."),
	TableSpec("22", "Pickup on Agents", "Pick it up."),
	TableSpec("23", "Drop on Agents", "Drop anything you're carrying."),
	TableSpec("24", "Need on Agents", "Say what's bothering you."),
	TableSpec("25", "Rest on Agents", "Becoming sleepy."),
	TableSpec("26", "West on Agents", "Walk idly to west."),
	TableSpec("27", "East on Agents", "Walk idly to east."),
	TableSpec("28", "Eat on Agents", "Eat it."),
	TableSpec("29", "Hit on Agents", "Hit it."),
	TableSpec("30", "Undefined on Agents",	"For future expansion."),
	TableSpec("31", "Undefined on Agents", "For future expansion."),

	TableSpec(),
	TableSpec("32", "Quiescent on Creatures", "Scripts 32 - 47 are executed on a creature when it decides to do something with its attention on another creature.  Quiescent means stand and twiddle your thumbs."),
	TableSpec("33", "Activate 1 on Creatures", "Mating script."),
	TableSpec("34", "Activate 2 on Creatures", "Mating script."),
	TableSpec("35", "Deactivate on Creatures", "Deactivate it."),
	TableSpec("36", "Approach on Creatures", "Go up and look at it."),
	TableSpec("37", "Retreat on Creatures", "Walk or run away from it."),
	TableSpec("38", "Pickup on Creatures", "Pick it up."),
	TableSpec("39", "Drop on Creatures", "Drop anything you're carrying."),
	TableSpec("40", "Need on Creatures", "Say what's bothering you."),
	TableSpec("41", "Rest on Creatures", "Rest or sleep."),
	TableSpec("42", "West on Creatures", "Walk idly to west."),
	TableSpec("43", "East on Creatures", "Walk idly to east."),
	TableSpec("44", "Eat on Creatures", "Eat it."),
	TableSpec("45", "Hit on Creatures", "Hit it."),
	TableSpec("46", "Undefined on Creatures", "For future expansion."),
	TableSpec("47", "Undefined on Creatures", "For future expansion."),
		
	TableSpec(),
	TableSpec("64", "Flinch", "Involuntary action called when the creature flinches."),
	TableSpec("65", "Lay Egg", "Involuntary action called when the creature lays an egg."),
	TableSpec("66", "Sneeze", "Involuntary action called when the creature sneezes."),
	TableSpec("67", "Cough", "Involuntary action called when the creature coughs."),
	TableSpec("68", "Shiver", "Involuntary action called when the creature shivers."),
	TableSpec("69", "Sleep", "Involuntary action called when the creature sleeps."),
	TableSpec("70", "Fainting", "Involuntary action called when the creature faints."),
	TableSpec("71", "Unassigned", "Involuntary action for future expansion."),
	TableSpec("72", "@Die@", "Special involuntary action called when a creature dies.  Any death animations go here."),

	TableSpec(),
	TableSpec("73", "@Raw Key Down@", "Called when a key is pressed and @#IMSK@ is set.  The key code is sent in @#_P1_@."),
	TableSpec("74", "@Raw Key Up@", "Called when a key is released and @#IMSK@ is set.  The key code is sent in @#_P1_@."),
	TableSpec("75", "@Raw Mouse Move@", "Called when the mouse moves and @#IMSK@ is set.  The new x and y position is sent in @#_P1_@ and @#_P2_@"),
	TableSpec("76", "@Raw Mouse Down@", "Called when a mouse button is pressed and @#IMSK@ is set.  The button is sent in @#_P1_@ - 1 left, 2 right, 4 middle."),
	TableSpec("77", "@Raw Mouse Up@", "Called when a mouse button is released and @#IMSK@ is set.  The button is sent in @#_P1_@ - 1 left, 2 right, 4 middle."),
	TableSpec("78", "@Raw Mouse Wheel@", "Called when the mouse wheel is moved and @#IMSK@ is set.  The delta value is sent in @#_P1_@ -  120 units per 'click'."),
	TableSpec("79", "@Raw Translated Char@", "Called when a translated character is received and @#IMSK@ is set.  For example, on Japanese systems the raw key down and up codes can be in Roman characters, but the Input Method Editor converts them to Japanese characters, which are sent to the game with this message.  The translated key code is sent in @#_P1_@.  You can use this for character input, but it is easier to use @#PAT: TEXT@."),

	TableSpec(),
	TableSpec("92", "UI Mouse Down", "Called when the mouse clicks on an agent."),

	TableSpec(),
	TableSpec("101", "Pointer Activate 1", "Called on the pointer when an agent is activated 1.  The script has the same classifier as the agent being activated."),
	TableSpec("102", "Pointer Activate 2", "Called on the pointer when an agent is activated 2.  The script has the same classifier as the agent being activated."),
	TableSpec("103", "Pointer Deactivate", "Called on the pointer when an agent is deactivated.  The script has the same classifier as the agent being deactivated."),
	TableSpec("104", "Pointer Pickup", "Called on the pointer when an agent is activated 1.  The script has the same classifier as the agent being activated."),
	TableSpec("105", "Pointer Drop", "Called on the pointer when an agent is dropped.  The script has the same classifier as the agent being dropped."),
	TableSpec("110", "Pointer Port Select", "Called on the pointer when you manipulate a port."),
	TableSpec("111", "Pointer Port Connect", "Called on the pointer when you complete the connection between two ports."),
	TableSpec("112", "Pointer Port Disconnect", "Called on the pointer when you complete the disconnection of two previously connected ports."),
	TableSpec("113", "Pointer Port Cancel", "Called on the pointer if you cancel a port change part way through."),
	TableSpec("114", "Pointer Port Error", "Called on the pointer if there is some error with the configuration of ports the user is trying to make."),
	TableSpec("116", "Pointer Clicked Background", "Called when the pointer is clicked but there aren't any agents under it."),
	TableSpec("117", "Pointer Action Dispatch", "Called on the pointer to tell it what action clicking would take on the creature under it. @#_P1_@ says what will happen: 0 means no action or not above a creature, 1 means deactivate (slap), 2 means activate 1 (tickle)."),
	TableSpec("118", "Connection Break", "Called on an agent when any of its ports are broken as a result of exceeding the maximum connection distance."),


	TableSpec(),
	TableSpec("120", "@Selected Creature Changed@", "Called on all agents with this script when the selected creature is changed by @#NORN@.  @#_P1_@ is the new creature, @#_P2_@ is the previously selected creature."),
	TableSpec("121", "Vehicle Pickup", "Called when an agent has been picked up by a vehicle."),
	TableSpec("122", "Vehicle Drop", "Called when an agent has been dropped by a vehicle."),
	TableSpec("123", "Window Resized", "Called on all agents with this script whenever the main view is resized."),
	TableSpec("124", "Got Carried Agent", "Tells an agent that they have just picked something up."),
	TableSpec("125", "Lost Carried Agent", "Tells an agent that they have just dropped something."),
	TableSpec("126", "Make Speech Bubble", "Called when a creature speaks, so scripts can display a speech bubble.  Every agent which has this script is called.  @#_P1_@ is the text being spoken, @#_P2_@ is the creature who is speaking."),
	TableSpec("127", "@Life Event@", "Called whenever there is a new life event, whether an event built into the engine, or a custom event sent with @#HIST EVNT@.  @#_P1_@ is the moniker that the event happened to, @#_P2_@ is the event number as an index into the events for that moniker."),
	TableSpec("128", "World Loaded", "Called when the world had just been loaded, whether from bootstrap or from a saved file."), 

	TableSpec(),
	TableSpec("135", "Net Online", "Connection to the Babel server has been made."),
	TableSpec("136", "Net Offline", "Connection to the Babel server is broken."), 
	TableSpec("137", "@User Online@", "User chosen with @#NET: WHON@ has gone online. @#_P1_@ contains the user's id."),
	TableSpec("138", "@User Offline@", "User chosen with @#NET: WHON@ has gone offline. @#_P1_@ contains the user's id."),

	TableSpec("150", "Creature Navigation Callback Valid It", "Called by the approach command with _P1_ and _P2_ set to the IT object's X and Y coordinates respectively."),
	TableSpec("151", "Creature Navigation Callback Outside Room System", "Called by the approach command when you are unable to use the CA because you're outside the room system."),
	TableSpec("152", "Creature Navigation Callback Neighbour", "Called by the approach command if there is no IT object.  _P1_ and _P2_ are set to the X and Y coordinates of the room's centre with the highest (or lowest if retreating) smell of IT nearby."),
	TableSpec("153", "Creature Navigation Callback Link", "As for 152 but called when the best room is a link."),
	TableSpec("154", "Creature Navigation Callback Current Room Best", "As for 152 but called when the room you're in is already the best one."),

	TableSpec("160", "Creature Ageing Callback", "Called when the creature is about to age.  _P1_ contains the value of the next age stage."),

	TableSpec("200", "Mate", "Reserved for use in the mating scripts, but not directly used by the engine.  Tells a male to mate."),
	TableSpec("255", "Agent Exception", "This script specifies the behaviour when an agent tries to do something to an invalid agent.  For example, if you try and access an @#OVxx@ with a @#NULL@ target.  If this script isn't present for @#OWNR@, you get a run time error.  If it is present, that script is called and you can reset variables as necessary."),
};

int sizeOurScriptNumbers = sizeof(ourScriptNumbers) / sizeof(TableSpec);

int dummyScriptNumbers = AutoDocumentationTable::RegisterTable(ourScriptNumbers, sizeof(ourScriptNumbers));

TableSpec ourAttributeFlags[] =
{
	TableSpec("Attribute Flags"),
	TableSpec("Number (for @#ATTR@)", "Name", "Description"),

	TableSpec("1", "Carryable", "Can be picked up by things other than creatures, vehicles and the pointer."),
	TableSpec("2", "Mouseable", "Can be picked up by the mouse."),
	TableSpec("4", "@Activateable@", "Can be activated using the mouse - otherwise @#CLAC@ and @#CLIK@ style events don't get sent when you click on the agent."),
	TableSpec("8", "@Greedy Cabin@", "When set on a vehicle, it will automatically pick up things dropped in its cabin."),
	TableSpec("16", "@Invisible@", "Creatures don't see you - applies to @#ESEE@ and @#STAR@ on creatures, as well as internal creature code."),
	TableSpec("32", "@Floatable@", "Agent floats relative to the screen.  Move it with @#FLTO@.  If you call @#FREL@, the agent will float relative to another agent instead of the screen."),
	TableSpec("64", "Suffer Collisions", "Will collide with room boundaries, according to its @#PERM@."),
	TableSpec("128", "Suffer Physics", "Agent falls with proper physics, including gravity, air resistance and friction.  Otherwise, it simply moves with velocity."),
	TableSpec("256", "Camera Shy", "Agent can't be seen on a remote camera (@#PAT: CMRA@) or photograph (@#SNAP@).  For vehicles, the contents is shy as well."),
	TableSpec("512", "Open Air Cabin", "When set on a vehicle, it allows creatures to see and activate its passengers.  The default behaviour is that they can't."),
	TableSpec("1024", "Rotatable", "When set on an agent the various rotation commands (@#SPIN@, @#ROTN@, @#AVEL@ etc.) can be used."),
	TableSpec("2048", "Presence", "An agent with this attribute set has a circular physical presence in the world and receives Impact script callbacks."),
};

int dummyAttributeFlags = AutoDocumentationTable::RegisterTable(ourAttributeFlags, sizeof(ourAttributeFlags));

TableSpec ourCreaturePermissions[] =
{
	TableSpec("Creature Permissions"),
	TableSpec("Number (for @#BHVR@)", "Name", "Description"),

	TableSpec("1", "Activate 1", "Creature can activate 1 this agent."),
	TableSpec("2", "Activate 2", "Creature can activate 2 this agent."),
	TableSpec("4", "Deactivate", "Creature can deactivate this agent."),
	TableSpec("8", "Hit", "Creature can hit this agent."),
	TableSpec("16", "Eat", "Creature can eat this agent."),
	TableSpec("32", "Pick Up", "Creature can pick up this agent."),
};

int dummyCreaturePermissions = AutoDocumentationTable::RegisterTable(ourCreaturePermissions, sizeof(ourCreaturePermissions));


