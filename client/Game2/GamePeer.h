
// GamePeer.h
// Scripting peer for system-level control of the game.
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

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

class MR_GameApp;
namespace HoverRace {
	namespace Client {
		class ConfigPeer;
		class Rulebook;
		typedef boost::shared_ptr<Rulebook> RulebookPtr;
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Scripting peer for system-level control of the game.
 * @author Michael Imamura
 */
class GamePeer {
	private:
		GamePeer() { }
	public:
		GamePeer(Script::Core *scripting, MR_GameApp *gameApp);
		virtual ~GamePeer();

	public:
		static void Register(Script::Core *scripting);

	public:
		void OnInit();

	public:
		ConfigPeer *LGetConfig();

		bool LIsInitialized();
		void LOnInit(const luabind::object &fn);
		void LGetOnInit();

		void LStartPractice(const std::string &track);
		void LStartPractice(const std::string &track, const luabind::object &rules);

	private:
		Script::Core *scripting;
		MR_GameApp *gameApp;
		bool initialized;
		int onInitRef;
		RulebookPtr deferredStart;
};

}  // namespace Client
}  // namespace HoverRace
