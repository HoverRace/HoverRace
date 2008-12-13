/* KeyServer.c

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


// #define _FAST_CGI_
// #define _NO_IPC_

*/

#ifdef _FAST_CGI_
   #include "/usr/local/fcgi/include/fcgi_stdio.h"
#else
   /* #include <stdio.h> */
#endif


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>


/* Required section to compile on Windows */
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
   /* struct semid_ds *buf; */
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

/* KEY SERVER DATA */
#define IR_SEM_KEY         75426
#define IR_SHMEM_KEY       75426


#define ID_MAJOR 2
#define ID_BASE  1
#define ID_MAX   200

struct CraftDescription
{
   int   mModel;
   char* mName;
};

struct CraftDescription CraftModels[] =
{
   { 0, "Basic Craft" },
   { 1, "Low Cx Craft" },
   { 2, "Bi-Turbo Craft" },
};

#define gNbCraftModels  ( sizeof( CraftModels )/sizeof( struct CraftDescription ) )

char* RecordPeriod[] =
{
   "This Week",
   "All Times",
};

#define gNbRecordPeriods ( sizeof( RecordPeriod )/sizeof( char* ) )

/* Structure definition */

struct TrackStruct
{
   /*public:*/
     char         mName[ IR_TRACK_NAME_LEN+1 ];
     unsigned int mSum;
};

typedef struct TrackStruct Track;


struct LapRecordStruct
{
   /*public:*/
   time_t mDate;
   int    mDuration;
   char   mUser[  IR_USER_NAME_LEN+1 ];
};

typedef struct LapRecordStruct LapRecord;

struct RaceRecordStruct
{
   time_t mDate;
   int    mDuration;
   char   mUser[  IR_USER_NAME_LEN+1 ];
   int    mNbLap;
   int    mNbPlayer;
   int    mCraft;
};

typedef struct RaceRecordStruct RaceRecord;




struct ISStateStruct
{
   /*protected:*/

      Track      mTrack[ IR_MAX_TRACK ];
      LapRecord  mLapRecord[ IR_MAX_TRACK ][gNbCraftModels][gNbRecordPeriods];

      int        mRaceTrackIndex[ IR_MAX_TRACK_BT ];
      int        mRaceTrackPlayers[ IR_MAX_TRACK_BT ];
      int        mRaceTrackLaps[ IR_MAX_TRACK_BT ];
      RaceRecord mRaceRecord[ IR_MAX_TRACK_BT ][ IR_MAX_PLAYER_BT ];
      int        mNbTrackEntries; 
      

   /*public:*/
      
 

      /* These actions print a return value */

};

typedef struct ISStateStruct ISState;

void StateResetWeekly( ISState* pThis );
void StateAddLapRecord( ISState* pThis,  const char* pTrackName, unsigned pTrackSum, const char* pAlias, int pDuration, int pModel );
void StateAddRaceRecord( ISState* pThis, const char* pTrackName, unsigned pTrackSum, const char* pAlias, int pDuration, int pModel, int pNbLap, int pNbPlayer );
void StatePrintLap( ISState* pThis, int pMode );   /* mode 0= Global, 1 = Alltime, 2 == week ) */
void StatePrintRace( ISState* pThis, int pTrack ); /* Track = track entry( 0,1,2 ) */

void StateRead( ISState* pThis );
void StateWrite( ISState* pThis );


#ifdef _NO_IPC_

   ISState gGlobalState;

#endif

/* Local functions */
static BOOL GetString( FILE* pFile, char* pBuffer, int pBufferSize );
static int  StrMaxCopy( char* pDest, const char* pSrc, int pDestSize );
static BOOL StrCompare( const char* pStr, const char* pToken );
static void Unpad( char* pStr );
static void UnpadQuery( char* pStr );
static void Filter( char* pStr );

static const char* GetDate( time_t pTime );


