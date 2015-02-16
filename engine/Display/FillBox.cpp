
// FillBox.cpp
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

#include "FillBox.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param size The size of the box, where @c x is the width
 *             and @c y is the height.
 * @param color The color of the box (including alpha).
 * @param layoutFlags Optional layout flags.
 */
FillBox::FillBox(const Vec2 &size, const Color color,
	uiLayoutFlags_t layoutFlags) :
	FillBox(size, color, 0, 0, layoutFlags)
{
}

/**
 * Constructor.
 * @param w The width of the box.
 * @param h The height of the box.
 * @param color The color of the box (including alpha).
 * @param layoutFlags Optional layout flags.
 */
FillBox::FillBox(double w, double h, const Color color,
	uiLayoutFlags_t layoutFlags) :
	FillBox(Vec2(w, h), color, layoutFlags)
{
}

/**
 * Constructor with border.
 * @param size The size of the box, where @c x is the width
 *             and @c y is the height.
 * @param color The color of the box (including alpha).
 * @param border The border thickness (zero is no border).
 * @param borderColor The border color.
 * @param layoutFlags Optional layout flags.
 */
FillBox::FillBox(const Vec2 &size, const Color color,
	double border, const Color borderColor, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags),
	size(size), color(color), border(border), borderColor(borderColor)
{
}

/**
 * Constructor with border.
 * @param w The width of the box.
 * @param h The height of the box.
 * @param color The color of the box (including alpha).
 * @param border The border thickness (zero is no border).
 * @param borderColor The border color.
 * @param layoutFlags Optional layout flags.
 */
FillBox::FillBox(double w, double h, const Color color,
	double border, const Color borderColor, uiLayoutFlags_t layoutFlags) :
	FillBox(Vec2(w, h), color, border, borderColor, layoutFlags)
{
}

/**
 * Set the color of the box.
 * @param color The color (including alpha).
 */
void FillBox::SetColor(const Color color)
{
	if (this->color != color) {
		this->color = color;
		FireModelUpdate(Props::COLOR);
	}
}

/**
 * Set the size of the box.
 * @param size The size of the box, where @c x is the width
 *             and @c y is the height.
 */
void FillBox::SetSize(const Vec2 &size)
{
	if (this->size != size) {
		this->size = size;
		FireModelUpdate(Props::SIZE);
	}
}

/**
 * Set the border thickness.
 * @param border The border.  May be zero for no border.
 */
void FillBox::SetBorder(double border)
{
	if (this->border != border) {
		this->border = border;
		FireModelUpdate(Props::BORDER);
	}
}

/**
 * Set the color of the border.
 * @param color The color.
 */
void FillBox::SetBorderColor(const Color color)
{
	if (borderColor != color) {
		borderColor = color;
		FireModelUpdate(Props::BORDER_COLOR);
	}
}


/**
 * Adjust the height, preserving the preferred aspect ratio.
 * @param h The new height.
 */
void FillBox::AdjustHeight(double h)
{
	SetSize(size.x * (h / size.y), h);
}

/**
 * Adjust the width, preserving the preferred aspect ratio.
 * @param w The new width.
 */
void FillBox::AdjustWidth(double w)
{
	SetSize(w, size.y * (w / size.x));
}

}  // namespace Display
}  // namespace HoverRace
