#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "NetHandlers.h"
#include "NetLogImplementation.h"

#include "../../engine/Caos/CAOSConstants.h"
#include "../../engine/Caos/OpSpec.h"
#include "../../engine/Caos/CAOSMachine.h"

#include <DSNetManager.h>
#include <NetMemoryPack.h>
#include <NetMemoryUnpack.h>
#include <NetMessages.h>

#include "../../common/PRAYFiles/PrayHandlers.h"
#include "../../common/PRAYFiles/PrayException.h"
#include "../../common/PRAYFiles/PrayBuilder.h"
#include "../../common/PRAYFiles/PrayManager.h"

#include "../../engine/AgentManager.h"
#include "../../engine/Agents/AgentConstants.h"
#include "../../engine/App.h"
#include "../../engine/Maths.h"

#include "../../common/FileFuncs.h"

#include <sstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

static DSNetManager* theNetManager;
void NetHandlers::SetNetManager(DSNetManager* netManager)
{
	theNetManager = netManager;
}
DSNetManager& NetHandlers::GetNetManager()
{
	return *theNetManager;
}

NetHandlers::WhoseWantedMap NetHandlers::myWhoseWantedMap;
NetHandlers::HearingMap NetHandlers::myHearingMap;

std::string NetHandlers::myNickname;
std::string NetHandlers::myPassword;
std::string NetHandlers::myUserID;

Configurator NetHandlers::myServerConfig;
Configurator NetHandlers::myPasswordConfig;

////////////////////////////////////////////////////////////////////////////////

// Three boring command handlers - there must be a
// better way of doing subcommands!

void NetHandlers::Command_NET( CAOSMachine& vm )
{
	static CommandHandler subcmds [] = 
	{
		SubCommand_NET_LINE, // async
		SubCommand_NET_WHON, // async
		SubCommand_NET_WHOF,
		SubCommand_NET_WHOD,
		SubCommand_NET_WRIT,
		SubCommand_NET_HEAR,
		SubCommand_NET_HEAD,
		SubCommand_NET_PASS,
		SubCommand_NET_WHOZ,
		SubCommand_NET_RUSO, // async
		SubCommand_NET_UNIK, // async
		SubCommand_NET_STAT, // async
	};
	int thiscmd;
	if (vm.IsBlocking()) // cope with blocking subcommands like RUSO
		thiscmd = vm.Pop();
	else
		thiscmd = vm.FetchOp();
	(subcmds[thiscmd])( vm );
	if (vm.IsBlocking())
		vm.Push(thiscmd);
}

int NetHandlers::IntegerRV_NET( CAOSMachine& vm )
{
	static IntegerRVHandler subhandlers [] =
	{
		IntegerRV_NET_LINE,
		IntegerRV_NET_EXPO, // async
		IntegerRV_NET_ULIN, // fast if user is in WWR
		IntegerRV_NET_MAKE, // async
		IntegerRV_NET_ERRA,
		IntegerRV_NET_RAWE,
	};
	int thisrv;
	thisrv = vm.FetchOp();
	return (subhandlers[thisrv])( vm );
}

void NetHandlers::StringRV_NET( CAOSMachine& vm, std::string& str )
{
	static StringRVHandler substrings [] =
	{
		StringRV_NET_USER,
		StringRV_NET_FROM,
		StringRV_NET_HOST,
		StringRV_NET_PASS,
		StringRV_NET_WHAT,
	};
	int thisst;
	thisst = vm.FetchOp();
	(substrings[thisst])( vm, str );
}

////////////////////////////////////////////////////////////////////////////////

// The interesting tables