/* Implementation */
void StateResetWeekly( ISState* pThis )
{
   int lTrack;
   int lCraft;

   for( lTrack = 0; lTrack < IR_MAX_TRACK; lTrack++ )
   {
      for( lCraft = 0; lCraft < gNbCraftModels; lCraft++ )
      {
         pThis->mLapRecord[lTrack][lCraft][0].mDate     = 0;
         pThis->mLapRecord[lTrack][lCraft][0].mDuration = 0;
         strcpy( pThis->mLapRecord[lTrack][lCraft][0].mUser, "none" );
      }
   }
   StateWrite( pThis );
}


void StateAddRaceRecord( ISState* pThis, const char* pTrackName, unsigned pTrackSum, const char* pAlias, int pDuration, int pModel, int pNbLap, int pNbPlayer )
{
   BOOL lSave;
   int  lCounter;
   int  lTrack;
   int  lModel;

   if( pDuration > 0 )
   {
      lSave = FALSE;
      lTrack = -1;

      /* Find the track having the record */
      for( lCounter = 0; lCounter < IR_MAX_TRACK_BT; lCounter++ )
      {
         if( pThis->mRaceTrackIndex[ lCounter ] != -1 )
         {
            if( !strcmp( pTrackName, pThis->mTrack[pThis->mRaceTrackIndex[ lCounter ]].mName ) )
            {
               if( (pNbPlayer >=  pThis->mRaceTrackPlayers[ lCounter ])&&
                   (pNbLap ==  pThis->mRaceTrackLaps[ lCounter ])   )
               {
                  lTrack = lCounter;
               }
               break;
            }
         }
      }
  
      if( lTrack != -1 )
      {
         if( pThis->mTrack[pThis->mRaceTrackIndex[ lTrack ]].mSum == 0 )
         {
            pThis->mTrack[ pThis->mRaceTrackIndex[ lTrack ]].mSum = pTrackSum;
            lSave = TRUE;
         }
         else if( pThis->mTrack[pThis->mRaceTrackIndex[ lTrack ]].mSum != pTrackSum )
         {
            lTrack = -1;
         }
      }

      if( lTrack != -1 )
      {
         /* Find the HoverModel */
         lModel = -1;

         for( lCounter = 0; lCounter < gNbCraftModels; lCounter++ )
         {
            if( CraftModels[ lCounter ].mModel == pModel )
            {
               lModel = lCounter;
               break;
            }
         }

         if( lModel != -1 )
         {
            /*
            // From the end result to the top result verify it we can not 
            // find an entry for this new record
            */
            if( (pDuration < pThis->mRaceRecord[ lTrack ][IR_MAX_PLAYER_BT-1].mDuration)||(pThis->mRaceRecord[ lTrack ][IR_MAX_PLAYER_BT-1].mDuration<=0) )
            {
               for( lCounter = IR_MAX_PLAYER_BT-2; lCounter >= 0; lCounter-- )
               {
                  if( (pDuration < pThis->mRaceRecord[ lTrack ][lCounter].mDuration)||(pThis->mRaceRecord[ lTrack ][lCounter].mDuration<=0) )
                  {
                     memcpy( &(pThis->mRaceRecord[ lTrack ][lCounter+1]) , &(pThis->mRaceRecord[ lTrack ][lCounter]), sizeof( pThis->mRaceRecord[0][0] ) );
                  }
                  else
                  {
                     break;
                  }
               }
               lCounter++;


               pThis->mRaceRecord[ lTrack ][lCounter].mDate     = time( NULL );
               pThis->mRaceRecord[ lTrack ][lCounter].mDuration = pDuration;
               StrMaxCopy( pThis->mRaceRecord[ lTrack ][lCounter].mUser, pAlias, IR_USER_NAME_LEN+1 );
               pThis->mRaceRecord[ lTrack ][lCounter].mNbLap     = pNbLap;
               pThis->mRaceRecord[ lTrack ][lCounter].mNbPlayer  = pNbPlayer;
               pThis->mRaceRecord[ lTrack ][lCounter].mCraft     = lModel;

               lSave = TRUE;
            }
         }
      }

      if( lSave )
      {
         StateWrite( pThis );
      }
   }
}



