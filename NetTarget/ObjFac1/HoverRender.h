// HoverRender.h
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

#ifndef HOVER_RENDER_H
#define HOVER_RENDER_H

#include "../ObjFacTools/FreeElementBase.h"
#include "../MainCharacter/MainCharacterRenderer.h"


class MR_HoverRender: public MR_MainCharacterRenderer
{
   protected:
      const MR_ResActor* mActor0;
      const MR_ResActor* mActor1;
      const MR_ResActor* mActor2;


      int   mFrame;

      MR_ShortSound*      mLineCrossingSound;
      MR_ShortSound*      mStartSound;
      MR_ShortSound*      mFinishSound;
      MR_ShortSound*      mBumpSound;
      MR_ShortSound*      mJumpSound;
      MR_ShortSound*      mFireSound;
      MR_ShortSound*      mMisJumpSound;
      MR_ShortSound*      mMisFireSound;
      MR_ShortSound*      mOutOfCtrlSound;
      MR_ContinuousSound* mMotorSound;
      MR_ContinuousSound* mFrictionSound;

      const MR_ResBitmap* mCockpitBitmap[10];
      const MR_ResBitmap* mCockpitBitmap2[10];

   public:
      MR_HoverRender( const MR_ObjectFromFactoryId& pId );
      ~MR_HoverRender();   

      void Render( MR_3DViewPort* pDest, 
                   const MR_3DCoordinate& pPosition, 
                   MR_Angle pOrientation,
                   BOOL     pMotorOn,
                   int      pHoverId,
                   int      pModel                   );

      MR_ShortSound*      GetLineCrossingSound();
      MR_ShortSound*      GetStartSound();
      MR_ShortSound*      GetFinishSound();
      MR_ShortSound*      GetBumpSound();
      MR_ShortSound*      GetJumpSound();
      MR_ShortSound*      GetFireSound();
      MR_ShortSound*      GetMisJumpSound();
      MR_ShortSound*      GetMisFireSound();
      MR_ShortSound*      GetOutOfCtrlSound();
      MR_ContinuousSound* GetMotorSound();
      MR_ContinuousSound* GetFrictionSound();


};



#endif



