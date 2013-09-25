
// RulebookEnv.h
//
// Copyright (c) 2013 Michael Imamura.
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

#include "RuntimeEnv.h"

namespace HoverRace {
	namespace Client {
		class Rulebook;
		class RulebookLibrary;
		namespace HoverScript {
			class GamePeer;
		}
	}
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Limited environment for defining rulebooks.
 * @author Michael Imamura
 */
class RulebookEnv : public RuntimeEnv {
	typedef RuntimeEnv SUPER;
	public:
		RulebookEnv(Script::Core *scripting, RulebookLibrary &rulebookLibrary);
		virtual ~RulebookEnv();

	protected:
		virtual void InitEnv();

	public:
		void ReloadRulebooks();
		void DefineRulebook(const std::string &name, const luabind::object &defn);
	private:
		void DefineRules(std::shared_ptr<Rulebook> rulebook, const luabind::object &rulesObj);

	private:
		static int LRulebookStage1(lua_State *L);
		static int LRulebookStage2(lua_State *L);

	private:
		RulebookLibrary &rulebookLibrary;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
