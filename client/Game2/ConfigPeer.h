
// ConfigPeer.h
// Scripting peer for access to the game configuration.
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

namespace HoverRace {
	namespace Script {
		class Core;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Scripting peer for access to the game configuration.
 * @author Michael Imamura
 */
class ConfigPeer {
	private:
		ConfigPeer() { }
	public:
		ConfigPeer(Script::Core *scripting);
		virtual ~ConfigPeer();

	public:
		static void Register(Script::Core *scripting);

	public:
		void LUnlink();

		void LGetVideoRes();
		void LSetVideoRes(int w, int h);

	private:
		Script::Core *scripting;
};

}  // namespace Client
}  // namespace HoverRace
