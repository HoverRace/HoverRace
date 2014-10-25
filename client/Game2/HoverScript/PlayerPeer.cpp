
// PlayerPeer.cpp
//
// Copyright (c) 2010, 2014 Michael Imamura.
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

#include "../../StdAfx.h"

#include <luabind/operator.hpp>

#include "../../../engine/MainCharacter/MainCharacter.h"
#include "../../../engine/Player/Player.h"
#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/Str.h"

#include "MetaPlayer.h"

#include "PlayerPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

std::ostream &operator<<(std::ostream &os, const PlayerPeer &playerPeer)
{
	os << "PlayerPeer{index=" << playerPeer.LGetIndex() << ", "
		"name=\"" << Str::Lua(playerPeer.LGetName()) << "\"}";
	return os;
}


PlayerPeer::PlayerPeer(Script::Core *scripting,
	std::shared_ptr<Player::Player> player) :
	SUPER(scripting, "Player"),
	player(std::move(player)), meta(nullptr)
{
}

PlayerPeer::~PlayerPeer()
{
}

/**
 * Register this peer in an environment.
 */
void PlayerPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L) [
		class_<PlayerPeer, SUPER, std::shared_ptr<PlayerPeer>>("Player")
			.def(tostring(self))
			.def("finish", &PlayerPeer::LFinish)
			.def("get_pos", &PlayerPeer::LGetPos)
			.property("fuel", &PlayerPeer::LGetFuel)
			.property("hud", &PlayerPeer::LGetHud)
			.property("index", &PlayerPeer::LGetIndex)
			.property("name", &PlayerPeer::LGetName)
	];
}

void PlayerPeer::SetHud(std::shared_ptr<HudPeer> hud)
{
	this->hud = std::move(hud);
}

MainCharacter::MainCharacter *PlayerPeer::VerifyAttached() const
{
	auto mchar = player->GetMainCharacter();
	if (!mchar) {
		luaL_error(GetScripting()->GetState(),
			"Player is not attached to the current session.");
	}
	return mchar;
}

void PlayerPeer::LFinish()
{
	if (auto mchar = VerifyAttached()) {
		mchar->Finish();
	}
}

double PlayerPeer::LGetFuel() const
{
	if (auto mchar = VerifyAttached()) {
		return mchar->GetFuelLevel();
	}
	else {
		return 0.0;
	}
}

std::shared_ptr<HudPeer> PlayerPeer::LGetHud() const
{
	return hud;
}

int PlayerPeer::LGetIndex() const
{
	if (auto mchar = VerifyAttached()) {
		return mchar->GetPlayerIndex();
	}
	else {
		return -1;
	}
}

const std::string &PlayerPeer::LGetName() const
{
	return player->GetName();
}

void PlayerPeer::LGetPos()
{
	lua_State *L = GetScripting()->GetState();

	if (auto mchar = VerifyAttached()) {
		MR_3DCoordinate &pos = mchar->mPosition;
		lua_pushnumber(L, pos.mX);
		lua_pushnumber(L, pos.mY);
		lua_pushnumber(L, pos.mZ);
	}
	else {
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
	}
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
