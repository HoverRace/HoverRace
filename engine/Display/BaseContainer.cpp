
// BaseContainer.cpp
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

#include "BaseContainer.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor for an unsized container without clipping.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
BaseContainer::BaseContainer(Display &display, uiLayoutFlags_t layoutFlags) :
	BaseContainer(display, Vec2(0, 0), false, layoutFlags)
{
}

/**
 * Constructor for a sized container.
 * @param display The display child elements will be attached to.
 * @param size The size of the container.
 * @param clip Enable (default) or disable clipping of child widgets.
 * @param layoutFlags Optional layout flags.
 */
BaseContainer::BaseContainer(Display &display, const Vec2 &size, bool clip,
                     uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display), size(size), clip(clip),
	opacity(1.0), visible(true)
{
}

/**
 * Resize the container to the minimum size that will fit all of the
 * child elements.
 */
void BaseContainer::ShrinkWrap()
{
	Vec2 max(0, 0);
	for (auto &child : children) {
		auto &c = child->child;
		Vec3 measured = c->Measure();
		measured += c->GetAlignedPos(measured.x, measured.y);
		if (measured.x > max.x) max.x = measured.x;
		if (measured.y > max.y) max.y = measured.y;
	}
	SetSize(max);
}

/**
 * Set the size of the container.
 * @param size The size of the container, where @c x is the width
 *             and @c y is the height.
 */
void BaseContainer::SetSize(const Vec2 &size)
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
void BaseContainer::SetClip(bool clip)
{
	if (this->clip != clip) {
		this->clip = clip;
		FireModelUpdate(Props::CLIP);
	}
}

/**
 * Set the opacity of the container.
 * 
 * The opacity is applied to the container as a whole; the container is first
 * rendered off-screen, then the opacity is applied when drawing the buffer
 * to the screen.
 * 
 * The value will be clamped to the range 0.0 to 1.0 inclusive.
 * 
 * @param opacity The opacity (1.0 is fully-opaque, 0.0 is fully-transparent).
 */
void BaseContainer::SetOpacity(double opacity)
{
	if (opacity < 0.0) opacity = 0.0;
	else if (opacity > 1.0) opacity = 1.0;

	if (this->opacity != opacity) {
		this->opacity = opacity;
		FireModelUpdate(Props::OPACITY);
	}
}

/**
 * Sets whether the widgets in this container are visible.
 * @param visible @c true to show, @c false to hide.
 */
void BaseContainer::SetVisible(bool visible)
{
	if (this->visible != visible) {
		this->visible = visible;
		FireModelUpdate(Props::VISIBLE);
	}
}

}  // namespace Display
}  // namespace HoverRace
