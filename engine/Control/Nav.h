
// Nav.h
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
namespace Control {

/**
 * A navigation direction.
 *
 * This is a placeholder for future support for analog directional controls.
 * For now, only digital (cardinal directions) are supported.
 *
 * @author Michael Imamura
 */
class MR_DllDeclare Nav
{
public:
	enum dir_t { NEUTRAL, UP, DOWN, LEFT, RIGHT, NEXT, PREV };

public:
	Nav(dir_t dir) : dir(dir) { }
	Nav(const Nav&) = default;
	Nav(Nav&&) = default;

	Nav &operator=(const Nav&) = default;
	Nav &operator=(Nav&&) = default;

public:
	/**
	 * Converts this direction into one of the cardinal directions.
	 * @return The direction.
	 */
	dir_t AsDigital() const { return dir; }
	
private:
	dir_t dir;
};

inline std::ostream &operator<<(std::ostream &os, const Nav &nav)
{
	switch (nav.AsDigital()) {
		case Nav::NEUTRAL: os << "NEUTRAL"; break;
		case Nav::UP: os << "UP"; break;
		case Nav::DOWN: os << "DOWN"; break;
		case Nav::LEFT: os << "LEFT"; break;
		case Nav::RIGHT: os << "RIGHT"; break;
		case Nav::NEXT: os << "NEXT"; break;
		case Nav::PREV: os << "PREV"; break;
		default: os << static_cast<int>(nav.AsDigital());
	}
	return os;
}

} // namespace Control
} // namespace HoverRace

#undef MR_DllDeclare
