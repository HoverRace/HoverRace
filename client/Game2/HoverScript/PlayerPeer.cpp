
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

#include "StdAfx.h"

#include "../../../engine/MainCharacter/MainCharacter.h"
#include "../../../engine/Script/Core.h"

#include "MetaPlayer.h"

#include "PlayerPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

PlayerPeer::PlayerPeer(Script::Core *scripting,
                       MainCharacter::MainCharacter *player) :
	SUPER(scripting, "Player"),
	player(player), meta(nullptr),
	props(luabind::newtable(scripting->GetState()))
{
	finishLineConn = player->GetFinishLineSignal().connect(
		[&](MainCharacter::MainCharacter*) {
			if (meta) {
				meta->OnFinishLine();
			}
		});
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
			.def("finish", &PlayerPeer::LFinish)
			.def_readonly("fuel", &PlayerPeer::LGetFuel)
			.def("get_pos", &PlayerPeer::LGetPos)
			.def_readonly("hud", &PlayerPeer::LGetHud)
			.def_readonly("index", &PlayerPeer::LGetIndex)
			.def_readonly("name", &PlayerPeer::LGetName)
			.property("props", &PlayerPeer::props)
	];
}

void PlayerPeer::SetHud(std::shared_ptr<HudPeer> hud)
{
	this->hud = std::move(hud);
}

void PlayerPeer::LFinish()
{
	player->Finish();
}

double PlayerPeer::LGetFuel()
{
	return player->GetFuelLevel();
}

std::shared_ptr<HudPeer> PlayerPeer::LGetHud()
{
	return hud;
}

int PlayerPeer::LGetIndex()
{
	return player->GetPlayerIndex();
}

const std::string &PlayerPeer::LGetName()
{
	return player->GetName();
}

void PlayerPeer::LGetPos()
{
	MR_3DCoordinate &pos = player->mPosition;

	lua_State *L = GetScripting()->GetState();
	lua_pushnumber(L, pos.mX);
	lua_pushnumber(L, pos.mY);
	lua_pushnumber(L, pos.mZ);
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
