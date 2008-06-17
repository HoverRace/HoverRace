// LevelBuilder.cpp
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
#include "LevelBuilder.h"
#include "Parser.h"
#include "../Util/StrRes.h"
#include "resource.h"

#include <math.h>

#define new DEBUG_NEW

// Helper structurtes
class MR_Connection
{
   public:
      int mRoom0;
      int mWall0;
      int mRoom1;
      int mWall1;
};

// Helper variables
static MR_LevelBuilder* gsCurrentLevelBuilder = NULL;

// Local helper functions
static MR_SurfaceElement* sLoadTexture( MR_Parser* pParser );


// class MR_LevelBuilder
BOOL MR_LevelBuilder::InitFromFile( FILE* pFile )
{
   BOOL lReturnValue = TRUE;

   if( lReturnValue )
   {
      lReturnValue = Parse( pFile );
   }

   if( lReturnValue )
   {
      lReturnValue = ComputeVisibleZones();
   }

   if( lReturnValue )
   {
      lReturnValue = ComputeAudibleZones();
   }

   if( lReturnValue )
   {
      OrderVisibleSurfaces();
   }

   return lReturnValue;
}

      

BOOL MR_LevelBuilder::Parse( FILE* pFile )
{
   BOOL        lReturnValue = TRUE;
   int         lCounter;
   MR_Parser   lParser( pFile );
      
   CMap< int, int, int, int > lRoomList;
   CMap< int, int, int, int > lFeatureList;

   // First get the Room and feature count
   mNbRoom    = 0;
   mNbFeature = 0;

   while( lReturnValue && (lParser.GetNextClass( "Room" ) != NULL) )
   {
      if( lParser.GetNextAttrib( "Id" ) == NULL )
      {
         lReturnValue = FALSE;

         printf( MR_LoadString( IDS_MISS_SEC_ID) , lParser.GetErrorLine() );
      }
      else
      {
         int lDummy;
         int lRoomId = (int)lParser.GetNextNumParam();

         if( lRoomList.Lookup( lRoomId, lDummy ) )
         {
            lReturnValue = FALSE;
            printf( MR_LoadString( IDS_DUP_SEC_ID ), lRoomId, lParser.GetErrorLine() );
         }
         else
         {
            lRoomList.SetAt( lRoomId, lRoomList.GetCount() );
         }
      }
   }

   lParser.Reset();

   while( lReturnValue && (lParser.GetNextClass( "Feature" ) != NULL) )
   {
      if( lParser.GetNextAttrib( "Id" ) == NULL )
      {
         lReturnValue = FALSE;
         printf( MR_LoadString( IDS_MISS_SEC_ID ), lParser.GetErrorLine() );
      }
      else
      {
         int lDummy;
         int lFeatureId = (int)lParser.GetNextNumParam();

         if( lFeatureList.Lookup( lFeatureId, lDummy ) )
         {
            lReturnValue = FALSE;
            printf( MR_LoadString( IDS_DUP_SEC_ID ), lFeatureId, lParser.GetErrorLine() );
         }
         else
         {
            lFeatureList.SetAt( lFeatureId, lFeatureList.GetCount() );
         }
      }
   }



   if( lReturnValue )
   {
      // Create the room and feature lists
      mNbRoom     = lRoomList.GetCount();
      mRoomList   = new Room[ mNbRoom ];

      mNbFeature   = lFeatureList.GetCount();
      mFeatureList = new Feature[ mNbFeature ];

      mFreeElementClassifiedByRoomList = new FreeElement*[ mNbRoom ];

      for( lCounter=0; lCounter<mNbRoom; lCounter++ )
      {
         mFreeElementClassifiedByRoomList[ lCounter ] = NULL;
      }

   }

   // Get the vertex count of each room and feature and allocate the related memory
   lParser.Reset();
   while( lReturnValue && (lParser.GetNextClass( "Room" ) != NULL) )
   {
      if( lParser.GetNextAttrib( "Id" ) == NULL )
      {
         ASSERT( FALSE ); // Already verified            
      }
      else
      {
         int lRoomIndex;
         int lRoomId = (int)lParser.GetNextNumParam();

         // Load the attributes

         if( !lRoomList.Lookup( lRoomId, lRoomIndex ) )
         {
            ASSERT( FALSE );
         }
         else
         {
            const char* lAttrib;

            // Count the number of vertex and find the parent section
            ASSERT( mRoomList[ lRoomIndex ].mNbVertex == 0 );

            while( (lAttrib=lParser.GetNextAttrib()) != NULL )
            {
               if( !strcmpi( lAttrib, "Wall" ) )
               {
                  mRoomList[ lRoomIndex ].mNbVertex++;
               }
               else if( !strcmpi( lAttrib, "Floor" ) )
               {
                  // Load floor attributes
                  mRoomList[ lRoomIndex ].mFloorLevel = (MR_Int32)(lParser.GetNextNumParam()*1000.0);

                  // Create the associated texture               
                  mRoomList[ lRoomIndex ].mFloorTexture = sLoadTexture( &lParser );

                  if( mRoomList[ lRoomIndex ].mFloorTexture == NULL )
                  {
                     lReturnValue = FALSE;
                  }
               }
               else if( !strcmpi( lAttrib, "Ceiling" ) )
               {
                  // Load floor attributes
                  mRoomList[ lRoomIndex ].mCeilingLevel = (MR_Int32)(lParser.GetNextNumParam()*1000.0);

                  // Create the associated texture               
                  mRoomList[ lRoomIndex ].mCeilingTexture = sLoadTexture( &lParser );

                  if( mRoomList[ lRoomIndex ].mCeilingTexture == NULL )
                  {
                     lReturnValue = FALSE;
                  }
               }
            }
         }
      }
   }

   

   lParser.Reset();
   while( lReturnValue && (lParser.GetNextClass( "Feature" ) != NULL) )
   {
      if( lParser.GetNextAttrib( "Id" ) == NULL )
      {
         ASSERT( FALSE ); // Already verified            
      }
      else
      {
         int lFeatureIndex;
         int lFeatureId = (int)lParser.GetNextNumParam();

         // Load the attributes

         if( !lFeatureList.Lookup( lFeatureId, lFeatureIndex ) )
         {
            ASSERT( FALSE );
         }
         else
         {
            const char* lAttrib;

            // Count the number of vertex and find the parent section
            ASSERT( mFeatureList[ lFeatureIndex ].mNbVertex == 0 );
            
            while( (lAttrib=lParser.GetNextAttrib()) != NULL )
            {
               if( !strcmpi( lAttrib, "Wall" ) )
               {
                  mFeatureList[ lFeatureIndex ].mNbVertex++;
               }
               else if( !strcmpi( lAttrib, "Floor" ) )
               {
                  // Load floor attributes
                  mFeatureList[ lFeatureIndex ].mFloorLevel = (MR_Int32)(lParser.GetNextNumParam()*1000.0);

                  // Create the associated texture               
                  mFeatureList[ lFeatureIndex ].mFloorTexture = sLoadTexture( &lParser );

                  if( mFeatureList[ lFeatureIndex ].mFloorTexture == NULL )
                  {
                     lReturnValue = FALSE;
                  }
               }
               else if( !strcmpi( lAttrib, "Ceiling" ) )
               {
                  // Load floor attributes
                  mFeatureList[ lFeatureIndex ].mCeilingLevel = (MR_Int32)(lParser.GetNextNumParam()*1000.0);

                  // Create the associated texture               
                  mFeatureList[ lFeatureIndex ].mCeilingTexture = sLoadTexture( &lParser );

                  if( mFeatureList[ lFeatureIndex ].mCeilingTexture == NULL )
                  {
                     lReturnValue = FALSE;
                  }
               }
               else if( !strcmpi( lAttrib, "Parent" ) )
               {                  
                  int lParentId =  (int)lParser.GetNextNumParam();

                  if( !lRoomList.Lookup( lParentId, mFeatureList[ lFeatureIndex ].mParentSectionIndex ) )
                  {
                     lReturnValue = FALSE;
                     printf( "Invalid parent ref on line %d\n", lParser.GetErrorLine() );
                  }
                  else
                  {
                     mRoomList[ mFeatureList[ lFeatureIndex ].mParentSectionIndex ].mNbChild++;
                  }
               }
            }
         }
      }
   }

   if( lReturnValue )
   {
     
      // Create the arrays that can be created
      for( lCounter = 0; lCounter < mNbRoom; lCounter++ )
      {
         mRoomList[ lCounter ].mVertexList   = new MR_2DCoordinate[ mRoomList[ lCounter ].mNbVertex];
         mRoomList[ lCounter ].mWallLen      = new MR_Int32[ mRoomList[ lCounter ].mNbVertex];

         mRoomList[ lCounter ].mNeighborList = new int[ mRoomList[ lCounter ].mNbVertex];

         // Init neighbor list
         for( int lNeighbor = 0; lNeighbor < mRoomList[ lCounter ].mNbVertex; lNeighbor++ )
         {
            mRoomList[ lCounter ].mNeighborList[ lNeighbor ] = -1;
         }
         mRoomList[ lCounter ].mWallTexture  = new MR_SurfaceElement*[ mRoomList[ lCounter ].mNbVertex];

         mRoomList[ lCounter ].mChildList    = new int[ mRoomList[ lCounter ].mNbChild];

         // Used the array size as a pointer
         mRoomList[ lCounter ].mNbChild = 0;

      }
     
      for( lCounter = 0; lCounter < mNbFeature; lCounter++ )
      {
         mFeatureList[ lCounter ].mVertexList   = new MR_2DCoordinate[ mFeatureList[ lCounter ].mNbVertex];
         mFeatureList[ lCounter ].mWallLen      = new MR_Int32[ mFeatureList[ lCounter ].mNbVertex];

         mFeatureList[ lCounter ].mWallTexture  = new MR_SurfaceElement*[ mFeatureList[ lCounter ].mNbVertex];

         // Create the Parent->Child relations
         int lParentIndex = mFeatureList[ lCounter ].mParentSectionIndex;
         mRoomList[ lParentIndex ].mChildList[ mRoomList[ lParentIndex ].mNbChild++ ] = lCounter;

      }
   }

   // Load the walls related stuff
   lParser.Reset();
   while( lReturnValue && (lParser.GetNextClass( "Room" ) != NULL) )
   {
      if( lParser.GetNextAttrib( "Id" ) == NULL )
      {
         ASSERT( FALSE ); // Already verified            
      }
      else
      {
         int lRoomIndex;
         int lRoomId = (int)lParser.GetNextNumParam();

         // Load the attributes

         if( !lRoomList.Lookup( lRoomId, lRoomIndex ) )
         {
            ASSERT( FALSE );
         }
         else
         {
            int lVertex = 0;

            while( lParser.GetNextAttrib( "Wall" ) != NULL )
            {
               ASSERT( lVertex < mRoomList[ lRoomIndex ].mNbVertex );
               
               // Load the associated position (and convert it from meters to milimeters)
               mRoomList[ lRoomIndex ].mVertexList[ lVertex ].mX = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
               mRoomList[ lRoomIndex ].mVertexList[ lVertex ].mY = (MR_Int32)(lParser.GetNextNumParam()*1000.0);

               // Create the associated texture               
               mRoomList[ lRoomIndex ].mWallTexture[ lVertex ] = sLoadTexture( &lParser );

               if( mRoomList[ lRoomIndex ].mWallTexture[ lVertex ] == NULL )
               {
                  lReturnValue = FALSE;
               }
               lVertex++;
            }
            ASSERT( lVertex >= 3 );
         }
      }
   }


   lParser.Reset();
   while( lReturnValue && (lParser.GetNextClass( "Feature" ) != NULL) )
   {
      if( lParser.GetNextAttrib( "Id" ) == NULL )
      {
         ASSERT( FALSE ); // Already verified            
      }
      else
      {
         int lFeatureIndex;
         int lFeatureId = (int)lParser.GetNextNumParam();

         // Load the attributes

         if( !lFeatureList.Lookup( lFeatureId, lFeatureIndex ) )
         {
            ASSERT( FALSE );
         }
         else
         {
            int lVertex = 0;

            while( lParser.GetNextAttrib( "Wall" ) != NULL )
            {
               ASSERT( lVertex < mFeatureList[ lFeatureIndex ].mNbVertex );
               
               // Load the associated position (and convert it from meters to milimeters)
               mFeatureList[ lFeatureIndex ].mVertexList[ lVertex ].mX = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
               mFeatureList[ lFeatureIndex ].mVertexList[ lVertex ].mY = (MR_Int32)(lParser.GetNextNumParam()*1000.0);

               // Create the associated texture               
               mFeatureList[ lFeatureIndex ].mWallTexture[ lVertex ] = sLoadTexture( &lParser );

               if( mFeatureList[ lFeatureIndex ].mWallTexture[ lVertex ] == NULL )
               {
                  lReturnValue = FALSE;
               }

               lVertex++;
            }

            ASSERT( lVertex >= 3 );
         }
      }
   }



   // Load the connections
   CList< MR_Connection, MR_Connection& > lConnectionList;

   lParser.Reset();

   if( lParser.GetNextClass( "Connection_List" ) == NULL )
   {
      lReturnValue = FALSE;
      printf( MR_LoadString( IDS_MISS_CONNECT_LIST ) );
   }
   else
   {
      while( lParser.GetNextLine() )
      {
         MR_Connection lNewConnection;

         lNewConnection.mRoom0 = (int)lParser.GetNextNumParam();
         lNewConnection.mWall0 = (int)lParser.GetNextNumParam();
         lNewConnection.mRoom1 = (int)lParser.GetNextNumParam();
         lNewConnection.mWall1 = (int)lParser.GetNextNumParam();

         lConnectionList.AddTail( lNewConnection );
      }
   

      // Assign the connections to the MR_Level
      POSITION lPos = lConnectionList.GetHeadPosition();

      while( lPos != NULL )
      {
         MR_Connection& lConnection = lConnectionList.GetNext( lPos );

         int lRoom0;
         int lRoom1;

         if( !lRoomList.Lookup( lConnection.mRoom0, lRoom0 ) )
         {
            // lReturnValue = FALSE;
            printf( MR_LoadString( IDS_BAD_CONNECT1 ), lConnection.mRoom0 );
         }
         else if( !lRoomList.Lookup( lConnection.mRoom1, lRoom1 ) )
         {
            // lReturnValue = FALSE;
            printf( MR_LoadString( IDS_BAD_CONNECT1 ), lConnection.mRoom1 );
         }
         else if( (mRoomList[ lRoom0 ].mVertexList[ lConnection.mWall0 ] !=
                   mRoomList[ lRoom1 ].mVertexList[ (lConnection.mWall1+1)%mRoomList[lRoom1].mNbVertex ] ) ||
                  (mRoomList[ lRoom1 ].mVertexList[ lConnection.mWall1 ] !=
                   mRoomList[ lRoom0 ].mVertexList[ (lConnection.mWall0+1)%mRoomList[lRoom0].mNbVertex ] )  )
         {
            // lReturnValue = FALSE;
            printf( MR_LoadString( IDS_BAD_CONNECT2 ) , lConnection.mRoom0, lConnection.mRoom1 );
            printf( "%3dA: %5d, %5d\n", lConnection.mRoom0, 
                                        mRoomList[ lRoom0 ].mVertexList[ lConnection.mWall0 ].mX,
                                        mRoomList[ lRoom0 ].mVertexList[ lConnection.mWall0 ].mY
                                        );
            printf( "%3dB: %5d, %5d\n", lConnection.mRoom0, 
                                        mRoomList[ lRoom0 ].mVertexList[ (lConnection.mWall0+1)%mRoomList[lRoom0].mNbVertex ].mX,
                                        mRoomList[ lRoom0 ].mVertexList[ (lConnection.mWall0+1)%mRoomList[lRoom0].mNbVertex ].mY
                                        );

            printf( "%3dA: %5d, %5d\n", lConnection.mRoom1, 
                                        mRoomList[ lRoom1 ].mVertexList[ lConnection.mWall1 ].mX,
                                        mRoomList[ lRoom1 ].mVertexList[ lConnection.mWall1 ].mY
                                        );
            printf( "%3dB: %5d, %5d\n", lConnection.mRoom1, 
                                        mRoomList[ lRoom1 ].mVertexList[ (lConnection.mWall1+1)%mRoomList[lRoom1].mNbVertex ].mX,
                                        mRoomList[ lRoom1 ].mVertexList[ (lConnection.mWall1+1)%mRoomList[lRoom1].mNbVertex ].mY
                                        );
         }
         else
         {
            // We can now do the connection
            mRoomList[ lRoom0 ].mNeighborList[ lConnection.mWall0 ] = lRoom1;
            mRoomList[ lRoom1 ].mNeighborList[ lConnection.mWall1 ] = lRoom0;
         }
      }
   }

   // Compute the bounding box of each Room and do some validation
   if( lReturnValue )
   {
      POSITION lPos = lRoomList.GetStartPosition();

      while( lPos != NULL )
      {
         int lRoomId;
         int lRoomIndex;

         lRoomList.GetNextAssoc( lPos, lRoomId, lRoomIndex );

         double lDiagSize = ComputeShapeConst( &(mRoomList[lRoomIndex]) );


         if( lDiagSize < 10.0 )
         {
            printf( MR_LoadString( IDS_SMALL_ROOM ), lRoomId );
         }

         if( lDiagSize > 100.0 )
         {
            printf( MR_LoadString( IDS_LARGE_ROOM ), lRoomId );
         }
      }
   }  

   if( lReturnValue )
   {
      POSITION lPos = lFeatureList.GetStartPosition();

      while( lPos != NULL )
      {
         int lFeatureId;
         int lFeatureIndex;

         lFeatureList.GetNextAssoc( lPos, lFeatureId, lFeatureIndex );

         double lDiagSize = ComputeShapeConst( &(mFeatureList[lFeatureIndex]) );

         if( lDiagSize < 1.0 )
         {
            printf( MR_LoadString( IDS_SMALL_FEATURE ), lFeatureId );
         }

         if( lDiagSize > 25.0 )
         {
            printf( MR_LoadString( IDS_LARGE_FEATURE ), lFeatureId );
         }
      }
   }  



   // Load the player's starting positions
   if( lReturnValue )
   {
      int lNbStartingPosition = 0;

      lParser.Reset();
      
      while( lReturnValue && (lParser.GetNextClass( "Initial_Position" ) != NULL) )
      {
         const char* lAttrib;

         while( (lAttrib = lParser.GetNextAttrib()) != NULL )
         {
            if( !stricmp( lAttrib, "Section" ) )
            {
               int lRoomId = (int)lParser.GetNextNumParam();

               if( !lRoomList.Lookup( lRoomId, mStartingRoom[ lNbStartingPosition ] ) )
               {
                  lReturnValue = FALSE;
                  printf( MR_LoadString( IDS_BAD_STARTPOS ), lRoomId );
               }
            }
            else if( !stricmp( lAttrib, "Position" ) )
            {
               mStartingPosition[ lNbStartingPosition ].mX = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
               mStartingPosition[ lNbStartingPosition ].mY = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
               mStartingPosition[ lNbStartingPosition ].mZ = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
            }
            else if( !stricmp( lAttrib, "Orientation" ) )
            {
               mStartingOrientation[ lNbStartingPosition ] = (MR_Angle)(lParser.GetNextNumParam()*MR_2PI/360.0);
            }
            else if( !stricmp( lAttrib, "Team" ) )
            {
               mPlayerTeam[ lNbStartingPosition ] = (int)lParser.GetNextNumParam();
            }

            // Do some validation

            // TODO
         }
         lNbStartingPosition++;
      }

      if( lReturnValue )
      {
         if( lNbStartingPosition == 0 )
         {
            lReturnValue = FALSE;
            printf( MR_LoadString( IDS_NO_STARTPOS ) );
         }
      }
      mNbPlayer = lNbStartingPosition;
   }


   // Load the mobile elements
   lParser.Reset();
   int lFreeElementCount = 0;

   while( lReturnValue && (lParser.GetNextClass( "Free_Element" ) != NULL) )
   {
      int                    lRoomIndex;
      MR_3DCoordinate        lPosition;
      MR_Angle               lOrientation;
      MR_ObjectFromFactoryId lElementType;

      lFreeElementCount++;

      // Load the attributes

      const char* lAttrib;

      while( (lAttrib=lParser.GetNextAttrib()) != NULL )
      {
         if( !stricmp( lAttrib, "Section" ) )
         {
            int lRoomId = (int)lParser.GetNextNumParam();

            if( !lRoomList.Lookup( lRoomId, lRoomIndex ) )
            {
               lReturnValue = FALSE;
               printf( MR_LoadString( IDS_INVALID_INDEX) , lParser.GetErrorLine() );
            }
         }
         else if( !stricmp( lAttrib, "Position" ) )
         {
            lPosition.mX = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
            lPosition.mY = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
            lPosition.mZ = (MR_Int32)(lParser.GetNextNumParam()*1000.0);
         }
         else if( !stricmp( lAttrib, "Orientation" ) )
         {
            lOrientation = (MR_Angle)(lParser.GetNextNumParam()*MR_2PI/360.0);
         }
         else if( !stricmp( lAttrib, "Element_Type" ) )
         {
            lElementType.mDllId   = (MR_UInt16)lParser.GetNextNumParam();
            lElementType.mClassId = (MR_UInt16)lParser.GetNextNumParam();
         }
      }


      if( lReturnValue )
      {
         
         MR_FreeElement* lElement = (MR_FreeElement*) MR_DllObjectFactory::CreateObject( lElementType );
            
         if( lElement == NULL )
         {
            lReturnValue = FALSE;
            printf( "Unable to create free element on line %d\n", lParser.GetErrorLine() );
         }
         else
         {
            // Initialise element position
            lElement->mPosition    = lPosition;
            lElement->mOrientation = lOrientation;

            InsertElement( lElement, lRoomIndex );
           

            // If needed, pass the initialisation string to the element
            // TODO
         }
      }
   }
 

   // Print some statistics
   if( lReturnValue )
   {
      printf
      ( MR_LoadString( IDS_RESUME ),
         lRoomList.GetCount(),
         lFeatureList.GetCount(),
         lConnectionList.GetCount(),
         lFreeElementCount,
         mNbPlayer
      );
   }

   return lReturnValue;
}


