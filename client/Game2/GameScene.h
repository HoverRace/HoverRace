
// GameScene.h
//
// Copyright (c) 2010, 2013, 2014 Michael Imamura.
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

#include "../../engine/Util/Config.h"

#include "Observer.h"
#include "GameDirector.h"
#include "Rules.h"

#include "Scene.h"

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class MetaPlayer;
			class MetaSession;
			class SessionPeer;
			typedef std::shared_ptr<SessionPeer> SessionPeerPtr;
			class SysEnv;
		}
		class ClientSession;
		class LoadingScene;
	}
	namespace Control {
		class InputEventController;
	}
	namespace Display {
		class Display;
		class Hud;
	}
	namespace Script {
		class Core;
	}
	namespace Util {
		class Loader;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Base for scenes that render and interact with tracks.
 * @author Michael Imamura
 */
class GameScene : public Scene
{
	typedef Scene SUPER;
	protected:
		struct Viewport {
			Viewport(Display::Display &display, Observer *observer,
				Display::Hud *hud);

			Viewport(const Viewport &o) = delete;
			Viewport(Viewport &&viewport) = default;

			Viewport &operator=(const Viewport &o) = delete;
			Viewport &operator=(Viewport &&viewport) = default;

			std::unique_ptr<Observer> observer;
			std::shared_ptr<Display::Hud> hud;
		};

	public:
		GameScene(Display::Display &display, GameDirector &director,
			Script::Core *scripting, std::shared_ptr<Rules> rules,
			std::shared_ptr<Util::Loader> loader);
		virtual ~GameScene();

	private:
		void Cleanup();

		void ScheduleLoad();

	public:
		virtual bool IsMouseCursorEnabled() const { return false; }

	protected:
		virtual void OnFinishedLoading();

	public:
		void SetHudVisible(bool visible);
		void SetMuted(bool muted);

	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		void OnRaceFinish();

	protected:
		Display::Display &display;
		GameDirector &director;
		Script::Core *scripting;
		std::shared_ptr<Rules> rules;
		std::shared_ptr<Util::Loader> loader;

	private:
		bool finishedLoading;
		bool muted;

	protected:
		std::vector<Viewport> viewports;
		ClientSession *session;

	private:
		boost::signals2::scoped_connection finishedLoadingConn;

		std::shared_ptr<HoverScript::MetaSession> metaSession;

		bool firedOnStart;
		bool firedOnRaceFinish;
};

}  // namespace Client
}  // namespace HoverRace
