# Microsoft Developer Studio Project File - Name="engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=engine - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak" CFG="engine - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "engine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Release with Debug Symbols" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 No optimize debug symbols" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 SDL Debug" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Direct Display Debug" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 SDL Release" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Direct Display Release" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Direct Display Release for profiling" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "engine - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Oy- /I "c:/mssdk/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FR /YX /Zl /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 zdll.lib dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib imagehlp.lib msvcrt.lib msvcprt.lib /nologo /subsystem:windows /profile /map /machine:I386 /nodefaultlib:"libc.lib" /out:"r:\Creatures Exodus\Docking Station\engine.exe" /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "c:/mssdk/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zdll.lib imagehlp.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"r:\Creatures Exodus\Docking Station\engine.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP BASE Intermediate_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP Intermediate_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /GR /GX /O2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gm /Gi /GR /GX /Zi /O2 /Ob2 /I "c:/mssdk/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /machine:I386
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zdll.lib imagehlp.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"r:\Creatures Exodus\Docking Station\engine.exe"
# SUBTRACT LINK32 /verbose

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP BASE Intermediate_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP Intermediate_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Gm /Gi /GR /GX /Zi /O2 /Ob2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /Gm /Gi /GR /GX /Zi /Od /Ob2 /I "c:/mssdk/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# SUBTRACT BASE LINK32 /verbose
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# SUBTRACT LINK32 /verbose

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "engine___Win32_SDL_Debug"
# PROP BASE Intermediate_Dir "engine___Win32_SDL_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "engine___Win32_SDL_Debug"
# PROP Intermediate_Dir "engine___Win32_SDL_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "c:/mssdk/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "c:/mssdk/include" /D "C2E_SDL" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# ADD LINK32 SDL_mixer.lib sdl.lib sdlmain.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "engine___Win32_Direct_Display_Debug"
# PROP BASE Intermediate_Dir "engine___Win32_Direct_Display_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "engine___Win32_Direct_Display_Debug"
# PROP Intermediate_Dir "engine___Win32_Direct_Display_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "c:/mssdk/include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "c:/mssdk/include" /I "../DisplayLibrary/Direct Display Engine/" /D "C2D_DIRECT_DISPLAY_LIB" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib /nologo /stack:0x3d0900 /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libc" /pdbtype:sept /FIXED:No
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_SDL_Release"
# PROP BASE Intermediate_Dir "engine___Win32_SDL_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_SDL_Release"
# PROP Intermediate_Dir "engine___Win32_SDL_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /GR /GX /O2 /Oy- /I "c:/mssdk/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Oy- /I "c:/mssdk/include" /D "C2E_SDL" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FR /YX /Zl /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib /nologo /subsystem:windows /profile /map /machine:I386 /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS
# ADD LINK32 msvcrt.lib SDL_mixer.lib sdl.lib sdlmain.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib /nologo /subsystem:windows /profile /map /machine:I386 /nodefaultlib:"libc.lib" /out:"engine___Win32_SDL_Release/engineSDL.exe" /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_Direct_Display_Release0"
# PROP BASE Intermediate_Dir "engine___Win32_Direct_Display_Release0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_Direct_Display_Release0"
# PROP Intermediate_Dir "engine___Win32_Direct_Display_Release0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /GR /GX /O2 /Oy- /I "c:/mssdk/include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Oy- /I "c:/mssdk/include" /I "../DisplayLibrary/Direct Display Engine/" /D "C2D_DIRECT_DISPLAY_LIB" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FR /YX /Zl /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib /nologo /subsystem:windows /profile /map /machine:I386 /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib msvcrt.lib msvcprt.lib /nologo /stack:0x3d0900 /subsystem:windows /profile /map /machine:I386 /nodefaultlib:"libc.lib" /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_Direct_Display_Release_for_profiling"
# PROP BASE Intermediate_Dir "engine___Win32_Direct_Display_Release_for_profiling"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_Direct_Display_Release_for_profiling"
# PROP Intermediate_Dir "engine___Win32_Direct_Display_Release_for_profiling"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Oy- /I "c:/mssdk/include" /I "../DisplayLibrary/Direct Display Engine/" /D "C2D_DIRECT_DISPLAY_LIB" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FR /YX /Zl /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /Zi /O2 /Oy- /I "c:/mssdk/include" /I "../DisplayLibrary/Direct Display Engine/" /D "C2D_DIRECT_DISPLAY_LIB" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /D "PRAY_INTEGRITY_CHECK" /FR /YX /Zl /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib msvcrt.lib msvcprt.lib /nologo /stack:0x3d0900 /subsystem:windows /profile /map /machine:I386 /nodefaultlib:"libc.lib" /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib zlib.lib imagehlp.lib msvcrt.lib msvcprt.lib /nologo /stack:0x3d0900 /subsystem:windows /profile /map /debug /machine:I386 /nodefaultlib:"libc.lib" /MAPINFO:EXPORTS /MAPINFO:LINES /MAPINFO:FIXUPS /FIXED:No
# SUBTRACT LINK32 /force

