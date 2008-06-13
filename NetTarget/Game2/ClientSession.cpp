// ClientSession.cpp
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

#include "ClientSession.h"
#include "../MazeCompiler/TrackCommonStuff.h"

MR_ClientSession::MR_ClientSession() 
				:mSession(TRUE) {
   mMainCharacter1      = NULL;
   mMainCharacter2      = NULL;
   mMainCharacter3      = NULL;
   mMainCharacter4      = NULL;
   mBackImage           = NULL;
   mMap                 = NULL;
   mNbLap               = 1;
   mAllowWeapons        = TRUE;

   InitializeCriticalSection(&mChatMutex);
}

MR_ClientSession::~MR_ClientSession()
{
   delete []mBackImage;
   delete mMap;

   DeleteCriticalSection( &mChatMutex );
}

void MR_ClientSession::Process(int pSpeedFactor) {
   mSession.Simulate();
}

void MR_ClientSession::ReadLevelAttrib( MR_RecordFile* pRecordFile, MR_VideoBuffer* pVideo )
{
   // Read level background palette
   if( (pVideo != NULL)&&(pRecordFile->GetNbRecords()>=3 ) )
   {
      pRecordFile->SelectRecord( 2 );

      {
         CArchive lArchive( pRecordFile, CArchive::load|CArchive::bNoFlushOnDelete );

         int lImageType;

         lArchive >> lImageType;

         if( lImageType == MR_RAWBITMAP )
         {
            MR_UInt8* lPalette = new MR_UInt8[ MR_BACK_COLORS*3 ];
         
            if( mBackImage == NULL )
            {
               mBackImage = new MR_UInt8[ MR_BACK_X_RES*MR_BACK_Y_RES ];
            }


            lArchive.Read( lPalette, MR_BACK_COLORS*3 );
            lArchive.Read( mBackImage, MR_BACK_X_RES*MR_BACK_Y_RES );

            pVideo->SetBackPalette( lPalette );
         }
      }
   }

   // Read map section
   if( pRecordFile->GetNbRecords()>=4 )
   {
      pRecordFile->SelectRecord( 3 );
      {
         CArchive lArchive( pRecordFile, CArchive::load|CArchive::bNoFlushOnDelete );

         int          lX0;
         int          lX1;
         int          lY0;
         int          lY1;

         MR_Sprite* lMapSprite = new MR_Sprite;

         lArchive >> lX0;
         lArchive >> lX1;
         lArchive >> lY0;
         lArchive >> lY1;

         lMapSprite->Serialize( lArchive );

         SetMap( lMapSprite, lX0, lY0, lX1, lY1 );
      }
   }


   // Read level midi stream
   if( pRecordFile->GetNbRecords()>=5 )
   {
      pRecordFile->SelectRecord( 4 );
      {
         // TODO
      }
   }
}



BOOL MR_ClientSession::LoadNew( const char* pTitle, MR_RecordFile* pMazeFile, int pNbLap, BOOL pAllowWeapons, MR_VideoBuffer* pVideo )
{
   BOOL lReturnValue;
   mNbLap        = pNbLap;
   mAllowWeapons = pAllowWeapons; 
   lReturnValue  = mSession.LoadNew( pTitle, pMazeFile );

   if( lReturnValue )
   {
      ReadLevelAttrib( pMazeFile, pVideo );
   }

   return lReturnValue;
}

const MR_UInt8* MR_ClientSession::GetBackImage()const
{
   return mBackImage;
}


