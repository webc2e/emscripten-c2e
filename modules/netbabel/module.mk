# -*- Makefile -*- module for Engine
# $Id: module.mk,v 1.3 2001/04/10 17:54:11 firving Exp $

NETBABEL_SOURCES := modules/netbabel/NetHandlers.cpp \
					modules/netbabel/NetLogImplementation.cpp \
					modules/netbabel/NetworkImplementation.cpp \
					../Babel/BabelCloak/DSNetManager.cpp \
					../Babel/BabelCloak/MessageThread.cpp \
					../Babel/BabelCloak/NetDebugListUsers.cpp \
					../Babel/BabelCloak/NetDirectLink.cpp \
					../Babel/BabelCloak/NetManager.cpp \
					../Babel/BabelCloak/NetMemoryPack.cpp \
					../Babel/BabelCloak/NetMemoryUnpack.cpp \
					../Babel/BabelCloak/NetUtilities.cpp \
					../Babel/BabelCloak/QueuedMessage.cpp \
					../Babel/BabelClient/BabelClient.cpp \
					../Babel/BabelClient/BabelClientPeerLink.cpp \
					../Babel/BabelCommon/BabelAsyncCore.cpp \
					../Babel/BabelCommon/BabelCriticalSection.cpp \
					../Babel/BabelCommon/BabelDebugMemory.cpp \
					../Babel/BabelCommon/BabelRawSocket.cpp \
					../Babel/BabelCommon/BabelSystemSocket.cpp \
					../Babel/BabelCommon/BabelSystemThread.cpp \
					../Babel/BabelCommon/BabelSystemUnicode.cpp \
					../Babel/BabelCommon/BabelTrace.cpp \
					../Babel/BabelCommon/BabelUserDataObject.cpp \
					../Babel/BabelClient/BabelConnection.cpp \
					../Babel/BabelClient/BabelDConnection.cpp \
					../Babel/BabelClient/BabelRadioLink.cpp \
					../Babel/BabelClient/BabelRemoteSwitch.cpp \
					../Babel/BabelClient/BabelSocket.cpp \
					../Babel/BabelClient/BabelUDPConnection.cpp \
					../Babel/BabelClient/BabelVirtualSocket.cpp \
					../Babel/BabelClient/StdAfx.cpp \
					server/HistoryFeed/HistoryTransferOut.cpp

lc2e-netbabel.so: $(patsubst %.cpp, %.o, $(NETBABEL_SOURCES))
lc2e-netbabel.do: $(patsubst %.cpp, %.od, $(NETBABEL_SOURCES))
lc2e-netbabel.ho: $(patsubst %.cpp, %.oh, $(NETBABEL_SOURCES))