!ENDIF 

# Begin Target

# Name "engine - Win32 Release"
# Name "engine - Win32 Debug"
# Name "engine - Win32 Release with Debug Symbols"
# Name "engine - Win32 No optimize debug symbols"
# Name "engine - Win32 SDL Debug"
# Name "engine - Win32 Direct Display Debug"
# Name "engine - Win32 SDL Release"
# Name "engine - Win32 Direct Display Release"
# Name "engine - Win32 Direct Display Release for profiling"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\c3.ico
# End Source File
# Begin Source File

SOURCE=.\Debug\c3.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# Begin Group "Catalogue Files"

# PROP Default_Filter "cat;txt"
# Begin Source File

SOURCE=.\Catalogues\Brain.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\CAOS.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\Norn.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\System.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\voices.catalogue
# End Source File
# End Group
# Begin Group "Creature"

# PROP Default_Filter ""
# Begin Group "Brain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\Brain\Brain.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Brain.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainComponent.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainConstants.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainIO.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainScriptFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainScriptFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Dendrite.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Dendrite.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Instinct.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Instinct.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Lobe.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Lobe.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Neuron.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Neuron.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\SVRule.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\SVRule.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Tract.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Tract.h
# End Source File
# End Group
# Begin Group "Biochemistry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\Biochemistry\Biochemistry.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Biochemistry.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\BiochemistryConstants.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\BiochemistryConstants.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Chemical.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Chemical.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\ChemicallyActive.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Emitter.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\NeuroEmitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\NeuroEmitter.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Organ.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Organ.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Reaction.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Receptor.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Receptor.h
# End Source File
# End Group
# Begin Group "Agents"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\BodyPartOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\BodyPartOverlay.h
# End Source File
# Begin Source File

SOURCE=.\creature\Creature.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\Creature.h
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureAgent.h
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureConstants.h
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureHead.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureHead.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Definitions.h
# End Source File
# Begin Source File

SOURCE=.\Entity.cpp
# End Source File
# Begin Source File

SOURCE=.\Entity.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Genome.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Genome.h
# End Source File
# Begin Source File

SOURCE=.\Creature\GenomeStore.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\GenomeStore.h
# End Source File
# Begin Source File

SOURCE=.\Creature\SkeletalCreature.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\SkeletalCreature.h
# End Source File
# Begin Source File

SOURCE=.\Creature\SkeletonConstants.h
# End Source File
# Begin Source File

SOURCE=.\Stimulus.cpp
# End Source File
# Begin Source File

SOURCE=.\Stimulus.h
# End Source File
# Begin Source File

SOURCE=.\Token.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Vocab.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Vocab.h
# End Source File
# End Group
# Begin Group "Faculty"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\AgentFacultyInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\AgentFacultyInterface.h
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureFacultyInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureFacultyInterface.h
# End Source File
# Begin Source File

SOURCE=.\creature\ExpressiveFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\ExpressiveFaculty.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Faculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\Faculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\LifeFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\LifeFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\LinguisticFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\LinguisticFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\MotorFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\MotorFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\MusicFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\MusicFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\ReproductiveFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\ReproductiveFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\SensoryFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\SensoryFaculty.h
# End Source File
# End Group
# Begin Group "History"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\History\CreatureHistory.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\History\CreatureHistory.h
# End Source File
# Begin Source File

SOURCE=.\Caos\HistoryHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\HistoryHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Creature\History\HistoryStore.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\History\HistoryStore.h
# End Source File
# Begin Source File

SOURCE=.\Creature\History\LifeEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\History\LifeEvent.h
# End Source File
# End Group
# End Group
# Begin Group "Handlers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Caos\AgentHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\AgentHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CompoundHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CompoundHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CreatureHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CreatureHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\DisplayHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\DisplayHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\GeneralHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\GeneralHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\MapHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\MapHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\PortHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\PortHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\SoundHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\SoundHandlers.h
# End Source File
# End Group
# Begin Group "Display"

# PROP Default_Filter ""
# Begin Group "SDL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Display\Sdl\SDL_DisplayEngine.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\Sdl\SDL_DisplayEngine.h

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\Sdl\SDL_Main.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\Sdl\SDL_Main.h

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "DirectX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Display\DirectX\DisplayEngine.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\DirectX\DisplayEngine.h

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\DirectX\ErrorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\DirectX\ErrorDialog.h
# End Source File
# Begin Source File

