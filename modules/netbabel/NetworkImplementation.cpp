#ifdef _MSC_VER
#pragma warning(disable : 4786 4503)
#endif // _MSC_VER

#include "NetworkImplementation.h"
#include "../../engine/AgentManager.h"
#include "../../engine/Agents/AgentConstants.h"
#include "../../server/HistoryFeed/HistoryTransferOut.h"
#include "NetHandlers.h"
#include <DSNetManager.h>
#include <QueuedMessage.h>

#ifndef MODULE_EMBEDDED
// TODO: These shouldn't be here, might get confused with the ones in the main
// app
AgentHandle C2E_MODULE_API NULLHANDLE;
CAOSVar C2E_MODULE_API INTEGERZERO;
CAOSVar C2E_MODULE_API COASVARAGENTNULL;
#endif

NetworkImplementation ourImplementation;

extern "C" {
OUR_EXPORT ModuleInterface *CreaturesEngineModuleInterface(void) {
  return &ourImplementation;
}
OUR_EXPORT int CreaturesEngineModuleInterfaceVersion(void) {
  return ModuleInterface::InterfaceVersion();
}
}

NetworkImplementation::NetworkImplementation()
    : myPreviousOnlineState(false), myHistoryResult(HISTORY_NONE) {
  ourNetLogger = new NetLogImplementation;
  ourNetManager = new DSNetManager(ourNetLogger);
  NetHandlers::SetNetManager(ourNetManager);
}

void NetworkImplementation::Shutdown() {
  delete ourNetManager;
  delete ourNetLogger;
}

void NetworkImplementation::ModuleName(std::string &name) { name = "netbabel"; }

int NetworkImplementation::ModuleVersion() { return MINOR_VER; }

void NetworkImplementation::AddCAOSCommands(CAOSDescription &caosDescription) {
  NetHandlers::PushTables(caosDescription);

  NetHandlers::Init();
}

void NetworkImplementation::SetDirectories(const std::string &expo,
                                           const std::string &import) {
  ourNetManager->SetOutboxDirectory(expo);
  ourNetManager->SetInboxDirectory(import);
}

std::string NetworkImplementation::GetUserId() {
  return NetHandlers::GetCachedUserID();
}

std::string NetworkImplementation::GetNickname() {
  return NetHandlers::GetNickname();
}

void NetworkImplementation::NotifyAgentsWhetherOnline(bool force) {
  bool onlineNow = ourNetManager->Online();
  if (force || myPreviousOnlineState != onlineNow) {
    int script = onlineNow ? SCRIPT_NET_ONLINE : SCRIPT_NET_OFFLINE;
    theAgentManager.ExecuteScriptOnAllAgents(script, COASVARAGENTNULL,
                                             INTEGERZERO, INTEGERZERO);
    myPreviousOnlineState = onlineNow;

    // If we've just gone offline, send agents using WWR messages
    if (!force && !onlineNow)
      NetHandlers::NotifyAllWWRState();
  }
}

void NetworkImplementation::Update(int tick) {
  ourNetManager->SendOrdinaryMessages();
  NetHandlers::ReceiveDirectMessages(tick);
  NotifyAgentsWhetherOnline(false);
  NetHandlers::NotifyUsersOnline(tick);
}

void NetworkImplementation::WorldLoaded() {
  NotifyAgentsWhetherOnline(true);

  // This will be that everyone is offline!
  // (worlds are loaded with the game offline)
  NetHandlers::NotifyAllWWRState();
}

void NetworkImplementation::WorldEnding() {
  GetNetManager()->Disconnect();
  NetHandlers::ClearWWR();
  NetHandlers::SetNickname("", "none");
}

bool NetworkImplementation::Write(CreaturesArchive &ar) {
  return NetHandlers::Write(ar);
}

bool NetworkImplementation::Read(CreaturesArchive &ar) {
  return NetHandlers::Read(ar);
}

bool NetworkImplementation::Persistent() { return true; }

void NetworkImplementation::Twinned(AgentHandle original, AgentHandle clone) {
  NetHandlers::Twinned(original, clone);
}

bool NetworkImplementation::HistoryFillInUserID(HistoryStore &store,
                                                const std::string &moniker) {
  std::string userID = GetUserId();

  // If there is no user ID, then they have never logged in with
  // this world (since user name was last changed).
  if (userID.empty())
    return false;

  // Loop through events, filling in user ID
  CreatureHistory &history = store.GetCreatureHistory(moniker);
  for (int i = 0; i < history.Count(); ++i) {
    LifeEvent &event = *history.GetLifeEvent(i);
    if (event.myNetworkUser.empty())
      event.myNetworkUser = userID;
  }

  return true;
}

// Helper class used just by NetworkImplementation
class HistoryQueuedMessage : public QueuedMessage {
public:
  HistoryQueuedMessage(const std::string &message,
                       NetworkImplementation *networkImplementation,
                       const std::string &scanningMoniker, std::string name)
      : myMessage(message), myNetworkImplementation(networkImplementation) {
    if (name == "")
      name = "<unnamed>";
    SetDebugText("Uploading history for " + name + " " + scanningMoniker);
  }

