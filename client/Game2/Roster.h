
// Roster.h
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
	namespace Player {
		class Player;
	}
}

namespace HoverRace {
namespace Client {

/**
 * A list of players.
 * @author Michael Imamura
 */
class Roster
{
public:
	Roster();

public:
	void AddPlayer(std::shared_ptr<Player::Player> player);

private:
	std::list<std::shared_ptr<Player::Player>> players;
};

}  // namespace Client
}  // namespace HoverRace
