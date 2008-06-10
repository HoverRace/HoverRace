# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=MazeCompiler - Win32 Release
!MESSAGE No configuration specified.  Defaulting to MazeCompiler - Win32\
 Release.
!ENDIF 

!IF "$(CFG)" != "FireBall - Win32 Release" && "$(CFG)" !=\
 "FireBall - Win32 Debug" && "$(CFG)" != "Util - Win32 Release" && "$(CFG)" !=\
 "Util - Win32 Debug" && "$(CFG)" != "MazeCompiler - Win32 Debug" && "$(CFG)" !=\
 "Model - Win32 Release" && "$(CFG)" != "Model - Win32 Debug" && "$(CFG)" !=\
 "ObjFac1 - Win32 Release" && "$(CFG)" != "ObjFac1 - Win32 Debug" && "$(CFG)" !=\
 "GameServer - Win32 Release" && "$(CFG)" != "GameServer - Win32 Debug" &&\
 "$(CFG)" != "MainCharacter - Win32 Release" && "$(CFG)" !=\
 "MainCharacter - Win32 Debug" && "$(CFG)" != "VideoServices - Win32 Release" &&\
 "$(CFG)" != "VideoServices - Win32 Debug" && "$(CFG)" !=\
 "Game2 - Win32 Release" && "$(CFG)" != "Game2 - Win32 Debug" && "$(CFG)" !=\
 "PaletteCreator - Win32 Debug" && "$(CFG)" != "ColorTools - Win32 Debug" &&\
 "$(CFG)" != "ObjFacTools - Win32 Release" && "$(CFG)" !=\
 "ObjFacTools - Win32 Debug" && "$(CFG)" != "ResourceCompiler - Win32 Debug" &&\
 "$(CFG)" != "MazeCompiler - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "NetTarget.mak" CFG="MazeCompiler - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FireBall - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FireBall - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Util - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Util - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MazeCompiler - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "Model - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Model - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjFac1 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjFac1 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GameServer - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "GameServer - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "MainCharacter - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MainCharacter - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "VideoServices - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "VideoServices - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Game2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Game2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "PaletteCreator - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "ColorTools - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjFacTools - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjFacTools - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ResourceCompiler - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "MazeCompiler - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Game2 - Win32 Release"

!IF  "$(CFG)" == "FireBall - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "ObjFacTools - Win32 Release" "Game2 - Win32 Release"\
 "VideoServices - Win32 Release" "MainCharacter - Win32 Release"\
 "GameServer - Win32 Release" "ObjFac1 - Win32 Release"\
 "MazeCompiler - Win32 Release" "$(OUTDIR)\NetTarget.exe"

CLEAN : 
	-@erase "$(OUTDIR)\NetTarget.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/NetTarget.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/NetTarget.pdb" /machine:I386 /out:"$(OUTDIR)/NetTarget.exe" 
LINK32_OBJS= \
	"$(OUTDIR)\MainCharacter.lib" \
	"$(OUTDIR)\ObjFac1.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\NetTarget.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "ResourceCompiler - Win32 Debug" "ObjFacTools - Win32 Debug"\
 "PaletteCreator - Win32 Debug" "Game2 - Win32 Debug"\
 "VideoServices - Win32 Debug" "MainCharacter - Win32 Debug"\
 "GameServer - Win32 Debug" "ObjFac1 - Win32 Debug" "MazeCompiler - Win32 Debug"\
 "$(OUTDIR)\NetTarget.exe"

CLEAN : 
	-@erase "$(OUTDIR)\NetTarget.exe"
	-@erase "$(OUTDIR)\NetTarget.ilk"
	-@erase "$(OUTDIR)\NetTarget.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/NetTarget.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/NetTarget.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/NetTarget.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/NetTarget.exe" 
LINK32_OBJS= \
	"$(OUTDIR)\MainCharacter.lib" \
	"$(OUTDIR)\ObjFac1.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\NetTarget.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Util - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Util\Release"
# PROP BASE Intermediate_Dir "Util\Release"
# PROP BASE Target_Dir "Util"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Util\Release"
# PROP Target_Dir "Util"
OUTDIR=.\Release
INTDIR=.\Util\Release

ALL : "$(OUTDIR)\Util.dll"

CLEAN : 
	-@erase "$(INTDIR)\DllObjectFactory.obj"
	-@erase "$(INTDIR)\FuzzyLogic.obj"
	-@erase "$(INTDIR)\Profiler.obj"
	-@erase "$(INTDIR)\RecordFile.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StrRes.obj"
	-@erase "$(INTDIR)\WorldCoordinates.obj"
	-@erase "$(OUTDIR)\Util.dll"
	-@erase "$(OUTDIR)\Util.exp"
	-@erase "$(OUTDIR)\Util.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MR_UTIL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /D "MR_UTIL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Util\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Util.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Util.pdb" /machine:I386 /out:"$(OUTDIR)/Util.dll"\
 /implib:"$(OUTDIR)/Util.lib" 
LINK32_OBJS= \
	"$(INTDIR)\DllObjectFactory.obj" \
	"$(INTDIR)\FuzzyLogic.obj" \
	"$(INTDIR)\Profiler.obj" \
	"$(INTDIR)\RecordFile.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StrRes.obj" \
	"$(INTDIR)\WorldCoordinates.obj"

"$(OUTDIR)\Util.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Util\Debug"
# PROP BASE Intermediate_Dir "Util\Debug"
# PROP BASE Target_Dir "Util"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Util\Debug"
# PROP Target_Dir "Util"
OUTDIR=.\Debug
INTDIR=.\Util\Debug

ALL : "$(OUTDIR)\Util.dll"

CLEAN : 
	-@erase "$(INTDIR)\DllObjectFactory.obj"
	-@erase "$(INTDIR)\FuzzyLogic.obj"
	-@erase "$(INTDIR)\Profiler.obj"
	-@erase "$(INTDIR)\RecordFile.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StrRes.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\WorldCoordinates.obj"
	-@erase "$(OUTDIR)\Util.dll"
	-@erase "$(OUTDIR)\Util.exp"
	-@erase "$(OUTDIR)\Util.ilk"
	-@erase "$(OUTDIR)\Util.lib"
	-@erase "$(OUTDIR)\Util.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MR_UTIL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "MR_UTIL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/Util.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Util\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Util.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /profile /map
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Util.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Util.dll"\
 /implib:"$(OUTDIR)/Util.lib" 
LINK32_OBJS= \
	"$(INTDIR)\DllObjectFactory.obj" \
	"$(INTDIR)\FuzzyLogic.obj" \
	"$(INTDIR)\Profiler.obj" \
	"$(INTDIR)\RecordFile.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\StrRes.obj" \
	"$(INTDIR)\WorldCoordinates.obj"

"$(OUTDIR)\Util.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MazeCompiler\Debug"
# PROP BASE Intermediate_Dir "MazeCompiler\Debug"
# PROP BASE Target_Dir "MazeCompiler"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "MazeCompiler\Debug"
# PROP Target_Dir "MazeCompiler"
OUTDIR=.\Debug
INTDIR=.\MazeCompiler\Debug

ALL : "VideoServices - Win32 Debug" "Model - Win32 Debug" "Util - Win32 Debug"\
 "$(OUTDIR)\MazeCompiler.exe"

CLEAN : 
	-@erase "$(INTDIR)\LevelBuilder.obj"
	-@erase "$(INTDIR)\LevelBuilderVisiblesZones.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MazeCompiler.res"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TrackMap.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\MazeCompiler.exe"
	-@erase "$(OUTDIR)\MazeCompiler.ilk"
	-@erase "$(OUTDIR)\MazeCompiler.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D\
 "_CONSOLE" /D "WIN32" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/MazeCompiler.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\MazeCompiler\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/MazeCompiler.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/MazeCompiler.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/MazeCompiler.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/MazeCompiler.exe" 
LINK32_OBJS= \
	"$(INTDIR)\LevelBuilder.obj" \
	"$(INTDIR)\LevelBuilderVisiblesZones.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MazeCompiler.res" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TrackMap.obj" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\MazeCompiler.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Model - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Model\Release"
# PROP BASE Intermediate_Dir "Model\Release"
# PROP BASE Target_Dir "Model"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Model\Release"
# PROP Target_Dir "Model"
OUTDIR=.\Release
INTDIR=.\Model\Release

ALL : "Util - Win32 Release" "$(OUTDIR)\Model.dll"

CLEAN : 
	-@erase "$(INTDIR)\ConcreteShape.obj"
	-@erase "$(INTDIR)\ContactEffect.obj"
	-@erase "$(INTDIR)\FreeElementMovingHelper.obj"
	-@erase "$(INTDIR)\GameSession.obj"
	-@erase "$(INTDIR)\Level.obj"
	-@erase "$(INTDIR)\MazeElement.obj"
	-@erase "$(INTDIR)\PhysicalCollision.obj"
	-@erase "$(INTDIR)\ShapeCollisions.obj"
	-@erase "$(INTDIR)\Shapes.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\Model.dll"
	-@erase "$(OUTDIR)\Model.exp"
	-@erase "$(OUTDIR)\Model.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MR_MODEL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /D "MR_MODEL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Model\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Model.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /debug
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/Model.pdb" /machine:I386 /out:"$(OUTDIR)/Model.dll"\
 /implib:"$(OUTDIR)/Model.lib" 
LINK32_OBJS= \
	"$(INTDIR)\ConcreteShape.obj" \
	"$(INTDIR)\ContactEffect.obj" \
	"$(INTDIR)\FreeElementMovingHelper.obj" \
	"$(INTDIR)\GameSession.obj" \
	"$(INTDIR)\Level.obj" \
	"$(INTDIR)\MazeElement.obj" \
	"$(INTDIR)\PhysicalCollision.obj" \
	"$(INTDIR)\ShapeCollisions.obj" \
	"$(INTDIR)\Shapes.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(OUTDIR)\Util.lib"

"$(OUTDIR)\Model.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Model\Debug"
# PROP BASE Intermediate_Dir "Model\Debug"
# PROP BASE Target_Dir "Model"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Model\Debug"
# PROP Target_Dir "Model"
OUTDIR=.\Debug
INTDIR=.\Model\Debug

ALL : "Util - Win32 Debug" "$(OUTDIR)\Model.dll"

