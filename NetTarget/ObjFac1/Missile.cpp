// Missile.cpp
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

#include "Missile.h"
#include "ObjFac1Res.h"
#include "../ObjFacTools/ObjectFactoryData.h"
#include "../Model/ConcreteShape.h"
#include "../Model/FreeElementMovingHelper.h"



const MR_Int32 cMissileRay    = 300;       // the missile have a diameter of 60cm
const MR_Int32 cMissileWeight = 100;
const MR_Int32 cLifeTime      = 7500;      // 7.6 sec..was 10 sec before
const MR_Int32 cStopTime      = 1200;      // 1.2 sec
const MR_Int32 cIgnitionTime  =  175;       // 0.175 sec

/*
const MR_Int32 cMissileRay    = 1;       // the missile have a diameter of 60cm
const MR_Int32 cMissileWeight = 130;
const MR_Int32 cLifeTime      = 500;      // 7.6 sec..was 10 sec before
const MR_Int32 cStopTime      = 10;      // 1.2 sec
const MR_Int32 cIgnitionTime  = 110;       // 0.175 sec
*/

#define TIME_SLICE                     5
#define MINIMUM_SPLITTABLE_TIME_SLICE  6
const double eSteadySpeed             = 21.0*2222.0/1000.0;


MR_Int32 MR_Missile::ZMin()const
{
   return mPosition.mZ-cMissileRay;
}

MR_Int32 MR_Missile::ZMax()const
{
   return mPosition.mZ+cMissileRay;
}

MR_Int32 MR_Missile::AxisX()const
{
   return mPosition.mX;
}

MR_Int32 MR_Missile::AxisY()const
{
   return mPosition.mY;
}

MR_Int32 MR_Missile::RayLen()const
{
   return cMissileRay;
}

MR_Missile::MR_Missile( const MR_ObjectFromFactoryId& pId )
              :MR_FreeElementBase( pId )
{
   mHoverId          = -1;
   mLived            = 0;
   mXSpeed           = 0;
   mYSpeed           = 0;
   mBounceSoundEvent = FALSE;
   mEffectList.AddTail( &mCollisionEffect );
   mEffectList.AddTail( &mLostOfControlEffect );
   mActor = gObjectFactoryData->mResourceLib.GetActor( MR_MISSILE );

   mBounceSound    = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_MISSILE_BOUNCE )->GetSound();
   mMotorSound     = gObjectFactoryData->mResourceLib.GetContinuousSound( MR_SND_MISSILE_MOTOR )->GetSound();
 
   mLostOfControlEffect.mType      = MR_LostOfControl::eMissile;
   mLostOfControlEffect.mElementId = -1;
   mLostOfControlEffect.mHoverId   = mHoverId;

}

MR_Missile::~MR_Missile()
{
}

void MR_Missile::SetOwnerId( int pHoverId )
{
   mHoverId = pHoverId;
   mLostOfControlEffect.mHoverId   = mHoverId;
}


const MR_ContactEffectList* MR_Missile::GetEffectList()
{

   if( mLived > cIgnitionTime )
   {
      mCollisionEffect.mWeight = cMissileWeight;
      mCollisionEffect.mXSpeed = mXSpeed*256;
      mCollisionEffect.mYSpeed = mYSpeed*256;
      mCollisionEffect.mZSpeed = 0;      

      return &mEffectList;
   }
   else
   {
      return NULL;
   }
}

const MR_ShapeInterface* MR_Missile::GetReceivingContactEffectShape()
{
   return this;
}

const MR_ShapeInterface* MR_Missile::GetGivingContactEffectShape()
{
   return this;
}



// Simulation
int MR_Missile::Simulate( MR_SimulationTime pDuration, MR_Level* pLevel, int pRoom )
{   

   // Do the simulation
   while( pDuration > 0 )
   {
      if( pDuration > TIME_SLICE )
      {
         pRoom = InternalSimulate( TIME_SLICE, pLevel, pRoom );
      }
      else
      {
         pRoom = InternalSimulate( pDuration, pLevel, pRoom );
      }
      pDuration -= TIME_SLICE;
   }
 
   // Determine the frame that must be displayed
  
   if( mLived > cLifeTime )
   {
      int lNbFrame = mActor->GetFrameCount( 2 );

      mCurrentSequence = 2;

      mCurrentFrame = lNbFrame*(mLived-cLifeTime)/cStopTime;

      if( mCurrentFrame >= lNbFrame )
      {
         mCurrentFrame = lNbFrame-1;
      }


   }
   else if( mLived < (cIgnitionTime*3) )
   {
      mCurrentSequence = 0;

      mCurrentFrame = mActor->GetFrameCount( mCurrentSequence )*mLived/(cIgnitionTime*3);

   }
   else
   {
      mCurrentSequence = 1;

      mCurrentFrame = (mLived/256)%2;
   }

   // Determined if the object should be deleted
   if( mLived >= (cLifeTime+cStopTime) )
   {
      pRoom = MR_Level::eMustBeDeleted;
   }

   return pRoom;
}

