// 3DViewportRendering.cpp
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

#include "stdafx.h"

#include "3DViewport.h"
#include "../Util/Profiler.h"


// #pragma optimize( "atw", on )

// Local constants
#define MR_PIXEL_FRACT 2048

// Local structures
struct MR_ColumnBltParam
{
   int         mColumn;
   MR_UInt8**  mBuffer;
   int         mYScreenStart_4096;
   int         mYScreenEnd_4096;
   int         mBufferLen;
   int         mBufferStep;
   MR_UInt16** mZBuffer;
   int         mZBufferStep;
   MR_UInt16   mZ;
   MR_UInt8*   mBitmap;
   int         mPixelStep;
   int         mBitmapColMask;
   MR_UInt8    mLightIntensity;
   MR_UInt8    mColor;
};

struct MR_LineBltParam
{
   MR_UInt8*  mBuffer;
   int        mBltLen;
   MR_UInt16* mZBuffer;
   MR_UInt16  mZ;
   MR_UInt8** mBitmap;
   MR_UInt32  mBitmapColMask;
   MR_UInt32  mBitmapRowMask;
   MR_UInt32  mBitmapCol_4096;
   MR_UInt32  mBitmapRow_4096;
   MR_UInt32  mBitmapColInc_4096;
   MR_UInt32  mBitmapRowInc_4096;

   MR_UInt8   mLightIntensity;
   MR_UInt8   mColor;
};


struct MR_TriangleDrawInfo
{
   MR_UInt8**  mBuffer;
   int         mLineLen;
   int         mXRes;
   int         mYRes;

   MR_UInt16** mZBuffer;
   int         mZLineLen;

   int         mVertexList[3];
   MR_Int32    mBitmapRow_4096[3];
   MR_Int32    mBitmapCol_4096[3];

   MR_UInt8**  mBitmap;
   MR_UInt32   mBitmapColMask;
   MR_UInt32   mBitmapRowMask;

   MR_UInt8    mLightIntensity;
   MR_UInt8    mColor;
};


// Local variables .. for fast rendering speed
static struct MR_ColumnBltParam   gsColumnBltParam;
static struct MR_LineBltParam     gsLineBltParam;
static struct MR_TriangleDrawInfo gsTriangleBltParam;


// Local functions
/*
static void InterpolateLine( const MR_3DCoordinate& p0, 
                             const MR_3DCoordinate& p1,
                             MR_3DCoordinate& pMid );
*/
static void BltPlainColumn();
static void BltColumn();
static void BltColumnWithTransparent();

static void BltPlainLineNoZCheck();
static void BltLineNoZCheck();
static void BltLineNoZCheckWithTransparent();

static void BltPlainTriangle();
static void BltTriangle();

// Local Macros


//
// Floor and Ceiling rendering
//
void MR_3DViewPort::RenderWallSurface( const MR_3DCoordinate& pUpperLeft, const MR_3DCoordinate& pLowerRight, MR_Int32 pLen, const MR_Bitmap* pBitmap )
{
   RenderAlternateWallSurface( pUpperLeft, pLowerRight, pLen, pBitmap, pBitmap, 1, 0 );
}

void MR_3DViewPort::RenderAlternateWallSurface( const MR_3DCoordinate& pUpperLeft, const MR_3DCoordinate& pLowerRight, MR_Int32 pLen, const MR_Bitmap* pBitmap, const MR_Bitmap* pBitmap2, int pSerialLen, int pSerialStart )
{       

   // DEBUG -- prin bitmap
   /* 
   MR_UInt8* lSrcTest  = pBitmap->GetBuffer(0);
   MR_UInt8* lDestTest = mBuffer;

   for( int lXTest = 0; lXTest<128; lXTest++ )
   {
      for( int lYTest = 0; lYTest<128; lYTest++ )
      {
         lDestTest[ lYTest ] = *lSrcTest;
         lSrcTest++;
      }
      lDestTest += mLineLen;
   }
  
   return;
   */

   // Debug Print color tab
   /*
   for( int lDebugColor = 0; lDebugColor<256; lDebugColor++ )
   {
      for( int lX = 0; lX<6; lX++ )
      {
         for( int lY = 0; lY<6; lY++ )
         {
            mBuffer[ (lDebugColor%16)*6+lX + mLineLen*((lDebugColor/16)*6+lY) ] = lDebugColor;
         }
      }
   }
   */

  

   // Basic formulas
   // lLen = (lColumn*mXVariationPerYInc*lY0Wall - lX0Wall)
   //        /(lXWallVariationPerMM - lColumn*mXVariationPerYInc*lYWallVariationPerMM)
   //
   // lDepth = lLen*lYWallVariationPerMM
   //
   // lLen                 = Len in mm of the intersection from the origin of the wall
   // lColumn              = XScreen coordinate from the center of the screen
   // mXVariationPerYInc   = The Horizontal deviation of the ray
   // lY0Wall and lX0Wall  = Origin of the wall
   // lXWallVariationPerMM,
   // lYWallVariationPerMM = Slope of the wall
   // lDepth               = Depth of the intersection for ZBuffering
   //
   //


   // Verify that we are on the good side of the plan

   if(  Int32x32To64( pLowerRight.mY-pUpperLeft.mY, pUpperLeft.mX-mPosition.mX )
       +Int32x32To64(-pLowerRight.mX+pUpperLeft.mX, pUpperLeft.mY-mPosition.mY )  >= 0 )
   {    
      // Wrong side
      return;
   }


   // Rotation and translation
   MR_3DCoordinate lRotated0;
   MR_3DCoordinate lRotated1;

   ApplyRotationMatrix( pUpperLeft,  lRotated0 );
   ApplyRotationMatrix( pLowerRight, lRotated1 );


   // Cut if out of Depth allowed domain

   MR_2DCoordinate lCutted0;
   
   MR_2DCoordinate lCutted1;

   if( lRotated0.mX<mPlanDist )
   {
      if( lRotated1.mX<mPlanDist )
      {
         // behind projection plan
         return;
      }

      lCutted0.mX    = mPlanDist;
      lCutted0.mY    = lRotated0.mY + MulDiv( lRotated1.mY-lRotated0.mY, mPlanDist-lRotated0.mX, lRotated1.mX-lRotated0.mX );
   }
   else if( lRotated0.mX > MR_ZBUFFER_UNIT*0xFFFE )
   {
      if( lRotated1.mX > MR_ZBUFFER_UNIT*0xFFFE )
      {
         // too far away
         return;
      }

      lCutted0.mX    = MR_ZBUFFER_UNIT*0xFFFE;
      lCutted0.mY    = lRotated0.mY + MulDiv( lRotated1.mY-lRotated0.mY, MR_ZBUFFER_UNIT*0xFFFE-lRotated0.mX, lRotated1.mX-lRotated0.mX );
   }
   else
   {
      lCutted0 = lRotated0;
   }

   
   if( lRotated1.mX<mPlanDist )
   {
      lCutted1.mX    = mPlanDist;
      lCutted1.mY    = lRotated0.mY + MulDiv( lRotated1.mY-lRotated0.mY, mPlanDist-lRotated0.mX, lRotated1.mX-lRotated0.mX );
   }
   else if( lRotated0.mX > MR_ZBUFFER_UNIT*0xFFFE )
   {

      lCutted1.mX    = MR_ZBUFFER_UNIT*0xFFFE;
      lCutted1.mY    = lRotated0.mY + MulDiv( lRotated1.mY-lRotated0.mY, MR_ZBUFFER_UNIT*0xFFFE-lRotated0.mX, lRotated1.mX-lRotated0.mX );

   }
   else
   {
      lCutted1 = lRotated1;
   }

   // Compute screen coordinates
   int lScreenX0 = MulDiv( -lCutted0.mY, mXRes_PlanDist, lCutted0.mX*mPlanHW*2 )+mXRes/2;
   int lScreenX1 = MulDiv( -lCutted1.mY, mXRes_PlanDist, lCutted1.mX*mPlanHW*2 )+mXRes/2;

   if( lScreenX1 <= lScreenX0 )
   {
      return;
   }
   

   if( (lScreenX1 <= 0)||(lScreenX0>=mXRes) )
   {
      // Out of horizontasl wiewing field
      return;
   }

   MR_Int32 lLevel0_YRes_PlanDist_PlanVW_2 = MulDiv( lRotated0.mZ, mYRes_PlanDist, mPlanVW*2 );
   MR_Int32 lLevel1_YRes_PlanDist_PlanVW_2 = MulDiv( lRotated1.mZ, mYRes_PlanDist, mPlanVW*2 );

   int lScreenY0Top     = -lLevel0_YRes_PlanDist_PlanVW_2/lCutted0.mX+mYRes/2+mScroll;
   int lScreenY0Bottom  = -lLevel1_YRes_PlanDist_PlanVW_2/lCutted0.mX+mYRes/2+mScroll;
   int lScreenY1Top     = -lLevel0_YRes_PlanDist_PlanVW_2/lCutted1.mX+mYRes/2+mScroll;
   int lScreenY1Bottom  = -lLevel1_YRes_PlanDist_PlanVW_2/lCutted1.mX+mYRes/2+mScroll;


   /*
   int lScreenY0Top     = -MulDiv( lRotated0.mZ, mYRes_PlanDist, lCutted0.mX*mPlanVW*2 )+mYRes/2+mScroll;
   int lScreenY0Bottom  = -MulDiv( lRotated1.mZ, mYRes_PlanDist, lCutted0.mX*mPlanVW*2 )+mYRes/2+mScroll;
   int lScreenY1Top     = -MulDiv( lRotated0.mZ, mYRes_PlanDist, lCutted1.mX*mPlanVW*2 )+mYRes/2+mScroll;
   int lScreenY1Bottom  = -MulDiv( lRotated1.mZ, mYRes_PlanDist, lCutted1.mX*mPlanVW*2 )+mYRes/2+mScroll;
   */

   // Select the bitmap that will be displayed
   // This is realy not the good way to do it.. we will get low resolution bitmap for dutted surfaces


   // Precomput surface contour
   int lDYTop_4096;       // Slope of the top of the wall
   int lDYBottom_4096;    // slope of the bottom of the wall
   int lYTop_4096     = lScreenY0Top*4096;
   int lYBottom_4096  = lScreenY0Bottom*4096;

   MR_Int32 lRatioVariationPerColumn_16384;

   if( lScreenX1 > lScreenX0 )
   {
      lDYTop_4096    = ((lScreenY1Top-lScreenY0Top)*4096)/(lScreenX1-lScreenX0 /*-1*/);
      lDYBottom_4096 = ((lScreenY1Bottom-lScreenY0Bottom)*4096)/(lScreenX1-lScreenX0/*-1*/);
      
      lRatioVariationPerColumn_16384 = ((lScreenY0Bottom-lScreenY0Top)-(lScreenY1Bottom-lScreenY1Top))*16384/((lScreenY0Bottom-lScreenY0Top)*(lScreenX1-lScreenX0));
   }
   else
   {
      lDYTop_4096    = 0;
      lDYBottom_4096 = 0;

      lRatioVariationPerColumn_16384 = 0;
   }


   // Out of horizontal field cut

   if( lScreenX0 < 0 )
   {  
      if( (-lScreenX0 <= 2*lScreenX1) /*|| (lScreenX1>mXRes)*/ )
      {
         lYTop_4096    += (0-lScreenX0)*lDYTop_4096;
         lYBottom_4096 += (0-lScreenX0)*lDYBottom_4096;
      }
      else
      {
         lYTop_4096    = (lScreenY1Top*4096)-(lScreenX1)*lDYTop_4096;
         lYBottom_4096 = (lScreenY1Bottom*4096)-(lScreenX1)*lDYBottom_4096;
      }
      lScreenX0 = 0;
   }

   if( lScreenX1 > mXRes )
   {
      lScreenX1 = mXRes;
   }


   // Precompute rendering constants
   
   // lLen = (lColumn*mXVariationPerYInc*lY0Wall - lX0Wall)
   //        /(lXWallVariationPerMM - lColumn*mXVariationPerYInc*lYWallVariationPerMM)
   //
   // lDepth = lLen*lYWallVariationPerMM
   //
   // lBitmapColumn = lLen*mBitmapXRes/mBitmapWidth
   //
   MR_Int32 lXWallVariationPerMM_16384 = (16384*(lRotated1.mX-lRotated0.mX))/pLen;
   MR_Int32 lYWallVariationPerMM_16384 = (16384*(lRotated1.mY-lRotated0.mY))/pLen;

   MR_Int32 lHVarPerDInc_X0Wall_16384        = (mHVarPerDInc_16384*lRotated0.mX);
   MR_Int32 lHVarPerDInc_XWallVarPerMM_16384 = -(mHVarPerDInc_16384*lXWallVariationPerMM_16384)/(16384);

   MR_Int32 lColumn_HVarPerDInc_X0Wall_Y0Wall_16384               = (lScreenX0-mXRes/2)*lHVarPerDInc_X0Wall_16384 - lRotated0.mY*16384;
   MR_Int32 lColumn_HVarPerDInc_XWallVarPerMM_YWallVarPerMM_16384 = (lScreenX0-mXRes/2)*lHVarPerDInc_XWallVarPerMM_16384 + lYWallVariationPerMM_16384;


   if( lDYTop_4096 < 0 )
   {
      lYTop_4096    += lDYTop_4096;
   }

   if( lDYBottom_4096 > 0 )
   {
      lYBottom_4096 += lDYBottom_4096;
   }


   // Bitmap attributes
   int lBitmapXRes   = pBitmap->GetMaxXRes( );
   int lBitmapYRes   = pBitmap->GetMaxYRes( );

   // Prefill the rendering structure
   gsColumnBltParam.mBuffer       = mBufferLine;
   gsColumnBltParam.mColumn       = lScreenX0;
   gsColumnBltParam.mBufferLen    = mYRes;
   gsColumnBltParam.mBufferStep   = mLineLen;
   gsColumnBltParam.mZBuffer      = mZBufferLine;
   gsColumnBltParam.mZBufferStep  = mZLineLen;
   gsColumnBltParam.mColor        = pBitmap->GetPlainColor();


   MR_Int32 lBitmapXRes_BitmapWidth      = (lBitmapXRes*MR_PIXEL_FRACT)/pBitmap->GetWidth();
   MR_Int32 lNbBitmapInHeight_4096       = ((pUpperLeft.mZ-pLowerRight.mZ)*4096)/pBitmap->GetHeight();
   MR_Int32 lNbBitmapInHeight_BitmapYRes = (lBitmapYRes*MR_PIXEL_FRACT*(pUpperLeft.mZ-pLowerRight.mZ))/pBitmap->GetHeight();
   MR_Int32 lBitmapHeight_256            = MulDiv(lYBottom_4096-lYTop_4096, 256, lNbBitmapInHeight_4096 );
   MR_Int32 lBitmapHeightVar_256         = (lDYBottom_4096-lDYTop_4096)*256/lNbBitmapInHeight_4096;

   int lPrevColumn = -1;

   for( int lColumn = lScreenX0; lColumn< lScreenX1; lColumn++ )
   {

      // Screen coordinate
      if( ( lYBottom_4096>0 )&& ( (lYTop_4096/4096)<mYRes ) &&((lYBottom_4096-lYTop_4096)/4096 != 0 ) )
      {

         MR_Int32 lLen_4;
         
         if( lColumn_HVarPerDInc_XWallVarPerMM_YWallVarPerMM_16384/4 != 0 )
         {
            lLen_4 = lColumn_HVarPerDInc_X0Wall_Y0Wall_16384/(lColumn_HVarPerDInc_XWallVarPerMM_YWallVarPerMM_16384/4);

            if( lLen_4 < 0 )
            {
               lLen_4 = 0;
            }
            else if( lLen_4 > pLen*4 )
            {
               lLen_4 = pLen*4;
            }
         }
         else
         {
            lLen_4 = pLen*2;
         }

         // Depth computation
         int lDepth = lRotated0.mX + Int64ShraMod32( Int32x32To64( lLen_4,lXWallVariationPerMM_16384), 16 /*1024*16384*/);

         if( lDepth < mPlanDist )
         {
            lDepth = mPlanDist;
         }

         lDepth /= MR_ZBUFFER_UNIT;

         if( lDepth > MR_ZBUFFER_LIMIT )
         {
            lDepth = MR_ZBUFFER_LIMIT;
         }


         // Bitmap selection
         // int lSelectedBitmap = pBitmap->GetBestBitmapForYRes( (lYBottom_4096-lYTop_4096)/lNbBitmapInHeight_4096 );
         int lSelectedBitmap = pBitmap->GetBestBitmapForYRes( lBitmapHeight_256/256 );

         gsColumnBltParam.mColumn              = lColumn;
         gsColumnBltParam.mYScreenStart_4096   = lYTop_4096;
         gsColumnBltParam.mYScreenEnd_4096     = lYBottom_4096+2*4096;
         gsColumnBltParam.mLightIntensity = MR_NORMAL_INTENSITY; //-(lDepth/256);
         gsColumnBltParam.mZ              = (MR_UInt16)lDepth;
            
         if( lSelectedBitmap == -1 )
         {
            BltPlainColumn();
         }
         else
         {
            int lBitmapColumn = lBitmapXRes_BitmapWidth*lLen_4/(4*MR_PIXEL_FRACT);

            lBitmapColumn &= (lBitmapXRes-1); // Fast but work only with pow of 2   

            if( lBitmapColumn < lPrevColumn )
            {
               pSerialStart--;
               if( pSerialStart <  0 )
               {
                  pSerialStart = pSerialLen-1;
               }
            }                        
            lPrevColumn = lBitmapColumn;            

            lBitmapColumn >>= pBitmap->GetXResShiftFactor( lSelectedBitmap );

            if( pSerialStart == 0 )
            {
               gsColumnBltParam.mBitmap         = pBitmap2->GetColumnBuffer( lSelectedBitmap, lBitmapColumn );
            }
            else
            {
               gsColumnBltParam.mBitmap         = pBitmap->GetColumnBuffer( lSelectedBitmap, lBitmapColumn );
            }

            gsColumnBltParam.mPixelStep      = (lNbBitmapInHeight_BitmapYRes*64/((lYBottom_4096-lYTop_4096)/64) )>>pBitmap->GetYResShiftFactor( lSelectedBitmap );
            gsColumnBltParam.mBitmapColMask  = pBitmap->GetXRes( lSelectedBitmap)-1;
            BltColumn();
         }


      }

      // Prepare the next column

      lYTop_4096    += lDYTop_4096;
      lYBottom_4096 += lDYBottom_4096;

      lColumn_HVarPerDInc_X0Wall_Y0Wall_16384               += lHVarPerDInc_X0Wall_16384;
      lColumn_HVarPerDInc_XWallVarPerMM_YWallVarPerMM_16384 += lHVarPerDInc_XWallVarPerMM_16384;

      lBitmapHeight_256  += lBitmapHeightVar_256;
   }
}



