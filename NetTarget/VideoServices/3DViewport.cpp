// 3DViewport.cpp
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
#include "GpuSceneRenderer.h"
#include "../Util/FastMemManip.h"
#include "../Util/Config.h"

#include <math.h>

namespace {
	long long GetPerfTick()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}

	DWORD PerfTicksToMs(long long ticks)
	{
		static LARGE_INTEGER frequency = {0};
		if(frequency.QuadPart == 0) {
			QueryPerformanceFrequency(&frequency);
		}
		if((ticks <= 0) || (frequency.QuadPart <= 0)) {
			return 0;
		}
		return static_cast<DWORD>((ticks * 1000) / frequency.QuadPart);
	}
}

// Methods implementation
MR_3DViewPort::MR_3DViewPort()
{
	mOrientation = 0;
	mPosition = MR_3DCoordinate(0, 0, 0);
	mVAngle = 1;
	mScroll = 0;

	mBufferLine = NULL;
	mZBufferLine = NULL;
	mBackgroundConst = NULL;
	mBackgroundSourceColumn = NULL;
	mBackgroundRowIndex_1024 = NULL;
	mWallSetupTicks = 0;
	mWallLoopTicks = 0;

}

MR_3DViewPort::~MR_3DViewPort()
{
	delete[]mBufferLine;
	delete[]mZBufferLine;
	delete[]mBackgroundConst;
	delete[]mBackgroundSourceColumn;
	delete[]mBackgroundRowIndex_1024;
}

MR_GpuScenePositionMatrix MR_3DViewPort::BuildGpuScenePositionMatrix(const MR_PositionMatrix &pMatrix) const
{
	MR_GpuScenePositionMatrix matrix;
	matrix.mRotation[0][0] = pMatrix.mRotation[0][0];
	matrix.mRotation[0][1] = pMatrix.mRotation[0][1];
	matrix.mRotation[1][0] = pMatrix.mRotation[1][0];
	matrix.mRotation[1][1] = pMatrix.mRotation[1][1];
	matrix.mDisplacement = pMatrix.mDisplacement;
	return matrix;
}

void MR_3DViewPort::OnMetricsChange(int pMetrics)
{
	if((pMetrics & ~eBuffer) != eNone) {
		// Recompute camera parameters

		MR_Config *cfg = MR_Config::GetInstance();

		if(cfg->video.useOriginalCameraParams) {
			mPlanDist = 200;						  // 20cm Fixed distance projection plan, good in most cases

			ASSERT(MR_Sin[mVAngle] != 0);
			mPlanHW = (mPlanDist * MR_Sin[mVAngle / 2]) / MR_Int32(MR_Cos[mVAngle / 2]);

			long long lPlanVW = (long long(mPlanHW) * long long(mYRes) * long long(mVideoBuffer->GetXPixelMeter())) / (long long(mXRes) * long long(mVideoBuffer->GetYPixelMeter()));
			mPlanVW = (MR_Int32) lPlanVW;
		} else {
			ASSERT(MR_Sin[mVAngle] != 0);
			mPlanHW = (200 * MR_Sin[mVAngle / 2]) / MR_Int32(MR_Cos[mVAngle / 2]);

			long long lPlanVW = (long long(mPlanHW) * long long(mYRes)) / (long long(mXRes));
			mPlanVW = (MR_Int32) lPlanVW;

			// Simple calculation but pretty good!
			// 200 / mPlanHW is to compensate for zoom changes
			mPlanDist = (float) lPlanVW * 4.0 / 3.0 * (200.0 / (float) mPlanHW);
		}

		mHVarPerDInc_16384 = -(mPlanHW * 2 * 16384 + 1) / (mPlanDist * mXRes);
		mVVarPerDInc_16384 = -(mPlanVW * 2 * 16384 + 1) / (mPlanDist * mYRes);

		mXRes_PlanDist = mXRes * mPlanDist;
		mYRes_PlanDist = mYRes * mPlanDist;

		mPlanHW_PlanDist_2_XRes_16384 = 2 * 16384 * mPlanHW / (mPlanDist * mXRes);

		mXRes_PlanDist_2PlanHW_4096 = mXRes * mPlanDist * 2048 / mPlanHW;

		pMetrics |= eBuffer;
	}

	if(pMetrics & eBuffer) {
		delete[]mBufferLine;
		delete[]mZBufferLine;
		delete[]mBackgroundSourceColumn;
		delete[]mBackgroundRowIndex_1024;

		mBufferLine = new MR_UInt8 *[mYRes];
		mZBufferLine = new MR_UInt16 *[mYRes];
		mBackgroundSourceColumn = new const MR_UInt8 *[mXRes];
		mBackgroundRowIndex_1024 = new MR_Int32[mXRes];

		MR_UInt8 *lLineBuffer = mBuffer;
		MR_UInt16 *lZLineBuffer = mZBuffer;

		for(int lCounter = 0; lCounter < mYRes; lCounter++) {
			mBufferLine[lCounter] = lLineBuffer;
			mZBufferLine[lCounter] = lZLineBuffer;

			lLineBuffer += mLineLen;
			lZLineBuffer += mZLineLen;
		}
	}

	ComputeBackgroundConst();
}

