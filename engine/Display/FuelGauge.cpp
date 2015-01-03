
// FuelGauge.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Player/Player.h"
#include "FillBox.h"

#include "FuelGauge.h"

namespace HoverRace {
namespace Display {

namespace {
	const double GAUGE_WIDTH = 300;
	const double GAUGE_HEIGHT = 40;
	const Color BG_COLOR = 0xbfc1cae7;
	const Color FG_COLOR = 0xff5473cf;
	const Color LOW_COLOR = 0xfff11b1b;
}

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 */
FuelGauge::FuelGauge(Display &display) :
	SUPER(display),
	gaugeSize(GAUGE_WIDTH, GAUGE_HEIGHT)
{
	SetSize(GAUGE_WIDTH, GAUGE_HEIGHT);
	SetClip(false);

	bg = AddChild(new FillBox(GAUGE_WIDTH, GAUGE_HEIGHT, BG_COLOR));
	fg = AddChild(new FillBox(GAUGE_WIDTH, GAUGE_HEIGHT, FG_COLOR));
}

void FuelGauge::OnHudRescaled(const Vec2 &hudScale)
{
	gaugeSize.x = GAUGE_WIDTH * hudScale.x;
	gaugeSize.y = GAUGE_HEIGHT * hudScale.y;

	bg->SetSize(gaugeSize);

	SetSize(gaugeSize);
}

void FuelGauge::Advance(Util::OS::timestamp_t)
{
	auto player = GetPlayer();
	if (!player) return;
	auto mchar = player->GetMainCharacter();
	if (!mchar) return;

	double fuel = mchar->GetFuelLevel();
	if (fuel < 0.0) fuel = 0.0;
	else if (fuel > 1.0) fuel = 1.0;

	fg->SetSize(gaugeSize.x * fuel, gaugeSize.y);
	bg->SetColor(fuel < 0.20 ? LOW_COLOR : BG_COLOR);
}

}  // namespace Display
}  // namespace HoverRace
