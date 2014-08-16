
// Wallpaper.cpp
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

#include "../StdAfx.h"

#include "Wallpaper.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param texture The texture.
 * @param fill The fill mode.
 * @param opacity The initial opacity (default is fully-opaque).
 * @param color The color modulation (default is white, i.e., no modulation).
 */
Wallpaper::Wallpaper(std::shared_ptr<Res<Texture>> texture,
                     Fill fill, double opacity, Color color) :
	SUPER(opacity),
	fill(fill), texture(std::move(texture)), color(color)
{
}

Wallpaper::~Wallpaper()
{
}

/**
 * Sets the fill mode.
 * @param fill The fill mode.
 */
void Wallpaper::SetFill(Fill fill)
{
	if (this->fill != fill) {
		this->fill = fill;
		FireModelUpdate(Props::FILL);
	}
}

/**
 * Set the color modulation of the texture.
 * @param color The modulation color (including alpha).
 */
void Wallpaper::SetColor(const Color color)
{
	if (this->color != color) {
		this->color = color;
		FireModelUpdate(Props::COLOR);
	}
}

}  // namespace Display
}  // namespace HoverRace
