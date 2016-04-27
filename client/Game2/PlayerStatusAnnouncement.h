
// PlayerStatusAnnouncement.h
//
// Copyright (c) 2016 Michael Imamura.
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

#include "MessageAnnouncement.h"

namespace HoverRace {
namespace Client {

/**
 * Player status change (join / leave) announcement.
 * @author Michael Imamura
 */
class PlayerStatusAnnouncement : public MessageAnnouncement
{
	using SUPER = MessageAnnouncement;

public:
	struct Status
	{
		enum status_t
		{
			PRESENT,  ///< Player is currently logged in.
			JOINED,  ///< Player has just joined the game.
		};
	};

public:
	PlayerStatusAnnouncement(Status::status_t status,
		std::shared_ptr<Player::Player> target,
		std::shared_ptr<Player::Player> subject);
	virtual ~PlayerStatusAnnouncement() { }

protected:
	static std::string RenderStatusText(Status::status_t status,
		const Player::Player &player);

public:
	std::shared_ptr<Display::Box> CreateIcon(Display::Display &display,
		Display::Container &parent) const override;

private:
	Status::status_t status;
	std::shared_ptr<Player::Player> subject;
};

}  // namespace Client
}  // namespace HoverRace
