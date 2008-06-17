// Level.cpp
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

#include "stdafx.h"

#include "Level.h"

#define new DEBUG_NEW

// MR_Level implementation
MR_Level::MR_Level( BOOL pAllowRendering )
{
   // Initialisation of an empty level 
   mAllowRendering = pAllowRendering;

   mNbRoom      = 0;
   mRoomList    = NULL;
   mNbFeature   = 0;
   mFeatureList = NULL;
   mFreeElementNonClassifiedList    = NULL;
   mFreeElementClassifiedByRoomList = NULL;
   mNbPlayer    = 0;

   mElementCreationBroadcastHook  = NULL;
   mPermElementStateBroadcastHook = NULL;
   // Initialize the players list

   mNbPermNetActor = 0;
   mPermActorCacheCount = 0;

}
     
MR_Level::~MR_Level()
{

   // Delete free elements   
   while( mFreeElementNonClassifiedList != NULL )
   {
      delete mFreeElementNonClassifiedList;
   }

   for( int lCounter= 0; lCounter<mNbRoom; lCounter++ )
   {
      while( mFreeElementClassifiedByRoomList[ lCounter ] != NULL )
      {
         delete mFreeElementClassifiedByRoomList[ lCounter ];
      }
   }

   delete []mFreeElementClassifiedByRoomList;

   // Delete structure
   delete []mRoomList;
   delete []mFeatureList;

}

void MR_Level::SetBroadcastHook( void (*pCreationHook)(MR_FreeElement*, int, void* ),
                                 void (*pStateHook)   (MR_FreeElement*, int, int, void* ),
                                 void* pHookData )
{
   mElementCreationBroadcastHook     = pCreationHook;
   mPermElementStateBroadcastHook    = pStateHook;
   mBroadcastHookData                = pHookData;
}


// Serialization
void MR_Level::Serialize( CArchive& pArchive )
{
   int lCounter;
   int lPlayerNo;

   // Serialize the starting position
   if( pArchive.IsStoring() )
   {
      pArchive << mNbPlayer;

      for( lPlayerNo = 0; lPlayerNo < mNbPlayer; lPlayerNo++ )
      {
         pArchive << mPlayerTeam[lPlayerNo];
         pArchive << mStartingRoom[lPlayerNo];
         mStartingPosition[lPlayerNo].Serialize( pArchive );
         pArchive << mStartingOrientation[lPlayerNo];
      }
   }
   else
   {
      mNbPermNetActor = 0;

      pArchive >> mNbPlayer;

      for( lPlayerNo = 0; lPlayerNo < mNbPlayer; lPlayerNo++ )
      {
         pArchive >> mPlayerTeam[lPlayerNo];
         pArchive >> mStartingRoom[lPlayerNo];
         mStartingPosition[lPlayerNo].Serialize( pArchive );
         pArchive >> mStartingOrientation[lPlayerNo];
      }
   }


   // Serialize the structure
   if( pArchive.IsStoring() )
   {
      pArchive << mNbRoom;
      pArchive << mNbFeature;
   }
   else
   {
      ASSERT( mRoomList    == NULL );
      ASSERT( mFeatureList == NULL );

      pArchive >> mNbRoom;
      pArchive >> mNbFeature;

      mRoomList                        = new Room[ mNbRoom ];
      mFeatureList                     = new Feature[ mNbFeature ];
      mFreeElementClassifiedByRoomList = new FreeElement*[ mNbRoom ];

      for( lCounter=0; lCounter<mNbRoom; lCounter++ )
      {
         mFreeElementClassifiedByRoomList[ lCounter ] = NULL;
      }
   }


   for( lCounter=0; lCounter<mNbRoom; lCounter++ )
   {
      mRoomList[ lCounter ].SerializeStructure( pArchive );
   }   

   for( lCounter=0; lCounter<mNbFeature; lCounter++ )
   {
      mFeatureList[ lCounter ].SerializeStructure( pArchive );
   }   

   // Serialise the actors

   FreeElement::SerializeList( pArchive, &mFreeElementNonClassifiedList );

   for( lCounter=0; lCounter<mNbRoom; lCounter++ )
   {
      FreeElement::SerializeList( pArchive, &mFreeElementClassifiedByRoomList[ lCounter ] );

      if( !pArchive.IsStoring() )
      {
         FreeElement* lCurrentElem = mFreeElementClassifiedByRoomList[ lCounter ];

         while( lCurrentElem != NULL )
         {         
            if( mNbPermNetActor < MR_NB_PERNET_ACTORS )
            {
               // offer the current number to the current actor
               if( lCurrentElem->mElement->AssignPermNumber( mNbPermNetActor ) )
               {
                  mPermNetActor[ mNbPermNetActor ] = lCurrentElem;
                  mNbPermNetActor++;
               }
            }
            else
            {
               ASSERT( FALSE );
            }
            lCurrentElem = lCurrentElem->mNext;
         }
      }
   }   

   // the logic state of each element can now be serialized because all
   // elements are now created (this is only a precaution in case that some
   // elements have a link between them
   for( lCounter=0; lCounter<mNbRoom; lCounter++ )
   {
      mRoomList[ lCounter ].SerializeSurfacesLogicState( pArchive );
   }   

   for( lCounter=0; lCounter<mNbFeature; lCounter++ )
   {
      mFeatureList[ lCounter ].SerializeSurfacesLogicState( pArchive );
   }   

}



