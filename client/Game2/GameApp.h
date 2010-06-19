// GameApp.h
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

#pragma once

#include "../../engine/Util/Config.h"

#include "Observer.h"
#include "ClientSession.h"

namespace HoverRace {
	namespace Client {
		namespace Control {
			class Controller;
			class UiHandler;
			typedef boost::shared_ptr<UiHandler> UiHandlerPtr;
		}
		namespace HoverScript {
			class GamePeer;
			class HighConsole;
			class SessionPeer;
			typedef boost::shared_ptr<SessionPeer> SessionPeerPtr;
			class SysEnv;
		}
		class FullscreenTest;
		class GameApp;
		class HighObserver;
		class IntroMovie;
		class Rulebook;
		typedef boost::shared_ptr<Rulebook> RulebookPtr;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {

class GameThread
{
	private:
		GameApp *mGameApp;
		CRITICAL_SECTION mMutex;
		HANDLE mThread;
		BOOL mTerminate;
		int mPauseLevel;

		static unsigned long __stdcall Loop(LPVOID pThread);

		GameThread(GameApp * pApp);
		~GameThread();

	public:
		static GameThread *New(GameApp * pApp);
		void Kill();
		void Pause();
		void Restart();
};

class GameApp
{
	friend GameThread;

	private:
		enum eViewMode { e3DView, eDebugView };

		static GameApp *This;				  // unique instance pointer

		HINSTANCE mInstance;
		HWND mMainWindow;
		HWND mBadVideoModeDlg;
		HACCEL mAccelerators;
		MR_VideoBuffer *mVideoBuffer;
		static const int MAX_OBSERVERS = HoverRace::Util::Config::MAX_PLAYERS;
		MR_Observer *observers[MAX_OBSERVERS];
		bool nonInteractiveShutdown;
		HighObserver *highObserver;
		HoverScript::HighConsole *highConsole;
		IntroMovie *introMovie;
		FullscreenTest *fullscreenTest;
		Script::Core *scripting;
		HoverScript::GamePeer *gamePeer;
		HoverScript::SysEnv *sysEnv;
		ClientSession *mCurrentSession;
		HoverScript::SessionPeerPtr sessionPeer;
		GameThread *mGameThread;
		RulebookPtr requestedNewSession;
		class UiInput;
		boost::shared_ptr<UiInput> uiInput;

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
		bool OnKeyDown(int keycode);
		int ReadAsyncInputControllerPlayer(int playerIdx);
		void ReadAsyncInputController();		  // Get the state of the input controler (KDB, joystick, mouse)

		// Message handlers
	public:
		void NewLocalSession(RulebookPtr rules=RulebookPtr());
		void NewSplitSession(int pSplitPlayers);
		void NewNetworkSession(BOOL pIsServer);
		void NewInternetSession();

		void RequestShutdown();

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

		static void TrackOpenFailMessageBox(HWND parent, const std::string &name, const std::string &details);

		Control::Controller *controller;

	public:
		GameApp(HINSTANCE pInstance, bool safeMode);
		~GameApp();

		BOOL IsFirstInstance() const;

		BOOL InitApplication();
		BOOL InitGame();

		int MainLoop();

		/// Signal that the selected IMR server has (possibly) changed.
		void SignalServerHasChanged() { mServerHasChanged = TRUE; }
		void ChangeAutoUpdates(bool newSetting) { mustCheckUpdates = newSetting; }

		MR_VideoBuffer *GetVideoBuffer() const { return mVideoBuffer; }

		Control::Controller *GetController() const { return controller; }
		Control::Controller *ReloadController();

		HWND GetWindowHandle() const { return mMainWindow; }

		// Helper stuff
		static void NewInternetSessionCall();
};

}  // namespace Client
}  // namespace HoverRace