OpSpec ourSubCommandTable_NET[] =
{
	OpSpec( "LINE", "i", "state", categoryNet, "Goes on or offline, connecting or disconnecting from the Babel server.  Set to 1 to connect, 0 to disconnect.  A @#NET: USER@ must be set first.  @#NET: ERRA@ is set to any error code.  While the connection is being made, this command can block for several ticks.  This command never runs in an @#INST@."),
	OpSpec( "WHON", "s", "user", categoryNet, "Add a user to the whose wanted register for the target agent.  Scripts @#User Online@ and @#User Offline@ are now called on this agent when that user goes on or offline, or indeed when the local user goes offline.  Use @#NET: WHOF@ to remove them from the register.  This command is blocking, it can take several ticks to return."),
	OpSpec( "WHOF", "s", "user", categoryNet, "Removes a user from the whose wanted list for the target agent.  See @#NET: WHON@."),
	OpSpec( "WHOD", "", "", categoryNet, "Dump debugging information about the whose wanted register."),
	OpSpec( "WRIT", "ssimm", "user_id channel message_id param_1 param_2", categoryNet, "Send a message to a remote machine, as specified by the user identifier.  All agents which are @#NET: HEAR@ing on the given channel will receive the message, and run the appropriate script.  If the specified user is offline, then the message is discarded.  The @#FROM@ variable of the receiving script contains the user id of the sender, as a string.  See also @#MESG WRIT@."),
	OpSpec( "HEAR", "s", "channel", categoryNet, "The target agent will now accept CAOS messages over the network on the specified channel, and execute their script as appropriate.  Use @#NET: WRIT@ to send the message."),
	OpSpec( "HEAD", "", "", categoryNet, "Dump debugging informatino about who is @#NET: HEAR@ing on what channels."),
	OpSpec( "PASS", "ss", "nick_name password", categoryNet, "Set nickname and password - do this before connecting with @#NET: LINE@.  If you set @#GAME@ \"@#engine_netbabel_save_passwords@\" to 1 then the password for each nickname is saved in user.cfg, so you can specify an empty string for the password after the first time.  The nickname is saved with the serialised world, so is cleared when you start a new world.  You can use @#NET: PASS@ to retrieve the user later."),
	OpSpec( "WHOZ", "", "", categoryNet, "Zap the target agent's whose wanted register, removing all entries."),
	OpSpec( "RUSO", "v", "store_result", categoryNet, "Returns (into store_result) a random user who is currently online.  Returns an empty string if you're offline, or if you aren't using the Docking Station Babel server module.  Since you're online, it can return yourself (especially if you're the only person online!).  The user is also only <i>likely</i> to be online - they could have gone offline since the server replied to you.<p>This is a command rather than an integer r-value because it is <i>blocking</i>.  This means that it might take several ticks before the server returns the result.  In this sense it is similar to a command like @#OVER@, so it does not run in an @#INST@.  You should use @#LOCK@ if you don't want your script interrupting."),
	OpSpec( "UNIK", "sv", "user_id store_result", categoryNet, "Returns the specified user's screen or nick name.  Returns empty string if offline, or no such user.  This command can take many ticks to execute while the server is quizzed, like @#NET: RUSO@."),
	OpSpec( "STAT", "vvvv", "time_online users_online bytes_received bytes_sent", categoryNet, "Returns statistics for the current Babel connection, or -1 if offline.  This command can block (doesn't execute in an @#INST@).  The statistics are:<br>time_online - Time online in milliseconds<br>users_online - Number of users currently connected to the server<br>bytes_received - Bytes received by the client<br>bytes_sent - Bytes sent from the client"),
};


OpSpec ourSubIntegerRVTable_NET[] =
{
	OpSpec( "LINE", "", "", categoryNet, "Returns 1 if you are connected to the Babel server, or 0 if you aren't."),
	OpSpec( "EXPO", "ss", "chunk_type dest_user_id", categoryNet, "Transwarp the target creature to the given user.  The Creature is exported to the warp out directory; this command is very similar to @#PRAY EXPO@. Return value is one of the following:<br>0 for success<br>1 if the creature, or if pregnant any of its offspring, are already on disk in some form.  This case won't happen much, if you use a special chunk name like WARP.<br>2 if the user hasn't been online in this world yet / since the user name changed, so we don't know who they are.<p>When receiving a creature, use @#NET: FROM@ to find out who sent it."),
	OpSpec( "ULIN", "s", "user_id", categoryNet, "Returns 1 if the specified user is online, or 0 if they are offline.  This is slow (i.e. has to call the server) unless the user is in the whose wanted register of any agent.  Use @#NET: WHON@ to add a user to the register."),
	OpSpec( "MAKE", "issv", "which_journal_spot journal_name user report_destination", categoryResources, "Like @#PRAY MAKE@, only sends the made pray file to the specified user. This will arrive in their inbox, where it can be read with normal PRAY commands and deleted with @#PRAY KILL@."),
	OpSpec( "ERRA", "", "", categoryNet, "Returns an error code from the last command.  Currently @#NET: LINE@ is the only command to set it.<p>Error codes are:<br>0 - Unknown<br>1 - Connection OK<br>2 - Connection failed, you or the server are offline<br>3 - Connection failed, invalid user name/password<br>4 - Connection failed, you are already logged in elsewhere<br>5 - Connection failed, too many users for server<br>6 - Connection failed, internal error<br>7 - Connection failed, new client version required.<p>Try @#NET: RAWE@ for more detailed diagnostic codes."),
	OpSpec( "RAWE", "", "", categoryNet, "Returns an internal error code from Babel.  Only use this for display and diagnostic purpose, use @#NET: ERRA@ for documented error codes which you can rely on."),
};


OpSpec ourSubStringRVTable_NET[] =
{
	OpSpec( "USER", "", "", categoryNet, "Returns the user's numeric Babel id, or an empty string if they have never logged in with this world since they last changed user name."),
	OpSpec( "FROM", "s", "resource_name", categoryNet, "The user who sent the PRAY file which contains the specified resource.  If the resource did not arrive as a message over the network via @#NET: MAKE@ or @#NET: EXPO@, then this returns an empty string. The user returned by this command is guaranteed in a way that looking at the content of the PRAY file would not be.  For example, the \"Last Network User\" attribute in an exported Creature made with @#NET: EXPO@ could be faked."),
	OpSpec( "HOST", "", "", categoryNet, "Returns the hostname, port, id and friendly name on that host that we are currently connected to, or empty string if offline.  The fields are space separated, although the last field (friendly name) may contain spaces."),
	OpSpec( "PASS", "", "", categoryNet, "Returns the currently set username, as selected with @#PASS@."),
	OpSpec("WHAT", "", "", categoryNet, "For debugging only.  Returns a string describing what the upload/query network thread is currently doing.  For example, it may be fetching a random online user, or uploading some creature history.  Returns an emptry string if it is doing nothing."),
};

