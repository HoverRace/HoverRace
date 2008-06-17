// BitmapHelper.cpp
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
#include "BitmapHelper.h"


// Local prototypes
static MR_UInt8*  BMPRead( FILE* pFile, int& pXRes, int& pYRes );
static MR_UInt8*  IMGRead( FILE* pFile, int& pXRes, int& pYRes );
static MR_UInt8*  PCXRead( FILE* pFile, int& pXRes, int& pYRes );


MR_UInt8* LoadBitmap( const char* lFileName, int& pXRes, int& pYRes, BOOL pReverse )
{
   MR_UInt8* lReturnValue = NULL;

   FILE* lFile = fopen( lFileName, "rb" );
   
   if( lFile == NULL )
   {
      printf( "ERROR: Unable to open input file\n" );
   }
   else
   {
      char *lExtension = strrchr( lFileName, '.' );

      if( !strcmpi( lExtension, ".PCX" ))
      {
         lReturnValue = PCXRead( lFile, pXRes, pYRes );
      }
      else if(!stricmp( lExtension, ".BMP" ))
      {
         lReturnValue = BMPRead( lFile, pXRes, pYRes );
      }
      else if(!stricmp( lExtension, ".IMG" ))
      {
         lReturnValue = IMGRead( lFile, pXRes, pYRes );
      }
      else
      {
         printf( "ERROR: Invalid file type\n" );
      }
   
      fclose( lFile );
   }
   
   if( (lReturnValue != NULL)&&pReverse )
   {
      // Make the invertion of the image pixels
      MR_UInt8* lOldBuffer = lReturnValue;

      lReturnValue = new MR_UInt8[ pXRes*pYRes ];

      int lDestIndex = 0;

      for( int lX = 0; lX<pXRes; lX++ )
      {
         MR_UInt8* lSource = lOldBuffer+lX; 

         for( int lY = 0; lY<pYRes; lY++ )
         {
            lReturnValue[lDestIndex] = *lSource;
            lSource += pXRes;
            lDestIndex++;
         }
      }
      delete lOldBuffer;
   }

   return lReturnValue;
}



MR_UInt8* BMPRead( FILE* pFile, int& pXRes, int& pYRes )
{
   MR_Int32  lDataPos;
   MR_UInt8* lReturnValue = NULL;
   
   // Read Header
   fseek( pFile, 10 ,SEEK_SET );
   fread( &lDataPos, 4, 1, pFile );
   
   fseek( pFile, 18 ,SEEK_SET );
   fread( &pXRes, 4, 1, pFile );
   fread( &pYRes, 4, 1, pFile );
   
   fseek( pFile, lDataPos ,SEEK_SET );

   ASSERT( pXRes > 0);
   ASSERT( pYRes > 0);


   lReturnValue = new MR_UInt8[ pXRes*pYRes ];

   fread( lReturnValue, pXRes*pYRes, 1, pFile );

   return lReturnValue;
   
}

MR_UInt8* IMGRead( FILE* /*pFile*/, int& /*pXRes*/, int& /*pYRes*/ )
{
   printf( "ERROR: Unsupported format\n" );

   return NULL;
   
}



MR_UInt8* PCXRead( FILE* pFile, int& pXRes, int& pYRes )
{
   MR_UInt8* lReturnValue = NULL;
   
   MR_Int16 lStartX;
   MR_Int16 lStartY;
   MR_Int16 lSizeX;
   MR_Int16 lSizeY;
   MR_Int16 lNbBytesPerLine;
   
   fseek( pFile, 4 ,SEEK_SET );
   fread( &lStartX, 2, 1, pFile );
   fread( &lStartY, 2, 1, pFile );  
   fread( &lSizeX, 2, 1, pFile );
   fread( &lSizeY, 2, 1, pFile );

   fseek( pFile, 66 ,SEEK_SET );
   fread( &lNbBytesPerLine, 2, 1, pFile );

   pXRes = lSizeX-lStartX+1;
   pYRes = lSizeY-lStartY+1;

   fseek( pFile, 128 ,SEEK_SET );

   ASSERT( pXRes>0);
   ASSERT( pYRes>0);

   lReturnValue = new MR_UInt8[ pXRes*pYRes ];

   int lOffset = 0;

   for( int lLineNb = 0; lLineNb<pYRes; lLineNb++ )
   {
      for( int lColNb = 0; lColNb<lNbBytesPerLine;  )
      {
         MR_UInt8 lBuffer;
         MR_UInt8 lBuffer2;
      
         fread( &lBuffer, 1, 1, pFile );

         if( (lBuffer&0xc0)==0xc0 )
         {
            fread( &lBuffer2, 1, 1, pFile );
            /*
            lBuffer2++;
            if( lBuffer2 == 255 )
            {
               lBuffer2 = 0;
            }
            */
                    
            for( int lCounter = 0; lCounter<(lBuffer&0x3f); lCounter++ )
            {
               if( lColNb<pXRes )
               {    
                  lReturnValue[ lOffset++ ] = lBuffer2;
               }
               lColNb++;
            }
         }
         else
         {
            if( lColNb<pXRes )
            {
               /*
               lBuffer++;
               if( lBuffer2 == 255 )
               {
                  lBuffer2 = 0;
               }
               */
               lReturnValue[ lOffset++ ] = lBuffer;
            }
            lColNb++;
         }
      }
   }

   return lReturnValue;
}

