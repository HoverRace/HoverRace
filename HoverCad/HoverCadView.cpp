// HoverCadView.cpp : implementation of the CHoverCadView class
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

#include "SelectAttrib.h"
#include "ZoomBar.h"
#include "HoverCad.h"
#include "HoverCadDoc.h"
#include "MainFrm.h"
#include "HoverCadView.h"
#include "GDIResources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// constants
#define HC_DISP_MODE            MM_LOMETRIC
#define HC_M_PER_CM                10.0   // Constant used to determined scaling
#define HC_MARGIN                  80
#define HC_WALL_TICKNESS           10
#define HC_NODE_RAY                10
#define HC_ANCHOR_RAY              21


/////////////////////////////////////////////////////////////////////////////
// CHoverCadView

IMPLEMENT_DYNCREATE(CHoverCadView, CView )

BEGIN_MESSAGE_MAP(CHoverCadView, CView )
	//{{AFX_MSG_MAP(CHoverCadView)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_UPDATE_COMMAND_UI(IDC_CUT, OnUpdateCut)
	ON_COMMAND(IDC_CUT, OnCutCmd )
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_UPDATE_COMMAND_UI(IDC_INSERT_NODE, OnUpdateInsertNode)
	ON_COMMAND(IDC_INSERT_NODE, OnInsertNodeCmd)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHoverCadView construction/destruction

CList< CHoverCadView*, CHoverCadView*> CHoverCadView::mViewList;


CHoverCadView::CHoverCadView()
{
	// TODO: add construction code here
   SetDisplayRatio( 1.0 );
   mCurrentMode = HC_GetApp()->GetCommandMode();
   mCaptureMode = FALSE;

   //mDocSize.x = 0;
   //mOrigin.y = 0;

   mMouseDown = FALSE;

   mViewList.AddTail( this );
}

CHoverCadView::~CHoverCadView()
{
   mViewList.RemoveAt( mViewList.Find( this ) );
}

void CHoverCadView::SetCurrentMode( int pMode )
{
   // Abort all current mode functions
   mCurrentNodeList.RemoveAll();

   mCurrentMode = pMode;

   // Deselect all
   // GetDocument()->ClearSelection();
}

void CHoverCadView::SetCurrentModeForAll( int pMode )
{
   POSITION lPos = mViewList.GetHeadPosition();

   while( lPos != NULL )
   {
      mViewList.GetNext( lPos )->SetCurrentMode( pMode );
   }
}


void CHoverCadView::SetDisplayRatio( double pRatio )
{
	// TODO: add construction code here
   mScaling = pRatio*HC_M_PER_CM/1000.0;
}

double CHoverCadView::GetDisplayRatio()
{
	// TODO: add construction code here
   return mScaling*1000.0/HC_M_PER_CM;
}

BOOL CHoverCadView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}


CPoint CHoverCadView::ClientToDoc( CPoint pPoint )
{
   return CPoint( /*mDocSize.left+*/(pPoint.x/*-HC_MARGIN*/)/mScaling,
                  /*mDocSize.top +*/(pPoint.y/*+HC_MARGIN*/)/mScaling );
}

CPoint CHoverCadView::DocToClient( CPoint pPoint )
{
   return CPoint( (pPoint.x/*-mDocSize.left*/)*mScaling/*+HC_MARGIN*/,
                  (pPoint.y/*-mDocSize.top*/ )*mScaling/*-HC_MARGIN*/ );
}


HCNode* CHoverCadView::GetNodeAtPosition( CPoint pPoint, int pRay, HCNode* pException )
{
   POSITION lPos  = GetDocument()->GetNodeListHead();
   HCNode*  lBest = NULL;
   int      lBestSqrDist = pRay*pRay;

   while( lPos != NULL )
   {
      HCNode* lNode = GetDocument()->GetNextNode( lPos );

      if( lNode != pException )
      {

         CPoint lNodePos = DocToClient( CPoint(lNode->mX, lNode->mY) );

         int lXDist = lNodePos.x-pPoint.x;
         int lYDist = lNodePos.y-pPoint.y;

         if( lXDist < 0 )
         {
            lXDist = -lXDist;
         }
      
         if( lYDist < 0 )
         {
            lYDist = -lYDist;
         }

         if( (lXDist <= pRay)&&(lYDist <= pRay) )
         {
            int lSqrDist = lXDist*lXDist+lYDist*lYDist;

            if( lSqrDist <= lBestSqrDist )
            {
               lBestSqrDist = lSqrDist;
               lBest = lNode;
            }
         }
      }
   }
   return lBest;
}

