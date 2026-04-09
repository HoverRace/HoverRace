// GpuSceneRenderer.cpp

#include "stdafx.h"

#include "GpuSceneRenderer.h"

#include "Bitmap.h"
#include "VideoBuffer.h"

MR_GpuSceneRenderer::MR_GpuSceneRenderer(MR_VideoBuffer *pVideoBuffer) :
	mVideoBuffer(pVideoBuffer),
	mFrameOpen(FALSE)
{
}

MR_GpuSceneRenderer::~MR_GpuSceneRenderer()
{
}

void MR_GpuSceneRenderer::BeginFrame(const RECT &pViewport, const MR_3DCoordinate &pCameraPosition,
	MR_Angle pOrientation, int pScroll, MR_Int32 pPlanDist,
	MR_Int32 pPlanHW, MR_Int32 pPlanVW)
{
	mFrame.Reset();
	mFrameOpen = TRUE;
	mFrame.mViewport = pViewport;
	mFrame.mCameraPosition = pCameraPosition;
	mFrame.mOrientation = pOrientation;
	mFrame.mScroll = pScroll;
	mFrame.mPlanDist = pPlanDist;
	mFrame.mPlanHW = pPlanHW;
	mFrame.mPlanVW = pPlanVW;
}

void MR_GpuSceneRenderer::EndFrame()
{
	if(mFrameOpen) {
		mFrames.push_back(mFrame);
	}
	mFrameOpen = FALSE;
}

void MR_GpuSceneRenderer::ResetFrame()
{
	mFrame.Reset();
	mFrameOpen = FALSE;
}

void MR_GpuSceneRenderer::ClearAllFrames()
{
	mFrames.clear();
	mFrame.Reset();
	mFrameOpen = FALSE;
}

void MR_GpuSceneRenderer::SubmitBackground(const MR_UInt8 *pBitmap)
{
	if(!mFrameOpen) {
		return;
	}

	mFrame.mBackgroundBitmap = pBitmap;
}

void MR_GpuSceneRenderer::SubmitWall(const MR_3DCoordinate &pUpperLeft,
	const MR_3DCoordinate &pLowerRight, MR_Int32 pLen,
	const MR_Bitmap *pBitmap, const MR_Bitmap *pBitmap2,
	int pSerialLen, int pSerialStart)
{
	if(!mFrameOpen) {
		return;
	}

	MR_GpuSceneWall wall;
	wall.mUpperLeft = pUpperLeft;
	wall.mLowerRight = pLowerRight;
	wall.mLen = pLen;
	wall.mPrimaryBitmap = pBitmap;
	wall.mAlternateBitmap = pBitmap2;
	wall.mSerialLen = pSerialLen;
	wall.mSerialStart = pSerialStart;
	wall.mPrimaryBitmapWidth = pBitmap ? pBitmap->GetWidth() : 0;
	wall.mPrimaryBitmapHeight = pBitmap ? pBitmap->GetHeight() : 0;
	wall.mAlternateBitmapWidth = pBitmap2 ? pBitmap2->GetWidth() : 0;
	wall.mAlternateBitmapHeight = pBitmap2 ? pBitmap2->GetHeight() : 0;
	mFrame.mWalls.push_back(wall);
}

void MR_GpuSceneRenderer::SubmitHorizontalSurface(int pNbVertex,
	const MR_2DCoordinate *pVertexList, MR_Int32 pLevel, BOOL pTop,
	const MR_Bitmap *pBitmap)
{
	if(!mFrameOpen) {
		return;
	}

	if((pNbVertex < 3) || (pNbVertex > MR_GPU_SCENE_MAX_POLYGON_VERTEX)
		|| (pVertexList == NULL)) {
		return;
	}

	MR_GpuSceneHorizontalSurface surface;
	surface.mNbVertex = pNbVertex;
	for(int i = 0; i < pNbVertex; i++) {
		surface.mVertexList[i] = pVertexList[i];
	}
	surface.mLevel = pLevel;
	surface.mTop = pTop;
	surface.mBitmap = pBitmap;
	mFrame.mHorizontalSurfaces.push_back(surface);
}

void MR_GpuSceneRenderer::SubmitPatch(const MR_Patch &pPatch,
	const MR_GpuScenePositionMatrix &pMatrix, const MR_Bitmap *pBitmap)
{
	if(!mFrameOpen) {
		return;
	}

	MR_GpuScenePatchBitmap patch;
	patch.mPatch = &pPatch;
	patch.mMatrix = pMatrix;
	patch.mBitmap = pBitmap;
	mFrame.mBitmapPatches.push_back(patch);
}

void MR_GpuSceneRenderer::SubmitPatch(const MR_Patch &pPatch,
	const MR_GpuScenePositionMatrix &pMatrix, MR_UInt8 pColor)
{
	if(!mFrameOpen) {
		return;
	}

	MR_GpuScenePatchColor patch;
	patch.mPatch = &pPatch;
	patch.mMatrix = pMatrix;
	patch.mColor = pColor;
	mFrame.mColorPatches.push_back(patch);
}

const MR_GpuSceneFrame &MR_GpuSceneRenderer::GetFrame() const
{
	return mFrame;
}

const std::vector<MR_GpuSceneFrame> &MR_GpuSceneRenderer::GetFrames() const
{
	return mFrames;
}

size_t MR_GpuSceneRenderer::GetFrameCount() const
{
	return mFrames.size();
}
