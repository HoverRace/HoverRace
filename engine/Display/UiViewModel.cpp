
// UiViewModel.cpp
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

#include "StdAfx.h"

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
 * Set the alignment of the component.
 * @param alignment The alignment (see Alignment).
 */
void UiViewModel::SetAlignment(Alignment::alignment_t alignment)
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
				boost::lexical_cast<std::string>(alignment));
	}
}

}  // namespace Display
}  // namespace HoverRace
