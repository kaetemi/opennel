# Microsoft Developer Studio Project File - Name="georges" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=georges - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "georges.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "georges.mak" CFG="georges - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "georges - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "georges - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "georges - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE "georges - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "georges - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/Release"
# PROP Intermediate_Dir "../obj/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdgeorges.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges.lib"

!ELSEIF  "$(CFG)" == "georges - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/Debug"
# PROP Intermediate_Dir "../obj/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_MBCS" /D "_LIB" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /Yu"stdgeorges.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges_debug.lib"

!ELSEIF  "$(CFG)" == "georges - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugFast"
# PROP BASE Intermediate_Dir "DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast"
# PROP Intermediate_Dir "../obj/DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "NL_DEBUG_FAST" /Yu"stdgeorges.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges_debug_fast.lib"

!ELSEIF  "$(CFG)" == "georges - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/ReleaseDebug"
# PROP Intermediate_Dir "../obj/ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NL_RELEASE_DEBUG" /Yu"stdgeorges.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges_rd.lib"

!ENDIF 

# Begin Target

# Name "georges - Win32 Release"
# Name "georges - Win32 Debug"
# Name "georges - Win32 DebugFast"
# Name "georges - Win32 ReleaseDebug"
# Begin Source File

SOURCE=.\georges\form.cpp
# End Source File
# Begin Source File

SOURCE=.\georges\form.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_dfn.cpp
# End Source File
# Begin Source File

SOURCE=.\georges\form_dfn.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_elm.cpp
# End Source File
# Begin Source File

SOURCE=.\georges\form_elm.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_loader.cpp
# End Source File
# Begin Source File

SOURCE=.\georges\form_loader.h
# End Source File
# Begin Source File

SOURCE=.\georges\header.cpp
# End Source File
# Begin Source File

SOURCE=.\georges\header.h
# End Source File
# Begin Source File

SOURCE=.\georges\stdgeorges.cpp
# ADD CPP /Yc"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=.\georges\stdgeorges.h
# End Source File
# Begin Source File

SOURCE=.\georges\type.cpp
# End Source File
# Begin Source File

SOURCE=.\georges\type.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\u_form.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\u_form_dfn.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\u_form_elm.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\u_form_loader.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\u_type.h
# End Source File
# End Target
# End Project
