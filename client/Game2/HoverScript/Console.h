
// Console.h
//
// Copyright (c) 2009, 2014-2016 Michael Imamura.
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

#include "../../../engine/Script/Env.h"
#include "../../../engine/Util/OS.h"
#include "../../../engine/Script/Help/HelpHandler.h"

namespace HoverRace {
	namespace Script {
		namespace Help {
			class Class;
		}
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Base class for debug consoles.
 * @author Michael Imamura
 */
class Console : public Script::Env, public Script::Help::HelpHandler
{
	using SUPER = Script::Env;

public:
	Console(Script::Core &scripting);
	virtual ~Console();

protected:
	void InitEnv() override;

public:
	virtual void Advance(Util::OS::timestamp_t tick) = 0;

	void SubmitChunk(const std::string &s);

	/// Clear the console.
	virtual void Clear() = 0;

protected:
	virtual void LogInfo(const std::string &s) = 0;
	virtual void LogError(const std::string &s) = 0;

public:
	enum class InputState {
		COMMAND,
		CONTINUE,
	};
	InputState GetInputState() const;
protected:
	void SetInputState(InputState newState);

private:
	static int LClear(lua_State *state);
	/*
	static int LReset(lua_State *state);
	static int LSandbox(lua_State *state);
	*/

private:
	InputState inputState;
	Script::Core::OutHandle outHandle;
	std::string chunk;

	class LogStreamBuf;
	class LogStream;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