// Local functions implementation

void BltPlainColumn()
{
   MR_UInt8*  lBuffer;
   MR_UInt16* lZBuffer;
   int        lNbPoints;
   MR_UInt8   lColor = gsColumnBltParam.mColor;
   // MR_UInt8   lColor = MR_ColorTable[ gsColumnBltParam.mLightIntensity ][ gsColumnBltParam.mColor ];

   if( gsColumnBltParam.mYScreenStart_4096 < 0 )
   {
      lBuffer       = gsColumnBltParam.mBuffer[0]+gsColumnBltParam.mColumn;
      lZBuffer      = gsColumnBltParam.mZBuffer[0]+gsColumnBltParam.mColumn;
      lNbPoints     = 0;

   }
   else
   {
      lBuffer       = gsColumnBltParam.mBuffer[ gsColumnBltParam.mYScreenStart_4096/4096]+gsColumnBltParam.mColumn;
      lZBuffer      = gsColumnBltParam.mZBuffer[ gsColumnBltParam.mYScreenStart_4096/4096]+gsColumnBltParam.mColumn;
      lNbPoints     = -gsColumnBltParam.mYScreenStart_4096/4096;
   }

   if( gsColumnBltParam.mYScreenEnd_4096/4096 < gsColumnBltParam.mBufferLen )
   {
      lNbPoints += gsColumnBltParam.mYScreenEnd_4096/4096;
   }
   else
   {
      lNbPoints += gsColumnBltParam.mBufferLen;
   }

   for( int lCounter = 0; lCounter < lNbPoints; lCounter++ )
   {
      if( *lZBuffer >= gsColumnBltParam.mZ )
      {
         *lBuffer  =  lColor;
         *lZBuffer =  gsColumnBltParam.mZ;
      }

      lBuffer  += gsColumnBltParam.mBufferStep;
      lZBuffer += gsColumnBltParam.mZBufferStep;
   }
}


void BltColumn()
{

   MR_UInt8*  lBuffer;
   MR_UInt16* lZBuffer;
   int        lBitmapOffset;
   int        lNbPoints;

   if( gsColumnBltParam.mYScreenStart_4096 < 0 )
   {
      lBuffer       = gsColumnBltParam.mBuffer[0]+gsColumnBltParam.mColumn;
      lZBuffer      = gsColumnBltParam.mZBuffer[0]+gsColumnBltParam.mColumn;
      lBitmapOffset = (4096-gsColumnBltParam.mYScreenStart_4096)*gsColumnBltParam.mPixelStep/4096;      
      lNbPoints     = 0;

   }
   else
   {
      lBuffer       = gsColumnBltParam.mBuffer [gsColumnBltParam.mYScreenStart_4096/4096]+gsColumnBltParam.mColumn;
      lZBuffer      = gsColumnBltParam.mZBuffer[gsColumnBltParam.mYScreenStart_4096/4096]+gsColumnBltParam.mColumn;
      lBitmapOffset = (4096-(gsColumnBltParam.mYScreenStart_4096&4095))*gsColumnBltParam.mPixelStep/4096;
      lNbPoints     = -gsColumnBltParam.mYScreenStart_4096/4096;

   }

   if( gsColumnBltParam.mYScreenEnd_4096/4096 < gsColumnBltParam.mBufferLen )
   {
      lNbPoints += gsColumnBltParam.mYScreenEnd_4096/4096;
   }
   else
   {
      lNbPoints += gsColumnBltParam.mBufferLen;
   }

   for( int lCounter = 0; lCounter < lNbPoints; lCounter++ )
   {
      if( *lZBuffer >= gsColumnBltParam.mZ )
      {
         // *lBuffer =  MR_ColorTable[ gsColumnBltParam.mLightIntensity ]
         //                         [ gsColumnBltParam.mBitmap[ (lBitmapOffset/MR_PIXEL_FRACT)&(gsColumnBltParam.mBitmapColMask) ] ];
         *lBuffer =   gsColumnBltParam.mBitmap[ (lBitmapOffset/MR_PIXEL_FRACT)&(gsColumnBltParam.mBitmapColMask) ];
         *lZBuffer =  gsColumnBltParam.mZ;
      }

      lBuffer  += gsColumnBltParam.mBufferStep;
      lZBuffer += gsColumnBltParam.mZBufferStep;

      lBitmapOffset += gsColumnBltParam.mPixelStep;
   }
}



