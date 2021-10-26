// For the Babel networking module, provides
// implementation of the engine's module interface

#ifndef NETWORK_IMPLEMENTATION_H
#define NETWORK_IMPLEMENTATION_H

#include "../NetworkInterface.h"
#include <SimpleMutex.h>
#include "../../engine/Creature/History/HistoryStore.h"
#include "NetLogImplementation.h"
#include <DSNetManager.h>

#ifdef _WIN32
	#define OUR_EXPORT __declspec(dllexport)
#else
	#define OUR_EXPORT
#endif

extern "C"
{
	OUR_EXPORT ModuleInterface* CreaturesEngineModuleInterface(void);
	OUR_EXPORT int CreaturesEngineModuleInterfaceVersion(void);
};

// Major version not used by version interface yet
#define MAJOR_VER 0
#define MINOR_VER 148

class NetworkImplementation : public NetworkInterface
{
public:
	NetworkImplementation();

	// From ModuleInterface.h	
	virtual void ModuleName(std::string& name);
	virtual void AddCAOSCommands(CAOSDescription& caosDescription);
	virtual void Shutdown();
	virtual int ModuleVersion();

	virtual void Update(int tick);
	virtual void WorldEnding();
	virtual void WorldLoaded();

	virtual bool Write(CreaturesArchive &ar);
	virtual bool Read(CreaturesArchive &ar);
	virtual bool Persistent();

	virtual void Twinned(AgentHandle original, AgentHandle clone);

	// From NetworkInterface.h
	virtual void SetDirectories(const std::string& expo, const std::string& import);
	virtual std::string GetUserId();
	virtual std::string GetNickname();

	virtual void HistoryFeed(HistoryStore& store);

	// Returns false if can't fill in the user's ID in the life event history 
	// because we don't know it (they haven't logged on).
	virtual bool HistoryFillInUserID(HistoryStore& store, const std::string& moniker);

	// This should really be private - only called by HistoryQueuedMessage
	void HistoryUploadComplete(bool result);
	DSNetManager* GetNetManager();

private:
	void NotifyAgentsWhetherOnline(bool force);

	bool myPreviousOnlineState;
	std::string myScanningMoniker;

	// History uploading
	SimpleMutex myHistoryUploadMutex;
	enum HistoryResult
	{
		HISTORY_NONE,
		HISTORY_UPLOADING,
		HISTORY_DONE_OK,
	};
	HistoryResult myHistoryResult;
	std::string myHistoryMoniker;
	CreatureHistory myUploadedHistory;

	NetLogImplementation* ourNetLogger;
	DSNetManager* ourNetManager;
};

#endif