HCSelectable* CHoverCadView::GetNearestSelectable( CPoint pPoint )
{
   HCNode* lBest = GetNodeAtPosition( pPoint, HC_ANCHOR_RAY );

   if( lBest != NULL )
   {
      CPoint lNodePos = DocToClient( CPoint(lBest->mX, lBest->mY) );

      int lXDist   = pPoint.x-lNodePos.x;
      int lYDist   = pPoint.y-lNodePos.y;
      int lSqrDist = lXDist*lXDist + lYDist*lYDist;

      if( lSqrDist <= (HC_NODE_RAY*HC_NODE_RAY) )
      {
         return lBest;
      }
      else
      {
         // Verify each anchor to see if the point is not inside the polygon
         POSITION lAnchorPos = lBest->mAnchorList.GetHeadPosition();

         while( lAnchorPos != NULL )
         {
            HCAnchorWall* lAnchor = lBest->mAnchorList.GetNext( lAnchorPos );
            HCAnchorWall* lNext   = lAnchor->GetNext();
            HCAnchorWall* lPrev   = lAnchor->GetPrev();

            CPoint lNextPos = DocToClient( CPoint(lNext->mNode->mX, lNext->mNode->mY) );
            CPoint lPrevPos = DocToClient( CPoint(lPrev->mNode->mX, lPrev->mNode->mY) );

            int lScalarProd =  (pPoint.x-lNodePos.x)*(lNextPos.y-lNodePos.y)
                              -(pPoint.y-lNodePos.y)*(lNextPos.x-lNodePos.x);

        
            if( lScalarProd > 0 )
            {
               lScalarProd =  (pPoint.x-lNodePos.x)*(lPrevPos.y-lNodePos.y)
                             -(pPoint.y-lNodePos.y)*(lPrevPos.x-lNodePos.x);
               if( lScalarProd < 0 )
               {
                  return lAnchor;
               }
            }
         }
      }
   }
   return NULL;
}

void CHoverCadView::SelectNearest( CPoint pPoint )
{
   // Find the closest nod to the point
   HCSelectable* lBest = GetNearestSelectable( pPoint );

   if( lBest != NULL )
   {
      lBest->mSelected = !lBest->mSelected;
      GetDocument()->UpdateAllViews( NULL );
   }
}

void CHoverCadView::SelectRegion( CPoint pStart, CPoint pEnd )
{
   // Find nodes and anchor included in the selection box
   BOOL lModified = FALSE;
   POSITION lNodePos = GetDocument()->GetNodeListHead();

   CRect lBoundingBox( pStart, pEnd );

   lBoundingBox.NormalizeRect();

   CRect lLargestBounding   = lBoundingBox;  // If the node is not this box.. notting is included
   CRect lSmallestBounding  = lBoundingBox;  // If the node is in this box.. all anchor are included
   CRect lNodeBounding      = lBoundingBox;  // If the node is in that box.. node is selected

   lLargestBounding.InflateRect( HC_ANCHOR_RAY, HC_ANCHOR_RAY );
   lSmallestBounding.DeflateRect( HC_ANCHOR_RAY, HC_ANCHOR_RAY );
   lNodeBounding.DeflateRect( HC_NODE_RAY, HC_NODE_RAY );


   while( lNodePos != NULL )
   {
      HCNode* lNode = GetDocument()->GetNextNode( lNodePos );

      CPoint lNodePoint = DocToClient( CPoint( lNode->mX, lNode->mY ) );

      if( lLargestBounding.PtInRect( lNodePoint ) )
      {
         // We can continue
         if( lSmallestBounding.PtInRect( lNodePoint ) )
         {
            // easy case.. all included
            lNode->mSelected = TRUE;

            POSITION lPos = lNode->mAnchorList.GetHeadPosition();

            while( lPos != NULL )
            {
               lNode->mAnchorList.GetNext( lPos )->mSelected = TRUE;
            }
            lModified = TRUE;
         }
         else
         {
            // Verify if the node is included
            if( lNodeBounding.PtInRect( lNodePoint ) )
            {
               lNode->mSelected = TRUE;
               lModified = TRUE;
            }

            // Verify each anchor individually
            // the tho normalized extremity must be included in bounding box

            POSITION lPos = lNode->mAnchorList.GetHeadPosition();

            while( lPos != NULL )
            {
               HCAnchorWall* lAnchor = lNode->mAnchorList.GetNext( lPos );

               CPoint lPrevPoint = DocToClient( CPoint( lAnchor->GetPrev()->mNode->mX, lAnchor->GetPrev()->mNode->mY ))-lNodePoint;
               CPoint lNextPoint = DocToClient( CPoint( lAnchor->GetNext()->mNode->mX, lAnchor->GetNext()->mNode->mY ))-lNodePoint;

               double lPrevLen = sqrt( lPrevPoint.x*lPrevPoint.x + lPrevPoint.y*lPrevPoint.y );
               double lNextLen = sqrt( lNextPoint.x*lNextPoint.x + lNextPoint.y*lNextPoint.y );

               lPrevPoint.x = lPrevPoint.x/lPrevLen;
               lPrevPoint.y = lPrevPoint.y/lPrevLen;
               lNextPoint.x = lNextPoint.x/lNextLen;
               lNextPoint.y = lNextPoint.y/lNextLen;

               lPrevPoint += lNodePoint;
               lNextPoint += lNodePoint;

               if( lBoundingBox.PtInRect( lPrevPoint ) && lBoundingBox.PtInRect( lNextPoint ) )
               {
                  lAnchor->mSelected = TRUE;
                  lModified = TRUE;
               }
            }
         }
      }
   }

   if( lModified )
   {
      GetDocument()->UpdateAllViews( NULL );
   }
}


