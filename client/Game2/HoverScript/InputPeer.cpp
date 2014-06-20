
// InputPeer.cpp
//
// Copyright (c) 2010, 2014 Michael Imamura.
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

#include "StdAfx.h"

#include "../../../engine/Control/Controller.h"
#include "../../../engine/Script/Core.h"

#include "InputPeer.h"

namespace HoverRace {
namespace Client {
namespace HoverScript {

InputPeer::InputPeer(Script::Core *scripting,
                     Control::InputEventController *controller) :
	SUPER(scripting, "Input"),
	controller(controller)
{
}

/**
 * Register this peer in an environment.
 */
void InputPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L)[
		class_<InputPeer, SUPER, std::shared_ptr<InputPeer>>("Input")
			.def("hotkey", &InputPeer::LHotkey)
	];
}

void InputPeer::LHotkey(const std::string &key, const luabind::object &fn)
{
	using namespace luabind;

	auto scripting = GetScripting();
	auto L = scripting->GetState();

	auto action = controller->Hotkey(key);
	if (!action) {
		luaL_error(L, "Invalid hotkey: %s", key.c_str());
	}
	else {
		hotkeyHandlers.emplace_back(scripting);
		auto &handler = hotkeyHandlers.back();
		handler.AddHandler(fn);

		auto idx = hotkeyHandlers.size() - 1;

		hotkeyConns.emplace_back(
			new boost::signals2::scoped_connection(action->Connect([=]() {
				this->hotkeyHandlers[idx].CallHandlers();
			})));
	}
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