  void Send(const MessageThread &messageThread) const {
    bool result = myNetworkImplementation->GetNetManager()->DSFeedHistory(
        (char *)myMessage.data(), myMessage.size());
    myNetworkImplementation->HistoryUploadComplete(result);
  }

private:
  std::string myMessage;
  NetworkImplementation *myNetworkImplementation;
};

void NetworkImplementation::HistoryFeed(HistoryStore &store) {
  // Look for existing history being uploaded...
  // If we're in mid-upload, don't send anything else
  // SimpleMutexLock lock(myHistoryUploadMutex);
  if (myHistoryResult == HISTORY_UPLOADING)
    return;

  // A previous upload has completed - store that it
  // finished OK so we don't upload the same history again
  if (myHistoryResult == HISTORY_DONE_OK) {
    // We got a response, so we can clear the history and
    // life events flags as we don't need to send them again.
    CreatureHistory &history = store.GetCreatureHistory(myHistoryMoniker);
    history.myNetworkNeedUploading = false;
    if (myUploadedHistory.Count() <= history.Count()) {
      for (int i = 0; i < myUploadedHistory.Count(); ++i) {
        std::string uploadedUserText =
            myUploadedHistory.GetLifeEvent(i)->myUserText;

        LifeEvent *event = history.GetLifeEvent(i);
        event->myNetworkNeedUploading = false;
        event->myUploadedUserText = uploadedUserText;
      }
    } else
      ASSERT(false); // this should only happen if they HIST WIPE during event
                     // upload!

    history.myUploadedName = myUploadedHistory.myName;

    myHistoryMoniker = "";
    myHistoryResult = HISTORY_NONE;
  }

  // Now upload the next piece of history...
  ASSERT(myHistoryResult == HISTORY_NONE);

  // Find next creature
  myScanningMoniker = store.Next(myScanningMoniker);
  if (myScanningMoniker.empty())
    return;

  // Must be online to upload history
  if (!ourNetManager->Online())
    return;

  CreatureHistory &history = store.GetCreatureHistory(myScanningMoniker);

  // See if they have ever warped (otherwise we don't track them)
  if (!history.myWarpHoleVeteran)
    return;

  // See if they have new life events (note that we
  // only upload new user text and name changes if there is a
  // life event - this avoids conflict of names)
  bool needed = false;
  if (history.myNetworkNeedUploading)
    needed = true;
  else {
    for (int i = 0; i < history.Count(); ++i) {
      LifeEvent *event = history.GetLifeEvent(i);
      if (!event->CreaturePresentForEvent()) {
        // We only upload events that
        // actually happen to the creature
        // e.g. we upload Born, but not
        // Your Child Was Born.
        event->myNetworkNeedUploading = false;
      } else if (event->myNetworkNeedUploading) {
        needed = true;
        break;
      }
    }
  }

  if (!needed)
    return;

  // Life events refer to the user ID where they happened -
  // make sure the values are filled in before sending the data.
  if (!HistoryFillInUserID(store, myScanningMoniker)) {
    // We are meant to be online, so this should never fail!
    // (except rare timing, where we went offline during the
    // above short stretch of code)
    ASSERT(false);
    return;
  }

  // We have some new stuff, so send it
  HistoryTransferOut out;
  out.WriteMoniker(history.myMoniker);

  // Upload their main data if needed
  if (history.myNetworkNeedUploading)
    out.WriteCreatureHistory(history);

  // Upload all new life events which are solid
  {
    for (CreatureHistory::SolidIndex i; i < history.SolidCount(); ++i.value) {
      LifeEvent &event = *history.GetSolidLifeEvent(i);
      if (event.myNetworkNeedUploading) {
        ASSERT(!event.myNetworkUser.empty());
        out.WriteLifeEvent(event, i.value);
      }
    }
  }

  // Upload name if it has changed
  if (history.myName != history.myUploadedName)
    out.WriteName(history.myName);

  // Upload changed user text
  {
    for (CreatureHistory::SolidIndex i; i < history.SolidCount(); ++i.value) {
      const LifeEvent &event = *history.GetSolidLifeEvent(i);
      if (event.myUserText != event.myUploadedUserText)
        out.WriteUserText(i.value, event.myUserText);
    }
  }

  // Check things are OK
  const void *address = out.GetAddress();
  int len = out.GetLength();

  if (!address || len <= 0)
    throw BasicException("Internal error formatting history for upload");

  // Asynchronously upload history..

  // ... remember what we are in the process of sending
  myHistoryResult = HISTORY_UPLOADING;
  myHistoryMoniker = myScanningMoniker;
  myUploadedHistory = history;

  // ... set it going
  HistoryQueuedMessage *queueMessage =
      new HistoryQueuedMessage(std::string((char *)address, len), this,
                               myScanningMoniker, history.myName);
  ourNetManager->QueueMessage(queueMessage);
  return;
}

void NetworkImplementation::HistoryUploadComplete(bool result) {
  // SimpleMutexLock lock(myHistoryUploadMutex);
  myHistoryResult = result ? HISTORY_DONE_OK : HISTORY_NONE;
}

DSNetManager *NetworkImplementation::GetNetManager() { return ourNetManager; }
