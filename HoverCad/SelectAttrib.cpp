// SelectAttrib.cpp : implementation file
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
#include "HoverCad.h"
#include "HoverCadDoc.h"
#include "HoverTypes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Local helper

CString NumToText( int pVal )
{
   CString lReturnValue;

   if( (pVal != HC_NO_SELECTION)&&(pVal != HC_DIFFERENT_SELECTION) )
   {
      lReturnValue.Format( "%.3f", pVal/1000.0 );
   }

   return lReturnValue;
}

int TextToNum( const char* pVal, int pMin, int pMax )
{
   int    lReturnValue = HC_READ_ONLY;
   double lFloatValue;

   if( sscanf( pVal, "%lf", &lFloatValue )==1 )
   {
      lReturnValue = lFloatValue*1000.0;

      if( lReturnValue < pMin )
      {
         lReturnValue = pMin;
      }

      if( lReturnValue > pMax )
      {
         lReturnValue = pMax;
      }
   }
   return lReturnValue;
}



/////////////////////////////////////////////////////////////////////////////
// SelectAttrib

SelectAttrib::SelectAttrib()
{
   mCurrentDoc = NULL;
}

SelectAttrib::~SelectAttrib()
{
   
}

BOOL SelectAttrib::Create( CWnd* pParent, int pStyle )
{
   BOOL lReturnValue = CDialogBar::Create( pParent, IDD_ATTRIB_TOOL, pStyle, IDD_ATTRIB_TOOL );

   if( lReturnValue )
   {
      ASSERT( GetDlgItem( IDC_WALL_TEXTURE ) != NULL );

      mWallTextureCtrl    .SubclassWindow( *GetDlgItem( IDC_WALL_TEXTURE ));
      mCeilingTextureCtrl .SubclassWindow( *GetDlgItem( IDC_CEILING_TEXTURE ));
      mFloorTextureCtrl   .SubclassWindow( *GetDlgItem( IDC_FLOOR_TEXTURE ));
      mCeilingLevelEdit   .SubclassWindow( *GetDlgItem( IDC_CEILING_LEVEL ));
      mCeilingLevelSlider .SubclassWindow( *GetDlgItem( IDC_CEILING_LEVEL_SLID ));
      mFloorLevelEdit     .SubclassWindow( *GetDlgItem( IDC_FLOOR_LEVEL ));
      mFloorLevelSlider   .SubclassWindow( *GetDlgItem( IDC_FLOOR_LEVEL_SLID ));
      mXPosEdit           .SubclassWindow( *GetDlgItem( IDC_X_POS ));
      mYPosEdit           .SubclassWindow( *GetDlgItem( IDC_Y_POS ));
      mZPosEdit           .SubclassWindow( *GetDlgItem( IDC_Z_POS ));
      mAngleEdit          .SubclassWindow( *GetDlgItem( IDC_DEG_POS ));
      mItemTypeCtrl       .SubclassWindow( *GetDlgItem( IDC_ITEM_TYPE ));   

      // Init these controls
      int lCounter;
      char lBuffer[ 60 ];

      for( lCounter = 0; lCounter < WallTextureListSize; lCounter++ )
      {
         LoadString( NULL, WallTextureList[ lCounter ].mStringId, lBuffer, sizeof( lBuffer ) );
         mWallTextureCtrl.InsertString( lCounter, lBuffer );
      }

      for( lCounter = 0; lCounter < CeilingTextureListSize; lCounter++ )
      {
         LoadString( NULL, CeilingTextureList[ lCounter ].mStringId, lBuffer, sizeof( lBuffer ) );
         mCeilingTextureCtrl.InsertString( lCounter, lBuffer );
      }

      for( lCounter = 0; lCounter < FloorTextureListSize; lCounter++ )
      {
         LoadString( NULL, FloorTextureList[ lCounter ].mStringId, lBuffer, sizeof( lBuffer ) );
         mFloorTextureCtrl.InsertString( lCounter, lBuffer );
      }

      for( lCounter = 0; lCounter < ObjectListSize; lCounter++ )
      {
         LoadString( NULL, ObjectList[ lCounter ].mStringId, lBuffer, sizeof( lBuffer ) );
         mItemTypeCtrl.InsertString( lCounter, lBuffer );
      }
   }

   Refresh();


   return lReturnValue;
}

