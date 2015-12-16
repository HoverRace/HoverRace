
// ActiveText.h
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

#include "BaseText.h"

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
 * A text widget where the contents may change often (i.e., updated every few
 * frames, or even every frame).
 *
 * Typically, this means that instead of rendering the text as a single
 * texture, with all the formatting advantages that brings, the text is
 * rendered glyph-by-glyph to avoid creating and destroying textures often.
 *
 * @author Michael Imamura
 */
class ActiveText : public BaseText
{
	using SUPER = BaseText;

public:
	struct Props
	{
		enum
		{
			CARET_VISIBLE = SUPER::Props::NEXT_,
			CARET_POS,
			NEXT_,  ///< First index for subclasses.
		};
	};

public:
	ActiveText(const std::string &text, const UiFont &font, const Color color,
		uiLayoutFlags_t layoutFlags = 0);
	virtual ~ActiveText() { }

public:
	void AttachView(Display &disp) override { AttachViewDynamic(disp, this); }

public:
	/**
	 * Check if the caret is visible.
	 * @return @c true if visible, @c false if hidden.
	 */
	bool IsCaretVisible() const { return caretVisible; }
	void SetCaretVisible(bool visible);

	/**
	 * Retrieve the position of the caret.
	 * @return The caret position (may be beyond the end of the text).
	 */
	size_t GetCaretPos() const { return caretPos; }
	void SetCaretPos(size_t pos);

private:
	bool caretVisible;
	size_t caretPos;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
