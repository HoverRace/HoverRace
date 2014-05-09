
// RulebookEnv.cpp
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include <boost/filesystem.hpp>

#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/Config.h"
#include "../../../engine/Util/Log.h"
#include "../../../engine/Util/OS.h"
#include "../../../engine/Util/Str.h"

#include "../Rulebook.h"
#include "../RulebookLibrary.h"

#include "RulebookEnv.h"

using namespace HoverRace::Script;
using namespace HoverRace::Util;
namespace fs = boost::filesystem;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {
	const luabind::object ExpectHandler(Script::Core *scripting,
	                                     const luabind::object &props,
	                                     const char *name)
	{
		using namespace luabind;

		const object obj = props[name];

		int objType = type(obj);
		if (objType != LUA_TNIL && objType != LUA_TFUNCTION) {
			luaL_error(scripting->GetState(),
				"'%s' is required to be a function or nil", name);
			return *scripting->NIL;
		}

		return obj;
	}
}

/**
 * Constructor.
 *
 * The root directory of the rulebook is where the rulebook scripts will be
 * found; a rulebook is not allowed to access any scripts outside of its
 * root directory.
 *
 * @param scripting The scripting context.
 * @param rulebookLibrary The rulebook library in which to register new
 *                        rulebooks.
 * @param basePath The root directory of the rulebook.
 */
RulebookEnv::RulebookEnv(Script::Core *scripting,
                         RulebookLibrary &rulebookLibrary,
                         const Util::OS::path_t &basePath) :
	SUPER(scripting),
	rulebookLibrary(rulebookLibrary),
	basePath(basePath)
{
}

RulebookEnv::~RulebookEnv()
{
}

void RulebookEnv::InitEnv()
{
	using namespace luabind;

	SUPER::InitEnv();

	Script::Core *scripting = GetScripting();
	lua_State *L = scripting->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	// Register our custom rulebook-aware "require()".
	lua_pushlightuserdata(L, this);  // table this
	lua_pushcclosure(L, RulebookEnv::LRequire, 1);  // table fn
	lua_pushstring(L, "require");  // table fn str
	lua_insert(L, -2);  // table str fn
	lua_rawset(L, -3);  // table

	// Register our Rulebook factory.
	lua_pushlightuserdata(L, this);  // table this
	lua_pushcclosure(L, RulebookEnv::LRulebookStage1, 1);  // table fn
	lua_pushstring(L, "Rulebook");  // table fn str
	lua_insert(L, -2);  // table str fn
	lua_rawset(L, -3);  // table
}

void RulebookEnv::DefineRulebook(const std::string &name,
                                 const luabind::object &defn)
{
	using namespace luabind;

	Script::Core *scripting = GetScripting();
	lua_State *L = scripting->GetState();

	if (type(defn) != LUA_TTABLE) {
		luaL_error(L, "Expected table.");
		return;
	}

	const object &titleObj = defn["title"];
	if (type(titleObj) != LUA_TSTRING) {
		luaL_error(L, "'title' is required to be a string.");
		return;
	}
	const std::string title = object_cast<std::string>(titleObj);

	const object &descObj = defn["description"];
	std::string desc;
	switch (type(descObj)) {
		case LUA_TNIL:
			break;
		case LUA_TSTRING:
			desc = object_cast<std::string>(descObj);
			break;
		default:
			luaL_error(L, "Expected 'desc' to be a string.");
			return;
	}

	const object &maxPlayersObj = defn["max_players"];
	int maxPlayers = 4;
	switch (type(maxPlayersObj)) {
		case LUA_TNIL:
			break;
		case LUA_TNUMBER:
			maxPlayers = object_cast<int>(maxPlayersObj);
			break;
		default:
			luaL_error(L, "Expected 'max_players' to be a number.");
			return;
	}

	auto rulebook = std::make_shared<Rulebook>(scripting, basePath,
		name, title, desc, maxPlayers);

	const object &rulesObj = defn["rules"];
	switch (type(rulesObj)) {
		case LUA_TNIL:
			break;
		case LUA_TTABLE:
			DefineRules(rulebook, rulesObj);
			break;
		default:
			luaL_error(L, "Expected 'rules' to be a table.");
			return;
	}

	rulebook->SetOnPreGame(ExpectHandler(scripting, defn, "on_pre_game"));
	rulebook->SetOnPostGame(ExpectHandler(scripting, defn, "on_post_game"));
	rulebook->SetOnPlayerJoined(ExpectHandler(scripting, defn, "on_player_joined"));

	rulebookLibrary.Add(rulebook);

	Log::Info("Registered: %s: %s, %s", name.c_str(), title.c_str(), desc.c_str());
}

