// ResSound.h
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


#ifndef MR_RES_SOUND_H
#define MR_RES_SOUND_H

#include "../VideoServices/SoundServer.h"

#ifdef MR_OBJ_FAC_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif


class MR_ResShortSound
{
   protected:
      int            mResourceId;
      MR_ShortSound* mSound;

      int   mNbCopy;
      int   mDataLen;
      char* mData;


   public:
      MR_DllDeclare MR_ResShortSound( int pResourceId );   // Only availlable for resourceLib and construction
      MR_DllDeclare ~MR_ResShortSound();

      MR_DllDeclare int  GetResourceId()const;
      MR_DllDeclare void Serialize( CArchive& pArchive );

      MR_DllDeclare MR_ShortSound* GetSound()const;

};

class MR_ResContinuousSound
{
   protected:
      int                 mResourceId;
      MR_ContinuousSound* mSound;

      int   mNbCopy;
      int   mDataLen;
      char* mData;

   public:
      MR_DllDeclare MR_ResContinuousSound( int pResourceId );   // Only availlable for resourceLib and construction
      MR_DllDeclare ~MR_ResContinuousSound();

      MR_DllDeclare int  GetResourceId()const;
      MR_DllDeclare void Serialize( CArchive& pArchive );

      MR_DllDeclare MR_ContinuousSound* GetSound()const;

};



#undef MR_DllDeclare

#endif



