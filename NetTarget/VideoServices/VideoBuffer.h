// VideoBuffer.h // DirectDraw encapsulation module
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

#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H

#include <ddraw.h>

#include "../Util/MR_Types.h"

#ifdef MR_VIDEO_SERVICES
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

struct MR_OpenGLState;
class MR_GpuSceneRenderer;
class MR_Bitmap;

class MR_VideoBuffer
{

	private:
		HWND mWindow;
		BOOL mFullScreen;
		HINSTANCE directDrawInst;
		LPDIRECTDRAW mDirectDraw;
		LPDIRECTDRAWSURFACE mFrontBuffer;
		LPDIRECTDRAWSURFACE mBackBuffer;
		LPDIRECTDRAWPALETTE mPalette;
		LPDIRECTDRAWCLIPPER mClipper;			  // To use in windows only
		// remove if too slow

		BOOL mSpecialWindowMode;				  // Use a 256 color mode when switching to window mode
		int mSpecialModeXRes;
		int mSpecialModeYRes;
		RECT mFullscreenRect;
		BOOL mRequestedAdapterGuidValid;
		GUID mRequestedAdapterGuid;
		BOOL mCurrentAdapterGuidValid;
		GUID mCurrentAdapterGuid;
		BOOL mUseGdiWindowedPresentFallback;
		int mWindowedInvalidRectStreak;
		int mWindowedPresentFailureStreak;
		HRESULT mWindowedPresentFailureHr;

		LONG mOriginalExStyle;					  // Only valid if mFullScreen
		LONG mOriginalStyle;					  // Only valid if mFullScreen
		RECT mOriginalPos;						  // Only valid if mFullScreen

		BOOL mModeSettingInProgress;

		int mX0;								  // used only when
		int mY0;								  // displaying in a window
		int mDisplayXRes;
		int mDisplayYRes;
		int mXRes;
		int mYRes;
		int mLineLen;
		int mRenderScalePercent;

		DWORD mBpp;								  // current bits per pixel
		DWORD mNativeBpp;						  // native (desktop) bits per pixel
		struct Channel
		{
			DWORD mShift;
			DWORD mSize;
			Channel():mShift(0), mSize(0) {
			}
			void SetMask(DWORD mask);
			DWORD GetMask() const;
			DWORD Pack(DWORD intensity) const;
		};
		Channel mRChan, mGChan, mBChan;
		DWORD *mPackedPalette;

		MR_UInt16 *mZBuffer;
		MR_UInt8 *mBuffer;
		MR_UInt8 *mRenderSurface;
		MR_UInt8 *mPaletteTexture;
		BOOL mPaletteDirty;
		MR_OpenGLState *mOpenGLState;
		MR_GpuSceneRenderer *mGpuSceneRenderer;
		MR_UInt8 mGpuClearColorIndex;
		int mOpenGLPresentFailureLogCount;
		int mOpenGLFrameTraceLogCount;
		DWORD mStagePerfAccumulatedClearMs;
		DWORD mStagePerfAccumulatedBackgroundMs;
		DWORD mStagePerfAccumulatedClearZMs;
		DWORD mStagePerfAccumulatedFloorMs;
		DWORD mStagePerfAccumulatedWallMs;
		DWORD mStagePerfAccumulatedWallSetupMs;
		DWORD mStagePerfAccumulatedWallLoopMs;
		DWORD mStagePerfAccumulatedActorMs;
		DWORD mStagePerfMaxClearMs;
		DWORD mStagePerfMaxBackgroundMs;
		DWORD mStagePerfMaxClearZMs;
		DWORD mStagePerfMaxFloorMs;
		DWORD mStagePerfMaxWallMs;
		DWORD mStagePerfMaxWallSetupMs;
		DWORD mStagePerfMaxWallLoopMs;
		DWORD mStagePerfMaxActorMs;
		int mStagePerfSampleCount;

		MR_UInt8 *mBackPalette;

		// IconMode releted functions
		BOOL mIconMode;
		//      int  mBeforeIconXRes;
		//      int  mBeforeIconYRes;

		// ColorRelated data
		double mGamma;
		double mContrast;
		double mBrightness;

		DWORD PackRGB(DWORD r, DWORD g, DWORD b);

		BOOL PrepareWindowedAdapter();
		void SetRequestedAdapterForMonitor(HMONITOR pMonitor);
		BOOL IsCurrentAdapterRequested() const;
		void ComputeRenderResolution(int pDisplayXRes, int pDisplayYRes, int &pRenderXRes, int &pRenderYRes) const;
		void ResetWindowedPresentFallback();
		BOOL PresentWindowedWithGdi();
		BOOL InitDirectDraw();
		BOOL InitOpenGL();
		void ReleaseOpenGL();
		BOOL EnsureOpenGLResources();
		BOOL PresentOpenGL();
		void RenderGpuSceneOverlay();
		unsigned int GetOrCreateGpuBitmapTexture(const MR_Bitmap *pBitmap, int pSubBitmap,
			int &pWidth, int &pHeight);
		void SetOpenGLColorFromPaletteIndex(MR_UInt8 pColorIndex, unsigned char pAlpha) const;
		BOOL ProcessCurrentBpp(const DDPIXELFORMAT & lFormat);
		void DeleteInternalSurfaces();
		void ReleaseDirectDraw();
		void ReturnToWindowsResolution();		  //Automaticly call DeleteInternalSurfaces

		void Flip();

	public:

		MR_DllDeclare MR_VideoBuffer(HWND pWindow, double lGamma, double lContrast, double mBrightness);
		MR_DllDeclare ~ MR_VideoBuffer();

		// Video mode selection
		MR_DllDeclare BOOL SetVideoMode();		  // In a window mode
												  // Full screen mode
		MR_DllDeclare BOOL SetVideoMode(int pXRes, int pYRes);
		MR_DllDeclare BOOL PrepareDesktopFullscreen(POINT *pResolution);
		MR_DllDeclare BOOL SetMenuVideoMode();	  // slow full screen mode for menus
		MR_DllDeclare void EnterIconMode();
		MR_DllDeclare void ExitIconMode();
		MR_DllDeclare void AssignPalette();

		MR_DllDeclare BOOL TryToSetColorMode(int colorBits);

		MR_DllDeclare BOOL IsWindowMode() const;
		BOOL IsEffectiveFullscreen() const;
		MR_DllDeclare BOOL IsIconMode() const;
		MR_DllDeclare BOOL IsModeSettingInProgress() const;

		MR_DllDeclare void CreatePalette(double pGamma, double pContrast, double pBrightness);
		MR_DllDeclare void GetPaletteAttrib(double &pGamma, double &pContrast, double &pBrightness);
		MR_DllDeclare void SetBackPalette(MR_UInt8 * pPalette);
		MR_DllDeclare MR_GpuSceneRenderer *GetGpuSceneRenderer() const;
		MR_DllDeclare void LogPerformanceSample(DWORD pFrameAvgMs, DWORD pCpuAvgMs, DWORD pPresentAvgMs,
			DWORD pFrameMaxMs, DWORD pCpuMaxMs, DWORD pPresentMaxMs, int pSampleCount);
		MR_DllDeclare void LogRenderStageSample(DWORD pClearMs, DWORD pBackgroundMs, DWORD pClearZMs, DWORD pFloorMs,
			DWORD pWallMs, DWORD pWallSetupMs, DWORD pWallLoopMs, DWORD pActorMs);
		MR_DllDeclare void ClearGpuResourceCache();

		// Buffers manipulation
		MR_DllDeclare BOOL Lock();				  // Must be called before drawing
		MR_DllDeclare void Unlock();

		// Buffer access functions
		MR_DllDeclare int GetXRes() const;
		MR_DllDeclare int GetYRes() const;
		MR_DllDeclare int GetDisplayXRes() const;
		MR_DllDeclare int GetDisplayYRes() const;
		MR_DllDeclare int GetLineLen() const;
		MR_DllDeclare int GetZLineLen() const;
		MR_DllDeclare int GetRenderScalePercent() const;
		MR_DllDeclare void SetRenderScalePercent(int pPercent);

		MR_DllDeclare MR_UInt8 *GetBuffer();
		MR_DllDeclare MR_UInt16 *GetZBuffer();

		MR_DllDeclare void Clear(MR_UInt8 pColor = 0);
		MR_DllDeclare void ClearZ(MR_UInt8 pDepth = -1);

		// geometry access functions
		MR_DllDeclare int GetXPixelMeter() const;
		MR_DllDeclare int GetYPixelMeter() const;

		// 2DDrawing primitives, defined in VideoBuffer2DDraw.cpp
		MR_DllDeclare void DrawPoint(int pX, int pY, MR_UInt8 pColor);
		MR_DllDeclare void DrawLine(int pX0, int pY0, int pX1, int pY1, MR_UInt8 pColor);
		MR_DllDeclare void DrawHorizontalLine(int pY, int pX0, int pX1, MR_UInt8 pColor);

		// 2DRendering primitives, defined in VideoBuffer3DRendering.cpp

};

#undef MR_DllDeclare
#endif