/////////////////////////////////////////////////////////////////////////////
// CHoverCadView drawing

void CHoverCadView::OnDraw(CDC* pDC)
{
   POSITION lPos;
	CHoverCadDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here


   // Draw all the track sections with there walls and anchor
   lPos = pDoc->GetPolygonListHead();
   

   pDC->SelectObject( gNormalPen );

   while( lPos != NULL )
   {
      int lCounter;

      HCPolygon* lPolygon = pDoc->GetNextPolygon( lPos );
      BOOL lPolySelected = TRUE;
      POINT lAnchorXY[MAX_NODE_PER_POLYGON+1];
      BOOL  lSelectAnchor[MAX_NODE_PER_POLYGON];
      BOOL  lFocussedAnchor[MAX_NODE_PER_POLYGON];
      int lNbAnchor = lPolygon->mAnchorWallList.GetCount();

      ASSERT( lNbAnchor <= MAX_NODE_PER_POLYGON );

      POSITION lAnchorPos;

      lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

      for( lCounter = 0; lCounter < lNbAnchor; lCounter++ )
      {
         ASSERT( lAnchorPos != NULL );

         HCAnchorWall* lAnchor = lPolygon->mAnchorWallList.GetNext( lAnchorPos );
         
         if( !lAnchor->mSelected )
         {
            lPolySelected = FALSE;
         }

         lAnchorXY[ lCounter ]= DocToClient( CPoint( lAnchor->mNode->mX, lAnchor->mNode->mY) );
         lSelectAnchor[ lCounter ]   = lAnchor->mSelected; 
         lFocussedAnchor[ lCounter ] = lAnchor->mFocussed; 
      }

      lAnchorXY[ lNbAnchor ] = lAnchorXY[0];

      if( lPolySelected || lPolygon->mFeature )
      {
         CRgn lRegion;

         lRegion.CreatePolygonRgn( lAnchorXY, lNbAnchor, ALTERNATE );
         
         pDC->SelectObject( lPolygon->mFeature ?
                         (
                            lPolySelected ? &gSelectFeatureBrush: &gFeatureBrush
                         )
                         :
                         (
                            lPolySelected ? &gSelectRoomBrush: &gRoomBrush
                         )
                       );

         pDC->PaintRgn( &lRegion );
      }
      else
      {
         pDC->Polyline( lAnchorXY, lNbAnchor+1 );
      }

      /*
      pDC->FillRgn( &lRegion,
                    lPolygon->mFeature ?
                         (
                            lPolySelected ? &gSelectFeatureBrush: &gFeatureBrush
                         )
                         :
                         (
                            lPolySelected ? &gSelectRoomBrush: &gRoomBrush
                         )
                   );
      */

      // Draw the walls

      // Draw the anchor
      for( lCounter = 0; lCounter < lNbAnchor; lCounter++ )
      {
      
         pDC->SelectObject( lFocussedAnchor[lCounter]?&gFocusBrush:(lSelectAnchor[lCounter]? &gSelectAnchorBrush: &gAnchorBrush) );
  
         pDC->Pie( lAnchorXY[ lCounter ].x-HC_ANCHOR_RAY,
                   lAnchorXY[ lCounter ].y+HC_ANCHOR_RAY,
                   lAnchorXY[ lCounter ].x+HC_ANCHOR_RAY,
                   lAnchorXY[ lCounter ].y-HC_ANCHOR_RAY,
                   lAnchorXY[ (lCounter-1+lNbAnchor)%lNbAnchor ].x,
                   lAnchorXY[ (lCounter-1+lNbAnchor)%lNbAnchor ].y,
                   lAnchorXY[ (lCounter+1)%lNbAnchor ].x,
                   lAnchorXY[ (lCounter+1)%lNbAnchor ].y            );

      }

      

   }


   // Draw all the nodes (and items when it apply)
   lPos = pDoc->GetNodeListHead();
   
   while( lPos != NULL )
   {
      HCNode* lNode = pDoc->GetNextNode( lPos );

      CPoint lPoint = DocToClient( CPoint( lNode->mX, lNode->mY) );

      if( lNode->mItem == NULL )
      {
         pDC->SelectObject( lNode->mFocussed? &gFocusBrush:(lNode->mSelected? &gSelectNodeBrush: &gNodeBrush) );
      }
      else
      {
         pDC->SelectObject( lNode->mSelected? &gSelectItemBrush: &gItemBrush );
      }

      pDC->Ellipse(lPoint.x-HC_NODE_RAY,
                   lPoint.y+HC_NODE_RAY,
                   lPoint.x+HC_NODE_RAY,
                   lPoint.y-HC_NODE_RAY );
   }


   // Draw edited element
   if( (mCurrentMode == IDC_SELECT)&&mMouseDown )
   {
      if( !(mSelectionStart == mSelectionEnd) )
      {  
         pDC->SelectObject( gSelectionPen );

         POINT lRect[5] =
         {
            {mSelectionStart.x, mSelectionStart.y},
            {mSelectionEnd.x, mSelectionStart.y},
            {mSelectionEnd.x, mSelectionEnd.y},
            {mSelectionStart.x, mSelectionEnd.y},
            {mSelectionStart.x, mSelectionStart.y}
         };

         pDC->Polyline( lRect, 5 );
      }         
   }

   if( (mCurrentMode == IDC_ROOM)||(mCurrentMode == IDC_FEATURE) )
   {
      if( !mCurrentNodeList.IsEmpty() )
      {
         POINT lNodeXY[MAX_NODE_PER_POLYGON];

         pDC->SelectObject( gTempWallPen );

         POSITION lPos = mCurrentNodeList.GetHeadPosition();

         int lCounter = 0;

         while( lPos != NULL )
         {
            HCNode* lNode = mCurrentNodeList.GetNext( lPos );

            lNodeXY[ lCounter++ ]= DocToClient( CPoint( lNode->mX, lNode->mY) );
         }

         // Draw the line between each point
         pDC->Polyline( lNodeXY, lCounter );

      }
   }

}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadView printing