// Register ourself with the game

// static
void NetHandlers::PushTables(CAOSDescription& caosDescription)
{
	caosDescription.PushTable(NetHandlers::Command_NET, idCommandTable, "NET:", ourSubCommandTable_NET, sizeof(ourSubCommandTable_NET) / sizeof(OpSpec));	
	caosDescription.PushTable(NetHandlers::IntegerRV_NET, idIntegerRVTable, "NET:", ourSubIntegerRVTable_NET, sizeof(ourSubIntegerRVTable_NET) / sizeof(OpSpec));
	caosDescription.PushTable(NetHandlers::StringRV_NET, idStringRVTable, "NET:", ourSubStringRVTable_NET, sizeof(ourSubStringRVTable_NET) / sizeof(OpSpec));
}

////////////////////////////////////////////////////////////////////////////////

void NetHandlers::SubCommand_NET_LINE( CAOSMachine& vm )
{
	int targetState;
	if (vm.IsBlocking())
		targetState = vm.Pop();
	else
		targetState = vm.FetchIntegerRV();

	if (targetState == 1)
	{
		// A user needs to be set before you can go online
		if (!GetNetManager().UserSet() || myNickname.empty())
			vm.ThrowTagRunError("net_babel", 0);
 
		// Find machine to connect to
		std::string overrideHost;
		int overridePort = 0;
		myServerConfig.Get("Override Server", overrideHost);
		myServerConfig.Get("Override Port", overridePort);
		if (overridePort == 0)
			overridePort = 49152;
		if (!overrideHost.empty())
			DSNetManager::OverrideHost(overrideHost, overridePort);

		// Find out who is for WWR
		std::vector<std::string> usersForWWR;
		WhoseWantedMap::iterator itMap = myWhoseWantedMap.begin();
		while(itMap != myWhoseWantedMap.end())
		{
			std::string user = itMap->first;
			usersForWWR.push_back(user);
			itMap++;
		}

		// Do the connection
		bool block;
		GetNetManager().ConnectInBackground(block, usersForWWR);
		// If asked to, block so this command is called again next tick
		if (block)
		{
			vm.Block();
			vm.Push(targetState);
		}
		else
		{
			vm.UnBlock();

			// Store the user ID that the server has returned
			std::string newUserID = GetNetManager().GetUser();
			if (!newUserID.empty())
				myUserID = newUserID;

			// Remember the new server list that we have been sent,
			// so next time we connect there is more of a choice
			WriteServerList();
		}
	}
	else if (targetState == 0)
	{
		GetNetManager().Disconnect();
	}
}

int NetHandlers::IntegerRV_NET_LINE( CAOSMachine& vm )
{
	return GetNetManager().Online() ? 1 : 0;
}

std::string NetHandlers::GetCachedUserID()
{
	return myUserID;
}

void NetHandlers::StringRV_NET_USER( CAOSMachine& vm, std::string& str )
{
	// This is only guaranteed when you have been online once in the
	// world already.
	str = myUserID;
}

void NetHandlers::SubCommand_NET_PASS( CAOSMachine& vm )
{
	std::string nickName, password;
	vm.FetchStringRV(nickName);
	vm.FetchStringRV(password);

	// User can't change while you're online
	if (GetNetManager().Online())
		vm.ThrowTagRunError("net_babel", 1);

	SetNickname(nickName, password);
}

void NetHandlers::SetNickname(std::string nickName, std::string password)
{
	if (password.empty())
	{
		myPasswordConfig.Get("Password " + nickName, password);
	}

	// Save the password if eame variables tell us to
	if (theApp.GetGameVar("engine_netbabel_save_passwords").GetInteger() == 1)
	{
		myPasswordConfig.Set("Password " + nickName, password);
		myPasswordConfig.Flush();
	}
	else
	{
		// Otherwise delete the saved password
		myPasswordConfig.Zap("Password " + nickName);
		myPasswordConfig.Flush();
	}

	if (myNickname != nickName || myPassword != password)
	{
		// Go offline if we are online
		if (GetNetManager().Online())
			GetNetManager().Disconnect();

		if (myNickname != nickName)
			myUserID = "";
		myNickname = nickName;
		myPassword = password;
	 	GetNetManager().SetUser(myNickname, myPassword);

		theApp.NotifyNewNickname(myNickname);
	}
}

std::string NetHandlers::GetNickname()
{
	return myNickname;
}

void NetHandlers::StringRV_NET_PASS( CAOSMachine& vm, std::string& str )
{
	str = GetNickname();
}

