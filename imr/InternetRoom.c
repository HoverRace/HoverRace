/*
// InternetRoom.cpp

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
//

// HoverRace Internet meeting room for Unix or Win32.
// Need some adjustements to run on Win32..specially if using IPC
//
// This is a CGI application 
//
// Query form (version 1)
//   URL?=REFRESH USER_ID TIME_STAMP
//   URL?=ADD_CHAT USER_ID MESSAGE
//   URL?=ADD_USER MAJOR-MINORID VERSION KEY2 KEY3 ALIAS
//   URL?=ADD_GAME USER_ID GAME_NAME TRACK_NAME NBLAP WEAPON PORT   note IP-ADDR fetch from REMOTE_ADDR env
//   URL?=JOIN_GAME GAME_ID USER_ID
//   URL?=DEL_GAME GAME_ID USER_ID
//   URL?=LEAVE_GAME GAME_ID USER_ID
//   URL?=DEL_USER USER_ID
//   URL?=START_GAME GAME_ID USER_ID
//   URL?=MESSAGE USER_ID TIME_LEFT:MINUTE_LEFT Message
//
//   USER_ID have the form INT-UNSIGNED  (INDEX-UNIQUE_ID)
//   GAME_ID have the form INT-UNSIGNED  (INDEX-UNIQUE_ID)
//
// ReturnValue have the followinf format
//    SUCCESS\nDATA
//    or  
//    ERROR ERROR_CODE\n
//    
// ERROR_CODE
//   100 Unable to add user??
//   101 No more Shareware user allowed
//   102 No more user allowed
//   103 Incompatible version
//   104 Expired key
//   105 Already used key  (report for investigation)
//   106 Reserved Alias name (choose an other one)
//
//   200 Unable to send refresh info ??
//   201 Not on-line
//   
//   300 Unable to add chat message ???
//   301 Not on-line
//   
//   400 Unable to add game ???
//   401 Not on-line
//   402 No more entry avail
//
//   500 Unable to join Game
//   501 Not On-Line
//   502 Game no more availlable
//   503 Game Full
//
//   600 Unable to delete Game
//   601 Not on-line
//   602 Game do not exist
//   603 Not Owner
//
//   700 Unable to leave game ???
//   701 Not on-line
//   702 Game do not exist
//   703 Must join first
//
//   800 Unable to delete user
//   801 Not on-line
//
//   900 Unable to start Game ???
//   901 Not on-line
//   902 Game do not exist
//   903 Not Owner
//
//  1000 Unable to add message
//  1001 Not on-line
//  1002 Not authorized

*/

/* #define _FAST_CGI_ */
/* #define _NO_IPC_    //use only with _FAST_CGI */
/* #define _EXPIRED_WARNING_ */
/* #define _EXPIRED_ */
/* #define _REG_ONLY_ */


#ifdef _FAST_CGI_
   #include "/usr/local/fcgi/include/fcgi_stdio.h"
#else
   // #include <stdio.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>


// Required section to compile on Windows
#ifdef WIN32

#include <direct.h>
#include <io.h>

#ifndef _NO_IPC_

struct sembuf
{
   unsigned short sem_num;
   short          sem_op;
   short          sem_flg;
};


union semun 
{
   int val;
   // struct semid_ds *buf;
   unsigned short *array;
};
 

int   semget( int, int, int );
int   semop( int, struct sembuf*, int );
int   semctl( int, int, int, ... );
int   shmget( int, int, int );
char* shmat( int, char*, int );
int   shmdt( char* );
int   shmctl( int, int, void* );
int   fork();
int   sleep( int );

#define IPC_CREAT 1024
#define IPC_EXCL  2048
#define IPC_RMID  4096
#define SETVAL 16

#endif

#else 

#ifndef _NO_IPC_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#endif


#include <unistd.h>


#endif


#ifndef BOOL 
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// BETA ROOM
// #define IR_SEM_KEY         71327
// #define IR_SHMEM_KEY       71327

// Main Room
#define IR_SEM_KEY         71329
#define IR_SHMEM_KEY       71329


#define IR_USER_NAME_LEN     30
#define IR_GAME_NAME_LEN     32 
#define IR_TRACK_NAME_LEN    32
#define IR_CHAT_MESSAGE_LEN 200
#define IR_MAX_PLAYER_GAME    8

#define IR_MAX_CLIENT        150
#define IR_MAX_GAME           30
#define IR_MAX_CHAT_MESSAGE   64

#define IR_EXPIRATION_DELAY   50  /*assumed not logued after 50 sec. */
#define IR_CLEANUP_DELAY      10  /* Test expiration every 10 sec */

#define IR_VERSION_MIN         1
#define IR_VERSION_MAX         1

#define IR_MAX_USER_IP        16  /*IP ADDR field len */ 
#define IR_MAX_USER_DOM       30  /*Domain field len */

#define IR_BANNEDSLOT       1000  /*Reserved ID for refused login */


struct UserID
{
   int mMajor;
   int mMinor;
};

/* Hardcoded banned ip */
const char* gBannedISP_IP[] =
{
   NULL
};

/* Hardcoded banned users */
const struct UserID gPermBannedUser[] =
{
   { -1, -1 }
};

const struct UserID gSUList[] =
{
   {  0,  0 },  /* Root account */
   { -1, -1 }
};



/* Structure definition */
 
typedef struct User
{
   int    mTimeStamp;
   time_t mExpiration;
   int    mId;

   struct UserID mKeyID;
   char          mName[ IR_USER_NAME_LEN+1 ]; /* Alias */


   /* Loging data */
   time_t mLogingTime;
   int    mGameCode;
   int    mNbRefresh;
   int    mNbChat;
   char   mIP[ IR_MAX_USER_IP+1 ];
   char   mDomain[ IR_MAX_USER_DOM+1 ];         

};

typedef struct Game
{
   BOOL   mUsed;
   BOOL   mStarted;
   int    mTimeStamp;
   int    mId;

   char      mName[  IR_GAME_NAME_LEN+1 ];
   char      mTrack[ IR_TRACK_NAME_LEN+1 ];
   int       mNbLap;
   int       mNbPlayer;
   int       mPlayers[ IR_MAX_PLAYER_GAME ];
   char      mAddr[16];
   unsigned  mPort;
   int       mWeapon;
};

