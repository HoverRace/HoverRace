// HoverCad.cpp : Defines the class behaviors for the application.
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which 
// you have taken this file. If you can not find the license you can not use 
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions 
// and limitations under the License.
//
//
//

#include "stdafx.h"
#include "HoverCad.h"

#include "SelectAttrib.h"
#include "ZoomBar.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "HoverCadDoc.h"
#include "HoverCadView.h"
#include "GDIResources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// The one and only CHoverCadApp object

CHoverCadApp theApp;

CHoverCadApp* HC_GetApp()
{
   return &theApp;
}



/////////////////////////////////////////////////////////////////////////////
// CHoverCadApp

BEGIN_MESSAGE_MAP(CHoverCadApp, CWinApp)
	//{{AFX_MSG_MAP(CHoverCadApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(IDC_SELECT, OnSelectCmd)
	ON_UPDATE_COMMAND_UI(IDC_SELECT, OnUpdateSelect)
	ON_COMMAND(IDC_ROOM, OnRoomCmd)
	ON_UPDATE_COMMAND_UI(IDC_ROOM, OnUpdateRoom)
	ON_COMMAND(IDC_FEATURE, OnFeatureCmd)
	ON_UPDATE_COMMAND_UI(IDC_FEATURE, OnUpdateFeature)
	ON_UPDATE_COMMAND_UI(IDC_MOVE_SELECT, OnUpdateMoveSelect)
	ON_COMMAND(IDC_MOVE_SELECT, OnMoveSelectCmd)
	ON_COMMAND(IDC_HAMMER, OnHammerCmd)
	ON_UPDATE_COMMAND_UI(IDC_HAMMER, OnUpdateHammer)
	ON_COMMAND(IDC_INSERT_OBJECT, OnInsertObjectCmd)
	ON_UPDATE_COMMAND_UI(IDC_INSERT_OBJECT, OnUpdateInsertObjectCmd)
	ON_COMMAND(ID_HELP_HELP, OnHelpHelp)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHoverCadApp construction

CHoverCadApp::CHoverCadApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
  mCommandMode = IDC_ROOM;   
}

void CHoverCadApp::SetCommandMode( int pCommandMode )
{
   mCommandMode = pCommandMode;

   // Notify all view
   CHoverCadView::SetCurrentModeForAll( mCommandMode );
}

int CHoverCadApp::GetCommandMode()const
{
   return mCommandMode;
}

HCURSOR CHoverCadApp::GetCommandModeCursor()const
{
   if( mCommandMode == IDC_SELECT )
   {
      return LoadStandardCursor( IDC_CROSS );
   }
   else
   {
      int lResource = -1;

      switch( mCommandMode )
      {
         case IDC_FEATURE:
            lResource = IDC_POINTER_INSERT_FEATURE;
            break;

         case IDC_ROOM:
            lResource = IDC_POINTER_ROOM;
            break;

         case IDC_INSERT_OBJECT:
            lResource = IDC_POINTER_INSERT_OBJECT;
            break;

         case IDC_INSERT_NODE:
            lResource = IDC_POINTER_PATH;
            break;

         case IDC_MOVE_SELECT:
            lResource = IDC_POINTER_MOVE;
            break;

         case IDC_HAMMER:
            lResource = IDC_POINTER_HAMMER;
            break;

      }
      ASSERT( lResource != -1 ); 

      return LoadCursor( lResource );
   }
}

SelectAttrib* CHoverCadApp::GetSelectAttribWnd()
{
   ASSERT( m_pMainWnd != NULL );

   return &(((CMainFrame*)m_pMainWnd)->mAttribTool);
}

CStatusBar* CHoverCadApp::GetStatusBar()
{
   ASSERT( m_pMainWnd != NULL );

   return &(((CMainFrame*)m_pMainWnd)->m_wndStatusBar);
}


CZoomBar* CHoverCadApp::GetZoomBar()
{
   ASSERT( m_pMainWnd != NULL );

   return &(((CMainFrame*)m_pMainWnd)->mZoomBar );
}

CDocAttrib* CHoverCadApp::GetDocAttribBar()
{
   ASSERT( m_pMainWnd != NULL );

   return &(((CMainFrame*)m_pMainWnd)->mDocAttrib );
}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadApp initialization

BOOL CHoverCadApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_HCADTYPE,
		RUNTIME_CLASS(CHoverCadDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CHoverCadView));
	AddDocTemplate(pDocTemplate);

   // Init drawing tools
   InitGDIResources();

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CHoverCadApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadApp commands

void CHoverCadApp::OnSelectCmd() 
{
   SetCommandMode( IDC_SELECT );
}

void CHoverCadApp::OnUpdateSelect(CCmdUI* pCmdUI) 
{
   // pCmdUI->Enable();
   pCmdUI->SetCheck( mCommandMode==IDC_SELECT?1:0);
}

void CHoverCadApp::OnRoomCmd() 
{
   SetCommandMode( IDC_ROOM );
}

void CHoverCadApp::OnUpdateRoom(CCmdUI* pCmdUI) 
{
   pCmdUI->Enable();
   pCmdUI->SetCheck( mCommandMode==IDC_ROOM?1:0);	
}

void CHoverCadApp::OnFeatureCmd() 
{
   SetCommandMode( IDC_FEATURE );
}

void CHoverCadApp::OnUpdateFeature(CCmdUI* pCmdUI) 
{
   // pCmdUI->Enable();
   pCmdUI->SetCheck( mCommandMode==IDC_FEATURE?1:0);	
}

void CHoverCadApp::OnUpdateMoveSelect(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck( mCommandMode==IDC_MOVE_SELECT?1:0);		
}

void CHoverCadApp::OnMoveSelectCmd() 
{
   SetCommandMode( IDC_MOVE_SELECT );	
}

void CHoverCadApp::OnHammerCmd() 
{
   SetCommandMode( IDC_HAMMER );		
}

void CHoverCadApp::OnUpdateHammer(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck( mCommandMode==IDC_HAMMER?1:0);			
}

void CHoverCadApp::OnInsertObjectCmd() 
{
   SetCommandMode( IDC_INSERT_OBJECT );			
}

void CHoverCadApp::OnUpdateInsertObjectCmd(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck( mCommandMode==IDC_INSERT_OBJECT?1:0);	
}


void CHoverCadApp::OnHelpHelp() 
{
  unsigned int lReturnCode = (int)ShellExecute( *m_pMainWnd, 
                                                NULL, 
                                                "..\\Help\\CADHelp.doc",// "..\\Help\\Index.html",
                                                NULL, 
                                                NULL,
                                                SW_SHOWNORMAL );

   if( lReturnCode <= 32 )
   {
      CString lMessage;
      CString lTitle;

      lMessage.LoadString( IDS_WORD_REQ );
      lTitle.LoadString( IDS_HELP_ERROR );

      MessageBox( *m_pMainWnd,
                  lMessage,
                  lTitle,
                  MB_ICONERROR|MB_APPLMODAL|MB_OK );
      

   }

	
}
