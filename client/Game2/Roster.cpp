
// Roster.cpp
//
// Copyright (c) 2014, 2016 Michael Imamura.
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

#include "../../engine/Player/Player.h"
#include "../../engine/Exception.h"

#include "Roster.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

Roster::Roster()
{
}

void Roster::AddPlayer(std::shared_ptr<Player::Player> player)
{
	if (!player) {
		throw Exception("Attempted to add a null player.");
	}

	players.emplace_back(player);
	playerAddedSignal(player);
}

/**
 * Retrieve just the first player in the roster.
 * @return The player (or @c nullptr if roster is empty).
 */
std::shared_ptr<Player::Player> Roster::ShareFirst() const
{
	if (players.empty()) return {};

	return players.front();
}

}  // namespace Client
}  // namespace HoverRace
