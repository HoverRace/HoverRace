
// Speedometer.cpp
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

#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Player/Player.h"
#include "FillBox.h"

#include "Speedometer.h"

namespace HoverRace {
namespace Display {

namespace {
	const double GAUGE_WIDTH = 650;
	const double GAUGE_HEIGHT = 20;
	const Color BG_COLOR = 0xbfc1cae7;
	const Color FG_COLOR = 0xff5473cf;
	const Color REVERSE_COLOR = 0xffcc154d;
}

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 */
Speedometer::Speedometer(Display &display) :
	SUPER(display),
	gaugeSize(GAUGE_WIDTH, GAUGE_HEIGHT)
{
	SetSize(GAUGE_WIDTH, GAUGE_HEIGHT * 2);
	SetClip(false);

	absBg = NewChild<FillBox>(GAUGE_WIDTH, GAUGE_HEIGHT, BG_COLOR);
	absFg = NewChild<FillBox>(GAUGE_WIDTH, GAUGE_HEIGHT, FG_COLOR);

	dirBg = NewChild<FillBox>(GAUGE_WIDTH, GAUGE_HEIGHT, BG_COLOR);
	dirBg->SetPos(0, GAUGE_HEIGHT);
	dirFg = NewChild<FillBox>(GAUGE_WIDTH, GAUGE_HEIGHT, FG_COLOR);
	dirFg->SetPos(0, GAUGE_HEIGHT);
}

void Speedometer::OnHudRescaled(const Vec2 &hudScale)
{
	gaugeSize.x = GAUGE_WIDTH * hudScale.x;
	gaugeSize.y = GAUGE_HEIGHT * hudScale.y;

	SetSize(gaugeSize.x, gaugeSize.y * 2.0);

	absBg->SetSize(gaugeSize);

	dirBg->SetSize(gaugeSize);
	dirBg->SetPos(0, gaugeSize.y);

	dirFg->SetPos(0, gaugeSize.y);
}

void Speedometer::Advance(Util::OS::timestamp_t)
{
	auto player = GetPlayer();
	if (!player) return;
	auto mchar = player->GetMainCharacter();
	if (!mchar) return;

	double absSpeed = mchar->GetAbsoluteSpeed();
	double dirSpeed = mchar->GetDirectionalSpeed();

	absFg->SetSize(gaugeSize.x * absSpeed, gaugeSize.y);
	if (dirSpeed < 0) {
		dirFg->SetSize(gaugeSize.x * -dirSpeed, gaugeSize.y);
		dirFg->SetColor(REVERSE_COLOR);
	}
	else {
		dirFg->SetSize(gaugeSize.x * dirSpeed, gaugeSize.y);
		dirFg->SetColor(FG_COLOR);
	}
}

}  // namespace Display
}  // namespace HoverRace
