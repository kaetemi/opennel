# Microsoft Developer Studio Project File - Name="driver_openal" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=driver_openal - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "driver_openal.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "driver_openal.mak" CFG="driver_openal - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "driver_openal - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_openal - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_openal - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_openal - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "driver_openal - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../obj/Release"
# PROP Intermediate_Dir "../../../../obj/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /Yu"stdopenal.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 user32.lib ALut.lib OpenAL32.lib eax.lib eaxguid.lib /nologo /dll /machine:I386 /out:"../../../../lib/nel_drv_openal_win.dll" /libpath:"C:\Netshare\openal\win\alut\release" /libpath:"C:\Netshare\openal\win\openal32\release"

!ELSEIF  "$(CFG)" == "driver_openal - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../obj/Debug"
# PROP Intermediate_Dir "../../../../obj/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__STL_DEBUG" /Fr /Yu"stdopenal.h" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ALut.lib OpenAL32.lib eax.lib eaxguid.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../../lib/nel_drv_openal_win_debug.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "driver_openal - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "driver_openal___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "driver_openal___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../obj/ReleaseDebug"
# PROP Intermediate_Dir "../../../../obj/ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /D "NL_RELEASE_DEBUG" /Yu"stdopenal.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ALut.lib OpenAL32.lib eax.lib eaxguid.lib /nologo /dll /machine:I386 /out:"../../../../obj/Debug/nel_drv_openal_win.dll" /libpath:"C:\Netshare\openal\win\alut\release" /libpath:"C:\Netshare\openal\win\openal32\release"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ALut.lib OpenAL32.lib eax.lib eaxguid.lib /nologo /dll /debug /machine:I386 /out:"../../../../lib/nel_drv_openal_win_rd.dll" /libpath:"C:\Netshare\openal\win\alut\release" /libpath:"C:\Netshare\openal\win\openal32\release"

!ELSEIF  "$(CFG)" == "driver_openal - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "driver_openal___Win32_DebugFast"
# PROP BASE Intermediate_Dir "driver_openal___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../obj/DebugFast"
# PROP Intermediate_Dir "../../../../obj/DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__STL_DEBUG" /Fr /FD /GZ /c
# SUBTRACT BASE CPP /Gy /YX
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Ob1 /Gf /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_OPENAL_EXPORTS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "NL_DEBUG_FAST" /Fr /Yu"stdopenal.h" /FD /GZ /c
# SUBTRACT CPP /Gy
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ALut.lib OpenAL32.lib eax.lib eaxguid.lib /nologo /dll /debug /machine:I386 /out:"../../../../lib/nel_drv_openal_win_debug.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ALut.lib OpenAL32.lib eax.lib eaxguid.lib /nologo /dll /debug /machine:I386 /out:"../../../../lib/nel_drv_openal_win_debug_fast.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "driver_openal - Win32 Release"
# Name "driver_openal - Win32 Debug"
# Name "driver_openal - Win32 ReleaseDebug"
# Name "driver_openal - Win32 DebugFast"
# Begin Source File

SOURCE=.\buffer_al.cpp
# End Source File
# Begin Source File

SOURCE=.\buffer_al.h
# End Source File
# Begin Source File

SOURCE=.\driver_openal.def
# End Source File
# Begin Source File

SOURCE=.\listener_al.cpp
# End Source File
# Begin Source File

SOURCE=.\listener_al.h
# End Source File
# Begin Source File

SOURCE=.\sound_driver_al.cpp
# End Source File
# Begin Source File

SOURCE=.\sound_driver_al.h
# End Source File
# Begin Source File

SOURCE=.\source_al.cpp
# End Source File
# Begin Source File

SOURCE=.\source_al.h
# End Source File
# Begin Source File

SOURCE=.\stdopenal.cpp
# ADD CPP /Yc"stdopenal.h"
# End Source File
# Begin Source File

SOURCE=.\stdopenal.h
# End Source File
# End Target
# End Project