// Internal helper functions

int MR_Level::GetRoomCount()const
{
   return mNbRoom;
}

int MR_Level::GetPlayerCount()const
{
   return mNbPlayer;
}

int MR_Level::GetPlayerTeam( int pPlayerId )const
{
   return mPlayerTeam[ pPlayerId ];
}

int MR_Level::GetStartingRoom( int pPlayerId )const
{
   return mStartingRoom[ pPlayerId ];
}

const MR_3DCoordinate& MR_Level::GetStartingPos( int pPlayerId )const
{
   return mStartingPosition[ pPlayerId ];
}

MR_Angle MR_Level::GetStartingOrientation( int pPlayerId )const
{
   return mStartingOrientation[ pPlayerId ];
}

MR_PolygonShape* MR_Level::GetRoomShape( int pRoomId )const
{
   return new SectionShape( &mRoomList[ pRoomId ] );
}

MR_PolygonShape* MR_Level::GetFeatureShape( int pFeatureId )const
{
   return new SectionShape( &mFeatureList[ pFeatureId ] );
}

MR_Int32 MR_Level::GetRoomWallLen( int pRoomId, int pVertex )const
{
   return mRoomList[ pRoomId ].mWallLen[ pVertex ];
}

MR_Int32 MR_Level::GetFeatureWallLen( int pFeatureId, int pVertex )const
{
   return mFeatureList[ pFeatureId ].mWallLen[ pVertex ];
}

const MR_2DCoordinate& MR_Level::GetRoomVertex( int pRoomId, int pVertex )const
{
   return mRoomList[ pRoomId ].mVertexList[ pVertex ];
}

const MR_2DCoordinate& MR_Level::GetFeatureVertex( int pFeatureId, int pVertex )const
{
   return mFeatureList[ pFeatureId ].mVertexList[ pVertex ];
}

int MR_Level::GetRoomVertexCount( int pRoomId )const
{
   return mRoomList[ pRoomId ].mNbVertex;
}

int MR_Level::GetFeatureVertexCount( int pFeatureId )const
{
   return mFeatureList[ pFeatureId ].mNbVertex;
}

MR_Int32 MR_Level::GetRoomBottomLevel( int pRoomId )const
{
   return mRoomList[ pRoomId ].mFloorLevel;
}

MR_Int32 MR_Level::GetFeatureBottomLevel( int pFeatureId )const
{
   return mFeatureList[ pFeatureId ].mFloorLevel;
}


MR_Int32 MR_Level::GetRoomTopLevel( int pRoomId )const
{
   return mRoomList[ pRoomId ].mCeilingLevel;
}

MR_Int32 MR_Level::GetFeatureTopLevel( int pFeatureId )const
{
   return mFeatureList[ pFeatureId ].mCeilingLevel;
}

const int* MR_Level::GetVisibleZones( int pRoomId, int& pNbVisibleZones )const
{

   pNbVisibleZones = mRoomList[ pRoomId ].mNbVisibleRoom;

   return mRoomList[ pRoomId ].mVisibleRoomList;
}

