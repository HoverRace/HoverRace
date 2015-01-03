
// Background.cpp
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

#include "Background.h"

namespace HoverRace {
namespace Display {

namespace {

double ClampOpacity(double opacity)
{
	if (opacity < 0.0) return 0.0;
	else if (opacity > 1.0) return 1.0;
	else return opacity;
}

}  // namespace

/**
 * Constructor.
 * @param opacity The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
 */
Background::Background(double opacity) :
	opacity(ClampOpacity(opacity))
{
}

Background::~Background()
{
}

/**
 * Set the opacity of the overlay.
 * The value will be clamped to the range 0.0 to 1.0 inclusive.
 * @param opacity The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
 */
void Background::SetOpacity(double opacity)
{
	opacity = ClampOpacity(opacity);

	if (this->opacity != opacity) {
		this->opacity = opacity;
		FireModelUpdate(Props::OPACITY);
	}
}

}  // namespace Display
}  // namespace HoverRace