typedef struct ChatMessage
{
   int  mTimeStamp;
   char mData[IR_CHAT_MESSAGE_LEN];
};

typedef struct IRState
{
   time_t       mLastCleanUp;
   int          mTimeStamp;
   unsigned int mNextUserId;
   unsigned int mNextGameId;

   User mUser[ IR_MAX_CLIENT ];
   Game mGame[ IR_MAX_GAME ];

   int         mChatTail;
   ChatMessage mChatFifo[IR_MAX_CHAT_MESSAGE];
};

/* protected stuff */
void PrintState( IRState* pThis );
void RefreshUser( IRState* pThis , int pUser );
void RemoveUser( IRState* pThis , int pUser, BOOL pTimeOut );

BOOL ValidUserId( IRState* pThis , int pUserIndex, int pUserId )const;
BOOL ValidGameId( IRState* pThis , int pGameIndex, int pGameId )const;

/*public stufff */      
void Clear( IRState* pThis );
void Read(  IRState* pThis, FILE* pFile );
void Write( IRState* pThis , FILE* pFile );

void VerifyExpirations( IRState* pThis );           /* Return TRUE if a modification have been made */

/* These actions print a return value */

void PrintStateChange( IRState* pThis , int pPlayerIndex, int pPlayerId, int pTimeStamp  ); /* Refresh */
void AddChatMessage( IRState* pThis , int pUserIndex, int pUserId, const char* pMessage );
void AddUser( IRState* pThis, const char* pUserName, int pVersion, int pMajorID, int pMinorID, unsigned int pKey2, unsigned int pKey3  );
void AddGame( IRState* pThis, const char* pGameName, const char* pTrackName, int pNbLap, int pPlayerIndex, int pPlayerId, const char* pRemoteAddr, unsigned int pPort, int pWeapon );
void JoinGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId );
void DeleteGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId );
void LeaveGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId  );
void DeleteUser( IRState* pThis, int pUserIndex, int pUserId  );
void StartGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId );




/* Local variable */
FILE* gLogFile = NULL;

#ifdef _NO_IPC_

   IRState gGlobalState;

#endif

/* Local functions */
static int  StrMaxCopy( char* pDest, const char* pSrc, int pDestSize );
static BOOL StrCompare( const char* pStr, const char* pToken );
static void Unpad( char* pStr );
static void UnpadQuery( char* pStr );

static BOOL InitLogFile();
static void CloseLogFile();
static const char* GetTime( time_t pTime );

static const char* chk( const char* pSrc );

/* Implementation */

void Clear( IRState* pThis )
{
   int lCounter;

   pThis->mLastCleanUp = time( NULL );
   pThis->mTimeStamp = 1;

   for( lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++ )
   {
      pThis->mUser[lCounter].mExpiration = 0;
      pThis->mUser[lCounter].mTimeStamp = 0;
   }

   for( lCounter = 0; lCounter < IR_MAX_GAME; lCounter++ )
   {
      pThis->mGame[lCounter].mUsed = 0;
      pThis->mUser[lCounter].mTimeStamp = 0;
   }

   pThis->mChatTail = 0;
   pThis->mNextUserId = 1;
   pThis->mNextGameId = 1;

   for( lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++ )
   {
      pThis->mChatFifo[lCounter].mTimeStamp = 0;
   }
}

void Read( IRState* pThis, FILE* pFile )
{
   fseek( pFile, 0, SEEK_SET );
   fread( this, sizeof(IRState), 1, pFile );
}

void IRState::Write( IRState* pThis, FILE* pFile )
{
   fseek( pFile, 0, SEEK_SET );
   fwrite( this, sizeof(IRState), 1, pFile );
}

BOOL ValidUserId( IRState* pThis, int pUserIndex, int pUserId )const
{
   BOOL lReturnValue = FALSE;

   if( (pUserIndex >= 0) && (pUserIndex<IR_MAX_CLIENT) )
   {
      if( (pThis->mUser[pUserIndex].mExpiration != 0)&&(pThis->mUser[pUserIndex].mId == pUserId ) )
      {
         lReturnValue = TRUE;
      }
   }
   return lReturnValue;
}

BOOL ValidGameId( IRState* pThis, int pGameIndex, int pGameId )const
{
   BOOL lReturnValue = FALSE;

   if( (pGameIndex >= 0) && (pGameIndex<IR_MAX_GAME) )
   {
      if( ( pThis->mGame[pGameIndex].mUsed)&&( pThis->mGame[pGameIndex].mId == pGameId ) )
      {
         lReturnValue = TRUE;
      }
   }
   return lReturnValue;
}

void IRState::VerifyExpirations()
{
   int  lCounter;

   time_t lCurrentTime = time( NULL );

   if( ( pThis->mLastCleanUp+IR_CLEANUP_DELAY)<=lCurrentTime )
   {
      for( lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++ )
      {
         if( (pThis->mUser[lCounter].mExpiration != 0)&&(pThis->mUser[lCounter].mExpiration < lCurrentTime) )
         {
            RemoveUser( pThis, lCounter, TRUE );
         }
      }
      pThis->mLastCleanUp = lCurrentTime;
   } 
}