//
// Floor and Ceiling rendering
//


void MR_3DViewPort::RenderHorizontalSurface( int pNbVertex, const MR_2DCoordinate* pVertexList, MR_Int32 pLevel, BOOL pTop, const MR_Bitmap* pBitmap )
{   

   // Algorithme
   // - Verify that we are on the visible side of the plane
   // - Rotate the coordinates of the coordinates
   // - Verify that all vertex are between mPlanDist and MaxDepth,
   //   If not, reject the surface or add (or move) some point 
   // - Compute screen coordinates of each vertex
   // - Compute the slope between each screen vertex
   // - Create two list of index to the vertex 
   //   (One for the left side of the polygon and the other for the right side)
   //
   // - Render each slice individually
   //

   ASSERT( pNbVertex <= MR_MAX_POLYGON_VERTEX );

   int             lCounter;
   MR_Int32        lLevel; 

   lLevel = pLevel - mPosition.mZ;

   if( (pTop && (lLevel >= 0 ) )||(!pTop && (lLevel <= 0) ) )
   {

      int lNb2Far   = 0;
      int lNb2Close = 0;
      int l2Close2Far[ MR_MAX_POLYGON_VERTEX ]; // 1 if 2 far, -1 if 2 close

      MR_2DCoordinate lRotated[ MR_MAX_POLYGON_VERTEX ];

      for( lCounter = 0 ; lCounter < pNbVertex; lCounter++ )
      {
         ApplyRotationMatrix( pVertexList[ lCounter ], lRotated[ lCounter ] );
       
      }


      for( lCounter = 0 ; lCounter < pNbVertex; lCounter++ )
      {
         if( lRotated[ lCounter ].mX<mPlanDist )
         {
            lNb2Close++;
            l2Close2Far[ lCounter ] = -1;
         }
         else if( lRotated[ lCounter ].mX > MR_ZBUFFER_UNIT*0xFFFE )
         {
            lNb2Far++;
            l2Close2Far[ lCounter ] = 1;
         }
         else
         {
            l2Close2Far[ lCounter ] = 0;
         }
      }

      if( (lNb2Close < pNbVertex) && (lNb2Far< pNbVertex) )
      {

         int lNbCutted = 0;

         MR_2DCoordinate lCutted[ MR_MAX_POLYGON_VERTEX+2 ];

         // Cut out of depth points
         for( lCounter = 0; lCounter< pNbVertex; lCounter++ )
         {
            if( l2Close2Far[ lCounter ]==0 )
            {
               lCutted[ lNbCutted++ ] = lRotated[ lCounter ];
            }
            else
            {
               int lPrevious = (lCounter+pNbVertex-1)%pNbVertex;
               int lNext     = (lCounter+1)%pNbVertex;

               if( l2Close2Far[ lCounter ] == -1 )
               {
                  if( l2Close2Far[ lPrevious ] != -1 )
                  {
                     lCutted[ lNbCutted ].mX = mPlanDist;
                     lCutted[ lNbCutted ].mY = lRotated[lCounter].mY + MulDiv( lRotated[lPrevious].mY-lRotated[lCounter].mY, mPlanDist-lRotated[lCounter].mX, lRotated[lPrevious].mX-lRotated[lCounter].mX );
                     lNbCutted++;
                  }

                  if( l2Close2Far[ lNext ] != -1 )
                  {
                     lCutted[ lNbCutted ].mX = mPlanDist;
                     lCutted[ lNbCutted ].mY = lRotated[lCounter].mY + MulDiv( lRotated[lNext].mY-lRotated[lCounter].mY, mPlanDist-lRotated[lCounter].mX, lRotated[lNext].mX-lRotated[lCounter].mX );
                     lNbCutted++;
                  }
               }
               else
               {
                  if( l2Close2Far[ lPrevious ]!= 1 )
                  {
                     lCutted[ lNbCutted ].mX = MR_ZBUFFER_UNIT*0xFFFE;
                     lCutted[ lNbCutted ].mY = lRotated[lCounter].mY + MulDiv( lRotated[lPrevious].mY-lRotated[lCounter].mY, MR_ZBUFFER_UNIT*0xFFFE-lRotated[lCounter].mX, lRotated[lPrevious].mX-lRotated[lCounter].mX );
                     lNbCutted++;
                  }

                  if( l2Close2Far[ lNext ] != 1 )
                  {
                     lCutted[ lNbCutted ].mX = MR_ZBUFFER_UNIT*0xFFFE;
                     lCutted[ lNbCutted ].mY = lRotated[lCounter].mY + MulDiv( lRotated[lNext].mY-lRotated[lCounter].mY, MR_ZBUFFER_UNIT*0xFFFE-lRotated[lCounter].mX, lRotated[lNext].mX-lRotated[lCounter].mX );
                     lNbCutted++;
                  }
               }              
            }
         }

         // Compute the Screen position of each coordinate
         int lNbOnRight  = 0;
         int lNbOnLeft   = 0;
         int lNbOnTop    = 0;
         int lNbOnBottom = 0;

         int lScreenX[ MR_MAX_POLYGON_VERTEX+2 ];
         int lScreenY[ MR_MAX_POLYGON_VERTEX+2 ];        

         MR_Int32 lLevel_YRes_PlanDist_PlanVW_2 = MulDiv( lLevel, mYRes_PlanDist, mPlanVW*2 );

         for( lCounter = 0; lCounter < lNbCutted; lCounter++ )
         {
            lScreenX[ lCounter ] =  MulDiv(-lCutted[ lCounter ].mY, mXRes_PlanDist, lCutted[ lCounter ].mX*mPlanHW*2 )+mXRes/2;
            lScreenY[ lCounter ] = -lLevel_YRes_PlanDist_PlanVW_2/lCutted[ lCounter ].mX + mYRes/2+mScroll;

            if( lScreenX[ lCounter ]<=0 )
            {
               lNbOnLeft++;
            }
            else if( lScreenX[ lCounter ] >= mXRes )
            {
               lNbOnRight++;
            }

            if( lScreenY[ lCounter ]<=0 )
            {
               lNbOnTop++;
            }
            else if( lScreenY[ lCounter ] >= mYRes )
            {
               lNbOnBottom++;
            }
         }

         // DEBUG, draw surface contour
         /*
         for( lCounter = 0; lCounter<lNbCutted; lCounter++ )
         {
            int lNext = (lCounter+1)%lNbCutted;
            DrawLine( lScreenX[ lCounter ], lScreenY[ lCounter ], lScreenX[ lNext ], lScreenY[ lNext ], 7 );
         }
         return;
         */



         if(   (lNbOnRight != lNbCutted)&&(lNbOnLeft != lNbCutted)
             &&(lNbOnTop != lNbCutted)&&(lNbOnBottom != lNbCutted) )
         {

            // Find the extremities
        
            int lHiLeft  = 0;
            int lHiRight = 0;
            int lLoLeft  = 0;
            int lLoRight = 0;

            for( lCounter = 1; lCounter<lNbCutted; lCounter++ )
            {
               if( lScreenY[ lCounter ] <= lScreenY[ lHiLeft ] )
               {
                  if( lScreenY[ lCounter ] < lScreenY[ lHiLeft ] )
                  {
                     lHiLeft  = lCounter;
                     lHiRight = lCounter;
                  }
                  else
                  {
                     if( !pTop )
                     {
                        if( lHiRight == lCounter-1 )
                        {
                           lHiRight = lCounter;
                        }
                        else
                        {
                           lHiLeft = lCounter;
                        }
                     }
                     else
                     {
                        if( lHiLeft == lCounter-1 )
                        {
                           lHiLeft = lCounter;
                        }
                        else
                        {
                           lHiRight = lCounter;
                        }
                     }
                  }
               }

               if( lScreenY[ lCounter ] >= lScreenY[ lLoLeft ] )
               {
                  if( lScreenY[ lCounter ] > lScreenY[ lLoLeft ] )
                  {
                     lLoLeft  = lCounter;
                     lLoRight = lCounter;
                  }
                  else
                  {
                     if( !pTop )
                     {
                        if( lLoLeft == lCounter-1 )
                        {
                           lLoLeft = lCounter;
                        }
                        else
                        {
                           lLoRight = lCounter;
                        }
                     }
                     else
                     {
                        if( lLoRight == lCounter-1 )
                        {
                           lLoRight = lCounter;
                        }
                        else
                        {
                           lLoLeft = lCounter;
                        }
                     }
                  }
               }
            }   


            // Create the Right and left blitting stripes
            int lLeftStripeLen  = 1;
            int lRightStripeLen = 1;
            int lRightStripe[ MR_MAX_POLYGON_VERTEX+2 ];
            int lLeftStripe[ MR_MAX_POLYGON_VERTEX+2 ];

            lLeftStripe[0]  = lHiLeft;
            lRightStripe[0] = lHiRight;

            if( !pTop )
            {
               // First find the first non-visible index of each column

               if( lScreenY[ lLeftStripe[0] ]<0 )
               {
                  while(1)
                  {
                     int lIndex = lLeftStripe[0]-1;

                     if( lIndex == -1 )
                     {
                        lIndex = lNbCutted-1;
                     }

                     if( lScreenY[ lIndex ]<=0 )
                     {
                        lLeftStripe[0] = lIndex;
                     }
                     else
                     {
                        break;
                     }
                  }
               }

               if( lScreenY[ lRightStripe[0] ]<0 )
               {
                  while(1)
                  {
                     int lIndex = lRightStripe[0]+1;

                     if( lIndex == lNbCutted )
                     {
                        lIndex = 0;
                     }

                     if( lScreenY[ lIndex ]<=0 )
                     {
                        lRightStripe[0] = lIndex;
                     }
                     else
                     {
                        break;
                     }
                  }
               }


               // Now create each stripe
               while( (lLeftStripe[ lLeftStripeLen-1] != lLoLeft) && (lScreenY[ lLeftStripe[ lLeftStripeLen-1] ]<mYRes) )
               {
                  int lIndex = lLeftStripe[ lLeftStripeLen-1]-1;

                  if( lIndex == -1 )
                  {
                     lIndex = lNbCutted-1;
                  }

                  lLeftStripe[ lLeftStripeLen++ ] = lIndex;
               }

               while( (lRightStripe[ lRightStripeLen-1] != lLoRight) && (lScreenY[ lRightStripe[ lRightStripeLen-1] ]<mYRes) )
               {
                  int lIndex = lRightStripe[ lRightStripeLen-1]+1;

                  if( lIndex == lNbCutted )
                  {
                     lIndex = 0;
                  }

                  lRightStripe[ lRightStripeLen++ ] = lIndex;
               }            
            }
            else
            {
               if( lScreenY[ lLeftStripe[0] ]<0 )
               {
                  while(1)
                  {
                     int lIndex = lLeftStripe[0]+1;

                     if( lIndex == lNbCutted )
                     {
                        lIndex = 0;
                     }

                     if( lScreenY[ lIndex ]<=0 )
                     {
                        lLeftStripe[0] = lIndex;
                     }
                     else
                     {
                        break;
                     }
                  }
               }

               if( lScreenY[ lRightStripe[0] ]<0 )
               {
                  while(1)
                  {
                     int lIndex = lRightStripe[0]-1;

                     if( lIndex == -1 )
                     {
                        lIndex = lNbCutted-1;
                     }

                     if( lScreenY[ lIndex ]<=0 )
                     {
                        lRightStripe[0] = lIndex;
                     }
                     else
                     {
                        break;
                     }
                  }
               }


               // Now create each stripe
               while( (lLeftStripe[ lLeftStripeLen-1] != lLoLeft) && (lScreenY[ lLeftStripe[ lLeftStripeLen-1] ]<mYRes) )
               {
                  int lIndex = lLeftStripe[ lLeftStripeLen-1]+1;

                  if( lIndex == lNbCutted )
                  {
                     lIndex = 0;
                  }

                  lLeftStripe[ lLeftStripeLen++ ] = lIndex;
               }

               while( (lRightStripe[ lRightStripeLen-1] != lLoRight) && (lScreenY[ lRightStripe[ lRightStripeLen-1] ]<mYRes) )
               {
                  int lIndex = lRightStripe[ lRightStripeLen-1]-1;

                  if( lIndex == -1 )
                  {
                     lIndex = lNbCutted-1;
                  }

                  lRightStripe[ lRightStripeLen++ ] = lIndex;
               }            
            }

            // Compute the slopes between each line segment
            MR_Int32 lDXLeft_4096[  MR_MAX_POLYGON_VERTEX+1 ];
            MR_Int32 lDXRight_4096[ MR_MAX_POLYGON_VERTEX+1 ];

            for( lCounter = 0; lCounter<lLeftStripeLen-1; lCounter++ )
            {
               MR_Int32 lLineGap = lScreenY[ lLeftStripe[lCounter+1] ]-lScreenY[ lLeftStripe[lCounter] ];

               if( lLineGap != 0 )
               {
                  lDXLeft_4096[ lCounter ] = 4096*( lScreenX[ lLeftStripe[lCounter+1] ]-lScreenX[ lLeftStripe[lCounter] ] )/lLineGap;
               }
               else
               {
                  lDXLeft_4096[ lCounter ] = 0;
               }

            }

            for( lCounter = 0; lCounter<lRightStripeLen-1; lCounter++ )
            {
               MR_Int32 lLineGap = lScreenY[ lRightStripe[lCounter+1] ]-lScreenY[ lRightStripe[lCounter] ];

               if( lLineGap != 0 )
               {
                  lDXRight_4096[ lCounter ] = 4096*( lScreenX[ lRightStripe[lCounter+1] ]-lScreenX[ lRightStripe[lCounter] ] )/lLineGap;
               }
               else
               {
                  lDXRight_4096[ lCounter ] = 0;
               }

            }
            


            // Draw the surface
            int lLeftIndex  = 0;
            int lRightIndex = 0;

            int lCurrentLine;
            int lLeftX_4096;
            int lRightX_4096;

            int lNextLineStop;
            int lNextStopSide; // 1 LeftSide, 2RightSide 3 Both 0 endofpolygon
            int lLeftStop;
            int lRightStop;


            // first sections starting initialisation
            lCurrentLine = lScreenY[ lLeftStripe[0] ];

            if( lCurrentLine >= 0 )
            {
               lLeftX_4096  = lScreenX[ lLeftStripe[0]  ]*4096;
               lRightX_4096 = lScreenX[ lRightStripe[0] ]*4096;

            }
            else
            {
               lLeftX_4096  = lScreenX[ lLeftStripe[0]  ]*4096 -lCurrentLine*lDXLeft_4096[0];
               lRightX_4096 = lScreenX[ lRightStripe[0] ]*4096 -lScreenY[ lRightStripe[0] ]*lDXRight_4096[0];

               lCurrentLine = 0;
            }

            lLeftStop  = lScreenY[ lLeftStripe[1] ];
            lRightStop = lScreenY[ lRightStripe[1] ];


            // Compute the rendering constants
            MR_Int32 lCurrentLine_VVarPerDInc_16384 = (lCurrentLine-mYRes/2-mScroll) * mVVarPerDInc_16384;            

            // MR_Int32 lBitmapHColVariation_16384 = ( mPlanHW_PlanDist_2_XRes_16384*pBitmap->GetMaxXRes()/pBitmap->GetWidth()  ) * MR_Sin[ mOrientation ]/ MR_TRIGO_FRACT;
            // MR_Int32 lBitmapHRowVariation_16384 = ( mPlanHW_PlanDist_2_XRes_16384*pBitmap->GetMaxYRes()/pBitmap->GetHeight() ) * MR_Cos[ mOrientation ]/ MR_TRIGO_FRACT;
            MR_Int32 lBitmapHColVariation_16384_64 = MulDiv( mPlanHW_PlanDist_2_XRes_16384,64*MR_Sin[ mOrientation ]*pBitmap->GetMaxXRes(),pBitmap->GetWidth()*MR_TRIGO_FRACT  );
            MR_Int32 lBitmapHRowVariation_16384_64 = MulDiv( mPlanHW_PlanDist_2_XRes_16384,64*MR_Cos[ mOrientation ]*pBitmap->GetMaxYRes(),pBitmap->GetHeight()*MR_TRIGO_FRACT );

            MR_Int32 lBitmapVColVariation_16384 = MulDiv( 16384*pBitmap->GetMaxXRes(),  MR_Cos[ mOrientation ], pBitmap->GetWidth()*MR_TRIGO_FRACT );
            MR_Int32 lBitmapVRowVariation_16384 = -MulDiv( 16384*pBitmap->GetMaxYRes(), MR_Sin[ mOrientation ], pBitmap->GetHeight()*MR_TRIGO_FRACT );
            MR_Int32 lBitmapVVariation_16384    = 16384*pBitmap->GetMaxXRes()/pBitmap->GetWidth();

            MR_Int32 lBitmapCol0_4096 = MulDiv( mPosition.mX, 4096*pBitmap->GetMaxXRes(), pBitmap->GetWidth() );
            MR_Int32 lBitmapRow0_4096 = -MulDiv( mPosition.mY, 4096*pBitmap->GetMaxYRes(), pBitmap->GetHeight() );

            MR_Int32   lBitmapW_XRes_PlanDist_2PlanHW_1024 = pBitmap->GetWidth()*(mXRes_PlanDist_2PlanHW_4096/4);

            MR_UInt8*  lLineBuffer  = mBufferLine[lCurrentLine];
            MR_UInt16* lZLineBuffer = mZBufferLine[lCurrentLine];


            MR_Int32   lPreviousDepth_8 = -1;

            while( 1 )
            {
               if( lLeftStop <= lRightStop )
               {
                  if( lLeftStop >= mYRes )
                  {
                     lNextStopSide = 0;
                     lNextLineStop = mYRes;
                  }
                  else if( lLeftIndex >= lLeftStripeLen-2 )
                  {
                     lNextStopSide = 0;
                     lNextLineStop = lLeftStop;
                  }
                  else
                  {
                     if( lLeftStop == lRightStop )
                     {
                        if( lRightIndex >= lRightStripeLen-2 )
                        {
                           lNextStopSide = 0;
                        }
                        else
                        {
                           lNextStopSide = 3;
                        }
                     }
                     else
                     {
                        lNextStopSide = 1;
                     }
                     lNextLineStop = lLeftStop;
                  }
               }
               else
               {
                  if( lRightStop >= mYRes )
                  {
                     lNextStopSide = 0;
                     lNextLineStop = mYRes;
                  }
                  else if( lRightIndex >= lRightStripeLen-2 )
                  {
                     lNextStopSide = 0;
                     lNextLineStop = lRightStop;
                  }
                  else
                  {
                     lNextStopSide = 2;
                     lNextLineStop = lRightStop;
                  }
               }

               /*
               if( lDXLeft_4096[  lLeftIndex  ] < 0 )
               {
                  lLeftX_4096  += lDXLeft_4096[  lLeftIndex  ];
               }

               if( lDXRight_4096[ lRightIndex ] > 0 )
               {
                  lRightX_4096 += lDXRight_4096[ lRightIndex ];
               }
               */


               while( lCurrentLine < lNextLineStop )
               {

                  int lLeft  = lLeftX_4096/4096;
                  int lRight = lRightX_4096/4096;

                  if( lLeft < 0 )
                  {
                     lLeft = 0;
                  }

                  if( lRight > mXRes )
                  {
                     lRight = mXRes;
                  }

                  if( lRight > lLeft )
                  {

                     int lDepth_8;
                     int lSelectedBitmap;

                     if( lCurrentLine_VVarPerDInc_16384/8 != 0 )
                     {
                        lDepth_8 = lLevel*16384/(lCurrentLine_VVarPerDInc_16384/8);

                        if( lPreviousDepth_8 == -1 )
                        {
                           if( ((lCurrentLine_VVarPerDInc_16384-mVVarPerDInc_16384)/8) == 0 )
                           {
                              lPreviousDepth_8 = lLevel*16384/((lCurrentLine_VVarPerDInc_16384+mVVarPerDInc_16384)/8);
                           }
                           else
                           {
                              lPreviousDepth_8 = lLevel*16384/((lCurrentLine_VVarPerDInc_16384-mVVarPerDInc_16384)/8);
                           }
                        }

                        int lDiff_8 = lPreviousDepth_8- lDepth_8;

                        if( lDiff_8 < 0 )
                        {
                           lDiff_8 = -lDiff_8;
                        }
                                              
                        lSelectedBitmap = pBitmap->GetBestBitmapForPitch_4096( lBitmapVVariation_16384*((lDiff_8)/(4*8)) );

                        lPreviousDepth_8 = lDepth_8;
                     }
                     else
                     {
                        lDepth_8 = 8*MR_ZBUFFER_LIMIT*MR_ZBUFFER_UNIT;

                        lSelectedBitmap = -1;
                     }
                     
                     gsLineBltParam.mBuffer  = lLineBuffer+lLeft;
                     gsLineBltParam.mBltLen  = lRight-lLeft;
                     gsLineBltParam.mZBuffer = lZLineBuffer+lLeft;
                     gsLineBltParam.mZ       = lDepth_8/(8*MR_ZBUFFER_UNIT);

                     gsLineBltParam.mLightIntensity = MR_NORMAL_INTENSITY;

           
                     // int lSelectedBitmap = pBitmap->GetBestBitmapForXRes( lBitmapW_XRes_PlanDist_2PlanHW_1024/(lDepth_8*(1024/8)) );

                     if( lSelectedBitmap == -1 )
                     {
                        gsLineBltParam.mColor = pBitmap->GetPlainColor();

                        BltPlainLineNoZCheck();

                     }
                     else
                     {

                        int lColShift = pBitmap->GetXResShiftFactor( lSelectedBitmap );
                        int lRowShift = pBitmap->GetYResShiftFactor( lSelectedBitmap );

                        gsLineBltParam.mBitmap          = pBitmap->GetColumnBufferTable( lSelectedBitmap );
                        gsLineBltParam.mBitmapColMask   = pBitmap->GetXRes( lSelectedBitmap )-1;
                        gsLineBltParam.mBitmapRowMask   = pBitmap->GetYRes( lSelectedBitmap )-1;

                        gsLineBltParam.mBitmapColInc_4096 = (( lBitmapHColVariation_16384_64 * lDepth_8 )>>lColShift)/(8*4*64);
                        gsLineBltParam.mBitmapRowInc_4096 = (( lBitmapHRowVariation_16384_64 * lDepth_8 )>>lRowShift)/(8*4*64);

                        gsLineBltParam.mBitmapCol_4096 = (lLeft-mXRes/2)*gsLineBltParam.mBitmapColInc_4096 + (((lBitmapVColVariation_16384*lDepth_8/(4*8)) + lBitmapCol0_4096)>>lColShift);
                        gsLineBltParam.mBitmapRow_4096 = (lLeft-mXRes/2)*gsLineBltParam.mBitmapRowInc_4096 + (((lBitmapVRowVariation_16384*lDepth_8/(4*8)) + lBitmapRow0_4096)>>lRowShift);

                        BltLineNoZCheck();
                     }


                      
                     // mPlanDist_PlanHW_PlanDist_2_XRes_16384;
                     // memset( lBuffer+lLeft, 255 - lDepth_1024/(1024*1024), lRight-lLeft );
                  }

                  lCurrentLine++;
                  lLineBuffer  += mLineLen;
                  lZLineBuffer += mZLineLen;

                  lLeftX_4096  += lDXLeft_4096[  lLeftIndex  ];
                  lRightX_4096 += lDXRight_4096[ lRightIndex ];

                  lCurrentLine_VVarPerDInc_16384 += mVVarPerDInc_16384;
               }

               // End of section adjustment
               if( lNextStopSide ==  0 )
               {
                  break;
               }
               else
               {
                  if( lNextStopSide & 1 )
                  {
                     lLeftIndex++;
                     lLeftX_4096 = lScreenX[ lLeftStripe[ lLeftIndex ] ]*4096;
                     lLeftStop   = lScreenY[ lLeftStripe[ lLeftIndex+1 ] ];
                  }

                  if( lNextStopSide & 2 )
                  {
                     lRightIndex++;
                     lRightX_4096 = lScreenX[ lRightStripe[ lRightIndex ] ]*4096;
                     lRightStop   = lScreenY[ lRightStripe[ lRightIndex+1 ] ];
                  }
               }

               
            } // End of sections loop

            

            


         }     
      }
   }
}




