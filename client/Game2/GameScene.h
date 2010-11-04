
// GameScene.h
// Main game scene.
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

#include "../../engine/Util/Config.h"

#include "Observer.h"
#include "GameDirector.h"

#include "Scene.h"

namespace HoverRace {
	namespace Client {
		namespace Control {
			class InputEventController;
		}
		namespace HoverScript {
			class GamePeer;
			class SessionPeer;
			typedef boost::shared_ptr<SessionPeer> SessionPeerPtr;
			class SysEnv;
		}
		class ClientSession;
		class Rulebook;
		typedef boost::shared_ptr<Rulebook> RulebookPtr;
	}
	class HighObserver;
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
		GameScene(GameDirector *director, VideoServices::VideoBuffer *videoBuf,
			Script::Core *scripting, HoverScript::GamePeer *gamePeer,
			RulebookPtr rules);
		virtual ~GameScene();

	private:
		void Cleanup();

	public:
		void Advance(Util::OS::timestamp_t tick);

		void Render();

	private:
		int frame;
		int numPlayers;

		VideoServices::VideoBuffer *videoBuf;

		static const int MAX_OBSERVERS = Util::Config::MAX_PLAYERS;
		Observer *observers[MAX_OBSERVERS];
		ClientSession *session;

		HighObserver *highObserver;
		HoverScript::HighConsole *highConsole;

		HoverScript::SessionPeerPtr sessionPeer;
};

}  // namespace HoverScript
}  // namespace Client
