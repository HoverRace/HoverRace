// HoverCadDoc.cpp : implementation of the CHoverCadDoc class
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
#include <process.h>
#include <direct.h>

// #include "HoverCad.h"
#include "HoverCadDoc.h"
#include "HoverTypes.h"
// #include "ScaleSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CHoverCadDoc::ClearSelection(BOOL pFocusOnly)
{
	// Clear selection and focus
	BOOL lModified = FALSE;
	POSITION lPolygonPos;
	POSITION lAnchorPos;
	POSITION lNodePos;

	// Clear all selected anchor
	lPolygonPos = mPolygonList.GetHeadPosition();

	while(lPolygonPos != NULL) {
		HCPolygon *lPolygon = mPolygonList.GetNext(lPolygonPos);

		lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

		while(lAnchorPos != NULL) {
			HCAnchorWall *lAnchor = lPolygon->mAnchorWallList.GetNext(lAnchorPos);

			if(!pFocusOnly && lAnchor->mSelected) {
				lModified = TRUE;
				lAnchor->mSelected = FALSE;
			}

			if(lAnchor->mFocussed) {
				lModified = TRUE;
				lAnchor->mFocussed = FALSE;
			}
		}
	}

	// Clear all selected nodes
	lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(!pFocusOnly && lNode->mSelected) {
			lModified = TRUE;
			lNode->mSelected = FALSE;
		}

		if(lNode->mFocussed) {
			lModified = TRUE;
			lNode->mFocussed = FALSE;
		}
	}

	if(lModified) {
		UpdateAllViews(NULL);
	}

}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadDoc construction/destruction
CHoverCadDoc::CHoverCadDoc()
{
	// TODO: add one-time construction code here

}

CHoverCadDoc::~CHoverCadDoc()
{
	// destroy lists content
	/*
	   while( !mItemList.IsEmpty() )
	   {
	   delete mItemList.GetHead();
	   mItemList.RemoveHead();
	   }
	 */

	while(!mPolygonList.IsEmpty()) {
		delete mPolygonList.GetHead();
		mPolygonList.RemoveHead();
	}

	while(!mNodeList.IsEmpty()) {
		delete mNodeList.GetHead();
		mNodeList.RemoveHead();
	}

}