BOOL MR_LevelBuilder::ComputeAudibleZones()
{
   BOOL lReturnValue = TRUE;

   return lReturnValue;
}

void MR_LevelBuilder::OrderVisibleSurfaces()
{
   int lCounter;

   // For each room, compute the list of the visible surfaces
   for( int lRoomId = 0; lRoomId < mNbRoom; lRoomId++ )
   {
      mRoomList[ lRoomId ].mNbVisibleSurface = 0;


      // Compute the number of visible surfaces

      mRoomList[ lRoomId ].mNbVisibleSurface = mRoomList[ lRoomId ].mNbChild + 1;

      for( lCounter = 0; lCounter< mRoomList[ lRoomId ].mNbVisibleRoom; lCounter++ )
      {
         mRoomList[ lRoomId ].mNbVisibleSurface += mRoomList[ mRoomList[ lRoomId ].mVisibleRoomList[ lCounter ] ].mNbChild + 1;
      }

      // Create the arrays containing the list of visible floor and ceiling
      int lCurrentIndex = 0;

      mRoomList[ lRoomId ].mVisibleFloorList   = new MR_SectionId[ mRoomList[ lRoomId ].mNbVisibleSurface ];
      mRoomList[ lRoomId ].mVisibleCeilingList = new MR_SectionId[ mRoomList[ lRoomId ].mNbVisibleSurface ];
         

      for( lCounter = -1; lCounter < mRoomList[ lRoomId ].mNbVisibleRoom; lCounter++ )
      {
         int lVisibleRoom;

         if( lCounter == -1 )
         {
            lVisibleRoom = lRoomId;
         }
         else
         {
            lVisibleRoom = mRoomList[ lRoomId ].mVisibleRoomList[ lCounter ];
         }

         mRoomList[ lRoomId ].mVisibleFloorList[   lCurrentIndex   ].mType = MR_SectionId::eRoom;
         mRoomList[ lRoomId ].mVisibleFloorList[   lCurrentIndex   ].mId   = lVisibleRoom;
         mRoomList[ lRoomId ].mVisibleCeilingList[ lCurrentIndex   ].mType = MR_SectionId::eRoom;
         mRoomList[ lRoomId ].mVisibleCeilingList[ lCurrentIndex++ ].mId   = lVisibleRoom;

         for( int lChildIndex = 0; lChildIndex < mRoomList[ lVisibleRoom ].mNbChild; lChildIndex++ )
         {               
            mRoomList[ lRoomId ].mVisibleFloorList[   lCurrentIndex ].mType = MR_SectionId::eFeature;
            mRoomList[ lRoomId ].mVisibleFloorList[   lCurrentIndex ].mId   = mRoomList[ lVisibleRoom ].mChildList[ lChildIndex ];
            mRoomList[ lRoomId ].mVisibleCeilingList[ lCurrentIndex ].mType = MR_SectionId::eFeature;
            mRoomList[ lRoomId ].mVisibleCeilingList[ lCurrentIndex++ ].mId = mRoomList[ lVisibleRoom ].mChildList[ lChildIndex ];
         }            

         ASSERT( lCurrentIndex <= mRoomList[ lRoomId ].mNbVisibleSurface );
      }

       ASSERT( lCurrentIndex == mRoomList[ lRoomId ].mNbVisibleSurface );


       // Order the surfaces list
       gsCurrentLevelBuilder = this;

       qsort( mRoomList[ lRoomId ].mVisibleFloorList,
              mRoomList[ lRoomId ].mNbVisibleSurface,
              sizeof( MR_SectionId ),
              OrderFloor   
            );
  
      qsort( mRoomList[ lRoomId ].mVisibleCeilingList,
             mRoomList[ lRoomId ].mNbVisibleSurface,
             sizeof( MR_SectionId ),
             OrderCeiling
           );


   }
}


