// BitmapSurface.cpp
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

#include "StdAfx.h"

#include "BitmapSurface.h"
#include "../Model/PhysicalCollision.h"

//Local data
static BOOL                 gLocalInitialized = FALSE;
static MR_PhysicalCollision gEffect;
static MR_ContactEffectList gEffectList;


// 

MR_BitmapSurface::MR_BitmapSurface( const MR_ObjectFromFactoryId& pId )
                 :MR_SurfaceElement( pId )
{
   // The task of initialising the data members is done by the superclass
   mBitmap       = NULL;
   mBitmap2      = NULL;
   mRotationSpeed = 0;
   mRotationLen = 1;

   if( !gLocalInitialized )
   {
      gLocalInitialized = TRUE;
      gEffectList.AddTail( &gEffect );

      gEffect.mWeight = MR_InertialMoment::eInfiniteWeight;
      gEffect.mXSpeed = 0;
      gEffect.mYSpeed = 0;
      gEffect.mZSpeed = 0;
   }
}


MR_BitmapSurface::MR_BitmapSurface( const MR_ObjectFromFactoryId& pId, /*const*/ MR_ResBitmap* pBitmap )
                 :MR_SurfaceElement( pId )
{
   // The task of initialising the data members i
   mBitmap       = pBitmap;
   mBitmap2      = pBitmap;
   mRotationSpeed = 0;
   mRotationLen = 1;

   if( !gLocalInitialized )
   {
      gLocalInitialized = TRUE;
      gEffectList.AddTail( &gEffect );

      gEffect.mWeight = MR_InertialMoment::eInfiniteWeight;
      gEffect.mXSpeed = 0;
      gEffect.mYSpeed = 0;
      gEffect.mZSpeed = 0;
   }

}


MR_BitmapSurface::MR_BitmapSurface( const MR_ObjectFromFactoryId& pId, MR_ResBitmap* pBitmap1, MR_ResBitmap* pBitmap2, int pRotationSpeed, int pRotationLen )
                 :MR_SurfaceElement( pId )
{
   // The task of initialising the data members i
   mBitmap        = pBitmap1;
   mBitmap2       = pBitmap2;
   mRotationSpeed = pRotationSpeed;
   mRotationLen   = pRotationLen;

   if( !gLocalInitialized )
   {
      gLocalInitialized = TRUE;
      gEffectList.AddTail( &gEffect );

      gEffect.mWeight = MR_InertialMoment::eInfiniteWeight;
      gEffect.mXSpeed = 0;
      gEffect.mYSpeed = 0;
      gEffect.mZSpeed = 0;
   }

}



MR_BitmapSurface::~MR_BitmapSurface()
{
}


void MR_BitmapSurface::RenderWallSurface( MR_3DViewPort* pDest, const MR_3DCoordinate& pUpperLeft, const MR_3DCoordinate& pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime )
{
   if( mBitmap != NULL )
   {
      if( mRotationSpeed != 0 )
      {
         int lStartPos = (pTime+40000)/mRotationSpeed;

         if( lStartPos < 0 )
         {
            lStartPos = mRotationLen-1-((-lStartPos)%mRotationLen);
         }
         else
         {
            lStartPos = (lStartPos)%mRotationLen;
         }
         pDest->RenderAlternateWallSurface( pUpperLeft, pLowerRight, pLen, mBitmap, mBitmap2, mRotationLen, lStartPos );
         
      }
      else
      {
         pDest->RenderWallSurface( pUpperLeft, pLowerRight, pLen, mBitmap );
      }
   }
}

void MR_BitmapSurface::RenderHorizontalSurface( MR_3DViewPort* pDest, int pNbVertex, const MR_2DCoordinate* pVertexList, MR_Int32 pLevel, BOOL pTop, MR_SimulationTime /*pTime*/ )
{
   if( mBitmap != NULL )
   {
      pDest->RenderHorizontalSurface( pNbVertex, pVertexList, pLevel, pTop, mBitmap );
   }
}

const MR_ContactEffectList* MR_BitmapSurface::GetEffectList()
{
   return &gEffectList;
}


// MR_VStretchBitmapSurface:public 

MR_VStretchBitmapSurface::MR_VStretchBitmapSurface( const MR_ObjectFromFactoryId& pId, /*const*/ MR_ResBitmap* pBitmap, int pMaxHeight )
                         :MR_BitmapSurface( pId, pBitmap )
{
   mMaxHeight = pMaxHeight;
}

      
MR_VStretchBitmapSurface::MR_VStretchBitmapSurface( const MR_ObjectFromFactoryId& pId, MR_ResBitmap* pBitmap1, MR_ResBitmap* pBitmap2, int pRotationSpeed, int pRotationLen, int pMaxHeight )
                         :MR_BitmapSurface( pId, pBitmap1, pBitmap2, pRotationSpeed, pRotationLen )
{
   mMaxHeight = pMaxHeight;
}

void MR_VStretchBitmapSurface::RenderWallSurface( MR_3DViewPort* pDest, const MR_3DCoordinate& pUpperLeft, const MR_3DCoordinate& pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime )
{
   if( mBitmap != NULL )
   {
      int lHeight = pUpperLeft.mZ-pLowerRight.mZ;

      if( lHeight > 0 )
      {
         int lDivisor = 1+(lHeight-1)/mMaxHeight;

         if( lDivisor > 1 )
         {
            lHeight = lHeight/lDivisor;
         }

         mBitmap->SetWidthHeight( lHeight, lHeight );
      
         MR_BitmapSurface::RenderWallSurface( pDest, pUpperLeft, pLowerRight, pLen, pTime );
      }
   }
}





