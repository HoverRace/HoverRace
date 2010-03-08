
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

#include "Console.h"

namespace HoverRace {
namespace Client {

class SysConsole : public Console
{
	typedef Console SUPER;

	public:
		SysConsole();
		virtual ~SysConsole();

	protected:
		virtual void InitEnv(Script::Core *scripting);
		virtual void InitGlobals(Script::Core *scripting);

	public:
		virtual void Advance(Util::OS::timestamp_t tick) { };
		virtual void Clear() { };
	
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
		int onInitRef;
};

}  // namespace Client
}  // namespace HoverRace
