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

#include "Controller.h"
#include "Observer.h"
#include "ClientSession.h"

class MR_GameApp;
class HighObserver;
namespace HoverRace {
	namespace Client {
		class FullscreenTest;
		class GamePeer;
		class HighConsole;
		class IntroMovie;
		class Rulebook;
		typedef boost::shared_ptr<Rulebook> RulebookPtr;
		class SessionPeer;
		typedef boost::shared_ptr<SessionPeer> SessionPeerPtr;
		class SysEnv;
	}
	namespace Script {
		class Core;
	}
}

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
		HACCEL mAccelerators;
		MR_VideoBuffer *mVideoBuffer;
		MR_Observer *mObserver1;
		MR_Observer *mObserver2;
		MR_Observer *mObserver3;
		MR_Observer *mObserver4;
		HighObserver *highObserver;
		HoverRace::Client::HighConsole *highConsole;
		HoverRace::Client::IntroMovie *introMovie;
		HoverRace::Client::FullscreenTest *fullscreenTest;
		HoverRace::Script::Core *scripting;
		HoverRace::Client::GamePeer *gamePeer;
		HoverRace::Client::SysEnv *sysEnv;
		HoverRace::Client::ClientSession *mCurrentSession;
		HoverRace::Client::SessionPeerPtr sessionPeer;
		MR_GameThread *mGameThread;

		eViewMode mCurrentMode;

		int mClrScrTodo;

		BOOL mPaletteChangeAllowed;

		BOOL mServerHasChanged;

		// Debug data
		time_t mNbFramesStartingTime;
		int mNbFrames;

		// we only want to check for updates once an instance so we have to keep track
		bool mustCheckUpdates;

		static LRESULT CALLBACK DispatchFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK BadModeDialogFunc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);

		BOOL CreateMainWindow();
		bool CreateMainMenu();

		void RefreshView();
		bool SetVideoMode(int pX, int pY, const std::string *monitor=NULL,
			bool testing=false);
		void RefreshTitleBar();

		void OnChar(char c);
		int ReadAsyncInputControllerPlayer(int playerIdx);
		void ReadAsyncInputController();		  // Get the state of the input controler (KDB, joystick, mouse)

		// Message handlers
	public:
		void NewLocalSession(HoverRace::Client::RulebookPtr rules=HoverRace::Client::RulebookPtr());
		void NewSplitSession(int pSplitPlayers);
		void NewNetworkSession(BOOL pIsServer);
		void NewInternetSession();

	private:
		void LoadRegistry();

		void Clean();

		void OnDisplayChange();
	public:
		void AssignPalette();

	private:
		void PauseGameThread();
		void RestartGameThread();

		void DeleteMovieWnd();

		void DrawBackground();

		void UpdateMenuItems();

		void DisplayHelp();
		void DisplaySite();
		void DisplayAbout();
		void DisplayPrefs();

		BOOL IsGameRunning();					  // return TRUE if a not terminated game is running
		int AskUserToAbortGame();				  // Return IDOK if OK

		HoverRace::Client::Control::Controller *controller;

	public:
		MR_GameApp(HINSTANCE pInstance, bool safeMode);
		~MR_GameApp();

		BOOL IsFirstInstance() const;

		BOOL InitApplication();
		BOOL InitGame();

		int MainLoop();

		/// Signal that the selected IMR server has (possibly) changed.
		void SignalServerHasChanged() { mServerHasChanged = TRUE; }
		void ChangeAutoUpdates(bool newSetting) { mustCheckUpdates = newSetting; }

		MR_VideoBuffer *GetVideoBuffer() const { return mVideoBuffer; }

		HoverRace::Client::Control::Controller *GetController() const { return controller; }
		HoverRace::Client::Control::Controller *ReloadController();

		HWND GetWindowHandle() const { return mMainWindow; }

		// Helper stuff
		static void NewInternetSessionCall();
};

#undef MR_DllDeclare
#endif