BOOL CHoverCadView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CHoverCadView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CHoverCadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadView diagnostics

#ifdef _DEBUG
void CHoverCadView::AssertValid() const
{
	CView::AssertValid();
}

void CHoverCadView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHoverCadDoc* CHoverCadView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHoverCadDoc)));
	return (CHoverCadDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CHoverCadView message handlers



void CHoverCadView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
   int lXMin = 0;
   int lXMax = 200000;
   int lYMin = 0;
   int lYMax = 200000;

   CHoverCadDoc* lDocument = GetDocument();

	POSITION lPos = lDocument->GetNodeListHead();

   while( lPos != NULL )
   {
      HCNode* lNode = lDocument->GetNextNode( lPos );
      lXMin = min( lXMin, lNode->mX );
      lXMax = max( lXMax, lNode->mX );
      lYMin = min( lYMin, lNode->mY );
      lYMax = max( lYMax, lNode->mY );
   }

   mDocSize = CRect( lXMin, lYMin, lXMax, lYMax );
   // CRect lSize( (lXMax-lXMin)*mScaling+2*HC_MARGIN, (lYMax-lYMin)*mScaling+2*HC_MARGIN ); // doc size in mm
   CRect lDispSize( lXMin*mScaling-HC_MARGIN,
                    lYMin*mScaling-HC_MARGIN,
                    lXMax*mScaling+HC_MARGIN,
                    lYMax*mScaling+HC_MARGIN );

   // SetScrollSizes( HC_DISP_MODE, lSize );

   // Adjust scrollbars in mm units
   // but we also need to get mm conversion to know how much is visible
 	CRect       lClientRect;
   CClientDC   dc(this);

	OnPrepareDC(&dc);
   GetClientRect( lClientRect );
	dc.DPtoLP( lClientRect );

   lDispSize.NormalizeRect();
   lClientRect.NormalizeRect();

   {
      SCROLLINFO lScrollInfo = 
      {
         sizeof( lScrollInfo ),
         SIF_RANGE|SIF_PAGE|SIF_DISABLENOSCROLL,
         lDispSize.left,
         lDispSize.right,
         lClientRect.Width(),
         lClientRect.left,
         0
      };
      SetScrollInfo( SB_HORZ, &lScrollInfo );
   }


   {
      SCROLLINFO lScrollInfo = 
      {
         sizeof( lScrollInfo ),
         SIF_RANGE|SIF_PAGE|SIF_DISABLENOSCROLL,
         lDispSize.top,
         lDispSize.bottom,
         lClientRect.Height(),
         lClientRect.top,
         0
      };
      SetScrollInfo( SB_VERT, &lScrollInfo );
   }
   
	InvalidateRect( NULL );

   // Update Attrib pannel if needed
   if( GetFocus() ==  this )
   {
      SelectAttrib* lAttribTool = HC_GetApp()->GetSelectAttribWnd();

      if( lAttribTool != NULL )
      {
         lAttribTool->Refresh( );
      }
   }

}

