/////////////////////////////////////////////////////////////////////////////
// HoverCadDoc.h : interface of the CHoverCadDoc class
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


// NOTE: All mesurement are in mm

#define MAX_XY                        10000000
#define MIN_XY                       -10000000
#define MAX_Z                            20000
#define MIN_Z                           -20000

#define HC_NO_SELECTION            -1000000001
#define HC_DIFFERENT_SELECTION     -1000000002
#define HC_READ_ONLY               -1000000002

#define MAX_NODE_PER_POLYGON       12


// SeriasableObject list template

template < class TYPE > class CSerialObjList:public CList< TYPE*, TYPE*>
{
   public:
      void Serialize( CArchive& ar )
      {
         int lCount;

         if( ar.IsStoring() )
         {
            lCount = GetCount();
            ar << lCount;

            POSITION lPos = GetHeadPosition();

            while( lPos )
            {
               ar << GetNext( lPos );
            }
         }
         else
         {
            ar >> lCount;

            for( int lCounter = 0; lCounter < lCount; lCounter++ )
            {
               TYPE* lPtr;

               ar >> lPtr;

               AddTail( lPtr );
            }
         }
      }
};



// Helper class


class HCAnchorWall;
class HCNode;
class HCPolygon;
class HCItem;

class HCSelectable: public CObject
{
   public:
      BOOL mSelected;
      BOOL mFocussed;

      HCSelectable();

      DECLARE_SERIAL( HCSelectable );
};

class HCAnchorWall: public HCSelectable
{
   public:
      int       mWallTexture;
      HCNode*    mNode;
      HCPolygon* mPolygon;

      int  mExportId;   // used only for Export function


      HCAnchorWall();
      ~HCAnchorWall();

      HCAnchorWall* GetNext();
      HCAnchorWall* GetPrev();

      void Serialize( CArchive& pArchive );
      HCAnchorWall& operator =( const HCAnchorWall& pAnchorWall );

      DECLARE_SERIAL( HCAnchorWall );

};


class HCNode:public HCSelectable
{
   public:
      int mX;
      int mY;


      CSerialObjList< HCAnchorWall > mAnchorList;
      HCItem* mItem;

      HCNode();
      ~HCNode();

      void Serialize( CArchive& pArchive );
      HCNode& operator =( const HCNode& pNode );

      DECLARE_SERIAL( HCNode );
};



class HCPolygon: public CObject
{
   public:
      BOOL mFeature;
      int  mFloorTexture;
      int  mCeilingTexture;
      int  mFloorLevel;
      int  mCeilingLevel;

      int  mExportId;   // used only for Export function

      CSerialObjList< HCAnchorWall > mAnchorWallList;
      // CList< CItem* , CItem* >            mItemList;

      HCPolygon();
      ~HCPolygon();

      BOOL      IsIncluding( int pX, int pY );

      void      Serialize( CArchive& pArchive );
      HCPolygon& operator =( const HCPolygon& pPolygon );

      DECLARE_SERIAL( HCPolygon );
};


class HCNodeList: public CList< HCNode*, HCNode*> 
{
};


class HCItem:public CObject
{
   public:
      HCNode* mNode;
      int     mElementType;
      int     mDistanceFromFloor;
      int     mOrientation;

      HCItem();
      ~HCItem();
      void   Serialize( CArchive& pArchive );
      HCItem& operator =( const HCItem& pItem );

      DECLARE_SERIAL( HCItem );
};



class CHoverCadDoc : public CDocument
{
protected: // create from serialization only
	CHoverCadDoc();
	DECLARE_DYNCREATE(CHoverCadDoc)

// Attributes
protected:

   CSerialObjList< HCNode    > mNodeList;
   CSerialObjList< HCPolygon > mPolygonList;
   // CSerialObjList< HCItem    > mItemList;

   CString mDescription;
   CString mBackImageName;


// Operations
private:
   // Helper func
   HCPolygon* GetRoomForNode( HCNode* pNode );

public:

   // Selections functions
   void ClearSelection( BOOL pFocusOnly = FALSE );
   // void SetSelectionFlag();
   // void AddSelection( CRect ); Must be implemented by the view

   // Operations on selection
   void DeleteSelection();
   void MoveSelection( int pXMove, int pYMove, int pZove );
   BOOL IsSelected( BOOL pNodeOnly = FALSE );

   int  SetWallTexture( int pTexture );    // All the next functions return the 
   int  SetCeilingTexture( int pTexture ); // final value. To get the current value, pass
   int  SetFloorTexture( int pTexture );   // a value of HC_READ_ONLY
   int  SetCeilingLevel( int pLevel );
   int  SetFloorLevel( int pLevel );

   int  SetItemType( int pType );
   int  SetXPos( int pValue );
   int  SetYPos( int pValue );
   int  SetZPos( int pValue );
   int  SetOrientation( int pValue );

   void ScaleSelection( double pRatio );

   // Construction functions
   // CNodeList GetNode( CRect pRect );
   HCNode* AddNode( int pX, int pY, BOOL pSelected = FALSE );

   void AddPolygon( BOOL pFeature, HCNodeList& pNodeList, int pFloorLevel, int pCeilingLevel, int pWallTexture, int pFloorTexture, int pCeilingTexture, BOOL pSelected = FALSE  );
   void AddItem( HCNode* pNode, int pItemType );

   HCAnchorWall* GetSelectedWall();
   BOOL          SplitWall( HCAnchorWall* pWall );
   HCNode*       DetachAnchor( HCAnchorWall* pAnchor, int pX, int pY );
   void          MergeNode( HCNode* pDest, HCNode* pSrc );

   BOOL GenerateOutputFile( FILE* pFile );

   // Info retrieving for drawing
   POSITION  GetNodeListHead();
   HCNode*    GetNextNode( POSITION& pPos );

   POSITION  GetPolygonListHead();
   HCPolygon* GetNextPolygon( POSITION& pPos );


   // Global Info
   CString GetBackImageName();
   CString GetDescription();
   void    SetBackImageName( const char* pName ); 
   void    SetDescription( const char* pDesc ); 

   
   /*
   POSITION  GetPolygonListHead();
   POSITION  GetNextPolygon( POSITION pPos );
   CPolygon  GetPolygon( POSITION pPos );
   */
   


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHoverCadDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHoverCadDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CHoverCadDoc)
	afx_msg void OnFileCompile();
	afx_msg void OnUpdateEditScaleselection(CCmdUI* pCmdUI);
	afx_msg void OnEditScaleselection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};





/////////////////////////////////////////////////////////////////////////////
