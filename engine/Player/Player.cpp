
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

namespace {

std::string NameForProfile(Profile *profile)
{
	if (profile) {
		return profile->GetName();
	}
	else {
		return "(no profile)";
	}
}

}  // namespace

Player::Player(std::shared_ptr<Profile> profile) :
	name(NameForProfile(profile.get())),
	profile(std::move(profile)),
	connectionState(ConnectionState::CONNECTING),
	mainCharacter(nullptr)
{
}

std::ostream &Player::StreamOut(std::ostream &os) const
{
	os << name;
	return os;
}

/**
 * Sets the suffix on the display name.
 *
 * The suffix is used to disambiguate the player name when multiple
 * players have the same name (e.g. they are sharing the same profile).
 *
 * @param suffix The suffix (may be empty for no suffix).
 */
void Player::SetNameSuffix(const std::string &suffix)
{
	std::string newName = NameForProfile(profile.get());
	if (!suffix.empty()) {
		newName += ' ';
		newName += suffix;
	}

	if (name != newName) {
		name = std::move(newName);
		nameChangedSignal();
	}
}

void Player::SetConnectionState(Player::ConnectionState state)
{
	if (this->connectionState != state) {
		this->connectionState = state;
		connectionStateChangedSignal();
	}
}

}  // namespace Player
}  // namespace HoverRace
