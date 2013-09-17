
// RulebookEnv.cpp
//
// Copyright (c) 2013 Michael Imamura.
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

RulebookEnv::RulebookEnv(Script::Core *scripting,
                         RulebookLibrary &rulebookLibrary) :
	SUPER(scripting),
	rulebookLibrary(rulebookLibrary)
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

	lua_pushlightuserdata(L, this);  // table this
	lua_pushcclosure(L, RulebookEnv::LRulebook, 1);  // table fn
	lua_pushstring(L, "Rulebook");  // table fn str
	lua_insert(L, -2);  // table str fn
	lua_rawset(L, -3);  // table
}

/**
 * Run all rulebook scripts to re-populate the rulebook library.
 */
void RulebookEnv::ReloadRulebooks()
{
	Config *cfg = Config::GetInstance();

	OS::path_t dir = cfg->GetMediaPath();
	dir /= Str::UP("rulebooks");

	if (!fs::exists(dir)) {
		Log::Error("Rulebook path does not exist: %s",
			(const char*)Str::PU(dir));
		return;
	}
	if (!fs::is_directory(dir)) {
		Log::Error("Rulebook path is not a directory (skipping): %s",
			(const char*)Str::PU(dir));
		return;
	}

	int rulebooksLoaded = 0;
	const OS::dirIter_t END;
	for (OS::dirIter_t iter(dir); iter != END; ++iter) {
		const OS::path_t &path = iter->path();
		if (path.extension() == ".lua") {
			Log::Info("Running: %s", (const char*)Str::PU(path));
			if (RunScript(path)) rulebooksLoaded++;
		}
	}

	if (rulebooksLoaded == 0) {
		Log::Error("Rulebook path contains no rulebooks: %s",
			(const char*)Str::PU(dir));
	}
	else {
		Log::Info("Loaded %d rulebook(s): %s",
			rulebooksLoaded,
			(const char*)Str::PU(dir));
	}
}

void RulebookEnv::DefineRulebook(const luabind::object &defn)
{
	using namespace luabind;

	Script::Core *scripting = GetScripting();
	lua_State *L = scripting->GetState();

	if (type(defn) != LUA_TTABLE) {
		luaL_error(L, "Expected table.");
		return;
	}

	const object &nameObj = defn["name"];
	if (type(nameObj) != LUA_TSTRING) {
		luaL_error(L, "'name' is required to be a string.");
		return;
	}
	const std::string name = object_cast<std::string>(nameObj);

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

	auto rulebook = std::make_shared<Rulebook>(scripting, name, desc);
	rulebook->SetOnPreGame(ExpectHandler(scripting, defn, "on_pre_game"));
	rulebook->SetOnPostGame(ExpectHandler(scripting, defn, "on_post_game"));

	rulebookLibrary.Add(rulebook);

	Log::Info("Registered: %s, %s", name.c_str(), desc.c_str());
}

int RulebookEnv::LRulebook(lua_State *L)
{
	// function Rulebook(defn)
	// Defines a new rulebook.
	//   defn - A table defining the rulebook:
	//            name - The name (will be used to create instances).
	//            description - (Optional) The one-line description.
	//            on_pre_game - (Optional) Function to call before the session starts.
	//            on_post_game - (Optional) Function to call after the session ends.
	using namespace luabind;

	RulebookEnv *self = static_cast<RulebookEnv*>(lua_touserdata(L, lua_upvalueindex(1)));
	self->DefineRulebook(object(from_stack(L, 1)));
	return 0;
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
