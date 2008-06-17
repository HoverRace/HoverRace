// FreeElementMovingInterface.cpp
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

#include "StdAfx.h"

#include "FreeElementMovingHelper.h"
#include "../Util/FastArray.h"



// MR_ObstacleCollisionReport

BOOL     MR_ObstacleCollisionReport::IsInMaze()const
{
   return mInMaze;
}

BOOL     MR_ObstacleCollisionReport::HaveContact()const
{
   ASSERT( mInMaze );

   return mHaveObstacleContact || (mClosestFloor < 0) || (mClosestCeiling < 0);
}



MR_Int32 MR_ObstacleCollisionReport::StepHeight()const
{
   MR_Int32 lReturnValue = -mClosestFloor;

   ASSERT( mInMaze );

   if( mHaveObstacleContact )
   {
      lReturnValue = max( lReturnValue, mObstacleTop-mShapeBottom );
   }
   
   return lReturnValue;
}

MR_Int32 MR_ObstacleCollisionReport::CeilingStepHeight()const
{
   MR_Int32 lReturnValue = -mClosestCeiling;

   ASSERT( mInMaze );

   if( mHaveObstacleContact )
   {
      lReturnValue = max( lReturnValue, mShapeTop-mObstacleBottom );
   }   
   return lReturnValue;
}

MR_Int32 MR_ObstacleCollisionReport::SpaceToFloor()const
{
   ASSERT( mInMaze );

   return mClosestFloor;
}

MR_Int32 MR_ObstacleCollisionReport::SpaceToCeiling()const
{
   ASSERT( mInMaze );

   return mClosestCeiling;
}

MR_Int32 MR_ObstacleCollisionReport::LargestHoleHeight()const
{
   return (mShapeTop-mShapeBottom)-(CeilingStepHeight()+StepHeight());
}

MR_Int32 MR_ObstacleCollisionReport::LargestHoleStep()const
{
   return StepHeight();
}

BOOL MR_ObstacleCollisionReport::AlmostCompleted()const
{
   return( mHaveObstacleContact &&( mObstacleTop >= mShapeTop )&&( mObstacleBottom <= mShapeBottom ) );
}





void MR_ObstacleCollisionReport::GetContactWithObstacles( MR_Level*                   pLevel,
                                                          const MR_ShapeInterface*    pShape,
                                                          int                         pRoom,
                                                          MR_FreeElement*             pElement,
                                                          BOOL                        pIgnoreActors )
{
   // Compute the touched rooms and the floor limits
   // Fort each room check collisions with the features
   // For each actor in the rooms, check the collisions with 
   // there part list

   // I think that it is a really intensive calculation
   // Hope it will be fast enough X

   // First test the current room


   // First find aroom touching the shape
  
   mCurrentRoom = pLevel->FindRoomForPoint( MR_2DCoordinate( pShape->XPos(), pShape->YPos() ), pRoom );

   if( mCurrentRoom == -1 )
   {
      mInMaze = FALSE;
   }      
   else
   {
      MR_RoomContactSpec lSpec;
      pLevel->GetRoomContact( mCurrentRoom, pShape, lSpec );

      ASSERT( lSpec.mTouchingRoom );


      mInMaze = TRUE;
      mHaveObstacleContact = FALSE;

      mShapeTop    = pShape->ZMax();
      mShapeBottom = pShape->ZMin();

      mClosestFloor        = lSpec.mDistanceFromFloor;
      mClosestCeiling      = lSpec.mDistanceFromCeiling;

      // Verify if the feature and actors of this room have contacts 
      // with the shape

      GetContactWithFeaturesAndActors( pLevel,
                                       pShape,
                                       mCurrentRoom,
                                       pElement,
                                       pIgnoreActors );


      // For each of the touched walls, 
      // Just verify immediate rooms for the moment
      for( int lCounter = 0; lCounter < lSpec.mNbWallContact; lCounter++ )
      {
         int lNextRoom = pLevel->GetNeighbor( mCurrentRoom, lSpec.mWallContact[ lCounter ] );

         // If that wall have no neighbor, adjust the step height
         if( lNextRoom == -1 )
         {
            mHaveObstacleContact = TRUE;
            mObstacleTop         = mShapeTop;
            mObstacleBottom      = mShapeBottom;
         }
         else
         {
            MR_RoomContactSpec lNextSpec;

            pLevel->GetRoomContact( lNextRoom, pShape, lNextSpec );

            if( lNextSpec.mTouchingRoom )
            {
               mClosestFloor        = min( mClosestFloor,   lNextSpec.mDistanceFromFloor );
               mClosestCeiling      = min( mClosestCeiling, lNextSpec.mDistanceFromCeiling );

               GetContactWithFeaturesAndActors( pLevel,
                                                pShape,
                                                lNextRoom,
                                                pElement, 
                                                pIgnoreActors );


            }

         }

      }
   }
}

