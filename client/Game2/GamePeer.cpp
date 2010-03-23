
// GamePeer.cpp
// Scripting peer for system-level control of the game.
//
// Copyright (c) 2010 Michael Imamura.
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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <lua.hpp>
#include <luabind/adopt_policy.hpp>

#include "../../engine/Script/Core.h"
#include "../../engine/Util/Config.h"
#include "ConfigPeer.h"
#ifdef _WIN32
#	include "GameApp.h"
#endif
#include "Rulebook.h"
#include "SessionPeer.h"

#include "GamePeer.h"

using HoverRace::Util::Config;

namespace HoverRace {
namespace Client {

GamePeer::GamePeer(Script::Core *scripting, MR_GameApp *gameApp) :
	scripting(scripting), gameApp(gameApp), initialized(false),
	onInit(scripting), onSessionStart(scripting)
{
}

GamePeer::~GamePeer()
{
}

/**
 * Register this peer in an environment.
 */
void GamePeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	open(L);

	module(L) [
		class_<GamePeer>("Game")
			.def("is_initialized", &GamePeer::LIsInitialized)
			.def("get_config", &GamePeer::LGetConfig, adopt(result))
			.def("on_init", (void(GamePeer::*)(const luabind::object&))&GamePeer::LOnInit)
			.def("on_init", (void(GamePeer::*)(const std::string&, const luabind::object&))&GamePeer::LOnInit)
			.def("on_session_start", (void(GamePeer::*)(const luabind::object&))&GamePeer::LOnSessionStart)
			.def("on_session_start", (void(GamePeer::*)(const std::string&, const luabind::object&))&GamePeer::LOnSessionStart)
			.def("start_practice", (void(GamePeer::*)(const std::string&))&GamePeer::LStartPractice)
			.def("start_practice", (void(GamePeer::*)(const std::string&, const luabind::object&))&GamePeer::LStartPractice)
	];
}

/**
 * Executes all "on_init" handlers.
 * This should be called just after the main game window is visible.
 */
void GamePeer::OnInit()
{
	initialized = true;

	onInit.CallHandlers();
}

/**
 * Executes all "on_session_start" handlers.
 * This should be called at the start of every session, after the track script
 * has been executed.
 * @param sessionPeer The session peer.
 */
void GamePeer::OnSessionStart(SessionPeerPtr sessionPeer)
{
	luabind::object sessionObj(scripting->GetState(), sessionPeer);
	onSessionStart.CallHandlers(sessionObj);
}

/**
 * Check if the last script execution requested a new session.
 * After this is called, subsequent calls will return @c NULL until a script
 * requests a new session.
 * @return @c NULL if no new session was requested, a pointer to the selected
 *         rules otherwise.
 */
RulebookPtr GamePeer::RequestedNewSession()
{
	RulebookPtr retv;
	retv.swap(deferredStart);
	return retv;
}

bool GamePeer::LIsInitialized()
{
	return initialized;
}

ConfigPeer *GamePeer::LGetConfig()
{
	// function get_config()
	// Returns the game configuration, so it can be modified.
	return new ConfigPeer(scripting);
}

void GamePeer::LOnInit(const luabind::object &fn)
{
	onInit.AddHandler(fn);
}

void GamePeer::LOnInit(const std::string &name, const luabind::object &fn)
{
	onInit.AddHandler(name, fn);
}

void GamePeer::LOnSessionStart(const luabind::object &fn)
{
	onSessionStart.AddHandler(fn);
}

void GamePeer::LOnSessionStart(const std::string &name, const luabind::object &fn)
{
	onSessionStart.AddHandler(name, fn);
}

void GamePeer::LStartPractice(const std::string &track)
{
	// function start_practice(track)
	// Start a new single-player practice session with default rules.
	//   track - The track name to load (e.g. "ClassicH").
	using namespace luabind;
	lua_State *L = scripting->GetState();
	lua_pushnil(L);
	object nilobj(from_stack(L, -1));
	LStartPractice(track, nilobj);
	lua_pop(L, 1);
}

void GamePeer::LStartPractice(const std::string &track, const luabind::object &rules)
{
	// function start_practice(track, rules)
	// Start a new single-player practice session.
	//   track - The track name to load (e.g. "ClassicH").
	//   rules - Table listing the rules for the session:
	//             laps - Number of laps (between 1 and 99, inclusive).
	using namespace luabind;

	lua_State *L = scripting->GetState();

	if (!initialized) {
		luaL_error(L, "Game is not yet initialized.");
		return;
	}

	bool hasExtension = boost::ends_with(track, Config::TRACK_EXT);

	int laps = 5;

	int rulesParamType = type(rules);
	if (rulesParamType != LUA_TNIL) {
		if (rulesParamType == LUA_TTABLE) {
			try {
				laps = boost::lexical_cast<int>(rules["laps"]);
				if (laps < 1) laps = 1;
				else if (laps > 99) laps = 99;
			}
			catch (boost::bad_lexical_cast&) { }
		}
		else {
			luaL_error(L, "Expected table or nil as second parameter.");
			return;
		}
	}

	//TODO: Check that the track actually exists and throw an error otherwise.

	// We can't safely start a new session while handlers are still executing
	// since a new thread will be spawned which will likely also try to execute
	// a script concurrently, leading to undefined behavior.
	// So, instead we defer the actual spawning of the new session until after
	// the handlers are finished.
	deferredStart = boost::make_shared<Rulebook>(
		hasExtension ? track : (track + Config::TRACK_EXT),
		laps,
		0x7f);
}

}  // namespace Client
}  // namespace HoverRace
