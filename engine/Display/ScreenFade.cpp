
// ScreenFade.cpp
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

#include "ScreenFade.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param color The color of the fade (the alpha component is ignored).
 * @param opacity The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
 */
ScreenFade::ScreenFade(Color color, double opacity) :
	SUPER(),
	color(color), opacity(opacity)
{
}

ScreenFade::~ScreenFade()
{
}

/**
 * Set the color of the overlay when fully-opaque.
 * @param color The color of the fade (the alpha component is ignored).
 */
void ScreenFade::SetColor(const Color color)
{
	if (this->color != color) {
		this->color = color;
		FireModelUpdate(Props::COLOR);
	}
}

/**
 * Set the opacity of the overlay.
 * The value will be clamped to the range 0.0 to 1.0 inclusive.
 * @param opacity The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
 */
void ScreenFade::SetOpacity(double opacity)
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
