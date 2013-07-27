
// DebugPeer.cpp
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

#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/OS.h"
#include "../../../engine/Util/Str.h"
#include "../GameDirector.h"
#include "../PaletteScene.h"
#include "../TestLabScene.h"

#include "DebugPeer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

DebugPeer::DebugPeer(Script::Core *scripting, GameDirector &gameDirector) :
	SUPER(scripting, "Debug"),
	gameDirector(gameDirector)
{
}

DebugPeer::~DebugPeer()
{
}

/**
 * Register this peer in an environment.
 */
void DebugPeer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L) [
		class_<DebugPeer, SUPER, std::shared_ptr<DebugPeer>>("Debug")
			.def("open_link", &DebugPeer::LOpenLink)
			.def("open_path", &DebugPeer::LOpenPath)
			.def("show_palette", &DebugPeer::LShowPalette)
			.def("start_test_lab", &DebugPeer::LStartTestLab)
	];
}

void DebugPeer::LOpenLink(const std::string &url)
{
	OS::OpenLink(url);
}

void DebugPeer::LOpenPath(const std::string &path)
{
	OS::OpenPath(Str::UP(path));
}

void DebugPeer::LShowPalette()
{
	gameDirector.RequestPushScene(std::make_shared<PaletteScene>(gameDirector));
}

void DebugPeer::LStartTestLab()
{
	gameDirector.RequestReplaceScene(std::make_shared<TestLabScene>(*gameDirector.GetDisplay(), gameDirector));
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace