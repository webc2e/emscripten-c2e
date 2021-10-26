# Microsoft Developer Studio Project File - Name="netbabel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=netbabel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "netbabel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "netbabel.mak" CFG="netbabel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "netbabel - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "netbabel - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "netbabel - Win32 Release with symbols" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "netbabel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETBABEL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /Oy- /I "../../../Babel/BabelClient" /I "../../../Babel/BabelCommon" /I "../../../c2e/common" /I "../../../Babel/BabelCloak" /D "O_WIN32" /D "_USRDLL" /D "NDEBUG" /D CLIENTVERSION=1 /D PRODUCTCODE=2 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_WIN32" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 wininet.lib ..\..\engine\Release\engine.lib msvcrt.lib msvcprt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /machine:I386 /out:"Release/engine-netbabel.dll"

!ELSEIF  "$(CFG)" == "netbabel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NETBABEL_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../Babel/BabelClient" /I "../../../Babel/BabelCommon" /I "../../../c2e/common" /I "../../../Babel/BabelCloak" /D "_DEBUG" /D "O_WIN32" /D CLIENTVERSION=1 /D PRODUCTCODE=2 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wininet.lib ..\..\engine\engine___Win32_SDL_Debug\engine.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /debug /machine:I386 /out:"c:\program files\docking station\engineDebug-netbabel.dll" /pdbtype:sept /libpath:"."
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy "Debug\engineDebug-netbabel.dll" "c:\program files\docking station\engineDebug-netbabel.dll"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "netbabel - Win32 Release with symbols"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "netbabel___Win32_Release_with_symbols"
# PROP BASE Intermediate_Dir "netbabel___Win32_Release_with_symbols"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "netbabel___Win32_Release_with_symbols"
# PROP Intermediate_Dir "netbabel___Win32_Release_with_symbols"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /Oy- /I "../../../Babel/BabelClient" /I "../../../Babel/BabelCommon" /I "../../../c2e/common" /I "../../../Babel/BabelCloak" /D "NDEBUG" /D "O_WIN32" /D CLIENTVERSION=1 /D PRODUCTCODE=2 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /Oy- /I "../../../Babel/BabelClient" /I "../../../Babel/BabelCommon" /I "../../../c2e/common" /I "../../../Babel/BabelCloak" /D "NDEBUG" /D "O_WIN32" /D CLIENTVERSION=1 /D PRODUCTCODE=2 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wininet.lib ..\..\engine\Release\engine.lib msvcrt.lib msvcprt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /machine:I386 /out:"Release/engine-netbabel.dll"
# ADD LINK32 wininet.lib ..\..\engine\Release\engine.lib msvcrt.lib msvcprt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /debug /machine:I386 /out:"c:\program files\docking station/engine-netbabel.dll"

!ENDIF 

# Begin Target

# Name "netbabel - Win32 Release"
# Name "netbabel - Win32 Debug"
# Name "netbabel - Win32 Release with symbols"
# Begin Group "Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ModuleInterface.h
# End Source File
# Begin Source File

SOURCE=..\NetworkInterface.h
# End Source File
# End Group
# Begin Group "Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\NetHandlers.h
# End Source File
# Begin Source File

SOURCE=.\NetLogImplementation.cpp
# End Source File
# Begin Source File

SOURCE=.\NetLogImplementation.h
# End Source File
# Begin Source File

SOURCE=.\NetworkImplementation.cpp

!IF  "$(CFG)" == "netbabel - Win32 Release"

!ELSEIF  "$(CFG)" == "netbabel - Win32 Debug"

# ADD CPP /GR

!ELSEIF  "$(CFG)" == "netbabel - Win32 Release with symbols"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkImplementation.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\FileFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\FileScanner.cpp
# End Source File
# End Group
# Begin Group "BabelCloak"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\DSNetManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\DSNetManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\DSNetMessages.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\MessageThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\MessageThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\MessageThread.inl
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetDebugListUsers.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetDebugListUsers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetDirectLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetDirectLink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetLogInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetManager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetMemoryPack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetMemoryPack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetMemoryUnpack.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetMemoryUnpack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetMessages.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetUtilities.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\NetUtilities.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\QueuedMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\QueuedMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\QueuedMessageClient.cpp

!IF  "$(CFG)" == "netbabel - Win32 Release"

# ADD CPP /I "..\..\..\Tech\Base\\"

!ELSEIF  "$(CFG)" == "netbabel - Win32 Debug"

!ELSEIF  "$(CFG)" == "netbabel - Win32 Release with symbols"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\QueuedMessageClient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\QueuedMessageClient.inl
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Babel\BabelCloak\Thread.inl
# End Source File
# End Group
# Begin Group "HistoryFeed"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\server\HistoryFeed\HistoryTransferOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\server\HistoryFeed\HistoryTransferOut.h
# End Source File
# End Group
# Begin Group "Tech"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Tech\Base\Common\Exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Tech\Base\Common\win32\win32_Debug.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChangeLog.txt
# End Source File
# Begin Source File

SOURCE=.\Catalogues\NetBabel.catalogue
# End Source File
# End Target
# End Project