int NetHandlers::IntegerRV_NET_ULIN( CAOSMachine& vm )
{
	std::string user;
	vm.FetchStringRV(user);

	return GetNetManager().IsUserOnline(user) ? 1 : 0;
}

void NetHandlers::SubCommand_NET_UNIK( CAOSMachine& vm )
{
	std::string user;
	if (!vm.IsBlocking())
		vm.FetchStringRV(user);
	else
		user = vm.PopString();

	std::string nickName;
	bool block;
	GetNetManager().FetchUserData(user, nickName, block);

	if (block)
	{
		vm.Block();
		vm.PushString(user);
	}
	else
	{
		vm.UnBlock();

		// Return the value
		CAOSVar& var = vm.FetchVariable();
		var.SetString(nickName);
	}
}

void NetHandlers::SubCommand_NET_STAT( CAOSMachine& vm)
{
	bool block;
	int timeOnline = -1, usersOnline = -1, bytesReceived = -1, bytesSent = -1;
	GetNetManager().GetStatus(timeOnline, usersOnline, bytesReceived, bytesSent, block);

	if (block)
		vm.Block(); // this function will get called again next tick, to see if the result has arrived
	else
	{
		vm.UnBlock();
		CAOSVar& timeOnlineResult = vm.FetchVariable();
		CAOSVar& usersOnlineResult = vm.FetchVariable();
		CAOSVar& bytesReceivedResult = vm.FetchVariable();
		CAOSVar& bytesSentResult = vm.FetchVariable();
		timeOnlineResult.SetInteger(timeOnline);
		usersOnlineResult.SetInteger(usersOnline);
		bytesReceivedResult.SetInteger(bytesReceived);
		bytesSentResult.SetInteger(bytesSent);
	}

}

void NetHandlers::StringRV_NET_HOST( CAOSMachine& vm, std::string& str )
{
	if (!GetNetManager().Online())
	{
		str = "";
		return;
	}

	std::ostringstream out;
	out << GetNetManager().GetServerHost();
	out << " " << GetNetManager().GetServerPort();
	out << " " << GetNetManager().GetServerID();
	out << " " << GetNetManager().GetServerFriendlyName();

	str = out.str();
}

int NetHandlers::IntegerRV_NET_ERRA( CAOSMachine& vm )
{
	return (int)GetNetManager().GetLastError();
}

int NetHandlers::IntegerRV_NET_RAWE( CAOSMachine& vm )
{
	return GetNetManager().RawGetLastError();
}

void NetHandlers::StringRV_NET_WHAT(CAOSMachine& vm, std::string& str)
{
	str = GetNetManager().DebugGetCurrentAction();
}

////////////////////////////////////////////////////////////////////////////////

int NetHandlers::IntegerRV_NET_EXPO( CAOSMachine& vm )
{
	std::string chunkType;
	vm.FetchStringRV(chunkType);
	std::string userID;
	vm.FetchStringRV(userID);
																		
	// Generate filename from name and moniker
	std::string filenameActual = GetNetManager().ReturnUniqueOutboxFilename(userID);
	if (filenameActual.empty())
		vm.ThrowTagRunError("net_babel", 2, userID.c_str());
	std::string filenameTemp = filenameActual + ".tmp";

	// It's a bit bad that this code has to worry about this, but
	// you have to kill agents differently if they are the virtual
	// machine currently running
	bool selfExporting = (vm.GetTarg() == vm.GetOwner());

	int ret = -1;
	try
	{
		ret = PrayHandlers::ExportHelper(vm.GetCreatureTarg(), chunkType, filenameTemp, selfExporting, true);
	}
	catch ( PrayException& pe )
	{
		vm.ThrowRunError( BasicException(pe.GetMessage().c_str()) );
	}

	// Move the temporary file into place (so the operation is
	// atomic)
	MoveFile(filenameTemp.c_str(), filenameActual.c_str());

	GetNetManager().SendOrdinaryMessages();

	return ret;
}

int NetHandlers::IntegerRV_NET_MAKE( CAOSMachine& vm )
{
	int WhichJournal = vm.FetchIntegerRV();
	std::string journalName;
	vm.FetchStringRV(journalName);
	std::string user;
	vm.FetchStringRV(user);
	CAOSVar& report = vm.FetchVariable();

	std::string aDir;
	if (WhichJournal == 0)
		theDirectoryManager.GetWorldDirectoryVersion(JOURNAL_DIR, aDir, false);
	else
		aDir = theDirectoryManager.GetDirectory(JOURNAL_DIR);

	if (aDir.empty())
		vm.ThrowRunError(CAOSMachine::sidPrayBuilderError, "Journal Folder not found");

	journalName = aDir + journalName;

	// Generate filename from name and moniker
	std::string filenameActual = GetNetManager().ReturnUniqueOutboxFilename(user);
	if (filenameActual.empty())
		vm.ThrowTagRunError("net_babel", 2, user.c_str());
	std::string filenameTemp = filenameActual + ".tmp";

	bool ok = PrayHandlers::MakeHelper(aDir, journalName, filenameTemp, report);
	
	if (ok)
	{
		// Move the temporary file into place (so the operation is atomic)
		MoveFile(filenameTemp.c_str(), filenameActual.c_str());

		// Send the pray file
		GetNetManager().SendOrdinaryMessages();
	}

	return ok ? 0 : 1;
}

	
void NetHandlers::StringRV_NET_FROM( CAOSMachine& vm, std::string& str )
{
	str = "";

	std::string chunk;
	vm.FetchStringRV( chunk );

	if (thePrayManager.CheckChunk(chunk) == 0)
		return;
	
	std::string file = thePrayManager.GetChunkParentFile(chunk);

	if (FileExtension(file) != "warp")
		return;

	str = GetNetManager().FindUserWhoSentFile(file);
}

