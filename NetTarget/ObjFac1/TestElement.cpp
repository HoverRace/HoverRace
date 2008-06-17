// TestElement.cpp
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

#include "TestElement.h"
#include "../ObjFacTools/ObjectFactoryData.h"
#include "../Model/FreeElementMovingHelper.h"

MR_TestElement::MR_TestElement( const MR_ObjectFromFactoryId& pId, int pActorRes )
               :MR_FreeElementBase( pId )
{
   mElapsedFrameTime = 0;
   mActor = gObjectFactoryData->mResourceLib.GetActor( pActorRes );

   // Init the state of the logic part
   mXSpeed = 0;
   mYSpeed = 0;

   mCollisionShape.mRay = 250;                
   mContactShape.mRay = 300;

   mContactEffect.mWeight = 90;
   mContactEffectList.AddTail( &mContactEffect );
}

MR_TestElement::~MR_TestElement()
{

}


int MR_TestElement::Simulate( MR_SimulationTime pDuration, MR_Level* pLevel, int pRoom )
{
   mElapsedFrameTime += pDuration;

   int lFrameIncrement = mElapsedFrameTime/75;

   if( lFrameIncrement > 0 )
   {
      mElapsedFrameTime %= 75;

      mCurrentFrame += lFrameIncrement; 

      if( mCurrentFrame >= mActor->GetFrameCount( mCurrentSequence ) )
      {
         mCurrentFrame = 0;
         mCurrentSequence++;

         if( mCurrentSequence >= mActor->GetSequenceCount() )
         {
            mCurrentSequence = 0;
         }
      }
   }

   // Now do the displacement

   // Compute translation effect
   MR_3DCoordinate lTranslation( (mXSpeed*(int)pDuration)/256,
                                 (mYSpeed*(int)pDuration)/256,
                                 (-2*256 *(int)pDuration)/256 );


   // Verify if the move is valid
   Cylinder                   lShape;
   MR_3DCoordinate            lNewPos;
   MR_Angle                   lNewOrientation;
   MR_ObstacleCollisionReport lReport;

   lNewPos         = mPosition;
   lNewOrientation = mOrientation;

   lNewPos.mX += lTranslation.mX;
   lNewPos.mY += lTranslation.mY;
   lNewPos.mZ += lTranslation.mZ;

   lShape.mRay = mCollisionShape.mRay;
   lShape.mPosition = lNewPos;

   lReport.GetContactWithObstacles( pLevel, &lShape, pRoom, this );

   if( lReport.IsInMaze() )
   {
      if( !lReport.HaveContact() )
      {
         mPosition    = lNewPos;
         mOrientation = lNewOrientation;
         pRoom        = lReport.Room();
      }
      else
      {
         // Determine if we can go on the object
         if( (lReport.SpaceToCeiling() > 0 )&&
             (lReport.StepHeight()<(100))  )
         {
            lNewPos.mZ += lReport.StepHeight()+1;

            lShape.mPosition = lNewPos;

            lReport.GetContactWithObstacles( pLevel, &lShape, pRoom, this );

            if( lReport.IsInMaze() )
            {
               if( !lReport.HaveContact() )
               {
                  mPosition    = lNewPos;
                  mOrientation = lNewOrientation;
                  pRoom        = lReport.Room();
               }
            }
         }
      }
   }
   return pRoom;
}



const MR_ShapeInterface* MR_TestElement::GetObstacleShape( )
{
   mCollisionShape.mPosition = mPosition;

   return &mCollisionShape;
}


void MR_TestElement::ApplyEffect( const MR_ContactEffect* pEffect,  MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 /*pZMin*/, MR_Int32 /*pZMax*/, MR_Level* /*pLevel*/ )
{
   MR_ContactEffect* lEffect = (MR_ContactEffect*)pEffect;
   const MR_PhysicalCollision* lPhysCollision = dynamic_cast<MR_PhysicalCollision*>(lEffect);
      
   if( (lPhysCollision != NULL)&&pValidDirection )
   {
      MR_InertialMoment lMoment;

      lMoment.mWeight = 90;
      lMoment.mXSpeed = mXSpeed;
      lMoment.mYSpeed = mYSpeed;
      lMoment.mZSpeed = 0;

      lMoment.ComputeCollision( lPhysCollision,
                                pHorizontalDirection );

      mXSpeed = lMoment.mXSpeed;
      mYSpeed = lMoment.mYSpeed;

   }

}

const MR_ContactEffectList* MR_TestElement::GetEffectList()
{
   *(int*)&mContactEffect.mXSpeed = mXSpeed;
   *(int*)&mContactEffect.mYSpeed = mYSpeed;
   *(int*)&mContactEffect.mZSpeed = 0;

   return &mContactEffectList;

}

const MR_ShapeInterface* MR_TestElement::GetReceivingContactEffectShape()
{
   mCollisionShape.mPosition = mPosition;
   return &mCollisionShape;
}

const MR_ShapeInterface* MR_TestElement::GetGivingContactEffectShape()
{
   mContactShape.mPosition = mPosition;
   return &mContactShape;
}


MR_Int32 MR_TestElement::Cylinder::ZMin()const
{
   return mPosition.mZ;
}

MR_Int32 MR_TestElement::Cylinder::ZMax()const
{
   return mPosition.mZ+1800;
}

MR_Int32 MR_TestElement::Cylinder::AxisX()const
{
   return mPosition.mX;
}

MR_Int32 MR_TestElement::Cylinder::AxisY()const
{
   return mPosition.mY;
}

MR_Int32 MR_TestElement::Cylinder::RayLen()const
{
   return mRay;
}












