// TrackMap.cpp
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

#include "TrackMap.h"



// Local prototypes


// Implementation

// class MR_MapSprite

BOOL MR_MapSprite::CreateMap( MR_Level* pLevel, int& pX0, int& pY0, int& pX1, int& pY1 )
{

   mNbItem     = 1;
   mWidth      = 200;
   mItemHeight = 200;
   mTotalHeight = mItemHeight;

   mData = new MR_UInt8[mTotalHeight*mWidth ];

   ComputeMinMax( pLevel );

   DrawMap( pLevel );

   pX0 = mXMin;
   pX1 = mXMax;
   pY0 = mYMin;
   pY1 = mYMax;

   return TRUE;
}

void MR_MapSprite::ComputeMinMax( MR_Level* pLevel )
{
   int lRoomCount = pLevel->GetRoomCount();
   
   mXMin = 1000000;
   mXMax = -1000000;
   mYMin = 1000000;
   mYMax = -1000000;

   for( int lRoom = 0; lRoom<lRoomCount; lRoom++ )
   {
      MR_PolygonShape* lShape = pLevel->GetRoomShape( lRoom );

      mXMin = min( mXMin, lShape->XMin() );
      mXMax = max( mXMax, lShape->XMax() );
      mYMin = min( mYMin, lShape->YMin() );
      mYMax = max( mYMax, lShape->YMax() );
      delete lShape;
   }

   mXMax += 1;
   mYMax += 1;

   // Now adjust Vertiacl and Horizontal ratio
   int lWidth  = mXMax-mXMin;
   int lHeight = mYMax-mYMin;
   int lDiff   = lWidth-lHeight;

   if( lDiff >= 0 )
   {
      mYMax += lDiff/2;
      mYMin -= lDiff/2;
   }
   else
   {
      mXMax += -lDiff/2;
      mXMin -= -lDiff/2;
   }
}

void MR_MapSprite::DrawMap( MR_Level* pLevel )
{
   // Clr the map
   memset( mData, 0, mTotalHeight*mWidth );

   // Do a kind of ray tracing
   int lRoomCount = pLevel->GetRoomCount();

   MR_ObjectFromFactoryId lFinishLineId = { 1, 202 };

   for( int lRoom = 0; lRoom<lRoomCount; lRoom++ )
   {
      int lColor = 61;

      // Verify if it is not the FinishLineSection
      MR_FreeElementHandle  lCurrentElem = pLevel->GetFirstFreeElement( lRoom );
      

      while( lCurrentElem != NULL )
      {
         if( MR_Level::GetFreeElement( lCurrentElem )->GetTypeId() == lFinishLineId )
         {
            lColor = 11;
         }

         lCurrentElem = MR_Level::GetNextFreeElement( lCurrentElem );
      }



      MR_PolygonShape* lShape = pLevel->GetRoomShape( lRoom );

      int lXStart = (lShape->XMin()-mXMin)*mWidth/(mXMax-mXMin);
      int lXEnd   = (lShape->XMax()-mXMin)*mWidth/(mXMax-mXMin);
      int lYStart = (lShape->YMin()-mYMin)*mItemHeight/(mYMax-mYMin);
      int lYEnd   = (lShape->YMax()-mYMin)*mItemHeight/(mYMax-mYMin);

      for( int lX = lXStart; lX <=lXEnd; lX++ )
      {
         for( int lY = lYStart; lY<=lYEnd; lY++ )
         {
            MR_2DCoordinate lPos;

            lPos.mX = (lX*(mXMax-mXMin)/mWidth)+mXMin;
            lPos.mY = (lY*(mYMax-mYMin)/mItemHeight)+mYMin;

            if( MR_GetPolygonInclusion( *lShape, lPos ) )
            {
               mData[ (mItemHeight-1-lY)*mWidth+lX ] = lColor;
            }
         }
      }
      delete lShape;
   }
}

