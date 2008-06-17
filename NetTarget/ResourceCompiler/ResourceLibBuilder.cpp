// ResourceLibBuilder.cpp
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

#include "StdAfx.h"
#include "ResourceLibBuilder.h"

MR_ResourceLibBuilder::MR_ResourceLibBuilder()
{
   // Notting to do
}
 
MR_ResourceLibBuilder::~MR_ResourceLibBuilder()
{
   // Notting to do
}

void MR_ResourceLibBuilder::AddBitmap( MR_ResBitmap* pBitmap )
{
   mBitmapList.SetAt( pBitmap->GetResourceId(), pBitmap );   
}
 
void MR_ResourceLibBuilder::AddActor(  MR_ResActor*  pActor  )
{
   mActorList.SetAt( pActor->GetResourceId(), pActor );
}

void MR_ResourceLibBuilder::AddSprite(  MR_ResSprite*  pSprite  )
{
   mSpriteList.SetAt( pSprite->GetResourceId(), pSprite );
}

void MR_ResourceLibBuilder::AddSound(  MR_ResShortSound*  pSound )
{
   mShortSoundList.SetAt( pSound->GetResourceId(), pSound );
}

void MR_ResourceLibBuilder::AddSound(  MR_ResContinuousSound*  pSound )
{
   mContinuousSoundList.SetAt( pSound->GetResourceId(), pSound );
}

BOOL MR_ResourceLibBuilder::Export( const char* pFileName )
{
   BOOL lReturnValue = TRUE;

   MR_RecordFile lFile;

   lReturnValue = lFile.CreateForWrite( pFileName,
                                        1,
                                        "\x8\rFireball object factory resource file, (c)GrokkSoft 1996\n\x1a" );


   if( !lReturnValue )
   {
      fprintf( stderr, "ERROR: Unable to create output file\n" );
   }
   else
   {
      lFile.BeginANewRecord();

      {

         // Write the magic number
         int lMagicNumber = MR_RESOURCE_FILE_MAGIC;

         CArchive lArchive( &lFile, CArchive::store );

         lArchive << lMagicNumber;

         WriteBitmaps( lArchive );

         WriteActors(  lArchive );
      
         WriteSprites( lArchive );

         WriteSounds(  lArchive );
         
      }
   }

   return lReturnValue;
}

void MR_ResourceLibBuilder::WriteBitmaps( CArchive& pArchive )
{
   int       lNbBitmap = mBitmapList.GetCount();
   POSITION  lPos      = mBitmapList.GetStartPosition();

   pArchive << lNbBitmap;



   while( lPos != NULL )
   {
      int           lKey;
      MR_ResBitmap* lValue;

      mBitmapList.GetNextAssoc( lPos, lKey, lValue );

      pArchive << lKey;      
      lValue->Serialize( pArchive );
   }  

}

void MR_ResourceLibBuilder::WriteActors(  CArchive& pArchive )
{
   int       lNbActor  = mActorList.GetCount();
   POSITION  lPos      = mActorList.GetStartPosition();

   pArchive << lNbActor;

   while( lPos != NULL )
   {
      int           lKey;
      MR_ResActor*  lValue;

      mActorList.GetNextAssoc( lPos, lKey, lValue );

      pArchive << lKey;      
      lValue->Serialize( pArchive );
   }  
}

void MR_ResourceLibBuilder::WriteSprites( CArchive& pArchive )
{
   int       lNbSprite = mSpriteList.GetCount();
   POSITION  lPos      = mSpriteList.GetStartPosition();

   pArchive << lNbSprite;


   while( lPos != NULL )
   {
      int           lKey;
      MR_ResSprite* lValue;

      mSpriteList.GetNextAssoc( lPos, lKey, lValue );

      pArchive << lKey;      
      lValue->Serialize( pArchive );
   }  
}

void MR_ResourceLibBuilder::WriteSounds( CArchive& pArchive )
{
   int       lNbSound;
   POSITION  lPos;


   lNbSound = mShortSoundList.GetCount();
   lPos     = mShortSoundList.GetStartPosition();  
   
   pArchive << lNbSound;

   while( lPos != NULL )
   {
      int               lKey;
      MR_ResShortSound* lValue;

      mShortSoundList.GetNextAssoc( lPos, lKey, lValue );

      pArchive << lKey;      
      lValue->Serialize( pArchive );
   }  

   lNbSound = mContinuousSoundList.GetCount();
   lPos     = mContinuousSoundList.GetStartPosition();  
   
   pArchive << lNbSound;

   while( lPos != NULL )
   {
      int                    lKey;
      MR_ResContinuousSound* lValue;

      mContinuousSoundList.GetNextAssoc( lPos, lKey, lValue );

      pArchive << lKey;      
      lValue->Serialize( pArchive );
   }  


}

