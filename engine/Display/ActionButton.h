
// ActionButton.h
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

#pragma once

#include "../Control/Controller.h"

#include "Button.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Display {

/**
 * A button that's bound to a controller action.
 *
 * This button gets its text from the hotkey and label of the attached action.
 *
 * When the button is clicked, then it fires both the "clicked" signal as well
 * as the attached action.  That way, if the scene is already listening to the
 * action, it doesn't need to explicitly set the clicked signal on the button.
 *
 * @author Michael Imamura
 */
class MR_DllDeclare ActionButton : public Button
{
	typedef Button SUPER;
	public:
		ActionButton(Display &display, uiLayoutFlags_t layoutFlags=0);
		ActionButton(Display &display, const Vec2 &size,
			uiLayoutFlags_t layoutFlags=0);
		virtual ~ActionButton();

	public:
		void AttachAction(Control::InputEventController &controller,
			Control::InputEventController::VoidActionPtr action);
		void DetachAction();

	protected:
		virtual void FireClickedSignal();

	private:
		std::weak_ptr<Control::InputEventController::VoidActionPtr::element_type> action;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