void StateAddLapRecord( ISState* pThis, const char* pTrackName, unsigned pTrackSum, const char* pAlias, int pDuration, int pModel )
{
   BOOL lSave;
   int  lCounter;
   int  lTrack;
   int  lModel;
   int  lPeriod;

   if( pDuration > 0 )
   {
      lSave = FALSE;
      lTrack = -1;

      /* Find the track having the record */
      for( lCounter = 0; lCounter < pThis->mNbTrackEntries; lCounter++ )
      {
         if( !strcmp( pTrackName, pThis->mTrack[lCounter].mName ) )
         {
            lTrack = lCounter;
            break;
         }
      }
  
      if( lTrack != -1 )
      {
         if( pThis->mTrack[lCounter].mSum == 0 )
         {
            pThis->mTrack[lCounter].mSum = pTrackSum;
            lSave = TRUE;
         }
         else if( pThis->mTrack[lCounter].mSum != pTrackSum )
         {
            lTrack = -1;
         }
      }

      if( lTrack != -1 )
      {
         /* Find the HoverModel */
         lModel = -1;

         for( lCounter = 0; lCounter < gNbCraftModels; lCounter++ )
         {
            if( CraftModels[ lCounter ].mModel == pModel )
            {
               lModel = lCounter;
               break;
            }
         }

         if( lModel != -1 )
         {
            for( lPeriod = 0; lPeriod< gNbRecordPeriods; lPeriod++ )
            {
               if(  (pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration <= 0 )
                  ||( pDuration < pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration  ) )
               {

                  pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration = pDuration;
                  pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDate = time( NULL );
                  StrMaxCopy( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mUser, pAlias, IR_USER_NAME_LEN+1 );

                  lSave = TRUE;
               }
            }
         }
      }

      if( lSave )
      {
         StateWrite( pThis );
      }
   }
}

void StatePrintRace( ISState* pThis, int pTrack )
{
   int lCounter;

   if( pThis->mRaceTrackIndex[pTrack] != -1 )
   {

      printf( "<table border=2>\n" );

      printf( "<tr><td colspan=5>%s %d Laps</td></tr>\n",
               pThis->mTrack[ pThis->mRaceTrackIndex[ pTrack ] ].mName,
               pThis->mRaceTrackLaps[ pTrack ] );

      printf( "<tr><td>Rank</td><td>Player</td><td>Time</td><td>Date</td><td>Craft</td></tr>\n" );

      for( lCounter = 0; lCounter < IR_MAX_PLAYER_BT; lCounter++ )
      {
         if( pThis->mRaceRecord[pTrack][lCounter].mDuration > 0 )
         {
            printf( "<tr><td>%d</td><td>%s</td><td>%2d:%02d:%02d</td><td>%s</td><td>%s</td></tr>\n",
                    lCounter+1,
                    pThis->mRaceRecord[pTrack][lCounter].mUser,
                    (pThis->mRaceRecord[pTrack][lCounter].mDuration/60000),
                    (pThis->mRaceRecord[pTrack][lCounter].mDuration/1000)%60,
                    (pThis->mRaceRecord[pTrack][lCounter].mDuration/10)%100,                  
                    GetDate(pThis->mRaceRecord[pTrack][lCounter].mDate),
                    CraftModels[pThis->mRaceRecord[pTrack][lCounter].mCraft].mName );
         }
      }
      printf( "</table>" );
   }
}


