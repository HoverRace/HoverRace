
// ClickRegion.cpp
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

#include "../Control/Action.h"
#include "../Util/Log.h"

#include "ClickRegion.h"

namespace Log = HoverRace::Util::Log;

namespace HoverRace {
namespace Display {

/**
 * Constructor for automatically-sized button.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
ClickRegion::ClickRegion(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display),
	size(0, 0), autoSize(true), needsSizing(true),
	pressed(false)
{
}

/**
 * Constructor for fixed-sized button.
 * @param display The display child elements will be attached to.
 * @param size The fixed button size.
 * @param layoutFlags Optional layout flags.
 */
ClickRegion::ClickRegion(Display &display, const Vec2 &size, uiLayoutFlags_t layoutFlags) :
	SUPER(layoutFlags), display(display),
	size(size), autoSize(false), needsSizing(false),
	pressed(false)
{
}

ClickRegion::~ClickRegion()
{
}

void ClickRegion::OnMouseMoved(const Vec2 &pos)
{
	if (IsEnabled() && TestHit(pos)) {
		//TODO: Set focus.
	}
	else {
		//TODO: Unset focus.
	}
}

void ClickRegion::OnMousePressed(const Control::Mouse::Click &click)
{
	if (IsEnabled() && TestHit(click.pos)) {
		SetPressed(true);
	}
}

void ClickRegion::OnMouseReleased(const Control::Mouse::Click &click)
{
	if (IsPressed() && TestHit(click.pos)) {
		FireClickedSignal();
	}
	SetPressed(false);
}

void ClickRegion::FireClickedSignal()
{
	clickedSignal(*this);
}

/**
 * Retrieve the size of the button.
 * If automatic sizing is enabled, then this will calculate the size
 * immediately (the same caveats as calling Measure()).
 * @return The size, where @c x is the width and @c y is the height.
 */
const Vec2 &ClickRegion::GetSize() const
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
void ClickRegion::SetSize(const Vec2 &size)
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
void ClickRegion::SetAutoSize()
{
	if (!autoSize) {
		autoSize = true;
		needsSizing = true;
		size.x = 0;
		size.y = 0;
		FireModelUpdate(Props::SIZE);
	}
}

void ClickRegion::SetPressed(bool pressed)
{
	if (this->pressed != pressed) {
		this->pressed = pressed;
		FireModelUpdate(Props::PRESSED);
	}
}

void ClickRegion::SetEnabled(bool enabled)
{
	if (this->enabled != enabled) {
		this->enabled = enabled;
		FireModelUpdate(Props::ENABLED);

		// Change back to unpressed and unfocused if disabled.
		if (!enabled) {
			SetPressed(false);
			//TODO: Unset focus.
		}
	}
}

bool ClickRegion::TestHit(const Vec2 &pos) const
{
	const Vec2 &sz = GetSize();
	const Vec2 pui = GetAlignedPos(display.ScreenToUiPosition(pos), -sz.x, -sz.y);
	return
		pui.x >= 0 && pui.x < sz.x &&
		pui.y >= 0 && pui.y < sz.y;
}

}  // namespace Display
}  // namespace HoverRace
