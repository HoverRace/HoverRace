
// GamePeer.cpp
//
// Copyright (c) 2010, 2013-2016 Michael Imamura.
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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <luabind/adopt_policy.hpp>

#include "../../../engine/Display/Display.h"
#include "../../../engine/Parcel/TrackBundle.h"
#include "../../../engine/Player/DemoProfile.h"
#include "../../../engine/Player/LocalPlayer.h"
#include "../../../engine/Player/ProfileGallery.h"
#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/Config.h"
#include "../../../engine/Util/Log.h"
#include "../GameDirector.h"
#include "../MessageAnnouncement.h"
#include "../Roster.h"
#include "../Rulebook.h"
#include "../RulebookLibrary.h"
#include "../Rules.h"
#include "ConfigPeer.h"
#include "SessionPeer.h"

#include "GamePeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

GamePeer::GamePeer(Script::Core &scripting, GameDirector &director,
	RulebookLibrary &rulebookLibrary) :
	SUPER(scripting, "Game"),
	director(director), rulebookLibrary(rulebookLibrary), display(nullptr),
	initialized(false),
	onInit(scripting), onShutdown(scripting),
	onSessionStart(scripting), onSessionEnd(scripting)
{
}

/**
 * Register this peer in an environment.
 * @param scripting The target environment.
 */