int MR_Level::GetNbVisibleSurface( int pRoomId )const
{
   return mRoomList[ pRoomId ].mNbVisibleSurface;
}

const MR_SectionId* MR_Level::GetVisibleFloorList( int pRoomId )const
{
   return mRoomList[ pRoomId ].mVisibleFloorList;
}

const MR_SectionId* MR_Level::GetVisibleCeilingList( int pRoomId )const
{
   return mRoomList[ pRoomId ].mVisibleCeilingList;
}


    
int MR_Level::GetNeighbor( int pRoomId, int  pVertex )const
{
   return mRoomList[ pRoomId ].mNeighborList[ pVertex ];
}

int MR_Level::GetParent( int pFeatureId )const
{
   return mFeatureList[ pFeatureId ].mParentSectionIndex;
}

int MR_Level::GetFeatureCount( int pRoomId )const
{
   return mRoomList[ pRoomId ].mNbChild;
}

int MR_Level::GetFeature( int pRoomId, int pChildIndex )const
{
   return mRoomList[ pRoomId ].mChildList[ pChildIndex ];
}


MR_SurfaceElement* MR_Level::GetRoomWallElement(  int pRoomId, int  pVertex )const
{
   return mRoomList[ pRoomId ].mWallTexture[ pVertex ];
}

MR_SurfaceElement* MR_Level::GetFeatureWallElement(  int pFeatureId, int  pVertex )const
{
   return mFeatureList[ pFeatureId ].mWallTexture[ pVertex ];
}

MR_SurfaceElement* MR_Level::GetRoomBottomElement( int pRoomId )const    
{
   return mRoomList[ pRoomId ].mFloorTexture;
}

MR_SurfaceElement* MR_Level::GetFeatureBottomElement( int pFeatureId )const
{
   return mFeatureList[ pFeatureId ].mFloorTexture;
}


MR_SurfaceElement* MR_Level::GetRoomTopElement( int pRoomId )const
{
   return mRoomList[ pRoomId ].mCeilingTexture;
}

MR_SurfaceElement* MR_Level::GetFeatureTopElement( int pFeatureId )const
{
   return mFeatureList[ pFeatureId ].mCeilingTexture;
}

// FreeElements manipulation
MR_FreeElementHandle MR_Level::GetFirstFreeElement( int pRoom )const
{
   FreeElement* lReturnValue;

   if( pRoom == eNonClassified )
   {
      lReturnValue = mFreeElementNonClassifiedList;
   }
   else
   {
      lReturnValue = mFreeElementClassifiedByRoomList[ pRoom ];
   }
   return (MR_FreeElementHandle)lReturnValue;
}

MR_FreeElementHandle MR_Level::GetNextFreeElement( MR_FreeElementHandle pHandle )
{
   return (MR_FreeElementHandle)((FreeElement*)pHandle)->mNext;
}

MR_FreeElement* MR_Level::GetFreeElement( MR_FreeElementHandle pHandle )
{
   return ((FreeElement*)pHandle)->mElement;
}

void MR_Level::MoveElement( MR_FreeElementHandle pHandle, int pNewRoom )
{
   if( pNewRoom == eNonClassified )
   {
      ((FreeElement*)pHandle)->LinkTo( &mFreeElementNonClassifiedList );
   }
   else
   {
      ((FreeElement*)pHandle)->LinkTo( &(mFreeElementClassifiedByRoomList[pNewRoom]) );
   }
}

MR_FreeElementHandle MR_Level::InsertElement( MR_FreeElement* pElement, int pRoom, BOOL pBroadcast  )
{
   FreeElement* lReturnValue = new FreeElement;

   if( mAllowRendering )
   {
      pElement->AddRenderer();
   }

   lReturnValue->mElement = pElement;

   MoveElement( (MR_FreeElementHandle)lReturnValue, pRoom );

   // Broadcast element creation if needed
   if( pBroadcast && ( mElementCreationBroadcastHook != NULL ) )
   {
      mElementCreationBroadcastHook( pElement, pRoom, mBroadcastHookData );
   }
   return (MR_FreeElementHandle)lReturnValue;
}

void MR_Level::DeleteElement( MR_FreeElementHandle pHandle )
{
   delete ((FreeElement*)pHandle);
}

