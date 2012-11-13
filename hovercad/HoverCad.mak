# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=HoverCad - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to HoverCad - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "HoverCad - Win32 Release" && "$(CFG)" !=\
 "HoverCad - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "HoverCad.mak" CFG="HoverCad - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HoverCad - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "HoverCad - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "HoverCad - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "HoverCad - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\NetTarget\Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\..\NetTarget\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\HoverCad.exe" ".\Release\HoverCad.pch"

CLEAN : 
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\GDIResources.obj"
	-@erase "$(INTDIR)\HoverCad.obj"
	-@erase "$(INTDIR)\HoverCad.pch"
	-@erase "$(INTDIR)\HoverCad.res"
	-@erase "$(INTDIR)\HoverCadDoc.obj"
	-@erase "$(INTDIR)\HoverCadView.obj"
	-@erase "$(INTDIR)\HoverTypes.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\ScaleSelect.obj"
	-@erase "$(INTDIR)\SelectAttrib.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\ZoomBar.obj"
	-@erase "$(OUTDIR)\HoverCad.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /MD /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /G5 /MD /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/HoverCad.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/HoverCad.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/HoverCad.pdb" /machine:I386 /out:"$(OUTDIR)/HoverCad.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\GDIResources.obj" \
	"$(INTDIR)\HoverCad.obj" \
	"$(INTDIR)\HoverCad.res" \
	"$(INTDIR)\HoverCadDoc.obj" \
	"$(INTDIR)\HoverCadView.obj" \
	"$(INTDIR)\HoverTypes.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\ScaleSelect.obj" \
	"$(INTDIR)\SelectAttrib.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\ZoomBar.obj"

"$(OUTDIR)\HoverCad.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\NetTarget\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\..\NetTarget\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\HoverCad.exe" ".\Debug\HoverCad.pch" "$(OUTDIR)\HoverCad.bsc"

CLEAN : 
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\GDIResources.obj"
	-@erase "$(INTDIR)\GDIResources.sbr"
	-@erase "$(INTDIR)\HoverCad.obj"
	-@erase "$(INTDIR)\HoverCad.pch"
	-@erase "$(INTDIR)\HoverCad.res"
	-@erase "$(INTDIR)\HoverCad.sbr"
	-@erase "$(INTDIR)\HoverCadDoc.obj"
	-@erase "$(INTDIR)\HoverCadDoc.sbr"
	-@erase "$(INTDIR)\HoverCadView.obj"
	-@erase "$(INTDIR)\HoverCadView.sbr"
	-@erase "$(INTDIR)\HoverTypes.obj"
	-@erase "$(INTDIR)\HoverTypes.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\ScaleSelect.obj"
	-@erase "$(INTDIR)\ScaleSelect.sbr"
	-@erase "$(INTDIR)\SelectAttrib.obj"
	-@erase "$(INTDIR)\SelectAttrib.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\ZoomBar.obj"
	-@erase "$(INTDIR)\ZoomBar.sbr"
	-@erase "$(OUTDIR)\HoverCad.bsc"
	-@erase "$(OUTDIR)\HoverCad.exe"
	-@erase "$(OUTDIR)\HoverCad.ilk"
	-@erase "$(OUTDIR)\HoverCad.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /c
# SUBTRACT CPP /YX /Yc /Yu
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/HoverCad.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/HoverCad.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\GDIResources.sbr" \
	"$(INTDIR)\HoverCad.sbr" \
	"$(INTDIR)\HoverCadDoc.sbr" \
	"$(INTDIR)\HoverCadView.sbr" \
	"$(INTDIR)\HoverTypes.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\ScaleSelect.sbr" \
	"$(INTDIR)\SelectAttrib.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\ZoomBar.sbr"

"$(OUTDIR)\HoverCad.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/HoverCad.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/HoverCad.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\GDIResources.obj" \
	"$(INTDIR)\HoverCad.obj" \
	"$(INTDIR)\HoverCad.res" \
	"$(INTDIR)\HoverCadDoc.obj" \
	"$(INTDIR)\HoverCadView.obj" \
	"$(INTDIR)\HoverTypes.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\ScaleSelect.obj" \
	"$(INTDIR)\SelectAttrib.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\ZoomBar.obj"

"$(OUTDIR)\HoverCad.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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

################################################################################
# Begin Target

# Name "HoverCad - Win32 Release"
# Name "HoverCad - Win32 Debug"

!IF  "$(CFG)" == "HoverCad - Win32 Release"

!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "HoverCad - Win32 Release"

