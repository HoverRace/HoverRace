// ResBitmapBuilder.cpp
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
#include "ResBitmapBuilder.h"
#include "BitmapHelper.h"

#include "../ColorTools/ColorTools.h"
#include "../VideoServices/ColorPalette.h"


// Local prototypes
static MR_UInt8   GetBestColor( MR_UInt8* pSrc, int pStripeLenToScan, int pNbStripeToScan, int pStripeLen, BOOL& pTransparentFlag );
static MR_UInt8   GetBestColorWithError( MR_UInt8* pSrc, int pStripeLenToScan, int pNbStripeToScan, int pStripeLen, BOOL& pTransparentFlag, double& mRedError, double& mGreenError, double& mBlueError );


MR_ResBitmapBuilder::MR_ResBitmapBuilder( int pResourceId, int pWidth, int pHeight )
                    :MR_ResBitmap( pResourceId )
{
   mWidth  = pWidth;
   mHeight = pHeight;
}

BOOL MR_ResBitmapBuilder::BuildFromFile( const char* pFile, int pAntiAliasScheme )
{
   BOOL lReturnValue = TRUE;

   MR_UInt8* lBuffer;
   
   lBuffer = LoadBitmap( pFile, mXRes, mYRes, TRUE );


   if( lBuffer == NULL )
   {
      lReturnValue = FALSE;
   }
   else
   {

      // Compute the number of required SubBitmap
      if( pAntiAliasScheme == 0 )
      {
         mSubBitmapCount = 1;
      }
      else
      {
         mSubBitmapCount = 0;

         int lXRes = mXRes;
         int lYRes = mYRes;

         while( (lXRes > 2)&&(lYRes>2) )
         {
            mSubBitmapCount++;
      
            lXRes /= 2;
            lYRes /= 2;
         }
      }

      // Alloc all the required memory
      mSubBitmapList = new SubBitmap[ mSubBitmapCount ];

      int lXRes = mXRes;
      int lYRes = mYRes;
      int lCounter = 0;

      while( lCounter<mSubBitmapCount )
      {
         mSubBitmapList[ lCounter ].mXRes            = lXRes;
         mSubBitmapList[ lCounter ].mYRes            = lYRes;
         mSubBitmapList[ lCounter ].mXResShiftFactor = lCounter;
         mSubBitmapList[ lCounter ].mYResShiftFactor = lCounter;
         mSubBitmapList[ lCounter ].mHaveTransparent = FALSE;

         mSubBitmapList[ lCounter ].mBuffer    = new MR_UInt8[ lXRes*lYRes ];

         mSubBitmapList[ lCounter ].mColumnPtr = new MR_UInt8*[ mSubBitmapList[ lCounter ].mXRes ];

         MR_UInt8* lPtr = mSubBitmapList[ lCounter ].mBuffer;

         for( int lColumn = 0; lColumn < mSubBitmapList[ lCounter ].mXRes; lColumn++ )
         {
            mSubBitmapList[ lCounter ].mColumnPtr[ lColumn ] = lPtr;
            lPtr += mSubBitmapList[ lCounter ].mYRes;
         }
      
         lXRes /= 2;
         lYRes /= 2;
         lCounter++;
      }

      ComputeIntermediateImages( lBuffer );   
   }

   delete []lBuffer;

   return lReturnValue;
}