void SelectAttrib::AttachDocument( CHoverCadDoc* pDoc )
{
   if( pDoc != mCurrentDoc )
   {
      mCurrentDoc = pDoc;
      Refresh();
   }
}

void SelectAttrib::DetachDocument( CHoverCadDoc* pDoc )
{
   if( pDoc == mCurrentDoc )
   {
      mCurrentDoc = NULL;
      Refresh();
   }
}

void SelectAttrib::Refresh()
{
   int lWallTexture    = HC_NO_SELECTION;
   int lCeilingTexture = HC_NO_SELECTION;
   int lFloorTexture   = HC_NO_SELECTION;
   int lItemType       = HC_NO_SELECTION;
   int lFloorLevel     = HC_NO_SELECTION;
   int lCeilingLevel   = HC_NO_SELECTION;
   int lXPos           = HC_NO_SELECTION;
   int lYPos           = HC_NO_SELECTION;
   int lZPos           = HC_NO_SELECTION;
   int lOrientation    = HC_NO_SELECTION;

   
   // Retrieve each interesting field from the document
   if( mCurrentDoc != NULL )
   {
      lWallTexture    = mCurrentDoc->SetWallTexture   ( HC_READ_ONLY );
      lCeilingTexture = mCurrentDoc->SetCeilingTexture( HC_READ_ONLY );
      lFloorTexture   = mCurrentDoc->SetFloorTexture  ( HC_READ_ONLY );
      lItemType       = mCurrentDoc->SetItemType      ( HC_READ_ONLY );
      lFloorLevel     = mCurrentDoc->SetFloorLevel    ( HC_READ_ONLY );
      lCeilingLevel   = mCurrentDoc->SetCeilingLevel  ( HC_READ_ONLY );
      lXPos           = mCurrentDoc->SetXPos          ( HC_READ_ONLY );
      lYPos           = mCurrentDoc->SetYPos          ( HC_READ_ONLY );
      lZPos           = mCurrentDoc->SetZPos          ( HC_READ_ONLY );
      lOrientation    = mCurrentDoc->SetOrientation   ( HC_READ_ONLY );

   }


   // Refresh the state of each control
   //   

   // CComboBoxs
   mWallTextureCtrl.EnableWindow( lWallTexture != HC_NO_SELECTION );
   mWallTextureCtrl.SetCurSel(    lWallTexture <0?-1:lWallTexture );

   mCeilingTextureCtrl.EnableWindow( lCeilingTexture != HC_NO_SELECTION );
   mCeilingTextureCtrl.SetCurSel(    lCeilingTexture <0?-1:lCeilingTexture );

   mFloorTextureCtrl.EnableWindow( lFloorTexture != HC_NO_SELECTION );
   mFloorTextureCtrl.SetCurSel(    lFloorTexture <0?-1:lFloorTexture );

   mItemTypeCtrl.EnableWindow( lItemType != HC_NO_SELECTION );
   mItemTypeCtrl.SetCurSel(    lItemType <0?-1:lItemType );


   // Edit boxes
   mFloorLevelEdit.EnableWindow( lFloorLevel != HC_NO_SELECTION );
   mFloorLevelEdit.SetWindowText( NumToText( lFloorLevel ) );

   mCeilingLevelEdit.EnableWindow( lCeilingLevel != HC_NO_SELECTION );
   mCeilingLevelEdit.SetWindowText( NumToText( lCeilingLevel ) );

   mXPosEdit.EnableWindow( lXPos != HC_NO_SELECTION );
   mXPosEdit.SetWindowText( NumToText( lXPos ) );

   mYPosEdit.EnableWindow( lYPos != HC_NO_SELECTION );
   mYPosEdit.SetWindowText( NumToText( lYPos ) );

   mZPosEdit.EnableWindow( lZPos != HC_NO_SELECTION );
   mZPosEdit.SetWindowText( NumToText( lZPos ) );

   mAngleEdit.EnableWindow( lOrientation != HC_NO_SELECTION );
   mAngleEdit.SetWindowText( NumToText( lOrientation ) );

   /*
   CComboBox   mCeilingTextureCtrl;
   CComboBox   mFloorTextureCtrl;

   CEdit       mCeilingLevelEdit;
   CSliderCtrl mCeilingLevelSlider;

   CEdit       mFloorLevelEdit;
   CSliderCtrl mFloorLevelSlider;

   CEdit       mXPosEdit;
   CEdit       mYPosEdit;
   CEdit       mZPosEdit;
   CEdit       mAngleEdit;

   CComboBox   mItemTypeCtrl;

   */








   
   /*
   lWallTextureCtrl    .Detach();
   lCeilingTextureCtrl .Detach();
   lFloorTextureCtrl   .Detach();
   lCeilingLevelEdit   .Detach();
   lCeilingLevelSlider .Detach();
   lFloorLevelEdit     .Detach();
   lFloorLevelSlider   .Detach();
   lXPosEdit           .Detach();
   lYPosEdit           .Detach();
   lZPosEdit           .Detach();
   lAngleEdit          .Detach();
   lItemTypeTextureCtrl.Detach();
   */

   /*
   IDC_WALL_TEXTURE
   IDC_CEILING_TEXTURE
   */
}