BOOL CHoverCadView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if( nHitTest == HTCLIENT )
   {
      SetCursor( HC_GetApp()->GetCommandModeCursor() );
      return TRUE;
   }
   else
   {	
	   return CView::OnSetCursor(pWnd, nHitTest, message);
   }
}

void CHoverCadView::OnLButtonDown(UINT nFlags, CPoint point) 
{
   // convert pont to GDI coordinate
 	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

   mMouseDown = TRUE;

	// TODO: Add your message handler code here and/or call default
   switch( mCurrentMode )
   {
      case IDC_ROOM:
      case IDC_FEATURE:
      {
         CPoint lPosition = ClientToDoc( point );

         // Verify if we are not near an other point
         HCNode* lNode = GetNodeAtPosition( point, HC_NODE_RAY );

         if( mCurrentNodeList.IsEmpty() )
         {
            GetDocument()->ClearSelection();
         }

         if( lNode == NULL )
         {
            // Create a new point at the selected position
            if( mCurrentNodeList.GetCount()<MAX_NODE_PER_POLYGON )
            {
               lNode = GetDocument()->AddNode( lPosition.x, lPosition.y );
            }
         }


         if( lNode != NULL )
         {
            if( !mCurrentNodeList.IsEmpty() && (lNode == mCurrentNodeList.GetHead()) )
            {
               if( mCurrentNodeList.GetCount() >= 3 )
               { 
                  // select all nodes from the list
                  POSITION lPos = mCurrentNodeList.GetHeadPosition();

                  while( lPos != NULL )
                  {
                     mCurrentNodeList.GetNext( lPos )->mSelected = TRUE;
                  }

                  // add the polygon
                  GetDocument()->AddPolygon( mCurrentMode == IDC_FEATURE, mCurrentNodeList, 0, 4000, 0, 0, 0, TRUE  );
                  mCurrentNodeList.RemoveAll();
               }
            }
            else if( mCurrentNodeList.GetCount() <MAX_NODE_PER_POLYGON )
            {
               // Deselect the last entered node
               if( !mCurrentNodeList.IsEmpty() )
               {
                  mCurrentNodeList.GetTail()->mSelected = FALSE;
               }
               // Make the node selected and add it to the list
               mCurrentNodeList.AddTail( lNode );
               lNode->mSelected = TRUE;
               GetDocument()->UpdateAllViews( NULL );
            }
         }

      }
      break;

      case IDC_SELECT:
      {
         // Capture the mouse and draw a selection rectangle
         mCaptureMode = TRUE;
         SetCapture();

         if( !(nFlags&MK_SHIFT) )
         {
            // delete old selection
            GetDocument()->ClearSelection();
         }

         // Save the selection starting position
         mSelectionEnd = mSelectionStart = point;
      }
      break;

      case IDC_MOVE_SELECT:
      {
         // Capture the mouse
         mCaptureMode = TRUE;
         SetCapture();

         // save mouse position
         mSelectionStart = point;
      }
      break;

      case IDC_HAMMER:
      {
         // Clear current selection
         GetDocument()->ClearSelection();
         mCurrentNodeList.RemoveAll();

         // Verify if we selected an anchor or a wall
         HCSelectable* lBest = GetNearestSelectable( point );

         mSelectionStart = point;


         if( lBest != NULL )
         {
            // Capture the mouse and draw a selection rectangle
            mCaptureMode = TRUE;
            SetCapture();

            CPoint lPosition = ClientToDoc( point );

            // Determine if it is a node or an anchor
            if( lBest->IsKindOf( RUNTIME_CLASS( HCNode ) ))
            {
               // Select the node this way it will follow the moust
               lBest->mSelected = TRUE;
               GetDocument()->UpdateAllViews( NULL );

               mCurrentNodeList.AddTail( (HCNode*)lBest );
            }
            else
            {
               // Duplicate the associatated node and move the anchore to the new node
               HCNode* lNewNode = GetDocument()->DetachAnchor( (HCAnchorWall*)lBest, lPosition.x, lPosition.y );

               mCurrentNodeList.AddTail( lNewNode );

            }
         }
      }
      break;

      case IDC_INSERT_OBJECT:
      {
         GetDocument()->ClearSelection();

         CPoint lPosition = ClientToDoc( point );

         HCNode* lNode = GetDocument()->AddNode( lPosition.x, lPosition.y, TRUE );
         GetDocument()->AddItem( lNode, 0 );
      }
      break;
   }

	
	// CView::OnLButtonDown(nFlags, point);
}