////////////////////////////////////////////////////////////////////////////////

void NetHandlers::SubCommand_NET_WHON( CAOSMachine& vm )
{
	std::string user;
	AgentHandle targ;
	if (vm.IsBlocking())
	{
		user = vm.PopString();
		targ = vm.PopHandle();
	}
	else
	{
		vm.FetchStringRV(user);
		targ = vm.GetTarg();
	}

	// Add the user to the WWR, if they are new to being watched
	bool block = false;
	WhoseWantedMap::const_iterator it = myWhoseWantedMap.find(user);
	if (it == myWhoseWantedMap.end())
		GetNetManager().AddToWWR(user, block);

	if (block)
	{
		vm.Block();

		vm.PushString(user);
		vm.PushHandle(targ);
	}
	else
	{
		vm.UnBlock();
	
		// Send initial state script
		int event = GetNetManager().IsUserOnline(user) ? SCRIPT_NET_USER_ONLINE : SCRIPT_NET_USER_OFFLINE;
		CAOSVar p1, p2;
		p1.SetString(user);
		theAgentManager.ExecuteScriptOnAgent(targ, event, COASVARAGENTNULL, p1, p2);

		// Remember that this agent is looking at the user
		myWhoseWantedMap[user].insert(targ);
	}
}

void NetHandlers::SubCommand_NET_WHOF( CAOSMachine& vm )
{
	std::string user;
	vm.FetchStringRV(user);
	AgentHandle targ = vm.GetTarg();

	// Remove our agent from the set of agents watching the user
	WhoseWantedMap::iterator it = myWhoseWantedMap.find(user);
	if (it != myWhoseWantedMap.end())
	{
		if (it->second.find(targ) != it->second.end())
			it->second.erase(it->second.find(targ));

		// If there are no agents left watching the user, 
		// remove him completely
		if (it->second.empty())
		{
			myWhoseWantedMap.erase(it);
			GetNetManager().RemoveFromWWR(user);
		}
	}
}

void NetHandlers::SubCommand_NET_WHOD( CAOSMachine& vm )
{
	std::ostream* out = vm.GetOutStream();

	// Dump our maps
	*out << myWhoseWantedMap.size() << " users in agent WWR" << std::endl;
	WhoseWantedMap::iterator itMap;
	for (itMap = myWhoseWantedMap.begin(); itMap != myWhoseWantedMap.end(); ++itMap)
	{
		*out << itMap->first << ": ";
		AgentSet& set = itMap->second;
		AgentSet::iterator itSet;
		for (itSet = set.begin(); itSet != set.end(); ++itSet)
		{
			// TODO: Print unique ID rather than someone
			if (*itSet == vm.GetTarg())
				*out << "targ ";
			else
				*out << "someone ";
		}
		*out << std::endl;
	}
}

// Erase all entries from targets whose wanted register
void NetHandlers::SubCommand_NET_WHOZ( CAOSMachine& vm )
{
	AgentHandle targ = vm.GetTarg();

	// Scan the map
	WhoseWantedMap::iterator itMap = myWhoseWantedMap.begin();
	while(itMap != myWhoseWantedMap.end())
	{
		// For each set, zap any entries pointing to target
		AgentSet& set = itMap->second;
		AgentSet::size_type count = set.erase(targ);
		// If there are no agents left watching the user, 
		// remove him completely
		if (set.empty())
		{
			GetNetManager().RemoveFromWWR(itMap->first);
			myWhoseWantedMap.erase(itMap);
			itMap = myWhoseWantedMap.begin(); // TODO: Find a way of not going back to start, if it is a speed problem
		}
		else
			itMap++;
	}
}

