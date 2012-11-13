; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CScaleSelect
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "HoverCad.h"
LastPage=0

ClassCount=10
Class1=CHoverCadApp
Class2=CHoverCadDoc
Class3=CHoverCadView
Class4=CMainFrame

ResourceCount=11
Resource1=IDR_MAINFRAME (Neutral (Sys. Default))
Resource2=IDD_SCALE_SELECTION (Neutral (Default))
Resource3=IDR_STATUS_IDS (Neutral (Default))
Class5=CAboutDlg
Class6=CChildFrame
Resource4=IDR_TOOLS_BAR (Neutral (Default))
Resource5=IDD_ZOOM_BAR (Neutral (Default))
Class7=SelectAttrib
Resource6=IDD_DOC_ATTR (Neutral (Default))
Resource7=IDR_HCADTYPE (Neutral (Default))
Class8=CZoomBar
Resource8=IDD_DIALOGBAR (Neutral (Default))
Class9=CDocAttrib
Resource9=IDR_MAINFRAME (Neutral (Default))
Resource10=IDD_ABOUTBOX (Neutral (Default))
Class10=CScaleSelect
Resource11=IDD_ATTRIB_TOOL (Neutral (Default))

[CLS:CHoverCadApp]
Type=0
HeaderFile=HoverCad.h
ImplementationFile=HoverCad.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=CHoverCadApp

[CLS:CHoverCadDoc]
Type=0
HeaderFile=HoverCadDoc.h
ImplementationFile=HoverCadDoc.cpp
Filter=N
LastObject=CHoverCadDoc
BaseClass=CDocument
VirtualFilter=DC

[CLS:CHoverCadView]
Type=0
HeaderFile=HoverCadView.h
ImplementationFile=HoverCadView.cpp
Filter=C
LastObject=IDC_ZOOM_SLIDER
BaseClass=CView 
VirtualFilter=VWC

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=IDC_SELECT
BaseClass=CMDIFrameWnd
VirtualFilter=fWC


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M

[CLS:CAboutDlg]
Type=0
HeaderFile=HoverCad.cpp
ImplementationFile=HoverCad.cpp
Filter=D

[CLS:SelectAttrib]
Type=0
HeaderFile=SelectAttrib.h
ImplementationFile=SelectAttrib.cpp
BaseClass=CDialogBar
Filter=W
LastObject=ID_APP_ABOUT
VirtualFilter=dWC

[CLS:CZoomBar]
Type=0
HeaderFile=ZoomBar.h
ImplementationFile=ZoomBar.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CZoomBar

[CLS:CDocAttrib]
Type=0
HeaderFile=selectattrib.h
ImplementationFile=selectattrib.cpp
BaseClass=CDialogBar
Filter=D
VirtualFilter=dWC
LastObject=CDocAttrib

[TB:IDR_MAINFRAME (Neutral (Default))]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_HELP_HELP
CommandCount=8

[TB:IDR_TOOLS_BAR (Neutral (Default))]
Type=1
Command1=IDC_SELECT
Command2=IDC_HAMMER
Command3=IDC_MOVE_SELECT
Command4=IDC_ROOM
Command5=IDC_FEATURE
Command6=IDC_INSERT_OBJECT
Command7=IDC_PATH
Command8=IDC_CUT
Command9=IDC_INSERT_NODE
CommandCount=9

[TB:IDR_STATUS_IDS (Neutral (Default))]
Type=1
Command1=ID_MOUSE_POS
CommandCount=1

[MNU:IDR_MAINFRAME (Neutral (Default))]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_TOOLS
Command8=ID_VIEW_SLECTIONATTRIBUTES
Command9=ID_VIEW_TRACKATTRIBUTES
Command10=ID_VIEW_STATUS_BAR
Command11=ID_HELP_HELP
Command12=ID_APP_ABOUT
CommandCount=12