void CHoverCadView::OnLButtonUp(UINT nFlags, CPoint point) 
{
   // convert pont to GDI coordinate
 	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

   mMouseDown = FALSE;

	// TODO: Add your message handler code here and/or call default
   if( mCaptureMode )
   {
      ReleaseCapture();
   }

   if( mCurrentMode == IDC_SELECT )
   {
      BOOL lRectangleSelect = TRUE;
      // Determine the type of selection that have been done
      int lXDiff = mSelectionStart.x - mSelectionEnd.x;
      int lYDiff = mSelectionStart.y - mSelectionEnd.y;

      if(   (lXDiff <= HC_NODE_RAY)&&(lXDiff >= -HC_NODE_RAY)
          &&(lYDiff <= HC_NODE_RAY)&&(lYDiff >= -HC_NODE_RAY) )
      {
         lRectangleSelect = FALSE;
      }

      if( !lRectangleSelect )
      {
         // Select the nearest nore or anchor
         SelectNearest( mSelectionEnd );
      }
      else
      {

         // Select everything in the specified region
         SelectRegion( mSelectionStart, mSelectionEnd );
      }
      // Ask for a redraw
      OnUpdate( NULL, NULL, NULL );
   }

   if( mCurrentMode == IDC_MOVE_SELECT )
   {

      // Move is already done
    
      // forget the rest
      // we now need to merge superposed nodes
      // those who are s

      // for each focussed and selected nodes, find a focussed node that 
      // have the same approx coordinate
      // replace selected node with the non selected and make onl one node

   }

   if( mCurrentMode == IDC_HAMMER )
   {
      // Verify if the selected node is not over an other node
      // if it is the case merge the two nodes

      if( mCurrentNodeList.GetCount() == 1 )
      {
         HCNode* lSrcNode  = mCurrentNodeList.GetHead();
         HCNode* lDestNode = GetNodeAtPosition( point, HC_NODE_RAY, lSrcNode );

         if( lDestNode != NULL )
         {
            GetDocument()->MergeNode( lDestNode, lSrcNode );
            mCurrentNodeList.RemoveAll();
         }
      }
   }

	
	// CView::OnLButtonUp(nFlags, point);
}

