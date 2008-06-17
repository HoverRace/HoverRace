// ZoomBar.h : header file
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

/////////////////////////////////////////////////////////////////////////////
// CZoomBar dialog
class CHoverCadView;

class CZoomBar : public CDialogBar
{
private:
   // Widgets
   CSliderCtrl mSlider;

   // Attributes
   CHoverCadView* mCurrentView;

// Construction
public:
   CZoomBar();
	BOOL Create(CWnd* pParent, UINT pStyle );

   void AttachView( CHoverCadView* pView );
   void DetachView( CHoverCadView* pView );

// Dialog Data
	//{{AFX_DATA(CZoomBar)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomBar)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CZoomBar)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
