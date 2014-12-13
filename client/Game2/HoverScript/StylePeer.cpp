
// StylePeer.cpp
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

#include "../../StdAfx.h"

#include "../../../engine/Script/Core.h"
#include "../../../engine/Display/Display.h"

#include "StylePeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

StylePeer::StylePeer(Script::Core *scripting, Display::Display &display) :
	SUPER(scripting, "Style"),
	display(display)
{
}

/**
 * Register this peer in an environment.
 */
void StylePeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	using Styles = Display::Display::styles_t;
	lua_State *L = scripting->GetState();

	// Register base classes.
	module(L) [
		class_<Styles, Styles*>("Styles")
			.def_readwrite("bodyFont", &Styles::bodyFont)
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