SOURCE=.\Display\DirectX\Window.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\DirectX\Window.h

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Display\Background.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Background.h
# End Source File
# Begin Source File

SOURCE=.\Display\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\Display\ClonedGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\ClonedGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedBitmap.h
# End Source File
# Begin Source File

SOURCE=.\Display\CreatureGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\CreatureGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\DisplayEnginePlotFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\display\DisplayEnginePlotFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Display\DisplayErrorConstants.h
# End Source File
# Begin Source File

SOURCE=.\display\DrawableObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\DrawableObject.h
# End Source File
# Begin Source File

SOURCE=.\Display\DrawableObjectHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\DrawableObjectHandler.h
# End Source File
# Begin Source File

SOURCE=.\Display\EasterEgg.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\EasterEgg.h
# End Source File
# Begin Source File

SOURCE=.\Display\ErrorMessageHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\ErrorMessageHandler.h
# End Source File
# Begin Source File

SOURCE=.\Display\Gallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Gallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\Line.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Line.h
# End Source File
# Begin Source File

SOURCE=.\Display\MemoryMappedFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\MemoryMappedFile.h
# End Source File
# Begin Source File

SOURCE=.\Display\SharedGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\SharedGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Sprite.h
# End Source File
# Begin Source File

SOURCE=.\Display\System.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\TintManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\TintManager.h
# End Source File
# Begin Source File

SOURCE=.\Display\Win32ScreenKiller.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Win32ScreenKiller.h
# End Source File
# End Group
# Begin Group "Agent"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Agents\Agent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\Agent.h
# End Source File
# Begin Source File

SOURCE=.\agents\AgentConstants.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\AgentConstants.h
# End Source File
# Begin Source File

SOURCE=.\Agents\AgentHandle.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\AgentHandle.h
# End Source File
# Begin Source File

SOURCE=.\AgentManager.cpp
# End Source File
# Begin Source File

SOURCE=.\AgentManager.h
# End Source File
# Begin Source File

SOURCE=.\agents\CameraPart.cpp
# End Source File
# Begin Source File

SOURCE=.\agents\CameraPart.h
# End Source File
# Begin Source File

SOURCE=.\Classifier.cpp
# End Source File
# Begin Source File

SOURCE=.\Classifier.h
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundAgent.h
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundPart.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundPart.h
# End Source File
# Begin Source File

SOURCE=.\Agents\InputPort.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\InputPort.h
# End Source File
# Begin Source File

SOURCE=.\Message.cpp
# End Source File
# Begin Source File

SOURCE=.\Message.h
# End Source File
# Begin Source File

SOURCE=.\Agents\MessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\MessageQueue.h
# End Source File
# Begin Source File

SOURCE=.\Agents\OutputPort.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\OutputPort.h
# End Source File
# Begin Source File

SOURCE=.\Agents\PointerAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\PointerAgent.h
# End Source File
# Begin Source File

SOURCE=.\Agents\Port.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\Port.h
# End Source File
# Begin Source File

SOURCE=.\Agents\PortBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\PortBundle.h
# End Source File
# Begin Source File

SOURCE=.\Agents\QuoteFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\QuoteFactory.h
# End Source File
# Begin Source File

SOURCE=.\Agents\QuoteFactoryHelper.h
# End Source File
# Begin Source File

SOURCE=.\Agents\SimpleAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\SimpleAgent.h
# End Source File
# Begin Source File

SOURCE=.\Agents\UIPart.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\UIPart.h
# End Source File
# Begin Source File

SOURCE=.\Agents\Vehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\Vehicle.h
# End Source File
# Begin Source File

SOURCE=.\creature\voice.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\voice.h
# End Source File
# End Group
# Begin Group "CAOS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\caos\AutoDocumentationTable.cpp
# End Source File
# Begin Source File

SOURCE=.\caos\AutoDocumentationTable.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSConstants.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSDescription.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSDescription.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSException.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSException.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSMachine.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSMachine.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSTables.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSVar.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSVar.h
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugInfo.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Lexer.h
# End Source File
# Begin Source File

SOURCE=.\Caos\MacroScript.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\MacroScript.h
# End Source File
# Begin Source File

SOURCE=.\Caos\OpSpec.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\OpSpec.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Orderiser.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Orderiser.h
# End Source File
# Begin Source File

SOURCE=.\Caos\RequestManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\RequestManager.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Scriptorium.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Scriptorium.h
# End Source File
# Begin Source File