// TODO: add reinitialization code here
BOOL CHoverCadDoc::OnNewDocument()
{
	if(!CDocument::OnNewDocument())
		return FALSE;

	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadDoc serialization

void CHoverCadDoc::Serialize(CArchive & ar)
{

	mNodeList.Serialize(ar);
	mPolygonList.Serialize(ar);
	// mItemList.Serialize( ar );

	if(ar.IsStoring()) {
		ar << mBackImageName;
		ar << mDescription;
	}
	else {
		ar >> mBackImageName;
		ar >> mDescription;
	}

	// SetModifiedFlag( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CHoverCadDoc diagnostics

#ifdef _DEBUG
void CHoverCadDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHoverCadDoc::Dump(CDumpContext & dc) const
{
	CDocument::Dump(dc);
}
#endif											  //_DEBUG

HCNode *CHoverCadDoc::AddNode(int pX, int pY, BOOL pSelected)
{
	HCNode *lReturnValue = new HCNode;

	if(lReturnValue != NULL) {
		lReturnValue->mX = pX;
		lReturnValue->mY = pY;
		lReturnValue->mSelected = pSelected;

		mNodeList.AddTail(lReturnValue);

		UpdateAllViews(NULL);
		SetModifiedFlag();
	}

	return lReturnValue;
}

void CHoverCadDoc::AddPolygon(BOOL pFeature, HCNodeList & pNodeList, int pFloorLevel, int pCeilingLevel, int pWallTexture, int pFloorTexture, int pCeilingTexture, BOOL pSelected)
{
	// Remove duplicate entries from the list
	POSITION lPos = pNodeList.GetHeadPosition();
	BOOL lClockwise = TRUE;

	while(lPos != NULL) {
		HCNode *lNode = pNodeList.GetNext(lPos);

		POSITION lFollowing = lPos;

		while(lFollowing != NULL) {
			POSITION lOldPos = lFollowing;

			if(pNodeList.GetNext(lFollowing) == lNode) {
				if(lOldPos == lPos) {
					lPos = lFollowing;
				}
				pNodeList.RemoveAt(lOldPos);
			}
		}
	}

	if(pNodeList.GetCount() >= 3) {
		// Order the list clockwise
		// not the best algo.. but i do not remember the best way

		double lSum = 0;

		POSITION lPos = pNodeList.GetHeadPosition();

		CPoint lFirstPoint = CPoint(pNodeList.GetAt(lPos)->mX, pNodeList.GetAt(lPos)->mY);
		pNodeList.GetNext(lPos);

		CPoint lSecondPoint = CPoint(pNodeList.GetAt(lPos)->mX, pNodeList.GetAt(lPos)->mY);
		pNodeList.GetNext(lPos);

		while(lPos != NULL) {
			CPoint lCurrentPoint = CPoint(pNodeList.GetAt(lPos)->mX, pNodeList.GetAt(lPos)->mY);
			pNodeList.GetNext(lPos);

			/*
			   lAngle +=  atan2( lCurrentPoint.y-lSecondPoint.y, lCurrentPoint.x-lSecondPoint.x )
			   -atan2( lSecondPoint.y-lFirstPoint.y, lSecondPoint.x-lFirstPoint.x );
			 */

			lSum = ((double) lCurrentPoint.y - (double) lSecondPoint.y) * ((double) lSecondPoint.x - (double) lFirstPoint.x)
				- ((double) lCurrentPoint.x - (double) lSecondPoint.x) * ((double) lSecondPoint.y - (double) lFirstPoint.y);

			lFirstPoint = lSecondPoint;
			lSecondPoint = lCurrentPoint;
		}

		if(lSum > 0.0) {
			// Wrong side
			lClockwise = FALSE;
		}

	}

	if(pNodeList.GetCount() >= 3) {
		HCPolygon *lNewPolygon = new HCPolygon;

		lNewPolygon->mFeature = pFeature;
		lNewPolygon->mCeilingTexture = pCeilingTexture;
		lNewPolygon->mFloorTexture = pFloorTexture;
		lNewPolygon->mCeilingLevel = pCeilingLevel;
		lNewPolygon->mFloorLevel = pFloorLevel;

		// Create the needed anchor
		POSITION lPos;

		if(lClockwise) {
			lPos = pNodeList.GetHeadPosition();
		}
		else {
			lPos = pNodeList.GetTailPosition();
		}

		while((lPos != NULL) && lNewPolygon->mAnchorWallList.GetCount() < MAX_NODE_PER_POLYGON) {
			HCAnchorWall *lAnchor = new HCAnchorWall;
			HCNode *lNode;

			if(lClockwise) {
				lNode = pNodeList.GetNext(lPos);
			}
			else {
				lNode = pNodeList.GetPrev(lPos);
			}

			lAnchor->mSelected = pSelected;
			lAnchor->mWallTexture = pWallTexture;
			lAnchor->mNode = lNode;
			lAnchor->mPolygon = lNewPolygon;

			lNode->mAnchorList.AddTail(lAnchor);
			lNewPolygon->mAnchorWallList.AddTail(lAnchor);
		}

		mPolygonList.AddTail(lNewPolygon);

		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
}

void CHoverCadDoc::AddItem(HCNode * pNode, int pItemType)
{
	ASSERT(pNode->mItem == NULL);

	pNode->mItem = new HCItem;

	pNode->mItem->mNode = pNode;
	pNode->mItem->mElementType = pItemType;
	pNode->mItem->mDistanceFromFloor = 0;

	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CHoverCadDoc::MergeNode(HCNode * pDest, HCNode * pSrc)
{
	// Move all the anchor from src to Dest and drop anchor
	// from polygons already attached to the dest

	POSITION lSrcPos = pSrc->mAnchorList.GetHeadPosition();

	while(lSrcPos != NULL) {
		POSITION lCurrentNodePos = lSrcPos;
		HCAnchorWall *lAnchor = pSrc->mAnchorList.GetNext(lSrcPos);

		// Verify if not already in dest
		BOOL lAlreadyIncluded = FALSE;

		POSITION lDestPos = pDest->mAnchorList.GetHeadPosition();

		while(lDestPos != NULL) {
			if(pDest->mAnchorList.GetNext(lDestPos)->mPolygon == lAnchor->mPolygon) {
				lAlreadyIncluded = TRUE;
				break;
			}
		}

		if(lAlreadyIncluded) {
			// Delete the anchor
			POSITION lPos = lAnchor->mPolygon->mAnchorWallList.Find(lAnchor);
			lAnchor->mPolygon->mAnchorWallList.RemoveAt(lPos);

			delete lAnchor;
		}
		else {
			// Move the anchor
			pSrc->mAnchorList.RemoveAt(lCurrentNodePos);
			pDest->mAnchorList.AddTail(lAnchor);
			lAnchor->mNode = pDest;
		}
	}

	// Delete the node
	POSITION lPos = mNodeList.Find(pSrc);

	ASSERT(lPos != NULL);
	mNodeList.RemoveAt(lPos);
	delete pSrc;

	// Ask for the refresh
	UpdateAllViews(NULL);
	SetModifiedFlag();

}

HCPolygon *CHoverCadDoc::GetRoomForNode(HCNode * pNode)
{
	HCPolygon *lReturnValue = NULL;
	POSITION lPos = mPolygonList.GetHeadPosition();

	while(lPos != NULL) {
		HCPolygon *lPolygon = mPolygonList.GetNext(lPos);

		if(!lPolygon->mFeature) {
			if(lPolygon->IsIncluding(pNode->mX, pNode->mY)) {
				lReturnValue = lPolygon;
				break;
			}
		}
	}
	return lReturnValue;
}

// Info retrieving for drawing
POSITION CHoverCadDoc::GetNodeListHead()
{
	return mNodeList.GetHeadPosition();
}

HCNode *CHoverCadDoc::GetNextNode(POSITION & pPos)
{
	return mNodeList.GetNext(pPos);
}

POSITION CHoverCadDoc::GetPolygonListHead()
{
	return mPolygonList.GetHeadPosition();
}

HCPolygon *CHoverCadDoc::GetNextPolygon(POSITION & pPos)
{
	return mPolygonList.GetNext(pPos);
}

HCPolygon *CHoverCadDoc::GetPolygonTail()
{
	return mPolygonList.GetTail();
}

int CHoverCadDoc::SetWallTexture(int pTexture)
{
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// For each selected anchor if the followng is also selected
	// set the texture to the new texture

	POSITION lPolygonPos = mPolygonList.GetHeadPosition();

	while(lPolygonPos != NULL) {
		HCPolygon *lPolygon = mPolygonList.GetNext(lPolygonPos);

		POSITION lAnchorPos = lPolygon->mAnchorWallList.GetTailPosition();

		BOOL lNextIsSelected = lPolygon->mAnchorWallList.GetHead()->mSelected;

		while(lAnchorPos != NULL) {
			HCAnchorWall *lAnchor = lPolygon->mAnchorWallList.GetPrev(lAnchorPos);

			if(lNextIsSelected && lAnchor->mSelected) {
				if((pTexture != HC_READ_ONLY) && (pTexture != lAnchor->mWallTexture)) {
					lModif = TRUE;
					lAnchor->mWallTexture = pTexture;
				}

				if(lReturnValue == HC_NO_SELECTION) {
					lReturnValue = lAnchor->mWallTexture;
				}
				else if(lReturnValue != lAnchor->mWallTexture) {
					lReturnValue = HC_DIFFERENT_SELECTION;
				}
			}
			lNextIsSelected = lAnchor->mSelected;
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}

	return lReturnValue;
}

int CHoverCadDoc::SetCeilingTexture(int pTexture)
{
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected polygons (polygons where all anchor are selected

	POSITION lPolygonPos = mPolygonList.GetHeadPosition();

	while(lPolygonPos != NULL) {
		BOOL lIsSelected = TRUE;

		HCPolygon *lPolygon = mPolygonList.GetNext(lPolygonPos);

		POSITION lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

		while(lIsSelected && (lAnchorPos != NULL)) {
			if(!lPolygon->mAnchorWallList.GetNext(lAnchorPos)->mSelected) {
				lIsSelected = FALSE;
			}
		}

		if(lIsSelected) {
			if((pTexture != HC_READ_ONLY) && (pTexture != lPolygon->mCeilingTexture)) {
				lModif = TRUE;
				lPolygon->mCeilingTexture = pTexture;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lPolygon->mCeilingTexture;
			}
			else if(lReturnValue != lPolygon->mCeilingTexture) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;
}

int CHoverCadDoc::SetFloorTexture(int pTexture)
{
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected polygons (polygons where all anchor are selected

	POSITION lPolygonPos = mPolygonList.GetHeadPosition();

	while(lPolygonPos != NULL) {
		BOOL lIsSelected = TRUE;

		HCPolygon *lPolygon = mPolygonList.GetNext(lPolygonPos);

		POSITION lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

		while(lIsSelected && (lAnchorPos != NULL)) {
			if(!lPolygon->mAnchorWallList.GetNext(lAnchorPos)->mSelected) {
				lIsSelected = FALSE;
			}
		}

		if(lIsSelected) {
			if((pTexture != HC_READ_ONLY) && (pTexture != lPolygon->mFloorTexture)) {
				lModif = TRUE;
				lPolygon->mFloorTexture = pTexture;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lPolygon->mFloorTexture;
			}
			else if(lReturnValue != lPolygon->mFloorTexture) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;
}

int CHoverCadDoc::SetCeilingLevel(int pLevel)
{
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected polygons (polygons where all anchor are selected

	POSITION lPolygonPos = mPolygonList.GetHeadPosition();

	while(lPolygonPos != NULL) {
		BOOL lIsSelected = TRUE;

		HCPolygon *lPolygon = mPolygonList.GetNext(lPolygonPos);

		POSITION lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

		while(lIsSelected && (lAnchorPos != NULL)) {
			if(!lPolygon->mAnchorWallList.GetNext(lAnchorPos)->mSelected) {
				lIsSelected = FALSE;
			}
		}

		if(lIsSelected) {
			if((pLevel != HC_READ_ONLY) && (pLevel != lPolygon->mCeilingLevel)) {
				lModif = TRUE;
				lPolygon->mCeilingLevel = pLevel;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lPolygon->mCeilingLevel;
			}
			else if(lReturnValue != lPolygon->mCeilingLevel) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;
}

int CHoverCadDoc::SetFloorLevel(int pLevel)
{
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected polygons (polygons where all anchor are selected

	POSITION lPolygonPos = mPolygonList.GetHeadPosition();

	while(lPolygonPos != NULL) {
		BOOL lIsSelected = TRUE;

		HCPolygon *lPolygon = mPolygonList.GetNext(lPolygonPos);

		POSITION lAnchorPos = lPolygon->mAnchorWallList.GetHeadPosition();

		while(lIsSelected && (lAnchorPos != NULL)) {
			if(!lPolygon->mAnchorWallList.GetNext(lAnchorPos)->mSelected) {
				lIsSelected = FALSE;
			}
		}

		if(lIsSelected) {
			if((pLevel != HC_READ_ONLY) && (pLevel != lPolygon->mFloorLevel)) {
				lModif = TRUE;
				lPolygon->mFloorLevel = pLevel;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lPolygon->mFloorLevel;
			}
			else if(lReturnValue != lPolygon->mFloorLevel) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;
}

int CHoverCadDoc::SetItemType(int pType)
{
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected nodes where an item is attached

	POSITION lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(lNode->mSelected && (lNode->mItem != NULL)) {
			if((pType != HC_READ_ONLY) && (pType != lNode->mItem->mElementType)) {
				lModif = TRUE;
				lNode->mItem->mElementType = pType;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lNode->mItem->mElementType;
			}
			else if(lReturnValue != lNode->mItem->mElementType) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;
}

int CHoverCadDoc::SetXPos(int pValue)
{
	// Usefull but dangerous function
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	POSITION lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(lNode->mSelected) {
			if((pValue != HC_READ_ONLY) && (pValue != lNode->mX)) {
				lModif = TRUE;
				lNode->mX = pValue;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lNode->mX;
			}
			else if(lReturnValue != lNode->mX) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;
}

int CHoverCadDoc::SetYPos(int pValue)
{
	// Usefull but dangerous function
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	POSITION lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(lNode->mSelected) {
			if((pValue != HC_READ_ONLY) && (pValue != lNode->mY)) {
				lModif = TRUE;
				lNode->mY = pValue;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lNode->mY;
			}
			else if(lReturnValue != lNode->mY) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;

}

int CHoverCadDoc::SetZPos(int pValue)
{
	// Only works with items.. return floor relative level
	// Only works with items
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected nodes where an item is attached

	POSITION lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(lNode->mSelected && (lNode->mItem != NULL)) {
			if((pValue != HC_READ_ONLY) && (pValue != lNode->mItem->mDistanceFromFloor)) {
				lModif = TRUE;
				lNode->mItem->mDistanceFromFloor = pValue;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lNode->mItem->mDistanceFromFloor;
			}
			else if(lReturnValue != lNode->mItem->mDistanceFromFloor) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;

}

void CHoverCadDoc::ScaleSelection(double pRatio)
{
	// First find the center of the selection
	int lXMin = MAX_XY;
	int lXMax = MIN_XY;
	int lYMin = MAX_XY;
	int lYMax = MIN_XY;

	POSITION lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(lNode->mSelected) {
			lXMin = min(lNode->mX, lXMin);
			lXMax = max(lNode->mX, lXMax);
			lYMin = min(lNode->mY, lYMin);
			lYMax = max(lNode->mY, lYMax);
		}
	}

	if(lXMax >= lXMin) {
		int lXCenter = (lXMin + lXMax) / 2;
		int lYCenter = (lXMin + lXMax) / 2;

		POSITION lNodePos = mNodeList.GetHeadPosition();

		while(lNodePos != NULL) {
			HCNode *lNode = mNodeList.GetNext(lNodePos);

			if(lNode->mSelected) {
				lNode->mX = ((lNode->mX - lXCenter) * pRatio) + lXCenter;
				lNode->mY = ((lNode->mY - lYCenter) * pRatio) + lYCenter;
			}
		}

		UpdateAllViews(NULL);
		SetModifiedFlag();
	}

}

int CHoverCadDoc::SetOrientation(int pValue)
{
	// Only works with items
	BOOL lModif = FALSE;
	int lReturnValue = HC_NO_SELECTION;

	// Modify all selected nodes where an item is attached

	POSITION lNodePos = mNodeList.GetHeadPosition();

	while(lNodePos != NULL) {
		HCNode *lNode = mNodeList.GetNext(lNodePos);

		if(lNode->mSelected && (lNode->mItem != NULL)) {
			if((pValue != HC_READ_ONLY) && (pValue != lNode->mItem->mOrientation)) {
				lModif = TRUE;
				lNode->mItem->mOrientation = pValue;
			}

			if(lReturnValue == HC_NO_SELECTION) {
				lReturnValue = lNode->mItem->mOrientation;
			}
			else if(lReturnValue != lNode->mItem->mOrientation) {
				lReturnValue = HC_DIFFERENT_SELECTION;
			}
		}
	}

	if(lModif) {
		UpdateAllViews(NULL);
		SetModifiedFlag();
	}
	return lReturnValue;

}

CString CHoverCadDoc::GetBackImageName()
{
	return mBackImageName;
}

CString CHoverCadDoc::GetDescription()
{
	return mDescription;
}

void CHoverCadDoc::SetBackImageName(const char *pName)
{
	if(CString(pName) != mBackImageName) {
		mBackImageName = pName;
		SetModifiedFlag();
	}
}

void CHoverCadDoc::SetDescription(const char *pDesc)
{
	if(CString(pDesc) != mDescription) {
		mDescription = pDesc;
		SetModifiedFlag();
	}
}

// CSelectable
HCSelectable::HCSelectable()
{
	mSelected = FALSE;
	mFocussed = FALSE;
}

IMPLEMENT_SERIAL(HCSelectable, CObject, 1);

// HCNode
HCNode::HCNode()
{
	mX = 0;
	mY = 0;
	mItem = NULL;
}

HCNode::~HCNode()
{
	delete mItem;
}

IMPLEMENT_SERIAL(HCNode, HCSelectable, 1);

void HCNode::Serialize(CArchive & pArchive)
{
	if(pArchive.IsStoring()) {
		pArchive << mX;
		pArchive << mY;
		pArchive << mItem;
	}
	else {
		pArchive >> mX;
		pArchive >> mY;
		pArchive >> mItem;
	}
	mAnchorList.Serialize(pArchive);

}

// HCAnchorWall
HCAnchorWall::HCAnchorWall()
{
	mWallTexture = 0;
	mNode = NULL;
	mPolygon = NULL;

}

HCAnchorWall::~HCAnchorWall()
{
	// Remove node reference
	if(mNode != NULL) {
		mNode->mAnchorList.RemoveAt(mNode->mAnchorList.Find(this));
	}
}

IMPLEMENT_SERIAL(HCAnchorWall, HCSelectable, 1);

HCAnchorWall *HCAnchorWall::GetNext()
{
	HCAnchorWall *lReturnValue = NULL;

	if(mPolygon != NULL) {
		POSITION lPos = mPolygon->mAnchorWallList.GetHeadPosition();

		while(lPos != NULL) {
			if(mPolygon->mAnchorWallList.GetNext(lPos) == this) {
				if(lPos == NULL) {
					lReturnValue = mPolygon->mAnchorWallList.GetHead();
				}
				else {
					lReturnValue = mPolygon->mAnchorWallList.GetAt(lPos);
				}

				break;
			}
		}
	}

	ASSERT(lReturnValue != NULL);

	return lReturnValue;
}

HCAnchorWall *HCAnchorWall::GetPrev()
{
	HCAnchorWall *lReturnValue = NULL;

	if(mPolygon != NULL) {
		POSITION lPos = mPolygon->mAnchorWallList.GetTailPosition();

		while(lPos != NULL) {
			if(mPolygon->mAnchorWallList.GetPrev(lPos) == this) {
				if(lPos == NULL) {
					lReturnValue = mPolygon->mAnchorWallList.GetTail();
				}
				else {
					lReturnValue = mPolygon->mAnchorWallList.GetAt(lPos);
				}
				break;
			}
		}

	}

	ASSERT(lReturnValue != NULL);

	return lReturnValue;
}

void HCAnchorWall::Serialize(CArchive & pArchive)
{
	if(pArchive.IsStoring()) {
		pArchive << WallTextureList[mWallTexture].mSerialId;
		pArchive << mNode;
		pArchive << mPolygon;
	}
	else {
		pArchive >> mWallTexture;
		mWallTexture = GetTypeIndex(mWallTexture, WallTexture);
		pArchive >> mNode;
		pArchive >> mPolygon;
	}
}

// HCPolygon
HCPolygon::HCPolygon()
{
	mFeature = FALSE;
}

HCPolygon::~HCPolygon()
{
	// Delete al the anchor from the list

	while(!mAnchorWallList.IsEmpty()) {
		delete mAnchorWallList.GetHead();
		mAnchorWallList.RemoveHead();
	}
}

IMPLEMENT_SERIAL(HCPolygon, CObject, 1);

BOOL HCPolygon::IsIncluding(int pX, int pY)
{
	BOOL lReturnValue = FALSE;

	// First do a bounding box test
	POSITION lPos;

	lPos = mAnchorWallList.GetHeadPosition();

	BOOL lXMin = FALSE;
	BOOL lXMax = FALSE;
	BOOL lYMin = FALSE;
	BOOL lYMax = FALSE;

	while(lPos != NULL) {
		HCNode *lNode = mAnchorWallList.GetNext(lPos)->mNode;

		if(pX >= lNode->mX) {
			lXMin = TRUE;
		}
		if(pX <= lNode->mX) {
			lXMax = TRUE;
		}
		if(pY >= lNode->mY) {
			lYMin = TRUE;
		}
		if(pY <= lNode->mY) {
			lYMax = TRUE;
		}
	}

	if(lXMin && lXMax && lYMin && lYMax) {
		lReturnValue = TRUE;

		lPos = mAnchorWallList.GetHeadPosition();

		while(lReturnValue && (lPos != NULL)) {
			HCNode *lNode = mAnchorWallList.GetNext(lPos)->mNode;

			HCNode *lNextNode;
			if(lPos == NULL) {
				lNextNode = mAnchorWallList.GetHead()->mNode;
			}
			else {
				lNextNode = mAnchorWallList.GetAt(lPos)->mNode;
			}

			LONGLONG lVectorProd = Int32x32To64((lNextNode->mY - lNode->mY), (pX - lNode->mX))
				- Int32x32To64((lNextNode->mX - lNode->mX), (pY - lNode->mY));

			if(lVectorProd < 0) {
				lReturnValue = FALSE;
			}
		}
	}
	return lReturnValue;
}

void HCPolygon::Serialize(CArchive & pArchive)
{
	if(pArchive.IsStoring()) {
		pArchive << mFeature;
		pArchive << FloorTextureList[mFloorTexture].mSerialId;
		pArchive << CeilingTextureList[mCeilingTexture].mSerialId;
		pArchive << mFloorLevel;
		pArchive << mCeilingLevel;

	}
	else {
		pArchive >> mFeature;
		pArchive >> mFloorTexture;
		mFloorTexture = GetTypeIndex(mFloorTexture, FloorTexture);
		pArchive >> mCeilingTexture;
		mCeilingTexture = GetTypeIndex(mCeilingTexture, CeilingTexture);
		pArchive >> mFloorLevel;
		pArchive >> mCeilingLevel;
	}
	mAnchorWallList.Serialize(pArchive);
}

// HCItem
HCItem::HCItem()
{
	mNode = 0;
	mElementType = 0;
	mDistanceFromFloor = 0;
	mOrientation = 0;
}

HCItem::~HCItem()
{
	if(mNode != NULL) {
		ASSERT(mNode->mItem = this);
		mNode->mItem = NULL;
	}
}

IMPLEMENT_SERIAL(HCItem, CObject, 1);

void HCItem::Serialize(CArchive & pArchive)
{
	if(pArchive.IsStoring()) {
		pArchive << mNode;
		pArchive << ObjectList[mElementType].mSerialId;
		pArchive << mDistanceFromFloor;
		pArchive << mOrientation;
	}
	else {
		pArchive >> mNode;
		pArchive >> mElementType;
		mElementType = GetTypeIndex(mElementType, Object);
		pArchive >> mDistanceFromFloor;
		pArchive >> mOrientation;
	}
}