void CHoverCadView::OnMouseMove(UINT nFlags, CPoint point) 
{
   // convert pont to GDI coordinate
 	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

   // Update position on status bar
   int lPosPaneIndex = HC_GetApp()->GetStatusBar()->CommandToIndex( ID_MOUSE_POS );

   if( lPosPaneIndex != -1 )
   {
      CPoint lDocPos = ClientToDoc( point );
      CString lTextPos;
      lTextPos.Format("%.3f , %.3f", lDocPos.x/1000.0, lDocPos.y/1000.0 );
      HC_GetApp()->GetStatusBar()->SetPaneText( lPosPaneIndex, lTextPos );
   }


   switch( mCurrentMode )
   {
      case IDC_ROOM:
      case IDC_FEATURE:
      {
         // CPoint lPosition = ClientToDoc( point );

         // Verify if we are not near an other point
         HCNode* lNode = GetNodeAtPosition( point, HC_NODE_RAY );

         if( lNode != NULL )
         {
            if( !lNode->mFocussed )
            {
               GetDocument()->ClearSelection( TRUE );
               lNode->mFocussed = TRUE;
               GetDocument()->UpdateAllViews( NULL );
            }
         }
         else
         {
            // Clear the focussed points
            GetDocument()->ClearSelection( TRUE );
         }
      }
      break;

      case IDC_SELECT:
         {
            if( mMouseDown )
            {
               if( mSelectionEnd != point )
               {
                  mSelectionEnd = point;
                  OnUpdate( NULL, NULL, NULL );
               }
            }
            else
            {
               // Highlight pointed element
               // Verify if we are not near an other point
               HCNode* lNode = GetNodeAtPosition( point, HC_NODE_RAY );

               if( lNode != NULL )
               {
                  if( !lNode->mFocussed )
                  {
                     GetDocument()->ClearSelection( TRUE );
                     lNode->mFocussed = TRUE;
                     GetDocument()->UpdateAllViews( NULL );
                  }
               }
               else
               {
                  // Clear the focussed points
                  GetDocument()->ClearSelection( TRUE );
               }
            }
         }
         break;

      case IDC_HAMMER:
      case IDC_MOVE_SELECT:
         {
            if( mMouseDown )
            {
               // Move the selection
               // Compute the move to do
               CSize lMove = ClientToDoc(point)-ClientToDoc(mSelectionStart);
               mSelectionStart = point;

               GetDocument()->MoveSelection( lMove.cx, lMove.cy, 0 );
            }            
         }
         break;

   }

	// CView::OnMouseMove(nFlags, point);
}

void CHoverCadView::OnUpdateCut(CCmdUI* pCmdUI) 
{
	// Scan document to see if there is something selected
   pCmdUI->Enable( GetDocument()->IsSelected() );
	
}

void CHoverCadView::OnCutCmd() 
{
   // Remove from the vurrent list node that will be deleted
   POSITION lPos = mCurrentNodeList.GetHeadPosition();

   while( lPos != NULL )
   { 
      POSITION lOldPos = lPos;

      if( mCurrentNodeList.GetNext( lPos )->mSelected )
      {
         mCurrentNodeList.RemoveAt( lOldPos );
      }
   }

	// Delete selection
   GetDocument()->DeleteSelection();	
}

void CHoverCadView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
   // TRACE( "OnActivate %d %d %d\n", (int)bActivate, (int)pActivateView, (int)pDeactiveView );

	// TODO: Add your specialized code here and/or call the base class
   if( pActivateView != NULL )
   {
      SelectAttrib* lAttribTool = HC_GetApp()->GetSelectAttribWnd();
      CZoomBar*     lZoomBar    = HC_GetApp()->GetZoomBar();
      CDocAttrib*   lDocAttrib  = HC_GetApp()->GetDocAttribBar();

      if( lAttribTool != NULL )
      {
         if( bActivate )
         {
            lAttribTool->AttachDocument( GetDocument() );
         }
         else
         {
            lAttribTool->DetachDocument( GetDocument() );
         }
      }

      if( lDocAttrib != NULL )
      {
         if( bActivate )
         {
            lDocAttrib->AttachDocument( GetDocument() );
         }
         else
         {
            lDocAttrib->DetachDocument( GetDocument() );
         }
      }
      if( lZoomBar != NULL )
      {
         if( bActivate )
         {
            lZoomBar->AttachView( this );
         }
         else
         {
            lZoomBar->DetachView( this );
         }
      }


   }
	
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


