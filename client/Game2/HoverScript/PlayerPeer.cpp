
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

#include "PlayerPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

PlayerPeer::PlayerPeer(Script::Core *scripting,
                       MainCharacter::MainCharacter *player) :
	SUPER(scripting, "Player"),
	player(player),
	props(luabind::newtable(scripting->GetState())),
	onStart(scripting), onFinish(scripting), onFinishLine(scripting)
{
	finishLineConn = player->GetFinishLineSignal().connect(
		[&](MainCharacter::MainCharacter*) {
			onFinishLine.CallHandlers();
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
			.def("on_start", &PlayerPeer::LOnStart)
			.def("on_start", &PlayerPeer::LOnStart_N)
			.def("on_finish", &PlayerPeer::LOnFinish)
			.def("on_finish", &PlayerPeer::LOnFinish_N)
			.def("on_finish_line", &PlayerPeer::LOnFinishLine)
			.def("on_finish_line", &PlayerPeer::LOnFinishLine_N)
			.property("props", &PlayerPeer::props)
	];
}

void PlayerPeer::SetHud(std::shared_ptr<HudPeer> hud)
{
	this->hud = std::move(hud);
}

void PlayerPeer::OnStart()
{
	onStart.CallHandlers();
}

void PlayerPeer::OnFinish()
{
	onFinish.CallHandlers();
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

void PlayerPeer::LOnStart(const luabind::object &fn)
{
	onStart.AddHandler(fn);
}

void PlayerPeer::LOnStart_N(const std::string &name, const luabind::object &fn)
{
	onStart.AddHandler(name, fn);
}

void PlayerPeer::LOnFinish(const luabind::object &fn)
{
	onFinish.AddHandler(fn);
}

void PlayerPeer::LOnFinish_N(const std::string &name, const luabind::object &fn)
{
	onFinish.AddHandler(name, fn);
}

void PlayerPeer::LOnFinishLine(const luabind::object &fn)
{
	onFinishLine.AddHandler(fn);
}

void PlayerPeer::LOnFinishLine_N(const std::string &name, const luabind::object &fn)
{
	onFinishLine.AddHandler(name, fn);
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