void StatePrintLap( ISState* pThis, int pMode )
{
   /*  HTML output */
   int lTrack;
   int lPeriod;
   int lPeriodEnd;
   int lModel;
   int lRecordModel;

   printf( "<table border=2>\n" );

   if( pMode == 0 )
   {
      lTrack = -2;
   }
   else
   {
      lTrack = -1;
   }

   for( ; lTrack < pThis->mNbTrackEntries; lTrack++ )
   {

      printf( "<tr>\n" );
      if( lTrack >= 0 )
      {
         printf( "<td>%s</td>\n",  pThis->mTrack[ lTrack ].mName );
      }
      else if( lTrack == -2 )
      {
         printf( "<td rowspan=2></td>\n" );
      }
      else if( pMode != 0 )
      {
         printf( "<td></td>\n" );
      }

      if( pMode == 0 )
      {
         lPeriod = 0;
         lPeriodEnd = gNbRecordPeriods;
      }
      else
      {
         lPeriod = gNbRecordPeriods-pMode;
         lPeriodEnd = lPeriod+1;
      }


      for( ; lPeriod < lPeriodEnd; lPeriod++ )
      {
         if( lTrack == -2 )
         {
            printf( "<td colspan=%d align=\"Center\">%s</td>\n", gNbCraftModels, RecordPeriod[ lPeriod ] );
         }
         else
         {
            
            lRecordModel = -1;

            if( lTrack != -1 )
            {
               for( lModel = 0; lModel< gNbCraftModels; lModel++ )
               {
                  if( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration > 0 )
                  {
                     if( lRecordModel == -1 )
                     {
                        lRecordModel = lModel;
                     }
                     else if( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration < pThis->mLapRecord[ lTrack ][ lRecordModel ][ lPeriod ].mDuration )
                     {
                        lRecordModel = lModel;
                     }
                  }
               }
            }
            
            for( lModel = 0; lModel< gNbCraftModels; lModel++ )
            {
               printf( "<td align=\"Center\">\n" );
               if( lTrack == -1 )
               {
                  printf( CraftModels[lModel].mName );
               }
               else
               {
                  if( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration > 0 )
                  {
                     printf( "%s%2d:%02d:%02d%s<br>%s<br>%s\n", 
                             (lModel == lRecordModel)?"<b>":"",
                             (pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration/60000),
                             (pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration/1000)%60,
                             (pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration/10)%100,
                             (lModel == lRecordModel)?"</b>":"",
                              pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mUser,
                              GetDate( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDate )   );
                  }
                  else
                  {
                     printf( "<br>\n" );
                  }
               }
               printf( "</td>\n" );
            }
         }
      }
      printf( "</tr>\n" );
   }
   printf( "</table>\n" );
}

void StateRead( ISState* pThis )
{
   char lLineBuffer[256];
   int lTrack;
   int lPeriod;
   int lModel;
   int lCounter;

   FILE* lFile;
   
   lFile = fopen( "scores.txt", "r" );

   if( lFile != NULL )
   {
      GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );         
      
      pThis->mNbTrackEntries = 0;

      sscanf( lLineBuffer, "%d", &(pThis->mNbTrackEntries) );

      for( lTrack = 0; lTrack < pThis->mNbTrackEntries; lTrack++ )
      {
         GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );         
         strncpy( pThis->mTrack[ lTrack ].mName, lLineBuffer, sizeof( pThis->mTrack[ lTrack ].mName ) );
         GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );         
         sscanf( lLineBuffer, "%u",  &(pThis->mTrack[ lTrack ].mSum) );

         for( lPeriod = 0; lPeriod < gNbRecordPeriods; lPeriod++ )
         {
            for( lModel = 0; lModel< gNbCraftModels; lModel++ )
            {
               GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );         

               sscanf( lLineBuffer, "%d %u", &(pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration),
                                             &(pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDate) );
               GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );         
               strncpy( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mUser, lLineBuffer, sizeof( pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mUser ) );
            }
         }
      }

      for( lTrack = 0; lTrack < IR_MAX_TRACK_BT; lTrack++ )
      {
         GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );

         sscanf( lLineBuffer, "%d %d %d", 
                 &pThis->mRaceTrackIndex[ lTrack ],
                 &pThis->mRaceTrackLaps[ lTrack ],
                 &pThis->mRaceTrackPlayers[ lTrack ] );

         for( lCounter = 0; lCounter < IR_MAX_PLAYER_BT; lCounter++ )
         {
            GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );

            sscanf( lLineBuffer, "%d %d %d %d %d", 
                    &pThis->mRaceRecord[ lTrack ][lCounter].mDate,
                    &pThis->mRaceRecord[ lTrack ][lCounter].mDuration,
                    &pThis->mRaceRecord[ lTrack ][lCounter].mNbLap,
                    &pThis->mRaceRecord[ lTrack ][lCounter].mNbPlayer,
                    &pThis->mRaceRecord[ lTrack ][lCounter].mCraft    );

            GetString( lFile, lLineBuffer, sizeof( lLineBuffer ) );
            strncpy( pThis->mRaceRecord[ lTrack ][lCounter].mUser, lLineBuffer, sizeof( pThis->mRaceRecord[ lTrack ][lCounter].mUser ) );

         }
      }
      fclose( lFile );
   }   
}