void RemoveUser( IRState* pThis, int pUser, BOOL pExpired )
{
   pThis->mUser[pUser].mExpiration = 0;
   pThis->mUser[pUser].mTimeStamp  = pThis->mTimeStamp;

   /* Log this user entry */
   if( InitLogFile() )
   {
      fprintf( gLogFile, "%s %d %c %d %d %d ^%s^(%d-%d) %s %s\n",
               GetTime( pThis->mUser[pUser].mLogingTime ),
               time( NULL )-pThis->mUser[pUser].mLogingTime,
               pExpired? 'o':'t',
               pThis->mUser[pUser].mGameCode,
               pThis->mUser[pUser].mNbRefresh,
               pThis->mUser[pUser].mNbChat,
               pThis->mUser[pUser].mName,
               pThis->mUser[pUser].mKeyID.mMajor,
               pThis->mUser[pUser].mKeyID.mMinor,
               pThis->mUser[pUser].mIP,
               pThis->mUser[pUser].mDomain                     );
   }


   /* Verify the games that the user take parts to */
   for( int lGame = 0; lGame < IR_MAX_GAME; lGame++ )
   {
      if( pThis->mGame[ lGame ].mUsed )
      {
         if( pThis->mGame[ lGame ].mPlayers[0] == pUser )
         {
            pThis->mGame[ lGame ].mUsed = FALSE;
            pThis->mGame[ lGame ].mTimeStamp = mTimeStamp;
         }
         else
         {
            for( int lPlayer = 1; lPlayer < pThis->mGame[ lGame ].mNbPlayer; lPlayer++ )
            {
               if( pThis->mGame[ lGame ].mPlayers[lPlayer] == pUser )
               {
                  pThis->mGame[ lGame ].mNbPlayer--;
                  pThis->mGame[ lGame ].mTimeStamp = mTimeStamp;

                  int lNbLeft = lPlayer-mGame[ lGame ].mNbPlayer;

                  if( lNbLeft > 0 )
                  {
                     memmove( &( pThis->mGame[ lGame ].mPlayers[lPlayer]), &(mGame[ lGame ].mPlayers[lPlayer+1]), sizeof( (mGame[ lGame ].mPlayers[lPlayer]) )*lNbLeft );
                  }
               }
            }
         }
      }
   }

   pThis->mTimeStamp++;
}


void RefreshUser( IRState* pThis, int pUser )
{
   time_t lTime = time( NULL );

   pThis->mUser[ pUser ].mExpiration = lTime+IR_EXPIRATION_DELAY;
   pThis->mUser[ pUser ].mNbRefresh++;

}

void PrintState( IRState* pThis )
{
   int     lCounter;

   /* Print current time-stamp */
   printf( "TIME_STAMP %d\n", mTimeStamp );

   /* Send the users list */
   for( lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++ )
   {
      if( mUser[ lCounter ].mExpiration != 0 )
      {
         printf( "USER %d NEW\n", lCounter );
         printf( "%d-%d\n", pThis->mUser[ lCounter ].mKeyID.mMajor, pThis->mUser[ lCounter ].mKeyID.mMinor );
         printf( "%s\n", pThis->mUser[ lCounter ].mName );
      }
   }

   /* Send the game list */
   for( lCounter = 0; lCounter < IR_MAX_GAME; lCounter++ )
   {
      if( pThis->mGame[ lCounter ].mUsed )
      {
         printf( "GAME %d NEW %u\n", lCounter, pThis->mGame[ lCounter ].mId );
         printf( "%s\n", pThis->mGame[ lCounter ].mName );
         printf( "%s\n", pThis->mGame[ lCounter ].mTrack );
         printf( "%s\n", pThis->mGame[ lCounter ].mAddr );
         printf( "%u %d %d %d\n", pThis->mGame[ lCounter ].mPort, pThis->mGame[ lCounter ].mNbLap, mGame[ lCounter ].mWeapon, pThis->mGame[ lCounter ].mNbPlayer );

         for( int lPlayer = 0; lPlayer < pThis->mGame[ lCounter ].mNbPlayer; lPlayer++ )
         {
            printf( "%d ", mGame[ lCounter ].mPlayers[ lPlayer ] );
         }
         printf( "\n" );
      }
   }

   /* Add some custom chat message */

   #ifdef _EXPIRED_WARNING_
         printf( "CHAT\n--\n" ); /* just a line to make things more clear */
         printf( "CHAT\nThis HoverRace version is now expired\n" );
         printf( "CHAT\nPlease download the new version from www.grokksoft.com\n" );
         printf( "CHAT\n--\n" ); /* just a line to make things more clear */
   #endif

   /* Send the chat messages */
   /* Old messages are not sended
   for( lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++ )
   {
      int lIndex = (lCounter+mChatTail)%IR_MAX_CHAT_MESSAGE;

      if( mChatFifo[lIndex].mTimeStamp != 0 )
      {
         printf( "CHAT\n%s\n", mChatFifo[ lIndex ].mData );
      }
   }
   */

}

void PrintStateChange( IRState* pThis, int pPlayerIndex, int pPlayerId, int pTimeStamp  )
{

   if( !ValidUserId( pThis, pPlayerIndex, pPlayerId ) )
   {
      if( pPlayerIndex == IR_BANNEDSLOT )
      {
         /* prevent the user from deloging..and loosing the important message */
         printf( "SUCCESS\n" );
      }
      else
      {
         printf( "ERROR 201\n" );
      }
   }
   else
   {
      int     lCounter;

      RefreshUser( pThis, pPlayerIndex );

      printf( "SUCCESS\n" );

      if( pTimeStamp == pThis->mTimeStamp )
      {
         /* notting changed */
         return;
      }

      /* Print current time-stamp */
      printf( "TIME_STAMP %d\n", pThis->mTimeStamp );

      /* Send the users that have been modified */
      for( lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++ )
      {
         if( (pThis->mUser[lCounter].mTimeStamp != 0)&&(pThis->mUser[lCounter].mTimeStamp >= pTimeStamp) )
         {
            if( pThis->mUser[ lCounter ].mExpiration == 0 )
            {
               // Expired or deleted
               printf( "USER %d DEL\n", lCounter );
            }
            else
            {
               printf( "USER %d NEW\n", lCounter );
               printf( "%d-%d\n", mUser[ lCounter ].mKeyID.mMajor, mUser[ lCounter ].mKeyID.mMinor );
               printf( "%s\n", mUser[ lCounter ].mName );
            }
         }
      }

      /* Send the games that have been modified */
      for( lCounter = 0; lCounter < IR_MAX_GAME; lCounter++ )
      {
         if( (pThis->mGame[lCounter].mTimeStamp != 0)&&(pThis->mGame[lCounter].mTimeStamp >= pTimeStamp) )
         {
            if( !pThis->mGame[ lCounter ].mUsed )
            {
               /* Expired or deleted */
               printf( "GAME %d DEL\n", lCounter );
            }
            else
            {
               printf( "GAME %d NEW %u\n", lCounter, mGame[ lCounter ].mId );
               printf( "%s\n", mGame[ lCounter ].mName );
               printf( "%s\n", mGame[ lCounter ].mTrack );
               printf( "%s\n", mGame[ lCounter ].mAddr );
               printf( "%u %d %d %d\n", mGame[ lCounter ].mPort, mGame[ lCounter ].mNbLap, mGame[ lCounter ].mWeapon, mGame[ lCounter ].mNbPlayer );

               for( int lPlayer = 0; lPlayer < mGame[ lCounter ].mNbPlayer; lPlayer++ )
               {
                  printf( "%d ", mGame[ lCounter ].mPlayers[ lPlayer ] );
               }
               printf( "\n" );
            }
         }
      }

      /* Send the new chat messages */
      for( lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++ )
      {
         int lIndex = (lCounter+mChatTail)%IR_MAX_CHAT_MESSAGE;

         if( (mChatFifo[lIndex].mTimeStamp != 0)&&(mChatFifo[lIndex].mTimeStamp >= pTimeStamp) )
         {
            printf( "CHAT\n%s\n", mChatFifo[ lIndex ].mData );
         }
      }
   }
}