// Main character controll and interogation
BOOL MR_ClientSession::CreateMainCharacter()
{

   // Add a main character in 
   
   ASSERT( mMainCharacter1 == NULL ); // why creating it twice?
   ASSERT( mSession.GetCurrentLevel() != NULL );

   mMainCharacter1 = MR_MainCharacter::New( mNbLap, mAllowWeapons );

   // Insert the character in the current level
   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();
      
   mMainCharacter1->mRoom        = lCurrentLevel->GetStartingRoom( 0 ); 
   mMainCharacter1->mPosition    = lCurrentLevel->GetStartingPos( 0 );
   mMainCharacter1->SetOrientation( lCurrentLevel->GetStartingOrientation( 0 ));
   mMainCharacter1->SetHoverId( 0 );

   lCurrentLevel->InsertElement( mMainCharacter1, lCurrentLevel->GetStartingRoom( 0 ) );

   // Insert two dummy (TEST)
   /*
   MR_MainCharacter* lMainCharacter;

   lMainCharacter = MR_MainCharacter::New();
   lMainCharacter->SetAsSlave();      
 
   lMainCharacter->mPosition    = lCurrentLevel->GetStartingPos( 1 );
   lMainCharacter->mOrientation = lCurrentLevel->GetStartingOrientation( 1 );
   lCurrentLevel->InsertElement( lMainCharacter, lCurrentLevel->GetStartingRoom( 1 ) );


   lMainCharacter = MR_MainCharacter::New();
   lMainCharacter->SetAsSlave();      
 
   lMainCharacter->mPosition    = lCurrentLevel->GetStartingPos( 2 );
   lMainCharacter->mOrientation = lCurrentLevel->GetStartingOrientation( 2 );
   lCurrentLevel->InsertElement( lMainCharacter, lCurrentLevel->GetStartingRoom( 2 ) );
   */

   return TRUE;
}

BOOL MR_ClientSession::CreateMainCharacter2()
{

   // Add a main character in 
   
   ASSERT( mMainCharacter2 == NULL ); // why creating it twice?
   ASSERT( mSession.GetCurrentLevel() != NULL );

   mMainCharacter2 = MR_MainCharacter::New( mNbLap, mAllowWeapons );

   // Insert the character in the current level
   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();

   mMainCharacter2->mRoom        = lCurrentLevel->GetStartingRoom( 1 );      
   mMainCharacter2->mPosition    = lCurrentLevel->GetStartingPos( 1 );
   mMainCharacter2->SetOrientation( lCurrentLevel->GetStartingOrientation( 1 ) );
   mMainCharacter2->SetHoverId( 1 );

   lCurrentLevel->InsertElement( mMainCharacter2, lCurrentLevel->GetStartingRoom( 1 ) );

   return TRUE;
}

BOOL MR_ClientSession::CreateMainCharacter3()
{

   // Add a main character in 
   
   ASSERT( mMainCharacter3 == NULL ); // why creating it twice?
   ASSERT( mSession.GetCurrentLevel() != NULL );

   mMainCharacter3 = MR_MainCharacter::New( mNbLap, mAllowWeapons );

   // Insert the character in the current level
   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();

   mMainCharacter3->mRoom        = lCurrentLevel->GetStartingRoom( 2 );      
   mMainCharacter3->mPosition    = lCurrentLevel->GetStartingPos( 2 );
   mMainCharacter3->SetOrientation( lCurrentLevel->GetStartingOrientation( 2 ) );
   mMainCharacter3->SetHoverId( 2 );

   lCurrentLevel->InsertElement( mMainCharacter3, lCurrentLevel->GetStartingRoom( 2 ) );

   return TRUE;
}

BOOL MR_ClientSession::CreateMainCharacter4()
{

   // Add a main character in 
   
   ASSERT( mMainCharacter4 == NULL ); // why creating it twice?
   ASSERT( mSession.GetCurrentLevel() != NULL );

   mMainCharacter4 = MR_MainCharacter::New( mNbLap, mAllowWeapons );

   // Insert the character in the current level
   MR_Level* lCurrentLevel = mSession.GetCurrentLevel();

   mMainCharacter4->mRoom        = lCurrentLevel->GetStartingRoom( 3 );      
   mMainCharacter4->mPosition    = lCurrentLevel->GetStartingPos( 3 );
   mMainCharacter4->SetOrientation( lCurrentLevel->GetStartingOrientation( 3 ) );
   mMainCharacter4->SetHoverId( 3 );

   lCurrentLevel->InsertElement( mMainCharacter4, lCurrentLevel->GetStartingRoom( 3 ) );

   return TRUE;
}

