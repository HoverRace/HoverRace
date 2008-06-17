// MainCharacterRenderer.h
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

#ifndef MAIN_CHARACTER_RENDERER_H
#define MAIN_CHARACTER_RENDERER_H



#include "../VideoServices/3DViewPort.h"
#include "../VideoServices/SoundServer.h"
#include "../Util/DllObjectFactory.h"


#ifdef MR_MAIN_CHARACTER
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


class MR_DllDeclare MR_MainCharacterRenderer:public MR_ObjectFromFactory
{


   private:



   public:
      // Construction
      MR_MainCharacterRenderer( const MR_ObjectFromFactoryId& pId );
      ~MR_MainCharacterRenderer();

      virtual void Render( MR_3DViewPort* pDest, 
                           const MR_3DCoordinate& pPosition, 
                           MR_Angle pOrientation,
                           BOOL     pMotorOn,
                           int      pHoverId,
                           int      pModel                  ) = 0;


      // Sound list
      virtual MR_ShortSound*      GetLineCrossingSound() = 0;
      virtual MR_ShortSound*      GetStartSound()        = 0;
      virtual MR_ShortSound*      GetFinishSound()       = 0;
      virtual MR_ShortSound*      GetBumpSound()         = 0;
      virtual MR_ShortSound*      GetJumpSound()         = 0;
      virtual MR_ShortSound*      GetFireSound()         = 0;
      virtual MR_ShortSound*      GetMisJumpSound()      = 0;
      virtual MR_ShortSound*      GetMisFireSound()      = 0;
      virtual MR_ShortSound*      GetOutOfCtrlSound()    = 0;
      virtual MR_ContinuousSound* GetMotorSound()        = 0;
      virtual MR_ContinuousSound* GetFrictionSound()     = 0;


};


#undef MR_DllDeclare

#endif