!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HoverCad.cpp
DEP_CPP_HOVER=\
	".\ChildFrm.h"\
	".\GDIResources.h"\
	".\HoverCad.h"\
	".\HoverCadDoc.h"\
	".\HoverCadView.h"\
	".\MainFrm.h"\
	".\SelectAttrib.h"\
	".\StdAfx.h"\
	".\ZoomBar.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\HoverCad.obj" : $(SOURCE) $(DEP_CPP_HOVER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\HoverCad.obj" : $(SOURCE) $(DEP_CPP_HOVER) "$(INTDIR)"

"$(INTDIR)\HoverCad.sbr" : $(SOURCE) $(DEP_CPP_HOVER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /G5 /MD /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/HoverCad.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/"\
 /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\HoverCad.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W4 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/HoverCad.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\HoverCad.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\HoverCad.h"\
	".\MainFrm.h"\
	".\SelectAttrib.h"\
	".\StdAfx.h"\
	".\ZoomBar.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ChildFrm.cpp
DEP_CPP_CHILD=\
	".\ChildFrm.h"\
	".\HoverCad.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"

"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HoverCadDoc.cpp
DEP_CPP_HOVERC=\
	".\HoverCad.h"\
	".\HoverCadDoc.h"\
	".\HoverTypes.h"\
	".\ScaleSelect.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\HoverCadDoc.obj" : $(SOURCE) $(DEP_CPP_HOVERC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\HoverCadDoc.obj" : $(SOURCE) $(DEP_CPP_HOVERC) "$(INTDIR)"

"$(INTDIR)\HoverCadDoc.sbr" : $(SOURCE) $(DEP_CPP_HOVERC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HoverCadView.cpp
DEP_CPP_HOVERCA=\
	".\GDIResources.h"\
	".\HoverCad.h"\
	".\HoverCadDoc.h"\
	".\HoverCadView.h"\
	".\MainFrm.h"\
	".\SelectAttrib.h"\
	".\StdAfx.h"\
	".\ZoomBar.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\HoverCadView.obj" : $(SOURCE) $(DEP_CPP_HOVERCA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\HoverCadView.obj" : $(SOURCE) $(DEP_CPP_HOVERCA) "$(INTDIR)"

"$(INTDIR)\HoverCadView.sbr" : $(SOURCE) $(DEP_CPP_HOVERCA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\HoverCad.rc
DEP_RSC_HOVERCAD=\
	".\arrow.cur"\
	".\res\cur00001.cur"\
	".\res\cur00002.cur"\
	".\res\cur00003.cur"\
	".\res\cur00004.cur"\
	".\res\cur00005.cur"\
	".\res\cursor1.cur"\
	".\res\HoverCad.ico"\
	".\res\HoverCad.rc2"\
	".\res\HoverCadDoc.ico"\
	".\res\pointer_.cur"\
	".\res\select.bmp"\
	".\res\selectit.bmp"\
	".\res\status_i.bmp"\
	".\res\Toolbar.bmp"\
	".\res\tools_ba.bmp"\
	".\res\zoom.bmp"\
	

"$(INTDIR)\HoverCad.res" : $(SOURCE) $(DEP_RSC_HOVERCAD) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\HoverTypes.cpp
DEP_CPP_HOVERT=\
	".\HoverTypes.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\HoverTypes.obj" : $(SOURCE) $(DEP_CPP_HOVERT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\HoverTypes.obj" : $(SOURCE) $(DEP_CPP_HOVERT) "$(INTDIR)"

"$(INTDIR)\HoverTypes.sbr" : $(SOURCE) $(DEP_CPP_HOVERT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GDIResources.cpp
DEP_CPP_GDIRE=\
	".\GDIResources.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\GDIResources.obj" : $(SOURCE) $(DEP_CPP_GDIRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\GDIResources.obj" : $(SOURCE) $(DEP_CPP_GDIRE) "$(INTDIR)"

"$(INTDIR)\GDIResources.sbr" : $(SOURCE) $(DEP_CPP_GDIRE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SelectAttrib.cpp
DEP_CPP_SELEC=\
	".\HoverCad.h"\
	".\HoverCadDoc.h"\
	".\HoverTypes.h"\
	".\SelectAttrib.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\SelectAttrib.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\SelectAttrib.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"

"$(INTDIR)\SelectAttrib.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ZoomBar.cpp
DEP_CPP_ZOOMB=\
	".\HoverCad.h"\
	".\HoverCadDoc.h"\
	".\HoverCadView.h"\
	".\StdAfx.h"\
	".\ZoomBar.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\ZoomBar.obj" : $(SOURCE) $(DEP_CPP_ZOOMB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\ZoomBar.obj" : $(SOURCE) $(DEP_CPP_ZOOMB) "$(INTDIR)"

"$(INTDIR)\ZoomBar.sbr" : $(SOURCE) $(DEP_CPP_ZOOMB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScaleSelect.cpp
DEP_CPP_SCALE=\
	".\HoverCad.h"\
	".\HoverCadDoc.h"\
	".\ScaleSelect.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "HoverCad - Win32 Release"


"$(INTDIR)\ScaleSelect.obj" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "HoverCad - Win32 Debug"


"$(INTDIR)\ScaleSelect.obj" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"

"$(INTDIR)\ScaleSelect.sbr" : $(SOURCE) $(DEP_CPP_SCALE) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
