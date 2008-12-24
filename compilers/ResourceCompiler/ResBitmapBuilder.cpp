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

#include "../../engine/ColorTools/ColorTools.h"
#include "../../engine/VideoServices/ColorPalette.h"

MR_ResBitmapBuilder::MR_ResBitmapBuilder(int pResourceId, int pWidth, int pHeight)
:MR_ResBitmap(pResourceId)
{
	mWidth = pWidth;
	mHeight = pHeight;
}

BOOL MR_ResBitmapBuilder::BuildFromFile(const char *pFile, int pAntiAliasScheme)
{
	BOOL lReturnValue = TRUE;

	Pixel24 *lBuffer;

	lBuffer = LoadBitmap(pFile, mXRes, mYRes, TRUE);

	if(lBuffer == NULL) {
		lReturnValue = FALSE;
	}
	else {
		// Compute the number of required SubBitmap
		if(pAntiAliasScheme == 0) {
			mSubBitmapCount = 1;
		}
		else {
			mSubBitmapCount = 0;

			int lXRes = mXRes;
			int lYRes = mYRes;

			while((lXRes > 2) && (lYRes > 2)) {
				mSubBitmapCount++;

				lXRes /= 2;
				lYRes /= 2;
			}
		}

		// Alloc all the required memory
		mSubBitmapList = new SubBitmap[mSubBitmapCount];

		int lXRes = mXRes;
		int lYRes = mYRes;
		int lCounter = 0;

		while(lCounter < mSubBitmapCount) {
			mSubBitmapList[lCounter].mXRes = lXRes;
			mSubBitmapList[lCounter].mYRes = lYRes;
			mSubBitmapList[lCounter].mXResShiftFactor = lCounter;
			mSubBitmapList[lCounter].mYResShiftFactor = lCounter;
			mSubBitmapList[lCounter].mHaveTransparent = FALSE;

			mSubBitmapList[lCounter].mBuffer = new Pixel24[lXRes * lYRes];

			mSubBitmapList[lCounter].mColumnPtr = new Pixel24 *[mSubBitmapList[lCounter].mXRes];

			Pixel24 *lPtr = mSubBitmapList[lCounter].mBuffer;

			for(int lColumn = 0; lColumn < mSubBitmapList[lCounter].mXRes; lColumn++) {
				mSubBitmapList[lCounter].mColumnPtr[lColumn] = lPtr;
				lPtr += mSubBitmapList[lCounter].mYRes;
			}

			lXRes /= 2;
			lYRes /= 2;
			lCounter++;
		}

		ComputeIntermediateImages(lBuffer);
	}

	delete[] lBuffer;

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

/**
 * Use bilinear interpolation to resample a bitmap.
 *
 * @param pSrc Input bitmap
 * @param pSrc Output bitmap
 */
void MR_ResBitmapBuilder::Resample(const SubBitmap *pSrc, SubBitmap *pDest)
{
	for(int x = 0; x < pDest->mXRes; x++) {
		for(int y = 0; y < pDest->mYRes; y++) {
			// map x and y onto old coordinates
			double xmap = ((double) x / (double) pDest->mXRes) * pSrc->mXRes;
			double ymap = ((double) y / (double) pDest->mYRes) * pSrc->mYRes;

			// set up for interpolation
			double x1 = floor(xmap);
			double x2 = ceil(xmap);
			if(x1 == x2) 
				x2++;

			double y1 = floor(ymap);
			double y2 = ceil(ymap);
			if(y1 == y2)
				y2++;

			double f1, f2, g1, g2;
			f1 = (x2 - xmap);
			f2 = (xmap - x1);
			g1 = (y2 - ymap);
			g2 = (ymap - y1);

			Pixel24 *s11, *s12, *s21, *s22;
			s11 = &pSrc->mColumnPtr[(int) x1][(int) y1];
			s12 = &pSrc->mColumnPtr[(int) x1][(int) y2];
			s21 = &pSrc->mColumnPtr[(int) x2][(int) y1];
			s22 = &pSrc->mColumnPtr[(int) x2][(int) y2];

			Pixel24 r1, r2;
			r1.r = (int) ((f1 * s11->r) + (f2 * s12->r));
			r1.g = (int) ((f1 * s11->g) + (f2 * s12->g));
			r1.b = (int) ((f1 * s11->b) + (f2 * s12->b));

			r2.r = (int) ((f1 * s21->r) + (f2 * s22->r));
			r2.g = (int) ((f1 * s21->g) + (f2 * s22->g));
			r2.b = (int) ((f1 * s21->b) + (f2 * s22->b));

			pDest->mColumnPtr[x][y]->r = (int) ((g1 * r1.r) + (g2 * r2.r));
			pDest->mColumnPtr[x][y]->g = (int) ((g1 * r1.g) + (g2 * r2.g));
			pDest->mColumnPtr[x][y]->b = (int) ((g1 * r1.b) + (g2 * r2.b));
		}
	}
}

void MR_ResBitmapBuilder::ComputeIntermediateImages(Pixel24 *pBuffer)
{
	// First copy the original image
	if(mSubBitmapCount != 0) {
		int lBitmapSize;
		int lCounter;

		lBitmapSize = mSubBitmapList[0].mXRes * mSubBitmapList[0].mYRes;

		// Put 0 in all transparent bit
		int lNbTransparent = 0;
		int lNbBadColors = 0;

		// no need to check for bad colors anymore with 24-bit color
/*		for(lCounter = 0; lCounter < lBitmapSize; lCounter++) {
			if(pBuffer[lCounter] < MR_RESERVED_COLORS_BEGINNING) {
				if(pBuffer[lCounter] != 0) {
					pBuffer[lCounter] = 0;
					lNbBadColors++;
				}
				lNbTransparent++;
			}
		}

		if(lNbTransparent != 0) {
			if(lNbBadColors > 0) {
				printf("WARNING: This image have invalid colors\n");
			}
			printf("INFO: This image have transparent pixels\n");
		}*/

		// First copy the original image
		memcpy(mSubBitmapList[0].mBuffer, pBuffer, lBitmapSize);

		// Resample the sub bitmaps
		for(lCounter = 1; lCounter < mSubBitmapCount; lCounter++) {
			printf("INFO: Resampling...\n");
			Resample(&mSubBitmapList[0], &mSubBitmapList[lCounter]);
		}

		// Init the plain color
		printf("INFO: Computing plain color\n");
		// use an average of all pixels
		double rAvg = 0;
		double gAvg = 0;
		double bAvg = 0;

		for(double counter = 0; counter < (mSubBitmapList[0].mXRes * mSubBitmapList[0].mYRes); counter++) {
			rAvg = (counter / (counter + 1)) * rAvg + (1 / (counter + 1)) * mSubBitmapList[0].mBuffer[(int) counter].r;
			gAvg = (counter / (counter + 1)) * gAvg + (1 / (counter + 1)) * mSubBitmapList[0].mBuffer[(int) counter].g;
			bAvg = (counter / (counter + 1)) * bAvg + (1 / (counter + 1)) * mSubBitmapList[0].mBuffer[(int) counter].b;
		}

		mPlainColor.r = (int) rAvg;
		mPlainColor.g = (int) gAvg;
		mPlainColor.b = (int) bAvg;
	}
	else {
		printf("WARNING: Bitmap too small, using single color\n");

		mPlainColor = *pBuffer;
	}
}