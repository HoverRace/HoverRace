
// HudCell.h
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

namespace HoverRace {
namespace Display {

/// Split-screen HUD grid locations.
enum class HudCell {
	FILL,   ///< Fill the screen.
	N,      ///< Upper half.
	NE,     ///< Upper-right quadrant.
	E,      ///< Right half.
	SE,     ///< Lower-right quadrant.
	S,      ///< Lower half.
	SW,     ///< Lower-left quadrant.
	W,      ///< Left half.
	NW,     ///< Upper-left quadrant.
};

inline std::ostream &operator<<(std::ostream &os, const HudCell cell)
{
	switch (cell) {
		case HudCell::FILL: os << "FILL"; break;
		case HudCell::N:    os << "N"; break;
		case HudCell::NE:   os << "NE"; break;
		case HudCell::E:    os << "E"; break;
		case HudCell::SE:   os << "SE"; break;
		case HudCell::S:    os << "S"; break;
		case HudCell::SW:   os << "SW"; break;
		case HudCell::W:    os << "W"; break;
		case HudCell::NW:   os << "NW"; break;
		default: os << static_cast<int>(cell);
	}
	return os;
}

}  // namespace Display
}  // namespace HoverRace
