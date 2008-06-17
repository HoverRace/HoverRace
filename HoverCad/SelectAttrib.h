// SelectAttrib.h : header file
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
// SelectAttrib window

class CHoverCadDoc;

class SelectAttrib : public CDialogBar
{
private:
   // create a wrapper for each control
   CComboBox   mWallTextureCtrl;
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

// Construction
public:
	SelectAttrib();

// Attributes
private:
   CHoverCadDoc* mCurrentDoc;

public:

// Operations
   BOOL Create( CWnd* pParent, int pStyle );

   void AttachDocument( CHoverCadDoc* pDoc );
   void DetachDocument( CHoverCadDoc* pDoc );
   void Refresh();
   

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SelectAttrib)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SelectAttrib();

	// Generated message map functions
protected:
	//{{AFX_MSG(SelectAttrib)
	afx_msg void OnSelchangeItemType();
	afx_msg void OnSelchangeWallTexture();
	afx_msg void OnSelchangeFloorTexture();
	afx_msg void OnSelchangeCeilingTexture();
	afx_msg void OnChangeXPos();
	afx_msg void OnChangeCeilingLevel();
	afx_msg void OnChangeFloorLevel();
	afx_msg void OnChangeYPos();
	afx_msg void OnChangeZPos();
	afx_msg void OnChangeDegPos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDocAttrib dialog

class CDocAttrib : public CDialogBar
{

private:
   // Attributes
   CButton     mSelectBackgroundName;
   CEdit       mBackgroundName;
   CEdit       mTrackDescription;

   CHoverCadDoc* mCurrentDoc;

public:
// Construction
   CDocAttrib();
   ~CDocAttrib();

   BOOL Create( CWnd* pParent, int pStyle );

   void AttachDocument( CHoverCadDoc* pDoc );
   void DetachDocument( CHoverCadDoc* pDoc );
   void Refresh();

// Dialog Data
	//{{AFX_DATA(CDocAttrib)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDocAttrib)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
	//{{AFX_MSG(CDocAttrib)
	afx_msg void OnChangeTrackDesc();
	afx_msg void OnChangeBackgroundName();
	afx_msg void OnBrowseBackgroundName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
