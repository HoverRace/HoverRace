
// BaseText.cpp
//
// Copyright (c) 2015-2016 Michael Imamura.
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

#include "BaseText.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor for automatically-sized widget.
 * @param text The text.
 * @param font The font.
 * @param color The text foreground color (including alpha).
 * @param layoutFlags Optional layout flags.
 */
BaseText::BaseText(const std::string &text,
	const UiFont &font,
	Color color,
	uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags),
	text(text), font(font), color(color), fixedScale(false)
{
}

void BaseText::SetColor(const Color color)
{
	if (this->color != color) {
		this->color = color;
		FireModelUpdate(Props::COLOR);
	}
}

void BaseText::SetFont(const UiFont &font)
{
	if (this->font != font) {
		this->font = font;
		FireModelUpdate(Props::FONT);
	}
}

void BaseText::SetText(const std::string &text)
{
	if (this->text != text) {
		this->text = text;
		FireModelUpdate(Props::TEXT);
	}
}

void BaseText::SetFixedScale(bool fixedScale)
{
	if (this->fixedScale != fixedScale) {
		this->fixedScale = fixedScale;
		FireModelUpdate(Props::FIXED_SCALE);
	}
}

}  // namespace Display
}  // namespace HoverRace
