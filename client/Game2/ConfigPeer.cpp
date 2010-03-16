
// ConfigPeer.cpp
// Scripting peer for access to the game configuration.
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

#include "../../engine/Script/Core.h"
#include "../../engine/Util/Config.h"

#include "ConfigPeer.h"

using HoverRace::Util::Config;

namespace HoverRace {
namespace Client {

ConfigPeer::ConfigPeer(Script::Core *scripting) :
	scripting(scripting)
{
}

ConfigPeer::~ConfigPeer()
{
}

/**
 * Register this peer in an environment.
 */
void ConfigPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	open(L);

	module(L) [
		class_<ConfigPeer>("Config")
			.def("unlink", &ConfigPeer::LUnlink)
			.def("get_video_res", &ConfigPeer::LGetVideoRes)
			.def("set_video_res", &ConfigPeer::LSetVideoRes)
	];
}

void ConfigPeer::LUnlink()
{
	// function unlink()
	// Unlinks the configuration from the configuration file.
	// This prevents changes to the configuration from being saved, useful for
	// unit tests.
	Config *cfg = Config::GetInstance();
	cfg->SetUnlinked(true);
}

void ConfigPeer::LGetVideoRes()
{
	// function width, height = get_video_res()
	// Returns the width and height of the game window when in windowed mode.
	Config *cfg = Config::GetInstance();
	lua_State *L = scripting->GetState();
	lua_pushnumber(L, cfg->video.xRes);
	lua_pushnumber(L, cfg->video.yRes);
}

void ConfigPeer::LSetVideoRes(int w, int h)
{
	// function set_video_res(width, height)
	// Set the width and height of the game window.
	// This is only effective if called before "on_init" is fired (since the window
	// is already visible by then).
	Config *cfg = Config::GetInstance();
	cfg->video.xRes = w;
	cfg->video.yRes = h;
}

}  // namespace Client
}  // namespace HoverRace