void MR_3DViewPort::Setup(MR_VideoBuffer * pBuffer, int pX0, int pY0, int pSizeX, int pSizeY, MR_Angle pApperture, int pMetrics)
{
	mZLineLen = pBuffer->GetZLineLen();
	MR_UInt16 *lNewZBuffer = pBuffer->GetZBuffer() + pX0 + mZLineLen * pY0;

	if(lNewZBuffer != mZBuffer) {
		mZBuffer = lNewZBuffer;
		pMetrics |= eBuffer;
	}

	if(pApperture != mVAngle) {
		mVAngle = pApperture;
		pMetrics |= eXSize | eYSize;			  // Generate a recomputation of mA and mB
	}

	MR_2DViewPort::Setup(pBuffer, pX0, pY0, pSizeX, pSizeY, pMetrics);
}

void MR_3DViewPort::SetupCameraPosition(const MR_3DCoordinate & pPosition, MR_Angle pOrientation, int pScroll)
{
	mPosition = pPosition;
	mOrientation = pOrientation;
	mScroll = pScroll * mYRes / 8;

	ComputeRotationMatrix();
}

void MR_3DViewPort::BeginGpuSceneFrame()
{
	if((mVideoBuffer == NULL) || (mBuffer == NULL)) {
		return;
	}

	MR_GpuSceneRenderer *renderer = mVideoBuffer->GetGpuSceneRenderer();
	if(renderer == NULL || !renderer->IsEnabled()) {
		return;
	}

	RECT viewport;
	viewport.left = 0;
	viewport.top = 0;
	viewport.right = mXRes;
	viewport.bottom = mYRes;
	renderer->BeginFrame(viewport, mPosition, mOrientation, mScroll,
		mPlanDist, mPlanHW, mPlanVW);
}

void MR_3DViewPort::EndGpuSceneFrame()
{
	if(mVideoBuffer == NULL) {
		return;
	}

	MR_GpuSceneRenderer *renderer = mVideoBuffer->GetGpuSceneRenderer();
	if(renderer == NULL || !renderer->IsEnabled()) {
		return;
	}

	renderer->EndFrame();
}

void MR_3DViewPort::ResetWallTimingStats()
{
	mWallSetupTicks = 0;
	mWallLoopTicks = 0;
}

DWORD MR_3DViewPort::GetWallSetupTimingMs() const
{
	return PerfTicksToMs(mWallSetupTicks);
}

DWORD MR_3DViewPort::GetWallLoopTimingMs() const
{
	return PerfTicksToMs(mWallLoopTicks);
}

void MR_3DViewPort::ComputeBackgroundConst()
{
	delete[]mBackgroundConst;

	mBackgroundConst = new BackColumn[mXRes];

	for(int lCounter = 0; lCounter < mXRes; lCounter++) {
		mBackgroundConst[lCounter].mBitmapColumn =
			static_cast<MR_Int32>(
				(atan((double) (256 * (lCounter - mXRes / 2) * mPlanHW / (mPlanDist * mXRes / 2)) / 256.0) * (double) MR_BACK_X_RES) / 6.28318530718
				);

		mBackgroundConst[lCounter].mLineIncrement_1024 =
			static_cast<MR_Int32>(
				1024 * MR_BACK_Y_RES * mPlanVW /
				(sqrt(pow(mPlanDist, 2.0f) + pow((lCounter - mXRes / 2) * mPlanHW / (mXRes / 2), 2.0f)) * (mYRes / 2))
				);
	}
}

