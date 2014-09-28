
// Chronometer.cpp
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

#include "../StdAfx.h"

#include "../Util/Clock.h"
#include "FillBox.h"
#include "Label.h"

#include "Chronometer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const Color BG_COLOR = 0x3f000000;
	const Color TITLE_COLOR = 0xffe3063e;
	const Color VALUE_COLOR = TITLE_COLOR;
	const double MARGIN_WIDTH = 15;
	const double MARGIN_HEIGHT = 10;
}

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param title The label text.
 * @param clock The time source.
 */
Chronometer::Chronometer(Display &display, const std::string &title,
                         std::shared_ptr<Util::Clock> clock) :
	SUPER(display),
	clock(std::move(clock)), lastTick(0)
{
	const UiFont titleFont(20);
	const UiFont valueFont(30, UiFont::BOLD);

	bg = AddChild(new FillBox(0, 0, BG_COLOR));

	titleLbl = AddChild(new Label(title, titleFont, TITLE_COLOR));
	titleLbl->SetAlignment(Alignment::W);

	valueLbl = AddChild(new Label("", valueFont, VALUE_COLOR));
}

void Chronometer::Layout()
{
	SUPER::Layout();

	const Vec3 titleSize = titleLbl->Measure();
	const Vec3 valueSize = valueLbl->Measure();

	titleLbl->SetPos(MARGIN_WIDTH, MARGIN_HEIGHT + (valueSize.y / 2.0));

	double x = MARGIN_WIDTH;
	if (!titleLbl->GetText().empty()) {
		x += titleSize.x + 10.0;
	}
	valueLbl->SetPos(x, MARGIN_HEIGHT);

	Vec2 sz(x + valueSize.x + MARGIN_WIDTH,
		MARGIN_HEIGHT + valueSize.y + MARGIN_HEIGHT);
	bg->SetSize(sz);
	SetSize(sz);
}

void Chronometer::Advance(Util::OS::timestamp_t tick)
{
	if (lastTick != tick) {
		lastTick = tick;
		size_t oldLen = valueLbl->GetText().length();

		// Negative times are countdowns, so leave off the negative sign.
		std::string text = clock->FmtLong();
		if (!text.empty() && text[0] == '-') {
			text.erase(0, 1);
		}
		valueLbl->SetText(text);

		// We assume that the digits in the font are the same width.
		// Even if they're slightly off, we only change the position if the
		// number of digits change so that the position remains stable.
		size_t newLen = valueLbl->GetText().length();
		if (oldLen != newLen) {
			RequestLayout();
		}
	}
}

}  // namespace Display
}  // namespace HoverRace