void BltPlainLineNoZCheck()
{
   if( gsLineBltParam.mBltLen > 0 )
   {
      // memset( gsLineBltParam.mBuffer, MR_ColorTable[ gsLineBltParam.mLightIntensity ][ gsLineBltParam.mColor ], gsLineBltParam.mBltLen );
      
      memset( gsLineBltParam.mBuffer, gsLineBltParam.mColor, gsLineBltParam.mBltLen );

      for( int lCounter = 0; lCounter<gsLineBltParam.mBltLen; lCounter++ )
      {
         gsLineBltParam.mZBuffer[ lCounter ] = gsLineBltParam.mZ;
      }
   }
}


void BltLineNoZCheck()
{
   
   MR_UInt8*  lBuffer  = gsLineBltParam.mBuffer;
   MR_UInt16* lZBuffer = gsLineBltParam.mZBuffer;

   MR_UInt32 lColumn_4096 = gsLineBltParam.mBitmapCol_4096;
   MR_UInt32 lRow_4096    = gsLineBltParam.mBitmapRow_4096;

   for( int lCounter = 0; lCounter< gsLineBltParam.mBltLen; lCounter++ )
   {
            

      // *(lBuffer++) =  MR_ColorTable[ gsLineBltParam.mLightIntensity ]
      //                              [ gsLineBltParam.mBitmap
      //                                 [ (lColumn_4096/4096)&gsLineBltParam.mBitmapColMask ]
      //                                 [ (lRow_4096/4096)&gsLineBltParam.mBitmapRowMask ]      ];

      *(lBuffer++) = gsLineBltParam.mBitmap[ (lColumn_4096/4096)&gsLineBltParam.mBitmapColMask ]
                                           [ (lRow_4096/4096)&gsLineBltParam.mBitmapRowMask ] ;
      *(lZBuffer++) = gsLineBltParam.mZ;

      lColumn_4096 += gsLineBltParam.mBitmapColInc_4096;
      lRow_4096    += gsLineBltParam.mBitmapRowInc_4096;
   }
}


   
/*
__declspec( naked ) void BltLineNoZCheck()
{

   static MR_UInt32 lOldSP;

   __asm
   {

      push eax
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push ebp
      
      mov lOldSP, esp


      mov ecx, gsLineBltParam.mBltLen
      cmp ecx,0
      je end

      mov edi, gsLineBltParam.mBuffer
      mov esi, gsLineBltParam.mColorTable
      xor eax, eax
      mov ah,  gsLineBltParam.mLightIntensity
      add esi, eax

      mov ebp, gsLineBltParam.mBitmap

      mov ebx, gsLineBltParam.mBitmapCol_4096
      mov edx, gsLineBltParam.mBitmapRow_4096

      loop_start:

      mov eax, ebx
      shr eax, 12
      and eax, gsLineBltParam.mBitmapColMask
      mov esp, [ebp][ eax*4 ]
      mov eax, edx
      shr eax, 12
      and eax, gsLineBltParam.mBitmapRowMask
      add esp, eax
      xor eax, eax
      mov al, [esp]
      mov al, [esi][eax]

      stosb
      
      add ebx, gsLineBltParam.mBitmapColInc_4096
      add edx, gsLineBltParam.mBitmapRowInc_4096

      loop loop_start

      mov ecx, gsLineBltParam.mBltLen
      mov edi, gsLineBltParam.mZBuffer
      mov ax,  gsLineBltParam.mZ

      rep stosw
     
      end:
      
      mov  esp, lOldSP

      pop  ebp
      pop  edi
      pop  esi
      pop  edx
      pop  ecx
      pop  ebx
      pop  eax

      ret

   };
   
}

*/


//
// Patch section
//

#define MAX_PATCH_RES 16

#define ON_SCREEN   0
#define ON_RIGHT    1
#define ON_LEFT     2
#define ON_TOP      4
#define ON_BOTTOM   8
#define ON_FRONT   16
#define ON_BACK    32


static MR_3DCoordinate gsRotatedPatch    [ MAX_PATCH_RES*MAX_PATCH_RES ];
static int             gsScreenXPatch    [ MAX_PATCH_RES*MAX_PATCH_RES ];
static int             gsScreenYPatch    [ MAX_PATCH_RES*MAX_PATCH_RES ];
static int             gsScreenVisibility[ MAX_PATCH_RES*MAX_PATCH_RES ];


void MR_3DViewPort::RenderPatch( const MR_Patch& pPatch, const MR_PositionMatrix& pMatrix , const MR_Bitmap* pBitmap )
{

   int lCounter;
   int lURes = pPatch.GetURes();
   int lVRes = pPatch.GetVRes();

   int lNbNodes = lURes*lVRes;

   
   const MR_3DCoordinate* lNodeList = pPatch.GetNodeList();

   for( lCounter = 0; lCounter< lNbNodes; lCounter++ )
   {
      // Rotate each vertex of the patch

      ApplyPositionMatrix( pMatrix, lNodeList[ lCounter ], gsRotatedPatch[ lCounter ] );


      // Compute the screen coordinate of the vertex
      gsScreenVisibility[ lCounter ] = ON_SCREEN;

      if( gsRotatedPatch[ lCounter ].mX < mPlanDist/2 )
      {
         gsScreenVisibility[ lCounter ] = ON_FRONT;
      }
      else if( gsRotatedPatch[ lCounter ].mX/MR_ZBUFFER_UNIT > MR_ZBUFFER_LIMIT )
      {
         gsScreenVisibility[ lCounter ] = ON_BACK;
      }
      else
      {
         gsScreenXPatch[ lCounter ] =  MulDiv( -gsRotatedPatch[ lCounter ].mY, mXRes_PlanDist, gsRotatedPatch[lCounter].mX*mPlanHW*2 )+mXRes/2;
         gsScreenYPatch[ lCounter ] = -MulDiv(  gsRotatedPatch[ lCounter ].mZ, mYRes_PlanDist, gsRotatedPatch[lCounter].mX*mPlanVW*2 )+mYRes/2+mScroll;

         // Debug
         // Display vertex
         /*
         if(    gsScreenXPatch[ lCounter ] >= 0 && gsScreenXPatch[ lCounter ]<mXRes
             && gsScreenYPatch[ lCounter ] >= 0 && gsScreenYPatch[ lCounter ]<mYRes )
         {
            mBuffer[ gsScreenXPatch[ lCounter ] + mLineLen*gsScreenYPatch[ lCounter ] ] = 0;
         }
         */

      }
   }

   // render each triangle of the patch
   int lBitmapXRes = pBitmap->GetMaxXRes();
   int lBitmapYRes = pBitmap->GetMaxYRes();
   
   int lSelectedBitmap = 0;

   lBitmapXRes = pBitmap->GetXRes( lSelectedBitmap );
   lBitmapYRes = pBitmap->GetYRes( lSelectedBitmap );

   gsTriangleBltParam.mBitmap         = pBitmap->GetColumnBufferTable( lSelectedBitmap );
   gsTriangleBltParam.mBitmapColMask  = lBitmapXRes-1;
   gsTriangleBltParam.mBitmapRowMask  = lBitmapYRes-1;

   gsTriangleBltParam.mLightIntensity = MR_NORMAL_INTENSITY;
   gsTriangleBltParam.mColor          = pBitmap->GetPlainColor();

   gsTriangleBltParam.mBuffer  = mBufferLine;
   gsTriangleBltParam.mLineLen = mLineLen;
   gsTriangleBltParam.mXRes    = mXRes;
   gsTriangleBltParam.mYRes    = mYRes;

   gsTriangleBltParam.mZBuffer  = mZBufferLine;
   gsTriangleBltParam.mZLineLen = mZLineLen;


   MR_Int32 lBitmapRowInc_4096 = lBitmapXRes*4096/(lVRes-1);
   MR_Int32 lBitmapColInc_4096 = lBitmapYRes*4096/(lURes-1);

   MR_Int32 lBitmapRow_4096_0 = 0;
   MR_Int32 lBitmapRow_4096_1 = lBitmapRowInc_4096;

   lCounter = 0;

   for( int lV = 0; lV<(lVRes-1); lV++ )
   {

      MR_Int32 lBitmapCol_4096_0 = 0;
      MR_Int32 lBitmapCol_4096_1 = lBitmapColInc_4096;

      for( int lU = 0; lU<( lURes-1); lU++ )
      {
         if( ( gsScreenVisibility[ lCounter+1 ] == ON_SCREEN ) &&
             ( gsScreenVisibility[ lCounter+lURes ] == ON_SCREEN ) )
         {
            if( gsScreenVisibility[ lCounter ] == ON_SCREEN )
            {
               gsTriangleBltParam.mVertexList[0] = lCounter;
               gsTriangleBltParam.mVertexList[1] = lCounter+1;
               gsTriangleBltParam.mVertexList[2] = lCounter+lURes;

               gsTriangleBltParam.mBitmapCol_4096[0] = lBitmapCol_4096_0;
               gsTriangleBltParam.mBitmapCol_4096[1] = lBitmapCol_4096_1;
               gsTriangleBltParam.mBitmapCol_4096[2] = lBitmapCol_4096_0;

               gsTriangleBltParam.mBitmapRow_4096[0] = lBitmapRow_4096_0;
               gsTriangleBltParam.mBitmapRow_4096[1] = lBitmapRow_4096_0;
               gsTriangleBltParam.mBitmapRow_4096[2] = lBitmapRow_4096_1;

               BltTriangle();

            }

            if( gsScreenVisibility[ lCounter+lURes+1 ] == ON_SCREEN )
            {
               gsTriangleBltParam.mVertexList[0] = lCounter+1;
               gsTriangleBltParam.mVertexList[1] = lCounter+lURes+1;
               gsTriangleBltParam.mVertexList[2] = lCounter+lURes;
              
               gsTriangleBltParam.mBitmapCol_4096[0] = lBitmapCol_4096_1;
               gsTriangleBltParam.mBitmapCol_4096[1] = lBitmapCol_4096_1;
               gsTriangleBltParam.mBitmapCol_4096[2] = lBitmapCol_4096_0;

               gsTriangleBltParam.mBitmapRow_4096[0] = lBitmapRow_4096_0;
               gsTriangleBltParam.mBitmapRow_4096[1] = lBitmapRow_4096_1;
               gsTriangleBltParam.mBitmapRow_4096[2] = lBitmapRow_4096_1;

               BltTriangle();

            }
         }
         lBitmapCol_4096_0 =  lBitmapCol_4096_1;
         lBitmapCol_4096_1 += lBitmapColInc_4096;

         lCounter++;
      }

      lBitmapRow_4096_0 =  lBitmapRow_4096_1;
      lBitmapRow_4096_1 += lBitmapRowInc_4096;

      lCounter++;
   }

}


