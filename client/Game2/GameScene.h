
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
			class GamePeer;
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
	public:
		GameScene(Display::Display &display, GameDirector &director,
			Script::Core *scripting, HoverScript::GamePeer *gamePeer,
			std::shared_ptr<Rules> rules);
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
		void Advance(Util::OS::timestamp_t tick);
		void Render();

	private:
		void OnRaceFinish();

	private:
		Display::Display &display;
		GameDirector &director;
		std::shared_ptr<Rules> rules;

		int frame;
		int numPlayers;
		bool muted;

		static const int MAX_OBSERVERS = Util::Config::MAX_PLAYERS;
		Observer *observers[MAX_OBSERVERS];
		ClientSession *session;

		boost::signals2::connection cameraZoomInConn;
		boost::signals2::connection cameraZoomOutConn;
		boost::signals2::connection cameraPanUpConn;
		boost::signals2::connection cameraPanDownConn;
		boost::signals2::connection cameraResetConn;

		boost::signals2::connection pauseConn;

		HoverScript::SessionPeerPtr sessionPeer;

		bool firedOnRaceFinish;
};

}  // namespace Client
}  // namespace HoverRace
