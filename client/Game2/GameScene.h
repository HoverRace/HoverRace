
// GameScene.h
//
// Copyright (c) 2010, 2013 Michael Imamura.
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
}

namespace HoverRace {
namespace Client {

/**
 * The actual, genuine gameplay scene.  Accept no imitations.
 *
 * This scene manages the lifetime of a single session, from start to finish.
 * Pushing this scene onto the stage starts a new game session, and likewise
 * popping this scene from the stage aborts the game session.
 *
 * Note that this scene may also be used non-interactively (demo mode),
 * e.g. as a background for the main menu.
 *
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
			Script::Core *scripting, std::shared_ptr<Rules> rules);
		virtual ~GameScene();

	private:
		void Cleanup();

	public:
		virtual void AttachController(Control::InputEventController &controller);
		virtual void DetachController(Control::InputEventController &controller);
		virtual bool IsMouseCursorEnabled() const { return false; }

	private:
		void OnCameraZoom(int increment);
		void OnCameraPan(int increment);
		void OnCameraReset();
		void OnPause();

	public:
		void SetHudVisible(bool visible);
		void SetMuted(bool muted);
		void StartDemoMode();

	public:
		virtual void Advance(Util::OS::timestamp_t tick);
		virtual void PrepareRender();
		virtual void Render();

	private:
		void OnRaceFinish();

	private:
		Display::Display &display;
		GameDirector &director;
		std::shared_ptr<Rules> rules;

		bool muted;

		std::vector<Viewport> viewports;
		ClientSession *session;

		boost::signals2::connection cameraZoomInConn;
		boost::signals2::connection cameraZoomOutConn;
		boost::signals2::connection cameraPanUpConn;
		boost::signals2::connection cameraPanDownConn;
		boost::signals2::connection cameraResetConn;

		boost::signals2::connection pauseConn;

		std::shared_ptr<HoverScript::MetaSession> metaSession;

		bool firedOnStart;
		bool firedOnRaceFinish;
};

}  // namespace Client
}  // namespace HoverRace
