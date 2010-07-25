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

namespace HoverRace {
namespace ObjFac1 {

static BOOL gLocalInitialized = FALSE;
static MR_PhysicalCollision gEffect;
static MR_ContactEffectList gEffectList;

BitmapSurface::BitmapSurface(const Util::ObjectFromFactoryId & pId) :
	Model::SurfaceElement(pId)
{
	// The task of initialising the data members is done by the superclass
	mBitmap = NULL;
	mBitmap2 = NULL;
	mRotationSpeed = 0;
	mRotationLen = 1;

	if(!gLocalInitialized) {
		gLocalInitialized = TRUE;
		gEffectList.push_back(&gEffect);

		gEffect.mWeight = MR_InertialMoment::eInfiniteWeight;
		gEffect.mXSpeed = 0;
		gEffect.mYSpeed = 0;
		gEffect.mZSpeed = 0;
	}
}

BitmapSurface::BitmapSurface(const Util::ObjectFromFactoryId & pId, /*const */ ObjFacTools::ResBitmap * pBitmap)
:Model::SurfaceElement(pId)
{
	// The task of initialising the data members i
	mBitmap = pBitmap;
	mBitmap2 = pBitmap;
	mRotationSpeed = 0;
	mRotationLen = 1;

	if(!gLocalInitialized) {
		gLocalInitialized = TRUE;
		gEffectList.push_back(&gEffect);

		gEffect.mWeight = MR_InertialMoment::eInfiniteWeight;
		gEffect.mXSpeed = 0;
		gEffect.mYSpeed = 0;
		gEffect.mZSpeed = 0;
	}

}

BitmapSurface::BitmapSurface(const Util::ObjectFromFactoryId & pId, ObjFacTools::ResBitmap * pBitmap1, ObjFacTools::ResBitmap * pBitmap2, int pRotationSpeed, int pRotationLen)
:Model::SurfaceElement(pId)
{
	// The task of initialising the data members i
	mBitmap = pBitmap1;
	mBitmap2 = pBitmap2;
	mRotationSpeed = pRotationSpeed;
	mRotationLen = pRotationLen;

	if(!gLocalInitialized) {
		gLocalInitialized = TRUE;
		gEffectList.push_back(&gEffect);

		gEffect.mWeight = MR_InertialMoment::eInfiniteWeight;
		gEffect.mXSpeed = 0;
		gEffect.mYSpeed = 0;
		gEffect.mZSpeed = 0;
	}

}

BitmapSurface::~BitmapSurface()
{
}

void BitmapSurface::RenderWallSurface(VideoServices::Viewport3D * pDest, const MR_3DCoordinate & pUpperLeft, const MR_3DCoordinate & pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime)
{
	if(mBitmap != NULL) {
		if(mRotationSpeed != 0) {
			int lStartPos = (pTime + 40000) / mRotationSpeed;

			if(lStartPos < 0) {
				lStartPos = mRotationLen - 1 - ((-lStartPos) % mRotationLen);
			}
			else {
				lStartPos = (lStartPos) % mRotationLen;
			}
			pDest->RenderAlternateWallSurface(pUpperLeft, pLowerRight, pLen, mBitmap, mBitmap2, mRotationLen, lStartPos);

		}
		else {
			pDest->RenderWallSurface(pUpperLeft, pLowerRight, pLen, mBitmap);
		}
	}
}

void BitmapSurface::RenderHorizontalSurface(VideoServices::Viewport3D * pDest, int pNbVertex, const MR_2DCoordinate * pVertexList, MR_Int32 pLevel, BOOL pTop, MR_SimulationTime /*pTime */ )
{
	if(mBitmap != NULL) {
		pDest->RenderHorizontalSurface(pNbVertex, pVertexList, pLevel, pTop, mBitmap);
	}
}

const MR_ContactEffectList *BitmapSurface::GetEffectList()
{
	return &gEffectList;
}

// VStretchBitmapSurface:public

VStretchBitmapSurface::VStretchBitmapSurface(const Util::ObjectFromFactoryId & pId, /*const */ ObjFacTools::ResBitmap * pBitmap, int pMaxHeight)
:BitmapSurface(pId, pBitmap)
{
	mMaxHeight = pMaxHeight;
}

VStretchBitmapSurface::VStretchBitmapSurface(const Util::ObjectFromFactoryId & pId, ObjFacTools::ResBitmap * pBitmap1, ObjFacTools::ResBitmap * pBitmap2, int pRotationSpeed, int pRotationLen, int pMaxHeight)
:BitmapSurface(pId, pBitmap1, pBitmap2, pRotationSpeed, pRotationLen)
{
	mMaxHeight = pMaxHeight;
}

void VStretchBitmapSurface::RenderWallSurface(VideoServices::Viewport3D * pDest, const MR_3DCoordinate & pUpperLeft, const MR_3DCoordinate & pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime)
{
	if(mBitmap != NULL) {
		int lHeight = pUpperLeft.mZ - pLowerRight.mZ;

		if(lHeight > 0) {
			int lDivisor = 1 + (lHeight - 1) / mMaxHeight;

			if(lDivisor > 1) {
				lHeight = lHeight / lDivisor;
			}

			mBitmap->SetWidthHeight(lHeight, lHeight);

			BitmapSurface::RenderWallSurface(pDest, pUpperLeft, pLowerRight, pLen, pTime);
		}
	}
}

}  // namespace Model
}  // namespace HoverRace