void StateWrite( ISState* pThis  )
{
   FILE* lFile;
   int lTrack;
   int lPeriod;
   int lModel;
   int lCounter;

   lFile = fopen( "scores.txt", "w" );

   if( lFile != NULL )
   {
      fprintf( lFile, "%d\n", pThis->mNbTrackEntries );

      for( lTrack = 0; lTrack < pThis->mNbTrackEntries; lTrack++ )
      {
         fprintf( lFile, "%s\n%u\n", pThis->mTrack[ lTrack ].mName, pThis->mTrack[ lTrack ].mSum );

         for( lPeriod = 0; lPeriod < gNbRecordPeriods; lPeriod++ )
         {
            for( lModel = 0; lModel< gNbCraftModels; lModel++ )
            {
               fprintf( lFile, "%d %u\n", pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDuration,
                                   pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mDate );
               fprintf( lFile, "%s\n",    pThis->mLapRecord[ lTrack ][ lModel ][ lPeriod ].mUser );
            }
         }
      }

      for( lTrack = 0; lTrack < IR_MAX_TRACK_BT; lTrack++ )
      {
         fprintf( lFile, "%d %d %d\n", 
                  pThis->mRaceTrackIndex[ lTrack ],
                  pThis->mRaceTrackLaps[ lTrack ],
                  pThis->mRaceTrackPlayers[ lTrack ] );

         for( lCounter = 0; lCounter < IR_MAX_PLAYER_BT; lCounter++ )
         {

            fprintf( lFile, "%d %d %d %d %d\n", 
                    pThis->mRaceRecord[ lTrack ][lCounter].mDate,
                    pThis->mRaceRecord[ lTrack ][lCounter].mDuration,
                    pThis->mRaceRecord[ lTrack ][lCounter].mNbLap,
                    pThis->mRaceRecord[ lTrack ][lCounter].mNbPlayer,
                    pThis->mRaceRecord[ lTrack ][lCounter].mCraft    );

            fprintf( lFile, "%s\n", pThis->mRaceRecord[ lTrack ][lCounter].mUser, sizeof( pThis->mRaceRecord[ lTrack ][lCounter].mUser ) );

         }
      }
      fclose( lFile );
   }
}