[MNU:IDR_HCADTYPE (Neutral (Default))]
Type=1
Class=CHoverCadView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_COMPILE
Command7=ID_FILE_PRINT
Command8=ID_FILE_PRINT_PREVIEW
Command9=ID_FILE_PRINT_SETUP
Command10=ID_FILE_MRU_FILE1
Command11=ID_APP_EXIT
Command12=ID_EDIT_UNDO
Command13=ID_EDIT_CUT
Command14=ID_EDIT_COPY
Command15=ID_EDIT_PASTE
Command16=ID_EDIT_SCALESELECTION
Command17=ID_VIEW_TOOLBAR
Command18=ID_VIEW_TOOLS
Command19=ID_VIEW_SLECTIONATTRIBUTES
Command20=ID_VIEW_TRACKATTRIBUTES
Command21=ID_VIEW_STATUS_BAR
Command22=ID_WINDOW_NEW
Command23=ID_WINDOW_CASCADE
Command24=ID_WINDOW_TILE_HORZ
Command25=ID_WINDOW_ARRANGE
Command26=ID_WINDOW_SPLIT
Command27=ID_HELP_HELP
Command28=ID_APP_ABOUT
CommandCount=28

[ACL:IDR_MAINFRAME (Neutral (Sys. Default))]
Type=1
Class=CMainFrame
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_PRINT
Command5=ID_FILE_SAVE
Command6=ID_EDIT_PASTE
Command7=ID_EDIT_UNDO
Command8=ID_HELP_HELP
Command9=ID_NEXT_PANE
Command10=ID_PREV_PANE
Command11=ID_FILE_COMPILE
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_EDIT_CUT
Command15=ID_EDIT_UNDO
CommandCount=15

[DLG:IDD_ABOUTBOX (Neutral (Default))]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_ATTRIB_TOOL (Neutral (Default))]
Type=1
Class=SelectAttrib
ControlCount=27
Control1=IDC_WALL_TEXTURE,combobox,1344340227
Control2=IDC_CEILING_TEXTURE,combobox,1344340227
Control3=IDC_FLOOR_TEXTURE,combobox,1344340227
Control4=IDC_CEILING_LEVEL,edit,1350631552
Control5=IDC_CEILING_LEVEL_SPIN,msctls_updown32,1073741856
Control6=IDC_FLOOR_LEVEL,edit,1350631552
Control7=IDC_FLOOR_LEVEL_SPIN,msctls_updown32,1073741856
Control8=IDC_X_POS,edit,1350631552
Control9=IDC_X_POS_SPIN,msctls_updown32,1073741856
Control10=IDC_Y_POS,edit,1350631552
Control11=IDC_Y_POS_SPIN,msctls_updown32,1073741856
Control12=IDC_Z_POS,edit,1350631552
Control13=IDC_Z_POS_SPIN,msctls_updown32,1073741856
Control14=IDC_DEG_POS,edit,1350631552
Control15=IDC_DEG_POS_SPIN,msctls_updown32,1073741856
Control16=IDC_ITEM_TYPE,combobox,1344340227
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,static,1342308352
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,static,1342308352
Control25=IDC_STATIC,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_STATIC,static,1342308352

[DLG:IDD_DIALOGBAR (Neutral (Default))]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342308352
Control2=IDC_TESTBTN1,button,1342242816
Control3=IDC_TESTSLD1,msctls_trackbar32,1342242840
Control4=IDC_TESTSPIN1,msctls_updown32,1342177312

[DLG:IDD_ZOOM_BAR (Neutral (Default))]
Type=1
Class=CZoomBar
ControlCount=2
Control1=IDC_ZOOM_SLIDER,msctls_trackbar32,1342242842
Control2=IDC_STATIC,static,1342177294

[DLG:IDD_DOC_ATTR (Neutral (Default))]
Type=1
Class=CDocAttrib
ControlCount=5
Control1=IDC_BACKGROUND_NAME,edit,1350631552
Control2=IDC_BROWSE,button,1342390272
Control3=IDC_TRACK_DESC,edit,1350635588
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352

[DLG:IDD_SCALE_SELECTION (Neutral (Default))]
Type=1
ControlCount=5
Control1=IDC_SCALE,edit,1350639744
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_PREVIEW,button,1342242816
Control5=IDC_STATIC,static,1342308352

[CLS:CScaleSelect]
Type=0
HeaderFile=ScaleSelect.h
ImplementationFile=ScaleSelect.cpp
BaseClass=CDialog
Filter=D
LastObject=CScaleSelect
VirtualFilter=dWC