BEGIN_MESSAGE_MAP(SelectAttrib, CDialogBar)
	//{{AFX_MSG_MAP(SelectAttrib)
	ON_CBN_SELCHANGE(IDC_ITEM_TYPE, OnSelchangeItemType)
	ON_CBN_SELCHANGE(IDC_WALL_TEXTURE, OnSelchangeWallTexture)
	ON_CBN_SELCHANGE(IDC_FLOOR_TEXTURE, OnSelchangeFloorTexture)
	ON_CBN_SELCHANGE(IDC_CEILING_TEXTURE, OnSelchangeCeilingTexture)
	ON_EN_CHANGE(IDC_X_POS, OnChangeXPos)
	ON_EN_CHANGE(IDC_CEILING_LEVEL, OnChangeCeilingLevel)
	ON_EN_CHANGE(IDC_FLOOR_LEVEL, OnChangeFloorLevel)
	ON_EN_CHANGE(IDC_Y_POS, OnChangeYPos)
	ON_EN_CHANGE(IDC_Z_POS, OnChangeZPos)
	ON_EN_CHANGE(IDC_DEG_POS, OnChangeDegPos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// SelectAttrib message handlers

BOOL SelectAttrib::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	// TRACE( "Notify %d %d %d\n", (int)message, (int)wParam, (int)lParam );

	return CDialogBar::OnChildNotify(message, wParam, lParam, pLResult);
}

BOOL SelectAttrib::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	// TRACE( "Child Notify %d %d\n", (int)wParam, (int)lParam );
	
	return CDialogBar::OnNotify(wParam, lParam, pResult);
}

BOOL SelectAttrib::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TRACE( "Command %d %d\n", (int)wParam, (int)lParam );	
	
	return CDialogBar::OnCommand(wParam, lParam);
}


void SelectAttrib::OnSelchangeWallTexture() 
{
   int lNewValue = mWallTextureCtrl.GetCurSel();

   if( (lNewValue >= 0)&&(mCurrentDoc != NULL) )
   {
      mCurrentDoc->SetWallTexture( lNewValue );
   }

}

void SelectAttrib::OnSelchangeFloorTexture() 
{
   int lNewValue = mFloorTextureCtrl.GetCurSel();

   if( (lNewValue >= 0)&&(mCurrentDoc != NULL) )
   {
      mCurrentDoc->SetFloorTexture( lNewValue );
   }
}

void SelectAttrib::OnSelchangeCeilingTexture() 
{
   int lNewValue = mCeilingTextureCtrl.GetCurSel();

   if( (lNewValue >= 0)&&(mCurrentDoc != NULL) )
   {
      mCurrentDoc->SetCeilingTexture( lNewValue );
   }
}

void SelectAttrib::OnSelchangeItemType() 
{
   int lNewValue = mItemTypeCtrl.GetCurSel();

   if( (lNewValue >= 0)&&(mCurrentDoc != NULL) )
   {
      mCurrentDoc->SetItemType( lNewValue );
   }
}

void SelectAttrib::OnChangeFloorLevel() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;
      mFloorLevelEdit.GetWindowText( lText );

      mCurrentDoc->SetFloorLevel( TextToNum( lText, MIN_Z, MAX_Z ) );
   }	
}

void SelectAttrib::OnChangeCeilingLevel() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;
      mCeilingLevelEdit.GetWindowText( lText );

      mCurrentDoc->SetCeilingLevel( TextToNum( lText, MIN_Z, MAX_Z ) );
   }	
}