void NetHandlers::NotifyUsersOnline(int tick)
{
	// Every now and again, cull NULLHANDLEs
	if (tick > 0 && tick % 20 == 0)
	{
		// Loop through users
		WhoseWantedMap::iterator itMap = myWhoseWantedMap.begin();
		while(itMap != myWhoseWantedMap.end())
		{
			// And through agents watching that user
			AgentSet& set = itMap->second;
			AgentSet::iterator itSet = set.begin();
			while(itSet != set.end())
			{
				// We make a copy of the handle, so the handle in the set isn't
				// modified by garbage collection (you can't change values in
				// sets, if it affects the ordering of them)
				AgentHandle agent = *itSet;
				if (agent.IsInvalid())
				{
					set.erase(itSet); 
					itSet = set.begin(); // TODO: Find a way of not going back to start, if it is a speed problem
				}
				else 
					++itSet;
			}
	
			// If there are no agents left watching the user, 
			// remove him completely
			if (set.empty())
			{
				GetNetManager().RemoveFromWWR(itMap->first);
				myWhoseWantedMap.erase(itMap);
				itMap = myWhoseWantedMap.begin(); // TODO: Find a way of not going back to start, if it is a speed problem
			}
			else
				itMap++;
		}
	}

	// Loop through all the users which have recently changed online state
	std::string user;
	bool online;
	GetNetManager().ResetIteratorWWR();
	while (GetNetManager().NextChangedUserWWR(user, online))
	{
		CAOSVar p1, p2;
		p1.SetString(user);

		// See if they are in our whose wanted map
		WhoseWantedMap::iterator it = myWhoseWantedMap.find(user);
		if (it != myWhoseWantedMap.end())
		{
			NotifyAgentsWWR(it->second, online, p1, p2);
		}
		else
		{
			// We got notification from a user who we don't think we're watching
			// - probably a notification stored up in NetManager, that we've removed
			// from our list
			ASSERT(true); // place to put breakpoints
		}
	}
}

void NetHandlers::NotifyAgentsWWR(AgentSet& sett, bool online, CAOSVar& p1, CAOSVar& p2)
{
	int event = online ? SCRIPT_NET_USER_ONLINE : SCRIPT_NET_USER_OFFLINE;

	// Make a local copy of the set - in case the script we execute changes
	// the whose wanted register (e.g. with a NET: WHOF)
	AgentSet localSett = sett;

	// Loop through each agent that is watching them, and inform him
	AgentSet::iterator itSet;
	for (itSet = localSett.begin(); itSet != localSett.end(); ++itSet)
	{
		// We make a copy of the handle, so the handle in the set isn't
		// modified by garbage collection (you can't change values in
		// sets, if it affects the ordering of them)
		AgentHandle agent = *itSet;
		if (agent.IsValid())
			theAgentManager.ExecuteScriptOnAgent(*itSet, event, COASVARAGENTNULL, p1, p2);
	}
}

// Informs all watching agents of the WWR state - we do this
// when there is a large change.  Such as going online, offline, or
// a world having just loaded.
void NetHandlers::NotifyAllWWRState()
{
	// Loop through users
	WhoseWantedMap::iterator itMap = myWhoseWantedMap.begin();
	while(itMap != myWhoseWantedMap.end())
	{
		std::string user = itMap->first;

		CAOSVar p1, p2;
		p1.SetString(user);
		NotifyAgentsWWR(itMap->second, GetNetManager().IsUserOnline(user), p1, p2);

		itMap++;
	}
}

