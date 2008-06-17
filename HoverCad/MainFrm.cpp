// MainFrm.cpp : implementation of the CMainFrame class
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
#include "SelectAttrib.h"
#include "ZoomBar.h"
#include "HoverCad.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(IDC_BROWSE, OnBackNameBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,       // status line indicator
   ID_MOUSE_POS,       // mouse position
   ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}



int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!mToolsBar.Create(this) ||
		!mToolsBar.LoadToolBar(IDR_TOOLS_BAR))
	{
		TRACE0("Failed to create tools bar\n");
		return -1;      // fail to create
	}

	if( !mZoomBar.Create( this, CBRS_LEFT ) )
	{
		TRACE0("Failed to create zoom toolbar\n");
		return -1;      // fail to create
	}

	if( !mAttribTool.Create( this, CBRS_RIGHT ) )
	{
		TRACE0("Failed to create attrib toolbar\n");
		return -1;      // fail to create
	}

	if( !mDocAttrib.Create( this, CBRS_RIGHT ) )
	{
		TRACE0("Failed to create attrib doc\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	mToolsBar.SetBarStyle(mToolsBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	mToolsBar.EnableDocking(CBRS_ALIGN_ANY);
	mZoomBar.EnableDocking(CBRS_FLOAT_MULTI|CBRS_ALIGN_RIGHT|CBRS_ALIGN_LEFT);
	mAttribTool.EnableDocking(CBRS_FLOAT_MULTI|CBRS_ALIGN_RIGHT|CBRS_ALIGN_LEFT);
	mDocAttrib.EnableDocking(CBRS_FLOAT_MULTI|CBRS_ALIGN_RIGHT|CBRS_ALIGN_LEFT);
   EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar, AFX_IDW_DOCKBAR_TOP );
	DockControlBar(&mToolsBar, AFX_IDW_DOCKBAR_LEFT );
	DockControlBar(&mZoomBar,  AFX_IDW_DOCKBAR_LEFT );
	DockControlBar(&mAttribTool, AFX_IDW_DOCKBAR_RIGHT );
	DockControlBar(&mDocAttrib, AFX_IDW_DOCKBAR_RIGHT );

   // Move ZoomBar under ToolsBar
   CRect lRectTools;
   CRect lRectZoom;
   mToolsBar.GetWindowRect( lRectTools );
   mZoomBar.GetWindowRect( lRectZoom );

   lRectTools.NormalizeRect();
   lRectZoom.NormalizeRect();

   lRectTools.right += 12;
   lRectZoom.right  += 12;
   lRectTools.bottom+= 200;
   lRectZoom.top    += 200;
   lRectZoom.bottom += 250;

   // lRectZoom.OffsetRect( 0, 220/*lRectTools.Height()*/ );
	DockControlBar(&mToolsBar,  AFX_IDW_DOCKBAR_LEFT, lRectTools );
	DockControlBar(&mZoomBar,  AFX_IDW_DOCKBAR_LEFT, lRectZoom );

   // Move DocAtttrib under AttribTool
   CRect lRectAttribTool;
   CRect lRectDocAttrib;

   mAttribTool.GetWindowRect( lRectAttribTool );
   mDocAttrib.GetWindowRect( lRectDocAttrib );

   lRectAttribTool.NormalizeRect();
   lRectDocAttrib.NormalizeRect();

   lRectAttribTool.right += 12;
   lRectDocAttrib.right  += 12;
   lRectAttribTool.bottom+= 200;
   lRectDocAttrib.top    += 200;
   lRectDocAttrib.bottom += 250;

   // lRectZoom.OffsetRect( 0, 220/*lRectTools.Height()*/ );
	DockControlBar(&mAttribTool,  AFX_IDW_DOCKBAR_RIGHT, lRectAttribTool );
	DockControlBar(&mDocAttrib,  AFX_IDW_DOCKBAR_RIGHT, lRectDocAttrib );


	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnBackNameBrowse() 
{
	mDocAttrib.OnBrowseBackgroundName();
	
}
