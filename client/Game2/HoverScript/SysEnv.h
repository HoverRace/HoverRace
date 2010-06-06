
// SysEnv.h
// The global system environment.
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

#include "../../../engine/Script/Env.h"
#include "../../../engine/Util/OS.h"

namespace HoverRace {
	namespace Client {
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

class SysEnv : private Script::Env
{
	typedef Script::Env SUPER;

	public:
		SysEnv(Script::Core *scripting, GamePeer *gamePeer);
		virtual ~SysEnv();

	protected:
		virtual void InitEnv();

	protected:
		virtual void LogInfo(const std::string &s);
		virtual void LogError(const std::string &s);

	public:
		void RunScript(const Util::OS::path_t &filename);

	private:
		GamePeer *gamePeer;
		Script::Core::OutHandle outHandle;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
