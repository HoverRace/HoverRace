
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

#include "GamePeer.h"

#include "RulebookEnv.h"

using namespace HoverRace::Script;
using namespace HoverRace::Util;
namespace fs = boost::filesystem;

namespace HoverRace {
namespace Client {
namespace HoverScript {

RulebookEnv::RulebookEnv(Script::Core *scripting, GamePeer *gamePeer,
                         RulebookLibrary &rulebookLibrary) :
	SUPER(scripting),
	gamePeer(gamePeer), rulebookLibrary(rulebookLibrary)
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

	//TODO: Define top-level "Rulebook" function instead of going through GamePeer.
	object env(from_stack(L, -1));
	env["game"] = gamePeer;
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

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