/*
void MR_ResBitmapBuilder::Resample( const SubBitmap* pSrc, SubBitmap* pDest )
{
   int lX;
   int lY;

   int lXPitch = 1024*pSrc->mXRes/pDest->mXRes;
   int lYPitch = 1024*pSrc->mYRes/pDest->mYRes;

   int lXSrc = 0;

   double* lPCRedError   = new double[ pDest->mYRes ];
   double* lPCGreenError = new double[ pDest->mYRes ];
   double* lPCBlueError  = new double[ pDest->mYRes ];


   for( lY = 0; lY<pDest->mYRes; lY++ )
   {
      lPCRedError  [ lY ] = 0.0;
      lPCGreenError[ lY ] = 0.0;
      lPCBlueError [ lY ] = 0.0;
   }


   pDest->mHaveTransparent = FALSE;

   for( lX = 0; lX<pDest->mXRes; lX++ )
   {
      int lYSrc = 0;

      double lPRRedError   = 0;
      double lPRGreenError = 0;
      double lPRBlueError  = 0;

      for( lY = 0; lY<pDest->mYRes; lY++ )
      {

         lPRRedError   = lPRRedError*0.30   + lPCRedError[ lY ]*0.30;
         lPRGreenError = lPRGreenError*0.30 + lPCGreenError[ lY ]*0.30;
         lPRBlueError  = lPRBlueError*0.30  + lPCBlueError[ lY ]*0.30;

         pDest->mColumnPtr[lX][lY] = GetBestColorWithError( &(pSrc->mColumnPtr[lXSrc/1024][lYSrc/1024]),
                                                            lYPitch/1024,
                                                            lXPitch/1024,
                                                            pSrc->mYRes,
                                                            pDest->mHaveTransparent,
                                                            lPRRedError,
                                                            lPRGreenError,
                                                            lPRBlueError   );

         lYSrc += lYPitch;

         lPCRedError[ lY ]    = lPRRedError;
         lPCGreenError[ lY ]  = lPRGreenError;
         lPCBlueError[ lY ]   = lPRBlueError;
      }
      lXSrc += lXPitch;
   }

   delete lPCRedError;
   delete lPCGreenError;
   delete lPCBlueError;

}
*/

void MR_ResBitmapBuilder::Resample( const SubBitmap* pSrc, SubBitmap* pDest )
{   

   int lX;
   int lY;

   // Create a continous tiled bitmap
   MR_UInt8* lSrc = new MR_UInt8[ pSrc->mXRes*3*pSrc->mYRes*3 ];

   for( lX = 0; lX<pSrc->mXRes*3; lX++ )
   {
      for( lY = 0; lY<pSrc->mYRes*3; lY++ )
      {
         lSrc[lX*pSrc->mYRes*3 + lY ] = pSrc->mColumnPtr[lX%pSrc->mXRes][lY%pSrc->mYRes];
      }
   }

   int lXPitch = 1024*pSrc->mXRes/pDest->mXRes;
   int lYPitch = 1024*pSrc->mYRes/pDest->mYRes;

   int lXSrc = 0;

   double* lPCRedError   = new double[ pDest->mYRes ];
   double* lPCGreenError = new double[ pDest->mYRes ];
   double* lPCBlueError  = new double[ pDest->mYRes ];


   for( lY = 0; lY<pDest->mYRes; lY++ )
   {
      lPCRedError  [ lY ] = 0.0;
      lPCGreenError[ lY ] = 0.0;
      lPCBlueError [ lY ] = 0.0;
   }


   pDest->mHaveTransparent = FALSE;

   for( lX = 0; lX<pDest->mXRes; lX++ )
   {
      int lYSrc = 0;

      double lPRRedError   = 0;
      double lPRGreenError = 0;
      double lPRBlueError  = 0;

      for( lY = 0; lY<pDest->mYRes; lY++ )
      {

         lPRRedError   = lPRRedError*0.30   + lPCRedError[ lY ]*0.30;
         lPRGreenError = lPRGreenError*0.30 + lPCGreenError[ lY ]*0.30;
         lPRBlueError  = lPRBlueError*0.30  + lPCBlueError[ lY ]*0.30;

         int lXSrcStart = (lXSrc -lXPitch/2)/1024   +pSrc->mXRes;
         int lXSrcEnd   = (lXSrc +3*lXPitch/2)/1024 +pSrc->mXRes;

         int lYSrcStart = (lYSrc -lYPitch/2)/1024   +pSrc->mYRes;
         int lYSrcEnd   = (lYSrc +3*lXPitch/2)/1024 +pSrc->mYRes;

         /*
         if( lXSrcStart < 0 )
         {
            lXSrcStart = 0;
         }

         if( lXSrcEnd > pSrc->mXRes )
         {
            lXSrcEnd = pSrc->mXRes;
         }

         if( lYSrcStart < 0 )
         {
            lYSrcStart = 0;
         }

         if( lYSrcEnd > pSrc->mYRes )
         {
            lYSrcEnd = pSrc->mYRes;
         }
         */
         

         pDest->mColumnPtr[lX][lY] = GetBestColorWithError( &(lSrc[lXSrcStart*pSrc->mYRes*3 + lYSrcStart ]), //&(pSrc->mColumnPtr[lXSrcStart][lYSrcStart]),
                                                            lYSrcEnd-lYSrcStart,
                                                            lXSrcEnd-lXSrcStart,
                                                            pSrc->mYRes*3,
                                                            pDest->mHaveTransparent,
                                                            lPRRedError,
                                                            lPRGreenError,
                                                            lPRBlueError   );

         lYSrc += lYPitch;

         lPCRedError[ lY ]    = lPRRedError;
         lPCGreenError[ lY ]  = lPRGreenError;
         lPCBlueError[ lY ]   = lPRBlueError;
      }
      lXSrc += lXPitch;
   }

   delete []lPCRedError;
   delete []lPCGreenError;
   delete []lPCBlueError;

   delete []lSrc;
}