MR_MainCharacter*  MR_ClientSession::GetMainCharacter()const      
{
   return mMainCharacter1; 
}

MR_MainCharacter*  MR_ClientSession::GetMainCharacter2()const      
{
   return mMainCharacter2; 
}

MR_MainCharacter*  MR_ClientSession::GetMainCharacter3()const      
{
   return mMainCharacter3; 
}

MR_MainCharacter*  MR_ClientSession::GetMainCharacter4()const      
{
   return mMainCharacter4; 
}

void MR_ClientSession::SetSimulationTime( MR_SimulationTime pTime )
{
   mSession.SetSimulationTime( pTime );
}

MR_SimulationTime MR_ClientSession::GetSimulationTime( )const
{
   return mSession.GetSimulationTime();
}

void MR_ClientSession::SetControlState( int pState1, int pState2, int pState3, int pState4 )
{
   if( mMainCharacter1 != NULL )
   {
      mMainCharacter1->SetControlState( pState1, mSession.GetSimulationTime() );
   }

   if( mMainCharacter2 != NULL )
   {
      mMainCharacter2->SetControlState( pState2, mSession.GetSimulationTime() );
   }

   if( mMainCharacter3 != NULL )
   {
      mMainCharacter3->SetControlState( pState3, mSession.GetSimulationTime() );
   }

   if( mMainCharacter4 != NULL )
   {
      mMainCharacter4->SetControlState( pState4, mSession.GetSimulationTime() );
   }
}

const MR_Level* MR_ClientSession::GetCurrentLevel()const
{
   MR_GameSession* lSession = (MR_GameSession*)&mSession;

   return lSession->GetCurrentLevel();
}


int MR_ClientSession::ResultAvaillable()const
{
   return 0;
}

void MR_ClientSession::GetResult( int, const char*& pPlayerName, int&, BOOL&, int&, MR_SimulationTime&, MR_SimulationTime& )const
{
   pPlayerName = "?";
   ASSERT( FALSE );
}

void MR_ClientSession::GetHitResult( int pPosition, const char*& pPlayerName, int& pId, BOOL& pConnected, int& pNbHitOther, int& pNbHitHimself )const
{
   pPlayerName = "?";
   ASSERT( FALSE );
}

int MR_ClientSession::GetNbPlayers()const
{
   BOOL lReturnValue = 0;

   if( mMainCharacter1 != NULL )
   {
      lReturnValue++;
   }
   if( mMainCharacter2 != NULL )
   {
      lReturnValue++;
   }
   if( mMainCharacter3 != NULL )
   {
      lReturnValue++;
   }
   if( mMainCharacter4 != NULL )
   {
      lReturnValue++;
   }
   return lReturnValue;
}

