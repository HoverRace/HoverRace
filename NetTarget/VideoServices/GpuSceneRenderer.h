// GpuSceneRenderer.h
//
// Hybrid GPU scene submission scaffold for the OpenGL renderer.

#ifndef MR_GPU_SCENE_RENDERER_H
#define MR_GPU_SCENE_RENDERER_H

#include <vector>

#include "../Util/MR_Types.h"
#include "Patch.h"

class MR_Bitmap;
class MR_VideoBuffer;

enum
{
	MR_GPU_SCENE_MAX_POLYGON_VERTEX = 16
};

struct MR_GpuScenePositionMatrix
{
	MR_Int32 mRotation[2][2];
	MR_3DCoordinate mDisplacement;
};

struct MR_GpuSceneWall
{
	MR_3DCoordinate mUpperLeft;
	MR_3DCoordinate mLowerRight;
	MR_Int32 mLen;
	const MR_Bitmap *mPrimaryBitmap;
	const MR_Bitmap *mAlternateBitmap;
	int mSerialLen;
	int mSerialStart;
	// Snapshot of bitmap dimensions at submission time.
	// MR_VStretchBitmapSurface mutates these per-wall via SetWidthHeight(),
	// so they may change before GPU rendering runs.
	int mPrimaryBitmapWidth;
	int mPrimaryBitmapHeight;
	int mAlternateBitmapWidth;
	int mAlternateBitmapHeight;
};

struct MR_GpuSceneHorizontalSurface
{
	int mNbVertex;
	MR_2DCoordinate mVertexList[MR_GPU_SCENE_MAX_POLYGON_VERTEX];
	MR_Int32 mLevel;
	BOOL mTop;
	const MR_Bitmap *mBitmap;
};

struct MR_GpuScenePatchBitmap
{
	const MR_Patch *mPatch;
	MR_GpuScenePositionMatrix mMatrix;
	const MR_Bitmap *mBitmap;
};

struct MR_GpuScenePatchColor
{
	const MR_Patch *mPatch;
	MR_GpuScenePositionMatrix mMatrix;
	MR_UInt8 mColor;
};

struct MR_GpuSceneFrame
{
	RECT mViewport;
	MR_3DCoordinate mCameraPosition;
	MR_Angle mOrientation;
	int mScroll;
	MR_Int32 mPlanDist;
	MR_Int32 mPlanHW;
	MR_Int32 mPlanVW;
	const MR_UInt8 *mBackgroundBitmap;
	std::vector<MR_GpuSceneWall> mWalls;
	std::vector<MR_GpuSceneHorizontalSurface> mHorizontalSurfaces;
	std::vector<MR_GpuScenePatchBitmap> mBitmapPatches;
	std::vector<MR_GpuScenePatchColor> mColorPatches;

	MR_GpuSceneFrame()
	{
		SetRectEmpty(&mViewport);
		mOrientation = 0;
		mScroll = 0;
		mPlanDist = 0;
		mPlanHW = 0;
		mPlanVW = 0;
		mBackgroundBitmap = NULL;
		memset(&mCameraPosition, 0, sizeof(mCameraPosition));
	}

	void Reset()
	{
		SetRectEmpty(&mViewport);
		mOrientation = 0;
		mScroll = 0;
		mPlanDist = 0;
		mPlanHW = 0;
		mPlanVW = 0;
		mBackgroundBitmap = NULL;
		memset(&mCameraPosition, 0, sizeof(mCameraPosition));
		mWalls.clear();
		mHorizontalSurfaces.clear();
		mBitmapPatches.clear();
		mColorPatches.clear();
	}
};

class MR_GpuSceneRenderer
{
	private:
		MR_VideoBuffer *mVideoBuffer;
		BOOL mFrameOpen;
		MR_GpuSceneFrame mFrame;

	public:
		MR_GpuSceneRenderer(MR_VideoBuffer *pVideoBuffer);
		~MR_GpuSceneRenderer();

		void BeginFrame(const RECT &pViewport, const MR_3DCoordinate &pCameraPosition,
			MR_Angle pOrientation, int pScroll, MR_Int32 pPlanDist,
			MR_Int32 pPlanHW, MR_Int32 pPlanVW);
		void EndFrame();
		void ResetFrame();

		void SubmitBackground(const MR_UInt8 *pBitmap);
		void SubmitWall(const MR_3DCoordinate &pUpperLeft, const MR_3DCoordinate &pLowerRight,
			MR_Int32 pLen, const MR_Bitmap *pBitmap, const MR_Bitmap *pBitmap2,
			int pSerialLen, int pSerialStart);
		void SubmitHorizontalSurface(int pNbVertex, const MR_2DCoordinate *pVertexList,
			MR_Int32 pLevel, BOOL pTop, const MR_Bitmap *pBitmap);
		void SubmitPatch(const MR_Patch &pPatch, const MR_GpuScenePositionMatrix &pMatrix,
			const MR_Bitmap *pBitmap);
		void SubmitPatch(const MR_Patch &pPatch, const MR_GpuScenePositionMatrix &pMatrix,
			MR_UInt8 pColor);

		const MR_GpuSceneFrame &GetFrame() const;
};

#endif
