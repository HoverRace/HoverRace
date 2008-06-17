// ScaleSelect.cpp : implementation file
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
#include "HoverCadDoc.h"
#include "ScaleSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScaleSelect dialog


CScaleSelect::CScaleSelect( CHoverCadDoc* pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(CScaleSelect::IDD, pParent)
{
   mDoc          = pDoc;
   mCurrentScale = 100.0;
	//{{AFX_DATA_INIT(CScaleSelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CScaleSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScaleSelect)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScaleSelect, CDialog)
	//{{AFX_MSG_MAP(CScaleSelect)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleSelect message handlers

void CScaleSelect::OnPreview() 
{
   double lScale = GetDlgItemInt( IDC_SCALE );	

   if( (lScale > 10) && (lScale < 1000) )
   {
      if( lScale != mCurrentScale )
      {        
         double lScaleToApply = (lScale/mCurrentScale);

         mCurrentScale = lScale;

         mDoc->ScaleSelection( lScaleToApply );
      }
   }	
}

void CScaleSelect::OnCancel() 
{
	// TODO: Add extra cleanup here
   if( mCurrentScale != 100 )
   {
      double lScaleToApply = (100.0/mCurrentScale);

      mDoc->ScaleSelection( lScaleToApply );
   }	
	
	CDialog::OnCancel();

}

void CScaleSelect::OnOK() 
{
   double lScale = GetDlgItemInt( IDC_SCALE );	

   if( (lScale > 10) && (lScale < 1000) )
   {
      if( lScale != mCurrentScale )
      {
         double lScaleToApply = (lScale/mCurrentScale);

         mDoc->ScaleSelection( lScaleToApply );
      }
    	CDialog::OnOK();
   }		
}
