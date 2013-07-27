
// Container.cpp
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

#include "Container.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor for an unsized container without clipping.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
Container::Container(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display), size(0, 0), clip(false)
{
}

/**
 * Constructor for a sized container.
 * @param display The display child elements will be attached to.
 * @param size The size of the container.
 * @param clip Enable (default) or disable clipping of child widgets.
 * @param layoutFlags Optional layout flags.
 */
Container::Container(Display &display, const Vec2 &size, bool clip,
                     uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display), size(size), clip(clip)
{
}

/**
 * Set the size of the container.
 * @param size The size of the container, where @c x is the width
 *             and @c y is the height.
 */
void Container::SetSize(const Vec2 &size)
{
	if (this->size != size) {
		this->size = size;
		FireModelUpdate(Props::SIZE);
	}
}

/**
 * Set whether child elements are clipped to the container bounds.
 * @param clip @c true if clipping is enabled, @c false if elements are
 *             allowed to render outside of the bounds.
 */
void Container::SetClip(bool clip)
{
	if (this->clip != clip) {
		this->clip = clip;
		FireModelUpdate(Props::CLIP);
	}
}

}  // namespace Display
}  // namespace HoverRace