void SelectAttrib::OnChangeXPos() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;
      mXPosEdit.GetWindowText( lText );

      mCurrentDoc->SetXPos( TextToNum( lText, MIN_XY, MAX_XY ) );
   }	
}

void SelectAttrib::OnChangeYPos() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;
      mYPosEdit.GetWindowText( lText );

      mCurrentDoc->SetYPos( TextToNum( lText, MIN_XY, MAX_XY ) );
   }	
	
}

void SelectAttrib::OnChangeZPos() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;
      mZPosEdit.GetWindowText( lText );

      mCurrentDoc->SetZPos( TextToNum( lText, MIN_Z, MAX_Z ) );
   }		
}

void SelectAttrib::OnChangeDegPos() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;
      mAngleEdit.GetWindowText( lText );

      mCurrentDoc->SetOrientation( TextToNum( lText, 0, 360*1000 ) );
   }				
}
/////////////////////////////////////////////////////////////////////////////
// CDocAttrib dialog


CDocAttrib::CDocAttrib()
{
   mCurrentDoc = NULL;
}

CDocAttrib::~CDocAttrib()
{
   
}

BOOL CDocAttrib::Create( CWnd* pParent, int pStyle )
{
   BOOL lReturnValue = CDialogBar::Create( pParent, IDD_DOC_ATTR, pStyle, IDD_DOC_ATTR );

   if( lReturnValue )
   {
      mSelectBackgroundName.SubclassWindow( *GetDlgItem( IDC_BROWSE ));
      mBackgroundName  .SubclassWindow( *GetDlgItem( IDC_BACKGROUND_NAME ));
      mTrackDescription.SubclassWindow( *GetDlgItem( IDC_TRACK_DESC ));
   }
   mSelectBackgroundName.ShowWindow( SW_SHOW );

   Refresh();
   return lReturnValue;
}

void CDocAttrib::AttachDocument( CHoverCadDoc* pDoc )
{
   if( pDoc != mCurrentDoc )
   {
      mCurrentDoc = pDoc;
      Refresh();
   }
}

void CDocAttrib::DetachDocument( CHoverCadDoc* pDoc )
{
   if( pDoc == mCurrentDoc )
   {
      mCurrentDoc = NULL;
      Refresh();
   }
}

void CDocAttrib::Refresh()
{
   TRACE( "Refresh\n" );

   
   if( mCurrentDoc == NULL )
   {
      mSelectBackgroundName.EnableWindow( FALSE );
      mBackgroundName.EnableWindow( FALSE );
      mBackgroundName.SetWindowText( "" );
      mTrackDescription.EnableWindow( FALSE );
      mTrackDescription.SetWindowText( "" );
   }
   else
   {
      mSelectBackgroundName.EnableWindow( TRUE );
      mBackgroundName.EnableWindow( TRUE );
      mBackgroundName.SetWindowText( mCurrentDoc->GetBackImageName() );
      mTrackDescription.EnableWindow( TRUE );
      mTrackDescription.SetWindowText( mCurrentDoc->GetDescription() );
   }
}

BEGIN_MESSAGE_MAP(CDocAttrib, CDialogBar)
	//{{AFX_MSG_MAP(CDocAttrib)
	ON_EN_CHANGE(IDC_TRACK_DESC, OnChangeTrackDesc)
	ON_EN_CHANGE(IDC_BACKGROUND_NAME, OnChangeBackgroundName)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowseBackgroundName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDocAttrib message handlers

void CDocAttrib::OnChangeTrackDesc() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;

      mTrackDescription.GetWindowText( lText );
      mCurrentDoc->SetDescription( lText );
   }
}

void CDocAttrib::OnChangeBackgroundName() 
{
   if( mCurrentDoc != NULL )
   {
      CString lText;

      mBackgroundName.GetWindowText( lText );
      mCurrentDoc->SetBackImageName( lText );
   }	
}

void CDocAttrib::OnBrowseBackgroundName() 
{
   if( mCurrentDoc != NULL )	
   {
      CString szFilter;
      szFilter.LoadString( IDS_BACKGROUND_FILTER );
      // static char BASED_CODE szFilter[] = "PCX 128colors (*.pcx)|*.pcx||";
  
      CFileDialog lDialog( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this );

      if( lDialog.DoModal()== IDOK )
      {
         CString lText = lDialog.GetPathName();

         mBackgroundName.SetWindowText( lText );
         mCurrentDoc->SetBackImageName( lText );
      }
   }
}