void BltTriangle()
{
   int lCounter;


   // First sort the vertex according to their ScreenCoordinate
   // Verify that we are on the good side of the triangle

   int  lTop;
   int  lMiddle;
   int  lBottom;


   MR_Int32 lDiffY[3];
   lDiffY[0] = gsScreenYPatch[ gsTriangleBltParam.mVertexList[1] ] - gsScreenYPatch[ gsTriangleBltParam.mVertexList[0] ];
   lDiffY[1] = gsScreenYPatch[ gsTriangleBltParam.mVertexList[2] ] - gsScreenYPatch[ gsTriangleBltParam.mVertexList[1] ];
   lDiffY[2] = gsScreenYPatch[ gsTriangleBltParam.mVertexList[0] ] - gsScreenYPatch[ gsTriangleBltParam.mVertexList[2] ];

   if( (lDiffY[0] == 0)&&(lDiffY[1] == 0) )
   {
      return;
   }
 
   MR_Int32 lDiffX[3];
   lDiffX[0] = gsScreenXPatch[ gsTriangleBltParam.mVertexList[1] ] - gsScreenXPatch[ gsTriangleBltParam.mVertexList[0] ];
   lDiffX[1] = gsScreenXPatch[ gsTriangleBltParam.mVertexList[2] ] - gsScreenXPatch[ gsTriangleBltParam.mVertexList[1] ];
   lDiffX[2] = gsScreenXPatch[ gsTriangleBltParam.mVertexList[0] ] - gsScreenXPatch[ gsTriangleBltParam.mVertexList[2] ];

   if( (lDiffX[0] == 0)&&(lDiffX[1] == 0) )
   {
      return;
   }


   MR_Int32 lDXDY_4096[3];

   for( lCounter = 0; lCounter<3; lCounter++ )
   {
      if( lDiffY[lCounter] != 0 )
      {
         lDXDY_4096[ lCounter ] = lDiffX[ lCounter ]*4096/lDiffY[ lCounter ];
      }
      else
      {
         if( lDiffX[ lCounter ] == 0 )
         {
            return; // two points are the same
         }
         else if( lDiffX[ lCounter ] > 0 )
         {
            lDXDY_4096[ lCounter ] =  1000000;
         }
         else
         {
            lDXDY_4096[ lCounter ] = -1000000;
         }
      }
   }


   MR_Int32 lLeftSlope;
   MR_Int32 lRightSlope;
   MR_Int32 lBottomSlope;

   BOOL     lMiddleShouldBeOnRight;


   if( lDiffY[0] > 0 )        // 0 higher than 1
   {      
      if( lDiffY[1] > 0 )     // 1 higher than 2
      {
         lTop = 0;
         lMiddle = 1;
         lBottom = 2;

         lLeftSlope   = lDXDY_4096[2];
         lRightSlope  = lDXDY_4096[0];
         lBottomSlope = lDXDY_4096[1];
         lMiddleShouldBeOnRight = TRUE;

      }
      else                   // 2 is higher than 1
      {
         if( lDiffY[2] > 0 )  // 2 is higher than 0
         {
            lTop    = 2;
            lMiddle = 0;
            lBottom = 1;

            lLeftSlope   = lDXDY_4096[1];
            lRightSlope  = lDXDY_4096[2];
            lBottomSlope = lDXDY_4096[0];
            lMiddleShouldBeOnRight = TRUE;

         }
         else                // 0 is higher than 2
         {
            lTop    = 0;
            lMiddle = 2;
            lBottom = 1;

            lLeftSlope   = lDXDY_4096[2];
            lRightSlope  = lDXDY_4096[0];
            lBottomSlope = lDXDY_4096[1];
            lMiddleShouldBeOnRight = FALSE;

         }
      }
   }
   else                      // 1 higher than 0
   {
      if( lDiffY[1] > 0 )     // 1 higher than 2
      {
         if( lDiffY[2] > 0 )  // 2 is higher than 0
         {
            lTop    = 1;
            lMiddle = 2;
            lBottom = 0;

            lLeftSlope   = lDXDY_4096[0];
            lRightSlope  = lDXDY_4096[1];
            lBottomSlope = lDXDY_4096[2];
            lMiddleShouldBeOnRight = TRUE;

         }
         else                // 0 is higher than 2
         {
            lTop    = 1;
            lMiddle = 0;
            lBottom = 2;

            lLeftSlope   = lDXDY_4096[0];
            lRightSlope  = lDXDY_4096[1];
            lBottomSlope = lDXDY_4096[2];
            lMiddleShouldBeOnRight = FALSE;

         }
      }
      else                   // 2 is higher than 1
      {
         lTop    = 2;
         lMiddle = 1;
         lBottom = 0;

         lLeftSlope   = lDXDY_4096[1];
         lRightSlope  = lDXDY_4096[2];
         lBottomSlope = lDXDY_4096[0];
         lMiddleShouldBeOnRight = FALSE;

      }
   }

   int lTopLine    = gsScreenYPatch[ gsTriangleBltParam.mVertexList[lTop]];
   int lMiddleLine = gsScreenYPatch[ gsTriangleBltParam.mVertexList[lMiddle]];
   int lBottomLine = gsScreenYPatch[ gsTriangleBltParam.mVertexList[lBottom]];


   // Verify that we are on screen

   if( (lBottomLine <= 0) || lTopLine >=gsTriangleBltParam.mYRes )
   {
      return;
   }



   // Verify that we are on the good side of the triangle
   if( lMiddleLine != lTopLine )
   {
      if( lLeftSlope >= lRightSlope )
      {
         return;
      }
   }
   else
   {
	  if( lMiddleShouldBeOnRight )
	  {
	     if( lLeftSlope <= lBottomSlope )
		 {
		    return;
		 }
	  }
	  else
	  {
	     if( lBottomSlope <= lRightSlope )
	     {
	        return;
		 }
	  }	   
   }

   

   // Bitmapping and Z variables
   int lU_4096;
   int lV_4096;
   int lZ_4096;

   int lDU_PerLine_4096;
   int lDV_PerLine_4096;
   int lDZ_PerLine_4096;
   int lDU_PerPixel_4096;
   int lDV_PerPixel_4096;
   int lDZ_PerPixel_4096;


   int lUOnMiddle = gsTriangleBltParam.mBitmapCol_4096[ lTop ] 
                   + (gsTriangleBltParam.mBitmapCol_4096[ lBottom ]-gsTriangleBltParam.mBitmapCol_4096[ lTop ])
                    *(lMiddleLine-lTopLine)/(lBottomLine-lTopLine);

   int lVOnMiddle = gsTriangleBltParam.mBitmapRow_4096[ lTop ] 
                   + (gsTriangleBltParam.mBitmapRow_4096[ lBottom ]-gsTriangleBltParam.mBitmapRow_4096[ lTop ])
                    *(lMiddleLine-lTopLine)/(lBottomLine-lTopLine);

   int lZOnMiddle = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX*4096
                   + (gsRotatedPatch[ gsTriangleBltParam.mVertexList[lBottom] ].mX - gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX )*4096
                    *(lMiddleLine-lTopLine)/(lBottomLine-lTopLine);

   int lXOnMiddle = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]]*4096
                   + (gsScreenXPatch[ gsTriangleBltParam.mVertexList[lBottom]]-gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]])*4096
                    *(lMiddleLine-lTopLine)/(lBottomLine-lTopLine);
 
   int lOnMiddleLen = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096 - lXOnMiddle;


   if( lOnMiddleLen/1024 == 0 )
   {
      // ASSERT( FALSE ); // I was thinking that the case was already trap
      return; // all points are on a single line
   }

   
   lDU_PerPixel_4096 = (gsTriangleBltParam.mBitmapCol_4096[ lMiddle ] - lUOnMiddle )*4/(lOnMiddleLen/1024);
   lDV_PerPixel_4096 = (gsTriangleBltParam.mBitmapRow_4096[ lMiddle ] - lVOnMiddle )*4/(lOnMiddleLen/1024);
   lDZ_PerPixel_4096 = (gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX*4096 - lZOnMiddle )*64/(lOnMiddleLen/64);


   if( lMiddleShouldBeOnRight )
   {
      lDU_PerLine_4096 = ( gsTriangleBltParam.mBitmapCol_4096[ lBottom ] - gsTriangleBltParam.mBitmapCol_4096[ lTop ] )/(lBottomLine-lTopLine);
      lDV_PerLine_4096 = ( gsTriangleBltParam.mBitmapRow_4096[ lBottom ] - gsTriangleBltParam.mBitmapRow_4096[ lTop ] )/(lBottomLine-lTopLine);
      lDZ_PerLine_4096 = ( gsRotatedPatch[ gsTriangleBltParam.mVertexList[lBottom] ].mX - gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX )*4096/(lBottomLine-lTopLine);
   }
   else
   {
      // Can not be calculated now
   }
   
   
   // Draw each line of the model
   int        lCurrentLine;
   MR_UInt8*  lLineBuffer;
   MR_UInt16* lLineZBuffer;


   int lXLeft_4096;
   int lXRight_4096;

   // Verify that the buttom of the triangle is below the top of the screen
   int lFirstStop  = lMiddleLine;
   int lSecondStop = lBottomLine;

   if( lSecondStop > 0 )
   {
      // Cut what is below the screen bottom
      if( lSecondStop > gsTriangleBltParam.mYRes )
      {
         lSecondStop = gsTriangleBltParam.mYRes;

         if( lFirstStop > gsTriangleBltParam.mYRes )
         {
            lFirstStop = gsTriangleBltParam.mYRes;
         }
      }

      // If the upper part of the triangle is below the screen top, draw it
      if( lFirstStop > 0 ) 
      {

         if( lFirstStop != lTopLine )
         {

            lCurrentLine = lTopLine;

            lXLeft_4096 = lXRight_4096 = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]]*4096;

            lU_4096 = gsTriangleBltParam.mBitmapCol_4096[ lTop ];
            lV_4096 = gsTriangleBltParam.mBitmapRow_4096[ lTop ];
            lZ_4096 = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX*4096;

            if( !lMiddleShouldBeOnRight )
            {
               lDU_PerLine_4096 = ( gsTriangleBltParam.mBitmapCol_4096[ lMiddle ] - gsTriangleBltParam.mBitmapCol_4096[ lTop ] )/(lMiddleLine-lTopLine);
               lDV_PerLine_4096 = ( gsTriangleBltParam.mBitmapRow_4096[ lMiddle ] - gsTriangleBltParam.mBitmapRow_4096[ lTop ] )/(lMiddleLine-lTopLine);
               lDZ_PerLine_4096 = ( gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX - gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX )*4096/(lMiddleLine-lTopLine);
            }


            if( lCurrentLine < 0 )
            {
               lXLeft_4096  += -lCurrentLine*lLeftSlope;
               lXRight_4096 += -lCurrentLine*lRightSlope;

               lU_4096      += -lCurrentLine*lDU_PerLine_4096;
               lV_4096      += -lCurrentLine*lDV_PerLine_4096;
               lZ_4096      += -lCurrentLine*lDZ_PerLine_4096;

               lCurrentLine = 0;              

            }

            lLineBuffer  = gsTriangleBltParam.mBuffer[lCurrentLine];
            lLineZBuffer = gsTriangleBltParam.mZBuffer[lCurrentLine];
            
            while( lCurrentLine < lFirstStop )
            {
              
               int lXLeft  = lXLeft_4096/4096;
               int lXRight = lXRight_4096/4096;

               if( (lXLeft < gsTriangleBltParam.mXRes) && (lXLeft < lXRight ) )
               {
                  int lLocalU_4096 = lU_4096;
                  int lLocalV_4096 = lV_4096;
                  int lLocalZ_4096 = lZ_4096;

                  if( lXRight > gsTriangleBltParam.mXRes )
                  {
                     lXRight = gsTriangleBltParam.mXRes;
                  }

                  if( lXLeft < 0 )
                  {
                     lLocalU_4096 += -lXLeft*lDU_PerPixel_4096;
                     lLocalV_4096 += -lXLeft*lDV_PerPixel_4096;
                     lLocalZ_4096 += -lXLeft*lDZ_PerPixel_4096;
                     lXLeft   = 0;
                  }

                  while( lXLeft < lXRight )
                  {
                     if( lLineZBuffer[ lXLeft ] >= lLocalZ_4096/(4096*MR_ZBUFFER_UNIT) )
                     {
                        lLineZBuffer[ lXLeft ]= lLocalZ_4096/(4096*MR_ZBUFFER_UNIT);
                        // lLineBuffer[ lXLeft ] = MR_ColorTable[ gsTriangleBltParam.mLightIntensity ]
                        //                                      [ gsTriangleBltParam.mBitmap[ (lLocalU_4096/4096)&gsTriangleBltParam.mBitmapColMask ]
                        //                                                                  [ (lLocalV_4096/4096)&gsTriangleBltParam.mBitmapRowMask ] ];
                        lLineBuffer[ lXLeft ] = gsTriangleBltParam.mBitmap[ (lLocalU_4096/4096)&gsTriangleBltParam.mBitmapColMask ]
                                                                          [ (lLocalV_4096/4096)&gsTriangleBltParam.mBitmapRowMask ];
                     }

                     lXLeft++;
                     lLocalU_4096 += lDU_PerPixel_4096;
                     lLocalV_4096 += lDV_PerPixel_4096;
                     lLocalZ_4096 += lDZ_PerPixel_4096;

                  }
               
               }
               lCurrentLine++;

               lLineBuffer  += gsTriangleBltParam.mLineLen;
               lLineZBuffer += gsTriangleBltParam.mZLineLen;

               lXLeft_4096  += lLeftSlope;
               lXRight_4096 += lRightSlope;

               lU_4096 += lDU_PerLine_4096;
               lV_4096 += lDV_PerLine_4096;                    
               lZ_4096 += lDZ_PerLine_4096;                    

            }

            // Prepare the next part of the triangle


            if( lFirstStop != lSecondStop )
            {
               if( !lMiddleShouldBeOnRight )
               {
                  lU_4096 = gsTriangleBltParam.mBitmapCol_4096[ lMiddle ];
                  lV_4096 = gsTriangleBltParam.mBitmapRow_4096[ lMiddle ];
                  lZ_4096 = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX*4096;


                  lDU_PerLine_4096 = ( gsTriangleBltParam.mBitmapCol_4096[ lBottom ] - gsTriangleBltParam.mBitmapCol_4096[ lMiddle ] )/(lBottomLine-lMiddleLine);
                  lDV_PerLine_4096 = ( gsTriangleBltParam.mBitmapRow_4096[ lBottom ] - gsTriangleBltParam.mBitmapRow_4096[ lMiddle ] )/(lBottomLine-lMiddleLine);
                  lDZ_PerLine_4096 = ( gsRotatedPatch[ gsTriangleBltParam.mVertexList[lBottom] ].mX - gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX )*4096/(lBottomLine-lMiddleLine);
               }

               if( lMiddleShouldBeOnRight )
               {
                  lXRight_4096 = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096;
                  lRightSlope = lBottomSlope;
               }
               else
               {
                  lXLeft_4096  = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096;
                  lLeftSlope = lBottomSlope;
               }
            }

         }
         else
         {
            // First stop equal top of triangle
            lCurrentLine = lFirstStop;

            if( lFirstStop != lSecondStop )
            {
    
               if( lMiddleShouldBeOnRight )
               {
                  lU_4096 = gsTriangleBltParam.mBitmapCol_4096[ lTop ];
                  lV_4096 = gsTriangleBltParam.mBitmapRow_4096[ lTop ];
                  lZ_4096 = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX*4096;

               }
               else
               {
                  lU_4096 = gsTriangleBltParam.mBitmapCol_4096[ lMiddle ];
                  lV_4096 = gsTriangleBltParam.mBitmapRow_4096[ lMiddle ];
                  lZ_4096 = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX*4096;


                  lDU_PerLine_4096 = ( gsTriangleBltParam.mBitmapCol_4096[ lBottom ] - gsTriangleBltParam.mBitmapCol_4096[ lMiddle ] )/(lBottomLine-lMiddleLine);
                  lDV_PerLine_4096 = ( gsTriangleBltParam.mBitmapRow_4096[ lBottom ] - gsTriangleBltParam.mBitmapRow_4096[ lMiddle ] )/(lBottomLine-lMiddleLine);
                  lDZ_PerLine_4096 = ( gsRotatedPatch[ gsTriangleBltParam.mVertexList[lBottom] ].mX - gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX )*4096/(lBottomLine-lMiddleLine);
               }


               if( lMiddleShouldBeOnRight )
               {
                  lXLeft_4096  = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]]*4096;
                  lXRight_4096 = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096;

                  lRightSlope = lBottomSlope;
               }
               else
               {
                  lXLeft_4096  = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096;
                  lXRight_4096 = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]]*4096;

                  lLeftSlope = lBottomSlope;
               }
            }
         }
      }
      else
      {
         // The first triangle have not been drawn, make some adjustment for the bottom part
         lCurrentLine = 0;


         if( lMiddleShouldBeOnRight )
         {
            lU_4096  = gsTriangleBltParam.mBitmapCol_4096[ lTop ];
            lV_4096  = gsTriangleBltParam.mBitmapRow_4096[ lTop ];
            lZ_4096 = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lTop] ].mX*4096;

            lU_4096 += -lTopLine*lDU_PerLine_4096;
            lV_4096 += -lTopLine*lDV_PerLine_4096;
            lZ_4096 += -lTopLine*lDZ_PerLine_4096;

         }
         else
         {
            lU_4096 = gsTriangleBltParam.mBitmapCol_4096[ lMiddle ];
            lV_4096 = gsTriangleBltParam.mBitmapRow_4096[ lMiddle ];
            lZ_4096 = gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX*4096;

            lDU_PerLine_4096 = ( gsTriangleBltParam.mBitmapCol_4096[ lBottom ] - gsTriangleBltParam.mBitmapCol_4096[ lMiddle ] )/(lBottomLine-lMiddleLine);
            lDV_PerLine_4096 = ( gsTriangleBltParam.mBitmapRow_4096[ lBottom ] - gsTriangleBltParam.mBitmapRow_4096[ lMiddle ] )/(lBottomLine-lMiddleLine);
            lDZ_PerLine_4096 = ( gsRotatedPatch[ gsTriangleBltParam.mVertexList[lBottom] ].mX - gsRotatedPatch[ gsTriangleBltParam.mVertexList[lMiddle] ].mX )*4096/(lBottomLine-lMiddleLine);

            lU_4096 += -lMiddleLine*lDU_PerLine_4096;
            lV_4096 += -lMiddleLine*lDV_PerLine_4096;
            lZ_4096 += -lMiddleLine*lDZ_PerLine_4096;

         }


         if( lMiddleShouldBeOnRight )
         {
            lXLeft_4096  = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]]*4096;
            lXRight_4096 = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096;

            lRightSlope = lBottomSlope;

            lXLeft_4096  += -lTopLine*lLeftSlope;
            lXRight_4096 += -lMiddleLine*lRightSlope;

         }
         else
         {
            lXLeft_4096  = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lMiddle]]*4096;
            lXRight_4096 = gsScreenXPatch[ gsTriangleBltParam.mVertexList[lTop]]*4096;

            lLeftSlope = lBottomSlope;

            lXLeft_4096  += -lMiddleLine*lLeftSlope;
            lXRight_4096 += -lTopLine*lRightSlope;
         }
      }

      // Draw the bottom part of the triangle
      lLineBuffer  = gsTriangleBltParam.mBuffer[lCurrentLine];
      lLineZBuffer = gsTriangleBltParam.mZBuffer[lCurrentLine];


      while( lCurrentLine < lSecondStop )
      {
         int lXLeft  = lXLeft_4096/4096;
         int lXRight = lXRight_4096/4096;

         if( (lXLeft < gsTriangleBltParam.mXRes) && (lXLeft < lXRight ) )
         {
            int lLocalU_4096 = lU_4096;
            int lLocalV_4096 = lV_4096;
            int lLocalZ_4096 = lZ_4096;

            if( lXRight > gsTriangleBltParam.mXRes )
            {
               lXRight = gsTriangleBltParam.mXRes;
            }

            if( lXLeft < 0 )
            {
               lLocalU_4096 += -lXLeft*lDU_PerPixel_4096;
               lLocalV_4096 += -lXLeft*lDV_PerPixel_4096;
               lLocalZ_4096 += -lXLeft*lDZ_PerPixel_4096;
               lXLeft   = 0;
            }

            while( lXLeft < lXRight )
            {
               if( lLineZBuffer[ lXLeft ] >= lLocalZ_4096/(4096*MR_ZBUFFER_UNIT) )
               {
                  lLineZBuffer[ lXLeft ]= lLocalZ_4096/(4096*MR_ZBUFFER_UNIT);
                  // lLineBuffer[ lXLeft ] = MR_ColorTable[ gsTriangleBltParam.mLightIntensity ]
                  //                                      [ gsTriangleBltParam.mBitmap[ (lLocalU_4096/4096)&gsTriangleBltParam.mBitmapColMask ]
                  //                                                                  [ (lLocalV_4096/4096)&gsTriangleBltParam.mBitmapRowMask ] ];
                  lLineBuffer[ lXLeft ] = gsTriangleBltParam.mBitmap[ (lLocalU_4096/4096)&gsTriangleBltParam.mBitmapColMask ]
                                                                    [ (lLocalV_4096/4096)&gsTriangleBltParam.mBitmapRowMask ];
               }


               lXLeft++;
               lLocalU_4096 += lDU_PerPixel_4096;
               lLocalV_4096 += lDV_PerPixel_4096;                    
               lLocalZ_4096 += lDZ_PerPixel_4096;                    
            }
               
         }
         lCurrentLine++;

         lLineBuffer  += gsTriangleBltParam.mLineLen;
         lLineZBuffer += gsTriangleBltParam.mZLineLen;

         lXLeft_4096  += lLeftSlope;
         lXRight_4096 += lRightSlope;

         lU_4096 += lDU_PerLine_4096;
         lV_4096 += lDV_PerLine_4096;                    
         lZ_4096 += lDZ_PerLine_4096;                    

      }
   }
}




