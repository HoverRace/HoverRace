
// SysConsole.h
// The global system console.
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

#include "../../engine/Script/Core.h"
#include "../../engine/Script/Env.h"

namespace HoverRace {
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {

class SysConsole : private Script::Env
{
	typedef Script::Env SUPER;

	public:
		SysConsole(Script::Core *scripting);
		virtual ~SysConsole();

	protected:
		virtual void InitEnv();

	protected:
		virtual void LogInfo(const std::string &s);
		virtual void LogError(const std::string &s);

	public:
		void RunScript(const std::string &filename);

	public:
		void OnInit();

	private:
		static int LOnInit(lua_State *state);
		static int LGetOnInit(lua_State *state);

	private:
		Script::Core::OutHandle outHandle;
		int onInitRef;
};

}  // namespace Client
}  // namespace HoverRace
