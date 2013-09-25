
// RuntimeEnv.h
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

#include "../../../engine/Script/Env.h"

namespace HoverRace {
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Basic restricted environment for potentially untrusted code.
 * @author Michael Imamura
 */
class RuntimeEnv : public Script::Env {
	typedef Script::Env SUPER;
	public:
		RuntimeEnv(Script::Core *scripting);
		virtual ~RuntimeEnv();

	private:
		Script::Core::OutHandle outHandle;

		class LogStreamBuf;
		class LogStream;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