/*
// For the future
void IRState::AddSpecialMessage( pMessage )
{
   mChatFifo[ mChatTail ].mTimeStamp = mTimeStamp;
   
   StrMaxCopy( mChatFifo[ mChatTail ].mData, pMessage, IR_CHAT_MESSAGE_LEN );

   mChatTail = (mChatTail+1)%IR_MAX_CHAT_MESSAGE;

   mTimeStamp++;
}
*/

void AddChatMessage( IRState* pThis, int pUserIndex, int pUserId, const char* pMessage )
{
   if( !ValidUserId( pThis, pUserIndex, pUserId ) )
   {
      printf( "ERROR 301" );
   }
   else
   {
      printf( "SUCCESS\n" );

      if( pMessage[0] == '/' )
      {
         /* verify if the user have super user rights */
         if(  ( pThis->mUser[ pUserIndex ].mExpiration != 0)
            &&( pThis->mUser[ pUserIndex ].mKeyID.mMajor != -1) )
         {
            int  lSUIndex = 0;
            BOOL lOK = FALSE;

            while( !lOK && (gSUList[ lSUIndex ].mMajor != -1) )
            {
               if(   ( gSUList[ lSUIndex ].mMajor == pThis->mUser[ pUserIndex ].mKeyID.mMajor )
                   &&( gSUList[ lSUIndex ].mMinor == pThis->mUser[ pUserIndex ].mKeyID.mMinor ) )
               {
                  lOK = TRUE;
               }
               lSUIndex++;
            }

            if( !strncmp( pMessage, "/kick ", 5 ))
            {
               if( lOK )
               {

                  /* find the user to kick (start from the end this way there is more chance to catch the good one */
                  for( int lCounter = IR_MAX_CLIENT-1; lCounter >= 0; lCounter-- )
                  {
                     if( ( pThis->mUser[lCounter].mTimeStamp != 0)&&(mUser[ lCounter ].mExpiration != 0) )
                     {
                        if( !strcmp( pThis->mUser[ lCounter ].mName, pMessage+6 ))
                        {
                           /* Kick and quit */
                           if( InitLogFile() )
                           {
                              fprintf( gLogFile, "KICK %s(%d-%d) by  %s(%d-%d)\n",
                                       pThis->mUser[ lCounter ].mName,
                                       pThis->mUser[ lCounter ].mKeyID.mMajor,
                                       pThis->mUser[ lCounter ].mKeyID.mMinor,
                                       pThis->mUser[ pUserIndex ].mName,
                                       pThis->mUser[ pUserIndex ].mKeyID.mMajor,
                                       pThis->mUser[ pUserIndex ].mKeyID.mMinor );
                           }
                           RemoveUser( pThis, lCounter, TRUE );
                           break;
                        }
                     }
                  }
               }
               else
               {
               }
            }
         }
      }
      else
      {

         int lLen = 0;


         pThis->mChatFifo[ pThis->mChatTail ].mTimeStamp = pThis->mTimeStamp;
   
         if( pThis->mUser[ pUserIndex ].mExpiration != 0 )
         {
            lLen += StrMaxCopy( mChatFifo[ mChatTail ].mData, pThis->mUser[ pUserIndex ].mName , IR_CHAT_MESSAGE_LEN/4 );
            pThis->mChatFifo[ mChatTail ].mData[ lLen++ ] = '>';
            pThis->mChatFifo[ mChatTail ].mData[ lLen++ ] = ' ';
         }

         StrMaxCopy( pThis->mChatFifo[ mChatTail ].mData+lLen, pMessage, IR_CHAT_MESSAGE_LEN-lLen );

         pThis->mChatTail = (mChatTail+1)%IR_MAX_CHAT_MESSAGE;

         pThis->mTimeStamp++;

         pThis->mUser[ pUserIndex ].mNbChat++;
      }
   }
}


