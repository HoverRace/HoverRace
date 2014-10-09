
// Player.cpp
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

#include "../StdAfx.h"

#include "Player.h"

namespace HoverRace {
namespace Player {

Player::Player(std::shared_ptr<Profile> profile) :
	profile(std::move(profile)),
	connectionState(ConnectionState::CONNECTING)
{
}

std::ostream &Player::StreamOut(std::ostream &os) const
{
	if (profile) {
		os << profile->GetName();
	}
	else {
		os << "(no profile)";
	}
	return os;
}
	
void Player::SetConnectionState(Player::ConnectionState state)
{
	if (this->connectionState != state) {
		this->connectionState = state;
		//TODO: Fire state changed signal.
	}
}

}  // namespace Player
}  // namespace HoverRace
