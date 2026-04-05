// GpuSceneRenderer.cpp

#include "stdafx.h"

#include "GpuSceneRenderer.h"

#include "Bitmap.h"
#include "VideoBuffer.h"

MR_GpuSceneRenderer::MR_GpuSceneRenderer(MR_VideoBuffer *pVideoBuffer) :
	mVideoBuffer(pVideoBuffer),
	mEnabled(FALSE),
	mFrameOpen(FALSE)
{
	char buffer[8] = { 0 };
	DWORD len = GetEnvironmentVariableA("HOVERRACE_GPU_SCENE", buffer, sizeof(buffer));
	if((len > 0) && (len < sizeof(buffer)) && (buffer[0] != '0')) {
		mEnabled = TRUE;
	}
}

MR_GpuSceneRenderer::~MR_GpuSceneRenderer()
{
}

BOOL MR_GpuSceneRenderer::IsEnabled() const
{
	return mEnabled;
}

void MR_GpuSceneRenderer::SetEnabled(BOOL pEnabled)
{
	mEnabled = pEnabled;
}

void MR_GpuSceneRenderer::BeginFrame(const RECT &pViewport, const MR_3DCoordinate &pCameraPosition,
	MR_Angle pOrientation, int pScroll, MR_Int32 pPlanDist,
	MR_Int32 pPlanHW, MR_Int32 pPlanVW)
{
	if(!mEnabled) {
		return;
	}

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
	mFrameOpen = FALSE;
}

void MR_GpuSceneRenderer::ResetFrame()
{
	mFrame.Reset();
	mFrameOpen = FALSE;
}

void MR_GpuSceneRenderer::SubmitBackground(const MR_UInt8 *pBitmap)
{
	if(!mEnabled || !mFrameOpen) {
		return;
	}

	mFrame.mBackgroundBitmap = pBitmap;
}

void MR_GpuSceneRenderer::SubmitWall(const MR_3DCoordinate &pUpperLeft,
	const MR_3DCoordinate &pLowerRight, MR_Int32 pLen,
	const MR_Bitmap *pBitmap, const MR_Bitmap *pBitmap2,
	int pSerialLen, int pSerialStart)
{
	if(!mEnabled || !mFrameOpen) {
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
	mFrame.mWalls.push_back(wall);
}

void MR_GpuSceneRenderer::SubmitPatch(const MR_Patch &pPatch,
	const MR_GpuScenePositionMatrix &pMatrix, const MR_Bitmap *pBitmap)
{
	if(!mEnabled || !mFrameOpen) {
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
	if(!mEnabled || !mFrameOpen) {
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
