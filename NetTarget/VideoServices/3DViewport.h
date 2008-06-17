// 3DViewport.h
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


#ifndef _3D_VIEWPORT_H
#define _3D_VIEWPORT_H

#include "2DViewport.h"
#include "ColorPalette.h"
#include "Bitmap.h"
#include "Patch.h"

#ifdef MR_VIDEO_SERVICES
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

// define
#define MR_BACK_X_RES 2048
#define MR_BACK_Y_RES  256

// Helper class
class MR_PositionMatrix
{
   public:
      MR_Int32        mRotation[2][2];
      MR_3DCoordinate mDisplacement; 
};



class MR_3DViewPort: public MR_2DViewPort
{   
   protected:

      class BackColumn
      {
         public:
            MR_Int32 mBitmapColumn;
            MR_Int32 mLineIncrement_1024;
      };


           
      //Camera setting
      MR_3DCoordinate mPosition;
      MR_Angle        mOrientation;           // Angle from the XAxis
      int             mScroll;                // Offset to YCenter
      MR_Angle        mVAngle;                // Vertical field angle width
      MR_Int32        mPlanDist;              // ProjectionPlan (screen) distance
      MR_Int32        mPlanHW;                // Horizontal Half
      MR_Int32        mPlanVW;                // Vertical Half 

      MR_UInt16*  mZBuffer;
      int         mZLineLen;

      MR_UInt8**  mBufferLine;
      MR_UInt16** mZBufferLine;

      // Usefull pre-defined constants
      MR_Int32        mHVarPerDInc_16384;     // Ray divergence by HPixel
      MR_Int32        mVVarPerDInc_16384;     // Ray divergence by VPixel
      MR_Int32        mXRes_PlanDist;
      MR_Int32        mYRes_PlanDist;

      MR_Int32        mPlanHW_PlanDist_2_XRes_16384; // 16384*mPlanHW/(mPlanDist*2*mXRes)
      MR_Int32        mXRes_PlanDist_2PlanHW_4096;   // mXRes*mPlanDist*2048/mPlanHW

      BackColumn*     mBackgroundConst;      // Constants used to display each bitmap column     

      MR_Int32  mRotationMatrix[3][3];

      void ComputeRotationMatrix();
      void ComputeBackgroundConst();

      void ApplyRotationMatrix( const MR_3DCoordinate& pSrc, MR_3DCoordinate& pDest )const;
      void ApplyRotationMatrix( const MR_2DCoordinate& pSrc, MR_2DCoordinate& pDest )const;
      void ApplyPositionMatrix( const MR_PositionMatrix& pMatrix, const MR_3DCoordinate& pSrc, MR_3DCoordinate& pDest )const;

      MR_DllDeclare void OnMetricsChange( int pMetrics );

   public:

      MR_DllDeclare MR_3DViewPort();
      MR_DllDeclare ~MR_3DViewPort();

      MR_DllDeclare void Setup( MR_VideoBuffer* pBuffer, int pX0, int pY0, int pSizeX, int pSizeY, MR_Angle pApperture, int pMetrics = eNone );

      MR_DllDeclare void SetupCameraPosition( const MR_3DCoordinate& pPosition,
                                              MR_Angle               pOrientation,
                                              int                    pScroll       );

      MR_DllDeclare void ClearZ();

      MR_DllDeclare BOOL ComputePositionMatrix( MR_PositionMatrix& pMatrix, const MR_3DCoordinate& pPosition, MR_Angle pOrientation, MR_Int32 pMaxObjRay );

      // WireFrame services
      MR_DllDeclare void DrawWFLine(  const MR_3DCoordinate& pP0, const MR_3DCoordinate& pP1, MR_UInt8 pColor );

      // Rendering services ( availlable in the file 3DViewportRendering.cpp )
      MR_DllDeclare void RenderWallSurface( const MR_3DCoordinate& pUpperLeft, const MR_3DCoordinate& pLowerRight, MR_Int32 pLen, const MR_Bitmap* pBitmap );
      MR_DllDeclare void RenderAlternateWallSurface( const MR_3DCoordinate& pUpperLeft, const MR_3DCoordinate& pLowerRight, MR_Int32 pLen, const MR_Bitmap* pBitmap, const MR_Bitmap* pBitmap2, int pSerialLen, int pSerialStart );

      MR_DllDeclare void RenderHorizontalSurface( int lNbVertex, const MR_2DCoordinate* pVertexList, MR_Int32 pLevel, BOOL lTop, const MR_Bitmap* pBitmap );

      MR_DllDeclare void RenderPatch( const MR_Patch& pPatch, const MR_PositionMatrix& pMatrix , const MR_Bitmap* pBitmap );
      MR_DllDeclare void RenderPatch( const MR_Patch& pPatch, const MR_PositionMatrix& pMatrix , MR_UInt8 pColor );


      MR_DllDeclare void RenderBackground( const MR_UInt8* pBitmap );
};

// Local constants (Used by the cpp of this module)
#define MR_ZBUFFER_LIMIT  0xFFFE
#define MR_ZBUFFER_UNIT        4 // Each increment is 4 mm
                                 // max deep of field is 260m
#define MR_MAX_POLYGON_VERTEX 16


#undef MR_DllDeclare

#endif



