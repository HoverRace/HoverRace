
// PlayerBar.cpp
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
#include "../../engine/Util/Log.h"

#include "GameDirector.h"
#include "PlayerStatusAnnouncement.h"
#include "Roster.h"

#include "PlayerBar.h"

namespace HoverRace {
namespace Client {

PlayerBar::PlayerBar(Display::Display &display, GameDirector &director,
	Display::uiLayoutFlags_t layoutFlags) :
	SUPER(display, layoutFlags),
	director(director)
{
	playerAddedConn =
		director.GetParty()->GetPlayerAddedSignal().connect(
			std::bind(&PlayerBar::OnPlayerAdded, this,
				std::placeholders::_1));
}

PlayerBar::~PlayerBar()
{
}

/**
 * Play animation to remind the user who is logged-in.
 */
void PlayerBar::PresentPlayers()
{
	director.GetParty()->ForEach([=](std::shared_ptr<Player::Player> &player) {
		HR_LOG(info) << "Active player: " << *player;

		director.RequestAnnouncement(
			std::make_shared<PlayerStatusAnnouncement>(
				PlayerStatusAnnouncement::Status::PRESENT, player, player));
	});
}

void PlayerBar::OnPlayerAdded(std::shared_ptr<Player::Player> player)
{
	HR_LOG(info) << "Player added: " << *player;

	director.RequestAnnouncement(
		std::make_shared<PlayerStatusAnnouncement>(
			PlayerStatusAnnouncement::Status::JOINED, player, player));
}

}  // namespace Client
}  // namespace HoverRace
