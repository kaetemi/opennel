# Microsoft Developer Studio Project File - Name="driver_direct3d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=driver_direct3d - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "driver_direct3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "driver_direct3d.mak" CFG="driver_direct3d - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "driver_direct3d - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_direct3d - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_direct3d - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_direct3d - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "driver_direct3d - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../obj/Release/driver_direct3d"
# PROP Intermediate_Dir "../../../../obj/Release/driver_direct3d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "driver_direct3d_EXPORTS" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stddirect3d.h" /FD /c
# SUBTRACT CPP /Fr
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 d3dx9.lib d3d9.lib dinput.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /out:"../../../../lib/nel_drv_direct3d_win_r.dll"

!ELSEIF  "$(CFG)" == "driver_direct3d - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../obj/Debug/driver_direct3d"
# PROP Intermediate_Dir "../../../../obj/Debug/driver_direct3d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /D "_WINDOWS" /D "_USRDLL" /D "driver_direct3d_EXPORTS" /D "__STL_DEBUG" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stddirect3d.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 d3dx9.lib d3d9.lib dinput.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../../lib/nel_drv_direct3d_win_d.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "driver_direct3d - Win32 ReleaseDebug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../obj/ReleaseDebug/driver_direct3d"
# PROP Intermediate_Dir "../../../../obj/ReleaseDebug/driver_direct3d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "_WINDOWS" /D "_USRDLL" /D "driver_direct3d_EXPORTS" /D "NL_RELEASE_DEBUG" /D "LIBXML_STATIC" /D "WIN32" /D "NDEBUG" /D "_MBCS" /Yu"stddirect3d.h" /FD /c
# SUBTRACT CPP /Fr
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 d3dx9.lib d3d9.lib dinput.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../../../lib/nel_drv_direct3d_win_rd.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "driver_direct3d - Win32 DebugFast"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../obj/DebugFast/driver_direct3d"
# PROP Intermediate_Dir "../../../../obj/DebugFast/driver_direct3d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /D "_WINDOWS" /D "_USRDLL" /D "driver_direct3d_EXPORTS" /D "_DEBUG" /D "NL_DEBUG_FAST" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stddirect3d.h" /FD /GZ /c
# SUBTRACT CPP /Gf /Fr
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 d3dx9.lib d3d9.lib dinput.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../../lib/nel_drv_direct3d_win_df.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "driver_direct3d - Win32 Release"
# Name "driver_direct3d - Win32 Debug"
# Name "driver_direct3d - Win32 ReleaseDebug"
# Name "driver_direct3d - Win32 DebugFast"
# Begin Group "Shaders"

# PROP Default_Filter "*.fx"
# Begin Source File

SOURCE=.\lightmap0.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap0blend.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap1.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap1blend.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap2.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap2blend.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap3.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap3blend.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap4.fx
# End Source File
# Begin Source File

SOURCE=.\lightmap4blend.fx
# End Source File
# End Group
# Begin Source File

SOURCE=.\direct3d.rc
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d.cpp
# ADD CPP /Yu"stddirect3d.h"
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d.def
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d.h
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_index.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_inputs.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_light.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_material.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_render.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_shader.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_vertex.cpp
# End Source File
# Begin Source File

SOURCE=.\driver_direct3d_vertex_program.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stddirect3d.cpp
# ADD CPP /Yc"stddirect3d.h"
# End Source File
# Begin Source File

SOURCE=.\stddirect3d.h
# End Source File
# End Target
# End Project