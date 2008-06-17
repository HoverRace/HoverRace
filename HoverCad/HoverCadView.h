// HoverCadView.h : interface of the CHoverCadView class
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
/////////////////////////////////////////////////////////////////////////////


class CHoverCadView : public CView
{
protected: // create from serialization only
	CHoverCadView();

	DECLARE_DYNCREATE(CHoverCadView)

// Attributes
protected:
   static CList< CHoverCadView*, CHoverCadView*> mViewList;
   int    mCurrentMode;
   BOOL   mCaptureMode;
   double mScaling;
   CRect  mDocSize;
   CRect  mViewSize;


   BOOL   mMouseDown;
   CPoint mSelectionStart;
   CPoint mSelectionEnd;

   HCNodeList mCurrentNodeList;

public:
	CHoverCadDoc* GetDocument();
   void          SetCurrentMode( int pMode );
   static void   SetCurrentModeForAll( int pMode );


// Operations
public:

   // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHoverCadView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHoverCadView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Helper func
public:
void   SetDisplayRatio( double pRatio );
double GetDisplayRatio( );

protected:
CPoint ClientToDoc( CPoint pPoint );
CPoint DocToClient( CPoint pPoint );

HCNode* GetNodeAtPosition( CPoint pPoint, int pRay, HCNode* pException = NULL );
HCSelectable* GetNearestSelectable( CPoint pPoint );
void    SelectNearest( CPoint pPoint );
void    SelectRegion( CPoint pStart, CPoint pEnd );


// Generated message map functions
protected:
	//{{AFX_MSG(CHoverCadView)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnUpdateCut(CCmdUI* pCmdUI);
	afx_msg void OnCutCmd();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnUpdateInsertNode(CCmdUI* pCmdUI);
	afx_msg void OnInsertNodeCmd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in HoverCadView.cpp
inline CHoverCadDoc* CHoverCadView::GetDocument()
   { return (CHoverCadDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
