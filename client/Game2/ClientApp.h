
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

#include "../../engine/Util/OS.h"

#include "Observer.h"
#include "GameDirector.h"

namespace HoverRace {
	namespace Client {
		namespace Control {
			class InputEventController;
		}
		namespace HoverScript {
			class GamePeer;
			class HighConsole;
			class SessionPeer;
			typedef std::shared_ptr<SessionPeer> SessionPeerPtr;
			class SysEnv;
		}
		class HighObserver;
		class Rulebook;
		typedef std::shared_ptr<Rulebook> RulebookPtr;
		class Scene;
		typedef std::shared_ptr<Scene> ScenePtr;
	}
	namespace Display {
		class Display;
		class Label;
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
		void OnWindowResize(int w, int h);
		void IncFrameCount();
		void AdvanceScenes(Util::OS::timestamp_t tick);
		void RenderScenes();

	public:
		void MainLoop();

		void NewLocalSession(RulebookPtr rules=RulebookPtr());

	private:
		typedef std::list<ScenePtr> sceneStack_t;
		void SetForegroundScene();
		void SetForegroundScene(const sceneStack_t::reverse_iterator &iter);
		void PushScene(const ScenePtr &scene);
		void PopScene();
		void ReplaceScene(const ScenePtr &scene);
		void TerminateAllScenes();

	public:
		// GameDirector
		virtual void RequestPushScene(const ScenePtr &scene);
		virtual void RequestPopScene();
		virtual void RequestReplaceScene(const ScenePtr &scene);
		virtual void RequestShutdown();
		virtual void SignalServerHasChanged();
		virtual void ChangeAutoUpdates(bool newSetting);
		virtual void AssignPalette();
		virtual VideoServices::VideoBuffer *GetVideoBuffer() const;
		virtual Control::InputEventController *GetController() const { return controller; }
		virtual Control::InputEventController *ReloadController();
#	ifdef _WIN32
		virtual HWND GetWindowHandle() const { return mainWnd; }
#	endif

	private:
		Util::OS::wnd_t mainWnd;
		Display::Display *display;
		class UiInput;
		std::shared_ptr<UiInput> uiInput;
		Control::InputEventController *controller;

		sceneStack_t sceneStack;
		sceneStack_t::reverse_iterator fgScene;  ///< The scene that currently has input focus.

		Script::Core *scripting;
		HoverScript::GamePeer *gamePeer;
		HoverScript::SysEnv *sysEnv;

		RulebookPtr requestedNewSession;

		// Stats counters.
		Display::Label *fpsLbl;
		unsigned int frameCount;
		Util::OS::timestamp_t lastTimestamp;
		double fps;
};

}  // namespace HoverScript
}  // namespace Client