MR_FreeElementHandle MR_Level::GetPermanentElementHandle( int pElem )const
{

   ASSERT( pElem >= 0 );
   ASSERT( pElem < mNbPermNetActor );

   return (MR_FreeElementHandle)mPermNetActor[pElem];
}

void MR_Level::SetPermElementPos( int pPermElement, int pRoom, const MR_3DCoordinate& pNewPos )
{
   if( pPermElement >=0 )
   {
      ASSERT( pPermElement < mNbPermNetActor );

      // MoveElement( (MR_FreeElementHandle)mPermNetActor[ pPermElement ], pRoom );

      mPermNetActor[ pPermElement ]->mElement->mPosition = pNewPos;
      if( mPermElementStateBroadcastHook != NULL )
      {
         mPermElementStateBroadcastHook( mPermNetActor[ pPermElement ]->mElement, pRoom, pPermElement, mBroadcastHookData ); 
      }

      // Insert the element in the moving cache
      mPermActorIndexCache[mPermActorCacheCount] = pPermElement;
      mPermActorNewRoomCache[mPermActorCacheCount] = pRoom;
      mPermActorCacheCount++;
   }
}

void MR_Level::FlushPermElementPosCache()
{
   for( int lCounter = 0; lCounter < mPermActorCacheCount; lCounter++ )
   {
      MoveElement( (MR_FreeElementHandle)mPermNetActor[ mPermActorIndexCache[lCounter] ], mPermActorNewRoomCache[lCounter] );
   }
   mPermActorCacheCount = 0;

}


void MR_Level::GetRoomContact(   int                      pRoom,
                                 const MR_ShapeInterface* pShape,
                                 MR_RoomContactSpec&      pAnswer           )
{

   // Verify if the current room contains the requires shape
   MR_DetectRoomContact( pShape, &SectionShape( &(mRoomList[ pRoom ]) ), pAnswer );
}

BOOL MR_Level::GetRoomWallContactOrientation( int                      pRoom,
                                              int                      pWall,
                                              const MR_ShapeInterface* pShape,
                                              MR_Angle&                pAnswer   )
{
   return MR_GetWallForceLongitude( pShape, &SectionShape( &(mRoomList[ pRoom ]) ), pWall, pAnswer );
}



BOOL MR_Level::GetFeatureContact( int                      pFeature,
                                  const MR_ShapeInterface* pShape,
                                  MR_ContactSpec&          pAnswer           )
{

   // Verify if the current room contains the requires shape
   return MR_DetectFeatureContact( pShape, &SectionShape( &(mFeatureList[ pFeature ]) ), pAnswer );
}

BOOL MR_Level::GetFeatureContactOrientation( int                      pFeature,
                                             const MR_ShapeInterface* pShape,
                                             MR_Angle&                pAnswer              )
{
   return MR_GetFeatureForceLongitude( pShape, &SectionShape( &(mFeatureList[ pFeature ]) ), pAnswer );
}


// Sub classes implementation

// class MR_Level::SectionShape
MR_Level::SectionShape::SectionShape( MR_Level::Section *pSection )
{
   mSection = pSection;
}

MR_Int32 MR_Level::SectionShape::XMin()const
{
   return mSection->mMin.mX;
}

MR_Int32 MR_Level::SectionShape::XMax()const
{
   return mSection->mMax.mX;
}

MR_Int32 MR_Level::SectionShape::YMin()const
{
   return mSection->mMin.mY;
}

MR_Int32 MR_Level::SectionShape::YMax()const
{
   return mSection->mMax.mY;
}

MR_Int32 MR_Level::SectionShape::ZMin()const
{
   return mSection->mFloorLevel;
}

MR_Int32 MR_Level::SectionShape::ZMax()const
{
   return mSection->mCeilingLevel;
}

int MR_Level::SectionShape::VertexCount()const
{
   return mSection->mNbVertex;
}

MR_Int32 MR_Level::SectionShape::XCenter()const
{
   ASSERT( FALSE ); // Why do you need that???
   return 0;
}

MR_Int32 MR_Level::SectionShape::YCenter()const
{
   ASSERT( FALSE ); // Why do you need that???
   return 0;
}