void MR_ResBitmapBuilder::ComputeIntermediateImages( MR_UInt8* pBuffer )
{

   // First copy the original image


   if( mSubBitmapCount != 0 )
   {
      int lBitmapSize;
      int lCounter;

      lBitmapSize = mSubBitmapList[0].mXRes*mSubBitmapList[0].mYRes;

      // Put 0 in all transparent bit
      int lNbTransparent = 0;
      int lNbBadColors   = 0;

      for( lCounter = 0; lCounter < lBitmapSize; lCounter++ )
      {
         if( pBuffer[ lCounter ] < MR_RESERVED_COLORS_BEGINNING )
         {
            if( pBuffer[ lCounter ] != 0 )
            {
               pBuffer[ lCounter ] = 0;
               lNbBadColors++;
            }
            lNbTransparent++;
         }
      }

      if( lNbTransparent != 0 )
      {
         if( lNbBadColors > 0 )
         {
            printf( "WARNING: This image have invalid colors\n");
         }
         printf( "INFO: This image have transparent pixels\n");
      }         
      
      // First copy the original image      
      memcpy( mSubBitmapList[0].mBuffer, pBuffer, lBitmapSize );

      // Resample the sub bitmaps
      for( lCounter = 1; lCounter<mSubBitmapCount; lCounter++ )
      {
         printf( "INFO: Resampling...\n" );
         Resample( &mSubBitmapList[0], &mSubBitmapList[ lCounter ] );
      }


      // Init the plain color
      BOOL lDummyBool;

      printf( "INFO: Computing plain color\n" );

      mPlainColor = GetBestColor( mSubBitmapList[0].mBuffer,
                                  mSubBitmapList[0].mYRes,
                                  mSubBitmapList[0].mXRes,
                                  mSubBitmapList[0].mYRes,
                                  lDummyBool                );
   }
   else
   {
      printf( "WARNING: Bitmap too small, using single color\n" );

      mPlainColor = *pBuffer;
   }   
}






// Local functions implementation


MR_UInt8 GetBestColor( MR_UInt8* pSrc, int pStripeLenToScan, int pNbStripeToScan, int pStripeLen, BOOL& pTransparentFlag )
{
   MR_UInt8 lReturnValue;
   int lNbPoints      = pStripeLenToScan*pNbStripeToScan;
   int lNbTransparent = 0;

   double lTotalRed   = 0;
   double lTotalGreen = 0;
   double lTotalBlue  = 0;

   for( int lStripe = 0; lStripe< pNbStripeToScan; lStripe++ )
   {
      
      double lStripeRed   = 0; // Intermediate results to reduce rounding errors
      double lStripeGreen = 0;
      double lStripeBlue  = 0;

      for( int lCounter = 0; lCounter< pStripeLenToScan; lCounter++ )
      {
         if( pSrc[ lCounter ] == 0 )
         {
            lNbTransparent++;
         }
         else
         {
            double lPointRed;
            double lPointGreen;
            double lPointBlue;

            MR_ColorTools::GetComponents( pSrc[ lCounter ]-MR_RESERVED_COLORS_BEGINNING, lPointRed, lPointGreen, lPointBlue );

            lStripeRed   += lPointRed;
            lStripeGreen += lPointGreen;
            lStripeBlue  += lPointBlue;
         }
      }
      pSrc += pStripeLen;

      lTotalRed   += lStripeRed;
      lTotalGreen += lStripeGreen;
      lTotalBlue  += lStripeBlue;
   }


   if( lNbTransparent > lNbPoints/2 )
   {
      lReturnValue = 0;
      pTransparentFlag = TRUE;
   }
   else
   {
      lReturnValue = MR_RESERVED_COLORS_BEGINNING 
                    +MR_ColorTools::GetNearest( lTotalRed  /(lNbPoints-lNbTransparent),
                                                lTotalGreen/(lNbPoints-lNbTransparent),
                                                lTotalBlue /(lNbPoints-lNbTransparent) );

   }
   return lReturnValue;
}

