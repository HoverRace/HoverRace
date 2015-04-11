
// BaseText.h
//
// Copyright (c) 2015 Michael Imamura.
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

#include "Color.h"
#include "UiFont.h"

#include "UiViewModel.h"

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
 * Base class for widgets that display text.
 * @author Michael Imamura
 */
class BaseText : public UiViewModel
{
	using SUPER = UiViewModel;

public:
	struct Props
	{
		enum
		{
			COLOR = SUPER::Props::NEXT_,
			FONT,
			TEXT,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	BaseText(const std::string &text, const UiFont &font, const Color color,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~BaseText() { }

public:
	const Color GetColor() const { return color; }
	void SetColor(const Color color);

	const UiFont &GetFont() const { return font; }
	void SetFont(const UiFont &font);

	const std::string &GetText() const { return text; }
	void SetText(const std::string &text);

private:
	std::string text;
	UiFont font;
	Color color;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
