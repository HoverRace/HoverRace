
// RuleLine.cpp
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../Exception.h"

#include "RuleLine.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param direction Whether the line is horizontal or vertical.
 * @param length The length of the line.
 * @param width The thickness of the line.
 * @param color The color of the line (including alpha).
 * @param layoutFlags Optional layout flags.
 */
RuleLine::RuleLine(Direction direction, double length, double width,
                   const Color color, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags),
	direction(direction), length(length), width(width), color(color)
{
}

/**
 * Sets the direction of the line.
 * @param direction The direction.
 */
void RuleLine::SetDirection(Direction direction)
{
	if (this->direction != direction) {
		this->direction = direction;
		FireModelUpdate(Props::DIRECTION);
	}
}

/**
 * Sets the length of the line.
 * @param length The length.
 */
void RuleLine::SetLength(double length)
{
	if (this->length != length) {
		this->length = length;
		FireModelUpdate(Props::SIZE);
	}
}

/**
 * Sets the width (thickness) of the line.
 * @param width The width.
 */
void RuleLine::SetWidth(double width)
{
	if (this->width != width) {
		this->width = width;
		FireModelUpdate(Props::SIZE);
	}
}

/**
 * Set the color of the line.
 * @param color The color (including alpha).
 */
void RuleLine::SetColor(const Color color)
{
	if (this->color != color) {
		this->color = color;
		FireModelUpdate(Props::COLOR);
	}
}

Vec3 RuleLine::Measure()
{
	return direction == Direction::H ?
		Vec3(length, width, 0) :
		Vec3(width, length, 0);
}

}  // namespace Display
}  // namespace HoverRace
