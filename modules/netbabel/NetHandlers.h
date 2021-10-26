#ifndef NET_HANDLERS_H
#define NET_HANDLERS_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

class CAOSMachine;
class CAOSDescription;
class DSNetManager;
class CAOSVar;
class NetMemoryPack;
class NetMemoryUnpack;
#include <string>
#include <map>
#include <set>

#include "../../engine/Agents/AgentHandle.h"
#include "../../common/Configurator.h"

class NetHandlers
{
public:
	static void Command_NET( CAOSMachine& vm );
	static int IntegerRV_NET( CAOSMachine& vm );
	static void StringRV_NET( CAOSMachine& vm, std::string& str );

	static void PushTables(CAOSDescription& caosDescription);

	static void SetNetManager(DSNetManager* netManager);
	static std::string GetNickname();

	static void NotifyUsersOnline(int tick = -1);
	static void ClearWWR();
	static void NotifyAllWWRState();

	static void ReceiveDirectMessages(int tick = -1);

	static bool Write(CreaturesArchive &ar);
	static bool Read(CreaturesArchive &ar);
	static void Twinned(AgentHandle original, AgentHandle clone);

	static void SetNickname(std::string nickName, std::string password);
	static std::string GetCachedUserID();

	static void Init();

private:
	static DSNetManager& GetNetManager();

	static int IntegerRV_NET_REGI( CAOSMachine& vm );

	static void SubCommand_NET_LINE( CAOSMachine& vm );
	static int IntegerRV_NET_LINE( CAOSMachine& vm );
	static void StringRV_NET_USER( CAOSMachine& vm, std::string& str );
	static void SubCommand_NET_PASS( CAOSMachine& vm );
	static void StringRV_NET_PASS( CAOSMachine& vm, std::string& str );

	static int IntegerRV_NET_ULIN( CAOSMachine& vm );
	static void SubCommand_NET_UNIK( CAOSMachine& vm);
	static void StringRV_NET_FROM( CAOSMachine& vm, std::string& str );
	static int IntegerRV_NET_ERRA( CAOSMachine& vm );
	static int IntegerRV_NET_RAWE( CAOSMachine& vm );

	static void SubCommand_NET_STAT( CAOSMachine& vm );
	static void StringRV_NET_HOST( CAOSMachine& vm, std::string& str );

	static int IntegerRV_NET_EXPO( CAOSMachine& vm );
	static int IntegerRV_NET_MAKE( CAOSMachine& vm );

	static void SubCommand_NET_WHON( CAOSMachine& vm );
	static void SubCommand_NET_WHOF( CAOSMachine& vm );
	static void SubCommand_NET_WHOD( CAOSMachine& vm );
	static void SubCommand_NET_WHOZ( CAOSMachine& vm );

	static void SubCommand_NET_WRIT(CAOSMachine& vm);
	static void SubCommand_NET_HEAR(CAOSMachine& vm);
	static void SubCommand_NET_HEAD( CAOSMachine& vm );
	
	static void SubCommand_NET_RUSO( CAOSMachine& vm);

	static void StringRV_NET_WHAT(CAOSMachine& vm, std::string& str);

	// Data
	typedef std::set< AgentHandle > AgentSet;
	typedef std::map< std::string, AgentSet > WhoseWantedMap;
	static WhoseWantedMap myWhoseWantedMap;
	
	typedef std::map< std::string, AgentSet > HearingMap;
	static HearingMap myHearingMap;

	static std::string myNickname;
	static std::string myPassword;
	static std::string myUserID; // returned by the Babel server

	static Configurator myPasswordConfig;
	static Configurator myServerConfig;

	// Helper functions
	static void NotifyAgentsWWR(AgentSet& sett, bool online, CAOSVar& p1, CAOSVar& p2);

	static int PackCaosVarSize(CAOSVar& var);
	static void PackCaosVar(NetMemoryPack& pack, CAOSVar& var);
	static CAOSVar UnpackCaosVar(NetMemoryUnpack& unpack);

	static void WriteServerList();
};

#endif //NET_HANDLERS_H