int main( int pArgc, const char** pArgs )
{
   const char* lQueryPtr;
   char  lQuery[4096];
   char  lOp[12];
   char* lPathEnd;
   union semun lNULL;
   int   lLock;        /* Semaphore */
   int   lMemId;       /* Shared memory */
   struct sembuf lSemOp;
   union  semun lArg;

   int          lDuration;
   char         lTrackName[80];
   char         lUser[80];
   unsigned int lChkSum;
   int          lCraft;

   int          lTotalTime;
   int          lNbLap;
   int          lNbPlayer;


   #ifdef _NO_IPC_
 
      gGlobalState.Read();
   #endif


   #ifdef _FAST_CGI_
   while( FCGI_Accept() >= 0 )
   #endif
   {
      lQuery[0] = 0;
      lOp[0]    = 0;

      if( getenv( "DOCUMENT_NAME" ) != NULL )
      {
         if( pArgc >=2 )
         {
            sprintf( lQuery, "=%s", pArgs[1] );
         }

      }
      else
      {
         /* Send the header required by the server in in CGI mode */
         printf("Content-type: text/plain%c%c", 10, 10);


         lQueryPtr = getenv( "QUERY_STRING" );


         if( lQueryPtr != NULL )
         {
            StrMaxCopy( lQuery, lQueryPtr, 4096 );
            UnpadQuery( lQuery );
         }
      }

      #ifndef _FAST_CGI_
         /* Change the local directory */
         lPathEnd = strrchr( pArgs[0], '/' );

         if( lPathEnd != NULL )
         {
            *lPathEnd = 0;
            chdir( pArgs[0] );
         }
      #endif



      sscanf( lQuery, "=%11s", lOp );


      if( !strcmp( lOp, "RESET" ) )
      {
         printf( "RESET OK\n\n" );

         #ifdef _NO_IPC_
            /*
            // gGlobalState.Clear();
            // sleep( 2 );
            // return 0;
            // break;
            */
         #else

            lLock  = semget( IR_SEM_KEY, 1, 0777 );
            lMemId = shmget( IR_SHMEM_KEY, 1 /*sizeof( ISState )*/, 0666 );

            if( lLock != -1 )
            {

               semctl( lLock, 0, IPC_RMID, lNULL );
            }

            if( lMemId != -1 )
            {
               shmctl( lMemId, IPC_RMID, NULL );
            }

         #endif


         #ifdef _FAST_CGI_
            break;
         #endif

      }
      else
      {
         ISState* lState = NULL;

         #ifdef _NO_IPC_
            lState = &gGlobalState;

         #else


            lSemOp.sem_flg = 0;  /*Avoid corruption but must not core-dump SEM_UNDO;  // Risky but prevents dead-lock */
            lSemOp.sem_num  = 0;

            /* First try to create the structure for the first time */
            /* Lock the data struct */
            lLock = semget( IR_SEM_KEY, 1, 0777|IPC_CREAT|IPC_EXCL );

            if( lLock != -1 )
            {
               
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

                  lMemId = shmget( IR_SHMEM_KEY, sizeof( ISState ), 0666|IPC_CREAT|IPC_EXCL );

                  if( lMemId != -1 )
                  {
                     lState = (ISState*)shmat( lMemId, NULL, 0 );

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
                        StateRead( lState );

                     }
                  }
                  else
                  {
                     lMemId = shmget( IR_SHMEM_KEY, sizeof( ISState ), 0666 );

                     if( lMemId == -1 )
                     {
                        printf( "Unable to get shmem\n" );
                     }
                     else
                     {
                        lState = (ISState*)shmat( lMemId, NULL, 0 );

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
            if( !strcmp( lOp, "BESTLAPS" )  )
            {

               /* Return the score table */
               StatePrintLap( lState, 0 );
            }
            else if( !strcmp( lOp, "BESTLAPS1" )  )
            {
               StatePrintLap( lState, 1 );
            }
            else if( !strcmp( lOp, "BESTLAPS2" )  )
            {
               StatePrintLap( lState, 2 );
            }
            else if( !strcmp( lOp, "BESTTIME1" )  )
            {
               StatePrintRace( lState, 0 );
            }
            else if( !strcmp( lOp, "BESTTIME2" )  )
            {
               StatePrintRace( lState, 1 );
            }
            else if( !strcmp( lOp, "BESTTIME3" )  )
            {
               StatePrintRace( lState, 2 );
            }
            else if( !strcmp( lOp, "BESTTIME4" )  )
            {
               StatePrintRace( lState, 2 );
            }
            else if( !strcmp( lOp, "BESTTIME5" )  )
            {
               StatePrintRace( lState, 2 );
            }
            else if( !strcmp( lOp, "LAPTIME" ) )
            {

               if( sscanf( lQuery, "%*s %d %s %s %u %d", &lDuration, lTrackName, lUser, &lChkSum, &lCraft )==5 )
               {
                  Unpad( lTrackName );
                  Unpad( lUser );
                  Filter( lUser );

                  StateAddLapRecord( lState, lTrackName, lChkSum, lUser, lDuration, lCraft );                  
               }
               printf( "OK\n\n" );
            }
            else if( !strcmp( lOp, "RESULT" ) )
            {
               if( sscanf( lQuery, "%*s %d %s %s %u %d %d %d %d", &lDuration, lTrackName, lUser, &lChkSum, &lCraft, &lTotalTime, &lNbLap, &lNbPlayer )==8 )
               {
                  Unpad( lTrackName );
                  Unpad( lUser );
                  Filter( lUser );

                  StateAddLapRecord( lState, lTrackName, lChkSum, lUser, lDuration, lCraft );
                  StateAddRaceRecord( lState, lTrackName, lChkSum, lUser, lTotalTime, lCraft, lNbLap, lNbPlayer );
               }
               printf( "OK\n\n" );
   
            }
            else if( !strcmp( lOp, "RESET_WEEK" ) )
            {
               StateResetWeekly( lState );
               printf( "RESET_WEEK OK\n\n" );
            }
            else
            {
               printf( "Score Server (c)1997 GrokkSoft inc.\n\n" );
            }
         }

         #ifdef _NO_IPC_
            lState = NULL;
         #else
   
            /* Release lock */
            if( lLock != -1 )
            {
               lSemOp.sem_op   = 1;

               semop( lLock, &lSemOp, 1 );

               /* Release memory */
               if( lState != NULL )
               {
                  shmdt( (char*)lState );
               }
            }
                  
         #endif               
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

/*
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
*/

void Filter( char* pStr )
{
   unsigned char* pSrc;
   unsigned char* pDest;

   pSrc  = (unsigned char*)pStr;
   pDest = (unsigned char*)pStr;

   while( *pSrc != 0 )
   {
      if( (*pSrc >= ' ')&&
          (*pSrc < 128 )&&
          (*pSrc != '<' )&&
          (*pSrc != '>' )&&
          (*pSrc != '&' )&&
          (*pSrc != '%' )    )
      {
         *pDest = *pSrc;

         pDest++;
      }
      pSrc++;
   }
   *pDest = 0;
}

static BOOL gNewEscape;

void Unpad( char* pStr )
{
   char* pSrc;
   char* pDest;
   char  lBuffer[3];
   int   lHexValue;

   if( gNewEscape )
   {
      pSrc = pStr;
      pDest = pStr;

      while( *pSrc != 0 )
      {
         if( (*pSrc == '%') )
         {            
            lBuffer[0] = pSrc[1];
            lBuffer[1] = pSrc[2];
            lBuffer[2] = 0;

            if( sscanf( lBuffer, "%x", &lHexValue ) == 1 )
            {
               pSrc += 2;
               *pDest = (char)(unsigned char)lHexValue;
            }
            else
            {
               *pDest = '%';
            }
         }
         else
         {
            *pDest = *pSrc;
         }
         pSrc++;
         pDest++;
      }
      *pDest = 0;      
   }
   else
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
}

void UnpadQuery( char* pStr )
{
   char* pSrc;
   char* pDest;

   gNewEscape = FALSE;



   if( strchr( pStr, '^' )==NULL )
   {
      gNewEscape = TRUE;

      pSrc = pStr;
      pDest = pStr;

      while( *pSrc != 0 )
      {
         if( (*pSrc == '%')&&(pSrc[1] == '%') )
         {
            *pDest = ' ';
            pSrc++;
         }
         else
         {
            *pDest = *pSrc;
         }
         pSrc++;
         pDest++;
      }
      *pDest = 0;
   }
   else
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
}



const char* GetDate( time_t pTime )
{
   static char lTimeBuffer[256];
   time_t lTimer = pTime;
      
   struct tm* lTime = localtime( &lTimer );

   lTimeBuffer[0] = 0;
   strftime( lTimeBuffer, sizeof( lTimeBuffer ), "%m-%d-%y", lTime );
   
   return lTimeBuffer;
}



BOOL GetString( FILE* pFile, char* pBuffer, int pBufferSize )
{
   pBuffer[0] = 0;

   if( fgets( pBuffer, pBufferSize, pFile ) == NULL )
   {
      return FALSE;
   }
   else
   {
      while( *pBuffer != 0 )
      {
         if( (*pBuffer == '\n')||(*pBuffer == '\r') )
         {
            *pBuffer = 0;
            break;
         }
         pBuffer++;
      }
      return TRUE;
   }
}


