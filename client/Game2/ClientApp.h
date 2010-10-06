
// ClientApp.h
// Experimental game shell.
//
// Copyright (c) 2010 Michael Imamura.
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

#pragma once

#include <SDL/SDL.h>

#include "../../engine/Util/OS.h"

#include "Observer.h"
#include "GameDirector.h"

#ifdef WITH_SDL

namespace HoverRace {
	namespace Client {
		namespace Control {
			class Controller;
		}
		namespace HoverScript {
			class GamePeer;
			class HighConsole;
			class SessionPeer;
			typedef boost::shared_ptr<SessionPeer> SessionPeerPtr;
			class SysEnv;
		}
		class HighObserver;
		class Rulebook;
		typedef boost::shared_ptr<Rulebook> RulebookPtr;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {

class ClientApp : public GameDirector
{
	typedef GameDirector SUPER;

	public:
		ClientApp();
		virtual ~ClientApp();

	private:
		void RefreshTitleBar();
		void DrawPalette();

	public:
		void MainLoop();

		void NewLocalSession(RulebookPtr rules=RulebookPtr());

	public:
		// GameDirector
		virtual void RequestShutdown();
		virtual void SignalServerHasChanged();
		virtual void ChangeAutoUpdates(bool newSetting);
		virtual void AssignPalette();
		virtual VideoServices::VideoBuffer *GetVideoBuffer() const { return videoBuf; }
		virtual Control::Controller *GetController() const { return controller; }
		virtual Control::Controller *ReloadController();
#	ifdef _WIN32
		virtual HWND GetWindowHandle() const { return mainWnd; }
#	endif

	private:
		Util::OS::wnd_t mainWnd;
		VideoServices::VideoBuffer *videoBuf;
		class UiInput;
		boost::shared_ptr<UiInput> uiInput;
		Control::Controller *controller;

		static const int MAX_OBSERVERS = HoverRace::Util::Config::MAX_PLAYERS;
		Observer *observers[MAX_OBSERVERS];
		ClientSession *currentSession;

		HighObserver *highObserver;
		HoverScript::HighConsole *highConsole;

		Script::Core *scripting;
		HoverScript::GamePeer *gamePeer;
		HoverScript::SessionPeerPtr sessionPeer;
		HoverScript::SysEnv *sysEnv;

		RulebookPtr requestedNewSession;
};

}  // namespace HoverScript
}  // namespace Client

#endif  // ifdef WITH_SDL