CLEAN : 
	-@erase "$(INTDIR)\ConcreteShape.obj"
	-@erase "$(INTDIR)\ContactEffect.obj"
	-@erase "$(INTDIR)\FreeElementMovingHelper.obj"
	-@erase "$(INTDIR)\GameSession.obj"
	-@erase "$(INTDIR)\Level.obj"
	-@erase "$(INTDIR)\MazeElement.obj"
	-@erase "$(INTDIR)\PhysicalCollision.obj"
	-@erase "$(INTDIR)\ShapeCollisions.obj"
	-@erase "$(INTDIR)\Shapes.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Model.dll"
	-@erase "$(OUTDIR)\Model.exp"
	-@erase "$(OUTDIR)\Model.ilk"
	-@erase "$(OUTDIR)\Model.lib"
	-@erase "$(OUTDIR)\Model.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MR_MODEL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "MR_MODEL" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Model\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Model.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /profile /map
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/Model.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Model.dll"\
 /implib:"$(OUTDIR)/Model.lib" 
LINK32_OBJS= \
	"$(INTDIR)\ConcreteShape.obj" \
	"$(INTDIR)\ContactEffect.obj" \
	"$(INTDIR)\FreeElementMovingHelper.obj" \
	"$(INTDIR)\GameSession.obj" \
	"$(INTDIR)\Level.obj" \
	"$(INTDIR)\MazeElement.obj" \
	"$(INTDIR)\PhysicalCollision.obj" \
	"$(INTDIR)\ShapeCollisions.obj" \
	"$(INTDIR)\Shapes.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(OUTDIR)\Util.lib"

"$(OUTDIR)\Model.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ObjFac1\Release"
# PROP BASE Intermediate_Dir "ObjFac1\Release"
# PROP BASE Target_Dir "ObjFac1"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "ObjFac1\Release"
# PROP Target_Dir "ObjFac1"
OUTDIR=.\Release
INTDIR=.\ObjFac1\Release

ALL : "VideoServices - Win32 Release" "Util - Win32 Release"\
 "ObjFacTools - Win32 Release" "Model - Win32 Release"\
 "MainCharacter - Win32 Release" "$(OUTDIR)\ObjFac1.dll"

CLEAN : 
	-@erase "$(INTDIR)\BabeElement.obj"
	-@erase "$(INTDIR)\BallElement.obj"
	-@erase "$(INTDIR)\BumperGate.obj"
	-@erase "$(INTDIR)\DefaultSurface.obj"
	-@erase "$(INTDIR)\DoubleSpeedSource.obj"
	-@erase "$(INTDIR)\FinishLine.obj"
	-@erase "$(INTDIR)\FuelSource.obj"
	-@erase "$(INTDIR)\GenericRenderer.obj"
	-@erase "$(INTDIR)\HoverRender.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\Mine.obj"
	-@erase "$(INTDIR)\Missile.obj"
	-@erase "$(INTDIR)\PowerUp.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TestElement.obj"
	-@erase "$(INTDIR)\WoodSurface.obj"
	-@erase "$(OUTDIR)\ObjFac1.dll"
	-@erase "$(OUTDIR)\ObjFac1.exp"
	-@erase "$(OUTDIR)\ObjFac1.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ObjFac1\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ObjFac1.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ObjFac1.pdb" /machine:I386 /out:"$(OUTDIR)/ObjFac1.dll"\
 /implib:"$(OUTDIR)/ObjFac1.lib" 
LINK32_OBJS= \
	"$(INTDIR)\BabeElement.obj" \
	"$(INTDIR)\BallElement.obj" \
	"$(INTDIR)\BumperGate.obj" \
	"$(INTDIR)\DefaultSurface.obj" \
	"$(INTDIR)\DoubleSpeedSource.obj" \
	"$(INTDIR)\FinishLine.obj" \
	"$(INTDIR)\FuelSource.obj" \
	"$(INTDIR)\GenericRenderer.obj" \
	"$(INTDIR)\HoverRender.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\Mine.obj" \
	"$(INTDIR)\Missile.obj" \
	"$(INTDIR)\PowerUp.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TestElement.obj" \
	"$(INTDIR)\WoodSurface.obj" \
	"$(OUTDIR)\MainCharacter.lib" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\ObjFac1.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ObjFac1\Debug"
# PROP BASE Intermediate_Dir "ObjFac1\Debug"
# PROP BASE Target_Dir "ObjFac1"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "ObjFac1\Debug"
# PROP Target_Dir "ObjFac1"
OUTDIR=.\Debug
INTDIR=.\ObjFac1\Debug
# Begin Custom Macros
TargetDir=.\Debug
# End Custom Macros

ALL : "VideoServices - Win32 Debug" "Util - Win32 Debug"\
 "ObjFacTools - Win32 Debug" "Model - Win32 Debug" "MainCharacter - Win32 Debug"\
 "$(OUTDIR)\ObjFac1.dll" "$(OUTDIR)\ObjFac1.bsc" "$(OUTDIR)\ObjFac1.dat"

CLEAN : 
	-@erase "$(INTDIR)\BabeElement.obj"
	-@erase "$(INTDIR)\BabeElement.sbr"
	-@erase "$(INTDIR)\BallElement.obj"
	-@erase "$(INTDIR)\BallElement.sbr"
	-@erase "$(INTDIR)\BumperGate.obj"
	-@erase "$(INTDIR)\BumperGate.sbr"
	-@erase "$(INTDIR)\DefaultSurface.obj"
	-@erase "$(INTDIR)\DefaultSurface.sbr"
	-@erase "$(INTDIR)\DoubleSpeedSource.obj"
	-@erase "$(INTDIR)\DoubleSpeedSource.sbr"
	-@erase "$(INTDIR)\FinishLine.obj"
	-@erase "$(INTDIR)\FinishLine.sbr"
	-@erase "$(INTDIR)\FuelSource.obj"
	-@erase "$(INTDIR)\FuelSource.sbr"
	-@erase "$(INTDIR)\GenericRenderer.obj"
	-@erase "$(INTDIR)\GenericRenderer.sbr"
	-@erase "$(INTDIR)\HoverRender.obj"
	-@erase "$(INTDIR)\HoverRender.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\Mine.obj"
	-@erase "$(INTDIR)\Mine.sbr"
	-@erase "$(INTDIR)\Missile.obj"
	-@erase "$(INTDIR)\Missile.sbr"
	-@erase "$(INTDIR)\PowerUp.obj"
	-@erase "$(INTDIR)\PowerUp.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TestElement.obj"
	-@erase "$(INTDIR)\TestElement.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\WoodSurface.obj"
	-@erase "$(INTDIR)\WoodSurface.sbr"
	-@erase "$(OUTDIR)\ObjFac1.bsc"
	-@erase "$(OUTDIR)\ObjFac1.dat"
	-@erase "$(OUTDIR)\ObjFac1.dll"
	-@erase "$(OUTDIR)\ObjFac1.exp"
	-@erase "$(OUTDIR)\ObjFac1.ilk"
	-@erase "$(OUTDIR)\ObjFac1.lib"
	-@erase "$(OUTDIR)\ObjFac1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /FR /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ObjFac1\Debug/
CPP_SBRS=.\ObjFac1\Debug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ObjFac1.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BabeElement.sbr" \
	"$(INTDIR)\BallElement.sbr" \
	"$(INTDIR)\BumperGate.sbr" \
	"$(INTDIR)\DefaultSurface.sbr" \
	"$(INTDIR)\DoubleSpeedSource.sbr" \
	"$(INTDIR)\FinishLine.sbr" \
	"$(INTDIR)\FuelSource.sbr" \
	"$(INTDIR)\GenericRenderer.sbr" \
	"$(INTDIR)\HoverRender.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\Mine.sbr" \
	"$(INTDIR)\Missile.sbr" \
	"$(INTDIR)\PowerUp.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TestElement.sbr" \
	"$(INTDIR)\WoodSurface.sbr"

"$(OUTDIR)\ObjFac1.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /profile /map
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ObjFac1.pdb" /debug /machine:I386 /out:"$(OUTDIR)/ObjFac1.dll"\
 /implib:"$(OUTDIR)/ObjFac1.lib" 
LINK32_OBJS= \
	"$(INTDIR)\BabeElement.obj" \
	"$(INTDIR)\BallElement.obj" \
	"$(INTDIR)\BumperGate.obj" \
	"$(INTDIR)\DefaultSurface.obj" \
	"$(INTDIR)\DoubleSpeedSource.obj" \
	"$(INTDIR)\FinishLine.obj" \
	"$(INTDIR)\FuelSource.obj" \
	"$(INTDIR)\GenericRenderer.obj" \
	"$(INTDIR)\HoverRender.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\Mine.obj" \
	"$(INTDIR)\Missile.obj" \
	"$(INTDIR)\PowerUp.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TestElement.obj" \
	"$(INTDIR)\WoodSurface.obj" \
	"$(OUTDIR)\MainCharacter.lib" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\ObjFac1.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

# Begin Custom Build
TargetDir=.\Debug
InputPath=.\Debug\ObjFac1.dll
SOURCE=$(InputPath)

"$(TargetDir)\ObjFac1.dat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   F:\project\NetTarget\Debug\ResourceCompiler ObjFac1\ObjFac1.fbres\
                                                                                                                                                                                                                                                   $(TargetDir)\ObjFac1.dat ObjFac1\ObjFac1Res.h

# End Custom Build

!ELSEIF  "$(CFG)" == "GameServer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GameServer\Release"
# PROP BASE Intermediate_Dir "GameServer\Release"
# PROP BASE Target_Dir "GameServer"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "GameServer\Release"
# PROP Target_Dir "GameServer"
OUTDIR=.\Release
INTDIR=.\GameServer\Release

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\GameServer\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/GameServer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /machine:I386
LINK32_FLAGS=/nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/GameServer.pdb" /machine:I386 /out:"$(OUTDIR)/GameServer.exe" 
LINK32_OBJS= \
	

!ELSEIF  "$(CFG)" == "GameServer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GameServer\Debug"
# PROP BASE Intermediate_Dir "GameServer\Debug"
# PROP BASE Target_Dir "GameServer"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "GameServer\Debug"
# PROP Target_Dir "GameServer"
OUTDIR=.\Debug
INTDIR=.\GameServer\Debug

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/GameServer.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\GameServer\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/GameServer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/GameServer.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/GameServer.exe" 
LINK32_OBJS= \
	

!ELSEIF  "$(CFG)" == "MainCharacter - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MainCharacter\Release"
# PROP BASE Intermediate_Dir "MainCharacter\Release"
# PROP BASE Target_Dir "MainCharacter"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "MainCharacter\Release"
# PROP Target_Dir "MainCharacter"
OUTDIR=.\Release
INTDIR=.\MainCharacter\Release

ALL : "VideoServices - Win32 Release" "Util - Win32 Release"\
 "Model - Win32 Release" "$(OUTDIR)\MainCharacter.dll"