void GamePeer::Register(Script::Core &scripting)
{
	using namespace luabind;
	lua_State *L = scripting.GetState();

	module(L) [
		class_<GamePeer,SUPER,std::shared_ptr<GamePeer>>("Game")
			.def("add_local_player", &GamePeer::LAddLocalPlayer)
			.def("add_local_player", &GamePeer::LAddLocalPlayer_N)
			.def("announce", &GamePeer::LAnnounce)
			.def("announce", &GamePeer::LAnnounce_T)
			.def("is_initialized", &GamePeer::LIsInitialized)
			.def("get_config", &GamePeer::LGetConfig, adopt(result))
			.def("on_init", &GamePeer::LOnInit)
			.def("on_init", &GamePeer::LOnInit_N)
			.def("on_shutdown", &GamePeer::LOnShutdown)
			.def("on_shutdown", &GamePeer::LOnShutdown_N)
			.def("on_session_begin", &GamePeer::LOnSessionBegin)
			.def("on_session_begin", &GamePeer::LOnSessionBegin_N)
			.def("on_session_end", &GamePeer::LOnSessionEnd)
			.def("on_session_end", &GamePeer::LOnSessionEnd_N)
			.def("screenshot", &GamePeer::LScreenshot)
			.def("start_main_menu", &GamePeer::LStartMenuMenu)
			.def("start_practice", &GamePeer::LStartPractice)
			.def("start_practice", &GamePeer::LStartPractice_O)
			.def("start_practice", &GamePeer::LStartPractice_RO)
			.def("shutdown", &GamePeer::LShutdown)
			.def("toggle_hud", &GamePeer::LToggleHud)
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
 * Executes all "on_shutdown" handlers.
 * This is called just before the game window closes.
 */
void GamePeer::OnShutdown()
{
	initialized = false;

	onShutdown.CallHandlers();
}

/**
 * Executes all "on_session_start" handlers.
 * This should be called at the start of every session, after the track script
 * has been executed.
 * @param sessionPeer The session peer.
 */
void GamePeer::OnSessionStart(std::shared_ptr<SessionPeer> sessionPeer)
{
	luabind::object sessionObj(GetScripting().GetState(), sessionPeer);
	onSessionStart.CallHandlers(sessionObj);
}

/**
 * Executes all "on_session_end" handlers.
 * This is called at the end of every session, before the game thread is
 * shut down.
 * @param sessionPeer The session peer.
 */
void GamePeer::OnSessionEnd(std::shared_ptr<SessionPeer> sessionPeer)
{
	luabind::object sessionObj(GetScripting().GetState(), sessionPeer);
	onSessionEnd.CallHandlers(sessionObj);
}

void GamePeer::VerifyInitialized() const
{
	if (!initialized) {
		luaL_error(GetScripting().GetState(), "Game is not yet initialized.");
		return;
	}
}

void GamePeer::LAddLocalPlayer()
{
	auto party = director.GetParty();
	if (!party) {
		luaL_error(GetScripting().GetState(), "Not ready to connect players.");
		return;
	}

	party->AddPlayer(
		std::make_shared<Player::LocalPlayer>(
			std::make_shared<Player::DemoProfile>(
				director.ShareAvatarGallery()), true, true));
}

void GamePeer::LAddLocalPlayer_N(const std::string &name)
{
	auto party = director.GetParty();
	if (!party) {
		luaL_error(GetScripting().GetState(), "Not ready to connect players.");
		return;
	}

	auto profile =
		Player::ProfileGallery(director.ShareAvatarGallery()).FindName(name);
	if (!profile) {
		luaL_error(GetScripting().GetState(),
			"Profile not found: %s", name.c_str());
		return;
	}

	party->AddPlayer(
		std::make_shared<Player::LocalPlayer>(profile, true, true));
}

void GamePeer::LAnnounce(const std::string &label)
{
	LAnnounce_T(label, "");
}

void GamePeer::LAnnounce_T(const std::string &label, const std::string &text)
{
	director.RequestAnnouncement(
		std::make_shared<MessageAnnouncement>(label, text));
}

bool GamePeer::LIsInitialized()
{
	return initialized;
}

ConfigPeer *GamePeer::LGetConfig()
{
	// function get_config()
	// Returns the game configuration, so it can be modified.
	return new ConfigPeer(GetScripting());
}

void GamePeer::LOnInit(const luabind::object &fn)
{
	onInit.AddHandler(fn);
}

void GamePeer::LOnInit_N(const std::string &name, const luabind::object &fn)
{
	onInit.AddHandler(name, fn);
}

void GamePeer::LOnShutdown(const luabind::object &fn)
{
	onShutdown.AddHandler(fn);
}

void GamePeer::LOnShutdown_N(const std::string &name, const luabind::object &fn)
{
	onShutdown.AddHandler(name, fn);
}

void GamePeer::LOnSessionBegin(const luabind::object &fn)
{
	onSessionStart.AddHandler(fn);
}

void GamePeer::LOnSessionBegin_N(const std::string &name, const luabind::object &fn)
{
	onSessionStart.AddHandler(name, fn);
}

void GamePeer::LOnSessionEnd(const luabind::object &fn)
{
	onSessionEnd.AddHandler(fn);
}

void GamePeer::LOnSessionEnd_N(const std::string &name, const luabind::object &fn)
{
	onSessionEnd.AddHandler(name, fn);
}

void GamePeer::LScreenshot()
{
	if (!display) {
		luaL_error(GetScripting().GetState(),
			"Cannot take screenshot because window hasn't been created yet.");
		return;
	}

	display->Screenshot();
}

void GamePeer::LStartMenuMenu()
{
	director.RequestMainMenu();
}

void GamePeer::LStartPractice(const std::string &track)
{
	// function start_practice(track)
	// Start a new single-player practice session with default rules.
	//   track - The track name to load (e.g. "ClassicH").
	using namespace luabind;
	lua_State *L = GetScripting().GetState();
	lua_pushnil(L);
	object nilobj(from_stack(L, -1));
	LStartPractice_RO(track, "", nilobj);
	lua_pop(L, 1);
}

void GamePeer::LStartPractice_O(const std::string &track,
	const luabind::object &opts)
{
	// This actually represents two different overloads, depending on the type
	// of the second parameter.

	using namespace luabind;

	if (type(opts) == LUA_TSTRING) {
		// function start_practice(track, rulebook)
		// Start a new single-player practice session with default rules.
		//   track - The track name to load (e.g. "ClassicH").
		//   rulebook - The rulebook name (e.g. "Race").
		lua_State *L = GetScripting().GetState();
		lua_pushnil(L);
		object nilobj(from_stack(L, -1));
		LStartPractice_RO(track, object_cast<std::string>(opts), nilobj);
		lua_pop(L, 1);
	}
	else {
		// function start_practice(track, opts)
		// Start a new single-player practice session with default rules.
		//   track - The track name to load (e.g. "ClassicH").
		//   opts - Table listing the rulebook options for the session:
		//             laps - Number of laps (between 1 and 99, inclusive).
		LStartPractice_RO(track, "", opts);
	}

}

void GamePeer::LStartPractice_RO(const std::string &track,
	const std::string &rulebookName, const luabind::object &opts)
{
	// function start_practice(track, rulebook, opts)
	// Start a new single-player practice session.
	//   track - The track name to load (e.g. "ClassicH").
	//   rulebook - The rulebook name (e.g. "Race").
	//   opts - Table listing the rulebook options for the session:
	//             laps - Number of laps (between 1 and 99, inclusive).
	using namespace luabind;

	VerifyInitialized();

	lua_State *L = GetScripting().GetState();

	bool hasExtension = boost::ends_with(track, Config::TRACK_EXT);

	int laps = 5;

	int rulesParamType = type(opts);
	if (rulesParamType != LUA_TNIL) {
		if (rulesParamType == LUA_TTABLE) {
			try {
				laps = boost::lexical_cast<int>(opts["laps"]);
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

	auto rulebook = rulebookName.empty() ?
		rulebookLibrary.GetDefault() :
		rulebookLibrary.Find(rulebookName);
	if (!rulebook) {
		luaL_error(L, "Rulebook '%s' does not exist.", rulebookName.c_str());
		return;
	}
	auto rules = std::make_shared<Rules>(rulebook);
	rules->SetTrackEntry(Config::GetInstance()->GetTrackBundle().OpenTrackEntry(
		hasExtension ? track : (track + Config::TRACK_EXT)));
	rules->SetLaps(laps);

	director.RequestNewPracticeSession(rules);
}

void GamePeer::LShutdown()
{
	// function shutdown()
	// Request an orderly shutdown of the game.
	// The shutdown may not be immediate; the game will try to shut down all
	// systems in an orderly fashion.

	VerifyInitialized();

	director.RequestShutdown();
}

bool GamePeer::LToggleHud()
{
	// function toggle_hud()
	// Enable or disable the in-game HUD.
	auto &enabled = Config::GetInstance()->runtime.enableHud;
	return (enabled = !enabled);
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace

