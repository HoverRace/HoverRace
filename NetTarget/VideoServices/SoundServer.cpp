// SoundServer.cpp
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

#include "stdafx.h"
#include "SoundServer.h"
#include "../Util/MR_Types.h"
#include <mmreg.h>
#include <dsound.h>

#define MR_MAX_SOUND_COPY 6

class MR_SoundBuffer
{
   private:
      static MR_SoundBuffer* mList;
      MR_SoundBuffer*        mNext;

   protected:

      int                 mNbCopy;
      IDirectSoundBuffer* mSoundBuffer[MR_MAX_SOUND_COPY];

      int                 mNormalFreq;

   public:
      MR_SoundBuffer();
      virtual ~MR_SoundBuffer();

      BOOL Init( const char* pData, int pNbCopy );

      void SetParams( int pCopy, int pDB, double pSpeed, int pPan );

      int GetNbCopy()const;

      static void DeleteAll();

      virtual void ApplyCumCommand();
      
      static void ApplyCumCommandForAll();

};


class MR_ShortSound:public MR_SoundBuffer
{

   protected:
      int mCurrentCopy;


   public:
      MR_ShortSound();
      ~MR_ShortSound();

      void Play( int pDB, double pSpeed, int pPan );
};


class MR_ContinuousSound:public MR_SoundBuffer
{
   protected:
      BOOL   mOn[MR_MAX_SOUND_COPY];
      int    mMaxDB[MR_MAX_SOUND_COPY];
      double mMaxSpeed[MR_MAX_SOUND_COPY];


      void ResetCumStat();

   public:
      MR_ContinuousSound();
      ~MR_ContinuousSound();

      void Pause(   int pCopy );
      void Restart( int pCopy );

      void ApplyCumCommand();
      void CumPlay( int pCopy, int pDB, double pSpeed );


};


// Variables
MR_SoundBuffer* MR_SoundBuffer::mList = NULL;

IDirectSound* gDirectSound = NULL;



// Implementation
MR_SoundBuffer::MR_SoundBuffer()
{
   mNbCopy = 0;

   for( int lCounter = 0; lCounter < MR_MAX_SOUND_COPY; lCounter++ )
   {
      mSoundBuffer[ lCounter ] = NULL;
   }

   // Add the new buffer to the list
   mNext = mList;
   mList = this;

}

MR_SoundBuffer::~MR_SoundBuffer()
{
   // Remove form list
   if( mList == this )
   {
      mList = mNext;
      mNext = NULL;
   }
   else
   {
      MR_SoundBuffer* mPrev = mList;

      while( mPrev->mNext != this )
      {
         ASSERT( mPrev != NULL );

         mPrev = mPrev->mNext;
      }

      mPrev->mNext = mNext;
      mNext = NULL;
   }

   // Delete the sound buffers
   for( int lCounter = 0; lCounter< mNbCopy; lCounter++ )
   {
      if( mSoundBuffer[ lCounter ] != NULL )
      {
         mSoundBuffer[ lCounter ]->Release();
         mSoundBuffer[ lCounter ] = NULL;
      }
   }
}

void MR_SoundBuffer::ApplyCumCommand()
{
   // Do notting by default
}

void MR_SoundBuffer::ApplyCumCommandForAll()
{
   MR_SoundBuffer* mCurrent = mList;

   while( mCurrent != NULL )
   {
      mCurrent->ApplyCumCommand();
      mCurrent = mCurrent->mNext;
   }
}

void MR_SoundBuffer::DeleteAll()
{
   while( mList != NULL )
   {
      delete mList;
   }
}

