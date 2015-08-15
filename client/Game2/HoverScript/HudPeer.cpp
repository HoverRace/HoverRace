
// HudPeer.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../../../engine/Script/Core.h"
#include "../../../engine/Display/Chronometer.h"
#include "../../../engine/Display/Counter.h"
#include "../../../engine/Display/FuelGauge.h"
#include "../../../engine/Display/Hud.h"
#include "../../../engine/Display/HudDecor.h"
#include "../../../engine/Display/HudText.h"
#include "../../../engine/Display/Minimap.h"
#include "../../../engine/Display/Speedometer.h"

#include "HudPeer.h"

using namespace HoverRace::Util;
typedef HoverRace::Display::Hud::HudAlignment HudAlignment;

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
 * @param scripting The target environment.
 */
void HudPeer::Register(Script::Core &scripting)
{
	using namespace luabind;
	lua_State *L = scripting.GetState();

	// Register base classes.
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
			.def("add_chronometer", &HudPeer::LAddChronometer)
			.def("add_counter", &HudPeer::LAddCounter_V)
			.def("add_counter", &HudPeer::LAddCounter_VT)
			.def("add_fuel_gauge", &HudPeer::LAddDecor<Display::FuelGauge>)
			.def("add_minimap", &HudPeer::LAddDecor<Display::Minimap>)
			.def("add_speedometer", &HudPeer::LAddDecor<Display::Speedometer>)
			.def("add_text", &HudPeer::LAddText)
			.def("clear", &HudPeer::LClear)
			.def("use_race_default", &HudPeer::LUseRaceDefault),

		class_<Display::HudDecor, std::shared_ptr<Display::HudDecor>>("HudDecor")
	];

	// Register HUD elements.
	module(L, "hud") [
		class_<Display::Chronometer, Display::HudDecor, std::shared_ptr<Display::HudDecor>>("Chronometer"),
		class_<Display::Counter, Display::HudDecor, std::shared_ptr<Display::HudDecor>>("Counter")
			.property("value", &Display::Counter::GetValue, &Display::Counter::SetValue)
			.property("total", &Display::Counter::GetTotal, &Display::Counter::SetTotal),
		class_<Display::FuelGauge, Display::HudDecor, std::shared_ptr<Display::HudDecor>>("FuelGauge"),
		class_<Display::HudText, Display::HudDecor, std::shared_ptr<Display::HudDecor>>("Text")
			.property("text", &Display::HudText::GetText, &Display::HudText::SetText),
		class_<Display::Minimap, Display::HudDecor, std::shared_ptr<Display::HudDecor>>("Minimap"),
		class_<Display::Speedometer, Display::HudDecor, std::shared_ptr<Display::HudDecor>>("Speedometer")
	];
}

std::shared_ptr<Display::Chronometer> HudPeer::LAddChronometer(int align,
	const std::string &title, std::shared_ptr<Util::Clock> clock)
{
	HudAlignment::type ha = ValidateAlignment(align);
	if (auto sp = hud.lock()) {
		return sp->At(ha).NewChild<Display::Chronometer>(display, title, clock);
	}
	else {
		return std::shared_ptr<Display::Chronometer>();
	}
}

std::shared_ptr<Display::Counter> HudPeer::LAddCounter_V(int align,
	const std::string &title, double initValue)
{
	HudAlignment::type ha = ValidateAlignment(align);
	if (auto sp = hud.lock()) {
		return sp->At(ha).NewChild<Display::Counter>(
			display, title, initValue);
	}
	else {
		return std::shared_ptr<Display::Counter>();
	}
}

std::shared_ptr<Display::Counter> HudPeer::LAddCounter_VT(int align,
	const std::string &title, double initValue, double total)
{
	HudAlignment::type ha = ValidateAlignment(align);
	if (auto sp = hud.lock()) {
		return sp->At(ha).NewChild<Display::Counter>(
			display, title, initValue, total);
	}
	else {
		return std::shared_ptr<Display::Counter>();
	}
}

std::shared_ptr<Display::HudText> HudPeer::LAddText(int align,
	const std::string &text)
{
	HudAlignment::type ha = ValidateAlignment(align);
	if (auto sp = hud.lock()) {
		return sp->At(ha).NewChild<Display::HudText>(display, text);
	}
	else {
		return std::shared_ptr<Display::HudText>();
	}
}

void HudPeer::LClear()
{
	if (auto sp = hud.lock()) {
		sp->Clear();
	}
}

void HudPeer::LUseRaceDefault()
{
	if (auto sp = hud.lock()) {
		using namespace Display;
		using HudAlignment = Hud::HudAlignment;

		sp->Clear();
		sp->At(HudAlignment::NE).NewChild<FuelGauge>(display);
		sp->At(HudAlignment::NW).NewChild<Speedometer>(display);
		sp->At(HudAlignment::WNW).NewChild<Minimap>(display);
	}
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