MR_Int32 MR_Level::SectionShape::X( int pIndex )const
{
   return mSection->mVertexList[pIndex].mX;
}

MR_Int32 MR_Level::SectionShape::Y( int pIndex )const
{
   return mSection->mVertexList[pIndex].mY;
}

MR_Int32 MR_Level::SectionShape::SideLen( int pIndex )const
{
   return mSection->mWallLen[pIndex];
}
     

// class MR_Level::Section::AudibleRoom

MR_Level::Room::AudibleRoom::AudibleRoom()
{
   mSectionSource    = -1;
   mNbVertexSources  = 0;
   mVertexList       = NULL;
   mSoundCoefficient = NULL;
}

MR_Level::Room::AudibleRoom::~AudibleRoom()
{
   delete []mVertexList;
   delete []mSoundCoefficient;
}

void MR_Level::Room::AudibleRoom::Serialize(  CArchive& pArchive )
{
   int lCounter;

   if( pArchive.IsStoring() )
   {
      pArchive << mNbVertexSources;
      
      for( lCounter = 0; lCounter < mNbVertexSources; lCounter++ )
      {
         pArchive << mVertexList[ lCounter ]
                  << mSoundCoefficient[ lCounter ];
      }
   }
   else
   {
      ASSERT( mVertexList == NULL );// Serialisation permited only once

      // Retrieve data
      pArchive >> mNbVertexSources;

      mVertexList = new int[ mNbVertexSources ];
      mSoundCoefficient = new BYTE[ mNbVertexSources ];

      
      for( lCounter = 0; lCounter < mNbVertexSources; lCounter++ )
      {
         pArchive >> mVertexList[ lCounter ]
                  >> mSoundCoefficient[ lCounter ];
      }
   }

}

int MR_Level::FindRoomForPoint( const MR_2DCoordinate& pPosition, int pStartingRoom )const
{
   int lReturnValue = -1;



   // Verify if the position is included in the current section
   if( MR_GetPolygonInclusion( SectionShape( &mRoomList[ pStartingRoom ] ),
                               pPosition     ))
   {
      lReturnValue = pStartingRoom;
   }
   else
   {
      // Verify neighbor rooms
      for( int lCounter = 0; lCounter< mRoomList[ pStartingRoom ].mNbVertex; lCounter++ )
      {
         int lNeighbor = mRoomList[ pStartingRoom ].mNeighborList[ lCounter ];

         if( lNeighbor != -1 )
         {
            if( MR_GetPolygonInclusion( SectionShape( &mRoomList[ lNeighbor ] ),
                                        pPosition     ))
            {
               lReturnValue = lNeighbor;
               break;
            }
         }
      }
   }

   /*
   if( lReturnValue == -1 )
   {
      lReturnValue = pStartingRoom;
   }
   */

   return lReturnValue;

}


// class MR_Level::Section

MR_Level::Section::Section()
{

   mNbVertex     = 0;
   mVertexList   = NULL;
   mWallLen      = NULL;

   mWallTexture      = NULL;
   mFloorTexture     = NULL;
   mCeilingTexture   = NULL;
 
}

MR_Level::Section::~Section()
{
   delete []mVertexList;
   delete []mWallLen;

   delete mFloorTexture;
   delete mCeilingTexture;

   for( int lCounter = 0; lCounter < mNbVertex; lCounter++ )
   {
      delete mWallTexture[ lCounter ];
   }
   delete []mWallTexture;
   
}