int MR_ObstacleCollisionReport::Room()const
{
   return mCurrentRoom;
}

                                                                                                             
void MR_ObstacleCollisionReport::GetContactWithFeaturesAndActors( MR_Level*                   pLevel,
                                                                  const MR_ShapeInterface*    pShape,
                                                                  int                         pRoom,
                                                                  MR_FreeElement*             pElement,
                                                                  BOOL                        pIgnoreActors          )
{

   int            lCounter;
   MR_ContactSpec lSpec;

   if( !AlmostCompleted() )
   {
      // First verify the features
      for( lCounter = 0; lCounter < pLevel->GetFeatureCount( pRoom ); lCounter++ )
      {
         if( pLevel->GetFeatureContact( pLevel->GetFeature( pRoom, lCounter ),
                                        pShape,
                                        lSpec ) )
         {
            if( mHaveObstacleContact )
            {
               mObstacleBottom      = min( mObstacleBottom, lSpec.mZMin );
               mObstacleTop         = max( mObstacleTop,    lSpec.mZMax );
            }
            else
            {
               mHaveObstacleContact = TRUE;
               mObstacleBottom      = lSpec.mZMin;
               mObstacleTop         = lSpec.mZMax;
            }

            if( AlmostCompleted() )
            {
               break;
            }
         }
      }

      
      // Verify the actors
      if( !AlmostCompleted() && !pIgnoreActors )
      {
         MR_FreeElementHandle  lObstacleHandle = pLevel->GetFirstFreeElement( pRoom );

         while( lObstacleHandle != NULL )
         {            
            MR_FreeElement* lObstacle = MR_Level::GetFreeElement( lObstacleHandle );


            if( lObstacle != pElement  )
            {

               const MR_ShapeInterface* lObstacleShape = lObstacle->GetObstacleShape();

               if( lObstacleShape != NULL )
               {
                  if( MR_DetectActorContact( pShape, lObstacleShape, lSpec ) )
                  {
                     if( mHaveObstacleContact )
                     {
                         mObstacleBottom      = min( mObstacleBottom, lSpec.mZMin );
                         mObstacleTop         = max( mObstacleTop,    lSpec.mZMax );
                     }
                     else
                     {
                         mHaveObstacleContact = TRUE;
                         mObstacleBottom      = lSpec.mZMin;
                         mObstacleTop         = lSpec.mZMax;
                     }

                     if( AlmostCompleted() )
                     {
                        break;
                     }
                  }
               }
            }
            lObstacleHandle = MR_Level::GetNextFreeElement( lObstacleHandle );
         }
      }
   }
}



/*
                                                                        
void MR_FreeElementMovingInterface::AbsoluteMove( const MR_3DCoordinate& pNewPosition,     
                                                  MR_Angle               pNewAngle     )
{
   // Find the section containing the new position
   //

   int pNewSection = mLevel->FindRoomForPoint( pNewPosition, mSection );

   if( pNewSection != -1 )
   {
      mLevel->MoveElement( mSection,
                           mElement,
                           pNewSection,
                           pNewPosition,
                           pNewAngle       );
   }

}

 
void MR_FreeElementMovingInterface::RelativeMove( const MR_3DCoordinate& pOffset, MR_Angle pRotation )
{
   // Compute new location and orientation
   MR_3DCoordinate lNewPosition;
   MR_Angle        lNewAngle;

   ComputePosition( pOffset, 
                    pRotation,
                    lNewPosition, 
                    lNewAngle     );



   AbsoluteMove( lNewPosition, lNewAngle );

}

void MR_FreeElementMovingInterface::ComputePosition( const MR_3DCoordinate& pOffset, 
                                                     MR_Angle               pRotation,
                                                     MR_3DCoordinate&       pNewPos, 
                                                     MR_Angle&              pNewOrientation )const
{
   // Compute new location and orientation
   MR_3DCoordinate lNewPosition;
   MR_Angle        lNewAngle;
   MR_Angle        lNewAngleP90;
   MR_FreeElement* lElement = mLevel->GetFreeElement( mLevel->GetFreeElementHandle( mSection, mElement ) ); 
   MR_3DCoordinate lOldPosition = lElement->mPosition;
   MR_Angle        lOldAngle    = lElement->mOrientation;
   
   lNewAngle    = MR_NORMALIZE_ANGLE( lOldAngle+pRotation );
   lNewAngleP90 = MR_NORMALIZE_ANGLE( lNewAngle+MR_PI/2 );

   lNewPosition.mZ = lOldPosition.mZ+pOffset.mZ;

   lNewPosition.mX = (pOffset.mX*MR_Cos[ lNewAngle    ]+pOffset.mY*MR_Cos[ lNewAngleP90 ])/MR_TRIGO_FRACT;
   lNewPosition.mY = (pOffset.mY*MR_Sin[ lNewAngleP90 ]+pOffset.mX*MR_Sin[ lNewAngle ])/MR_TRIGO_FRACT;

   lNewPosition.mX += lOldPosition.mX;
   lNewPosition.mY += lOldPosition.mY;

   pNewPos         = lNewPosition;
   pNewOrientation = lNewAngle;

}
*/
