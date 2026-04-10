// MR_GameApp.h
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
#ifndef GAME_APP_H
#define GAME_APP_H

#include "Observer.h"
#include "ClientSession.h"
#include "NetInterface.h"

class ISteamUser;

class MR_GameApp;

class MR_GameThread
{
	private:
		MR_GameApp *mGameApp;
		CRITICAL_SECTION mMutex;
		HANDLE mThread;
		BOOL mTerminate;
		int mPauseLevel;

		static unsigned long __stdcall Loop(LPVOID pThread);

		MR_GameThread(MR_GameApp * pApp);
		~MR_GameThread();

	protected:		
		MR_NetworkInterface mNetInterface;

	public:
		static MR_GameThread *New(MR_GameApp * pApp);
		void Kill();
		void Pause();
		void Restart();
};

class MR_GameApp
{
	friend MR_GameThread;

	private:
		enum eViewMode { e3DView, eDebugView };

		static MR_GameApp *This;				  // unique instance pointer
		enum { ADAPTIVE_RENDER_SCALE_CACHE_SIZE = 8 };

		HINSTANCE mInstance;
		HWND mMainWindow;
		HWND mBadVideoModeDlg;
		HWND mMovieWnd;
		HMENU mWindowedMenu;
		char mDesktopFullscreenDevice[CCHDEVICENAME];
		HACCEL mAccelerators;
		MR_VideoBuffer *mVideoBuffer;
		MR_Observer *mObserver1;
		MR_Observer *mObserver2;
		MR_Observer *mObserver3;
		MR_Observer *mObserver4;
		MR_ClientSession *mCurrentSession;
		MR_GameThread *mGameThread;
		CSteamID mSteamIDLocalUser;

		eViewMode mCurrentMode;

		bool safeMode;
		bool allowMultipleInstances;
		bool mDesktopFullscreen;
		bool mInResizeLoop;
		RECT mWindowedRect;
		LONG mWindowedStyle;
		LONG mWindowedExStyle;
		int mAdaptiveRenderScalePercent;
		int mAppliedRenderScalePercent;
		int mAdaptiveRenderScaleMaxPercent;
		int mAdaptiveRenderScaleUpBasePercent;
		int mAdaptiveRenderScaleDownChainCount;
		DWORD mAdaptiveRenderScaleLastChangeTick;
		DWORD mAdaptiveRenderScaleLastIncreaseTick;
		DWORD mAdaptiveRenderScaleAccumulatedMs;
		int mAdaptiveRenderScaleSampleCount;
		int mAdaptiveRenderScaleGoodSampleCount;
		int mAdaptiveRenderScaleCacheWidth[ADAPTIVE_RENDER_SCALE_CACHE_SIZE];
		int mAdaptiveRenderScaleCacheHeight[ADAPTIVE_RENDER_SCALE_CACHE_SIZE];
		int mAdaptiveRenderScaleCachePercent[ADAPTIVE_RENDER_SCALE_CACHE_SIZE];
		int mAdaptiveRenderScaleCacheNext;
		DWORD mRenderPerfAccumulatedFrameMs;
		DWORD mRenderPerfAccumulatedCpuMs;
		DWORD mRenderPerfAccumulatedPresentMs;
		DWORD mRenderPerfMaxFrameMs;
		DWORD mRenderPerfMaxCpuMs;
		DWORD mRenderPerfMaxPresentMs;
		int mRenderPerfSampleCount;

		int mClrScrTodo;
		DWORD mLastMouseMoveTick;
		BOOL mCursorVisible;

		BOOL mPaletteChangeAllowed;

		BOOL mServerHasChanged;

		// Debug data
		time_t mNbFramesStartingTime;
		int mNbFrames;

		void SetProperties();
		static LRESULT CALLBACK DispatchFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK DisplayIntensityDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK ControlDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK MiscDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK BadModeDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK AboutDlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK FirstChoiceDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);

		static void UpdateIntensityDialogLabels(HWND pWindow);

		BOOL CreateMainWindow();

		void RefreshView();
		void SetVideoMode(int pX, int pY);
		void RefreshTitleBar();
		void ResetAdaptiveRenderScale();
		void ResetAdaptiveRenderScaleForResize(int pPercent = 100);
		BOOL ShouldUseAdaptiveRenderScale() const;
		void RequestAdaptiveRenderScale(int pPercent);
		void ApplyAdaptiveRenderScale(int pPercent);
		void UpdateAdaptiveRenderScale(DWORD pFrameMs);
		void AccumulateRenderPerformanceSample(DWORD pFrameMs, DWORD pCpuMs, DWORD pPresentMs);
		void RememberAdaptiveRenderScaleForSize(int pDisplayXRes, int pDisplayYRes, int pPercent);
		int GetCachedAdaptiveRenderScaleForSize(int pDisplayXRes, int pDisplayYRes) const;

		int ReadAsyncInputControllerPlayer(int playerIdx);
		void ReadAsyncInputController();		  // Get the state of the input controler (KDB, joystick, mouse)

		// Message handlers
		void NewLocalSession();
		void NewSplitSession(int pSplitPlayers);
		void NewNetworkSession(BOOL pIsServer);
		void NewInternetSession();

		void ProcessCmdLine(int argc, char **argv);
		void LoadRegistry();
		void SaveRegistry();

		void Clean();

		void OnDisplayChange();
		void AssignPalette();
		BOOL GetDesktopResolution(POINT* lpPoint);

		void PauseGameThread();
		void RestartGameThread();

		void DeleteMovieWnd();

		void DrawBackground();
		BOOL GetDesktopFullscreenRect(RECT *pRect);
		BOOL GetActiveDesktopFullscreenRect(RECT *pRect);
		BOOL GetPrimaryMonitorRect(RECT *pRect, char *deviceName = NULL);
		BOOL GetWindowMonitorRect(const RECT &windowRect, RECT *pRect, char *deviceName = NULL);
		void ClampWindowRectToMonitor(RECT *pRect, const RECT &monitorRect);
		void ResolveInitialWindowRect(RECT *pRect);
		void ApplyDesktopFullscreenRect(const RECT &rect);
		void NormalizeWindowedRect(const RECT &monitorRect);
		void EnterDesktopFullscreen();
		void ExitDesktopFullscreen();
		void RefreshDesktopFullscreenPlacement();

		void SwitchToDesktopFullscreen();

		void UpdateMenuItems();

		void DisplayHelp();
		void DisplayBetaZone();
		void DisplaySite();
		void DisplayAbout();

		BOOL IsGameRunning();					  // return TRUE if a not terminated game is running
		int AskUserToAbortGame();				  // Return IDOK if OK

	public:
		MR_GameApp(HINSTANCE pInstance);
		~MR_GameApp();

		BOOL IsFirstInstance() const;

		BOOL InitApplication();
		BOOL InitGame();

		int MainLoop();

		// Helper stuff
		static void NewInternetSessionCall();
};

#undef MR_DllDeclare
#endif