void MR_Level::Section::SerializeStructure( CArchive& pArchive )
{
   int lCounter;

   if( pArchive.IsStoring() )
   {
      pArchive << mNbVertex;

      pArchive << mFloorLevel
               << mCeilingLevel;

      mMin.Serialize( pArchive ); 
      mMax.Serialize( pArchive );

      // Arrays
      for( lCounter = 0; lCounter < mNbVertex; lCounter++ )
      {
         mVertexList[ lCounter ].Serialize( pArchive );
         pArchive << mWallLen[ lCounter ];
      }


   }
   else
   {
      ASSERT( mVertexList == NULL ); // Can only be called once

      // Simple data
      pArchive >> mNbVertex;
      pArchive >> mFloorLevel
               >> mCeilingLevel;

      mMin.Serialize( pArchive ); 
      mMax.Serialize( pArchive );

      // Arrays
      mVertexList      = new MR_2DCoordinate[ mNbVertex ];
      mWallLen         = new MR_Int32[ mNbVertex ];


      for( lCounter = 0; lCounter < mNbVertex; lCounter++ )
      {
         mVertexList[ lCounter ].Serialize( pArchive );
         pArchive >> mWallLen[ lCounter ];
      }
   }

   // Serialize the textures
   MR_ObjectFromFactory::SerializePtr( pArchive, (MR_ObjectFromFactory*&) mFloorTexture );
   MR_ObjectFromFactory::SerializePtr( pArchive, (MR_ObjectFromFactory*&)mCeilingTexture );

   if( !pArchive.IsStoring() )
   {
      mWallTexture = new MR_SurfaceElement*[ mNbVertex ];
   }

   for( lCounter = 0; lCounter < mNbVertex; lCounter++ )
   {
      MR_ObjectFromFactory::SerializePtr( pArchive, (MR_ObjectFromFactory*&)mWallTexture[ lCounter ] );
   }
}


void MR_Level::Section::SerializeSurfacesLogicState( CArchive& pArchive )
{
   // Serialize the textures state
   if( mFloorTexture != NULL )
   {
      mFloorTexture->SerializeLogicState( pArchive );
   }

   if( mCeilingTexture != NULL )
   {
      mCeilingTexture->SerializeLogicState( pArchive );
   }

   for( int lCounter = 0; lCounter < mNbVertex; lCounter++ )
   {
      if( mWallTexture[ lCounter ] != NULL )
      {
         mWallTexture[ lCounter ]->SerializeLogicState( pArchive );
      }
   } 
}


// class MR_Level::Room

MR_Level::Room::Room()
{
   mNbChild            = 0;
   mChildList          = NULL;
   mNeighborList       = NULL;
   mNbVisibleRoom      = 0;
   mVisibleRoomList    = NULL;
   mNbVisibleSurface   = 0;
   mVisibleFloorList   = NULL;
   mVisibleCeilingList = NULL;

   mNbAudibleRoom      = 0;
   mAudibleRoomList    = NULL;

}

MR_Level::Room::~Room()
{
   delete []mChildList;
   delete []mNeighborList;
   delete []mVisibleRoomList;
   delete []mAudibleRoomList;
   delete []mVisibleFloorList;
   delete []mVisibleCeilingList;
}

void MR_Level::Room::SerializeStructure( CArchive& pArchive )
{
   int lCounter;
   Section::SerializeStructure( pArchive );

   if( pArchive.IsStoring() )
   {
      pArchive << mNbChild;
      pArchive << mNbVisibleRoom;
      pArchive << mNbVisibleSurface;
      pArchive << mNbAudibleRoom;

      for( lCounter = 0; lCounter < mNbVertex; lCounter++ )
      {
         pArchive << mNeighborList[ lCounter ];
      }

      for( lCounter = 0; lCounter < mNbChild; lCounter++ )
      {
         pArchive << mChildList[ lCounter ];
      }

      for( lCounter = 0; lCounter < mNbVisibleRoom; lCounter++ )
      {
         pArchive << mVisibleRoomList[ lCounter ];  // List of the room that are visible from the current room
      }

      for( lCounter = 0; lCounter < mNbVisibleSurface; lCounter++ )
      {
         mVisibleFloorList[   lCounter ].Serialize( pArchive );
         mVisibleCeilingList[ lCounter ].Serialize( pArchive );
      }

      for( lCounter = 0; lCounter < mNbAudibleRoom; lCounter++ )
      {
         mAudibleRoomList[ lCounter ].Serialize( pArchive );
      }
   }
   else
   {
      pArchive >> mNbChild;
      pArchive >> mNbVisibleRoom;
      pArchive >> mNbVisibleSurface;
      pArchive >> mNbAudibleRoom;

      // Arrays

      mNeighborList    = new int[ mNbVertex ];

      if( mNbChild != 0 )
      {
         mChildList       = new int[ mNbChild ];
      }

      if( mNbVisibleRoom != 0 )
      {
         mVisibleRoomList = new int[ mNbVisibleRoom ];
      }

      if( mNbVisibleSurface != 0 )
      {
         mVisibleFloorList   = new MR_SectionId[ mNbVisibleSurface ];
         mVisibleCeilingList = new MR_SectionId[ mNbVisibleSurface ];
      }

      if( mNbAudibleRoom != 0 )
      {
         mAudibleRoomList = new AudibleRoom[ mNbAudibleRoom ];
      }


      for( lCounter = 0; lCounter < mNbVertex; lCounter++ )
      {
         pArchive >> mNeighborList[ lCounter ];
      }

      for( lCounter = 0; lCounter < mNbChild; lCounter++ )
      {
         pArchive >> mChildList[ lCounter ];
      }

      for( lCounter = 0; lCounter < mNbVisibleRoom; lCounter++ )
      {
         pArchive >> mVisibleRoomList[ lCounter ];  // List of the room that are visible from the current room
      }

      for( lCounter = 0; lCounter < mNbVisibleSurface; lCounter++ )
      {
         mVisibleFloorList[   lCounter ].Serialize( pArchive );
         mVisibleCeilingList[ lCounter ].Serialize( pArchive );
      }

      for( lCounter = 0; lCounter < mNbAudibleRoom; lCounter++ )
      {
         mAudibleRoomList[ lCounter ].Serialize( pArchive );
      }
   }
}

