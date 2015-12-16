
// ActiveText.cpp
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

#include "ActiveText.h"

namespace HoverRace {
namespace Display {

/**
 * Constructor.
 * @param text The text.
 * @param font The font.
 * @param color The text foreground color (including alpha).
 * @param layoutFlags Optional layout flags.
 */
ActiveText::ActiveText(const std::string &text,
	const UiFont &font,
	Color color,
	uiLayoutFlags_t layoutFlags) :
	SUPER(text, font, color, layoutFlags),
	caretVisible(false), caretPos(0)
{
}

/**
 * Set the visibility of the caret.
 * @param visible @c true if visible, @c false otherwise.
 */
void ActiveText::SetCaretVisible(bool visible)
{
	if (caretVisible != visible) {
		caretVisible = visible;
		FireModelUpdate(Props::CARET_VISIBLE);
	}
}

/**
 * Set the caret position.
 * @param pos The position (not clamped to the length of the text).
 * @see SetCaretVisible(bool)
 */
void ActiveText::SetCaretPos(size_t pos)
{
	if (caretPos != pos) {
		caretPos = pos;
		FireModelUpdate(Props::CARET_POS);
	}
}

}  // namespace Display
}  // namespace HoverRace
