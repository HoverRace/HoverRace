
// Rulebook.cpp
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

#include "../../engine/Script/Core.h"
#include "../../engine/Util/Log.h"
#include "../../engine/Util/Str.h"

#include "HoverScript/PlayerPeer.h"
#include "HoverScript/RulebookEnv.h"
#include "HoverScript/SessionPeer.h"
#include "ClientSession.h"

#include "Rulebook.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {
	class ConstantRule : public Rule
	{
		typedef Rule SUPER;
		public:
			ConstantRule(const luabind::object &val) : SUPER(), val(val) { }
			virtual ~ConstantRule() { }

		public:
			virtual luabind::object GetDefault() const { return val; }

		private:
			luabind::object val;
	};
}

Rulebook::Rulebook(Script::Core *scripting, const Util::OS::path_t &basePath) :
	scripting(scripting),
	basePath(basePath),
	defaultName((const char*)Str::PU(basePath.filename())),
	name(defaultName), title(defaultName), description(),
	maxPlayers(0),
	rules(),
	onLoad(scripting),
	onPreGame(scripting), onPostGame(scripting),
	onPlayerJoined(scripting),
	metas(scripting),
	loaded(false)
{
	env = std::make_shared<HoverScript::RulebookEnv>(scripting, basePath, *this);
}

Rulebook::~Rulebook()
{
}

void Rulebook::AddRule(const std::string &name, const luabind::object &obj)
{
	rules.insert(rules_t::value_type(name, std::make_shared<ConstantRule>(obj)));
}

luabind::object Rulebook::CreateDefaultRules() const
{
	luabind::object obj = luabind::newtable(scripting->GetState());

	for (auto ent : rules) {
		obj[ent.first] = ent.second->GetDefault();
	}

	return obj;
}

/**
 * Load just enough of the rulebook to retrieve the metadata (title, etc.).
 *
 * In practice, this just runs the "bootstrap" script in the rulebook directory.
 * If an error occurred while running the script, then this rulebook's metadata
 * will be reset to their default values.
 *
 * @return @c true If the rulebook loaded successfully, @c false otherwise.
 */
bool Rulebook::LoadMetadata()
{
	bool retv = env->RunRulebookScript();
	if (!retv) {
		name = defaultName;
		title = defaultName;
		description.clear();
		loaded = false;
	}
	return retv;
}

/**
 * Ensure that the rulebook is fully-loaded.
 *
 * Initially, the rulebook is only partially loaded for speed.
 * This function must be called before a game session starts.
 */
void Rulebook::Load() const
{
	if (!loaded) {
		OnLoad();
		loaded = true;
	}
}

void Rulebook::SetOnLoad(const luabind::object &fn)
{
	using namespace luabind;

	lua_State *L = scripting->GetState();

	switch (type(fn)) {
		case LUA_TFUNCTION:
			onLoad = fn;
			break;
		case LUA_TNIL:
			onLoad.Clear();
			break;
		default:
			luaL_error(L, "Expected: (function)");
	}
}

void Rulebook::OnLoad() const
{
	lua_State *L = scripting->GetState();

	// Initial stack: (empty)

	if (onLoad) {
		metas_t metas(scripting);

		// Call the function.
		onLoad.Push();  // fn
		scripting->Invoke(0, nullptr, [&](lua_State *L, int num) {
			using namespace luabind;

			if (num == 0) {
				Log::Warn("on_init function did not return a table.");
				return 0;
			}
			else if (num > 1) {
				Log::Warn("Ignoring extra return values from on_init function.");
			}

			// fn (return values)
			object classes(from_stack(L, -num));

			if (type(classes) != LUA_TTABLE) {
				Log::Error("on_init function did not return a table.");
				lua_pop(L, num);
				return 0;
			}

			object playerFac(classes["player"]);

			switch (type(playerFac)) {
				case LUA_TFUNCTION:
				case LUA_TUSERDATA:
					metas.player = playerFac;
					break;
				case LUA_TNIL:
					// Ignore.
					break;
				default:
					Log::Warn("\"player\" is not a function or constructor.");
			}

			lua_pop(L, num);
			return 0;
		});

		// If we got this far without a ScriptExn, then we're ready to set
		// the factories.
		this->metas = std::move(metas);
	}
}

void Rulebook::SetOnPreGame(const luabind::object &fn)
{
	onPreGame.AddHandler(fn);
}

void Rulebook::OnPreGame(HoverScript::SessionPeerPtr session) const
{
	onPreGame.CallHandlers(luabind::object(scripting->GetState(), session));
}

void Rulebook::SetOnPostGame(const luabind::object &fn)
{
	onPostGame.AddHandler(fn);
}

void Rulebook::OnPostGame(HoverScript::SessionPeerPtr session) const
{
	onPostGame.CallHandlers(luabind::object(scripting->GetState(), session));
}

void Rulebook::SetOnPlayerJoined(const luabind::object &fn)
{
	onPlayerJoined.AddHandler(fn);
}

void Rulebook::OnPlayerJoined(HoverScript::SessionPeerPtr session,
                            std::shared_ptr<HoverScript::PlayerPeer> player) const
{
	lua_State *L = scripting->GetState();
	onPlayerJoined.CallHandlers(luabind::object(L, session),
		luabind::object(L, player));
}

}  // namespace Client
}  // namespace HoverRace

