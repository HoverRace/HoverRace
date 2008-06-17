// Resource compiler.cpp
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


// This tool compiles bitmaps and meshings associateds to an
// object factory in a binary file used at run time

// CommandLine
//
//    ResourceCompiler <InputFile> <OutputFile> [<DefineFile>]
//
//
//
// Input File format
// 
//    The input file format can only contains 2 keywords
//
//     BITMAP or ACTOR or SPRITE
//
//  The format of a valid line is
//
//     <Keyword> <ExternalFile> <NumericId of the resource>
//
//
//
//  DefineFileFormat
//
//     The define file is used to make a string to int relation. This way all
//     numeric constants defined in input files can be expressed in the
//     form of a string like in C
//
//     The valid lines in this file must have the following form
//     #define <Character string> <Integer constant>
//

#include "stdafx.h"
#include "TextParsing.h"
#include "ResBitmapBuilder.h"
#include "ResActorBuilder.h"
#include "ResSpriteBuilder.h"
#include "ResSoundBuilder.h"
#include "ResourceLibBuilder.h"
#include "../ColorTools/ColorTools.h"


// Local functions prototypes
static BOOL ParseInputFile( const char* pFileName );
static void PrintUsage();

// Local data
static MR_ResourceLibBuilder gsLib;

int main( int pArgc, const char** pArgs )
{
   BOOL lReturnValue = TRUE;

   printf( "Resources Compiler               (C)Copyright GrokkSoft inc, 1996\n" );

   MR_ColorTools::Init();   

   if( pArgc <= 2 )
   {
      PrintUsage();
   }
   else
   {
      if( pArgc >= 4 )
      {
         lReturnValue = MR_ReadPredefinedConstants( pArgs[3] );
      }

      if( lReturnValue )
      {
         lReturnValue = ParseInputFile( pArgs[1] );
      }

      if( lReturnValue )
      {
         lReturnValue = gsLib.Export( pArgs[2] );
      }
   }

   return( lReturnValue ? 0 : -1 );
}

void PrintUsage()
{
   printf( "\nResourceCompiler <InputFile> <OutputFile> [<DefineFile>]\n\n" );
}

BOOL ParseInputFile( const char* pFileName )
{
   static const char* lKeywordList[] = { "BITMAP", "ACTOR", "SPRITE", "SHORT_SOUND", "CONT_SOUND", NULL };

   BOOL lReturnValue = TRUE;

   FILE* lFile = fopen( pFileName, "r" );

   if( lFile == NULL )
   {
      lReturnValue = FALSE;
      fprintf( stderr, "ERROR: Unable to open input file(%s)\n", pFileName );
   }
   else
   {
      char lLineBuffer[ 250 ];

      while( lReturnValue && fgets( lLineBuffer, sizeof( lLineBuffer ), lFile ) )
      {
         int    lResourceId;
         int    lNbItem;
         int    lNbCopy;
         float  lWidth;
         float  lHeight;
         int    lAntiAliasScheme;

         CString lLine = MR_PreProcLine( lLineBuffer );

         switch( MR_ContainsKeyword( lLine, lKeywordList ) )
         {
            case 0: // BITMAP
               if( sscanf( lLine, " %*s %s %d %f %f %d", lLineBuffer, &lResourceId, &lWidth, &lHeight, &lAntiAliasScheme ) != 5 )
               {
                  fprintf( stderr, "ERROR reading bitmap parameters\n" );
                  lReturnValue = FALSE;
               }
               else
               {
                  MR_ResBitmapBuilder* lBitmap = new MR_ResBitmapBuilder( lResourceId, lWidth*1000, lHeight*1000 );

                  lReturnValue = lBitmap->BuildFromFile( lLineBuffer, lAntiAliasScheme );

                  if( lReturnValue )
                  {
                     gsLib.AddBitmap( lBitmap );
                  }
                  else
                  {
                     delete lBitmap;
                  }
               }
               break;

            case 1: // ACTOR
               if( sscanf( lLine, " %*s %s %d ", lLineBuffer, &lResourceId ) != 2 )
               {
                  fprintf( stderr, "ERROR reading actor parameters\n" );
               }
               else
               {
                  MR_ResActorBuilder* lActor = new MR_ResActorBuilder( lResourceId );

                  lReturnValue = lActor->BuildFromFile( lLineBuffer, &gsLib );

                  if( lReturnValue )
                  {
                     gsLib.AddActor( lActor );
                  }
                  else
                  {
                     delete lActor;
                  }
               }
               break;

            case 2: // SPRITE
               if( sscanf( lLine, " %*s %s %d %d", lLineBuffer, &lResourceId, &lNbItem ) != 3 )
               {
                  fprintf( stderr, "ERROR reading sprite parameters\n" );
                  lReturnValue = FALSE;
               }
               else
               {
                  MR_ResSpriteBuilder* lSprite = new MR_ResSpriteBuilder( lResourceId );

                  lReturnValue = lSprite->BuildFromFile( lLineBuffer, lNbItem );

                  if( lReturnValue )
                  {
                     gsLib.AddSprite( lSprite );
                  }
                  else
                  {
                     delete lSprite;
                  }
               }
               break;

            case 3: // SHORT_SOUND
               if( sscanf( lLine, " %*s %s %d %d", lLineBuffer, &lResourceId, &lNbCopy ) != 3 )
               {
                  fprintf( stderr, "ERROR reading sound parameters\n" );
                  lReturnValue = FALSE;
               }
               else
               {
                  MR_ResShortSoundBuilder* lSound = new MR_ResShortSoundBuilder( lResourceId );

                  lReturnValue = lSound->BuildFromFile( lLineBuffer, lNbCopy );

                  if( lReturnValue )
                  {
                     gsLib.AddSound( lSound );
                  }
                  else
                  {
                     delete lSound;
                  }
               }
               break;

            case 4: // CONT_SOUND
               if( sscanf( lLine, " %*s %s %d %d", lLineBuffer, &lResourceId, &lNbCopy ) != 3 )
               {
                  fprintf( stderr, "ERROR reading sound parameters\n" );
                  lReturnValue = FALSE;
               }
               else
               {
                  MR_ResContinuousSoundBuilder* lSound = new MR_ResContinuousSoundBuilder( lResourceId );

                  lReturnValue = lSound->BuildFromFile( lLineBuffer, lNbCopy );

                  if( lReturnValue )
                  {
                     gsLib.AddSound( lSound );
                  }
                  else
                  {
                     delete lSound;
                  }
               }
               break;

         }

      }
      fclose( lFile );
   }

   printf( "--PROGRAM END-- Press enter to continue\n" );
   getchar();

   return lReturnValue;
}

