
// HudPeer.cpp
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

#include "StdAfx.h"

#include "../../../engine/Script/Core.h"
#include "../../../engine/Display/FuelGauge.h"
#include "../../../engine/Display/Hud.h"
#include "../../../engine/Display/Speedometer.h"

#include "HudPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

HudPeer::HudPeer(Script::Core *scripting, Display::Display &display,
                 std::weak_ptr<Display::Hud> hud) :
	SUPER(scripting, "Hud"),
	display(display), hud(std::move(hud))
{
}

HudPeer::~HudPeer()
{
}

/**
 * Register this peer in an environment.
 */
void HudPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	typedef Display::Hud::HudAlignment HudAlignment;
	lua_State *L = scripting->GetState();

	module(L) [
		class_<HudPeer, SUPER, std::shared_ptr<HudPeer>>("Hud")
			.enum_("HudAlignment") [
				value("ABOVE", HudAlignment::ABOVE),
				value("BELOW", HudAlignment::BELOW),
				value("N", HudAlignment::N),
				value("NNE", HudAlignment::NNE),
				value("NE", HudAlignment::NE),
				value("ENE", HudAlignment::ENE),
				value("E", HudAlignment::E),
				value("ESE", HudAlignment::ESE),
				value("SE", HudAlignment::SE),
				value("SSE", HudAlignment::SSE),
				value("S", HudAlignment::S),
				value("SSW", HudAlignment::SSW),
				value("SW", HudAlignment::SW),
				value("WSW", HudAlignment::WSW),
				value("W", HudAlignment::W),
				value("WNW", HudAlignment::WNW),
				value("NW", HudAlignment::NW),
				value("NNW", HudAlignment::NNW)
			]
			.def("use_race_default", &HudPeer::LUseRaceDefault)
	];
}

void HudPeer::LUseRaceDefault()
{
	if (auto sp = hud.lock()) {
		typedef Display::Hud::HudAlignment HudAlignment;
		sp->AddHudChild(HudAlignment::NE, new Display::FuelGauge(display));
		sp->AddHudChild(HudAlignment::NW, new Display::Speedometer(display));
	}
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
