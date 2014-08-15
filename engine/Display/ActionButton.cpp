
// ActionButton.cpp
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

#include "../StdAfx.h"

#include "ActionButton.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor for automatically-sized button.
 * @note The button will have no text until an action is attached.
 * @param display The display child elements will be attached to.
 * @param layoutFlags Optional layout flags.
 */
ActionButton::ActionButton(Display &display, uiLayoutFlags_t layoutFlags) :
	SUPER(display, "", layoutFlags)
{
}

/**
 * Constructor for fixed-sized button.
 * @note The button will have no text until an action is attached.
 * @param display The display child elements will be attached to.
 * @param size The fixed button size.
 * @param layoutFlags Optional layout flags.
 */
ActionButton::ActionButton(Display &display, const Vec2 &size,
                           uiLayoutFlags_t layoutFlags) :
	SUPER(display, size, "", layoutFlags)
{
}

ActionButton::~ActionButton()
{
}

/**
 * Attach an action to the button.
 * This updates the button's text with the name and hotkey of the action.
 * Clicking on the button will fire the attached action.
 * This is typically called from Client::Scene::AttachController.
 * @param controller The controller that owns the action.
 * @param action The action to attach.
 */
void ActionButton::AttachAction(Control::InputEventController &controller,
	Control::InputEventController::VoidActionPtr action)
{
	this->action = action;

	std::ostringstream oss;
	oss << '[' << controller.HashToString(action->GetPrimaryTrigger()) <<
		"] " << action->GetName();
	SetText(oss.str());
}

/**
 * Detach the currently-attached action (if any).
 * @note It is usually unnecessary to call this from
 *       Client::Scene::DetachController since this class maintains a weak
 *       reference to the attached action.
 */
void ActionButton::DetachAction()
{
	action.reset();
}

void ActionButton::FireClickedSignal()
{
	SUPER::FireClickedSignal();
	if (auto actionLock = action.lock()) {
		(*actionLock)(1);
	}
}

}  // namespace Display
}  // namespace HoverRace