void NetHandlers::ClearWWR()
{
	// Loop through users
	WhoseWantedMap::iterator itMap = myWhoseWantedMap.begin();
	while(itMap != myWhoseWantedMap.end())
	{
		GetNetManager().RemoveFromWWR(itMap->first);
		itMap++;
	}

	myWhoseWantedMap.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool NetHandlers::Write(CreaturesArchive &ar)
{
	ar << myWhoseWantedMap;
	ar << myHearingMap;
	ar << myNickname;
	ar << myUserID;
	return true;
}

bool NetHandlers::Read(CreaturesArchive &ar)
{
	ASSERT(myWhoseWantedMap.empty());
	ar >> myWhoseWantedMap;
	ar >> myHearingMap;
	if (ar.GetFileVersion() > 28)
	{
		std::string nickName;
		ar >> nickName;
		SetNickname(nickName, "");
		if (ar.GetFileVersion() > 31)
		{
			ar >> myUserID; 
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

int NetHandlers::PackCaosVarSize(CAOSVar& var)
{
	int type = var.GetType();
	int size = sizeof(type);

	switch (type)
	{
		case CAOSVar::typeInteger:
			size += sizeof(int);
			break;

		case CAOSVar::typeFloat:
			size += sizeof(float);
			break;

		case CAOSVar::typeString:
			{
				std::string str;
				var.GetString(str);
				int len = str.size();
				size += sizeof(int);
				size += len;
				break;
			}

		case CAOSVar::typeAgent:
			break;

		default:
			ASSERT(false);
			break;
	}

	return size;
}


void NetHandlers::PackCaosVar(NetMemoryPack& pack, CAOSVar& var)
{
	int type = var.GetType();
	pack.PackIn(&type, sizeof(type));

	switch (type)
	{
		case CAOSVar::typeInteger:
			{
				int i = var.GetInteger();
				pack.PackIn(&i, sizeof(i));
				break;
			}

		case CAOSVar::typeFloat:
			{
				float f = var.GetFloat();
				pack.PackIn(&f, sizeof(f));
				break;
			}

		case CAOSVar::typeString:
			{
				std::string str;
				var.GetString(str);				
				pack.PackIn(str);
				break;
			}

		case CAOSVar::typeAgent:
			break;

		default:
			ASSERT(false);
			break;
	}
}

CAOSVar NetHandlers::UnpackCaosVar(NetMemoryUnpack& unpack)
{
	CAOSVar var;

	// Get type header
	int type;
	if (!unpack.PackOut(&type, sizeof(type)))
	{
		ASSERT(false);
		return var;
	}
	
	switch (type)
	{
		case CAOSVar::typeInteger:
			{
				int i = 0;
				if (!unpack.PackOut(&i, sizeof(i)))
					ASSERT(false);
				var.SetInteger(i);
				break;
			}

		case CAOSVar::typeFloat:
			{
				float f = 0.0;
				if (!unpack.PackOut(&f, sizeof(f)))
					ASSERT(false);
				// Check it is a valid float
				if (isnan(f))
					f = 0.0;
				var.SetFloat(f);
				break;
			}

		case CAOSVar::typeString:
			{
				std::string str;
				unpack.PackOut(str);
				var.SetString(str);
				break;
			}

		case CAOSVar::typeAgent:
			{
				var.SetAgent(NULLHANDLE);
			}
			break;

		default:
			ASSERT(false);
			break;
	}

	return var;
}

// Post a CAOS message remotely to another computer
void NetHandlers::SubCommand_NET_WRIT(CAOSMachine& vm)
{
	std::string user;
	vm.FetchStringRV(user);
	std::string channel;
	vm.FetchStringRV(channel);
	int channelLength = channel.size();

	int msgid = vm.FetchIntegerRV();
	CAOSVar p1 = vm.FetchGenericRV();
	CAOSVar p2 = vm.FetchGenericRV();

	if (!GetNetManager().Online())
		return;

	NetMessages::BinaryMessage messageHeader;
	messageHeader.length = sizeof(messageHeader);
	messageHeader.type = NetMessages::MESG_WRIT;
	messageHeader.reserved = 0;

	int totalSize = sizeof(messageHeader) + sizeof(int) + channelLength + sizeof(int) + 
		PackCaosVarSize(p1) + PackCaosVarSize(p2);
	NetMemoryPack pack(totalSize);

	pack.PackIn(&messageHeader, sizeof(messageHeader));
	pack.PackIn(&channelLength, sizeof(channelLength));
	pack.PackIn(&channel[0], channelLength);
	pack.PackIn(&msgid, sizeof(msgid));
	PackCaosVar(pack, p1);
	PackCaosVar(pack, p2);

	GetNetManager().PostDirectMessage(user, (char*)pack.GetAddress(), totalSize);
}

void NetHandlers::SubCommand_NET_HEAR(CAOSMachine& vm)
{
	std::string channel;
	vm.FetchStringRV(channel);
	vm.ValidateTarg();

	myHearingMap[channel].insert(vm.GetTarg());

}

void NetHandlers::SubCommand_NET_HEAD( CAOSMachine& vm )
{
	std::ostream* out = vm.GetOutStream();

	// Dump our maps
	*out << myHearingMap.size() << " channels" << std::endl;
	HearingMap::iterator itMap;
	for (itMap = myHearingMap.begin(); itMap != myHearingMap.end(); ++itMap)
	{
		*out << itMap->first << ": ";
		AgentSet& set = itMap->second;
		AgentSet::iterator itSet;
		for (itSet = set.begin(); itSet != set.end(); ++itSet)
		{
			// TODO: Print unique ID rather than someone
			if (*itSet == vm.GetTarg())
				*out << "targ ";
			else
				*out << "someone ";
		}
		*out << std::endl;
	}
}

void NetHandlers::ReceiveDirectMessages(int tick)
{
	// Clear any null handles for recently destroyed agents
	if (tick > 0 && tick % 20 == 10)
	{
		// Loop through users
		HearingMap::iterator itMap = myHearingMap.begin();
		while(itMap != myHearingMap.end())
		{
			AgentSet& set = itMap->second;
			AgentSet::iterator itSet = set.begin();
			while(itSet != set.end())
			{
				// We make a copy of the handle, so the handle in the set isn't
				// modified by garbage collection (you can't change values in
				// sets, if it affects the ordering of them)
				AgentHandle agent = *itSet;
				if (agent.IsInvalid())
				{
					set.erase(itSet);
					itSet = set.begin(); // TODO: Find a way of not going back to start
				}
				else 
					++itSet;
			}
			
			if (set.empty())
			{
				myHearingMap.erase(itMap);
				itMap = myHearingMap.begin(); // TODO: Find a way of not going back to start  
			}
			else
				itMap++;
		}
	}

	// Look for hearing messages
	std::vector<std::string> users;
	std::vector<NetMessages::MessageType> types;
	std::vector<std::string> messages;
	GetNetManager().GetMessages(users, types, messages);
	ASSERT(users.size() == messages.size());
	ASSERT(users.size() == types.size());
	for (int i = 0; i < messages.size(); ++i)
	{
		// Look for MESG WRIT over the network
		if (types[i] != NetMessages::MESG_WRIT)
		{
			ASSERT(false);
			continue;
		}

		// Unpack the message
		std::string message = messages[i];
		NetMemoryUnpack unpack(&message[0], message.size());

		// Exctract CAOS message
		std::string channel;
		if (!unpack.PackOut(channel))
		{
			ASSERT(false);
			continue;
		}
		int msgid = -1;
		if (!unpack.PackOut(&msgid, sizeof(msgid)))
		{
			ASSERT(false);
			continue;
		}
		CAOSVar p1 = UnpackCaosVar(unpack);
		CAOSVar p2 = UnpackCaosVar(unpack);

		// Give them the actual user that the message came from
		CAOSVar from;
		from.SetString(users[i]);
					
		// See if they are in our hearing channel map
		HearingMap::iterator it = myHearingMap.find(channel);
		if (it != myHearingMap.end())
		{
			AgentSet& sett = it->second;
			AgentSet::iterator itSet;
			for (itSet = sett.begin(); itSet != sett.end(); ++itSet)
			{
				// We make a copy of the handle, so the handle in the set isn't
				// modified by garbage collection (you can't change values in
				// sets, if it affects the ordering of them)
				AgentHandle agent = *itSet;
				if (agent.IsValid())
				{
					theAgentManager.ExecuteScriptOnAgent(*itSet, msgid, from, p1, p2);
				}
			}
		}
	}
}

void NetHandlers::Twinned(AgentHandle original, AgentHandle clone)
{
	// A twin should be able to hear the same as the original can

	// Whose wanted register
	WhoseWantedMap::iterator itWhose;
	for (itWhose = myWhoseWantedMap.begin(); itWhose != myWhoseWantedMap.end(); ++itWhose)
	{
		AgentSet& set = itWhose->second;
		if (set.find(original) != set.end())
			set.insert(clone);
	}

	// Direct connection hearing list
	HearingMap::iterator itHearing = myHearingMap.begin();
	for (itHearing = myHearingMap.begin(); itHearing != myHearingMap.end(); ++itHearing)
	{
		AgentSet& set = itHearing->second;
		if (set.find(original) != set.end())
			set.insert(clone);
	}
}

////////////////////////////////////////////////////////////////////////////////

void NetHandlers::SubCommand_NET_RUSO(CAOSMachine& vm)
{
	bool block;
	std::string str = GetNetManager().DSFetchRandomUser(block);

	if (block)
		vm.Block(); // this function will get called again next tick, to see if the result has arrived
	else
	{
		vm.UnBlock();
		CAOSVar& result = vm.FetchVariable();
		result.SetString(str);
	}
}

////////////////////////////////////////////////////////////////////////////////


void NetHandlers::Init()
{
	myPasswordConfig.BindToFile("password.cfg");
	myServerConfig.BindToFile("server.cfg");

	// Read in initial servers
	int n = 0;
	myServerConfig.Get("Server Count", n);
	std::vector<CBabelServerInfo> servers;
	servers.resize(n);
	for (int i = 0; i < n; ++i)
	{
		std::string host, friendlyName;
		int port, id;
		{
			std::ostringstream out;
			out << "Server " << i << " Host";
			myServerConfig.Get(out.str(), host);
		}
		{
			std::ostringstream out;
			out << "Server " << i << " Port";
			myServerConfig.Get(out.str(), port);
		}
		{
			std::ostringstream out;
			out << "Server " << i << " ID";
			myServerConfig.Get(out.str(), id);
		}
		{
			std::ostringstream out;
			out << "Server " << i << " FriendlyName";
			myServerConfig.Get(out.str(), friendlyName);
		}

		servers[i].SetServer(host, friendlyName, port, id);
	}

	// Let Babel default to its own things
	if (n > 0)
		GetNetManager().SetServerList(servers);
}

void NetHandlers::WriteServerList()
{
	std::vector<CBabelServerInfo> servers;
	GetNetManager().GetServerList(servers);

	if (servers.size() > 0)
	{
		int n = servers.size();
		myServerConfig.Set("Server Count", n);
		for (int i = 0; i < n; ++i)
		{
			{
				std::ostringstream out;
				out << "Server " << i << " Host";
				myServerConfig.Set(out.str(), *servers[i].GetServer());
			}
			{
				std::ostringstream out;
				out << "Server " << i << " Port";
				myServerConfig.Set(out.str(), servers[i].GetPort());
			}
			{
				std::ostringstream out;
				out << "Server " << i << " ID";
				myServerConfig.Set(out.str(), servers[i].GetID());
			}
			{
				std::ostringstream out;
				out << "Server " << i << " FriendlyName";
				myServerConfig.Set(out.str(), *servers[i].GetNotes());
			}
		}
		myServerConfig.Flush();
	}
}

