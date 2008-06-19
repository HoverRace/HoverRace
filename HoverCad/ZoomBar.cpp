// ZoomBar.cpp : implementation file
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
#include <math.h>
#include "HoverCad.h"
#include "ZoomBar.h"
#include "HoverCadDoc.h"
#include "HoverCadView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomBar dialog

CZoomBar::CZoomBar()
{
	mCurrentView = NULL;
}

BOOL CZoomBar::Create(CWnd * pParent, UINT pStyle)
{
	BOOL lReturnValue = CDialogBar::Create(pParent, IDD_ZOOM_BAR, pStyle, IDD_ZOOM_BAR);

	if(lReturnValue) {
		mSlider.SubclassWindow(*GetDlgItem(IDC_ZOOM_SLIDER));
		mSlider.EnableWindow(FALSE);
	}

	return lReturnValue;
}

void CZoomBar::AttachView(CHoverCadView * pView)
{
	if(pView != mCurrentView) {
		mCurrentView = pView;

		// Update current state
		if(pView != NULL) {
			mSlider.EnableWindow(TRUE);

			int lSliderValue = 33.0 - 33.0 * log10(pView->GetDisplayRatio());

			mSlider.SetPos(lSliderValue);
		}
		else {
			mSlider.EnableWindow(FALSE);
		}
	}
}

void CZoomBar::DetachView(CHoverCadView * pView)
{
	if(pView == mCurrentView) {
		mSlider.EnableWindow(FALSE);
		mCurrentView = NULL;
	}
}

/*
void CZoomBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZoomBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
*/

BEGIN_MESSAGE_MAP(CZoomBar, CDialogBar)
//{{AFX_MSG_MAP(CZoomBar)
ON_WM_VSCROLL()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CZoomBar message handlers
void CZoomBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	TRACE("Zoom move %d %d\n", (int) nSBCode, (int) nPos);

	// if( (nSBCode==)||(nSBCode==))
	{
		double lScaling = pow((double) 10, (double) (33 - mSlider.GetPos()) / 33.0);

		if(mCurrentView != NULL) {
			mCurrentView->SetDisplayRatio(lScaling);
			mCurrentView->OnUpdate(NULL, NULL, NULL);
		}
	}
	CDialogBar::OnVScroll(nSBCode, nPos, pScrollBar);
}
