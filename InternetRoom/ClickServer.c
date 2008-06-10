/*
// ClickServer.c

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

// ClickServer that must be used with BannerServer.c
// Use IP address to linkl banners and click. No cookies

// Curently work only on UNIX because of the lock mechanism 
*/



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>



#ifndef BOOL 
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define BS_DEF_FILE     "banners.dat"
#define BS_QUEUE_FILE   "queue.dat"
#define BS_QUEUE_ENTRY  300

int ReadQueueEntry( unsigned long pIP )
{
   int lIndex;
   int lCounter;
   int lReturnValue;
   int lNbRead;
   FILE* lFile;
   unsigned long lBuffer[ 2*BS_QUEUE_ENTRY+1 ];
   
   lFile = fopen( BS_QUEUE_FILE, "rb" );

   lReturnValue = 0;

   if( lFile != NULL )
   {
      /* Lock the file */
      if( flock( fileno( lFile ), LOCK_SH ) != -1 )
      {
         lNbRead = fread( lBuffer, 1, sizeof( lBuffer ), lFile );
         flock( fileno( lFile ), LOCK_UN );

         if( lNbRead == sizeof( lBuffer ) )
         {
            lIndex = lBuffer[0];

            for( lCounter = 0; lCounter < BS_QUEUE_ENTRY; lCounter++ )
            {
               if( lBuffer[ 2*lIndex+1 ] == pIP )
               {
                  lReturnValue = lBuffer[ 2*lIndex+2 ];
               }
               
               lIndex++;
               if( lIndex >= BS_QUEUE_ENTRY )
               {
                  lIndex = 0;
               }
            }
         }
      }
      fclose( lFile );
   }

   return lReturnValue;
}

unsigned long GetIP()
{
   unsigned long lReturnValue;
   const char*   lIP;

   unsigned int lNibble[4];

   lIP = getenv( "REMOTE_ADDR" );

   if( lIP != NULL )
   {
      sscanf( lIP,
              "%d.%d.%d.%d",
              &lNibble[0],
              &lNibble[1],
              &lNibble[2],
              &lNibble[3]     );

      lReturnValue =  lNibble[0]
                     +(lNibble[1]<<8)
                     +(lNibble[2]<<16)
                     +(lNibble[3]<<24);

   }
   return lReturnValue;
}

void PrintReference( int pBannerIndex )
{
   const char* lLocation;
   char        lLineBuffer[512];
   FILE*       lFile;
   int         lNbEntry;
   int         lCounter;


   lLocation = "http://www.grokksoft.com";

   lFile = fopen( BS_DEF_FILE, "r" );


   if( lFile != NULL )
   {
      if( fgets( lLineBuffer, sizeof( lLineBuffer ), lFile ) )
      {
         lNbEntry = 0;
         sscanf( lLineBuffer, "%d", &lNbEntry );

         if( pBannerIndex < lNbEntry )
         {
            for( lCounter = 0; lCounter < pBannerIndex; lCounter++ )
            {
               fgets( lLineBuffer, sizeof( lLineBuffer ), lFile );
            }
            if( fgets( lLineBuffer, sizeof( lLineBuffer ), lFile ) )
            {
               lLocation = lLineBuffer;
            }
         }
      }
      fclose( lFile );
   }
   printf( "Location: %s\n\n", lLocation );
}


int main( int pArgc, const char** pArgs )
{

   unsigned long lIP;
   char*         lPathEnd;

   
   lPathEnd = strrchr( pArgs[0], '/' );
   if( lPathEnd != NULL )
   {
      *lPathEnd = 0;
      chdir( pArgs[0] );
   }

   PrintReference( ReadQueueEntry( GetIP() ) );

   return 0;
}