BOOL MR_SoundBuffer::Init( const char* pData, int pNbCopy )
{
   BOOL lReturnValue = TRUE;

   ASSERT( mSoundBuffer[0] == NULL ); // Already initialized

   if( pNbCopy > MR_MAX_SOUND_COPY )
   {
      ASSERT( FALSE );
      pNbCopy = MR_MAX_SOUND_COPY;
   }

   mNbCopy = pNbCopy;

   // Parse pData
   DSBUFFERDESC    lDesc;
   MR_UInt32       lBufferLen  = *(MR_UInt32*)pData;
   WAVEFORMATEX*   lWaveFormat = (WAVEFORMATEX*)(pData+sizeof(MR_UInt32));
   const char*     lSoundData  = pData+sizeof( MR_UInt32 ) + sizeof( WAVEFORMATEX );

   lWaveFormat->cbSize = 0;

   lDesc.dwSize        = sizeof( lDesc );
   lDesc.dwFlags       = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
   lDesc.dwReserved    = 0;
   lDesc.dwBufferBytes = lBufferLen;
   lDesc.lpwfxFormat   = lWaveFormat;

   mNormalFreq = lWaveFormat->nSamplesPerSec;

   // Initialize the first copy
   int lCode;
   if( (lCode = gDirectSound->CreateSoundBuffer(&lDesc, 
                                       &(mSoundBuffer[0]),
                                       NULL                )) != DS_OK )
   {
      ASSERT( lCode != DSERR_ALLOCATED );
      ASSERT( lCode != DSERR_BADFORMAT );
      ASSERT( lCode != DSERR_INVALIDPARAM );
      ASSERT( lCode != DSERR_NOAGGREGATION );
      ASSERT( lCode != DSERR_OUTOFMEMORY );
      ASSERT( FALSE );
      lReturnValue = FALSE;
   }

   if( lReturnValue )
   {
      void*         lSoundBuffer;
      unsigned long lSoundBufferLen;

      if( mSoundBuffer[0]->Lock( 0, lBufferLen, &lSoundBuffer, &lSoundBufferLen, NULL, NULL, 0 ) == DS_OK )
      {
         ASSERT( lSoundBufferLen == lBufferLen );

         memcpy( lSoundBuffer, lSoundData, lSoundBufferLen );

         mSoundBuffer[0]->Unlock( lSoundBuffer, lSoundBufferLen, NULL, 0 );
      }
      else
      {
         ASSERT( FALSE );
         lReturnValue = FALSE;
      }
   }

   // Init extra copy
   for( int lCounter = 1; lReturnValue&&(lCounter < mNbCopy); lCounter++ )
   {
      if( gDirectSound->DuplicateSoundBuffer( mSoundBuffer[0],&mSoundBuffer[lCounter] ) != DS_OK )
      {
         lReturnValue = FALSE;
      }
   }

   return lReturnValue;
}



void MR_SoundBuffer::SetParams( int pCopy, int pDB, double pSpeed, int pPan )
{
   if( pCopy >= mNbCopy )
   {
      pCopy = mNbCopy-1;
   }

   if( mSoundBuffer[ pCopy ] != NULL )
   {
      unsigned long lFreq = mNormalFreq*pSpeed;

      if( lFreq > 100000 )
      {
         lFreq = 100000;
      }
      mSoundBuffer[pCopy]->SetVolume( pDB );
      mSoundBuffer[pCopy]->SetFrequency( lFreq );
      mSoundBuffer[pCopy]->SetPan( pPan );
   }
}

int MR_SoundBuffer::GetNbCopy()const
{
   return mNbCopy;
}


// MR_ShortSound
MR_ShortSound::MR_ShortSound()
{
   mCurrentCopy = 0;
}

MR_ShortSound::~MR_ShortSound()
{
}

void MR_ShortSound::Play( int pDB, double pSpeed, int pPan )
{
   mSoundBuffer[ mCurrentCopy ]->SetCurrentPosition( 0 );
   SetParams( mCurrentCopy, pDB, pSpeed, pPan );
   mSoundBuffer[ mCurrentCopy ]->Play( 0, 0, 0 );   

   mCurrentCopy++;
   if(mCurrentCopy >= mNbCopy )
   {
      mCurrentCopy = 0;
   }
}


// class MR_ContinuousSound
MR_ContinuousSound::MR_ContinuousSound()
{
   ResetCumStat();
}
 
MR_ContinuousSound::~MR_ContinuousSound()
{
}

void MR_ContinuousSound::ResetCumStat()
{
   for( int lCounter = 0; lCounter < mNbCopy; lCounter++ )
   {
      mOn[ lCounter ] = FALSE;
      mMaxSpeed[ lCounter ] = 0;
      mMaxDB[ lCounter ] = -10000;
   }
}


void MR_ContinuousSound::Pause( int pCopy )
{
   if( pCopy >= mNbCopy )
   {
      pCopy = mNbCopy-1;
   }

   mSoundBuffer[ pCopy ]->Stop( );   

}

void MR_ContinuousSound::Restart( int pCopy )
{
   if( pCopy >= mNbCopy )
   {
      pCopy = mNbCopy-1;
   }
   mSoundBuffer[ pCopy ]->Play( 0, 0, DSBPLAY_LOOPING );   
}

