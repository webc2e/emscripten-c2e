// NetworkInterface.h

// Interface that the engine uses to communicate with 
// external networking modules

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "ModuleInterface.h"

class HistoryStore;

class NetworkInterface : public ModuleInterface
{
public:
	virtual bool IsNetworkInterface() { return true; }

	virtual void SetDirectories(const std::string& expo, const std::string& import) = 0;
	virtual std::string GetUserId() = 0;
	virtual std::string GetNickname() = 0;

	virtual void HistoryFeed(HistoryStore& store) = 0;
	virtual bool HistoryFillInUserID(HistoryStore& store, const std::string& moniker) = 0;
};


#endif