int MR_Missile::InternalSimulate( MR_SimulationTime pDuration, MR_Level* pLevel, int pRoom )
{

   mLived += pDuration;

   // MotorEffect
   double lSpeed = eSteadySpeed;

   if( mLived > cLifeTime )
   {
      lSpeed = (cStopTime-mLived-cLifeTime)*eSteadySpeed/cStopTime;

      if( lSpeed < 0 )
      {
         lSpeed = 0;
      }
   }

   // Debug patch (stop the missile for observation
   /*
   if( mLived > 1000 )
   {
      lSpeed = 0;
      mLived = 1200;
   }
   */

   
   // Determine new dispacement

   MR_Cylinder lShape;

   lShape.mRayLen = 1; //cMissileRay;
   lShape.mZMin   = mPosition.mZ-cMissileRay;
   lShape.mZMax   = mPosition.mZ+cMissileRay;

   // Compute speed objectives
   MR_2DCoordinate lTranslation;

   mXSpeed = lSpeed*MR_Cos[ mOrientation ]/MR_TRIGO_FRACT;
   mYSpeed = lSpeed*MR_Sin[ mOrientation ]/MR_TRIGO_FRACT;
   
   lTranslation.mX = pDuration*mXSpeed;
   lTranslation.mY = pDuration*mYSpeed;
   
   // Verify if the move is valid
   MR_ObstacleCollisionReport lReport;


   BOOL              lSuccessfullTry;
   MR_SimulationTime lDuration = pDuration;

   lShape.mAxis.mX = mPosition.mX+lTranslation.mX;
   lShape.mAxis.mY = mPosition.mY+lTranslation.mY;

   while(1)
   {      
      lSuccessfullTry = FALSE;

      lReport.GetContactWithObstacles( pLevel, &lShape, pRoom, this );   

      if( lReport.IsInMaze() )
      {
         if( !lReport.HaveContact() )
         {
            mPosition.mX = lShape.mAxis.mX;
            mPosition.mY = lShape.mAxis.mY;
            pRoom        = lReport.Room();

            lSuccessfullTry = TRUE;
         }
      }


      
      if( lDuration < MINIMUM_SPLITTABLE_TIME_SLICE )
      {
         break;
      }
      else
      {
         if( lSuccessfullTry )
         {
            if( lDuration == pDuration )
            {
               break; // success on first attempt
            }

            lDuration /= 2;

            lShape.mAxis.mX += lDuration*lTranslation.mX/pDuration;
            lShape.mAxis.mY += lDuration*lTranslation.mY/pDuration;
         }
         else
         {
            lDuration /= 2;

            lShape.mAxis.mX -= lDuration*lTranslation.mX/pDuration;
            lShape.mAxis.mY -= lDuration*lTranslation.mY/pDuration;
         }
      }

   }

   return pRoom;
}

void MR_Missile::ApplyEffect( const MR_ContactEffect* pEffect,  MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 /*pZMin*/, MR_Int32 /*pZMax*/, MR_Level* /*pLevel*/ )
{
   MR_ContactEffect* lEffect = (MR_ContactEffect*)pEffect;
   const MR_PhysicalCollision* lPhysCollision = dynamic_cast<MR_PhysicalCollision*>(lEffect);
      
   if( (lPhysCollision != NULL)&&pValidDirection )
   {

      if( lPhysCollision->mWeight < MR_PhysicalCollision::eInfiniteWeight )
      {
         if( mLived >= cIgnitionTime )
         {
            // Hitted a free object
            // Must died
            mLived = cLifeTime+cStopTime;
         }
      }
      else
      {
         // Hitted structure..make a perfect bounce
         MR_Angle lDiff = MR_NORMALIZE_ANGLE( pHorizontalDirection-mOrientation+MR_PI );

         if( (lDiff < (MR_PI/2) ) || ( lDiff > (MR_PI+MR_PI/2)) )
         {
            mOrientation = MR_NORMALIZE_ANGLE( pHorizontalDirection + lDiff );
            mBounceSoundEvent = TRUE;
         }
      }
   }
}

// State broadcast

class MR_MissileState
{
   public:
      MR_Int32                  mPosX;          // 4    4
      MR_Int32                  mPosY;          // 4    8
      MR_Int16                  mPosZ;          // 2   10 

      MR_Angle                  mOrientation;   // 2   12  
      MR_Int8                   mHoverId;       // 1   14
};


MR_ElementNetState MR_Missile::GetNetState()const
{
   static MR_MissileState lsState; // Static is ok because the variable will be used immediatly

   MR_ElementNetState lReturnValue;

   lReturnValue.mDataLen = sizeof( lsState );
   lReturnValue.mData    = (MR_UInt8*)&lsState;


   lsState.mPosX          = mPosition.mX;
   lsState.mPosY          = mPosition.mY;
   lsState.mPosZ          = mPosition.mZ;

   lsState.mOrientation   = mOrientation;

   lsState.mHoverId       = mHoverId;

   return lReturnValue;

}

void MR_Missile::SetNetState( int pDataLen, const MR_UInt8* pData )
{

   const MR_MissileState* lState = (const MR_MissileState*)pData;

   mPosition.mX = lState->mPosX;
   mPosition.mY = lState->mPosY;         
   mPosition.mZ = lState->mPosZ;         

   mOrientation = lState->mOrientation;  

   if( pDataLen >= sizeof( MR_MissileState ) )
   {
      mHoverId = lState->mHoverId;
      mLostOfControlEffect.mHoverId   = mHoverId;
   }

}



void MR_Missile::PlayExternalSounds( int pDB, int pPan )
{
   if( mBounceSoundEvent )
   {
      mBounceSoundEvent = FALSE;

      MR_SoundServer::Play( mBounceSound, pDB, 1.0, pPan );
   }

   MR_SoundServer::Play( mMotorSound, 1, pDB, 1.0, pPan );
}