int MR_ClientSession::GetRank( const MR_MainCharacter* pPlayer )const
{
   int lReturnValue = 1;

   if( mMainCharacter1 != NULL )
   {
      if( pPlayer == mMainCharacter1 )
      {
         if( mMainCharacter2->HasFinish() )
         {
            if( mMainCharacter2->GetTotalTime() <  mMainCharacter1->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter3->HasFinish() )
         {
            if( mMainCharacter3->GetTotalTime() <  mMainCharacter1->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter4->HasFinish() )
         {
            if( mMainCharacter4->GetTotalTime() <  mMainCharacter1->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
      }   
      if (pPlayer == mMainCharacter2)
      {
         lReturnValue = 1;

         if( mMainCharacter1->HasFinish() )
         {
            if( mMainCharacter1->GetTotalTime() <  mMainCharacter2->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter3->HasFinish() )
         {
            if( mMainCharacter3->GetTotalTime() <  mMainCharacter2->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter4->HasFinish() )
         {
            if( mMainCharacter4->GetTotalTime() <  mMainCharacter2->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
      }
	  if (pPlayer == mMainCharacter3)
      {
         lReturnValue = 1;

         if( mMainCharacter1->HasFinish() )
         {
            if( mMainCharacter1->GetTotalTime() <  mMainCharacter3->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter2->HasFinish() )
         {
            if( mMainCharacter2->GetTotalTime() <  mMainCharacter3->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter4->HasFinish() )
         {
            if( mMainCharacter4->GetTotalTime() <  mMainCharacter3->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
      }
	  if (pPlayer == mMainCharacter4)
      {
         lReturnValue = 1;

         if( mMainCharacter1->HasFinish() )
         {
            if( mMainCharacter1->GetTotalTime() <  mMainCharacter4->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter2->HasFinish() )
         {
            if( mMainCharacter2->GetTotalTime() <  mMainCharacter4->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
		 if( mMainCharacter3->HasFinish() )
         {
            if( mMainCharacter3->GetTotalTime() <  mMainCharacter4->GetTotalTime() )
            {
               lReturnValue++;
            }
         }
      }
   }
   return lReturnValue;
}

void MR_ClientSession::SetMap( MR_Sprite* pMap, int pX0, int pY0, int pX1, int pY1 )
{
   delete mMap;

   mMap = pMap;

   mX0Map = pX0;
   mY0Map = pY0;
   mWidthMap = pX1-pX0;
   mHeightMap = pY1-pY0;

   mWidthSprite   = mMap->GetItemWidth();
   mHeightSprite  = mMap->GetItemHeight();

}

const MR_Sprite* MR_ClientSession::GetMap()const
{
   return mMap;
}

void MR_ClientSession::ConvertMapCoordinate( int& pX, int& pY, int pRatio )const
{
   pX = ( pX-mX0Map )*mWidthSprite/ (mWidthMap*pRatio);
   pY = (mHeightSprite-1-( pY-mY0Map )*mHeightSprite/mHeightMap)/pRatio;
}

const MR_MainCharacter* MR_ClientSession::GetPlayer( int pPlayerIndex )const
{
   const MR_MainCharacter* lReturnValue = NULL;

   switch( pPlayerIndex )
   {
      case 0:
         lReturnValue = mMainCharacter1;
         break;

      case 1:
         lReturnValue = mMainCharacter2;
         break;

	  case 2:
         lReturnValue = mMainCharacter3;
         break;

	  case 3:
         lReturnValue = mMainCharacter3;
         break;
   }
   ASSERT( lReturnValue != NULL );

   return lReturnValue;
}

void MR_ClientSession::AddMessageKey( char /*pKey*/ )
{

}

void MR_ClientSession::GetCurrentMessage( char* pDest )const
{
   pDest[0] = 0;
}

BOOL MR_ClientSession::GetMessageStack( int pLevel, char* pDest, int pExpiration )const
{
   BOOL lReturnValue = FALSE;


   if( pLevel < MR_CHAT_MESSAGE_STACK )
   {
      EnterCriticalSection( &((MR_ClientSession*)this)->mChatMutex );

      if( ((mMessageStack[ pLevel ].mCreationTime+pExpiration) > time( NULL ))&&(mMessageStack[ pLevel ].mBuffer.GetLength()>0) )
      {
         lReturnValue = TRUE;
         strcpy( pDest, mMessageStack[ pLevel ].mBuffer );
      }
      LeaveCriticalSection( &((MR_ClientSession*)this)->mChatMutex );
   }   

   return lReturnValue;
}

void MR_ClientSession::AddMessage( const char* pMessage )
{
   EnterCriticalSection( &mChatMutex );

   for( int lCounter = MR_CHAT_MESSAGE_STACK -1; lCounter > 0 ; lCounter-- )
   {
      mMessageStack[ lCounter ] =  mMessageStack[ lCounter-1 ];
   }

   mMessageStack[0].mCreationTime = time(NULL);

   mMessageStack[0].mBuffer  = Ascii2Simple( pMessage );

   LeaveCriticalSection( &mChatMutex );

}

