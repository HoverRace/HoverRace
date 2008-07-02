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

class MR_GameApp;

class MR_GameThread
{
	private:
		MR_GameApp * mGameApp;
		CRITICAL_SECTION mMutex;
		HANDLE mThread;
		BOOL mTerminate;
		int mPauseLevel;

		static unsigned long __stdcall Loop(LPVOID pThread);

		MR_GameThread(MR_GameApp * pApp);
		~MR_GameThread();

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

		HINSTANCE mInstance;
		HWND mMainWindow;
		HWND mBadVideoModeDlg;
		HWND mMovieWnd;
		HACCEL mAccelerators;
		MR_VideoBuffer *mVideoBuffer;
		MR_Observer *mObserver1;
		MR_Observer *mObserver2;
		MR_Observer *mObserver3;
		MR_Observer *mObserver4;
		MR_ClientSession *mCurrentSession;
		MR_GameThread *mGameThread;

		eViewMode mCurrentMode;
		// int                      mMenuStack;

		int mClrScrTodo;

		BOOL mPaletteChangeAllowed;

		// Keyboard configuration
		int mMotorOn1;
		int mRight1;
		int mLeft1;
		int mJump1;
		int mFire1;
		int mBreak1;
		int mWeapon1;
		int mLookBack1;

		int mMotorOn2;
		int mRight2;
		int mLeft2;
		int mJump2;
		int mFire2;
		int mBreak2;
		int mWeapon2;
		int mLookBack2;

		int mMotorOn3;
		int mRight3;
		int mLeft3;
		int mJump3;
		int mFire3;
		int mBreak3;
		int mWeapon3;
		int mLookBack3;

		int mMotorOn4;
		int mRight4;
		int mLeft4;
		int mJump4;
		int mFire4;
		int mBreak4;
		int mWeapon4;
		int mLookBack4;

		double mGamma;
		double mContrast;
		double mBrightness;

		CString mNickName;

		CString mOwner;
		CString mCompany;
		int mMajorID;
		int mMinorID;
		// CString mKey1;
		// CString mKey2;

		CString mMainServer;
		BOOL mServerHasChanged;

		BOOL mDisplayFirstScreen;
		BOOL mIntroMovie;
		BOOL mNativeBppFullscreen;

		// Debug data
		time_t mNbFramesStartingTime;
		int mNbFrames;

		void SetProperties();
		static LRESULT CALLBACK DispatchFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK DisplayIntensityDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK ControlDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK MiscDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK BadModeDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK MovieDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK AboutDlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK NoticeDlgFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK LoginFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK LoginPasswdFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK RegistrationPasswdFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK RegistrationFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK FirstChoiceDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);

		BOOL CreateMainWindow();

		void RefreshView();
		void SetVideoMode(int pX, int pY);
		void RefreshTitleBar();

		void ReadAsyncInputController();		  // Get the state of the input controler (KDB, joystick, mouse)

		// Message handlers
		void NewLocalSession();
		void NewSplitSession(int pSplitPlayers);
		void NewNetworkSession(BOOL pIsServer);
		void NewInternetSession();

		void LoadRegistry();
		void SaveRegistry();

		void Clean();

		void OnDisplayChange();
		void AssignPalette();
		BOOL GetDesktopResolution(POINT* lpPoint);

		void PauseGameThread();
		void RestartGameThread();

		// void EnterMenuLoop();
		// void EndMenuLoop();

		void DeleteMovieWnd();

		void DrawBackground();

		void SwitchToDesktopFullscreen();

		void UpdateMenuItems();

		void DisplayHelp();
		void DisplayBetaZone();
		void DisplaySite();
		void DisplayRegistrationInfo(HWND pWindow);
		void DisplayRegistrationSite();
		void DisplayAbout();

		BOOL IsGameRunning();					  // return TRUE if a not terminated game is running
		int AskUserToAbortGame();				  // Return IDOK if OK

	public:
		MR_GameApp(HINSTANCE pInstance);
		~MR_GameApp();

		BOOL IsFirstInstance() const;

		BOOL DisplayNotice();
		BOOL DisplayLoginWindow();
		BOOL InitApplication();
		BOOL InitGame();

		int MainLoop();

		// Helper stuff
		static void NewInternetSessionCall();
};

#undef MR_DllDeclare
#endif
