// TextParsing.cpp
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

#include "TextParsing.h"

// Local data
static CMapStringToString gDefineMap;


BOOL MR_ReadPredefinedConstants( const char* pFileName )
{
   BOOL  lReturnValue = TRUE;
   FILE* lFile = fopen( pFileName, "r" );

   if( lFile == NULL )
   {
      lReturnValue = FALSE;

      fprintf( stderr, "ERROR: Unable to open %s(defines file)\n", pFileName );
   }
   else
   {
      char lBuffer[ 250 ];
      while( lReturnValue && fgets( lBuffer, sizeof( lBuffer ), lFile ) )
      {
         CString lLine = MR_PreProcLine( lBuffer );

         if( MR_BeginByKeyword( lLine, "#define" ) )
         {            
            char lKey[100];
            char lValue[100];
             
            if( sscanf( lLine, " #define %s %s ", lKey, &lValue ) != 2 )
            {
               lReturnValue = FALSE;  

               fprintf( stderr, "ERROR: syntax error in defines file\n" );

            }
            else
            {
               // add the define to the list
               gDefineMap.SetAt( lKey, lValue );
            }
         }
      }

      fclose( lFile );
   }

   return lReturnValue;
}

CString MR_PreProcLine( const char* pLine )
{
   // scan the line and seardh for predefined keyword
   CString lReturnValue;

   if( pLine != NULL )
   {
      const char* lPtr     = pLine;
      BOOL        lInToken = FALSE;
      BOOL        lEnd     = FALSE;
      const char* lTokenStart;

      while( !lEnd )
      {
         switch( *lPtr )
         {
            case 0:
               lEnd = TRUE;
            case ' ':
            case ',':
            case '\n':
            case '\r':
            case '\t':

               if( lInToken )
               {
                  CString lValue;

                  CString lKey( lTokenStart, lPtr-lTokenStart );

                  lInToken = FALSE;

                  if( gDefineMap.Lookup( lKey, lValue ) )
                  {
                     lReturnValue += lValue;
                  }
                  else
                  {
                     lReturnValue += lKey;
                  }


               }

               if( !lEnd )
               {
                  lReturnValue += *lPtr;
               }

               break;

            default:
               if( !lInToken )
               {
                  lInToken = TRUE;
                  lTokenStart = lPtr;
               }
               break;
         }

         lPtr++;
      }
   }
   return lReturnValue;
}

BOOL MR_BeginByKeyword( const char* pLine, const char* pKeyword )
{
   BOOL lReturnValue = FALSE;

   if( !strncmp( MR_SkipLeadingSpaces( pLine ), pKeyword, strlen( pKeyword ) ) )
   {
      lReturnValue = TRUE;
   }

   return lReturnValue;
}

int MR_ContainsKeyword( const char* pLine, const char** pKeywordList )
{
   int lReturnValue = 0;

   pLine = MR_SkipLeadingSpaces( pLine );

   while( pKeywordList[ lReturnValue ] != NULL )
   {
      if( !strncmp( pLine, pKeywordList[ lReturnValue ], strlen( pKeywordList[ lReturnValue ] ) ) )
      {
         return lReturnValue;
      }
      lReturnValue++;
   }
   return -1;   
}

const char*   MR_SkipLeadingSpaces( const char*pString )
{
   const char* lReturnValue = pString;

   while( *lReturnValue == ' ' || *lReturnValue == '\t' )
   {
      lReturnValue++;
   }

   return lReturnValue;
}


