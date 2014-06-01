
// InputPeer.h
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../../../engine/Script/Handlers.h"
#include "../../../engine/Script/Peer.h"

namespace HoverRace {
	namespace Control {
		class InputEventController;
	}
	namespace Script {
		class Core;
	}
}


namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Scripting peer for input.
 * @author Michael Imamura
 */
class InputPeer : public Script::Peer
{
	typedef Script::Peer SUPER;
	public:
		InputPeer(Script::Core *scripting,
			Control::InputEventController *controller);
		virtual ~InputPeer() { }

	public:
		static void Register(Script::Core *scripting);

	public:
		void LHotkey(const std::string &key, const luabind::object &fn);

	private:
		Control::InputEventController *controller;
		std::vector<Script::Handlers> hotkeyHandlers;
		std::vector<std::unique_ptr<boost::signals2::scoped_connection>> hotkeyConns;
};

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