// class MR_Level::Feature

void MR_Level::Feature::SerializeStructure( CArchive& pArchive )
{
   Section::SerializeStructure( pArchive );

   if( pArchive.IsStoring() )
   {
      pArchive << mParentSectionIndex;
   }
   else
   {
      pArchive >> mParentSectionIndex;
   }
}


// class MR_Level::FreeElement
MR_Level::FreeElement::FreeElement()
{
   mPrevLink = NULL;
   mNext     = NULL;
   mElement  = NULL;
}

MR_Level::FreeElement::~FreeElement()
{
   Unlink();
   delete mElement;
}

void MR_Level::FreeElement::Unlink()
{
   if( mNext != NULL )
   {
      mNext->mPrevLink = mPrevLink;
   }

   if( mPrevLink != NULL )
   {
      *mPrevLink = mNext;
   }
   mNext     = NULL;
   mPrevLink = NULL;
}

void MR_Level::FreeElement::LinkTo( FreeElement** pPrevLink )
{
   Unlink();

   mPrevLink  = pPrevLink;
   mNext      = *mPrevLink;
   *mPrevLink = this;

   if( mNext != NULL )
   {
      mNext->mPrevLink = &mNext;
   }
}

void MR_Level::FreeElement::SerializeList( CArchive& pArchive, FreeElement** pListHead )
{
   if( pArchive.IsStoring() )
   {
      FreeElement* lFreeElement = *pListHead;

      while( lFreeElement )
      {
         MR_ObjectFromFactory::SerializePtr( pArchive, (MR_ObjectFromFactory*&)lFreeElement->mElement );

         lFreeElement->mElement->mPosition.Serialize( pArchive );
         pArchive << lFreeElement->mElement->mOrientation;

         lFreeElement = lFreeElement->mNext;
      }

      MR_ObjectFromFactory* lNullPtr = NULL;

      MR_ObjectFromFactory::SerializePtr( pArchive, lNullPtr );

   }
   else
   {
      ASSERT( *pListHead == NULL );

      MR_FreeElement* lCurrentElement;

      do
      {
         MR_ObjectFromFactory::SerializePtr( pArchive, (MR_ObjectFromFactory*&)lCurrentElement );

         if( lCurrentElement != NULL )
         {
            FreeElement* lFreeElement = new FreeElement;

            lCurrentElement->mPosition.Serialize( pArchive );
            pArchive >> lCurrentElement->mOrientation;

            lFreeElement->mElement = lCurrentElement;
            lFreeElement->LinkTo( pListHead );
         }
      
      }while( lCurrentElement != NULL );
   }
}
 

// class MR_SectionId
void MR_SectionId::Serialize(  CArchive& pArchive )
{
   if( pArchive.IsStoring() )
   {
      pArchive << (int)mType;
      pArchive << mId;
   }
   else
   {
      int lType;

      pArchive >> lType;
      pArchive >> mId;

      mType = (eSectionType)lType;
   }
}