CLEAN : 
	-@erase "$(INTDIR)\MainCharacter.obj"
	-@erase "$(INTDIR)\MainCharacterRenderer.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\MainCharacter.dll"
	-@erase "$(OUTDIR)\MainCharacter.exp"
	-@erase "$(OUTDIR)\MainCharacter.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "_WINDOWS" /D "MR_MAIN_CHARACTER" /D "_WINDLL" /D "WIN32" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D\
 "_WINDOWS" /D "MR_MAIN_CHARACTER" /D "_WINDLL" /D "WIN32" /D "_AFXDLL" /D\
 "_MBCS" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\MainCharacter\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/MainCharacter.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/MainCharacter.pdb" /machine:I386\
 /out:"$(OUTDIR)/MainCharacter.dll" /implib:"$(OUTDIR)/MainCharacter.lib" 
LINK32_OBJS= \
	"$(INTDIR)\MainCharacter.obj" \
	"$(INTDIR)\MainCharacterRenderer.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\MainCharacter.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MainCharacter - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MainCharacter\Debug"
# PROP BASE Intermediate_Dir "MainCharacter\Debug"
# PROP BASE Target_Dir "MainCharacter"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "MainCharacter\Debug"
# PROP Target_Dir "MainCharacter"
OUTDIR=.\Debug
INTDIR=.\MainCharacter\Debug

ALL : "VideoServices - Win32 Debug" "Util - Win32 Debug" "Model - Win32 Debug"\
 "$(OUTDIR)\MainCharacter.dll"

CLEAN : 
	-@erase "$(INTDIR)\MainCharacter.obj"
	-@erase "$(INTDIR)\MainCharacterRenderer.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\MainCharacter.dll"
	-@erase "$(OUTDIR)\MainCharacter.exp"
	-@erase "$(OUTDIR)\MainCharacter.ilk"
	-@erase "$(OUTDIR)\MainCharacter.lib"
	-@erase "$(OUTDIR)\MainCharacter.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D "_WINDOWS" /D "MR_MAIN_CHARACTER" /D "_WINDLL" /D "WIN32" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D\
 "_WINDOWS" /D "MR_MAIN_CHARACTER" /D "_WINDLL" /D "WIN32" /D "_AFXDLL" /D\
 "_MBCS" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\MainCharacter\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/MainCharacter.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /profile /map
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/MainCharacter.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/MainCharacter.dll" /implib:"$(OUTDIR)/MainCharacter.lib" 
LINK32_OBJS= \
	"$(INTDIR)\MainCharacter.obj" \
	"$(INTDIR)\MainCharacterRenderer.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\MainCharacter.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VideoServices - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "VideoServices\Release"
# PROP BASE Intermediate_Dir "VideoServices\Release"
# PROP BASE Target_Dir "VideoServices"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "VideoServices\Release"
# PROP Target_Dir "VideoServices"
OUTDIR=.\Release
INTDIR=.\VideoServices\Release

ALL : "Util - Win32 Release" "$(OUTDIR)\VideoServices.dll"

CLEAN : 
	-@erase "$(INTDIR)\2DViewPort.obj"
	-@erase "$(INTDIR)\3DViewport.obj"
	-@erase "$(INTDIR)\3DViewportRendering.obj"
	-@erase "$(INTDIR)\Bitmap.obj"
	-@erase "$(INTDIR)\ColorPalette.obj"
	-@erase "$(INTDIR)\ColorTab.obj"
	-@erase "$(INTDIR)\SoundServer.obj"
	-@erase "$(INTDIR)\Sprite.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\VideoBuffer.obj"
	-@erase "$(INTDIR)\VideoBuffer2DDraw.obj"
	-@erase "$(OUTDIR)\VideoServices.dll"
	-@erase "$(OUTDIR)\VideoServices.exp"
	-@erase "$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MR_VIDEO_SERVICES" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /D "MR_VIDEO_SERVICES" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\VideoServices\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/VideoServices.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 \gdk\sdk\lib\ddraw.lib \gdk\sdk\lib\dsound.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /map /debug
LINK32_FLAGS=\gdk\sdk\lib\ddraw.lib \gdk\sdk\lib\dsound.lib winmm.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/VideoServices.pdb"\
 /machine:I386 /out:"$(OUTDIR)/VideoServices.dll"\
 /implib:"$(OUTDIR)/VideoServices.lib" 
LINK32_OBJS= \
	"$(INTDIR)\2DViewPort.obj" \
	"$(INTDIR)\3DViewport.obj" \
	"$(INTDIR)\3DViewportRendering.obj" \
	"$(INTDIR)\Bitmap.obj" \
	"$(INTDIR)\ColorPalette.obj" \
	"$(INTDIR)\ColorTab.obj" \
	"$(INTDIR)\SoundServer.obj" \
	"$(INTDIR)\Sprite.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\VideoBuffer.obj" \
	"$(INTDIR)\VideoBuffer2DDraw.obj" \
	"$(OUTDIR)\Util.lib"

"$(OUTDIR)\VideoServices.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "VideoServices\Debug"
# PROP BASE Intermediate_Dir "VideoServices\Debug"
# PROP BASE Target_Dir "VideoServices"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "VideoServices\Debug"
# PROP Target_Dir "VideoServices"
OUTDIR=.\Debug
INTDIR=.\VideoServices\Debug

ALL : "ColorTools - Win32 Debug" "Util - Win32 Debug"\
 "$(OUTDIR)\VideoServices.dll"

CLEAN : 
	-@erase "$(INTDIR)\2DViewPort.obj"
	-@erase "$(INTDIR)\3DViewport.obj"
	-@erase "$(INTDIR)\3DViewportRendering.obj"
	-@erase "$(INTDIR)\Bitmap.obj"
	-@erase "$(INTDIR)\ColorPalette.obj"
	-@erase "$(INTDIR)\ColorTab.obj"
	-@erase "$(INTDIR)\SoundServer.obj"
	-@erase "$(INTDIR)\Sprite.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\VideoBuffer.obj"
	-@erase "$(INTDIR)\VideoBuffer2DDraw.obj"
	-@erase "$(OUTDIR)\VideoServices.dll"
	-@erase "$(OUTDIR)\VideoServices.exp"
	-@erase "$(OUTDIR)\VideoServices.ilk"
	-@erase "$(OUTDIR)\VideoServices.lib"
	-@erase "$(OUTDIR)\VideoServices.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MR_VIDEO_SERVICES" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /D "MR_VIDEO_SERVICES" /D "_WINDLL" /D "_AFXDLL" /D\
 "_MBCS" /Fp"$(INTDIR)/VideoServices.pch" /YX"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\VideoServices\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/VideoServices.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 \gdk\sdk\lib\ddraw.lib \gdk\sdk\lib\dsound.lib winmm.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /profile /map
LINK32_FLAGS=\gdk\sdk\lib\ddraw.lib \gdk\sdk\lib\dsound.lib winmm.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/VideoServices.pdb"\
 /debug /machine:I386 /out:"$(OUTDIR)/VideoServices.dll"\
 /implib:"$(OUTDIR)/VideoServices.lib" 
LINK32_OBJS= \
	"$(INTDIR)\2DViewPort.obj" \
	"$(INTDIR)\3DViewport.obj" \
	"$(INTDIR)\3DViewportRendering.obj" \
	"$(INTDIR)\Bitmap.obj" \
	"$(INTDIR)\ColorPalette.obj" \
	"$(INTDIR)\ColorTab.obj" \
	"$(INTDIR)\SoundServer.obj" \
	"$(INTDIR)\Sprite.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\VideoBuffer.obj" \
	"$(INTDIR)\VideoBuffer2DDraw.obj" \
	"$(OUTDIR)\ColorTools.lib" \
	"$(OUTDIR)\Util.lib"

"$(OUTDIR)\VideoServices.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Game2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Game2\Release"
# PROP BASE Intermediate_Dir "Game2\Release"
# PROP BASE Target_Dir "Game2"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Game2\Release"
# PROP Target_Dir "Game2"
OUTDIR=.\Release
INTDIR=.\Game2\Release

ALL : "ObjFacTools - Win32 Release" "MainCharacter - Win32 Release"\
 "Model - Win32 Release" "Util - Win32 Release" "VideoServices - Win32 Release"\
 "$(OUTDIR)\Game2.exe"

CLEAN : 
	-@erase "$(INTDIR)\Banner.obj"
	-@erase "$(INTDIR)\ClientSession.obj"
	-@erase "$(INTDIR)\CommonDialog.obj"
	-@erase "$(INTDIR)\Game2.res"
	-@erase "$(INTDIR)\GameApp.obj"
	-@erase "$(INTDIR)\InternetRoom.obj"
	-@erase "$(INTDIR)\License.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MatchReport.obj"
	-@erase "$(INTDIR)\NetInterface.obj"
	-@erase "$(INTDIR)\NetworkSession.obj"
	-@erase "$(INTDIR)\Observer.obj"
	-@erase "$(INTDIR)\Sponsor.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TrackSelect.obj"
	-@erase "$(OUTDIR)\Game2.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_MBNET_" /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_MBNET_" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Game2\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL" /d "_MBNET_"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Game2.res" /d "NDEBUG" /d "_AFXDLL" /d\
 "_MBNET_" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Game2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 vfw32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug
LINK32_FLAGS=vfw32.lib wsock32.lib winmm.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/Game2.pdb" /machine:I386\
 /out:"$(OUTDIR)/Game2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Banner.obj" \
	"$(INTDIR)\ClientSession.obj" \
	"$(INTDIR)\CommonDialog.obj" \
	"$(INTDIR)\Game2.res" \
	"$(INTDIR)\GameApp.obj" \
	"$(INTDIR)\InternetRoom.obj" \
	"$(INTDIR)\License.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MatchReport.obj" \
	"$(INTDIR)\NetInterface.obj" \
	"$(INTDIR)\NetworkSession.obj" \
	"$(INTDIR)\Observer.obj" \
	"$(INTDIR)\Sponsor.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TrackSelect.obj" \
	"$(OUTDIR)\MainCharacter.lib" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\Game2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Game2\Debug"
# PROP BASE Intermediate_Dir "Game2\Debug"
# PROP BASE Target_Dir "Game2"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Game2\Debug"
# PROP Target_Dir "Game2"
OUTDIR=.\Debug
INTDIR=.\Game2\Debug

ALL : "ObjFacTools - Win32 Debug" "MainCharacter - Win32 Debug"\
 "Model - Win32 Debug" "Util - Win32 Debug" "VideoServices - Win32 Debug"\
 "$(OUTDIR)\Game2.exe" "$(OUTDIR)\Game2.bsc"

