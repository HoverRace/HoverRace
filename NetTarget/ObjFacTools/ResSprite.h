// ResSprite.h
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


#ifndef MR_RES_SPRITE_H
#define MR_RES_SPRITE_H

#include "../VideoServices/Sprite.h"

#ifdef MR_OBJ_FAC_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

class MR_ResSprite: public MR_Sprite
{
   protected:
      int mResourceId;

   public:
      MR_DllDeclare MR_ResSprite( int pResourceId );   // Only availlable for resourceLib and construction
      MR_DllDeclare ~MR_ResSprite();

      MR_DllDeclare int  GetResourceId()const;

};



#undef MR_DllDeclare

#endif



