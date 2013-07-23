
// Button.cpp
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

#include "../Control/Action.h"
#include "../Util/Config.h"
#include "../Util/Log.h"
#include "UiFont.h"
#include "FillBox.h"
#include "Label.h"

#include "Button.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const double DEFAULT_PADDING_HEIGHT = 10;
	const double DEFAULT_PADDING_WIDTH = 30;
}

/**
 * Constructor for automatically-sized button.
 * @param display The display child elements will be attached to.
 * @param text The text for the button.
 * @param layoutFlags Optional layout flags.
 */
Button::Button(Display &display, const std::string &text,
               uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags), display(display),
	paddingTop(DEFAULT_PADDING_HEIGHT), paddingRight(DEFAULT_PADDING_WIDTH),
	paddingBottom(DEFAULT_PADDING_HEIGHT), paddingLeft(DEFAULT_PADDING_WIDTH)
{
	Init(text);
}

/**
 * Constructor for fixed-sized button.
 * @param display The display child elements will be attached to.
 * @param size The fixed button size.
 * @param text The text for the button.
 * @param layoutFlags Optional layout flags.
 */
Button::Button(Display &display, const Vec2 &size, const std::string &text,
               uiLayoutFlags_t layoutFlags) :
	SUPER(display, size, layoutFlags), display(display),
	paddingTop(DEFAULT_PADDING_HEIGHT), paddingRight(DEFAULT_PADDING_WIDTH),
	paddingBottom(DEFAULT_PADDING_HEIGHT), paddingLeft(DEFAULT_PADDING_WIDTH)
{
	Init(text);
}

Button::~Button()
{
}

void Button::Init(const std::string &text)
{
	//TODO: Get background, font, and padding from theme.
	Config *cfg = Config::GetInstance();
	UiFont font(cfg->GetDefaultFontName(), 30);

	label.reset(new Label(text, font, 0xffffffff));
	label->SetAlignment(Alignment::CENTER);
	label->AttachView(display);

	background.reset(new FillBox(0, 0, 0x3f00007f));
	background->AttachView(display);
}

void Button::Layout()
{
	Log::Info("Button layout!");

	const Vec2 size = GetSize();
	
	background->SetSize(size);

	// Move label to center.
	label->SetPos(size.x / 2.0, size.y / 2.0);
}

Vec3 Button::Measure() const
{
	if (IsAutoSize()) {
		const Vec3 labelSize = label->Measure();
		return Vec3(
			labelSize.x + paddingLeft + paddingRight,
			labelSize.y + paddingTop + paddingBottom,
			0);
	}
	else {
		return SUPER::Measure();
	}
}

}  // namespace Display
}  // namespace HoverRace
