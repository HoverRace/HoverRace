/*
// BannerServer.c

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

// BannerServer that must be used with ClickServer.c
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

void SetQueueEntry( unsigned long pIP, int pBanner )
{
   FILE* lFile;
   unsigned long lBuffer[ 2*BS_QUEUE_ENTRY+1 ];
   
   lFile = fopen( BS_QUEUE_FILE, "r+b" );

   if( lFile == NULL )
   {
      lFile = fopen( BS_QUEUE_FILE, "wb" );
   }

   if( lFile != NULL )
   {
      /* Lock the file */
      if( flock( fileno( lFile ), LOCK_EX ) != -1 )
      {
         if( fread( lBuffer, 1, sizeof( lBuffer ), lFile ) != sizeof( lBuffer ) )
         {
            /* Set to 0 */
            memset( lBuffer, 0, sizeof( lBuffer ) );
         }

         lBuffer[ 2*lBuffer[0]+1 ] = pIP;
         lBuffer[ 2*lBuffer[0]+2 ] = pBanner;
         lBuffer[0] = (lBuffer[0]+1)%BS_QUEUE_ENTRY;

         fseek( lFile, 0 , SEEK_SET );
         fwrite( lBuffer, 1, sizeof( lBuffer ), lFile );

         flock( fileno( lFile ), LOCK_UN );
      }
      fclose( lFile );
   }
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

int GetBannerCount()
{
   int lReturnValue;
   FILE* lFile;

   lReturnValue = 0;

   lFile = fopen( BS_DEF_FILE, "r" );

   if( lFile != NULL )
   {
      fscanf( lFile, "%d", &lReturnValue );

      fclose( lFile );

      if( lReturnValue > 0 )
      {
         lReturnValue--; /* Remove entry 0 */
      }
   }

   return lReturnValue;
}

void PrintBanner( int pID )
{
   FILE* lFile;
   char lName[20];
   char lBuffer[25*1024];
   int  lSize;

   printf( "Content-type: image/gif\n\n");

   sprintf( lName, "b%d.gif", pID );

   lFile = fopen( lName, "rb" );

   if( lFile != NULL )
   {
      lSize = fread( lBuffer, 1, sizeof( lBuffer ), lFile );

      fclose( lFile );

      if( lSize > 0 )
      {
         fwrite( lBuffer, 1, lSize, stdout );
      }
   }

   printf( "\n\n");

}

int main( int pArgc, const char** pArgs )
{

   unsigned long lIP;
   int           lIndex;
   int           lNbBanners;
   char*         lPathEnd;

   
   lPathEnd = strrchr( pArgs[0], '/' );
   if( lPathEnd != NULL )
   {
      *lPathEnd = 0;
      chdir( pArgs[0] );
   }

   lNbBanners = GetBannerCount();
   
   if( lNbBanners <= 0 )
   {
      lIndex = 0;
   }
   else
   {
      srand( time( NULL ) );
      lIndex = 1+(((unsigned)rand())%lNbBanners);
   }

   SetQueueEntry( GetIP(), lIndex );

   PrintBanner( lIndex );

   return 0;
}


