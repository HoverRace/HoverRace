
// SessionPeer.cpp
//
// Copyright (c) 2010, 2013, 2014 Michael Imamura.
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

#include <luabind/operator.hpp>

#include "../../engine/Script/Core.h"
#include "../../engine/Util/Clock.h"
#include "../../engine/Util/Log.h"
#include "../ClientSession.h"
#include "../Rulebook.h"
#include "../Rules.h"
#include "MetaPlayer.h"
#include "PlayerPeer.h"
#include "TrackPeer.h"

#include "SessionPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

std::ostream &operator<<(std::ostream &os, const SessionPeer &sessionPeer)
{
	os << "SessionPeer";
	return os;
}

SessionPeer::SessionPeer(Script::Core *scripting, ClientSession *session) :
	SUPER(scripting, "Session"), session(session), meta(nullptr),
	rules(),
	players()
{
	if (session) {
		OnSessionStart(session);
	}
}

SessionPeer::~SessionPeer()
{
}

/**
 * Register this peer in an environment.
 */
void SessionPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L) [
		class_<SessionPeer,SUPER,std::shared_ptr<SessionPeer>>("Session")
			.def(tostring(self))
			.def("countdown_to_next_phase", &SessionPeer::LCountdownToNextPhase)
			.def("get_num_players", &SessionPeer::LGetNumPlayers)
			.def_readonly("players", &SessionPeer::players)
			.def_readonly("rules", &SessionPeer::rules)
			.property("clock", &SessionPeer::LGetClock)
			.property("countdown", &SessionPeer::LGetCountdown)
			.property("time", &SessionPeer::LGetTime)
			.property("track", &SessionPeer::LGetTrack)
	];
}

/**
 * Attach a session.
 * @param session The session to attach (may be @c nullptr).
 */
void SessionPeer::OnSessionStart(ClientSession *session)
{
	if (!session) {
		OnSessionEnd();
		return;
	}
	this->session = session;

	auto scripting = GetScripting();

	rules = session->GetRules()->GetRules();
	players = luabind::newtable(scripting->GetState());

	playerRefs.clear();
	for (int i = 0; i < session->GetNbPlayers(); i++) {
		auto player = session->GetRules()->GetRulebook()->GetMetas().player(
			std::make_shared<PlayerPeer>(scripting, session->GetPlayer(i)));
		player->OnInit();

		playerRefs.push_back(player);
		players[i] = player;
	}
}

/**
 * Signal that the session has ended.
 * This will detach this peer from the session.
 */
void SessionPeer::OnSessionEnd()
{
	session = nullptr;
	rules = luabind::object();
	players = luabind::object();
	playerRefs.clear();
}

/**
 * Verify that the session is still active.
 * If the session is no longer active, then a Lua error will be triggered and
 * this function will not return.
 */
void SessionPeer::VerifySession() const
{
	if (!session) {
		luaL_error(GetScripting()->GetState(), "Session has ended.");
	}
}

int SessionPeer::LGetNumPlayers() const
{
	VerifySession();
	return session->GetNbPlayers();
}

MR_SimulationTime SessionPeer::LGetTime() const
{
	VerifySession();
	return session->GetSimulationTime();
}

std::shared_ptr<Util::Clock> SessionPeer::LGetClock() const
{
	VerifySession();
	return session->GetClock();
}

std::shared_ptr<Util::Clock> SessionPeer::LGetCountdown() const
{
	VerifySession();
	return session->GetCountdown();
}

void SessionPeer::LCountdownToNextPhase(const std::string &s) const
{
	VerifySession();
	session->CountdownToNextPhase(s);
}

std::shared_ptr<TrackPeer> SessionPeer::LGetTrack() const
{
	VerifySession();
	return session->GetTrackPeer();
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