int MR_LevelBuilder::OrderFloor( const void* pSurface0, const void* pSurface1 )
{
   int lLevel0;
   int lLevel1;

   MR_SectionId lSurface0 = *(MR_SectionId*)pSurface0;
   MR_SectionId lSurface1 = *(MR_SectionId*)pSurface1;

   if( lSurface0.mType == MR_SectionId::eRoom )
   {
      lLevel0 = gsCurrentLevelBuilder->mRoomList[ lSurface0.mId ].mFloorLevel;
   }
   else
   {
      lLevel0 = gsCurrentLevelBuilder->mFeatureList[ lSurface0.mId ].mCeilingLevel;
   }

   if( lSurface1.mType == MR_SectionId::eRoom )
   {
      lLevel1 = gsCurrentLevelBuilder->mRoomList[ lSurface1.mId ].mFloorLevel;
   }
   else
   {
      lLevel1 = gsCurrentLevelBuilder->mFeatureList[ lSurface1.mId ].mCeilingLevel;
   }

   return( lLevel0-lLevel1 );
}


int MR_LevelBuilder::OrderCeiling( const void* pSurface0, const void* pSurface1 )
{
   int lLevel0;
   int lLevel1;

   MR_SectionId lSurface0 = *(MR_SectionId*)pSurface0;
   MR_SectionId lSurface1 = *(MR_SectionId*)pSurface1;

   if( lSurface0.mType == MR_SectionId::eFeature )
   {
      lLevel0 = gsCurrentLevelBuilder->mFeatureList[ lSurface0.mId ].mFloorLevel;
   }
   else
   {
      lLevel0 = gsCurrentLevelBuilder->mRoomList[ lSurface0.mId ].mCeilingLevel;
   }

   if( lSurface1.mType == MR_SectionId::eFeature )
   {
      lLevel1 = gsCurrentLevelBuilder->mFeatureList[ lSurface1.mId ].mFloorLevel;
   }
   else
   {
      lLevel1 = gsCurrentLevelBuilder->mRoomList[ lSurface1.mId ].mCeilingLevel;
   }

   return( lLevel1-lLevel0 );
}

