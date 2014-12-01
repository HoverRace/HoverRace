
// RadioButton.h
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

#include "StateButton.h"

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
	}
}

namespace HoverRace {
namespace Display {

/**
 * Base class for radio buttons.
 * @author Michael Imamura
 */
class MR_DllDeclare RadioButtonBase : public StateButton
{
	using SUPER = StateButton;

public:
	RadioButtonBase(Display &display, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	RadioButtonBase(Display &display, const Vec2 &size, const std::string &text,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~RadioButtonBase() { }

private:
	void Init();
	void InitIcon(bool enabled, bool checked);
};

/**
 * A single radio button.
 *
 * To be useful, a radio button must be added to a RadioGroup.
 * The selected item and value should be managed by the group rather than
 * calling individual buttons.
 *
 * @tparam T The type of the value held by the radio button.
 * @author Michael Imamura
 */
template<class T>
class RadioButton : public RadioButtonBase
{
	using SUPER = RadioButtonBase;

public:
	RadioButton(Display &display, const std::string &text, const T &value,
		uiLayoutFlags_t layoutFlags = 0) :
		SUPER(display, text, layoutFlags), value(value) { }
	RadioButton(Display &display, const Vec2 &size, const std::string &text,
		const T &value, uiLayoutFlags_t layoutFlags = 0) :
		SUPER(display, size, text, layoutFlags), value(value) { }
	virtual ~RadioButton() { }

public:
	const T &GetValue() const { return value; }

private:
	T value;
};

/**
 * A group of radio buttons.
 * @tparam T The type of the values held by the radio buttons.
 * @author Michael Imamura
 */
template<class T>
class RadioGroup
{
public:
	RadioGroup() : selButton(nullptr) { }

public:
	using button_t = RadioButton<T>;

private:
	void SetSelectedButton(button_t *sel)
	{
		if (selButton != sel) {
			if (selButton) {
				selButton->SetChecked(false);
			}
			selButton = sel;
			sel->SetChecked(true);

			valueChangedSignal();
		}
	}

public:
	/**
	 * Add a radio button to the group.
	 * @param button The button to add (may not be @c nullptr).
	 */
	void Add(std::shared_ptr<button_t> button)
	{
		button_t *addBtn = button.get();

		clickedConns.emplace_back(new boost::signals2::scoped_connection(
			button->GetClickedSignal().connect([=](ClickRegion&) {
				SetSelectedButton(addBtn);
			})));

		if (!selButton) {
			selButton = button.get();
			selButton->SetChecked(true);
		}

		buttons.emplace_back(button);
	}

	const T &GetValue() const
	{
		if (!selButton) throw Exception("No selected value");
		return selButton->GetValue();
	}

	/**
	 * Set the selected radio button by value.
	 *
	 * If none of the radio buttons in this group match the value, then the
	 * selection will stay unchanged.
	 *
	 * @param val The value.
	 */
	void SetValue(const T &val)
	{
		// Find the first radio button matching this value.
		// We assume that the list is small enough that we don't need a
		// lookup table.
		button_t *newSel = nullptr;
		for (const auto &btn : buttons) {
			if (btn->GetValue() == val) {
				newSel = btn.get();
				break;
			}
		}

		if (newSel && selButton != newSel) {
			selButton->SetChecked(false);
			selButton = newSel;
			newSel->SetChecked(true);
		}
	}

public:
	using valueChangedSignal_t = boost::signals2::signal<void()>;
	valueChangedSignal_t &GetValueChangedSignal() { return valueChangedSignal; }

private:
	std::vector<std::shared_ptr<button_t>> buttons;
	std::vector<std::unique_ptr<boost::signals2::scoped_connection>> clickedConns;
	button_t *selButton;
	valueChangedSignal_t valueChangedSignal;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