void CHoverCadView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
   SCROLLINFO lScrollHorz;
   SCROLLINFO lScrollVert;

   GetScrollInfo( SB_HORZ, &lScrollHorz );
   GetScrollInfo( SB_VERT, &lScrollVert );
	pDC->SetMapMode( HC_DISP_MODE );
   // pDC->SetViewportOrg( lScrollHorz.nPos, lScrollVert.nPos  );
	pDC->SetWindowOrg( lScrollHorz.nPos, lScrollVert.nMax+lScrollVert.nMin-lScrollVert.nPos  );

	CView ::OnPrepareDC(pDC, pInfo);
}


BOOL CHoverCadView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CView ::OnScroll(nScrollCode, nPos, bDoScroll);
}

void CHoverCadView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
   SCROLLINFO lInfo;

   pScrollBar->GetScrollInfo( &lInfo, SIF_ALL );

   switch( nSBCode )
   {
      case SB_BOTTOM:
         lInfo.nPos = lInfo.nMax-lInfo.nPage;
         break;

      case SB_TOP:
         lInfo.nPos = lInfo.nMin;
         break;

      case SB_LINEDOWN:
         lInfo.nPos += max( 1, lInfo.nPage/20 );
         break;

      case SB_LINEUP:
         lInfo.nPos -= max( 1, lInfo.nPage/20 );
         break;

      case SB_PAGEDOWN:
         lInfo.nPos += lInfo.nPage*8/10;
         break;

      case SB_PAGEUP:
         lInfo.nPos -= lInfo.nPage*8/10;
         break;

      case SB_THUMBPOSITION:
      case SB_THUMBTRACK:
         lInfo.nPos = lInfo.nTrackPos;
         break;

      case SB_ENDSCROLL:
         break;
   }
   pScrollBar->SetScrollInfo( &lInfo );
	
	// CView ::OnVScroll(nSBCode, nPos, pScrollBar);

   // Ask for a redraw
   OnUpdate( NULL, NULL, NULL );

}

void CHoverCadView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
   SCROLLINFO lInfo;

   pScrollBar->GetScrollInfo( &lInfo, SIF_ALL );

   switch( nSBCode )
   {
      case SB_RIGHT:
         lInfo.nPos = lInfo.nMax-lInfo.nPage;
         break;

      case SB_LEFT:
         lInfo.nPos = lInfo.nMin;
         break;

      case SB_LINERIGHT:
         lInfo.nPos += max( 1, lInfo.nPage/20 );
         break;

      case SB_LINELEFT:
         lInfo.nPos -= max( 1, lInfo.nPage/20 );
         break;

      case SB_PAGERIGHT:
         lInfo.nPos += lInfo.nPage*8/10;
         break;

      case SB_PAGELEFT:
         lInfo.nPos -= lInfo.nPage*8/10;
         break;

      case SB_THUMBPOSITION:
      case SB_THUMBTRACK:
         lInfo.nPos = lInfo.nTrackPos;
         break;

      case SB_ENDSCROLL:
         break;
   }
   pScrollBar->SetScrollInfo( &lInfo );

   // Ask for a redraw
   OnUpdate( NULL, NULL, NULL );

	// CView ::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CHoverCadView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CView::OnChar(nChar, nRepCnt, nFlags);
}

void CHoverCadView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
   if( nChar == VK_DELETE )
   {
      OnCutCmd();
   }
   if( nChar == VK_INSERT )
   {
      OnInsertNodeCmd();
   }
	
	CView ::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CHoverCadView::OnUpdateInsertNode(CCmdUI* pCmdUI) 
{
   // Verify that there only one selected wall in the document
   pCmdUI->Enable( GetDocument()->GetSelectedWall() != NULL );

}

void CHoverCadView::OnInsertNodeCmd() 
{
	// TODO: Add your command handler code here
   HCAnchorWall* lWall = GetDocument()->GetSelectedWall();

   if( lWall != NULL )
   {
      GetDocument()->SplitWall( lWall );
   }
}
