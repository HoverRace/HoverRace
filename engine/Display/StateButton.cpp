
// StateButton.cpp
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

#include "SymbolIcon.h"

#include "StateButton.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

/**
 * Constructor for automatically-sized button.
 * @param display The display child elements will be attached to.
 * @param text The text for the button label.
 * @param layoutFlags Optional layout flags.
 */
StateButton::StateButton(Display &display, const std::string &text,
	uiLayoutFlags_t layoutFlags) :
	SUPER(display, text, layoutFlags),
	checked(false)
{
}

/**
 * Constructor for fixed-sized button.
 * @param display The display child elements will be attached to.
 * @param size The fixed button size.
 * @param text The text for the button label.
 * @param layoutFlags Optional layout flags.
 */
StateButton::StateButton(Display &display, const Vec2 &size,
	const std::string &text, uiLayoutFlags_t layoutFlags) :
	SUPER(display, size, text, layoutFlags),
	checked(false)
{
}

StateButton::~StateButton()
{
}

/**
 * Set the button state.
 * @param checked @c true for checked, @c false for unchecked.
 */
void StateButton::SetChecked(bool checked)
{
	if (this->checked != checked) {
		this->checked = checked;
		FireModelUpdate(Props::CHECKED);
		UpdateIcon();
	}
}

/**
 * Set the appropriate icon for the current widget state.
 */
void StateButton::UpdateIcon()
{
	SetIcon(icons[{IsEnabled(), IsChecked()}]);
}

void StateButton::FireModelUpdate(int prop)
{
	if (prop == ClickRegion::Props::ENABLED) {
		UpdateIcon();
	}

	SUPER::FireModelUpdate(prop);
}

}  // namespace Display
}  // namespace HoverRace