void MR_ContinuousSound::ApplyCumCommand( )
{

   for( int lCounter = 0; lCounter < mNbCopy; lCounter++ )
   {
      if( mOn[ lCounter ] )
      {
         SetParams( lCounter, mMaxDB[ lCounter ], mMaxSpeed[ lCounter ], 0 );
         Restart( lCounter );
      }
      else
      {
         Pause( lCounter );
      }
   }
   ResetCumStat();

}

void MR_ContinuousSound::CumPlay( int pCopy, int pDB, double pSpeed )
{
   if( pCopy >= mNbCopy )
   {
      pCopy = mNbCopy-1;
   }

   mOn[ pCopy ]       = TRUE;
   mMaxDB[ pCopy ]    = max( mMaxDB   [ pCopy ], pDB );
   mMaxSpeed[ pCopy ] = max( mMaxSpeed[ pCopy ], pSpeed );

}




// namespace MR_SoundServer

BOOL MR_SoundServer::Init( HWND pWindow )
{
   BOOL lReturnValue = TRUE;

   if( gDirectSound == NULL )
   {
      if( DirectSoundCreate( NULL, &gDirectSound, NULL ) == DS_OK )
      {
         
         if( gDirectSound->SetCooperativeLevel( pWindow, DSSCL_NORMAL ) != DS_OK )
         {
            ASSERT( FALSE );
            lReturnValue = FALSE;
         }         
      }
      else
      {
         lReturnValue = FALSE;
      }
   }

   return lReturnValue;
}

void MR_SoundServer::Close()
{
   MR_SoundBuffer::DeleteAll();

   if( gDirectSound != NULL )
   {
      gDirectSound->Release();
      gDirectSound = NULL;
   }
}


MR_ShortSound* MR_SoundServer::CreateShortSound( const char* pData, int pNbCopy )
{
   if( gDirectSound != NULL )
   {
      MR_ShortSound* lReturnValue = new MR_ShortSound;

      if( !lReturnValue->Init( pData, pNbCopy ) )
      {
         lReturnValue = NULL;
      }
      return lReturnValue;
   }
   else
   {
      return NULL;
   }
}

void MR_SoundServer::DeleteShortSound( MR_ShortSound* pSound )
{
   delete pSound;
}

void MR_SoundServer::Play( MR_ShortSound* pSound, int pDB, double pSpeed, int pPan )
{
   if( pSound != NULL )
   {
      pSound->Play( pDB, pSpeed, pPan );
   }
}

int MR_SoundServer::GetNbCopy( MR_ShortSound* pSound )
{
   if( pSound != NULL )
   {
      return pSound->GetNbCopy();
   }
   else
   {
      return 1;
   }
}

MR_ContinuousSound* MR_SoundServer::CreateContinuousSound( const char* pData, int pNbCopy )
{
   if( gDirectSound != NULL )
   {
      MR_ContinuousSound* lReturnValue = new MR_ContinuousSound;

      if( !lReturnValue->Init(pData, pNbCopy ) )
      {
         lReturnValue = NULL;
      }
      return lReturnValue;
   }
   else
   {
      return NULL;
   }
}

void MR_SoundServer::DeleteContinuousSound( MR_ContinuousSound* pSound )
{
   delete pSound;
}

void MR_SoundServer::Play( MR_ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int /*pPan*/ )
{
   if( pSound != NULL )
   {
      pSound->CumPlay( pCopy, pDB, pSpeed );
   }
}


void MR_SoundServer::ApplyContinuousPlay()
{
   if( gDirectSound != NULL )
   {
      MR_SoundBuffer::ApplyCumCommandForAll();
   }
}

/*
void MR_SoundServer::Pause(   MR_ContinuousSound* pSound, int pCopy )
{
   if( pSound != NULL )
   {
      pSound->Pause( pCopy );
   }
}

void MR_SoundServer::Restart( MR_ContinuousSound* pSound, int pCopy )
{
   if( pSound != NULL )
   {
      pSound->Restart( pCopy );
   }
}

void MR_SoundServer::SetParams( MR_ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int pPan )
{
   if( pSound != NULL )
   {
      pSound->SetParams( pCopy, pDB, pSpeed, pPan );
   }
}
*/

int MR_SoundServer::GetNbCopy( MR_ContinuousSound* pSound )
{
   if( pSound != NULL )
   {
      return pSound->GetNbCopy();
   }
   else
   {
      return 1;
   }
}
