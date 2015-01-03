
// HudText.cpp
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

#include "FillBox.h"
#include "Label.h"

#include "HudText.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const Color BG_COLOR = 0x3f000000;
	const Color FG_COLOR = 0xffffffff;
	const double MARGIN_WIDTH = 15;
	const double MARGIN_HEIGHT = 10;
}

/**
 * Constructor.
 * @param display The display child elements will be attached to.
 * @param text The text.
 */
HudText::HudText(Display &display, const std::string &text) :
	SUPER(display)
{
	bg = AddChild(new FillBox(0, 0, BG_COLOR));

	textLbl = AddChild(new Label(text, display.styles.hudNormalFont, FG_COLOR));
}

void HudText::OnHudRescaled(const Vec2 &hudScale)
{
	const auto &s = display.styles;
	bool small = hudScale.x < 1.0 || hudScale.y < 1.0;

	if (small) {
		textLbl->SetFont(s.hudSmallFont);
	}
	else {
		textLbl->SetFont(s.hudNormalFont);
	}

	RequestLayout();
}

/**
 * Get the text.
 * @return The current text.
 */
const std::string &HudText::GetText() const
{
	return textLbl->GetText();
}

/**
 * Set the text.
 * @param text The text.
 */
void HudText::SetText(const std::string &text)
{
	if (textLbl->GetText() != text) {
		textLbl->SetText(text);
		RequestLayout();
	}
}

void HudText::Layout()
{
	SUPER::Layout();

	const Vec3 textSize = textLbl->Measure();

	textLbl->SetPos(MARGIN_WIDTH, MARGIN_HEIGHT);

	Vec2 sz(MARGIN_WIDTH + textSize.x + MARGIN_WIDTH,
		MARGIN_HEIGHT + textSize.y + MARGIN_HEIGHT);
	bg->SetSize(sz);
	SetSize(sz);
}

}  // namespace Display
}  // namespace HoverRace
