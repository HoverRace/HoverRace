
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
 * A radio button.
 * @tparam T The type of the value held by the radio button.
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

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
