
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

#include "StdAfx.h"

#include "Wallpaper.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param fill The fill mode.
 * @param texture The texture.
 * @param opacity The initial opacity.
 */
Wallpaper::Wallpaper(std::shared_ptr<Res<Texture>> texture,
                     Fill fill, double opacity) :
	SUPER(),
	fill(fill), texture(texture), opacity(opacity)
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
 * Set the opacity of the overlay.
 * The value will be clamped to the range 0.0 to 1.0 inclusive.
 * @param opacity The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
 */
void Wallpaper::SetOpacity(double opacity)
{
	if (opacity < 0.0) opacity = 0.0;
	else if (opacity > 1.0) opacity = 1.0;

	if (this->opacity != opacity) {
		this->opacity = opacity;
		FireModelUpdate(Props::OPACITY);
	}
}

}  // namespace Display
}  // namespace HoverRace