/*
void MR_3DViewPort::ComputeA()
{
   delete []mA;

   mA = new MR_Int32[ mXRes+1 ];

   for( int lCounter = 0; lCounter <= mXRes; lCounter++ )
   {
	  mA[lCounter] = ( mPlanHW * MR_PROJ_FRACT * (2*lCounter-mXRes) )/( mPlanDist*mXRes );
   }

}

void MR_3DViewPort::ComputeB()
{
   delete []mB;

   mB = new MR_Int32[ mYRes+1 ];

   for( int lCounter = 0; lCounter <= mYRes; lCounter++ )
   {
	  mB[lCounter] = ( mPlanVW * MR_PROJ_FRACT * (2*lCounter-mYRes) )/( mPlanDist*mYRes );
   }

}
*/

void MR_3DViewPort::ComputeRotationMatrix()
{
	mRotationMatrix[0][0] = MR_Cos[mOrientation];
	mRotationMatrix[0][1] = MR_Sin[mOrientation];
	mRotationMatrix[0][2] = 0;

	mRotationMatrix[1][0] = -MR_Sin[mOrientation];
	mRotationMatrix[1][1] = MR_Cos[mOrientation];
	mRotationMatrix[1][2] = 0;

	mRotationMatrix[2][0] = 0;
	mRotationMatrix[2][1] = 0;
	mRotationMatrix[2][2] = MR_TRIGO_FRACT;

	/* Rotation matrix with tilt
	   // Do not take the pan in account for the moment

	   mRotationMatrix[0][0] = (MR_Cos[ mOrientation ] * MR_Cos[ mTilt ]);
	   mRotationMatrix[0][1] = (MR_Sin[ mOrientation ] * MR_Cos[ mTilt ]);
	   mRotationMatrix[0][2] = MR_Sin[ mTilt ]*MR_TRIGO_FRACT;

	   mRotationMatrix[1][0] = -MR_Sin[ mOrientation ]*MR_TRIGO_FRACT;
	   mRotationMatrix[1][1] = MR_Cos[ mOrientation ]*MR_TRIGO_FRACT;
	   mRotationMatrix[1][2] = 0;

	   mRotationMatrix[2][0] = (MR_Cos[ mOrientation ] * -MR_Sin[ mTilt ]);
	   mRotationMatrix[2][1] = (MR_Sin[ mOrientation ] * -MR_Sin[ mTilt ]);
	   mRotationMatrix[2][2] = MR_Cos[ mTilt ]*MR_TRIGO_FRACT;
	 */

}

void MR_3DViewPort::ApplyRotationMatrix(const MR_3DCoordinate & pSrc, MR_3DCoordinate & pDest) const
{
	MR_3DCoordinate lPos;

	lPos.mX = pSrc.mX - mPosition.mX;
	lPos.mY = pSrc.mY - mPosition.mY;
	lPos.mZ = pSrc.mZ - mPosition.mZ;

	pDest.mX = (MR_Int32) Int64ShraMod32(Int32x32To64(lPos.mX, mRotationMatrix[0][0]) + Int32x32To64(lPos.mY, mRotationMatrix[0][1]), MR_TRIGO_SHIFT);
	pDest.mY = (MR_Int32) Int64ShraMod32(Int32x32To64(lPos.mX, mRotationMatrix[1][0]) + Int32x32To64(lPos.mY, mRotationMatrix[1][1]), MR_TRIGO_SHIFT);
	pDest.mZ = lPos.mZ;

	/*
	   pDest.mX = (MR_Int32)Int64ShraMod32( Int32x32To64( lPos.mX, mRotationMatrix[0][0] ) + Int32x32To64( lPos.mY, mRotationMatrix[0][1] ) + Int32x32To64( lPos.mZ, mRotationMatrix[0][2]), 2*MR_TRIGO_SHIFT );
	   pDest.mY = (MR_Int32)Int64ShraMod32( Int32x32To64( lPos.mX, mRotationMatrix[1][0] ) + Int32x32To64( lPos.mY, mRotationMatrix[1][1] ) + Int32x32To64( lPos.mZ, mRotationMatrix[1][2]), 2*MR_TRIGO_SHIFT );
	   pDest.mZ = (MR_Int32)Int64ShraMod32( Int32x32To64( lPos.mX, mRotationMatrix[2][0] ) + Int32x32To64( lPos.mY, mRotationMatrix[2][1] ) + Int32x32To64( lPos.mZ, mRotationMatrix[2][2]), 2*MR_TRIGO_SHIFT );
	 */
}