double MR_LevelBuilder::ComputeShapeConst( Section* pSection )
{
   int    lCounter;
   double lReturnValue;

   pSection->mMin.mX = pSection->mVertexList[0].mX;
   pSection->mMax.mX = pSection->mVertexList[0].mX;
   pSection->mMin.mY = pSection->mVertexList[0].mY;
   pSection->mMax.mY = pSection->mVertexList[0].mY;

   for( lCounter = 1; lCounter < pSection->mNbVertex; lCounter++ )
   {
      if( pSection->mMin.mX > pSection->mVertexList[lCounter].mX )
      {
         pSection->mMin.mX = pSection->mVertexList[lCounter].mX;
      }
      if( pSection->mMax.mX < pSection->mVertexList[lCounter].mX )
      {
         pSection->mMax.mX = pSection->mVertexList[lCounter].mX;
      }

      if( pSection->mMin.mY > pSection->mVertexList[lCounter].mY )
      {
         pSection->mMin.mY = pSection->mVertexList[lCounter].mY;
      }

      if( pSection->mMax.mY < pSection->mVertexList[lCounter].mY )
      {
         pSection->mMax.mY = pSection->mVertexList[lCounter].mY;
      }
   }

   for( lCounter = 0; lCounter < pSection->mNbVertex; lCounter++ )
   {
      int lNext = (lCounter+1)%pSection->mNbVertex;

      double lXLen =   pSection->mVertexList[lNext].mX 
                      - pSection->mVertexList[lCounter].mX;

      double lYLen =   pSection->mVertexList[lNext].mY 
                      - pSection->mVertexList[lCounter].mY;

      pSection->mWallLen[lCounter] = (MR_Int32)sqrt( pow( lXLen, 2 )+ pow( lYLen, 2 ) );
   }


   // Compute bonding box diag size
   lReturnValue = sqrt( pow( pSection->mMax.mX -
                             pSection->mMin.mX, 2  )+
                        pow( pSection->mMax.mY -
                             pSection->mMin.mY, 2  ) )/1000.0;

   return lReturnValue;
}


// Helpers implementation
MR_SurfaceElement* sLoadTexture( MR_Parser* pParser )
{
   MR_SurfaceElement*    lReturnValue = NULL;
   MR_ObjectFromFactory* lTempPtr     = NULL;

   MR_ObjectFromFactoryId lType;

   lType.mDllId   = (MR_UInt16)pParser->GetNextNumParam();
   lType.mClassId = (MR_UInt16)pParser->GetNextNumParam();
               
   lTempPtr =  MR_DllObjectFactory::CreateObject( lType );

   if( lTempPtr != NULL )
   {
      lReturnValue = dynamic_cast<MR_SurfaceElement*>(lTempPtr);

      if( lReturnValue == NULL )
      {
         lTempPtr = NULL;
      }
   }

   if( lReturnValue == NULL )
   {
      printf( "Unable to load texture %d %d on line %d\n", 
              lType.mDllId, 
              lType.mClassId,
              pParser->GetErrorLine() );
   }

   return lReturnValue;
}


