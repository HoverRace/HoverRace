// ResSound.cpp
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
#include "ResSound.h"

#define new DEBUG_NEW

MR_ResShortSound::MR_ResShortSound( int pResourceId )
{
   mResourceId = pResourceId;
   mNbCopy     = 0;
   mDataLen    = 0;
   mData       = NULL;
   mSound      = NULL;

}


MR_ResShortSound::~MR_ResShortSound()
{
   MR_SoundServer::DeleteShortSound( mSound );
   mSound = NULL;

   delete []mData;
   mData = NULL;
}

int MR_ResShortSound::GetResourceId()const
{
   return mResourceId;
}

void MR_ResShortSound::Serialize( CArchive& pArchive )
{
   if( pArchive.IsStoring() )
   {
      pArchive << mNbCopy;
      pArchive << mDataLen;         
      pArchive.Write( mData, mDataLen );

   }
   else
   {

      MR_SoundServer::DeleteShortSound( mSound );
      mSound = NULL;

      delete []mData;
      mData = NULL;

      pArchive >> mNbCopy;
      pArchive >> mDataLen;         

      mData = new char[ mDataLen ];

      pArchive.Read( mData, mDataLen );

      mSound = MR_SoundServer::CreateShortSound( mData, mNbCopy );

   }
}

MR_ShortSound* MR_ResShortSound::GetSound()const
{
   return mSound;
}





MR_ResContinuousSound::MR_ResContinuousSound( int pResourceId )
{
   mResourceId = pResourceId;
   mNbCopy     = 0;
   mDataLen    = 0;
   mData       = NULL;
   mSound      = NULL;
}


MR_ResContinuousSound::~MR_ResContinuousSound()
{
   MR_SoundServer::DeleteContinuousSound( mSound );
   mSound = NULL;

   delete []mData;
   mData = NULL;
}

int MR_ResContinuousSound::GetResourceId()const
{
   return mResourceId;
}

void MR_ResContinuousSound::Serialize( CArchive& pArchive )
{
   if( pArchive.IsStoring() )
   {
      pArchive << mNbCopy;
      pArchive << mDataLen;         
      pArchive.Write( mData, mDataLen );

   }
   else
   {
      MR_SoundServer::DeleteContinuousSound( mSound );
      mSound = NULL;

      delete []mData;
      mData = NULL;

      pArchive >> mNbCopy;
      pArchive >> mDataLen;         

      mData = new char[ mDataLen ];

      pArchive.Read( mData, mDataLen );

      mSound = MR_SoundServer::CreateContinuousSound( mData, mNbCopy );

   }
}

MR_ContinuousSound* MR_ResContinuousSound::GetSound()const
{
   return mSound;
}






