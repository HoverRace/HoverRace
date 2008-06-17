// HoverRender.cpp
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

#include "HoverRender.h"
#include "ObjFac1Res.h"
#include "../ObjFacTools/ObjectFactoryData.h"
#include "../Util/FuzzyLogic.h"

// Local stuff

static class MR_ResActorFriend
{
   public:
      static void Draw( const MR_ResActor* pActor, MR_3DViewPort* pDest, const MR_PositionMatrix& pMatrix, int pSequence, int pFrame, const MR_Bitmap* pCockpitBitmap );
};
                    
                    





MR_HoverRender::MR_HoverRender( const MR_ObjectFromFactoryId& pId )
               :MR_MainCharacterRenderer( pId )
{
   mFrame = 0;
   mActor0 = gObjectFactoryData->mResourceLib.GetActor( MR_ELECTRO_CAR );
   mActor1 = gObjectFactoryData->mResourceLib.GetActor( MR_HITECH_CAR );
   mActor2 = gObjectFactoryData->mResourceLib.GetActor( MR_BITURBO_CAR );


   mLineCrossingSound = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_LINE_CROSSING )->GetSound();
   mStartSound        = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_START         )->GetSound();
   mFinishSound       = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_FINISH        )->GetSound();
   mBumpSound         = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_BUMP          )->GetSound();
   mJumpSound         = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_JUMP          )->GetSound();
   mFireSound         = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_FIRE          )->GetSound();
   mMisJumpSound      = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_MIS_JUMP      )->GetSound();
   mMisFireSound      = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_MIS_FIRE      )->GetSound();
   mOutOfCtrlSound    = gObjectFactoryData->mResourceLib.GetShortSound( MR_SND_OUT_OF_CTRL   )->GetSound();
   mMotorSound        = gObjectFactoryData->mResourceLib.GetContinuousSound( MR_SND_MOTOR    )->GetSound();
   mFrictionSound     = gObjectFactoryData->mResourceLib.GetContinuousSound( MR_SND_FRICTION )->GetSound();

   for( int lCounter = 0; lCounter < 10; lCounter++ )
   {
      mCockpitBitmap[ lCounter ] = gObjectFactoryData->mResourceLib.GetBitmap( MR_CAR_COCKPIT1+lCounter );
      mCockpitBitmap2[ lCounter ] = gObjectFactoryData->mResourceLib.GetBitmap( MR_CAR_COCKPIT21+lCounter );
   }

}

MR_HoverRender::~MR_HoverRender()
{

}

void MR_HoverRender::Render( MR_3DViewPort* pDest, 
                             const MR_3DCoordinate& pPosition, 
                             MR_Angle pOrientation,
                             BOOL     pMotorOn, 
                             int      pHoverId,
                             int      pModel        )
{

   // Compute the required rotation matrix
   MR_PositionMatrix lMatrix;

   if( pDest->ComputePositionMatrix( lMatrix, pPosition, pOrientation, 1000 /* TODO Object ray must be precomputed at compilation*/ ) )
   {
      int lSeq   = pMotorOn?1:0;

      if( pMotorOn )
      {
         mFrame = (mFrame+1)%2;
      }
      else
      {
         mFrame =0;
      }

      if( pModel == 1 )
      {
         MR_ResActorFriend::Draw( mActor1, pDest, lMatrix, lSeq, mFrame, mCockpitBitmap2[ pHoverId%10 ] );
      }
      else if( pModel == 2 )
      {
         MR_ResActorFriend::Draw( mActor2, pDest, lMatrix, lSeq, mFrame, mCockpitBitmap[ pHoverId%10 ] );
      }
      else
      {
         MR_ResActorFriend::Draw( mActor0, pDest, lMatrix, lSeq, mFrame, mCockpitBitmap[ pHoverId%10 ] );
      }
   } 
}   

MR_ShortSound* MR_HoverRender::GetLineCrossingSound()
{
   return mLineCrossingSound;
}

MR_ShortSound* MR_HoverRender::GetStartSound()
{
   return mStartSound;
}

MR_ShortSound* MR_HoverRender::GetFinishSound()
{
   return mFinishSound;
}

MR_ShortSound* MR_HoverRender::GetBumpSound()
{
   return mBumpSound;
}

MR_ShortSound* MR_HoverRender::GetJumpSound()
{
   return mJumpSound;
}

MR_ShortSound* MR_HoverRender::GetFireSound()
{
   return mFireSound;
}

MR_ShortSound* MR_HoverRender::GetMisJumpSound()
{
   return mMisJumpSound;
}

MR_ShortSound* MR_HoverRender::GetMisFireSound()
{
   return mMisFireSound;
}

MR_ShortSound* MR_HoverRender::GetOutOfCtrlSound()
{
   return mOutOfCtrlSound;
}

MR_ContinuousSound* MR_HoverRender::GetMotorSound()
{
   return mMotorSound;
}

MR_ContinuousSound* MR_HoverRender::GetFrictionSound()
{
   return mFrictionSound;
}


void MR_ResActorFriend::Draw( const MR_ResActor* pActor, MR_3DViewPort* pDest, const MR_PositionMatrix& pMatrix, int pSequence, int pFrame, const MR_Bitmap* pCockpitBitmap )
{
   MR_ResActor::Frame* lFrame = &(pActor->mSequenceList[ pSequence ].mFrameList[ pFrame ]);

   ASSERT( lFrame != NULL );

   for( int lCounter = 0; lCounter < lFrame->mNbComponent; lCounter++ )
   {
      MR_ResActor::Patch* lPatch = (MR_ResActor::Patch*)lFrame->mComponentList[ lCounter ];

      int lBitmapResId = lPatch->mBitmap->GetResourceId();

      if( (lBitmapResId == MR_CAR_COCKPIT)||(lBitmapResId == MR_CAR2_COCKPIT)/*||(lBitmapResId == MR_ECAR_COCKPIT)*/ )
      {
         pDest->RenderPatch( *lPatch, pMatrix , pCockpitBitmap );
      }
      else
      {
         pDest->RenderPatch( *lPatch, pMatrix , lPatch->mBitmap );
      }
   }
}
