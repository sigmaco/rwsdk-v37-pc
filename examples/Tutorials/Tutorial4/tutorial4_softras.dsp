# Microsoft Developer Studio Project File - Name="tutorial4" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=tutorial4 - Win32 Softras Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tutorial4_softras.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tutorial4_softras.mak" CFG="tutorial4 - Win32 Softras Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tutorial4 - Win32 Softras Release" (based on "Win32 (x86) Application")
!MESSAGE "tutorial4 - Win32 Softras Debug" (based on "Win32 (x86) Application")
!MESSAGE "tutorial4 - Win32 Softras Metrics" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tutorial4 - Win32 Softras Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/softras"
# PROP BASE Intermediate_Dir "obj/softras"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/softras"
# PROP Intermediate_Dir "obj/softras"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../rwsdk/include/softras" /I "./src" /I "./src/win" /I "../../../shared/democom" /I "../../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtgncpip.lib rtcharse.lib rtfsyst.lib rtbmp.lib rtpng.lib rtintsec.lib rtpick.lib rpcollis.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./tutorial4_softras.exe" /libpath:"../../../rwsdk/lib/softras/release"

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "obj/softrasd"
# PROP BASE Intermediate_Dir "obj/softrasd"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/softrasd"
# PROP Intermediate_Dir "obj/softrasd"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../rwsdk/include/softras" /I "./src" /I "./src/win" /I "../../../shared/democom" /I "../../../shared/skel" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWDEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rtgncpip.lib rtcharse.lib rtfsyst.lib rtbmp.lib rtpng.lib rtintsec.lib rtpick.lib rpcollis.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./tutorial4_softrasd.exe" /pdbtype:sept /libpath:"../../../rwsdk/lib/softras/debug"

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Metrics"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/softrasm"
# PROP BASE Intermediate_Dir "obj/softrasm"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/softrasm"
# PROP Intermediate_Dir "obj/softrasm"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../../../rwsdk/include/softras" /I "../src" /I "../src/win" /I "../../../shared/democom" /I "../../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../rwsdk/include/softras" /I "./src" /I "./src/win" /I "../../../shared/democom" /I "../../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWMETRICS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtgncpip.lib rtcharse.lib rtfsyst.lib rtbmp.lib rtpng.lib rtintsec.lib rtpick.lib rpcollis.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./tutorial4_softrasm.exe" /libpath:"../../../rwsdk/lib/softras/metrics"

!ENDIF 

# Begin Target

# Name "tutorial4 - Win32 Softras Release"
# Name "tutorial4 - Win32 Softras Debug"
# Name "tutorial4 - Win32 Softras Metrics"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "demoskel"

# PROP Default_Filter ""
# Begin Group "win"

# PROP Default_Filter ""
# Begin Source File

SOURCE=../../../shared/skel\win\win.c
# End Source File
# End Group
# Begin Source File

SOURCE=../../../shared/skel\metrics.c

!IF  "$(CFG)" == "tutorial4 - Win32 Softras Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Metrics"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=../../../shared/skel\skeleton.c
# End Source File
# Begin Source File

SOURCE=../../../shared/skel\vecfont.c

!IF  "$(CFG)" == "tutorial4 - Win32 Softras Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Metrics"

!ENDIF 

# End Source File
# End Group
# Begin Group "democom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=../../../shared/democom\camera.c
# End Source File
# Begin Source File

SOURCE=../../../shared/democom\menu.c
# End Source File
# End Group
# Begin Group "demo"

# PROP Default_Filter ""
# Begin Group "demowin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\win\events.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=.\src\utils.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "demoskel Hdrs"

# PROP Default_Filter ""
# Begin Group "win Hdrs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=../../../shared/skel\win.h
# End Source File
# End Group
# Begin Source File

SOURCE=../../../shared/skel\events.h
# End Source File
# Begin Source File

SOURCE=../../../shared/skel\metrics.h

!IF  "$(CFG)" == "tutorial4 - Win32 Softras Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Metrics"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=../../../shared/skel\platform.h
# End Source File
# Begin Source File

SOURCE=../../../shared/skel\skeleton.h
# End Source File
# Begin Source File

SOURCE=../../../shared/skel\vecfont.h

!IF  "$(CFG)" == "tutorial4 - Win32 Softras Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "tutorial4 - Win32 Softras Metrics"

!ENDIF 

# End Source File
# End Group
# Begin Group "democom Hdrs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=../../../shared/democom\camera.h
# End Source File
# Begin Source File

SOURCE=../../../shared/democom\menu.h
# End Source File
# End Group
# Begin Group "demo Hdrs"

# PROP Default_Filter ""
# End Group
# End Group
# Begin Group "Misc Files"

# PROP Default_Filter ""
# Begin Group "win misc files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=../../../shared/skel\win\win.rc
# End Source File
# End Group
# End Group
# Begin Group "Readme Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win.txt
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
