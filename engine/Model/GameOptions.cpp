
// GameOptions.cpp
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

#include "Level.h"

#include "GameOptions.h"

namespace HoverRace {
namespace Model {

/**
 * Pick a random craft from the list of all crafts.
 * @return A craft.
 */
GameOptions::Craft GameOptions::PickRandomCraft()
{
	return static_cast<Craft>(rand() % 4);
}

/**
 * Converts this instance to legacy "gameOpts" flags.
 * @return This instance as a char.
 */
char GameOptions::ToFlags() const
{
	char retv = 0;

	if (weaponsEnabled) retv |= OPT_ALLOW_WEAPONS;

	if (IsCraftAllowed(Craft::EON)) retv |= OPT_ALLOW_EON;
	if (IsCraftAllowed(Craft::CX)) retv |= OPT_ALLOW_CX;
	if (IsCraftAllowed(Craft::BI)) retv |= OPT_ALLOW_BI;
	if (IsCraftAllowed(Craft::BASIC)) retv |= OPT_ALLOW_BASIC;

	if (IsObjectAllowed(152)) retv |= OPT_ALLOW_CANS;
	if (IsObjectAllowed(151)) retv |= OPT_ALLOW_MINES;

	return retv;
}

}  // namespace Model
}  // namespace HoverRace
