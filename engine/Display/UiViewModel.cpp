
// UiViewModel.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../Exception.h"

#include "UiViewModel.h"

namespace HoverRace {
namespace Display {

/**
 * Set the position of the component.
 * @param pos The position, relative to the container (if any).
 */
void UiViewModel::SetPos(const Vec2 &pos)
{
	if (this->pos != pos) {
		this->pos = pos;
		FireModelUpdate(Props::POS);
	}
}

/**
 * Sets the position translation.
 *
 * The translation allows the position to be adjusted (e.g. for animations)
 * without adjusting the actual position.
 *
 * @param translation The translation.
 */
void UiViewModel::SetTranslation(const Vec2 &translation)
{
	if (this->translation != translation) {
		this->translation = translation;
		// As far as the view is concerned, the translation is part of the
		// position.
		FireModelUpdate(Props::POS);
	}
}

/**
 * Set the alignment of the component.
 * @param alignment The alignment (see Alignment).
 */
void UiViewModel::SetAlignment(Alignment alignment)
{
	if (this->alignment != alignment) {
		this->alignment = alignment;
		FireModelUpdate(Props::ALIGNMENT);
	}
}

/**
 * Retrieve an arbitrary position adjusted by the current alignment.
 * @param pos The position to adjust.
 * @param w The width of the component.
 * @param h The height of the component.
 * @return The adjusted position.
 */
Vec2 UiViewModel::GetAlignedPos(const Vec2 &pos, double w, double h) const
{
	switch (alignment) {
		case Alignment::NW: return pos;
		case Alignment::N: return Vec2(pos.x - (w / 2.0), pos.y);
		case Alignment::NE: return Vec2(pos.x - w, pos.y);
		case Alignment::E: return Vec2(pos.x - w, pos.y - (h / 2.0));
		case Alignment::SE: return Vec2(pos.x - w, pos.y - h);
		case Alignment::S: return Vec2(pos.x - (w / 2.0), pos.y - h);
		case Alignment::SW: return Vec2(pos.x, pos.y - h);
		case Alignment::W: return Vec2(pos.x, pos.y - (h / 2.0));
		case Alignment::CENTER: return Vec2(pos.x - (w / 2.0), pos.y - (h / 2.0));
		default:
			throw Exception("Unknown alignment: " +
				boost::lexical_cast<std::string>(static_cast<int>(alignment)));
	}
}

/**
 * Set the focused state.
 * @param focused @c true if focused, @c false otherwise.
 */
void UiViewModel::SetFocused(bool focused)
{
	if (this->focused != focused) {
		this->focused = focused;
		FireModelUpdate(Props::FOCUSED);
	}
}

/**
 * Attempt to hand over the input focus to this widget.
 *
 * The request may not be successful, possibly leaving no widget focused.
 */
void UiViewModel::RequestFocus()
{
	if (!focused) {
		focusRequestedSignal(*this);
	}
}

/**
 * Voluntarily give up input focus, passing it on to the next widget.
 * @param nav The direction in which focus should shift, if possible.
 */
void UiViewModel::RelinquishFocus(const Control::Nav &nav)
{
	if (focused) {
		focusRelinquishedSignal(*this, nav);
	}
}

}  // namespace Display
}  // namespace HoverRace