void AddUser(  IRState* pThis, const char* pUserName, int pVersion, int pMajorID, int pMinorID, unsigned int pKey2, unsigned int pKey3  )
{


   /* First verify the version */
   if( (pVersion<IR_VERSION_MIN)||(pVersion>IR_VERSION_MAX))
   {
      printf( "ERROR 103\n" );
   }
   else
   {
      int  lCounter;


      /* find an empty entry for the new user */
      for( lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++ )
      {
         if( pThis->mUser[ lCounter ].mExpiration == 0 )
         {   
            break;
         }
      }

      if( lCounter == IR_MAX_CLIENT )
      {
         printf( "ERROR 102\n" );      
      }
      else
      {
         BOOL lBanned    = FALSE;
         BOOL lExpired   = FALSE;
         BOOL lDoubleLog = FALSE;
         BOOL lNotReg    = FALSE;

         int lIPIndex = 0;

         if( pMajorID == -1 )
         {
            while( !lBanned && (gBannedISP_IP[ lIPIndex ] != NULL) )
            {
               if( !strncmp( gBannedISP_IP[lIPIndex], chk(getenv( "REMOTE_ADDR" )), strlen(gBannedISP_IP[lIPIndex])) )
               {
                  lBanned = TRUE;
               }
               lIPIndex++;
            }
         }

         #ifdef _REG_ONLY_
         if( pMajorID == -1 )
         {
            lNotReg = TRUE;
         }
         #endif

         /* Verify if the user is not already logued */
         if( pMajorID != -1 )
         {
            /* Go thru user list */
            for( int lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++ )
            {
               if( (pThis->mUser[lCounter].mTimeStamp != 0)&&(pThis->mUser[ lCounter ].mExpiration != 0) )
               {   
                  if( (pThis->mUser[lCounter].mKeyID.mMajor == pMajorID) && (pThis->mUser[lCounter].mKeyID.mMinor == pMinorID) )
                  {
                     lDoubleLog = TRUE;
                  }
               }
            }
         }


         #ifdef _EXPIRED_
            lExpired = TRUE;
         #endif

         if( !lBanned && !lExpired && !lDoubleLog && !lNotReg )
         {
            pThis->mUser[ lCounter ].mTimeStamp          = mTimeStamp;
            pThis->mUser[ lCounter ].mExpiration         = time( NULL )+IR_EXPIRATION_DELAY;
            pThis->mUser[ lCounter ].mId                 = mNextUserId++;
            pThis->mUser[lCounter].mKeyID.mMajor         = pMajorID;
            pThis->mUser[lCounter].mKeyID.mMinor         = pMinorID;

            StrMaxCopy( mUser[ lCounter ].mName, pUserName, IR_USER_NAME_LEN );
            pThis->mTimeStamp++;

            /* Fill loging fields */
            pThis->mUser[ lCounter ].mLogingTime = time( NULL );
            pThis->mUser[ lCounter ].mGameCode   = 0;
            pThis->mUser[ lCounter ].mNbRefresh  = 0;
            pThis->mUser[ lCounter ].mNbChat     = 0;
            StrMaxCopy( pThis->mUser[ lCounter ].mIP, chk(getenv( "REMOTE_ADDR" )), IR_MAX_USER_IP );
            StrMaxCopy( pThis->mUser[ lCounter ].mDomain, chk(getenv( "REMOTE_HOST" )) ,IR_MAX_USER_DOM );

 
            printf( "SUCCESS\n", lCounter );
            printf( "USER_ID %d-%u\n", lCounter, pThis->mUser[ lCounter ].mId );

            PrintState( pThis );
         }
         else
         {
            /* We got one */
            printf( "SUCCESS\n", lCounter );
            printf( "USER_ID %d-%u\n", IR_BANNEDSLOT, 0 );

            if( lExpired )
            {
               printf( "CHAT\n--\n" );
               printf( "CHAT\nThis version of HoverRace is expired\n" );
               printf( "CHAT\nPlease download the new version from www.grokksoft.com\n" );
               printf( "CHAT\n--\n" );

               /* Write this to the LOG */
               if( InitLogFile() )
               {
                  fprintf( gLogFile, "EXPIRED ^%s^ %d-%d %s\n",
                           pUserName,
                           pMajorID,
                           pMinorID,
                           chk(getenv( "REMOTE_ADDR" ))
                         );
               }
            }
            else if( lBanned )
            {
               printf( "CHAT\nYour ISP have been temporary banned from this gameroom because one of there customer was causing troubles\n" );
               printf( "CHAT\nWe have informed your ISP and we will reenable your access as soon as they will have corrected the situation\n" );
               printf( "CHAT\nYou can communicate with them to accelerate the process. Sorry for the inconvenients\n" );
               printf( "CHAT\nYou can also communicate with us support@grokksoft.com to get a special code that will let you access the game\n" );
               printf( "CHAT\nAlso, If you register the game you will be able to play since we can track registred users\n" );

               /* Write this to the LOG */
               if( InitLogFile() )
               {
                  fprintf( gLogFile, "BANNED ^%s^ %s\n",
                           pUserName,
                           chk(getenv( "REMOTE_ADDR" ))
                         );
               }
            }
            else if( lDoubleLog )
            {
               printf( "CHAT\nYour can not login because our database indicates that you are already logued in\n" );
               printf( "CHAT\nThis error may due to an error network. Retry to connect in a minute.\n" );
               printf( "CHAT\nIf it still dont works please contact support@grokksoft.com\n" );

               /* Write this to the LOG */
               if( InitLogFile() )
               {
                  fprintf( gLogFile, "DOUBLELOG ^%s^ %d-%d %s\n",
                           pUserName,
                           pMajorID,
                           pMinorID,                           
                           chk(getenv( "REMOTE_ADDR" ))
                         );
               }
            }
            else if( lNotReg )
            {
               printf( "CHAT\nYou must registrer to have access to this room\n" );
               printf( "CHAT\nSorry\n" );

               /* Write this to the LOG */
               if( InitLogFile() )
               {
                  fprintf( gLogFile, "UNREG ^%s^ %s\n",
                           pUserName,
                           chk(getenv( "REMOTE_ADDR" ))
                         );
               }
            }
         }
      }
   }
}


void AddGame( IRState* pThis, const char* pGameName, const char* pTrackName, int pNbLap, int pPlayerIndex, int pPlayerId, const char* pRemoteAddr, unsigned int pPort, int pWeapon )
{

   /* Verify the the user is valid */
   if( !ValidUserId( pThis, pPlayerIndex, pPlayerId ) )
   {
      printf( "ERROR 401\n" );
   }
   else
   {
      int  lCounter;

   
      /* find an empty entry for the new game */
      for( lCounter = 0; lCounter < IR_MAX_GAME; lCounter++ )
      {
         if( !pThis->mGame[ lCounter ].mUsed )
         {
            break;
         }
      }

      if( lCounter == IR_MAX_GAME )
      {
         printf( "ERROR 402\n" );
      }
      else
      {
         pThis->mGame[ lCounter ].mTimeStamp  = mTimeStamp;
         pThis->mGame[ lCounter ].mUsed       = TRUE;
         pThis->mGame[ lCounter ].mStarted    = FALSE;
         pThis->mGame[ lCounter ].mId         = pThis->mNextGameId++;
         pThis->mGame[ lCounter ].mNbLap      = pNbLap;
         pThis->mGame[ lCounter ].mNbPlayer   = 1;
         pThis->mGame[ lCounter ].mPlayers[0] = pPlayerIndex;
         pThis->mGame[ lCounter ].mWeapon     = pWeapon;
         pThis->mGame[ lCounter ].mPort       = pPort;

         StrMaxCopy( pThis->mGame[ lCounter ].mName, pGameName, IR_GAME_NAME_LEN );
         StrMaxCopy( pThis->mGame[ lCounter ].mTrack, pTrackName, IR_TRACK_NAME_LEN );
         StrMaxCopy( pThis->mGame[ lCounter ].mAddr, pRemoteAddr, sizeof( pThis->mGame[ lCounter ].mAddr ) );

         printf( "SUCCESS\nGAME_ID %d-%u\n", lCounter, pThis->mGame[ lCounter ].mId );
         pThis->mTimeStamp++;

         pThis->mUser[ pPlayerIndex ].mGameCode   = 1;

      }
   }
}



void JoinGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId )
{

   if( !ValidUserId( pThis, pPlayerIndex, pPlayerId ) )
   {
      printf( "ERROR 501\n" );
   }
   else if( !ValidGameId( pThis, pGameIndex, pGameId) )
   {
      printf( "ERROR 502\n" );
   }
   else if( (pThis->mGame[ pGameIndex ].mUsed)&&(pThis->mGame[ pGameIndex ].mNbPlayer>=IR_MAX_PLAYER_GAME) )
   {
      printf( "ERROR 503\n" );
   }
   else
   {
      pThis->mGame[ pGameIndex ].mTimeStamp  = pThis->mTimeStamp;
      pThis->mGame[ pGameIndex ].mPlayers[ pThis->mGame[ pGameIndex ].mNbPlayer++ ] = pPlayerIndex;

      printf( "SUCCESS\n" );
      pThis->mTimeStamp++;

      pThis->mUser[ pPlayerIndex ].mGameCode   = 2;

   }
}


void DeleteGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId )
{
   if( !ValidUserId( pThis, pPlayerIndex, pPlayerId ) )
   {
      printf( "ERROR 601\n" );
   }
   else if( !ValidGameId( pThis, pGameIndex, pGameId) )
   {
      printf( "ERROR 602\n" );
   }
   else if( pThis->mGame[ pGameIndex ].mPlayers[ 0 ] != pPlayerIndex )
   {
      printf( "ERROR 603\n" );
   }
   else
   {
      printf( "SUCCESS\n" );

      pThis->mGame[ pGameIndex ].mTimeStamp  = pThis->mTimeStamp;
      pThis->mGame[ pGameIndex ].mUsed = FALSE;
      pThis->mTimeStamp++;

      pThis->mUser[ pPlayerIndex ].mGameCode   = 3;

   }   
}

void LeaveGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId  )
{
   if( !ValidUserId( pThis, pPlayerIndex, pPlayerId ) )
   {
      printf( "ERROR 701\n" );
   }
   else if( !ValidGameId( pThis, pGameIndex, pGameId) )
   {
      printf( "ERROR 702\n" );
   }
   else
   {
      int lCounter;

      // Find player index
      for( lCounter = 1; lCounter < mGame[ pGameIndex ].mNbPlayer; lCounter++ )
      {
         if( pThis->mGame[ pGameIndex ].mPlayers[ lCounter ] == pPlayerIndex )
         {
            break;
         }
      }

      if( lCounter == pThis->mGame[ pGameIndex ].mNbPlayer )
      {
         printf( "ERROR 703\n" );
      }
      else
      {
         pThis->mGame[ pGameIndex ].mNbPlayer--;
         pThis->mGame[ pGameIndex ].mTimeStamp = mTimeStamp;

         int lNbLeft = lCounter-pThis->mGame[ pGameIndex ].mNbPlayer;

         if( lNbLeft > 0 )
         {
            memmove( &(mGame[ pGameIndex ].mPlayers[lCounter]), &(mGame[ pGameIndex ].mPlayers[lCounter+1]), sizeof( (mGame[ pGameIndex ].mPlayers[lCounter]) )*lNbLeft );
         }

         printf( "SUCCESS\n" );

         pThis->mTimeStamp++;

         pThis->mUser[ pPlayerIndex ].mGameCode   = 4;
      }
   }
}

void DeleteUser( IRState* pThis, int pUserIndex, int pUserId  )
{
   if( !ValidUserId( pThis, pUserIndex, pUserId ) )
   {
      printf( "ERROR 801\n" );
   }
   else
   {
      printf( "SUCCESS\n" );
      pThis->RemoveUser( pUserIndex, FALSE );
   }
}

void StartGame( IRState* pThis, int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId )
{
   if( !ValidUserId( pThis, pPlayerIndex, pPlayerId ) )
   {
      printf( "ERROR 901\n" );
   }
   else if( !ValidGameId( pThis, pGameIndex, pGameId) )
   {
      printf( "ERROR 902\n" );
   }
   else if( pThis->mGame[ pGameIndex ].mPlayers[ 0 ] != pPlayerIndex )
   {
      printf( "ERROR 903\n" );
   }
   else
   {
      printf( "SUCCESS\n" );

      pThis->mGame[ pGameIndex ].mStarted = TRUE;
      pThis->mTimeStamp++;
   }   
}










