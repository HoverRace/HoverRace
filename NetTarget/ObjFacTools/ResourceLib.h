// ResourceLib.h
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


#ifndef MR_RESOURCE_LIB_H
#define MR_RESOURCE_LIB_H

#include "../Util/RecordFile.h"
#include "ResActor.h"
#include "ResSprite.h"
#include "ResSound.h"

#ifdef MR_OBJ_FAC_TOOLS
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif



class MR_ResourceLib
{
   // Each module can have its own MR_ResourceLib

   protected:
      MR_RecordFile  mRecordFile;

      CMap< int, int, MR_ResBitmap*,          MR_ResBitmap* >          mBitmapList;
      CMap< int, int, MR_ResActor*,           MR_ResActor*  >          mActorList;
      CMap< int, int, MR_ResSprite*,          MR_ResSprite* >          mSpriteList;
      CMap< int, int, MR_ResShortSound*,      MR_ResShortSound* >      mShortSoundList;
      CMap< int, int, MR_ResContinuousSound*, MR_ResContinuousSound* > mContinuousSoundList;
      
      void LoadBitmaps( CArchive& pArchive );
      void LoadActors(  CArchive& pArchive );
      void LoadSprites( CArchive& pArchive );
      void LoadSounds(  CArchive& pArchive );

      MR_DllDeclare MR_ResourceLib();

   public:
      MR_DllDeclare MR_ResourceLib( const char* pFileName );
      MR_DllDeclare ~MR_ResourceLib();

      MR_DllDeclare /*const*/ MR_ResBitmap*          GetBitmap(          int pBitmapId  );
      MR_DllDeclare const MR_ResActor*           GetActor(           int pActorId   );
      MR_DllDeclare const MR_ResSprite*          GetSprite(          int pSpriteId  );
      MR_DllDeclare const MR_ResShortSound*      GetShortSound(      int pSoundId   );
      MR_DllDeclare const MR_ResContinuousSound* GetContinuousSound( int pSoundId   );

};

#define MR_RESOURCE_FILE_MAGIC    12345

#undef MR_DllDeclare

#endif