void MR_3DViewPort::ApplyRotationMatrix(const MR_2DCoordinate & pSrc, MR_2DCoordinate & pDest) const
{
	MR_3DCoordinate lPos;

	lPos.mX = pSrc.mX - mPosition.mX;
	lPos.mY = pSrc.mY - mPosition.mY;

	pDest.mX = (MR_Int32) Int64ShraMod32(Int32x32To64(lPos.mX, mRotationMatrix[0][0]) + Int32x32To64(lPos.mY, mRotationMatrix[0][1]), MR_TRIGO_SHIFT);
	pDest.mY = (MR_Int32) Int64ShraMod32(Int32x32To64(lPos.mX, mRotationMatrix[1][0]) + Int32x32To64(lPos.mY, mRotationMatrix[1][1]), MR_TRIGO_SHIFT);
}

BOOL MR_3DViewPort::ComputePositionMatrix(MR_PositionMatrix & pMatrix, const MR_3DCoordinate & pPosition, MR_Angle pOrientation, MR_Int32 pMaxObjRay)
{
	BOOL lReturnValue = TRUE;

	// First verify that the requested position is in an allowed range
	MR_3DCoordinate lPosition;

	ApplyRotationMatrix(pPosition, lPosition);

	if((lPosition.mX < mPlanDist - pMaxObjRay) || (lPosition.mX > (MR_ZBUFFER_LIMIT * MR_ZBUFFER_UNIT) + pMaxObjRay)) {
		lReturnValue = FALSE;
	}
	else {
		/*
		   MR_Int32 lXScreen =  MulDiv( -lPosition.mY, mXRes_PlanDist, (lPosition.mX+2*mPlanDist)*mPlanHW*2 )+mXRes/2;
		   MR_Int32 lYScreen = -MulDiv( -lPosition.mZ, mYRes_PlanDist, (lPosition.mX+2*mPlanDist)*mPlanVW*2 )+mYRes/2+mScroll;

		   // Theses next rules are not correct and may failled if the
		   // apperture of the camera is very small
		   if( ( lXScreen <  0     ) ||
		   ( lXScreen >  mXRes ) ||
		   ( lYScreen <  0     ) ||
		   ( lYScreen >  mYRes )    )
		   {
		   // lReturnValue = FALSE;
		   }
		 */
	}

	// Compute the rotation matrix to return
	if(lReturnValue) {
		pMatrix.mDisplacement = pPosition;

		pMatrix.mRotation[0][0] = MR_Cos[pOrientation];
		pMatrix.mRotation[0][1] = -MR_Sin[pOrientation];

		pMatrix.mRotation[1][0] = MR_Sin[pOrientation];
		pMatrix.mRotation[1][1] = MR_Cos[pOrientation];
	}

	return lReturnValue;
}

void MR_3DViewPort::ApplyPositionMatrix(const MR_PositionMatrix & pMatrix, const MR_3DCoordinate & pSrc, MR_3DCoordinate & pDest) const
{
	MR_2DCoordinate lPos;

	lPos.mX = (MR_Int32) Int64ShraMod32(Int32x32To64(pSrc.mX, pMatrix.mRotation[0][0]) + Int32x32To64(pSrc.mY, pMatrix.mRotation[0][1]), MR_TRIGO_SHIFT) + pMatrix.mDisplacement.mX;
	lPos.mY = (MR_Int32) Int64ShraMod32(Int32x32To64(pSrc.mX, pMatrix.mRotation[1][0]) + Int32x32To64(pSrc.mY, pMatrix.mRotation[1][1]), MR_TRIGO_SHIFT) + pMatrix.mDisplacement.mY;

	ApplyRotationMatrix(lPos, pDest);

	pDest.mZ = pSrc.mZ + pMatrix.mDisplacement.mZ - mPosition.mZ;

}

