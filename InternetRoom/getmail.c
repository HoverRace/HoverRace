/*
// getmail.c

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

*/



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>




#ifndef BOOL 
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif


static const char* GetTime( time_t pTime );
static const char* GetVar( const char* pVar );
static const char* GetEnv( const char* pVar );
static void LogEntry();

const char* GetTime( time_t pTime )
{
   static char lTimeBuffer[256];
   time_t lTimer;
   struct tm* lTime;

   lTimer = pTime;
   lTime = localtime( &lTimer );

   strcpy( lTimeBuffer, "error" );
   strftime( lTimeBuffer, sizeof( lTimeBuffer ), "%y-%m-%d_%H:%M:%S", lTime );
   
   return lTimeBuffer;
}

const char* GetVar( const char* pVar )
{
   /* pVar must include the = sign */

   const char* lQuery;
   const char* lEnd;
   static char lBuffer[256];

   strcpy( lBuffer, "n/a" );
   lQuery = getenv( "QUERY_STRING" );

   if( lQuery != NULL )
   {

      lQuery = strstr( lQuery, pVar );

      if( lQuery != NULL )
      {
         lQuery += strlen( pVar );

         lEnd = strchr( lQuery, '&' );

         if( lEnd == NULL )
         {
            strcpy( lBuffer, lQuery );
         }
         else
         {
            strncpy( lBuffer, lQuery, lEnd-lQuery );
            lBuffer[lEnd-lQuery] = 0;
         }
      }
   }

   return lBuffer;
}


const char* GetEnv( const char* pVar )
{
   const char* lReturnValue;

   lReturnValue = getenv( pVar );

   if( lReturnValue == NULL )
   {
      lReturnValue = "n/a";
   }
   return lReturnValue;
}

void LogEntry()
{
   FILE* lFile;

   lFile = fopen( "log/getmail.log", "a" );

   if( lFile != NULL )
   {
      fprintf( lFile, 
               "%s <%s> %s",
               GetTime( time( NULL ) ),
               GetVar( "email=" ),
               GetEnv( "REMOTE_ADDR" )
            );

      fprintf( lFile,
               " %s\n",
               GetVar( "next_url=" )
            );
              
      fclose( lFile );
   }

}

int main( int pArgc, const char** pArgs )
{
   char*         lPathEnd;

   printf( "Location: %s\n\n", GetVar( "next_url=" ) );

   lPathEnd = strrchr( pArgs[0], '/' );
   if( lPathEnd != NULL )
   {
      *lPathEnd = 0;
      chdir( pArgs[0] );
   }

   LogEntry();

   return 0;
}