void MR_3DViewPort::RenderBackground( const MR_UInt8* pBitmap )
{

   int lStartingLine     = mYRes/2-1+mScroll;
   int lBottomLine       = lStartingLine + mYRes/8;

   if( lBottomLine >= mYRes )
   {
      lBottomLine = mYRes-1;

      if( lStartingLine >= mYRes )
      {
         // Not correct but if the guy whant to look at the sky 
         // I bon't care if it is not correct
         lStartingLine = mYRes-1;
      }
   }

   if( lStartingLine < 0 )
   {
      // Forget about that
      return;
   }


   for( int lColumn = 0; lColumn < mXRes; lColumn++ )
   {
      int       lRow;
      int       lBitmapColumn  = (MR_BACK_X_RES+((MR_PI/2-mOrientation)*MR_BACK_X_RES/MR_2PI)+mBackgroundConst[ lColumn ].mBitmapColumn)&(MR_BACK_X_RES-1);
      MR_UInt8* lDest          = mBufferLine[lStartingLine]+lColumn;

      const MR_UInt8* lSrc     = pBitmap+lBitmapColumn*MR_BACK_Y_RES;
      MR_Int32  lSrcIndex_1024 = MR_BACK_Y_RES*1024/9;
      MR_Int32  lSrcInc_1024   = mBackgroundConst[ lColumn ].mLineIncrement_1024;
      
      for( lRow = lStartingLine; lRow >= 0; lRow-- )
      {
         *lDest = lSrc[ (lSrcIndex_1024/1024)>(MR_BACK_Y_RES-1)?(MR_BACK_Y_RES-1):(lSrcIndex_1024/1024) ];

         lDest          -= mLineLen;
         lSrcIndex_1024 += lSrcInc_1024;
      }

      lDest              = mBufferLine[lStartingLine+1]+lColumn;
      lSrcIndex_1024     = (MR_BACK_Y_RES*1024/9)-lSrcInc_1024;
      
      for( lRow = lStartingLine+1; lRow <lBottomLine; lRow++ )
      {
         *lDest = lSrc[ (lSrcIndex_1024/1024)<0?0:(lSrcIndex_1024/1024) ];

         lDest          += mLineLen;
         lSrcIndex_1024 -= lSrcInc_1024;
      }
      

   }
}