void MR_3DViewPort::ClearZ()
{
	if((mZBuffer == NULL) || (mXRes <= 0) || (mYRes <= 0)) {
		return;
	}

	if(mZLineLen == mXRes) {
		memset(mZBuffer, 0xFF, static_cast<size_t>(mXRes) * static_cast<size_t>(mYRes) * sizeof(MR_UInt16));
	}
	else {
		MR_UInt16 *lZBuffer = mZBuffer;

		for(int lCounter = 0; lCounter < mYRes; lCounter++) {
			memset(lZBuffer, 0xFF, sizeof(MR_UInt16) * mXRes);
			lZBuffer += mZLineLen;
		}
	}
}

void MR_3DViewPort::DrawWFLine(const MR_3DCoordinate & pP0, const MR_3DCoordinate & pP1, MR_UInt8 pColor)
{
	MR_3DCoordinate lP0;
	MR_3DCoordinate lP1;

	// Rotate the coordinates in the camera direction
	//

	ApplyRotationMatrix(pP0, lP0);
	ApplyRotationMatrix(pP1, lP1);

	// Clip the line
	if(lP0.mX < mPlanDist) {
		if(lP1.mX < mPlanDist) {
			return;
		}
		else {
			lP0.mZ = lP0.mZ + MulDiv(lP1.mZ - lP0.mZ, mPlanDist - lP0.mX, lP1.mX - lP0.mX);
			lP0.mY = lP0.mY + MulDiv(lP1.mY - lP0.mY, mPlanDist - lP0.mX, lP1.mX - lP0.mX);
			lP0.mX = mPlanDist;
		}
	}
	else if(lP1.mX < mPlanDist) {
		lP1.mZ = lP1.mZ + MulDiv(lP0.mZ - lP1.mZ, mPlanDist - lP1.mX, lP0.mX - lP1.mX);
		lP1.mY = lP1.mY + MulDiv(lP0.mY - lP1.mY, mPlanDist - lP1.mX, lP0.mX - lP1.mX);
		lP1.mX = mPlanDist;
	}
	// Clip what is behind ZBuffer limit
	if(lP0.mX / MR_ZBUFFER_UNIT > 0xFFFE) {
		if(lP1.mX / MR_ZBUFFER_UNIT > 0xFFFE) {
			return;
		}
		else {
			lP0.mZ = lP0.mZ + MulDiv(lP1.mZ - lP0.mZ, MR_ZBUFFER_UNIT * 0xFFFE - lP0.mX, lP1.mX - lP0.mX);
			lP0.mY = lP0.mY + MulDiv(lP1.mY - lP0.mY, MR_ZBUFFER_UNIT * 0xFFFE - lP0.mX, lP1.mX - lP0.mX);
			lP0.mX = MR_ZBUFFER_UNIT * 0xFFFE;
		}
	}
	else if(lP1.mX / MR_ZBUFFER_UNIT > 0xFFFE) {
		lP1.mZ = lP1.mZ + MulDiv(lP0.mZ - lP1.mZ, MR_ZBUFFER_UNIT * 0xFFFE - lP1.mX, lP0.mX - lP1.mX);
		lP1.mY = lP1.mY + MulDiv(lP0.mY - lP1.mY, MR_ZBUFFER_UNIT * 0xFFFE - lP1.mX, lP0.mX - lP1.mX);
		lP1.mX = MR_ZBUFFER_UNIT * 0xFFFE;
	}

	// Add perpective to Ys and Zs
	int lX0 = MulDiv(-lP0.mY, mXRes * mPlanDist, lP0.mX * mPlanHW * 2);
	int lY0 = MulDiv(lP0.mZ, mYRes * mPlanDist, lP0.mX * mPlanVW * 2);
	int lX1 = MulDiv(-lP1.mY, mXRes * mPlanDist, lP1.mX * mPlanHW * 2);
	int lY1 = MulDiv(lP1.mZ, mYRes * mPlanDist, lP1.mX * mPlanVW * 2);

	DrawLine(lX0 + mXRes / 2, -lY0 + mYRes / 2 + mScroll, lX1 + mXRes / 2, -lY1 + mYRes / 2 + mScroll, pColor);

}
