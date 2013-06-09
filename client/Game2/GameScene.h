
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
		class Rulebook;
		typedef std::shared_ptr<Rulebook> RulebookPtr;
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

class GameScene : public Scene
{
	typedef Scene SUPER;
	public:
		GameScene(GameDirector *director, Display::Display &display,
			Script::Core *scripting, HoverScript::GamePeer *gamePeer,
			RulebookPtr rules);
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
	protected:
		virtual void OnPhaseChanged(Phase::phase_t oldPhase);

	public:
		void Advance(Util::OS::timestamp_t tick);
		void Render();

	private:
		void OnRaceFinish();

	private:
		int frame;
		int numPlayers;

		GameDirector *director;
		Display::Display &display;

		static const int MAX_OBSERVERS = Util::Config::MAX_PLAYERS;
		Observer *observers[MAX_OBSERVERS];
		ClientSession *session;

		boost::signals2::connection cameraZoomInConn;
		boost::signals2::connection cameraZoomOutConn;
		boost::signals2::connection cameraPanUpConn;
		boost::signals2::connection cameraPanDownConn;
		boost::signals2::connection cameraResetConn;

		HoverScript::SessionPeerPtr sessionPeer;

		bool firedOnRaceFinish;
};

}  // namespace HoverScript
}  // namespace Client
