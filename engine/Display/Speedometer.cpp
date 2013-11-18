
// Speedometer.cpp
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

#include "../../engine/MainCharacter/MainCharacter.h"
#include "FillBox.h"

#include "Speedometer.h"

namespace HoverRace {
namespace Display {

namespace {
	const double GAUGE_WIDTH = 650;
	const double GAUGE_HEIGHT = 20;
	const Color BG_COLOR = 0xffc1cae7;
	const Color FG_COLOR = 0xff5473cf;
	const Color REVERSE_COLOR = 0xffcc154d;
}

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
Speedometer::Speedometer(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags)
{
	SetSize(GAUGE_WIDTH, GAUGE_HEIGHT * 2);
	SetClip(false);

	absBg = AddChild(new FillBox(GAUGE_WIDTH, GAUGE_HEIGHT, BG_COLOR));
	absFg = AddChild(new FillBox(GAUGE_WIDTH, GAUGE_HEIGHT, FG_COLOR));

	dirBg = AddChild(new FillBox(GAUGE_WIDTH, GAUGE_HEIGHT, BG_COLOR));
	dirBg->SetPos(0, GAUGE_HEIGHT);
	dirFg = AddChild(new FillBox(GAUGE_WIDTH, GAUGE_HEIGHT, FG_COLOR));
	dirFg->SetPos(0, GAUGE_HEIGHT);
}

void Speedometer::Advance(Util::OS::timestamp_t tick)
{
	auto player = GetPlayer();
	double absSpeed = player->GetAbsoluteSpeed();
	double dirSpeed = player->GetDirectionalSpeed();

	absFg->SetSize(GAUGE_WIDTH * absSpeed, GAUGE_HEIGHT);
	if (dirSpeed < 0) {
		dirFg->SetSize(GAUGE_WIDTH * -dirSpeed, GAUGE_HEIGHT);
		dirFg->SetColor(REVERSE_COLOR);
	}
	else {
		dirFg->SetSize(GAUGE_WIDTH * dirSpeed, GAUGE_HEIGHT);
		dirFg->SetColor(FG_COLOR);
	}
}

}  // namespace Display
}  // namespace HoverRace
