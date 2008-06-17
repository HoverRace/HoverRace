// HoverCad.h : main header file for the HOVERCAD application
//
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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CHoverCadApp:
// See HoverCad.cpp for the implementation of this class
//

class SelectAttrib;
class CZoomBar;
class CDocAttrib;

class CHoverCadApp : public CWinApp
{
protected:
   int mCommandMode;

   void SetCommandMode( int mCommandMode );
public:

   int           GetCommandMode()const;
   HCURSOR       GetCommandModeCursor()const;
   SelectAttrib* GetSelectAttribWnd();
   CStatusBar*   GetStatusBar();
   CZoomBar*     GetZoomBar();
   CDocAttrib*   GetDocAttribBar();
public:
	CHoverCadApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHoverCadApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHoverCadApp)
	afx_msg void OnAppAbout();
	afx_msg void OnSelectCmd();
	afx_msg void OnUpdateSelect(CCmdUI* pCmdUI);
	afx_msg void OnRoomCmd();
	afx_msg void OnUpdateRoom(CCmdUI* pCmdUI);
	afx_msg void OnFeatureCmd();
	afx_msg void OnUpdateFeature(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMoveSelect(CCmdUI* pCmdUI);
	afx_msg void OnMoveSelectCmd();
	afx_msg void OnHammerCmd();
	afx_msg void OnUpdateHammer(CCmdUI* pCmdUI);
	afx_msg void OnInsertObjectCmd();
	afx_msg void OnUpdateInsertObjectCmd(CCmdUI* pCmdUI);
	afx_msg void OnHelpHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CHoverCadApp* HC_GetApp();

/////////////////////////////////////////////////////////////////////////////