MR_UInt8 GetBestColorWithError( MR_UInt8* pSrc, 
                                int pStripeLenToScan, 
                                int pNbStripeToScan,
                                int pStripeLen,
                                BOOL& pTransparentFlag,
                                double& pRedError,
                                double& pGreenError,
                                double& pBlueError      )
{
   MR_UInt8 lReturnValue;
   int lNbPoints      = pStripeLenToScan*pNbStripeToScan;
   int lNbTransparent = 0;

   double lTotalRed   = 0;
   double lTotalGreen = 0;
   double lTotalBlue  = 0;

   // remove error peeks

   if( pRedError < -0.20 )
   {
      pRedError = -0.20;
   }
   else if( pRedError > 0.20 )
   {
      pRedError = 0.20;
   }

   if( pGreenError < -0.20 )
   {
      pGreenError = -0.20;
   }
   else if( pGreenError > 0.20 )
   {
      pGreenError = 0.20;
   }
      
   if( pBlueError < -0.20 )
   {
      pBlueError = -0.20;
   }
   else if( pBlueError > 0.20 )
   {
      pBlueError = 0.20;
   }


   for( int lStripe = 0; lStripe< pNbStripeToScan; lStripe++ )
   {
      
      double lStripeRed   = 0; // Intermediate results to reduce rounding errors
      double lStripeGreen = 0;
      double lStripeBlue  = 0;

      for( int lCounter = 0; lCounter< pStripeLenToScan; lCounter++ )
      {
         if( pSrc[ lCounter ] == 0 )
         {
            lNbTransparent++;
         }
         else
         {
            double lPointRed;
            double lPointGreen;
            double lPointBlue;

            MR_ColorTools::GetComponents( pSrc[ lCounter ]-MR_RESERVED_COLORS_BEGINNING, lPointRed, lPointGreen, lPointBlue );

            // Give more weight to central points

            /*
            if( (pStripeLenToScan>=3)&&(pNbStripeToScan >= 3) )
            {
               if( (lCounter >= pStripeLenToScan/3)&&(lCounter <= (pStripeLenToScan-pStripeLenToScan/3) ))
               {
                  if( (lStripe >= pNbStripeToScan/3)&&(lStripe <= (pNbStripeToScan-pNbStripeToScan/3) ))
                  {
                     lNbPoints++;

                     lStripeRed   += lPointRed;
                     lStripeGreen += lPointGreen;
                     lStripeBlue  += lPointBlue;
                  }
               }
            }
            */

            lStripeRed   += lPointRed;
            lStripeGreen += lPointGreen;
            lStripeBlue  += lPointBlue;
         }
      }
      pSrc += pStripeLen;

      lTotalRed   += lStripeRed;
      lTotalGreen += lStripeGreen;
      lTotalBlue  += lStripeBlue;
   }

  
   if( lNbTransparent > lNbPoints/2 )
   {
      lReturnValue = 0;
      pTransparentFlag = TRUE;

      pRedError    = 0.0;
      pGreenError  = 0.0;
      pBlueError   = 0.0;

   }
   else
   {
      lTotalRed   /= (lNbPoints-lNbTransparent);
      lTotalGreen /= (lNbPoints-lNbTransparent);
      lTotalBlue  /= (lNbPoints-lNbTransparent);

      lTotalRed   += pRedError;
      lTotalGreen += pGreenError;
      lTotalBlue  += pBlueError;

      lReturnValue = MR_RESERVED_COLORS_BEGINNING
                    +MR_ColorTools::GetNearest( lTotalRed,
                                                lTotalGreen,
                                                lTotalBlue   );

      MR_ColorTools::GetComponents( lReturnValue-MR_RESERVED_COLORS_BEGINNING, pRedError, pGreenError, pBlueError );

      pRedError   = lTotalRed   - pRedError;
      pGreenError = lTotalGreen - pGreenError;
      pBlueError  = lTotalBlue  - pBlueError;

   }
   return lReturnValue;
}

