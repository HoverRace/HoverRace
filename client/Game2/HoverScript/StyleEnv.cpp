
// StyleEnv.cpp
//
// Copyright (c) 2014 Michael Imamura.
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

#include "../../StdAfx.h"

#include "../../../engine/Display/Display.h"
#include "../../../engine/Util/Log.h"
#include "../../../engine/Util/Str.h"

#include "StyleEnv.h"

using namespace HoverRace::Script;
using namespace HoverRace::Util;
namespace fs = boost::filesystem;

namespace HoverRace {
namespace Client {
namespace HoverScript {

/**
 * Constructor.
 *
 * The root directory of the rulebook is where the rulebook scripts will be
 * found; a rulebook is not allowed to access any scripts outside of its
 * root directory.
 *
 * @param scripting The scripting context.
 * @param basePath The root directory of the rulebook.
 * @param rulebook The rulebook bound to this environment.
 */
StyleEnv::StyleEnv(Script::Core *scripting, Display::Display &display,
	const Util::OS::path_t &basePath) :
	SUPER(scripting),
	display(display), basePath(basePath)
{
}

void StyleEnv::InitEnv()
{
	using namespace luabind;

	SUPER::InitEnv();

	lua_State *L = GetScripting()->GetState();

	// Start with the standard global environment.
	CopyGlobals();

	object env(from_stack(L, -1));
	env["styles"] = &display.styles;
}

/**
 * Run the stylesheet.
 */
bool StyleEnv::RunStylesheet()
{
	auto stylePath = basePath;
	stylePath /= Str::UP("styles.lua");

	if (!fs::exists(stylePath)) {
		HR_LOG(info) << "Stylesheet path does not have a styles.lua: " <<
			basePath;
		return false;
	}

	HR_LOG(info) << "Running: " << stylePath;

	return RunScript(stylePath);
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace
