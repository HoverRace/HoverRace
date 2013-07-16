
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

#include "Button.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor for automatically-sized button.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
Button::Button(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display),
	size(0, 0), autoSize(true), needsSizing(true)
{
}

/**
 * Constructor for fixed-sized button.
 * @param display The display child elements will be attached to.
 * @param size The fixed button size.
 * @param layoutFlags Optional layout flags.
 */
Button::Button(Display &display, const Vec2 &size, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display),
	size(size), autoSize(false), needsSizing(false)
{
}

Button::~Button()
{
}

void Button::OnMouseMoved(const Vec2 &pos)
{
	//TODO: Check hitbox.
	//TODO: Set focus.
}

void Button::OnMousePressed(const Control::Mouse::Click &click)
{
	//TODO: Check hitbox.
	//TODO: Set pressed state.
}

void Button::OnMouseReleased(const Control::Mouse::Click &click)
{
	//TODO: Check hitbox.
	//TODO: Unset pressed state.
	//TODO: Call OnPressed listeners.
}

/**
 * Retrieve the size of the button.
 * If automatic sizing is enabled, then this will calculate the size
 * immediately (the same caveats as calling Measure()).
 * @return The size, where @c x is the width and @c y is the height.
 */
const Vec2 &Button::GetSize() const
{
	if (needsSizing) {
		Vec3 calcSize = Measure();
		size.x = calcSize.x;
		size.y = calcSize.y;
		needsSizing = false;
	}
	return size;
}

/**
 * Set the button to a fixed size.
 * @param size The size of the container, where @c x is the width
 *             and @c y is the height.
 * @see SetAutoSize()
 */
void Button::SetSize(const Vec2 &size)
{
	if (this->size != size) {
		this->size = size;
		autoSize = false;
		needsSizing = false;
		FireModelUpdate(Props::SIZE);
	}
}

/**
 * Enable automatic sizing.
 * The size of the button will be determined by the contents.
 * To set a fixed size, call SetSize(const Vec2&).
 */
void Button::SetAutoSize()
{
	if (!autoSize) {
		autoSize = true;
		needsSizing = true;
		size.x = 0;
		size.y = 0;
		FireModelUpdate(Props::SIZE);
	}
}

}  // namespace Display
}  // namespace HoverRace
