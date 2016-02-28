
// PlayerBar.h
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

#pragma once

#include "../../engine/Display/Container.h"

namespace HoverRace {
	namespace Client {
		class GameDirector;
	}
	namespace Display {
		class Display;
	}
	namespace Player {
		class Player;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Sidebar overlay that lists the connected players.
 * @author Michael Imamura
 */
class PlayerBar : public Display::Container
{
	typedef Display::Container SUPER;
public:
	PlayerBar(Display::Display &display, GameDirector &director,
		Display::uiLayoutFlags_t layoutFlags = 0);
	virtual ~PlayerBar();

public:
	void PresentPlayers();

private:
	void OnPlayerAdded(std::shared_ptr<Player::Player> player);

private:
	GameDirector &director;
	boost::signals2::scoped_connection playerAddedConn;
};

}  // namespace Client
}  // namespace HoverRace
