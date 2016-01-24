
// SysEnv.h
//
// Copyright (c) 2010, 2013-2016 Michael Imamura.
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

namespace HoverRace {
	namespace Client {
		namespace HoverScript {
			class DebugPeer;
			class GamePeer;
			class InputPeer;
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
 * The global system environment.
 * @author Michael Imamura
 */
class SysEnv : private Script::Env
{
	using SUPER = Script::Env;

public:
	SysEnv(Script::Core &scripting, DebugPeer &debugPeer,
		GamePeer &gamePeer, InputPeer &inputPeer);
	virtual ~SysEnv();

protected:
	void InitEnv() override;

protected:
	virtual void LogInfo(const std::string &s);
	virtual void LogError(const std::string &s);

public:
	// Expose privately-inherited function.
	void RunScript(const Util::OS::path_t &filename) { SUPER::RunScript(filename); }

private:
	DebugPeer &debugPeer;
	GamePeer &gamePeer;
	InputPeer &inputPeer;
	Script::Core::OutHandle outHandle;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
