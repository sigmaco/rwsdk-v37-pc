# Microsoft Developer Studio Project File - Name="imgformt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=imgformt - Win32 D3D8 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "imgformt_win.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "imgformt_win.mak" CFG="imgformt - Win32 D3D8 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "imgformt - Win32 D3D8 Release" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 D3D8 Debug" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 D3D8 Metrics" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 OGL Release" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 OGL Debug" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 OGL Metrics" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 D3D9 Release" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 D3D9 Debug" (based on "Win32 (x86) Application")
!MESSAGE "imgformt - Win32 D3D9 Metrics" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
!IF  "$(CFG)" == "imgformt - Win32 D3D8 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/d3d8"
# PROP BASE Intermediate_Dir "obj/d3d8"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/d3d8"
# PROP Intermediate_Dir "obj/d3d8"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/d3d8" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./imgformt_d3d8.exe" /libpath:"../../rwsdk/lib/d3d8/release"
!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "obj/d3d8d"
# PROP BASE Intermediate_Dir "obj/d3d8d"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/d3d8d"
# PROP Intermediate_Dir "obj/d3d8d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../rwsdk/include/d3d8" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWDEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./imgformt_d3d8d.exe" /pdbtype:sept /libpath:"../../rwsdk/lib/d3d8/debug"
!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Metrics"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/d3d8m"
# PROP BASE Intermediate_Dir "obj/d3d8m"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/d3d8m"
# PROP Intermediate_Dir "obj/d3d8m"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/d3d8" /I "../src" /I "../src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/d3d8" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWMETRICS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./imgformt_d3d8m.exe" /libpath:"../../rwsdk/lib/d3d8/metrics"
!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/ogl"
# PROP BASE Intermediate_Dir "obj/ogl"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/ogl"
# PROP Intermediate_Dir "obj/ogl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/opengl" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./imgformt_ogl.exe" /libpath:"../../rwsdk/lib/opengl/release"
!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "obj/ogld"
# PROP BASE Intermediate_Dir "obj/ogld"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/ogld"
# PROP Intermediate_Dir "obj/ogld"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../rwsdk/include/opengl" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWDEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./imgformt_ogld.exe" /pdbtype:sept /libpath:"../../rwsdk/lib/opengl/debug"
!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Metrics"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/oglm"
# PROP BASE Intermediate_Dir "obj/oglm"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/oglm"
# PROP Intermediate_Dir "obj/oglm"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/opengl" /I "../src" /I "../src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/opengl" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWMETRICS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./imgformt_oglm.exe" /libpath:"../../rwsdk/lib/opengl/metrics"
!IF  "$(CFG)" == "imgformt - Win32 D3D9 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/d3d9"
# PROP BASE Intermediate_Dir "obj/d3d9"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/d3d9"
# PROP Intermediate_Dir "obj/d3d9"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/d3d9" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./imgformt_d3d9.exe" /libpath:"../../rwsdk/lib/d3d9/release"
!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "obj/d3d9d"
# PROP BASE Intermediate_Dir "obj/d3d9d"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "obj/d3d9d"
# PROP Intermediate_Dir "obj/d3d9d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../rwsdk/include/d3d9" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWDEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"./imgformt_d3d9d.exe" /pdbtype:sept /libpath:"../../rwsdk/lib/d3d9/debug"
!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Metrics"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "obj/d3d9m"
# PROP BASE Intermediate_Dir "obj/d3d9m"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj/d3d9m"
# PROP Intermediate_Dir "obj/d3d9m"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/d3d9" /I "../src" /I "../src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../rwsdk/include/d3d9" /I "./src" /I "./src/win" /I "../../shared/democom" /I "../../shared/skel" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "RWLOGO" /D "RWMETRICS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 rtcharse.lib rtfsyst.lib rtpng.lib rtbmp.lib rpworld.lib rwcore.lib rplogo.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"./imgformt_d3d9m.exe" /libpath:"../../rwsdk/lib/d3d9/metrics"
!ENDIF 

# Begin Target

# Name "imgformt - Win32 D3D8 Release"
# Name "imgformt - Win32 D3D8 Debug"
# Name "imgformt - Win32 D3D8 Metrics"
# Name "imgformt - Win32 OGL Release"
# Name "imgformt - Win32 OGL Debug"
# Name "imgformt - Win32 OGL Metrics"
# Name "imgformt - Win32 D3D9 Release"
# Name "imgformt - Win32 D3D9 Debug"
# Name "imgformt - Win32 D3D9 Metrics"
# Begin Group "Source Files"

# PROP Default_Filter ""

# Begin Group "demoskel"

# PROP Default_Filter ""

# Begin Group "win"

# PROP Default_Filter ""
# Begin Source File

SOURCE=../../shared/skel\win\win.c

# End Source File
# End Group
# Begin Source File

SOURCE=../../shared/skel\skeleton.c

# End Source File
# Begin Source File

SOURCE=../../shared/skel\metrics.c

!IF  "$(CFG)" == "imgformt - Win32 D3D8 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Metrics"

# PROP Exclude_From_Build 0

!ENDIF 
# End Source File
# Begin Source File

SOURCE=../../shared/skel\vecfont.c

!IF  "$(CFG)" == "imgformt - Win32 D3D8 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Metrics"

# PROP Exclude_From_Build 0

!ENDIF 
# End Source File
# End Group
# Begin Group "democom"

# PROP Default_Filter ""

# Begin Source File

SOURCE=../../shared/democom\camera.c

# End Source File
# Begin Source File

SOURCE=../../shared/democom\menu.c

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

SOURCE=.\src\tga.c

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

SOURCE=../../shared/skel\win\win.h

# End Source File
# End Group

# Begin Source File

SOURCE=../../shared/skel\events.h

# End Source File
# Begin Source File

SOURCE=../../shared/skel\platform.h

# End Source File
# Begin Source File

SOURCE=../../shared/skel\skeleton.h

# End Source File
# Begin Source File

SOURCE=../../shared/skel\metrics.h

!IF  "$(CFG)" == "imgformt - Win32 D3D8 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Metrics"

# PROP Exclude_From_Build 0

!ENDIF 
# End Source File
# Begin Source File

SOURCE=../../shared/skel\vecfont.h

!IF  "$(CFG)" == "imgformt - Win32 D3D8 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Metrics"

# PROP Exclude_From_Build 0

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Metrics"

# PROP Exclude_From_Build 0

!ENDIF 
# End Source File
# End Group
# Begin Group "democom Hdrs"

# PROP Default_Filter ""

# Begin Source File

SOURCE=../../shared/democom\camera.h

# End Source File
# Begin Source File

SOURCE=../../shared/democom\menu.h

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

SOURCE=../../shared/skel\win\win.rc

# End Source File
# End Group
# End Group
# Begin Group "Readme Files"

# PROP Default_Filter ""

# Begin Source File

SOURCE=.\win.txt

!IF  "$(CFG)" == "imgformt - Win32 D3D8 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D8 Metrics"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 OGL Metrics"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "imgformt - Win32 D3D9 Metrics"

# PROP Exclude_From_Build 1

!ENDIF 
# End Source File
# End Group
# End Target
# End Project

