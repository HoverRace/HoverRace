
// Console.h
// Header for the debug console base class.
//
// Copyright (c) 2009 Michael Imamura.
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

#include <lua.hpp>

#include "../../engine/Util/OS.h"

namespace HoverRace {

namespace Script {
	class Env;
}

/**
 * Base class for debug consoles.
 * @author Michael Imamura
 */
class Console
{
	public:
		Console();
		virtual ~Console();

		virtual void Advance(Util::OS::timestamp_t tick) = 0;

		void SubmitChunk(const std::string &s);

		/// Clear the console.
		virtual void Clear() = 0;

	protected:
		virtual void LogInfo(const std::string &s) = 0;
		virtual void LogError(const std::string &s) = 0;

		enum inputState_t {
			ISTATE_COMMAND,
			ISTATE_CONTINUE,
		};
		void SetInputState(inputState_t newState);
		inputState_t GetInputState() const;

	private:
		static int LClear(lua_State *state);

	private:
		Script::Env *scripting;
		std::string chunk;
		inputState_t inputState;

		class LogStreamBuf;
		class LogStream;
};

}  // namespace HoverRace
