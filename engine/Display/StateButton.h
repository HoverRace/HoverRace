
// StateButton.h
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

#pragma once

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
		class Display;
		class FillBox;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Base class for buttons with state (i.e. checkboxes, radio buttons, etc.).
 * @author Michael Imamura
 */
class MR_DllDeclare StateButton : public Button
{
	using SUPER = Button;

public:
	struct Props
	{
		enum {
			CHECKED = SUPER::Props::NEXT_,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	StateButton(Display &display, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	StateButton(Display &display, const Vec2 &size, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~StateButton();

public:
	bool IsChecked() const { return checked; }
	void SetChecked(bool checked);

protected:
	/**
	 * Sets the icon for a button state.
	 * @param enabled The enabled state.
	 * @param checked The checked state.
	 * @param icon The icon (may be @c nullptr).
	 */
	void SetStateIcon(bool enabled, bool checked,
		std::shared_ptr<FillBox> icon)
	{
		icons[{enabled, checked}] = std::move(icon);
		if (enabled == IsEnabled() && checked == IsChecked()) {
			UpdateIcon();
		}
	}

	/// Update the button's icon based on the current state.
	void UpdateIcon();

public:
	void FireModelUpdate(int prop) override;

private:
	bool checked;

	typedef std::pair<bool, bool> iconsKey_t;  ///< enabled, checked
	typedef std::shared_ptr<FillBox> iconsVal_t;
	typedef std::map<iconsKey_t, iconsVal_t> icons_t;
	icons_t icons;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