int main( int pArgc, const char** pArgs )
{

   #ifdef _NO_IPC_
 
      gGlobalState.Clear();
      if( InitLogFile() )
      {
         fprintf( gLogFile, "IMR Init version 0.13.13b %s\n", GetTime( time(NULL)) );
      }

   #endif


   #ifdef _FAST_CGI_
   while( FCGI_Accept() >= 0 )
   #endif
   {
      BOOL    lPrintTitle = TRUE;

      // Send the header required by the server
      printf("Content-type: text/plain%c%c", 10, 10);


      const char* lQueryPtr = getenv( "QUERY_STRING" );

      if( lQueryPtr == NULL )
      {
         printf( "No parameters\n" );
      }
      else
      {
         char lQuery[4096];
         char lOp[12];

         StrMaxCopy( lQuery, lQueryPtr, 4096 );

         UnpadQuery( lQuery );

         if( sscanf( lQuery, "=%11s", lOp ) == 1 )
         {
            #ifndef _FAST_CGI_
               // Change the local directory
               char* lPathEnd = strrchr( pArgs[0], '/' );

               if( lPathEnd != NULL )
               {
                  *lPathEnd = 0;
                  chdir( pArgs[0] );
               }
            #endif


            if( !strcmp( lOp, "RESET" ) )
            {
               printf( "RESET OK\n\n" );

               #ifdef _NO_IPC_
                  // gGlobalState.Clear();
                  // sleep( 2 );
                  // return 0;
                  // break;
               #else

                  int      lLock  = semget( IR_SEM_KEY, 1, 0777 );
                  int      lMemId = shmget( IR_SHMEM_KEY, sizeof( IRState ), 0666 );

                  if( lLock != -1 )
                  {
                     union semun lNULL;

                     semctl( lLock, 0, IPC_RMID, lNULL );
                  }

                  if( lMemId != -1 )
                  {
                     shmctl( lMemId, IPC_RMID, NULL );
                  }

               #endif

               if( InitLogFile() )
               {
                  fprintf( gLogFile, "IMR Reset %s\n", GetTime( time(NULL)) );
               }

               #ifdef _FAST_CGI_
                  break;
               #endif

            }
            else if( !strcmp( lOp, "SET_MESSAGE" ) )
            {
            }
            /*
            else if( !strcmp( lOp, "STICK" ) )
            {
               // Fork and leave a child running for 1 hour

               int lCode = fork();
               if( lCode == 0 )
               {
                  if( InitLogFile() )
                  {
                     fprintf( gLogFile, "STICK Start %s\n", GetTime( time(NULL)) );
                     CloseLogFile();
                  }

                  close( 0 );
                  close( 1 );
                  close( 2 );
                  sleep( 3600 );

                  if( InitLogFile() )
                  {
                     fprintf( gLogFile, "STICK End %s\n", GetTime( time(NULL)) );
                     CloseLogFile();
                  }

               }
               
               else if( lCode == -1 )
               {
                  printf( "ERROR\n" );
               }
               else
               {
                  printf( "SUCCESS\n" );
               }
               
            }
            */
            else
            {
               IRState* lState = NULL;

               #ifdef _NO_IPC_
                  lState = &gGlobalState;

               #else

                  int      lLock;        /* Semaphore */
                  int      lMemId;       /* Shared memory */

                  struct sembuf lSemOp;

                  lSemOp.sem_flg = 0;  /*Avoid corruption but must not core-dump SEM_UNDO;  // Risky but prevents dead-lock */
                  lSemOp.sem_num  = 0;

                  /* First try to create the structure for the first time */
                  /* Lock the data struct */
                  lLock = semget( IR_SEM_KEY, 1, 0777|IPC_CREAT|IPC_EXCL );

                  if( lLock != -1 )
                  {
                     union semun lArg;
               
                     /* Initialize the newly created semaphore */
                     lArg.val = 1;
 
                     semctl( lLock, 0, SETVAL, lArg );
                  }
                  else
                  {
                     lLock = semget( IR_SEM_KEY, 1, 0777 );
                  }

                  if( lLock == -1 )
                  {
                     printf( "Unable to get semaphore\n" );
                  }
                  else
                  {
                     lSemOp.sem_op   = -1;

                     if( semop( lLock, &lSemOp, 1 ) == -1 )
                     {
                        printf( "Unable to decrement semaphore\n" );
                        lLock = -1;
                     }
                     else
                     {

                        /* From here we can work safely */

                        lMemId = shmget( IR_SHMEM_KEY, sizeof( IRState ), 0666|IPC_CREAT|IPC_EXCL );

                        if( lMemId != -1 )
                        {
                           lState = (IRState*)shmat( lMemId, NULL, 0 );

                           if( (int)lState == -1 )
                           {
                              lState = NULL;
                           }

                           if( lState == NULL )
                           {
                              printf( "Unable to attach shmem\n" );
                           }
                           else
                           {
                              Clear( lState );

                              if( InitLogFile() )
                              {
                                 fprintf( gLogFile, "IMR Init %s\n", GetTime( time(NULL)) );
                              }

                           }
                        }
                        else
                        {
                           lMemId = shmget( IR_SHMEM_KEY, sizeof( IRState ), 0666 );

                           if( lMemId == -1 )
                           {
                              printf( "Unable to get shmem\n" );
                           }
                           else
                           {
                              lState = (IRState*)shmat( lMemId, NULL, 0 );

                              if( (int)lState == -1 )
                              {
                                 lState = NULL;
                              }

                              if( lState == NULL )
                              {
                                 printf( "Unable to attach shmem\n" );
                              }
                           }
                        }
                     }
                  }

               #endif
   
               if( lState != NULL )
               {

                  lPrintTitle = FALSE;

                  VerifyExpirations( lState );

                  if( !strcmp( lOp, "REFRESH" ) )
                  {
                     int lUserIndex;
                     int lUserId;
                     int lTimeStamp;

                     if( sscanf( lQuery, "%*s %d-%u %d", &lUserIndex, &lUserId, &lTimeStamp )==3 )
                     {
                        PrintStateChange( lState, lUserIndex, lUserId, lTimeStamp );
                     }
                  }
                  else if( !strcmp( lOp, "ADD_CHAT" ) )
                  {
                     int  lUserIndex;
                     int  lUserId;
                     char lChatMessage[200];

                     if( sscanf( lQuery, "%*s %d-%u %200s", &lUserIndex, &lUserId, lChatMessage )==3 )
                     {
                        Unpad( lChatMessage );
                        AddChatMessage( lState, lUserIndex, lUserId, lChatMessage );
                     }
                  }
                  //   URL?=ADD_USER MAJOR-MINORID VERSION KEY2 KEY3 ALIAS
                  else if( !strcmp( lOp, "ADD_USER" ) )
                  {
                     int lMajorID;
                     int lMinorID;
                     int  lVersion;
                     unsigned int lKey2;
                     unsigned int lKey3;
                     char lUserName[40];

                     #ifdef _EXPIRED_ 
                        AddUser(  lState, "User", 1,-1, -1, 0, 0  );
                     #else
                        if( sscanf( lQuery, "%*s %d-%d %d %d %d %40s", &lMajorID, &lMinorID, &lVersion, &lKey2, &lKey3, lUserName )==6 )
                        {
                           Unpad( lUserName );
                           AddUser( lState, lUserName, lVersion,lMajorID, lMinorID, lKey2, lKey3  );
                        }
                     #endif
                  }
                  // URL?=ADD_GAME USER_ID GAME_NAME TRACK_NAME NBLAP WEAPON PORT
                  else if( !strcmp( lOp, "ADD_GAME" ) )
                  {
                     int       lUserIndex;
                     int       lUserId;
                     int       lNbLap;
                     char      lGameName[40];
                     char      lTrackName[40];
                     int       lWeapon;
                     unsigned  lPort;
               
                     if( sscanf( lQuery, "%*s %d-%u %40s %40s %d %d %u", &lUserIndex, &lUserId, lGameName, lTrackName, &lNbLap, &lWeapon, &lPort )==7 )
                     {
                        const char* lRemoteAddr = getenv( "REMOTE_ADDR" );

                        if( (lRemoteAddr != NULL)&&(strlen(lRemoteAddr) != 0) )
                        {
                           Unpad( lTrackName );
                           Unpad( lGameName );
                           AddGame( lState, lGameName, lTrackName, lNbLap, lUserIndex, lUserId, lRemoteAddr, lPort, lWeapon  );
                        }
                     }
                  }
                  else if( !strcmp( lOp, "JOIN_GAME" ) )
                  {
                     int   lUserIndex;
                     int   lUserId;
                     int   lGameIndex;
                     int   lGameId;
               
                     if( sscanf( lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId )==4 )
                     {
                        JoinGame( lState, lGameIndex, lGameId, lUserIndex, lUserId );
                     }
                  }
                  else if( !strcmp( lOp, "DEL_GAME" ) )
                  {
                     int   lUserIndex;
                     int   lUserId;
                     int   lGameIndex;
                     int   lGameId;
               
                     if( sscanf( lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId )==4 )
                     {
                        DeleteGame( lState, lGameIndex, lGameId, lUserIndex, lUserId );
                     }
                  }
                  else if( !strcmp( lOp, "LEAVE_GAME" ) )
                  {
                     int   lUserIndex;
                     int   lUserId;
                     int   lGameIndex;
                     int   lGameId;
               
                     if( sscanf( lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId )==4 )
                     {
                        LeaveGame( lState, lGameIndex, lGameId, lUserIndex, lUserId );
                     }
                  }
                  else if( !strcmp( lOp, "DEL_USER" ) )
                  {
                     int   lUserIndex;
                     int   lUserId;
               
                     if( sscanf( lQuery, "%*s %d-%u", &lUserIndex, &lUserId )==2 )
                     {
                        DeleteUser( lState, lUserIndex, lUserId );
                     }
                  }
                  else if( !strcmp( lOp, "START_GAME" ) )
                  {
                     int   lUserIndex;
                     int   lUserId;
                     int   lGameIndex;
                     int   lGameId;
               
                     if( sscanf( lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId )==4 )
                     {
                        StartGame( lState, lGameIndex, lGameId, lUserIndex, lUserId );
                     }
                  }
                  else
                  {
                     lPrintTitle = TRUE;
                  }
               }

               #ifdef _NO_IPC_
                  lState = NULL;
               #else
                  // Release lock
                  if( lLock != -1 )
                  {
                     lSemOp.sem_op   = 1;

                     semop( lLock, &lSemOp, 1 );

                     // Release memory
                     if( lState != NULL )
                     {
                        shmdt( (char*)lState );
                     }
                  }
                  
               #endif
               
            }
         }
      }

      CloseLogFile();

      if( lPrintTitle )
      {
         printf( "Internet Meeting Room (c)1996,97 GrokkSoft inc.\n" );
      }
   }

   return 0;
}