void RulebookEnv::DefineRules(std::shared_ptr<Rulebook> rulebook,
                              const luabind::object &rulesObj)
{
	using namespace luabind;

	for (iterator iter(rulesObj), end; iter != end; ++iter) {
		const std::string name = object_cast<std::string>(iter.key());
		const object ruleObj = *iter;

		//TODO: Check type of ruleObj and cast to Rule if necessary.
		rulebook->AddRule(name, ruleObj);

		Log::Info("Added rule '%s' with type %d.", name.c_str(),
			type(ruleObj));
	}
}

/**
 * Run the boot script for a rulebook.
 */
bool RulebookEnv::RunRulebookScript(const Util::OS::path_t &path)
{
	auto bootPath = path;
	bootPath /= Str::UP("rulebook.lua");

	if (!fs::exists(bootPath)) {
		Log::Info("Rulebook path does not have a rulebook.lua: %s",
			(const char*)Str::PU(path));
		return false;
	}

	Log::Info("Running: %s", (const char*)Str::PU(bootPath));

	return RunScript(bootPath);
}

int RulebookEnv::LRequire(lua_State *L)
{
	// require(module_name)
	//
	// Loads a module from the current rulebook base directory.
	//   module_name - The name of the module (the filename, minus the ".lua"
	//                 extension.
	//
	// Returns the return value of executing the script.

	RulebookEnv *self = static_cast<RulebookEnv*>(lua_touserdata(L, lua_upvalueindex(1)));

	const auto &basePath = self->basePath;

	if (basePath.empty()) {
		Log::Error("Rulebook require() called outside of Rulebook context.");
		return 0;
	}

	if (lua_gettop(L) != 1) {
		luaL_error(L, "Usage: require 'module_name'");
		return 0;
	}

	std::string name = lua_tostring(L, 1);
	name += ".lua";

	auto modulePath = basePath;
	modulePath /= Str::UP(name);

	Log::Info("Loading module '%s' from: %s", name.c_str(),
		(const char*)Str::PU(modulePath));

	//TODO

	return 0;
}

int RulebookEnv::LRulebookStage1(lua_State *L)
{
	// Rulebook name defn
	//
	// Defines a new rulebook.
	//   name - The name of the rulebook.
	//   defn - A table defining the rulebook:
	//            title - The title.
	//            description - (Optional) The one-line description.
	//            on_pre_game - (Optional) Function to call before the session starts.
	//            on_post_game - (Optional) Function to call after the session ends.

	if (lua_gettop(L) != 1) {
		luaL_error(L, "Usage: Rulebook 'name' { ... }");
		return 0;
	}

	// This is a little syntax trick to make rulebooks look declarative.
	// This stage captures the name of the rulebook, then returns the function
	// (stage 2) that will combine it with the object.
	// Inspired by Luabind's class definition mechanism.

	lua_pushvalue(L, lua_upvalueindex(1));  // name self
	lua_pushcclosure(L, &RulebookEnv::LRulebookStage2, 2);  // fn
	return 1;
}

int RulebookEnv::LRulebookStage2(lua_State *L)
{
	using namespace luabind;

	if (lua_gettop(L) != 1) {
		luaL_error(L, "Usage: Rulebook 'name' { ... }");
		return 0;
	}

	const char *name = lua_tostring(L, lua_upvalueindex(1));
	RulebookEnv *self = static_cast<RulebookEnv*>(lua_touserdata(L, lua_upvalueindex(2)));

	if (!name) {
		luaL_error(L, "Rulebook name must be a string.");
		return 0;
	}

	self->DefineRulebook(name, object(from_stack(L, 1)));

	return 0;
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