CLEAN : 
	-@erase "$(INTDIR)\Banner.obj"
	-@erase "$(INTDIR)\Banner.sbr"
	-@erase "$(INTDIR)\ClientSession.obj"
	-@erase "$(INTDIR)\ClientSession.sbr"
	-@erase "$(INTDIR)\CommonDialog.obj"
	-@erase "$(INTDIR)\CommonDialog.sbr"
	-@erase "$(INTDIR)\Game2.res"
	-@erase "$(INTDIR)\GameApp.obj"
	-@erase "$(INTDIR)\GameApp.sbr"
	-@erase "$(INTDIR)\InternetRoom.obj"
	-@erase "$(INTDIR)\InternetRoom.sbr"
	-@erase "$(INTDIR)\License.obj"
	-@erase "$(INTDIR)\License.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\MatchReport.obj"
	-@erase "$(INTDIR)\MatchReport.sbr"
	-@erase "$(INTDIR)\NetInterface.obj"
	-@erase "$(INTDIR)\NetInterface.sbr"
	-@erase "$(INTDIR)\NetworkSession.obj"
	-@erase "$(INTDIR)\NetworkSession.sbr"
	-@erase "$(INTDIR)\Observer.obj"
	-@erase "$(INTDIR)\Observer.sbr"
	-@erase "$(INTDIR)\Sponsor.obj"
	-@erase "$(INTDIR)\Sponsor.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TrackSelect.obj"
	-@erase "$(INTDIR)\TrackSelect.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Game2.bsc"
	-@erase "$(OUTDIR)\Game2.exe"
	-@erase "$(OUTDIR)\Game2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_MBNET_" /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "_MBNET_" /FR"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Game2\Debug/
CPP_SBRS=.\Game2\Debug/

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL" /d "_MBNET_"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Game2.res" /d "_DEBUG" /d "_AFXDLL" /d\
 "_MBNET_" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Game2.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Banner.sbr" \
	"$(INTDIR)\ClientSession.sbr" \
	"$(INTDIR)\CommonDialog.sbr" \
	"$(INTDIR)\GameApp.sbr" \
	"$(INTDIR)\InternetRoom.sbr" \
	"$(INTDIR)\License.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\MatchReport.sbr" \
	"$(INTDIR)\NetInterface.sbr" \
	"$(INTDIR)\NetworkSession.sbr" \
	"$(INTDIR)\Observer.sbr" \
	"$(INTDIR)\Sponsor.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TrackSelect.sbr"

"$(OUTDIR)\Game2.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 \msdev\lib\Vfw32.lib vfw32.lib wsock32.lib winmm.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386
# SUBTRACT LINK32 /profile /map
LINK32_FLAGS=\msdev\lib\Vfw32.lib vfw32.lib wsock32.lib winmm.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/Game2.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/Game2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Banner.obj" \
	"$(INTDIR)\ClientSession.obj" \
	"$(INTDIR)\CommonDialog.obj" \
	"$(INTDIR)\Game2.res" \
	"$(INTDIR)\GameApp.obj" \
	"$(INTDIR)\InternetRoom.obj" \
	"$(INTDIR)\License.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MatchReport.obj" \
	"$(INTDIR)\NetInterface.obj" \
	"$(INTDIR)\NetworkSession.obj" \
	"$(INTDIR)\Observer.obj" \
	"$(INTDIR)\Sponsor.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TrackSelect.obj" \
	"$(OUTDIR)\MainCharacter.lib" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\Game2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PaletteCreator - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PaletteCreator\Debug"
# PROP BASE Intermediate_Dir "PaletteCreator\Debug"
# PROP BASE Target_Dir "PaletteCreator"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "PaletteCreator\Debug"
# PROP Target_Dir "PaletteCreator"
OUTDIR=.\Debug
INTDIR=.\PaletteCreator\Debug

ALL : "ColorTools - Win32 Debug" "$(OUTDIR)\PaletteCreator.exe"

CLEAN : 
	-@erase "$(INTDIR)\PaletteCreator.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PaletteCreator.exe"
	-@erase "$(OUTDIR)\PaletteCreator.ilk"
	-@erase "$(OUTDIR)\PaletteCreator.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\PaletteCreator\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PaletteCreator.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386
# SUBTRACT LINK32 /nodefaultlib
LINK32_FLAGS=/nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/PaletteCreator.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/PaletteCreator.exe" 
LINK32_OBJS= \
	"$(INTDIR)\PaletteCreator.obj" \
	"$(OUTDIR)\ColorTools.lib"

"$(OUTDIR)\PaletteCreator.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ColorTools - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ColorTools\Debug"
# PROP BASE Intermediate_Dir "ColorTools\Debug"
# PROP BASE Target_Dir "ColorTools"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "ColorTools\Debug"
# PROP Target_Dir "ColorTools"
OUTDIR=.\Debug
INTDIR=.\ColorTools\Debug

ALL : "$(OUTDIR)\ColorTools.dll"

CLEAN : 
	-@erase "$(INTDIR)\ColorTools.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\ColorTools.dll"
	-@erase "$(OUTDIR)\ColorTools.exp"
	-@erase "$(OUTDIR)\ColorTools.ilk"
	-@erase "$(OUTDIR)\ColorTools.lib"
	-@erase "$(OUTDIR)\ColorTools.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /O2 /Ob2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MR_COLOR_TOOLS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /Zi /O2 /Ob2 /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "MR_COLOR_TOOLS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ColorTools\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ColorTools.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ColorTools.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/ColorTools.dll" /implib:"$(OUTDIR)/ColorTools.lib" 
LINK32_OBJS= \
	"$(INTDIR)\ColorTools.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\ColorTools.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ObjFacTools\Release"
# PROP BASE Intermediate_Dir "ObjFacTools\Release"
# PROP BASE Target_Dir "ObjFacTools"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "ObjFacTools\Release"
# PROP Target_Dir "ObjFacTools"
OUTDIR=.\Release
INTDIR=.\ObjFacTools\Release

ALL : "Util - Win32 Release" "Model - Win32 Release"\
 "VideoServices - Win32 Release" "$(OUTDIR)\ObjFacTools.dll"

CLEAN : 
	-@erase "$(INTDIR)\BitmapSurface.obj"
	-@erase "$(INTDIR)\FreeElementBase.obj"
	-@erase "$(INTDIR)\FreeElementBaseRenderer.obj"
	-@erase "$(INTDIR)\ObjectFactoryData.obj"
	-@erase "$(INTDIR)\ResActor.obj"
	-@erase "$(INTDIR)\ResBitmap.obj"
	-@erase "$(INTDIR)\ResourceLib.obj"
	-@erase "$(INTDIR)\ResSound.obj"
	-@erase "$(INTDIR)\ResSprite.obj"
	-@erase "$(INTDIR)\SpriteHandle.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\ObjFacTools.dll"
	-@erase "$(OUTDIR)\ObjFacTools.exp"
	-@erase "$(OUTDIR)\ObjFacTools.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MR_OBJ_FAC_TOOLS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "NDEBUG" /D "WIN32"\
 /D "_WINDOWS" /D "MR_OBJ_FAC_TOOLS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\ObjFacTools\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ObjFacTools.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/ObjFacTools.pdb" /machine:I386 /out:"$(OUTDIR)/ObjFacTools.dll"\
 /implib:"$(OUTDIR)/ObjFacTools.lib" 
LINK32_OBJS= \
	"$(INTDIR)\BitmapSurface.obj" \
	"$(INTDIR)\FreeElementBase.obj" \
	"$(INTDIR)\FreeElementBaseRenderer.obj" \
	"$(INTDIR)\ObjectFactoryData.obj" \
	"$(INTDIR)\ResActor.obj" \
	"$(INTDIR)\ResBitmap.obj" \
	"$(INTDIR)\ResourceLib.obj" \
	"$(INTDIR)\ResSound.obj" \
	"$(INTDIR)\ResSprite.obj" \
	"$(INTDIR)\SpriteHandle.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\ObjFacTools.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ObjFacTools\Debug"
# PROP BASE Intermediate_Dir "ObjFacTools\Debug"
# PROP BASE Target_Dir "ObjFacTools"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "ObjFacTools\Debug"
# PROP Target_Dir "ObjFacTools"
OUTDIR=.\Debug
INTDIR=.\ObjFacTools\Debug

ALL : "Util - Win32 Debug" "Model - Win32 Debug" "VideoServices - Win32 Debug"\
 "$(OUTDIR)\ObjFacTools.dll"

CLEAN : 
	-@erase "$(INTDIR)\BitmapSurface.obj"
	-@erase "$(INTDIR)\FreeElementBase.obj"
	-@erase "$(INTDIR)\FreeElementBaseRenderer.obj"
	-@erase "$(INTDIR)\ObjectFactoryData.obj"
	-@erase "$(INTDIR)\ResActor.obj"
	-@erase "$(INTDIR)\ResBitmap.obj"
	-@erase "$(INTDIR)\ResourceLib.obj"
	-@erase "$(INTDIR)\ResSound.obj"
	-@erase "$(INTDIR)\ResSprite.obj"
	-@erase "$(INTDIR)\SpriteHandle.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\ObjFacTools.dll"
	-@erase "$(OUTDIR)\ObjFacTools.exp"
	-@erase "$(OUTDIR)\ObjFacTools.ilk"
	-@erase "$(OUTDIR)\ObjFacTools.lib"
	-@erase "$(OUTDIR)\ObjFacTools.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MR_OBJ_FAC_TOOLS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "_DEBUG" /D\
 "WIN32" /D "_WINDOWS" /D "MR_OBJ_FAC_TOOLS" /D "_WINDLL" /D "_AFXDLL" /D\
 "_MBCS" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ObjFacTools\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

MTL=mktyplib.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ObjFacTools.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /incremental:no
LINK32_FLAGS=/nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/ObjFacTools.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/ObjFacTools.dll" /implib:"$(OUTDIR)/ObjFacTools.lib" 
LINK32_OBJS= \
	"$(INTDIR)\BitmapSurface.obj" \
	"$(INTDIR)\FreeElementBase.obj" \
	"$(INTDIR)\FreeElementBaseRenderer.obj" \
	"$(INTDIR)\ObjectFactoryData.obj" \
	"$(INTDIR)\ResActor.obj" \
	"$(INTDIR)\ResBitmap.obj" \
	"$(INTDIR)\ResourceLib.obj" \
	"$(INTDIR)\ResSound.obj" \
	"$(INTDIR)\ResSprite.obj" \
	"$(INTDIR)\SpriteHandle.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\ObjFacTools.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ResourceCompiler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ResourceCompiler\Debug"
# PROP BASE Intermediate_Dir "ResourceCompiler\Debug"
# PROP BASE Target_Dir "ResourceCompiler"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "ResourceCompiler\Debug"
# PROP Target_Dir "ResourceCompiler"
OUTDIR=.\Debug
INTDIR=.\ResourceCompiler\Debug

ALL : "VideoServices - Win32 Debug" "ColorTools - Win32 Debug"\
 "Util - Win32 Debug" "ObjFacTools - Win32 Debug"\
 "$(OUTDIR)\ResourceCompiler.exe"

CLEAN : 
	-@erase "$(INTDIR)\BitmapHelper.obj"
	-@erase "$(INTDIR)\ResActorBuilder.obj"
	-@erase "$(INTDIR)\ResBitmapBuilder.obj"
	-@erase "$(INTDIR)\ResourceCompiler.obj"
	-@erase "$(INTDIR)\ResourceLibBuilder.obj"
	-@erase "$(INTDIR)\ResSoundBuilder.obj"
	-@erase "$(INTDIR)\ResSpriteBuilder.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TextParsing.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\ResourceCompiler.exe"
	-@erase "$(OUTDIR)\ResourceCompiler.ilk"
	-@erase "$(OUTDIR)\ResourceCompiler.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS"\
 /Fp"$(INTDIR)/ResourceCompiler.pch" /YX"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\ResourceCompiler\Debug/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ResourceCompiler.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/ResourceCompiler.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/ResourceCompiler.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BitmapHelper.obj" \
	"$(INTDIR)\ResActorBuilder.obj" \
	"$(INTDIR)\ResBitmapBuilder.obj" \
	"$(INTDIR)\ResourceCompiler.obj" \
	"$(INTDIR)\ResourceLibBuilder.obj" \
	"$(INTDIR)\ResSoundBuilder.obj" \
	"$(INTDIR)\ResSpriteBuilder.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TextParsing.obj" \
	"$(OUTDIR)\ColorTools.lib" \
	"$(OUTDIR)\ObjFacTools.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\ResourceCompiler.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MazeCompiler\MazeComp"
# PROP BASE Intermediate_Dir "MazeCompiler\MazeComp"
# PROP BASE Target_Dir "MazeCompiler"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "MazeCompiler\Release"
# PROP Target_Dir "MazeCompiler"
OUTDIR=.\Release
INTDIR=.\MazeCompiler\Release

ALL : "VideoServices - Win32 Release" "Model - Win32 Release"\
 "Util - Win32 Release" "$(OUTDIR)\MazeCompiler.exe"

CLEAN : 
	-@erase "$(INTDIR)\LevelBuilder.obj"
	-@erase "$(INTDIR)\LevelBuilderVisiblesZones.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MazeCompiler.res"
	-@erase "$(INTDIR)\Parser.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TrackMap.obj"
	-@erase "$(OUTDIR)\MazeCompiler.exe"
	-@erase "$(OUTDIR)\MazeCompiler.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /GX /Zi /Od /I "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
# ADD CPP /nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "_CONSOLE" /D "WIN32" /D "_AFXDLL" /D "_MBCS" /YX"stdafx.h" /c
CPP_PROJ=/nologo /MD /W4 /GR /GX /O2 /I "\gdk\sdk\inc" /D "_CONSOLE" /D "WIN32"\
 /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/MazeCompiler.pch" /YX"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\MazeCompiler\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/MazeCompiler.res" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/MazeCompiler.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 /nologo /subsystem:console /machine:I386
# SUBTRACT LINK32 /debug
LINK32_FLAGS=/nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/MazeCompiler.pdb" /machine:I386\
 /out:"$(OUTDIR)/MazeCompiler.exe" 
LINK32_OBJS= \
	"$(INTDIR)\LevelBuilder.obj" \
	"$(INTDIR)\LevelBuilderVisiblesZones.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MazeCompiler.res" \
	"$(INTDIR)\Parser.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TrackMap.obj" \
	"$(OUTDIR)\Model.lib" \
	"$(OUTDIR)\Util.lib" \
	"$(OUTDIR)\VideoServices.lib"

"$(OUTDIR)\MazeCompiler.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

################################################################################
# Begin Target

# Name "FireBall - Win32 Release"
# Name "FireBall - Win32 Debug"

!IF  "$(CFG)" == "FireBall - Win32 Release"

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "MazeCompiler"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"MazeCompiler - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="MazeCompiler - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"MazeCompiler - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="MazeCompiler - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ObjFac1"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"ObjFac1 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFac1 - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"ObjFac1 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFac1 - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "GameServer"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"GameServer - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="GameServer - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"GameServer - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="GameServer - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "MainCharacter"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"MainCharacter - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="MainCharacter - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"MainCharacter - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="MainCharacter - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"VideoServices - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="VideoServices - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Game2"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"Game2 - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Game2 - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"Game2 - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Game2 - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "PaletteCreator"

!IF  "$(CFG)" == "FireBall - Win32 Release"

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"PaletteCreator - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="PaletteCreator - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ObjFacTools"

!IF  "$(CFG)" == "FireBall - Win32 Release"

"ObjFacTools - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Release" 

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"ObjFacTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ResourceCompiler"

!IF  "$(CFG)" == "FireBall - Win32 Release"

!ELSEIF  "$(CFG)" == "FireBall - Win32 Debug"

"ResourceCompiler - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="ResourceCompiler - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "Util - Win32 Release"
# Name "Util - Win32 Debug"

!IF  "$(CFG)" == "Util - Win32 Release"

!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Util\RecordFile.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_RECOR=\
	".\Util\RecordFile.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\RecordFile.obj" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_RECOR=\
	".\Util\RecordFile.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\RecordFile.obj" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Util\StdAfx.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_STDAF=\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_STDAF=\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Util\DllObjectFactory.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_DLLOB=\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\DllObjectFactory.obj" : $(SOURCE) $(DEP_CPP_DLLOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_DLLOB=\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\DllObjectFactory.obj" : $(SOURCE) $(DEP_CPP_DLLOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Util\WorldCoordinates.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_WORLD=\
	".\Util\MR_Types.h"\
	".\Util\StdAfx.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\WorldCoordinates.obj" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_WORLD=\
	".\Util\MR_Types.h"\
	".\Util\StdAfx.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\WorldCoordinates.obj" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Util\FuzzyLogic.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_FUZZY=\
	".\Util\FuzzyLogic.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\FuzzyLogic.obj" : $(SOURCE) $(DEP_CPP_FUZZY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_FUZZY=\
	".\Util\FuzzyLogic.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\FuzzyLogic.obj" : $(SOURCE) $(DEP_CPP_FUZZY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Util\Profiler.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_PROFI=\
	".\Util\Profiler.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\Profiler.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_PROFI=\
	".\Util\Profiler.h"\
	".\Util\StdAfx.h"\
	

"$(INTDIR)\Profiler.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Util\StrRes.cpp

!IF  "$(CFG)" == "Util - Win32 Release"

DEP_CPP_STRRE=\
	".\Util\MR_Types.h"\
	".\Util\StdAfx.h"\
	".\Util\StrRes.h"\
	

"$(INTDIR)\StrRes.obj" : $(SOURCE) $(DEP_CPP_STRRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Util - Win32 Debug"

DEP_CPP_STRRE=\
	".\Util\MR_Types.h"\
	".\Util\StdAfx.h"\
	".\Util\StrRes.h"\
	

"$(INTDIR)\StrRes.obj" : $(SOURCE) $(DEP_CPP_STRRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "MazeCompiler - Win32 Debug"
# Name "MazeCompiler - Win32 Release"

!IF  "$(CFG)" == "MazeCompiler - Win32 Debug"

!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Release"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "MazeCompiler - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\main.cpp
DEP_CPP_MAIN_=\
	".\MazeCompiler\LevelBuilder.h"\
	".\MazeCompiler\Parser.h"\
	".\MazeCompiler\StdAfx.h"\
	".\MazeCompiler\TrackCommonStuff.h"\
	".\MazeCompiler\TrackMap.h"\
	".\Model\ContactEffect.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\StdAfx.cpp
DEP_CPP_STDAF=\
	".\MazeCompiler\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Model"

!IF  "$(CFG)" == "MazeCompiler - Win32 Debug"

"Model - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Debug" 

!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Release"

"Model - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Release" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\LevelBuilder.cpp
DEP_CPP_LEVEL=\
	".\MazeCompiler\LevelBuilder.h"\
	".\MazeCompiler\Parser.h"\
	".\MazeCompiler\StdAfx.h"\
	".\Model\ContactEffect.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\LevelBuilder.obj" : $(SOURCE) $(DEP_CPP_LEVEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\Parser.cpp
DEP_CPP_PARSE=\
	".\MazeCompiler\Parser.h"\
	".\MazeCompiler\StdAfx.h"\
	

"$(INTDIR)\Parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\LevelBuilderVisiblesZones.cpp
DEP_CPP_LEVELB=\
	".\MazeCompiler\LevelBuilder.h"\
	".\MazeCompiler\StdAfx.h"\
	".\Model\ContactEffect.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\LevelBuilderVisiblesZones.obj" : $(SOURCE) $(DEP_CPP_LEVELB)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\TrackMap.cpp
DEP_CPP_TRACK=\
	".\MazeCompiler\StdAfx.h"\
	".\MazeCompiler\TrackMap.h"\
	".\Model\ContactEffect.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\TrackMap.obj" : $(SOURCE) $(DEP_CPP_TRACK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "MazeCompiler - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Release"

"VideoServices - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="VideoServices - Win32 Release" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\MazeCompiler\MazeCompiler.rc

!IF  "$(CFG)" == "MazeCompiler - Win32 Debug"


"$(INTDIR)\MazeCompiler.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/MazeCompiler.res" /i "MazeCompiler" /d\
 "_DEBUG" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeCompiler - Win32 Release"


"$(INTDIR)\MazeCompiler.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/MazeCompiler.res" /i "MazeCompiler" /d\
 "_AFXDLL" $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "Model - Win32 Release"
# Name "Model - Win32 Debug"

!IF  "$(CFG)" == "Model - Win32 Release"

!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

!ENDIF 

################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "Model - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\Model\StdAfx.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_STDAF=\
	".\Model\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_STDAF=\
	".\Model\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\Level.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_LEVEL_=\
	".\Model\ContactEffect.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\Level.obj" : $(SOURCE) $(DEP_CPP_LEVEL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_LEVEL_=\
	".\Model\ContactEffect.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\Level.obj" : $(SOURCE) $(DEP_CPP_LEVEL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\MazeElement.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_MAZEE=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\MazeElement.obj" : $(SOURCE) $(DEP_CPP_MAZEE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_MAZEE=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\MazeElement.obj" : $(SOURCE) $(DEP_CPP_MAZEE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\ConcreteShape.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_CONCR=\
	".\Model\ConcreteShape.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\ConcreteShape.obj" : $(SOURCE) $(DEP_CPP_CONCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_CONCR=\
	".\Model\ConcreteShape.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\ConcreteShape.obj" : $(SOURCE) $(DEP_CPP_CONCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\Shapes.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_SHAPE=\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\Shapes.obj" : $(SOURCE) $(DEP_CPP_SHAPE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_SHAPE=\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\Shapes.obj" : $(SOURCE) $(DEP_CPP_SHAPE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\ContactEffect.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_CONTA=\
	".\Model\ContactEffect.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\ContactEffect.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_CONTA=\
	".\Model\ContactEffect.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\ContactEffect.obj" : $(SOURCE) $(DEP_CPP_CONTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\ShapeCollisions.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_SHAPEC=\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\ShapeCollisions.obj" : $(SOURCE) $(DEP_CPP_SHAPEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_SHAPEC=\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\ShapeCollisions.obj" : $(SOURCE) $(DEP_CPP_SHAPEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\GameSession.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_GAMES=\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\GameSession.obj" : $(SOURCE) $(DEP_CPP_GAMES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_GAMES=\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\GameSession.obj" : $(SOURCE) $(DEP_CPP_GAMES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\PhysicalCollision.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_PHYSI=\
	".\Model\ContactEffect.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\PhysicalCollision.obj" : $(SOURCE) $(DEP_CPP_PHYSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_PHYSI=\
	".\Model\ContactEffect.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	

"$(INTDIR)\PhysicalCollision.obj" : $(SOURCE) $(DEP_CPP_PHYSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Model\FreeElementMovingHelper.cpp

!IF  "$(CFG)" == "Model - Win32 Release"

DEP_CPP_FREEE=\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\FreeElementMovingHelper.obj" : $(SOURCE) $(DEP_CPP_FREEE)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Model - Win32 Debug"

DEP_CPP_FREEE=\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Model\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\FreeElementMovingHelper.obj" : $(SOURCE) $(DEP_CPP_FREEE)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "ObjFac1 - Win32 Release"
# Name "ObjFac1 - Win32 Debug"

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ObjFac1\StdAfx.cpp
DEP_CPP_STDAF=\
	".\ObjFac1\StdAfx.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\main.cpp
DEP_CPP_MAIN_=\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\RaceEffects.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\BabeElement.h"\
	".\ObjFac1\BallElement.h"\
	".\ObjFac1\BumperGate.h"\
	".\ObjFac1\DefaultSurface.h"\
	".\ObjFac1\DoubleSpeedSource.h"\
	".\ObjFac1\FinishLine.h"\
	".\ObjFac1\FuelSource.h"\
	".\ObjFac1\GenericRenderer.h"\
	".\ObjFac1\HoverRender.h"\
	".\ObjFac1\Mine.h"\
	".\ObjFac1\Missile.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\PowerUp.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFac1\TestElement.h"\
	".\ObjFac1\WoodSurface.h"\
	".\ObjFacTools\BitmapSurface.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\FreeElementBaseRenderer.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\SpriteHandle.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\main.sbr" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\DefaultSurface.cpp
DEP_CPP_DEFAU=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\DefaultSurface.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\BitmapSurface.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\DefaultSurface.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\DefaultSurface.obj" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DefaultSurface.sbr" : $(SOURCE) $(DEP_CPP_DEFAU) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\WoodSurface.cpp
DEP_CPP_WOODS=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFac1\WoodSurface.h"\
	".\ObjFacTools\BitmapSurface.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\WoodSurface.obj" : $(SOURCE) $(DEP_CPP_WOODS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\WoodSurface.obj" : $(SOURCE) $(DEP_CPP_WOODS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\WoodSurface.sbr" : $(SOURCE) $(DEP_CPP_WOODS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\BallElement.cpp
DEP_CPP_BALLE=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\BallElement.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\BallElement.obj" : $(SOURCE) $(DEP_CPP_BALLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\BallElement.obj" : $(SOURCE) $(DEP_CPP_BALLE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\BallElement.sbr" : $(SOURCE) $(DEP_CPP_BALLE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\BabeElement.cpp
DEP_CPP_BABEE=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\BabeElement.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\BabeElement.obj" : $(SOURCE) $(DEP_CPP_BABEE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\BabeElement.obj" : $(SOURCE) $(DEP_CPP_BABEE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\BabeElement.sbr" : $(SOURCE) $(DEP_CPP_BABEE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\TestElement.cpp
DEP_CPP_TESTE=\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFac1\TestElement.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\TestElement.obj" : $(SOURCE) $(DEP_CPP_TESTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\TestElement.obj" : $(SOURCE) $(DEP_CPP_TESTE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TestElement.sbr" : $(SOURCE) $(DEP_CPP_TESTE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\GenericRenderer.cpp
DEP_CPP_GENER=\
	".\ObjFac1\GenericRenderer.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBaseRenderer.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\GenericRenderer.obj" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\GenericRenderer.obj" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\GenericRenderer.sbr" : $(SOURCE) $(DEP_CPP_GENER) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\FuelSource.cpp
DEP_CPP_FUELS=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\RaceEffects.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\FuelSource.h"\
	".\ObjFac1\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\FuelSource.obj" : $(SOURCE) $(DEP_CPP_FUELS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\FuelSource.obj" : $(SOURCE) $(DEP_CPP_FUELS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\FuelSource.sbr" : $(SOURCE) $(DEP_CPP_FUELS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\DoubleSpeedSource.cpp
DEP_CPP_DOUBL=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\RaceEffects.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\DoubleSpeedSource.h"\
	".\ObjFac1\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\DoubleSpeedSource.obj" : $(SOURCE) $(DEP_CPP_DOUBL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\DoubleSpeedSource.obj" : $(SOURCE) $(DEP_CPP_DOUBL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\DoubleSpeedSource.sbr" : $(SOURCE) $(DEP_CPP_DOUBL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\HoverRender.cpp
DEP_CPP_HOVER=\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\HoverRender.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FuzzyLogic.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\HoverRender.obj" : $(SOURCE) $(DEP_CPP_HOVER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\HoverRender.obj" : $(SOURCE) $(DEP_CPP_HOVER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\HoverRender.sbr" : $(SOURCE) $(DEP_CPP_HOVER) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "MainCharacter"

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"

"MainCharacter - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="MainCharacter - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

"MainCharacter - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="MainCharacter - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\FinishLine.cpp
DEP_CPP_FINIS=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\RaceEffects.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\FinishLine.h"\
	".\ObjFac1\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\FinishLine.obj" : $(SOURCE) $(DEP_CPP_FINIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\FinishLine.obj" : $(SOURCE) $(DEP_CPP_FINIS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\FinishLine.sbr" : $(SOURCE) $(DEP_CPP_FINIS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\Missile.cpp
DEP_CPP_MISSI=\
	".\Model\ConcreteShape.h"\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\RaceEffects.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\Missile.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\Missile.obj" : $(SOURCE) $(DEP_CPP_MISSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\Missile.obj" : $(SOURCE) $(DEP_CPP_MISSI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Missile.sbr" : $(SOURCE) $(DEP_CPP_MISSI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Model"

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"

"Model - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

"Model - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ObjFacTools"

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"

"ObjFacTools - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

"ObjFacTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"

"VideoServices - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="VideoServices - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\BumperGate.cpp
DEP_CPP_BUMPE=\
	".\Model\ConcreteShape.h"\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\RaceEffects.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\BumperGate.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\BumperGate.obj" : $(SOURCE) $(DEP_CPP_BUMPE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\BumperGate.obj" : $(SOURCE) $(DEP_CPP_BUMPE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\BumperGate.sbr" : $(SOURCE) $(DEP_CPP_BUMPE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\PowerUp.cpp
DEP_CPP_POWER=\
	".\Model\ConcreteShape.h"\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\RaceEffects.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\PowerUp.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\PowerUp.obj" : $(SOURCE) $(DEP_CPP_POWER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\PowerUp.obj" : $(SOURCE) $(DEP_CPP_POWER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\PowerUp.sbr" : $(SOURCE) $(DEP_CPP_POWER) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFac1\Mine.cpp
DEP_CPP_MINE_=\
	".\Model\ConcreteShape.h"\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\RaceEffects.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFac1\Mine.h"\
	".\ObjFac1\ObjFac1Res.h"\
	".\ObjFac1\StdAfx.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "ObjFac1 - Win32 Release"


"$(INTDIR)\Mine.obj" : $(SOURCE) $(DEP_CPP_MINE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFac1 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\Mine.obj" : $(SOURCE) $(DEP_CPP_MINE_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Mine.sbr" : $(SOURCE) $(DEP_CPP_MINE_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "GameServer - Win32 Release"
# Name "GameServer - Win32 Debug"

!IF  "$(CFG)" == "GameServer - Win32 Release"

!ELSEIF  "$(CFG)" == "GameServer - Win32 Debug"

!ENDIF 

# End Target
################################################################################
# Begin Target

# Name "MainCharacter - Win32 Release"
# Name "MainCharacter - Win32 Debug"

!IF  "$(CFG)" == "MainCharacter - Win32 Release"

!ELSEIF  "$(CFG)" == "MainCharacter - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\MainCharacter\StdAfx.cpp
DEP_CPP_STDAF=\
	".\MainCharacter\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainCharacter\MainCharacter.cpp
DEP_CPP_MAINC=\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\MainCharacter\StdAfx.h"\
	".\Model\ContactEffect.h"\
	".\Model\FreeElementMovingHelper.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\RaceEffects.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\FuzzyLogic.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\MainCharacter.obj" : $(SOURCE) $(DEP_CPP_MAINC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Model"

!IF  "$(CFG)" == "MainCharacter - Win32 Release"

"Model - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Release" 

!ELSEIF  "$(CFG)" == "MainCharacter - Win32 Debug"

"Model - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "MainCharacter - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ELSEIF  "$(CFG)" == "MainCharacter - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\MainCharacter\MainCharacterRenderer.cpp
DEP_CPP_MAINCH=\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\MainCharacter\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\MainCharacterRenderer.obj" : $(SOURCE) $(DEP_CPP_MAINCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "MainCharacter - Win32 Release"

"VideoServices - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="VideoServices - Win32 Release" 

!ELSEIF  "$(CFG)" == "MainCharacter - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "VideoServices - Win32 Release"
# Name "VideoServices - Win32 Debug"

!IF  "$(CFG)" == "VideoServices - Win32 Release"

!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\VideoServices\VideoBuffer.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_VIDEO=\
	".\Util\MR_Types.h"\
	".\Util\Profiler.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\VideoBuffer.obj" : $(SOURCE) $(DEP_CPP_VIDEO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_VIDEO=\
	".\Util\MR_Types.h"\
	".\Util\Profiler.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\VideoBuffer.obj" : $(SOURCE) $(DEP_CPP_VIDEO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\StdAfx.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_STDAF=\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_STDAF=\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\ColorPalette.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_COLOR=\
	".\Util\MR_Types.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\ColorPalette.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_COLOR=\
	".\Util\MR_Types.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\ColorPalette.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\VideoBuffer2DDraw.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_VIDEOB=\
	".\Util\MR_Types.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\VideoBuffer2DDraw.obj" : $(SOURCE) $(DEP_CPP_VIDEOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_VIDEOB=\
	".\Util\MR_Types.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\VideoBuffer2DDraw.obj" : $(SOURCE) $(DEP_CPP_VIDEOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\ColorTab.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_COLORT=\
	".\Util\MR_Types.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\ColorTab.obj" : $(SOURCE) $(DEP_CPP_COLORT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_COLORT=\
	".\Util\MR_Types.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\ColorTab.obj" : $(SOURCE) $(DEP_CPP_COLORT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\2DViewPort.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_2DVIE=\
	".\Util\MR_Types.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\2DViewPort.obj" : $(SOURCE) $(DEP_CPP_2DVIE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_2DVIE=\
	".\Util\MR_Types.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\2DViewPort.obj" : $(SOURCE) $(DEP_CPP_2DVIE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\3DViewport.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_3DVIE=\
	".\Util\FastMemManip.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\3DViewport.obj" : $(SOURCE) $(DEP_CPP_3DVIE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_3DVIE=\
	".\Util\FastMemManip.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\3DViewport.obj" : $(SOURCE) $(DEP_CPP_3DVIE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "VideoServices - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\VideoServices\3DViewportRendering.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_3DVIEW=\
	".\Util\MR_Types.h"\
	".\Util\Profiler.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	
# ADD CPP /GB /GR- /GX- /Ox /Ot /Oa /Og /Oi /Oy /Ob2
# SUBTRACT CPP /Ow

"$(INTDIR)\3DViewportRendering.obj" : $(SOURCE) $(DEP_CPP_3DVIEW) "$(INTDIR)"
   $(CPP) /nologo /GB /MD /W4 /GR- /GX- /Ox /Ot /Oa /Og /Oi /Oy /Ob2 /I\
 "\gdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MR_VIDEO_SERVICES" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_3DVIEW=\
	".\Util\MR_Types.h"\
	".\Util\Profiler.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	
# ADD CPP /GR- /GX- /Ox /Ot /Oa /Oi /Oy /Ob1
# SUBTRACT CPP /Og

"$(INTDIR)\3DViewportRendering.obj" : $(SOURCE) $(DEP_CPP_3DVIEW) "$(INTDIR)"
   $(CPP) /nologo /MDd /W4 /Gm /GR- /GX- /Zi /Ox /Ot /Oa /Oi /Oy /Ob1 /I\
 "\gdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MR_VIDEO_SERVICES" /D\
 "_WINDLL" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/VideoServices.pch"\
 /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\Bitmap.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_BITMA=\
	".\Util\MR_Types.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\Bitmap.obj" : $(SOURCE) $(DEP_CPP_BITMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_BITMA=\
	".\Util\MR_Types.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\StdAfx.h"\
	

"$(INTDIR)\Bitmap.obj" : $(SOURCE) $(DEP_CPP_BITMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ColorTools"

!IF  "$(CFG)" == "VideoServices - Win32 Release"

!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

"ColorTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ColorTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\VideoServices\Sprite.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_SPRIT=\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\Sprite.obj" : $(SOURCE) $(DEP_CPP_SPRIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_SPRIT=\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\StdAfx.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\Sprite.obj" : $(SOURCE) $(DEP_CPP_SPRIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoServices\SoundServer.cpp

!IF  "$(CFG)" == "VideoServices - Win32 Release"

DEP_CPP_SOUND=\
	".\Util\MR_Types.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\StdAfx.h"\
	"\gdk\sdk\inc\dsound.h"\
	

"$(INTDIR)\SoundServer.obj" : $(SOURCE) $(DEP_CPP_SOUND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "VideoServices - Win32 Debug"

DEP_CPP_SOUND=\
	".\Util\MR_Types.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\StdAfx.h"\
	"\gdk\sdk\inc\dsound.h"\
	

"$(INTDIR)\SoundServer.obj" : $(SOURCE) $(DEP_CPP_SOUND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "Game2 - Win32 Release"
# Name "Game2 - Win32 Debug"

!IF  "$(CFG)" == "Game2 - Win32 Release"

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Game2\StdAfx.cpp
DEP_CPP_STDAF=\
	".\Game2\StdAfx.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\main.cpp
DEP_CPP_MAIN_=\
	".\Game2\ClientSession.h"\
	".\Game2\GameApp.h"\
	".\Game2\Observer.h"\
	".\Game2\StdAfx.h"\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\SpriteHandle.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\main.sbr" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\GameApp.cpp
DEP_CPP_GAMEA=\
	".\Game2\Banner.h"\
	".\Game2\ClientSession.h"\
	".\Game2\CommonDialog.h"\
	".\Game2\GameApp.h"\
	".\Game2\InternetRoom.h"\
	".\Game2\NetInterface.h"\
	".\Game2\NetworkSession.h"\
	".\Game2\Observer.h"\
	".\Game2\Security.h"\
	".\Game2\Sponsor.h"\
	".\Game2\StdAfx.h"\
	".\Game2\TrackSelect.h"\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\SpriteHandle.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\FuzzyLogic.h"\
	".\Util\MR_Types.h"\
	".\Util\Profiler.h"\
	".\Util\RecordFile.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\GameApp.obj" : $(SOURCE) $(DEP_CPP_GAMEA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\GameApp.obj" : $(SOURCE) $(DEP_CPP_GAMEA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\GameApp.sbr" : $(SOURCE) $(DEP_CPP_GAMEA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\Game2.rc
DEP_RSC_GAME2=\
	".\Game2\icon3.ico"\
	".\Game2\intro1.bmp"\
	".\Game2\introeon.bmp"\
	".\Game2\warning.ico"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\Game2.res" : $(SOURCE) $(DEP_RSC_GAME2) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/Game2.res" /i "Game2" /d "NDEBUG" /d "_AFXDLL"\
 /d "_MBNET_" $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


"$(INTDIR)\Game2.res" : $(SOURCE) $(DEP_RSC_GAME2) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/Game2.res" /i "Game2" /d "_DEBUG" /d "_AFXDLL"\
 /d "_MBNET_" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "Game2 - Win32 Release"

"VideoServices - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="VideoServices - Win32 Release" 

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "Game2 - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\Game2\CommonDialog.cpp
DEP_CPP_COMMO=\
	".\Game2\CommonDialog.h"\
	".\Game2\StdAfx.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\CommonDialog.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CommonDialog.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CommonDialog.sbr" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Model"

!IF  "$(CFG)" == "Game2 - Win32 Release"

"Model - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Release" 

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

"Model - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "MainCharacter"

!IF  "$(CFG)" == "Game2 - Win32 Release"

"MainCharacter - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="MainCharacter - Win32 Release" 

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

"MainCharacter - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="MainCharacter - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\Game2\ClientSession.cpp
DEP_CPP_CLIEN=\
	".\Game2\ClientSession.h"\
	".\Game2\StdAfx.h"\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\MazeCompiler\TrackCommonStuff.h"\
	".\Model\ContactEffect.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\ClientSession.obj" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ClientSession.obj" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ClientSession.sbr" : $(SOURCE) $(DEP_CPP_CLIEN) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\NetworkSession.cpp
DEP_CPP_NETWO=\
	".\Game2\Banner.h"\
	".\Game2\ClientSession.h"\
	".\Game2\InternetRoom.h"\
	".\Game2\NetInterface.h"\
	".\Game2\NetworkSession.h"\
	".\Game2\StdAfx.h"\
	".\Game2\TrackSelect.h"\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\NetworkSession.obj" : $(SOURCE) $(DEP_CPP_NETWO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\NetworkSession.obj" : $(SOURCE) $(DEP_CPP_NETWO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\NetworkSession.sbr" : $(SOURCE) $(DEP_CPP_NETWO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\License.cpp
DEP_CPP_LICEN=\
	".\Game2\License.h"\
	".\Game2\StdAfx.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\License.obj" : $(SOURCE) $(DEP_CPP_LICEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\License.obj" : $(SOURCE) $(DEP_CPP_LICEN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\License.sbr" : $(SOURCE) $(DEP_CPP_LICEN) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\TrackSelect.cpp
DEP_CPP_TRACKS=\
	".\Game2\License.h"\
	".\Game2\StdAfx.h"\
	".\Game2\TrackSelect.h"\
	".\MazeCompiler\TrackCommonStuff.h"\
	".\Util\Cursor.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\StrRes.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\TrackSelect.obj" : $(SOURCE) $(DEP_CPP_TRACKS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\TrackSelect.obj" : $(SOURCE) $(DEP_CPP_TRACKS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TrackSelect.sbr" : $(SOURCE) $(DEP_CPP_TRACKS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\Observer.cpp
DEP_CPP_OBSER=\
	".\Game2\ClientSession.h"\
	".\Game2\Observer.h"\
	".\Game2\StdAfx.h"\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\SpriteHandle.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\MR_Types.h"\
	".\Util\Profiler.h"\
	".\Util\RecordFile.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\Observer.obj" : $(SOURCE) $(DEP_CPP_OBSER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\Observer.obj" : $(SOURCE) $(DEP_CPP_OBSER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Observer.sbr" : $(SOURCE) $(DEP_CPP_OBSER) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ObjFacTools"

!IF  "$(CFG)" == "Game2 - Win32 Release"

"ObjFacTools - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Release" 

!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"

"ObjFacTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\Game2\NetInterface.cpp
DEP_CPP_NETIN=\
	".\Game2\NetInterface.h"\
	".\Game2\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\StrRes.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\NetInterface.obj" : $(SOURCE) $(DEP_CPP_NETIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\NetInterface.obj" : $(SOURCE) $(DEP_CPP_NETIN) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\NetInterface.sbr" : $(SOURCE) $(DEP_CPP_NETIN) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\InternetRoom.cpp
DEP_CPP_INTER=\
	".\Game2\Banner.h"\
	".\Game2\ClientSession.h"\
	".\Game2\InternetRoom.h"\
	".\Game2\MatchReport.h"\
	".\Game2\NetInterface.h"\
	".\Game2\NetworkSession.h"\
	".\Game2\StdAfx.h"\
	".\Game2\TrackSelect.h"\
	".\MainCharacter\MainCharacter.h"\
	".\MainCharacter\MainCharacterRenderer.h"\
	".\Model\ContactEffect.h"\
	".\Model\GameSession.h"\
	".\Model\Level.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\ShapeCollisions.h"\
	".\Model\Shapes.h"\
	".\Util\BitPacking.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\FastArray.h"\
	".\Util\FastFifo.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\StrRes.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\InternetRoom.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\InternetRoom.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\InternetRoom.sbr" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\Banner.cpp
DEP_CPP_BANNE=\
	".\Game2\Banner.h"\
	".\Game2\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\Util\StrRes.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\Banner.obj" : $(SOURCE) $(DEP_CPP_BANNE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\Banner.obj" : $(SOURCE) $(DEP_CPP_BANNE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Banner.sbr" : $(SOURCE) $(DEP_CPP_BANNE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\Sponsor.cpp
DEP_CPP_SPONS=\
	".\Game2\Banner.h"\
	".\Game2\Sponsor.h"\
	".\Game2\StdAfx.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\Sponsor.obj" : $(SOURCE) $(DEP_CPP_SPONS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\Sponsor.obj" : $(SOURCE) $(DEP_CPP_SPONS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Sponsor.sbr" : $(SOURCE) $(DEP_CPP_SPONS) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Game2\MatchReport.cpp
DEP_CPP_MATCH=\
	".\Game2\Banner.h"\
	".\Game2\MatchReport.h"\
	".\Game2\StdAfx.h"\
	

!IF  "$(CFG)" == "Game2 - Win32 Release"


"$(INTDIR)\MatchReport.obj" : $(SOURCE) $(DEP_CPP_MATCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Game2 - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\MatchReport.obj" : $(SOURCE) $(DEP_CPP_MATCH) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\MatchReport.sbr" : $(SOURCE) $(DEP_CPP_MATCH) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "PaletteCreator - Win32 Debug"
################################################################################
# Begin Source File

SOURCE=.\PaletteCreator\PaletteCreator.cpp
DEP_CPP_PALET=\
	".\ColorTools\ColorTools.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\ColorPalette.h"\
	

"$(INTDIR)\PaletteCreator.obj" : $(SOURCE) $(DEP_CPP_PALET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ColorTools"

!IF  "$(CFG)" == "PaletteCreator - Win32 Debug"

"ColorTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ColorTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
################################################################################
# Begin Target

# Name "ColorTools - Win32 Debug"
################################################################################
# Begin Source File

SOURCE=.\ColorTools\ColorTools.cpp
DEP_CPP_COLORTO=\
	".\ColorTools\ColorTools.h"\
	".\ColorTools\StdAfx.h"\
	".\Util\MR_Types.h"\
	

"$(INTDIR)\ColorTools.obj" : $(SOURCE) $(DEP_CPP_COLORTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ColorTools\StdAfx.cpp
DEP_CPP_STDAF=\
	".\ColorTools\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
################################################################################
# Begin Target

# Name "ObjFacTools - Win32 Release"
# Name "ObjFacTools - Win32 Debug"

!IF  "$(CFG)" == "ObjFacTools - Win32 Release"

!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\StdAfx.cpp
DEP_CPP_STDAF=\
	".\ObjFacTools\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "ObjFacTools - Win32 Release"

"VideoServices - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak"\
 CFG="VideoServices - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\ObjectFactoryData.cpp
DEP_CPP_OBJEC=\
	".\ObjFacTools\ObjectFactoryData.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ObjectFactoryData.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\BitmapSurface.cpp
DEP_CPP_BITMAP=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\PhysicalCollision.h"\
	".\Model\Shapes.h"\
	".\ObjFacTools\BitmapSurface.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\BitmapSurface.obj" : $(SOURCE) $(DEP_CPP_BITMAP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Model"

!IF  "$(CFG)" == "ObjFacTools - Win32 Release"

"Model - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Debug"

"Model - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Model - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\ResBitmap.cpp
DEP_CPP_RESBI=\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\Bitmap.h"\
	

"$(INTDIR)\ResBitmap.obj" : $(SOURCE) $(DEP_CPP_RESBI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\ResourceLib.cpp
DEP_CPP_RESOU=\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResourceLib.obj" : $(SOURCE) $(DEP_CPP_RESOU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\ResActor.cpp
DEP_CPP_RESAC=\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResActor.obj" : $(SOURCE) $(DEP_CPP_RESAC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "ObjFacTools - Win32 Release"

"Util - Win32 Release" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Release" 

!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\FreeElementBase.cpp

!IF  "$(CFG)" == "ObjFacTools - Win32 Release"

DEP_CPP_FREEEL=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\FreeElementBase.obj" : $(SOURCE) $(DEP_CPP_FREEEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ObjFacTools - Win32 Debug"

DEP_CPP_FREEEL=\
	".\Model\ContactEffect.h"\
	".\Model\MazeElement.h"\
	".\Model\Shapes.h"\
	".\ObjFacTools\FreeElementBase.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\FreeElementBase.obj" : $(SOURCE) $(DEP_CPP_FREEEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\FreeElementBaseRenderer.cpp
DEP_CPP_FREEELE=\
	".\ObjFacTools\FreeElementBaseRenderer.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\FreeElementBaseRenderer.obj" : $(SOURCE) $(DEP_CPP_FREEELE)\
 "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\ResSprite.cpp
DEP_CPP_RESSP=\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResSprite.obj" : $(SOURCE) $(DEP_CPP_RESSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\SpriteHandle.cpp
DEP_CPP_SPRITE=\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ObjFacTools\SpriteHandle.h"\
	".\ObjFacTools\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\SpriteHandle.obj" : $(SOURCE) $(DEP_CPP_SPRITE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ObjFacTools\ResSound.cpp
DEP_CPP_RESSO=\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\StdAfx.h"\
	".\VideoServices\SoundServer.h"\
	

"$(INTDIR)\ResSound.obj" : $(SOURCE) $(DEP_CPP_RESSO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
################################################################################
# Begin Target

# Name "ResourceCompiler - Win32 Debug"
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\ResourceCompiler.cpp
DEP_CPP_RESOUR=\
	".\ColorTools\ColorTools.h"\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ResourceCompiler\ResActorBuilder.h"\
	".\ResourceCompiler\ResBitmapBuilder.h"\
	".\ResourceCompiler\ResourceLibBuilder.h"\
	".\ResourceCompiler\ResSoundBuilder.h"\
	".\ResourceCompiler\ResSpriteBuilder.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\ResourceCompiler\TextParsing.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResourceCompiler.obj" : $(SOURCE) $(DEP_CPP_RESOUR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\StdAfx.cpp
DEP_CPP_STDAF=\
	".\ResourceCompiler\StdAfx.h"\
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\TextParsing.cpp
DEP_CPP_TEXTP=\
	".\ResourceCompiler\StdAfx.h"\
	".\ResourceCompiler\TextParsing.h"\
	

"$(INTDIR)\TextParsing.obj" : $(SOURCE) $(DEP_CPP_TEXTP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\ResourceLibBuilder.cpp
DEP_CPP_RESOURC=\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ResourceCompiler\ResourceLibBuilder.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResourceLibBuilder.obj" : $(SOURCE) $(DEP_CPP_RESOURC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ObjFacTools"

!IF  "$(CFG)" == "ResourceCompiler - Win32 Debug"

"ObjFacTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ObjFacTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Project Dependency

# Project_Dep_Name "Util"

!IF  "$(CFG)" == "ResourceCompiler - Win32 Debug"

"Util - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="Util - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\ResActorBuilder.cpp
DEP_CPP_RESACT=\
	".\ObjFacTools\ResActor.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ObjFacTools\ResourceLib.h"\
	".\ObjFacTools\ResSound.h"\
	".\ObjFacTools\ResSprite.h"\
	".\ResourceCompiler\ResActorBuilder.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\ResourceCompiler\TextParsing.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\Util\RecordFile.h"\
	".\Util\WorldCoordinates.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\3DViewport.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	".\VideoServices\Patch.h"\
	".\VideoServices\SoundServer.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResActorBuilder.obj" : $(SOURCE) $(DEP_CPP_RESACT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\ResBitmapBuilder.cpp
DEP_CPP_RESBIT=\
	".\ColorTools\ColorTools.h"\
	".\ObjFacTools\ResBitmap.h"\
	".\ResourceCompiler\BitmapHelper.h"\
	".\ResourceCompiler\ResBitmapBuilder.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\Bitmap.h"\
	".\VideoServices\ColorPalette.h"\
	

"$(INTDIR)\ResBitmapBuilder.obj" : $(SOURCE) $(DEP_CPP_RESBIT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "ColorTools"

!IF  "$(CFG)" == "ResourceCompiler - Win32 Debug"

"ColorTools - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="ColorTools - Win32 Debug" 

!ENDIF 

# End Project Dependency
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\BitmapHelper.cpp
DEP_CPP_BITMAPH=\
	".\ResourceCompiler\BitmapHelper.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\Util\MR_Types.h"\
	

"$(INTDIR)\BitmapHelper.obj" : $(SOURCE) $(DEP_CPP_BITMAPH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\ResSpriteBuilder.cpp
DEP_CPP_RESSPR=\
	".\ObjFacTools\ResSprite.h"\
	".\ResourceCompiler\BitmapHelper.h"\
	".\ResourceCompiler\ResSpriteBuilder.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\Util\DllObjectFactory.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\2DViewport.h"\
	".\VideoServices\Sprite.h"\
	".\VideoServices\VideoBuffer.h"\
	"\gdk\sdk\inc\ddraw.h"\
	

"$(INTDIR)\ResSpriteBuilder.obj" : $(SOURCE) $(DEP_CPP_RESSPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ResourceCompiler\ResSoundBuilder.cpp
DEP_CPP_RESSOU=\
	".\ObjFacTools\ResSound.h"\
	".\ResourceCompiler\BitmapHelper.h"\
	".\ResourceCompiler\ResSoundBuilder.h"\
	".\ResourceCompiler\StdAfx.h"\
	".\Util\MR_Types.h"\
	".\VideoServices\SoundServer.h"\
	

"$(INTDIR)\ResSoundBuilder.obj" : $(SOURCE) $(DEP_CPP_RESSOU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Project Dependency

# Project_Dep_Name "VideoServices"

!IF  "$(CFG)" == "ResourceCompiler - Win32 Debug"

"VideoServices - Win32 Debug" : 
   $(MAKE) /$(MAKEFLAGS) /F ".\NetTarget.mak" CFG="VideoServices - Win32 Debug" 

!ENDIF 

# End Project Dependency
# End Target
# End Project
################################################################################