static int StrMaxCopy( char* pDest, const char* pSrc, int pDestSize )
{
   int lCounter;

   for( lCounter = 0; lCounter<pDestSize; lCounter++ )
   {
      if( (pDest[ lCounter ] = pSrc[ lCounter ]) == 0 )
      {
         break;
      }
   }
   
   if( pDestSize>0 )
   {
      pDest[ pDestSize-1 ] = 0;
   }

   return lCounter;
}

BOOL StrCompare( const char* pStr, const char* pToken )
{
   BOOL lReturnValue = FALSE;

   while( isspace( *pStr ) )
   {
      pStr++;
   }

   while( (*pToken!=0)&&(toupper( *pStr ) == *pToken) )
   {
      pStr++;
      pToken++;
   }

   if( (*pToken==0) )
   {
      lReturnValue = TRUE;
   }
   return lReturnValue;
}


void Unpad( char* pStr )
{
   while( *pStr != 0 )
   {
      if( *pStr == '*' )
      {
         *pStr = ' ';
      }
      pStr++;
   }
}

void UnpadQuery( char* pStr )
{
   while( *pStr != 0 )
   {
      if( *pStr == '^' )
      {
         *pStr = ' ';
      }
      pStr++;
   }
}

BOOL InitLogFile()
{
   if( gLogFile == NULL )
   {
      gLogFile = fopen( "log/ir.log", "a" );      
   }
   return( gLogFile != NULL );
}

void CloseLogFile()
{
   if( gLogFile != NULL )
   {
      fclose( gLogFile );
      gLogFile = NULL;
   }
}

const char* GetTime( time_t pTime )
{
   static char lTimeBuffer[256];
   time_t lTimer = pTime;
      
   struct tm* lTime = localtime( &lTimer );

   strcpy( lTimeBuffer, "error" );
   strftime( lTimeBuffer, sizeof( lTimeBuffer ), "%y-%m-%d_%H:%M:%S", lTime );
   
   return lTimeBuffer;
}

const char* chk( const char* pSrc )
{
   if( pSrc == NULL )
   {
      return "null";
   }
   else
   {
      return pSrc;
   }
}