SOURCE=.\caos\TableSpec.cpp
# End Source File
# Begin Source File

SOURCE=.\caos\TableSpec.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Win32Server.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Win32Server.h
# End Source File
# End Group
# Begin Group "Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Map\CARates.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\CARates.h
# End Source File
# Begin Source File

SOURCE=.\Map\Map.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\Map.h
# End Source File
# Begin Source File

SOURCE=.\Map\MapCA.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\RoomCA.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\RoomCA.h
# End Source File
# Begin Source File

SOURCE=..\common\Vector2D.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Vector2D.h
# End Source File
# Begin Source File

SOURCE=.\Caos\VelocityVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\VelocityVariable.h
# End Source File
# Begin Source File

SOURCE=.\World.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\World.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Group "stub"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sound\stub\stub_CDRomDevice.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\stub\stub_CDRomDevice.h

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\stub\stub_MidiModule.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\stub\stub_MidiModule.h

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\stub\stub_Soundlib.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\stub\stub_Soundlib.h

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "SDL_Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sound\Sdl\SDL_Soundlib.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\Sdl\SDL_Soundlib.h

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Sound\CDRomDevice.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\CDRomDevice.h

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound\midimodule.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound\midimodule.h

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAction.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAction.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAleotoricLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAleotoricLayer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicEffect.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicErrors.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicExpression.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicGlobals.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLayer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLoopLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLoopLayer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicManager.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicNamedItem.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicRandom.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicScript.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicScript.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTimer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTrack.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTypes.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicUpdatable.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicUpdatable.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariable.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariableContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariableContainer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicWave.h
# End Source File
# Begin Source File

SOURCE=.\Sound\Soundlib.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\Soundlib.h

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\Win95SpecificMCIDeviceHandles.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sound\Win95SpecificMCIDeviceHandles.h

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\C2eServices.cpp
# End Source File
# Begin Source File

SOURCE=.\C2eServices.h
# End Source File
# Begin Source File

SOURCE=..\common\C2eTypes.h
# End Source File
# Begin Source File

SOURCE=..\common\Catalogue.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Catalogue.h
# End Source File
# Begin Source File

SOURCE=..\common\Configurator.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Configurator.h
# End Source File
# Begin Source File

SOURCE=.\General.cpp
# End Source File
# Begin Source File

SOURCE=.\General.h
# End Source File
# Begin Source File

SOURCE=.\Maths.cpp
# End Source File
# Begin Source File

SOURCE=.\Maths.h
# End Source File
# Begin Source File

SOURCE=..\common\ProfanityFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\common\ProfanityFilter.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDialog.h
# End Source File
# Begin Source File

SOURCE=.\QuickFsp.cpp
# End Source File
# Begin Source File

SOURCE=.\QuickFsp.h
# End Source File
# Begin Source File

SOURCE=.\RegistryHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryHandler.h
# End Source File
# Begin Source File

SOURCE=.\Scramble.cpp
# End Source File
# Begin Source File

SOURCE=.\Scramble.h
# End Source File
# Begin Source File

SOURCE=..\common\SimpleLexer.cpp
# End Source File
# Begin Source File

SOURCE=..\common\SimpleLexer.h
# End Source File
# Begin Source File

SOURCE=..\common\StringFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\common\StringFuncs.h
# End Source File
# Begin Source File

SOURCE=.\UniqueIdentifier.cpp
# End Source File
# Begin Source File

SOURCE=.\UniqueIdentifier.h
# End Source File
# End Group
# Begin Group "File"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CreaturesArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\CreaturesArchive.h
# End Source File
# Begin Source File

SOURCE=.\File.cpp
# End Source File
# Begin Source File

SOURCE=.\File.h
# End Source File
# Begin Source File

SOURCE=..\common\FileFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\common\FileFuncs.h
# End Source File
# Begin Source File

SOURCE=..\common\FileLocaliser.cpp
# End Source File
# Begin Source File

SOURCE=..\common\FileLocaliser.h
# End Source File
# Begin Source File

SOURCE=.\FilePath.cpp
# End Source File
# Begin Source File

SOURCE=.\FilePath.h
# End Source File
# Begin Source File

SOURCE=..\common\FileScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\common\FileScanner.h
# End Source File
# Begin Source File

SOURCE=.\FlightRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\FlightRecorder.h
# End Source File
# Begin Source File

SOURCE=.\PersistentObject.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentObject.h
# End Source File
# End Group
# Begin Group "Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\App.cpp
# End Source File
# Begin Source File

SOURCE=.\App.h
# End Source File
# Begin Source File

SOURCE=..\common\BasicException.cpp
# End Source File
# Begin Source File

