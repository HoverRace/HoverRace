// SpriteHandle.h
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

// The class defined in this file can be used as a base class for 
// Sprites
//
//

#ifndef SPRITE_HANDLE_H
#define SPRITE_HANDLE_H

#include "ResourceLib.h"

#ifdef MR_OBJ_FAC_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


class MR_SpriteHandle: public MR_ObjectFromFactory
{
   protected:
      const MR_ResSprite* mSprite;

   public:
      MR_DllDeclare  MR_SpriteHandle( const MR_ObjectFromFactoryId& pId, const MR_ResSprite* pSprite );
      MR_DllDeclare ~MR_SpriteHandle();
   
      MR_DllDeclare const MR_Sprite* GetSprite()const;

};

#undef MR_DllDeclare

#endif