// Old stuff.. keep until june 96

/*

void InterpolateLine( const MR_3DCoordinate& p0, 
                      const MR_3DCoordinate& p1,
                      MR_3DCoordinate& pMid     )
{
   pMid.mY = p0.mY + MulDiv( p1.mY-p0.mY, pMid.mX-p0.mX, p1.mX-p0.mX );
   pMid.mZ = p0.mZ + MulDiv( p1.mZ-p0.mZ, pMid.mX-p0.mX, p1.mX-p0.mX );
}

*/

/*
   // Cut out of depth
   int lNbVertex = 0;
   MR_3DCoordinate lCuttedCoordinate[ 8 ];

   int lPointDepth[4] = { 0,0,0,0 };
   int lNbFront = 0;
   int lNbBack  = 0;

   for( lCounter = 0; lCounter<4; lCounter++ )
   {
      if( lRotatedCoordinate[lCounter].mX<mPlanDist )
      {
         lPointDepth[ lCounter ] = -1;
         lNbFront++;
      }
      else if( lRotatedCoordinate[lCounter].mX > MR_ZBUFFER_UNIT*0xFFFE )
      {
         lPointDepth[ lCounter ] = 1;
         lNbBack++;
      }
   }


   if( (lNbFront == 4)||(lNbBack==4) )
   {
      // All the points are outside the view
      return;
   }

   for( lCounter = 0; lCounter < 4; lCounter++ )
   {
      if( lPointDepth[ lCounter ] == -1 )
      {
         // check the previous
         if( lPointDepth[ (lCounter+3)%4 ] != -1 )
         {
            lCuttedCoordinate[ lNbVertex ].mX = mPlanDist;
            InterpolateLine( lRotatedCoordinate[ (lCounter+3)%4 ],
                             lRotatedCoordinate[ lCounter ],
                             lCuttedCoordinate[ lNbVertex ] );

            lNbVertex++;
         }

         // Check the next
         if( lPointDepth[ (lCounter+1)%4 ] != -1 )
         {
            lCuttedCoordinate[ lNbVertex ].mX = mPlanDist;
            InterpolateLine( lRotatedCoordinate[ (lCounter+1)%4 ],
                             lRotatedCoordinate[ lCounter ],
                             lCuttedCoordinate[ lNbVertex ] );
            lNbVertex++;
         }
      }
      else if( lPointDepth[ lCounter ] == 1 )
      {
         // check the previous
         if( lPointDepth[ (lCounter+3)%4 ] != 1 )
         {
            lCuttedCoordinate[ lNbVertex ].mX = MR_ZBUFFER_UNIT*0xFFFE;
            InterpolateLine( lRotatedCoordinate[ (lCounter+3)%4 ],
                             lRotatedCoordinate[ lCounter ],
                             lCuttedCoordinate[ lNbVertex ] );

            lNbVertex++;
         }

         // Check the next
         if( lPointDepth[ (lCounter+1)%4 ] != 1 )
         {
            lCuttedCoordinate[ lNbVertex ].mX = MR_ZBUFFER_UNIT*0xFFFE;
            InterpolateLine( lRotatedCoordinate[ (lCounter+1)%4 ],
                             lRotatedCoordinate[ lCounter ],
                             lCuttedCoordinate[ lNbVertex ] );

            lNbVertex++;
         }
      }
      else
      {
         lCuttedCoordinate[ lNbVertex++ ] = lRotatedCoordinate[ lCounter ];
      }
   }

   // Transform in screen coordinates
   MR_2DCoordinate lScreenCoordinate[8];

   for( lCounter = 0; lCounter<lNbVertex; lCounter++ )
   {
      lScreenCoordinate[ lCounter ].mX = MulDiv( -lCuttedCoordinate[ lCounter ].mY,  mXRes * mPlanDist, lCuttedCoordinate[ lCounter ].mX*mPlanHW*2 ) + mXRes/2;
      lScreenCoordinate[ lCounter ].mY = MulDiv( -lCuttedCoordinate[ lCounter ].mZ,  mYRes * mPlanDist, lCuttedCoordinate[ lCounter ].mX*mPlanVW*2 ) + mYRes/2;
   }

   // Cut out of out of screen coordinates
   

   // Determine the Depth Constant slope
   MR_3DCoordinate lNormalVector;
   int             lDepthDX;      // Virtual coordinates slope
   int             lDepthDY;

   lNormalVector.mX =  (lRotatedCoordinate[0].mZ-lRotatatedCoordinate[3].mZ)*(lRotatedCoordinate[0].mY-lRotatatedCoordinate[3].mY);
   lNormalVector.mY = -(lRotatedCoordinate[0].mZ-lRotatatedCoordinate[3].mZ)*(lRotatedCoordinate[0].mX-lRotatatedCoordinate[3].mX);
   lNormalVector.mZ = -(lRotatedCoordinate[0].mX-lRotatatedCoordinate[3].mX)*(lRotatedCoordinate[0].mY-lRotatatedCoordinate[3].mY);

   if( lNormalVector.mY = 0 )
   {
      lDepthSlope = 0;
   }
   else
   {
      lDepthSlope = MulDiv( lNormalVector.mZ, 4096, -lNormalVector.mY );
      lDepthSlope = (lDepthSlope * mPlanVW * mXRes)/(mPlanHW * mYRes);
   }

   // Debug, draw the surface contour
   
   for( lCounter = 0; lCounter< lNbVertex; lCounter++ )
   {
      DrawLine( lScreenCoordinate[ lCounter ].mX, 
                lScreenCoordinate[ lCounter ].mY,
                lScreenCoordinate[ (lCounter+1)%lNbVertex ].mX,
                lScreenCoordinate[ (lCounter+1)%lNbVertex ].mY,
                7 );
   }

   // Debug Draw a Depth constant line

*/