SOURCE=..\common\BasicException.h
# End Source File
# Begin Source File

SOURCE=.\build.h
# End Source File
# Begin Source File

SOURCE=.\CosInstaller.cpp
# End Source File
# Begin Source File

SOURCE=.\CosInstaller.h
# End Source File
# Begin Source File

SOURCE=.\DirectoryManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectoryManager.h
# End Source File
# Begin Source File

SOURCE=.\engine.rc
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409
# End Source File
# Begin Source File

SOURCE=.\InputEvent.h
# End Source File
# Begin Source File

SOURCE=.\InputManager.cpp
# End Source File
# Begin Source File

SOURCE=.\InputManager.h
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\mfchack.cpp
# End Source File
# Begin Source File

SOURCE=.\mfchack.h
# End Source File
# Begin Source File

SOURCE=..\common\Position.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Position.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Caos\RuntimeErrorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\RuntimeErrorDialog.h
# End Source File
# Begin Source File

SOURCE=..\common\ServerSide.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=..\common\ServerSide.h
# End Source File
# Begin Source File

SOURCE=.\ServerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerThread.h
# End Source File
# Begin Source File

SOURCE=.\Display\System.h
# End Source File
# Begin Source File

SOURCE=..\..\..\NettyMessage\thenetty.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TimeFuncs.cpp
# End Source File
# End Group
# Begin Group "Pray System Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayBuilder.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayChunk.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayChunk.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayException.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayHandlers.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayHandlers.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayManager.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayManager.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayStructs.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\StringIntGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\StringIntGroup.h
# End Source File
# End Group
# Begin Group "Debugging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\C2eDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\common\C2eDebug.h
# End Source File
# Begin Source File

SOURCE=..\common\CStyleException.cpp
# End Source File
# Begin Source File

SOURCE=..\common\CStyleException.h
# End Source File
# Begin Source File

SOURCE=.\CustomHeap.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomHeap.h
# End Source File
# Begin Source File

SOURCE=..\common\MapScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\common\MapScanner.h
# End Source File
# End Group
# Begin Group "Module"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\modules\ModuleAPI.h
# End Source File
# Begin Source File

SOURCE=.\ModuleImporter.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ModuleImporter.h
# End Source File
# Begin Source File

SOURCE=..\modules\ModuleInterface.h
# End Source File
# Begin Source File

SOURCE=..\modules\NetworkInterface.h
# End Source File
# End Group
# Begin Group "Agent Display"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AgentDisplay\EntityImage.cpp
# End Source File
# Begin Source File

SOURCE=.\AgentDisplay\EntityImage.h
# End Source File
# Begin Source File

SOURCE=.\AgentDisplay\EntityImageClone.cpp
# End Source File
# Begin Source File

SOURCE=.\AgentDisplay\EntityImageClone.h
# End Source File
# Begin Source File

SOURCE=.\AgentDisplay\EntityImageWithEmbeddedCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\AgentDisplay\EntityImageWithEmbeddedCamera.h
# End Source File
# End Group
# Begin Group "Camera"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Camera\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera\Camera.h
# End Source File
# Begin Source File

SOURCE=.\Camera\CameraSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera\CameraSprite.h
# End Source File
# Begin Source File

SOURCE=.\Camera\MainCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera\MainCamera.h
# End Source File
# Begin Source File

SOURCE=.\Camera\MapImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera\MapImage.h
# End Source File
# Begin Source File

SOURCE=.\Camera\RemoteCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera\RemoteCamera.h
# End Source File
# End Group
# Begin Group "Rotation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Angle.cpp
# End Source File
# Begin Source File

SOURCE=.\Angle.h
# End Source File
# Begin Source File

SOURCE=.\Circle.cpp
# End Source File
# Begin Source File

SOURCE=.\Circle.h
# End Source File
# Begin Source File

SOURCE=.\Presence.cpp
# End Source File
# Begin Source File

SOURCE=.\Presence.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChangeLog.txt
# End Source File
# Begin Source File

SOURCE=.\Display\Sdl\SDLStretch\SDLStretch___Win32_SDL_Release\SDLStretch.lib

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Display\SDL\SDLStretch\SDLStretch___Win32_SDL_Debug\SDLStretch.lib

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\DisplayLibrary\Direct Display Engine\Debug\Direct Display Engine_d.lib"

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\DisplayLibrary\Direct Display Engine\Release\Direct Display Engine.lib"

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\DisplayLibrary\Direct Display Engine\Direct_Display_Engine___Win32_Release_for_profiling\Direct Display Engine_p.lib"

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 SDL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Direct Display Release for profiling"

!ENDIF 

# End Source File
# End Target
# End Project
